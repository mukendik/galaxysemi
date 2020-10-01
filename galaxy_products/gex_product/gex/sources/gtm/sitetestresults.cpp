#ifdef GCORE15334

#include <gqtl_log.h>
#include <gqtl_global.h>

#include "sitetestresults.h"
#include "clientnode.h"
#include "ctest.h"
#include "pat_info.h"
#include "pat_definition.h"

//#include "DebugMemory.h" // must be the last include


namespace GS
{
namespace Gex
{

unsigned GS::Gex::BinSummary::sNumOfInstances=0;

PartCounters::PartCounters()
{
    Reset();
}

PartCounters::~PartCounters()
{
}

void PartCounters::Reset()
{
    QMutexLocker lML(&mMutex);
    mTestedParts=0;
    mPassParts=0;
    mVirtualOutlierParts=0;
    mRealOutlierParts=0;
    mSoftBinSummary.clear();
    mHardBinSummary.clear();
}

void PartCounters::AddPart(const PartCategory Cat)
{
    QMutexLocker lML(&mMutex);

    ++mTestedParts;
    if(Cat==pass)
        ++mPassParts;
    else if(Cat==outlier)
        ++mRealOutlierParts;
}

unsigned SiteTestResults::sNumOfInstances=0;

SiteTestResults::SiteTestResults(const int SiteIndex, const int SiteNb, const int RollingBufferSize,
                                 const int TestKey)
{
    // Determine test key
    GS::Gex::SiteTestList::siteTestKey lTestKey=GS::Gex::SiteTestList::TESTNUMBER;
    switch(TestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
        default:
            lTestKey = GS::Gex::SiteTestList::TESTNUMBER;
            break;
        case GEX_TBPAT_KEY_TESTNAME:
            lTestKey = GS::Gex::SiteTestList::TESTNAME;
            break;
        case GEX_TBPAT_KEY_TESTMIX:
            lTestKey = GS::Gex::SiteTestList::TESTNAME_AND_TESTNUMBER;
            break;
    }

    mTestList.SetTestKey(lTestKey);
    sNumOfInstances++;
    mSiteIndex = SiteIndex;
    mSiteNb = SiteNb;
    mRB_Size = RollingBufferSize;

    // Counters never reset during PAT process
    mBaselineCount = 0;
    mTuningCount = 0;

    // Pointers
    mPartResults=NULL;

    Clear();
}

///////////////////////////////////////////////////////////
// Destructor: Test data in a given site
SiteTestResults::~SiteTestResults()
{
    sNumOfInstances--;
    Clear();
}

void SiteTestResults::Clear()
{
    //QMutexLocker lML(&mMutex);

    // Destroy Test list
    mTestList.Clear();

    // Destroy part results rolling buffer
    if(mPartResults)
    {
        delete [] mPartResults;
        mPartResults=NULL;
    }

    // Rolling buffer
    mRB_CurIndex = 0;
    mRB_ValidParts = 0;

    // Reset per site counters
    mParts_IRB.Reset();             // Parts in intial RB until RB is full
    mParts_SLL.Reset();             // Parts since last tuning for this site
    mParts_SLBL.Reset();            // Parts since Baseline
    mParts_Lot.Reset();             // Total parts for current lot

    // First state is BASELINE
    mSiteState = SITESTATE_BASELINE;

    // Early tuning not done yet
    mWaitForEarlyTuning=false;

    // Holds total parts in each SoftBin
    mSoftBinSummary.clear();
    // Holds total parts in each HardBin
    mHardBinSummary.clear();

}

void SiteTestResults::RestartBaseline()
{
    //QMutexLocker lML(&mMutex); // just to be sure this instance wont be modified by another thread at the same time

    // Invalidate all results in Rolling Buffer
    mTestList.ResetResults();
    if(mPartResults)
    {
        for(unsigned int lIndex=0; lIndex<mRB_Size; ++lIndex)
        {
            mPartResults[lIndex].Reset();
        }
    }

    // Rolling buffer
    mRB_CurIndex = 0;
    mRB_ValidParts = 0;

    // Reset per site counters
    mParts_IRB.Reset();             // Parts in intial RB until RB is full
    mParts_SLL.Reset();             // Parts since last limits computation for this site
    mParts_SLBL.Reset();            // Parts since Baseline

    // Reset state is BASELINE
    mSiteState = SITESTATE_BASELINE;

    // Early tuning not done yet
    mWaitForEarlyTuning=false;
}

QString SiteTestResults::Init(const QHash<QString, CPatDefinition *> & patDefinitions,
                              const GS::Gex::SiteTestResults::siteState SiteState)
{
    //QMutexLocker lML(&mMutex);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Initializing site %1 ...").arg(mSiteNb).toLatin1().constData() );

    // Create test list
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating Test List...").toLatin1().data());

    CPatDefinition  *lPatDef=0;
    CTest           *lTestCell=0;
    bool            lCreateFailure = false;
    unsigned int    lTestCount=0;

    for(QHash<QString, CPatDefinition*>::const_iterator lPatDefIt = patDefinitions.begin();
        lPatDefIt != patDefinitions.end(); ++lPatDefIt)
    {
        lPatDef = *lPatDefIt;

        // Check if we've reached the end of the list...
        if(lPatDef == NULL)
        {
            lCreateFailure = true;	// Failure while creating Test list
            break;
        }

        // Get test cell (create it if not existing yet)
        lTestCell = FindTestCell(lPatDef->m_lTestNumber, lPatDef->mPinIndex,
                                 true, true,lPatDef->m_strTestName);
        if(lTestCell == NULL)
        {
            lCreateFailure = true;	// Failure while creating Test list
            break;
        }

        // Allocate test cell buffer
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Allocating TestCell Buffer for site %1, test# %2 name '%3', size=%4")
              .arg(mSiteNb).arg(lTestCell->lTestNumber).arg(lTestCell->strTestName).arg(mRB_Size)
              .toLatin1().data() );

        // Allocate buffer for test data storage.
        QString lR=lTestCell->m_testResult.createResultTable(mRB_Size);
        if(lR.startsWith(QString("error")))
        {
            lCreateFailure = true;	// Failure while creating Test list
            break;
        }

        // Set test limits
        if(lPatDef->m_lfLowLimit != -GEX_TPAT_DOUBLE_INFINITE)
        {
            lTestCell->GetCurrentLimitItem()->lfLowLimit = lPatDef->m_lfLowLimit;
            lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
        }
        if(lPatDef->m_lfHighLimit != GEX_TPAT_DOUBLE_INFINITE)
        {
            lTestCell->GetCurrentLimitItem()->lfHighLimit = lPatDef->m_lfHighLimit;
            lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
        }

        ++lTestCount;
    }

