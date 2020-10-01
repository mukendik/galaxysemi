#include <gqtl_log.h>

#include "stdf.h"	// for BYTE typedef
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "report_options.h"
#include "cbinning.h"
#include "cpart_info.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "product_info.h"
#include "message.h"
#include <QMessageBox>
#include <QPushButton>

#include "gexperformancecounter.h"

extern CReportOptions	ReportOptions;
extern double	ScalingPower(int iPower);

bool CGexGroupOfFiles::m_sbIgnoreAllMRRWarning = false;

///////////////////////////////////////////////////////////
// This class holds information about a group of files
///////////////////////////////////////////////////////////
CGexGroupOfFiles::CGexGroupOfFiles(QString GroupName, int lGroupId)
{
  // Saves group name
  strGroupName = GroupName;
  mGroupId = lGroupId;

  // Ensure we use latest options set
  UpdateOptions(ReportOptions);
  m_cStats.UpdateOptions(&ReportOptions);
  m_bIgnoreMRRWarning = false;
  mFlowId = 0;
  mGroupMapTestsByNumber.clear();
  mLimitSetId = 1;
  m_nGroupRunOffset = 0;
}

///////////////////////////////////////////////////////////
// Destructor : Closes file if not done,
// reset all private variables.
///////////////////////////////////////////////////////////
CGexGroupOfFiles::~CGexGroupOfFiles()
{
    if(cStackedWaferMapData.cWafMap != NULL)
        delete [] cStackedWaferMapData.cWafMap;
    cStackedWaferMapData.cWafMap = NULL;

    // Ensure the list is empty
    while(pFilesList.isEmpty() == false)
        delete pFilesList.takeFirst();

    mGroupMapTestsByNumber.clear();
}

void  CGexGroupOfFiles::Init(CGexGroupOfFiles* pGroup, CReportOptions* ptReportOptions,  FILE*hAdvFile, int iPass)
{

    UpdateOptions(*ptReportOptions);
    for(int i = 0; i < pGroup->pFilesList.count(); ++i)
     {
        pGroup->pFilesList[i]->Init(ptReportOptions,&pGroup->cMergedData, hAdvFile, iPass);
        pGroup->resetGroupRunOffset();
     }

    /* QString strDatasetSorting = (ptReportOptions->GetOption("dataprocessing", "sorting")).toString();

     if((iPass == 2) && (strDatasetSorting=="date"))
     {
         // Sort files by date in MIR
         if (!pGroup->pFilesList.isEmpty())
             qSort(pGroup->pFilesList.begin(), pGroup->pFilesList.end(), CGexFileInGroup::lessThan);

         for (int nIndex = 0; nIndex < pGroup->pFilesList.count(); nIndex++)
             pGroup->pFilesList.at(nIndex)->lFileID = nIndex;
     }*/

}

bool CGexGroupOfFiles::operator <(const CGexGroupOfFiles& other) const
{
    CGexFileInGroup * pFileMin = pFilesList.first();
    int iIdx;
    for(iIdx = 0; iIdx<pFilesList.count();iIdx++){
        if(*(pFilesList.at(iIdx)) < *pFileMin){
            pFileMin = pFilesList.at(iIdx);
        }
    }

    CGexFileInGroup * pFileMinOther = other.pFilesList.first();
    for(iIdx = 0; iIdx<other.pFilesList.count();iIdx++){
        if(*(other.pFilesList.at(iIdx)) < *pFileMinOther){
            pFileMinOther = other.pFilesList.at(iIdx);
        }
    }

    return (*pFileMin)<(*pFileMinOther);
}

CGexGroupOfFiles::LimitUsed CGexGroupOfFiles::getLimitUsed() const
{
    return mLimitUsed;
}

int CGexGroupOfFiles::getLimitSetId() const
{
    return mLimitSetId;
}

void CGexGroupOfFiles::setLimitSetId(int limitSetId)
{
    mLimitSetId = limitSetId;
}

bool CGexGroupOfFiles::UpdateOptions(CReportOptions& options)
{
    m_strDatasetSorting = options.GetOption("dataprocessing", "sorting").toString();
    QString lLimit = options.GetOption("dataprocessing", "limit_selection_criteria").toString();

    if (lLimit == "largest")
    {
        mLimitSelection = LargestLimit;
    }
    else if (lLimit == "first")
    {
        mLimitSelection = FirstLimit;
    }
    else if (lLimit == "last")
    {
        mLimitSelection = LastLimit;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString(" error : unknown option 'dataprocessing/limit_selection_criteria' '%1' ")
              .arg(lLimit).toLatin1().constData());

        return false;
    }

    QString lLimitScope = options.GetOption("dataprocessing", "scope").toString();

    if (lLimitScope == "per_site")
    {
        mLimitScope = LimitPerSite;
    }
    else if (lLimitScope == "over_all_site")
    {
        mLimitScope = LimitOverAllSite;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString(" error : unknown option 'dataprocessing/scope' '%1' ")
              .arg(lLimitScope).toLatin1().constData());

        return false;
    }

    // Set limit used in analysing files
    QString limitUsed = options.GetOption("dataprocessing", "used_limits").toString();

    if (limitUsed=="spec_limits_if_any")
        mLimitUsed = specLimitIfAny;
    else if (limitUsed == "standard_limits_only")
        mLimitUsed = standardLimitOny;
    else if (limitUsed == "spec_limits_only")
        mLimitUsed = specLimitOnly;
    else
    {
        mLimitUsed = standardLimitOny;

        GSLOG(SYSLOG_SEV_ERROR,
              QString(" error : unknown option used_limits' '%1' ")
              .arg(limitUsed).toLatin1().constData());

        return false;
    }

    mLimitSetId = 0;

    return true;
}

///////////////////////////////////////////////////////////
// Return Bin name for a given Bin# (HBIN or SBIN)
///////////////////////////////////////////////////////////
QString CGexGroupOfFiles::GetBinName(bool bSoftBin,int iBin)
{
  CBinning	*ptBinCell;
  if(bSoftBin)
    ptBinCell = cMergedData.ptMergedSoftBinList;
  else
    ptBinCell = cMergedData.ptMergedHardBinList;

  while(ptBinCell != NULL)
  {
    if(ptBinCell->iBinValue == iBin)
      return ptBinCell->strBinName;

    // Move to next Bin cell
    ptBinCell = ptBinCell->ptNextBin;
  };

  // Bin not found
  return "";
}

