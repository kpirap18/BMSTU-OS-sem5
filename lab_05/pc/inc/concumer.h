#ifndef CONSUMER_H

#define CONSUMER_H

#include <sys/sem.h>

#include "constants.h"
#include "delay.h"
#include "buffer.h"


void consumer_run(buffer_s* const buffer, const int sem_id, const int con_id);

void consumer_create(buffer_s* const buffer, const int con_id, const int sem_id);

#endif