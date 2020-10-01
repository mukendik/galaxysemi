#ifdef GCORE15334


#include "station.h"
#include "clientnode.h"
#include "test_data.h"


namespace GS
{
namespace Gex
{
    unsigned CTestData::sNumOfInstances=0;

    CTestData::CTestData()
    {
        sNumOfInstances++;
        // Clear all variables.
        Clear();
        // Variables that are never reset during the PAT process
        mNbSites = 0;			// Total number of testing sites
    }

    CTestData::~CTestData()
    {
        sNumOfInstances--;
        // Clear all variables & delete all memory allocations done.
        Clear();
    }

///////////////////////////////////////////////////////////
// Clear content
void CTestData::Clear()
{
    Reset();
}

void CTestData::Reset()
{
    // Holds total parts in each SoftBin
    mSoftBinSummary.clear();
    // Holds total parts in each HardBin
    mHardBinSummary.clear();

    // 6935: do not delete site objects, just reset them
    mSites.Reset();

    // Reset top message notification displayed
    mErrorLevel = GTM_ERRORTYPE_INFO;
}

///////////////////////////////////////////////////////////
// Increment counters with part result
///////////////////////////////////////////////////////////
QString CTestData::UpdateBinSummary(int PatSoftBin, int PatHardBin)
{
    if(PatSoftBin < 0)
        return "error: PatSoftBin less than 0";

    // Increment binning counters
    mSoftBinSummary.IncBinning(PatSoftBin);
    mHardBinSummary.IncBinning(PatHardBin);
    return "ok";
}

} // Gex
} // GS
#endif
