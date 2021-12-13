#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include "constants.h"
#include "writer.h"
#include "reader.h"


int *counter = NULL;

int main(void)
{
    int sem_descr;
    int status;

// Функция shmget() создает новый разделяемый сегмент или,
// если сегмент уже существует, то права доступа подтверждаются. 
// S_IRUSR	Владелец может читать.
// S_IWUSR	Владелец может писать.
// S_IRGRP	Группа может читать.
// S_IROTH	Остальные могут читать.
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, sizeof(int), perms);
	if (shmid == -1)
	{
		perror("Ошибка при создании разделяемого сегмента.\n");
		return -1;
	}

// Функция shmat() возвращает указатель на сегмент
    counter = shmat(shmid, NULL, 0);
	if (*(char *)counter == -1)
	{
		perror("Ошибка при возврата указателя на сегмент.\n");
		return -1;
	}
    *counter = 0;
    
// Функция semget() создает новый набор семафоров или открывает уже имеющийся.
// -1 в слечае не удачи
// Если значением key является макрос IPC_PRIVATE, 
// то создается набор семафоров, который смогут использовать только процессы, 
// порожденные процессом, создавшим семафор. 
    sem_descr = semget(IPC_PRIVATE, 4, IPC_CREAT | perms);

	if (sem_descr == -1)
	{
		perror("Ошибка при создании набора семафоров.");
		return -1;
	}

// Функция semctl() позволяет изменять управляющие параметры набора семафоров
// указанным в semid(1 арг) или над семафором с номером semnum(2 арг) из этого набора.
// https://www.opennet.ru/man.shtml?topic=semctl&category=2&russian=0 
    if (semctl(sem_descr, CAN_WRITE, SETVAL, 0) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

	if (semctl(sem_descr, CAN_READ, SETVAL, 0) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

	if (semctl(sem_descr, ACTIVE_READERS, SETVAL, 0) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}

    if (semctl(sem_descr, WAIT_WRITERS, SETVAL, 0) == -1)
	{
		perror( "!!! Can't set control semaphors." );
		return -1;
	}


    for (int i = 0; i < READERS_COUNT; i++)
		reader_create(sem_descr, i + 1);

	for (int i = 0; i < WRITERS_COUNT; i++)
		writer_create(sem_descr, i + 1);

	for (int i = 0; i < READERS_COUNT + WRITERS_COUNT; i++)
		wait(&status);

// Функция shmdt() «отключает» разделяемый сегмент от адресного пространства процесса 
	if (shmdt(counter) == -1)
		perror("Ошибка при попытке отключить разделяемый сегмент от адресного пространства процесса.");
	
	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Ошибка при попытке удаления семафора.");
		return -1;
	}

    return 0;    
}
