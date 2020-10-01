/******************************************************************************!
 * \file drillData3DInfo.cpp
 * \brief Interactive Drill: 3D Data Mining
 ******************************************************************************/
#include <gqtl_log.h>
#include "drillData3DInfo.h"
#include "browser_dialog.h"
#include "gex_report.h"
#include "report_options.h"
#include "gex_group_of_files.h"

// main.cpp
extern GexMainwindow* pGexMainWindow;

// in report_build.cpp
extern CGexReport* gexReport;  // Handle to report class
extern CReportOptions ReportOptions;  // Holds options (report_build.h)

namespace Gex {

/******************************************************************************!
 * \fn DrillData3DInfo
 * \brief Constructor
 ******************************************************************************/
DrillData3DInfo::DrillData3DInfo(int nGroupID  /*= 0*/, int nFileID  /*= 0*/)
{
  GSLOG(SYSLOG_SEV_DEBUG, QString("DrillData3DInfo creator on group %1 file %2...")
        .arg(nGroupID).arg(nFileID).toLatin1().constData());
  setObjectName("DrillData3DInfo");

  m_nGroupID    = nGroupID;
  m_nFileID     = nFileID;
    Reset();

  // Group#
  if (m_nGroupID >= 0 && m_nGroupID < gexReport->getGroupsList().count())
  {
    m_pGroup = gexReport->getGroupsList().at(m_nGroupID);  // Group#
  }
  else
  {
    m_pGroup = gexReport->getGroupsList().size() > 0 ?
                gexReport->getGroupsList().first() : NULL;  // Group#1
    m_nGroupID = 0;
  }

  if (m_pGroup)
  {
    if (m_nFileID >= 0 && m_nFileID < m_pGroup->pFilesList.count())
    {
      m_pFile = (m_pGroup->pFilesList.isEmpty()) ? NULL :
        (m_pGroup->pFilesList.at(m_nFileID));
    }
    else
    {
      m_pFile = (m_pGroup->pFilesList.isEmpty()) ? NULL :
        (m_pGroup->pFilesList.first());
      m_nFileID = 0;
    }
  }
  GSLOG(SYSLOG_SEV_DEBUG,
        QString("DrillData3DInfo creator : group=%1 fileID=%2").
            arg(m_pGroup?m_pGroup->strGroupName.toLatin1().data():"N/A").
            arg(m_pFile?QString::number(m_pFile->lFileID):QString("N/A")).
            toLatin1().constData());
}

/******************************************************************************!
 * \fn DrillData3DInfo
 * \brief Copy constructor
 ******************************************************************************/
DrillData3DInfo::DrillData3DInfo(const DrillData3DInfo& drillDataInfo)
  : QObject(drillDataInfo.parent())
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillData3DInfo copy constructor...");
  *this = drillDataInfo;
}

/******************************************************************************!
 * \fn ~DrillData3DInfo
 * \brief Destructor
 ******************************************************************************/
DrillData3DInfo::~DrillData3DInfo()
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillData3DInfo destructor...");
}

void DrillData3DInfo::Reset()
{
    m_bFirst      = true;
    //m_nTestNumber = -1;
    m_nPinmap     = GEX_PTEST;
    m_nDieX       = -32768;
    m_nDieY       = -32768;

    m_ptCurrentTestCell  = NULL;
    m_ptPreviousTestCell = NULL;
    m_ptNextTestCell     = NULL;
    m_pGroup             = NULL;
    m_pFile              = NULL;
}

/******************************************************************************!
 * \fn operator=
 ******************************************************************************/
DrillData3DInfo&
DrillData3DInfo::operator=(const DrillData3DInfo& drillDataInfo)
{
  if (this != &drillDataInfo)
  {
    m_bFirst              = drillDataInfo.m_bFirst;
    m_nGroupID            = drillDataInfo.m_nGroupID;
    m_nFileID             = drillDataInfo.m_nFileID;
    m_nTestNumber         = drillDataInfo.m_nTestNumber;
    m_nPinmap             = drillDataInfo.m_nPinmap;
    m_nDieX               = drillDataInfo.m_nDieX;
    m_nDieY               = drillDataInfo.m_nDieY;
    m_strCurrentTestLabel = drillDataInfo.m_strCurrentTestLabel;
    m_ptNextTestCell      = drillDataInfo.m_ptNextTestCell;
    m_ptCurrentTestCell   = drillDataInfo.m_ptCurrentTestCell;
    m_ptPreviousTestCell  = drillDataInfo.m_ptPreviousTestCell;
    m_pGroup              = drillDataInfo.m_pGroup;
    m_pFile               = drillDataInfo.m_pFile;
  }

  return *this;
}

