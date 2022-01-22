#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "socket-common.h"
#include "linkedlist.h"
#include "channel.h"
#include "user.h"
#include "message.h"
#include "anutil.h"
#include "Astring.h"
#include <sys/un.h>
#include "SSI.h"
#include "packet.h"
#include "packet_parser.h"

list* userlist;
list* channellist;
void AN_protocol_setup()
{
	userlist = emptyList;
	channellist = emptyList;
}


bool validateUser(char* usrname, char* pwd)
{
	printf("@validate with %s and %s\n", usrname, pwd);
	forEachList(userlist, i)
	{
		user* curruser = getData(i);
		if((strcmp(curruser->username, usrname) == 0) && (strcmp(curruser->password, pwd) == 0))
		{
			return true;
		}
	}
	return false;
}

channel* checkChannelExistance(char *name)
{
	channel* req = NULL;
	forEachList(channellist, i)
	{
		channel* ch = getData(i);
		if(strcmp(ch->name, name) == 0)
			req = ch;
	}
	return req;
}

bool checkAccessToChannel(channel* req, char* usrname)
{
	bool flag = false;
	forEachList(req->userlist, i)
	{
		user* u = getData(i);
		if(strcmp(u->username, usrname) == 0)
			flag = true;
	}
	return flag;
}

bool checkUserExistance(char* usrname)
{	
	user* u;
	forEachList(userlist, i)
	{
		u = getData(i);
		if(strcmp(usrname, u->username)==0)
			return true;
	}
	return false;
}

#define USERNAME 0
#define COMMAND 1
#define ARGUMENT 2
#define EXTRAUSERNAME 4
#define CHANNELPARAM 3
#define PASSWORD 2
#define MSGNUM 4
packet AN_protocol_execute(packet* p)
{
	char buf[BUFSIZ];
	memset(buf, 0, sizeof(buf));
	if(p->command == CREATE_USER)
	{
		//add user to user list
		if(checkUserExistance(p->arg1)){
			return packetServerS("User already exists");
		}
		userlist = cons(user_constructor(p->arg1, p->arg2), userlist);
		printf("[director] Created user with username : %s, password : %s\n", ((user*)head(userlist))->username, ((user*)head(userlist))->password);
		printf("list length %d\n", listlength(userlist));
		return packetServerS("User created sucessfully.");
	}
	else if(p->command == CREATE_CHANNEL)
	{
		forEachList(channellist, ch)
		{
			if(strcmp(p->arg3,((channel*)getData(ch))->name) == 0)
			{
				printf("[director] channel already exists\n");
				return packetServerF("Channel already exists.");
			}
		}

		channellist = cons(channel_costructor(p->arg3, cons(user_constructor(p->arg1, "") ,emptyList), emptyList), channellist);
		printf("[director] Created channel with channelname : %s username : %s, password %s\n", ((channel*)head(channellist))->name, ((user*)head(((channel*)head(channellist))->userlist))->username, ((user*)head(((channel*)head(channellist))->userlist))->password);
		printf("[director] Channellist length %d\n", listlength(channellist));
		sprintf(buf, "Created channel %s", p->arg3);
		return packetServerS(buf);
		
	}
	else if(p->command == ADD_USER)
	{
		// //user 
		// //add channel user |
		//validate user. 
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//check that the channel exists.
		
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL){			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");
		}
		if(!checkUserExistance(p->arg4))
		{
			
			printf("[director] no such user exists %s\n", p->arg4);
			return packetServerF("No such user exists");
		}

		//check that he has access to the channel.
		bool flag = checkAccessToChannel(req, p->arg1);
		if(!flag){
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		//add user to the channel.
		req->userlist = cons(user_constructor(p->arg4, ""), req->userlist);
		printf("[director] added user %s to %s\n", p->arg4, req->name);
		sprintf(buf, "added user %s to %s\n", p->arg4, req->name);
		return packetServerS(buf);
	}
	else if(p->command == SEND)
	{
		//as a server we receive the message here.
		//validateUser
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL)
		{			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");

		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, p->arg1))
		{			
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		memcpy(buf, p->body, p->length);
		int previousId = (req->messagelist == emptyList) ? -1 : (((message*)head(req->messagelist))->id);
		req->messagelist = cons( message_constructor(++previousId, buf , user_constructor(p->arg1, "")) , req->messagelist);
		printf("[director] msg = %s to %s\n", (((message*)(head(req->messagelist)))->text), req->name);
		return packetServerS("Sent packet successfully");

	}
	else if(p->command == READ)
	{

		//validateUser
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL)
		{			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");

		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, p->arg1))
		{			
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		//no we have to give all the messages that are greater or equal to the requested one.
		//might fix to recursion in another lifetime
		int maxid = ((message*)head(req->messagelist))->id;
		int id = p->id;
		forEachList(req->messagelist, i)
		{
			message* msg = getData(i);
			if(msg->id == id)
			{
				packet temp =  packetServerS(msg->text);
				temp.id = maxid;
				return temp;
			}
		}
		return packetServerF("No such packet");
	}
	else
	{
		printf("[director] failed command\n");
		return packetServerF("Failed Question");
	}

}

#define MAX_CLIENT_QUEUE 20

int main(int argc, char** argv)
{
	printf("[director] socketname = %s\n", argv[1]);
	char* socketname = argv[1];
	AN_protocol_setup();
	
	SSI* s = ssi_un_open(socketname, true, MAX_CLIENT_QUEUE);
	packet p, response;
	int client;
	while(1)
	{
		client = ssi_un_server_accept(s);
		//read the packet.
		insist_read(client, &p, sizeof(p));
		//execute command based on packet.
		response = AN_protocol_execute(&p);
		insist_write(client, &response, sizeof(response));
		close(client);
	}
}

