/*
 * socket-server.c
 * Simple TCP/IP communication using sockets
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 */

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
#include "ANutils/linkedlist.h"
#include "ANutils/channel.h"
#include "ANutils/user.h"
#include "ANutils/message.h"
#include "utillib/anutil.h"
#include "utillib/Astring.h"
#include <sys/un.h>
/*
 * Arguments are commands
 * REQUESTS TO SERVER
 * <username> C <channelname> | 
 * <username> A <password> <channel> <user> |
 * <username> CU <password> |
 * <username> S <password> <channel> <msg> =<...msg> |
 * <username> R <password> <channel>  <msgnumber> |
 * 
 * 
 * RESPONSES FROM SERVER
 * REQUEST DONE |
 * REQUEST FAILED |
 * MSG <channel name> <number>
 * <...> |
 * NO MORE MESSAGES
 */
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
int AN_protocol_execute(int sock, char* original_msg, int bytesread ,char** words,int numofwords)
{
	for(int i =0; i < bytesread+1; i++)
	{
		if(original_msg[i]=='|'){
			original_msg[++i] = 0;
			break;
		}
	}
	char buf[2*MAX_MSIZE];
	printf("[director] got here with %d and msg :%s \n", numofwords, original_msg);
	if(numofwords < 4)
	{
		printf("wrong command\n");
		return -1;
	}
	if(strcmp(words[COMMAND], "CU") == 0)
	{
		//add user to user list
		userlist = cons(user_constructor(words[USERNAME], words[PASSWORD]), userlist);
		printf("username : %s, password %s\n", ((user*)head(userlist))->username, ((user*)head(userlist))->password);
		printf("list length %d\n", listlength(userlist));
		const_safe_write(sock, "REQUEST DONE |");
	}
	else if(strcmp(words[1], "C") == 0)
	{
		printf("entered C with %d words and msg %s\n", numofwords,original_msg);
		//run channel list and see that name is available
		forEachList(channellist, ch)
		{
			if(strcmp(words[ARGUMENT],((channel*)getData(ch))->name) == 0)
			{
				const_safe_write(sock, "REQUEST FAILED |");
				goto exitfault;
			}
		}

		channellist = cons(channel_costructor(words[ARGUMENT], cons(user_constructor(words[USERNAME], "") ,emptyList), emptyList), channellist);
		const_safe_write(sock, "REQUEST DONE |");
		printf("channelname : %s username : %s, password %s\n", ((channel*)head(channellist))->name, ((user*)head(((channel*)head(channellist))->userlist))->username, ((user*)head(((channel*)head(channellist))->userlist))->password);
		printf("list length %d\n", listlength(channellist));
	}
	else if(strcmp(words[1], "A") == 0)
	{
		if(numofwords < 6)
			goto exitfault;
		// //user 
		// //add channel user |
		//validate user. 
		if(!validateUser(words[USERNAME], words[PASSWORD]))
		{
			printf("failed to validate\n");
			const_safe_write(sock, "REQUEST FAILED |");
			goto exitfault;
		}
		//check that the channel exists.
		
		channel* req = checkChannelExistance(words[CHANNELPARAM]);
		if(req == NULL){
			const_safe_write(sock, "REQUEST FAILED |");
			printf("channel not found requested = %s\n", words[CHANNELPARAM]);
			goto exitfault;
		}
		if(!checkUserExistance(words[EXTRAUSERNAME]))
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("no such user exists %s\n", words[EXTRAUSERNAME]);
			goto exitfault;
		}

		//check that he has access to the channel.
		bool flag = checkAccessToChannel(req, words[USERNAME]);
		if(!flag){
			const_safe_write(sock, "REQUEST FAILED |");
			printf("user %s does not have access to %s\n", words[USERNAME], req->name);
			goto exitfault;
		}
		//add user to the channel.
		req->userlist = cons(user_constructor(words[EXTRAUSERNAME], ""), req->userlist);
		const_safe_write(sock, "REQUEST DONE |");
		printf("added user %s to %s\n", words[EXTRAUSERNAME], req->name);
	}
	else if(strcmp(words[1], "S") == 0)
	{
		//as a server we receive the message here.
		//validateUser
		if(!validateUser(words[USERNAME], words[PASSWORD]))
		{
			printf("failed to validate\n");
			const_safe_write(sock, "REQUEST FAILED |");
			goto exitfault;
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(words[CHANNELPARAM]);
		if(req == NULL)
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("channel not found requested = %s\n", words[CHANNELPARAM]);
			goto exitfault;
		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, words[USERNAME]))
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("user %s does not have access to %s\n", words[USERNAME], req->name);
			goto exitfault;
		}
		//add new message to the channel.
		//now we have to keep from the original message only the portion that is after 
		int i = 0; int len = strlen(original_msg);
		for(i = 0 ; i < strlen(original_msg); i++)
		{
			if(original_msg[i] == '=')
			{
				i++; 
				break;
			}
		}
		memcpy(buf, (original_msg+i), len-i);
		for(int k = len-i; k < sizeof(buf); k++) buf[k] = 0;
		printf("buf = %s\n", buf);
		int previousId = (req->messagelist == emptyList) ? -1 : (((message*)head(req->messagelist))->id);
		req->messagelist = cons( message_constructor(++previousId, buf , user_constructor(words[USERNAME], "")) , req->messagelist);
		const_safe_write(sock, "REQUEST DONE |");
		printf("msg = %s to %s\n", (((message*)(head(req->messagelist)))->text), req->name);

	}
	else if(strcmp(words[1], "R") == 0)
	{
		if (numofwords < 6)
			goto exitfault;
		//validate user
		if(!validateUser(words[USERNAME], words[PASSWORD]))
		{
			printf("failed to validate\n");
			const_safe_write(sock, "REQUEST FAILED |");
			goto exitfault;
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(words[CHANNELPARAM]);
		if(req == NULL)
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("channel not found requested = %s\n", words[CHANNELPARAM]);
			goto exitfault;
		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, words[USERNAME]))
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("user %s does not have access to %s\n", words[USERNAME], req->name);
			goto exitfault;
		}
		//no we have to give all the messages that are greater or equal to the requested one.
		//might fix to recursion in another lifetime
		int maxid = ((message*)head(req->messagelist))->id;
		if(!isNumber(words[MSGNUM]))
		{
			const_safe_write(sock, "REQUEST FAILED |");
			printf("%s is not a number\n", words[MSGNUM]);
			goto exitfault;
		}
		int id = atoi(words[MSGNUM]);
		while(id <= maxid)
		{
			forEachList(req->messagelist, i)
			{
				message* msg = getData(i);
				if(msg->id == id)
				{
					//send message id+1
					/* MSG <channel name> <number>
 					 * <...> |
					 */
					for(int i =0 ; i< sizeof(buf) ; i++) buf[i] = 0;
					sprintf(buf, "MSG %s %d\n%s", req->name, id, msg->text);
					safe_write(sock, buf, strlen(buf));
					id++;
					break;
				}
			}
			
		}
		sprintf(buf, "NO NEW MESSAGES %s |", req->name);
		safe_write(sock, buf, strlen(buf));
	}
	else
	{
exitfault:
		printf("failed command\n");
		return -1;
	}
	return 0;

}

