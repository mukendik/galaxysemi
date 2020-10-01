#include <stdlib.h>
#include <locale.h>
//#include <ctime> // C++ !!!!
#include <gstdl_utils_c.h>
#include <gtc_netmessage.h>
#include "gtl_core.h"
#include "gtl_error.h"
#include "gtl_constants.h"
#include "gtl_message.h"
#include "gtl_server.h"
#include "gtl_testlist.h"
#include "gtl_output.h"
#include "gtl_profile.h"

#define LIB_NAME "Quantix Tester Library"

extern GTL_GlobalInfo gtl_stGlobalInfo;	/* GTL Global information */
extern GTL_ProdInfo gtl_stProdInfo; /* Global Prod info */
extern GNM_RESULTS gtl_stResults; /* Test result structure */
extern GTL_PatSettings gtl_stPatSettings; /* Pat settings */
extern GTL_Server gtl_stGTMServer; /* GTM server settings */
extern GTL_Station gtl_stStation; /* Tester station information */
extern int gtl_ReadConfigFile();
extern int gtl_InitLog();
extern unsigned gtl_NumOfPAT;
extern int gtl_nEndlotReceived;
extern char	gtl_szApplicationFullName[];
extern int gtl_SetState_Disabled();
extern int gtl_SetState_Enabled();
extern int gtl_GetDisabledRootcauseCode(int);

/* From gtl_testlist.c */
extern SiteItem	gtl_tl_ptSiteArray[];               /* Array of sites */

/*======================================================================================*/
/* Description  : initialize GTL structures.											*/
/* Argument(s)  :                                                                       */
/* Return Codes : GTL_CORE_ERR_OKAY                                                     */
int gtl_InitStructures()
{
    GTL_LOG(5, "gtl_InitStructures...", 0);

    unsigned int uiIndex=0;
    int	nFirstSiteNb=0, nLastSiteNb=0;

    /* Init test result structure */
    gtcnm_InitStruct(GNM_TYPE_RESULTS, &gtl_stResults);

    /* Init PAT settings */
    gtl_stPatSettings.uiBaseline = 0;
    gtl_stPatSettings.uiFlags = 0;

    /* Init ProdInfo structure */
    // 6995: Must be now set by gtl_set_prod_info() before gtl_init
    //gtl_clear_prod_info();

    /* Init global information structure */
    gtl_stGlobalInfo.nCurrentLibState = GTL_STATE_NOT_INITIALIZED;
    gtl_stGlobalInfo.nGtlModules = 0;
    gtl_stGlobalInfo.nDisabledRootcause = GTL_DISABLED_ROOTCAUSE_UNKNOWN;
    strcpy(gtl_stGlobalInfo.szDisabledRootcause, "");
    gtl_stGlobalInfo.uiFindTestKey = GTC_FINDTEST_TESTNUMBER;
    gtl_stGlobalInfo.uiResultFrequency = 0;
    gtl_stGlobalInfo.uiNbRunsBeforeProdInfo = 0;
    gtl_stGlobalInfo.uiNbTests = 0;
    gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
    gtl_stGlobalInfo.uiRunIndex_Lot = 0;
    gtl_stGlobalInfo.uiPartIndex = 0;
    gtl_stGlobalInfo.mNbPartsGood = 0;
    //gtl_stGlobalInfo.mCurrentRetestIndex=0; // moved to prod info
    for(uiIndex=0; uiIndex<256; uiIndex++)
    {
        gtl_stGlobalInfo.pnPatBinning[uiIndex] = -1;
        gtl_stGlobalInfo.pnGoodSoftBins[uiIndex] = -1;
        gtl_stGlobalInfo.pnGoodHardBins[uiIndex] = -1;
    }
    gtl_stGlobalInfo.mWaitForGtmAck = 0;

    /* Init server structure */
    strcpy(gtl_stGTMServer.szServerName, "localhost");

    // Anitcrash
    //strcpy(gtl_stGTMServer.szServerIP, "127.0.0.1"); // or "0.0.0.0"
    strncpy(gtl_stGTMServer.szServerIP, "127.0.0.1", GTC_MAX_IPADDR_LEN); // or "0.0.0.0"

    gtl_stGTMServer.ulServerIP = 0;
    gtl_stGTMServer.uiServerPort = GTL_DEFAULT_SERVER_PORT;
    gtl_stGTMServer.pstNextServer = NULL;

    /* Init Station structure */
    // 6995: Dont clear as should be set before by user through gtl_set_node_info(...)
    //gtl_clear_station();

    /* Call tester interface funcion to retrieve station information */
    /* 6995 : depreacted : user must call gtl_set_node_info(....)
        if(!tester_GetNodeInfo(	&(gtl_stStation.uiStationNb), &(gtl_stStation.uiNbSites),
                                gtl_stStation.pnSiteNumbers, gtl_stStation.szHostID,
                                gtl_stStation.szNodeName, gtl_stStation.szUserName,
                                gtl_stStation.szTesterType, gtl_stStation.szTesterExec,
                                gtl_stStation.szTestJobName, gtl_stStation.szTestJobFile,
                                gtl_stStation.szTestJobPath, gtl_stStation.szTestSourceFilesPath))
        {
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_CGNI, 0, "");
            return GTL_CORE_ERR_CGNI;
        }
    */
    nFirstSiteNb = nLastSiteNb = -1;
    for(uiIndex=0; (uiIndex<256) && (gtl_stStation.pnSiteNumbers[uiIndex] != -1); uiIndex++)
    {
        if(nFirstSiteNb == -1)
            nFirstSiteNb = gtl_stStation.pnSiteNumbers[uiIndex];
        nLastSiteNb = gtl_stStation.pnSiteNumbers[uiIndex];
    }

    /* Normalize paths */
    ut_NormalizePath(gtl_stStation.szTestJobPath);
    ut_NormalizePath(gtl_stStation.szTestSourceFilesPath);

    /* Make sure paths has no ending '/' or '\\' */
    if((gtl_stStation.szTestJobPath[strlen(gtl_stStation.szTestJobPath)-1] == '/')
            || (gtl_stStation.szTestJobPath[strlen(gtl_stStation.szTestJobPath)-1] == '\\'))
        gtl_stStation.szTestJobPath[strlen(gtl_stStation.szTestJobPath)-1] = '\0';
    if((gtl_stStation.szTestSourceFilesPath[strlen(gtl_stStation.szTestSourceFilesPath)-1] == '/')
            || (gtl_stStation.szTestSourceFilesPath[strlen(gtl_stStation.szTestSourceFilesPath)-1] == '\\'))
        gtl_stStation.szTestSourceFilesPath[strlen(gtl_stStation.szTestSourceFilesPath)-1] = '\0';

    return GTL_CORE_ERR_OKAY;
}



