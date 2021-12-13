#ifndef PRODUCER_H
#define PRODUCER_H

#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include "constants.h"
#include "buffer.h"

void producer_run(buffer_s* const buffer, const int sem_id, const int pro_id);

void producer_create(buffer_s* const buffer, const int pro_id, const int sem_id);

#endif