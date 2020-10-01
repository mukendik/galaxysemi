#ifdef GCORE15334

#ifndef TEST_DATA_H
#define TEST_DATA_H

#include "sitetestresults.h"
#include <QDateTime>
#include <QHash>

namespace GS
{
namespace Gex
{

//! \class Holds Results for all testing sites
class CTestData : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CTestData)
    static unsigned sNumOfInstances;

public:
    static unsigned GetNumOfInstances() { return sNumOfInstances; }
    CTestData();
    virtual ~CTestData();

    void                Clear();                // Clear content
    //! \brief Reset fields (like when starting new lot).
    void                Reset();
    // Increment counters with part result
    QString             UpdateBinSummary(int PatSoftBin, int PatHardBin);

    unsigned int        mNbSites;				// Nb of sites

    // Holds total part count for each Soft bin received
    GS::Gex::BinSummary mSoftBinSummary;
    // Holds total part count for each Hard bin received
    GS::Gex::BinSummary mHardBinSummary;

    // Tests results (per testing site)
    GS::Gex::SiteList   mSites;

    // Final test outliers summary (holds outliers stats in baseline & lot)
    QMap<QString,COutlierSummary*> FT_OutliersSummary;

    // SPC Histogram chart.
    // Keeps track of the highest error level that occured during a session ('errorType')
    int                 mErrorLevel;
private:

};

} // Gex

} // GS

#endif // TEST_DATA_H
#endif
