#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "concumer.h"
#include "buffer.h"

// Потребитель.
struct sembuf ConsumerBegin[2] = 
{
	{SF, P, 0}, // Ожидает, что будет заполнена хотя бы одна ячейка буфера.
	{SB, P, 0}  // Ожидает, пока другой производитель или потребитель выйдет из критической зоны.
};

struct sembuf ConsumerEnd[2] = 
{
	{SB, V, 0}, // Освобождает критическую зону.
	{SE, V, 0}  // Увеличивает кол-во пустых ячеек.
};

void consumer_run(buffer_s* const buffer, const int sem_id, const int con_id)
{
    char ch;
	// Создаем случайные задержки.
	int sleep_time = rand() % CONSUMER_DELAY_TIME + 1;
	sleep(sleep_time);

	// Получаем доступ к критической зоне.
	int rv = semop(sem_id, ConsumerBegin, 2);
	if (rv == -1)
	{
		perror("Потребитель не может изменить значение семафора.\n");
		exit(-1);
	}

	// Началась критическая зона
    if (read_buffer(buffer, &ch) == -1) 
	{
        perror("Something went wrong with buffer reading!");
        exit(-1);
    }
    printf(" \e[1;31mConsumer #%d \tread:  \t%c \tsleep: %d\e[0m\n", con_id, ch, sleep_time);
    // Закончилась критическая зона

	rv = semop(sem_id, ConsumerEnd, 2);
	if (rv == -1)
	{
		perror("Потребитель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void consumer_create(buffer_s* const buffer, const int con_id, const int sem_id)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		// Если при порождении процесса произошла ошибка.
		perror("Ошибка при порождении процесса потребителя.");
		exit(-1);
	}
	else if (!childpid) // childpid == 0
	{
		// Это процесс потомок.

		// Каждый потребитель потребляет
		// ITERATIONS_AMOUNT товаров.
		for (int i = 0; i < 8; i++)
			consumer_run(buffer, sem_id, con_id);

		exit(0);
	}
}