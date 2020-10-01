// -------------------------------------------------------------------------- //
// gsmalloc.h
// -------------------------------------------------------------------------- //
#ifndef GS_MALLOC_H
#define GS_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <map>

// -------------------------------------------------------------------------- //
// Memory tracer
// -------------------------------------------------------------------------- //
namespace GS {
  namespace MemoryTracer {

    typedef std::map <void*, size_t> GS_MT_MAP;
    typedef std::pair<void*, size_t> GS_MT_PAIR;

    // Members
    unsigned int mTrace = 0;
    GS_MT_MAP mEntries = GS_MT_MAP();
    const char* mFile = "";
    int mLine = 0;

    // BeginTrace
    void BeginTrace() { mTrace = 1; }

    // EndTrace
    void EndTrace() {
      mTrace = 0;
      GS_MT_MAP::const_iterator it;
      for (it  = mEntries.begin();
           it != mEntries.end(); ++it) {
        void*  ptr  = (*it).first;
        size_t size = (*it).second;
        printf("MemoryTracer: !%9lu at %p\n",
               static_cast<long unsigned int>(size), ptr);
      }
      mEntries.clear();
    }

    // Alloc
    void* Alloc(size_t size, const char* file, int line) {
      void* ptr = malloc(size);
      if (mTrace == 1) {
        GS_MT_PAIR entry = GS_MT_PAIR(ptr, size);
        mEntries.insert(entry);
        printf("MemoryTracer: +%9lu at %p in %s:%d\n",
               static_cast<long unsigned int>(size), ptr, file, line);
      }
      return ptr;
    }

    // Dealloc
    void Dealloc(void* ptr, const char* file, int line) {
      if (mTrace == 1) {
        GS_MT_MAP::iterator it = mEntries.find(ptr);
        if (it == mEntries.end()) {
          //printf("MemoryTracer: ?   not found %p in %s:%d\n",
          //       ptr, file, line);
          free(ptr);
          return;
        }
        size_t size = (*it).second;
        mEntries.erase(it);
        printf("MemoryTracer: -%9lu at %p in %s:%d\n",
               static_cast<long unsigned int>(size), ptr, file, line);
      }
      free(ptr);
    }

  } // MemoryTracer
} // GS

// -------------------------------------------------------------------------- //
// Operators
// -------------------------------------------------------------------------- //
inline void* operator new(size_t size, const char* file, int line, const char* func)
{
  return GS::MemoryTracer::Alloc(size, file, line);
}

inline void* operator new[](size_t size, const char* file, int line, const char* func)
{
  return GS::MemoryTracer::Alloc(size, file, line);
}

#define new new(__FILE__, __LINE__, __PRETTY_FUNCTION__)

inline void operator delete(void* ptr)
{
  GS::MemoryTracer::Dealloc(ptr,
                            GS::MemoryTracer::mFile,
                            GS::MemoryTracer::mLine);
}

inline void operator delete[](void* ptr)
{
  GS::MemoryTracer::Dealloc(ptr,
                            GS::MemoryTracer::mFile,
                            GS::MemoryTracer::mLine);
}

#define delete GS::MemoryTracer::mFile = __FILE__, \
               GS::MemoryTracer::mLine = __LINE__, \
               delete

#endif