/******************************************************************************!
 * \fn hasValidDieCoord
 * \brief Return if die coord are valid
 ******************************************************************************/
bool DrillData3DInfo::hasValidDieCoord() const
{
  return (m_nDieX != -32768 && m_nDieY != -32768);
}

/******************************************************************************!
 * \fn isFakeTest
 * \brief Return if test is fake (eg: create by gex)
 ******************************************************************************/
bool DrillData3DInfo::isFakeTest() const
{
  if (m_nTestNumber >= GEX_TESTNBR_OFFSET_EXT_MIN && m_nTestNumber <=
      GEX_TESTNBR_OFFSET_EXT_MAX) {
    return true;
  } else {
    return false;
  }
}

/******************************************************************************!
 * \fn setDieCoord
 * \brief Sets the die coord to select
 ******************************************************************************/
void DrillData3DInfo::setDieCoord(int nDieX, int nDieY)
{
  m_nDieX = nDieX;
  m_nDieY = nDieY;
}

/******************************************************************************!
 * \fn findTestCell
 * \brief Find a test
 ******************************************************************************/
bool DrillData3DInfo::findTestCell(int nTestNumber,
                                   int nPinmap,
                                   QString strTestName  /* = "" */)
{
  CTest* pTestCell = NULL;
  bool   bSucceed  = false;

  m_nTestNumber = nTestNumber;

  if (nPinmap != GEX_UNKNOWNTEST && nPinmap != GEX_INVALIDTEST)
  {
    m_nPinmap = nPinmap;

    // Find the right test
    if (m_pFile->FindTestCell(m_nTestNumber, m_nPinmap, &pTestCell, true,
                              false,
                              strTestName.toLatin1().data()) == 1) {
      m_ptCurrentTestCell  = pTestCell;
      m_ptPreviousTestCell = previousValidTestCell(m_pFile->PrevTest(pTestCell));
      m_ptNextTestCell     =
        nextValidTestCell(m_ptCurrentTestCell->GetNextTest());

      if (m_ptCurrentTestCell) {
        char szTestName[2 * GEX_MAX_STRING];

        gexReport->BuildTestNameString(m_pFile,
                                       m_ptCurrentTestCell,
                                       szTestName);
        m_strCurrentTestLabel = m_ptCurrentTestCell->szTestLabel;
        m_strCurrentTestLabel += " : ";
        m_strCurrentTestLabel += szTestName;
      }

      bSucceed = true;
    }
  } else {
    // Find the right test
    if (m_pFile->FindTestCell(m_nTestNumber, GEX_PTEST, &pTestCell, true,
                              false,
                              strTestName.toLatin1().data()) == 1 ||
        m_pFile->FindTestCell(m_nTestNumber, GEX_MPTEST, &pTestCell, true,
                              false,
                              strTestName.toLatin1().data()) == 1 ||
        m_pFile->FindTestCell(m_nTestNumber, GEX_FTEST, &pTestCell, true,
                              false,
                              strTestName.toLatin1().data()) == 1) {
      m_ptCurrentTestCell  = pTestCell;
      m_ptPreviousTestCell = previousValidTestCell(m_pFile->PrevTest(pTestCell));
      m_ptNextTestCell     = nextValidTestCell(m_ptCurrentTestCell->GetNextTest());

      if (m_ptCurrentTestCell) {
        char szTestName[2 * GEX_MAX_STRING];

        gexReport->BuildTestNameString(m_pFile,
                                       m_ptCurrentTestCell,
                                       szTestName);
        m_strCurrentTestLabel = m_ptCurrentTestCell->szTestLabel;
        m_strCurrentTestLabel += " : ";
        m_strCurrentTestLabel += szTestName;

        m_nPinmap = m_ptCurrentTestCell->lPinmapIndex;
      }

      bSucceed = true;
    }
  }

  return bSucceed;
}

/******************************************************************************!
 * \fn loadDefaultTestCell
 * \brief Initialize data with a default test
 ******************************************************************************/
