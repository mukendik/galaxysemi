#include "g-trigger.h"


#include <QtGui>

namespace GS
{
namespace GTrigger
{

///////////////////////////////////////////////////////////
// Structure filled when reading PROMIS file
///////////////////////////////////////////////////////////
CPromisTextFile::CPromisTextFile()
{
  clear();
}

void CPromisTextFile::clear()
{
  strPromisLotID = "";		// Column1
  iWafersInSplit = 0;		// Col2 - Total wafers in spplit-lot
  strFacilityID = "";		// Col3 - FabSite (eg: SCFAB)
  strEquipmentID = "";		// Col4 - Equipment ID (eg: 3301400)
  strDsPartNumber = "";	// Col5 - DS Part Number (eg: DSANBADT40BCAA6A.04)
  strGeometryName = "";		// Col6 - Geometry (eg: ANBAD40BCAA)
  iGrossDie = -1;		// Col7 - Gross die count
  lfDieW = 0;				// Col8 - DieX size
  lfDieH = 0;				// Col9 - DieY size
  iFlat = 0;				// Col10 - Flat orientation
  strSiteLocation = "";	// Col11 - Site Location
}

////////////////////////////////////////////////////////////////////////////////////////////
// CInputFile class: utility class to read lines from input file
////////////////////////////////////////////////////////////////////////////////////////////
CInputFile::CInputFile(const QString & strFile)
{
  m_strFileName = strFile;
  m_pclFile = NULL;
  m_uiLineNb = 0;
}

bool CInputFile::Open()
{
  // Make sure file is closed, then open it
  if(m_pclFile)
  {
    m_pclFile->close();
    delete m_pclFile;
  }
  m_pclFile = new QFile(m_strFileName);
  if(m_pclFile->open(QIODevice::ReadOnly) == false)
  {
    delete m_pclFile;
    m_pclFile = NULL;
    return false;
  }

  // Set text stream, reset line nb
  m_clStream.setDevice(m_pclFile);
  m_uiLineNb = 0;

  return true;
}

void CInputFile::Close()
{
  if(m_pclFile)
  {
    m_pclFile->close();
    delete m_pclFile;
    m_pclFile = NULL;
  }
}

bool CInputFile::NextLine(QString & strLine)
{
  // Make sure file is opened
  if(!m_pclFile)
    return false;

  QString strStrippedLine;
  // Read line until EOF or non-empty line read
  do
  {
    strLine = m_clStream.readLine();
    strStrippedLine = strLine.trimmed();
    m_uiLineNb++;
  }
  while(!strLine.isNull() && strStrippedLine.isEmpty());

  // Check if EOF
  if(strLine.isNull())
    return false;
  return true;
}

///////////////////////////////////////////////////////////
// Wafer files (used when generating GTF)
///////////////////////////////////////////////////////////
WaferFiles::WaferFiles(int WaferNb, const GS::GTrigger::WaferType Type /*= GS::GTrigger::eUnknownWafer*/)
{
    mWaferNb = WaferNb;
    mWaferType = Type;
    // Build WaferID as 2 digits number. eg: 03, except if < 0 (not known yet)
    if(WaferNb >= 0)
        mWaferID = QString("%1").arg(WaferNb, 2, 10, QLatin1Char('0'));
}

WaferFiles::~WaferFiles(void)
{
    // Clear list of input files
    mDataFiles.clear();
}

void WaferFiles::AddDataFile(const QString & FileName, const QDateTime & TimeStamp/* = QDateTime::currentDateTime()*/)
{
    QPair<QDateTime, QString> pair(TimeStamp, FileName);
    mDataFiles.append(pair);
}

void WaferFiles::DataFiles(QStringList & DataFiles) const
{
    DataFiles.clear();
    for (int lIndex = 0; lIndex < mDataFiles.size(); ++lIndex)
    {
        DataFiles.append(mDataFiles.at(lIndex).second);
    }
}

unsigned int WaferFilesList::DistinctWafers(const GS::GTrigger::WaferType Type, bool IgnoreWafersWithNoFiles)
{
    unsigned int lCount=0;
    const_iterator lWaferIt;
    for(lWaferIt = constBegin(); lWaferIt != constEnd(); ++lWaferIt)
    {
        if((lWaferIt.value().GetWaferType() == Type) && (!IgnoreWafersWithNoFiles || (lWaferIt.value().Count() > 0)))
            ++lCount;
    }
    return lCount;
}

unsigned int WaferFilesList::DistinctWafers(bool IgnoreWafersWithNoFiles)
{
    unsigned int lCount=0;
    const_iterator lWaferIt;
    for(lWaferIt = constBegin(); lWaferIt != constEnd(); ++lWaferIt)
    {
        if(!IgnoreWafersWithNoFiles || (lWaferIt.value().Count() > 0))
            ++lCount;
    }
    return lCount;
}

unsigned int WaferFilesList::DistinctFiles(const GS::GTrigger::WaferType Type)
{
    unsigned int lCount=0;
    const_iterator lWaferIt;
    for(lWaferIt = constBegin(); lWaferIt != constEnd(); ++lWaferIt)
    {
        if(lWaferIt.value().GetWaferType() == Type)
            lCount+=lWaferIt.value().Count();
    }
    return lCount;
}

unsigned int WaferFilesList::DistinctFiles()
{
    unsigned int lCount=0;
    const_iterator lWaferIt;
    for(lWaferIt = constBegin(); lWaferIt != constEnd(); ++lWaferIt)
        lCount+=lWaferIt.value().Count();
    return lCount;
}

///////////////////////////////////////////////////////////
// Wafer data (used when creating summary log file)
///////////////////////////////////////////////////////////
WaferInfo::WaferInfo(void)
{
  Clear();
}

///////////////////////////////////////////////////////////
// Reset wafer structure
///////////////////////////////////////////////////////////
void	WaferInfo::Clear(void)
{
  strPassFail = "*F*";
  lWaferID = -1;
  iGrossDie=-1;
  lTotalDies=0;

  lTotalGood_PrePAT=0;
  lTotalGood_PostPAT=0;

  lfYield_PrePAT=0;
  lfYield_PostPAT=0;
  lfDeltaYield=0;

  eWaferType=eUnknownWafer;
  bValidWafer=false;

  // Clear binnings
  strlBinNames.clear();
  strlBinNumbers.clear();
  strlBinCounts.clear();
}

///////////////////////////////////////////////////////////
// Summary data (used when creating summary log file)
///////////////////////////////////////////////////////////
SummaryInfo::SummaryInfo(const QString & strOutFolderLog, const QString & strProductionFolder, const QString & strSplitLotID, const QString & strLotID)
{
  m_strOutFolderLog = strOutFolderLog;
  m_strProductionFolder = strProductionFolder;
  m_strLotID = strLotID;
  m_strSplitLotID = strSplitLotID;
  Clear();
}

SummaryInfo::SummaryInfo(const QString & strOutFolderLog, const QString & strProductionFolder)
{
  m_strOutFolderLog = strOutFolderLog;
  m_strProductionFolder = strProductionFolder;
  Clear();
}

SummaryInfo::~SummaryInfo(void)
{
  Clear();
}

///////////////////////////////////////////////////////////
// Reset wafer structure
///////////////////////////////////////////////////////////
void SummaryInfo::Clear(void)
{
  m_strProduct = "";
  m_strProgram = "";
  m_strRecipe = "";
  m_strAlarmInfo = "";

  // Free memory: delete QMap objects.
  QMap <long,WaferInfo*>::Iterator	itWafer;
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    delete *itWafer;
}

bool SummaryInfo::Contains(long lWaferID)
{
  return m_cWaferList.contains(lWaferID);
}

bool SummaryInfo::Add(WaferInfo *pWafer)
{
  // Check if wafer already exists
  WaferInfo* pWafer2 = At(pWafer->lWaferID);

  // If wafer doesn't exist, add it
  if(pWafer2 == NULL)
  {
    m_cWaferList[pWafer->lWaferID] = pWafer;
    return true;
  }

  // If wafer entry already exists, only take latest (most recent one).
  if(pWafer->cDateTime > pWafer2->cDateTime)
  {
    // Current new entry is the most recent: overwrite previous wafer entry saved
    Remove(pWafer->lWaferID);
    m_cWaferList[pWafer->lWaferID] = pWafer;
    return true;
  }

  // No wafer added
  return false;
}

WaferInfo* SummaryInfo::At(long lWaferID)
{
  if(!Contains(lWaferID))
    return NULL;

  return m_cWaferList[lWaferID];
}

void SummaryInfo::Remove(long lWaferID)
{
  WaferInfo *pWafer = At(lWaferID);
  if(pWafer)
    delete pWafer;
  m_cWaferList.remove(lWaferID);
}

bool SummaryInfo::WriteSummaryLogFile(QString & SummaryLogFileName, const QString & AppName)
{
  // If no wafers, exit
  if(m_cWaferList.count() <= 0)
    return false;

  // Create summary.log file.
  QFile								cLogFile;
  QTextStream						hLogFile;
  QString							strString;
  WaferInfo							*pWafer;
  QMap <long,WaferInfo*>::Iterator	itWafer;

  SummaryLogFileName = m_strProductionFolder + "/" + m_strSplitLotID + "-summary.log";
  cLogFile.setFileName(SummaryLogFileName);
  if(cLogFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    return false;		// Failed opening .log file.

  // Write into file
  hLogFile.setDevice(&cLogFile);	// Assign file handle to data stream
  hLogFile << "# PAT-Man SUMMARY log file" << endl;
  hLogFile << "# Created by: " << AppName << endl << endl;

  pWafer = *(m_cWaferList.begin());
  hLogFile << "Date           : " << QDateTime::currentDateTime().toString("dd MMMM yyyy hh:mm:ss") << endl;
  hLogFile << "Geometry       : " << m_strProduct << endl;
  hLogFile << "Lot            : " << m_strSplitLotID << endl;
  hLogFile << "Program        : " << m_strProgram << endl;
  if(pWafer->eWaferType == GS::GTrigger::ePatWafer)
  {
    hLogFile << "PAT Recipe     : " << m_strRecipe << endl;
    hLogFile << "Alarm level    : " << m_strAlarmInfo << " - PAT Yield loss alarm level" << endl;
  }
  hLogFile << endl;

  // List Wafer#s
  hLogFile << "-----------------------";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    hLogFile << "--------";
  hLogFile << "--------" << endl;

  hLogFile << "Wafer#                :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
  {
    strString.sprintf("%6ld  ",(*itWafer)->lWaferID);
    hLogFile << strString;
  }
  hLogFile << endl;

  hLogFile << "-----------------------";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    hLogFile << "--------";
  hLogFile << "--------" << endl;

  hLogFile << "Total Dies            :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
  {
    strString.sprintf("%7ld ",(*itWafer)->lTotalDies);
    hLogFile << strString;
  }
  hLogFile << endl;

  hLogFile << "Gross Die (GPDW)      :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
  {
    if((*itWafer)->iGrossDie > 0)
      strString.sprintf("%7ld ",(*itWafer)->iGrossDie);
    else
      strString.sprintf("    n/a ");
    hLogFile << strString;
  }
  hLogFile << endl;

  if(pWafer->eWaferType == GS::GTrigger::ePatWafer)
    hLogFile << "Good dies (Pre-PAT)   :";
  else
    hLogFile << "Good dies             :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
  {
    strString.sprintf("%7ld ",(*itWafer)->lTotalGood_PrePAT);
    hLogFile << strString;
  }
  hLogFile << endl;

  if(pWafer->eWaferType == GS::GTrigger::ePatWafer)
    hLogFile << "Yield level (Pre-PAT) :";
  else
    hLogFile << "Yield level           :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
  {
    strString.sprintf("%6.2f%% ",(*itWafer)->lfYield_PrePAT);
    hLogFile << strString;
  }
  hLogFile << endl;

  if(pWafer->eWaferType == GS::GTrigger::ePatWafer)
  {
    hLogFile << "Good dies (Post-PAT)  :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    {
      strString.sprintf("%7ld ",(*itWafer)->lTotalGood_PostPAT);
      hLogFile << strString;
    }
    hLogFile << endl;

    hLogFile << "Yield level (Post-PAT):";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    {
      strString.sprintf("%6.2f%% ",(*itWafer)->lfYield_PostPAT);
      hLogFile << strString;
    }
    hLogFile << endl;

    hLogFile << "Yield loss due to PAT :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    {
      strString.sprintf("%6.2f%% ",(*itWafer)->lfDeltaYield);
      hLogFile << strString;
    }
    hLogFile << endl;

    hLogFile << "-----------------------";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
      hLogFile << "--------";
    hLogFile << "--------" << endl;

    hLogFile << "PAT alarm status      :";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    {
      strString.sprintf("%7s ",(*itWafer)->strPassFail.toLatin1().data());
      hLogFile << strString;
    }
    hLogFile << endl;
  }

