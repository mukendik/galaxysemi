#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h> // to have access to intptr_t
#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif

extern "C"
{
    #include <gstdl_ringbuffer_c.h>
}

// ring buffer unit test

int main(int argc, char** argv)
{
    printf("main: %d args: %s\n", argc, argv[0]?argv[0]:"?");

    srand(69); // with this seed and recipe, we should expect a ringbuffer of ? elements at the end of the unittest

    const int s=30;
    RingBuffer rb; //=RingBuffer_create(s);
    rb.first=0;
    rb.last=0;
    rb.size=0;
    rb.maxsize=s;
    // dont free user data as in this ut user data are not pointers to some struct but just a counter
    rb.free_user_data='n';
    printf("Ring Buffer created: size=%d\n", rb.size);
    //if (!rb)
      //  return EXIT_FAILURE;
    printf("Pushing back %d elements...\n", s+s/2);
    char* c = NULL;
    for (int i = 0; i < s + s / 2; ++i, ++c)
    {
        if (rb.size==s)
            printf("Max RingBuffer size reached...\n");
        int r = RingBufferPushBack(&rb, c);
        if (r<0)
        {
            printf("RingBufferPushBack failed: %d\n", r);
            return EXIT_FAILURE;
        }
        printf("Now %d elemetns in RB\n", r);
        RingBufferPrintf(&rb);
    }
    if (rb.size!=s)
    {
        printf("Bad size: %d vs %d\n", rb.size, s);
        return EXIT_FAILURE;
    }
    printf("Poping back...\n");
    void* p=0;
    int r=RingBufferPopBack(&rb, &p);
    if (r<0 || r!=s-1)
    {
        printf("RingBufferPopBack failed: %d\n", r);
        return EXIT_FAILURE;
    }
    RingBufferPrintf(&rb);
    // Fix Linux compilation error: error: cast from ?void*? to ?int? loses precision
    // Replace (int)p cast with (int)(intptr_t(p)) cast in following lines [BG]
    if ((int)((intptr_t)p) != ((s+s/2)-1))
    {
        printf("bad value: %d vs %d\n", (int)((intptr_t)p), (s+s/2)-1 );
        return EXIT_FAILURE;
    }
    if (rb.size!=s-1)
    {
        printf("RingBuffer has unexpected size : %d vs %d", rb.size, s-1);
        return EXIT_FAILURE;
    }

    int expected_last=(int)((intptr_t)rb.last->userdata); // hack
    int expected_first=(int)((intptr_t)rb.first->userdata);

    printf("Poping front or back...\n");
    c = NULL;
    for (int i = 0; i < 20; ++i, ++c)
    {
        if (rand()<RAND_MAX/2)
        {
            if (rand()<RAND_MAX/2)
            {
                r=RingBufferPopBack(&rb, &p);
                if (expected_last!=-1 && ((intptr_t)p)!=expected_last)
                {
                    printf("Unexpected value: returned=%d vs expected=%d\n", (int)((intptr_t)p), expected_last);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                r=RingBufferPopFront(&rb, &p);
                if (expected_first!=-1 && (int)((intptr_t)p)!=expected_first)
                {
                    printf("Unexpected value: returned=%d vs expected=%d\n", (int)((intptr_t)p), expected_first);
                    return EXIT_FAILURE;
                }
            }

            if (r<0)
            {
                printf("RingBufferPop failed : %d\n", r );
                return EXIT_FAILURE;
            }
        }
        else
        {
            r = RingBufferPushBack(&rb, c);
            //expected_last=i;
        }
        expected_last=(int)((intptr_t)rb.last->userdata); // hack
        expected_first=(int)((intptr_t)rb.first->userdata);

        RingBufferPrintf(&rb);
    }

    //if (rb.size!=22)
      //  return EXIT_FAILURE;

    printf("Freeing RingBuffer...\n");
    RingBufferClear(&rb, 'n');

    return EXIT_SUCCESS;
}
