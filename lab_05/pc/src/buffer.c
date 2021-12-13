#include "buffer.h"
#include <string.h>
 
int init_buffer(buffer_s* const buffer) 
{
    if (!buffer) 
        return -1;

    memset(buffer, 0, sizeof(buffer_s));
    return 0;
}

int write_buffer(buffer_s* const buffer, const char elem) 
{
    if (!buffer)
        return -1;

    buffer->data[buffer->wpos++] = elem;
    buffer->wpos += 1;
    return 0;
}

int read_buffer(buffer_s* const buffer, char* const dest) 
{
    if (!buffer)
        return -1;

    *dest = buffer->data[buffer->rpos++];
    buffer->rpos += 1;
    return 0;
}