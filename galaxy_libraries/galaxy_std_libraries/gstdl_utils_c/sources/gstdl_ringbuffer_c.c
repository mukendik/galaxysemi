#include <stdint.h> // to have access to intptr_t
#include <stdlib.h>
#include <stdio.h>

#include "gstdl_ringbuffer_c.h"

int RingBufferClear(RingBuffer *rb, char free_userdata)
{
    if (!rb)
        return -1;
    if (rb->size==0)
        return 0;
    void* p=0;
    while (rb->size!=0)
    {
        int r=RingBufferPopBack(rb, &p);
        if (r<0)
            return r;
        if (p && free_userdata=='y')
            free(p);
    }

    if (rb->first)
    {
        rb->first->next=0;
        rb->first->prev=0;
    }

    rb->first=0;
    rb->last=0;
    rb->size=0;
    return 0;
}

int RingBufferPushBack(RingBuffer* rb, void *p)
{
    /*
    printf("RingBufferPushBack: rb->first=%p\n", rb->first);
    if (rb->first)
        printf("RingBufferPushBack: first->next=%p\n", rb->first->next);
    */
    if (!rb)
        return -1;

    if (rb->maxsize<=0)
        return -2; // in your dreams !

    RingBufferCell *c=rb->last;
    if (c)
        c=rb->last->next;

    if (!c && rb->size<rb->maxsize)
    {
         // new cell
         c=(RingBufferCell*)malloc(sizeof(RingBufferCell));
         if (!c)
             return -3;
         c->next=0;
         c->prev=0;
         rb->size++;
     }
     else
     {
         c=rb->first;

         // no more place
         #ifdef GSTDL_DEBUG
            printf("RingBufferPushBack: no more place. Let s delete the first (%p)...\n", rb->first);
         #endif
         if (rb->first)
            rb->first=rb->first->next;
         // delete the 'old' userdata ?
         if (c->userdata && rb->free_user_data=='y')
             free(c->userdata);
    }

    c->userdata=p;

    if (rb->last)
    {
        c->prev=rb->last;
        rb->last->next=c;
    }

    rb->last=c;

    if (!rb->first)
        rb->first=c;

    #ifdef GSTDL_DEBUG
        printf("RingBufferPushBack: rb->first=%p\n", rb->first);
        if (rb->first)
            printf("RingBufferPushBack: first->next=%p\n", rb->first->next);
    #endif
    return rb->size;
}

int RingBufferPopFront(RingBuffer* rb, void* *user_data)
{
    if (!rb)
        return -1;
    if (rb->size==0)
        return 0;
    if (!rb->first)
        return -2;

    *user_data=rb->first->userdata;

    RingBufferCell *c=rb->first;

    // remove first
    if (rb->first->next)
    {
        RingBufferCell* newfirst=(RingBufferCell*)rb->first->next;
        newfirst->prev=0;
        rb->first=newfirst;
    }
    else
        rb->first=0; // rb should be empty

    c->userdata=0;
    c->next=0;
    c->prev=0;

    // we cannot free this element as the user is perhaps going to ask to read/use this element...
    //free(c); // ?

    rb->size--;

    return rb->size;
}

int RingBufferPopBack(RingBuffer* rb, void* *userdata)
{
    if (!rb)
        return -1;
    if (!rb->last)
        return -2;

    //if (*userdata!=0)
        *userdata=rb->last->userdata;

    RingBufferCell *c=rb->last;

    // remove last
    if (rb->last->prev)
    {
        RingBufferCell* newlast=(RingBufferCell*)rb->last->prev;
        newlast->next=0;
        rb->last=newlast;
    }
    else
        rb->last=0; // rb should be empty

    c->userdata=0;
    c->next=0;
    c->prev=0;

    // We cannot free this one because the user is perhaps going to read/use it
    //free(c); ?

    rb->size--;

    return rb->size;
}

void RingBufferPrintf(RingBuffer* rb)
{
    if (!rb)
        return;

    printf("%d : ", rb->size);
    RingBufferCell* c=rb->first;
    do
    {
        if (!c)
            break;
        printf("%d ", (int)((intptr_t)c->userdata));
        if (c==rb->last)
            break;
        c=c->next;
    }
    while(c);
    printf("\n");
    return;
}
