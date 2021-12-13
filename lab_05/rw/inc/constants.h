#ifndef CONSTANTS_H
#define CONSTANTS_H

// // Errors
// #define OK              0
// #define ERROR           1
// #define ERROR_FORK      -1
// #define ERROR_PIPE      -1
// #define ERROR_SEMOP     -1
// #define ERROR_SEMGET    -1
// #define ERROR_SHMAT     -1
// #define ERROR_SHMGET    -1

#define READERS_COUNT   5
#define WRITERS_COUNT   3

#define READER_SLEEP_TIME 2
#define WRITER_SLEEP_TIME 2

// Семафоры:
#define ACTIVE_READERS  0 // Кол-во активных читателей;
#define CAN_READ        1 // Читатель может читать? (есть ли активный писатель)
#define CAN_WRITE       2 // писатель может записать?
#define WAIT_WRITERS    3 // Кол-во ожидающий писателей, которые хотят записать.

// Операции над семафорами:
#define P -1 // Пропустить;
#define V 1	 // Освободить.
#define S 0	 // sleep.

#endif