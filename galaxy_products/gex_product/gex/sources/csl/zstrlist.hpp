/* Copyright (c) 2001 IBK-Landquart-Switzerland. All rights reserved.
 *
 * Module      :  ZStrlist.hpp
 * Application :  IBK Open Class Library
 * Purpose     :  Manages a list of ZString's.
 *
 * Date        Description                                 Who
 * --------------------------------------------------------------------------
 * 2001.05.20  First implementation                        P.Koch, IBK
 *
 * OPEN SOURCE LICENSE
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to IBK at info@ibk-software.ch.
 */

#ifndef _ZSTRLIST_
#define _ZSTRLIST_

#include "zstring.hpp"
#include "zptrlist.hpp"

class ZStringlist : public ZBase 
{ 
   public: 
      ZStringlist() {}

      ZStringlist(const ZString& aZString) 
         { iList.addAsLast(new ZString(aZString)); } 

      ZStringlist& operator=(const ZStringlist& aList) 
         { iList = aList.iList; return *this; } 

      ZStringlist& operator+=(const ZStringlist& aList) 
         { iList += aList.iList; return *this; } 

      ZStringlist& operator+=(const ZString& aZString) 
         { iList.addAsLast(new ZString(aZString)); return *this; } 

      ZStringlist& operator+=(ZString* aZString) 
         { iList.addAsLast(aZString); return *this; } 

      ZString& operator[](long aIndex) const 
         { return *(ZString*)iList[aIndex]; } 

      ZStringlist& removeFromPos(long aIndex) 
         { iList.removeFromPos(aIndex); return *this; } 

      ZStringlist& addAtPos(long aIndex, const ZString& aZString) 
         { iList.addAtPos(aIndex, new ZString(aZString)); return *this; } 

      ZStringlist& addAtPos(long aIndex, ZString* aZString) 
         { iList.addAtPos(aIndex, aZString); return *this; } 

      ZStringlist& addAsFirst(const ZString& aZString) 
         { iList.addAsFirst(new ZString(aZString)); return *this; } 

      ZStringlist& addAsFirst(ZString* aZString) 
         { iList.addAsFirst(aZString); return *this; } 

      ZStringlist& addAsLast(const ZString& aZString) 
         { iList.addAsLast(new ZString(aZString)); return *this; } 

      ZStringlist& addAsLast(ZString* aZString) 
         { iList.addAsLast(aZString); return *this; } 

      ZStringlist& drop() 
         { iList.drop(); return *this; } 

      long count() const 
         { return iList.count(); } 

      long find(const ZString& aZString) const 
         { return iList.find(&aZString); } 

   private: 
      class List : public ZPointerlist 
      { 
         private: 
            void* copy(void* aPointer) const 
               { return new ZString(*(ZString*)(aPointer)); } 

            void del(void* aPointer) 
               { delete (ZString*)aPointer; } 

            ZBoolean equal(const void* aPointer1, const void* aPointer2) const 
               { return *(ZString*)aPointer1 == *(ZString*)aPointer2 ; } 
      }; 

      List iList; 
};

#endif // _ZSTRLIST_
