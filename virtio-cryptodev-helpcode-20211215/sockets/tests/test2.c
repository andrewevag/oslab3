#include <stdio.h>
#include "SSI.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "anutil.h"
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include "cryptops.h"
#include "socket-common.h"

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

static int fill_urandom_buf(unsigned char *buf, size_t cnt)
{
        int crypto_fd;
        int ret = -1;

        crypto_fd = open("/dev/urandom", O_RDONLY);
        if (crypto_fd < 0)
                return crypto_fd;

        ret = insist_read(crypto_fd, buf, cnt);
        close(crypto_fd);

        return ret;
}



void* check1(void* arg)
{
	unsigned char buffer[DATA_SIZE];
	unsigned char encrypted[DATA_SIZE];
	unsigned char decrypted[DATA_SIZE];
	errorcheck(fill_urandom_buf(buffer,DATA_SIZE),-1,"getting data from /dev/urandom\n");
	
	encryption(buffer,encrypted,DATA_SIZE);
	decryption(encrypted,decrypted,DATA_SIZE);

	/* Verify the result */
	if (memcmp(buffer, decrypted, sizeof(buffer)) != 0) {
		res = 0;
	} else
		res = 1;

	return NULL;
}

void* check2(void* arg)
{
	int fd = open("tempfile", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
	errorcheck(fd, -1, "open file test");
	packet p = packetCU("gkrinia","nusta1");
	errorcheck(encrypt_insist_write(fd,&p,sizeof(p)),-1,"encrypt insist write error");
	printf("write return\n");
	packet q;
	errorcheck(decrypt_insist_read(fd,&q,sizeof(q)),-1, "decrypt insist read error");
	if(memcmp(&p,&q,sizeof(q)) != 0){
		res = 0;
	}else{
		res = 1;
	}
	return NULL;
}





//TESTING PARSER SUITE
void (*befores[])(void) = {NULL, NULL};
void* (*tests[])(void*) = {check1,check2};
void (*afters[])(void) = {NULL, NULL};
char* testnames[] = {"encrypt/decrypt", "insist encrypt/decrypt"};


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