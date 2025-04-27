#include "avars.h"
#include "decode_msg.h"
#include "input_parse.h"
#include "udp_send.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int iResult;

// Define the maximum number of events and the maximum input length
#define MAX_EVENTS 5
#define INPUT_LENGTH 1400

// Definition of variables to save text data for each of the tokens
char command[10], username[20], secret[128], display_name[20], channel_id[20],
    receive_buff[INPUT_LENGTH], msg[INPUT_LENGTH], read_buff[INPUT_LENGTH], msg_content[INPUT_LENGTH];

// Definitions of variables described in "avars.h"
struct addrinfo *result = NULL;
struct addrinfo hints;

struct sockaddr_in server_addr;
socklen_t server_addr_len;

int client_socket;

uint16_t message_id;
char *protocol = NULL;
char *host_ip = NULL;
uint16_t server_port = 4567;
uint16_t udp_timeout = 250;
uint8_t max_retransmissions = 3;

// End definition

// Definition of handler to catch the SIGINT signal for ending the connection to the server for both variants
void sigint_handler()
{
    if (strcmp(protocol, "tcp") == 0)
    {
        // sending BYE message for TCP
        sendto(client_socket, "BYE\r\n", 5, 0, (struct sockaddr *)&server_addr, server_addr_len);
    }
    else
    {
        // constructing BYE message for UDP
        char bye_msg[3];
        bye_msg[0] = 0xff;
        *((uint16_t *)(bye_msg + 1)) = htons(message_id);

        // sending BYE message
        udp_send(bye_msg, receive_buff, 3);
    }

    // Freeing all pointers
    free(result);
    close(client_socket);
    exit(0);
}

// Definition of a function that parses program arguments
void argument_parse(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
    {
        switch (opt)
        {
        case 't':
            protocol = optarg;
            break;
        case 's':
            host_ip = optarg;
            break;
        case 'p':
            server_port = atoi(optarg);
            break;
        case 'd':
            udp_timeout = atoi(optarg);
            break;
        case 'r':
            max_retransmissions = atoi(optarg);
            break;
        case 'h':
            printf("Usage: %s [-t tcp|udp] [-s server_ip] [-p server_port] [-d udp_timeout] [-r max_retransmissions] [-h]\n", argv[0]);
            exit(EXIT_SUCCESS);
        case '?':
            if (optopt == 't' || optopt == 's' || optopt == 'p' || optopt == 'd' || optopt == 'r')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            abort();
        }
    }

    // Check if required options are provided
    if (protocol == NULL || host_ip == NULL)
    {
        fprintf(stderr, "Options -t and -s are required.\n");
        exit(EXIT_FAILURE);
    }
    return;
}

