/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_main.c                                                                   */
/*                                                                                      */
/* Main file of the GTL (Galaxy Tester Library).                                        */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/* o Teradyne PreProcessor needs '#if' to start at column 0, without any blank.         */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 17 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_MAIN_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined unix || defined __MACH__
    #include <sys/types.h>
    #include <pwd.h>
    //#include <curses.h>
    #include <unistd.h>
#else
    #include <conio.h>
    #include <io.h>
    #include <sys/locking.h>
#endif

#include <gstdl_utils_c.h>
#include <gtc_netmessage.h>

#include "gtl_core.h"
#include "gtl_constants.h"
#include "gtl_server.h"
#include "gtl_output.h"
#include "gtl_testlist.h"
#include "gtl_error.h"
#include "gtl_message.h"
#include "gtc_netmessage.h"
#include "gtl_profile.h"

extern unsigned int uiPacketNb;

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* Type definitions :					                                                */
/*======================================================================================*/

/*======================================================================================*/
/* EXTERN Variables                                                                     */
/*======================================================================================*/
/* From gtl_testlist.c */
extern SiteItem	gtl_tl_ptSiteArray[];			/* Array of sites */

/*======================================================================================*/
/* PUBLIC Variables : declaration                                                       */
/*======================================================================================*/
#define STR(X) (#X)
#define STR2(X) STR(X)
#define JOIN(A, B) A ## B
#define LIBVERSION JOIN(STR(LIB_VERSION_MAJOR), STR(LIB_VERSION_MINOR))

// in gtl_message.c
extern int gtl_push_back_new_message(int severity, char* message, int message_id);
extern int gtl_clear_messages_stack();
extern int gtl_IsDisabled();

char gtl_szApplicationFullName[256];
        //= JOIN(STR2(LIB_NAME), STR2(LIBVERSION));
        //STR2(LIB_VERSION_MAJOR);
        //STR2(LIB_VERSION_MINOR);
int	gtl_nDisabled = 0;					/* Set to 1 if GTL is disabled */
int	gtl_nEndlotReceived = 0;
int	gtl_nDebugMsg_ON = 0;
int	gtl_nBeginJobExecuted = 0;
unsigned gtl_NumOfPAT=0;                     // Number of PAT (Static or Dynamic) in a session (lot)

/* GTM server settings */
GTL_Server	gtl_stGTMServer;
/* Tester Station information */
GTL_Station	gtl_stStation={ 1, 0, 0, 0, 0, {0}, 0, 0, "", "", "", "", "", "", "", "", "" };

// Tester Production information. Set to null to be sure to be null if customer do not call set_prod_info.
// Which value for mRetestIndex: -1 invalid or 0 default ?
// Let s go for 0 because it is the default behavior if no previous session in this lot.
// If previous lot, then gtl_open/init will find previous SQLite and increment the retest index according to the DB.
GTL_ProdInfo gtl_stProdInfo = {
    .mFacilityID[0] = '\0',
    .mFamilyID[0] = '\0',
    .mFlowID[0] = '\0',
    .mFloorID[0] = '\0',
    .mRomCode[0] = '\0',
    .szOperatorName[0] = '\0',
    .szJobRevision[0] = '\0',
    .szLotID[0] = '\0',
    .szSublotID[0] = '\0',
    .szProductID[0] = '\0',
    .mSplitlotID = -1,
    .mRetestIndex = 0,
    .mRetestHbins[0] = 0,
    .mModeCode = ' ',
    .mRetestCode = ' ',
    .mSpecName[0] = '\0',
    .mUserText[0] = '\0',
    .mSpecVersion[0] = '\0',
    .mSetupID[0] = '\0',
    .mTestCode[0] = '\0',
    .mTemperature[0] = '\0'
};

//
GTL_PerfInfo gtl_stPerfInfo={ 0, 0, 0, 0, 0, 0, 0, 0 };

// GTL Global information
GTL_GlobalInfo gtl_stGlobalInfo = {
    .mNbPartsGood = 0,
    .nCurrentLibState = GTL_STATE_NOT_INITIALIZED,
    .mAuxFileName[0] = '\0',
    .mOutputFolder[0] = '\0',
    .mTempFolder[0] = '\0',
    .mLogFileName[0] = '\0',
    .mSocketTraceFileName[0] = '\0',
    .mDataFileName[0] = '\0',
    .mOutputFileName[0] = '\0',
    .mQueryOutputFileName[0] = '\0',
    //.mTraceabilityMode="on\0",
    .mGtmCommunicationMode = GTL_GTMCOMM_ASYNCHRONOUS,
    .mWaitForGtmAck = 0,
    .mEventIndex = 0,
    .mSocketTrace = 0,
    .mReconnectionMode = GTL_RECONNECT_ON,
    .mRecipe = 0,
    .mSocketConnetionMode = GTL_SOCK_BLOCKING,
    .mSocketCommunicationMode = GTL_SOCK_BLOCKING,
    .mSocketConnectionTimeout = GTL_SOCK_CONNECT_TIMEOUT,
    .mSocketReceiveTimeout = GTL_SOCK_RCV_TIMEOUT,
    .mSocketSendTimeout = GTL_SOCK_SND_TIMEOUT,
    //.mCurrentRetestIndex=0, moved to prod info
    .mConfigFileName[0] = '\0',
    .mRecipeFileName[0] = '\0',
    .mDesiredLimits[0] = '\0',
    .mQueryBuffer[0] = '\0',
    .mCurrentMessageID = -1,
    .mCurrentMessageSeverity = -1,
    .mCurrentMessage[0] = '\0',
    .mDesiredSiteNb = -1,
    .mFieldsToMatch = "product_id,lot_id,sublot_id,tester_name,tester_type",
    .mReloadLimits = GTL_RELOAD_LIMITS_ON,
    .mLimitsGetCondition = "if_changed"
};

