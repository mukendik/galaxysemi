/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_testlist.c                                                               */
/*                                                                                      */
/* Functions to handle the test list (of monitored tests).								*/
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_TESTLIST_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif
//#include <syslog.h>
#include <gstdl_utils_c.h> // for ut_normalize

#include "gtl_core.h"
#include "gtl_testlist.h"
#include "gtl_constants.h"
#include "gtl_message.h"

/*======================================================================================*/
/* Externs                                                                              */
/*======================================================================================*/
/* From gtl_main.c */
extern GTL_Station gtl_stStation; /* Tester station information */
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern GTL_PatSettings gtl_stPatSettings; /* Pat settings */
extern GNM_RESULTS gtl_stResults; /* Test result structure */
extern int gtl_OutputTestDefs(PT_GNM_TESTDEF, unsigned);
extern int gtl_OutputNewLimits(SiteItem *s, int lOriginalBin, int lPATBin, const char* lPartID);
extern int gtl_OutputNewOutlieredTest(TestItem* t, float Result, char LimitType);

// QA usefull dump function
extern char* OpenDump(char* lFileName);
extern void OutputStringToDump(char* lString);
extern void OutputFloatToDump(float lFloat, int lPrecision);
extern void OutputIntToDump(int lInt);
extern char DumpOpened();
extern void CloseDump();

/*======================================================================================*/
/* Public declarations                                                                  */
/*======================================================================================*/
/* Variables */
SiteItem gtl_tl_ptSiteArray[256];			/* Array of sites */

/*======================================================================================*/
/* Private declarations                                                                 */
/*======================================================================================*/
/* Variables */

/* Functions */
TestItem* gtl_tl_FindTestBySiteNb(unsigned int uiSiteNb, long lTestNb, char* szTestName, int PinIndex,
                                    unsigned int uiFindTestKey, int nFindMode);
TestItem* gtl_tl_FindTestBySiteIndex(unsigned int uiSiteIndex, long lTestNb, char* szTestName, int PinIndex,
                                       unsigned int uiFindTestKey, int nFindMode);
int       gtl_tl_CheckTestResults(SiteItem* Site, unsigned int* Outliers, char* PatBinType);

/*======================================================================================*/
/* Public CODE                                                                          */
/*======================================================================================*/

/* Description  : initialize the SiteList/TestList structures.							*/
/* Return Codes : 1 if successful, 0 else                                               */
int gtl_tl_Init()
{
    unsigned int uiSiteNb=0;

	for(uiSiteNb=0; uiSiteNb<256; uiSiteNb++)
	{
        gtl_tl_ptSiteArray[uiSiteNb].mCurrentSiteState = GTL_SITESTATE_BASELINE;
        gtl_tl_ptSiteArray[uiSiteNb].mSiteIndex = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mSiteNb = uiSiteNb;
        gtl_tl_ptSiteArray[uiSiteNb].mPartCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mPassCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mFailCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mFailCount_Pat = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mNbTests = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mNextTest_Find = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mLastLimitsPartIndex=-1; // in case of no limits
        gtl_tl_ptSiteArray[uiSiteNb].mTestList = NULL;
        gtl_tl_ptSiteArray[uiSiteNb].mRunResults = NULL;
        // 7304 BG: use new event type & subtype
        gtl_tl_ptSiteArray[uiSiteNb].mEventType[0] = '\0';
        gtl_tl_ptSiteArray[uiSiteNb].mEventSubtype[0] = '\0';
        gtl_tl_ptSiteArray[uiSiteNb].mPendingEvents = 0;
    }

	return 1;
}

/*======================================================================================*/
/* Description  : create the testlist of monitored tests.								*/
/* Argument(s)  :                                                                       */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_Create(PT_GNM_R_TESTLIST  pstTestList_Reply)
{
    PT_GNM_TESTDEF ptTestDefinition=0;
    unsigned int uiNbSites=0, uiNbTests=0, uiNbRuns=0;
    unsigned int uiSiteIndex=0, uiSiteNb=0, uiTestIndex=0;
    unsigned long ulIndex=0;
    TestItem* pstTestList=0;
    char* lQaOutput=NULL;
    char lFileName[GTC_MAX_PATH]="", lSiteString[10]="";
    FILE* lQaDumpFile=NULL;

	/* Make sure list is free'ed */
    gtl_tl_Free();

    /* Make sure TestList has some tests */
    if ((pstTestList_Reply == NULL) || (pstTestList_Reply->pstTestDefinitions == NULL))
    {
        if (!pstTestList_Reply)
            GTL_LOG(3, "TestList reply null", 0);
        if (!pstTestList_Reply->pstTestDefinitions)
            GTL_LOG(3, "TestDefinitions null", 0);
        return 0;
    }

	/* Check sizes */
	uiNbSites = gtl_stStation.uiNbSites;
	uiNbTests = pstTestList_Reply->uiNbTests;
	uiNbRuns = gtl_stGlobalInfo.uiResultFrequency;
	ulIndex = uiNbSites*uiNbTests*uiNbRuns;
	if(ulIndex == 0)
    {
        GTL_LOG(3, "Zero Index: NbSites(%d)*NbTests(%d)*NbRuns(%d)=0", uiNbSites, uiNbTests, uiNbRuns);
        return 0;
    }

    // Output Test list now or later ?

    //pstTestList_Reply->pstTestDefinitions : table of TestDefinition
    int lR=gtl_OutputTestDefs(pstTestList_Reply->pstTestDefinitions, uiNbTests);
    if (lR!=0)
    {
        GTL_LOG(3, "Cannot output tests list: %d", lR);
        return 0;
    }

	/* Allocate and Init test result structure */
    gtl_stResults.mTestResults = (PT_GNM_TESTRESULT)malloc(uiNbSites*uiNbTests*uiNbRuns*sizeof(GNM_TESTRESULT));
    if(gtl_stResults.mTestResults == NULL)
	{
        GTL_LOG(3, "Cannot allocate TestResults", 0);
		gtl_tl_Free();
		return 0;
	}
    gtl_stResults.mRunResults = (PT_GNM_RUNRESULT)malloc(uiNbSites*uiNbRuns*sizeof(GNM_RUNRESULT));
    if(gtl_stResults.mRunResults == NULL)
	{
        GTL_LOG(3, "Cant allocate RunResults", 0);
		gtl_tl_Free();
		return 0;
	}
    gtl_stResults.mTestStats = (PT_GNM_TESTSTAT_RESULTS)malloc(uiNbSites*uiNbTests*sizeof(GNM_TESTSTAT_RESULTS));
    if(gtl_stResults.mTestStats == NULL)
	{
        GTL_LOG(3, "Cant allocate TestStats", 0);
		gtl_tl_Free();
		return 0;
	}
    gtl_stResults.mNbSites = uiNbSites;
    gtl_stResults.mNbTests = uiNbTests;
    gtl_stResults.mNbAllocatedRuns = uiNbRuns;
    gtl_stResults.mNbValidRuns = 0;

    /* GS_QA: open dump file */
    lQaOutput=getenv("GS_QA_OUTPUT_FOLDER");
    if(getenv("GS_QA") && lQaOutput)
    {
        /* Build output file name & open file*/
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_socket_testlist_rx");
        if(uiNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stStation.pnSiteNumbers[0]);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);

        /* Open output file */
        lQaDumpFile=fopen(lFileName, "a+");
    }

    /* GS_QA: dump limits for site */
    if(lQaDumpFile)
    {
        fprintf(lQaDumpFile, "#### Test List ####\n");
        fprintf(lQaDumpFile, "Test#,Pin#,TestName,TestFlags\n");
    }

    /* Create sites and test lists */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		/* Init site */
		uiSiteNb = gtl_stStation.pnSiteNumbers[uiSiteIndex];
        gtl_tl_ptSiteArray[uiSiteNb].mSiteIndex = uiSiteIndex;
        gtl_tl_ptSiteArray[uiSiteNb].mSiteNb = uiSiteNb;
        gtl_tl_ptSiteArray[uiSiteNb].mNbTests = uiNbTests;

		/* Create test list */
        pstTestList = (TestItem*)malloc(uiNbTests*sizeof(TestItem));
		if(pstTestList == NULL)
		{
            GTL_LOG(3, "Cant allocate TestList", 0);
			gtl_tl_Free();
            return 0;
		}
        gtl_tl_ptSiteArray[uiSiteNb].mTestList = pstTestList;
        gtl_tl_ptSiteArray[uiSiteNb].mRunResults = gtl_stResults.mRunResults + uiSiteIndex*uiNbRuns;

		/* Init TestList */
		for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
		{
            // Get test definition
            ptTestDefinition = pstTestList_Reply->pstTestDefinitions+uiTestIndex;

			/* Init test item */
            pstTestList[uiTestIndex].mSite= &gtl_tl_ptSiteArray[uiSiteNb];
            pstTestList[uiTestIndex].fLowLimit_Static = -GTL_INFINITE_VALUE_FLOAT;
            pstTestList[uiTestIndex].fHighLimit_Static = GTL_INFINITE_VALUE_FLOAT;
            pstTestList[uiTestIndex].fLowLimit1_Dynamic = -GTL_INFINITE_VALUE_FLOAT;
            pstTestList[uiTestIndex].fHighLimit1_Dynamic = GTL_INFINITE_VALUE_FLOAT;
            pstTestList[uiTestIndex].fLowLimit2_Dynamic = -GTL_INFINITE_VALUE_FLOAT;
            pstTestList[uiTestIndex].fHighLimit2_Dynamic = GTL_INFINITE_VALUE_FLOAT;
			pstTestList[uiTestIndex].nPatStaticBinning = -1;
			pstTestList[uiTestIndex].nPatDynamicBinning = -1;
			pstTestList[uiTestIndex].lfCumul_X = 0.0;
			pstTestList[uiTestIndex].lfCumul_X2 = 0.0;
            pstTestList[uiTestIndex].lfMax = GTL_INVALID_VALUE_DOUBLE;
            pstTestList[uiTestIndex].lfMean = GTL_INVALID_VALUE_DOUBLE;
            pstTestList[uiTestIndex].lfMin = GTL_INVALID_VALUE_DOUBLE;
            pstTestList[uiTestIndex].lfSigma = GTL_INVALID_VALUE_DOUBLE;
			pstTestList[uiTestIndex].lTestNumber = ptTestDefinition->lTestNumber;
            strcpy(pstTestList[uiTestIndex].szTestName, ptTestDefinition->szTestName);
            pstTestList[uiTestIndex].mPinIndex = ptTestDefinition->mPinIndex;
			pstTestList[uiTestIndex].uiFailCount = 0;
			pstTestList[uiTestIndex].uiNbExecs = 0;
			pstTestList[uiTestIndex].uiTestFlags = ptTestDefinition->uiTestFlags;
			pstTestList[uiTestIndex].uiTestIndex = uiTestIndex;

            /* Init last stats from Gtm */
            strcpy(pstTestList[uiTestIndex].mLastStatsFromGtm.mDistributionShape, "");
            pstTestList[uiTestIndex].mLastStatsFromGtm.mN_Factor = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mT_Factor = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mMean = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mSigma = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mMin = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mQ1 = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mMedian = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mQ3 = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mMax = GTL_INVALID_VALUE_FLOAT;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mExecCount = 0;
            pstTestList[uiTestIndex].mLastStatsFromGtm.mFailCount = 0;

			/* Set ptrs to result structure */
			ulIndex = uiSiteIndex*uiNbTests+uiTestIndex;
            pstTestList[uiTestIndex].pstTestStats = gtl_stResults.mTestStats + ulIndex;
			ulIndex = uiSiteIndex*uiNbRuns*uiNbTests + uiTestIndex*uiNbRuns;
            pstTestList[uiTestIndex].mTestResults = gtl_stResults.mTestResults + ulIndex;

			/* Set test number and name in test stats */
            pstTestList[uiTestIndex].pstTestStats->mSiteNb = uiSiteNb;
			pstTestList[uiTestIndex].pstTestStats->lTestNumber = ptTestDefinition->lTestNumber;
            strcpy(pstTestList[uiTestIndex].pstTestStats->mTestName, ptTestDefinition->szTestName);
            pstTestList[uiTestIndex].pstTestStats->mPinIndex = ptTestDefinition->mPinIndex;

            /* GS_QA: dump received test definition */
            if(lQaDumpFile)
            {
                fprintf(lQaDumpFile,"%ld,%d,%s,%d\n",
                  ptTestDefinition->lTestNumber, ptTestDefinition->mPinIndex,
                  ptTestDefinition->szTestName, ptTestDefinition->uiTestFlags);
            }
        }
	}

    // GS_QA: dump end marker & close
    if(lQaDumpFile)
    {
        fprintf(lQaDumpFile,"#########################################\n");
        fclose(lQaDumpFile);
    }

    /* Init global variables */
    gtl_stGlobalInfo.uiNbTests = uiNbTests;

	return gtl_tl_ResetTestResult();
}

