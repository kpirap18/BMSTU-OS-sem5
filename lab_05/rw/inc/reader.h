#ifndef READER_H
#define READER_H

void reader_create(const int sem_id, const int reader_id);

void reader_run(const int sem_id, const int reader_id);

#endif