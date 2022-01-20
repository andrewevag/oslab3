#include "SSI.h"
#include <stdbool.h>
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
#include <arpa/inet.h>
#include <netinet/in.h>
#include "SafeCalls.h"


SSI* ssi_open(char* name, uint16_t port, bool server, int tcp_backlog)
{
	if(server) {
		int sd;
		struct sockaddr_in sa;
		
		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}
		fprintf(stderr, "Created TCP socket\n");

		/* Bind to a port */
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		
		if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			perror("bind");
			exit(1);
		}
		fprintf(stderr, "Bound TCP socket to port %d\n", port);
		
		if (listen(sd, tcp_backlog) < 0) {
			perror("listen");
			exit(1);
		}

		SSI* ssi = sfmalloc(sizeof(SSI));
		ssi->ssi_fd = sd;
		memset(ssi->ssi_name_server, 0, sizeof(ssi->ssi_name_server));

		ssi->port = port;
		ssi->ssi_server = true;
		return ssi;
	}
	//client
	else{
		int sd;
		char *hostname = name;
		struct hostent *hp;
		struct sockaddr_in sa;

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}
		fprintf(stderr, "Created TCP socket\n");

		/* Look up remote hostname on DNS */
		if ( !(hp = gethostbyname(hostname))) {
			printf("DNS lookup failed for host %s\n", hostname);
			exit(1);
		}

		/* Connect to remote TCP port */
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		fprintf(stderr, "Connecting to remote host... "); fflush(stderr);
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			perror("connect");
			exit(1);
		}
		fprintf(stderr, "Connected.\n");
		SSI* ssi = sfmalloc(sizeof(SSI));
		ssi->port = port;
		ssi->ssi_fd = sd;
		strcpy(ssi->ssi_name_server, hostname);
		ssi->ssi_server = false;
		return ssi;
	}
}


int ssi_server_accept(SSI* ssip)
{
	if(!ssip->ssi_server){
		fprintf(stderr, "You gave me a client to accpet a server ...\n");
		return -1;
	}
	struct sockaddr_in sa;
	socklen_t len;
	int newsd;
	char addrstr[INET_ADDRSTRLEN];
	int sd = ssip->ssi_fd;

	if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
		perror("accept");
		return -1;
	}
	if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
		perror("could not format IP address");
		return -1;
	}
	fprintf(stderr, "Incoming connection from %s:%d\n", addrstr, ntohs(sa.sin_port));

	return newsd;
}


bool ssi_close(SSI* ssip)
{
	//  i leave space for future edits to it.
	if(ssip->ssi_server)
	{
		close(ssip->ssi_fd);
		free(ssip);
		return true;
	}
	else
	{
		close(ssip->ssi_fd);
		free(ssip);
		return true;
	}
}
