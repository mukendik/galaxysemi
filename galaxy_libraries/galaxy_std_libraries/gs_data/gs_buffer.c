/******************************************************************************!
 * \file gs_buffer.c
 * \brief Generic buffer which is automaticaly resized when needed
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "gs_buffer.h"

#define GS_BUFFER_SIZE_INIT 536U  // TCP_MSS_DEFAULT is a very good choice

/******************************************************************************!
 * \fn gs_buffer_new
 * \brief Create a descriptor
 * \return GsBuffer descriptor
 ******************************************************************************/
struct GsBuffer* gs_buffer_new()
{
    struct GsBuffer* b = malloc(sizeof(struct GsBuffer));
    if (b == NULL)
    {
        return NULL;
    }
    b->capacity = GS_BUFFER_SIZE_INIT;
    b->ptr = malloc(b->capacity);
    if (b->ptr == NULL)
    {
        free(b);
        return NULL;
    }
    b->size = 0;
    return b;
}

/******************************************************************************!
 * \fn gs_buffer_init
 * \brief Initialize a descriptor
 * \return GsBuffer descriptor
 ******************************************************************************/
struct GsBuffer* gs_buffer_init(struct GsBuffer* b)
{
    if (b == NULL)
    {
        return NULL;
    }
    if (b->ptr == NULL)
    {
        return NULL;
    }
    b->size = 0;
    return b;
}

/******************************************************************************!
 * \fn gs_buffer_quit
 * \brief The oposite of init, do not forget to free the descriptor
 * \return void
 ******************************************************************************/
void gs_buffer_quit(struct GsBuffer* b)
{
    if (b == NULL)
    {
        return;
    }
    if (b->ptr != NULL)
    {
        free(b->ptr);
        b->ptr = NULL;
    }
}

/******************************************************************************!
 * \fn gs_buffer_grow
 * \brief Grow the capacity
 *        Used by gs_buffer_add only
 ******************************************************************************/
int gs_buffer_grow(struct GsBuffer* b)
{
    char* p;
    int c;

    if (b == NULL)
    {
        return 0;
    }

    c = b->capacity + (b->capacity >> 1);
    p = realloc(b->ptr, c);
    if (p == NULL)
    {
        return 0;
    }

    b->ptr = p;
    b->capacity = c;

    return 1;
}

/******************************************************************************!
 * \fn gs_buffer_add
 * \brief Add a formatted string to the buffer, like printf
 *        When the string is not well known, use:
 *        gs_buffer_add(buff, "%s", str), not: gs_buffer_add(buff, str)
 * \return void
 ******************************************************************************/
void gs_buffer_add(struct GsBuffer* b, const char* fmt, ...)
{
    va_list ap;
    int r;

    if (b == NULL)
    {
        return;
    }

    do
    {
        va_start(ap, fmt);
        r = vsnprintf(b->ptr + b->size, b->capacity - b->size, fmt, ap);
        if (r < 0) {
            va_end(ap);
            return;
        }
    } while (r + b->size >= b->capacity && gs_buffer_grow(b));
    if (r + b->size >= b->capacity)
    {
        va_end(ap);
        return;
    }
    b->size += r;

    va_end(ap);
}

/******************************************************************************!
 * \fn gs_buffer_get
 * \brief Get the finalized string
 * \return String
 ******************************************************************************/
char* gs_buffer_get(struct GsBuffer* b)
{
    if (b == NULL)
    {
        return NULL;
    }
    return b->ptr;
}