/* Pat settings */
GTL_PatSettings	gtl_stPatSettings={.uiBaseline=0, .uiFlags=0 };
/* Test result structure */
GNM_RESULTS	gtl_stResults={.mPacketNb=0, .mSilentMode=0, .mNbSites=0, .mNbTests=0, .mNbAllocatedRuns=0, .mNbValidRuns=0,
                           .mTestResults=0, .mRunResults=0, .mTestStats=0
                            };

/*======================================================================================*/
/* PRIVATE Variables : declaration                                                      */
/*======================================================================================*/
GTL_SOCK_MSG gtl_stMessage;

/*======================================================================================*/
/* PRIVATE Functions : declaration                                                      */
/*======================================================================================*/
int	gtl_PrivateInit();
int	gtl_ReadConfigFile();
int	gtl_InitStructures();
int	gtl_SetState_NotInitialized();
int	gtl_SetState_Enabled();
int	gtl_SetState_Offline();
int	gtl_SetState_Disabled();
int	gtl_ProcessMessage();
int	gtl_ProcessMessage_Notify();
int	gtl_ProcessMessage_DynamicPatConfig();
int	gtl_ProcessMessage_Command();
int	gtl_ProcessMessage_WriteToStdf();
int	gtl_DisplayStatus();
int	gtl_DisplayKeys();
void gtl_DisplayInfo();
void gtl_GetRootcauseString();
int	gtl_GetDisabledRootcauseCode();
int gtl_CheckForMessages();
void gtl_Restart_Baseline();
int gtl_PerformEndlot();
int gtl_IsInitialized();
int gtl_InitLog();

/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/

int gtl_set_node_info(unsigned int StationNumber, char* HostID, char* NodeName, char* UserName, char* TesterType,
                  char* TesterExec, char* TestJobName, char* TestJobFile, char* TestJobPath, char* TestSourceFilesPath)
{
    GTL_LOG(5, "gtl_set_node_info %d", StationNumber);

    /* return error code if GTL already initialized */
    if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
        return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

    gtl_stStation.uiStationNb=StationNumber;

    if (HostID)
        gtcnm_CopyString(gtl_stStation.szHostID, HostID, GTC_MAX_HOSTID_LEN);
    if (NodeName)
        gtcnm_CopyString(gtl_stStation.szNodeName, NodeName, GTC_MAX_NODENAME_LEN);
    if (UserName)
        gtcnm_CopyString(gtl_stStation.szUserName, UserName, GTC_MAX_USERNAME_LEN);
    if (TesterType)
        gtcnm_CopyString(gtl_stStation.szTesterType, TesterType, GTC_MAX_TESTERTYPE_LEN);
    if (TesterExec)
        gtcnm_CopyString(gtl_stStation.szTesterExecType, TesterExec, GTC_MAX_TESTEREXEC_LEN);
    if (TestJobName)
        gtcnm_CopyString(gtl_stStation.szTestJobName, TestJobName, GTC_MAX_JOBNAME_LEN);
    if (TestJobFile)
        gtcnm_CopyString(gtl_stStation.szTestJobFile, TestJobFile, GTC_MAX_PATH);
    if (TestJobPath)
        gtcnm_CopyString(gtl_stStation.szTestJobPath, TestJobPath, GTC_MAX_PATH);
    if (TestSourceFilesPath)
        gtcnm_CopyString(gtl_stStation.szTestSourceFilesPath, TestSourceFilesPath, GTC_MAX_PATH);

    return GTL_CORE_ERR_OKAY;
}

int gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID)
{
    GTL_LOG(5, "gtl_set_prod_info lot %s", LotID?LotID:"?" );

    if(gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_NOT_INITIALIZED)
        return GTL_CORE_ERR_LIB_ALREADY_INITIALIZED;

    if (OperatorName)
        gtcnm_CopyString(gtl_stProdInfo.szOperatorName, OperatorName, GTC_MAX_OPERATORNAME_LEN);
    else
        strcpy(gtl_stProdInfo.szOperatorName, "");

    if (JobRevision)
        gtcnm_CopyString(gtl_stProdInfo.szJobRevision, JobRevision, GTC_MAX_JOBREV_LEN);
    else
        strcpy(gtl_stProdInfo.szJobRevision, "");

    if (LotID)
        gtcnm_CopyString(gtl_stProdInfo.szLotID, LotID, GTC_MAX_LOTID_LEN);
    else
        strcpy(gtl_stProdInfo.szLotID, "");

    if (SublotID)
        gtcnm_CopyString(gtl_stProdInfo.szSublotID, SublotID, GTC_MAX_SLOTID_LEN);
    else
        strcpy(gtl_stProdInfo.szSublotID, "");

    if (ProductID)
        gtcnm_CopyString(gtl_stProdInfo.szProductID, ProductID, GTC_MAX_PRODUCTID_LEN);
    else
        strcpy(gtl_stProdInfo.szProductID, "");

    return GTL_CORE_ERR_OKAY;
}

int  gtl_get_lib_version(major, minor)
int *major;
int *minor;
{
    if (major)
        *major=GTL_VERSION_MAJOR;
    if (minor)
        *minor=GTL_VERSION_MINOR;
    return 0;
}

