#include "SafeCalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
void *sfmalloc(ssize_t size)
{
	void* mem = malloc(size);
	if(mem == NULL){
		perror("malloc failed safe malloc exiting\n");
		exit(1);
	}
	return mem;
}