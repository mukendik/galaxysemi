/******************************************************************************!
 * \file main.cpp
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gs_buffer.h"

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main()
{
    struct GsBuffer* b = gs_buffer_new();
    b = gs_buffer_init(b);
    if (b == NULL)
    {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 1000; ++i)
    {
        gs_buffer_add(b, "Azertyu");
    }

    fprintf(stderr, "capacity = %u\n", b->capacity);
    fprintf(stderr, "size = %zu\n", b->size);

    if (strlen(gs_buffer_get(b)) != 7000)
    {
        gs_buffer_quit(b);
        return EXIT_FAILURE;
    }

    gs_buffer_quit(b);
    free(b);
    return EXIT_SUCCESS;
}
