#include <gtc_netmessage.h>
#include <stdlib.h>
#include <stdio.h>
#include "gtl_core.h"
#include "gtl_constants.h"
#include "gtl_testlist.h"

extern SiteItem	gtl_tl_ptSiteArray[]; // Array of sites
extern GTL_GlobalInfo gtl_stGlobalInfo; // GTL Global information

extern int gtl_InitSQLiteOutput();
extern int gtl_CloseSQLiteOutput();
extern int gtl_InitXMLOutput();
extern int gtl_InitMySQLOutput();
extern int gtl_InitSTDFOutput();

extern int gtl_SQLiteOutputRecipe(char*, char*);
extern int gtl_SQLiteOutputTestDefs(PT_GNM_TESTDEF td, unsigned n);
extern int gtl_SQLiteOutputNewLimits(SiteItem *s, int lOriginalBin, int lPATBin, const char* lPartID);
extern int gtl_SQLiteOutputNewEvent(char* lEventType, char* lEventSubtype, char* lEventMessage);
extern int gtl_SQLiteOutputNewRun(SiteItem *lSite, const char* lPartID, int lHBin, int lSBin, char lPartStatus);
extern int gtl_SQLiteOutputBin(char bin_type, int bin_no, char bin_cat, char* bin_family, char* bin_subfamily);
extern int gtl_SQLiteOutputDynStats(SiteItem *);
extern int gtl_SQLiteCloseSplitlot();
extern int gtl_SQLiteNewSplitlot();
extern int gtl_SQLiteOutputNewOutlieredTest(TestItem*, float, char LimitType);
extern int gtl_SQLiteOutputNewMessage(GTL_Message* lM);

int gtl_InitOutput()
{
    // code me : if output type == sqlite
    #ifdef GTLDEBUG
        int lR=gtl_InitSTDFOutput();
        GTL_LOG(5, "InitSTDFOutput %d", lR);
    #endif

    return gtl_InitSQLiteOutput();
}

int gtl_CloseOutput()
{
    // code me : if output type == sqlite
    return gtl_CloseSQLiteOutput();
}

int gtl_OutputRecipe(char* lFileName, char* lRecipe)
{
    // code me : if output type == sqlite
    return gtl_SQLiteOutputRecipe(lFileName, lRecipe);
}

int gtl_OutputNewRun(SiteItem *lSite, const char* lPartID, int lHBin, int lSBin, char lPartStatus)
{
    // code me : if output type == sqlite
    return gtl_SQLiteOutputNewRun(lSite, lPartID, lHBin, lSBin, lPartStatus);
}

int gtl_OutputCloseSplitlot()
{
    return gtl_SQLiteCloseSplitlot();
}

// open a new splitlot using the current retest index in prod info. Will set SplitlotID.
int gtl_OutputNewSplitlot()
{
    return gtl_SQLiteNewSplitlot();
}

// return 0 on success
int gtl_OutputNewEvent(char* lEventType, char* lEventSubtype, char* lEventMessage)
{
    // code me : if output type == sqlite
    return gtl_SQLiteOutputNewEvent(lEventType, lEventSubtype, lEventMessage);
}

int gtl_OutputTestDefs(PT_GNM_TESTDEF td, unsigned n)
{
    /*
    if (strcmp(gtl_stGlobalInfo.mTraceabilityMode, "off")==0)
        return 0;
    */

    // code me : if output type == sqlite
    return gtl_SQLiteOutputTestDefs(td, n);
}

int gtl_OutputBin(char bin_type, int bin_no, char bin_cat, char* bin_family, char* bin_subfamily)
{
    // code me : if output type == sqlite
    return gtl_SQLiteOutputBin(bin_type, bin_no, bin_cat, bin_family, bin_subfamily);
}

int gtl_OutputNewLimits(SiteItem *s, int lOriginalBin, int lPATBin, const char* lPartID)
{
    // code me : if output type == sqlite
    return gtl_SQLiteOutputNewLimits(s, lOriginalBin, lPATBin, lPartID);
}