    // Check if failed creating test list for this site...
    if(lCreateFailure)
        return "error: failed to create test list";

    GSLOG(6, QString("%1 tests created in Test List.").arg(lTestCount).toLatin1().data() );

    // Allocating Part results buffer
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Allocating PartResult Buffer for site %1, size=%2")
          .arg(mSiteNb).arg(mRB_Size).toLatin1().data() );
    mPartResults = new PartResult[mRB_Size];
    if(!mPartResults)
        return "error: failed to allocate part results buffer";

    // Set initial site state. If initial state is DPAT, activate early tuning
    mSiteState = SiteState;

    return "ok";
}

/////////////////////////////////////////////////////////////////////////////
// Save test result value into buffer.
/////////////////////////////////////////////////////////////////////////////
QString SiteTestResults::CollectTestResult(CTest* TestCell, const PT_GNM_TESTRESULT TestResult,
                                           bool UpdateRB)
{
    //QMutexLocker lML(&mMutex);

    // have to update rolling buffer?
    if(!UpdateRB || !(TestResult->mFlags & GTC_TRFLAG_VALID) || (TestResult->mValue == GTL_INVALID_VALUE_FLOAT))
        return "ok: nothing to do";

    // Valid test?
    if (!TestCell)
        return "error: TestCell null";
    if(TestCell->m_testResult.count() == 0)
        return "error: TestCell buffer not allocated";

    // Security check for overflow!
    if(mRB_CurIndex >= mRB_Size)
    {
        // Rolling buffer overflow!
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Rolling buffer overflow: trying to write result at index %1, past allocated buffer of size %2")
              .arg(mRB_CurIndex).arg(mRB_Size).toLatin1().data() );
        return "error: rolling buffer overflow";
    }

    // Save current sample
    if(TestCell->m_testResult.isValidIndex(mRB_CurIndex))
        TestCell->m_testResult.pushResultAt(mRB_CurIndex, TestResult->mValue, true);

    // Update low level statistics (to allow quick Mean, Sigma, anc Cpk computation)
    TestCell->lfSamplesTotal += TestResult->mValue;                     // Sum of X
    TestCell->lfSamplesTotalSquare += (TestResult->mValue*TestResult->mValue);	// Sum of X*X
    // Keeps track of total samples collected
    TestCell->ldSamplesExecs++;

    return "ok";
}

