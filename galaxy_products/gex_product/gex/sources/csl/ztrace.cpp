#define ZC_BUILDING_ZBASE

#include "ztrace.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "../scripting_io.h"

using namespace std;

ZBoolean ZTrace::iInitialized = zFalse;
ZBoolean ZTrace::iPrintFile = zFalse;
ZBoolean ZTrace::iPrintLine = zFalse;

ZTrace::Destination ZTrace::iDestination = ZTrace::none;

long ZTrace::iFuncLevel = 0;

ZString ZTrace::iOutFile;

ZExport0 ZTrace::ZTrace(
   const ZString& aFile,
   long aLine,
   const ZString& aName) :
   iName(aName),
   iFile(aFile),
   iLine(aLine)
{
   write(iFile, iLine, "+"+iName);
   iFuncLevel++;
} // ZTrace

ZExport0 ZTrace::~ZTrace()
{
   if (iFuncLevel > 0) iFuncLevel--;
   write(iFile, iLine, "-"+iName);
} // ~Trace

ZExport(void) ZTrace::writeMsg(
   const ZString& aFile,
   long aLine,
   const ZString& aMessage)
{
   write(aFile, aLine, ZString(">")+aMessage);
} // writeMsg

void ZTrace::write(
   const ZString& aFile,
   long aLine,
   const ZString& aMessage)
{
   if (!iInitialized) {
      char* info = getenv("ZTRACE");
      if (info) {
         int mode(3);
         ZString dest(info);
         long pos = dest.indexOf(',');
         if (pos) {
            mode = dest.subString(pos+1).strip().asInt();
            dest = dest.subString(1,pos-1);
         } // if
         dest.strip();
         if (ZString::lowerCase(dest) == "stderr")
            iDestination = stdError;
         else
            if (ZString::lowerCase(dest) == "stdout")
               iDestination = stdOutput;
            else
               if (dest.size()) {
                  iDestination = toFile;
                  iOutFile = dest;
               } // if
         if (mode & 1) iPrintFile = zTrue;
         if (mode & 2) iPrintLine = zTrue;
      } // if
      iInitialized = zTrue;
   } // if

   ZString prefix;
   if (iPrintFile)
      prefix += aFile.subString(aFile.lastIndexOf(ZC_PATHSEPARATOR)+1);
   if (iPrintLine)
      prefix += "("+ZString(aLine)+")";
   if (prefix.size())
      prefix += ' ';

   ZString space;
   if (iFuncLevel > 0)
      space.leftJustify(iFuncLevel);

   ZString output(prefix+space+aMessage);

   switch (iDestination) {
      case stdError:
	   // Jan2002, Phil: Send Message to Script Window.
	   WriteScriptMessage(output,true);	// Text + EOL
         break;
      case stdOutput:
	   // Jan2002, Phil: Send Message to Script Window.
	   WriteScriptMessage(output,true);	// Text + EOL
         break;
      case toFile: {
         ofstream trc(iOutFile.constBuffer(), ios::app);
         trc << output << endl;
         break;
      }
      default:;
   } // switch
} // write
