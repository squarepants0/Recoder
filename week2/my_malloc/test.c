/*************************************************************************
    > File Name       : test.c
    > Author          : Square_R
    > Created Time    : Fri 09 Apr 2021 10:17:21 PM CST
 ************************************************************************/


#include<stdio.h>
#include<linux/types.h>

extern void * mm_malloc(size_t rsize);

int main(int argc,char *argv[])
{
	void *ptrlist[0x200];
	setvbuf(stdin, 0, 2, 0);
	setvbuf(stdout, 0, 2, 0);
	for(int i=0; i<0x200 ;i++){
		ptrlist[i] = mm_malloc(0x28);
		printf("ptrlist[%d] got addresss: %p\n", i, ptrlist[i]);
	}
	ptrlist[0] = mm_malloc(0xf000);
	ptrlist[1] = mm_malloc(0x200);
		

     return 0;
}
