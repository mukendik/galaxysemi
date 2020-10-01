#ifndef _ZPLATFRM_
#define _ZPLATFRM_

#include "zbase.hpp"

class ZPlatform : public ZBase {
   public:
      static ZBaseLink(ZBoolean) isOS2();
      static ZBaseLink(ZBoolean) isLinux();
      static ZBaseLink(ZBoolean) isBSD();
      static ZBaseLink(ZBoolean) isFreeBSD();
      static ZBaseLink(ZBoolean) isNetBSD();
      static ZBaseLink(ZBoolean) isAIX();
      static ZBaseLink(ZBoolean) isSolaris();
      static ZBaseLink(ZBoolean) isUnix();
      static ZBaseLink(ZBoolean) isUnixFamily();
      static ZBaseLink(ZBoolean) isWin95();
      static ZBaseLink(ZBoolean) isWin98();
      static ZBaseLink(ZBoolean) isWinME();
      static ZBaseLink(ZBoolean) isWinNT3();
      static ZBaseLink(ZBoolean) isWinNT4();
      static ZBaseLink(ZBoolean) isWin2000();
      static ZBaseLink(ZBoolean) isWinXP();
      static ZBaseLink(ZBoolean) isWinServer();
      static ZBaseLink(ZBoolean) isWin95Family();
      static ZBaseLink(ZBoolean) isWinNTFamily();
      static ZBaseLink(ZBoolean) isWindows();

      static ZBaseLink(int) numProcessors();

   private:
      ZPlatform();
      ZPlatform(const ZPlatform& aPlatform);
      ZPlatform& operator=(const ZPlatform&);
      static void init();

      static ZBoolean iOS2;
      static ZBoolean iLinux;
      static ZBoolean iFreeBSD;
      static ZBoolean iNetBSD;
      static ZBoolean iAIX;
      static ZBoolean iSolaris;
      static ZBoolean iUnix;
      static ZBoolean iWin95;
      static ZBoolean iWin98;
      static ZBoolean iWinME;
      static ZBoolean iWinNT3;
      static ZBoolean iWinNT4;
      static ZBoolean iWin2000;
      static ZBoolean iWinXP;
      static ZBoolean iWinServer;

      static int iNumProcessors;
};

#endif // _ZPLATFRM_
