#include "vars.h"

unsigned short calculate_tcp_checksum(struct iphdr *iph, struct tcphdr *tcph, unsigned short len_tcp)
{
    unsigned short *buf = (unsigned short *)tcph;
    unsigned int sum = 0;
    unsigned short checksum = 0;

    // Pseudo-header sum
    struct
    {
        unsigned int source_address;
        unsigned int dest_address;
        unsigned char placeholder;
        unsigned char protocol;
        unsigned short tcp_length;
    } pseudo_header;

    pseudo_header.source_address = iph->saddr;
    pseudo_header.dest_address = iph->daddr;
    pseudo_header.placeholder = 0;
    pseudo_header.protocol = IPPROTO_TCP;
    pseudo_header.tcp_length = htons(len_tcp);

    unsigned short *pseudo_buf = (unsigned short *)&pseudo_header;

    for (long unsigned int i = 0; i < sizeof(pseudo_header) / 2; i++)
    {
        sum += ntohs(pseudo_buf[i]);
    }

    // TCP header sum
    for (int i = 0; i < len_tcp / 2; i++)
    {
        sum += ntohs(*buf++);
    }

    // If length of TCP header is odd, add padding
    if (len_tcp % 2)
    {
        sum += ((unsigned char *)tcph)[len_tcp - 1];
    }

    // Add carry-over
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Invert checksum
    checksum = ~sum;

    return checksum;
}

// Функция для вычисления контрольной суммы UDP
uint16_t calculate_udp_checksum(struct iphdr *iph, struct udphdr *udph, int data_length)
{
    uint32_t sum = 0;

    // Calculate sum of IPv4 pseudo-header
    sum += (iph->saddr >> 16) & 0xFFFF; // Source IP address (upper 16 bits)
    sum += iph->saddr & 0xFFFF;         // Source IP address (lower 16 bits)
    sum += (iph->daddr >> 16) & 0xFFFF; // Destination IP address (upper 16 bits)
    sum += iph->daddr & 0xFFFF;         // Destination IP address (lower 16 bits)
    sum += htons(IPPROTO_UDP);          // Protocol (UDP)
    sum += htons(data_length);          // Data length

    // Calculate sum of UDP header and data
    uint16_t *buf = (uint16_t *)udph;
    for (unsigned long int i = 0; i < sizeof(struct udphdr) / 2; i++)
    {
        sum += *buf++;
    }

    // Add carry-over
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Invert bits and return checksum
    return (uint16_t)~sum;
}

