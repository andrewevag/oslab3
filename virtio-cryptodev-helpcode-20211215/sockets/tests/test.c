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

	char buf[] = {'Q', 'C', 'U', 'a', 'n', 'd', 'r', 'e', 'a', 's', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	insist_write(fd, buf, sizeof(buf));
	close(fd);
	
}

//packet parser tests.
void* packet_parse1(void* arg)
{
	//try packets;
	fd =  open("tempfile", O_RDONLY);
	if(fd < 0){
		perror("open failed");
		return NULL;
	}
	packet *p = packet_parse(fd);
	if(p->packet_type == QUESTION &&
	   p->command == CREATE_USER &&
	   strcmp(p->arg1, "andreas") == 0 &&
	   strcmp(p->arg2, "passwd") == 0 &&
	   strlen(p->arg3) == 0 &&
	   strlen(p->arg4) == 0 &&
	   p->length == 0
	)
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
void (*befores[])(void) = {packet_parse1_before};
void* (*tests[])(void*) = {packet_parse1};
void (*afters[])(void) = {packet_parse1_after};
char* testnames[] = {"CU test"};


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