/* ======================================================================================
  Description  : initialize GTL library.
  Argument(s)  :
  Return Codes : GTL_CORE_ERR_OKAY if initialization OK, GTL_CORE_ERR_XXX error code else.
*/
int gtl_PrivateInit(char *szFullConfigFileName, char *szFullRecipeFileName)
{
    PT_GNM_R_INIT pstInit_Reply=NULL;
    PT_GNM_R_TESTLIST pstTestList_Reply=NULL;
    PT_GNM_R_PATCONFIG_STATIC pstStaticPatConfig_Reply=NULL;
    PT_GNM_PATCONFIG_DYNAMIC pstDynamicPatConfig_Reply=NULL;
    int nStatus=0;
    unsigned int uiIndex=0;

    GTL_LOG(6, "Conf file=%s", szFullConfigFileName?szFullConfigFileName:"NULL");
    GTL_LOG(6, "Recipe file=%s", szFullRecipeFileName?szFullRecipeFileName:"NULL");

    /* Initialize GTL structures */
    nStatus = gtl_InitStructures();
    if(nStatus != GTL_CORE_ERR_OKAY)
        return nStatus;

    /* Set log file names */
    gtl_msg_InitLogfileNames();

    /* Info message */
    gtl_msg_Info(1, 1, GTL_MSGINFO_INIT, "");

    /* Init communication with GTM server */
    /* Read configuration file */
    nStatus = gtl_ReadConfigFile(szFullConfigFileName);
    if(nStatus != GTL_CORE_ERR_OKAY)
        return nStatus;

    /* If recipe file name passed, make sure it exists (if NULL, the recipe will be read by the GTM server) */
    if(szFullRecipeFileName && (strlen(szFullRecipeFileName)>0))
    {
        ut_NormalizePath(szFullRecipeFileName);
        if(!ut_CheckFile(szFullRecipeFileName)) // just try to open in r mode
        {
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_CRRF, 0, szFullRecipeFileName);
            return GTL_CORE_ERR_CRRF;
        }
    }

    /* Init connection to GTM server, then close recipe file handler */
    int r=gtl_svr_Init(&pstInit_Reply, szFullRecipeFileName);
    if(r!=GTL_CORE_ERR_OKAY)
    {
        gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_INIT, 0, "");
        //if (pstInit_Reply && pstInit_Reply->nStatus==GTC_STATUS_MAXCONNECTIONS)
        //    return GTL_CORE_ERR_GTM_MAXCONN_REACHED;
        return r;
    }

    /* Check status */
    switch(pstInit_Reply->nStatus)
    {
        case GTC_STATUS_OK:
            gtl_stGlobalInfo.nGtlModules = pstInit_Reply->nGtlModules;
            gtl_stGlobalInfo.uiFindTestKey = pstInit_Reply->uiFindTestKey;
            gtl_stGlobalInfo.uiResultFrequency = pstInit_Reply->uiResultFrequency;
            gtl_stGlobalInfo.uiNbRunsBeforeProdInfo = pstInit_Reply->uiNbRunsBeforeProdInfo;
            for(uiIndex=0; uiIndex<256; uiIndex++)
                gtl_stGlobalInfo.pnGoodSoftBins[uiIndex] = pstInit_Reply->pnGoodSoftBins[uiIndex];
            for(uiIndex=0; uiIndex<256; uiIndex++)
                gtl_stGlobalInfo.pnGoodHardBins[uiIndex] = pstInit_Reply->pnGoodHardBins[uiIndex];
            break;

        case GTC_STATUS_CRCF:
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_RECIPE, 0, pstInit_Reply->mMessage);
            return GTL_CORE_ERR_GTM_RECIPE;

        case GTC_STATUS_LICEXPIRED:
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_LICEXPIRED, 0, pstInit_Reply->mMessage);
            return GTL_CORE_ERR_GTM_LICEXPIRED;

        case GTC_STATUS_NO_GTM_LIC:
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_NOLIC, 0, "No GTM license");
            return GTL_CORE_ERR_GTM_NOLIC;

        case GTC_STATUS_NO_GTL_LIC :
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTL_NOLIC, 0, "No GTL license");
            return GTL_CORE_ERR_GTL_NOLIC;

        case GTC_STATUS_MAXCONNECTIONS:
            GTL_LOG(4, "Server max conn reached", 0);
            return GTL_CORE_ERR_GTM_MAXCONN_REACHED;

        default:
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_UNKNOWN_STATUS, 0, "");
            return GTL_CORE_ERR_GTM_UNKNOWN_STATUS;
    }

    /* Check modules to activate */
    if(gtl_stGlobalInfo.nGtlModules & GTC_GTLMODULE_PAT)
    {
        if(!gtl_svr_InitTestList(&pstTestList_Reply))
        {
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_TESTLIST, 0, "");
            return GTL_CORE_ERR_GTM_TESTLIST;
        }
        if(!gtl_tl_Create(pstTestList_Reply))
        {
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_TESTLIST, 0, "");
            return GTL_CORE_ERR_GTM_TESTLIST;
        }

        if(!gtl_svr_InitPat(&pstStaticPatConfig_Reply, &pstDynamicPatConfig_Reply))
        {
          gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_INITPAT, 0, "");
          return GTL_CORE_ERR_GTM_INITPAT;
        }
        if(!gtl_tl_UpdateWithStaticPat(pstStaticPatConfig_Reply))
        {
          gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_UPDATE_SPAT, 0, "");
          return GTL_CORE_ERR_UPDATE_SPAT;
        }
        if(!gtl_tl_UpdateWithDynamicPat(pstDynamicPatConfig_Reply))
        {
          gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_UPDATE_DPAT, 0, "");
          return GTL_CORE_ERR_UPDATE_DPAT;
        }
    }

  /* Free stuff */
  if(pstInit_Reply)
  {
    gtcnm_FreeStruct(GNM_TYPE_R_INIT, pstInit_Reply);
    free(pstInit_Reply);
  }
  if(pstTestList_Reply)
  {
    gtcnm_FreeStruct(GNM_TYPE_R_TESTLIST, pstTestList_Reply);
    free(pstTestList_Reply);
  }
  if(pstStaticPatConfig_Reply)
  {
    gtcnm_FreeStruct(GNM_TYPE_R_PATCONFIG_STATIC, pstStaticPatConfig_Reply);
    free(pstStaticPatConfig_Reply);
  }
  if(pstDynamicPatConfig_Reply)
  {
    gtcnm_FreeStruct(GNM_TYPE_PATCONFIG_DYNAMIC, pstDynamicPatConfig_Reply);
    free(pstDynamicPatConfig_Reply);
  }

  return GTL_CORE_ERR_OKAY;
}

