#include <stdio.h>
#include "SSI.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "anutil.h"
#include <signal.h>
#include <string.h>

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







// TESTING CONFIGS AND PARAMETERS
void (*befores[])(void) = {ssi_clientTestBefore, NULL};
void* (*tests[])(void*) = {ssi_clientTest, ssi_serverTest};
void (*afters[])(void) = {ssi_clientTestAfter, ssi_serverTestAfter};
char* testnames[] = {"ssi_clientTest", "ssi_serverTest"};

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
		if(afters[i] != NULL){
			afters[i]();
		}
		pthread_join(pid, NULL);
		display(i, res);
	}

	return 0;
}