void DrillData3DInfo::loadDefaultTestCell()
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillData3DInfo loadDefaultTestCell...");
    CTest* pFirstValid = NULL;

  if (m_pFile)
  {
    if (m_pFile->ptTestList)
    {
      pFirstValid = nextValidTestCell(m_pFile->ptTestList);
    }
  }

  if (pFirstValid)
  {
    loadTestCell(pFirstValid->lTestNumber, pFirstValid->lPinmapIndex);
  }
}

/******************************************************************************!
 * \fn loadTestCell
 * \brief Load the current test
 ******************************************************************************/
void DrillData3DInfo::loadTestCell(int nTestNumber, int nPinmap)
{
  bool bSucceed = false;

  if (isValid() &&
      nTestNumber >= GEX_MINTEST &&
      static_cast<unsigned long>(nTestNumber) <= GEX_MAXTEST)
  {
    bSucceed = findTestCell(nTestNumber, nPinmap);
  }

  emit dataLoaded(bSucceed, m_bFirst);

  if (m_bFirst)
  {
    m_bFirst = false;
  }
}

/******************************************************************************!
 * \fn previousValidTestCell
 * \brief Find the previous valid test
 ******************************************************************************/
CTest* DrillData3DInfo::previousValidTestCell(CTest* prevTestCell)
{
  bool bValidPrevTest  = false;
  QString strOptionStorageDevice = (ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString();

  if (isValid())
  {
    // Iterate on next test until new test has valid data...
      while (prevTestCell && bValidPrevTest == false)
      {
          if (m_pFile->FindTestCell(prevTestCell->lTestNumber,
                                    prevTestCell->lPinmapIndex,
                                    &prevTestCell, false, false,
                                    prevTestCell->strTestName.toLatin1().data()) == 1)
          {
              if ((prevTestCell->ldExecs == 0 &&
                   prevTestCell->GetCurrentLimitItem()->ldOutliers == 0) ||
                   prevTestCell->lResultArraySize > 0 ||
                   gexReport->isInteractiveTestFiltered(prevTestCell) == false)
              {
                  prevTestCell = m_pFile->PrevTest(prevTestCell);
              }
              else
                  // Ignore Generic Galaxy Parameters
                  if (prevTestCell->bTestType == '-' &&
                          strOptionStorageDevice == "hide")
                  {
                      prevTestCell = m_pFile->PrevTest(prevTestCell);
                  }
                  else
                  {
                      bValidPrevTest = true;
                  }
          }
          else
          {
              prevTestCell = NULL;
          }
      }
  }

  return prevTestCell;
}

/******************************************************************************!
 * \fn nextValidTestCell
 * \brief Find the next valid test
 ******************************************************************************/
CTest* DrillData3DInfo::nextValidTestCell(CTest* pNextTestCell)
{
  bool bValidNextTest  = false;
  QString strOptionStorageDevice =
    (ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString();

  if (isValid()) {
    // Iterate on next test until new test has valid data...
    while (pNextTestCell && bValidNextTest == false) {
      if (m_pFile->
          FindTestCell(pNextTestCell->lTestNumber,
                       pNextTestCell->lPinmapIndex, &pNextTestCell,
                       false, false,
                       pNextTestCell->strTestName.toLatin1().data()) == 1) {
        if ((pNextTestCell->ldExecs == 0 && pNextTestCell->GetCurrentLimitItem()->ldOutliers == 0) ||
            // pNextTestCell->bTestType == 'F' ||
            pNextTestCell->lResultArraySize > 0 ||
            gexReport->isInteractiveTestFiltered(pNextTestCell) == false) {
          pNextTestCell = pNextTestCell->GetNextTest();
        } else if ((pNextTestCell->bTestType == '-')
                   && (strOptionStorageDevice == "hide")) {
          // Ignore Generic Galaxy Parameters
          pNextTestCell = pNextTestCell->GetNextTest();
        } else {
          bValidNextTest = true;
        }
      } else {
        pNextTestCell = NULL;
      }
    }
  }

  return pNextTestCell;
}

/******************************************************************************!
 * \fn previousTestCell
 * \brief Go to the previous test
 ******************************************************************************/
void DrillData3DInfo::previousTestCell()
{
  if (isValid() && hasPreviousTestCell()) {
    findTestCell(m_ptPreviousTestCell->lTestNumber,
                 m_ptPreviousTestCell->lPinmapIndex,
                 m_ptPreviousTestCell->strTestName);

    emit testNavigate();
  }
}

/******************************************************************************!
 * \fn nextTestCell
 * \brief Go to the next test
 ******************************************************************************/
void DrillData3DInfo::nextTestCell()
{
  if (isValid() && hasNextTestCell()) {
    findTestCell(m_ptNextTestCell->lTestNumber,
                 m_ptNextTestCell->lPinmapIndex,
                 m_ptNextTestCell->strTestName);

    emit testNavigate();
  }


}

/******************************************************************************!
 * \fn applyTestFilter
 * \brief apply test filter to the current test, if it doesn't
 *        match, use the first valid test in the filter
 ******************************************************************************/
void DrillData3DInfo::applyTestFilter()
{
  if (gexReport->isInteractiveTestFiltered(m_ptCurrentTestCell) == false)
  {
    CTest* pFirstValid = nextValidTestCell(m_pFile->ptTestList);

    if (pFirstValid)
    {
      navigateTo(pFirstValid->lTestNumber,
                 pFirstValid->lPinmapIndex,
                 pFirstValid->strTestName);
    }
  }
}

/******************************************************************************!
 * \fn navigateTo
 * \brief Navigate to a specific test
 ******************************************************************************/
void DrillData3DInfo::navigateTo(int nTestNumber,
                                 int nPinmap,
                                 QString strTestName)
{
  if (isValid() &&
      nTestNumber >= GEX_MINTEST &&
      static_cast<unsigned long>(nTestNumber) <= GEX_MAXTEST) {
    findTestCell(nTestNumber, nPinmap, strTestName);

    emit testNavigate();
  }


}

/******************************************************************************!
 * \fn changeFileID
 * \brief Change the current file ID and load test
 ******************************************************************************/
void DrillData3DInfo::changeFileID(int nFileID)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("change File ID to %1").arg( nFileID).toLatin1().constData());
  if (m_pGroup)
  {
    m_nFileID = nFileID;

    if (m_nFileID >= 0 && m_nFileID < m_pGroup->pFilesList.count()) {
      m_pFile =
        (m_pGroup->pFilesList.isEmpty()) ? NULL :
        (m_pGroup->pFilesList.at(m_nFileID));
    } else {
      m_pFile =
        (m_pGroup->pFilesList.isEmpty()) ? NULL :
        (m_pGroup->pFilesList.first());
      m_nFileID = 0;
    }
  }

  // Load the test for with file with current test number and pinmap
  loadTestCell(m_nTestNumber, m_nPinmap);
}

