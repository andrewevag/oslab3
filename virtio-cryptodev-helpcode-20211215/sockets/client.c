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

#define Black "\033[0;30m"
#define Red "\033[0;31m"
#define Green "\033[0;32m"
#define Yellow "\033[0;33m"
#define Blue "\033[0;34m"
#define Purple "\033[0;35m"
#define Cyan "\033[0;36m"
#define White "\033[0;37m"
#define RESET_COLOR "\e[m"

char input[BUFSIZ];
char username[BUFSIZ];
char password[BUFSIZ];
int port;
char servername[256];
SSI* s;
//commands will be 
void handle_create();
void handle_login();
int read_response();
void handle_help();
void handle_send();
void handle_follow();
void handle_add();

ssize_t encrypt_insist_write_wrapper(const void *buf, size_t cnt)
{
	s = ssi_open(servername, port, false, 0);
	int fd = s->ssi_fd;
	return encrypt_insist_write(fd, buf, cnt);
}

int main(int argc, char** argv)
{
	
	if(argc != 3){
		printf("Usage ./client <address> <port>\n");
		exit(1);
	}
	port = atoi(argv[2]);
	memset(servername, 0, sizeof(servername));
	memcpy(servername, argv[1], strlen(argv[1]));

	memset(input, 0, sizeof(input));
	handle_login();
	while(1)
	{
		printf("> ");
		scanf("%s", input);
		if(!strcmp(input, "exit")){
			goto exit;
		}
		else if(!strcmp(input, "create")){
			handle_create();
		}
		else if(!strcmp(input, "send")){
			handle_send();
		}
		else if(!strcmp(input, "follow")){
			handle_follow();
		}
		else if(!strcmp(input, "help")){
			handle_help();
		}
		else if(!strcmp(input, "add")){
			handle_add();
		}
		else{
			printf(Red"-"RESET_COLOR" Uknown command (try help)\n");
		}
		memset(input, 0, sizeof(input));

	}

exit:
	ssi_close(s);
	return 0;
}


int read_response()
{
	packet p, q;
	insist_read(s->ssi_fd, &q, sizeof(packet));
	decryption(&q, &p, sizeof(packet));
	ssi_close(s);
	//here we need to decrypt it.
	if(p.command == SERVER_SUCCESS){
		printf(Green"- "RESET_COLOR"%s\n", p.body);
		return p.id;
	}
	else if(p.command == SERVER_FAILURE){
		printf(Red"- "RESET_COLOR"%s\n", p.body);
	}
	return 0;
}

void handle_login()
{
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	printf("Enter a username [only 8 chars will be accepted] : ");
	scanf("%s", username);
	printf("Enter a password : [only 8 chars will be accepted] : ");
	scanf("%s", password);
	packet p = packetCU(username, password);
	int read = encrypt_insist_write_wrapper(&p, sizeof(p));
	read_response();
}

void handle_create()
{
	memset(input, 0, sizeof(input));
	printf("channel name to be created : ");
	scanf("%s", input);
	packet p = packetC(username, input);
	int read = encrypt_insist_write_wrapper(&p, sizeof(p));
	read_response();

}

void handle_help(){
	printf("Available Commands Are :\n"Blue"create\t"RESET_COLOR"to create newchannel\n");
	printf(Blue"follow\t"RESET_COLOR"read contents of a channel\n");
	printf(Blue"send\t"RESET_COLOR"send msg to a channel\n");
	printf(Blue"add\t"RESET_COLOR"add user to a channel\n");
	printf(Blue"exit\t"RESET_COLOR"terminate connection\n");
	
}

void handle_send()
{
	memset(input, 0, sizeof(input));
	char channelname[BUFSIZ];
	printf("channel : ");
	scanf("%s", channelname);
	printf("Enter message >");
	scanf("\n");
	memset(input, 0, sizeof(input));
	int nread = 0, n;
	fgets(input, PACKET_MAX_BODY_LENGTH, stdin);
	char* cp = strchr(input, '\n');
	*cp = 0;
	packet p;
	p = packetS(username, password, channelname, input);
	int read = encrypt_insist_write_wrapper(&p, sizeof(p));
	read_response();
	memset(input, 0, sizeof(input));
}


void handle_follow()
{
	memset(input, 0, sizeof(input));
	char channelname[BUFSIZ];
	printf("channel : ");
	scanf("%s", channelname);
	//now get all msges from that channel.
	int id = 0;
	packet p = packetR(username, password, channelname, id);
	int maxid = id+1;

	while(id <= maxid)
	{
		int read = encrypt_insist_write_wrapper(&p, sizeof(p));
		maxid = read_response();
		p.id++;id++;
	}

}


void handle_add()
{
	memset(input, 0, sizeof(input));
	char channelname[BUFSIZ];
	printf("channel : ");
	scanf("%s", channelname);
	char extrauser[BUFSIZ];
	memset(extrauser, 0, sizeof(extrauser));
	printf("user : ");
	scanf("%s", extrauser);

	packet p = packetA(username, password, channelname, extrauser);
	int read = encrypt_insist_write_wrapper(&p, sizeof(p));
	read_response();

}

