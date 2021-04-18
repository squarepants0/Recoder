/*************************************************************************
    > File Name       : recursive.c
    > Author          : Square_R
    > Created Time    : Sat 17 Apr 2021 10:25:59 AM CST
 ************************************************************************/


#include<stdio.h>
#include<stdlib.h>

typedef struct ELE{
	long val;
	struct ELE *next;
} *list_ptr;

long rsum_list(list_ptr ls);

int main(int argc,char *argv[])
{	
	list_ptr ls = (list_ptr)malloc(sizeof(struct ELE));
	list_ptr ls1 = (list_ptr)malloc(sizeof(struct ELE));	
	list_ptr ls2 = (list_ptr)malloc(sizeof(struct ELE));
	ls->val = 0x100;
	ls->next = ls1;
	ls1->val = 0x200;
	ls1->next = ls2;
	ls2->val = 0x300;
	ls2->next = NULL;

	rsum_list(ls);

	return 0;
}

long rsum_list(list_ptr ls){
	if(!ls)
		return 0;
	else{
		long val = ls->val;
		long rest = rsum_list(ls->next);
		return val+rest;
			
	}
}

