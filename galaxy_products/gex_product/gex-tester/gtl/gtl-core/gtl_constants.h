/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_constants.h                                                              */
/*                                                                                      */
/* This file has global definitions used throughout all GTL modules.                    */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 18 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTL_CONSTANTS_H_
#define _GTL_CONSTANTS_H_

#include <gtl_core.h>
#include <gtc_constants.h>
#include <gstdl_ringbuffer_c.h>
#include <time.h>

#if defined(__CYGWIN__) && !defined(__MINGW32__)
    #include <sys/time.h>
#endif

#ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif

/*======================================================================================*/
/* Constants                                                                            */
/*======================================================================================*/
/* Definitions for nUserOptions */
#define GTL_USEROPTION_UNUSED_1			0x000001	/* Unused, available */
#define GTL_USEROPTION_QUIETON			0x000004	/* Quiet mode (no messages) */
#define GTL_USEROPTION_LOADOPT_DISABLED	0x000008	/* Load option = -gtl_disabled */

/* EXTENSIONS */
#define GTL_TRACE_EXT				".trace"		/* extension for GTL trace files */
#define GTL_LOG_EXT					".log"			/* extension for GTL log files */
#define GTL_SEMF_EXT				"sem"			/* extension for GTL semf files */

/* Events */
#define GTL_EVENT_NEW_SPAT_LIMITS   0x01
#define GTL_EVENT_NEW_DPAT_LIMITS   0x02

/*======================================================================================*/
/* Type Definitions                                                                     */
/*======================================================================================*/
/* GTL server configuration (for GTM connection) */
struct tagGTL_Server
{
    /* Server name */
    char szServerName[GTC_MAX_SRVNAME_LEN];
    /* Server IP address as a string */
    char szServerIP[GTC_MAX_IPADDR_LEN];
    /* Server IP address as a number */
    unsigned long ulServerIP;
    /* Server Port number */
    unsigned int uiServerPort;
    /* Ptr to next server (NULL if just 1 server allowed) */
    struct tagGTL_Server* pstNextServer;
};
typedef struct tagGTL_Server GTL_Server;
typedef struct tagGTL_Server *PT_GTL_Server;

struct tagGTL_Message
{
    int mSeverity; // <0:critical, 0:warning, >0:notice
    int mMessageID; // Example: 1=Too many outliers in baseline, 2=Distrib changed, 3=...
    char mMessage[GTL_MESSAGE_STRING_SIZE];
};
typedef struct tagGTL_Message GTL_Message;

typedef enum { GTL_SOCK_BLOCKING, GTL_SOCK_NON_BLOCKING } gtlSockMode;

/* GTL global information */
struct tagGTL_GlobalInfo
{
    /* Option specifying the key to use to find a test in the TestList */
    unsigned int uiFindTestKey;
    /* Frequency at which GTL should send results */
    unsigned int uiResultFrequency;
    /* GTL should send Production Info structure after this nb of runs */
    unsigned int uiNbRunsBeforeProdInfo;
    /* Nb of tests (from TestList sent by GTM) */
    unsigned int uiNbTests;
    /* Current rolling run index (reset on uiResultFrequency basis) */
    unsigned int uiRunIndex_Rolling;
    /* Current total run index for current lot/sublot (reset on new lot/sublot) */
    unsigned int uiRunIndex_Lot;
    /* Current part index for current lot/sublot (reset on new lot/sublot) */
    unsigned int uiPartIndex;
    /* Number of good parts for the current splitlot : exclude failed bin and any PAT (S or D). */
    /* Updated through gtl_binning() or gtl_set_binning() */
    unsigned int mNbPartsGood;
    // PAT Binning calculated on current run, per site (-1 if no test failed PAT limits)
    int	pnPatBinning[256];
    // Gtl modules to activate (PAT, SKIPTEST...)
    int	nGtlModules;
    // Current state of GTL
    int	nCurrentLibState;

    // Array of 256 with list of good soft bins (-1 = end of the list)
    int	pnGoodSoftBins[256];
    // Todo : Array of 256 with list of good hard bins (-1 = end of the list)
    int	pnGoodHardBins[256]; // Bernard : any way to have that table also ?

    /* Rootcause code if GTL is Disabled */
    int	nDisabledRootcause;
    /* Rootcause string if GTL is Disabled */
    char szDisabledRootcause[GTC_MAX_ROOTCAUSE_LEN];
    /* Several options */
    int	nUserOptions;
    /* Messages (critical, warning or notice) stack */
    RingBuffer mMessages;
    // Aux file
    char mAuxFileName[GTC_MAX_PATH];
    /* Output folder for all files generated by GTL (log, traceability...) */
    char mOutputFolder[GTC_MAX_PATH];
    //! \brief Temporary folder to be used by GTL
    char mTempFolder[GTC_MAX_PATH];
    /* Name of current log file */
    char mLogFileName[GTC_MAX_PATH];
    /* Name of current socket trace file */
    char mSocketTraceFileName[GTC_MAX_PATH];
    /* Name of file where the tester generates results data */
    char mDataFileName[GTC_MAX_PATH];

    // Traceability switch : on, off,...
    // 7501 : "traceability" i.e. SQLite output is now no more an option but a must have.
    //char mTraceabilityMode[GTC_MAX_PATH];

    // Name to use for the GTL output file (traceability,...)
    char mOutputFileName[GTC_MAX_PATH];
    //! \brief File path and name of the SQL output
    char mQueryOutputFileName[GTC_MAX_PATH];