#define MAX_CLIENT_QUEUE 20

int main(int argc, char** argv)
{
	printf("[director] socketname = %s\n", argv[1]);
	const char* socketname = argv[1];
	AN_protocol_setup();
	struct sockaddr_un addr;
	
	int sock = errorcheck(socket(AF_UNIX, SOCK_STREAM, 0), -1, "[director] error creating socket");
	int client;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, socketname, sizeof(addr.sun_path)-1);
	// errorcheck(unlink(socketname), -1, "[director] failed to unlink socket");
	errorcheck(bind(sock, (struct sockaddr*)&addr, sizeof(addr)), -1, "[director] failed to bind socket");
	errorcheck(listen(sock, MAX_CLIENT_QUEUE), -1, "[director] listen failed");

	int readchars = 0;
	char buf[4096];
	int numofwords;
	char* words[MAX_MSIZE];
	char* packet, *original_msg;
	string* s;
	memset(buf, 0, sizeof(buf));
	while(1)
	{
		readchars = 0;
		client = accept(sock, NULL, NULL);
		if(client < 0)
		{
			printf("failed to acccept\n");
			continue;
		}
		printf("connection\n");
		while(strchr(buf, '|') == NULL)
		{
			int temp =  read(client, buf, sizeof(buf));
			if(temp <= 0){
				if(temp == 0)
					printf("[director] closed connection\n");
				else
					printf("[director] error reading from client\n");
				
				
				break;

			}
			readchars+=temp;
		}
		
		printf("[director] packet :%s{0}\n", buf);
		s = string_constructor(buf, strlen(buf));
		// string_appendStr(s, string_constructor("| ",sizeof("| ")));
		packet = string_tocharpointerNULLTERM(s);
		original_msg = string_tocharpointerNULLTERM(s);
		numofwords = splitToWords(packet, strlen(packet), words, MAX_MSIZE);
		//execute protocol
		AN_protocol_execute(client, original_msg, strlen(original_msg), words, numofwords);

		free(original_msg);
		free(packet);
		// string_destructor(s);
		close(client);
		memset(buf, 0, sizeof(buf));

	}


}