/* Definition of two submain functions for both variants "UDP" and "TCP"
   This function waits for an event (on STDIN descriptor or socket descriptor) and provides actions:
   - For STDIN event:
     * Parsing arguments
     * Checking if it supports the command for the current state
     * Sending a message
   - For SOCKET event:
     * Receiving a message
     * Decoding the message
     * Printing the message (optionally taking actions in case of ERR or BYE)
   This function returns the FSM event, which will provide later functionality in main.
*/
//-------------------------------UDP--------------------------
eSystemEvent ReadEvent(eSystemState state, int epoll_fd, struct epoll_event *events, int max_events,
                       int client_socket)
{
    if (state == Error_State)
    {
        return null_Event;
    }

    // Declaration of FSM events
    eSystemEvent input_event, incoming_event = null_Event;

    memset(msg, 0, sizeof(msg));
    int msg_length = 0;
    char server_username[20];
    memset(server_username, 0, sizeof(server_username));

    // Wait for an event
    iResult = epoll_wait(epoll_fd, events, max_events, -1);

    if (events[0].data.fd == 0)
    {
        // Case for STDIN event
        input_event = input_parse(command, username, secret, display_name, channel_id, read_buff, msg, &msg_length);

        // Check if input event is supported for the current state
        if ((state == Auth_State && input_event != auth_Event) || (state == Open_State && input_event == auth_Event))
        {
            // fprintf(stderr, "Client: Your command is not supported by this state\n");
            return null_Event;
        }
        if (input_event == null_Event)
            return null_Event;

        // Sending message
        incoming_event = udp_send(msg, receive_buff, msg_length);
        memset(receive_buff, 0, sizeof(receive_buff));
    }
    else
    {
        // Case for SOCKET event
        memset(receive_buff, 0, sizeof(receive_buff));

        // Receiving message
        if (recvfrom(client_socket, receive_buff, INPUT_LENGTH, 0, (struct sockaddr *)&server_addr, &server_addr_len) == -1)
        {
            perror("recvfrom");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        // Decoding message
        incoming_event = decode_msg(receive_buff, server_username, msg_content);

        // Switch that decides what type of message was received and optionally takes actions
        switch (incoming_event)
        {
        case reply_ok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            return incoming_event;
        case reply_nok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            return incoming_event;
        case error_inc_Event:
            fprintf(stderr, "ERR FROM %s: %s\n", server_username, msg_content);
            return incoming_event;
        case bye_Event:
            return incoming_event;
        case msg_error_Event:
            fprintf(stderr, "\nERR: Failed to be parsed\n");

            // Construct ERR message
            char err_msg[20];
            memset(err_msg, 0, sizeof(err_msg));
            err_msg[0] = 0xfe;
            *((uint16_t *)(err_msg + 1)) = htons(message_id);
            *err_msg += 3;
            strcpy(err_msg, "Failed to be parsed");

            // Send ERR message
            udp_send(err_msg, receive_buff, 20);

            return incoming_event;
        default:
            printf("%s: %s\n", server_username, msg_content);
        }
    }
    return incoming_event;
}

//--------------------------------TCP--------------------------------------

eSystemEvent ReadEventTcp(eSystemState state, int epoll_fd, struct epoll_event *events, int max_events,
                          int client_socket)
{
    if (state == Error_State)
    {
        return null_Event;
    }

    // Declaration of FSM events
    eSystemEvent input_event, incoming_event = null_Event;

    memset(msg, 0, sizeof(msg));
    int msg_length = 0;
    char server_username[20];

    // Wait for an event
    iResult = epoll_wait(epoll_fd, events, max_events, -1);

    if (events[0].data.fd == 0)
    {
        // Case for STDIN event
        input_event = input_parse_tcp(command, username, secret, display_name, channel_id, read_buff, msg, &msg_length);

        // Check if input event is supported for the current state
        if ((state == Auth_State && input_event != auth_Event) || (state == Open_State && input_event == auth_Event))
        {
            // fprintf(stderr, "ERR: Your command is not supported by this state\n");
            return null_Event;
        }
        if (input_event == null_Event)
            return null_Event;

        // Sending message
        iResult = sendto(client_socket, msg, msg_length, 0, (struct sockaddr *)&server_addr, server_addr_len);
        if (iResult <= 0)
        {
            perror("send failed");
            return -1;
        }
    }
    else
    {
        // Case for SOCKET event
        memset(receive_buff, 0, sizeof(receive_buff));

        // Receiving message
        if (recv(client_socket, receive_buff, INPUT_LENGTH, 0) == -1)
        {
            perror("recvfrom");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        // Decoding message
        incoming_event = decode_msg_tcp(receive_buff, server_username, msg_content);

        // Switch that decides what type of message was received and optionally takes actions
        switch (incoming_event)
        {
        case reply_ok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            return incoming_event;
        case reply_nok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            return incoming_event;
        case error_inc_Event:
            fprintf(stderr, "ERR FROM %s: %s\n", server_username, msg_content);
            return incoming_event;
        case bye_Event:
            return incoming_event;
        case msg_error_Event:
            fprintf(stderr, "ERR: Failed to be parsed\n");

            // Construct ERR message
            char err_msg[55];
            memset(err_msg, 0, sizeof(err_msg));
            strcat(err_msg, "ERR_FROM ");
            strcat(err_msg, display_name);
            strcat(err_msg, " IS ");
            strcat(err_msg, "Failed to be parsed\r\n");
            msg_length = strlen(err_msg);

            // Send ERR message
            iResult = sendto(client_socket, err_msg, msg_length, 0, (struct sockaddr *)&server_addr, server_addr_len);
            if (iResult <= 0)
            {
                perror("send failed");
                return null_Event;
            }

            return incoming_event;
        default:
            printf("%s: %s\n", server_username, msg_content);
        }
    }
    return incoming_event;
}

int main(int argc, char **argv)
{
    eSystemState eNextState = Auth_State;
    eSystemEvent eNewEvent;
    argument_parse(argc, argv); // Parsing arguments

    // Definition of structure to use option timeout with socket
    struct timeval timeout;
    timeout.tv_usec = udp_timeout * 1000;
    timeout.tv_sec = 0;

    // Cases to define hints to function "getaddrinfo();" for both variants, and optionally exit program
    memset(&hints, 0, sizeof(struct addrinfo));
    if (strcmp(protocol, "udp") == 0)
    {
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
    }
    else if (strcmp(protocol, "tcp") == 0)
    {
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else
    {
        fprintf(stderr, "Wrong protocol provided\n");
        exit(EXIT_FAILURE);
    }

    // Resolve hostname
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%hu", server_port);
    iResult = getaddrinfo(host_ip, port_str, &hints, &result);
    if (iResult != 0)
    {
        perror("GetAddrinfo");
        return -1;
    }

    // Initialization of socket, and adding timeout
    client_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (client_socket <= 0)
    {
        perror("ERROR: socket");
        return -1;
    }
    iResult = setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (iResult == -1)
        perror("sOPT");

    // Initialization of connection for TCP, or migrating data from addrinfo to sockaddr_in for UDP
    if (strcmp(protocol, "udp") == 0)
    {
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
        server_addr.sin_port = htons(server_port);
        server_addr_len = sizeof(server_addr);
    }
    else
    {
        iResult = connect(client_socket, result->ai_addr, result->ai_addrlen);
        if (iResult == -1)
        {
            perror("connect");
            close(client_socket);
            freeaddrinfo(result);
            return EXIT_FAILURE;
        }
    }

    // Initialization of epoll instance
    int epoll_fd = epoll_create1(0);
    assert(epoll_fd != -1);

    struct epoll_event sock_event;
    struct epoll_event stdin_event;

    sock_event.events = EPOLLIN;
    stdin_event.events = EPOLLIN;

    sock_event.data.fd = client_socket;
    stdin_event.data.fd = 0;

    iResult = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &sock_event);
    assert(iResult == 0);
    iResult = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &stdin_event);
    assert(iResult == 0);

    struct epoll_event events[MAX_EVENTS];

    // Initialization of SIGINT handler
    signal(SIGINT, sigint_handler);

    // Main cycle of program that provides FSM logic
    while (1)
    {
        // Read event function for both variants
        if (strcmp(protocol, "udp") == 0)
            eNewEvent = ReadEvent(eNextState, epoll_fd, events, MAX_EVENTS, client_socket);
        else
            eNewEvent = ReadEventTcp(eNextState, epoll_fd, events, MAX_EVENTS, client_socket);

        // Actions for each state
        switch (eNextState)
        {
        case Auth_State:
        {
            if (reply_ok_Event == eNewEvent)
            {
                eNextState = Open_State;
            }
            else if (reply_nok_Event == eNewEvent)
            {
            }
            else if (error_Event == eNewEvent || msg_error_Event == eNewEvent || error_inc_Event == eNewEvent)
            {
                eNextState = Error_State;
            }
            else if (bye_Event == eNewEvent)
            {
                close(client_socket);
                free(result);
                return 0;
            }

            break;
        }
        case Open_State:
        {

            if (auth_Event == eNewEvent)
                fprintf(stderr, "ERR: Your command is not supportet by this state\n");
            else if (error_Event == eNewEvent || msg_error_Event == eNewEvent || error_inc_Event == eNewEvent)
            {
                eNextState = Error_State;
            }
            else if (bye_Event == eNewEvent)
            {
                close(client_socket);
                free(result);
                return 0;
            }
            break;
        }
        case Error_State:
        {
            sigint_handler(2);
        }

        default:
        }
    }

    return 0;
}
