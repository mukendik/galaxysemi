/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_testlist.h                                                               */
/*                                                                                      */
/* Functions to handle the test list (of monitored tests).								*/
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 10 June 2005: Created.                                                        */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTL_TESTLIST_H_
#define _GTL_TESTLIST_H_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
//#include <list> // As this header will be included by .c sources, the compil will failed with a  "list not found"
#include <gtc_constants.h>
#include <gtc_netmessage.h>

/*======================================================================================*/
/* Macros                                                                               */
/*======================================================================================*/

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/
#define	GTL_TL_FIND_NOREWIND 0
#define	GTL_TL_FIND_REWINDFIRST	1
#define	GTL_TL_FIND_REWINDATEND	2

/*======================================================================*/
/* Typedefs                                                             */
/*======================================================================*/

//struct SiteItem;

typedef struct TagTestItem
{
    void*               mSite; // Site (SiteItem pointer) this test belongs to
	unsigned int		uiTestIndex;
	unsigned int		uiTestFlags;
	long				lTestNumber;
	char				szTestName[GTC_MAX_TESTNAME_LEN];
    int                 mPinIndex;
    float				fLowLimit_Static;
	float				fHighLimit_Static;
	float				fLowLimit1_Dynamic;
	float				fHighLimit1_Dynamic;
	float				fLowLimit2_Dynamic;
	float				fHighLimit2_Dynamic;
	int					nPatStaticBinning;
	int					nPatDynamicBinning;
	unsigned int		uiNbExecs;
	unsigned int		uiFailCount;
	double				lfCumul_X;
	double				lfCumul_X2;
	double				lfMin;
	double				lfMax;
	double				lfMean;
	double				lfSigma;
    GNM_TESTDEF_STATS   mLastStatsFromGtm;

    /* Ptr to Array of test results for this test: keep only results for packets to send to GTM */
    PT_GNM_TESTRESULT   mTestResults;
    /* Ptr to Array of test statistics for this test: stats forpacket to send to GTM */
    PT_GNM_TESTSTAT_RESULTS	pstTestStats;
} TestItem;

typedef struct TagSiteItem
{
    int             mCurrentSiteState; /* Current state for this site (init, baseline, pat) */
    unsigned int	mSiteIndex;
    unsigned int    mSiteNb; // Site number as declared by the user : 10, 11, 12, ...
    unsigned int	mPartCount; // not used currently
    unsigned int	mPassCount; // not used currently
    unsigned int	mFailCount; // not used currently
    unsigned int	mFailCount_Pat; // not used currently
    unsigned int	mNbTests; // ?
    int             mLastLimitsPartIndex; // -1 if no limits received, else the PartIndex when limits (static or dyn)
    // FLag used for new limits events (raised when new limits received, cleared by gtl_get_xpat_limits() functions)
    // Could be used for other events
    unsigned int    mPendingEvents;
    // Event type (GTL_SPAT_LIMITS, GTL_DPAT_LIMITS) and sub_type (BASELINE, TUNING).
    // Set and cleared internally
    char            mEventType[GTC_MAX_EVENT_TYPE];
    char            mEventSubtype[GTC_MAX_EVENT_SUBTYPE];
    //
    /* Index of current test, -1 if no current test (used for find optimizations) */
    unsigned int	mNextTest_Find;
    /* TestList array (dynamically allocated) */
    TestItem		*mTestList;
    /* Ptr to Array of run results. */
    PT_GNM_RUNRESULT mRunResults;
} SiteItem, *PT_SITE_ITEM;

