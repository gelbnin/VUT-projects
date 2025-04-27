#include "parse_args.h"
#include "vars.h"

void print_usage()
{
    printf("Usage: ./ipk-l4-scan \n [-i interface | --interface interface]\n [--pu port-ranges | --pt port-ranges | -u port-ranges | -t port-ranges]\n {-w timeout}\n [domain-name | ip-address]\n");
    return;
}
// Function for printing available interfaces
void print_interfaces()
{
    struct ifaddrs *ifap, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifap) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    printf("Available interfaces:\n");
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET)
        {
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
            {
                printf("%s\n", ifa->ifa_name);
            }
        }
    }

    freeifaddrs(ifap);
    return;
}
// TODO
Arguments parse_arguments(int argc, char *argv[])
{
    int opt;
    Arguments args;
    args.interface = NULL;
    args.port_ranges_udp = NULL;
    args.port_ranges_tcp = NULL;
    args.timeout = 5000; // Default timeout 5000 milliseconds
    args.target = NULL;

    // Options for getopt_long
    static struct option long_options[] = {
        {"interface", 2, NULL, 'i'},
        {"pu", required_argument, NULL, 'u'},
        {"pt", required_argument, NULL, 't'},
        {"u", required_argument, NULL, 'u'},
        {"t", required_argument, NULL, 't'},
        {"w", required_argument, NULL, 'w'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "i?t:u:w:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'i':
            if (argv[optind] == NULL)
            {
                print_interfaces();
                exit(EXIT_SUCCESS);
            }
            else
            {
                args.interface = argv[optind];
            }
            break;
        case 't':
            if (args.port_ranges_tcp != NULL)
            {
                print_usage();
                exit(EXIT_FAILURE);
            }
            args.port_ranges_tcp = optarg;
            break;
        case 'u':
            if (args.port_ranges_udp != NULL)
            {
                print_usage();
                exit(EXIT_FAILURE);
            }
            args.port_ranges_udp = optarg;
            break;
        case 'w':
            args.timeout = atoi(optarg);
            break;
        default:
            print_usage();
            exit(EXIT_FAILURE);
        }
    }

    // Parsing target
    if (optind < argc)
    {
        args.target = argv[argc - 1];
    }
    else
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    if ((args.port_ranges_tcp == NULL && args.port_ranges_udp == NULL) ||
        args.target == NULL || args.interface == NULL)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    return args;
}