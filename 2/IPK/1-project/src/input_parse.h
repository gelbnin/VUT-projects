#include <stdint.h>
#include "fsm.h"

eSystemEvent input_parse(char *command, char * username, char *secret, char *display_name,
                         char *channel_id, char *read_buff, char *msg
                         ,int *msg_lenght);

eSystemEvent input_parse_tcp(char *command, char * username, char *secret, char *display_name,
                         char *channel_id, char *read_buff, char *msg
                         ,int *msg_lenght);