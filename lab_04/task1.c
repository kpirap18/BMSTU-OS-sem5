#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 2
#define TIME_SLEEP 2

#define OK 0
#define ERR_FORK -1

#define FORK_FAILURE 1

int main()
{
    int child[N];

    printf("Parent process start! PID: %d, GROUP: %d\n", getpid(), getpgrp());

    for (int i = 0; i < N; i++)
    {
        int child_pid = fork();

        if(child_pid == ERR_FORK)
        {
            perror("Can\'t fork()\n");
            return FORK_FAILURE;
        }
        else if (!child_pid)
        {
            printf("BEFORE SLEEP Child %d! PID: %d, PPID: %d, GROUP: %d \n", i + 1, getpid(), getppid(), getpgrp());

            sleep(TIME_SLEEP);
            printf("AFTER SLEEP Child %d! PID: %d, PPID: %d, GROUP: %d\n", i + 1, getpid(), getppid(), getpgrp());
            exit(OK);
        }
        else
        {
            child[i] = child_pid;
        }
        
    }

    printf("Parent process finished! Children: %d, %d! \nParent: PID: %d, GROUP: %d\n", child[0], child[1], getpid(), getpgrp());

    return OK;
}