int gtl_OutputTestResults()
{
    GTL_LOG(3, "Not implemented", 0);
    return -1;
}

int gtl_UpdateOutput(SiteItem *lSite, int lOriginalHBin, int lOriginalSBin,
                     int lHBinToAssign, int lSBinToAssign, const char* lPartID, char PatBinType, char OrgBinType)
{
    int lR=0;

    //GTL_LOG(5, "gtl_UpdateOutput for site %d part %s PATbin=%d", lSite->mSiteNb, lPartID, lPATBin);

    // Insert the non PAT H bin in the DB ?
    // Must because we will have to insert the run when limits received, i.e. which can have any kind of bin.

    // 1) In case we have a PAT binning, add it
    //    o In case the original Bin is a PAT BIN assigned by the tester (ie Unison), OrgBinType will be 'S' or 'D'
    //    o Else original bin will be different from assigned bin
    char lPATtype[255]=""; sprintf(lPATtype, "%cPAT", PatBinType);
    if((OrgBinType == 'S') || (OrgBinType == 'D') || (lHBinToAssign!=lOriginalHBin))
    {
        lR=gtl_OutputBin('h', lHBinToAssign, 'F', "PAT", lPATtype);
        if (lR!=0)
            return lR;
        lR=gtl_OutputBin('s', lSBinToAssign, 'F', "PAT", lPATtype);
        if (lR!=0)
            return lR;
    }
    // 2) In case the original bin is not a PAT bin, add it
    if(OrgBinType == '?')
    {
        char hbin_cat='F', sbin_cat='F';
        if(gtl_tl_IsGoodSoftBin(lOriginalSBin))
        {
            sbin_cat='P';
        }
        if(gtl_tl_IsGoodHardBin(lOriginalHBin))
        {
            hbin_cat='P';
        }
        // Insert in both hard and soft bin table
        int lR=gtl_OutputBin('h', lOriginalHBin, hbin_cat, "", "");
        if (lR!=0)
            return lR;
        lR=gtl_OutputBin('s', lOriginalSBin, sbin_cat, "", "");
        if (lR!=0)
            return lR;
    }

    // Do we have to insert the run only when a die is considered as an outlier ?
    if ((OrgBinType == 'S') || (OrgBinType == 'D') || (lHBinToAssign!=lOriginalHBin) ||
            !strcmp(lSite->mEventType,"GTL_DPAT_LIMITS")
            || !strcmp(lSite->mEventType,"GTL_SPAT_LIMITS"))
    {
        // For the moment, let's consider there is no really soft and hard difference, just the same bin
        int lR=gtl_OutputNewRun(lSite, lPartID, lHBinToAssign, lSBinToAssign, gtl_tl_IsGoodHardBin(lHBinToAssign)?'P':'F');
        if (lR!=0)
            return lR;
    }

    // In case of outlier, insert event.
    // Also insert test result if outlier ?
    if((OrgBinType == 'S') || (OrgBinType == 'D') || (lHBinToAssign!=lOriginalHBin))
    {
        // for all tests:
        //(ptTestItem->mTestResults)[gtl_stGlobalInfo.uiRunIndex_Rolling].mValue <= the test result
        char lM[255]="";
        sprintf(lM, "New outlier for site %d Part %s", lSite->mSiteNb, lPartID);
        lR=gtl_OutputNewEvent("GTL_OUTLIER", lPATtype, lM);
        if (lR!=0)
            return lR;
    }

    // 7304 BG: use new event type & subtype
    if(strcmp(lSite->mEventType,"GTL_DPAT_LIMITS") && strcmp(lSite->mEventType,"GTL_SPAT_LIMITS"))
        return GTL_CORE_ERR_OKAY;

    return gtl_OutputNewLimits(lSite, lOriginalHBin, lHBinToAssign, lPartID);
}

int gtl_OutputNewOutlieredTest(TestItem* t, float Result, char LimitType)
{
    return gtl_SQLiteOutputNewOutlieredTest(t, Result, LimitType);
}

int gtl_OutputNewMessage(GTL_Message* m)
{
    return gtl_SQLiteOutputNewMessage(m);
}
