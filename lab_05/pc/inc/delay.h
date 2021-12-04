#ifndef DELAY_H

#define DELAY_H

typedef struct
{
	int count;
    int i;
	int *array;
} delay_s;

delay_s *delays_random_create(int const count, const int dtime);

void init_delays(delay_s *d, const int dtime);

int get_delay(delay_s *d);

void delete_delay(delay_s *d);

#endif