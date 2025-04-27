#ifndef ARGUMENTS_PARSER_H
#define ARGUMENTS_PARSER_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    char *interface;
    char *port_ranges_udp;
    char *port_ranges_tcp;
    int timeout;
    char *target;
} Arguments;

Arguments parse_arguments(int argc, char *argv[]);

#endif /* ARGUMENTS_PARSER_H */
