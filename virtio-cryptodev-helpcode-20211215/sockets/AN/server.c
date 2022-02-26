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



//server
int main()
{
	printf("[server] commencing my pid to kill me is %d", getpid());
	SSI *server = ssi_open(NULL, TCP_PORT, true, TCP_BACKLOG);
	// signal(SIGINT, SIG_IGN); 
	int newsd;
	while( (newsd = ssi_server_accept(server)) != -1)
	{
		printf("got client\n");
		close(newsd);
	}

	ssi_close(server);

}