//! \brief Reload any limit using desired limits type (last,...) and right splitlot if retest or resume
//! \param lIgnoreUnknownTests : 'y' or 'n'
//! \return Returns 0 on success
int gtl_InitReloadAnyLimits(char lIgnoreUnknownTests)
{
    if(strcmp(gtl_stGlobalInfo.mDesiredLimits,"")==0)
    {
        return 0;
    }

    // If retest, get last initial test splitlot id
    int lSplitlotID=0;
    int lRC=gtl_GetLastSplitlotId(&lSplitlotID, '0');
    if (lRC!=0)
    {
        GTL_LOG(5, "Cannot retrieve last splitlot id: %d", lRC);
        return lRC;
    }
    if (gtl_stProdInfo.mRetestIndex>0)
    {
        GTL_LOG(5, "Retest: last initial test splitlot is %d", lSplitlotID);
    }
    else
    {
        // resume (GCORE-590)
        if (gtl_stProdInfo.mSplitlotID<1)
        {
            GTL_LOG(4, "Resume: initial test: cannot reload any limits", 0);
            return 0;
            //goto finish_init;
        }
        lSplitlotID=gtl_stProdInfo.mSplitlotID-1;

        GTL_LOG(5, "Resume: previous splitlot is %d", lSplitlotID);
    }

    if (gtl_stGlobalInfo.mReloadLimits == GTL_RELOAD_LIMITS_ON)
    {
        lRC=gtl_SQLiteLoadLimits(gtl_stGlobalInfo.mDesiredLimits, lSplitlotID, lIgnoreUnknownTests);
        if(lRC != 0)
        {
            GTL_LOG(5, "Load Limits failed: %d", lRC);
            return lRC;
        }
    }

    char lM[255]="";
    char lEventType[255]="";
    if (gtl_stProdInfo.mRetestIndex>0)
    {
        sprintf(lEventType, "RETEST");
        sprintf(lM, "Tuning will be turned off because of retest");
    }
    else
    {
        // resume
        sprintf(lEventType, "RESUME");
        sprintf(lM, "Resuming: reloading any available %s limits...", gtl_stGlobalInfo.mDesiredLimits);
    }
    lRC=gtl_OutputNewEvent(lEventType, "", lM); // will insert an event for the current splitlot !
    if (lRC!=0)
    {
        GTL_LOG(3, "Outputing new event failed: %d", lRC)
        return lRC;
    }

    return 0;
}

