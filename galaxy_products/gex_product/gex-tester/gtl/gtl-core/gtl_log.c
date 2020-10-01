#include <stdarg.h> // for VA arg
#include <stdio.h>
#include <stdlib.h>
//#include <syslog.h>
#include <string.h> // for strcmp
#include <time.h>
#include <gstdl_utils_c.h>

#include "sqlite3.h"
#include "gtl_core.h"
#include "gtl_constants.h"

extern GTL_GlobalInfo gtl_stGlobalInfo;				/* GTL Global information */
extern sqlite3 *sSQLiteDB;
extern int gtl_syslog(int sev, char* message);

char sLogQuery[10000];

char* sLevelsStrings[8]={ (char*)"Emerg", (char*)"Alert", (char*)"Critical", (char*)"Error", (char*)"Warning", (char*)"Notice", (char*)"Info", (char*)"Debug" };

/*======================================================================================*/
/* PRIVATE Functions                                                                    */

/* Description  : check if log file can be written.                                     */
/* Argument(s)  :                                                                       */
/* Return Codes : GTL_CORE_ERR_OKAY if initialization OK, GTL_CORE_ERR_XXX error code   */
/*                else.                                                                 */
/*======================================================================================*/
int gtl_InitLog()
{
    GTL_LOG(5, "Init log...", 0);

#if 0 // CODEME (not compiling in debug)
    //int lR=
       gtl_syslog(5, "gtl_InitLog");
    #ifdef GTLDEBUG
        //printf("gtl_syslog %d\n", lR);
    #endif
#endif

    char lFileName[GTC_MAX_PATH]="";
    char lTimeStamp[UT_MAX_TIMESTAMP_LEN]="";
    char lPID[100]="";

    GTL_LOG(5, "Testing output folder value: '%s'...", gtl_stGlobalInfo.mOutputFolder);
    if(strcmp(gtl_stGlobalInfo.mOutputFolder,"") == 0)
        return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;

    /* Get TimeStamp string */
    strcpy(lTimeStamp, "");
    ut_GetCompactTimeStamp(lTimeStamp);
    GTL_LOG(5, "Init log: TimeStamp=%s", lTimeStamp?lTimeStamp:"?");

    /* Get PID */
    strcpy(lPID, "");
    ut_GetPID(lPID);
    GTL_LOG(5, "Init log: PID=%s", lPID?lPID:"?");

    /* Set full log file name */
    sprintf(lFileName, "%s/gtl_%s_%s.log", gtl_stGlobalInfo.mOutputFolder, lTimeStamp, lPID);
    GTL_LOG(5, "Building log path and filename: '%s'", gtl_stGlobalInfo.mOutputFolder);

    GTL_LOG(5, "Normalizing log filename...", 0);
    ut_NormalizePath(lFileName);
    GTL_LOG(5, "Log file path and name normalized: '%s'", lFileName);

    FILE* f=fopen(lFileName, "a");

    /* Try to open log file in append mode */
    if(!f)
    {
        GTL_LOG(3, "Cannot open log file '%s'", lFileName);
        return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;
    }

    GTL_LOG(5, "Log file opened: %s", lFileName);

    /* Success, save log file name */
    fclose(f);
    strcpy(gtl_stGlobalInfo.mLogFileName, lFileName);

    return GTL_CORE_ERR_OKAY;
}

int  gtl_log(int severity, char* function, char* file, int line_no, char* m, ...)
{
    /* Get log level */
    const char* gtlloglevel=getenv("GTL_LOGLEVEL");
    if (!gtlloglevel)
        gtlloglevel=(char*)"5";
    int ll=atoi(gtlloglevel);

    /* Check severity */
    if ( severity> ll)
        return 0;

    /* Format message */
    static char message[8048]="";
    if (m)
    {
        va_list args;
        va_start(args, m);
        #if defined unix || defined __MACH__
           /* vsnprintf(message, sizeof message, file, function); */
           int r = vsnprintf(message, sizeof message, m, args);
           if (r==-1)
               return 0;
        #else
           // _vsnprintf is Microsoft version, supposed to be secure
           _vsnprintf(message, sizeof message, m, args);
        #endif
        va_end(args);
    }
    else
        sprintf(message, "?");

    /* First syslog message */
    char* gtl_no_syslog=getenv("GTL_NO_SYSLOG");
    if(!gtl_no_syslog)
    {
        // What about loging (also) to the server IP ? gtl_stGTMServer.szServerName or szServerIP ?
        // system syslog is implemented on all Linux but do log only to localhost
        // on Windows, there is some implementation of syslog (as syslog-win32) but then it is not cross platform
        // Let s try to use the gstdl jwsmtp cross platform wrappers :
        //char *use_gstdl_syslog=getenv("GSTDL_SYSLOG");
        //if (use_gstdl_syslog)
            gtl_syslog(severity, message);
        //else
          //  syslog(severity, "%s   in %s   in %s   at line %d", message, function?function:"?", file?file:"?", line_no);
    }

    /* Make sure log file name is set */
    // gtl_stGlobalInfo.mLogFileName is initialized by gtl_InitLog() called by gtlInit()
    if(strcmp(gtl_stGlobalInfo.mLogFileName, "") != 0)
    {
        /* Open log file & append to it */
        FILE* f=fopen(gtl_stGlobalInfo.mLogFileName, "a");
        if (f)
        {
           time_t rawtime;
           time(&rawtime);
           struct tm *timeinfo = localtime (&rawtime);
           char buffer[80]="";
           strftime(buffer,80,"%H:%M:%S", timeinfo);
           ut_ReplaceChar(message, '\n', ' ');
           fprintf(f, "%s: %d: %s   in %s   in %s   at line %d\n",
                   buffer, severity, message, function?function:"?", file?file:"?", line_no);
           fclose(f);
        }
        else
        {
            if(!gtl_no_syslog)
            {
                sprintf(message, "Cannot open GTL log file %s. Unable to append to log file.", gtl_stGlobalInfo.mLogFileName);
                gtl_syslog(3, message);
            }
        }

        //if(!gtl_no_syslog)
            //syslog(3, "GTL log file name not set. Unable to append to log file.");
        //return 0;
    }

    // output to sqlite at the end because message will be retouched to remove all ' chars
    if (sSQLiteDB)
    {
        char* zErrMsg=0;
        ut_ReplaceChar(message, '\'', ' ');
        sprintf(sLogQuery, "insert into logs VALUES(datetime('now','localtime'), %d, '%s', '%s', '%s', %d)",
                severity, message, function?function:"?", file?file:"?", line_no );
        int r=sqlite3_exec(sSQLiteDB, sLogQuery, 0, 0, &zErrMsg);
        if (r!=SQLITE_OK)
        {
            //printf("Cannot output log '%s' to SQLite DB: SQLite error %d\n", message, r);
            //exit(EXIT_FAILURE);
        }
    }

   return 0;
}
