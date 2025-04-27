#ifndef VARS_H
#define VARS_H
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>

//Declaration of basic variables to use around code using "extern"

//Variable that contains file descriptor of socket
extern int client_socket;

//Structers that provide functionality to "getaddrinfo();"
//These structures contains IP adress of domain hostname. 
extern struct addrinfo *result;
extern struct addrinfo hints;


//Structures that provide functionality to functions "send();", "recv();", "recvfrom".
//These structures also contains IP adress, but for more comfortable work with "send();", "recv();", "recvfrom"
//Mainly because of the UDP variant, with dynamical server port. 
extern struct sockaddr_in server_addr;
extern socklen_t server_addr_len;


//Variable that contains message ID, to UDP variant
extern uint16_t message_id;

//Variables that contains arguments of programm
extern char *protocol;
extern char *host_ip;
extern uint16_t server_port;
extern uint16_t udp_timeout;
extern uint8_t max_retransmissions;

#endif 