///////////////////////////////////////////////////////////
// Update (or create if needed) SBIN/HBIN entry
///////////////////////////////////////////////////////////
void	CGexGroupOfFiles::UpdateBinSummary(bool bSoftBin,int iBin,BYTE bPass,QString strBinName)
{
  CBinning	*ptBinCell,*ptPrevBinCell=NULL;
  if(bSoftBin)
    ptBinCell = cMergedData.ptMergedSoftBinList;
  else
    ptBinCell = cMergedData.ptMergedHardBinList;

  while(ptBinCell != NULL)
  {
    if(ptBinCell->iBinValue == iBin)
    {
      // Entry found, simply increment it
      ptBinCell->ldTotalCount++;
      return;
    }

    // Move to next Bin cell
    ptPrevBinCell = ptBinCell;
    ptBinCell = ptBinCell->ptNextBin;
  };

  // Entry not found, add it to the list!
  ptBinCell = new CBinning;
  ptBinCell->iBinValue = iBin;
  ptBinCell->cPassFail = bPass;
  ptBinCell->ldTotalCount = 1;
  ptBinCell->strBinName = strBinName;
  ptBinCell->ptNextBin = NULL;
  ptPrevBinCell->ptNextBin = ptBinCell;
}

///////////////////////////////////////////////////////////
// Update the soft bins and hard bins tables (from samples array)
///////////////////////////////////////////////////////////
void	CGexGroupOfFiles::UpdateBinTables(bool bSoftBin)
{
  QString strBinningComputationOption = (ReportOptions.GetOption("binning","computation")).toString();

  // if (ReportOptions.bBinningUseWafermapOnly)
  if (strBinningComputationOption == "wafer_map")
  {
    CBinning *	ptBinCell		= NULL;

    // Get the right bin table
    if(bSoftBin)
    {
      ptBinCell = cMergedData.ptMergedSoftBinList;

      cMergedData.lTotalSoftBins	= 0;
    }
    else
    {
      ptBinCell = cMergedData.ptMergedHardBinList;

      cMergedData.lTotalHardBins	= 0;
    }

    QMap<int, int> mapBinCount;

    GetWaferBinCount(bSoftBin, mapBinCount);

    // Loop on all binnings
    while(ptBinCell != NULL)
    {
      if (mapBinCount.contains(ptBinCell->iBinValue))
      {
        // Update the count
        ptBinCell->ldTotalCount = mapBinCount.value(ptBinCell->iBinValue);

        if (bSoftBin)
          cMergedData.lTotalSoftBins += ptBinCell->ldTotalCount;
        else
          cMergedData.lTotalHardBins += ptBinCell->ldTotalCount;
      }
      else
        ptBinCell->ldTotalCount = 0;

      // Move to next Bin cell
      ptBinCell = ptBinCell->ptNextBin;
    };
  }
  // else if (ReportOptions.bBinningUseSamplesOnly)
  else if (strBinningComputationOption == "samples")
  {
    CBinning *	ptBinCell		= NULL;

    // Get the right bin table
    if(bSoftBin)
    {
      ptBinCell = cMergedData.ptMergedSoftBinList;

      cMergedData.lTotalSoftBins	= 0;
    }
    else
    {
      ptBinCell = cMergedData.ptMergedHardBinList;

      cMergedData.lTotalHardBins	= 0;
    }

    QMap<int, int> mapBinCount;

    GetSampleBinCount(bSoftBin, mapBinCount);

    // Loop on all binnings
    while(ptBinCell != NULL)
    {
      if (mapBinCount.contains(ptBinCell->iBinValue))
      {
        // Update the count
        ptBinCell->ldTotalCount = mapBinCount.value(ptBinCell->iBinValue);

        if (bSoftBin)
          cMergedData.lTotalSoftBins += ptBinCell->ldTotalCount;
        else
          cMergedData.lTotalHardBins += ptBinCell->ldTotalCount;
      }
      else
        ptBinCell->ldTotalCount = 0;

      // Move to next Bin cell
      ptBinCell = ptBinCell->ptNextBin;
    };
  }
}

///////////////////////////////////////////////////////////
// Update the soft bins and hard bins tables (from Wafermap array)
///////////////////////////////////////////////////////////
void	CGexGroupOfFiles::UpdateBinTables_Wafermap(bool bSoftBin)
{
  //FIXME: not used ?
  //CBinning* ptBinCell = NULL;

  // Check if SBIN or HBIN table to update...
  if(bSoftBin)
  {
    //FIXME: not used ?
    //ptBinCell = cMergedData.ptMergedSoftBinList;
    cMergedData.lTotalSoftBins	= 0;
  }
  else
  {
    //FIXME: not used ?
    //ptBinCell = cMergedData.ptMergedHardBinList;
    cMergedData.lTotalHardBins	= 0;
  }

  // Read the complete map, and update Bin count!

}

///////////////////////////////////////////////////////////
// Return total SoftBin records for a given bin# from sample
///////////////////////////////////////////////////////////
void CGexGroupOfFiles::GetSampleBinCount(bool bSoftBin, QMap<int,int>& mapBinCount)
{
  CTest	*ptTestCell=0;

  // Get handle to file list in group.
  CGexFileInGroup *pFile = (this->pFilesList.isEmpty()) ? NULL : this->pFilesList.at(0);
  if(pFile == NULL)
    return;

  // Get handle to Bin results.
  if(bSoftBin)
  {
    // Looking at SoftBin
    if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
      return;
  }
  else
  {
    // Looking at HardBin
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
      return;
  }

  mapBinCount.clear();

  int nValue;
  for(long lOffset = 0; lOffset < ptTestCell->m_testResult.count(); lOffset++)
  {
    if (ptTestCell->m_testResult.isValidResultAt(lOffset))
    {
      nValue = (int) ptTestCell->m_testResult.resultAt(lOffset);

      if (mapBinCount.contains(nValue))
        mapBinCount[nValue]++;
      else
        mapBinCount[nValue] = 1;
    }
  }
}

