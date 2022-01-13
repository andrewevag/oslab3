#include "channel.h"
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "message.h"


channel* channel_costructor(char* name, list* userlist, list* messagelist)
{
	channel* newitem = malloc(sizeof(channel));
	if(newitem == NULL)
	{
		errorcheck(-1, -1, "failed to allocate space for new challenge");
	}
	newitem->userlist = userlist;
	newitem->messagelist = messagelist;
	char* str = malloc(strlen(name)+1);
	newitem->name = memcpy(str, name, strlen(name)+1);
	return newitem;
}

void channel_destructor(channel* ch)
{
	deleteList(ch->messagelist, message_destructor);
	deleteList(ch->userlist, user_destructor);
	free(ch->name);
	free(ch);
}