#ifdef GCORE15334

#ifndef SITETESTRESULTS_H
#define SITETESTRESULTS_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QMutex>

#include "gtc_netmessage.h"
#include "gtl_core.h"
#include "sitetestlist.h"

// Move me into a global header !
#if defined unix || __MACH__
  #define STRING_COMPARE(a,b)  (strcasecmp(a,b))
#else
  #define STRING_COMPARE(a,b)  (stricmp(a,b))
#endif

class CTest;
class CPatInfo;
class CPatDefinition;
#ifdef GCORE15334

class COutlierSummary;
#endif

namespace GS
{
namespace Gex
{

class PartResult
{
public:

    //Q_DISABLE_COPY(PartResult)

    PartResult() { Reset(); }
    //! \brief Copy constr is needed.
    PartResult(const PartResult&);
    virtual ~PartResult() { }
    PartResult& operator=(const PartResult&);

    //! \brief Set ?
    void            Set(PT_GNM_RUNRESULT PartResult);
    //! \breif Reset ?
    void            Reset();
    int             OrgSoftBin() const { return mOrgSoftBin; }
    int             OrgHardBin() const { return mOrgHardBin; }
    int             PatSoftBin() const { return mPatSoftBin; }
    int             PatHardBin() const { return mPatHardBin; }
    QString         PartID() const;
    unsigned int    PartIndex() const { return mPartIndex; }

private:
    mutable QMutex  mMutex;
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
// Holds Part counters (for Baseline, Tuning, Lot...)
///////////////////////////////////////////////////////////
class PartCounters
{
public:
    enum PartCategory
    {
        pass, fail, outlier
    };

    // Constructor/Destructor
    PartCounters();
    virtual ~PartCounters();

    void            Reset();
    void            AddPart(const PartCategory Cat);

    // Set
    void            SetVirtualOutliers(unsigned int Outliers)
    {
        //QMutexLocker lML(&mMutex);
        mVirtualOutlierParts=Outliers;
    }

    // Get
    unsigned int    TestedParts() const { QMutexLocker lML(&mMutex); return mTestedParts; }
    unsigned int    PassParts() const { QMutexLocker lML(&mMutex); return mPassParts;}
    unsigned int    VirtualOutlierParts() const {QMutexLocker lML(&mMutex); return mVirtualOutlierParts;}
    unsigned int    RealOutlierParts() const { QMutexLocker lML(&mMutex); return mRealOutlierParts;}

private:
    Q_DISABLE_COPY(PartCounters)
    //! \brief Mutex to be multi-thread compliant
    mutable QMutex  mMutex;
    //! \brief Tested part ?
    unsigned int    mTestedParts;
    unsigned int    mPassParts;
    // Outliers that would have been outliers with new DPAT limits, but were not flagged as such (ie Baseline outliers)
    unsigned int    mVirtualOutlierParts;
    // Parts that were detected outliers
    unsigned int    mRealOutlierParts;
    // Holds total part count for each Soft bin received
    BinSummary      mSoftBinSummary;
    // Holds total part count for each Hard bin received
    BinSummary      mHardBinSummary;
};

///////////////////////////////////////////////////////////
// Holds test results for a given site
///////////////////////////////////////////////////////////
class SiteTestResults : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SiteTestResults)
    static unsigned sNumOfInstances;

public:
    enum siteState
    {
        SITESTATE_DISABLED,
        SITESTATE_BASELINE,
        SITESTATE_DPAT
    };

    // Constructor/Destructor
    SiteTestResults(const int SiteIndex, const int SiteNb, const int RollingBufferSize,
                    const int TestKey);
    virtual ~SiteTestResults();

    // Clear counters/variables
    void Clear();
    // Init counters/variables
    QString Init(const QHash<QString, CPatDefinition*> & patDefinitions,
                     const GS::Gex::SiteTestResults::siteState SiteState);
    // Restart baseline (invalidate RB results, reset some counters)
    void RestartBaseline();

    // Set
    void SetSiteState(siteState State)
    {
        //QMutexLocker lML(&mMutex);
        mSiteState=State;
    }
    void IncBaselineCount()
    {
        //QMutexLocker lML(&mMutex);
        ++mBaselineCount;
    }
    void IncTuningCount()
    {
        //QMutexLocker lML(&mMutex);
        ++mTuningCount;
    }
    void SetWaitForEarlyTuning(bool ET)
    {
        //QMutexLocker lML(&mMutex);
        mWaitForEarlyTuning=ET;
    }

