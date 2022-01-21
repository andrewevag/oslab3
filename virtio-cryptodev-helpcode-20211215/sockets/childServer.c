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


int main(int argc, char** argv){

	errorcheck(isNumber(argv[1]), false, "[child] invalid socket @type : Not A number");
	printf("[child is handling]\n");
	int newsd = atoi(argv[1]);
	const char * socketname = argv[2];
	char* pack;
	packet tempp;
	char* temp;
	/*
	 * Handle incoming packages and send them to director;
	 */
	while(1) {
		pack = packet_parse(newsd);
		if(pack == NULL){
			tempp = packetServerF("Failed to parse Packet");
			send_packet(&tempp, newsd);
			goto exit;
		}
		

		
	}

exit:
	close(newsd);

	exit(0);
}