/*======================================================================================*/
/* PUBLIC Functions																		*/
/*======================================================================================*/
#ifdef _GTL_TESTLIST_MODULE_

    int			gtl_tl_Init();
    int			gtl_tl_Create(PT_GNM_R_TESTLIST pstTestList_Reply);
    int			gtl_tl_UpdateWithStaticPat(PT_GNM_R_PATCONFIG_STATIC pstStaticPatConfig);
    int			gtl_tl_UpdateWithDynamicPat(PT_GNM_PATCONFIG_DYNAMIC pstDynamicPatConfig);
    int         gtl_tl_Free();
    int			gtl_tl_ResetTestResult();
    int			gtl_tl_ResetTestlist();
    int         gtl_tl_ResetSiteTestlist(const int lSiteNb);
    int         gtl_tl_UpdateTestResult(unsigned int uiSiteNb, long lTestNb, char* szTestName, float fResult);
    int			gtl_tl_UpdateRunResult(unsigned int SiteNb, int* HardBin, int* SoftBin, const char *PartID,
                                       unsigned int* Outliers, char* PatBinType, char OrgBinType);
    void		gtl_tl_DisplayTest(long lTestNumber);
    void		gtl_tl_DisplayTestlist();
    int         gtl_tl_IsGoodSoftBin(int SoftBin);

#elif defined(__cplusplus)

    extern "C"
    {
    int	gtl_tl_Init();
    int	gtl_tl_Create(PT_GNM_R_TESTLIST pstTestList_Reply);
    int	gtl_tl_UpdateWithStaticPat(PT_GNM_R_PATCONFIG_STATIC pstStaticPatConfig_Reply);
    int	gtl_tl_UpdateWithDynamicPat(PT_GNM_PATCONFIG_DYNAMIC pstDynamicPatConfig);
    // return GTL_CORE_ERR_OKAY, ...
    int	gtl_tl_Free();
    int	gtl_tl_ResetTestResult();
    int	gtl_tl_ResetTestlist();
    int gtl_tl_ResetSiteTestlist(const int lSiteNb);
    // Update test result structure with a test result.
    // Returns OK or any other code
    int	gtl_tl_UpdateTestResult(unsigned int uiSiteNb, long lTestNb, char* szTestName, float fResult);
    int	gtl_tl_UpdateRunResult(unsigned int SiteNb, int* HardBin, int* SoftBin, const char *PartID,
                               unsigned int* Outliers, char* PatBinType, char OrgBinType);
    void gtl_tl_DisplayTest(long lTestNumber);
    void gtl_tl_DisplayTestlist();
    int gtl_tl_IsGoodSoftBin(int BinNo);
    int gtl_tl_IsGoodHardBin(int BinNo);
    //int gtl_tl_UpdateTestResult(unsigned int uiSiteNb, long lTestNb, char* lTestName, float fResult);
    int gtl_tl_UpdateMPTestResults(unsigned int SiteNb, long TestNb, char* TestName, double *Results, int* PinIndexes, long ArraySize);

    }
 
#else

    extern int	gtl_tl_Init();
    extern int	gtl_tl_Create();
    extern int	gtl_tl_UpdateWithStaticPat();
    extern int	gtl_tl_UpdateWithDynamicPat();
    extern int	gtl_tl_Free();
    extern int	gtl_tl_ResetTestResult();
    extern int	gtl_tl_ResetTestlist();
    extern int  gtl_tl_ResetSiteTestlist(const int lSiteNb);
    extern int	gtl_tl_UpdateTestResult(unsigned int uiSiteNb, long lTestNb, char* szTestName, float fResult);
    extern int	gtl_tl_UpdateRunResult(unsigned int SiteNb, int* HardBin, int* SoftBin, const char *PartID,
                                       unsigned int* Outliers, char* PatBinType, char OrgBinType);
    extern void	gtl_tl_DisplayTest(long lTestNumber);
    extern void	gtl_tl_DisplayTestlist();
    extern int gtl_tl_IsGoodSoftBin(int SoftBinNo);
    extern int gtl_tl_IsGoodHardBin(int HardBinNo);

#endif /* #ifdef _GTL_TESTLIST_MODULE_ */

#endif /* #ifndef _GTL_TESTLIST_H_ */


