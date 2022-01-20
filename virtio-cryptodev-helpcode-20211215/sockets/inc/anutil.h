#include <stdbool.h>
#include <stdlib.h>

int subprocesscall(char* path, char* args[]);



bool isNumber(char* s);

void waitsubprocessexit(int cid, int* status);



/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt);

void safe_write(int fd, const void *buf, size_t cnt);
#define const_safe_write(fd, cswbuf) safe_write(fd, cswbuf, sizeof(cswbuf))

int splitToWords(char* str, int length,char** words, int maxwords);