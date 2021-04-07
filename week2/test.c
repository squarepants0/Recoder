#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

extern void *mm_malloc(size_t size);
extern void mm_free(void *bp);


int main(){
	void *ptrlist[0x20];
	void *smallist[0x20];
	void *tinylist[0x20];
	for(int i=0;i<=20;i++){
		ptrlist[i] = mm_malloc(0x100);
		strcpy(ptrlist[i],"AAAA");
		smallist[i] = mm_malloc(0x20);
		strcpy(smallist[i],"AAAA");
		tinylist[i] = mm_malloc(0x2);
		strcpy(tinylist[i],"AA");
		printf("%d\n", i);
	}

	ptrlist[12] = mm_malloc(0x100);
	strcpy(ptrlist[12],"AAAA");
	smallist[12] = mm_malloc(0x20);
	strcpy(smallist[12],"AAAA");
	tinylist[12] = mm_malloc(0x2);
	strcpy(tinylist[12],"AA");
	printf("%d\n", 12);
	
	for(int i=15;i<=20;i++){
		printf("ptrlist[%d]0x%p:deleted!!\n", i, ptrlist[i]);
		mm_free(ptrlist[i]);
		mm_free(smallist[i]);
	}

	for(int i=0;i<=5;i++){
		ptrlist[i] = mm_malloc(0x100);
		printf("ptrlist[%d] get an address:0x%p\n", i, ptrlist[i]);
	}
	//ptrlist[1] = mm_malloc(0x100)

	return 0;
	
}

