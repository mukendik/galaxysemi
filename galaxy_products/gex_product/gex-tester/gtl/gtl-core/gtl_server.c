/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_server.c                                                                 */
/*                                                                                      */
/* Functions to communicate with GTM (Galaxy Tester Monitor) server.                    */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_SERVER_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h> // also available in mingw (thanks mingw)
#include <gstdl_utils_c.h>

#include "gtl_core.h"
#include "gtl_server.h"
#include "gtl_constants.h"
#include "gtl_testlist.h"
#include "gtl_profile.h"

// Packet number : initialize at 0. Incremented at each gtl_svr_Results.
// Number of packets (with n runs) that GTL send to server.
unsigned int uiPacketNb = 0;

/*======================================================================================*/
/* Externs                                                                              */
/*======================================================================================*/
/* From gtl_main.c */
extern char gtl_szApplicationFullName[];
extern GTL_Station gtl_stStation;					/* Tester station information */
extern GTL_ProdInfo	gtl_stProdInfo;					/* Tester Production information */
extern GTL_GlobalInfo gtl_stGlobalInfo;				/* GTL Global information */
extern GNM_RESULTS gtl_stResults;					/* Test result structure */
extern int gtl_OutputRecipe(char*, char*);
/* From gtl_testlist.c */
extern SiteItem	gtl_tl_ptSiteArray[];               /* Array of sites */

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/

/*======================================================================================*/
/* Public declarations                                                                  */
/*======================================================================================*/

/*======================================================================================*/
/* Private declarations                                                                 */
/*======================================================================================*/
/* Variables */

/* Functions */

/*======================================================================================*/
/* Public CODE                                                                          */

