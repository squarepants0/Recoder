#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>

extern void *mm_malloc(size_t size);
extern void mm_free(void *bp);


int main(){
	void *ptrlist[0x20];
	void *smallist[0x20];
	void *tinylist[0x20];
	void *test = malloc(0x20);
	// printf("%p\n", brk(0));
	// read(0,test, 0x10);
	for(int i=0;i<=20;i++){
		ptrlist[i] = mm_malloc(0x100);
		strcpy(ptrlist[i],"AAAA");
		smallist[i] = mm_malloc(0x20);
		strcpy(smallist[i],"AAAA");
		tinylist[i] = mm_malloc(0x2);
		strcpy(tinylist[i],"AA");
		printf("%d\n", i);
	}
	read(0,tinylist[2],0x20);
	ptrlist[12] = mm_malloc(0x100);
	strcpy(ptrlist[12],"AAAA");
	smallist[12] = mm_malloc(0x20);
	strcpy(smallist[12],"AAAA");
	tinylist[12] = mm_malloc(0x2);
	strcpy(tinylist[12],"AA");
	printf("%d\n", 12);
	
	for(int i=15;i<=20;i++){
		printf("ptrlist[%d] %p:deleted!!\n", i, ptrlist[i]);
		mm_free(ptrlist[i]);
		mm_free(smallist[i]);
	}

	for(int i=0;i<=5;i++){
		ptrlist[i] = mm_malloc(0x100);
		printf("ptrlist[%d] get an address:%p\n", i, ptrlist[i]);
	}
	//ptrlist[1] = mm_malloc(0x100)

	return 0;
	
}

