#include <gtc_constants.h>
#include <gtc_netmessage.h>
#include <gstdl_utils_c.h>
#include <gtl_core.h>
#include "gtl_socket.h"
#include "gtl_testlist.h"
#include "gtl_constants.h"
#include "gtl_output.h"
#include "gtl_profile.h"
#include "gtl_getset.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strcpy...

extern unsigned int uiPacketNb;
extern unsigned gtl_NumOfPAT;
extern int gtl_nEndlotReceived;
extern int gtl_nDisabled;
extern int gtl_IsInitialized();
extern int gtl_SetState_NotInitialized();
extern GNM_RESULTS gtl_stResults; /* Test result structure */
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern int gtl_PerformEndlot();
extern int gtl_PerformEndOfSplitlot();
extern int gtl_SendEmail();
extern int gtl_SQLiteExecQuery(char* lQuery, char* lOutputFileName);

int gtl_close()
{
    GTL_PROFILE_START

    GTL_LOG(5, "gtl_close", 0);

    //gtl_LogPerfInfo(5);
    //gtl_SendEmail();

    if(!gtl_IsInitialized())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    unsigned nb_parts=gtl_stGlobalInfo.uiPartIndex;
    unsigned nb_parts_good=gtl_stGlobalInfo.mNbPartsGood;

    // Bernard: it is perhaps not a end of the lot, perhaps end of initial test, perhaps end of retest 1,....
    // Let s keep gtl_PerformEndlot() in case of but replace here by gtl_PerformEndOfSplitlot()
    //gtl_PerformEndlot(); // reinit gtl_stGlobalInfo.uiPartIndex to 0

    int lR=gtl_PerformEndOfSplitlot();
    if (lR!=0)
        GTL_LOG(3, "Perform End Of Splitlot failed: %d", lR);

    uiPacketNb = 0;
    gtl_nDisabled = 0;

    // Write QA file
    // GS_QA: dump results buffer
    char* lQaOutput=getenv("GS_QA_OUTPUT_FOLDER");
    if(getenv("GS_QA") && lQaOutput)
    {
        FILE *lFile=NULL;
        int lRes=GTL_CORE_ERR_OKAY;
        char lFileName[GTC_MAX_PATH]="", lSiteString[10]="", lQuery[1024]="";

        // Dump ft_ptest_rollinglimits SQL table
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_sqlite_dump_ft_ptest_rollinglimits");
        if(gtl_stResults.mNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);
        // for the moment no choice: for QA there are precisions diffs between platforms, let s round...
        int lPrecision=4;
        char* lGSQAPrec=getenv("GS_QA_PRECISION_DPAT_RL");
        if (lGSQAPrec)
           if (sscanf(lGSQAPrec, "%d", &lPrecision)!=1)
               lPrecision=4;
        if (lPrecision<0)
            sprintf(lQuery, "select splitlot_id, ptest_info_id, run_id, limit_index, limit_type, limit_mode, "
                "LL, HL from ft_ptest_rollinglimits "
                "order by splitlot_id,ptest_info_id,run_id");
        else
            sprintf(lQuery, "select splitlot_id, ptest_info_id, run_id, limit_index, limit_type, limit_mode, "
                "round(LL,%d), round(HL,%d) from ft_ptest_rollinglimits "
                "order by splitlot_id,ptest_info_id,run_id", lPrecision, lPrecision);
        gtl_SQLiteExecQuery(lQuery, lFileName);

        // Dump ft_mptest_rollinglimits SQL table
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_sqlite_dump_ft_mptest_rollinglimits");
        if(gtl_stResults.mNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);
        sprintf(lQuery, (char*)"select * from ft_mptest_rollinglimits order by splitlot_id,mptest_info_id,run_id");
        gtl_SQLiteExecQuery(lQuery, lFileName);

        // Dump ft_run SQL table
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_sqlite_dump_ft_run");
        if(gtl_stResults.mNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);
        sprintf(lQuery, (char*)"select * from ft_run order by splitlot_id,run_id");
        gtl_SQLiteExecQuery(lQuery, lFileName);

        // Dump ft_splitlot SQL table
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_sqlite_dump_ft_splitlot");
        if(gtl_stResults.mNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);
        // Guys : in GTL, dont use public gtl command query: it now allows only "select" queries.
        // When inside GTL, use instead gtl_SQLiteExecQuery(...)
        //gtl_set(GTL_KEY_QUERY_OUTPUT_FILE, lFileName);
        strcpy(lQuery, "update ft_splitlot set setup_t=0, start_t=0, finish_t=0, insertion_time=0, day=''");
        strcat(lQuery, ", week_nb=0, month_nb=0, quarter_nb=0, year_nb=0, year_and_week='', year_and_month=''");
        strcat(lQuery, ", year_and_quarter=''");
        //gtl_set(GTL_KEY_QUERY, lQuery);
        //gtl_command(GTL_COMMAND_QUERY);
        lRes=gtl_SQLiteExecQuery(lQuery, lFileName);
        if(lRes != GTL_CORE_ERR_OKAY)
        {
            lFile=fopen(lFileName, "a+");
            if(lFile)
            {
                fprintf(lFile, "ERROR EXECUTING SPLITLOT UPDATE QUERY. GTL ERROR CODE is %d.\n", lRes);
                fclose(lFile);
            }
        }
        else
        {
            sprintf(lQuery, (char*)"select * from ft_splitlot order by splitlot_id");
            gtl_SQLiteExecQuery(lQuery, lFileName);
        }

        // Dump some close information
        // Build output file name
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_socket_close");
        if(gtl_stResults.mNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);

        /* Open output file */
        char lValue[1024];
        lFile=fopen(lFileName, "a+");
        if(lFile)
        {
            /* Header */
            fprintf(lFile, "#### Close GTL ####\n");
            fprintf(lFile, "Nb of DPAT parts,%d\n", gtl_NumOfPAT);

            // Dump GTL keys (do not dump keys that can have a different value depending on the platform)
#if 0
            gtl_get(GTL_KEY_CONFIG_FILE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_CONFIG_FILE, lValue);
            gtl_get(GTL_KEY_RECIPE_FILE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_RECIPE_FILE, lValue);
#endif
            gtl_get(GTL_KEY_LIB_STATE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_LIB_STATE, lValue);
            gtl_get(GTL_KEY_SPLITLOT_ID, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SPLITLOT_ID, lValue);
            gtl_get(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lValue);
#if 0
            gtl_get(GTL_KEY_OUTPUT_FOLDER, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_OUTPUT_FOLDER, lValue);
            gtl_get(GTL_KEY_TEMP_FOLDER, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_TEMP_FOLDER, lValue);
#endif
            gtl_get(GTL_KEY_OUTPUT_FILENAME, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_OUTPUT_FILENAME, lValue);
            gtl_get(GTL_KEY_GTM_COMMUNICATION_MODE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_GTM_COMMUNICATION_MODE, lValue);
            gtl_get(GTL_KEY_SOCK_CONNECT_MODE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCK_CONNECT_MODE, lValue);
            gtl_get(GTL_KEY_SOCK_COMM_MODE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCK_COMM_MODE, lValue);
            gtl_get(GTL_KEY_SOCK_CONNECT_TIMEOUT, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCK_CONNECT_TIMEOUT, lValue);
            gtl_get(GTL_KEY_SOCK_RECEIVE_TIMEOUT, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCK_RECEIVE_TIMEOUT, lValue);
            gtl_get(GTL_KEY_SOCK_SEND_TIMEOUT, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCK_SEND_TIMEOUT, lValue);
            gtl_get(GTL_KEY_SOCKET_TRACE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_SOCKET_TRACE, lValue);
            gtl_get(GTL_KEY_RECONNECTION_MODE, lValue);
            fprintf(lFile, "KEY: \"%s\", VALUE:\"%s\"\n", GTL_KEY_RECONNECTION_MODE, lValue);
            fprintf(lFile, "#########################################\n\n");
            fclose(lFile);
        }
    }

    int r=GTL_CORE_ERR_OKAY;

    /* Shutdown socket connection to GTM */
    gtl_sock_Stop();

    /* Free SiteList and TestList */
    r=gtl_tl_Free();
    if (r!=0)
        GTL_LOG(3, "gtl_tl_Free failed: %d", r);

    /* 7304 BG: moved to call gtl_CloseOutput() only if GTL is initialized */
    /* 7266 WT: be sure to close output at the very end of the close to be sure to catch all event in functions above */
    /*          (sock_stop, tl_free, setstate, ...) */
    /* BG: Moved just before gtl_SetState_NotInitialized(), otherwise the output folder has been reset */

    // Tiny hack : gtl_CloseOutput is going to write nb_parts in splitlot table using gtl_stGlobalInfo.uiPartIndex
    // which has been reset by perform end splitlot/lot

    GTL_PROFILE_STOP
    gtl_LogPerfInfo(5);

    gtl_stGlobalInfo.uiPartIndex=nb_parts;
    gtl_stGlobalInfo.mNbPartsGood=nb_parts_good;
    r=gtl_CloseOutput(); // will write/update nb_parts/nb_parts_good into the splitlot table
    gtl_stGlobalInfo.uiPartIndex=0;
    gtl_stGlobalInfo.mNbPartsGood=0;

    // Clear desired limits in order not to impose retest/resume :
    gtcnm_CopyString(gtl_stGlobalInfo.mDesiredLimits, "", GTC_MAX_STRING);

    if (r!=0)
        GTL_LOG(3, "gtl_CloseOutput failed: %d", r);

    /* Set variable to specify that library is not initialized */
    gtl_SetState_NotInitialized();

    // to prevent the next init to think it is already an end of lot
    gtl_nEndlotReceived = 0;

    //LogCurrentPerfInfo();

    return r;
}
