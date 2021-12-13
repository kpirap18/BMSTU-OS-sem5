#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "reader.h"
#include "constants.h"

extern int *counter;

struct sembuf StartRead[5] = 
{
	{CAN_READ, V, 0},         
	{WAIT_WRITERS, S, 0},     // проверка, если ли ждущие писатели
	{CAN_WRITE, S, 0},         // проверка, что писатель не пишет
	{ACTIVE_READERS, V, 0},    // инкремент активных читателей
	{CAN_READ, P, 0}
};

struct sembuf StopRead[1] = 
{
	{ACTIVE_READERS, P, 0}    // Декремент активных читателей
};



int start_read(int sem_id) 
{
    return semop(sem_id, StartRead, 5);
}

int stop_read(int sem_id) 
{
    return semop(sem_id, StopRead, 1);
}


void reader_run(const int sem_id, const int reader_id)
{
	int sleep_time = rand() % READER_SLEEP_TIME + 1;
	sleep(sleep_time);

	int rv = start_read(sem_id); 
	if (rv == -1)
	{
		perror("Читатель не может изменить значение семафора.\n");
		exit(-1);
	}

	// Началась критическая зона
	printf("\e[1;33mReader #%d \tread: \t%d \tsleep: %d\e[0m \n", 
				reader_id, *counter, sleep_time);
	// Закончилась критическая зона
	
	rv = stop_read(sem_id);
	if (rv == -1)
	{
		perror("Читатель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void reader_create(const int sem_id, const int reader_id)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("Ошибка при порождении читателя.");
		exit(-1);
	}
	else if (childpid == 0)
	{
		// Это процесс потомок.
		//for (int i = 0; i < 4; i++)
		while (1)
			reader_run(sem_id, reader_id);
		exit(0);
	}
}