int gtl_svr_Init(PT_GNM_R_INIT *ppstInit_Reply, char *szFullRecipeFileName)
{
    int	nStatus=0, lBytesRead=0;
    long lFileSize=0;
    GTL_SOCK_MSG stQueryMessage, stReplyMessage;
    GNM_Q_INIT stInit_Query;
    unsigned int uiIndex=0;
    FILE *recipeFile=NULL;
    // char *lReadBuffer=NULL; // case 7266 : replaced by gtl_stGlobalInfo.mRecipe

    // Read recipe file only the first time
    if(szFullRecipeFileName && (strlen(szFullRecipeFileName)>0) && gtl_stGlobalInfo.mRecipe==0 )
    {
        // Get size of the file to read
        if((lFileSize = (long)ut_GetFileSize(szFullRecipeFileName)) == -1)
            return GTL_CORE_ERR_CANT_STAT_RECIPE; // was 0

        GTL_LOG(7, "Recipe file size as reported by stat is %d", lFileSize, 0);

        // Open file
        recipeFile = fopen(szFullRecipeFileName, "rb");
        if(!recipeFile)
            return GTL_CORE_ERR_CANT_OPEN_RECIPE; // was 0

        // Get file size (cross platform)
        struct stat lStat;
        int lRes=fstat( fileno(recipeFile), &lStat);
        GTL_LOG(5, "Recipe fstat returned %d: size=%ld", lRes, lStat.st_size);
        if (lRes!=0)
            GTL_LOG(4, "fstat failed: errno=%d", errno);

        // under mingw48 : #define fstat64  _fstat64 /* for POSIX */
        //Impossible : will provoke _fstat64 unfindable in msvcrt.dll under WinNT
        //struct stat64 lStat64;
        //lRes=fstat64(recipeFile, &lStat64);
        //GTL_LOG(5, "Recipe fstat64 returned %d: size=%ld", lRes, lStat64.st_size);

        lRes=fseek(recipeFile, 0, SEEK_END);
        if (lRes!=0)
            GTL_LOG(4, "Recipe fseek failed: %d", lRes);
        lFileSize = ftell(recipeFile);
        rewind(recipeFile);

        GTL_LOG(6, "Recipe file size as reported by ftell is %d", lFileSize, 0);

        // Allocate read buffer
        //lReadBuffer = (char*)malloc((lFileSize+1)*sizeof(char)); // 7266
        if (gtl_stGlobalInfo.mRecipe) // perhaps already allocated in a previous session not closed.
        {
            free(gtl_stGlobalInfo.mRecipe);
            gtl_stGlobalInfo.mRecipe=0;
        }
        gtl_stGlobalInfo.mRecipe = (char*)malloc((lFileSize+1)*sizeof(char)); // 7266
        memset(gtl_stGlobalInfo.mRecipe, '\0', (lFileSize+1)*sizeof(char));
        if(!gtl_stGlobalInfo.mRecipe)
        {
            GTL_LOG(2, "Cannot malloc %d octet for recipe", (lFileSize+1)*sizeof(char) );
            fclose(recipeFile);
            return GTL_CORE_ERR_MALLOC_FAILED;
        }

        /* Read file to buffer */
        lBytesRead = fread(gtl_stGlobalInfo.mRecipe, sizeof(char), lFileSize, recipeFile);
        if(lBytesRead != lFileSize)
        {
            free(gtl_stGlobalInfo.mRecipe);
            gtl_stGlobalInfo.mRecipe=0;
            fclose(recipeFile);
            return GTL_CORE_ERR_CRRF; //0;
        }
        fclose(recipeFile);

        // Make really sure the string has the right size
        if(strlen(gtl_stGlobalInfo.mRecipe) != lFileSize)
        {
            free(gtl_stGlobalInfo.mRecipe);
            gtl_stGlobalInfo.mRecipe=0;
            return GTL_CORE_ERR_CRRF; //0;
        }

        // Insert the recipe in the GTL output right now ? Why not ?
        int lR=gtl_OutputRecipe(szFullRecipeFileName, gtl_stGlobalInfo.mRecipe);
        if (lR!=0)
        {
            GTL_LOG(3, "gtl_OutputRecipe returned: %d", lR);
            return lR;
        }
    }

	/* Start socket system */
    /* BG 20130806: connection and communication mode now set through grl_set() command, checked from inside */
    /*              gtl_sock_Start() */
    nStatus = gtl_sock_Start(0, 0); // 7266 : NumOfServerstoTry: 0=all
	if(nStatus != GTL_SOCK_ERR_OKAY)
    {
        // was returning GTL_SOCK_ERR_CONNECT if max connection reached...
        GTL_LOG(4, "gtl_sock_Start failed: %d", nStatus);
        if (nStatus==GTL_SOCK_ERR_CONNECT_MAXCONN)
            return GTL_CORE_ERR_GTM_MAXCONN_REACHED;
        else if(nStatus==GTL_SOCK_ERR_VERSIONMISMATCH)
            return GTL_CORE_ERR_VERSION_MISMATCH;
        else if(nStatus==GTL_SOCK_ERR_NO_GTL_LIC)
            return GTL_CORE_ERR_GTL_NOLIC;
        else if(nStatus==GTL_SOCK_ERR_NO_GTM_LIC)
            return GTL_CORE_ERR_GTM_NOLIC;
        return GTL_CORE_ERR_GTM_INIT; //0;
    }

    /* Initialize Init structure */
	gtcnm_InitStruct(GNM_TYPE_Q_INIT, &stInit_Query);

    /* Fill Init structure */
    stInit_Query.mStationNb = gtl_stStation.uiStationNb;
    stInit_Query.mNbSites = gtl_stStation.uiNbSites;
    GTL_LOG(5, "Number of sites: %d", stInit_Query.mNbSites);

    for(uiIndex=0; uiIndex<256; ++uiIndex)
    {
        stInit_Query.mSiteNumbers[uiIndex] = gtl_stStation.pnSiteNumbers[uiIndex];
        //if (stInit_Query.mSiteNumbers[uiIndex]>-1)
          //  GTL_LOG(5, "Site '%d' discovered", stInit_Query.mSiteNumbers[uiIndex]);
    }

    for(uiIndex=0; uiIndex<256; ++uiIndex)
    {
        stInit_Query.mSitesStates[uiIndex] = gtl_tl_ptSiteArray[uiIndex].mCurrentSiteState;
        if (stInit_Query.mSitesStates[uiIndex]==GTL_SITESTATE_DPAT)
        //if (stInit_Query.mSitesStates[uiIndex]>=0)
            GTL_LOG(5, "Site %d is already in DPAT state", uiIndex);
    }

    strcpy(stInit_Query.mGtlVersion, gtl_szApplicationFullName);
    GTL_LOG(6, "Sending GTL version: '%s'", stInit_Query.mGtlVersion);
    GTL_LOG(6, "Sending misc node info...", 0);

    if (gtl_stStation.szHostID)
        strcpy(stInit_Query.mHostID, gtl_stStation.szHostID);
    strcpy(stInit_Query.mNodeName, gtl_stStation.szNodeName);
    strcpy(stInit_Query.mUserName, gtl_stStation.szUserName);
    strcpy(stInit_Query.mTesterExec, gtl_stStation.szTesterExecType);
    strcpy(stInit_Query.mTesterType, gtl_stStation.szTesterType);
    strcpy(stInit_Query.mTestJobName, gtl_stStation.szTestJobName);
    strcpy(stInit_Query.mTestJobPath, gtl_stStation.szTestJobPath);
    strcpy(stInit_Query.mTestSourceFilesPath, gtl_stStation.szTestSourceFilesPath);
    if(gtl_stGlobalInfo.mRecipe)
    {
        GTL_LOG(6, "gtl_svr_Init: recipe length %d", strlen(gtl_stGlobalInfo.mRecipe));
        stInit_Query.mRecipeBuffer = gtl_stGlobalInfo.mRecipe;   //lReadBuffer;
    }

	/* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_Q_INIT;
    stQueryMessage.mMsgStruct = (void *)&stInit_Query;

    GTL_LOG(5, "Sending INIT message...", 0);
	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, &stReplyMessage, GNM_TYPE_R_INIT);

	if(nStatus != GNET_ERR_OKAY)
    {
        GTL_LOG(3, "gtl_sock_SendMessage failed: %d", nStatus);
        return GTL_CORE_ERR_GTM_INIT; //0;
    }

    /* Set ptr to REPLY structure */
    *ppstInit_Reply = (PT_GNM_R_INIT)stReplyMessage.mMsgStruct;

    return 0; // was 1 !
}

