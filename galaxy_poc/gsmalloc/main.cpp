// -------------------------------------------------------------------------- //
// gsmalloc.cpp
// -------------------------------------------------------------------------- //
#include <stdlib.h>
#include "gsmalloc.h"

int main(int , char** )
{
  char* gs_memory_trace = getenv("GS_MEMORY_TRACER");
  if (gs_memory_trace != NULL) {
    GS::MemoryTracer::BeginTrace();
  }

  char* str = new char[80];
  int*  tab = new  int[10];
  struct elem {
    char c;
    int  a;
  };
  struct elem* obj1 = new struct elem;
  struct elem* obj2 = new struct elem;
  delete str;
  delete tab;
  delete obj1;
  obj2->a = 0;

  if (gs_memory_trace != NULL) {
    GS::MemoryTracer::EndTrace();
  }
  return 0;
}
