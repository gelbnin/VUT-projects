#include <errno.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

typedef struct
{
    char *src_ip;
    char *dst_ip;
    int tcp_ports[65535]; // A relatively dumb way of declaring a field that may have 65535 items,
    int udp_ports[65535]; // it is better to use a smarter approach
} Data;

// Get address info for the target host
struct addrinfo *get_host_address(const char *host);

// Get and convert into char* IP address of the source interface
char *get_char_ip_address(const char *interface_name, int family);

// Printing header of the aplication
void print_header(struct addrinfo *dest_info, char *host_name);

// Convert port ranges from string to integer array
void ports_convert(char *char_ports, int int_ports[65535]);

// Handler for captured packets
int handle_response(const u_char *packet, int type, int protocol);

// Sending a packet type of "int protocol"
int send_packet(const char *src_ip, struct addrinfo *dest_info, int src_port, int dest_port, pcap_t *handle, int sockfd, struct bpf_program *fp, int protocol);
