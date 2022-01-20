
#pragma once
#include "linkedlist.h"

typedef struct {
	char* name;
	list* userlist;
	list* messagelist;
} channel;



channel* channel_costructor(char* name, list* userlist, list* messagelist);

void channel_destructor(channel* ch);