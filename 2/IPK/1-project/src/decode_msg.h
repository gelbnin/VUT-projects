#include "fsm.h"
#include <stdint.h>
#include <sys/socket.h>

eSystemEvent decode_msg(char *receive_buffer, char *users_name, char *msg_content);

eSystemEvent decode_msg_tcp(char *receive_buffer, char *users_name, char *msg_content);
               
