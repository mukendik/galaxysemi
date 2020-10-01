// gstdutils.h : Allows to map a file into memory for Windows/Unix
// ----------------------------------------------------------------------------------------------------------

#ifndef _GALAXY_STANDARD_UTILS_HEADER_
#define _GALAXY_STANDARD_UTILS_HEADER_

#include <gstdl_type.h>
#include <gstdl_errormgr.h>

#define GSTDUTILS_API

// ----------------------------------------------------------------------------------------------------------
// class CGStdUtils
namespace GS
{
    class GSTDUTILS_API CGStdUtils
    {
        // Constructor / Destructor
        public:
            CGStdUtils();
            ~CGStdUtils();

            // File related functions

            // System related functions
            #if defined(_WIN32)
                void	GetSystemErrorMsg(PSTR szMessage, INT32 iMaxLength);
            #endif // defined(_WIN32)
    };
}
#endif // _GALAXY_STANDARD_UTILS_HEADER_
