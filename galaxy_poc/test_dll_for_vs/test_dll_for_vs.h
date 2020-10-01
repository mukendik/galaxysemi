/* test_dll_for_vs.h

   Declares the functions to be imported by our application, and exported by our
   DLL, in a flexible and elegant way.
*/

/* Make sure functions are exported with C linkage under C++ compilers. */

#ifdef __cplusplus
extern "C"
{
#endif

/* Declare our Add function using the above definitions. */
int test_dll_f1(int a, int b);

#ifdef __cplusplus
} // __cplusplus defined.
#endif
