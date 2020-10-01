#ifndef _ZPROCESS_
#define _ZPROCESS_

#include "zbase.hpp"

class ZCurrentThread : public ZBase {
   public:
      static ZBaseLink(void) sleep(unsigned long aSeconds);

   private:
      ZCurrentThread(); // no instance creation
}; // ZCurrentThread

#endif // _ZPROCESS_
