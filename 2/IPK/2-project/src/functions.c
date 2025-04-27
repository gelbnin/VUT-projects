#include "vars.h"

#define OPEN 0
#define CLOSED 1
#define UNKNOWN -1

struct addrinfo *get_host_address(const char *host)
{
    struct addrinfo hints, *result;
    int status;

    // Defining hints for getaddrinfo
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 или IPv6
    hints.ai_flags = 0;
    hints.ai_protocol = 0; // Any protocol

    // Get info
    status = getaddrinfo(host, NULL, &hints, &result);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return NULL;
    }

    return result;
}

char *get_char_ip_address(const char *interface_name, int family)
{
    // Using ifaddrs structure for searching user provided interface name
    struct ifaddrs *ifaddr, *ifa;

    // Pointer to ours ip address
    char *ip_address = NULL;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    // Going through all avaible interfaces untill we find that specific, that we are looking for
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        // In case of application it can be interface with type family IPV4 or IPV6
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == family)
        {
            if (strcmp(ifa->ifa_name, interface_name) == 0)
            {
                char addr_buf[INET6_ADDRSTRLEN];
                if (family == AF_INET)
                {
                    // Case for IPV4
                    struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
                    if (inet_ntop(AF_INET, &addr->sin_addr, addr_buf, INET_ADDRSTRLEN) != NULL)
                    {
                        ip_address = strdup(addr_buf);
                        break;
                    }
                }
                else if (family == AF_INET6)
                {
                    // Case for IPV6
                    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
                    if (inet_ntop(AF_INET6, &addr->sin6_addr, addr_buf, INET6_ADDRSTRLEN) != NULL)
                    {
                        ip_address = strdup(addr_buf);
                        break;
                    }
                }
            }
        }
    }
    if (ip_address == NULL)
    {
        free(ip_address);
    }

    freeifaddrs(ifaddr);

    // Warning, now "ip_address" is mallocated, and must be freed when exiting program
    return ip_address;
}

void print_header(struct addrinfo *dest_info, char *host_name)
{
    // Resolving destination IP
    void *dest_addr_ptr;
    char dest_addr_str[INET6_ADDRSTRLEN];
    memset(dest_addr_str, 0, INET6_ADDRSTRLEN);

    if (dest_info->ai_family == AF_INET)
    {
        struct sockaddr_in *ipv4_dest = (struct sockaddr_in *)dest_info->ai_addr;
        dest_addr_ptr = &(ipv4_dest->sin_addr);
    }
    else if (dest_info->ai_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6_dest = (struct sockaddr_in6 *)dest_info->ai_addr;
        dest_addr_ptr = &(ipv6_dest->sin6_addr);
    }
    else
    {
        fprintf(stderr, "Unsupported address family\n");
        exit(EXIT_FAILURE);
    }

    // Convert address format into a string
    if (inet_ntop(dest_info->ai_family, dest_addr_ptr, dest_addr_str, sizeof(dest_addr_str)) == NULL)
    {
        perror("print_header: inet_ntop");
        exit(EXIT_FAILURE);
    }

    printf("Interesting ports on %s (%s):\n", host_name, dest_addr_str);
    printf("PORT STATE\n");
    return;
}

void ports_convert(char *char_ports, int int_ports[65535])
{
    if (char_ports == NULL)
        return;
    int i = 0;
    // Check if the string contains "-"
    char *dash_ptr = strchr(char_ports, '-');
    if (dash_ptr != NULL)
    { // If it contains "-"
        // Extract the start and end values of the range
        int start = atoi(char_ports);
        int end = atoi(dash_ptr + 1);

        // Ensure start and end values are within the valid range
        if (start < 1)
            start = 1;
        if (end > 65535)
            end = 65535;

        // Fill the array with values from start to end
        for (int j = start; j <= end && i < 65535; j++)
        {
            int_ports[i++] = j;
        }
    }
    else
    { // If it does not contain "-"
        // Use comma as delimiter for substrings
        char *token = strtok(char_ports, ",");

        // Loop through substrings
        while (token != NULL && i < 65535)
        {
            int port = atoi(token);

            // Check if the value is within the valid range
            if (port >= 1 && port <= 65535)
            {
                // If it is valid, save it to the array
                int_ports[i++] = port;
            }
            else
            {
                fprintf(stderr, "Error: Port %d is out of range.\n", port);
                exit(EXIT_FAILURE);
            }

            // Get the next substring
            token = strtok(NULL, ",");
        }
    }
    return;
}

