#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>

//#define ALPHABET "abcdefghijklmnopqrstuvwxyz"

// различные флаги
//#define PERMS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
//#define KEY IPC_PRIVATE

// Для задержек.
#define CONSUMER_DELAY_TIME 4 // Потребитель.
#define PRODUCER_DELAY_TIME 2 // Производитель.

// производитель производит по 8 товара
// всего производителей 3
// получаем 24 ячейки памяти надо
#define N 24
#define COUNT_PRODUCER 3
#define COUNT_CONSUMER 3

// // Errors
// #define OK 0
// #define ERROR -2
// #define ERROR_FORK -1
// #define ERROR_PIPE -1
// #define ERROR_SEMOP -1
// #define ERROR_SEMGET -1
// #define ERROR_SHMAT -1

// semaphore
#define SF 0 // buffer full
#define SE 1 // buffer empty
#define SB 2 // binary

// Операции над семафорами
#define P -1 // Пропустить
#define V 1	 // Освободить

#endif