int gtl_endjob()
{
    GTL_PROFILE_START

    /* Make sure GTL is initialized */
    if(!gtl_IsInitialized())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    /* If GTL disabled, return error code */
    if(gtl_IsDisabled())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_DISABLED;
    }

    if(!gtl_nBeginJobExecuted)
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_BEGINJOB_NOT_EXEC;
    }

	gtl_nBeginJobExecuted = 0;

	gtl_stGlobalInfo.uiRunIndex_Lot++;
	gtl_stGlobalInfo.uiRunIndex_Rolling++;

    int r=GTL_CORE_ERR_OKAY;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
	{
        case GTL_STATE_ENABLED:
			/* Check if we should send Production information to GTM */
            if((gtl_stGlobalInfo.uiRunIndex_Lot-1) == gtl_stGlobalInfo.uiNbRunsBeforeProdInfo)
			{
				if(!gtl_svr_SendProdInfo())
				{
                    GTL_LOG(3, "gtl_endjob: gtl_svr_SendProdInfo failed", 0);
                    gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_COMM, 0, "");
                    gtl_SetState_Disabled(GTL_DISABLED_ROOTCAUSE_GTM_COMM);
                    GTL_PROFILE_STOP
                    return GTL_CORE_ERR_FAILED_TO_SEND_PRODINFO;
				}
			}

			/* Check if we need to send results to GTM */
			if(gtl_stGlobalInfo.uiRunIndex_Rolling >= gtl_stGlobalInfo.uiResultFrequency)
			{
				if(!gtl_svr_Results(0))
				{
                    GTL_LOG(3, "gtl_endjob: gtl_svr_Results failed", 0);
                    gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_COMM, 0, "");
                    gtl_SetState_Offline();
                    GTL_PROFILE_STOP
                    return GTL_CORE_ERR_FAILED_TO_SEND_RESULTS;
				}
                int lR=gtl_tl_ResetTestResult();
                if (lR==0)
                    GTL_LOG(3, "gtl_endjob: gtl_tl_ResetTestResult failed", 0);
				gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
			}
			break;

        case GTL_STATE_OFFLINE:
            /* Check if we need to reset test result structure */
			if(gtl_stGlobalInfo.uiRunIndex_Rolling >= gtl_stGlobalInfo.uiResultFrequency)
			{
                int lR=gtl_tl_ResetTestResult();
                if (lR==0)
                    GTL_LOG(3, "gtl_endjob: gtl_tl_ResetTestResult failed: %d", lR);
				gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
			}
			break;

		case GTL_STATE_DISABLED:
		default:
			break;
	}

    //GTL_LOG(7, "gtl_endjob() DONE", 0);

    GTL_PROFILE_STOP

    return r;
}

void gtl_clear_station()
{
    unsigned uiIndex=0;
    gtl_stStation.nStationInfoValid = 1;
    gtl_stStation.uiStationNb = 0;
    gtl_stStation.uiNbSites = 0;
    gtl_stStation.uiFirstSiteNb = 0;
    gtl_stStation.uiLastSiteNb = 0;
    for(uiIndex=0; uiIndex<256; uiIndex++)
        gtl_stStation.pnSiteNumbers[uiIndex] = -1;
    gtl_stStation.nHandlerProberID = 0;
    gtl_stStation.nLoadbordID = 0;
    strcpy(gtl_stStation.szHostID, "");
    strcpy(gtl_stStation.szNodeName, "");
    strcpy(gtl_stStation.szTesterExecType, "");
    strcpy(gtl_stStation.szTesterExecVersion, "");
    strcpy(gtl_stStation.szTesterType, "");
    strcpy(gtl_stStation.szUserName, "");
    strcpy(gtl_stStation.szTestJobName, "");
    strcpy(gtl_stStation.szTestJobFile, "");
    strcpy(gtl_stStation.szTestJobPath, "");
    strcpy(gtl_stStation.szTestSourceFilesPath, "");
}

// No more used because prod info could be set before gtl_init/open.
void gtl_clear_prod_info()
{
    strcpy(gtl_stProdInfo.mFacilityID, "");
    strcpy(gtl_stProdInfo.mFamilyID, "");

    strcpy(gtl_stProdInfo.szJobRevision, "");
    strcpy(gtl_stProdInfo.szLotID, "");
    strcpy(gtl_stProdInfo.szOperatorName, "");
    strcpy(gtl_stProdInfo.szProductID, "");
    strcpy(gtl_stProdInfo.szSublotID, "");
    gtl_stProdInfo.mSplitlotID=-1; // invalid ?
    GTL_LOG(4, "Check me: clear prod info: set RetestIndex to -1 or 0 ?", 0)
    gtl_stProdInfo.mRetestIndex=0; // invalid or default value ?
    strcpy(gtl_stProdInfo.mRetestHbins, "");
}

/*======================================================================================*/
/* Description  : 										*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
#if 0
int gtl_endlot()
{
    GTL_LOG(5, "gtl_endlot", 0);

    if(!gtl_IsInitialized())
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;

    #if 0 // gtl_PerformEndlot() now called directly (so to make sure it is called, even if no more gtl_beginjob() called
        gtl_nEndlotReceived = 1;
    #endif

    gtl_PerformEndlot();

    // let s clear prod info to force the customer to recall gtl_set_prod_info()
    gtl_clear_prod_info();
    // let s clear node info to force the customer to recall gtl_set_node_info()
    gtl_clear_station();

    return GTL_CORE_ERR_OKAY;
}
#endif



/*======================================================================================*/
/* Description  : called at end of production lot.										*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY on success, GTL_CORE_ERR_XXX else                   */
/*======================================================================================*/
int gtl_PerformEndlot()
{
    GTL_LOG(7, "gtl_PerformEndlot", 0);

    int r=GTL_CORE_ERR_OKAY;

    /* Endlot message */
	gtl_msg_Info(1, 1, GTL_MSGINFO_ENDLOT, "");

    gtl_stGlobalInfo.uiPartIndex=0;
    gtl_stGlobalInfo.mNbPartsGood=0;

	/* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
	{
        case GTL_STATE_ENABLED:
			/* Check if we need to send results to GTM */
			if(gtl_stGlobalInfo.uiRunIndex_Rolling > 0)
			{
				gtl_svr_Results(1);
                int lR=gtl_tl_ResetTestResult();
                if (lR==0)
                    GTL_LOG(3, "gtl_tl_ResetTestResult failed", 0);
				gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
			}
			/* Reset some local variables */
			gtl_stGlobalInfo.uiRunIndex_Lot = 0;
			/* Reset testlist */
			gtl_tl_ResetTestlist();
			/* Send endlot to GTM */
			gtl_svr_Endlot();
			break;

        case GTL_STATE_OFFLINE:
            /* Check if we need to reset test result structure */
			if(gtl_stGlobalInfo.uiRunIndex_Rolling > 0)
			{
                r=gtl_tl_ResetTestResult();
				gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
			}
			/* Reset some local variables */
			gtl_stGlobalInfo.uiRunIndex_Lot = 0;
			/* Reset testlist */
            r=gtl_tl_ResetTestlist();
			/* Close library, GTL will attempt to re-initialize on next run */
            //gtl_close(); // DO NOT CALL that to avoid infinite loop between gtl_close and gtl_PerformEndLot.
            break;

		case GTL_STATE_DISABLED:
		default:
			break;
	}
    return r;
}


