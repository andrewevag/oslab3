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
int ANAFILE;
void AN_protocol_setup()
{
	//create a file to manage channel access
	ANAFILE = errorcheck(open("ANaccess", O_CREAT | O_TRUNC | O_APPEND, S_IRUSR | S_IWUSR) < 0, 1,"failed to create the access file\n");
	

}

int AN_protocol_execute(char** words,int numofwords)
{
	printf("got here with %d\n", numofwords);
	if(numofwords < 4)
	{
		printf("wrong command\n");
		return -1;
	}
	if(strcmp(words[1], "C") == 0)
	{
		//create file channel and add user as andreas
		insist_write(ANAFILE, )
	}
	else if(strcmp(words[1], "A") == 0)
	{

	}
	else if(strcmp(words[1], "S") == 0)
	{

	}
	else if(strcmp(words[1], "R") == 0)
	{

	}
	else
	{
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
			AN_protocol_execute(words, numofwords);

			
			
			goto exit;
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

