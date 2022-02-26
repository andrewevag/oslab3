#include "anutil.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <ctype.h>
int errorcheck2(int val, int targetval, char* msg)
{
	if(val == targetval)
	{
		perror(msg);
		exit(EXIT_FAILURE);
	}
	return val;
}
//creates a child process and executes program at path with argument args
//Returns value pid of child.
int subprocesscall(char* path, char* args[]){
    int forked_val = fork();
    errorcheck2(forked_val,-1,"fork failed at number 1");
    if(forked_val == 0){
        //child code
        //Just run riddle.
        execv(path, args);
        printf("Return from execv something went wrong\n");
        exit(EXIT_FAILURE);
    }
    else{
        return forked_val;
    }
}

int subprocesscallfn(void* (*fun)(void*), void* args)
{
	int forked_val = fork();
	errorcheck2(forked_val, -1, "forked failed at subprocesscallfn\n");
	if(forked_val == 0)
	{
		//child code;
		fun(args);
		exit(0);
	}else{
		return forked_val;
	}
}




bool isNumber(char* s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (!isdigit(s[i]))
              return false;
    }
    return true;
}


void waitsubprocessexit(int cid, int* status){
    errorcheck2(waitpid(cid, status, 0),-1,"wait failed");
    if (WIFEXITED(*status)){} //do nothing for now.
    else if(!WIFEXITED(*status)){ //translates it to if not exited normally (calling exit(0) etc.)
        if((WIFSIGNALED(*status)) && (WTERMSIG(*status) != SIGTERM) && (WTERMSIG(*status) != SIGPIPE)){
            printf("Terminated by the following signal %d\n", WTERMSIG(*status));
            printf("exected to exit and it didn't");
            exit(EXIT_FAILURE);
        }
        else{} //do nothing for now
    }
}


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
	errorcheck2(insist_write(fd, buf, cnt) <0, 1, "failed to send @safe write");
}

int splitToWords(char* str, int length,char** words, int maxwords){
	// "delimiters are \n and space"
	int numofwords = 0;
	words[numofwords++] = str; 
	for(int i = 0; i < length; i++)
	{	
		if(str[i] == ' ' || str[i] == '\n' )
		{
			str[i++] = 0;
			while(str[i] == ' ' || str[i] == '\n'){
				i++;
			}
			words[numofwords++] = &(str[i]);
			if(numofwords >= maxwords){
				return numofwords-1;
			}
		}
	}
	return numofwords-1;
}


ssize_t insist_read(int fd, void* buf, size_t nbyte)
{
	ssize_t nread = 0, n;

	do{
		if( (n = read(fd, &((char*) buf)[nread], nbyte-nread) == -1) ){
			if(errno)
				continue;
			else return -1;
		}
		if (n==0)
			return nread;
		nread += n;
			
	} while(nread < nbyte);

	return nread;
}

/* Insist until all of the data has been read */
// ssize_t insist_read(int fd, void *buf, size_t cnt)
// {
//         ssize_t ret;
//         size_t orig_cnt = cnt;

//         while (cnt > 0) {
//                 ret = read(fd, buf, cnt);
//                 if (ret < 0)
//                         return ret;
//                 buf += ret;
//                 cnt -= ret;
//         }

//         return orig_cnt;
// }




void interchangefds(int* fd1,int* fd2){
    int first = *fd1, second = *fd2;

    //they way i do it is dup one of them and close the origin
    int temp = dup(first);
    errorcheck2(temp, -1, "failed to dup at interchangefds");
    errorcheck2(close(first), -1, "failed to close 1 interchangefds");

    //dup2 the other in the first place
    errorcheck2(dup2(second, first), -1, "dup2 1 failed");
    errorcheck2(close(second), -1, "failed to close 2 interchangefds");
    //then dup2 the newone in the

    errorcheck2(dup2(temp, second), -1, "dup2 1 failed");
    errorcheck2(close(temp), -1, "failed to close temp");

    temp = *fd1;
    *fd1 = *fd2;
    *fd2 = temp;
}
