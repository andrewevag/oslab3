#ifndef PACKET_H
#define PACKET_H

#include "inttypes.h"
#include <stdlib.h>

#define PACKET_HEADERLENGTH 34
#define PACKET_TYPE_LENGTH 1
#define PACKET_COMMAND_LENGTH 2
#define PACKET_USERNAME_LENGTH 8
#define PACKET_PASSWORD_LENGTH 8
#define PACKET_CHANNEL_LENGTH 8
#define PACKET_EXTRA_ARG_LENGTH 8
#define PACKET_LENGTH_LENGTH 2
#define PACKET_MAX_BODY_LENGTH 256


#define PACKET_TYPE_OFFSET 0
#define PACKET_COMMAND_OFFSET 1
#define PACKET_USERNAME_OFFSET 3
#define PACKET_PASSWORD_OFFSET 11
#define PACKET_CHANNEL_OFFSET 19
#define PACKET_EXTRA_ARG_OFFSET 27
#define PACKET_LENGTH_OFFSET 35
#define PACKET_BODY_OFFSET 37




typedef enum PACKET_TYPE {
	QUESTION, ANSWER
} PACKET_TYPE ;

typedef enum COMMAND_TYPE{
	CREATE_USER, CREATE_CHANNEL, ADD_USER, SEND, READ, SERVER_SUCCESS, SERVER_FAILURE
} COMMAND_TYPE;

typedef struct packet{
	uint8_t packet_type;// 1 byte
	uint8_t command;	// 1 byte
	char arg1[8];       // 8 bytes
	char arg2[8];       // 8 bytes
	char arg3[8];       // 8 bytes
	char arg4[8];       // 8 bytes
	int length;			//body length
	int id; 			//optional argument in case of read.
	char body[256];     // 256 bytes
} packet;

// need to be 0 tho so a memset is mandatory before the packet.
// packet p = (packet) {QUESTION, CREATE_USER, "andreas", "pass", NULL, NULL,0, NULL};


#endif