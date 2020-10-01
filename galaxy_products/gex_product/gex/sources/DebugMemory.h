// http://www.codeproject.com/Articles/16560/Memory-Leak-Detector-for-C

#ifdef DEBUG_MEMORY
#ifndef _DEBUG_MEMORY_INCLUDED_
#define _DEBUG_MEMORY_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Some datatypes we use.  Define them if they're not defined already.
#ifndef BOOL
    #define BOOL int
#endif
#if !defined(_WINDOWS_H)
    typedef unsigned long DWORD;
#endif
//#ifndef WORD
//#define WORD unsigned int
//#endif
///#define true 1
//#define false 0

#ifndef WIN32
    #define __cdecl __attribute__((__cdecl__))
#endif

// Here are the overloaded operators and debug versions of the standard malloc, calloc, and free
// stdlib routines.
void * __cdecl operator new(unsigned int size, const char *file, int line);
void * __cdecl operator new [](unsigned int size, const char *file, int line);
// delete
void __cdecl operator delete(void *p);
void __cdecl operator delete [] (void *p);
void __cdecl operator delete(void *p,  const char *file, int line);
void __cdecl operator delete [] (void *p,  const char *file, int line);
// free
void __cdecl _debug_free(void *p);
void * __cdecl _debug_malloc(unsigned int size, const char *file, int line);
void * __cdecl _debug_calloc(unsigned int nNum, unsigned int size, const char *file, int line);

// Prototype to dump the unfreed list and display any leaks.
void DumpUnfreed(BOOL bFreeList);
// Prototype to turn on the log file for all memory allocations and deallocations to trace them
void DumpMemoryLogAllAllocations(BOOL bEnable);

BOOL DelMemoryBreadcrumb(void *addr);

// These macros replace the existing new, delete, malloc, calloc and free with our debug
// versions.  Then override the debug versions with the standard names using a macro so
// that the implementor does not have to rename the standard functions.
#ifndef DEBUG_MEMORY_IMPLEMENTATION
    // CPP
    #define DEBUG_NEW new(__FILE__, __LINE__)
    #define new DEBUG_NEW
    // DEBUG_DELETE seems unusefull because the pointer to the object to delete is enough to identify the entry
    //#define DEBUG_DELETE delete(__FILE__, __LINE__)
    //#define delete DEBUG_DELETE
    // Test to force the deletion through the DebugMemory system as sometimes the DebugMemory delete is used and sometimes not.
    //#define GSDELETE(p) if (DelMemoryBreadcrumb(p)) free(p);
    //#define GSNEW(p)
    // C
    #define DEBUG_MALLOC(x) _debug_malloc(x, __FILE__, __LINE__)
    #define malloc DEBUG_MALLOC
    #define DEBUG_FREE(x) _debug_free(x)
    #define free DEBUG_FREE
    #define DEBUG_CALLOC(x,y) _debug_calloc(x,y, __FILE__, __LINE__)
    #define calloc DEBUG_CALLOC
#endif 


#endif
#else // not DEBUG_MEMORY
    #define GSDELETE(x) delete x;
    #define GSNEW(p) new p();
#endif