/*======================================================================================*/
/* Description  : update the testlist with static PAT limits.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_UpdateWithStaticPat(PT_GNM_R_PATCONFIG_STATIC pstStaticPatConfig)
{
    if (!pstStaticPatConfig)
        return 0;

    PT_GNM_TESTDEF_STATICPAT ptTestDefinition=0;
    unsigned int uiNbSites=0, uiNbTests=0;
    unsigned int uiSiteIndex=0, uiTestIndex=0;
    unsigned long ulIndex=0;
    char* lQaOutput=NULL;
    TestItem* pstTestList=0;
    char lFileName[GTC_MAX_PATH]="", lSiteString[10];
    FILE* lQaDumpFile=NULL;
    SiteItem *lSite=0;

    /* Get sizes */
    uiNbSites = gtl_stStation.uiNbSites;
    uiNbTests = pstStaticPatConfig->uiNbTests;
    GTL_LOG(6, "UpdateWithStaticPat %d sites %d tests", uiNbSites, uiNbTests);

    /* GS_QA: open dump file */
    lQaOutput=getenv("GS_QA_OUTPUT_FOLDER");
    if(getenv("GS_QA") && lQaOutput)
    {
        /* Build output file name & open file*/
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_socket_spatlimits_rx");
        if(uiNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stStation.pnSiteNumbers[0]);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);

        /* Open output file */
        lQaDumpFile=fopen(lFileName, "a+");
        if(lQaDumpFile)
        {
            fprintf(lQaDumpFile, "#### SPAT limits ####\n");
            fprintf(lQaDumpFile, "Test#,Pin#,TestName,TestFlags,LL,HL,SpatBinning\n");
        }
    }

    /* Make sure PAT configuration has some tests */
	if((pstStaticPatConfig == NULL) || (pstStaticPatConfig->pstTestDefinitions == NULL))
    {
        // GS_QA: dump end marker & close
        if(lQaDumpFile)
        {
            fprintf(lQaDumpFile,"#########################################\n");
            fclose(lQaDumpFile);
        }
        return 1;
    }

    /* Check sizes */
    ulIndex = uiNbSites*uiNbTests;
    if(ulIndex == 0)
    {
        // GS_QA: dump end marker & close
        if(lQaDumpFile)
        {
            fprintf(lQaDumpFile,"#########################################\n");
            fclose(lQaDumpFile);
        }
        return 0;
    }

    /* Retrieve some global settings */
	gtl_stPatSettings.uiBaseline = pstStaticPatConfig->uiBaseline;
	gtl_stPatSettings.uiFlags = pstStaticPatConfig->uiFlags;
    GTL_LOG(5, "Update PAT config : baseline : %d", gtl_stPatSettings.uiBaseline);

    /* Go through all tests */
	for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
	{
		/* Get static PAT test definition (only 1 site for static PAT) */
		ptTestDefinition = pstStaticPatConfig->pstTestDefinitions+uiTestIndex;
        if (!ptTestDefinition)
            continue; // is n't it abnormal

		/* Go through all sites */
		for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
		{
            pstTestList = gtl_tl_FindTestBySiteIndex(uiSiteIndex, ptTestDefinition->mTestNumber,
                                                     ptTestDefinition->mTestName, ptTestDefinition->mPinIndex,
                                                     gtl_stGlobalInfo.uiFindTestKey, GTL_TL_FIND_REWINDATEND);
			if(pstTestList)
			{
                pstTestList->fLowLimit_Static = ptTestDefinition->mLowLimit1;
                pstTestList->fHighLimit_Static = ptTestDefinition->mHighLimit1;
                pstTestList->nPatStaticBinning = ptTestDefinition->mPatBinning;
                pstTestList->uiTestFlags = ptTestDefinition->mTestFlags;
                //pstTestList->mPinIndex = ptTestDefinition->mPindex; ?
                /* Init last stats from Gtm */
                strcpy(pstTestList->mLastStatsFromGtm.mDistributionShape,
                       ptTestDefinition->mTestStats.mDistributionShape);
                pstTestList->mLastStatsFromGtm.mN_Factor = ptTestDefinition->mTestStats.mN_Factor;
                pstTestList->mLastStatsFromGtm.mT_Factor = ptTestDefinition->mTestStats.mT_Factor;
                pstTestList->mLastStatsFromGtm.mMean = ptTestDefinition->mTestStats.mMean;
                pstTestList->mLastStatsFromGtm.mSigma = ptTestDefinition->mTestStats.mSigma;
                pstTestList->mLastStatsFromGtm.mMin = ptTestDefinition->mTestStats.mMin;
                pstTestList->mLastStatsFromGtm.mQ1 = ptTestDefinition->mTestStats.mQ1;
                pstTestList->mLastStatsFromGtm.mMedian = ptTestDefinition->mTestStats.mMedian;
                pstTestList->mLastStatsFromGtm.mQ3 = ptTestDefinition->mTestStats.mQ3;
                pstTestList->mLastStatsFromGtm.mMax = ptTestDefinition->mTestStats.mMax;
                pstTestList->mLastStatsFromGtm.mExecCount = ptTestDefinition->mTestStats.mExecCount;
                pstTestList->mLastStatsFromGtm.mFailCount = ptTestDefinition->mTestStats.mFailCount;

                /* GS_QA: dump received limits */
                if(lQaDumpFile)
                {
                    fprintf(lQaDumpFile,"%ld,%d,%s,%d,%.6e,%.6e,%d\n",
                            ptTestDefinition->mTestNumber, ptTestDefinition->mPinIndex,
                            ptTestDefinition->mTestName,ptTestDefinition->mTestFlags,
                            ptTestDefinition->mLowLimit1,ptTestDefinition->mHighLimit1,
                            ptTestDefinition->mPatBinning);
                }
            }

            // New event: received new limits or SPAT settings
            lSite = gtl_tl_ptSiteArray + gtl_stStation.pnSiteNumbers[uiSiteIndex];
            if (lSite)
            {
                // Raise flag in Pending events for new DPAT limits
                lSite->mPendingEvents |= GTL_EVENT_NEW_SPAT_LIMITS;
                strcpy(lSite->mEventType, "GTL_SPAT_LIMITS");
                strcpy(lSite->mEventSubtype, "\0");
            }

        } // all sites

    } // all tests

    // GS_QA: dump end marker & close
    if(lQaDumpFile)
    {
        fprintf(lQaDumpFile,"#########################################\n");
        fclose(lQaDumpFile);
    }

    return 1;
}

