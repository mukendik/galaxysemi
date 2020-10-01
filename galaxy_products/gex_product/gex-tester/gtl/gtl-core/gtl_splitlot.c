#include <gtl_core.h>
#include "gtl_constants.h"

extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */

int gtl_PerformEndOfSplitlot()
{
    GTL_LOG(5, "Perform EndOfSplitlot...", 0);

    gtl_msg_Info(1, 1, GTL_MSGINFO_END_SPLITLOT, "");

    #ifdef GTLDEBUG
        gtl_LogPerfInfo(5);
    #endif
    int r=0;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
            /* Check if we need to send results to GTM */
            if(gtl_stGlobalInfo.uiRunIndex_Rolling > 0)
            {
                gtl_svr_Results(1);
                r=gtl_tl_ResetTestResult();
                if (r==0)
                    GTL_LOG(3, "gtl_tl_ResetTestResult failed", 0);
                gtl_stGlobalInfo.uiRunIndex_Rolling = 0;
            }

            /* Reset some local variables */
            gtl_stGlobalInfo.uiRunIndex_Lot = 0;

            //GTL_LOG(3, "Check me : do we have to reset test list ?", 0);
            // No: perhaps they will continue with retest on the same test list.
            //gtl_tl_ResetTestlist();

            gtl_svr_EndOfSplitlot();
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

            GTL_LOG(4, "Check me : reset test list ?", 0)
            //r=gtl_tl_ResetTestlist();

            // Close library, GTL will attempt to re-initialize on next run ?
            //gtl_close(); // DO NOT CALL that to avoid infinite loop between gtl_close and gtl_PerformEndLot.

            break;

        case GTL_STATE_DISABLED:
        default:
            break;
    }

    r=gtl_OutputCloseSplitlot(); //call gtl_SQLiteCloseSplitlot(); : set splitlotID to -1

    // Check me: do we have to ask for the next slitlotID to SQLite or just increment the current one ?
    //gtl_stProdInfo.mSplitlotID++; // mSplitlotID will be set by NewSplitlot()
    gtl_stGlobalInfo.uiPartIndex=0;
    gtl_stGlobalInfo.mNbPartsGood=0;
    //gtl_stGlobalInfo.mN

    return r;
}

