#ifndef SITETESTRESULTS_H_LEGACY
#define SITETESTRESULTS_H_LEGACY

#include <QObject>
#include <QMap>
#include <gstdl_errormgr.h> // needed for BOOL ?
#include <gqtl_utils.h> // for Range

#include "gtc_netmessage.h"

// Move me into a global header !
#if defined unix || __MACH__
  #define STRING_COMPARE(a,b)  (strcasecmp(a,b))
#else
  #define STRING_COMPARE(a,b)  (stricmp(a,b))
#endif

class CTest;
class CPatInfo;
class COutlierSummary;

namespace GS
{
namespace Gex
{

class ClientNode;
class CSiteTestResults;

class PartResult
{
public:
    PartResult() { Reset(); }
    virtual ~PartResult() { }

    void            Set(PT_GNM_RUNRESULT PartResult)
                        {mOrgSoftBin=PartResult->mOrgSoftBin; mOrgHardBin=PartResult->mOrgHardBin;
                        mPatSoftBin=PartResult->mPatSoftBin; mPatHardBin=PartResult->mPatHardBin;
                        mPartID=PartResult->mPartID; mPartIndex=PartResult->mPartIndex;}
    void            Reset() {mOrgSoftBin=-1;mOrgHardBin=-1;mPatSoftBin=-1;mPatHardBin=-1;mPartID.clear();
                            mPartIndex=0;}
    int             OrgSoftBin() const { return mOrgSoftBin; }
    int             OrgHardBin() const { return mOrgHardBin; }
    int             PatSoftBin() const { return mPatSoftBin; }
    int             PatHardBin() const { return mPatHardBin; }
    QString         PartID() const { return mPartID; }
    unsigned int    PartIndex() const { return mPartIndex; }

private:
    int             mOrgSoftBin;
    int             mOrgHardBin;
    int             mPatSoftBin;
    int             mPatHardBin;
    QString         mPartID;
    unsigned int    mPartIndex;
};

///////////////////////////////////////////////////////////
// 6935: Holds Binning summary (total part count for each bin)
///////////////////////////////////////////////////////////
class BinSummary : public QMap<int,int>
{
    static unsigned sNumOfInstances;
public:
    unsigned GetNumberOfInstances() { return sNumOfInstances; }
    BinSummary() { sNumOfInstances++; }
    virtual ~BinSummary() { sNumOfInstances--; }
    void IncBinning(int BinNo)
    {
        if(!contains(BinNo))
            insert(BinNo, 1); // First time we receive this binning
        else
            insert(BinNo, value(BinNo)+1);
    }

private:
    Q_DISABLE_COPY(BinSummary)
};

///////////////////////////////////////////////////////////
// 6935: Holds list of sites
class CSiteList : public QMap<int, GS::Gex::CSiteTestResults*>
{
public:
    CSiteList();                 // Constructor
    virtual ~CSiteList();		// Destructor
    void    DeleteAll();
    void    Reset();

    // Get
    int                 GetTotalOutlierParts(); // Get total outlier parts over all sites
    int                 GetTotalPartsTested();  // Get total parts tested over all sites
    int                 GetTotalPartsGood();    // Get total good parts tested over all sites
    int                 GetTotalPatFailures();  // Get total parts failed because of PAT (since last tuning) over all sites
    // Get total parts failed because of PAT (during baseline) over all sites
    int                 GetTotalBaselinePatFailures();
    // Get total parts failed because of PAT (during baseline) over all sites
    int                 GetTotalPatFailuresSinceLastBaseline();

private:
    Q_DISABLE_COPY(CSiteList)
};

///////////////////////////////////////////////////////////
// Holds list of tests in a test program run
///////////////////////////////////////////////////////////
class CSiteTestResults : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CSiteTestResults)
    static unsigned sNumOfInstances;

public:
    enum siteState
    {
        SITESTATUS_DISABLED,
        SITESTATUS_BASELINE,
        SITESTATUS_DPAT
    };

    // Constructor/Destructor
    CSiteTestResults(GS::Gex::ClientNode *pParent, const int lSiteNb, const int lPartsToBuildBaseline);
    virtual ~CSiteTestResults();

    // Clear counters/variables
    void  Clear();

    //! \brief Find Test Cell (by name) structure in Test List
    CTest *FindTestCellName(int lTestNumber, BOOL bCreateIfNew/*=TRUE*/, BOOL bResetList/*=FALSE*/,char *szName);
    //! \brief Find Test Cell structure (by test# and test name) in Test List
    CTest *FindTestCell(unsigned int lTestNumber,BOOL bCreateIfNew=true, BOOL bResetList=false,char *szName=NULL);
    //! \brief Save test result value into buffer.
    QString CollectTestResult(CTest *ptTestCell, unsigned int uiRunIndex, double lfValue);
    //! \brief Creates & initializes buffer to hold test result samples
    bool AllocateTestCellBuffer(CTest *ptTestCell);
    bool ReAllocateTestCellBuffer(void);
    bool AllocateBinningBuffer(void);
    bool SaveBinning(const int lSoftBin);
    bool GetBinning(const int lIndex, int & lSoftBin);
    bool ReAllocateBinningBuffer(long ldNewBufferSize);
    void ResetSamplesBuffer(void);
    //! \brief Return pointer to first cell in Test list
    CTest *getTestList(void);

