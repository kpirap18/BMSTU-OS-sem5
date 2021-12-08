#include <stdio.h>
#include <stdlib.h>

#include "delay.h"
#include "constants.h"


delay_s *delays_random_create(int const count, const int dtime)
{
    delay_s *d = malloc(sizeof(delay_s));

    d->array = malloc(count * sizeof(int));
    d->count = count - 1;

    init_delays(d, dtime);

    return d;
}

void init_delays(delay_s *d, const int dtime)
{
    for (int i = 0; i < d->count; i++)
    {
        d->array[i] = rand() % dtime + 1;
    }
}

int get_delay(delay_s *d)
{
    if (d->i > d->count)
    {
        d->i = 0;
    }
    
    return d->array[d->i + 1];
}

void delete_delay(delay_s *d)
{
    if (d->array)
    {
        free(d->array);
    }

    if (d)
    {
        free(d);
    }
}