#include "udp_send.h"
#include "avars.h"
#include "decode_msg.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INPUT_LENGTH 1400

/* Definition of function to send UDP messages
    This funcion only exists because of unreleability of UDP protocol,
    to provide releability to UDP

    Function sending an message to server, an waits to confirm message, according to default or user-provided timeout
    and default or user-provided number of retries + 1

    In case of while functions waits for an confirm message, receives not condirm message, this message will be parsed and,
    some event will be assigned to "event" variable
    In that case function returns an event on the basis of which it will be decided, to take some actions or not

    Else, returns null_event and no action is taken

*/
eSystemEvent udp_send(char *msg, char *receive_buff, int msg_length)
{
    int i2Result;

    // Independent variable that controls number of messages, sended due one livetime of function
    int msg_out_ctrl = 0;


    char *buffer = msg;
    uint8_t cnt = max_retransmissions;
    uint16_t hMessage_id = htons(message_id);
    eSystemEvent event = null_Event;

    // First sending of message
    i2Result = sendto(client_socket, buffer, msg_length, 0, (struct sockaddr *)&server_addr, server_addr_len);
    msg_out_ctrl++;
    if (i2Result <= 0)
    {
        perror("send failed");
        return null_Event;
    }

    // Case if this function has sended a confirm type message, so no need to wait to confirmation to that confirmation
    if (buffer[0] == 0x00)
        return null_Event;


    // Waiting for response from server by timeout 
    i2Result = recv(client_socket, receive_buff, INPUT_LENGTH, 0);
    if (i2Result >= 0 && *((uint16_t *)(receive_buff + 1)) == hMessage_id && (receive_buff[0] == 0x00))
    {
        // If confirm message was received and refMessageID is matching sended MessageID, MessageID will be incremented and returned null_event 
        message_id++;
    }
    else if ((i2Result >= 0 && receive_buff[0] != 0x00))
    {
        // Case if received message is not confirm message

        // Simply decode that message and wait for confirm message
        char server_username[20];
        char msg_content[1400];
        memset(msg_content, 0, sizeof(msg_content));
        memset(server_username, 0 ,sizeof(server_username));
        event = decode_msg(receive_buff, server_username, msg_content);
        switch (event)
        {
        case reply_ok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            event =  reply_ok_Event;
            break;
        case reply_nok_Event:
            fprintf(stderr, "%s: %s\n", server_username, msg_content);
            event =  reply_nok_Event;
            break;
        case error_inc_Event:
            fprintf(stderr, "ERR FROM %s: %s\n", server_username, msg_content);
            event =  error_inc_Event;
            break;
        case bye_Event:
            event =  bye_Event;
            break;
        case msg_error_Event:
            fprintf(stderr, "Failed to be parsed\n");
            event =  msg_error_Event;
            break;
        default:
            printf("%s: %s\n", server_username, msg_content);
            break;
        }
            cnt++;
    }
    else
    {
        // Cycle for number of retries 
        while (cnt-- > 0)
        {
            // Check if number of sended messages is > 20
            if (cnt > 20 || msg_out_ctrl > 20)
            {
                fprintf(stderr, "ERR: limit of max retransmissions reached 15");
                free(result);
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            // Another try to send message
            i2Result = sendto(client_socket, buffer, msg_length, 0, (struct sockaddr *)&server_addr, server_addr_len);
            msg_out_ctrl++;

            if (i2Result <= 0)
            {
                perror("send failed");
                return null_Event;
            }

            // Another waiting to receive message
            i2Result = recv(client_socket, receive_buff, INPUT_LENGTH, 0);
            if (i2Result >= 0 && *((uint16_t *)(receive_buff + 1)) == hMessage_id && (receive_buff[0] == 0x00))
            {
                message_id++;
                break;
            }
            else if ((i2Result >= 0 && receive_buff[0] != 0x00))
            {
                // Another case if received message was not a confirm message
                char server_username[20];
                char msg_content[1400];
                event = decode_msg(receive_buff, server_username, msg_content);
                switch (event)
                {
                case reply_ok_Event:
                    fprintf(stderr, "%s: %s\n", server_username, msg_content);
                    event =  reply_ok_Event;
                    break;
                case reply_nok_Event:
                    fprintf(stderr, "%s: %s\n", server_username, msg_content);
                    event =  reply_nok_Event;
                    break;
                case error_inc_Event:
                    fprintf(stderr, "ERR FROM %s: %s\n", server_username, msg_content);
                    event =  error_inc_Event;
                    break;
                case bye_Event:
                    event =  bye_Event;
                    break;
                case msg_error_Event:
                    fprintf(stderr, "Failed to be parsed\n");
                    event =  msg_error_Event;
                    break;
                default:
                    printf("%s: %s\n", server_username, msg_content);
                    break;
                }
                    cnt++;
            }
            if (cnt == 0)
                fprintf(stderr, "ERR: NO CONFIRM RESPONSE\n");
        }
    }
    memset(buffer, 0, sizeof(&buffer));

    return event;
}