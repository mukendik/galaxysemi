#include "cmerged_results.h"
#include "ctest.h"
#include "cbinning.h"

CMergedResults::CMergedResults()
{
  // List of tests+Bins to merge over all files in same group.
  ptMergedTestList    = NULL;			// Pointing to list of Tests
  ptMergedSoftBinList = NULL;			// Pointing to list of Software Binnings
  ptMergedHardBinList = NULL;			// Pointing to list of Hardware Binnings

  lMergedAverageTestTime_Good = 0;	// Counts total execution time of GOOD parts
  lMergedTestTimeParts_Good   = 0;	// Number of parts used to compute test time of GOOD parts.
  lMergedAverageTestTime_All = 0;		// Counts total execution time of ALL parts
  lMergedTestTimeParts_All   = 0;		// Number of parts used to compute test time of ALL parts.
  lTotalSoftBins = 0;					// Sum of all SOFT Binning results
  lTotalHardBins = 0;					// Sum of all SOFT Binning results

  bPartsFiltered=false;				// true if some parts excluded from analysis.

  m_nGrossDieCount	= 0;

}

CMergedResults::~CMergedResults()
{
  // Destroy lists
  CTest		*ptTestCell,*ptNextTest;
  CBinning	*ptBinCell,*ptNextBin;

  // Destroy Test list
  ptTestCell = ptMergedTestList;
  while(ptTestCell != NULL)
  {
    ptNextTest = ptTestCell->GetNextTest();
    delete ptTestCell;
    ptTestCell = ptNextTest;
  };
  ptMergedTestList = NULL;

// HTH: This doesnt look useful anymore since GCORE-10297
//  CTest::deleteInvalideLimitItem();

  // Destroy Software Binning list
  ptBinCell = ptMergedSoftBinList;
  while(ptBinCell != NULL)
  {
    ptNextBin = ptBinCell->ptNextBin;
    delete ptBinCell;
    ptBinCell = ptNextBin;
  };
  ptMergedSoftBinList = NULL;

  // Destroy Harware Binning list
  ptBinCell = ptMergedHardBinList;
  while(ptBinCell != NULL)
  {
    ptNextBin = ptBinCell->ptNextBin;
    delete ptBinCell;
    ptBinCell = ptNextBin;
  };
  ptMergedHardBinList = NULL;

  // Empty QMap holding Pass/Fail info for each softbin.
  mSBinPassFailFlag.clear();
  // Empty QMap holding Pass/Fail info for each softbin.
  mHBinPassFailFlag.clear();
}


void CMergedResults::UpdateBinSiteCounters( CPartInfo &partInfo)
{
    int softBinNum  = partInfo.iSoftBin;
    int hardBinNum  = partInfo.iHardBin;
    int siteNum     = partInfo.m_site;

    // -- check if not already taking into account
    // -- manage the re-test. Take the last one into account
    if(abs(partInfo.iDieX) != 32768 && abs(partInfo.iDieY) != 32768)
    {
        QPair<int, int> lXYDie(partInfo.iDieX, partInfo.iDieY);
        if(mBinPerDieXY.contains(lXYDie))
        {
            int lLastSoftBin = mBinPerDieXY[lXYDie];
            --mSoftBinSiteCounter[lLastSoftBin][siteNum];
            --mHardBinSiteCounter[lLastSoftBin][siteNum];
            mBinPerDieXY[lXYDie] = softBinNum;
        }
        else
        {
            mBinPerDieXY.insert(lXYDie, softBinNum );
        }
    }
    else if(!partInfo.getPartID().isEmpty())
    {
        QString lKeyStr = partInfo.getPartID();
        if(mBinPerDiePartID.contains(lKeyStr))
        {
            int lLastSoftBin = mBinPerDiePartID[lKeyStr];
            --mSoftBinSiteCounter[lLastSoftBin][siteNum];
            --mHardBinSiteCounter[lLastSoftBin][siteNum];

            mBinPerDiePartID[lKeyStr] = softBinNum;
        }
        else
        {
            mBinPerDiePartID.insert(lKeyStr, softBinNum );
        }
    }

    UpdateBinSiteCounter(mSoftBinSiteCounter, softBinNum, siteNum);
    UpdateBinSiteCounter(mHardBinSiteCounter, hardBinNum, siteNum);
    mSitesList.insert(siteNum);
}


void CMergedResults::UpdateBinSiteCounter(T_BinSiteCounter& binSiteCounter, int binNum, int siteNum)
{
    if( binSiteCounter.contains(binNum) == false)
        binSiteCounter.insert(binNum, QMap<int, int>());

     if(binSiteCounter[binNum].contains(siteNum))
     {
         ++binSiteCounter[binNum][siteNum];
     }
     else
     {
          binSiteCounter[binNum][siteNum] = 1;
     }
}

int CMergedResults::GetNumberOfTests()
{
    int i=0;
    CTest* ptTestCell = ptMergedTestList;
    while(ptTestCell != NULL)
    {
      ++i;
      ptTestCell  = ptTestCell->GetNextTest();
    }
    return i;
}
