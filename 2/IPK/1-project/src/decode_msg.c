#include "decode_msg.h"
#include "udp_send.h"
#include <stdio.h>
#include <string.h>
/*  Definition of function, that decodes received message and sending corresponding confirm message 
    Implementet in both variants (UDP and TCP)
*/

//-------------------------UDP--------------------------------
eSystemEvent decode_msg(char *receive_buffer, char *users_name, char *msg_content)
{
    short int msg_result;
    char confirm_msg[3];

    // Constructing and sending corresponding confirm message
    confirm_msg[0] = 0x00;
    *((uint16_t *)(confirm_msg + 1)) = *((uint16_t *)(receive_buffer + 1));
    udp_send(confirm_msg, "", 3);

    // Switch case for type of the message
    switch (receive_buffer[0])
    {
    case (char)0x01:
    {
        // Case for REPLY
        msg_result = receive_buffer[3];
        receive_buffer += 6;
        strcpy(msg_content, receive_buffer);
        if (msg_result == 1)
        {
            strcpy(users_name, "Success");
            return reply_ok_Event;
        }
        else if (msg_result == 0)
        {
            strcpy(users_name, "Failure");
            return reply_nok_Event;
        }
        else
            return error_Event;
        break;
    }
    case (char)0x04:
    {
        // Case for MSG
        receive_buffer += 3;
        strcpy(users_name, receive_buffer);
        receive_buffer += strlen(users_name) + 1;
        strcpy(msg_content, receive_buffer);
        return msg_inc_Event;
        break;
    }
    case (char)0xfe:
    {
        // Case for ERROR
        receive_buffer += 3;
        strcpy(users_name, receive_buffer);
        receive_buffer += strlen(users_name) + 1;
        strcpy(msg_content, receive_buffer);
        msg_result = 0;
        return error_inc_Event;
        break;
    }
    case (char)0xff:
    {
        // Case for BYE 
        return bye_Event;
    }
    default:
        // Case for unknown message
        return msg_error_Event;
        break;
    }

    return 0;
}

//-----------------------------TCP------------------------------
// TCP variant of function uses same principle with token parsing, that used in "input_parse();"
eSystemEvent decode_msg_tcp(char *receive_buffer, char *users_name, char *msg_content)
{
    char *token_result[6];

    // Token check for command type
    token_result[0] = strtok(receive_buffer, " ");
    if (strcmp(token_result[0], "REPLY") == 0)
    {
        // Check if tokens are correct
        for (int i = 1; i <= 3; i++)
        {
            if (i == 3)
                token_result[i] = strtok(NULL, "\r\n");
            else
                token_result[i] = strtok(NULL, " ");

            if (token_result[i] == NULL)
            {
                return msg_error_Event;
            }
        }

        if (strcmp(token_result[2], "IS") != 0)
            return msg_error_Event;

        strcpy(msg_content, token_result[3]);

        if (strcmp(token_result[1], "OK") == 0)
        {
            strcpy(users_name, "Success");
            return reply_ok_Event;
        }
        else if (strcmp(token_result[1], "NOK") == 0)
        {
            strcpy(users_name, "Failure");
            return reply_nok_Event;
        }
    }
    else if (strcmp(token_result[0], "MSG") == 0)
    {
        // Check if tokens are correct
        for (int i = 1; i <= 4; i++)
        {
            if (i == 4)
                token_result[i] = strtok(NULL, "\r\n");
            else
                token_result[i] = strtok(NULL, " ");

            if (token_result[i] == NULL)
            {
                return msg_error_Event;
            }
        }

        if (strcmp(token_result[1], "FROM") != 0)
            return msg_error_Event;

        if (strcmp(token_result[3], "IS") != 0)
            return msg_error_Event;

        strcpy(users_name, token_result[2]);
        strcpy(msg_content, token_result[4]);

        return msg_inc_Event;
    }
    else if (strcmp(token_result[0], "ERR") == 0)
    {
        // Check if tokens are correct
        for (int i = 1; i <= 4; i++)
        {
            if (i == 4)
                token_result[i] = strtok(NULL, "\r\n");
            else
                token_result[i] = strtok(NULL, " ");

            if (token_result[i] == NULL)
            {
                return msg_error_Event;
            }
        }

        if (strcmp(token_result[1], "FROM") != 0)
            return msg_error_Event;

        if (strcmp(token_result[3], "IS") != 0)
            return msg_error_Event;

        strcpy(users_name, token_result[2]);
        strcpy(msg_content, token_result[4]);

        return error_inc_Event;
    }
    else if (strcmp(token_result[0], "BYE") == 0)
    {
        return bye_Event;
    }
    else
    {
        return msg_error_Event;
    }
    return null_Event;
}