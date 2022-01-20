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
#include "ANutils/linkedlist.h"
#include "utillib/anutil.h"
#include "utillib/Astring.h"



int main(int argc, char** argv){

	errorcheck(isNumber(argv[1]), false, "invalid socket @type : Not A number");
	printf("[child is handling]\n");
	int newsd = atoi(argv[1]);
	int n;
	char buf[4096];
	const char * socketname = argv[2];
	
	/*
	 * Handle incoming packages and send them to director;
	 */
	int dirReadbytes = 0;
	char dirResponse[MAX_MSIZE];
	list* packets;
	string* s;
	int readbytes = 0;
	int packetsread = 0;
	while(1) {
		// | msg ends at this character.
		//maximum message length 
		
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
		//split into packets and send each one to director;
		s = string_constructor(buf, readbytes);
		packets = string_splitAt(s, '|');
		forEachList(packets, p)
		{
			string* packet = getData(p);
			// string_filter(&packet, '\n');
			// packet = string_slice(packet, 0, packet->length-1);
			char* temp = string_tocharpointerNULLTERM(packet);
			printf("[packet] : %s", temp);
			free(temp);
		}
		
		memset(buf, 0, sizeof(buf));
		readbytes = 0;
		packetsread = string_countChar(s, '|');
		if(packetsread < listlength(packets))
		{
			char* temp = string_tocharpointerNULLTERM(list_getNth(packets, listlength(packets)-1));
			printf("[child] remaining packet %s\n", temp);
			memcpy(buf,temp, strlen(temp));
			printf("[child] buf now = %s", buf);
			readbytes = strlen(temp);
			free(temp);
		}else printf("full packets\n");

		/*
		* Prepare the socket for the inner connection;
		*/
		struct sockaddr_un addr;
		int directorSock;
		//send packets
		int i =0 ;

		forEachList(packets, p)
		{
			if(!(i++ < packetsread))
				break;
	
			directorSock = errorcheck(socket(AF_UNIX, SOCK_STREAM, 0), -1, "failed to create socket as a child\n");
			memset(&addr, 0, sizeof(addr));
			addr.sun_family=AF_UNIX;
			strncpy(addr.sun_path,socketname, sizeof(addr.sun_path)-1);
			printf("[child] trying to connect to %s\n", socketname);
			errorcheck(connect(directorSock, (struct sockaddr*)&addr, sizeof(addr)), -1, "[child] failed to connect to director");

			string* packet = getData(p);
			string_appendStr(packet, string_constructor("| ", sizeof("| ")));
			string_filter(&packet, '\n');
			char* temp = string_tocharpointerNULLTERM(packet);
			insist_write(directorSock, temp, strlen(temp));
			free(temp);
			printf("after free\n");
			//read response from server.
			while(1)
			{
				n = read(directorSock, dirResponse, sizeof(dirResponse));
				dirReadbytes += n;
				if(n <= 0){
					if(n < 0)
						perror("[child] Read from director failed\n");
					if(n == 0)
						printf("[child read packet from director] %s\n", dirResponse);
					break;
				}
			}
			//close it fast in case someone needs it.
			close(directorSock);
			insist_write(newsd, dirResponse, dirReadbytes);
			printf("[child] sent to client the message from server ReadBytes = %d ", dirReadbytes);
			insist_write(STDOUT_FILENO, dirResponse, dirReadbytes);
			memset(dirResponse, 0 , sizeof(dirResponse));
		}
		
		//now open connection with server at socketname and send all packets.
		//packets are correct to be sent 
		//deleteList(packets, (void (*)(void*))string_destructor);
		
		
	}

exit:
	close(newsd);

	exit(0);
}