  hLogFile << "-----------------------";
    for (itWafer = m_cWaferList.begin(); itWafer != m_cWaferList.end(); ++itWafer )
    hLogFile << "--------";
  hLogFile << "--------" << endl;


  // close file
  cLogFile.close();

  return true;
}

bool SummaryInfo::WriteWaferLogFile(const QString & AppName, long WaferID, QString & WaferLogFileName,
                                    float ValidWaferThreshold_GrossDieRatio, bool WithExt/*=true*/)
{
  // If wafer not found, return
  WaferInfo *pWafer = At(WaferID);
  if(pWafer == NULL)
    return false;

  // Create wafer.log file.
  QFile									cLogFile;
  QTextStream								hLogFile;
  QString									strWaferID, strWaferTime;
  QMap <long,WaferInfo*>::Iterator	itWafer;
  QStringList::iterator					it;

  strWaferID.sprintf("%02ld", WaferID);
  strWaferTime = pWafer->cDateTime.toString("yyyyMMdd.hhmmss");
  WaferLogFileName = m_strOutFolderLog + "/" + m_strSplitLotID + "-" + strWaferID + "_" + strWaferTime;

  CreateWaferLogFileDirectory( WaferLogFileName );

  if(WithExt)
    WaferLogFileName += ".log";
  cLogFile.setFileName(WaferLogFileName);
  if(cLogFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    return false;		// Failed opening .log file.

  // Write into file
  hLogFile.setDevice(&cLogFile);	// Assign file handle to data stream
  hLogFile << "# G-Trigger FET_NOPAT wafer log file" << endl;
  hLogFile << "# Created by: " << AppName << endl;
  hLogFile << "#" << endl;
  hLogFile << "# Global info" << endl;
  hLogFile << "Date," << QDateTime::currentDateTime().toString("dd MMMM yyyy hh:mm:ss") << endl;
  hLogFile << "SetupTime," << pWafer->cDateTime.toString("ddd MM dd hh:mm:ss yyyy") << endl;
  hLogFile << "Product," << m_strProduct << endl;
  hLogFile << "Lot," << m_strLotID << endl;
  hLogFile << "SubLot," << m_strSplitLotID << endl;
  hLogFile << "WaferID," << strWaferID << endl;
  hLogFile << "#" << endl;
  hLogFile << "# STDF Wafer details" << endl;
  hLogFile << "Stdf_TotalDies," << QString::number(pWafer->lTotalDies) << endl;
  hLogFile << "Stdf_GrossDiesPerWafer," << QString::number(pWafer->iGrossDie) << endl;
  hLogFile << "#" << endl;
  hLogFile << "# Binning info" << endl;
  hLogFile << "BinName,";
  for(it=pWafer->strlBinNames.begin(); it!=pWafer->strlBinNames.end(); it++)
    hLogFile << *it << ",";
  hLogFile << endl;
  hLogFile << "Bin#,";
  for(it=pWafer->strlBinNumbers.begin(); it!=pWafer->strlBinNumbers.end(); it++)
    hLogFile << *it << ",";
  hLogFile << endl;
  hLogFile << "BinCount,";
  for(it=pWafer->strlBinCounts.begin(); it!=pWafer->strlBinCounts.end(); it++)
    hLogFile << *it << ",";
  hLogFile << endl;
  hLogFile << "#" << endl;
  hLogFile << "# Yield details" << endl;
  hLogFile << "GoodParts," << QString::number(pWafer->lTotalGood_PrePAT) << endl;
  hLogFile << "#" << endl;
  hLogFile << "# Promis details" << endl;
  hLogFile << "# GrossDie=" << QString::number(pWafer->iGrossDie) << endl;
  hLogFile << "# #Define Gross die ratio to be used to accept a wafer (parts > ratio*GrossDie)" << endl;
  hLogFile << "# ValidWaferThreshold_GrossDieRatio=" << QString::number(ValidWaferThreshold_GrossDieRatio) << endl;
  hLogFile << endl;

  // close file
  cLogFile.close();

  return true;
}

void SummaryInfo::CreateWaferLogFileDirectory(const QString &aWaferLogFilePath) const
{
    const QString &lPath = QFileInfo{ aWaferLogFilePath }.absolutePath();
    if( ! QDir{ lPath }.exists() )
        QDir{}.mkpath( lPath );
}

bool SummaryList::GetNextSummaryFile(QString & strSummaryFile)
{
  // Check if there are still items in the list
  if(count() > 0)
  {
    // Get first name in list.
    strSummaryFile = first();
    // Remove this file from the list
    erase(begin());
    // Extract folder, file
    m_strSummaryFolder = strSummaryFile.section('|', 0, 0);
    strSummaryFile = strSummaryFile.section('|', 1, 1);
    return true;
  }

  return false;
}

} // namespace GS
} // namespace GTrigger