/*======================================================================================*/
/* Description  : update the testlist with dynamic PAT limits.							*/
/* Argument(s)  :                                                                       */
/* Return Codes : 1 if successful, 0 else                                               */
int gtl_tl_UpdateWithDynamicPat(PT_GNM_PATCONFIG_DYNAMIC pstDynamicPatConfig)
{
    PT_GNM_TESTDEF_DYNAMICPAT ptTestDefinition=0;
    unsigned int uiNbSites=0, uiNbTests=0;
    unsigned int uiSiteIndex=0, uiTestIndex=0;
    unsigned long ulIndex=0;
    TestItem* pstTestList=0;
    /* GS_QA: open dump file */
    //FILE* lQaDumpFile=NULL; // Let's use GTL dump functions: OpenDump(),...

    char lFileName[GTC_MAX_PATH]="", lSiteString[10]="";
    char* lQaOutput=NULL;
    SiteItem *lSite=0;

	/* Make sure PAT configuration has some tests */
	if((pstDynamicPatConfig == NULL) || (pstDynamicPatConfig->pstTestDefinitions == NULL))
    {
        if (!pstDynamicPatConfig)
            GTL_LOG(4, "Update With DPAT config: null PAT config", 0);

        if (!pstDynamicPatConfig->pstTestDefinitions)
            GTL_LOG(4, "Update With DPAT config: null TestDefs", 0);

        return 1; // todo: return an error ?
    }

	/* Check sizes */
    if(pstDynamicPatConfig->pstTestDefinitions->mSiteNb == -1)
        uiNbSites = gtl_stStation.uiNbSites;
    else
        uiNbSites = pstDynamicPatConfig->uiNbSites;
	uiNbTests = pstDynamicPatConfig->uiNbTests;

    GTL_LOG(5, "New DPAT config received for %d sites and %d tests at part index %d",
            uiNbSites, uiNbTests, gtl_stGlobalInfo.uiPartIndex);

	ulIndex = uiNbSites*uiNbTests;
	if(ulIndex == 0)
		return 0;

    // For limits, only a precision of 0 will allow to pass the QA on all platforms: exple: 1e-002 ?
    // todo : use a perfect precision library to do at least PAT ?
    // for the moment no choice: for QA there are precisions diffs between platforms, let s round...
    int lPrecision=4;
    char* lGSQAPrec=getenv("GS_QA_PRECISION_DPAT_LIMITS_RX");
    if (lGSQAPrec)
       if (sscanf(lGSQAPrec, "%d", &lPrecision)!=1)
           lPrecision=4;
    if (lPrecision<0)
       lPrecision=4;

    /* Go through all sites */
    for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
        /* Get dynamic PAT test definition for first test in the site, to retrieve site# */
        if(pstDynamicPatConfig->pstTestDefinitions->mSiteNb == -1)
            ptTestDefinition = pstDynamicPatConfig->pstTestDefinitions;
        else
            ptTestDefinition = pstDynamicPatConfig->pstTestDefinitions+(uiSiteIndex*uiNbTests);

        /* GS_QA: open dump file, except if receiving DPAT config for all sites */
        lQaOutput=getenv("GS_QA_OUTPUT_FOLDER");
        if(getenv("GS_QA") && lQaOutput && (ptTestDefinition->mSiteNb != -1))
        {
            /* Build output file name & open file*/
            if(uiNbSites==1)
              sprintf(lSiteString, "_s%d", ptTestDefinition->mSiteNb);
            else
              strcpy(lSiteString, "_smulti");
            strcpy(lFileName, lQaOutput);
            if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
              strcat(lFileName, "/");
            strcat(lFileName, "gs_gtl_socket_dpatlimits_rx");
            strcat(lFileName, lSiteString);
            strcat(lFileName, ".csv");
            ut_NormalizePath(lFileName);

            /* Open output file */
            //lQaDumpFile=fopen(lFileName, "a+");
            char* lRes=OpenDump(lFileName);
            if (strcmp(lRes, "ok")!=0)
                GTL_LOG(4, "Opening QA dump '%s' failed: %s", lFileName, lRes);
        }

        /* GS_QA: dump limits for site */
        //if(lQaDumpFile && (ptTestDefinition->mSiteNb != -1))
        if(DumpOpened()=='Y' && (ptTestDefinition->mSiteNb != -1))
        {
            //fprintf(lQaDumpFile, "#### DPAT limits ####\n");
            OutputStringToDump("#### DPAT limits ####\n");
            //fprintf(lQaDumpFile, "Site#,Test#,Pin#,TestName,LL2,LL1,HL1,HL2\n");
            OutputStringToDump("Site#,Test#,Pin#,TestName,LL2,LL1,HL1,HL2\n");
        }

        /* Go through all tests */
        for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
        {
            if(pstDynamicPatConfig->pstTestDefinitions->mSiteNb == -1)
            {
                ulIndex = uiTestIndex;
                /* Get dynamic PAT test definition for this test/site */
                ptTestDefinition = pstDynamicPatConfig->pstTestDefinitions+ulIndex;

                pstTestList = gtl_tl_FindTestBySiteIndex(uiSiteIndex, ptTestDefinition->mTestNumber,
                                                      ptTestDefinition->mTestName, ptTestDefinition->mPinIndex,
                                                      gtl_stGlobalInfo.uiFindTestKey,
                                                      GTL_TL_FIND_REWINDATEND);
            }
            else
            {
                ulIndex = uiSiteIndex*uiNbTests + uiTestIndex;
                /* Get dynamic PAT test definition for this test/site */
                ptTestDefinition = pstDynamicPatConfig->pstTestDefinitions+ulIndex;

                pstTestList = gtl_tl_FindTestBySiteNb(ptTestDefinition->mSiteNb, ptTestDefinition->mTestNumber,
                                                      ptTestDefinition->mTestName, ptTestDefinition->mPinIndex,
                                                      gtl_stGlobalInfo.uiFindTestKey,
                                                      GTL_TL_FIND_REWINDATEND);
            }
			if(pstTestList)
			{
                // We do not support bi-modal distributions/limits at FT, so make sure we set the same limits
                // for mode1 and mode2.

                // Set LL
                if(ptTestDefinition->mLowLimit1 == ptTestDefinition->mLowLimit2)
                {
                    pstTestList->fLowLimit1_Dynamic = pstTestList->fLowLimit2_Dynamic = ptTestDefinition->mLowLimit1;
                }
                else if(ptTestDefinition->mLowLimit1 == -GTL_INFINITE_VALUE_FLOAT)
                {
                    pstTestList->fLowLimit1_Dynamic = pstTestList->fLowLimit2_Dynamic = ptTestDefinition->mLowLimit2;
                }
                else if(ptTestDefinition->mLowLimit2 == -GTL_INFINITE_VALUE_FLOAT)
                {
                    pstTestList->fLowLimit1_Dynamic = pstTestList->fLowLimit2_Dynamic = ptTestDefinition->mLowLimit1;
                }
                else if(ptTestDefinition->mLowLimit1 < ptTestDefinition->mLowLimit2)
                {
                    pstTestList->fLowLimit1_Dynamic = pstTestList->fLowLimit2_Dynamic = ptTestDefinition->mLowLimit1;
                }
                else
                {
                    pstTestList->fLowLimit1_Dynamic = pstTestList->fLowLimit2_Dynamic = ptTestDefinition->mLowLimit2;
                }

                // Set HL
                if(ptTestDefinition->mHighLimit1 == ptTestDefinition->mHighLimit2)
                {
                    pstTestList->fHighLimit1_Dynamic = pstTestList->fHighLimit2_Dynamic = ptTestDefinition->mHighLimit1;
                }
                else if(ptTestDefinition->mHighLimit1 == GTL_INFINITE_VALUE_FLOAT)
                {
                    pstTestList->fHighLimit1_Dynamic = pstTestList->fHighLimit2_Dynamic = ptTestDefinition->mHighLimit2;
                }
                else if(ptTestDefinition->mHighLimit2 == GTL_INFINITE_VALUE_FLOAT)
                {
                    pstTestList->fHighLimit1_Dynamic = pstTestList->fHighLimit2_Dynamic = ptTestDefinition->mHighLimit1;
                }
                else if(ptTestDefinition->mHighLimit1 > ptTestDefinition->mHighLimit2)
                {
                    pstTestList->fHighLimit1_Dynamic = pstTestList->fHighLimit2_Dynamic = ptTestDefinition->mHighLimit1;
                }
                else
                {
                    pstTestList->fHighLimit1_Dynamic = pstTestList->fHighLimit2_Dynamic = ptTestDefinition->mHighLimit2;
                }

                // Set binning & flags
                pstTestList->nPatDynamicBinning = ptTestDefinition->mPatBinning;
                pstTestList->uiTestFlags = ptTestDefinition->mTestFlags;

                /* Init last stats from Gtm */
                strcpy(pstTestList->mLastStatsFromGtm.mDistributionShape,
                       ptTestDefinition->mTestStats.mDistributionShape);
                pstTestList->mLastStatsFromGtm.mN_Factor = ptTestDefinition->mTestStats.mN_Factor;
                pstTestList->mLastStatsFromGtm.mT_Factor = ptTestDefinition->mTestStats.mT_Factor;
                pstTestList->mLastStatsFromGtm.mMean = ptTestDefinition->mTestStats.mMean;
                pstTestList->mLastStatsFromGtm.mSigma = ptTestDefinition->mTestStats.mSigma;
                pstTestList->mLastStatsFromGtm.mMin = ptTestDefinition->mTestStats.mMin;
                pstTestList->mLastStatsFromGtm.mQ1 = ptTestDefinition->mTestStats.mQ1;
                pstTestList->mLastStatsFromGtm.mMedian = ptTestDefinition->mTestStats.mMedian;
                pstTestList->mLastStatsFromGtm.mQ3 = ptTestDefinition->mTestStats.mQ3;
                pstTestList->mLastStatsFromGtm.mMax = ptTestDefinition->mTestStats.mMax;
                pstTestList->mLastStatsFromGtm.mExecCount = ptTestDefinition->mTestStats.mExecCount;
                pstTestList->mLastStatsFromGtm.mFailCount = ptTestDefinition->mTestStats.mFailCount;

                /* GS_QA: dump received limits */
                //if(lQaDumpFile && (ptTestDefinition->mSiteNb != -1))
                if(DumpOpened()=='Y' && (ptTestDefinition->mSiteNb != -1))
                {
                    // No choice: our QA servers have precision diffs.
                    // Let s round for the moment using GTL smart dump functions using dynamic variable precision.
                    /*
                    fprintf(lQaDumpFile,"%d,%ld,%d,%s,%.6e,%.6e,%.6e,%.6e\n",
                            ptTestDefinition->mSiteNb,ptTestDefinition->mTestNumber,ptTestDefinition->mPinIndex,
                            ptTestDefinition->mTestName,
                            ptTestDefinition->mLowLimit2,ptTestDefinition->mLowLimit1,
                            ptTestDefinition->mHighLimit1,ptTestDefinition->mHighLimit2);
                     */

                    OutputIntToDump(ptTestDefinition->mSiteNb); OutputStringToDump(",");
                    OutputLongToDump(ptTestDefinition->mTestNumber); OutputStringToDump(",");
                    OutputIntToDump(ptTestDefinition->mPinIndex); OutputStringToDump(",");
                    OutputStringToDump(ptTestDefinition->mTestName); OutputStringToDump(",");
                    OutputFloatToDump(ptTestDefinition->mLowLimit2, lPrecision); OutputStringToDump(",");
                    OutputFloatToDump(ptTestDefinition->mLowLimit1, lPrecision); OutputStringToDump(",");
                    OutputFloatToDump(ptTestDefinition->mHighLimit1, lPrecision); OutputStringToDump(",");
                    OutputFloatToDump(ptTestDefinition->mHighLimit2, lPrecision); OutputStringToDump(",\n");
                }
            }
            else
                GTL_LOG(4, "Unfindable test %ld pin %d name '%s'", ptTestDefinition->mTestNumber,
                        ptTestDefinition->mPinIndex, ptTestDefinition->mTestName);
		}

        // GS_QA: dump end marker & close
        //if(lQaDumpFile && (ptTestDefinition->mSiteNb != -1))
        if(DumpOpened()=='Y' && (ptTestDefinition->mSiteNb != -1))
        {
            //fprintf(lQaDumpFile,"#########################################\n");
            OutputStringToDump("#########################################\n");
            //fclose(lQaDumpFile);
            CloseDump();
        }

        if(ptTestDefinition->mSiteNb != -1)
        {
            // New event: received new limits or DPAT settings
            // Get Site ptr
            lSite = gtl_tl_ptSiteArray + ptTestDefinition->mSiteNb;
            GTL_LOG(5, "Received and applied DPAT configuration/limits for site %d", ptTestDefinition->mSiteNb);
            strcpy(lSite->mEventType, "GTL_DPAT_LIMITS");
            strcpy(lSite->mEventSubtype, "TUNING");
            if(lSite->mCurrentSiteState == GTL_SITESTATE_BASELINE)
                strcpy(lSite->mEventSubtype, "BASELINE");

            /* Set site state to DPAT */
            lSite->mCurrentSiteState = GTL_SITESTATE_DPAT;

            // Raise flag in Pending events for new DPAT limits
            lSite->mPendingEvents |= GTL_EVENT_NEW_DPAT_LIMITS;
        }
        else
        {
            GTL_LOG(5, "Received DPAT configuration for all sites at initialization. Applied configuration for site %d",
                    gtl_stStation.pnSiteNumbers[uiSiteIndex]);
            // Should we trigger an event ?
            //strcpy(lSite->mEventType, "GTL_DPAT_CONFIG");
            //strcpy(lSite->mEventSubtype, "\0");
        }

    } // all sites

    return 1;
}

