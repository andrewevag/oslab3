#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include "packet.h"

/**
 * @brief Parses a packet from an open stream (here stream is a file descriptor)
 * 
 * @param fd the file descriptor
 * @return a pointer to a packet if parsed correctly else a NULL pointer.
 */
packet* packet_parse(int fd);

/**
 * @brief Creates the msg to be sent through communication line from a packet.
 * 
 * @param p A pointer to the packet
 * @return char* a new string exactly the size of the packet [needs to be freed], NULL on bad packet.
 * 
 */
char* packet_format(packet *p);




packet format_wrapper(PACKET_TYPE t, COMMAND_TYPE cmd, char* arg1,
char* arg2, char* arg3, char* arg4, int length, int id, 
char* body);

#define packetCU(username, password) format_wrapper(QUESTION, CREATE_USER, username, password, NULL, NULL, 0, 0, NULL)

#define packetC(username, channelname) format_wrapper(QUESTION, CREATE_CHANNEL, username, NULL, channelname, NULL, 0, 0, NULL)

#define packetA(username, password, channelname, secondusername) format_wrapper(QUESTION, ADD_USER, username, password, channelname, secondusername, 0, 0, NULL)

#define packetR(username, password, channelname, id) format_wrapper(QUESTION, READ, username, password, channelname, NULL, 0, id, NULL)

#define packetS(username, password, channelname, msg) format_wrapper(QUESTION, SEND, username, password, channelname, NULL, strlen(msg),0, msg)

#define packetServerS(msg) format_wrapper(ANSWER, SERVER_SUCCESS, NULL, NULL, NULL, NULL, strlen(msg), 0, msg)

#define packetServerF(msg) format_wrapper(ANSWER, SERVER_FAILURE, NULL, NULL, NULL, NULL, strlen(msg), 0, msg)

/**
 * @brief Sends a packet formatted to the 
 * 
 * @param p packet
 * @param fd file descriptor
 * @return int -1 if failed insist_write is used.
 */
int send_packet(packet p, int fd);



#endif