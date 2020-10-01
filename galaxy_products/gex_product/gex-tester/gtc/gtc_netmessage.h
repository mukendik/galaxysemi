/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtc_netmessage.h                                                             */
/* GTC : Galaxy Tester Common                                                           */
/* Write structures used to communicate between GTL (Galaxy Tester Library) and GTM     */
/* (Galaxy Tester Monitor) to a buffer (in network byte order) for transfer through     */
/* a socket.                                                                            */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 17 May 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTC_NETMESSAGE_H_
#define _GTC_NETMESSAGE_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <gstdl_neterror_c.h>    /* For error codes */
#include <gstdl_netbuffer_c.h>

#include "gtc_constants.h"

/*======================================================================================*/
/* Message Header Type Definition														*/
/*======================================================================================*/
#define GNM_TYPE_BEGIN_NUMBER           0x01	/* Must alays be the first define value.        */
#define	GNM_TYPE_Q_CFC                  0x01	/* 01: Query: Check For connection.				*/
#define	GNM_TYPE_R_CFC                  0x02	/* 02: Response: Check For Connection.			*/
#define	GNM_TYPE_Q_INIT                 0x03	/* 03: Query: Initialization.					*/
#define	GNM_TYPE_R_INIT                 0x04	/* 04: Response: Initialization.				*/
#define	GNM_TYPE_PRODINFO               0x05	/* 05: Production Info.							*/
#define	GNM_TYPE_Q_TESTLIST             0x06	/* 06: Query: Get TestList from GTM.			*/
#define	GNM_TYPE_R_TESTLIST             0x07	/* 07: TestList.								*/
#define	GNM_TYPE_Q_PATCONFIG_STATIC     0x08	/* 08: Query: Get Static PAT config from GTM.	*/
#define	GNM_TYPE_R_PATCONFIG_STATIC     0x09	/* 09: Statis PAT config.						*/
#define GNM_TYPE_Q_ENDLOT               0x0a	// 10: End of lot Q
#define GNM_TYPE_R_ENDLOT               0x0b	// 11: End of lot R
#define	GNM_TYPE_PATCONFIG_DYNAMIC      0x0c	/* 12: Dynamic PAT config.						*/
#define	GNM_TYPE_RESULTS                0x0d	/* 13: Send run results (tests + bin).			*/
#define GNM_TYPE_NOTIFY                 0x0e	/* 14: Notify tester with event (used in OnNotifyTester())	*/
#define GNM_TYPE_COMMAND                0x0f	/* 15: Send command to tester					*/
#define GNM_TYPE_WRITETOSTDF            0x10	/* 16: Ask tester to write string to STDF file	*/
#define GNM_TYPE_HEARTBEAT              0x11	/* 17: HeartBeat	(unused in GTL, not implemented in GTM)	*/
#define	GNM_TYPE_PATCONFIG_DYNAMIC_2	0x12	/* 18: Dynamic PAT config.						*/
#define	GNM_TYPE_ACK                    0x13 // 19: Acknowledge. When activated, only used by Results  GNM_TYPE_RESULTS and type prodinfo
#define GNM_TYPE_Q_END_OF_SPLITLOT      0x14 // 20: End of splitlot Q
#define GNM_TYPE_R_END_OF_SPLITLOT      0x15 // 21: End of splitlot R
#define	GNM_TYPE_Q_PATCONFIG_DYNAMIC    0x16 // 22: Query: Get Dynamic PAT config from GTM.

#define GNM_TYPE_END_NUMBER             0x17 // Must always be the last define value

/*======================================================================================*/
/* Error Codes																			*/
/*======================================================================================*/
/* See gneterror.h */

/*======================================================================================*/
/* String Max SIZES                        . 											*/
/*======================================================================================*/
/* Deprecated ?
    #define	GNM_HOSTNAME_SIZE           64	// UNIX : See MAXHOSTNAMELEN in <sys/param.h>
    #define GNM_USER_SIZE               64
    #define GNM_PROGNAME_SIZE           256
    #define GNM_PROJNAME_SIZE			256
    #define GNM_VERSIONNAME_SIZE		256
    #define	GNM_SESSIONNAME_SIZE        256
    #define GNM_MESSAGE_SIZE            256
    #define GNM_TESTERNAME_SIZE         256
    #define GNM_HOSTID_SIZE             16
    #define GNM_TESTERTYPE_SIZE         32
    #define GNM_USERLOGIN_SIZE          256
    #define GNM_STATION_SIZE            256
    #define GNM_TIME_SIZE               64
    #define GNM_PRODUCTID_SIZE			32
    #define GNM_USERNAME_SIZE			128
*/