///////////////////////////////////////////////////////////
// Return total SoftBin records for a given bin# from wafermap
///////////////////////////////////////////////////////////
void CGexGroupOfFiles::GetWaferBinCount(bool bSoftBin, QMap<int,int>& mapBinCount)
{
    // Get handle to file list in group.
    CGexFileInGroup *pFile = NULL;

    // Clear the bin count map
    mapBinCount.clear();

    // Temp double
    int 	nValue;
    int		nBinValue;

    for (int nSubLot = 0; nSubLot < pFilesList.count(); nSubLot++)
    {
        pFile = pFilesList.at(nSubLot);

        if (pFile)
        {
            // Get die coordinates handles
            CTest * ptDieX		= NULL;
            CTest * ptDieY		= NULL;
            CTest * ptPartID	= NULL;
            CTest *	ptTestCell	= NULL;
            int		nBegin		= 0;
            int		nEnd		= 0;

            // Count total occurance of given binning (only count latest retest)
            QMap	<QString,int>	cWafermapBin;
            QString					strDieLocation;

            QString lPartIdentification = ReportOptions.GetOption("dataprocessing",
                                                                  "part_identification").toString();

            if (lPartIdentification.compare("auto", Qt::CaseInsensitive) == 0)
            {
                if (pFile->getWaferMapData().bWirExists)
                    lPartIdentification = "xy";
                else
                    lPartIdentification = "part_id";
            }

            // Get handle to Bin results.
            if(bSoftBin)
            {
                // Looking at SoftBin
                if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
                    continue;
            }
            else
            {
                // Looking at HardBin
                if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true) != 1)
                    continue;
            }

            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptDieX,true,false) != 1)
                continue;
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptDieY,true,false) != 1)
                continue;
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_PARTID, GEX_PTEST, &ptPartID, true, false) != 1)
                    continue;

            // Check if No die info available
            if((ptDieX->m_testResult.count() == 0) || (ptDieY->m_testResult.count() == 0))
                continue;

            ptTestCell->findSublotOffset(nBegin, nEnd, nSubLot);

            // Use Last bin value
            if (ReportOptions.GetOption("wafer","retest_policy").toString()=="last_bin")
            {
                for(long lOffset = nBegin; lOffset < nEnd; lOffset++)
                {
                    if (ptTestCell->m_testResult.isValidResultAt(lOffset))
                    {
                        // Build die location key
                        if (lPartIdentification.compare("xy", Qt::CaseInsensitive) == 0)
                            strDieLocation	= QString::number(ptDieX->m_testResult.resultAt(lOffset)) + "-" +
                                              QString::number(ptDieY->m_testResult.resultAt(lOffset));
                        else
                            strDieLocation	= QString::number(ptPartID->m_testResult.resultAt(lOffset));

                        nValue = (int) ptTestCell->m_testResult.resultAt(lOffset);

                        if (cWafermapBin.contains(strDieLocation))
                        {
                            nBinValue	= cWafermapBin.value(strDieLocation);

                            if (nBinValue != nValue)
                            {
                                // Decrease count for previous bin
                                mapBinCount[nBinValue]--;

                                // Increase count for latest bin found
                                if (mapBinCount.contains(nValue))
                                    mapBinCount[nValue]++;
                                else
                                    mapBinCount[nValue] = 1;

                                cWafermapBin[strDieLocation] = nValue;
                            }
                        }
                        else
                        {
                            if (mapBinCount.contains(nValue))
                                mapBinCount[nValue]++;
                            else
                                mapBinCount[nValue] = 1;

                            cWafermapBin[strDieLocation] = nValue;
                        }
                    }
                }

            }
            // Use highest bin value
            else
            {
                //QMap<QString,int>	cWafermapDie;

                for(long lOffset = nBegin; lOffset < nEnd; lOffset++)
                {
                    if (ptTestCell->m_testResult.isValidResultAt(lOffset))
                    {
                        // Build die location key
                        if (lPartIdentification.compare("xy", Qt::CaseInsensitive) == 0)
                            strDieLocation	= QString::number(ptDieX->m_testResult.resultAt(lOffset)) + "-" +
                                              QString::number(ptDieY->m_testResult.resultAt(lOffset));
                        else
                            strDieLocation	= QString::number(ptPartID->m_testResult.resultAt(lOffset));

                        nValue			= (int) ptTestCell->m_testResult.resultAt(lOffset);

                        if (cWafermapBin.contains(strDieLocation))
                        {
                            nBinValue	= cWafermapBin.value(strDieLocation);

                            if (nBinValue < nValue)
                            {
                                // Decrease count for previous bin
                                mapBinCount[nBinValue]--;

                                // Increase count for latest bin found
                                if (mapBinCount.contains(nValue))
                                    mapBinCount[nValue]++;
                                else
                                    mapBinCount[nValue] = 1;

                                cWafermapBin[strDieLocation] = nValue;
                            }
                        }
                        else
                        {
                            if (mapBinCount.contains(nValue))
                                mapBinCount[nValue]++;
                            else
                                mapBinCount[nValue] = 1;

                            cWafermapBin[strDieLocation] = nValue;
                        }
                    }
                }
            }
        }
    }
}

