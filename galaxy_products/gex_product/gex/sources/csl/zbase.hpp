#ifndef _ZBASE_
#define _ZBASE_

#include "zbase.h"

#ifndef ZC_BUILDING_ZBASE
  #define ZBaseLink0 ZImport0
  #define ZBaseLink(type) ZImport(type)
#else
  #define ZBaseLink0 ZExport0
  #define ZBaseLink(type) ZExport(type)
#endif

typedef int ZBoolean;

enum ZBooleanConstants { zFalse = 0, zTrue = 1 };

class ZBase
{
   public:
      // declare other constants
      enum Constants {
         maxLong = 0x7fffffff,
         maxUlong = 0xffffffff
      }; // Constants

}; // ZBase

#endif // _ZBASE_
