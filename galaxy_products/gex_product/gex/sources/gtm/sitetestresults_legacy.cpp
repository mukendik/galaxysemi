#include <gqtl_log.h>
#include "sitetestresults_legacy.h"
#include "clientnode.h"
#include "ctest.h"
#include "pat_info.h"

//#include "DebugMemory.h" // must be the last include


namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////
// Constructor: List of sites
CSiteList::CSiteList()
{
}

///////////////////////////////////////////////////////////
// Destructor: List of sites
///////////////////////////////////////////////////////////
CSiteList::~CSiteList()
{
    DeleteAll();
}

///////////////////////////////////////////////////////////
// Get total outlier parts over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalOutlierParts()
{
    // Sum outliers over all sites
    int	lTotalOutlierParts=0;
    GS::Gex::CSiteTestResults* lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalOutlierParts += lSite->GetTotalOutlierParts();
    };
    return lTotalOutlierParts;
}

///////////////////////////////////////////////////////////
// Get total parts tested over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalPartsTested()
{
    // Sum outliers over all sites
    int	lTotalPartsTested=0;
    GS::Gex::CSiteTestResults *lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalPartsTested += lSite->GetTotalPartsTested();
    };
    return lTotalPartsTested;
}

///////////////////////////////////////////////////////////
// Get total good parts tested over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalPartsGood()
{
    // Sum outliers over all sites
    int	lTotalPartsGood=0;
    GS::Gex::CSiteTestResults *lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalPartsGood += lSite->GetTotalPartsGood();
    };
    return lTotalPartsGood;
}

///////////////////////////////////////////////////////////
// Get total parts failed because of PAT (since last tuning) over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalPatFailures()
{
    // Sum outliers over all sites
    int	lTotalPatFailures=0;
    GS::Gex::CSiteTestResults *lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalPatFailures += lSite->GetTotalPatFailures();
    };
    return lTotalPatFailures;
}

///////////////////////////////////////////////////////////
// Get total parts failed because of PAT (since last baseline) over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalPatFailuresSinceLastBaseline()
{
    // Sum outliers over all sites
    int	lTotalPatFailures=0;
    GS::Gex::CSiteTestResults *lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalPatFailures += lSite->GetOutlierPartsSinceLastBaseline();
    };
    return lTotalPatFailures;
}

///////////////////////////////////////////////////////////
// Get total parts failed because of PAT (since last tuning) over all sites
///////////////////////////////////////////////////////////
int CSiteList::GetTotalBaselinePatFailures()
{
    // Sum outliers over all sites
    int	lTotalBaselinePatFailures=0;
    GS::Gex::CSiteTestResults *lSite=0;
    for(GS::Gex::CSiteList::Iterator it= begin();it!=end();++it)
    {
        // Get site ptr.
        lSite = it.value();
        if(lSite)
            lTotalBaselinePatFailures += lSite->GetBaselinePatFailures();
    };
    return lTotalBaselinePatFailures;
}

///////////////////////////////////////////////////////////
// Clear list of sites
///////////////////////////////////////////////////////////
void CSiteList::DeleteAll()
{
    // Delete Sites memory blocks
    GS::Gex::CSiteTestResults *lSite;
    Iterator it = begin();
    while(it != end())
    {
        // Get site ptr
        lSite = it.value();
        if(lSite)
        {
            // Delete site block.
            delete lSite;
        }

        // Move to next site
        ++it;
    };

    // Empty Map
    clear();
}

///////////////////////////////////////////////////////////
// Reset list of sites
///////////////////////////////////////////////////////////
void CSiteList::Reset()
{
    // Delete Sites memory blocks
    GS::Gex::CSiteTestResults *lSite=0;
    CSiteList::Iterator it = begin();
    while(it != end())
    {
        // Get valid site# used and site ptr.
        lSite = it.value(); // 6935: use value, instead of []
        if(lSite)
        {
            lSite->Clear();
        }

        // Move to next site
        ++it;
    };
}

unsigned CSiteTestResults::sNumOfInstances=0;

///////////////////////////////////////////////////////////
// Constructor: Test data in a given site
CSiteTestResults::CSiteTestResults(ClientNode *pParent, const int lSiteNb,
                                   const int lPartsToBuildBaseline): QObject(pParent)
{
    sNumOfInstances++;
    ptTestList=NULL;		// Pointer to the list of Test cells.
    ptPrevCell=NULL;        // After call to 'FindTestCell', points to test preceeding test# searched.
    m_ptCellFound=NULL;     // Internal variable, used in 'FindTestCell'
    ptSoftBinsResults=NULL; // SBin results
    //mStationParent = pParent;
    mSiteNb = lSiteNb;
    mPartsToBuildBaseline = lPartsToBuildBaseline;

    // Baseline count never reset during PAT process
    mBaselineCount = 0;

    Clear();
}

