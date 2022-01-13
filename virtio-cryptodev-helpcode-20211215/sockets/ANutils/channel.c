#include "channel.h"
#include <stdlib.h>
#include <string.h>



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
	// free(ch->userlist); //can cause problems later.
	// free(ch)
	free(ch);
}