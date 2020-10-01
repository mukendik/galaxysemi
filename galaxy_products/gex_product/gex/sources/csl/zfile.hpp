#ifndef _ZFILE_
#define _ZFILE_

#include "zstring.hpp"

class ZFile : public ZBase {
   public:
      static ZBaseLink(ZString)
         stripExtension(                        // split filename & extension
            const ZString& aFileName,              // file name
            ZString& aExtension);                  // return removed extension
                                                   // returns filename without ext
      static ZBaseLink(ZString)
         stripExtension(                        // remove extension from filename
            const ZString& aFileName);             // file name
                                                   // returns filename without ext
      static ZBaseLink(ZString)
         stripPath(                             // split directory path & filename
            const ZString& aFileName,              // file name
            ZString& aPath);                       // return removed path
                                                   // returns filename without dir
      static ZBaseLink(ZString)
         stripPath(                             // remove directory from filename
            const ZString& aFileName);             // file name
                                                   // returns filename without dir
      static ZBaseLink(ZString)
         addExtension(                          // add extension if none present
            const ZString& aFileName,              // filename
            const ZString& aExtension);            // extension
                                                   // returns filename with extension
      static ZBaseLink(ZString)
         fullPath(                              // get full path of partial name
            const ZString& aFileName);             // partial filename
                                                   // returns full path name
      static ZBaseLink(ZString)
         locateFile(                            // locate file by env var
            const ZString& aFileName,              // file to locate
            const ZString& aEnvVar);               // env var with colon sep. paths
                                                   // returns full path if found,
                                                   //         empty string if not
   private:
      ZFile(); // no instance creation
}; // ZFile

#endif // _ZFILE_