///////////////////////////////////////////////////////////
// Check for outlier during production...
///////////////////////////////////////////////////////////
QString	SiteTestResults::CheckForProductionOutlier(CTest* TestCell, const CPatDefinition* PatDef,
                                                   PT_GNM_RUNRESULT PR, PT_GNM_TESTRESULT TR,
                                                   QMap<QString,COutlierSummary*> & GlobalOS,
                                                   QString & OutlierMsg)
{
    // Check pointers
    if(!TestCell)
        return "error: testcell null";

    if(!PatDef)
        return "error: patdef null";

    if(mSiteState != GS::Gex::SiteTestResults::SITESTATE_DPAT)
        return "ok: site not in DPAT mode, nothing to do";

    // If this run doesn't include outliers, return.
    if(PR->mOrgSoftBin == PR->mPatSoftBin)
        return "ok: not a PAT binning, nothing to do";

    // If test is not an outlier, return.
    if((TR->mValue == GTL_INVALID_VALUE_FLOAT) || !(TR->mFlags & GTC_TRFLAG_VALID) ||
            !(TR->mFlags & (GTC_TRFLAG_SPAT_OUTLIER|GTC_TRFLAG_DPAT_OUTLIER)))
        return "ok: test result is not an outlier";

    // We have an outlier: add it to outlier summary for this test
    // Update the outlier summary structure of this test.

    COutlierSummary *pOutlierSummary=0;	// Structure to hold/update outlier count
    QString strString = QString::number(TestCell->lTestNumber) + "." + TestCell->strTestName;
    pOutlierSummary = mOutliersSummary[strString];
    if(pOutlierSummary == NULL)
    {
        // Test never failed before...create structure
        pOutlierSummary = new COutlierSummary();
        pOutlierSummary->ptTestCell = TestCell;
        pOutlierSummary->iProdFails = 1;
    }
    else
        pOutlierSummary->iProdFails++;
    mOutliersSummary[strString] = pOutlierSummary;
    // Update global outlier summary
    pOutlierSummary = GlobalOS[strString];
    if(pOutlierSummary == NULL)
    {
        // Test never failed before...create structure
        pOutlierSummary = new COutlierSummary();
        pOutlierSummary->ptTestCell = TestCell;
        pOutlierSummary->iProdFails = 1;
    }
    else
        pOutlierSummary->iProdFails++;
    GlobalOS[strString] = pOutlierSummary;

    // If option to display outliers identified, show it!
    int iSeverityLimits = PatDef->m_iOutlierLimitsSet;

    // Build outlier message
    QString lLL="n/a", lHL="n/a";
    if(PatDef->mDynamicLimits[mSiteNb].mLowDynamicLimit1[iSeverityLimits] > -GEX_TPAT_DOUBLE_INFINITE)
        lLL=QString("%1").arg(PatDef->mDynamicLimits[mSiteNb].mLowDynamicLimit1[iSeverityLimits]);
    if(PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit1[iSeverityLimits] < GEX_TPAT_DOUBLE_INFINITE)
        lHL=QString("%1").arg(PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit1[iSeverityLimits]);
    OutlierMsg = QString("[Site %1, PartID %2] <b>T%3:%4 (PinIndex %5)</b> LL: %6, HL: %7")
            .arg(mSiteNb).arg(PR->mPartID).arg(TestCell->lTestNumber).arg(TestCell->strTestName)
            .arg(TestCell->lPinmapIndex).arg(lLL).arg(lHL);

    // If we have two sets of limits, display the other set too!
    if((PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit2[iSeverityLimits]
            != PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit1[iSeverityLimits]) &&
        (PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit2[iSeverityLimits] < GEX_TPAT_DOUBLE_INFINITE))
    {
        OutlierMsg += QString("LL2: %1, HL2: %2")
                .arg(PatDef->mDynamicLimits[mSiteNb].mLowDynamicLimit2[iSeverityLimits])
                .arg(PatDef->mDynamicLimits[mSiteNb].mHighDynamicLimit2[iSeverityLimits]);
    }

    OutlierMsg += QString(" value: %1 %2<br>").arg(TR->mValue).arg(PatDef->m_strUnits);

    return "ok";
}