long CGexGroupOfFiles::GetWaferBinCount(bool bSoftBin,int iBin)
{
  long	lTotalCount = 0;

  // Get handle to file list in group.
  CGexFileInGroup *pFile = NULL;

  for (int nSubLot = 0; nSubLot < pFilesList.count(); nSubLot++)
  {
    pFile = pFilesList.at(nSubLot);

    if (pFile)
    {
      // Get die coordinates handles
      CTest *	ptDieX		= NULL;
      CTest * ptDieY		= NULL;
      CTest *	ptTestCell	= NULL;
      int		nBegin		= 0;
      int		nEnd		= 0;

      // Count total occurance of given binning (only count latest retest)
      QMap	<QString,int>	cWafermapBin;
      QString					strDieLocation;

      if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptDieX,true,false) != 1)
        continue;
      if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptDieY,true,false) != 1)
        continue;

      // Get handle to Bin results.
      if(bSoftBin)
      {
        // Looking at SoftBin
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
          continue;
      }
      else
      {
        // Looking at HardBin
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) != 1)
          continue;
      }

      // Check if No die info available
      if((ptDieX->m_testResult.count() == 0) || (ptDieY->m_testResult.count() == 0))
        continue;

      ptTestCell->findSublotOffset(nBegin, nEnd, nSubLot);

      // Use Last bin value
      if (ReportOptions.GetOption("wafer","retest_policy").toString()=="last_bin")		//ReportOptions.bWafMapRetest_HighestBin == false)
      {
        for(long lOffset = nBegin; lOffset < nEnd; lOffset++)
        {
          if(ptTestCell->m_testResult.isValidResultAt(lOffset))
          {
            // Build die location key
            strDieLocation = QString::number(ptDieX->m_testResult.resultAt(lOffset)) + "-" + QString::number(ptDieY->m_testResult.resultAt(lOffset));

            // Check if we've reached a binning of interest
            if(iBin == (int) ptTestCell->m_testResult.resultAt(lOffset))
              cWafermapBin[strDieLocation] = 1;		// Flag this die has a binning we want to count
            else
              cWafermapBin.remove(strDieLocation);	// Ensure we do NOT count dies with a bin# we don't want to count.
          }
        }
      }
      // Use highest bin value
      else
      {
        QMap<QString,int>	cWafermapDie;
        int					nDieBinValue;

        for(long lOffset = nBegin; lOffset < nEnd; lOffset++)
        {
          if (ptTestCell->m_testResult.isValidResultAt(lOffset))
          {
            // Build die location key
            strDieLocation = QString::number(ptDieX->m_testResult.resultAt(lOffset)) + "-" + QString::number(ptDieY->m_testResult.resultAt(lOffset));

            if (cWafermapDie.contains(strDieLocation))
              nDieBinValue = qMax(cWafermapDie.value(strDieLocation), (int) ptTestCell->m_testResult.resultAt(lOffset));
            else
              nDieBinValue = (int) ptTestCell->m_testResult.resultAt(lOffset);

            cWafermapDie[strDieLocation] = nDieBinValue;

            // Check if we've reached a binning of interest
            if(iBin == nDieBinValue)
              cWafermapBin[strDieLocation] = iBin;
            else if (cWafermapBin.contains(strDieLocation) && nDieBinValue > cWafermapBin[strDieLocation])
              cWafermapBin.remove(strDieLocation);	// Ensure we do NOT count dies with a bin# we don't want to count.
          }
        }
      }

      // Return total occurances of given Bin
      lTotalCount += cWafermapBin.count();
    }
  }

  return lTotalCount;
}

int CGexGroupOfFiles::GetTotalMirDataPartCount()
{
  int t=0;
  foreach(CGexFileInGroup* f, pFilesList)
    if (f)
      t=t+f->getPcrDatas().lPartCount;

  return t;
}