/*======================================================================================*/
/* PAT flags.																			*/
/*======================================================================================*/
/* GNM_TYPE_ACK : Acknowledge */
typedef struct tagACK
{
    unsigned int                mTimeStamp;
    int                         mStatus;
} GNM_ACK, *PT_GNM_ACK;

/* GNM_TYPE_Q_CFC : Check For Connection */
typedef struct tagQ_CFC
{
    unsigned int mTimeStamp;
    unsigned int mGtlVersionMajor;
    unsigned int mGtlVersionMinor;
} GNM_Q_CFC, *PT_GNM_Q_CFC;

/* GNM_TYPE_R_CFC : Check For Connection */
typedef struct tagR_CFC
{
    unsigned int       mTimeStamp;
    int                mStatus;
} GNM_R_CFC, *PT_GNM_R_CFC;

/* GNM_TYPE_Q_INIT : Initialization */
typedef struct tagQ_INIT
{
    unsigned int			mStationNb;
    unsigned int			mNbSites;
    int						mSiteNumbers[256];
    int                     mSitesStates[256];
    char					mGtlVersion[GTC_MAX_VERSION_LEN];
    char					mHostID[GTC_MAX_HOSTID_LEN];
    char					mNodeName[GTC_MAX_NODENAME_LEN];
    char					mUserName[GTC_MAX_USERNAME_LEN];
    char					mTesterExec[GTC_MAX_TESTEREXEC_LEN];
    char					mTesterType[GTC_MAX_TESTERTYPE_LEN];
    char					mTestJobName[GTC_MAX_JOBNAME_LEN];
    char					mTestJobFile[GTC_MAX_PATH];
    char					mTestJobPath[GTC_MAX_PATH];
    char					mTestSourceFilesPath[GTC_MAX_PATH];
    char                    *mRecipeBuffer;
} GNM_Q_INIT, *PT_GNM_Q_INIT;

/* GNM_TYPE_R_INIT : Initialization */
typedef struct tagR_INIT
{
    int						nStatus;				/* Status: OK, Couldn't read config file... */
    int						nGtlModules;			/* GTL modules to activate: PAT, SKIPTEST... */
    unsigned int			uiFindTestKey;			/* Option specifying the key to use to find a test in the TestList */
    unsigned int			uiResultFrequency;		/* GTL should send run results every N runs (not parts, equivalent if single site, but not if multi-site) */
    unsigned int			uiNbRunsBeforeProdInfo;	/* GTL should send Production info structure after N runs (not parts, equivalent if single site, but not if multi-site) */
    unsigned int			uiNbGoodSBins;			/* Nb of good bins in the pnGoodBins array */
    int pnGoodSoftBins[256]; // Good bin numbers
    unsigned int			uiNbGoodHBins;			/* Nb of good Hbins in the pnGoodHardBins array */
    int pnGoodHardBins[256]; // Good bin numbers : todo
    char                    mMessage[GTC_MAX_MESSAGE_LEN]; /* Message returned by GTM (could be empty) */
} GNM_R_INIT,*PT_GNM_R_INIT;

/* GNM_TYPE_PRODINFO : Production info */
typedef struct tagPRODINFO
{
    char szOperatorName[GTC_MAX_OPERATORNAME_LEN];
    char szJobRevision[GTC_MAX_JOBREV_LEN];
    char szLotID[GTC_MAX_LOTID_LEN];
    char szSublotID[GTC_MAX_SLOTID_LEN];
    char szProductID[GTC_MAX_PRODUCTID_LEN];
    int mSplitlotID; // controlled by GTL. -1 means invalid.
    int mRetestIndex; // controlled by GTL. -1 means invalid.
} GNM_PRODINFO, *PT_GNM_PRODINFO;

