/*************************************************************************
    > File Name       : brkt.c
    > Author          : Square_R
    > Created Time    : Wed 07 Apr 2021 04:40:51 PM CST
 ************************************************************************/


#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc,char *argv[])
{
	printf("%p\n", sbrk(0));
	printf("%p\n", sbrk(0x1000));
	printf("%p\n", brk(0));
	
     return 0;
}
