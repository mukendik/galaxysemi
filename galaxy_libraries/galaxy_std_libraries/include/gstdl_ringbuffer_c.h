#ifndef _RingBuffer_h
#define _RingBuffer_h

    //struct RingBufferCell;

    typedef struct
    {
        void* next;
        void* prev;
        void* userdata;
    } RingBufferCell;

    typedef struct
    {
        RingBufferCell* first;
        RingBufferCell* last;
        int size;
        int maxsize;
        char free_user_data;
    } RingBuffer;

    // max number of cell
    RingBuffer *RingBufferCreate(int length);
    int RingBufferPushBack(RingBuffer* rb, void *p);

    // Pop the front if any, put userdata in userdata if any and return new size or -1
    int RingBufferPopFront(RingBuffer* rb, void* *user_data);
    // Pop the last if any, put userdata in p if any and return new size or -1
    int RingBufferPopBack(RingBuffer* rb, void* *p);

    // printf userdata as int
    void RingBufferPrintf(RingBuffer* rb);
    // free userdata ? 'y' or 'n'
    int RingBufferClear(RingBuffer *buffer, char free_userdata);

    #define RingBuffer_available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)
    #define RingBuffer_available_space(B) ((B)->length - (B)->end - 1)
    #define RingBuffer_full(B) (RingBuffer_available_data((B)) - (B)->length == 0)
    #define RingBuffer_empty(B) (RingBuffer_available_data((B)) == 0)
    #define RingBuffer_puts(B, D) RingBuffer_write((B), bdata((D)), blength((D)))
    #define RingBuffer_get_all(B) RingBuffer_gets((B), RingBuffer_available_data((B)))
    #define RingBuffer_starts_at(B) ((B)->buffer + (B)->start)
    #define RingBuffer_ends_at(B) ((B)->buffer + (B)->end)
    #define RingBuffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
    #define RingBuffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)

#endif
