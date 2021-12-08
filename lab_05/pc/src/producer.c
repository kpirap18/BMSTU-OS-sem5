#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>  

#include "producer.h"
#include "buffer.h"

// в структуре struct sembuf, состоящей из полей:
//     short sem_num;   /* semaphore number: 0 = first */
//     short sem_op;    /* semaphore operation */
//     short sem_flg;   /* operation flags */

struct sembuf producer_begin[2] = 
{
	{SE, P, 0}, // Ожидает освобождения хотя бы одной ячейки буфера.
	{SB, P, 0}  // Ожидает, пока другой производитель или потребитель выйдет из критической зоны.
};
struct sembuf producer_end[2] = 
{
	{SB, V, 0}, // Освобождает критическую зону.
	{SF, V, 0}  // Увеличивает кол-во заполненных ячеек.
};

void producer_run(buffer_s* const buffer, const int sem_id, const int pro_id)
{
	int sleep_time = rand() % PRODUCER_DELAY_TIME + 1;
	sleep(sleep_time);

	// Получаем доступ к критической зоне.
	int rv = semop(sem_id, producer_begin, 2); 
	if (rv == -1)
	{
		perror("Произведитель не может изменить значение семафора.\n");
		exit(-1);
	}

	// Началась критическая зона
	// Положить в буфер.
	const char symb = (buffer->wpos % 26) + 'a';
    if (write_buffer(buffer, symb) == -1) 
	{
        exit(-1);
    }
	printf(" \e[1;32mProducer #%d \twrite: \t%c \tsleep: %d\e[0m \n", pro_id, symb, sleep_time);
	// Закончилась критическая зона

	rv = semop(sem_id, producer_end, 2);
	if (rv == -1)
	{
		perror("Произведитель не может изменить значение семафора.\n");
		exit(-1);
	}
}

void producer_create(buffer_s* const buffer, const int pro_id, const int sem_id)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		// Если при порождении процесса произошла ошибка.
		perror("Ошибка при порождении процесса производителя.");
		exit(-1);
	}
	else if (!childpid) // childpid == 0
	{
		// Это процесс потомок.

		// Каждый производитель производит
		// ITERATIONS_AMOUNT товаров.
		for (int i = 0; i < 8; i++)
			producer_run(buffer, sem_id, pro_id);

		exit(0);
	}
}