#include "linkedlist.h"
#include "message.h"
#include <stdlib.h>
#include "string.h"




message* message_constructor(int id, char* text, user* usfrom)
{
	message* nm = malloc(sizeof(message));
	if(nm == NULL)
		errorcheck(-1, -1, "failed to allocate memory for message");
	
	char* nt = malloc(sizeof(char)*(strlen(text)+1));
	if(nt == NULL)
		errorcheck(-1, -1, "failed to allocate memory for text in msg");

	nt = memcpy(nt, text, strlen(text)+1);

	nm->id=id;
	nm->text = nt;
	nm->userfrom=usfrom;
	return nm;
}


void message_destructor(message* msg)
{
	free(msg->text);
	user_destructor(msg->userfrom);
	free(msg);
}