#ifndef CMERGED_RESULTS_H
#define CMERGED_RESULTS_H

#include <QMap>
#include <QSet>
#include "cpart_info.h"
//#include "report_options.h"
//extern CReportOptions	ReportOptions;
class CTest;
class CBinning;



///////////////////////////////////////////////////////////
// This class holds all data/pointers that are merged
// for all files belonging to a same group.
// This class holds merged information about all files in the same group
///////////////////////////////////////////////////////////
class CMergedResults
{
public:
    CMergedResults();
  // Destructor: erase all memory used to merge results
    ~CMergedResults();
    CTest	 *ptMergedTestList;				// Pointing to list of Tests
    CBinning *ptMergedSoftBinList;          // Pointing to list of Software Binnings
    CBinning *ptMergedHardBinList;			// Pointing to list of Hardware Binnings
    long	lMergedAverageTestTime_Good;	// Counts in Milli-seconds (Good parts).
    long	lMergedTestTimeParts_Good;		// Number of parts used to compute test time (Good parts).
    long	lMergedAverageTestTime_All;		// Counts in Milli-seconds (All parts).
    long	lMergedTestTimeParts_All;		// Number of parts used to compute test time (All parts).
    long	lTotalSoftBins;					// Sum of all SOFT Binning results
    long	lTotalHardBins;					// Sum of all SOFT Binning results
    bool	bPartsFiltered;					// true if some parts excluded
    QMap<int, int> mSBinPassFailFlag;		// For each softbin, tells if it is a Pass(1) or Fail(0) bin type.
    QMap<int, int> mHBinPassFailFlag;		// For each HARDbin, tells if it is a Pass(1) or Fail(0) bin type.



    int		grossDieCount() const					{ return m_nGrossDieCount; }

    void	addGrossDieCount(int nGrossDieCount)	{ m_nGrossDieCount += nGrossDieCount; }

  // Return the number of CTest linked to this MergedResults
  int GetNumberOfTests();


  void UpdateBinSiteCounters(CPartInfo &partInfo);

  typedef QMap<int, int>                 T_SiteCounter;
  typedef QMap<int, T_SiteCounter >     T_BinSiteCounter;

  const T_BinSiteCounter& HardBinSiteCounter() const  { return mHardBinSiteCounter;}
  const T_BinSiteCounter& SoftBinSiteCounter() const  { return mSoftBinSiteCounter;}

  int                     GetNbSites        () const  { return mSitesList.size();}
  QSet<int>               GetSitesList      () const  { return mSitesList;}

private:
    //-- counter of site iteration, per site number, per softbinNumber
    //-- QMap<softbinNumber, QMap<siteNumber, counter> >
    T_BinSiteCounter mSoftBinSiteCounter;
    T_BinSiteCounter mHardBinSiteCounter;
    QSet<int>        mSitesList;

    //-- Contain the sofbin per die.
    //-- QMap<QPair<xDie, yDie>, softBinNumber>
    QMap<QPair<int, int>, int>      mBinPerDieXY;
    QMap<QString, int>              mBinPerDiePartID;

    int                             m_nGrossDieCount;

    void    UpdateBinSiteCounter(T_BinSiteCounter& binSiteCounter, int binNum, int siteNum);

};




#endif // CMERGED_RESULTS_H
