#include <list>
#include <map>
#include <string> // STL
#include <gstdl_compat.h> // contains namespace : cannot be included in .c file
#include <gstdl_mailer.h>
#include <gstdl_systeminfo.h>
//#include "gtl_message.h"
//#include "gtl_output.h"

extern "C"
{
    // be sure we are in C mode
    #include <gtl_core.h>
    #include "gtl_profile.h"
    #include "gtl_constants.h"
    extern GTL_GlobalInfo gtl_stGlobalInfo; // GTL Global information

    //extern int gtl_log(int severity, char* function, char* file, int line_no, char* message, ...);

    int gtl_syslog(int sev, char* message)
    {
        if (!message)
            return -1;
        return jwsmtp::Syslog(sev, message);
    }

    //! \brief Returns the temp folder to be used by GTL: example: c:/temp/GalaxySemi
    char* gtl_GetGalaxySemiTempFolder()
    {
        if (gtl_stGlobalInfo.mTempFolder[0]!='\0')
        {
            //std::string lMessage = "Temp folder already set. First character is " + gtl_stGlobalInfo.mTempFolder[0];
            GTL_LOG(7, (char *)"Temp folder already set.", 0);
            return gtl_stGlobalInfo.mTempFolder;
        }
        // Let s guess from gstdl, i.e. env vars (TMP_DIR, ...)
        GTL_LOG(7, (char *)"CGSystemInfo variable", 0);
        CGSystemInfo gSystemInfo;
        GTL_LOG(7, (char *)"Calling CGSystemInfo.ReadSystemInfo", 0);
        gSystemInfo.ReadSystemInfo();
        //GTL_LOG(4, (char*)"Cannot read system info", 0);
        GTL_LOG(5, (char*)"Detected Platform: '%s'", gSystemInfo.m_strPlatform.c_str() );
        GTL_LOG(5, (char*)"Detected OS: '%s'", gSystemInfo.m_strOS.c_str());
        GTL_LOG(5, (char*)"Found MAC: '%s'", gSystemInfo.m_strNetworkBoardID.c_str());
        GTL_LOG(5, (char*)"Detected num of processors: '%ld'", gSystemInfo.m_lNumberOfProcessors );
        GTL_LOG(5, (char*)"Account name: '%s'", gSystemInfo.m_strAccountName.c_str() );
        GTL_LOG(5, (char*)"Host ID: '%s'", gSystemInfo.m_strHostID.c_str() );
        GTL_LOG(5, (char*)"Host IP: '%s'", gSystemInfo.m_strHostIP.c_str() );
        GTL_LOG(5, (char*)"Host Name: '%s'", gSystemInfo.m_strHostName.c_str() );
        GTL_LOG(5, (char*)"NetworkBoards IDs: '%s'", gSystemInfo.m_strNetworkBoardsIDs.c_str() );

        return gSystemInfo.GetGalaxySemiTempFolder();
    }

}
