
#ifndef _ZTRACE_
#define _ZTRACE_

#include "zstring.hpp"

class ZTrace : public ZBase
{
   public:
      ZBaseLink0
         ZTrace(
            const ZString& aFile,
            long aLine,
            const ZString& aName);

      ZBaseLink0 ~ZTrace();

      static ZBaseLink(void)
         writeMsg(
            const ZString& aFile,
            long aLine,
            const ZString& aMessage);

   private:
      enum Destination {
         none,
         stdError,
         stdOutput,
         toFile
      };

      static void
         write(
            const ZString& aFile,
            long aLine,
            const ZString& aMessage);

      static ZBoolean iInitialized;
      static ZBoolean iPrintFile;
      static ZBoolean iPrintLine;
      static Destination iDestination;
      static long iFuncLevel;
      static ZString iOutFile;

      ZString iName;
      ZString iFile;
      long iLine;
}; // ZTrace

#ifdef ZC_TRACE_ALL
   #define ZMODTRACE_ALL(module)   ZTrace trc_x_3_y_5(__FILE__,__LINE__,module)
   #define ZTRACE_ALL(message)     ZTrace::writeMsg(__FILE__,__LINE__,message)
   #ifndef ZC_TRACE_DEVELOP
     #define ZC_TRACE_DEVELOP
   #endif
#else
   #define ZMODTRACE_ALL(modname)
   #define ZTRACE_ALL(message)
#endif

#ifdef ZC_TRACE_DEVELOP
   #define ZMODTRACE_DEVELOP(module) ZTrace trc_x_4_y_6(__FILE__,__LINE__,module)
   #define ZTRACE_DEVELOP(message)   ZTrace::writeMsg(__FILE__,__LINE__,message)
   #ifndef ZC_TRACE_RUNTIME
     #define ZC_TRACE_RUNTIME
   #endif
#else
   #define ZMODTRACE_DEVELOP(module)
   #define ZTRACE_DEVELOP(message)
#endif

#ifdef ZC_TRACE_RUNTIME
   #define ZMODTRACE_RUNTIME(module) ZTrace trc_x_5_y_7(__FILE__,__LINE__,module)
   #define ZTRACE_RUNTIME(message)   ZTrace::writeMsg(__FILE__,__LINE__,message)
#else
   #define ZMODTRACE_RUNTIME(module)
   #define ZTRACE_RUNTIME(message)
#endif

#ifdef __FUNCTION__
  #define ZFUNCTRACE_ALL(function) ZMODTRACE_ALL(__FUNCTION__)
  #define ZFUNCTRACE_DEVELOP(function) ZMODTRACE_DEVELOP(__FUNCTION__)
  #define ZFUNCTRACE_RUNTIME(function) ZMODTRACE_RUNTIME(__FUNCTION__)
#else
  #define ZFUNCTRACE_ALL(function) ZMODTRACE_ALL(function)
  #define ZFUNCTRACE_DEVELOP(function) ZMODTRACE_DEVELOP(function)
  #define ZFUNCTRACE_RUNTIME(function) ZMODTRACE_RUNTIME(function)
#endif


#endif // _ZTRACE_
