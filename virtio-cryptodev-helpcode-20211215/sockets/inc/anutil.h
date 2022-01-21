#include <stdbool.h>
#include <stdlib.h>

int subprocesscall(char* path, char* args[]);

int subprocesscallfn(void* (*fun)(void*), void* args);


bool isNumber(char* s);

void waitsubprocessexit(int cid, int* status);



/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt);

void safe_write(int fd, const void *buf, size_t cnt);
#define const_safe_write(fd, cswbuf) safe_write(fd, cswbuf, sizeof(cswbuf))

ssize_t insist_read(int fd, void* buf, size_t nbyte);


int splitToWords(char* str, int length,char** words, int maxwords);

//interchanges the file descriptors in numbers and returns the different values so they can be accessed without
//the need for new variables
void interchangefds(int* fd1,int* fd2);