#include "input_parse.h"
#include "avars.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#define INPUT_LENGTH 1400

/* Definition of function input parse implemented in both variants (UDP and TCP)
    This function provides parsing of arguments provided by user in STDIN,
    and returns an event on the basis of which it will be decided whether to send a message or not.

    Function also constructing a message, determine lenght of this message
    and assigns it to a variables "char *msg" and "int *msg_length".
*/
eSystemEvent input_parse(char *command, char *username, char *secret, char *display_name,
                         char *channel_id, char *read_buff, char *msg, int *msg_length)
{
    // Assign message ID to network byte order
    uint16_t hMessage_id = htons(message_id);

    char *token_result;

    // Assign a variable adrres pointing to an array msg for later determination of message length
    char *start_msg_length = msg;

    memset(command, 0, 10);

    // Fgets LOL :)
    memset(read_buff, 0, INPUT_LENGTH);
    fgets(read_buff, INPUT_LENGTH, stdin);

    /* IF-ELSE down bellow is decideing which of the commands is provided by user
    if read_buff is starting with "/", else it will imply, that user provided an MESSAGE, not an command
    */
    if (read_buff[0] == '/')
    {
        // Case for command

        // Removing "\n" char from read_buff, so it can easier work with tokens
        read_buff[strlen(read_buff) - 1] = 0x00;

        // Splitting into tokens using "strtok();"
        token_result = strtok(read_buff, " ");
        if (token_result == NULL || strlen(token_result) > 10)
        {
            fprintf(stderr, "ERR: command error\n");
            return null_Event;
        }
        strcpy(command, token_result);
        if (strcmp(command, "/auth") == 0)
        {
            // Case for /auth

            memset(username, 0, 20);
            memset(secret, 0, 128);
            memset(channel_id, 0, 20);
            memset(display_name, 0, 20);

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(username, token_result);

            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 128)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(secret, token_result);

            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(display_name, token_result);

            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Constructing message to be send by moving pointer to an array "msg"
            // to make sure, that 0x00 bytes will be included into a message
            msg[0] = 0x02;
            msg++;
            *((uint16_t *)(msg + 1)) = hMessage_id;
            msg += 2;
            strcpy(msg, username);
            msg += strlen(username) + 1;

            strcpy(msg, display_name);
            msg += strlen(display_name) + 1;

            strcpy(msg, secret);
            msg += strlen(secret) + 1;

            // Finalising determination of message length by substracting
            *msg_length = msg - start_msg_length;

            return auth_Event;
        }
        else if (strcmp(command, "/join") == 0)
        {
            // Case for "/join"

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            memset(channel_id, 0, 20);
            strcpy(channel_id, token_result);

            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Constructing message to be send by moving pointer to an array "msg"
            // to make sure, that 0x00 bytes will be included into a message
            msg[0] = 0x03;
            *((uint16_t *)(msg + 1)) = hMessage_id;
            msg += 3;

            strcpy(msg, channel_id);
            msg += strlen(channel_id) + 1;

            strcpy(msg, display_name);
            msg += strlen(display_name) + 1;

            // Finalising determination of message length by substracting
            *msg_length = msg - start_msg_length;

            return join_Event;
        }
        else if (strcmp(command, "/rename") == 0)
        {
            // Case for "/rename"

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }

            memset(display_name, 0, 20);
            strcpy(display_name, token_result);

            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Rename is an local command so no need to send message
            return null_Event;
        }
        else if (strcmp(command, "/help") == 0)
        {
            printf("    /auth 	    {Username} {Secret} {DisplayName}\n");
            printf("    /join 	    {ChannelID}\n");
            printf("    /rename 	{DisplayName}\n");
            printf("    /help \n");

            // No need to send message
            return null_Event;
        }
        else
        {
            fprintf(stderr, "ERR: unknown command\n");
            return null_Event;
        }
    }
    else
    {
        // Case for MSG message

        memset(msg, 0 , 1400);

        // Check if frist symbol is printable
        if (!(read_buff[0] >= 0x21 && read_buff[0] <= 0x7E))
            return null_Event;

        // Constructing message to be send by moving pointer to an array "msg"
        // to make sure, that 0x00 bytes will be included into a message
        msg[0] = 0x04;
        *((uint16_t *)(msg + 1)) = hMessage_id;

        msg += 3;
        strcpy(msg, display_name);

        msg += strlen(display_name) + 1;
        strcpy(msg, read_buff);

        msg += strlen(read_buff) + 1;
        *msg_length = msg - start_msg_length;

        return msg_outc_Event;
    }
    return null_Event;
}

