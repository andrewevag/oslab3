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
#include "cryptops.h"


int main(int argc, char** argv){

	errorcheck(isNumber(argv[1]), false, "[child] invalid socket @type : Not A number");
	
	int newsd = atoi(argv[1]);
	//close inherited server
	close(atoi(argv[3]));
	
	char * socketname = argv[2];
	printf("[child] is handling client @%d \n", newsd);
	packet tempp;
	SSI* dirsock;
	int nread, readbytes = 0;;
	/*
	 * Handle incoming packages and send them to director;
	 */
	while(1) {
		//read the packet.
		while(readbytes < sizeof(tempp)){
			nread = read(newsd, ((void*)&tempp) + readbytes, sizeof(tempp) - readbytes); //mini bug read the leftover bytes. and plus the offset.
			if(nread == 0){
				printf("[child] connection closed exitting\n");
				exit(0);
			}
			if(nread < 0){
				printf("[child] failed to read will now exit\n");
			}
			readbytes += nread;
		}
		packet res;
		decryption(&tempp ,&res, sizeof(tempp));
		fprintf(stderr, "[child] read %d bytes\n", readbytes);
		memcpy(&tempp, &res, sizeof(tempp));
		readbytes = 0;
		dirsock = ssi_un_open(socketname, false, 0);
		if (insist_write(dirsock->ssi_fd, &tempp, sizeof(tempp)) < 0)
		{
			fprintf(stderr, "failed to write to server\n");
			tempp = packetServerF("Server is down");
			encrypt_insist_write(newsd, &tempp, sizeof(tempp));
			exit(1);
		}
		//read the new packet - response.
		insist_read(dirsock->ssi_fd, &tempp, sizeof(tempp));		
		ssi_close(dirsock);
		encrypt_insist_write(newsd, &tempp, sizeof(tempp));
	}

	close(newsd);

	exit(0);
}