    // Path and name of the config file
    char mConfigFileName[GTC_MAX_PATH];
    // GTL->GTM communication mode.
    enum { GTL_GTMCOMM_SYNCHRONOUS, GTL_GTMCOMM_ASYNCHRONOUS } mGtmCommunicationMode;
    // When using synchronous communication, set to 1 if gtl_beginjob() has to wait for Acknowledge reception
    unsigned short mWaitForGtmAck;
    // Index incremented at each new event in order to get an EventID
    unsigned int mEventIndex;
    // Set socket trace file ON/OFF
    unsigned short mSocketTrace;
    // Reconnection mode
    enum { GTL_RECONNECT_ON, GTL_RECONNECT_OFF } mReconnectionMode;
    // The recipe path and file name. Needed for gtl_open.
    char mRecipeFileName[GTC_MAX_PATH];
    // Recipe content (raw, as read from the file). Malloc in gtl_svr_Init. NULL if not yet read/deleted.
    char* mRecipe;
    // GTL socket connection mode
    gtlSockMode mSocketConnetionMode;
    // GTL socket communication mode
    gtlSockMode mSocketCommunicationMode;
    // GTL socket connection timeout
    unsigned int mSocketConnectionTimeout;
    // GTL socket receive timeout
    unsigned int mSocketReceiveTimeout;
    // GTL socket send timeout
    unsigned int mSocketSendTimeout;
    //! \brief Desired limits: for reload, or get : last, tighest,...
    char mDesiredLimits[GTC_MAX_STRING];
    //! \brief For interface purpose: for example for gtl_get("site_state"). Set to -1 on startup.
    int mDesiredSiteNb;
    //! \brief Query cache used by all SQL functions and overwritten by gtl_set("query"). Will be init asap.
    char mQueryBuffer[GTC_MAX_QUERY_LEN];
    //
    int mCurrentMessageID;
    int mCurrentMessageSeverity;
    char mCurrentMessage[GTL_MESSAGE_STRING_SIZE];
    // Fields to match
    char mFieldsToMatch[GTC_MAX_STRING];
    // Reload limits
    enum { GTL_RELOAD_LIMITS_ON, GTL_RELOAD_LIMITS_OFF } mReloadLimits;
    //! \brief Limits get condition: "for reload, or get : last, tighest,..."always", "if_changed"
    char mLimitsGetCondition[GTC_MAX_STRING];
};
typedef struct tagGTL_GlobalInfo GTL_GlobalInfo;
typedef struct tagGTL_GlobalInfo *PT_GTL_GlobalInfo;

/* Tester Station information */
struct tagGTL_Station
{
    int	nStationInfoValid;
    unsigned int uiStationNb;
    unsigned int uiNbSites;
    unsigned int uiFirstSiteNb;
    unsigned int uiLastSiteNb;
    int	pnSiteNumbers[256];
    int	nLoadbordID;
    int	nHandlerProberID;
    char szHostID[GTC_MAX_HOSTID_LEN];
    char szNodeName[GTC_MAX_NODENAME_LEN];
    char szTesterExecType[GTC_MAX_TESTEREXEC_LEN];
    char szTesterExecVersion[GTC_MAX_TESTEREXEC_LEN];
    char szTesterType[GTC_MAX_TESTERTYPE_LEN];
    char szUserName[GTC_MAX_USERNAME_LEN];
    char szTestJobName[GTC_MAX_JOBNAME_LEN];
    char szTestJobFile[GTC_MAX_PATH];
    char szTestJobPath[GTC_MAX_PATH];
    char szTestSourceFilesPath[GTC_MAX_PATH];
};
typedef struct tagGTL_Station GTL_Station;
typedef struct tagGTL_Station *PT_GTL_Station;

/* Production information */
struct tagGTL_ProdInfo
{
    char mFacilityID[GTC_MAX_STRING];
    char mFamilyID[GTC_MAX_STRING];
    char mFlowID[GTC_MAX_STRING];
    char mFloorID[GTC_MAX_STRING];
    char mRomCode[GTC_MAX_STRING];
    char szOperatorName[GTC_MAX_OPERATORNAME_LEN];
    char szJobRevision[GTC_MAX_JOBREV_LEN];
    char szLotID[GTC_MAX_LOTID_LEN];
    char szSublotID[GTC_MAX_SLOTID_LEN];
    char szProductID[GTC_MAX_PRODUCTID_LEN];
    int mSplitlotID; // controlled by GTL. -1 means invalid.
    int mRetestIndex; // controlled by GTL. -1 means invalid.
    char mRetestHbins[GTC_MAX_STRING]; // empty default
    //char set;    // set or not : true ('y') if set prod info called ?
    char mModeCode;
    char mRetestCode;
    char mSpecName[GTC_MAX_STRING];
    char mSpecVersion[GTC_MAX_STRING];
    char mSetupID[GTC_MAX_STRING];
    char mTestCode[GTC_MAX_STRING];
    char mTemperature[GTC_MAX_STRING];
    char mUserText[GTC_MAX_STRING];
};
typedef struct tagGTL_ProdInfo GTL_ProdInfo;
typedef struct tagGTL_ProdInfo *PT_GTL_ProdInfo;


/* PAT settings */
struct tagGTL_PatSettings
{
    unsigned int uiBaseline;
    unsigned int uiFlags;
};
typedef struct tagGTL_PatSettings GTL_PatSettings;
typedef struct tagGTL_PatSettings *PT_GTL_PatSettings;

/******************************************************************************!
 * \struct KeyFieldList
 * \brief For gtl_is_same_context function
 ******************************************************************************/
struct KeyFieldList
{
    enum { KEY_FIELD_EMPTY, KEY_FIELD_STR, KEY_FIELD_NUM } type;
    char* str;
    int num;
    struct KeyFieldList* next;
};

#endif /* #ifndef _GTL_CONSTANTS_H_ */
