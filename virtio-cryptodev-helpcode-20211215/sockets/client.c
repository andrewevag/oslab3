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
#include <sys/wait.h>
#include <sys/un.h>
#include "linkedlist.h"
#include "anutil.h"
#include "Astring.h"
#include "packet.h"
#include "packet_parser.h"
#include "SSI.h"


int main(int argc, char** argv)
{
	if(argc != 3){
		printf("Usage ./client <address> <port>\n");
		exit(1);
	}
	int port = atoi(argv[2]);
	SSI* s = ssi_open("localhost", port, false, 0);

	packet p = packetCU("andreas", "passwd");
	printf("insist write returned %ld\n", insist_write(s->ssi_fd, &p, sizeof(p)));

	insist_read(s->ssi_fd, &p, sizeof(p));
	printf("%s\n", p.body);

	ssi_close(s);
	return 0;
}

