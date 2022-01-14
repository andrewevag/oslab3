/*
 * socket-client.c
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
#include <sys/select.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket-common.h"


#include "ANutils/linkedlist.h"
#include "ANutils/channel.h"
#include "ANutils/user.h"
#include "ANutils/message.h"
#define MAX_MSIZE 4096


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

void safe_write(int fd, const void *buf, size_t cnt){
	errorcheck(insist_write(fd, buf, cnt) <0, 1, "failed to send @safe write");
}
#define const_safe_write(fd, cswbuf) safe_write(fd, cswbuf, sizeof(cswbuf))


/*
 * Arguments are commands
 * n -> name assigns name to that user
 * c -> channel create channel parameter channel name
 * a -> add user to channel -channel -name
 * s -> send message to channel \n -> msg
 * r -> read channel request read of all the channel.
 */
int main(int argc, char *argv[])
{
	int sd, port;
	ssize_t n;
	char buf[100];
	char *hostname;
	struct hostent *hp;
	struct sockaddr_in sa;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
		exit(1);
	}
	hostname = argv[1];
	port = atoi(argv[2]); /* Needs better error checking */

	/* Create TCP/IP socket, used as main chat channel */
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

	/* Be careful with buffer overruns, ensure NUL-termination */
	strncpy(buf, "andreas CU mouni | ", sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO,&set);
    FD_SET(sd,&set);
    int maxfd = ((sd > STDIN_FILENO)?sd:STDIN_FILENO) + 1;

    char stin_buf[2*MAX_MSIZE];
    char sock_buf[2*MAX_MSIZE];
    int readstin = 0;
    int readsock = 0;
    int ni,ns;

    list* msglist = emptyList;


    memset(stin_buf,0,sizeof(stin_buf));
    memset(sock_buf,0,sizeof(sock_buf));
    while(1){
        
        errorcheck(select(maxfd,&set,NULL,NULL,NULL),-1,"error on select\n");
        if(FD_ISSET(STDIN_FILENO,&set)){
            ni = read(STDIN_FILENO, stin_buf+readstin, sizeof(stin_buf));
            readstin +=ni;
            errorcheck(ni,-1,"read from STDIN failed\n");
            char* cp = strchr(stin_buf,'|');
            if(cp != NULL){
                *(cp+1) = '\n';
                safe_write(sd,stin_buf,strlen(stin_buf)+1);
                printf("%s\n",stin_buf);
                memset(stin_buf,0,sizeof(stin_buf));
                readstin = 0;
            }

        }

        if(FD_ISSET(sd,&set)){
            ns = read(sd, sock_buf+readsock, sizeof(sock_buf));
            readsock +=ns;
            errorcheck(ns,-1,"read from socket failed\n");
            char* cp = strchr(sock_buf,'|');
            if(cp != NULL){
                int i,j;
                for(i = 0, j = 0; j < readsock; j++){
                    if(sock_buf[j] == '|'){
                        char* start = &(sock_buf[i]);
                        int size = j-i+1;
                        msglist = cons(message_constructor_size(start,size),msglist);
                        j += 2;
                        i = j;
                    }
                }
                j = 0;
                while(i < readsock){
                    sock_buf[j++] = sock_buf[i];
                    i++;
                }
                readsock = j;

                msglist = reverse(msglist);
                forEachList(msglist,i){
                    message* msg = getData(i);
                    safe_write(STDOUT_FILENO,msg->text,strlen(msg->text));
                }
				deleteList(tail(msglist),(void(*)(void *)) message_destructor_size);
				message_destructor_size((message *)(getData(msglist)));
				msglist = emptyList;

            }

        }



        FD_ZERO(&set);
        FD_SET(STDIN_FILENO,&set);
        FD_SET(sd,&set);

    }



	/* Say something... */
	if (insist_write(sd, buf, strlen(buf)) != strlen(buf)) {
		perror("write");
		exit(1);
	}
	fprintf(stdout, "I said:\n%s\nRemote says:\n", buf);
	fflush(stdout);

	/*
	 * Let the remote know we're not going to write anything else.
	 * Try removing the shutdown() call and see what happens.
	 */
	if (shutdown(sd, SHUT_WR) < 0) {
		perror("shutdown");
		exit(1);
	}

	/* Read answer and write it to standard output */
	for (;;) {
		n = read(sd, buf, sizeof(buf));

		if (n < 0) {
			perror("read");
			exit(1);
		}

		if (n <= 0)
			break;

		if (insist_write(0, buf, n) != n) {
			perror("write");
			exit(1);
		}
	}

	fprintf(stderr, "\nDone.\n");
	return 0;
	while(1);
}