/* GNM_TYPE_Q_TESTLIST : query TESTLIST GTM */
typedef struct tagQ_TESTLIST
{
	unsigned int				uiTimeStamp;
} GNM_Q_TESTLIST, *PT_GNM_Q_TESTLIST;

/* GNM_TYPE_TESTDEF : test definition (test name/number, limits...) */
typedef struct tagTESTDEF
{
    long		 lTestNumber;
    char		 szTestName[GTC_MAX_TESTNAME_LEN];
    char		 szTestUnits[GTC_MAX_TESTUNITS_LEN];
    float		 fLowLimit;
    float		 fHighLimit;
    unsigned int uiTestFlags;
    //! \brief Position/index in the table (sometimes called RtnIndex). Starting from 0,1,2... It is NOT the PinName.
    //! \brief -1 for PTR tests, else a number >= 0.
    int          mPinIndex;
} GNM_TESTDEF, *PT_GNM_TESTDEF;

/* GNM_TYPE_R_TESTLIST : TESTLIST */
typedef struct tagR_TESTLIST
{
    unsigned int			uiNbTests; // Number of tests in the TestDefs (i.e. only test under PAT)
    PT_GNM_TESTDEF			pstTestDefinitions;
} GNM_R_TESTLIST, *PT_GNM_R_TESTLIST;

/* GNM_TYPE_Q_PATCONFIG_STATIC : query static PAT config from GTM (including static PAT limits) */
typedef struct tagQ_PATCONFIG_STATIC
{
	unsigned int				uiTimeStamp;
} GNM_Q_PATCONFIG_STATIC, *PT_GNM_Q_PATCONFIG_STATIC;

/* GNM_TYPE_TESTDEF_STATS : stats for 1 test  */
typedef struct tagTESTDEF_STATS
{
    char            mDistributionShape[GTC_MAX_DISTRIBUTIONSHAPE_LEN];
    float           mN_Factor;
    float           mT_Factor;
    float           mMean;
    float           mSigma;
    float           mMin;
    float           mQ1;
    float           mMedian;
    float           mQ3;
    float           mMax;
    unsigned int    mExecCount;
    unsigned int    mFailCount;
} GNM_TESTDEF_STATS, *PT_GNM_TESTDEF_STATS;

/* GNM_TYPE_TESTDEF_STATICPAT : static PAT limits for test */
typedef struct tagTESTDEF_STATICPAT
{
    long				mTestNumber;
    char				mTestName[GTC_MAX_TESTNAME_LEN];
    float				mLowLimit1;
    float				mHighLimit1;
    unsigned int		mTestFlags;
    int					mPatBinning;
    GNM_TESTDEF_STATS   mTestStats;
    //! -1 for PTR, else position/index in a table.
    int                 mPinIndex;
} GNM_TESTDEF_STATICPAT, *PT_GNM_TESTDEF_STATICPAT;

/* GNM_TYPE_R_PATCONFIG_STATIC : static PAT configuration */
typedef struct tagR_PATCONFIG_STATIC
{
	unsigned int				uiBaseline;
    unsigned int				uiFlags;
	unsigned int				uiNbSites;
	unsigned int				uiNbTests;
	PT_GNM_TESTDEF_STATICPAT	pstTestDefinitions;
} GNM_R_PATCONFIG_STATIC, *PT_GNM_R_PATCONFIG_STATIC;

/* GNM_TYPE_Q_PATCONFIG_DYNAMIC : query dynamic PAT config from GTM (DPAT settings, DPAT limits if any) */
typedef struct tagQ_PATCONFIG_DYNAMIC
{
    unsigned int				uiTimeStamp;
} GNM_Q_PATCONFIG_DYNAMIC, *PT_GNM_Q_PATCONFIG_DYNAMIC;

/* GNM_TYPE_TESTDEF_DYNAMICPAT : dynamic PAT limits for test */
typedef struct tagTESTDEF_DYNAMICPAT
{
    // Site number: if -1, it concerns all sites
    int                mSiteNb;
    long			   mTestNumber;
    char			   mTestName[GTC_MAX_TESTNAME_LEN];
    float			   mLowLimit1;
    float			   mHighLimit1;
    float			   mLowLimit2;
    float			   mHighLimit2;
    unsigned int	   mTestFlags;
    int				   mPatBinning;
    //! -1 for PTR, else position/index in a table.
    int                mPinIndex;
    GNM_TESTDEF_STATS  mTestStats;
} GNM_TESTDEF_DYNAMICPAT, *PT_GNM_TESTDEF_DYNAMICPAT;

