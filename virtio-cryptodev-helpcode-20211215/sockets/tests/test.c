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
	char buf6[] = {'Q', 'S', 0, 'n', 'i', 'k', 0, 0, 0, 0, 0, 'k', 'w', 'd', 'i', 'k', 'o', 's', 0, 'c', 'h', 'a', 'n', 'n', 'e', 'l', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'h', 'e', 'l', 'l', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '!'};
	//correct
	char buf7[] = {'A', 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'n', 'e', 'w', 'm', 's', 'g'};
	//correct
	char buf8[] = {'A', 'S', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'n', 'e', 'w', 'm', 's', 'g'};
	//wrong
	char buf9[] = {'A', 'W', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'n', 'e', 'w', 'm', 's', 'g'};
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
	if(lseek(fd, 10*37, SEEK_SET) < 0)
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

void packet_parse1_after(void)
{
	
}

// TESTING CONFIGS AND PARAMETERS
// void (*befores[])(void) = {ssi_clientTestBefore, NULL, NULL};
// void* (*tests[])(void*) = {ssi_clientTest, ssi_serverTest, insist_read_test};
// void (*afters[])(void) = {ssi_clientTestAfter, ssi_serverTestAfter, NULL};
// char* testnames[] = {"ssi_clientTest", "ssi_serverTest", "insist read test"};



//TESTING PARSER SUITE
void (*befores[])(void) = {packet_parse1_before, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
void* (*tests[])(void*) = {packet_parse0,packet_parse1, packet_parse2, packet_parse3,packet_parse4, packet_parse5,
packet_parse6,packet_parse7, packet_parse8, packet_parse9};
void (*afters[])(void) = {NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
char* testnames[] = {"Q test", "A test", "Should fail test", "Create Channel Test", "Create User Test", 
"Read Test", "Send test", "Server failure test", "Server Success Test", "Uknown command test"};


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