int  gtl_get_lib_state()
{
    return gtl_stGlobalInfo.nCurrentLibState;
}

int  gtl_get_site_state(site_nb)
const int site_nb;
{
    /* Make sure GTL is initialized */
    if(!gtl_IsInitialized())
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;

    /* If GTL disabled, return error code */
    if(gtl_IsDisabled())
        return GTL_CORE_ERR_DISABLED;

    if ((site_nb<0) || (site_nb>255))
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    SiteItem *lSite = gtl_tl_ptSiteArray + site_nb;
    if(!lSite)
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    return lSite->mCurrentSiteState;
}

/*
int gtl_get_limit_type(site_nb)
const int site_nb;
{
    GTL_LOG(3, "gtl_get_limit_type(%d): code me...", site_nb);
    return GTL_LIMIT_TYPE_NONE;
}
*/

#if 0 // Deprecated for now since GTL V3.6
    /*======================================================================================*/
    /* Description  : reset GTL library.                                                    */
    /*                                                                                      */
    /* Argument(s)  :                                                                       */
    /*                                                                                      */
    /* Return Codes : none                                                                  */
    /*======================================================================================*/
    int gtl_reset()
    {
        GTL_LOG(7, "gtl_reset", 0);

        if(!gtl_IsInitialized())
            return GTL_CORE_ERR_LIB_NOT_INITIALIZED;

        /* Baseline restart message */
        gtl_msg_Info(1, 1, GTL_MSGINFO_RESET, "");

        gtl_close();
        return GTL_CORE_ERR_OKAY;
    }
#endif

/*======================================================================================*/
/* Description  : read configuration file (host name, IP, port...).						*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY if initialization OK, GTL_CORE_ERR_XXX error code   */
/*                else.                                                                 */
/*======================================================================================*/
int gtl_ReadConfigFile(char *szFullConfigFileName)
{
    char szFile[GTC_MAX_PATH]="", szLoadOption[256]="";
    FILE *fpConfFile=NULL;
    int nLastLine=0;

	/* Build configuration file name */
    /* 1) If config file name provided, use it */
    if(szFullConfigFileName && (strlen(szFullConfigFileName)>0))
    {
        ut_NormalizePath(szFullConfigFileName);
        fpConfFile = fopen(szFullConfigFileName, "r");
        if(!fpConfFile)
        {
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_CONF, 0, szFullConfigFileName);
            return GTL_CORE_ERR_CONF;
        }
    }

    /* 2) try $GTL_TESTER_CONF environment variable */
    if(!fpConfFile)
    {
        char *env = getenv("GTL_TESTER_CONF");
        if(env)
        {
            strcpy(szFile, env);
            ut_NormalizePath(szFile);
            fpConfFile = fopen(szFile, "r");
            if(!fpConfFile)
            {
                gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_CONF, 0, szFile);
                return GTL_CORE_ERR_CONF;
            }
        }
    }

    /* 3) if not found, try <source files path>/gtl_tester.conf */
    if(!fpConfFile)
    {
        sprintf(szFile, "%s/gtl_tester.conf", gtl_stStation.szTestSourceFilesPath);
        ut_NormalizePath(szFile);
        fpConfFile = fopen(szFile, "r");
    }

    /* Check if we were able to open the configuration file with one of above trials */
    if(!fpConfFile)
    {
        gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_CONF, 0, szFile);
        return GTL_CORE_ERR_CONF;
    }

    /* SECTION = Server */
    PT_GTL_Server lCurrentServer = NULL;
    int lSectionsToSkip=0;
    int lStatus = ut_ReadFromIniFile(fpConfFile, "Server", "Name", szLoadOption, GTC_MAX_PATH, &nLastLine, lSectionsToSkip);
    while(lStatus == UT_ERR_OKAY)
    {
        /* Check if first server */
        /* CHECKME: check malloc result, and make sure allocated structures are free'ed */
        if(lCurrentServer == NULL)
        {
            lCurrentServer = &gtl_stGTMServer;
        }
        else
        {
            lCurrentServer->pstNextServer = (PT_GTL_Server)malloc(sizeof(GTL_Server));
            lCurrentServer = lCurrentServer->pstNextServer;
        }
        lCurrentServer->pstNextServer = NULL;

        /* Fill server data */
        strcpy(lCurrentServer->szServerName, szLoadOption);
        if(ut_ReadFromIniFile(fpConfFile, "Server", "IP", szLoadOption, GTC_MAX_PATH, &nLastLine, lSectionsToSkip) == UT_ERR_OKAY)
        {
            // anticrash
            //strcpy(lCurrentServer->szServerIP, szLoadOption);
            strncpy(lCurrentServer->szServerIP, szLoadOption, GTC_MAX_IPADDR_LEN);
            lCurrentServer->szServerIP[GTC_MAX_IPADDR_LEN-1]='\0';
            lCurrentServer->ulServerIP = ut_IPtoUL(szLoadOption);
        }
        if(ut_ReadFromIniFile(fpConfFile, "Server", "SocketPort", szLoadOption, GTC_MAX_PATH, &nLastLine, lSectionsToSkip) == UT_ERR_OKAY)
        {
            sscanf(szLoadOption, "%u", &(lCurrentServer->uiServerPort));
        }

        ++lSectionsToSkip;
        lStatus = ut_ReadFromIniFile(fpConfFile, "Server", "Name", szLoadOption, GTC_MAX_PATH, &nLastLine, lSectionsToSkip);
    }

    /* SECTION = LoadOptions */
    if(ut_ReadFromIniFile(fpConfFile, "LoadOptions", "-gtl_disabled", szLoadOption, GTC_MAX_PATH, &nLastLine, 0) == UT_ERR_OKAY)
    {
        if(!strcmp(szLoadOption, "1"))
            /* Disable GTL after init */
            gtl_stGlobalInfo.nUserOptions |= GTL_USEROPTION_LOADOPT_DISABLED;
        else
            /* Don't disable GTL after init */
            gtl_stGlobalInfo.nUserOptions &= ~GTL_USEROPTION_LOADOPT_DISABLED;
    }
    if(ut_ReadFromIniFile(fpConfFile, "LoadOptions", "-gtl_quieton", szLoadOption, GTC_MAX_PATH, &nLastLine, 0) == UT_ERR_OKAY)
    {
        if(!strcmp(szLoadOption, "1"))
            /* Run in quiet mode */
            gtl_stGlobalInfo.nUserOptions |= GTL_USEROPTION_QUIETON;
        else
            /* Don't run in quiet mode */
            gtl_stGlobalInfo.nUserOptions &= ~GTL_USEROPTION_QUIETON;
    }
    if(ut_ReadFromIniFile(fpConfFile, "LoadOptions", "-gtl_debugon", szLoadOption, GTC_MAX_PATH, &nLastLine, 0) == UT_ERR_OKAY)
    {
        if(!strcmp(szLoadOption, "1"))
            /* Enable debug messages              			          */
            gtl_nDebugMsg_ON = 1;
        else
            /* Disable debug messages              			          */
            gtl_nDebugMsg_ON = 0;
    }