///////////////////////////////////////////////////////////
// Destructor: Test data in a given site
///////////////////////////////////////////////////////////
CSiteTestResults::~CSiteTestResults()
{
    sNumOfInstances--;

    Clear();
}

void CSiteTestResults::Clear()
{
    // Destroy Test list
    CTest *ptTestCell = ptTestList;
    CTest *ptNextTest;
    while(ptTestCell != NULL)
    {
        ptNextTest = ptTestCell->ptNextTest;
        delete ptTestCell;
        ptTestCell = ptNextTest;
    };
    ptTestList = NULL;

    // Holds the list of soft bins of the samples window to process (baseline, or rolling N*runs in production)
    if(ptSoftBinsResults)
    {
        delete [] ptSoftBinsResults;
        ptSoftBinsResults=NULL;
    }

    // Set buffer size = min(2*BaseLine, 200)
    mSamplesBufferSize = 2*mPartsToBuildBaseline > 200 ? 2*mPartsToBuildBaseline:200;
    mSamplesBufferOffset = 0;		// Run# when collecting Test Results & SoftBins in baseline

    // Reset per site counters
    mTotalPartsTested=0;
    mTotalPartsGood=0;
    mOutlierPartsSinceLastLimits = 0;
    mPartsTestedSinceLastLimits = 0;
    mOutlierPartsSinceLastBaseline = 0;
    mBaselinePatFailures=0;
    mTotalPatFailures=0;
    mTotalOutlierParts=0;
    // Holds total parts in each SoftBin
    cBinListSummary.clear();

    // Misc.
    ptPrevCell=NULL;        // After call to 'FindTestCell', points to test preceeding test# searched.
    m_ptCellFound=NULL;     // Internal variable, used in 'FindTestCell'
    bMergeDuplicateTestName=true;
    bMergeDuplicateTestNumber=true;

    // First state is BASELINE
    mSiteState = SITESTATUS_BASELINE;
}

CTest *CSiteTestResults::getTestList(void)
{
    return ptTestList;
}

