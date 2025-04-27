#ifndef FSM_H
#define FSM_H

// Declaration of the fine state machine to provide logic for programm 

// Declaration of states
typedef enum
{
    Auth_State,
    Open_State,
    Error_State,
} eSystemState;


// Declaration of events
typedef enum
{
//--INPUT_EVENTS--
    auth_Event,
    join_Event,
    msg_outc_Event,
//--INCOMING_EVENTS--
    msg_inc_Event,
    error_inc_Event,
    reply_ok_Event,
    reply_nok_Event,
    msg_error_Event,
    error_Event,
    bye_Event,
//---------------
    null_Event,
} eSystemEvent;


#endif