/*======================================================================================
  Description  : can be called at load of test program to initialize GTL.
  Argument(s)  :
     char *szFullRecipeFileName : Full recipe file name. Can be NULL if recipe to be loded by GTM.
     ...
  Return Codes : GTL_CORE_ERR_OKAY if initialization OK, GTL_CORE_ERR_XXX error code else
*/
int gtl_init(szFullConfigFileName, szFullRecipeFileName, lMaxNumberOfActiveSites, lSitesNumbers, lMaxMessageStackSize)
const char* szFullConfigFileName;
const char* szFullRecipeFileName;
int lMaxNumberOfActiveSites;
int* lSitesNumbers;
const int lMaxMessageStackSize;
{
    GTL_PROFILE_START

    GTL_LOG(5, "gtl_init: NbSites=%d", lMaxNumberOfActiveSites);

    int lStatus=0;

    if(gtl_IsInitialized())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;
    }

    /* FIRST of the FIRST: init log system (requires valid output folder!!) */
    lStatus = gtl_InitLog();
    if(lStatus != GTL_CORE_ERR_OKAY)
    {
        GTL_LOG(4, "gtl_initLog(): failed to init log file.", 0);
        GTL_PROFILE_STOP
        return lStatus;
    }

    /* Now we can log as much as we want... */
    GTL_LOG(7, "Starting GTL initialization...", 0);

    char* lOSType=getenv("OSTYPE");
    GTL_LOG(5, "OSTYPE : %s", lOSType?lOSType:"?");

    char* lHostName=getenv("HOSTNAME");
    GTL_LOG(5, "HOSTNAME : '%s'", lHostName?lHostName:"?");

    char* lHostType=getenv("HOSTTYPE");
    GTL_LOG(5, "HOSTTYPE : '%s'", lHostType?lHostType:"?");

    char* lMachType=getenv("MACHTYPE");
    GTL_LOG(5, "MACHTYPE : '%s'", lMachType?lMachType:"?");

