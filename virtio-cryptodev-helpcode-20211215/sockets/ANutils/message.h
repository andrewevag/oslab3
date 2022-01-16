#pragma once
#include "user.h"



typedef struct{
	int id;
	char* text;
	user* userfrom;
} message;


message* message_constructor(int id, char* text, user* usfrom);
message* message_constructor_size(char* text, int len);


void message_destructor(message* msg);
void message_destructor_size(message* msg);