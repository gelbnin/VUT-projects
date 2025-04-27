# ipk_proj_1

- [IPK24 - chat client](#ipk24---chat-lient)
  - [Executive Summary](#executive-summary)
  - [USAGE](#usage)
    - [Getting Started](#getting-started)
    - [Supported Commands](#supported-commands)
  - [Program Implementation Overview](#program-implementation-overview)
  - [Testing](#testing)
  - [Bibliography](#bibliography)

---

## IPK24 - Chat Client

### Executive Summary

The **IPK24-CHAT** protocol defines a communication standard for client-server interactions in a chat application, supporting both TCP and UDP transport protocols. It specifies various message types such as **AUTH** for authentication, **JOIN** for joining chat channels, **MSG** for sending messages, **ERR** for error handling, **REPLY** for confirmation or rejection of requests, and **BYE** for terminating the conversation.

- **UDP**
  - In the **UDP** variant, which is connection-less and unreliable, additional mechanisms are implemented at the application layer to handle issues like packet loss, duplication, and reordering. These include mandatory message confirmation with timeouts and retransmissions for lost messages, as well as tracking unique message IDs to detect duplication.
  
- **TCP**
  - The **TCP** variant, on the other hand, is connection-oriented and reliable, requiring less concern at the application layer for handling transport issues. Messages are sent as text-based commands, following a defined grammar, and terminated with `\r\n`. 
  
- **Client Application**
  - The client application supports various commands such as `/auth` for authentication, `/join` for joining channels, `/rename` for changing display names, and `/help` for displaying supported commands.
  
- **Summary**
  - Overall, the implemented client application adheres to the **IPK24-CHAT** protocol specifications, handling both TCP and UDP transport protocols.

---

## USAGE

### Getting Started

To start using the program, you need to input the required arguments and options:

| Option            | Description                                            |
|-------------------|--------------------------------------------------------|
| -t <udp \| tcp>   | Specify the protocol type (UDP or TCP)                 |
| -s <server_host>  | Specify the server hostname or IP address              |

Optional arguments include:

| Option           | Description                                      | Default Value |
|------------------|--------------------------------------------------|---------------|
| -p <server_port> | Specify the server port                         | 4567          |
| -d <UDP_timeout> | Specify the UDP timeout duration in milliseconds | 250ms         |
| -r <retries>     | Specify the number of retries                   | 3             |

After successful validation of the hostname (and establishing a connection with the server in the case of TCP), the program transitions to the AUTH STATE, where you need to input your credentials correctly.

### Supported Commands:

| Command           | Description                                                                                                                                                   |
|-------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|
| /auth {Username} {Secret} {DisplayName} | Sends an AUTH message with the provided data to the server and sets the local DisplayName (same as /rename command)                                           |
| /join {ChannelID}   | Sends a JOIN message with the channel name to the server                                                                                                      |
| /rename {DisplayName} | Locally changes the display name of the user for new messages/commands                                                                                       |
| /help              | Prints out supported local commands with their parameters and descriptions                                                                                   |

After receiving a successful response from the server, the program transitions to the OPEN STATE, where further communication with the server occurs.

To end the program, simply press Ctrl+C.

---

## Program Implementation Overview

The implementation of this program is based on Finite State Machine (FSM) and Epoll instance. 

- **FSM Integration**
  - The code waits for any event, such as user input or incoming messages. Based on the event, the code conducts certain actions and returns an "incoming_event". This incoming event serves as a signal for the FSM to execute specific actions or change states.

- **Epoll Instance**
  - Epoll is used to efficiently handle multiple I/O events in a scalable manner. It allows the code to wait for events on multiple file descriptors, such as standard input or socket descriptors.

- **Signal Handling**
  - A signal handler is defined to catch the SIGINT signal for ending the connection to the server for both TCP and UDP variants.

- **Event Handling Functions**
  - Two main functions are defined for handling events for both UDP and TCP variants. These functions wait for an event (on STDIN descriptor or socket descriptor) and perform actions accordingly.

- **Main Function**
  - The main function orchestrates the FSM logic. It initializes necessary components such as socket, epoll instance, and signal handler. Then, it enters a loop where it reads events and performs actions based on the current state and incoming events.

- **State Transition**
  - The main loop includes a switch-case statement to handle state transitions and actions for each state. States include Auth_State, Open_State, and Error_State. Actions are taken based on incoming events, such as authentication success or failure, message reception, or error handling.

---

## Testing

The program was initially tested locally and then (as indicated below) on the reference server anton5.fit.vutbr.cz.

Successful connection to the server:
> \> stdin: /auth xshmon00 \*secret\* test4444
> \> server: Success: Authentication successful. Server: test4444 joined discord.general.

Unsuccessful connection to the server:
> \> stdin: /auth xwrong00 wrong wrong
>
> \> server: Failure: Authentication failed - Provided user secret is not valid.

Attempting to send a message before connecting to the server:
> \> stdin: message
>
> \> server:

Attempting authentication after successful authentication:
> \> stdin: /auth xshmon00 \*secret\* test4444
>
> \> server:

Sending a message to the server:
> \> stdin: message
>
> \> server:

Receiving a message from the server:
> \> stdin: 
>
> \> server: abc: asd

Joining another channel on the server:
> \> stdin: /join discord.general
>
> \> server: Success: Channel discord.general successfully joined. Server: test4444 joined discord.general.

---

## Bibliography

- [RFC2119] Bradner, S. _Key words for use in RFCs to Indicate Requirement Levels_ [online]. March 1997. [cited 2024-02-11]. DOI: 10.17487/RFC2119. Available at: https://datatracker.ietf.org/doc/html/rfc2119

- [RFC5234] Crocker, D. and Overell, P. _Augmented BNF for Syntax Specifications: ABNF_ [online]. January 2008. [cited 2024-02-11]. DOI: 10.17487/RFC5234. Available at: https://datatracker.ietf.org/doc/html/rfc5234

- [RFC9293] Eddy, W. _Transmission Control Protocol (TCP)_ [online]. August 2022. [cited 2024-02-11]. DOI: 10.17487/RFC9293. Available at: https://datatracker.ietf.org/doc/html/rfc9293

- [RFC894] Hornig, C. _A Standard for the Transmission of IP Datagrams over Ethernet Networks_ [online]. April 1984. [cited 2024-02-14]. DOI: 10.17487/RFC894. Available at: https://datatracker.ietf.org/doc/html/rfc894

- [RFC791] Information Sciences Institute, University of Southern California. _Internet Protocol_ [online]. September 1981. [cited 2024-02-14]. DOI: 10.17487/RFC791. Available at: https://datatracker.ietf.org/doc/html/rfc791

- [RFC768] Postel, J. _User Datagram Protocol_ [online]. March 1997. [cited 2024-02-11]. DOI: 10.17487/RFC0768. Available at: https://datatracker.ietf.org/doc/html/rfc768

- [RFC1350] Sollins, D. _The TFTP Protocol (Revision 2)_ [online]. July 1992. [cited 2024-02-12]. DOI: 10.17487/RFC1350. Available at: https://datatracker.ietf.org/doc/html/rfc1350
