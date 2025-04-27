#include <stdint.h>
#include <sys/socket.h>
#include "fsm.h"

eSystemEvent udp_send(char *buffer, char *receive_buff, int msg_length);