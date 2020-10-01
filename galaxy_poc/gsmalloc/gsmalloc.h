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
    static const char* mFile;
    static int mLine;
    static const char* mFunc;

    // BeginTrace
    void BeginTrace();

    // EndTrace
    void EndTrace();

    // Alloc
    void* Alloc(size_t size, const char* file, int line, const char* func);

    // Dealloc
    void Dealloc(void* ptr, const char* file, int line, const char* func);

  } // MemoryTracer
} // GS

// -------------------------------------------------------------------------- //
// Operators
// -------------------------------------------------------------------------- //
inline
void* operator new(size_t size, const char* file, int line, const char* func)
{
  return GS::MemoryTracer::Alloc(size, file, line, func);
}

inline
void* operator new[](size_t size, const char* file, int line, const char* func)
{
  return GS::MemoryTracer::Alloc(size, file, line, func);
}

#define new new(__FILE__, __LINE__, __FUNCTION__)

inline void operator delete(void* ptr)
{
  GS::MemoryTracer::Dealloc(ptr,
                            GS::MemoryTracer::mFile,
                            GS::MemoryTracer::mLine,
                            GS::MemoryTracer::mFunc);
}

inline void operator delete[](void* ptr)
{
  GS::MemoryTracer::Dealloc(ptr,
                            GS::MemoryTracer::mFile,
                            GS::MemoryTracer::mLine,
                            GS::MemoryTracer::mFunc);
}

#define delete GS::MemoryTracer::mFile = __FILE__, \
               GS::MemoryTracer::mLine = __LINE__, \
               GS::MemoryTracer::mFunc = __FUNCTION__, \
               delete

#endif