///////////////////////////////////////////////////////////
// Check for outlier during production...
///////////////////////////////////////////////////////////
QString	SiteTestResults::CheckForCpkAlarm(CTest* TestCell, const CPatDefinition* PatDef, QString & AlarmMsg)
{
    AlarmMsg.clear();

    // Disabled for now (V7.1)
    return "ok";

    // Check pointers
    if(!TestCell)
        return "error: testcell null";
    if(!PatDef)
        return "error: patdef null";

    if(mSiteState != GS::Gex::SiteTestResults::SITESTATE_DPAT)
        return "ok: site not in DPAT mode, nothing to do";

    // Need at least 2 samples to compute Cpk
    if(TestCell->ldSamplesExecs < 2)
        return "ok";	// No alarm. All is fine!

    // Compute Cpk and see if lower than alarm level.
    double	lMean = TestCell->lfSamplesTotal / TestCell->ldSamplesExecs;
    double	lSigma = sqrt(fabs((((double)TestCell->ldSamplesExecs*TestCell->lfSamplesTotalSquare)
                                 - GS_POW(TestCell->lfSamplesTotal,2))/
                        ((double)TestCell->ldSamplesExecs*((double)TestCell->ldSamplesExecs-1))));

    // If all values are identical (sigma = 0), then quietly return
    if(lSigma == 0)
        return "ok";

    double	lCpkL = fabs((lMean-PatDef->m_lfLowLimit)/(3.0*lSigma));
    double	lCpkH = fabs((PatDef->m_lfHighLimit-lMean)/(3.0*lSigma));
    double	lCpk = TestCell->GetCurrentLimitItem()->lfCpk = gex_min(lCpkL,lCpkH);

    // Check if Cpk lower than threshold...
    if(lCpk < PatDef->m_SPC_CpkAlarm)
    {
        AlarmMsg = QString("<font color=\"#ff0000\"><b>Low Cpk alarm</b></font>");
        AlarmMsg += QString("<br>Site #:    %1").arg(mSiteNb);
        AlarmMsg += QString("<br>Test #:    %1 (%2)").arg(TestCell->lTestNumber).arg(PatDef->m_strTestName);
        AlarmMsg += QString("<br>Cpk alarm: <font color=\"#ff0000\"><b>%1</b></font>").arg(TestCell->GetCurrentLimitItem()->lfCpk,0,'f',2);
        if(PatDef->m_SPC_CpkAlarm > 0)
            AlarmMsg += QString(" &lt; <b>%1</b> (threshold)").arg(PatDef->m_SPC_CpkAlarm,0,'f',2);
    }

    return "ok";
}