    // 6935: Getters / Setters
    int                 GetSiteNb() const {return mSiteNb;}
    siteState           GetSiteState() const {return mSiteState;}
    void                SetSiteState(siteState lState) {mSiteState=lState;}
    long                GetTotalPartsTested() const {return mTotalPartsTested;}
    long                GetTotalPartsGood() const {return mTotalPartsGood;}
    long                GetPartsTestedSinceLastLimits() const {return mPartsTestedSinceLastLimits;}
    void                ResetPartsTestedSinceLastLimits() {mPartsTestedSinceLastLimits=0;}
    long                GetBaselinePatFailures() const {return mBaselinePatFailures;}
    void                SetBaselinePatFailures(long lBaselinePatFailures) {mBaselinePatFailures=lBaselinePatFailures;}
    long                GetTotalPatFailures() const {return mTotalPatFailures;}
    void                ResetTotalPatFailures() {mTotalPatFailures=0;}
    void                IncTotalPatFailures(long lIncrement=1) {mTotalPatFailures+=lIncrement;}
    long                GetTotalOutlierParts() const {return mTotalOutlierParts;}
    int                 GetBaselineCount() const {return mBaselineCount;}
    void                IncBaselineCount(long lIncrement=1) {mBaselineCount+=lIncrement;}
    int                 GetCriticalBinsParts(GS::QtLib::Range *lCriticalBinsList) const;
    QString             AddPart(PT_GNM_RUNRESULT PartResult, const CPatInfo *PatInfo);
    //! Save Bin result into buffer.,update Histogram bars & Yield level info
    void                CollectBinResult(PT_GNM_RUNRESULT PartResult, const CPatInfo *PatInfo);
    int                 GetSamplesBufferSize() const {return mSamplesBufferSize;}
    int                 GetSampleBufferOffset() const {return mSamplesBufferOffset;}
    PartResult          LastPart() const {return mLastPart;}
    //
    long                GetOutlierPartsSinceLastLimits() const {return mOutlierPartsSinceLastLimits;}
    void                ResetOutlierPartsSinceLastLimits() {mOutlierPartsSinceLastLimits=0;}
    long                GetOutlierPartsSinceLastBaseline() const {return mOutlierPartsSinceLastBaseline;}
    void                ResetOutlierPartsSinceLastBaseline() {mOutlierPartsSinceLastBaseline=0;}

private:

    int                 mSiteNb;                // Site nb. for this site
    int                 mSamplesBufferSize;     // Size of test result and binning buffers for this site
    int                 mSamplesBufferOffset;   // Offset in samples buffer while collecting Baseline
    int                 mPartsToBuildBaseline;  // Nb. of parts required for BaseLine

    int                 *ptSoftBinsResults; // Holds the list of soft bins of the samples window to process (baseline,
                                            // or rolling N*runs in production)

    //Gtm_TesterWindow    *mStationParent;    // Used to hold a copy to the 'Gtm_TesterWindow' parent.
    CTest               *ptTestList;		// Pointer to the list of Test cells.

    // Misc.
    CTest           *ptPrevCell;        // After call to 'FindTestCell', points to test preceeding test# searched.
    CTest           *m_ptCellFound;     // Internal variable, used in 'FindTestCell'
    bool            bMergeDuplicateTestName;
    bool            bMergeDuplicateTestNumber;
    // 6935: store state per site
    siteState       mSiteState;
    // 6935: per site counters
    long            mTotalPartsTested;              // Total parts tested on this site
    long            mTotalPartsGood;                // Total good parts on this site

    // Total parts tested on this site since last limits update
    long            mPartsTestedSinceLastLimits;
    // Outlier parts tested on this site since last limits update
    long            mOutlierPartsSinceLastLimits;
    // Outlier parts tested on this site since last Baseline
    long            mOutlierPartsSinceLastBaseline;

    long            mBaselinePatFailures;           // Total parts failed because of PAT (during baseline) on this site
    long            mTotalPatFailures;              // Total parts failed because of PAT (since last tuning) on this site
    long            mTotalOutlierParts;             // Total outlier parts in lot (including baseline) on this site
    int             mBaselineCount;                 // Holds total number of baseline computations done in the lot on this site
    BinSummary      cBinListSummary;                // Holds total part count for each Soft bin received
    // Final test outliers summary (holds outliers stats in baseline & lot)
    QMap<QString, COutlierSummary*> FT_OutliersSummary;

    // Last part result for the site
    PartResult      mLastPart;
};

} // Gex

} // GS

#endif // SITETESTRESULTS_H_LEGACY
