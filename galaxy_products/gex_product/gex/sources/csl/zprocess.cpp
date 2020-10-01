#define ZC_BUILDING_ZBASE

#include "zprocess.hpp"
#include "ztrace.hpp"

#ifdef ZC_WIN
   #include <windows.h>
#endif

#ifdef ZC_OS2
   #define INCL_DOSPROCESS
   #include <os2.h>
#endif

#ifdef ZC_UNIXFAM
   #include <sys/types.h>
   #include <sys/time.h>
   #include <unistd.h>
#endif

ZExport(void) ZCurrentThread::sleep(unsigned long aMilliSecs)
{
   ZFUNCTRACE_DEVELOP("ZCurrentThread::sleep(unsigned long aMilliSecs)");
#if ZC_WIN
   Sleep(aMilliSecs);
#elif ZC_OS2
   DosSleep(aMilliSecs);
#elif ZC_UNIXFAM
   struct timeval timeout;
   timeout.tv_sec = aMilliSecs / 1000;
   timeout.tv_usec = (aMilliSecs % 1000) * 1000;
   select(0, NULL, NULL, NULL, &timeout);
#else
   Not yet implemented.
#endif
} // sleep
