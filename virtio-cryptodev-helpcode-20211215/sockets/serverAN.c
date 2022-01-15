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

bool isNumber(char* s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (!isdigit(s[i]))
              return false;
    }
    return true;
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
	printf("got here with %d\n", numofwords);
	if(numofwords < 4)
	{
		printf("wrong command\n");
		goto exitfault;
		
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
		const_safe_write(sock, "REQUEST FAILED |");
		printf("failed command\n");
		return -1;
	}
	return 0;

}




int main(void)
{
	char buf[MAX_MSIZE];
	char bufcpy[MAX_MSIZE];
	char addrstr[INET_ADDRSTRLEN];
	int sd, newsd;
	ssize_t n;
	socklen_t len;
	struct sockaddr_in sa;
	AN_protocol_setup();
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
	}

	/* This will never happen */
	return 1;
}