/* GNM_TYPE_PATCONFIG_DYNAMIC : dynamic PAT configuration */
typedef struct tagPATCONFIG_DYNAMIC
{
    unsigned int				uiNbSites;
    unsigned int				uiNbTests;
    PT_GNM_TESTDEF_DYNAMICPAT	pstTestDefinitions;
} GNM_PATCONFIG_DYNAMIC, *PT_GNM_PATCONFIG_DYNAMIC;

/* GNM_TYPE_SITE_DYNAMICPAT */
/* Unused for the moment */
typedef struct tagSITE_DYNAMICPAT
{
    unsigned int				uiSiteNb;
    PT_GNM_TESTDEF_DYNAMICPAT	pstTestDefinitions;
} GNM_SITE_DYNAMICPAT, *PT_GNM_SITE_DYNAMICPAT;

/* GNM_TYPE_PATCONFIG_DYNAMIC_2 : dynamic PAT configuration */
/* This structure is slightly different than GNM_TYPE_PATCONFIG_DYNAMIC */
/* It stores DPAT config per site, with additional SiteNb information */
/* Unused for the moment */
typedef struct tagPATCONFIG_DYNAMIC_2
{
    unsigned int				uiNbSites;
    unsigned int				uiNbTests;
    PT_GNM_SITE_DYNAMICPAT      pstSitesDynamicPatConfig;
} GNM_PATCONFIG_DYNAMIC_2, *PT_GNM_PATCONFIG_DYNAMIC_2;

/* GNM_TYPE_NOTIFY : notify tester of specific event */
typedef struct tagNOTIFY
{
	int							nTimeout;
	char						szMessage[GTC_MAX_TESTERMESSAGE];
} GNM_NOTIFY, *PT_GNM_NOTIFY;

/* GNM_TYPE_COMMAND : send tester a command */
typedef struct tagCOMMAND
{
    unsigned int		uiCommandID;
    char				szCommand[GTC_MAX_TESTERCOMMAND];
} GNM_COMMAND, *PT_GNM_COMMAND;

/* GNM_TYPE_WRITETOSTDF : ask tester library to write a string to the STDF file */
typedef struct tagWRITETOSTDF
{
	int							nRecTyp;
	int							nRecSub;
	char						szString[GTC_MAX_STRINGTOSTDF];
} GNM_WRITETOSTDF, *PT_GNM_WRITETOSTDF;

/* GNM_TYPE_TESTSTAT_RESULTS : stats for 1 test (test name/number, execs) used when sending results from GTL to GTM */
typedef struct tagTESTSTAT_RESULTS
{
    unsigned int	mSiteNb;
    long			lTestNumber;
    char			mTestName[GTC_MAX_TESTNAME_LEN];
    int             mPinIndex;
    unsigned int	uiExecs;
} GNM_TESTSTAT_RESULTS, *PT_GNM_TESTSTAT_RESULTS;

/* GNM_TYPE_RUNRESULT : result for 1 part (binning, PAT binning) */
typedef struct tagRUNRESULT
{
    int					mOrgSoftBin;
    int                 mOrgHardBin;
    int					mPatSoftBin;
    int					mPatHardBin;
    unsigned int		mNbTestsExecuted;
    char                mPartID[GTC_MAX_PARTID_LEN];
    char                mFlags;         // Part flags (part fail...)
    unsigned int        mPartIndex;     // Overall index incremented by GTL each part.
} GNM_RUNRESULT, *PT_GNM_RUNRESULT;

/* GNM_TYPE_TESTRESULT : result for 1 test (value, flags) */
typedef struct tagTESTRESULT
{
    float mValue;
    char  mFlags; // Valid, DPAT, SPAT.
} GNM_TESTRESULT, *PT_GNM_TESTRESULT;

