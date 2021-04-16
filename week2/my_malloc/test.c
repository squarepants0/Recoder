/*************************************************************************
    > File Name       : test.c
    > Author          : Square_R
    > Created Time    : Fri 09 Apr 2021 10:17:21 PM CST
 ************************************************************************/


#include<stdio.h>
#include<linux/types.h>
#include <string.h>

extern void *mm_malloc(size_t rsize);
extern int mm_free(void *p);

int main(int argc,char *argv[])
{
	void *ptrlist[0x200];
	void *ptrlist2[0x200];
	setvbuf(stdin, 0, 2, 0);
	setvbuf(stdout, 0, 2, 0);

	// void *try = malloc(0x20);
	// void *try2 = malloc(0x20);
	// free(try);

	for(int i=0; i<7 ;i++){
		ptrlist[i] = mm_malloc(0x50); 	//fastbin chunks
	}

	for(int i=7; i<7*2 ;i++){
		ptrlist[i] = mm_malloc(0x100);	//smallbin chunks
	}

	for (int i = 7*2; i < 7*3; ++i)
	{
		ptrlist[i] = mm_malloc(0x500);	//into UB
	}
	
	for(int i=0; i<3 ;i++)
		mm_free(ptrlist[i]);

	for(int i=7; i<7+3 ;i++)
		mm_free(ptrlist[i]);

	for(int i=7*2; i<7*2+3 ;i++)
		mm_free(ptrlist[i]);

	for(int i=0; i<7 ;i++){
		ptrlist2[i] = mm_malloc(0x1000);
	}

     return 0;
}
