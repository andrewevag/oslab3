#include <stdio.h>
#include "SSI.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "anutil.h"
#include <signal.h>
#include <string.h>
#include <fcntl.h>
// #include "packet.h"
#include "packet_parser.h"
#include "linkedlist.h"
#define Black "\033[0;30m"
#define Red "\033[0;31m"
#define Green "\033[0;32m"
#define Yellow "\033[0;33m"
#define Blue "\033[0;34m"
#define Purple "\033[0;35m"
#define Cyan "\033[0;36m"
#define White "\033[0;37m"
#define RESET_COLOR "\e[m"

int res;
void testsetup()
{
}
//SERVER TESTS
void* ssi_serverTest(void* arg)
{
	SSI* server = ssi_open(NULL, 35001, true, 10);
	if(ssi_server_accept(server) != -1)
	{
		ssi_close(server);
		res = 1;
		return NULL;
	}
	ssi_close(server);
	res = 0;
	return NULL;
}
void ssi_serverTestAfter(void)
{
	SSI* client = ssi_open("localhost", 35001, false, 0);
	ssi_close(client);
}

//CIENT TEST.
void* ssi_clientTest(void * arg)
{
	SSI* client = ssi_open("localhost", 50001, false, 0);
	write(client->ssi_fd, "hello", sizeof("hello"));
	ssi_close(client);
	return NULL;
}
SSI* global_server;
void ssi_clientTestBefore(void)
{
	global_server = ssi_open(NULL, 50001, true, 10);
	
}
void ssi_clientTestAfter(void)
{
	int clientid = ssi_server_accept(global_server);
	char buf[sizeof("hello")];
	memset(buf, 0, sizeof(buf));
	read(clientid, buf, sizeof(buf));
	if(strcmp(buf, "hello") == 0){
		res = 1;
	}else res = 0;
	ssi_close(global_server);
}

//INSIST READ TEST.
void* insist_read_test(void * arg)
{	
	char msg[10];
	insist_read(STDIN_FILENO, msg, 10);

	if(strcmp(msg,"helloooooo") == 0)
	{
		res = 1;
		return NULL;
	}
	else res = 0;
	return NULL;
}