/*======================================================================================*/
/* Description  : send Production info to GTM server.									*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_svr_SendProdInfo()
{
    int	nStatus=0;
    GTL_SOCK_MSG stMessage;
    GNM_PRODINFO stProdInfo;

	/* Initialize ProdInfo structure */
	gtcnm_InitStruct(GNM_TYPE_PRODINFO, &stProdInfo);

	/* Fill ProdInfo structure */
	strcpy(stProdInfo.szOperatorName, gtl_stProdInfo.szOperatorName);
	strcpy(stProdInfo.szJobRevision, gtl_stProdInfo.szJobRevision);
	strcpy(stProdInfo.szLotID, gtl_stProdInfo.szLotID);
	strcpy(stProdInfo.szSublotID, gtl_stProdInfo.szSublotID);
	strcpy(stProdInfo.szProductID, gtl_stProdInfo.szProductID);
    stProdInfo.mSplitlotID=gtl_stProdInfo.mSplitlotID;
    if (stProdInfo.mSplitlotID<1)
        GTL_LOG(4, "Illegal/abnormal splitlot ID: %d", stProdInfo.mSplitlotID);
    stProdInfo.mRetestIndex=gtl_stProdInfo.mRetestIndex;

	/* Socket message structure */
    stMessage.mType = GNM_TYPE_PRODINFO;
    stMessage.mMsgStruct = (void *)&stProdInfo;

    GTL_LOG(5, "Sending prod info...", 0)

    /* Send message to server */
    nStatus = gtl_sock_SendMessage(&stMessage, NULL, 0);
    if(nStatus != GNET_ERR_OKAY)
		return 0;

	return 1;
}

/*======================================================================================*/
/* Description  : get TestList from GTM server.											*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_svr_InitTestList(PT_GNM_R_TESTLIST *ppstTestList_Reply)
{
    char szTimestamp[UT_MAX_TIMESTAMP_LEN]="";
    int nStatus=0;
    GTL_SOCK_MSG stQueryMessage, stReplyMessage;
    GNM_Q_TESTLIST stTestlist_Query;

	/* Initialize PatConfig Query structure */
	gtcnm_InitStruct(GNM_TYPE_Q_TESTLIST, &stTestlist_Query);

	/* Fill PatConfig Query structure */
	stTestlist_Query.uiTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);;

	/* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_Q_TESTLIST;
    stQueryMessage.mMsgStruct = (void *)&stTestlist_Query;

    GTL_LOG(5, "Sending test list query...", 0);
	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, &stReplyMessage, GNM_TYPE_R_TESTLIST);
	if(nStatus != GNET_ERR_OKAY)
		return 0;

    if (!stReplyMessage.mMsgStruct)
        GTL_LOG(3, "Received a null message", 0);

	/* Set ptr to TESTLIST structure */ 
    *ppstTestList_Reply = (PT_GNM_R_TESTLIST)stReplyMessage.mMsgStruct;
	
	return 1;
}

