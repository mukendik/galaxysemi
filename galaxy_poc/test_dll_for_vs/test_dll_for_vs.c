#include <stdlib.h>
#include <stdio.h>
#include "test_dll_for_vs.h"

/* You should define TEST_EXPORTS *only* when building the DLL. */
#ifdef TEST_EXPORTS
  #define TESTAPI __declspec(dllexport)
#else
  #define TESTAPI
#endif

/* Define calling convention in one place, for convenience. */
#define TESTCALL __cdecl

TESTAPI int TESTCALL test_dll_f1(int l1, int l2)
{
    printf("#### TEST DLL: test_dll_f1\n");
    return (l1+l2);
}