int fd;
void packet_parse1_before(void)
{
	fd = open("tempfile", O_CREAT | O_TRUNC| O_RDWR, S_IRWXU);
	if(fd < 0){
		perror("open failed\n");
		exit(EXIT_FAILURE);
	}
	//correct
	char buf[] = {'Q', 'C', 'U', 'a', 'n', 'd', 'r', 'e', 'a', 's', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//correct
	char buf1[] = {'A', 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//wrong
	char buf2[] = {'F', 'C', 'U', 'a', 'n', 'd', 'r', 'e', 'a', 's', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//correct
	char buf3[] = {'Q', 'C', 0, 'n', 'i', 'k', 'o', 'l', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'c', 'h', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//correct
	char buf4[] = {'Q', 'C', 'U', 'a', 'n', 'd', 'r', 'e', 'a', 's', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//correct
	char buf5[] = {'Q', 'R', 0, 'a', 'n', 'd', 'r', 'e', 'a', 's', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 'c', 'h', 'a', 'n', 'n', 'e', 'l', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//correct
	char buf6[] = {'Q', 'S', 0, 'n', 'i', 'k', 0, 0, 0, 0, 0, 'k', 'w', 'd', 'i', 'k', 'o', 's', 0, 'c', 'h', 'a', 'n', 'n', 'e', 'l', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 'h', 'e', 'l', 'l', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '!'};
	//correct
	char buf7[] = {'A', 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 'n', 'e', 'w', 'm', 's', 'g'};
	//correct
	char buf8[] = {'A', 'S', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 'n', 'e', 'w', 'm', 's', 'g'};
	//wrong
	char buf9[] = {'A', 'W', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 'n', 'e', 'w', 'm', 's', 'g'};
	insist_write(fd, buf, sizeof(buf));
	insist_write(fd, buf1, sizeof(buf1));
	insist_write(fd, buf2, sizeof(buf2));
	insist_write(fd, buf3, sizeof(buf3));
	insist_write(fd, buf4, sizeof(buf4));
	insist_write(fd, buf5, sizeof(buf5));
	insist_write(fd, buf6, sizeof(buf6));
	insist_write(fd, buf7, sizeof(buf7));
	insist_write(fd, buf8, sizeof(buf8));
	insist_write(fd, buf9, sizeof(buf9));
	close(fd);
	//try packets;
	fd =  open("tempfile", O_RDONLY);
	if(fd < 0){
		perror("open failed");
		return NULL;
	}
}

//packet parser tests.
void* packet_parse0(void* arg)
{
	
	packet *p = packet_parse(fd);
	if(p->packet_type == QUESTION)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse1(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->packet_type == ANSWER)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse2(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 3*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p==NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse3(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == CREATE_CHANNEL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse4(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == CREATE_USER)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse5(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == READ)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse6(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == SEND)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse7(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == SERVER_FAILURE)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse8(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->command == SERVER_SUCCESS)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}


void* packet_parse9(void* arg)
{	
	packet *p = packet_parse(fd);
	// if(lseek(fd, 10*37, SEEK_SET) < 0)
	// {
	// 	perror("lseek failed");
	// 	exit(1);
	// }
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void packet_parse1_after(void)
{
	
}

void packet_parse10_before(void)
{
	close(fd);
	fd = open("tempfile", O_CREAT | O_TRUNC| O_RDWR, S_IRWXU);
	if(fd < 0){
		perror("open failed\n");
		exit(EXIT_FAILURE);
	}
	char buf10[] = {'Q', 'C', 'U', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf11[] = {'Q', 'C', 'U', 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'O', 'K', 'P', 'A', 'S', 'S', 'W', 'D', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf12[] = {'Q', 'C', 'U', 0, 'K', 'O', 'N', 'O', 'M', 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//pass fail
	char buf13[] = {'Q', 'C', 'U', 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//pass correct
	char buf14[] = {'Q', 'C', 'U', 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'O', 'K', 'P', 'A', 'S', 'S', 'W', 'D', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf15[] = {'Q', 'C', 'U', 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 0, 0, 0, 'w', 'r', 'o', 'n', 'g', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	char buf16[] = {'Q', 'C', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'n', 'o', 't', 'w', 'r', 'o', 'n', 'g', 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf17[] = {'Q', 'C', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf18[] = {'Q', 'C', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'n', 'o', 't', 'w', 'r', 'o', 'n', 'g', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf19[] = {'Q', 'C', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	

	insist_write(fd, buf10, sizeof(buf10));
	insist_write(fd, buf11, sizeof(buf11));
	insist_write(fd, buf12, sizeof(buf12));
	insist_write(fd, buf13, sizeof(buf13));
	insist_write(fd, buf14, sizeof(buf14));
	insist_write(fd, buf15, sizeof(buf15));
	insist_write(fd, buf16, sizeof(buf16));
	insist_write(fd, buf17, sizeof(buf17));
	insist_write(fd, buf18, sizeof(buf18));
	insist_write(fd, buf19, sizeof(buf19));
	
	fd =  open("tempfile", O_RDONLY);
	if(fd < 0){
		perror("open failed");
		return NULL;
	}
}

void* packet_parse10(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 1*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}

}

void* packet_parse11(void* arg)
{	
	packet *p = packet_parse(fd);
	if(strcmp(p->arg1, "OKONOMA") == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse12(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 3*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse13(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 4*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse14(void* arg)
{	
	packet *p = packet_parse(fd);
	if(strcmp(p->arg2, "OKPASSWD") == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse15(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 6*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse16(void* arg)
{	
	packet *p = packet_parse(fd);
	if(strcmp(p->arg3, "kanali") == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}
void* packet_parse17(void* arg)
{	
	packet *p = packet_parse(fd);
	if(strcmp(p->arg3, "kanali") == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse18(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 19*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

void* packet_parse19(void* arg)
{	
	packet *p = packet_parse(fd);
	if(lseek(fd, 20*37, SEEK_SET) < 0)
	{
		perror("lseek failed");
		exit(1);
	}
	if(p == NULL)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}


void packet_parse20_before(void* arg)
{
	close(fd);
	fd = open("tempfile", O_CREAT | O_TRUNC| O_RDWR, S_IRWXU);
	if(fd < 0){
		perror("open failed\n");
		exit(EXIT_FAILURE);
	}
	char buf20[] = {'Q', 'R', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'n', 'o', 't', 'w', 'r', 'o', 'n', 'g', 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, '3', '2', 0, 0, 0, 0, 0, 0, 0, 0};
	char buf21[] = {'Q', 'R', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char buf22[] = {'Q', 'R', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'n', 'o', 't', 'w', 'r', 'o', 'n', 'g', 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, '3', '2', 0, 0, 0};
	char buf23[] = {'Q', 'R', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, '1', '2', '3', 0, 0, 0, 0, 0, 0, 0};

	insist_write(fd, buf20, sizeof(buf20));
	insist_write(fd, buf21, sizeof(buf21));
	insist_write(fd, buf22, sizeof(buf22));
	insist_write(fd, buf23, sizeof(buf23));
	fd =  open("tempfile", O_RDONLY);
	if(fd < 0){
		perror("open failed");
		return NULL;
	}
}


void* packet_parse20(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->id == 32)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}
void* packet_parse21(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->id == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}
void* packet_parse22(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->id == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}
void* packet_parse23(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->id == 123)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}



void packet_parse24_before(void* arg)
{
	close(fd);
	fd = open("tempfile", O_CREAT | O_TRUNC| O_RDWR, S_IRWXU);
	if(fd < 0){
		perror("open failed\n");
		exit(EXIT_FAILURE);
	}
	char buf24[] = {'Q', 'S', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 'h', 'e', 'l', 'l', 'o', ' ', 'p', 'e', 'o', 'p', 'l', 'e', ' ', 'h', 'e', 'r', 'e', ' ', 'w', 'e', ' ', 'g', 'o'};
	insist_write(fd, buf24, sizeof(buf24));
	

	fd =  open("tempfile", O_RDONLY);
	if(fd < 0){
		perror("open failed");
		return NULL;
	}
}


void* packet_parse24(void* arg)
{	
	packet *p = packet_parse(fd);
	if(p->length == 23 && strcmp(p->body, "hello people here we go") == 0)
	{
		res = 1;
		return NULL;
	}
	else{
		res = 0;
		return NULL;
	}
}

// TESTING CONFIGS AND PARAMETERS
// void (*befores[])(void) = {ssi_clientTestBefore, NULL, NULL};
// void* (*tests[])(void*) = {ssi_clientTest, ssi_serverTest, insist_read_test};
// void (*afters[])(void) = {ssi_clientTestAfter, ssi_serverTestAfter, NULL};
// char* testnames[] = {"ssi_clientTest", "ssi_serverTest", "insist read test"};



//TESTING PARSER SUITE
void (*befores[])(void) = {packet_parse1_before, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, packet_parse10_before, NULL, NULL, NULL, NULL, NULL
, NULL, NULL, NULL, NULL, packet_parse20_before, NULL, NULL, NULL, packet_parse24_before};
void* (*tests[])(void*) = {packet_parse0,packet_parse1, packet_parse2, packet_parse3,packet_parse4, packet_parse5,
packet_parse6,packet_parse7, packet_parse8, packet_parse9, packet_parse10, packet_parse11, packet_parse12, packet_parse13, packet_parse14, packet_parse15,
packet_parse16, packet_parse17, packet_parse18, packet_parse19,
packet_parse20, packet_parse21, packet_parse22, packet_parse23, packet_parse24};
void (*afters[])(void) = {NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
char* testnames[] = {"Q test", "A test", "Should fail test", "Create Channel Test", "Create User Test", 
"Read Test", "Send test", "Server failure test", "Server Success Test", "Uknown command test", 
"Wrong username (all 0)", "Correct username", "Wrong username (starting with 0)",
"Wrong password (all 0)", "Correct password", "Wrong password (starting with 0)", 
"Channel w/ passwd should be correct??", "Channel w/ passwd should be correct",
"Channel all 0", "Channel (starting with 0)",
"Id 32", "Id all 0", "Id (starting 0)", "Id 123", 
"MSG hello"};


int successes  = 0;
void display(int testnum, int result)
{
	if(result == 0)
	{
		printf("["Red"---"RESET_COLOR"]"" Test %d : "Blue"%s"RESET_COLOR" failed\n", testnum, testnames[testnum]);
	}
	else printf("["Green"✓✓✓"RESET_COLOR"]"" Test %d : "Blue"%s"RESET_COLOR" succeded\n", testnum, testnames[testnum]);
}

int main(){
	testsetup();
	pthread_t pid;
	for(int i = 0; i < sizeof(tests) / sizeof(int (*)(void)); i++)
	{
		//it should run on another thread. so you can parallelize it's condition
		if(befores[i] != NULL){
			befores[i]();
		}
		pthread_create(&pid, NULL, tests[i], NULL);
		// int cid = subprocesscallfn(tests[i], NULL);
		if(afters[i] != NULL){
			afters[i]();
		}
		// int status;
		// waitsubprocessexit(cid, &status);
		pthread_join(pid, NULL);
		display(i, res);
	}

	return 0;
}