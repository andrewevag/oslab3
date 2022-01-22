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
#include "anutil.h"
#include <sys/wait.h>
#include "linkedlist.h"
#include "SSI.h"

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
	
	int newsd;
	char tempbuf[20];
	char tempbuf2[20];
	/* Make sure a broken connection doesn't kill us */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, child_handler);
	signal(SIGINT, int_handler);
	//fork director.
	//add the socket to for director.
	char* directorArgs[] = {"./serverAN", socketname, NULL};
	dpid = subprocesscall("./serverAN", directorArgs);
	
	/* Create TCP/IP socket, used as main chat channel */
	SSI *s = ssi_open(NULL, TCP_PORT, true, TCP_BACKLOG);

	/* Loop forever, accept()ing connections */
	while(1) {
		newsd = ssi_server_accept(s);
		printf("[father] got new client @%d\n", newsd);
		
		sprintf(tempbuf, "%d", newsd);
		sprintf(tempbuf2, "%d", s->ssi_fd);
		char* args[] = {"./childServer", tempbuf, socketname, tempbuf2,NULL};
		subprocesscall("./childServer", args);
		sleep(1);
		close(newsd);

	}
	
}