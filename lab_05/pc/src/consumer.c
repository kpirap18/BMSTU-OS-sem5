#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "concumer.h"
#include "buffer.h"

// Потребитель.
struct sembuf ConsumerBegin[2] = 
{
	{SF, P, SEM_FLG}, // Ожидает, что будет заполнена хотя бы одна ячейка буфера.
	{SB, P, SEM_FLG}  // Ожидает, пока другой производитель или потребитель выйдет из критической зоны.
};

struct sembuf ConsumerEnd[2] = 
{
	{SB, V, SEM_FLG}, // Освобождает критическую зону.
	{SE, V, SEM_FLG}  // Увеличивает кол-во пустых ячеек.
};

void consumer_run(buffer_s* const buffer, const int sem_id, const int con_id, delay_s *delays)
{
    char ch;
	// Создаем случайные задержки.
	int sleep_time = rand() % CONSUMER_DELAY_TIME + 1;
	sleep(sleep_time);

	// Получаем доступ к критической зоне.
	int rv = semop(sem_id, ConsumerBegin, 2);
	if (rv == ERROR_SEMOP)
	{
		perror("Потребитель не может изменить значение семафора.\n");
		exit(ERROR);
	}

	// Началась критическая зона
    if (read_buffer(buffer, &ch) == -1) 
	{
        perror("Something went wrong with buffer reading!");
        exit(ERROR);
    }
    printf(" \e[1;31mConsumer #%d \tread:  \t%c \tsleep: %d\e[0m\n", con_id, ch, sleep_time);
    // Закончилась критическая зона

	rv = semop(sem_id, ConsumerEnd, 2);
	if (rv == ERROR_SEMOP)
	{
		perror("Потребитель не может изменить значение семафора.\n");
		exit(ERROR);
	}
}

void consumer_create(buffer_s* const buffer, const int con_id, const int sem_id, delay_s *delays)
{
	pid_t childpid;
	if ((childpid = fork()) == ERROR_FORK)
	{
		// Если при порождении процесса произошла ошибка.
		perror("Ошибка при порождении процесса потребителя.");
		exit(ERROR);
	}
	else if (!childpid) // childpid == 0
	{
		// Это процесс потомок.

		// Каждый потребитель потребляет
		// ITERATIONS_AMOUNT товаров.
		for (int i = 0; i < ITERATIONS_AMOUNT; i++)
			consumer_run(buffer, sem_id, con_id, delays);

		exit(0);
	}
}