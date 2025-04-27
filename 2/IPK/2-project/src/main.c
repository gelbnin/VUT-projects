#include "parse_args.h"
#include "vars.h"

#define SRC_PORT 59999
#define OPEN 0
#define CLOSED 1
#define UNKNOWN -1

Data data;
struct addrinfo *result;
pcap_t *handle;
int sockfd;
struct bpf_program fp;

// Safe exit function
void sigint_handler()
{
    if (sockfd != 0)
        close(sockfd);
    if (result != NULL)
        freeaddrinfo(result);
    if (data.src_ip != NULL)
        free(data.src_ip);
    if (handle != NULL)
        pcap_close(handle);
    if (fp.bf_insns != NULL)
        pcap_freecode(&fp);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // Register SIGINT signal handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        fprintf(stderr, "Failed to set SIGINT handler\n");
        exit(EXIT_FAILURE);
    }

    int res;

    // Parse command line arguments
    Arguments args = parse_arguments(argc, argv);

    // Init default values in data struct
    data.dst_ip = "";
    data.src_ip = "";
    data.tcp_ports[0] = 0;
    data.udp_ports[0] = 0;

    // Convert port ranges from string to integer array
    ports_convert(args.port_ranges_tcp, data.tcp_ports);
    ports_convert(args.port_ranges_udp, data.udp_ports);

    // Get address info for the target host
    result = get_host_address(args.target);
    if (result == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // Open packet capture device
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live(args.interface, BUFSIZ, 1, -1, errbuf);
    if (handle == NULL)
    {
        fprintf(stderr, "pcap_open_live failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    // Open socket with RAW options
    sockfd = socket(result->ai_family, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to a user provided network interface
    res = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, args.interface, strlen(args.interface) + 1);
    if (res < 0)
    {
        perror("SetSockOpt:");
        exit(EXIT_FAILURE);
    }

    // Get IP address of the source interface
    data.src_ip = get_char_ip_address(args.interface, result->ai_family);

    print_header(result, args.target);

    // Flag for optional message retransmission in case of NO REPLY from port
    int flag = 0;

    //                                        //----------TCP SCANNIG SECTION START----------//

    // TCP port handling (cycle goes through all available TCP ports)
    for (int i = 0; i < 65535 && data.tcp_ports[i] != 0; i++)
    {
        // Initialization for select
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(pcap_get_selectable_fd(handle), &fds);

        // Initialization of structure for timeout
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = args.timeout * 1000;

        // Function that sends "ping" packet into a port
        send_packet(data.src_ip, result, SRC_PORT, data.tcp_ports[i], handle, sockfd, &fp, IPPROTO_TCP);

        // Waiting for packet, to be received
        res = select(pcap_get_selectable_fd(handle) + 1, &fds, NULL, NULL, &tv);
        if (res == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (res == 0)
        {
            // Case for no packet received
            // Reattempting to send packet to the same port
            if (flag == 0)
            {
                i--;
                flag = 1;
            }
            else // If no response in second attempt
            {
                printf("%i/tcp filtered\n", data.tcp_ports[i]);
                flag = 0;
            }
        }
        else
        {
            // Packet received, read and process it
            struct pcap_pkthdr *header;
            const u_char *packet;
            int res = pcap_next_ex(handle, &header, &packet);
            if (res == 1)
            {
                // Packet successfully captured, handle it
                res = handle_response(packet, result->ai_family, IPPROTO_TCP);
                switch (res)
                {
                case OPEN:
                    printf("%i/tcp open\n", data.tcp_ports[i]);
                    break;
                case CLOSED:
                    printf("%i/tcp closed\n", data.tcp_ports[i]);
                    break;
                case UNKNOWN:
                    // Trying packet retransmition
                    if (flag == 0)
                    {
                        i--;
                        flag = 1;
                    }
                    else
                    {
                        printf("%i/tcp filtered\n", data.tcp_ports[i]);
                        flag = 0;
                    }
                    break;

                default:
                    break;
                }
            }
            else if (res == 0)
            {
                // Case for no packets was received (Basicly this is a dead code, and while using "select()",
                // this case can not occure, but just in case of some error in pcap_next_ex, or select, it have to be handeled)
                fprintf(stderr, "Error in pcap file descriptor occured: change in file descriptor, but no packets was captured\n");
                if (flag == 0)
                {
                    i--;
                    flag = 1;
                }
                else
                {
                    printf("%i/tcp filtered\n", data.tcp_ports[i]);
                    flag = 0;
                }
            }
            else if (res == -1)
            {
                // Valid error in pcap_next_ex
                perror("pcap_next_ex");
            }
        }
        // Freeing filter code
        pcap_freecode(&fp);
    }

    //                                        //----------TCP SCANNIG SECTION END----------//

    //                                        //----------UDP SCANNIG SECTION START----------//

    for (int i = 0; i < 65535 && data.udp_ports[i] != 0; i++)
    {
        // Initialization for select
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(pcap_get_selectable_fd(handle), &fds);

        // Initialization of structure for timeout
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = args.timeout * 1000;

        // Function that sends "ping" packet into a port
        send_packet(data.src_ip, result, SRC_PORT, data.udp_ports[i], handle, sockfd, &fp, IPPROTO_UDP);

        // Waiting for packet, to be received
        res = select(pcap_get_selectable_fd(handle) + 1, &fds, NULL, NULL, &tv);
        if (res == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (res == 0)
        {
            // Case for no packet received
            // Reattempting to send packet to the same port
            if (flag == 0)
            {
                i--;
                flag = 1;
            }
            else // If no response in second attempt
            {
                printf("%i/udp open\n", data.udp_ports[i]);
                flag = 0;
            }
        }
        else
        {
            // Packet received, read and process it
            struct pcap_pkthdr *header;
            const u_char *packet;
            int res = pcap_next_ex(handle, &header, &packet);
            if (res == 1)
            {
                // Packet successfully captured, handle it
                res = handle_response(packet, result->ai_family, IPPROTO_UDP);
                switch (res)
                {
                case OPEN:
                    printf("%i/udp open\n", data.udp_ports[i]);
                    break;
                case CLOSED:
                    printf("%i/udp closed\n", data.udp_ports[i]);
                    sleep(1); // Added sleep to ensure that limit of icmp messages wont break limit 80msgs/4seconds
                    break;

                default:
                    break;
                }
            }
            else if (res == 0)
            {
                // Case for no packets was received (Basicly this is a dead code, and while using "select()",
                // this case can not occure, but just in case of some error in pcap_next_ex, or select, it have to be handeled)
                if (flag == 0)
                {
                    i--;
                    flag = 1;
                }
                else
                {
                    printf("%i/udp open\n", data.udp_ports[i]);
                    flag = 0;
                }
            }
            else if (res == -1)
            {
                perror("pcap_next_ex");
            }
        }
        pcap_freecode(&fp);
    }

    //                                          //----------UDP SCANNIG SECTION END----------/

    // Freeing allocated data
    close(sockfd);
    if (result != NULL)
        freeaddrinfo(result);
    if (data.src_ip != NULL)
        free(data.src_ip);
    if (handle != NULL)
        pcap_close(handle);

    return 0;
}
