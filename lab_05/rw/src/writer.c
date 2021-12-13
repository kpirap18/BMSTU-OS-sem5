#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "writer.h"

extern int *counter;

struct sembuf StartWrite[6] = 
{
	{WAIT_WRITERS, V, 0},    // инкремент счётчика ждущих писателей
	{ACTIVE_READERS, S, 0},  // проверка, есть ли активный читатель
	{CAN_WRITE, S, 0},       // проверка, пишет ли другой писатель
	{CAN_WRITE, V, 0},       // захват семафора активного писателя
	{CAN_READ, V, 0},        // захват семафора может ли читать (то есть запрет чтения)
	{WAIT_WRITERS, P, 0}     // декремент счётчика ждущих писателей
};

struct sembuf StopWrite[2] = {
	{CAN_READ, P, 0},        // Разрешает читать
	{CAN_WRITE, P, 0}        // Разрешает писать. освобождение активного писателя
};

// Функция производит операции над выбранными элементами из набора 
// семафоров semid(1). 
// Каждый из элементов nsops(3) в массиве sops(2) определяет операцию,
// производимую над семафором в структуре struct sembuf
int start_write(int sem_id) 
{
    return semop(sem_id, StartWrite, 6);
}

int stop_write(int sem_id) 
{
    return semop(sem_id, StopWrite, 2);
}


void writer_run(const int sem_id, const int writer_id)
{
	int sleep_time = rand() % WRITER_SLEEP_TIME + 1;
	sleep(sleep_time);

	int rv = start_write(sem_id); 
	if (rv == -1)
	{
		perror("Писатель не может изменить значение семафора.\n");
		exit(-1);
	}

	// Началась критическая зона
	(*counter)++;
	printf("\e[1;32mWriter #%d \twrite: \t%d \tsleep: %d\e[0m\n", 
				writer_id, *counter, sleep_time);
	// Закончилась критическая зона

	rv = stop_write(sem_id);
	if (rv == -1)
	{
		perror("Писатель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void writer_create(const int sem_id, const int writer_id)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("Ошибка при порождении писателя.");
		exit(-1);
	}
	else if (childpid == 0)
	{
		// Это процесс потомок.
		//for (int i = 0; i < 3; i++)
		while (1)
			writer_run(sem_id, writer_id);
		exit(0);
	}
}