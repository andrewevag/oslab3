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

void* check1(void* arg)
{
	
	return NULL;
}

void* check2(void* arg)
{
	char buf[] = {'Q', 'S', 0, 'O', 'K', 'O', 'N', 'O', 'M', 'A', 0, 'p', 'a', 's', 's', 'w', 'd', 0, 0, 'k', 'a', 'n', 'a', 'l', 'i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 'h', 'e', 'l', 'l', 'o', ' ', 'p', 'e', 'o', 'p', 'l', 'e', ' ', 'h', 'e', 'r', 'e', ' ', 'w', 'e', ' ', 'g', 'o'};

	packet p;
	memset(&p, 0, sizeof(p));
	p.packet_type = QUESTION;
	p.command = SEND;
	memcpy(p.arg1, "OKONOMA", sizeof("OKONOMA"));
	memcpy(p.arg2, "passwd", sizeof("passwd"));
	memcpy(p.arg3, "kanali", sizeof("kanali"));
	p.length = 23;
	memcpy(p.body, "hello people here we go", sizeof("hello people here we go"));
	char* ret = packet_format(&p);
	if(ret == NULL){
		res = 0;
		return NULL;
	}else{
		// if(memcmp(buf, ret, sizeof(buf))== 0){
		// 	res = 1;
		// 	return NULL;
		// }
		for(int i = 0; i < sizeof(buf); i++){
			unsigned char c = ret[i];
			if(ret[i] != (unsigned char)buf[i]);
				fprintf(stderr, "differennce at %d %u!=%u\n", i, ret[i], buf[i]);
		}
	}
	res = 0;
	return NULL;
}





//TESTING PARSER SUITE
void (*befores[])(void) = {NULL, NULL};
void* (*tests[])(void*) = {check1, check2};
void (*afters[])(void) = {NULL, NULL};
char* testnames[] = {"CU msg", "Variable msg send"};


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