int send_packet(const char *src_ip, struct addrinfo *dest_info, int src_port, int dest_port, pcap_t *handle, int sockfd, struct bpf_program *fp, int protocol)
{
    int res;
    char filter_exp[100];
    char datagram[4096];
    memset(filter_exp, 0, 100);
    memset(datagram, 0, 4096);

    //                                         //----------FORMING IP HEADER SECTION START----------//

    struct iphdr *iph = (struct iphdr *)datagram;
    struct ip6_hdr *iph6 = (struct ip6_hdr *)datagram;

    void *transport_hdr;
    int transport_hdr_len;

    // Initialize datagram based on address family
    if (dest_info->ai_family == AF_INET)
    {
        // Structure for destination address
        struct sockaddr_in *ipv4_dest = (struct sockaddr_in *)dest_info->ai_addr; // ? may be not used
        iph->version = 4;

        struct tcphdr *tcph;
        struct udphdr *udph;

        // Initialize TCP or UDP header based on protocol
        if (protocol == IPPROTO_TCP)
        {
            // Pointing to a part of datagram, that will contain TCP header
            tcph = (struct tcphdr *)(datagram + sizeof(struct iphdr));
            memset(tcph, 0, sizeof(struct tcphdr));

            transport_hdr = tcph;
            transport_hdr_len = sizeof(struct tcphdr);
        }
        else if (protocol == IPPROTO_UDP)
        {
            // Pointing to a part of datagram, that will contain UDP header
            udph = (struct udphdr *)(datagram + sizeof(struct iphdr));
            memset(udph, 0, sizeof(struct udphdr));

            transport_hdr = udph;
            transport_hdr_len = sizeof(struct udphdr);
        }
        else
        {
            fprintf(stderr, "Unsupported protocol\n");
            exit(EXIT_FAILURE);
        }

        // Set destination address and fill IP header
        struct in_addr *ipv4_dest_addr = &(ipv4_dest->sin_addr);       // Copy into in_addr ipv4_dest_addr address format, from sockaddr_in
        inet_pton(AF_INET, src_ip, &(iph->saddr));                     // Copying char *src_ip into appropriate format into iph
        inet_pton(AF_INET, inet_ntoa(*ipv4_dest_addr), &(iph->daddr)); // Copying in_addr ipv_dest_addr into appropriate format into iph

        iph->ihl = 5;
        iph->tos = 0;
        iph->id = htonl(54321);
        iph->frag_off = 0;
        iph->ttl = 255;
        iph->protocol = protocol;
        iph->check = 0;

        // Setting filter to a specific ip and port
        snprintf(filter_exp, sizeof(filter_exp), "ip and dst host %s and dst port %i", src_ip, src_port);
    }
    else if (dest_info->ai_family == AF_INET6)
    {
        iph6->ip6_ctlun.ip6_un1.ip6_un1_flow = 0b01100000; // Set family ip hdr to ipv6
        iph6->ip6_ctlun.ip6_un1.ip6_un1_plen = 0;
        iph6->ip6_ctlun.ip6_un1.ip6_un1_nxt = 0;
        iph6->ip6_ctlun.ip6_un1.ip6_un1_hlim = 0;
        iph6->ip6_ctlun.ip6_un1.ip6_un1_hlim = 0;

        struct sockaddr_in6 *ipv6_dest = (struct sockaddr_in6 *)dest_info->ai_addr;

        struct tcphdr *tcph;
        struct udphdr *udph;

        if (protocol == IPPROTO_TCP)
        {
            // Pointing to a part of datagram, that will contain TCP header
            tcph = (struct tcphdr *)(datagram + sizeof(struct ip6_hdr));
            memset(tcph, 0, sizeof(struct tcphdr));

            transport_hdr = tcph;
            transport_hdr_len = sizeof(struct tcphdr);
        }
        else if (protocol == IPPROTO_UDP)
        {
            // Pointing to a part of datagram, that will contain UDP header
            udph = (struct udphdr *)(datagram + sizeof(struct ip6_hdr));
            memset(udph, 0, sizeof(struct udphdr));

            transport_hdr = udph;
            transport_hdr_len = sizeof(struct udphdr);
        }
        else
        {
            fprintf(stderr, "Unsupported protocol\n");
            exit(EXIT_FAILURE);
        }

        // Set destination address and fill IP header
        struct in6_addr *ipv6_dest_addr = &(ipv6_dest->sin6_addr); // Copy into in_addr ipv6_dest_addr address format, from sockaddr_in format
        inet_pton(AF_INET6, src_ip, &(iph6->ip6_src));             // Set iph6 src address
        iph6->ip6_dst = *ipv6_dest_addr;                           // Set iph6 dest address
        iph6->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(transport_hdr_len);
        iph6->ip6_ctlun.ip6_un1.ip6_un1_nxt = protocol;

        // Set filter
        snprintf(filter_exp, sizeof(filter_exp), "ip6 and dst host %s and dst port %i", src_ip, src_port);
    }
    else
    {
        fprintf(stderr, "Unsupported address family\n");
        exit(EXIT_FAILURE);
    }

    //                                              //----------FORMING IP HEADER SECTION END----------//

    // Setting filter to a specific ip and port
    res = pcap_compile(handle, fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN);
    if (res == -1)
    {
        fprintf(stderr, "Could not parse filter: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        exit(EXIT_FAILURE);
    }
    res = pcap_setfilter(handle, fp);
    if (res == -1)
    {
        fprintf(stderr, "Could not install filter: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        exit(EXIT_FAILURE);
    }

    //                                    //----------FORMING TRANSPORT LAYER PROTOCOL HEADER SECTION START----------//

    // Fill transport layer header (TCP or UDP)
    if (protocol == IPPROTO_TCP)
    {
        struct tcphdr *tcph = (struct tcphdr *)transport_hdr;
        tcph->source = htons(src_port);
        tcph->dest = htons(dest_port);
        tcph->seq = 0;
        tcph->ack_seq = 0;
        tcph->doff = 5;
        tcph->fin = 0;
        tcph->syn = 1; // Flag SYN
        tcph->rst = 0;
        tcph->psh = 0;
        tcph->ack = 0;
        tcph->urg = 0;
        tcph->window = htons(5840);
        tcph->check = 0;
        tcph->urg_ptr = 0;
        tcph->check = htons(calculate_tcp_checksum(iph, tcph, transport_hdr_len));

        if (dest_info->ai_family == AF_INET6)
            tcph->check -= htons(2); // Checksum correction for IPV6
    }
    else if (protocol == IPPROTO_UDP)
    {
        // In case of UDP should be received icmp messgaes, so changing filter
        // "icmp[28] == 0xea and icmp[29] == 0x5f" is a filter to an icmp message responding to message which was sended from port 59999 (application port)
        res = pcap_compile(handle, fp, "(icmp and icmp[28] == 0xea and icmp[29] == 0x5f) or (icmp6 and icmp6[48] == 0xea and icmp6[49] == 0x5f)", 0, PCAP_NETMASK_UNKNOWN);
        if (res == -1)
        {
            fprintf(stderr, "Could not parse filter: %s\n", pcap_geterr(handle));
            pcap_close(handle);
            exit(EXIT_FAILURE);
        }
        res = pcap_setfilter(handle, fp);
        if (res == -1)
        {
            fprintf(stderr, "Could not install filter: %s\n", pcap_geterr(handle));
            pcap_close(handle);
            exit(EXIT_FAILURE);
        }
        struct udphdr *udph = (struct udphdr *)transport_hdr;
        udph->source = htons(src_port);
        udph->dest = htons(dest_port);
        udph->len = htons(transport_hdr_len);
        udph->check = 0;

        udph->check = calculate_udp_checksum(iph, udph, transport_hdr_len);
        if (dest_info->ai_family == AF_INET6)
            udph->check -= htons(2); // Checksum correction for IPV6
    }

    //                                    //----------FORMING TRANSPORT LAYER PROTOCOL HEADER SECTION END----------//

    // Send packet
    res = sendto(sockfd, datagram, (dest_info->ai_family == AF_INET6) ? sizeof(struct ip6_hdr) + transport_hdr_len : sizeof(struct iphdr) + transport_hdr_len, 0, dest_info->ai_addr, dest_info->ai_addrlen);
    if (res < 0)
    {
        perror("Sendto failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
