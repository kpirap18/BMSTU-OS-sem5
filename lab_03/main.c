#include<stdio.h>
#include<unistd> // она сказала это для маков

int main(void)
{
	int childpid;
	
	if((childpid = fork()) == -1)
	{
		perror("Cant work");
		exit(1);
	}
	else if (childpid == 0)
	{
		printf("childpid %d", gitpid());
		return 0;
	}
	else
	{
		printf("parent: childpid = %d, pid = %d", childpid, gitpid());
		return 0;
	}
}