/* GNM_TYPE_RESULTS : send run results (test values + binning) */
typedef struct tagRESULTS
{
    unsigned int			mPacketNb;
    /* 1=No notification (alarm...) in return from GTM, 0=GTM can send notifications in return */
    unsigned int			mSilentMode;
    unsigned int			mNbSites;			/* Nb of sites */
    unsigned int			mNbTests;			/* Nb of tests */
    /* Nb of runs allocated in the structure (not nb of parts. 10 runs, 4 sites = 40 parts) */
    unsigned int			mNbAllocatedRuns;
    /* Nb of runs with valid results (generally the same as uiNbAllocatedRuns, except for the last results of the lot) */
    unsigned int			mNbValidRuns;
    /* Array of test results. Size = NbSites*NbRuns*NbTests. Index = SiteIndex*NbRuns*NbTests + TestIndex*NbRuns + RunIndex */
    PT_GNM_TESTRESULT       mTestResults;
    /* Array of run results. Size = NbSites*NbRuns. Index = SiteIndex*NbRuns + RunIndex */
    PT_GNM_RUNRESULT		mRunResults;
    /* Array of test statistics. Size = NbSites*NbTests. Index = SiteIndex*NbTests + TestIndex */
    PT_GNM_TESTSTAT_RESULTS	mTestStats;
} GNM_RESULTS, *PT_GNM_RESULTS;

/* GNM_TYPE_Q_ENDLOT : End of lot */
typedef struct tagQ_ENDLOT
{
	unsigned int				uiTimeStamp;
} GNM_Q_ENDLOT, *PT_GNM_Q_ENDLOT;

// GNM_TYPE_Q_END_OF_SPLITLOT :
typedef struct tagQ_END_OF_SPLITLOT
{
    unsigned int mTimeStamp;
} GNM_Q_END_OF_SPLITLOT, *PT_GNM_Q_END_OF_SPLITLOT;

/* GNM_TYPE_R_ENDLOT : EndLot reply */
typedef struct tagR_ENDLOT
{
    unsigned int      mTimeStamp;
    int               mStatus;
} GNM_R_ENDLOT, *PT_GNM_R_ENDLOT;

/* GNM_TYPE_R_END_OF_SPLITLOT : reply */
typedef struct tagR_END_OF_SPLITLOT
{
    unsigned int mTimeStamp;
    int mStatus;
} GNM_R_END_OF_SPLITLOT, *PT_GNM_R_END_OF_SPLITLOT;

/* GNM_TYPE_HEARTBEAT : Heartbeat */
typedef struct tagHEARTBEAT
{
    unsigned int mTimeStamp;
} GNM_HEARTBEAT, *PT_GNM_HEARTBEAT;

/*======================================================================================*/
/* Exported Functions.																	*/
/*======================================================================================*/

/*
    GTCNM : GalaxyTester Common Network Message
    gtcnm_CreateBufferData: Create buffer data
     Description  : create buffer from Data structure.
     Argument(s)  :
         unsigned int uiType
             Type of data structure
         PT_GNB_HBUFFER	phBuffer
             Ptr to the buffer that will receive the data.

         void* ptData
             Ptr to the data structure used to fill the buffer.

     Return Codes :
         GNET_ERR_OKAY if successfull; otherwise, return an error	code defined in
         gnetbuffer.h header file
*/

#ifdef _GTC_NETMESSAGE_MODULE_
    int     gtcnm_CreateBufferData();
    int     gtcnm_ReadBufferData();
    int     gtcnm_InitStruct();
    int     gtcnm_FreeStruct();
    void    gtcnm_CopyString();
#else

#if defined(__cplusplus)
    extern "C"
    {
        int     gtcnm_CreateBufferData(unsigned int lType, PT_GNB_HBUFFER, void*);
        int     gtcnm_ReadBufferData(unsigned int lType, PT_GNB_HBUFFER, void*);
        int     gtcnm_InitStruct(unsigned int lType, void*);
        int     gtcnm_FreeStruct(unsigned int lType, void*);
        void    gtcnm_CopyString(char*, const char*, size_t);
    }
#else

    extern int  gtcnm_CreateBufferData();
    extern int  gtcnm_ReadBufferData();
    extern int  gtcnm_InitStruct();
    extern int  gtcnm_FreeStruct();
    extern void gtcnm_CopyString();

#endif /* #if defined(__cplusplus) */

#endif /* #ifdef _GTC_NETMESSAGE_MODULE_ */

#endif /* #ifndef _GTC_NETMESSAGE_H_ */
