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

#include "socket-common.h"


#include "ANutils/linkedlist.h"

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
 * 
 * c -> channel create channel parameter channel name
 * a -> add user to channel -channel -name
 * s -> send message to channel \n -> msg
 * r -> read channel request read of all the channel.
 * 
 * DATA PACKET WILL BE
 * USERNAME
 * COMMAND ARGUMENT
 * data
 * |
 * 
 */

int errorcheck(int val, int targetval, char* msg)
{
	if(val == targetval)
	{
		perror(msg);
		exit(EXIT_FAILURE);
	}
	return val;
}
#define HASHTABLESIZE 4096
typedef struct {
	char* userlist;
	int fd;
} channel;
channel* channel_costructor(char* channelname, int fd)
{
	channel* newitem = malloc(sizeof(channel));
	if(newitem == NULL)
	{
		errorcheck(-1, -1, "failed to allocate space for new challenge");
	}
	newitem->userlist = channelname;
	newitem->fd = fd;
	return newitem;
}

void channel_destructor(channel* ch)
{
	free(ch->userlist); //can cause problems later.
	free(ch);
}
channel* CHANNELHASH[HASHTABLESIZE];
//hash function
unsigned long hash( char *str)
{ 
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % HASHTABLESIZE;
}
//lookup =
channel* lookup(char* key)
{
	return CHANNELHASH[hash(key)];

}

void update(char* key, channel* val)
{
	CHANNELHASH[hash(key)] = val;
}


void AN_protocol_setup()
{
	//create a file to manage channel access
	for(int i = 0; i < HASHTABLESIZE; i++) CHANNELHASH[i] = NULL;	
	
}

#define USERNAME 0
#define COMMAND 1
#define ARGUMENT 2
#define EXTRAUSERNAME 3
int AN_protocol_execute(int sock, char** words,int numofwords)
{
	char buf[2*MAX_MSIZE];
	printf("got here with %d\n", numofwords);
	if(numofwords < 4)
	{
		printf("wrong command\n");
		return -1;
	}
	if(strcmp(words[1], "C") == 0)
	{
		//create file channel and add user as andreas
		if(lookup(words[ARGUMENT]) == NULL)
		{
			//allocate space for userlist for the new channel.
			char* tempval = malloc(sizeof(char)*MAX_MSIZE);
			if(tempval == NULL)
				errorcheck(-1, -1, "malloc failed");
			sprintf(tempval, "%s ", words[USERNAME]);


			char filename[MAX_MSIZE];
			sprintf(filename, "AN/%s", words[ARGUMENT]);
			int fd = errorcheck(open(filename, O_CREAT | O_TRUNC | O_APPEND, S_IRUSR | S_IWUSR), -1, "failed to create channel file");

			update(words[ARGUMENT], channel_costructor(tempval, fd));

			printf("Channel created %s with user %s\n", words[ARGUMENT], words[USERNAME]);
			errorcheck(insist_write(sock, "Channel created\n", sizeof("Channel created\n")) < 0, 1, "failed to write to client");

			

		}
		else{
			printf("channel already exists\n");
			errorcheck(insist_write(sock, "channel already exists\n", sizeof("channel already exists\n"))< 0, 1, "failed to write to client");
		}
		
	}
	else if(strcmp(words[1], "A") == 0)
	{
		if(numofwords < 5)
			goto exitfault;
		//user 
		//add channel user |
		channel* ch = lookup(words[ARGUMENT]);
		char* userlist = ch->userlist;
		if(ch == NULL)
		{
			printf("channel does not exist\n");
			errorcheck(insist_write(sock, "channel does not exist\n", sizeof("channel does not exist\n")) < 0, 1, "failed to write to client");
		}
		else
		{
			//CHANNEL EXISTS.
			if(strstr(userlist, words[USERNAME]) == NULL)
			{
				printf("user %s does not have access to write to channel %s \n Those who have are %s\n",words[USERNAME],words[ARGUMENT],userlist);
				sprintf(buf, "user %s does not have access to write to channel %s \n",words[USERNAME],words[ARGUMENT]);
				insist_write(sock, buf, strlen(buf));
				goto exitfault;
			}
			//can check for access
			strcat(userlist, words[EXTRAUSERNAME]);
			strcat(userlist, " ");
			printf("added user to userlist %s\n", words[EXTRAUSERNAME]);
			errorcheck(insist_write(sock, "added user successfully\n", sizeof("added user successfully\n")) < 0, 1, "failed to write to client");
		}
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
		printf("wrong command\n");
		return -1;
	}
	return 0;

}




int main(void)
{
	char buf[MAX_MSIZE];
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
	for (;;) {

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
		


		//Variables used for handling a customer
		for(int i = 0; i < MAX_MSIZE; i++) buf[i] = 0;
		char* words[MAX_MSIZE];

		/* We break out of the loop when the remote peer goes away */
		for (;;) {
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
			int numofwords = splitToWords(buf, strlen(buf), words, MAX_MSIZE);

			//IMPLEMENTING PROTOCOL...
			AN_protocol_execute(newsd,words, numofwords);

			
			// toupper_buf(buf, n);
			// if (insist_write(newsd, buf, n) != n) {
			// 	perror("write to remote peer failed");
			// 	break;
			// }
		}
exit:
		/* Make sure we don't leak open files */
		if (close(newsd) < 0)
			perror("close");
	}

	/* This will never happen */
	return 1;
}