#if 0 // Deprecated since GTL V3.6
    if(ut_ReadFromIniFile(fpConfFile, "LoadOptions", "-gtl_traceon", szLoadOption, GTC_MAX_PATH, &nLastLine, 0) == UT_ERR_OKAY)
    {
        if(!strcmp(szLoadOption, "1"))
            /* Enable trace file              			          */
            gtl_nTraceMsg_ON = 1;
        else
            /* Disable trace file              			          */
            gtl_nTraceMsg_ON = 0;
    }
#endif
#if 0
    /* Output folder specified in gtl_tester.conf. The problem with this is that by teh time this file is read,  */
    /* GTL may have already written some logs. Solution is to raise a flag when the gtl_tester.conf file is read, */
    /* and in each exported GTL function, at the very beginning of the function, check this flag and read the file */
    /* if not already read */
    if(ut_ReadFromIniFile(fpConfFile, "LoadOptions", "-gtl_output_folder", szLoadOption, GTC_MAX_PATH, &nLastLine, 0) == UT_ERR_OKAY)
    {
        if(strcmp(szLoadOption,"") != 0)
            gtl_set(GTL_KEY_OUTPUT_FOLDER, szLoadOption);
    }
#endif
    fclose(fpConfFile);

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : set GTL state to GTL_STATE_NOT_INITIALIZED.                           */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_SetState_NotInitialized()
{
    gtl_msg_Info(1, 1, GTL_MSGINFO_SETSTATE_NOT_INITIALIZED, "");
    gtl_stGlobalInfo.nCurrentLibState = GTL_STATE_NOT_INITIALIZED;
    gtl_stGlobalInfo.mOutputFolder[0] = '\0';
    gtl_stGlobalInfo.mLogFileName[0] = '\0';
    gtl_stGlobalInfo.mSocketTraceFileName[0] = '\0';
    gtl_stGlobalInfo.mDataFileName[0] = '\0';
    gtl_stGlobalInfo.mOutputFileName[0] = '\0';
    gtl_stGlobalInfo.mEventIndex=0;
    if (gtl_stGlobalInfo.mRecipe)
    {
        free(gtl_stGlobalInfo.mRecipe);
        gtl_stGlobalInfo.mRecipe=0;
    }

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : set GTL state to GTL_STATE_ENABLED.                                   */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_SetState_Enabled()
{
    gtl_msg_Info(1, 1, GTL_MSGINFO_SETSTATE_ENABLED, "");
    gtl_stGlobalInfo.nCurrentLibState = GTL_STATE_ENABLED;
    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : set GTL state to GTL_STATE_OFFLINE.                                   */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_SetState_Offline()
{
    gtl_msg_Info(1, 1, GTL_MSGINFO_SETSTATE_OFFLINE, "");
    gtl_stGlobalInfo.nCurrentLibState = GTL_STATE_OFFLINE;
    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : set GTL state to GTL_STATE_DISABLED.									*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
int gtl_SetState_Disabled(nRootcauseCode)
int nRootcauseCode;
{
    GTL_LOG(4, "gtl_SetState_Disabled: rootcause: %d", nRootcauseCode);
	/* Set rootcause */
	gtl_stGlobalInfo.nDisabledRootcause = nRootcauseCode;
	gtl_GetRootcauseString(nRootcauseCode, gtl_stGlobalInfo.szDisabledRootcause);

	gtl_msg_Info(1, 1, GTL_MSGINFO_SETSTATE_DISABLED, "");
    gtl_stGlobalInfo.nCurrentLibState = GTL_STATE_DISABLED;
	gtl_nDisabled = 1;
    return GTL_CORE_ERR_OKAY;
}


/*======================================================================================*/
/* Description  : Re-start Baseline									.					*/
/*                                                                                      */
/* Argument(s)  : lSiteNb = site for which the baseline has to be restarted             */
/*                                                                                      */
/* Return Codes : GTL_CORE_ERR_OKAY.													*/
/*======================================================================================*/
void gtl_Restart_Baseline(lSiteNb)
const int lSiteNb;
{
    GTL_LOG(5, "gtl_Restart_Baseline for site %d", lSiteNb);

    char szInfo[10]="";
    sprintf(szInfo, "%d", lSiteNb);

	/* Baseline restart message */
    gtl_msg_Info(1, 1, GTL_MSGINFO_RESTART_BASELINE, szInfo);

    char lM[255]=""; sprintf(lM, "for site %d from part index %d", lSiteNb, gtl_stGlobalInfo.uiPartIndex);
    int lR=gtl_OutputNewEvent("RESTART_BASELINE", szInfo, lM);
    if (lR!=0)
        GTL_LOG(3, "OutputNewEvent failed", 0);

    /* case 7260: no specific action required, no DPAT limits received */
#if 0
	/* Check if we need to send results to GTM */
	if(gtl_stGlobalInfo.uiRunIndex_Rolling > 0)
	{
		/* gtl_svr_Results(1); */
		gtl_tl_ResetTestResult();
		gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
	}
	/* Reset some local variables */
	gtl_stGlobalInfo.uiRunIndex_Lot = 0;

	/* Reset testlist */
    gtl_tl_ResetSiteTestlist(lSiteNb);
    /* Set state to enabled */
    /*gtl_SetState_Enabled();*/
#endif
}

/*======================================================================*/
/* Display GTL keys     		                                        */
/*                                                                      */
/* Return Codes : 1                                                     */
/*======================================================================*/
int gtl_DisplayKeys()
{
    return 1;
}

/*======================================================================*/
/* Display library status		                                        */
/*                                                                      */
/* Return Codes : 1                                                     */
/*======================================================================*/
int gtl_DisplayStatus()
{
    char szTemp[250]="";
    char szTempBuffer[GTC_MAX_STRING]="";
	unsigned int	uiIndex;

	/* Print Header */
	sprintf(szTempBuffer, "**** GTL STATUS ****************************************************************");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer, "---- GLOBAL");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"gtl_szApplicationFullName     : %s", gtl_szApplicationFullName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer,		"---- GTMSERVER");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	sprintf(szTempBuffer,		"szServerName                  : %s", gtl_stGTMServer.szServerName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szServerIP                    : %s", gtl_stGTMServer.szServerIP);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"ulServerIP                    : %ld", gtl_stGTMServer.ulServerIP);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiServerPort                  : %d", gtl_stGTMServer.uiServerPort);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer,		"---- STATION");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	sprintf(szTempBuffer,		"nStationInfoValid             : %d", gtl_stStation.nStationInfoValid);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiStationNb                   : %d", gtl_stStation.uiStationNb);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiNbSites                     : %d", gtl_stStation.uiNbSites);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiFirstSiteNb                 : %d", gtl_stStation.uiFirstSiteNb);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiLastSiteNb                  : %d", gtl_stStation.uiLastSiteNb);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer, "pnSiteNumbers[]               :");
	for(uiIndex=0; (uiIndex < 256) && (gtl_stStation.pnSiteNumbers[uiIndex] != -1); uiIndex ++)
	{
		sprintf(szTemp, " %d:%d", uiIndex, gtl_stStation.pnSiteNumbers[uiIndex]);
		strcat(szTempBuffer, szTemp);
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"nLoadbordID                   : %d", gtl_stStation.nLoadbordID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"nHandlerProberID              : %d", gtl_stStation.nHandlerProberID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szHostID                      : %s", gtl_stStation.szHostID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szNodeName                    : %s", gtl_stStation.szNodeName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"szTesterExecType              : %s", gtl_stStation.szTesterExecType);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"szTesterExecVersion           : %s", gtl_stStation.szTesterExecVersion);
    gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"szTesterType                  : %s", gtl_stStation.szTesterType);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szUserName                    : %s", gtl_stStation.szUserName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szTestJobName                 : %s", gtl_stStation.szTestJobName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szTestJobFile                 : %s", gtl_stStation.szTestJobFile);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szTestJobPath                 : %s", gtl_stStation.szTestJobPath);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szTestSourceFilesPath         : %s", gtl_stStation.szTestSourceFilesPath);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer,		"---- GLOBALINFO");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	sprintf(szTempBuffer,		"uiFindTestKey                 : %d", gtl_stGlobalInfo.uiFindTestKey);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	switch(gtl_stGlobalInfo.uiFindTestKey)
	{
		case GTC_FINDTEST_TESTNUMBER:
			sprintf(szTempBuffer, "                              : GTC_FINDTEST_TESTNUMBER");
			break;
		case GTC_FINDTEST_TESTNAME:
			sprintf(szTempBuffer, "                              : GTC_FINDTEST_TESTNAME");
			break;
		case GTC_FINDTEST_TESTNUMBERANDNAME:
			sprintf(szTempBuffer, "                              : GTC_FINDTEST_TESTNUMBERANDNAME");
			break;
		default:
			sprintf(szTempBuffer, "                              : <unknown>");
			break;
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiResultFrequency             : %d", gtl_stGlobalInfo.uiResultFrequency);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiNbRunsBeforeProdInfo        : %d", gtl_stGlobalInfo.uiNbRunsBeforeProdInfo);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiNbTests                     : %d", gtl_stGlobalInfo.uiNbTests);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiRunIndex_Rolling            : %d", gtl_stGlobalInfo.uiRunIndex_Rolling);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiRunIndex_Lot                : %d", gtl_stGlobalInfo.uiRunIndex_Lot);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer, "pnPatBinning[]                :");
	for(uiIndex = gtl_stStation.uiFirstSiteNb; uiIndex <= gtl_stStation.uiLastSiteNb; uiIndex ++)
	{
		if(gtl_stGlobalInfo.pnPatBinning[uiIndex] != -1)
		{
			sprintf(szTemp, " %d:%d", uiIndex, gtl_stGlobalInfo.pnPatBinning[uiIndex]);
			strcat(szTempBuffer, szTemp);
		}
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"nGtlModules                   : %d", gtl_stGlobalInfo.nGtlModules);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"                              :");
	if(gtl_stGlobalInfo.nGtlModules == 0)
		strcat(szTempBuffer, " <none>");
	else
	{
		if(gtl_stGlobalInfo.nGtlModules & GTC_GTLMODULE_PAT)
			strcat(szTempBuffer, " GTC_GTLMODULE_PAT");
		if(gtl_stGlobalInfo.nGtlModules & GTC_GTLMODULE_SKIPTEST)
			strcat(szTempBuffer, " GTC_GTLMODULE_SKIPTEST");
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"nCurrentLibState                 : %d", gtl_stGlobalInfo.nCurrentLibState);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    if(gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_NOT_INITIALIZED)
        sprintf(szTempBuffer, "                              : GTL_STATE_NOT_INITIALIZED");
    else if(gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_ENABLED)
        sprintf(szTempBuffer, "                              : GTL_STATE_ENABLED");
    else if(gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_DISABLED)
        sprintf(szTempBuffer, "                              : GTL_STATE_DISABLED");
    else if(gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_OFFLINE)
        sprintf(szTempBuffer, "                              : GTL_STATE_OFFLINE");
	else
        sprintf(szTempBuffer, "                              : UNKNOWN");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer, "pnGoodBins[]                  :");
    for(uiIndex=0; (uiIndex < 256) && (gtl_stGlobalInfo.pnGoodSoftBins[uiIndex] != -1); uiIndex ++)
	{
        sprintf(szTemp, " %d", gtl_stGlobalInfo.pnGoodSoftBins[uiIndex]);
		strcat(szTempBuffer, szTemp);
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"nDisabledRootcause            : %d", gtl_stGlobalInfo.nDisabledRootcause);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szDisabledRootcause           : %s", gtl_stGlobalInfo.szDisabledRootcause);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"nUserOptions                  : %d", gtl_stGlobalInfo.nUserOptions);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"                              :");
	if(gtl_stGlobalInfo.nUserOptions == 0)
		strcat(szTempBuffer, " <none>");
	else
	{
		if(gtl_stGlobalInfo.nUserOptions & GTL_USEROPTION_QUIETON)
			strcat(szTempBuffer, " GTL_USEROPTION_QUIETON");
		if(gtl_stGlobalInfo.nUserOptions & GTL_USEROPTION_LOADOPT_DISABLED)
			strcat(szTempBuffer, " GTL_USEROPTION_LOADOPT_DISABLED");
	}
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer,		"---- PRODINFO");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	sprintf(szTempBuffer,		"szOperatorName                : %s", gtl_stProdInfo.szOperatorName);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szJobRevision                 : %s", gtl_stProdInfo.szJobRevision);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szLotID                       : %s", gtl_stProdInfo.szLotID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szSublotID                    : %s", gtl_stProdInfo.szSublotID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"szProductID                   : %s", gtl_stProdInfo.szProductID);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"SplitlotID                    : %d", gtl_stProdInfo.mSplitlotID);
    gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"RetestIndex                   : %d", gtl_stProdInfo.mRetestIndex);
    gtl_msg_DebugMacro(1, 1, szTempBuffer);
    sprintf(szTempBuffer,		"RetestHbins                   : %s", gtl_stProdInfo.mRetestHbins);
    gtl_msg_DebugMacro(1, 1, szTempBuffer);

    sprintf(szTempBuffer,		"---- PATSETTINGS");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	sprintf(szTempBuffer,		"uiBaseline                    : %d", gtl_stPatSettings.uiBaseline);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer,		"uiFlags                       : %d", gtl_stPatSettings.uiFlags);
	gtl_msg_DebugMacro(1, 1, szTempBuffer);
	sprintf(szTempBuffer, "********************************************************************************");
	gtl_msg_DebugMacro(1, 1, szTempBuffer);

	return 1;
}

