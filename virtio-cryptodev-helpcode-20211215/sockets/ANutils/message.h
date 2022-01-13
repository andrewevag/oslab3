#include "user.h"



typedef struct{
	int id;
	char* text;
	user* userfrom;
} message;


message* message_constructor(int id, char* text, user* usfrom);


void message_destructor(message* msg);