    // Get
    int                    SiteIndex() const {return mSiteIndex;}
    int                    SiteNb() const {return mSiteNb;}
    siteState              SiteState() const {return mSiteState;}
    //! \brief Get name of the site's state
    QString                SiteStateName() const;
    unsigned int           RBSize() const {QMutexLocker lML(&mMutex); return mRB_Size;}
    unsigned int           RBCurIndex() const {QMutexLocker lML(&mMutex); return mRB_CurIndex;}
    unsigned int           RBValidParts() const {QMutexLocker lML(&mMutex); return mRB_ValidParts;}
    PartCounters &         LotParts() {QMutexLocker lML(&mMutex); return mParts_Lot;}
    PartCounters &         IRBParts() {return mParts_IRB;}
    PartCounters &         SLLParts() {return mParts_SLL;}
    unsigned int           SLLPartsTestedParts() { QMutexLocker lML(&mMutex); return mParts_SLL.TestedParts();}
    unsigned int           SLLPartsPassParts() { QMutexLocker lML(&mMutex); return mParts_SLL.PassParts();}
    //! \brief Returns a copy of the last part. Needs a copy constructor...
    PartResult             LastPart() {return mLastPart;}
    //! \brief Returns the part ID of the last part
    QString                LastPartID();
    //! \brief Returns the part index of the last part
    unsigned int           LastPartIndex();

    //! ????
    CTest*                 TestList() {return mTestList.TestList();}
    bool                   WaitForEarlyTuning() {return mWaitForEarlyTuning;}
    unsigned int           BaselineCount() {QMutexLocker lML(&mMutex); return mBaselineCount;}
    unsigned int           TuningCount() {QMutexLocker lML(&mMutex); return mTuningCount;}
    //! \brief Get part results for a given run index
    const PartResult*      GetPartResult(const unsigned int RI) const;

    //! \brief Find Test Cell. ResetList is actually a RewindList in order to restart from the start of the list.
    CTest* FindTestCell(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                             bool ResetList, const QString & TestName);
    // Collect test result
    QString CollectTestResult(CTest* TestCell, const PT_GNM_TESTRESULT TestResult, bool UpdateRB);
    // Check for production outliers

    QString CheckForProductionOutlier(CTest* TestCell, const CPatDefinition* PatDef,
                                              PT_GNM_RUNRESULT PR, PT_GNM_TESTRESULT TR,
                                              QMap<QString, COutlierSummary *> &GlobalOS, QString &OutlierMsg);
    // Check for Cpk alarm
    QString         CheckForCpkAlarm(CTest* TestCell, const CPatDefinition* PatDef, QString & AlarmMsg);
    // Collect part results
    // Save part result with data received from GTL
    QString         CollectPartResult(PT_GNM_RUNRESULT GtlPartResult, const CPatInfo* PatInfo, bool UpdateRB);

    // Final test outliers summary (holds outliers stats in baseline & lot)
    QMap<QString,COutlierSummary*>  mOutliersSummary;

private:
    int             mSiteIndex;         // Site index for this site (-1 when 'all sites' with mSiteNb=255)
    int             mSiteNb;            // Site nb. for this site
    siteState       mSiteState;         // See enum
    unsigned int    mBaselineCount;     // Nb of baselines executed on the site
    unsigned int    mTuningCount;       // Nb of tunings executed on this site
    bool            mWaitForEarlyTuning;// set to true once if site is waiting for early tuning
    PartResult      mLastPart;          // Last part received from GTL (whether saved in RB or not)

    // Holds total part count for each Soft bin received
    GS::Gex::BinSummary mSoftBinSummary;
    // Holds total part count for each Hard bin received
    GS::Gex::BinSummary mHardBinSummary;

    // Part Counters
    PartCounters    mParts_IRB;             // Parts in intial RB until RB is full
    PartCounters    mParts_SLL;             // Parts since last limits computation for this site
    PartCounters    mParts_SLBL;            // Parts since Baseline
    PartCounters    mParts_Lot;             // Total parts for current lot

    // Rolling buffer (RB)
    unsigned int    mRB_Size;               // Size of the rolling buffer
    unsigned int    mRB_CurIndex;           // Current index in rolling buffer
    unsigned int    mRB_ValidParts;         // Valid parts in Rolling buffer
                                            // (incremented with mRB_CurIndex until buffer full)
    PartResult*     mPartResults;           // Part results for the rolling window
    SiteTestList    mTestList;              // Test list (with results for rolling buffer window)
    mutable QMutex mMutex;
};

///////////////////////////////////////////////////////////
// Holds list of sites
///////////////////////////////////////////////////////////
class SiteList : public QMap<int, SiteTestResults*>
{
public:
    SiteList();                 // Constructor
    virtual ~SiteList();		// Destructor
    void    DeleteAll();
    //! \brief // Reset list of sites
    void    Reset();

    // Get
    // Get total tested parts over all sites
    int     TotalTestedParts() const;
    // Get total PASS parts over all sites
    int     TotalPassParts() const;
    // Get total real outlier parts over all sites
    int     TotalRealOutlierParts() const;
    // Get total virtual outlier parts over all sites
    int     TotalVirtualOutlierParts() const;
    // Get total valid parts in rolling buffer over all sites (Only sites with nb. part >= SiteThreshold are counted)
    int     RBTotalValidParts(unsigned int SiteThreshold=0) const;
    // Get list of site numbers
    void    SiteNumbers(QList<int> & SiteNumbers) const;

private:
    Q_DISABLE_COPY(SiteList)
};

} // Gex

} // GS

#endif // SITETESTRESULTS_H
#endif
