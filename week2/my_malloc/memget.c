#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAX_HEAP 0x100000

extern int errno;

/*recode heap area*/
static char *Heap_foot;
static char *Heap_roof;
static size_t Used_size;

/*initialize the heap with 0x1000 size*/
char *mem_init(void){
	Heap_foot = (char *)sbrk(0); 		/*get current brk*/
	sbrk(0x10000); 	
	Heap_roof = (char *)sbrk(0);
	Used_size = 0x10000;				/*this cannt over MAX_HEAP*/

	return Heap_foot;
}

char *mem_sbrk(int incr){
	if(Used_size > MAX_HEAP){
		errno = ENOMEM;
		perror("***mem_sbrk wrong***");
		return (char *)-1;
	}

	Heap_foot = Heap_roof;
	sbrk(0x10000);
	Heap_roof = (char *)sbrk(0);
	Used_size += 0x10000;

	return Heap_foot;
}
