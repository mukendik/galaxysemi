#ifndef _ZEXCEPT_
#define _ZEXCEPT_

#include "ztrace.hpp"
#include "zstrlist.hpp"

class ZException : public ZStringlist { 
   public: 
      ZBaseLink0 ZException(); 
      ZBaseLink0 ZException(const ZString& aMessage); 
	  ZBaseLink(void) log(const char* message) const;
      ZBaseLink(void) logException() const; 
      ZBaseLink(void) setLocation(const char* aFile, long aLine); 
   private: 
      ZString iFile; 
      long iLine; 
};

#ifndef ZC_EXCEPT_CPP
extern void ZTHROWEXC(ZString aText);
#endif

#endif // _ZEXCEPT_