QString SiteTestResults::CollectPartResult(PT_GNM_RUNRESULT GtlPartResult, const CPatInfo* PatInfo, bool UpdateRB)
{
    // Check pointers
    if(!GtlPartResult)
        return "error: GtlPartResult null";
    if(!PatInfo)
        return "error: PatInfo null";

    //QMutexLocker lML(&mMutex);

    // if invalid binning, return
    if(GtlPartResult->mPatSoftBin < 0)
        return "ok: SoftBin < 0";

    // Save part results in rolling buffer?
    if(UpdateRB)
    {
        // Security check for overflow!
        if(!mPartResults || (mRB_CurIndex >= mRB_Size))
        {
            // Rolling buffer overflow!
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Rolling buffer overflow: trying to write result at index %1, past allocated buffer of size %2")
                  .arg(mRB_CurIndex).arg(mRB_Size).toLatin1().data() );
            return "error: rolling buffer overflow";
        }

        // Save current part
        mPartResults[mRB_CurIndex].Set(GtlPartResult);
    }

    // Determine part category (pass, fail, outlier)
    PartCounters::PartCategory lPartCategory = PartCounters::fail;
    if(PatInfo->GetRecipeOptions().pGoodSoftBinsList->Contains(GtlPartResult->mPatSoftBin))
        lPartCategory = PartCounters::pass;
    else if(GtlPartResult->mPatSoftBin != GtlPartResult->mOrgSoftBin)
        lPartCategory = PartCounters::outlier;

    // Update part counters
    if(mRB_ValidParts < mRB_Size)
        mParts_IRB.AddPart(lPartCategory);
    mParts_SLL.AddPart(lPartCategory);
    mParts_Lot.AddPart(lPartCategory);
    if(mSiteState != SITESTATE_BASELINE)
        mParts_SLBL.AddPart(lPartCategory);

    // Update RB variables ?
    if(UpdateRB)
    {
        if(mRB_ValidParts < mRB_Size)
            ++mRB_ValidParts;
        if(++mRB_CurIndex >= mRB_Size)
            mRB_CurIndex = 0;
    }

    // Update site bin summary
    mSoftBinSummary.IncBinning(GtlPartResult->mPatSoftBin);
    mHardBinSummary.IncBinning(GtlPartResult->mPatHardBin);

    // Set last part
    mLastPart.Set(GtlPartResult);

    return "ok";
}

QString SiteTestResults::LastPartID()
{
    QMutexLocker lML(&mMutex);
    return mLastPart.PartID();
}

unsigned int SiteTestResults::LastPartIndex()
{
    QMutexLocker lML(&mMutex); // really needed ?
    return mLastPart.PartIndex();
}

QString SiteTestResults::SiteStateName() const
{
    QString lName;

    // used in Gtm_TesterWindow::ReloadSitesTab(void). Let's lock this function.
    QMutexLocker lML(&mMutex);

    switch(mSiteState)
    {
        case GS::Gex::SiteTestResults::SITESTATE_DISABLED:
            lName = QString("DISABLED");
            break;
        case GS::Gex::SiteTestResults::SITESTATE_BASELINE:
            lName = QString("BASELINE");
            break;
        case GS::Gex::SiteTestResults::SITESTATE_DPAT:
            lName = QString("DPAT");
            break;
        default:
            lName = QString("UNKNOWN");
            break;
    }
    return lName;
}

