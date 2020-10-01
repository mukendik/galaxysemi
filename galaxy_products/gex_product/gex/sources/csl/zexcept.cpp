
#define ZC_BUILDING_ZBASE
#define ZC_EXCEPT_CPP		// This flags that Exception functions are local to this file instead of 'extern'

#include <qstring.h>
#include "zexcept.hpp"

ZExport0 ZException::ZException() : iLine(0) {}

ZExport0 ZException::ZException(const ZString& aMessage) :
   ZStringlist(aMessage), iLine(0) {}

ZExport(void) ZException::logException() const
{
   ZTrace trc(iFile, iLine, "ZException");
   for (long i = 0; i < count(); i++)
      ZTrace::writeMsg(iFile, iLine, (*this)[i]);
}

ZBaseLink(void) ZException::log(const char* message) const
{
    ZTrace trc(iFile, iLine, "ZException");
    ZTrace::writeMsg(iFile, iLine, ZString(message));
}

ZExport(void) ZException::setLocation(const char* aFile, long aLine)
{
   iFile = aFile;
   iLine = aLine;
}

//////////////////////////////////////////////////
// Exception functions called by ALL other modules.
//////////////////////////////////////////////////
void ZTHROW(ZException exc)
{
   exc.setLocation(__FILE__, __LINE__);
   exc.logException();
   throw exc;
}

void ZTHROWEXC(ZString aText)
{
   ZException exc(aText);
   ZTHROW(exc);
}