CTest *CSiteTestResults::FindTestCellName(int lTestNumber,
                                          BOOL bCreateIfNew/*=TRUE*/, BOOL bResetList/*=FALSE*/, char *szName)
{
    CTest *ptTestCell=0;
    CTest *ptNewCell=0;
    int	iCompare=0;
    // Clear pointer that keeps track of previous test.
    ptPrevCell=NULL;

    if(szName == NULL)
        return NULL;

    if(ptTestList == NULL)
    {
        if(bCreateIfNew == false)
            return NULL;

        // First test : list is currently empty.
        ptTestList = new CTest;
        ptTestList->lTestNumber = lTestNumber;
        ptTestList->strTestName = szName;
        ptTestList->ptNextTest  = NULL;
        m_ptCellFound = ptTestList;
        ptPrevCell = NULL;
        return ptTestList;	// Success
    }

    // Check if rewind list prior to search in it.
    if(bResetList)
        goto rewind_list;

    // Scan list from current cell to find cell (if exists)
    ptTestCell = m_ptCellFound;
    while(ptTestCell != NULL)
    {
        // We've passed the cell
        iCompare = STRING_COMPARE(ptTestCell->strTestName.toLatin1().data(), szName);
        if(iCompare > 0)
        {
            // Cell not found, do we have to create it?
            if(bCreateIfNew)
                goto create_test;
            else
                goto rewind_list;	// Test not found, maybe was earlier in the list; so rewind it!
        }
        if(iCompare == 0)
        {
            m_ptCellFound = ptTestCell;
            return ptTestCell; // Test found, pointer to it returned.
        }
       ptPrevCell = ptTestCell;
       ptTestCell = ptTestCell->ptNextTest;
    };

rewind_list:
    // Start from first test in list
    ptTestCell = ptTestList;
    m_ptCellFound = ptTestList;
    ptPrevCell = NULL;

    while(ptTestCell != NULL)
    {
        // We've passed the cell
        iCompare = STRING_COMPARE((char *)ptTestCell->strTestName.toLatin1().data(),szName);
        if(iCompare > 0)
        {
            // Cell not found, do we have to create it?
            if(bCreateIfNew)
                goto create_test;
            else
                return NULL;	// Test not found
        }

        // Matching test name
        if(iCompare == 0)
        {
            m_ptCellFound = ptTestCell;
            return ptTestCell; // Test found, pointer to it returned.
        }
       ptPrevCell = ptTestCell;
       ptTestCell = ptTestCell->ptNextTest;
    };

    // Cell not found, do we have to create it?
    if(bCreateIfNew)
        goto create_test;
    else
        return NULL;	// Test not found

    // Insert new cell in current position.
create_test:
    m_ptCellFound = ptNewCell = new CTest;
    ptNewCell->lTestNumber = lTestNumber;
    ptNewCell->strTestName = szName;
    ptNewCell->ptNextTest = NULL;

    if(ptPrevCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextTest = ptTestList;
        ptTestList = ptNewCell;
    }
    else
    {
        // Insert cell in list
        ptPrevCell->ptNextTest = ptNewCell;
        ptNewCell->ptNextTest  = ptTestCell;
    }
    return ptNewCell;	// Success
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
CTest *CSiteTestResults::FindTestCell(unsigned int lTestNumber,
                                      BOOL bCreateIfNew/*=true*/,
                                      BOOL bResetList/*=false*/,
                                      char *szName/*=NULL*/)
{
    CTest *ptTestCell=0;
    CTest *ptNewCell=0;

    // Clear pointer that keeps track of previous test.
    ptPrevCell=NULL;

    if(ptTestList == NULL)
    {
        // First test : list is currently empty.
        ptTestList = new CTest;
        ptTestList->lTestNumber = lTestNumber;
        if(szName != NULL)
            ptTestList->strTestName = szName;
        ptTestList->ptNextTest  = NULL;
        m_ptCellFound = ptTestList;
        ptPrevCell = NULL;
        return ptTestList;	// Success
    }

    // Check if rewind list prior to search in it.
    if(bResetList)
        goto rewind_list;

    // Scan list from current cell to find cell (if exists)
    ptTestCell = m_ptCellFound;
    while(ptTestCell != NULL)
    {
        // We've passed the cell
        if(ptTestCell->lTestNumber > lTestNumber)
        {
            // Cell not found, do we have to create it?
            if(bCreateIfNew)
                goto create_test;
            else
                goto rewind_list;	// Test not found, maybe was earlier in the list; so rewind it!
        }

        // Matching test#
        if(ptTestCell->lTestNumber == lTestNumber)
        {
            // If searching for a given Name
            if((szName != NULL) && (*szName))
            {
                // Check if matching name (unless this checking is disabled)
                if((bMergeDuplicateTestNumber==false) && ptTestCell->strTestName != szName)
                    break; // no matching name...
            }

            m_ptCellFound = ptTestCell;
            return ptTestCell; // Test found, pointer to it returned.
        }
       ptPrevCell = ptTestCell;
       ptTestCell = ptTestCell->ptNextTest;
    };

    // Cell not in list, if we have to create it, let's do it
    if(bCreateIfNew)
        goto create_test;
    else
        return NULL;

rewind_list:
    // Start from first test in list
    ptTestCell = ptTestList;
    m_ptCellFound = ptTestList;
    ptPrevCell = NULL;

    while(ptTestCell != NULL)
    {
        // We've passed the cell
        if(ptTestCell->lTestNumber > lTestNumber)
        {
            // Cell not found, do we have to create it?
            if(bCreateIfNew)
                goto create_test;
            else
                return NULL;	// Test not found
        }

        // Matching test#
        if(ptTestCell->lTestNumber == lTestNumber)
        {
            // If searching for a given Name
            if((szName != NULL) && (*szName))
            {
                // Check if matching name (unless this checking is disabled)
                if((bMergeDuplicateTestNumber==false) && ptTestCell->strTestName != szName)
                    break; // no matching name...
            }

            m_ptCellFound = ptTestCell;
            return ptTestCell; // Test found, pointer to it returned.
        }
       ptPrevCell = ptTestCell;
       ptTestCell = ptTestCell->ptNextTest;
    };

    // Cell not found, do we have to create it?
    if(bCreateIfNew)
        goto create_test;
    else
        return NULL;	// Test not found

    // Insert new cell in current position.
create_test:
    m_ptCellFound = ptNewCell = new CTest;
    ptNewCell->lTestNumber = lTestNumber;
    if(szName != NULL)
        ptNewCell->strTestName = szName;
    ptNewCell->ptNextTest = NULL;

    if(ptPrevCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextTest = ptTestList;
        ptTestList = ptNewCell;
    }
    else
    {
        // Insert cell in list
        ptPrevCell->ptNextTest = ptNewCell;
        ptNewCell->ptNextTest  = ptTestCell;
    }
    return ptNewCell;	// Success
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::AllocateTestCellBuffer(CTest *ptTestCell)
{
    if(!ptTestCell)
        return false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Allocating TestCell Buffer for site %1, test %2, size=%3")
          .arg(mSiteNb).arg(ptTestCell->lTestNumber).arg(mSamplesBufferSize)
          .toLatin1().data());

    // Allocate buffer for test data storage.
    QString status=ptTestCell->m_testResult.createResultTable(mSamplesBufferSize);
    if(status.startsWith(QString("error")))
        return false;

    // Success
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Reallocates & initializes buffer to hold test result samples
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::ReAllocateTestCellBuffer(void)
{
    CTest	*ptTestCell = ptTestList;
    QString strStatus;
    // Compute new buffer size (add one baseline count)
    long	ldNewBufferSize = mSamplesBufferSize + mPartsToBuildBaseline;

    while(ptTestCell != NULL)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reallocating TestCell Buffer for site %1, test %2, size=%3->%4")
              .arg(GetSiteNb()).arg(ptTestCell->lTestNumber).arg(mSamplesBufferSize).arg(ldNewBufferSize)
              .toLatin1().constData());

        // If this test cell is not currently in use, skip it for now!
        if(ptTestCell->m_testResult.count() == 0)
            goto next_cell;

        strStatus = ptTestCell->m_testResult.resizeResultTable(ldNewBufferSize);
        //ptTestCell->ldSamplesExecs
        if(strStatus.startsWith("error"))
            return false;

        // Move to next test cell
next_cell:
        ptTestCell = ptTestCell->ptNextTest;
    };

    GSLOG(7, QString("TestResult TotalAllocated=%1 o").arg(CTestResult::GetTotalAllocated()).toLatin1().data() );

    // Reallocate buffer for binnings
    if (!ReAllocateBinningBuffer(ldNewBufferSize))
        GSLOG(3, "ReAllocateBinningBuffer failed");

    // Save New buffer size
    mSamplesBufferSize = ldNewBufferSize;

    // Success
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Creates & initializes buffer to hold binning results
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::AllocateBinningBuffer(void)
{
    if(ptSoftBinsResults != NULL)
        return true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Allocating Binning Buffer for site %1, size=%2")
          .arg(GetSiteNb()).arg(mSamplesBufferSize).toLatin1().constData() );

    ptSoftBinsResults = new int[mSamplesBufferSize];
    if(ptSoftBinsResults == NULL)
        return false;

    memset(ptSoftBinsResults,0,mSamplesBufferSize*sizeof(int));	// Clear buffer
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Get soft bin
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::GetBinning(const int lIndex, int & lSoftBin)
{
    if(lIndex >= mSamplesBufferSize)
        return false;

    lSoftBin = ptSoftBinsResults[lIndex];
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Save soft bin
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::SaveBinning(const int lSoftBin)
{
    if(mSamplesBufferOffset >= mSamplesBufferSize)
        return false;

    ptSoftBinsResults[mSamplesBufferOffset] = lSoftBin;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Creates & initializes buffer to hold binning results
/////////////////////////////////////////////////////////////////////////////
bool CSiteTestResults::ReAllocateBinningBuffer(long ldNewBufferSize)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reallocating Binning Buffer for site %1, size=%2->%3")
          .arg(GetSiteNb()).arg(mSamplesBufferSize).arg(ldNewBufferSize).toLatin1().constData() );

    // Create new buffer
    int	*ptInt = new int[ldNewBufferSize];
    if(ptInt == NULL)
        return false;

    // Copy previous (smaller) buffer data into new buffer
    memcpy(ptInt,ptSoftBinsResults,mSamplesBufferSize*sizeof(int));
    memset(&ptInt[mSamplesBufferSize],0,(ldNewBufferSize-mSamplesBufferSize)*sizeof(int));	// Clear buffer

    // Free old buffer
    // delete [] or delete ?
    delete [] ptSoftBinsResults;
    //delete ptSoftBinsResults;

    // Connect pointer to new buffer
    ptSoftBinsResults = ptInt;
    return true;
}

///////////////////////////////////////////////////////////
// Reset samples buffer offsets in case buffer getting full
///////////////////////////////////////////////////////////
void CSiteTestResults::ResetSamplesBuffer(void)
{
    // Reset buffer offset (Buffer holding test results no longer need to store the full base line, but only the packet received)
    // Check if samples buffer almost full...
    if(mSamplesBufferOffset >= mSamplesBufferSize)
        mSamplesBufferOffset = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Add binning to bin summary.
/////////////////////////////////////////////////////////////////////////////
int CSiteTestResults::GetCriticalBinsParts(GS::QtLib::Range *lCriticalBinsList) const
{
    QMap<int,int>::ConstIterator it;
    long	lBinID;
    long	lTotalCriticalBins=0;
    for ( it = cBinListSummary.begin(); it != cBinListSummary.end(); ++it )
    {
        // Get Bin
        lBinID = it.key();

        // Check if belongs to our list of critical bins
        if(lCriticalBinsList->Contains(lBinID))
            //lTotalCriticalBins += it.data();	// Cumulater total of critical bins : Qt3
            lTotalCriticalBins += it.value();	// Cumulater total of critical bins
    }

    return lTotalCriticalBins;
}

void CSiteTestResults::CollectBinResult(PT_GNM_RUNRESULT PartResult, const CPatInfo *PatInfo)
{
    if(!PatInfo)
        return;

    // Save binning into buffer for later reprocessing (if building baseline), or outlier counting in production.
    AllocateBinningBuffer();
    SaveBinning(PartResult->mPatSoftBin);

    // Add part
    AddPart(PartResult, PatInfo);

    // Inc buffer offset
    ++mSamplesBufferOffset;
}

/////////////////////////////////////////////////////////////////////////////
// Increment counters with part result
/////////////////////////////////////////////////////////////////////////////
QString CSiteTestResults::AddPart(PT_GNM_RUNRESULT PartResult, const CPatInfo *PatInfo)
{
    if(!PatInfo)
        return "error : PatInfo null";
    if(!PartResult)
        return "error : PartResult null";

    // Keep track of total outlier parts
    if(PartResult->mPatSoftBin != PartResult->mOrgSoftBin)
    {
        ++mTotalOutlierParts;
        ++mOutlierPartsSinceLastLimits;
        ++mOutlierPartsSinceLastBaseline;
    }

    // Increment binning counters
    cBinListSummary.IncBinning(PartResult->mPatSoftBin);

    // Keep track of total GOOD parts tested
    if(PatInfo->cOptionsPat.pGoodSoftBinsList->Contains(PartResult->mPatSoftBin))
        ++mTotalPartsGood;

    // Keep track of total parts tested
    ++mTotalPartsTested;
    ++mPartsTestedSinceLastLimits;

    // Save last PartID for this site
    mLastPart.Set(PartResult);

    return "ok";
}

QString CSiteTestResults::CollectTestResult(CTest *ptTestCell, unsigned int uiRunIndex, double lfValue)
{
    if (!ptTestCell)
        return "error: TestCell null";

    // Valid test!
    if(ptTestCell->m_testResult.count() == 0)
    {
        // Allocate buffer for test data storage.
        if(AllocateTestCellBuffer(ptTestCell) == false)
            return "error: AllocateTestCellBuffer failed";	// Failed allocating buffer.
    }

    // Update low level statistics (to allow quick Mean, Sigma, anc Cpk computation)
    ptTestCell->lfSamplesTotal += lfValue;					// Sum of X
    ptTestCell->lfSamplesTotalSquare += (lfValue*lfValue);	// Sum of X*X

    // Keeps track of total samples collected
    ptTestCell->ldSamplesExecs++;

    // Security check for overflow!
    if(mSamplesBufferOffset+(int)uiRunIndex >= mSamplesBufferSize)
    {
        // Need to realloc buffers as original space to hold baseline is not big enough!
        if(ReAllocateTestCellBuffer() == false)
        {
            GSLOG(3, "ReAllocateTestCellBuffer failed");
            return "error: ReAllocateTestCellBuffer failed";
        }
    }

    // Save current sample (during baseline building only)
    if(ptTestCell->m_testResult.isValidIndex(mSamplesBufferOffset+uiRunIndex))
        ptTestCell->m_testResult.pushResultAt(mSamplesBufferOffset+uiRunIndex, lfValue, true);

    return "ok";
}

} // Gex
} // GS