/*======================================================================================*/
/* Description  : get PAT settings from GTM server.                                     */
/* Return Codes : 1 if successful, 0 else                                               */
int gtl_svr_InitPat(PT_GNM_R_PATCONFIG_STATIC *ppstStaticPatConfig_Reply,
                    PT_GNM_PATCONFIG_DYNAMIC *ppstDynamicPatConfig_Reply)
{
    char szTimestamp[UT_MAX_TIMESTAMP_LEN]="";
    int nStatus=0;
    GTL_SOCK_MSG stQueryMessage, stReplyMessage;

    // FIRST get static PAT settings
    GNM_Q_PATCONFIG_STATIC stStaticPatConfig_Query;

	/* Initialize PatConfig Query structure */
	gtcnm_InitStruct(GNM_TYPE_Q_PATCONFIG_STATIC, &stStaticPatConfig_Query);

	/* Fill PatConfig Query structure */
	stStaticPatConfig_Query.uiTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);;

	/* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_Q_PATCONFIG_STATIC;
    stQueryMessage.mMsgStruct = (void *)&stStaticPatConfig_Query;

    GTL_LOG(5, "Sending PATCONFIG_STATIC query...", 0)
	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, &stReplyMessage, GNM_TYPE_R_PATCONFIG_STATIC);
	if(nStatus != GNET_ERR_OKAY)
		return 0;

	/* Set ptr to static PAT config structure */ 
    *ppstStaticPatConfig_Reply = (PT_GNM_R_PATCONFIG_STATIC)stReplyMessage.mMsgStruct;

    // SECOND get dynamic PAT settings
    GNM_Q_PATCONFIG_DYNAMIC stDynamicPatConfig_Query;

    /* Initialize PatConfig Query structure */
    gtcnm_InitStruct(GNM_TYPE_Q_PATCONFIG_DYNAMIC, &stDynamicPatConfig_Query);

    /* Fill PatConfig Query structure */
    stDynamicPatConfig_Query.uiTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);;

    /* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_Q_PATCONFIG_DYNAMIC;
    stQueryMessage.mMsgStruct = (void *)&stDynamicPatConfig_Query;

    GTL_LOG(5, "Sending PATCONFIG_DYNAMIC query...", 0)
    /* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, &stReplyMessage, GNM_TYPE_PATCONFIG_DYNAMIC);
    if(nStatus != GNET_ERR_OKAY)
        return 0;

    /* Set ptr to static PAT config structure */
    *ppstDynamicPatConfig_Reply = (PT_GNM_PATCONFIG_DYNAMIC)stReplyMessage.mMsgStruct;

    return 1;
}

