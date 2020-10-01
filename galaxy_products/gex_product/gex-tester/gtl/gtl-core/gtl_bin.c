#include <gtl_core.h>
#include "gtl_message.h"
#include "gtl_output.h"
#include "gtl_constants.h"
#include "gtl_testlist.h"
#include "gtl_profile.h"

extern unsigned gtl_NumOfPAT;
extern int gtl_nBeginJobExecuted;
extern int gtl_IsInitialized();
extern int gtl_IsDisabled();
extern GTL_GlobalInfo gtl_stGlobalInfo; /* GTL Global information */
extern SiteItem	gtl_tl_ptSiteArray[]; /* Array of sites */

/*
    Given a bin no, return the type of PAT : 'D' for dynamic, 'S' for static, '?' if no PAT
    Todo : move me somewhere else...
*/
char gtl_GetPatType(int bin_no, SiteItem *Site)
{
    if (!Site)
        return '?';
    unsigned i=0;
    for (i=0; i<Site->mNbTests; i++)
    {
        TestItem *lTI=Site->mTestList+i;
        if (!lTI)
            break;
        GTL_LOG(7, "gtl_GetPatType() on test %ld: compare test SPAT bin (%d) and test DPAT bin (%d) with part bin (%d)",
            lTI->lTestNumber, lTI->nPatStaticBinning, lTI->nPatDynamicBinning, bin_no);
        if (lTI->nPatDynamicBinning==bin_no)
            return 'D';
        if (lTI->nPatStaticBinning==bin_no)
            return 'S';
    }
    return '?';
}

