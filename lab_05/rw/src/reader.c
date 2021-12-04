#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "reader.h"
#include "constants.h"

extern int *counter;

struct sembuf StartRead[3] = 
{
	{WAIT_WRITERS, S, SEM_FLG},     // Пропускает всех ожидающих запись писателей.
	{CAN_READ, S, SEM_FLG},         // Ждет, пока писатель допишет.
	{ACTIVE_READERS, V, SEM_FLG}    // Увеличивает кол-во активных читателей.
};

struct sembuf StopRead[1] = 
{
	{ACTIVE_READERS, P, SEM_FLG}    // Уменьшает кол-во активных читателей.
};


int start_read(int sem_id) 
{
    return semop(sem_id, StartRead, 3) != ERROR_SEMOP;
}

int stop_read(int sem_id) 
{
    return semop(sem_id, StopRead, 1) != ERROR_SEMOP;
}


void reader_run(const int sem_id, const int reader_id)
{
	int sleep_time = rand() % READER_SLEEP_TIME + 1;
	sleep(sleep_time);

	int rv = start_read(sem_id); 
	if (rv == 0)
	{
		perror("Читатель не может изменить значение семафора.\n");
		exit(ERROR_SEMOP);
	}

	// Началась критическая зона
	printf("\e[1;31mReader #%d \tread: \t%d \tsleep: %d\e[0m \n", reader_id, *counter, sleep_time);
	// Закончилась критическая зона
	
	rv = stop_read(sem_id);
	if (rv == 0)
	{
		perror("Читатель не может изменить значение семафора.\n");
		exit(ERROR_SEMOP);
	}
}

void reader_create(const int sem_id, const int reader_id)
{
	pid_t childpid;
	if ((childpid = fork()) == ERROR_FORK)
	{
		perror("Ошибка при порождении читателя.");
		exit(ERROR_FORK);
	}
	else if (childpid == 0)
	{
		// Это процесс потомок.
		while (1)
			reader_run(sem_id, reader_id);
		exit(OK);
	}
}