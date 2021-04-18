#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define N 	20

extern int errno;

void unix_error(char *msg);
pid_t Fork(void);
unsigned int snooze(unsigned int secs);

int main(int argc, char const *argv[])
{
	int status, i;
	pid_t pid;

	snooze(5);
	/*create N children*/
	for(int i=0; i < N ;i++)
		if((pid = Fork()) == 0)
			exit(100+i);

	while((pid = waitpid(-1, &status, 0)) > 0){
		if(WIFEXITED(status)) //如果正常退出
			printf("child %d terminated normally with eixt status=%d\n", pid, WEXITSTATUS(status));
		else
			printf("child %d terminated abnormally\n", pid);
	}	

	if(errno != ECHILD)
		unix_error("waitpid error");

	return 0;
}

void unix_error(char *msg){
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

pid_t Fork(void){
	pid_t pid;

	if((pid = fork()) < 0)
		unix_error("Fork error");
	return pid;
}

unsigned int snooze(unsigned int secs){
	unsigned int rc = sleep(secs);

	printf("Slet for %d of %d secs.\n", secs-rc, secs);
	exit(0);
}