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

<<<<<<< HEAD

#include "ANutils/linkedlist.h"
#include "ANutils/channel.h"
#include "ANutils/user.h"
#include "ANutils/message.h"
#define MAX_MSIZE 4096


/* Convert a buffer to upercase */
void toupper_buf(char *buf, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++)
		buf[i] = toupper(buf[i]);
}


/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt)
{
	ssize_t ret;
	size_t orig_cnt = cnt;
	
	while (cnt > 0) {
	        ret = write(fd, buf, cnt);
	        if (ret < 0)
	                return ret;
	        buf += ret;
	        cnt -= ret;
	}

	return orig_cnt;
}


void safe_write(int fd, const void *buf, size_t cnt){
	errorcheck(insist_write(fd, buf, cnt) <0, 1, "failed to send @safe write");
}
#define const_safe_write(fd, cswbuf) safe_write(fd, cswbuf, sizeof(cswbuf))

int splitToWords(char* str, int length,char** words, int maxwords){
	// "delimiters are \n and space"
	int numofwords = 0;
	words[numofwords++] = str; 
	for(int i = 0; i < length; i++)
	{	
		if(str[i] == ' ' || str[i] == '\n' )
		{
			str[i++] = 0;
			while(str[i] == ' ' || str[i] == '\n'){
				i++;
			}
			words[numofwords++] = &(str[i]);
			if(numofwords >= maxwords){
				return numofwords-1;
			}
		}
	}
	return numofwords-1;
}

list* splitToPackets(char* str, int length, int* restleft)
{
	list* l = emptyList;
	int i,j;
	for(i = 0, j = 0; j < length; j++){
		if(str[j] == '|'){
			char* start = &(str[i]);
			j++;
			int size = j-i+1;
			l = cons(message_constructor_size(start,size),l);
			j += 2;
			i = j;
		}
	}
	j = 0;
	while(i < length){
		str[j++] = str[i];
		i++;
	}
	*restleft = j;

	l = reverse(l);
	return l;
}
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
=======
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9
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
<<<<<<< HEAD
	for(int i =0; i < bytesread+1; i++)
	{
		if(original_msg[i]=='|'){
			original_msg[++i] = 0;
			break;
		}
	}
	char buf[2*MAX_MSIZE];
	printf("got here with %d\n", numofwords);
	if(numofwords < 4)
	{
		printf("wrong command\n");
		goto exitfault;
		
	}
	if(strcmp(words[COMMAND], "CU") == 0)
=======
	char buf[BUFSIZ];
	memset(buf, 0, sizeof(buf));
	if(p->command == CREATE_USER)
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9
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
		// memcpy(buf, p->body, p->length);
		snprintf(buf,5+strlen(p->arg1)+p->length ,"[%s]\t %s", p->arg1, p->body);
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
<<<<<<< HEAD
exitfault:
		const_safe_write(sock, "REQUEST FAILED |");
		printf("failed command\n");
		return -1;
=======
		printf("[director] failed command\n");
		return packetServerF("Failed Question");
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9
	}

}

#define MAX_CLIENT_QUEUE 20

int main(int argc, char** argv)
{
	printf("[director] socketname = %s\n", argv[1]);
	char* socketname = argv[1];
	AN_protocol_setup();
<<<<<<< HEAD
	/* Make sure a broken connection doesn't kill us */
	signal(SIGPIPE, SIG_IGN);

	/* Create TCP/IP socket, used as main chat channel */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	fprintf(stderr, "Created TCP socket\n");

	/* Bind to a well-known port */
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(TCP_PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		perror("bind");
		exit(1);
	}
	fprintf(stderr, "Bound TCP socket to port %d\n", TCP_PORT);

	/* Listen for incoming connections */
	if (listen(sd, TCP_BACKLOG) < 0) {
		perror("listen");
		exit(1);
	}

	/* Loop forever, accept()ing connections */
	while(1) {

		fprintf(stderr, "Waiting for an incoming connection...\n");

		/* Accept an incoming connection */
		len = sizeof(struct sockaddr_in);
		if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
			perror("accept");
			exit(1);
		}
		if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
			perror("could not format IP address");
			exit(1);
		}
		fprintf(stderr, "Incoming connection from %s:%d\n",
			addrstr, ntohs(sa.sin_port));
		

		//ACTUAL HANDLING OF CLIENT.
		
		//Variables used for handling a customer
		for(int i = 0; i < MAX_MSIZE; i++) buf[i] = 0;
		char* words[MAX_MSIZE];
		/* We break out of the loop when the remote peer goes away */
		while(1) {
			// | msg ends at this character.
			//maximum message length 
			int readbytes = 0;
			
			while(strchr(buf, '|') == NULL){
				n = read(newsd, buf+readbytes, sizeof(buf));
				readbytes+=n;
				if (n <= 0) {
					if (n < 0){
						perror("read from remote peer failed");
						goto exit;
					}
					else{
						fprintf(stderr, "Peer went away\n");
						goto exit;
					}
					
				}
				
			}
			
			// memcpy(bufcpy, buf, readbytes);
			// int numofwords = splitToWords(buf, readbytes, words, MAX_MSIZE);
			
			//IMPLEMENTING PROTOCOL...
			int readbytes2, numofwords;
			list* packetList = splitToPackets(buf, readbytes, &readbytes2);
			forEachList(packetList, packet)
			{
				message* msg = getData(packet);
				memcpy(bufcpy, msg->text, strlen(msg->text));
				numofwords = splitToWords(msg->text, strlen(msg->text), words, MAX_MSIZE);
				AN_protocol_execute(newsd, bufcpy, strlen(msg->text), words, numofwords);
			}
			deleteList(tail(packetList), (void (*)(void*))message_destructor_size);
			printf("got here\n");
			message_destructor_size(head(packetList));
			packetList = emptyList;
			memset(buf, 0, sizeof(buf));
			memset(bufcpy, 0, sizeof(bufcpy));
			
		}
exit:
		/* Make sure we don't leak open files */
		if (close(newsd) < 0)
			perror("close");
=======
	
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
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9
	}
}

