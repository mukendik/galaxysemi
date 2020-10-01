#ifndef _ZPTRLIST_
#define _ZPTRLIST_

#include "zbase.hpp"

class ZPointerlist : public ZBase
{
   public:
      // default constructor
      ZBaseLink0 ZPointerlist();

      // copy constructor
      ZBaseLink0 ZPointerlist(const ZPointerlist& aPointerlist);

      // destructor
      virtual ZBaseLink0 ~ZPointerlist();

      // assignment and concatination
      ZBaseLink(ZPointerlist&) operator=(const ZPointerlist& aPointerlist);

      ZBaseLink(ZPointerlist&) operator+=(const ZPointerlist& aPointerlist);

      // subscribing
      ZBaseLink(void*) operator[](long aIndex) const;

      // list manipulation
      ZBaseLink(ZPointerlist&) removeFromPos(long aIndex);

      ZBaseLink(ZPointerlist&) addAtPos(long aIndex, void* aPointer);

      ZPointerlist& addAsFirst(void* aPointer)
         { return addAtPos(0, aPointer); }

      ZPointerlist& addAsLast(void* aPointer)
         { return addAtPos(iCount, aPointer); }

      // clear list
      ZBaseLink(ZPointerlist&) drop();

      // query list size
      long count() const { return iCount; }

      // find an element
      ZBaseLink(long) find(const void* aPointer) const;

   protected:
      virtual void* copy(void* aPointer) const
         { return aPointer; }

	  virtual void del(void* /*aPointer*/)
         {}

      virtual ZBoolean equal(const void* aPointer1, const void* aPointer2) const
         { return aPointer1 == aPointer2; }

   private:

      long     iCount;                   // # of elements
      long     iSize;                    // current array size
      void**   iList;                    // array of void*
}; // ZPointerlist


#endif // _PTRLIST_