int gtl_tl_UpdateTestResult(unsigned int uiSiteNb, long lTestNb, char* szTestName, float fResult)
{
    SiteItem *lSite = gtl_tl_ptSiteArray + uiSiteNb;
    TestItem *ptTestItem=0;
    PT_GNM_TESTSTAT_RESULTS	pstTestStats=0;
    unsigned int uiRunIndex=0;

    /* Make sure GTL is enabled (or offline) */
    if( (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_ENABLED) &&
        (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
        return GTL_CORE_ERR_NOT_ENABLED;

    /* Check site pointer */
    if(!lSite)
        return GTL_CORE_ERR_INVALID_SITE_PTR;

    // Find test
    ptTestItem = gtl_tl_FindTestBySiteNb(uiSiteNb, lTestNb, szTestName, -1, // PTR !!!
                                         gtl_stGlobalInfo.uiFindTestKey, GTL_TL_FIND_REWINDATEND);
	if(ptTestItem == NULL)
        return GTL_CORE_ERR_INVALID_TEST;

	/* Update TestList and result structures */
	uiRunIndex = gtl_stGlobalInfo.uiRunIndex_Rolling;
	pstTestStats = ptTestItem->pstTestStats;

	/* Update TestList */
	(ptTestItem->uiNbExecs)++;

	/* Test results */
    (ptTestItem->mTestResults)[uiRunIndex].mValue = fResult;
    (ptTestItem->mTestResults)[uiRunIndex].mFlags |= GTC_TRFLAG_VALID;

	/* Test statistics */
	(pstTestStats->uiExecs)++;

	/* Run results */
    ((lSite->mRunResults)[uiRunIndex].mNbTestsExecuted)++;

    return GTL_CORE_ERR_OKAY;
}

int gtl_tl_UpdateMPTestResults(unsigned int lSiteNb, long lTestNb, char* lTestName,
                              double *lResults, int* lPinIndexes, long lArraysSize)
{
    long i=0;
    if (!lTestName || !lResults || !lPinIndexes)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    SiteItem *lSite = gtl_tl_ptSiteArray + lSiteNb;
    if (!lSite)
    {
        GTL_LOG(4, "Illegal site %d for test %ld '%s'", lSiteNb, lTestNb, lTestName);
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    }

    TestItem *lTestItem=0;
    PT_GNM_TESTSTAT_RESULTS	lTestStats=0;
    unsigned int uiRunIndex=0;

    /* Make sure GTL is enabled (or offline) */
    if( (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_ENABLED) &&
        (gtl_stGlobalInfo.nCurrentLibState != GTL_STATE_OFFLINE))
        return GTL_CORE_ERR_NOT_ENABLED;

    for (i=0; i<lArraysSize; i++)
    {
        int lPinIndex=lPinIndexes[i];
        //GTL_LOG(6, "MPResult %ld = %g", lPin, lResults[lPin]);
        // Find test
        lTestItem = gtl_tl_FindTestBySiteNb(lSiteNb, lTestNb, lTestName, lPinIndex,
            gtl_stGlobalInfo.uiFindTestKey, GTL_TL_FIND_REWINDATEND);

        if(lTestItem == NULL)
        {
            //GTL_LOG(4, "Update MPTestResults: failed to find the TestItem for test %ld pin %d", lTestNb, lPinIndex);
            return GTL_CORE_ERR_INVALID_PIN; // actually perhaps it is the test nb...
        }

        // Update TestList and result structures
        uiRunIndex = gtl_stGlobalInfo.uiRunIndex_Rolling;
        lTestStats = lTestItem->pstTestStats;

        // Update TestList
        (lTestItem->uiNbExecs)++;

        // Test results
        (lTestItem->mTestResults)[uiRunIndex].mValue = (float)(lResults[i]); // yes GTL currently send float, not double
        (lTestItem->mTestResults)[uiRunIndex].mFlags |= GTC_TRFLAG_VALID;

        // Test statistics
        (lTestStats->uiExecs)++;

        // Run results
        ((lSite->mRunResults)[uiRunIndex].mNbTestsExecuted)++;
    }

    return 0;
}

int gtl_tl_IsGoodSoftBin(int SoftBin)
{
    unsigned int lIndex=0;
    for(lIndex=0; lIndex<256; lIndex++)
    {
        if(gtl_stGlobalInfo.pnGoodSoftBins[lIndex] == SoftBin)
            return 1;
        if(gtl_stGlobalInfo.pnGoodSoftBins[lIndex] == -1)
            break;
    }
    return 0;
}

int gtl_tl_IsGoodHardBin(int Bin)
{
    unsigned int lIndex=0;
    for(lIndex=0; lIndex<256; lIndex++)
    {
        if(gtl_stGlobalInfo.pnGoodHardBins[lIndex] == Bin)
            return 1;
        if(gtl_stGlobalInfo.pnGoodHardBins[lIndex] == -1)
            break;
    }
    return 0;
}

/*======================================================================================*/
/*
    Description  : update run result structure with a run result.
    Return Codes : 0 if successful, else GTL_ERR.....
*/
int gtl_tl_UpdateRunResult(SiteNb, HardBin, SoftBin, PartID, Outliers, PatBinType, OrgBinType)
unsigned int SiteNb;
int* HardBin;
int* SoftBin;
const char* PartID;
unsigned int* Outliers;
char* PatBinType;
char OrgBinType;
{
    *Outliers=0;
    *PatBinType='?';

    GTL_LOG(7, "gtl_tl_UpdateRunResult() on site nb %d part %s : hbin=%d, sbin=%d, OrgBinType=%c, ...",
        SiteNb, PartID?PartID:"?", *HardBin, *SoftBin, OrgBinType);

    SiteItem *lSite = gtl_tl_ptSiteArray + SiteNb;

    /* Make sure we have valid pointers */
    if(lSite == NULL)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    if(lSite->mRunResults == NULL)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    if((SoftBin == NULL) || (HardBin == NULL))
        return GTL_CORE_ERR_INVALID_PARAM_POINTER; // anti crash

    /* Make sure binning function has not yet been called for this part */
    if((lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mOrgSoftBin != -1)
    {
        GTL_LOG(4, "gtl_binning() or gtl_set_binning() has already been called for this part (%s): sbin %d, hbin %d",
                PartID?PartID:"NULL", (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mOrgSoftBin,
            (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mOrgHardBin);
        /* Binning has already been assigned and eventually overloaded for this part */
        /* Just set binning assigned previously */
        *SoftBin  = (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPatSoftBin;
        *HardBin  = (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPatHardBin;
        return GTL_CORE_ERR_GTL_BIN_ALREADY_CALLED; //1;
    }

    /* Update site array */
    (lSite->mPartCount)++;
	
	/* Update results structure */
    (gtl_stResults.mNbValidRuns)++;

	/* Run results */
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mOrgSoftBin = *SoftBin;
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mOrgHardBin = *HardBin;

    /* If this is a good binning, or already a PAT binning assigned by the tester in case the tester is managing PAT */
    /* limits directly (ie Unison), check if some test results are outliers*/
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mFlags |= GTC_PRFLAG_FAIL;
    if(gtl_tl_IsGoodSoftBin(*SoftBin) || (OrgBinType == 'D') || (OrgBinType == 'S'))
    {
        if(gtl_tl_CheckTestResults(lSite, Outliers, PatBinType) && (*Outliers > 0))
        {
            /* Outliers detected: update binning */
            *SoftBin = *HardBin = gtl_stGlobalInfo.pnPatBinning[SiteNb];
            (lSite->mFailCount_Pat)++;
        }
        else
            (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mFlags &= ~GTC_PRFLAG_FAIL;
    }

    /* Update PAT binning for this part */
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPatSoftBin = *SoftBin;
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPatHardBin = *HardBin;

    /* Update PartID */
    gtcnm_CopyString((lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPartID, PartID, GTC_MAX_PARTID_LEN);
    /* In case no '\0' in first GTC_MAX_PARTID_LEN characters */
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPartID[GTC_MAX_PARTID_LEN-1] = '\0';

    /* Update PartIndex */
    (lSite->mRunResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mPartIndex = gtl_stGlobalInfo.uiPartIndex;

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : check test results for outliers (and update sqlite traceability file) */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_CheckTestResults(SiteItem* Site, unsigned int* Outliers, char* PatBinType)
{
    *Outliers=0;
    *PatBinType='?';

    if(Site == NULL)
    {
        GTL_LOG(3, "Null site pointer.", 0);
        return 0;
    }

    /* Check Test results */
    unsigned int lTestIndex=0;
    TestItem* lTestItem=0;
    float lTestResult=0.0F;
    for(lTestIndex=0; lTestIndex<gtl_stGlobalInfo.uiNbTests; lTestIndex++)
    {
        /* Stop on fail: stop if Part already flagged as PAT */
        if(gtl_stGlobalInfo.pnPatBinning[Site->mSiteNb] != -1)
            break;

        /* Compare test value to PAT limits for valid test results */
        /* Mark results as outliers if valid results and outlide PAT limits */
        lTestItem = Site->mTestList + lTestIndex;
        lTestResult = lTestItem->mTestResults[gtl_stGlobalInfo.uiRunIndex_Rolling].mValue;
        if(!(lTestItem->mTestResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mFlags & GTC_TRFLAG_VALID)
            continue;
        switch(Site->mCurrentSiteState)
        {
            case GTL_SITESTATE_BASELINE:
                /* Check value against static PAT limits (if any) */
                if((lTestItem->nPatStaticBinning != -1) &&
                    (((lTestItem->uiTestFlags & GTL_TFLAG_HAS_LL1) && (lTestResult < lTestItem->fLowLimit_Static)) ||
                        ((lTestItem->uiTestFlags & GTL_TFLAG_HAS_HL1) && (lTestResult > lTestItem->fHighLimit_Static))
                    )
                )
                {
                    gtl_stGlobalInfo.pnPatBinning[Site->mSiteNb] = lTestItem->nPatStaticBinning;
                    (lTestItem->mTestResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mFlags |= GTC_TRFLAG_SPAT_OUTLIER;
                    (*Outliers)++;
                    *PatBinType='S';
                    int lR=gtl_OutputNewOutlieredTest(lTestItem, lTestResult, *PatBinType);
                    if (lR!=0)
                        GTL_LOG(3, "gtl_OutputNewOutlieredTest failed : %d", lR);
                }
                break;

            case GTL_SITESTATE_DPAT:
                /* Check value against dynamic PAT limits */
                if(lTestItem->nPatDynamicBinning != -1)
                {
                    if(	(((lTestItem->uiTestFlags & GTL_TFLAG_HAS_LL1) && (lTestResult >= lTestItem->fLowLimit1_Dynamic)) &&
                            ((lTestItem->uiTestFlags & GTL_TFLAG_HAS_HL1) && (lTestResult <= lTestItem->fHighLimit1_Dynamic))
                        ) ||
                        ( ((lTestItem->uiTestFlags & GTL_TFLAG_HAS_LL2) && (lTestResult >= lTestItem->fLowLimit2_Dynamic)) &&
                            ((lTestItem->uiTestFlags & GTL_TFLAG_HAS_HL2) && (lTestResult <= lTestItem->fHighLimit2_Dynamic))
                        )
                      )
                    {
                        /* Test within its limits, do nothing! */
                    }
                    else
                    {
                        gtl_stGlobalInfo.pnPatBinning[Site->mSiteNb] = lTestItem->nPatDynamicBinning;
                        (lTestItem->mTestResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mFlags |= GTC_TRFLAG_DPAT_OUTLIER;
                        (*Outliers)++;
                        *PatBinType='D';
                        int lR=gtl_OutputNewOutlieredTest(lTestItem, lTestResult, *PatBinType);
                        if (lR!=0)
                            GTL_LOG(3, "gtl_OutputNewOutlieredTest failed : %d", lR);
                    }

                }
                break;
        }
    }

    return 1;
}

/*======================================================================================*/
/* Description  : reset test result structure.											*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_ResetTestResult()
{
    unsigned int uiNbSites, uiNbTests, uiNbRuns;
    unsigned int uiSiteIndex, uiTestIndex, uiRunIndex;
    unsigned long ulSiteIndex, ulTestIndex, ulIndex;

	/* Check sizes */
	uiNbSites = gtl_stStation.uiNbSites;
	uiNbTests = gtl_stGlobalInfo.uiNbTests;
	uiNbRuns = gtl_stGlobalInfo.uiResultFrequency;
	ulIndex = uiNbSites*uiNbTests*uiNbRuns;
	if(ulIndex == 0)
    {
        GTL_LOG(3, "Zero index : NbSites(%d)*NbTests(%d)*NbRuns(%d)=0", uiNbSites, uiNbTests, uiNbRuns);
        return 0;
    }

	/* Check data ptrs */
    if(!gtl_stResults.mTestResults || !gtl_stResults.mRunResults || !gtl_stResults.mTestStats)
		return 0;

	/* Reset data */
	/* Test results */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ulSiteIndex = uiSiteIndex*uiNbTests*uiNbRuns;
		for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
		{
			ulTestIndex = ulSiteIndex + uiTestIndex*uiNbRuns;
			for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
			{
                gtl_stResults.mTestResults[ulTestIndex + uiRunIndex].mValue = GTL_INVALID_VALUE_FLOAT;
                gtl_stResults.mTestResults[ulTestIndex + uiRunIndex].mFlags = 0;
            }
		}
	}

	/* Run results */
    gtl_stResults.mNbValidRuns = 0;
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ulSiteIndex = uiSiteIndex*uiNbRuns;
		for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
		{
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mOrgSoftBin = -1;
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mOrgHardBin = -1;
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mPatSoftBin = -1;
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mPatHardBin = -1;
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mNbTestsExecuted = 0;
            strcpy(gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mPartID, "n/a");
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mFlags = 0;
            gtl_stResults.mRunResults[ulSiteIndex + uiRunIndex].mPartIndex = 0;
        }
	}

	/* Test statistics */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ulSiteIndex = uiSiteIndex*uiNbTests;
		for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
		{
            gtl_stResults.mTestStats[ulSiteIndex + uiTestIndex].uiExecs = 0;
		}
	}

	return 1;
}

/*======================================================================================*/
/* Description  : reset test list.														*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_ResetTestlist()
{
    unsigned int uiSiteIndex, uiNbSites = gtl_stStation.uiNbSites, uiTestIndex;
    SiteItem *ptSite=0;
    TestItem *ptTestItem=0;

	/* Reset SiteList/TestList(s) */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		ptSite = gtl_tl_ptSiteArray + gtl_stStation.pnSiteNumbers[uiSiteIndex];
        if(ptSite && ptSite->mTestList != NULL)
		{
			/* Reset site info */
            ptSite->mCurrentSiteState = GTL_SITESTATE_BASELINE;
            ptSite->mFailCount = 0;
            ptSite->mFailCount_Pat = 0;
            ptSite->mNextTest_Find = 0;
            ptSite->mPartCount = 0;
            ptSite->mPassCount = 0;
            ptSite->mPendingEvents = 0;

			/* Reset testlist */
            for(uiTestIndex=0; uiTestIndex<ptSite->mNbTests; uiTestIndex++)
			{
                ptTestItem = ptSite->mTestList + uiTestIndex;
				/* Reset counters */
                ptTestItem->uiNbExecs = 0;
				ptTestItem->uiFailCount = 0;
				/* Reset dynamic limits */
                ptTestItem->fLowLimit1_Dynamic = ptTestItem->fLowLimit2_Dynamic = -GTL_INFINITE_VALUE_FLOAT;
                ptTestItem->fHighLimit1_Dynamic = ptTestItem->fHighLimit2_Dynamic = GTL_INFINITE_VALUE_FLOAT;
			}
		}
	}

	return 1;
}

/* case 7260: added per site reset, but finally not needed yet */
/*======================================================================================*/
/* Description  : reset test list.														*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_ResetSiteTestlist(const int lSiteNb)
{
    unsigned int uiTestIndex=0;
    SiteItem *ptSite=gtl_tl_ptSiteArray + lSiteNb;
    TestItem *ptTestItem=0;

    /* Reset SiteList/TestList(s) */
    if(ptSite && ptSite->mTestList != NULL)
    {
        /* Reset site info */
        ptSite->mCurrentSiteState = GTL_SITESTATE_BASELINE;
        ptSite->mFailCount = 0;
        ptSite->mFailCount_Pat = 0;
        ptSite->mNextTest_Find = 0;
        ptSite->mPartCount = 0;
        ptSite->mPassCount = 0;
        ptSite->mPendingEvents = 0;

        /* Reset testlist */
        for(uiTestIndex=0; uiTestIndex<ptSite->mNbTests; uiTestIndex++)
        {
            ptTestItem = ptSite->mTestList + uiTestIndex;
            /* Reset counters */
            ptTestItem->uiNbExecs = 0;
            ptTestItem->uiFailCount = 0;
            /* Reset dynamic limits */
            ptTestItem->fLowLimit1_Dynamic = ptTestItem->fLowLimit2_Dynamic = -GTL_INFINITE_VALUE_FLOAT;
            ptTestItem->fHighLimit1_Dynamic = ptTestItem->fHighLimit2_Dynamic = GTL_INFINITE_VALUE_FLOAT;
        }
    }

    return 1;
}

/*======================================================================================*/
/* Description  : set state for specified site.                                         */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none																	*/
/*======================================================================================*/
void gtl_tl_SetSiteState(SiteItem *lSite, const int lState)
{
    if(lSite)
        lSite->mCurrentSiteState=lState;
}

/*======================================================================================*/
/* Description  : set state for all sites.                                              */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none																	*/
/*======================================================================================*/
void gtl_tl_SetSiteState_All(const int lState)
{
    SiteItem  *ptSite = NULL;
    unsigned int uiSiteIndex, uiSiteNb, uiNbSites = gtl_stStation.uiNbSites;

    /* Go through all sites */
    for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
    {
        uiSiteNb = gtl_stStation.pnSiteNumbers[uiSiteIndex];
        ptSite = gtl_tl_ptSiteArray + uiSiteNb;
        gtl_tl_SetSiteState(ptSite, lState);
    }
}

/*======================================================================================*/
/* Description  : display info about a specific test.									*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none																	*/
/*======================================================================================*/
void gtl_tl_DisplayTest(lTestNb)
long lTestNb;
{
    TestItem                *ptTestItem = NULL;
    PT_GNM_TESTSTAT_RESULTS pstTestStats=0;
    unsigned int            uiSiteIndex, uiSiteNb, uiNbSites = gtl_stStation.uiNbSites;
    char                    szString[GTC_MAX_STRING];
    int                     nTestFound;
	
	/* Go through all sites */
	for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		nTestFound = 0;
		uiSiteNb = gtl_stStation.pnSiteNumbers[uiSiteIndex];

		/* Find all tests with specified test number in current site */
        ptTestItem = gtl_tl_FindTestBySiteNb(uiSiteNb, lTestNb, NULL, -1, GTC_FINDTEST_TESTNUMBER, GTL_TL_FIND_REWINDFIRST);
		while(ptTestItem)
		{
			nTestFound = 1;

			/* Display all test information */
			pstTestStats = ptTestItem->pstTestStats;

            sprintf(szString, "**** GTL TEST: TEST %6ld, PIN %d, SITE %3d *******************************************",
                    lTestNb, ptTestItem->mPinIndex, uiSiteNb);
			gtl_msg_DebugMacro(1, 1, szString);
			/* szTestName */
			sprintf(szString, "szTestName             : %s", ptTestItem->szTestName);
			gtl_msg_DebugMacro(1, 1, szString);
			/* uiTestIndex */
			sprintf(szString, "uiTestIndex            : %d", ptTestItem->uiTestIndex);
			gtl_msg_DebugMacro(1, 1, szString);
			/* uiTestFlags */
			sprintf(szString, "uiTestFlags            : %d", ptTestItem->uiTestFlags);
			gtl_msg_DebugMacro(1, 1, szString);
			sprintf(szString, "                       :");
			if(ptTestItem->uiTestFlags == 0)
				strcat(szString, " <none>");
			else
			{
                if(ptTestItem->uiTestFlags & GTL_TFLAG_HAS_LL1)
                    strcat(szString, " GTL_TFLAG_HAS_LL1");
                if(ptTestItem->uiTestFlags & GTL_TFLAG_HAS_HL1)
                    strcat(szString, " GTL_TFLAG_HAS_HL1");
                if(ptTestItem->uiTestFlags & GTL_TFLAG_HAS_LL2)
                    strcat(szString, " GTL_TFLAG_HAS_LL2");
                if(ptTestItem->uiTestFlags & GTL_TFLAG_HAS_HL2)
                    strcat(szString, " GTL_TFLAG_HAS_HL2");
			}
			gtl_msg_DebugMacro(1, 1, szString);
			/* fLowLimit_Static */
			sprintf(szString, "fLowLimit_Static       : %g", ptTestItem->fLowLimit_Static);
			gtl_msg_DebugMacro(1, 1, szString);
			/* fHighLimit_Static */
			sprintf(szString, "fHighLimit_Static      : %g", ptTestItem->fHighLimit_Static);
			gtl_msg_DebugMacro(1, 1, szString);
			/* fLowLimit1_Dynamic */
			sprintf(szString, "fLowLimit1_Dynamic     : %g", ptTestItem->fLowLimit1_Dynamic);
			gtl_msg_DebugMacro(1, 1, szString);
			/* fHighLimit1_Dynamic */
			sprintf(szString, "fHighLimit1_Dynamic    : %g", ptTestItem->fHighLimit1_Dynamic);
			gtl_msg_DebugMacro(1, 1, szString);
			/* fLowLimit2_Dynamic */
			sprintf(szString, "fLowLimit2_Dynamic     : %g", ptTestItem->fLowLimit2_Dynamic);
			gtl_msg_DebugMacro(1, 1, szString);
			/* fHighLimit2_Dynamic */
			sprintf(szString, "fHighLimit2_Dynamic    : %g", ptTestItem->fHighLimit2_Dynamic);
			gtl_msg_DebugMacro(1, 1, szString);
			/* nPatStaticBinning */
			sprintf(szString, "nPatStaticBinning      : %d", ptTestItem->nPatStaticBinning);
			gtl_msg_DebugMacro(1, 1, szString);
			/* nPatDynamicBinning */
			sprintf(szString, "nPatDynamicBinning     : %d", ptTestItem->nPatDynamicBinning);
			gtl_msg_DebugMacro(1, 1, szString);
			/* uiNbExecs */
			sprintf(szString, "uiNbExecs              : %d", ptTestItem->uiNbExecs);
			gtl_msg_DebugMacro(1, 1, szString);
			sprintf(szString, "********************************************************************************");
			gtl_msg_DebugMacro(1, 1, szString);
			

            ptTestItem = gtl_tl_FindTestBySiteNb(uiSiteNb, lTestNb, NULL, -1, // todo : handle MPR
                                                    GTC_FINDTEST_TESTNUMBER, GTL_TL_FIND_NOREWIND);
		}

		/* Check if Test found in current site */
		if(!nTestFound)
		{
			sprintf(szString, "Test %ld not found in site %d !", lTestNb, uiSiteNb);
			gtl_msg_DebugMacro(1, 1, szString);
		}
	}
}

/*======================================================================================*/
/* Description  : display testlist.														*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none																	*/
/*======================================================================================*/
void gtl_tl_DisplayTestlist()
{
	TestItem			*ptTestItem = NULL;
	SiteItem			*ptSite = NULL;
	unsigned int		uiSiteIndex, uiSiteNb, uiNbSites = gtl_stStation.uiNbSites, uiTestIndex;
    char				szString[GTC_MAX_STRING], szTemp[GTC_MAX_STRING];
	char				szTestName[16];
	
	/* Go through all sites */
    for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
	{
		uiSiteNb = gtl_stStation.pnSiteNumbers[uiSiteIndex];
		ptSite = gtl_tl_ptSiteArray + uiSiteNb;

		sprintf(szString, "**** SITE %3d *****************************************************************", uiSiteNb);
		gtl_msg_DebugMacro(1, 1, szString);

        if(ptSite->mTestList != NULL)
		{
			/* Go through all tests */
            for(uiTestIndex=0; uiTestIndex<ptSite->mNbTests; uiTestIndex++)
			{
                ptTestItem = ptSite->mTestList + uiTestIndex;
                gtcnm_CopyString(szTestName, ptTestItem->szTestName, 15);
				if(strlen(ptTestItem->szTestName) > 15)
				{
					szTestName[12] = szTestName[13] = szTestName[14] = '.';
				}
                sprintf(szString,	"Test %6ld pin %d (%15s): ", ptTestItem->lTestNumber, ptTestItem->mPinIndex, szTestName);
                if(ptTestItem->fLowLimit_Static == -GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, "SLPL=n/a");
                else
                    sprintf(szTemp, "SLPL=%.2g", ptTestItem->fLowLimit_Static);
                strcat(szString, szTemp);
                if(ptTestItem->fHighLimit_Static == GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, ", SUPL=n/a");
                else
                    sprintf(szTemp, ", SUPL=%.2g", ptTestItem->fHighLimit_Static);
                strcat(szString, szTemp);
                if(ptTestItem->fLowLimit1_Dynamic == -GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, ", DLPL1=n/a");
                else
                    sprintf(szTemp, ", DLPL1=%.2g", ptTestItem->fLowLimit1_Dynamic);
                strcat(szString, szTemp);
                if(ptTestItem->fHighLimit1_Dynamic == GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, ", DUPL1=n/a");
                else
                    sprintf(szTemp, ", DUPL1=%.2g", ptTestItem->fHighLimit1_Dynamic);
                strcat(szString, szTemp);
                if(ptTestItem->fLowLimit2_Dynamic == -GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, ", DLPL2=n/a");
                else
                    sprintf(szTemp, ", DLPL2=%.2g", ptTestItem->fLowLimit2_Dynamic);
                strcat(szString, szTemp);
                if(ptTestItem->fHighLimit2_Dynamic == GTL_INFINITE_VALUE_FLOAT)
                    sprintf(szTemp, ", DUPL2=n/a");
                else
                    sprintf(szTemp, ", DUPL2=%.2g", ptTestItem->fHighLimit2_Dynamic);
                strcat(szString, szTemp);
				gtl_msg_DebugMacro(1, 1, szString);
			}
		}

		sprintf(szString, "********************************************************************************");
		gtl_msg_DebugMacro(1, 1, szString);
	}
}

/*======================================================================================*/
/* Description  : free memory used by testlist(s).										*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_tl_Free()
{
    unsigned int uiSiteNb=0;

	/* Free SiteList/TestList(s) */
	for(uiSiteNb=0; uiSiteNb<256; uiSiteNb++)
	{
        if(gtl_tl_ptSiteArray[uiSiteNb].mTestList != NULL)
		{
            free(gtl_tl_ptSiteArray[uiSiteNb].mTestList);
            gtl_tl_ptSiteArray[uiSiteNb].mTestList = NULL;
		}
        gtl_tl_ptSiteArray[uiSiteNb].mRunResults = NULL;
        gtl_tl_ptSiteArray[uiSiteNb].mSiteIndex = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mSiteNb = uiSiteNb;
        gtl_tl_ptSiteArray[uiSiteNb].mPartCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mPassCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mFailCount = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mFailCount_Pat = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mNextTest_Find = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mNbTests = 0;
        gtl_tl_ptSiteArray[uiSiteNb].mCurrentSiteState = GTL_SITESTATE_BASELINE;
        gtl_tl_ptSiteArray[uiSiteNb].mPendingEvents = 0;
    }

    /* Free Test result structure */
	gtcnm_FreeStruct(GNM_TYPE_RESULTS, &gtl_stResults);
	gtcnm_InitStruct(GNM_TYPE_RESULTS, &gtl_stResults);

	/* Reset global pointers */

    return GTL_CORE_ERR_OKAY; // was 1 but nobidy was checking returned code...
}

/*======================================================================================*/
/* Private CODE                                                                         */
/*======================================================================================*/

//  Description  : find specific test in the TestList
//    Return Codes : ptr on Test if successful, NULL else
TestItem *gtl_tl_FindTestBySiteNb(unsigned int uiSiteNb, long lTestNb, char* szTestName, int lPinIndex,
                                  unsigned int uiFindTestKey, int nFindMode)
{
    if (uiSiteNb>255) //gtl_stStation.uiNbSites)
    {
        GTL_LOG(4, "Illegal site nb %d requested", uiSiteNb);
        return NULL;
    }
    SiteItem *ptSite = gtl_tl_ptSiteArray + uiSiteNb;
    if (!ptSite)
        return 0;
    TestItem *ptTestItem=0;
    unsigned int uiNbTests=0;
    unsigned int uiTestIndex=0;

	/* Check if TestList available */
    if(ptSite->mTestList == NULL)
		return NULL;

	/* If no valid test name provided, force to find by test number */
	if((szTestName == NULL) || (*szTestName == '\0'))
		uiFindTestKey = GTC_FINDTEST_TESTNUMBER;

	/* Check if we should rewind before the search */
	if(nFindMode == GTL_TL_FIND_REWINDFIRST)
        ptSite->mNextTest_Find = 0;

	/* Find index of specified test */
    uiNbTests = ptSite->mNbTests;
    for(uiTestIndex=ptSite->mNextTest_Find; uiTestIndex<uiNbTests; uiTestIndex++)
	{
        ptTestItem = ptSite->mTestList + uiTestIndex;
        if (ptTestItem->mPinIndex!=lPinIndex)
            continue;
		switch(uiFindTestKey)
		{
			case GTC_FINDTEST_TESTNAME:
				if(!strcmp(ptTestItem->szTestName, szTestName))
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;

			case GTC_FINDTEST_TESTNUMBERANDNAME:
				if((ptTestItem->lTestNumber == lTestNb) && !strcmp(ptTestItem->szTestName, szTestName))
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;

			case GTC_FINDTEST_TESTNUMBER:
			default:
				if(ptTestItem->lTestNumber == lTestNb)
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;
		}
	}

	/* Check if we should rewind and continue the search from start of the list */
	if(nFindMode != GTL_TL_FIND_REWINDATEND)
		return NULL;

	/* Couldn't find test starting from next test position, try starting from the start */
    for(uiTestIndex=0; uiTestIndex<ptSite->mNextTest_Find; uiTestIndex++)
	{
        ptTestItem = ptSite->mTestList + uiTestIndex;

        if (ptTestItem->mPinIndex!=lPinIndex)
            continue;

		switch(uiFindTestKey)
		{
			case GTC_FINDTEST_TESTNAME:
				if(!strcmp(ptTestItem->szTestName, szTestName))
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;

			case GTC_FINDTEST_TESTNUMBERANDNAME:
				if((ptTestItem->lTestNumber == lTestNb) && !strcmp(ptTestItem->szTestName, szTestName))
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;

			case GTC_FINDTEST_TESTNUMBER:
			default:
				if(ptTestItem->lTestNumber == lTestNb)
				{
                    ptSite->mNextTest_Find = uiTestIndex + 1;
					return ptTestItem;
				}
				break;
		}
	}

	/* Test not found */
	return NULL;
}

/*======================================================================================*/
/* Description  : find specific test in the TestList.									*/
/* Return Codes : ptr on Test if successful, NULL else									*/
TestItem *gtl_tl_FindTestBySiteIndex(unsigned int uiSiteIndex, long lTestNb, char* szTestName, int lPinIndex,
                                     unsigned int uiFindTestKey, int nFindMode)
{
    return gtl_tl_FindTestBySiteNb(gtl_stStation.pnSiteNumbers[uiSiteIndex], lTestNb, szTestName, lPinIndex,
                                   uiFindTestKey, nFindMode);
}

/*======================================================================================*/
/*
    Description  : return current SPAT limits if any.
    Return Codes : GTL_CORE_ERR_OKAY if successful, GTL_CORE_ERR_... else
*/
int gtl_get_spat_limits(SiteNb, TestNumbers, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArrayTotalSize, ArrayFilledSize)
unsigned int SiteNb;
long* TestNumbers;
unsigned int* Flags;
double* HighLimits;
double* LowLimits;
unsigned int* HardBins;
unsigned int* SoftBins;
unsigned int ArrayTotalSize;
unsigned int* ArrayFilledSize;
{
    unsigned int    lTestIndex=0;
    SiteItem*       lSite = gtl_tl_ptSiteArray + SiteNb;
    TestItem*       lTestItem = NULL;
    char            lValue[1024];

    // Set ArrayFilledSize to 0, we'll set it to the right value in case limits are returned
    *ArrayFilledSize=0;

    // Check pointers
    if((TestNumbers == 0) || (Flags == 0) || (HighLimits == 0) || (LowLimits == 0) || (ArrayTotalSize == 0))
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    // Make sure arrays are big enough
    if(ArrayTotalSize < gtl_stGlobalInfo.uiNbTests)
        return GTL_CORE_ERR_ARRAY_SIZE;

    // Get site ptr
    if(!lSite)
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;

    // Check if SPAT limits recieved since last get OR key set to always return limits
    if((lSite->mPendingEvents & GTL_EVENT_NEW_SPAT_LIMITS) || !strcmp(gtl_stGlobalInfo.mLimitsGetCondition, "always"))
    {
        // Fill array with SPAT limits for all tests
        if(lSite->mTestList != NULL)
        {
            /* Go through all tests */
            for(lTestIndex=0; lTestIndex<lSite->mNbTests; lTestIndex++)
            {
                lTestItem = lSite->mTestList + lTestIndex;
                // For the moment, support PTEST only (no MPTEST)
                if(lTestItem->mPinIndex == -1)
                {
                    // Make sure test has some valid limits
                    if( (lTestItem->nPatStaticBinning != -1) &&
                        (   (lTestItem->fLowLimit_Static != -GTL_INFINITE_VALUE_FLOAT) ||
                            (lTestItem->fHighLimit_Static != GTL_INFINITE_VALUE_FLOAT)
                        )
                      )
                    {
                        TestNumbers[lTestIndex] = lTestItem->lTestNumber;
                        Flags[lTestIndex] = lTestItem->uiTestFlags;
                        LowLimits[lTestIndex] = lTestItem->fLowLimit_Static;
                        HighLimits[lTestIndex] = lTestItem->fHighLimit_Static;
                        SoftBins[lTestIndex] = lTestItem->nPatStaticBinning;
                        HardBins[lTestIndex] = lTestItem->nPatStaticBinning;
                        (*ArrayFilledSize)++;
                    }
                }
            }
        }

        // Reset pending event
        lSite->mPendingEvents &= ~GTL_EVENT_NEW_SPAT_LIMITS;
    }

    return GTL_CORE_ERR_OKAY;
}

/*======================================================================================*/
/*
    Description  : return current DPAT limits if any.
    Return Codes : GTL_CORE_ERR_OKAY if successful, GTL_CORE_ERR_... else
*/
int gtl_get_dpat_limits(SiteNb, TestNumbers, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArrayTotalSize, ArrayFilledSize)
unsigned int SiteNb;
long* TestNumbers;
unsigned int* Flags;
double* LowLimits;
double* HighLimits;
unsigned int* HardBins;
unsigned int* SoftBins;
unsigned int ArrayTotalSize;
unsigned int* ArrayFilledSize;
{
    unsigned int    lTestIndex=0;
    SiteItem*       lSite = gtl_tl_ptSiteArray + SiteNb;
    TestItem*       lTestItem = NULL;
    char            lValue[1024];

    // Set ArrayFilledSize to 0, we'll set it to the right value in case limits are returned
    *ArrayFilledSize=0;

    // Check pointers
    if((TestNumbers == 0) || (Flags == 0) || (HighLimits == 0) || (LowLimits == 0) || (ArrayTotalSize == 0))
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    // Make sure arrays are big enough
    if(ArrayTotalSize < gtl_stGlobalInfo.uiNbTests)
        return GTL_CORE_ERR_ARRAY_SIZE;

    // Get site ptr
    if(!lSite)
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;

    // Check if DPAT limits recieved since last get OR key set to always return limits
    if((lSite->mPendingEvents & GTL_EVENT_NEW_DPAT_LIMITS) || !strcmp(gtl_stGlobalInfo.mLimitsGetCondition, "always"))
    {
        // Fill array with DPAT limits for all tests
        if(lSite->mTestList != NULL)
        {
            /* Go through all tests */
            for(lTestIndex=0; lTestIndex<lSite->mNbTests; lTestIndex++)
            {
                lTestItem = lSite->mTestList + lTestIndex;
                // For the moment, support PTEST only (no MPTEST)
                if(lTestItem->mPinIndex == -1)
                {
                    // Make sure test has some valid limits
                    if( (lTestItem->nPatDynamicBinning != -1) &&
                        (   (lTestItem->fLowLimit1_Dynamic != -GTL_INFINITE_VALUE_FLOAT) ||
                            (lTestItem->fHighLimit1_Dynamic != GTL_INFINITE_VALUE_FLOAT)
                        )
                      )
                    {
                        // For the moment, in case of bi-modal distribution, return largest limits
                        TestNumbers[lTestIndex] = lTestItem->lTestNumber;
                        Flags[lTestIndex] = lTestItem->uiTestFlags;
                        LowLimits[lTestIndex] = lTestItem->fLowLimit1_Dynamic<lTestItem->fLowLimit2_Dynamic ?
                                    lTestItem->fLowLimit1_Dynamic:lTestItem->fLowLimit2_Dynamic;
                        HighLimits[lTestIndex] = lTestItem->fHighLimit1_Dynamic>lTestItem->fHighLimit2_Dynamic ?
                                    lTestItem->fHighLimit1_Dynamic:lTestItem->fHighLimit2_Dynamic;
                        SoftBins[lTestIndex] = lTestItem->nPatDynamicBinning;
                        HardBins[lTestIndex] = lTestItem->nPatDynamicBinning;
                        (*ArrayFilledSize)++;
                    }
                }
            }
        }

        // Reset pending event
        lSite->mPendingEvents &= ~GTL_EVENT_NEW_DPAT_LIMITS;
    }

    return GTL_CORE_ERR_OKAY;
}


