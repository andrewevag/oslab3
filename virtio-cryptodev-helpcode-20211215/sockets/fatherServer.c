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
#include "utillib/anutil.h"
#include <sys/wait.h>
#include "ANutils/linkedlist.h"

#define MAX_CLIENT_QUEUE 10

char serverAN[] = "serverAN";
int dpid = 0;
char* socketname = "directorSocket";

void child_handler(int signum)
{
	int status=0;
	int cpid = waitpid(-1, &status, WNOHANG);
	errorcheck(cpid, -1, "failed at wait");
	if(cpid == 0)
		return;
	if(WIFEXITED(status))
	{
		if(cpid == dpid){
			fprintf(stdout, "director exited");
			dpid = 0;
		}
		else fprintf(stdout, "session closed\n");
	}
}

void int_handler(int signum)
{
	if(dpid !=0)
	{
		errorcheck(kill(dpid, SIGTERM), -1,"failed to sigterm director");
		int status=0;
		waitsubprocessexit(dpid, &status);
		errorcheck(unlink(socketname), -1, "failed to unlink socket as a father");
	}
	exit(0);
}


int main(){
	
	char addrstr[INET_ADDRSTRLEN];
	int sd, newsd;
	socklen_t len;
	struct sockaddr_in sa;
	/* Make sure a broken connection doesn't kill us */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, child_handler);
	signal(SIGINT, int_handler);
	//fork director.
	//add the socket to for director.
	char* directorArgs[] = {"./serverAN", socketname, NULL};
	dpid = subprocesscall("./serverAN", directorArgs);
	
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
		
		char tempbuf[20];
		sprintf(tempbuf, "%d", newsd);
		char* args[] = {"./child", tempbuf, socketname ,NULL};
		subprocesscall("./child", args);
		close(newsd);


	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	// struct sockaddr_un addr;
	
	// int sock = errorcheck(socket(AF_UNIX, SOCK_STREAM, 0), -1, "error creating socket");
	// int client;
	// memset(&addr, 0, sizeof(addr));
	// addr.sun_family = AF_UNIX;
	// memcpy(addr.sun_path, serverAN, sizeof(serverAN)-1);
	// unlink(serverAN);
	
	// errorcheck(bind(sock, (struct sockaddr*)&addr, sizeof(addr)), -1, "failed to bind socket");
	// errorcheck(listen(sock, MAX_CLIENT_QUEUE), -1, "listen failed");

	// int readchars = 0; char readchar;
	// while(1)
	// {
	// 	readchars = 0;
	// 	client = accept(sock, NULL, NULL);
	// 	if(client < 0)
	// 	{
	// 		printf("failed to acccept\n");
	// 		continue;
	// 	}
	// 	printf("connection\n");
	// 	while(readchars < 1)
	// 	{
	// 		int temp =  read(client, &readchar, sizeof(char));
	// 		if(temp == -1)
	// 			errorcheck(-1, -1,"read failed");
	// 		readchars+=temp;
	// 	}
	// 	printf("%c\n", readchar);
	// 	close(client);

	// }


}