/*======================================================================*/
/* Display info about GTL state and version								*/
/*                                                                      */
/* Return Codes : none													*/
/*======================================================================*/
void gtl_DisplayInfo()
{
	gtl_msg_Info(1, 1, GTL_MSGINFO_VERSION, "");
	gtl_msg_Info(1, 1, GTL_MSGINFO_STATE, "");
}

/*======================================================================*/
/* Return a code specifying why GTL is Disabled based on an	error code	*/
/*                                                                      */
/* Return Codes : Disabled rootcause code								*/
/*======================================================================*/
int gtl_GetDisabledRootcauseCode(nErrCode)
int nErrCode;
{
	switch(nErrCode)
	{
        case GTL_CORE_ERR_CONF:
			return GTL_DISABLED_ROOTCAUSE_CONF;
        case GTL_CORE_ERR_CRRF:
            return GTL_DISABLED_ROOTCAUSE_CRRF;
        case GTL_CORE_ERR_GTM_INIT:
			return GTL_DISABLED_ROOTCAUSE_GTM_INIT;
        case GTL_CORE_ERR_GTM_RECIPE:
            return GTL_DISABLED_ROOTCAUSE_GTM_RECIPE;
        case GTL_CORE_ERR_GTM_LICEXPIRED:
			return GTL_DISABLED_ROOTCAUSE_GTM_LICEXPIRED;
        case GTL_CORE_ERR_GTM_NOLIC:
            return GTL_DISABLED_ROOTCAUSE_GTM_NOLIC;
        case GTL_CORE_ERR_GTM_TESTLIST:
			return GTL_DISABLED_ROOTCAUSE_GTM_TESTLIST;
        case GTL_CORE_ERR_GTM_INITPAT:
			return GTL_DISABLED_ROOTCAUSE_GTM_INITPAT;
        case GTL_CORE_ERR_GTM_COMM:
			return GTL_DISABLED_ROOTCAUSE_GTM_COMM;
        case GTL_CORE_ERR_GTM_NOPATMODULE:
            return GTL_DISABLED_ROOTCAUSE_GTM_NOPATMODULE;
        case GTL_CORE_ERR_GTM_UNKNOWN_STATUS:
            return GTL_DISABLED_ROOTCAUSE_GTM_UNKNOWN_STATUS;
        case GTL_CORE_ERR_LOADOPTION:
            return GTL_DISABLED_ROOTCAUSE_LOADOPTION;
        case GTL_CORE_ERR_BAD_PROD_INFO:
            return GTL_DISABLED_ROOTCAUSE_BAD_PRODINFO;
        case GTL_CORE_ERR_INVALID_NUM_OF_SITES:
            return GTL_DISABLED_ROOTCAUSE_INVALID_NUM_OF_SITES;
        case GTL_CORE_ERR_PROCESSMESSAGE:
            return GTL_DISABLED_ROOTCAUSE_PROCESSMESSAGE;

        //case GTL_CORE_ERR_BEGINJOB_NOT_EXEC; : should not provoke a disable
        default:
			return GTL_DISABLED_ROOTCAUSE_UNKNOWN;
	}
}

