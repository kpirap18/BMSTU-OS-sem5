#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h> 
#include <signal.h>

#define N 2
#define TIME_SLEEP 2
#define LEN 64

#define OK 0
#define ERR_FORK -1
#define ERR_EXEC -1
#define ERR_PIPE -1

#define FORK_FAILURE 1
#define EXEC_FAILURE 2
#define PIPE_FAILURE 3

_Bool flag = false;


void catch_sig(int sig_num)
{
	flag = true;
	printf("catch_sig: %d\n", sig_num);
}

int main()
{
    int child[N];
    int fd[N];
    char text[LEN] = { 0 };
    char *mes[N] = {"BMSTU IU7-52\n", "ABCDEFG\n"};

    if (pipe(fd) == ERR_PIPE)
    {
        perror("Can't pipe!");
        return PIPE_FAILURE;
    }

    printf("Parent process start! PID: %d, GROUP: %d\n", getpid(), getpgrp());
    signal(SIGINT, catch_sig);
    sleep(2);

    for (int i = 0; i < N; i++)
    {
        int child_pid = fork();

        if(child_pid == ERR_FORK)
        {
            perror("Can\'t fork()\n");
            return ERR_FORK;
        }
        else if (!child_pid)
        {
            if (flag)
            {
                close(fd[0]);
                write(fd[1], mes[i], strlen(mes[i]));
                printf("Message %d sent to parent! %s", i + 1, mes[i]);
            }

            return OK;
        }        
        else
        {
            child[i] = child_pid;
        }
    }

	for (int i = 0; i < N; i++)
	{
		int status;
		int statval = 0;

		pid_t child_pid = wait(&status);

		printf("Child process %d finished. Status: %d\n", child_pid, status);

		if (WIFEXITED(statval))
		{
			printf("Child process %d finished. Code: %d\n", i + 1, WEXITSTATUS(statval));
		}
		else if (WIFSIGNALED(statval))
		{
			printf("Child process %d finished from signal with code: %d\n", i + 1, WTERMSIG(statval));
		}
		else if (WIFSTOPPED(statval))
		{
			printf("Child process %d finished stopped. Number signal: %d\n", i + 1, WSTOPSIG(statval));
		}
	}

    
    printf("\nMessage receive :\n");
    close(fd[1]);
    read(fd[0], text, LEN);
    printf("%s\n", text);
    

    printf("Parent process finished! Children: %d, %d! \nParent: PID: %d, GROUP: %d\n", child[0], child[1], getpid(), getpgrp());

    return OK;
}