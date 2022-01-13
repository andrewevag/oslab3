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


/*
 * Arguments are commands
 * REQUESTS TO SERVER
 * <username> C <channelname> | 
 * <username> A <channel> <user> <password> |
 * <username> CU <password> |
 * <username> S <password> <channel> <msg> |
 * <username> R <password> <channel>  <msgnumber> |
 * 
 * 
 * RESPONSES FROM SERVER
 * REQUEST DONE |
 * REQUEST FAILED |
 * MSG <channel name> <number>
 * <...> |
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
	forEachList(userlist, i)
	{
		user* curruser = getData(i);
		if((strcmp(curruser->username, usrname) == 0) && (strcmp(curruser->password, pwd)))
		{
			return true;
		}
	}
	return false;
}

#define USERNAME 0
#define COMMAND 1
#define ARGUMENT 2
#define EXTRAUSERNAME 3
#define PASSWORD 2
int AN_protocol_execute(int sock, char* original_msg ,char** words,int numofwords)
{
	char buf[2*MAX_MSIZE];
	printf("got here with %d\n", numofwords);
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
		if(numofwords < 7)
			goto exitfault;
		// //user 
		// //add channel user |
		//validate user. 
		//check that the channel exists.
		//check that he has access to the channel.
		//add user to the channel.
	}
	else if(strcmp(words[1], "S") == 0)
	{
		
	}
	else if(strcmp(words[1], "R") == 0)
	{

	}
	else
	{
exitfault:
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
			memcpy(bufcpy, buf, strlen(buf)+1);
			int numofwords = splitToWords(buf, strlen(buf), words, MAX_MSIZE);

			//IMPLEMENTING PROTOCOL...
			AN_protocol_execute(newsd, bufcpy, words, numofwords);

		}
exit:
		/* Make sure we don't leak open files */
		if (close(newsd) < 0)
			perror("close");
	}

	/* This will never happen */
	return 1;
}