int handle_response(const u_char *packet, int type, int protocol)
{
    // TCP case
    if (protocol == IPPROTO_TCP)
    {
        // Structure for pointer to start of TCP data in packet
        struct tcphdr *tcp_header;

        /* Here it is important to specify which ip packet is received here,
         because ipv4 and ipv6 headers have different sizes, so the tcp_hdr pointer should take this into account. */
        if (type == AF_INET) // IPV4
            tcp_header = (struct tcphdr *)(packet + sizeof(struct iphdr) + sizeof(struct ether_header));
        else // IPV6
            tcp_header = (struct tcphdr *)(packet + sizeof(struct ip6_hdr) + sizeof(struct ether_header));

        // If tcp is type SYN
        if (tcp_header->syn && tcp_header->ack)
            return OPEN;
        // If tcp is type RST
        else if (tcp_header->rst)
            return CLOSED;
        // Unknown type of packet, in this case, we should have sent our package again.
        else
            return UNKNOWN;
    }
    // UDP case
    else if (protocol == IPPROTO_UDP)
    {
        if (type == AF_INET) // IPv4
        {
            const struct ip *ip_hdr = (struct ip *)(packet + sizeof(struct ether_header)); // Offset of 14 bytes for Ethernet

            // Check if the packet is ICMP type
            if (ip_hdr->ip_p == IPPROTO_ICMP)
            {
                // Offset for IP header and variable length
                const struct icmp *icmp_hdr = (struct icmp *)(packet + sizeof(struct ether_header) + (ip_hdr->ip_hl << 2));

                // Check if the ICMP packet type is Destination Unreachable (code 3)
                if (icmp_hdr->icmp_type == ICMP_UNREACH)
                {
                    // Check if the ICMP packet subtype is Port Unreachable (code 3)
                    if (icmp_hdr->icmp_code == ICMP_UNREACH_PORT)
                    {
                        return CLOSED; // ICMPv6 packet type Destination Unreachable
                    }
                }
            }
        }
        else if (type == AF_INET6) // IPv6
        {
            const struct ip6_hdr *ip6_hdr = (struct ip6_hdr *)(packet + sizeof(struct ether_header)); // Offset of 14 bytes for Ethernet
            // struct udphdr *udp_header = (struct udphdr)(packet + 2 * sizeof(struct ether_header) + 2 * sizeof(struct ip6_hdr))
            //  Check if the packet is IPv6 and ICMPv6
            if (ip6_hdr->ip6_nxt == IPPROTO_ICMPV6)
            {
                const struct icmp6_hdr *icmp6_hdr = (struct icmp6_hdr *)(packet + sizeof(struct ether_header) + sizeof(struct ip6_hdr)); // Offset for IP header

                // Check if the ICMPv6 packet type is Destination Unreachable (type 1)
                if (icmp6_hdr->icmp6_type == ICMP6_DST_UNREACH)
                {
                    // Check if the ICMPv6 packet subtype is Port Unreachable (code 4)
                    if (icmp6_hdr->icmp6_code == ICMP6_DST_UNREACH_NOPORT)
                    {
                        return CLOSED; // ICMPv6 packet type Destination Unreachable
                    }
                }
            }
        }
        // This return may be useless in this code, because if our pcap with filter accepted any packet
        return OPEN; // Packet is not ICMP or ICMPv6 Destination Unreachable type
    }
    return UNKNOWN;
}