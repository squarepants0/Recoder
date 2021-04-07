#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAX_HEAP 0X10000

/*private global variables*/
static char *mem_heap; 		/*堆顶*/
static char *mem_brk;		/*堆底*/
static char *mem_max_addr;	/*最大可分配堆地址*/

extern int errno;

/*initialize the heap area*/
void mem_init(void){
	mem_heap = (char *)mmap(NULL, MAX_HEAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);  /*这里我想用mmap系统调用可能好一点*/
	mem_brk = mem_heap;
	mem_max_addr = (char *)(mem_heap+MAX_HEAP);
}

/*expand current heap foot*/
char *mem_sbrk(int incr){
	char *old_brk = mem_brk;

	if((incr<0) || ((mem_brk+incr)>mem_max_addr)){
		errno = ENOMEM;
		perror("***mem_sbrk wrong***");
		return (void *)-1;
	}
	mem_brk += incr;
	return old_brk;
}