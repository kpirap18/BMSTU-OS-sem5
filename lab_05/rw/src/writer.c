#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "writer.h"

extern int *counter;

struct sembuf StartWrite[6] = 
{
	{WAIT_WRITERS, V, SEM_FLG},    // Увеличивает кол-во ожидающий писателей.
	{ACTIVE_READERS, S, SEM_FLG},  // Ждет, пока все читатели дочитают.
	// {CW, P, SEM_FLG},           // Ждет, пока писатель допишет.
	{CAN_WRITE, S, SEM_FLG},       // Ждет, пока что другой писатель допишет.
	{CAN_WRITE, V, SEM_FLG},       // Запрещает писать.
	{CAN_READ, V, SEM_FLG},        // Запрещает читать.
	{WAIT_WRITERS, P, SEM_FLG}     // Уменьшает кол-во ожидающий писателей. Т.к. он уже не ждет, а пишет
};

struct sembuf StopWrite[2] = {
	{CAN_READ, P, SEM_FLG},        // Разрешает читать
	// {CW, V, SEM_FLG}            // Разрешает писать.
	{CAN_WRITE, P, SEM_FLG}        // Разрешает писать.
};

// Функция производит операции над выбранными элементами из набора 
// семафоров semid(1). Каждый из элементов nsops(3) в массиве sops(2) определяет операцию,
// производимую над семафором в структуре struct sembuf
int start_write(int sem_id) 
{
    return semop(sem_id, StartWrite, 6) != -1;
}

int stop_write(int sem_id) 
{
    return semop(sem_id, StopWrite, 2) != -1;
}


void writer_run(const int sem_id, const int writer_id)
{
	int sleep_time = rand() % WRITER_SLEEP_TIME + 1;
	sleep(sleep_time);

	int rv = start_write(sem_id); 
	if (rv == 0)
	{
		perror("Писатель не может изменить значение семафора.\n");
		exit(ERROR);
	}

	// Началась критическая зона
	(*counter)++;
	printf("\e[1;32mWriter #%d \twrite: \t%d \tsleep: %d\e[0m\n", writer_id, *counter, sleep_time);
	// Закончилась критическая зона

	rv = stop_write(sem_id);
	if (rv == 0)
	{
		perror("Писатель не может изменить значение семафора.\n");
		exit(ERROR);
	}
}

void writer_create(const int sem_id, const int writer_id)
{
	pid_t childpid;
	if ((childpid = fork()) == ERROR_FORK)
	{
		perror("Ошибка при порождении писателя.");
		exit(ERROR);
	}
	else if (childpid == 0)
	{
		// Это процесс потомок.
		while (1)
			writer_run(sem_id, writer_id);
		exit(OK);
	}
}