int gtl_binning(site_nb, original_hbin, original_sbin, new_hbin, new_sbin, part_id)
unsigned int site_nb;
int	original_hbin;
int	original_sbin;
int* new_hbin;
int* new_sbin;
const char* part_id;
{
    GTL_PROFILE_START

    /* Check pointers */
    if(!new_hbin || !new_sbin)
    {
        GTL_LOG(3, "gtl_binning: new hardbin and/or new softbin pointers null (GTL_CORE_ERR_INVALID_PARAM_POINTER)", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;
    }

    /* Initialize new binninhgs to original binnings */
    *new_hbin = original_hbin;
    *new_sbin = original_sbin;

    /* If GTL not initialized, return error code (keep original binnings) */
    if(!gtl_IsInitialized())
    {
        GTL_LOG(3, "gtl_binning(): lib not initialized (GTL_CORE_ERR_LIB_NOT_INITIALIZED)", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    /* If GTL disabled, return error code (keep original binnings) */
    if(gtl_IsDisabled())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_DISABLED;
    }

    //gtl_stPerfInfo.mTotalNumOfBinningCall++;

    /* If BEGINJOB not executed, return error code (keep original binnings) */
    if(!gtl_nBeginJobExecuted)
    {
        GTL_LOG(3, "gtl_binning(): begin job not executed (GTL_CORE_ERR_BEGINJOB_NOT_EXEC)", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_BEGINJOB_NOT_EXEC;
    }

    // 7304 : add a new "Index", incremented at each dies, not at each runs.
    // Check me : move me before gtl_IsInitialized() ?
    gtl_stGlobalInfo.uiPartIndex++;

    GTL_LOG(6, "gtl_binning() on site nb %d part %s : hbin=%d sbin=%d part_index=%d...",
      site_nb, part_id?part_id:"?", original_hbin, original_sbin, gtl_stGlobalInfo.uiPartIndex);

    if (site_nb>255)
    {
        GTL_LOG(3, "Illegal site nb : %d", site_nb);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    }

    SiteItem *lSite = gtl_tl_ptSiteArray + site_nb;
    if (!lSite)
    {
        GTL_LOG(3, "Failed to retrieve a valid SiteItem for site nb %d", site_nb);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_SITE_PTR;
    }

    // mEventType has been updated in begin_job
    if (!strcmp(lSite->mEventType,"GTL_DPAT_LIMITS") || !strcmp(lSite->mEventType,"GTL_SPAT_LIMITS"))
        // new limits received so let's save when
        lSite->mLastLimitsPartIndex=gtl_stGlobalInfo.uiPartIndex;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
        case GTL_STATE_OFFLINE:
            {
                /* Update run results (also checks for outlier results) */
                unsigned int lOutliers=0;
                char lPatBinType='?';
                char lOrgBinType = '?';
                int r=gtl_tl_UpdateRunResult(site_nb, new_hbin, new_sbin, part_id, &lOutliers, &lPatBinType, lOrgBinType);
                if (r!=GTL_CORE_ERR_OKAY)
                {
                    GTL_LOG(3, "gtl_binning(): gtl_tl_UpdateRunResult() failed with code %d", r);
                    GTL_PROFILE_STOP
                    return r;
                }

                /* Check if this part failed PAT */
                if (original_sbin!=*new_sbin)
                {
                    gtl_NumOfPAT++;
                    if (part_id && part_id[0]!='?' && part_id[0]!='\0')
                    {
                        GTL_LOG(5, "PAT for part '%s' at part index %d",
                                part_id?part_id:"?", gtl_stGlobalInfo.uiPartIndex);
                    }
                }
                else
                {
                    /* If not a PAT binning, clear all test outlier flags : ??? */

                    // Let s update some global counters
                    int lGoodSBin=gtl_tl_IsGoodSoftBin(original_sbin);
                    int lGoodHBin=gtl_tl_IsGoodHardBin(original_hbin);
                    if ( lGoodSBin && lGoodHBin ) // according to Kate both must be equal
                        gtl_stGlobalInfo.mNbPartsGood++;
                    else
                    {
                        if (lGoodSBin != lGoodHBin )
                            GTL_LOG(3, "Discrepancy between soft (%d:%c) and hard bin (%d:%c) good status",
                                original_sbin, lGoodSBin?'P':'F',
                                original_hbin, lGoodHBin?'P':'F' );
                    }
                }

                // 7304 : Do here a lot of things when needed
                int lR=gtl_UpdateOutput(gtl_tl_ptSiteArray+site_nb, original_hbin, original_sbin,
                                        *new_hbin, *new_sbin, part_id, lPatBinType, lOrgBinType);
                if (lR!=GTL_CORE_ERR_OKAY)
                {
                    GTL_LOG(3, "UpdateOutput failed: %d", lR);
                    GTL_PROFILE_STOP
                    return lR;
                }
            }
            break;

        default:
            break;
    }

    GTL_PROFILE_STOP

    return GTL_CORE_ERR_OKAY;
}

int gtl_set_binning(site_nb, hbin, sbin, part_id)
unsigned int site_nb;
int	hbin;
int	sbin;
const char* part_id;
{
    GTL_PROFILE_START

    int lHardBin = hbin;
    int lSoftBin = sbin;

    /* If GTL not initialized, return error code */
    if(!gtl_IsInitialized())
    {
        GTL_LOG(3, "gtl_set_binning(): lib not initialized (GTL_CORE_ERR_LIB_NOT_INITIALIZED)", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_LIB_NOT_INITIALIZED;
    }

    /* If GTL disabled, return error code */
    if(gtl_IsDisabled())
    {
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_DISABLED;
    }

    //gtl_stPerfInfo.mTotalNumOfBinningCall++;

    /* If BEGINJOB not executed, return error code */
    if(!gtl_nBeginJobExecuted)
    {
        GTL_LOG(3, "gtl_set_binning(): begin job not executed (GTL_CORE_ERR_BEGINJOB_NOT_EXEC)", 0);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_BEGINJOB_NOT_EXEC;
    }

    // 7304 : add a new "Index", incremented at each dies, not at each runs.
    // Check me : move me before gtl_IsInitialized() ?
    gtl_stGlobalInfo.uiPartIndex++;

    GTL_LOG(7, "gtl_set_binning() on site nb %d part %s : hbin=%d sbin=%d part_index=%d...",
      site_nb, part_id?part_id:"?", hbin, sbin, gtl_stGlobalInfo.uiPartIndex);

    if (site_nb>255)
    {
        GTL_LOG(3, "Illegal site nb : %d", site_nb);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_SITE_NUMBER;
    }

    SiteItem *lSite = gtl_tl_ptSiteArray + site_nb;
    if (!lSite)
    {
        GTL_LOG(3, "Failed to retrieve a valid SiteItem for site nb %d", site_nb);
        GTL_PROFILE_STOP
        return GTL_CORE_ERR_INVALID_SITE_PTR;
    }

    // mEventType has been updated in begin_job
    if (!strcmp(lSite->mEventType,"GTL_DPAT_LIMITS") || !strcmp(lSite->mEventType,"GTL_SPAT_LIMITS"))
        // new limits received so let's save when
        lSite->mLastLimitsPartIndex=gtl_stGlobalInfo.uiPartIndex;

    /* Check current GTL state */
    switch(gtl_stGlobalInfo.nCurrentLibState)
    {
        case GTL_STATE_ENABLED:
        case GTL_STATE_OFFLINE:
            {
                /* Update run results (also checks for outlier results) */
                unsigned int lOutliers=0;
                char lPatBinType='?';
                char lOrgBinType = gtl_GetPatType(lSoftBin, lSite);
                int r=gtl_tl_UpdateRunResult(site_nb, &lHardBin, &lSoftBin, part_id, &lOutliers, &lPatBinType, lOrgBinType);
                if (r!=GTL_CORE_ERR_OKAY)
                {
                    GTL_LOG(3, "gtl_set_binning(): gtl_tl_UpdateRunResult() failed with code %d", r);
                    GTL_PROFILE_STOP
                    return r;
                }

                GTL_LOG(7, "gtl_set_binning(): OrgHB=%d, OrgSB=%d, NewHB=%d, NewSB=%d, PatBinType=%c, OrgBinType=%c, Outliers=%d",
                  hbin, sbin, lHardBin, lSoftBin, lPatBinType, lOrgBinType, lOutliers);

                /* Check if this part failed PAT */
                if ((lOrgBinType == 'S') || (lOrgBinType == 'D'))
                {
                    gtl_NumOfPAT++;
                    if (part_id && part_id[0]!='?' && part_id[0]!='\0')
                    {
                        GTL_LOG(5, "PAT for part '%s' at part index %d",
                                part_id?part_id:"?", gtl_stGlobalInfo.uiPartIndex);
                    }
                }
                else
                {
                    /* If no outliers, clear all test outlier flags : ??? */

                    // Let s update some global counters
                    int lGoodSBin=gtl_tl_IsGoodSoftBin(sbin);
                    int lGoodHBin=gtl_tl_IsGoodHardBin(hbin);
                    if ( lGoodSBin && lGoodHBin ) // according to Kate both must be equal
                        gtl_stGlobalInfo.mNbPartsGood++;
                    else
                    {
                        if (lGoodSBin != lGoodHBin )
                            GTL_LOG(3, "Discrepancy between soft (%d:%c) and hard bin (%d:%c) good status",
                                sbin, lGoodSBin?'P':'F',
                                hbin, lGoodHBin?'P':'F' );
                    }
                }

                // 7304 : Do here a lot of things when needed
                int lR=gtl_UpdateOutput(gtl_tl_ptSiteArray+site_nb, hbin, sbin,
                                        lHardBin, lSoftBin, part_id, lPatBinType, lOrgBinType);
                if (lR!=GTL_CORE_ERR_OKAY)
                {
                    GTL_LOG(3, "UpdateOutput failed: %d", lR);
                    GTL_PROFILE_STOP
                    return lR;
                }
            }
            break;

        default:
            break;
    }

    GTL_PROFILE_STOP

    return GTL_CORE_ERR_OKAY;
}