#if defined(__WIN32)
    // Let's spy a little bit
    char* lCompName=getenv("COMPUTERNAME");
    GTL_LOG(5, "COMPUTERNAME: %s", lCompName?lCompName:"?");

    char* lNumOfProc=getenv("NUMBER_OF_PROCESSORS");
    GTL_LOG(5, "NUMBER_OF_PROCESSORS: %s", lNumOfProc?lNumOfProc:"?");

    char* lProcArch=getenv("PROCESSOR_ARCHITECTURE");
    GTL_LOG(5, "PROCESSOR_ARCHITECTURE: %s",  lProcArch?lProcArch:"?");

    char* lProcId=getenv("PROCESSOR_IDENTIFIER");
    GTL_LOG(5, "PROCESSOR_IDENTIFIER : %s", lProcId?lProcId:"?");

    GTL_LOG(5, "Estimated CPU speed: %d mhz", gtl_EstimateCPUFreq() );

    char* lTimeZone=getenv("TZ");
    GTL_LOG(5, "Time Zone : %s", lTimeZone?lTimeZone:"?");
#endif

#if defined(__unix__)
    // Todo: read /proc/cpuinfo
    // FILE *lFile=fopen("/proc/cpuinfo", "r");
#endif
    /* SECOND of the FIRST: try to set LOCALE */
    setlocale(LC_ALL, (char*)""); // Just to retrieve the OS local
    char* lLocal=setlocale(LC_ALL, 0);
    char* lLocalNum=setlocale(LC_NUMERIC, 0);
    GTL_LOG(5, "Default OS Local is: '%s'  Num:'%s' Test:%f", lLocal?lLocal:"?", lLocalNum?lLocalNum:"?", 1.2f);

    char* lLang=getenv("LANG");
    GTL_LOG(5, "LANG='%s'", lLang?lLang:"?");

    GTL_LOG(5, "Trying to force Local to C", 0);
    setlocale(LC_ALL, (char*)"C"); // Just to try to impose C local in order to have right float separator '.'
    lLocal=setlocale(LC_ALL, 0);
    lLocalNum=setlocale(LC_NUMERIC, 0);
    GTL_LOG(5, "Local is now: '%s'  Num:'%s' Test:%f", lLocal?lLocal:"?", lLocalNum?lLocalNum:"?", 1.2f);

    // Let s init this one asap in order to know it in gtl_InitOutput()
    gtl_stStation.uiNbSites=lMaxNumberOfActiveSites;

    /* Init GTL output if activated */
    lStatus=gtl_InitOutput();
    if (lStatus!=0)
    {
        GTL_LOG(3, "gtl_InitOutput failed : %d", lStatus);
        GTL_PROFILE_STOP
        return lStatus; //GTL_CORE_ERR_FAILED_TO_INIT_OUTPUT;
    }

    /* If already initialized, return OK */
    if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
    {
        GTL_LOG(5, "GTL already initialized", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_OKAY;
    }

    if (gtl_stStation.uiNbSites==0)
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_NUM_OF_SITES;
    }

    /* let clean messages from the last session if any */
    gtl_clear_messages_stack();

    /* very first things to do as the next functions could emit messages */
    int max_num_of_messages=lMaxMessageStackSize/((int)sizeof(GTL_Message));
    if (lMaxMessageStackSize<(int)sizeof(GTL_Message))
        max_num_of_messages=0;

    // Let s clear messages from any previous session before any possible resize of the RingBuffer
    RingBufferClear(&gtl_stGlobalInfo.mMessages, 'y');
    // in this case, RB elements are struct that we want to be deleted automatically by the RingBuffer lib
    gtl_stGlobalInfo.mMessages.free_user_data='y';
    gtl_stGlobalInfo.mMessages.maxsize=max_num_of_messages;
    GTL_LOG(5, "gtl_init : Max Message Stack Size=%d max num of message=%d size of message=%d octet",
        lMaxMessageStackSize, max_num_of_messages, sizeof(GTL_Message));

    gtl_NumOfPAT=0;

    /* very important : on igxl the lib is not closed and all the static variables keep their current value. */
    gtl_nEndlotReceived=0;

    sprintf(gtl_szApplicationFullName, "%s %d.%d", LIB_NAME, GTL_VERSION_MAJOR, GTL_VERSION_MINOR );

    /* Init GTL */
    lStatus = GTL_CORE_ERR_OKAY;
    if (lMaxNumberOfActiveSites<1 || lMaxNumberOfActiveSites>255)
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_NUM_OF_SITES;
    }

    int i=0;
    // init to default -1 : mandatory
    //for (i=0; i<256; i++)
      //  gtl_stStation.pnSiteNumbers[i]=-1;

    GTL_LOG(5, "Checking %d site numbers for licit numbers", lMaxNumberOfActiveSites);
    for (i=0; i<lMaxNumberOfActiveSites; i++)
    {
        if (lSitesNumbers[i]<0 || lSitesNumbers[i]>255)
        {
            GTL_LOG(4, "Found a strange site number: %d", lSitesNumbers[i]);
            GTL_PROFILE_STOP
            return GTL_CORE_ERR_INVALID_SITE_NUMBER;
        }
        gtl_stStation.pnSiteNumbers[i]=lSitesNumbers[i];
    }

    // GCORE-590 : let's reload any limits asap in order to send uptodate sites state to GTM
    lStatus=gtl_InitReloadAnyLimits('y');
    if (lStatus!=0)
    {
        GTL_LOG(4, "Reload limits pass 1 failed: %d", lStatus);
        GTL_PROFILE_STOP
        return lStatus;
    }

    lStatus = gtl_PrivateInit((char*)szFullConfigFileName, (char*)szFullRecipeFileName);

    /* If status NOK, return (stay in GTL_STATE_NOT_INITIALIZED state) */
    if(lStatus != GTL_CORE_ERR_OKAY)
    {
        //gtl_close(); // Just to be sure to close the socket if any. Does not work because state is not initialized.
        /* Shutdown any socket connection to GTM */
        gtl_sock_Stop();
        gtl_tl_Free(); // if any
        gtl_nEndlotReceived = 0;
        // Make sure recipe file is reloaded on next trial
        if (gtl_stGlobalInfo.mRecipe)
        {
            free(gtl_stGlobalInfo.mRecipe);
            gtl_stGlobalInfo.mRecipe=0;
        }
        GTL_PROFILE_STOP
        return lStatus;
    }

    /* Check if PAT module allowed (in license checked by GTM) */
    if(!(gtl_stGlobalInfo.nGtlModules & GTC_GTLMODULE_PAT))
    {
        gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_NOPATMODULE, 0, "");
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_GTM_NOPATMODULE;
    }

    /* Check if disabled by LOAD option */
    if(gtl_stGlobalInfo.nUserOptions & GTL_USEROPTION_LOADOPT_DISABLED)
    {
        gtl_SetState_Disabled(gtl_GetDisabledRootcauseCode(GTL_CORE_ERR_LOADOPTION));
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_OKAY;
    }

    // GCORE-590
    // If retest or even resume, reload any limits
    lStatus=gtl_InitReloadAnyLimits('n');
    if (lStatus!=0)
    {
        GTL_LOG(4, "Reload limits pass 2 failed: %d", lStatus);
        GTL_PROFILE_STOP
        return lStatus;
    }

finish_init:

    /* Next state is to run baseline with static PAT limits if any */
    gtl_SetState_Enabled();

    #ifdef GTLDEBUG
        //gtl_set((char*)"http_server", (char*)"on" );
    #endif

    GTL_PROFILE_STOP

    return GTL_CORE_ERR_OKAY;
}
