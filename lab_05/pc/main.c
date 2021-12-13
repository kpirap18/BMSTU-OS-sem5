#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <wait.h>
#include <unistd.h>
#include <sys/stat.h>

#include "buffer.h"
#include "constants.h"
#include "producer.h"
#include "concumer.h"

int main(void)
{
	setbuf(stdout, NULL);
    srand(time(NULL));

// Значение IPC_PRIVATE указывает, что к разделяемой
// памяти нельзя получить доступ другим процессам.
// shmget - создает новый разделяемый сегмент.
// S_IRUSR	Владелец может читать.
// S_IWUSR	Владелец может писать.
// S_IRGRP	Группа может читать.
// S_IROTH	Остальные могут читать.
    int sem_descr;
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, sizeof(buffer_s), IPC_CREAT | perms);
    if (shmid == -1) 
	{
        perror("Ошибка при создании нового разделяемого сегмента.\n");
        return -1;
    }

// Функция shmat() возвращает указатель на сегмент
// shmaddr (второй аргумент) равно NULL,
// то система выбирает подходящий (неиспользуемый)
// адрес для подключения сегмента.
    buffer_s *buffer;
    if ((buffer = (buffer_s*)shmat(shmid, 0, 0)) == (buffer_s*)-1) 
	{
        perror("Ошибка при попытке возврата указателя на сегмент.\n");
        return -1;
    }

    if (init_buffer(buffer) == -1) 
	{
        perror("Ошибка при инициализации буфера.\n");
        return -1;
    }

// IPC_PRIVATE - ключ, который показывает, что
// Набор семафоров могут использовать только процессы,
// Порожденные процессом, создавшим семафор.
// Создание семафоров (3 семафора)
    sem_descr = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);

	if (sem_descr == -1)
	{
		perror("Ошибка при создании набора семафоров.");
		return -1;
	}
	
	if (semctl(sem_descr, SB, SETVAL, 1) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

    if (semctl(sem_descr, SE, SETVAL, N) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

    if (semctl(sem_descr, SF, SETVAL, 0) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

    for (int i = 0; i < COUNT_PRODUCER; i++)
	{
		producer_create(buffer, i + 1, sem_descr);
	}

	for (int i = 0; i < COUNT_CONSUMER; i++)
	{
		consumer_create(buffer, i + 1, sem_descr);
	}

    for (size_t i = 0; i < COUNT_PRODUCER + COUNT_CONSUMER; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("Something wrong with children waiting!");
            return -1;
        }
        if (!WIFEXITED(status))
            printf("One of children terminated abnormally\n");
    }

    printf(" \e[1;34mOk\n");

// IPC_RMID используется для пометки сегмента как удаленного.
	if (shmctl(shmid, IPC_RMID, NULL))
	{
		perror("Ошибка при попытке пометить сегмент как удаленный.");
		return -1;
	}

	if (shmdt((void*)buffer) == -1)
	{
		perror("Ошибка при попытке отключить разделяемый сегмент от адресного пространства процесса.");
		return -1;
	}

	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Ошибка при попытке удаления семафора.");
		return -1;
	}

	return 0;
}