/*======================================================================*/
/* Returns the Rootcause (GTL Disabled rootcause) string				*/
/*																		*/
/* In : nRootcause = rootcause ID										*/
/*																		*/
/* Out : szRootcause = rootcause string									*/
/*                                                                      */
/* Return Codes : none                                                  */
/*======================================================================*/
void gtl_GetRootcauseString(nRootcause, szRootcause)
int		nRootcause;
char	*szRootcause;
{
	switch(nRootcause)
	{
		case GTL_DISABLED_ROOTCAUSE_UNKNOWN:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_UNKNOWN);
			break;
		case GTL_DISABLED_ROOTCAUSE_INIT:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_INIT);
			break;
		case GTL_DISABLED_ROOTCAUSE_CONF:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_CONF);
			break;
		case GTL_DISABLED_ROOTCAUSE_CGNI:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_CGNI);
			break;
        case GTL_DISABLED_ROOTCAUSE_CRRF:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_CRRF);
            break;
        case GTL_DISABLED_ROOTCAUSE_GTM_INIT:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_INIT);
			break;
        case GTL_DISABLED_ROOTCAUSE_GTM_RECIPE:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_RECIPE);
			break;
		case GTL_DISABLED_ROOTCAUSE_GTM_LICEXPIRED:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_LICEXPIRED);
			break;
        case GTL_DISABLED_ROOTCAUSE_GTM_NOLIC:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_NOLIC);
            break;
        case GTL_DISABLED_ROOTCAUSE_GTM_TESTLIST:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_TESTLIST);
			break;
		case GTL_DISABLED_ROOTCAUSE_GTM_INITPAT:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_INITPAT);
			break;
		case GTL_DISABLED_ROOTCAUSE_GTM_COMM:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_COMM);
			break;
        case GTL_DISABLED_ROOTCAUSE_PROCESSMESSAGE:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_PROCESSMESSAGE);
            break;
        case GTL_DISABLED_ROOTCAUSE_GTM_UNKNOWN_STATUS:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_UNKNOWN_STATUS);
            break;
        case GTL_DISABLED_ROOTCAUSE_LOADOPTION:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_LOADOPTION);
			break;
        case GTL_DISABLED_ROOTCAUSE_GTM_NOPATMODULE:
            strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_GTM_NOPATMODULE);
            break;
        case GTL_DISABLED_ROOTCAUSE_USERCMD:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_USERCMD);
			break;
		default:
			strcpy(szRootcause, GTL_DISABLED_ROOTCAUSE_SZ_UNKNOWN);
			break;
	}
}

