// gstdutils.cpp : Defines the entry point for the DLL application.

#define _GALAXY_STDUTILS_EXPORTS_

#include "gstdl_utilsdll.h"
#include "gstdl_utils.h"

namespace GS
{

CGStdUtils::CGStdUtils()
{
}

CGStdUtils::~CGStdUtils()
{
}

// ----------------------------------------------------------------------------------------------------------
// FUNCTIONS AVAILABLE UNDER WIN32 ONLY
// ----------------------------------------------------------------------------------------------------------

#if defined(_WIN32)
    #include <windows.h>

    void CGStdUtils::GetSystemErrorMsg(PSTR szMessage, INT32 iMaxLength)
    {
        LPVOID lpMsgBuf;

        if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         GetLastError(),
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                         (LPTSTR) &lpMsgBuf,
                         0,NULL) == 0)
        // If FormatMessage fail, allocate our own message string.
        {
            lpMsgBuf = (LPVOID)LocalAlloc(0,32);
            strncpy((LPSTR)lpMsgBuf,"Unknown error",iMaxLength);
        }

        (void)strncpy(szMessage,(const char*)lpMsgBuf,iMaxLength);
        LocalFree(lpMsgBuf);
    }
#endif // defined (_WIN32)

}
