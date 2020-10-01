#include <time.h>
#include <gtl_core.h>
#include "gtl_constants.h"
#include "gtl_testlist.h"
#include "gtl_profile.h"

extern int gtl_nBeginJobExecuted;
extern int gtl_IsInitialized();
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern int gtl_IsDisabled();

/*======================================================================================*/
/* Description  : called for each test to monitor.                                      */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     unsigned int uiSiteNb                                                            */
/*         Site nb                                                                      */
/*                                                                                      */
/*     long lTestNb                                                                     */
/*         Test number                                                                  */
/*                                                                                      */
/*     char *szTestName                                                                 */
/*         Test name                                                                    */
/*                                                                                      */
/*     float fResult                                                                    */
/*         Test result                                                                  */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
int gtl_test(unsigned int uiSiteNb, long lTestNb, char* szTestName, double lfResult)
{
    GTL_PROFILE_START

    gtl_stPerfInfo.mTotalNumOfTestCall++;

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

    int r=GTL_CORE_ERR_OKAY;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
        case GTL_STATE_OFFLINE:
            {
                /* Update test result in TestList */
                r=gtl_tl_UpdateTestResult(uiSiteNb, lTestNb, szTestName, (float)lfResult);
            }
            break;

        case GTL_STATE_DISABLED:
        default:
            break;
    }

    GTL_PROFILE_STOP

    return r;
}

int gtl_mptest(
        unsigned int lSiteNb, long lTestNb, char *lTestName, double* lTestResults, int* lPinIndexes, long lArraysSize)
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

    if (!lTestResults)
    {
        GTL_LOG(4, "MPTestResults array null", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    }

    if (!lPinIndexes)
    {
        GTL_LOG(4, "Pin indexes array null", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    }

    if (lArraysSize<1)
    {
        GTL_LOG(4, "Invalid number of results/pins in MP array: %ld", lArraysSize);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_PARAM_VALUE;
    }

    int r=GTL_CORE_ERR_OKAY;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
        case GTL_STATE_OFFLINE:
                r=gtl_tl_UpdateMPTestResults(lSiteNb, lTestNb, lTestName, lTestResults, lPinIndexes, lArraysSize);
            break;
        case GTL_STATE_DISABLED:
        default:
            r=GTL_CORE_ERR_DISABLED;
            break;
    }

    GTL_PROFILE_STOP
    return r;
}