bool	CGexGroupOfFiles::GetDieYieldValue(int iDieX,int iDieY,double &lfValue)
{
  QString strString;
  QString strSubLotID;
  bool	bOk;
  int		iBin;
  double	lfDieWeight=0.0;
  double	lfSumWeight=0.0;
  double	lfTotalWeights=0.0;

  // Get handle to file list in group.
  CGexFileInGroup * pFile       = NULL;
  CBinning *        pBinning    = NULL;

  for (int nSubLot = 0; nSubLot < pFilesList.count(); nSubLot++)
  {
    // Get handle to sublot data
    pFile = pFilesList.at(nSubLot);
    if (pFile == NULL)
      continue;

    // Get value at die location (iDieX,iDieY)
    iBin = pFile->getWaferMapData().binValue(iDieX,iDieY,CWaferMap::LastTest);
    if(iBin < 0)
      continue;	// Invalid die location!

    strSubLotID = pFile->getMirDatas().szSubLot;
    if(strSubLotID.startsWith("ZPAT-Mask/",Qt::CaseInsensitive))
    {
      // This is a ZPAT wafer generated by PAT-Man, extract # of wafer used to generate this map
      // format: 'ZPAT-Mask/<number>'
      strString = strSubLotID.section('/',1);
      lfDieWeight = strString.toDouble(&bOk);
      if(bOk == false)
        continue;	// Unexpected missing/invalid number
    }
    else
    {
      // Standard Wafermap.
      lfDieWeight = 1.0;
    }

    if (pFile->getWaferMapData().lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
        pBinning = cMergedData.ptMergedSoftBinList->Find(iBin);
    else
        pBinning = cMergedData.ptMergedHardBinList->Find(iBin);

    // Compute yield data
    if(pBinning && pBinning->cPassFail == 'P')
      lfSumWeight += lfDieWeight;	// This die location is PASS

    lfTotalWeights += lfDieWeight;
  }

  // Compute yield level
  if(lfTotalWeights)
  {
    lfValue = (100.0*lfSumWeight)/lfTotalWeights;
    return true;
  }

  // No yield could be computed for this die location (eg: missing die)
  return false;
}


////////////////////////////////////////////////////
// Returns true if given bin is of 'Pass' type.
///////////////////////////////////////////////////////////
bool CGexGroupOfFiles::isPassingBin(bool bSoftBin,int iBin)
{
  CBinning	*ptBinCell;
  if(bSoftBin)
    ptBinCell = cMergedData.ptMergedSoftBinList;
  else
    ptBinCell = cMergedData.ptMergedHardBinList;

  while(ptBinCell != NULL)
  {
    // If binnning found, check if it is a Pass or Fail bin
    if(ptBinCell->iBinValue == iBin)
    {
      return (ptBinCell->cPassFail == 'P');
    }

    // Binning entry not found yet...move to next one
    ptBinCell = ptBinCell->ptNextBin;
  }

  // Binning not found in list!
  return false;
}

//////////////////////////////////////////////////////////
// Returns total logical files in group that hold valid data (that match query criteria)
///////////////////////////////////////////////////////////
long CGexGroupOfFiles::GetTotalValidFiles(void)
{
  QString	strWaferID;
  long	lCount=0;

  CGexFileInGroup *pFile = NULL;
  QListIterator<CGexFileInGroup*> itFilesList(pFilesList);

  while (itFilesList.hasNext())
  {
    pFile		= itFilesList.next();
    strWaferID	= pFile->getWaferMapData().szWaferID;

    // Check if this logical file matches our query criteria
    if(pFile->strWaferToExtract.isEmpty() || pFile->isMatchingWafer(pFile->strWaferToExtract,strWaferID))
      lCount++;
  };

  return lCount;
}


int CGexGroupOfFiles::GetGroupId() const
{
    return mGroupId;
}


///////////////////////////////////////////////////////////
// Remove a list of parts
///////////////////////////////////////////////////////////
void CGexGroupOfFiles::removeParts(QList<long> lstParts)
{
  // Remove all results for given Run#
  QList<long>::const_iterator   lPartOffset;
  CTest *						ptTest	= cMergedData.ptMergedTestList;
  QVariant                      varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
  bool                          bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);

  // Clean Part Info list, remove filtered part if clean_samples options is activated
  if (bOptionsCleanSamples)
  {
      // Sort sample index
      qSort(lstParts);

      int nBegin    = 0;
      int nEnd      = 0;
      int nSample   = -1;

      for (int nFile = 0; nFile < pFilesList.count(); nFile++)
      {
          CGexFileInGroup * pFile = pFilesList.at(nFile);

          if (pFile)
          {
            ptTest->findSublotOffset(nBegin, nEnd, pFile->lFileID);
            for (int nCount = lstParts.count()-1; nCount >= 0; --nCount)
            {
                nSample = lstParts.at(nCount) - nBegin;

                if (nSample >= nBegin && nSample < nEnd)
                    pFilesList.at(nFile)->pPartInfoList.removeAt(nSample);
            }
          }
      }
  }

  // First loop on testcell, remove all filtered parts
  while(ptTest)
  {
    // Erase ALL samples tested at given X,Y die coordinates (if retests, allows removing all retests instances at once!)
    if(ptTest->m_testResult.count() > 0)
    {
      // Mark samples as deleted for all parts tested at this DieX,Y location
      for (lPartOffset = lstParts.constBegin(); lPartOffset != lstParts.constEnd(); ++lPartOffset)
        ptTest->m_testResult.deleteResultAt(*lPartOffset);

      // Clean samples by moving deleted result to the end of the array
      if (bOptionsCleanSamples)
      {
          // Compress data
          ptTest->m_testResult.cleanDeletedResults();

          ptTest->ldSamplesExecs -= lstParts.count();

          for (int nFile = 0; nFile < pFilesList.count(); nFile++)
              ptTest->pSamplesInSublots[0] = pFilesList.at(nFile)->pPartInfoList.count();
      }
    }

    // Next test
    ptTest = ptTest->GetNextTest();
  };

  // Get the first test in the testlist
  ptTest	= cMergedData.ptMergedTestList;

  /////////////////////////////////////////////////////////////////////////////////////
  // Second loop, update vector list for functional tests and compute the statistics

  // tmp variable(s)
  int nTestCellRsltCnt;

  while(ptTest)
  {
      nTestCellRsltCnt = ptTest->m_testResult.count();

      //if (ptTest->bTestType == 'F' && ptTest->m_testResult.count() > 0)
      if (ptTest->bTestType == 'F' && nTestCellRsltCnt > 0)
      {
          // clear the existing one
          ptTest->mVectors.clear();

          //Compute execs and fails
          CFunctionalTest cVectorResult;
          //for (int iIndex = 0; iIndex < ptTest->ldSamplesExecs; iIndex++)
          for (int iIndex = 0; iIndex < nTestCellRsltCnt; iIndex++)
          {
              if (ptTest->m_testResult.isValidResultAt(iIndex))
              {
                  cVectorResult.lExecs++;

                  if (ptTest->m_testResult.resultAt(iIndex) == 0)
                      cVectorResult.lFails++;
              }
          }

          ptTest->mVectors[cVectorResult.strVectorName] = cVectorResult;
      }

      //if(ptTest->m_testResult.count() > 0)
      if(nTestCellRsltCnt > 0)
      {
          float lfExponent = ScalingPower(ptTest->res_scal);
          m_cStats.ComputeLowLevelTestStatistics(ptTest, lfExponent);
          m_cStats.ComputeBasicTestStatistics(ptTest, true);
          m_cStats.RebuildHistogramArray(ptTest, ReportOptions.iHistogramType);
          QString pf=ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();
          m_cStats.ComputeAdvancedDataStatistics(ptTest, true, pf=="percentile"?true:false); //cStats.ComputeAdvancedDataStatistics(ptTest, true, ReportOptions.bStatsCpCpkPercentileFormula);
      }

      // Next test
      ptTest = ptTest->GetNextTest();
  }

  UpdateBinTables(true);
  UpdateBinTables(false);
}

