#ifndef BUFFER_H
#define BUFFER_H

#include "constants.h"

typedef struct 
{
    size_t rpos;
    size_t wpos;
    char data[N];
} buffer_s;

int init_buffer(buffer_s* const buffer);
int write_buffer(buffer_s* const buffer, const char elem);
int read_buffer(buffer_s* const buffer, char* const dest);

#endif