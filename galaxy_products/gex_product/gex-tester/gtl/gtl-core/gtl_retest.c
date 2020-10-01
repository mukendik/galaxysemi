#include <gtl_core.h>
#include <stdio.h>
#include <stdlib.h>
#include "gtl_constants.h"
#include "gtl_testlist.h"
#include "gtl_output.h"
#include "gtl_message.h"

extern GTL_GlobalInfo gtl_stGlobalInfo; // GTL Global information
extern GTL_ProdInfo gtl_stProdInfo;
extern GTL_Station gtl_stStation;
extern GTL_PatSettings gtl_stPatSettings; // Pat settings
extern SiteItem	gtl_tl_ptSiteArray[]; // Array of sites
extern char* gtl_GetGalaxySemiTempFolder();
extern int gtl_PerformEndOfSplitlot();
extern int gtl_IsInitialized();
extern int gtl_push_back_new_message(int severity, char* message, int message_id);
extern int gtl_SQLiteLoadLimits(char* lType, const int lSplitlotID, char lIgnoreUnknownTests);
extern int gtl_nBeginJobExecuted;

int gtl_retest(char* lDesiredLimitsToKeep, char* lRetestType)
{
    GTL_LOG(5, "gtl_retest: LimitsToUse:%s RetestType=%s",
            lDesiredLimitsToKeep?lDesiredLimitsToKeep:"?",
            lRetestType?lRetestType:"?"
            //gtl_stGlobalInfo.mCurrentRetestIndex, gtl_stGlobalInfo.mCurrentRetestIndex+1
            );

    if(!gtl_IsInitialized())
    {
        // Multi sessions retest ?
        // DesiredLimits should has been set. If not then it is an issue.
        if (strcmp(gtl_stGlobalInfo.mDesiredLimits, (char*)"")==0)
        {
            GTL_LOG(3, "Unknown desired limits: %s",gtl_stGlobalInfo.mDesiredLimits);
            return -1; // todo: new err code
        }
        //gtl_stProdInfo.mRetestHbins could be empty

        gtl_stProdInfo.mRetestIndex=1; // todo: set a special value to proof this is a retest multisession ?
        //GTL_LOG(4, "The retest command can be used for 'on the fly' retest only", 0);
        return GTL_CORE_ERR_OKAY; //GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    GTL_LOG(5, "gtl_retest: on the fly retest: RetestIndex jumping from %d to %d",
            gtl_stProdInfo.mRetestIndex, gtl_stProdInfo.mRetestIndex+1
            );

    if(gtl_nBeginJobExecuted)
    {
        GTL_LOG(4, "On the fly retest command not allowed inside a run (beginjob/endjob)", 0);
        return GTL_CORE_ERR_ILLEGAL_ON_THE_FLY_RETEST;
    }

    GTL_LOG(5, "Checking that ALL sites have reached DPAT mode...", 0);
    unsigned int uiSiteIndex, uiNbSites = gtl_stStation.uiNbSites, uiTestIndex;
    SiteItem *ptSite=0;
    //TestItem *ptTestItem=0;
    for(uiSiteIndex=0; uiSiteIndex<uiNbSites; uiSiteIndex++)
    {
        ptSite = gtl_tl_ptSiteArray + gtl_stStation.pnSiteNumbers[uiSiteIndex];
        if(ptSite)
        {
            if (ptSite->mCurrentSiteState != GTL_SITESTATE_DPAT)
                return GTL_CORE_ERR_NOT_ALL_SITES_IN_DPAT;
        }
    }

    GTL_LOG(5, "Retest: current part index=%d", gtl_stGlobalInfo.uiPartIndex);

    // will invalidate splitlot ID
    int lRC=gtl_PerformEndOfSplitlot(); // will call CloseSplitlot
    if (lRC!=0)
    {
        GTL_LOG(3, "Failed to PerformEndOfSplitlot: %d", lRC);
        return lRC;
    }

    // inc retest index because it is retest
    gtl_stProdInfo.mRetestIndex++;

    // open a new splitlot using the current retest index in prod info. Will set SplitlotID.
    lRC=gtl_OutputNewSplitlot();
    if (lRC!=0)
    {
        GTL_LOG(3, "gtl_OutputNewSplitlot failed: %d", lRC)
        return lRC;
    }

    GTL_LOG(5, "Current splitlot is %d", gtl_stProdInfo.mSplitlotID);

    //gtl_msg_Info();
    //gtl_OutputNewMessage();
    // will insert an xt_event for current splitlot id
    lRC=gtl_push_back_new_message(1, "Tuning will be turned off because of retest", -1);
    if (lRC<0)
    {
        GTL_LOG(4, "gtl_push_back_new_message failed: %d", lRC)
        return GTL_CORE_ERR_MESSAGE_POPING_FAILED;
    }

    char lM[255]="";
    sprintf(lM, "Tuning will be turned off because of retest");
    lRC=gtl_OutputNewEvent("RETEST", "", lM); // will insert an event for the current splitlot !
    if (lRC!=0)
    {
        GTL_LOG(4, "gtl OutputNewEvent() failed: %d", lRC);
        return lRC;
    }

    // Get last initial test splitlot id
    int lSplitlotID=0;
    lRC=gtl_GetLastSplitlotId(&lSplitlotID, '0');
    if (lRC!=0)
    {
        GTL_LOG(5, "Cannot retrieve last splitlot id: %d", lRC);
        return lRC;
    }
    GTL_LOG(5, "Last initial test splitlot is splitlot %d", lSplitlotID);

    if (gtl_stGlobalInfo.mReloadLimits == GTL_RELOAD_LIMITS_ON)
    {
        lRC=gtl_SQLiteLoadLimits(gtl_stGlobalInfo.mDesiredLimits, lSplitlotID, 'n');
        if (lRC!=0)
        {
            GTL_LOG(4, "LoadLimits '%s' failed: %d", gtl_stGlobalInfo.mDesiredLimits, lRC);
            return lRC;
        }
    }

    return lRC;
}
