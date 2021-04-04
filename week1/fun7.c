/*************************************************************************
    > File Name       : fun7.c
    > Author          : Square_R
    > Created Time    : Fri 02 Apr 2021 09:39:07 PM CST
 ************************************************************************/
struct n;
struct n{
	long num;
	n *ptr1, *ptr2;
};
int fun7(n *ptr, long num){
	if(ptr == NULL) return -1;
	
	if(ptr->num <= num){
		if(ptr->num == num)
			return 0;
		return fun7(ptr->ptr2, num)*2 + 1;
	}else{
		return fun7(ptr->ptr1, num)*2;

	}

}