/* ======================================================================================
  Description : Sending test result to the server ?
  Argument(s) :
*/
int gtl_svr_Results(unsigned int uiSilentMode)
{
    int nStatus=0;
    GTL_SOCK_MSG stMessage;

    //GTL_LOG(7, "Sending test results...", 0) // too much logs kill logs...

    /* Socket message structure */
    stMessage.mType = GNM_TYPE_RESULTS;
    gtl_stResults.mSilentMode = uiSilentMode;
    gtl_stResults.mPacketNb = uiPacketNb++;
    stMessage.mMsgStruct = (void*)&gtl_stResults;

    /* GS_QA: dump results buffer */
    char* lQaOutput=getenv("GS_QA_OUTPUT_FOLDER");
    if(getenv("GS_QA") && lQaOutput)
    {
        /* Dump received information */
        unsigned int uiNbSites = gtl_stResults.mNbSites;
        unsigned int uiNbTests = gtl_stResults.mNbTests;
        unsigned int uiNbRuns = gtl_stResults.mNbAllocatedRuns;
        unsigned int uiNbValidRuns = gtl_stResults.mNbValidRuns;
        unsigned long ulRunResult_SiteIndex=0;
        unsigned int uiSiteIndex, uiTestIndex, uiRunIndex;
        unsigned long ulTestResult_SiteIndex, ulTestStat_SiteIndex, ulIndex;
        float fResult=0.f;
        FILE *lFile=NULL;
        char lFileName[GTC_MAX_PATH], lSiteString[10];

        /* Build output file name */
        strcpy(lFileName, lQaOutput);
        if((lFileName[strlen(lFileName)-1] != '/') && (lFileName[strlen(lFileName)-1] != '\\'))
            strcat(lFileName, "/");
        strcat(lFileName, "gs_gtl_socket_results_tx");
        if(uiNbSites == 1)
        {
            sprintf(lSiteString, "_s%d", gtl_stResults.mTestStats[0].mSiteNb);
            strcat(lFileName, lSiteString);
        }
        strcat(lFileName, ".csv");
        ut_NormalizePath(lFileName);

        /* Open output file */
        lFile=fopen(lFileName, "a+");
        if(lFile)
        {
            /* Header */
            fprintf(lFile, "#### RESULT packet %d ####\n", gtl_stResults.mPacketNb);
            fprintf(lFile, "Nb of sites,%d\n", uiNbSites);
            fprintf(lFile, "Nb of tests,%d\n", uiNbTests);
            fprintf(lFile, "Nb of allocated runs,%d\n", uiNbRuns);
            fprintf(lFile, "Nb of valid runs,%d\n\n", uiNbValidRuns);

            /* Site loop */
            for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
            {
                fprintf(lFile, "#### SITE,%d\n", gtl_stResults.mTestStats[uiSiteIndex*uiNbTests].mSiteNb);

                /* Original Software binning */
                fprintf(lFile, "Original Software binning,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                  fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mOrgSoftBin);
                fprintf(lFile, "\n");

                /* Original Hardware binning */
                fprintf(lFile, "Original Hardware binning,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                    fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mOrgHardBin);
                fprintf(lFile, "\n");

                /* PAT Software binning */
                fprintf(lFile, "PAT Software binning,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                  fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mPatSoftBin);
                fprintf(lFile, "\n");

                /* PAT Hardware binning */
                fprintf(lFile, "PAT Hardware binning,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                    fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mPatHardBin);
                fprintf(lFile, "\n");

                /* Part flags */
                fprintf(lFile, "Part flags,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                  fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mFlags);
                fprintf(lFile, "\n");

                /* Tests executed */
                fprintf(lFile, "Tests executed,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                  fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mNbTestsExecuted);
                fprintf(lFile, "\n");

                /* PartID */
                fprintf(lFile, "PartID,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                    fprintf(lFile, "%s ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mPartID);
                fprintf(lFile, "\n");

                /* Part Index */
                fprintf(lFile, "PartIndex,");
                ulRunResult_SiteIndex = uiSiteIndex*uiNbRuns;
                for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                  fprintf(lFile, "%d ", gtl_stResults.mRunResults[ulRunResult_SiteIndex+uiRunIndex].mPartIndex);
                fprintf(lFile, "\n\n");

                /* Header for test results */
                fprintf(lFile, "Test#,Pin#,TestName,Nb. results,(Result|Flags)x(Nb. results)\n");

                /* Test loop */
                ulTestResult_SiteIndex = uiSiteIndex*uiNbTests*uiNbRuns;
                ulTestStat_SiteIndex = uiSiteIndex*uiNbTests;
                for(uiTestIndex=0; uiTestIndex<uiNbTests; uiTestIndex++)
                {
                    ulIndex = ulTestStat_SiteIndex + uiTestIndex;
                    fprintf(lFile, "%ld,%d,%s,%d,", gtl_stResults.mTestStats[ulIndex].lTestNumber,
                            gtl_stResults.mTestStats[ulIndex].mPinIndex,
                            gtl_stResults.mTestStats[ulIndex].mTestName,
                            gtl_stResults.mTestStats[ulIndex].uiExecs);

                    /* Run loop */
                    ulIndex = ulTestResult_SiteIndex + uiTestIndex*uiNbRuns;
                    for(uiRunIndex=0; uiRunIndex<uiNbRuns; uiRunIndex++)
                    {
                        fResult = gtl_stResults.mTestResults[ulIndex+uiRunIndex].mValue;
                        if(fResult == GTL_INVALID_VALUE_FLOAT)
                            fprintf(lFile, "n/a|%d ", gtl_stResults.mTestResults[ulIndex+uiRunIndex].mFlags);
                        else
                            fprintf(lFile, "%.6e|%d ", fResult, gtl_stResults.mTestResults[ulIndex+uiRunIndex].mFlags);
                    }
                    fprintf(lFile, "\n");
                }
                fprintf(lFile, "\n");
            }
            fprintf(lFile, "#########################################\n\n");
            fclose(lFile);
        }
    }

	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stMessage, NULL, 0);
	if(nStatus != GNET_ERR_OKAY)
		return 0;

	return 1;
}

/*======================================================================================*/
/* Description  : send ENDLOT to GTM server.											*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int gtl_svr_Endlot()
{
    int	nStatus=0;
    GTL_SOCK_MSG stMessage, stReplyMessage;
    GNM_Q_ENDLOT stEndlot;
    PT_GNM_R_ENDLOT	pstENDLOT_Reply=0;
    char szTimestamp[UT_MAX_TIMESTAMP_LEN]="";

	/* Initialize Reset structure */
    gtcnm_InitStruct(GNM_TYPE_Q_ENDLOT, &stEndlot);

	/* Fill Reset structure */
	stEndlot.uiTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);;

	/* Socket message structure */
    stMessage.mType = GNM_TYPE_Q_ENDLOT;
    stMessage.mMsgStruct = (void *)&stEndlot;

    GTL_LOG(6, "Sending endlot msg...", 0);
	/* Send message to server */
    nStatus = gtl_sock_SendMessage(&stMessage, &stReplyMessage, GNM_TYPE_R_ENDLOT);
	if(nStatus != GNET_ERR_OKAY)
		return 0;

    /* Check reply: no need to check status for now, the reply is just to make sure GTM processsed the ENDLOT */
    pstENDLOT_Reply = (PT_GNM_R_ENDLOT)(stReplyMessage.mMsgStruct);
    nStatus = pstENDLOT_Reply->mStatus;
    if(nStatus != GTC_STATUS_OK)
        GTL_LOG(4, "GTM returned status %d on ENDLOT.", nStatus);

    /* Free stuff */
    gtcnm_FreeStruct(GNM_TYPE_R_ENDLOT, pstENDLOT_Reply);
    free(pstENDLOT_Reply);

	return 1;
}

int gtl_svr_EndOfSplitlot()
{
    GTL_LOG(5, "EndOfSplitlot", 0);

    int	nStatus=0;
    GTL_SOCK_MSG stMessage, stReplyMessage;
    GNM_Q_END_OF_SPLITLOT stEnd;
    PT_GNM_R_END_OF_SPLITLOT pstEND_Reply=0;
    char szTimestamp[UT_MAX_TIMESTAMP_LEN]="";

    // Initialize Reset structure
    gtcnm_InitStruct(GNM_TYPE_Q_END_OF_SPLITLOT, &stEnd);

    // Fill Reset structure
    stEnd.mTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);;

    // Socket message structure
    stMessage.mType = GNM_TYPE_Q_END_OF_SPLITLOT;
    stMessage.mMsgStruct = (void *)&stEnd;

    GTL_LOG(6, "Sending end of splitlot message...", 0);
    /* Send message to server */
    // Bug : for the moment, let s do not ask for a reply
    //nStatus = gtl_sock_SendMessage(&stMessage, &stReplyMessage, GNM_TYPE_R_END_OF_SPLITLOT);
    nStatus = gtl_sock_SendMessage(&stMessage, 0, GNM_TYPE_R_END_OF_SPLITLOT);
    if(nStatus != GNET_ERR_OKAY)
        return 0;

    // Hack because of the bug
    return 0;

    /* Check reply: no need to check status for now, the reply is just to make sure GTM processsed the Q */
    pstEND_Reply = (PT_GNM_R_END_OF_SPLITLOT)(stReplyMessage.mMsgStruct);
    if (pstEND_Reply)
    {
        nStatus = pstEND_Reply->mStatus;
        if(nStatus != GTC_STATUS_OK)
            GTL_LOG(4, "GTM returned status %d for END_OF_SPLITLOT.", nStatus);
        /* Free stuff */
        gtcnm_FreeStruct(GNM_TYPE_R_END_OF_SPLITLOT, pstEND_Reply);
        free(pstEND_Reply);
    }

    return 0;
}

/*======================================================================================*/
/* Description  : check if message from GTM server available.							*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GNET_ERR_XXX                                                          */
/*======================================================================================*/
int gtl_svr_CheckForMessage(pnMessageReceived, pstMessage)
int				*pnMessageReceived;
GTL_SOCK_MSG	*pstMessage;
{
	*pnMessageReceived = 0;

	/* Try to get a message from the socket system */
    return gtl_sock_CheckForMessage(pstMessage, pnMessageReceived);
}
