#define ZC_BUILDING_ZCSL

#include "zcsl.hpp"

ZCsl::Parameter::Parameter(
   const ZString& aName,
   ZBoolean aIsConst,
   ZBoolean aIsRef,
   ZBoolean aIsOptional,
   Parameter* aNext) :
   iName(aName),
   iIsConst(aIsConst),
   iIsRef(aIsRef),
   iIsOptional(aIsOptional),
   iNext(aNext)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Parameter::Parameter(...)");
} // Parameter

ZCsl::Parameter::~Parameter()
{
   ZFUNCTRACE_DEVELOP("ZCsl::Parameter::~Parameter()");
   if (iNext) delete iNext;
} // Parameter