const PartResult* SiteTestResults::GetPartResult(const unsigned int RI) const
{
    if(RI >= mRB_Size)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Trying to access part results past the rolling buffer size.");
        return NULL;
    }

    if(!mPartResults)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Part results buffer is null." );
        return NULL;
    }

    return (mPartResults+RI);
}

CTest *SiteTestResults::FindTestCell(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                                     bool ResetList, const QString &TestName)
{
    return mTestList.FindTestCell(TestNumber, lRtnIndex, CreateIfNew, ResetList, TestName);
}

///////////////////////////////////////////////////////////
// Constructor: List of sites
///////////////////////////////////////////////////////////
SiteList::SiteList()
{
}

///////////////////////////////////////////////////////////
// Destructor: List of sites
///////////////////////////////////////////////////////////
SiteList::~SiteList()
{
    DeleteAll();
}

///////////////////////////////////////////////////////////
// Get total tested parts over all sites
///////////////////////////////////////////////////////////
int SiteList::TotalTestedParts() const
{
    // Sum tested parts over all sites
    int	lTotalParts=0;
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalParts += lSite->LotParts().TestedParts();
    };
    return lTotalParts;
}

///////////////////////////////////////////////////////////
// Get total PASS parts over all sites
///////////////////////////////////////////////////////////
int SiteList::TotalPassParts() const
{
    // Sum PASS parts over all sites
    int	lTotalParts=0;
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalParts += lSite->LotParts().PassParts();
    };
    return lTotalParts;
}

///////////////////////////////////////////////////////////
// Get total real outlier parts over all sites
///////////////////////////////////////////////////////////
int SiteList::TotalRealOutlierParts() const
{
    // Sum PASS parts over all sites
    int	lTotalParts=0;
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalParts += lSite->LotParts().RealOutlierParts();
    };
    return lTotalParts;
}

///////////////////////////////////////////////////////////
// Get total virtual outlier parts over all sites
///////////////////////////////////////////////////////////
int SiteList::TotalVirtualOutlierParts() const
{
    // Sum PASS parts over all sites
    int	lTotalParts=0;
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalParts += lSite->LotParts().VirtualOutlierParts();
    };
    return lTotalParts;
}

///////////////////////////////////////////////////////////
// Get total valid parts in rolling buffer over all sites
// Only sites with nb. part >= SiteThreshold are counted
///////////////////////////////////////////////////////////
int SiteList::RBTotalValidParts(unsigned int SiteThreshold/*=0*/) const
{
    // Sum PASS parts over all sites
    int	lTotalParts=0;
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite && lSite->RBValidParts() >= SiteThreshold)
            lTotalParts += lSite->RBValidParts();
    };
    return lTotalParts;
}

///////////////////////////////////////////////////////////
// Get list of site numbers
///////////////////////////////////////////////////////////
void SiteList::SiteNumbers(QList<int> & SiteNumbers) const
{
    GS::Gex::SiteTestResults *lSite=0;
    for(GS::Gex::SiteList::ConstIterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            SiteNumbers.append(lSite->SiteNb());
    };
}

///////////////////////////////////////////////////////////
// Clear list of sites
///////////////////////////////////////////////////////////
void SiteList::DeleteAll()
{
    // Delete Sites memory blocks
    SiteTestResults *pSite;
    Iterator it = begin();
    while(it != end())
    {
        // Get site ptr
        pSite = it.value();
        if(pSite)
        {
            // Delete site block.
            delete pSite;
        }

        // Move to next site
        ++it;
    };

    // Empty Map
    clear();
}

void SiteList::Reset()
{
    // Delete Sites memory blocks
    SiteTestResults *pSite=0;
    SiteList::Iterator it = begin();
    while(it != end())
    {
        // Get valid site# used and site ptr.
        pSite = it.value(); // 6935: use value, instead of []
        if(pSite)
        {
            pSite->Clear();
        }
        // Move to next site
        ++it;
    };
}

} // Gex
} // GS

#endif
