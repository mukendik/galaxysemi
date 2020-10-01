#ifndef _ZCSL_H_
#define _ZCSL_H_

#include "zbase.h"

#define CSL_MAJOR_VERSION 4
#define CSL_MINOR_VERSION 3

#ifndef ZC_BUILDING_ZCSL
  #define ZCslLink0 ZImport0
  #define ZCslLink(type) ZImport(type)
#else
  #define ZCslLink0 ZExport0
  #define ZCslLink(type) ZExport(type)
#endif

#endif /* _ZCSL_H_ */