void CGexGroupOfFiles::SwitchLimitIndex()
{
    if (mLimitUsed == multiLimitIfAny)
    {
        // Limit id is a 1-based number, so options has to be greater than 0
        if (mLimitSetId > 0)
        {
            GS::Core::MultiLimitItem*   lMLItem;
            CTest *                     lTestCell	= cMergedData.ptMergedTestList;
            CGexFileInGroup*            lFile       = pFilesList.at(0);
            if (!lFile)
                return;

            // As Limit id is 1-based, we need to transform it to a 0-based index to access the multi-limit item
            // list
            int                         lLimitIndex = mLimitSetId - 1;

            // Loop over all tests for this group
            while (lTestCell)
            {
                // If this test has the limit set, use it.
                // If not, consider this test has no limit.
                lMLItem = lTestCell->GetMultiLimitItem(lLimitIndex);
                if (!lMLItem)
                {
                    lTestCell = lTestCell->GetNextTest();
                    continue;
                }

                // Set the current limit item to
                lTestCell->setCurrentLimitItem(lMLItem);

                // When low limit is valid, update the test low limit and update the test limit flag accordingly
                if (lMLItem->IsValidLowLimit())
                {
                    lTestCell->GetCurrentLimitItem()->lfLowLimit = lMLItem->GetLowLimit();
                    lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
                }
                else
                    lTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;

                // When high limit is valid, update the test high limit and update the test limit flag accordingly
                if (lMLItem->IsValidHighLimit())
                {
                    lTestCell->GetCurrentLimitItem()->lfHighLimit = lMLItem->GetHighLimit();
                    lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
                }
                else
                    lTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;

                lFile->FormatTestLimit(lTestCell,
                                       lTestCell->GetCurrentLimitItem()->szLowL,
                                       lTestCell->GetCurrentLimitItem()->lfLowLimit,
                                       lTestCell->llm_scal);
                lFile->FormatTestLimit(lTestCell,
                                       lTestCell->GetCurrentLimitItem()->szHighL,
                                       lTestCell->GetCurrentLimitItem()->lfHighLimit,
                                       lTestCell->llm_scal);

                lTestCell = lTestCell->GetNextTest();
            };
        }
    }
}


void CGexGroupOfFiles::UpdateTestLimits()
{
    if (mLimitUsed == multiLimitIfAny)
    {
        // Limit id is a 1-based number, so options has to be greater than 0
        if (mLimitSetId > 0)
        {
            GS::Core::MultiLimitItem*   lMLItem;
            CTest *                     lTestCell	= cMergedData.ptMergedTestList;
            CGexFileInGroup*            lFile       = pFilesList.at(0);
            if (!lFile)
                return;

            // Loop over all tests for this group
            while (lTestCell)
            {
                // If this test has the limit set, use it.
                // If not, consider this test has no limit.
                int lNbLimits = lTestCell->MultiLimitItemCount();
                if(lNbLimits < mLimitSetId)
                    lNbLimits = mLimitSetId;

                for(int lLimitIndex = 0; lLimitIndex < lNbLimits; ++lLimitIndex)
                {
                    lMLItem = lTestCell->GetMultiLimitItem(lLimitIndex);
                    if (!lMLItem)
                        continue;

                    // Set the current limit item to
                    lTestCell->setCurrentLimitItem(lMLItem);

                    // When low limit is valid, update the test low limit and update the test limit flag accordingly
                    if (lMLItem->IsValidLowLimit())
                    {
                        lTestCell->GetCurrentLimitItem()->lfLowLimit = lMLItem->GetLowLimit();
                        lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
                    }
                    else
                        lTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;

                    // When high limit is valid, update the test high limit and update the test limit flag accordingly
                    if (lMLItem->IsValidHighLimit())
                    {
                        lTestCell->GetCurrentLimitItem()->lfHighLimit = lMLItem->GetHighLimit();
                        lTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
                    }
                    else
                        lTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;

                    lFile->FormatTestLimit(lTestCell,
                                           lTestCell->GetCurrentLimitItem()->szLowL,
                                           lTestCell->GetCurrentLimitItem()->lfLowLimit,
                                           lTestCell->llm_scal);
                    lFile->FormatTestLimit(lTestCell,
                                           lTestCell->GetCurrentLimitItem()->szHighL,
                                           lTestCell->GetCurrentLimitItem()->lfHighLimit,
                                           lTestCell->llm_scal);
                }

                lTestCell->setCurrentLimitItem(mLimitSetId - 1);
                lTestCell = lTestCell->GetNextTest();
            };
        }
    }
}

