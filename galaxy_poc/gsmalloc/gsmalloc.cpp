// -------------------------------------------------------------------------- //
// gsmalloc.cpp
// -------------------------------------------------------------------------- //
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "gsmalloc.h"

// -------------------------------------------------------------------------- //
// Memory tracer
// -------------------------------------------------------------------------- //
namespace GS {
namespace MemoryTracer {

// Members
unsigned int mTrace = 0;
GS_MT_MAP mEntries = GS_MT_MAP();

// BeginTrace
void BeginTrace()
{
    mTrace = 1;
}

// EndTrace
void EndTrace()
{
    mTrace = 0;
    GS_MT_MAP::const_iterator it;
    for (it = mEntries.begin();
         it != mEntries.end(); ++it)
    {
        void* ptr = (*it).first;
        size_t size = (*it).second;
        printf("MemoryTracer: !%9lu at %p\n",
               static_cast<long unsigned int>(size), ptr);
    }
    mEntries.clear();
}

// Alloc
void* Alloc(size_t size, const char* file, int line, const char* func)
{
    void* ptr = malloc(size);
    if (mTrace == 1)
    {
        GS_MT_PAIR entry = GS_MT_PAIR(ptr, size);
        mEntries.insert(entry);
        printf("MemoryTracer: +%9lu at %p in %s:%d (%s)\n",
               static_cast<long unsigned int>(size), ptr, file, line, func);
    }
    return ptr;
}

// Dealloc
void Dealloc(void* ptr, const char* file, int line, const char* func)
{
    if (ptr == NULL)
    {
        return;
    }
    if (mTrace == 1)
    {
        GS_MT_MAP::iterator it = mEntries.find(ptr);
        if (it == mEntries.end())
        {
            //printf("MemoryTracer: ?   not found %p in %s:%d\n",
            //       ptr, file, line);
            free(ptr);
            return;
        }
        size_t size = (*it).second;
        mEntries.erase(it);
        printf("MemoryTracer: -%9lu at %p in %s:%d (%s)\n",
               static_cast<long unsigned int>(size), ptr, file, line, func);
    }
    free(ptr);
}

}  // MemoryTracer
}  // GS