/******************************************************************************!
 * \fn changeGroupID
 * \brief Change the current group ID and load test
 ******************************************************************************/
void DrillData3DInfo::changeGroupID(int nGroupID)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("change GroupID to %1...").arg( nGroupID).toLatin1().constData());
  m_nGroupID  = nGroupID;

  // Group#
  if (m_nGroupID >= 0 && m_nGroupID <= gexReport->getGroupsList().count())
  {
    m_pGroup = gexReport->getGroupsList().at(m_nGroupID);  // Group#
  }
  else
  {
    m_pGroup = gexReport->getGroupsList().size() > 0 ?
      gexReport->getGroupsList().first() : NULL;  // Group#1
    m_nGroupID = 0;
  }

  if (m_pGroup)
  {
    m_nFileID = 0;
    m_pFile = (m_pGroup->pFilesList.isEmpty()) ? NULL :
      (m_pGroup->pFilesList.first());
  }

  // If test number exists in this group, reload it
  if (findTestCell(m_nTestNumber, m_nPinmap))
  {
    loadTestCell(m_nTestNumber, m_nPinmap);
  }
  else
  {
    // Otherwise, load the default test
    loadTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST);
  }

}

/******************************************************************************!
 * \fn makeExportLink
 * \brief Make an url link
 ******************************************************************************/
QString DrillData3DInfo::makeExportLink() const
{
  QString urlLink;

  urlLink = "drill_3d=";

  if (m_nTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN) {
    urlLink += "wafer_sbin";
  } else if (m_nTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN) {
    urlLink += "wafer_hbin";
  } else {
    urlLink += "wafer";
  }

  urlLink += "--";
  urlLink += "g=" + QString::number(m_nGroupID);
  urlLink += "--";
  urlLink += "f=" + QString::number(m_nFileID);

  return urlLink;
}

}  // namespace Gex