///////////////////////////////////////////////////////////
// Apply filter defined for each file in the group
///////////////////////////////////////////////////////////
void CGexGroupOfFiles::postLoadProcessing()
{
  CGexFileInGroup *			pFile = NULL;
  QList<long>					lstUnfilteredParts;

  // OPTIONS
  QVariant  qvHBinListOption    = ReportOptions.GetOption(QString("samples"), QString("hbin_list"));
  QString   strHBinLIstOption   = qvHBinListOption.toString();
  QString   lOptionsSublotOrder = ReportOptions.GetOption("dataprocessing", "sublot_sorting").toString();
  GS::QtLib::Range	cgrHBinListOptionGexRange;
  cgrHBinListOptionGexRange.SetRange(strHBinLIstOption);

  // Remove filtered tests
  QListIterator<CGexFileInGroup*> itFilesList(pFilesList);

  while (itFilesList.hasNext())
  {
    pFile = itFilesList.next();

    // The merge list pointer is passed as reference because the new test
    // can be created in the front of the list. So we shouldn't lose it.
    pFile->applyTestCreation(&cMergedData.ptMergedTestList);

    // apply filter defined with dataset config file
    pFile->applyTestFilter(&cMergedData);
  }

  // Update all file with pointer on merged list
  itFilesList.toFront();

  while (itFilesList.hasNext())
  {
    pFile = itFilesList.next();

    // update test list pointer for each pFile
    pFile->ptTestList = cMergedData.ptMergedTestList;
  }

  // Override current limits
  UpdateTestLimits();

  // Update all file with pointer on merged list
  itFilesList.toFront();

  while (itFilesList.hasNext())
  {
    pFile = itFilesList.next();

    // Extracts list of unfiltered parts
    pFile->fillUnfilteredPartsList(lstUnfilteredParts);
  }

  // Remove all results
  if (lstUnfilteredParts.count())
    removeParts(lstUnfilteredParts);

  // Samples filtering
  // Remove all sample from parametric test which correspond to a HBin in the bin list
  // if (ReportOptions.m_pSamplesHBinList != NULL)
  if(qvHBinListOption.isValid())
  {
      CTest * pTestHardBin	= NULL;
      CTest * pTestCell		= NULL;

      itFilesList.toFront();

      while (itFilesList.hasNext())
      {
          pFile = itFilesList.next();

          // Find the Hard Bin test cell
          int lR=pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &pTestHardBin, true, false);
          if(lR == 1)
          {
              // Loop on all part for this test
              //for (int nIndex = 0; nIndex < pTestHardBin->ldSamplesExecs; ++nIndex)
              for (int nIndex = 0; nIndex < pTestHardBin->m_testResult.count(); ++nIndex)
              {
                  // If part is valid and binning doesn't belong to the filter list, set all parametric test to invalid state
                if (pTestHardBin->m_testResult.isValidResultAt(nIndex) &&
                   cgrHBinListOptionGexRange.Contains(
                     static_cast<unsigned long>(
                       pTestHardBin->m_testResult.resultAt(nIndex))) == false)
                  {
                      pTestCell	= cMergedData.ptMergedTestList;

                      while (pTestCell)
                      {
                          // Invalid only parametric result
                          //if ((pTestCell->bTestType == 'P' || pTestCell->bTestType == 'M') && pTestCell->ldSamplesValidExecs > 0 && pTestCell->m_testResult.count() > 0)
                          if ((pTestCell->bTestType == 'P' || pTestCell->bTestType == 'M') &&  (pTestCell->ldSamplesValidExecs > 0) && pTestCell->m_testResult.isValidIndex(nIndex))
                              pTestCell->m_testResult.invalidateResultAt(nIndex);

                          pTestCell = pTestCell->GetNextTest();
                      }
                  }
              }
          }
          else if (lR==-1)
          {
              GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
              return;
          }

      }

  }

  // Check if the sublot test result should be sorted by xy coordinates

  if (lOptionsSublotOrder != "none")
  {
    itFilesList.toFront();
    while (itFilesList.hasNext())
    {
      pFile = itFilesList.next();

      // apply filter defined with dataset config file
      pFile->orderRun(lOptionsSublotOrder);
    }
  }
}
bool CGexGroupOfFiles::FindPartId(int coordX, int coordY, QString& partId)
{
    CGexFileInGroup *   lFile       = NULL;
    int                 lRunIndex   = -1;
    
    if (pFilesList.count() > 0)
        lFile = pFilesList.first();
    
    if (lFile)
    {
        CTest * lTestDieX = NULL;
        CTest * lTestDieY = NULL;
        
        // Retrieve CTest for Die X
        if (lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST,  &lTestDieX, true, false) != 1)
        {
            return false;
        }
        
        // Retrieve CTest for Die Y
        if (lFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST,  &lTestDieY, true, false) != 1)
        {
            return false;
        }
        
        int lMinSample  = qMin(lTestDieX->m_testResult.count(), lTestDieY->m_testResult.count());
        
        // Look for run index matching with X and Y coordinate
        for(int lIdx=0; lIdx < lMinSample && lRunIndex == -1; ++lIdx)
        {
            if((lTestDieX->m_testResult.resultAt(lIdx) == (double) coordX) &&
               (lTestDieY->m_testResult.resultAt(lIdx) == (double) coordY))
            {
                lRunIndex = lIdx;
            }
        }
    }
    
    if (lRunIndex >= 0)
    {
        // Look for CPartInfo object matching with the run Index found for given X and Y coordinates
        CPartInfo * lPartInfo   = NULL;
        
        for (int idxFile = 0; idxFile < pFilesList.count() && lPartInfo == NULL; ++idxFile)
        {
            CGexFileInGroup * pTmpFile = pFilesList.at(idxFile);
            
            if (pTmpFile)
            {
                if (lRunIndex < pTmpFile->pPartInfoList.count())
                    lPartInfo = pTmpFile->pPartInfoList.at(lRunIndex);
                else
                    lRunIndex -= pTmpFile->pPartInfoList.count();
            }
        }
        
        if (lPartInfo != NULL)
        {
            partId = lPartInfo->getPartID();
            return true;
        }
    }
    
    return false;
}

CTest* CGexGroupOfFiles::FindTestCell(unsigned int lTestNumber, QString strTestName, int lPinmapIndex)
{
    CTest* t=0;
    CGexFileInGroup* f;
    for(int j=0; j<pFilesList.size(); ++j)
    {
        f = pFilesList[j];
        if (!f)
            continue;
        int i=f->FindTestCell(lTestNumber, lPinmapIndex, &t, false, false, strTestName.toLatin1().data());
        if (i==-1)
        {
            // GSLOG(3, "Failed to create new test (low mem,...)");
            return 0;
        }
        if (i!=1)
            continue;
        if (t)
            return t;
    }
    return t;
}

bool CGexGroupOfFiles::addTestSiteLimits(CTest * ptTestCell, int iProcessSite, BYTE bSite,
                                         double dLowLimit, double dHighLimit, BYTE optFlag, time_t limitTimeStamp)
{
    if (m_mapTestSiteLimits.contains(ptTestCell))
    {
        tdHashSiteLimits& lHashSiteLimits = m_mapTestSiteLimits.find(ptTestCell).value();

        if (mLimitScope == LimitPerSite)
        {
            // Filtering by site activated
            if (iProcessSite == -1 || iProcessSite == bSite)
            {
                if (lHashSiteLimits.contains("processedSites"))
                {
                    if (updateTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp,
                                             lHashSiteLimits.find("processedSites").value()) == false)
                    {
                        return false;
                    }
                }
                else
                {
                    CGexSiteLimits lSiteLimits;
                    createTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp, lSiteLimits);

                    lHashSiteLimits.insert("processedSites", lSiteLimits);
                }
            }

            if (updateTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp,
                                    lHashSiteLimits.find("allSites").value()) == false)
            {
                return false;
            }
        }
        else if (mLimitScope == LimitOverAllSite)
        {
            if (updateTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp,
                                     lHashSiteLimits.find("allSites").value()) == false)
            {
                return false;
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid limit scope detected");
            return false;
        }
    }
    else
    {
        tdHashSiteLimits lHashSiteLimits;

        if (mLimitScope == LimitPerSite)
        {
            CGexSiteLimits lSiteLimits;

            createTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp, lSiteLimits);

            // Filtering by site activated
            if (iProcessSite == -1 || iProcessSite == bSite)
            {
                lHashSiteLimits.insert("processedSites", lSiteLimits);
            }

            lHashSiteLimits.insert("allSites", lSiteLimits);
        }
        else if (mLimitScope == LimitOverAllSite)
        {
            CGexSiteLimits lSiteLimits;

            createTestSiteLimits(dLowLimit, dHighLimit, optFlag, limitTimeStamp, lSiteLimits);

            lHashSiteLimits.insert("allSites", lSiteLimits);

        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid limit scope detected");
            return false;
        }

        m_mapTestSiteLimits.insert(ptTestCell, lHashSiteLimits);
    }

    return true;
}

