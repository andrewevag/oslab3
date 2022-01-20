#include "inttypes.h"
#include <stdlib.h>

#define HEADERLENGTH 34
#define PACKET_TYPE_LENGTH 8
#define PACKET_COMMAND_LENGTH 8
#define PACKET_USERNAME_LENGTH 8
#define PACKET_PASSWORD_LENGTH 8
#define PACKET_EXTRA_ARG_1_LENGTH 8
#define PACKET_EXTRA_ARG_2_LENGTH 8
#define PACKET_MAX_BODY_LENGTH 8

typedef enum PACKET_TYPE {
	REQUEST, RESPONSE
} PACKET_TYPE ;

typedef enum COMMAND_TYPE{
	CREATE_USER, CREATE_CHANNEL, ADD_USER, SEND, READ, SERVER_SUCCESS, SERVER_FAILURE
} COMMAND_TYPE;

typedef struct packet{
	uint8_t packet_type;// 1 byte
	uint8_t command;	// 1 byte
	char username[8];   // 8 bytes
	char password[8];   // 8 bytes
	char extraArg1[8];  // 8 bytes
	char extraArg2[8];  // 8 bytes
	char body[256];     // 256 bytes
} packet;

// need to be 0 tho so a memset is mandatory before the packet.
// packet p = (packet) {REQUEST, CREATE_USER, "andreas", "pass", NULL, NULL, NULL};
