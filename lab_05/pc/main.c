#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

#include "buffer.h"
#include "constants.h"
#include "delay.h"
#include "producer.h"
#include "concumer.h"

struct sembuf init_value[2] = 
{
	{SB, 1, 0}, // SB изначально установлен в 1.
	{SE, N, 0}  // SE изначально равно N.
};


int main(void)
{
	setbuf(stdout, NULL);
    srand(time(NULL));
    int sem_descr;

    delay_s *delaysProducer = delays_random_create(8, PRODUCER_DELAY_TIME);
	delay_s *delaysConsumer = delays_random_create(8, CONSUMER_DELAY_TIME);

// IPC_PRIVATE - ключ, который показывает, что
// Набор семафоров могут использовать только процессы,
// Порожденные процессом, создавшим семафор.
// shmget - создает новый разделяемый сегмент.
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, sizeof(buffer_s), IPC_CREAT | perms);
    if (shmid == -1) {
        perror("Ошибка при создании нового разделяемого сегмента.\n");
        return ERROR_SEMGET;
    }

// Функция shmat() возвращает указатель на сегмент
// shmaddr (второй аргумент) равно NULL,
// то система выбирает подходящий (неиспользуемый)
// адрес для подключения сегмента.
    buffer_s *buffer;
    if ((buffer = (buffer_s*)shmat(shmid, 0, 0)) == (void*)-1) 
	{
        perror("Ошибка при попытке возврата указателя на сегмент.\n");
        return ERROR_SHMAT;
    }

    if (init_buffer(buffer) == -1) 
	{
        perror("Ошибка при инициализации буфера.\n");
        return ERROR_SHMAT;
    }

// Создание семафоров (3 семафора)
    sem_descr = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);

	if (sem_descr == ERROR_SEMGET)
	{
		perror("Ошибка при создании набора семафоров.");
		return ERROR;
	}
	
	semctl(sem_descr, SB, SETVAL, 1);
    semctl(sem_descr, SE, SETVAL, N);
    semctl(sem_descr, SF, SETVAL, 0);
	// if (semop(shmid, init_value, 2))
	// {
	// 	perror("Ошибка при попытке изменить семафор.");
	// 	return ERROR;
	// }

    for (int i = 0; i < COUNT; i++)
	{
		producer_create(buffer, i + 1, sem_descr, delaysProducer);
		consumer_create(buffer, i + 1, sem_descr, delaysConsumer);

		// Обновляем задержки.
		init_delays(delaysProducer, PRODUCER_DELAY_TIME);
		init_delays(delaysConsumer, CONSUMER_DELAY_TIME);
	}


    for (size_t i = 0; i < COUNT_PRODUCER + COUNT_CONSUMER; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("Something wrong with children waiting!");
            exit(ERROR);
        }
        if (!WIFEXITED(status))
            printf("One of children terminated abnormally\n");
    }

    printf(" \e[1;34mOk\n");

	delete_delay(delaysProducer);
	delete_delay(delaysConsumer);

// IPC_RMID используется для пометки сегмента как удаленного.
	if (shmctl(shmid, IPC_RMID, NULL))
	{
		perror("Ошибка при попытке пометить сегмент как удаленный.");
		return ERROR;
	}

	if (shmdt((void*)buffer) == -1)
	{
		perror("Ошибка при попытке отключить разделяемый сегмент от адресного пространства процесса.");
		return ERROR;
	}

	if (semctl(sem_descr, 0, IPC_RMID) == -1)
	{
		perror("Ошибка при попытке удаления семафора.");
		return ERROR;
	}

	return 0;
}