bool CGexGroupOfFiles::updateTestSiteLimits(double dLowLimit, double dHighLimit, BYTE optFlag, time_t limitTimeStamp,
                                            CGexSiteLimits& lSiteLimits)
{
    bool lSucceed   = true;
    bool sortOnDate = (m_strDatasetSorting == "date");

    if (dLowLimit != -C_INFINITE && dHighLimit != C_INFINITE)
    {
        if (dLowLimit > dHighLimit)
        {
            double lSwapLimit = dHighLimit;

            dHighLimit  = dLowLimit;
            dLowLimit   = lSwapLimit;
        }
    }

    switch (mLimitSelection)
    {
        case LargestLimit:
            if ((optFlag & 0x10) == 0 && dLowLimit < lSiteLimits.lowLimit() && dLowLimit != -C_INFINITE)
                lSiteLimits.setLowLimit(dLowLimit);

            if ((optFlag & 0x20) == 0 && dHighLimit > lSiteLimits.highLimit() && dHighLimit != C_INFINITE)
                lSiteLimits.setHighLimit(dHighLimit);
            break;

        case FirstLimit:
            if ((sortOnDate == true && lSiteLimits.limitTimeStamp() > limitTimeStamp && limitTimeStamp > 0) ||
                lSiteLimits.limitTimeStamp() == 0)
            {
                if ((optFlag & 0x10) == 0 && dLowLimit != -C_INFINITE)
                    lSiteLimits.setLowLimit(dLowLimit);

                if ((optFlag & 0x20) == 0 && dHighLimit != C_INFINITE)
                    lSiteLimits.setHighLimit(dHighLimit);

                // If one of the limit is valid, set the time stamp for this limit set
                if ((optFlag & 0x10) == 0 || (optFlag & 0x20) == 0)
                    lSiteLimits.setLimitTimeStamp(limitTimeStamp);
            }
            break;

        case LastLimit:
            if (sortOnDate == false ||
               (lSiteLimits.limitTimeStamp() < limitTimeStamp && limitTimeStamp > 0))
            {
                if ((optFlag & 0x10) == 0 && dLowLimit != -C_INFINITE)
                    lSiteLimits.setLowLimit(dLowLimit);

                if ((optFlag & 0x20) == 0 && dHighLimit != C_INFINITE)
                    lSiteLimits.setHighLimit(dHighLimit);

                // If one of the limit is valid, set the time stamp for this limit set
                if ((optFlag & 0x10) == 0 || (optFlag & 0x20) == 0)
                    lSiteLimits.setLimitTimeStamp(limitTimeStamp);
            }
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Unknown site limit criterion detected");
            lSucceed = false;
            break;
    }

    return lSucceed;
}

void CGexGroupOfFiles::createTestSiteLimits(double dLowLimit, double dHighLimit, BYTE optFlag,
                                            time_t limitTimeStamp, CGexSiteLimits &lSiteLimits)
{
    if (dLowLimit != -C_INFINITE && dHighLimit != C_INFINITE)
    {
        if (dLowLimit > dHighLimit)
        {
            double lSwapLimit = dHighLimit;

            dHighLimit  = dLowLimit;
            dLowLimit   = lSwapLimit;
        }
    }

    if ((optFlag & 0x10) == 0 && dLowLimit != -C_INFINITE)
        lSiteLimits.setLowLimit(dLowLimit);

    if ((optFlag & 0x20) == 0 && dHighLimit != C_INFINITE)
        lSiteLimits.setHighLimit(dHighLimit);

    // If one of the limit is valid, set the time stamp for this limit set
    if ((optFlag & 0x10) == 0 || (optFlag & 0x20) == 0)
        lSiteLimits.setLimitTimeStamp(limitTimeStamp);
}

bool CGexGroupOfFiles::getTestSiteLimits(CTest * ptTestCell, int iProcessSite, BYTE bSite, double& dLowLimit, double& dHighLimit)
{
    if (m_mapTestSiteLimits.contains(ptTestCell))
    {
        tdHashSiteLimits& lHashSiteLimits = m_mapTestSiteLimits.find(ptTestCell).value();

        if (mLimitScope == LimitPerSite)
        {
            // Filtering by site activated
            if (iProcessSite == -1 || iProcessSite == bSite)
            {
                if (lHashSiteLimits.contains("processedSites"))
                    lHashSiteLimits.find("processedSites").value().limits(dLowLimit, dHighLimit);
                else
                    lHashSiteLimits.find("allSites").value().limits(dLowLimit, dHighLimit);
            }
            else
                lHashSiteLimits.find("allSites").value().limits(dLowLimit, dHighLimit);
        }
        else if (mLimitScope == LimitOverAllSite)
        {
            lHashSiteLimits.find("allSites").value().limits(dLowLimit, dHighLimit);
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Invalid limit scope detected");
            return false;
        }

        return true;
    }

    return false;
}

//returns the value for the given condition name in a variant object
QVariant CGexGroupOfFiles::GetTestConditionsValue(const QString &strTestConditionName){
    if(m_oConditionsMaps.contains(strTestConditionName))
        return m_oConditionsMaps[strTestConditionName];
    return QVariant();
}
//add a new Test conditions name and value to the group. If test conditions name already exists, erase the old value with the new one.
void CGexGroupOfFiles::AddTestConditions(const QString &strTestConditionName, const QString &strTestConditionValue){

    if(!m_oConditionsMaps.contains(strTestConditionName))
        m_oConditionsMaps.insert(strTestConditionName, strTestConditionValue);

}
//remove a test condition from the group
void CGexGroupOfFiles::RemoveTestConditions(const QString &strTestConditionName){
    if(m_oConditionsMaps.contains(strTestConditionName))
         m_oConditionsMaps.remove(strTestConditionName);
}