// TCP message construct is way more simplier than UDP variant
eSystemEvent input_parse_tcp(char *command, char *username, char *secret, char *display_name,
                             char *channel_id, char *read_buff, char *msg, int *msg_length)
{
    char *token_result;

    // Assign a variable adrres pointing to an array msg for later determination of message length
    char *start_msg_length = msg;

    memset(command, 0, 10);

    memset(read_buff, 0, INPUT_LENGTH);
    fgets(read_buff, INPUT_LENGTH, stdin);

    /* IF-ELSE down bellow is decideing which of the commands is provided by user
    if read_buff is starting with "/", else it will imply, that user provided an MESSAGE, not an command
    */
    if (read_buff[0] == '/')
    {
        // Case for command

        // Removing "\n" char from read_buff, so it can easier work with tokens
        read_buff[strlen(read_buff) - 1] = 0x00;

        // Splitting into tokens using "strtok();"
        token_result = strtok(read_buff, " ");
        if (token_result == NULL || strlen(token_result) > 10)
        {
            fprintf(stderr, "ERR: command error\n");
            return null_Event;
        }
        strcpy(command, token_result);
        if (strcmp(command, "/auth") == 0)
        {
            // Case for "/auth"

            memset(username, 0, 20);
            memset(secret, 0, 128);
            memset(channel_id, 0, 20);
            memset(display_name, 0, 20);

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(username, token_result);

            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 128)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(secret, token_result);

            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            strcpy(display_name, token_result);

            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Constructing message
            strcpy(msg, "AUTH ");
            msg += strlen(msg);
            strcpy(msg, username);
            msg += strlen(msg);

            strcpy(msg, " AS ");
            msg += strlen(msg);

            strcpy(msg, display_name);
            msg += strlen(msg);

            strcpy(msg, " USING ");
            msg += strlen(msg);

            strcpy(msg, secret);
            msg += strlen(msg);
            *msg = 0x0D;
            msg++;
            *msg = 0x0A;

            // Finalising determination of message length
            *msg_length = strlen(start_msg_length);

            return auth_Event;
        }
        else if (strcmp(command, "/join") == 0)
        {
            // Case for "/join"

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            memset(channel_id, 0, 20);
            strcpy(channel_id, token_result);

            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Constructing message
            strcpy(msg, "JOIN ");
            msg += strlen(msg);

            strcpy(msg, channel_id);
            msg += strlen(msg);

            strcpy(msg, " AS ");
            msg += strlen(msg);

            strcpy(msg, display_name);
            msg += strlen(msg);
            *msg = 0x0D;
            msg++;
            *msg = 0x0A;

            // Finalising determination of message length
            *msg_length = strlen(start_msg_length);
            return join_Event;
        }
        else if (strcmp(command, "/rename") == 0)
        {
            // Case for "/rename"

            // Cheking if command format is correct
            token_result = strtok(NULL, " ");
            if (token_result == NULL || strlen(token_result) > 20)
            {
                fprintf(stderr, "ERR: command error\n");
                return null_Event;
            }
            memset(display_name, 0, 20);
            strcpy(display_name, token_result);
            
            token_result = strtok(NULL, " ");
            if (token_result != NULL)
            {
                fprintf(stderr, "ERR: command error: too long\n");
                return null_Event;
            }

            // Rename is an local command so no need to send message
            return null_Event;
        }
        else if (strcmp(command, "/help") == 0)
        {
            printf("    /auth 	    {Username} {Secret} {DisplayName}\n");
            printf("    /join 	    {ChannelID}\n");
            printf("    /rename 	{DisplayName}\n");
            printf("    /help \n");

            // No need to send message
            return null_Event;
        }
        else
        {
            fprintf(stderr, "ERR: Unknown command");
            return null_Event;
        }
    }
    else
    {
        // Case for MSG message

        memset(msg, 0, 1400);
        // Check if frist symbol is printable
        if (!(read_buff[0] >= 0x21 && read_buff[0] <= 0x7E))
            return null_Event;

        // Constructing message
        strcpy(msg, "MSG FROM ");
        msg += strlen(msg);

        strcpy(msg, display_name);
        msg += strlen(msg);

        strcpy(msg, " IS ");
        msg += strlen(msg);

        strcpy(msg, read_buff);
        msg += strlen(msg);
        *msg = 0x0D;
        msg++;
        *msg = 0x0A;

        // Finalising determination of message length
        *msg_length = strlen(start_msg_length);
        return msg_outc_Event;
    }
    return null_Event;
}