/*======================================================================================*/
/* Description  : check for messages from GTM.											*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
int gtl_CheckForMessages()
{
    int	nMessageReceived=0, nStatus=0;

    /* Do we need to wait for Ack messages from GTM? */
    if(gtl_stGlobalInfo.mWaitForGtmAck>0)
        GTL_LOG(6, "gtl_CheckForMessage: waiting for %d Ack messages", gtl_stGlobalInfo.mWaitForGtmAck);

    /* Check for message from GTM */
    /* If in synchronous communication mode, we may have to wait for a Acknowledge */
    do
        nStatus = gtl_svr_CheckForMessage(&nMessageReceived, &gtl_stMessage);
    while((nStatus==GNET_ERR_OKAY) && (nMessageReceived==0) && (gtl_stGlobalInfo.mWaitForGtmAck>0));

    while((nStatus==GNET_ERR_OKAY) && nMessageReceived)
	{
        /* Process message */
        int r=gtl_ProcessMessage(&gtl_stMessage);
        if (r!=GTL_CORE_ERR_OKAY)
        {
            GTL_LOG(3, "gtl_ProcessMessage failed", 0);
            gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_PROCESSMESSAGE, 0, "");
            gtl_SetState_Disabled(GTL_DISABLED_ROOTCAUSE_PROCESSMESSAGE);
            return GTL_CORE_ERR_PROCESSMESSAGE;
        }

        /* Check if message is an Acknowledge */
        if(gtl_stMessage.mType == GNM_TYPE_ACK)
            (gtl_stGlobalInfo.mWaitForGtmAck)--;

        /* Receive additional messages if any */
        do
            nStatus = gtl_svr_CheckForMessage(&nMessageReceived, &gtl_stMessage);
        while((nStatus==GNET_ERR_OKAY) && (nMessageReceived==0) && (gtl_stGlobalInfo.mWaitForGtmAck>0));
	}

    /* 7266: we should have received all Ack, except if some error occured (ie connection error). */
    /* Make sure we reset mWaitForGtmAck */
    gtl_stGlobalInfo.mWaitForGtmAck = 0;

	/* Check if error during message reception */
    if(nStatus!=GNET_ERR_OKAY)
	{
        GTL_LOG(4, "gtl_CheckForMessage: error receiving message (status=%d)", nStatus);
        gtl_err_Display(GTL_MODULE_MAIN, GTL_CORE_ERR_GTM_COMM, 0, "");
        GTL_LOG(5, "Switching to OFFLINE mode", 0);
        gtl_SetState_Offline();
        return GTL_CORE_ERR_GTM_COMM;
	}

    return GTL_CORE_ERR_OKAY;
}

int gtl_IsInitialized()
{
    return (gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_NOT_INITIALIZED ? 0:1);
}

int gtl_IsDisabled()
{
    return (gtl_stGlobalInfo.nCurrentLibState == GTL_STATE_DISABLED ? 1:0);
}
