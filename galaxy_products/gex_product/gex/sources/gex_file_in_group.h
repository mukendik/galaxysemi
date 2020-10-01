/******************************************************************************!
 * \file gex_file_ingroup.h
 ******************************************************************************/
#ifndef GEX_FILE_IN_GROUP_H
#define GEX_FILE_IN_GROUP_H

#include <QString>
#include "cmir.h"
#include "wafermap.h"
#include "cpart_binning.h"
#include "gexpartfilter.h"
#include "gexdatasetconfig.h"
#include "cstats.h"

#include "stdf.h"


class StdfDataFile;

namespace GQTL_STDF
{
class Stdf_DTR_V4;
}

class CGexGroupOfFiles;
class CReportOptions;
class CMergedResults;
class CPinmap;
class CBinning;
class CTest;
class CGexFilterRangeCoord;
class CPartInfo;
class GP_SiteDescription;
class GP_SiteDescriptionMap;
class GP_DataResult;

typedef QMap<int, int>                   CiZoningTestResult;
typedef QMap<int, QList<CTest*> >        CListTestCellMap;
typedef QMap<int, CTest*>                CFailTestCell;
typedef QMap<unsigned int, unsigned int> CFailTestNumberMap;
typedef QMap<int, int>                   CFailTestPinmapMap;
typedef QMap<int, float>                 CFailTestValue;
typedef QMap<int, float>                 CfZoningTestResult;
typedef QMap<unsigned, unsigned>         CSiteBlockMap;

/******************************************************************************!
 * \class TestSiteLimits
 ******************************************************************************/
class TestSiteLimits
{
public:
  /*!
   * \fn TestSiteLimits
   */
  TestSiteLimits();
  /*!
   * \fn ~TestSiteLimits
   */
  ~TestSiteLimits();
  /*!
   * \fn TestSiteLimits
   */
  TestSiteLimits(const TestSiteLimits& roOther);
  /*!
   * \fn reset
   */
  static void reset();

protected:
  /*!
   * \var m_dLowLimit
   */
  double m_dLowLimit;
  /*!
   * \var m_dHighLimit
   */
  double m_dHighLimit;
  /*!
   * \var m_iCurrentIdx
   */
  int m_iCurrentIdx;
  /*!
   * \var m_lStartT
   */
  time_t m_lStartT;
  /*!
   * \var m_iLimitIndex
   */
  static int m_iLimitIndex;

public:
  double getLowLimit();
  void setLowLimit(double );

  double getHighLimit();
  void setHighLimit(double);

  int getCurrentIdx();
  void setCurrentIdx(int );

  time_t getStartT();
  void setStartT(time_t);

  static int getLimitIndex();
  static void setLimitIndex(int );

};

/******************************************************************************!
 * \class CGexFileInGroup
 ******************************************************************************/
class CGexFileInGroup : public QObject
{
    Q_OBJECT

public:
   /*!
   * \fn CGexFileInGroup
   * \brief FileName is the file to process any format supported (STDF, ATDF,
   *        GDF, etc.). The file will be converted if needed
   */
  CGexFileInGroup(CGexGroupOfFiles* parent,
                  int               iGroupID,
                  QString           FileName,
                  int               iSite,
                  int               iPartBin,
                  const char*       szRangeList,
                  QString           strFileMapTests,
                  QString           strWaferToExtract = "",
                  double            lfTemperature = -1000,
                  QString           strDataName = "",
                  QString           sampleGroup = "");
  /*!
   * \fn ~CGexFileInGroup
   * \brief Destructor
   */
  ~CGexFileInGroup();


  StdfDataFile* mStdfDataFileParent;
  void setParentStdfDataFile(StdfDataFile* stdfDataFileParent) { mStdfDataFileParent = stdfDataFileParent;}

  void Init(CReportOptions* ptReportOptions,
            CMergedResults* ptMergedData,
            FILE*           hAdvFile,
            int iPass);


  /*!
   * \fn FindPinmapCell
   */
  int FindPinmapCell(CPinmap** ptPinmapCellFound, int iPinmapIndex);
  /*!
   * \fn AddBinCell
   * \brief bModeUsed to specify if the fct insert a soft bin or a hard bin
   */
  void AddBinCell(CBinning**  ptBinList,
                  int         iSiteID,
                  int         iDieX,
                  int         iDieY,
                  int         iSoftBinValue,
                  int         iHardBinValue,
                  BYTE        cPassFail,
                  int         ldTotalCount,
                  bool        bCumulate,
                  bool        bReadingDataFiles,
                  const char* szBinName = "",
                  bool        bModeUsed = false);
  /*!
   * \fn setBinningType
   * \brief Sets the 'P' / 'F' status of a binning entry (Soft-bin or Hard-bin)
   */
  void setBinningType(bool bSoftBin, int iBinValue, bool bPassBin);
  /*!
   * \fn BuildPartsToProcess
   */
  QString BuildPartsToProcess();
  /*!
   * \fn BuildSitesToProcess
   */
  QString BuildSitesToProcess();
  /*!
   * \fn BuildWaferMapSize
   */
  char* BuildWaferMapSize(char*);
  /*!
   * \fn BuildDieSize
   */
  char* BuildDieSize(char*);
  /*!
   * \fn FindTestCell
   * \brief Find Test Cell structure in Test List. Return 1 on succcess. -1 on 'new' error: bad_alloc.
   */
  int FindTestCell(unsigned int     lTestNumber,
                   int              lResultIndex,
                   CTest**          t, // result if found
                   bool             bAllowMapping = true,
                   bool             bCreateIfNew = true,
//                   bool             bResetList = false,
                   const QString&   strName = QString(),
                   bool enablePinmapIndex = true);

/*  bool CreateTestCell(unsigned int lTestNumber,
                      int          lPinmapIndex,
                      CTest**      ptTestCellFound,
                      bool         bCreateIfNew,
                      const QString lFormattedTestName,
                      const bool bTestNameMapped);

  bool CreateNewTestList(const unsigned int  lTestNumber,
                         const int           lPinmapIndex,
                         CTest**             ptTestCellFound,
                         const QString       lFormattedTestName,
                         const bool bTestNameMapped);*/

  int CalculOffsetIndexResult(const QString& fileName);
  /*!
   * \fn FindTestCellName
   */
  int FindTestCellName(const QString& lName, CTest** ptTestCellFound);
  /*!
   * \fn PrevTest
   */
  CTest* PrevTest(CTest* ref);
  /*!
   * \fn NextTest
   */
  CTest* NextTest(CTest* ref) const;
  /*!
   * \fn ChangeTestSortOrder
   */
  void ChangeTestSortOrder();
  /*!
   * \fn GetMaxTestnumber
   */
  int GetMaxTestnumber();
  /*!
   * \fn GetProcessBins
   */
  int GetProcessBins() { return iProcessBins;}
  /*!
   * \fn NormalizeScaleFactor
   */
  int NormalizeScaleFactor(int iPower);
  /*!
   * \fn BuildTestNumberString
   */
  void BuildTestNumberString(CTest* ptTestCell);
  /*!
   * \fn FormatTestLimit
   */
  char* FormatTestLimit(CTest* ptTestCell,
                        char*  szBuffer,
                        double lfValue,
                        int    iPower,
                        bool   bUseOutputFormat = true);
  /*!
   * \fn FormatTestLimit
   */
  void FormatTestLimit(CTest* ptTestCell);
  /*!
   * \fn isMatchingWafer
   */
  bool isMatchingWafer(QString strFilter, QString strWaferID);
  /*!
   * \fn SwapWaferMapDies
   */
  bool SwapWaferMapDies(bool bXaxis = true, bool bYaxis = true);
  /*!
   * \fn GetWafermapBinOccurance
   * \brief Returns occurance of a given bin#
   */
  int GetWafermapBinOccurance(int iBin);
  /*!
   * \fn GetWafermapYieldInfo
   */
  void GetWafermapYieldInfo(bool    bSoftBin,
                            int&    iMatch,
                            int&    iUnmatch,
                            QString strBinList = "");
  /*!
   * \fn findRunNumber
   * \brief Find the run number associated with the die coord
   */
  long findRunNumber(int nXDie, int nYDie, CTest* pTestCell,
                     int iStartFrom = 0);

  /*!
   * \fn initDynamicTest
   * \brief Functions to build a Dynamic test (created from 'FindTestCell')
   *        Init CTest structure ready for filling datasamples array
   */
  bool initDynamicTest(CTest* ptTestCell,
                       double lfLowL,
                       int    iLlScale,
                       double lfHighL,
                       int    iHlScale,
                       int    iResScale);
  /*!
   * \fn computeAllStatistics
   * \brief Compute/Update all test statistics
   */
  void computeAllStatistics(CTest* ptTestCell,
                            bool   bPercentile,
                            int    iHistogramType);
  /*!
   * \fn onPrepareSecondPass
   * \brief Called before the second pass
   */
  void onPrepareSecondPass();
  /*!
   * \fn fillUnfilteredPartsList
   */
  void fillUnfilteredPartsList(QList<long>& lstUnfilteredParts);
  /*!
   * \fn applyTestFilter
   */
  void applyTestFilter(CMergedResults* pMergedResults);
  /*!
   * \fn applyTestCreation
   */
  void applyTestCreation(CTest **ptMergedTestList);
  /*!
   * \fn orderRun
   */
  bool orderRun(const QString& lOptionsSublotOrder);
  /*!
   * \fn getPRRCount
   */
  int getPRRCount() const {return m_iPRRCount;}
  /*!
   * \fn getPIRCount
   */
  int getPIRCount() const {return m_iPIRCount;}
  /*!
   * \fn getMRRCount
   */
  int getMRRCount() const;
  /*!
   * \fn grossDieCount
   */
  int grossDieCount() const {return m_nGrossDieCount;}

   /*!
   * \var m_iPIRCount
   */
  int m_iPIRCount;
  /*!
   * \var m_iPRRCount
   */
  int m_iPRRCount;
  /*!
   * \var m_iMRRCount
   */
  int m_iMRRCount;
  /*!
   * \fn setGrossDieCount
   */
  void setGrossDieCount(const int nGrossDieCount){m_nGrossDieCount = nGrossDieCount;}
  /*!
   * \var lGroupID
   * \brief GroupID to which this file belongs to
   */
  int lGroupID;
  /*!
   * \var lFileID
   * \brief FileID (index of file in the group: starts at 0, ...)
   */
  int lFileID;
  /*!
   * \var iProcessSite
   * \brief -1: All sites, otherwise specified site only
   */
  int iProcessSite;
  /*!
   * \var iStdfAtrTesterBrand
   * \brief Holds STDF tester type if information defined in a ATR record,
   *        holds -1 otherwise
   */
  int iStdfAtrTesterBrand;
  /*!
   * \var iProcessStatus
   */
  int iProcessStatus;
  /*!
   * \var lProcessDone
   */
  long lProcessDone;
  /*!
   * \var lRecordIndex
   */
  long lRecordIndex;
  int mOffsetResult;
  /*!
   * \var bStdfProcessed
   */
  BOOL bStdfProcessed;
  /*!
   * \var lStdfFullFileSize
   * \brief STDF file real size
   */
  long lStdfFullFileSize;
  /*!
   * \var ldTotalPartsSampled
   * \brief After pass1, holds number of PRR found
   *        (used to decide if use summary if NO samples are found)
   */
  long ldTotalPartsSampled;
  /*!
   * \var m_lCurrentInternalPartID
   * \brief Holds current internal part ID
   */
  long m_lCurrentInternalPartID;
  /*!
   * \var m_mapPartID
   * \brief Holds internal ID associated with a part ID
   */
  QMap<QString, long> m_mapPartID;
  /*!
   * \var m_strPart_TXT
   * \brief Holds list of Part_TXT found for each run
   */
  QStringList m_strPart_TXT;
  /*!
   * \var pReportOptions
   */
  CReportOptions* pReportOptions;
  /*!
   * \var StdfRecordHeader
   */
  GS::StdLib::StdfRecordReadInfo StdfRecordHeader;
  /*!
   * \var strFileName
   * \brief File path+name to process: can be STDF, ATDF, CSV, etc.
   */
  QString strFileName;
  /*!
   * \var strFileNameSTDF
   * \brief File path+name to process
   */
  QString strFileNameSTDF;
  /*!
   * \var strRangeList
   * \brief Bin/Parts range
   */
  QString strRangeList;
  /*!
   * \var strDatasetName
   * \brief If defined, holds a custom dataset name
   *        E.g: can be used as a marker label on trend chart
   */
  QString strDatasetName;
  /*!
   * \var mSampleGroup
   * \brief If defined this group is a controlgroup
   *         holds the sample group name
   *        Samples: refers to the units under test which
   *                will be tested at multiple temperatures.
   *        Controls: refers to the control units which are
   *                not necessarily under test, they are known
   *                good units with known shifts.
   */
  QString mSampleGroup;
  /*!
   * \var strMapTests
   * \brief File path+name for Mapping Test#
   */
  QString strMapTests;
  /*!
   * \var Only WaferID to extract in STDF file (use in Query mode)
   * \brief strWaferToExtract
   */
  QString strWaferToExtract;
  /*!
   * \var m_lfTemperature
   * \brief Testing temperature for this file (if known, otherwise = -1000)
   */
  double m_lfTemperature;

  /*!
   * \brief mIndexWafermap is used to identify a wafermap. In some file, there are several
   * wafermap described (between WIR/WRR record). For each a pFile is associated
   */
  int    mIndexWafermap;


  const CMir& getMirDatas() const;
  CMir& getMirDatas() ;
  CPcr& getPcrDatas() ;
 // CMir& getMirPCRDatas() ;

private:
  /*!
   * \var MirData
   * \brief Includes all STDF .MIR record data
   */
  CMir MirData;

  CPcr mPCRData;

public:

  /*!
   * \var imapZoningTestValue
   * \brief If doing wafermap zoning, tells current zoning value
   *        for the part tested
   */
  CiZoningTestResult imapZoningTestValue;
  /*!
   * \var pGexRangeList
   * \brief List of Ranges to process (parts/bins, ...)
   */
  GS::QtLib::Range* pGexRangeList;
  /*!
   * \var m_pFilterRangeCoord
   * \brief Range of coordinate valid for a die
   */
  CGexFilterRangeCoord* m_pFilterRangeCoord;
  /*!
   * \var pPartInfoList
   * \brief List of PartInfo : When in Drill mode,
   *        includes info about Each part tested
   */
  QList<CPartInfo*> pPartInfoList;
  /*!
   * \var GetPartsFromPartID
   * \brief Retrieve list of PartInfo from the PartID string
   */
  bool GetPartsFromPartID(QString partID, QList<CPartInfo*>&);
  /*!
   * \var PartProcessed
   * \brief Used in the process to filter some parts from the report generation
   */
  CPartBinning PartProcessed;
  /*!
   * \var lAdvBinningTrendTotalMatch
   * \brief Total Bins matching the Trend filter
   */
  long lAdvBinningTrendTotalMatch;
  /*!
   * \var lAdvBinningTrendTotal
   * \brief Total Bins found (Yield = 100 * lAdvBinningTrendTotalMatch /
   *        lAdvBinningTrendTotal)
   */
  long lAdvBinningTrendTotal;
  /*!
   * \var m_pGlobalEquipmentID
   * \brief To store site description information (from SDR)
   */
  GP_SiteDescription* m_pGlobalEquipmentID;
  /*!
   * \var m_pSiteEquipmentIDMap
   */
  GP_SiteDescriptionMap* m_pSiteEquipmentIDMap;
  /*!
   * \var lAverageTestTime_Good
   * \brief Counts in Milli-seconds on Good Parts
   */
  long lAverageTestTime_Good;
  /*!
   * \var lTestTimeParts_Good
   * \brief Number of parts used to compute test time on Good Parts
   */
  long lTestTimeParts_Good;
  /*!
   * \var lAverageTestTime_All
   * \brief Counts in Milli-seconds on All Parts
   */
  long lAverageTestTime_All;
  /*!
   * \var lTestTimeParts_All
   * \brief Number of parts used to compute test time on All Parts
   */
  long lTestTimeParts_All;

  /*!
   * \fn FormatTestResult
   * \brief Format result to be scaled to correct unit: eg. Mv, or uA, etc...
   * \return return: string with value+units
   */
  char* FormatTestResult(CTest* ptTestCell,
                         double lfValue,
                         int    iPower,
                         bool   bUseOutputFormat = true);
  /*!
   * \fn FormatTestResultNoUnits
   */
  void FormatTestResultNoUnits(double* ptlfValue, int nResScale);
  /*!
   * \fn rescaleValue
   */
  static double rescaleValue(double dValue, int nScaleFactor);
  /*!
   * \fn addPartFilterToDatasetConfig
   * \brief add filters to the GexDatasetConfig
   */
  void addPartFilterToDatasetConfig(long                       lTestNumber,
                                    double                     dValue,
                                    CGexPartFilter::filterType eFilterType)
  {
    m_datasetConfig.addPartFilter(lTestNumber, dValue, eFilterType);
  }
  /*!
   * \fn addPartFilterToDatasetConfig
   */
  void addPartFilterToDatasetConfig(long                       lTestNumber,
                                    double                     dLowValue,
                                    double                     dHighValue,
                                    CGexPartFilter::filterType eFilterType)
  {
    m_datasetConfig.addPartFilter(lTestNumber,
                                  dLowValue,
                                  dHighValue,
                                  eFilterType);
  }
  /*!
   * \fn writeDatasetConfig
   */
  void writeDatasetConfig(const QString& strDatasetConfig)
  {
    m_datasetConfig.write(strDatasetConfig);
  }
  /*!
   * \fn operator<
   */
  bool operator<(const CGexFileInGroup& other) const;
  /*!
   * \fn lessThan
   */
  static bool lessThan(const CGexFileInGroup* pItem1,
                       const CGexFileInGroup* pItem2)
  {return (*pItem1) < (*pItem2);}
  /*!
   * \var m_cStats
   * \brief Statistics engine
   */
  CGexStats m_cStats;
  /*!
   * \fn UpdateOptions
   * \brief Update options cache
   */
  bool UpdateOptions(CReportOptions* ro);
  void updateOutputFormat(const QString& aOuputFormat);

  //! \brief Property storing any JS DTRs
  static const QString sConcatenatedJavaScriptDTRs;

  /*!
   * \fn GetMaxMultiLimitItems
   * \brief return max multi limit items for one test
   */
  int GetMaxMultiLimitItems() const { return mMaxMultiLimitItems;}

  /*!
   * \var ptTestList
   * \brief Pointing to list of Tests
   */
  CTest* ptTestList;
  /*!
   * \var ptPrevCell
   * \brief After call to 'FindTestCell', points to test preceeding test#
   *        searched
   */
  CTest* ptPrevCell;

  /*!
   * \var m_ptCellFound
   * \brief Internal vriable, used in 'FindTestCell'
   */
  CTest* m_ptCellFound;

public:
  /*!
   * \var mMaxMultiLimitItems
   * \brief stores max multi limit items for one test
   */
  int mMaxMultiLimitItems;
  /*!
   * \var m_OutputFormat
   * \brief GEX_OPTION_OUTPUT_HTML, GEX_OPTION_OUTPUT_CSV,... in gex_constants.h
   */
  int m_OutputFormat;
  /*!
   * \var mHistoBarsCount
   * \brief Count of bars in the histogram
   */
  int mHistoBarsCount;

  /*!
   * \enum m_eDatalogFormat
   */
  enum DatalogFormat { ONE_ROW, TWO_ROWS } m_eDatalogFormat;
  /*!
   * \enum m_eRetestPolicy
   */
  enum RetestPolicy { HIGHEST_BIN, LAST_BIN } m_eRetestPolicy;

  /*!
   * \enum m_eOutlierRemoveMode
   */
  enum OutlierRemoveMode { NONE, N_SIGMA, EXCLUDE_N_SIGMA, N_POURCENT, N_IQR } m_eOutlierRemoveMode;
  /*!
   * \var m_dOutlierRemovalValue
   */
  double m_dOutlierRemovalValue;
  /*!
   * \enum m_eMPRMergeMode
   */
  enum MultiParametricMergeMode { MERGE, NO_MERGE } m_eMPRMergeMode;
  /*!
   * \enum mTestMergeRule
   */
  int       mTestMergeRule;
  /*!
   * \enum m_eIgnoreFunctionalTests
   */
  enum IgnoreFunctionalTests { FTR_DISABLED, FTR_ENABLED } m_eIgnoreFunctionalTests;

  /*!
   * \enum m_eScalingMode
   */
  enum ScalingMode { SCALING_NONE, SCALING_SMART, SCALING_TO_LIMITS, SCALING_NORMALIZED } m_eScalingMode;
  /*!
   * \enum AdvDatalogField
   */
  enum AdvDatalogField { ADF_NO_FIELD    = 0,
                         ADF_COMMENTS  = 1,
                         ADF_TEST_NUMBER = 2,
                         ADF_TEST_NAME = 4,
                         ADF_LIMITS      = 8,
                         ADF_DIE_XY    = 16 };
  Q_DECLARE_FLAGS(AdvDatalogFields, AdvDatalogField)
  /*!
   * \enum m_eDatalogTableOptions
   */
  AdvDatalogFields m_eDatalogTableOptions;
  /*!
   * \var m_bFullWafermap
   */
  bool m_bFullWafermap;
  /*!
   * \var m_bUsePassFailFlag
   */
  bool m_bUsePassFailFlag;
  /*!
   * \var m_ParentGroup
   * \brief Handle to parent group
   */
  CGexGroupOfFiles* m_ParentGroup;
  /*!
   * \fn PreparePass
   */
  void PreparePass();
  /*!
   * \fn EndPass
   */
  void EndPass(CMergedResults* ptMergedData);
  /*!
   * \fn ReadStringToField
   */
  int ReadStringToField(char* szField);
  /*!
   * \fn SaveLotSamplesOffsets
   */
  void SaveLotSamplesOffsets();
  /*!
   * \fn RemoveComas
   * \brief Avoid conflict on .CSV ',' delimiter
   */
  void RemoveComas(char* szString);
  /*!
   * \fn FindExt
   */
  char* FindExt(char* szString);
  /*!
   * \fn FindPath
   */
  char* FindPath(char* szString);
  /*!
   * \fn FormatTestName
   */
  char* FormatTestName(char* szString);
  /*!
   * \fn FormatTestName
   */
  void FormatTestName(QString& strString);
  /*!
   * \fn ReadATR
   * \brief Extract ATR data
   */
  void ReadATR();
  /*!
   * \fn ReadMIR
   * \brief Extract MIR data
   */
  void ReadMIR();
  /*!
   * \fn ReadMRR
   * \brief Extract MRR data
   */
  void ReadMRR();
  /*!
   * \fn ReadSDR
   * \brief Extract SDR data
   */
  void ReadSDR();
  /*!
   * \fn ReadWIR
   * \brief Extract WIR data
   */
  void ReadWIR();
  /*!
   * \fn ReadWRR
   * \brief Extract WRR data
   */
  void ReadWRR();
  /*!
   * \fn ReadWCR
   * \brief Extract WCR data
   */
  void ReadWCR();
  /*!
   * \fn ReadPCR
   * \brief Extract PCR data
   */
  void ReadPCR();
  /*!
   * \fn ReadGDR
   * \brief Extract GD data
   */
  void ReadGDR();
  /*!
   * \fn ReadDTR
   * \brief Extract DTR data
   */
  bool ReadDTR();

  //////////////////////////////////////////////////////////////////////////////
  /// Process some reticle information in a stdf V4 DTR record
  //////////////////////////////////////////////////////////////////////////////
  bool ProcessReticleInformationsIn( const GQTL_STDF::Stdf_DTR_V4 &dtr );

  /*!
   * \fn ReadPDR
   * \brief Extract Parametric Test Definition (STDF V3 only)
   */
  void ReadPDR();
  /*!
   * \fn ReadFDR
   * \brief Extract Functional Test Definition (STDF V3 only)
   */
  void ReadFDR();
  /*!
   * \fn ReadFTR
   * \brief Extract FTR data
   */
  bool ReadFTR();
  /*!
   * \fn UpdateCustomParameter
     \brief Create a Parameter entry for special STDF fields (SBIN, HBIN, TESTTIME,etc...).
     \return Returns true on success otherwise false
   */
  bool UpdateCustomParameter(bool bIgnorePart,
                             QString    strParameterName,
                             int        iCustomTestNumber,
                             double     lResult,
                             CPartInfo* pPartInfo);
  /*!
   * \fn WriteDatalogCheckPageBreak
   */
  void WriteDatalogCheckPageBreak(int iSite=-1, int iHead=-1);
  /*!
   * \fn WriteDatalogPartHeader
   */
  void WriteDatalogPartHeader(int iSiteID = -1, int iHead =-1);
  /*!
   * \fn PartIsOutlier
   */
  bool PartIsOutlier(CTest* ptTestCell, float fValue);
  /*!
   * \fn CountTrackFailingTestInfo
   */
  void CountTrackFailingTestInfo(int    iSiteID,
                                 CTest* ptTestCell,
                                 int    iPinmapIndex,
                                 float  fValue);
  /*!
   * \fn CountTestAsFailure
   */
  bool CountTestAsFailure(int    iSiteID,
                          CTest* ptTestCell,
                          int    iPinmapInde,
                          float  fValue,
                          bool   bTestIsFail);
  /*!
   * \fn IsTestFail
   */
  bool IsTestFail(CTest*                      ptTestCell,
                  float                       fValue,
                  CTestResult::PassFailStatus ePassFailStatus,
                  bool&                       bOutsideLimit);
  /*!
   * \fn UpdateMinMaxValues
   */
  void UpdateMinMaxValues(CTest* ptTestCell, double fValue);
  /*!
   * \fn AdvCollectDatalog
   */
  void AdvCollectDatalog(CTest* ptTestCell,
                         float  fValue,
                         bool   bFailResult,
                         bool   bAlarmResult,
                         bool   bIsOutlier,
                         int    iSiteID
                         , int iHead, int iSite);
  /*!
   * \fn AdvCollectTestResult
   * \brief Alloc & save test result values in a TestResult class for
   *        charting its trend,... later on... called on PASS2
   *        Return true when success otherwise false
   */
  bool AdvCollectTestResult(CTest*                      ptTestCell,
                               double                      lfValue,
                               CTestResult::PassFailStatus ePassFailStatus,
                               int                         iSiteID,
                               bool                        bForceCollection =
                                 false);
  /*!
   * \fn AdvCollectBinningTrend
   */
  void AdvCollectBinningTrend(CBinning** ptBinList,
                              int        iBinValue,
                              int        ldTotalCount);
  /*!
   * \fn ReadPTR
   * \brief Extract Test results
   */
  bool ReadPTR();
  /*!
   * \fn ReadPMR
   * \brief Pin Map Record
   */
  void ReadPMR();
  /*!
   * \fn ReadPGR
   * \brief Pin Group Record
   */
  void ReadPGR();
  /*!
   * \fn ReadPLR
   * \brief Pin List Record
   */
  void ReadPLR();
  /*!
   * \fn ReadStaticDataMPR
   */
  int ReadStaticDataMPR(CTest**       ptTestCell,
                        unsigned long lTestNumber,
                        long          iPinmapMergeIndex,
                        BYTE          bSite,
                        long          nJcount,
                        long          nKcount,
                        int*          piCustomScaleFactor,
                        bool          bStrictLL,
                        bool          bStrictHL);
  /*!
   * \fn BuildMultiResultParamTestName
   */
  void BuildMultiResultParamTestName(CTest* ptTestCell, CTest* ptMPTestCell);
  /*!
   * \fn ReadMPR
   * \brief Extract Test results (Multiple Parametric)
   */
  bool ReadMPR();
  /*!
   * \fn ReadHBR
   * \brief Extract HARD Bin results
   */
  void ReadHBR();
  /*!
   * \fn ReadSBR
   * \brief Extract SOFT Bin results
   */
  void ReadSBR();
  /*!
   * \fn ReadBinningRecord
   */
  void ReadBinningRecord(CBinning** ptBinList, int*, bool*);
  /*!
   * \fn WriteDatalogBinResult
   * \brief Writes Datalog BIN result+ Part ID+Die location (HTML mode only)
   */
  void WriteDatalogBinResult(int iBin, CPartInfo*);
  /*!
   * \fn ReadPIR
   * \brief Part header bloc
   */
  void ReadPIR();
  /*!
   * \fn ReadPRR
   * \brief Extract WaferMap results
   */
  void ReadPRR();
  /*!
   * \fn UpdateWaferMap
   * \brief Update Wafer Map
   */
  void UpdateWaferMap(int iBin, CPartInfo* pPartInfo, long lInternalPartID);
  /*!
   * \fn UpdateBinResult
   */
  bool UpdateBinResult(CPartInfo* pPartInfo);
  /*!
   * \var m_First_InstanceDies
   * \brief Keeps list of first test instance Dies (coordinates)
   */
  QMap<QString, long> m_First_InstanceDies;
  /*!
   * \var m_Last_InstanceDies
   * \brief Keeps list of first test instance Dies (coordinates)
   */
  QMap<QString, long> m_Last_InstanceDies;
  /*!
   * \var m_First_Instance
   * \brief Keeps list of part# (as seen when entering PIR) for
   *        first-instance dies
   */
  QList<long> m_First_Instance;
  /*!
   * \var m_Last_Instance
   * \brief Keeps list of part# (as seen when entering PIR) for
   *        last-instance dies
   */
  QList<long> m_Last_Instance;
  /*!
   * \var szExaminatorPath
   * \brief Location of the Examinator application
   */
  char szExaminatorPath[2048];
  /*!
   * \var lPass
   * \brief STDF File is analysed in 2 passes
   */
  int lPass;
  /*!
   * \var StdfFile
   * \brief Handle to STDF File to process
   */
  GS::StdLib::Stdf StdfFile;
  /*!
   * \var bData
   * \brief Temporary buffer when reading STDF object
   */
  BYTE bData;
  /*!
   * \var wData
   * \brief Same as above
   */
  int wData;
  /*!
   * \var lData
   * \brief Same as above
   */
  long lData;
  /*!
   * \var fData
   * \brief Same as above
   */
  float fData;
  /*!
   * \var lStdfFileSize
   * \brief STDF file size / 1024
   */
  long lStdfFileSize;
  /*!
   * \var iSoftBinRecords
   * \brief Keeps track of # of SBR found (matching site# filter)
   */
  int iSoftBinRecords;
  /*!
   * \var iHardBinRecords
   * \brief Keeps track of # of HBR found (matching site# filter)
   */
  int iHardBinRecords;
  /*!
   * \var bMergeSoftBin
   * \brief 'true' if need to merge all SBR sites in Pass2
   */
  bool bMergeSoftBin;
  /*!
   * \var bMergeHardBin
   * \brief 'true' if need to merge all SBR sites in Pass2
   */
  bool bMergeHardBin;
  /*!
   * \var iPirNestedLevel
   * \brief Incremented when entering a PIR, decremented on a PRR
   */
  int iPirNestedLevel;
  /*!
   * \var m_map_ptFailedTestGoodResult
   * \brief Use to track all test failed which have a valid value
   */
  CListTestCellMap m_map_ptFailedTestGoodResult;
  /*!
   * \var m_map_ptPassedTestBadResult
   * \brief Use to track all test passed which have a value outside the limits
   */
  CListTestCellMap m_map_ptPassedTestBadResult;
  /*!
   * \var m_map_ptFailTestCell
   * \brief Use to point to last test structure that failed in execution flow
   */
  CFailTestCell m_map_ptFailTestCell;
  /*!
   * \var m_mapFailTestNumber
   * \brief Used to track last failing test# in program flow
   */
  CFailTestNumberMap m_mapFailTestNumber;
  /*!
   * \var m_mapFailTestPinmap
   * \brief Used to track last failing test pinmap# in program flow
   */
  CFailTestPinmapMap m_mapFailTestPinmap;
  /*!
   * \var m_mapfFailingValue
   * \brief Used to track last failing value
   */
  CFailTestValue m_mapfFailingValue;
  /*!
   * \var fmapZoningTestValue
   * \brief Used to hold Zonal value for DRILL module
   */
  CfZoningTestResult fmapZoningTestValue;
  /*!
   * \var bIgnoreThisSubLot
   * \brief Set to true if a filter on a WaferID is set,
   *        and we read a section to ignore (not matching WaferID)
   */
  bool bIgnoreThisSubLot;

  typedef QMultiMap<int, CTest*> T_Container;
  typedef T_Container::iterator T_Iter;

  /*!
   * \var mMapTestsByNumber
   * \brief Contains the list of tests in the current file sorted by test number
   */
 // T_Container mMapTestsByNumber;

  T_Container& GetCTests();
  /*!
   * \var m_lTestFlowID
   * \brief Variable to keep track of testing flow order of tests
   */
  long m_lTestFlowID;
  /*!
   * \var bFirstWafer
   * \brief Variables to check if same wafer than previous one:
   *        used in pass 2 to verify if need to send eventLot to plug-in
   *        true for first wafer in file after MIR
   */
  bool bFirstWafer;
  /*!
   * \var strPreviousWaferID
   * \brief ID of previous wafer
   */
  QString strPreviousWaferID;
  /*!
   * \var uiAutoWaferID
   * \brief Used to assign auto-increment wafer-IDs if WaferID
   *        is missing in STDF file
   */
  unsigned int uiAutoWaferID;
  /*!
   * \var lMappingSiteHead
   * \brief Flag set when entering in a site block: PIR(head,site),
   *        and cleared when exiting: PRR(head,site)
   */
  CSiteBlockMap lMappingSiteHead;
  /*!
   * \var m_bFailingSiteDetails
   * \brief Used to keep track of failing site details: test, what-if test, both
   */
  std::map< unsigned short, unsigned char >m_bFailingSiteDetails;
  /*!
   * \var ptPinmapList
   * \brief Pointing to list of Pinmap definitions
   */
  //CPinmap* ptPinmapList;
  /*!
   * \var bBinningSummary
   * \brief false after end of pass1 if need to
   *        build bin-summary from samples data
   */
  BOOL bBinningSummary;
  /*!
   * \var szBuffer
   * \brief Useful for FormatTestResult()
   */
  static char szBuffer[50];
  /*!
   * \var hAdvancedReport
   * \brief Handle for writing Advanced Report (Datalog,...) into .HTML ONLY
   */
  FILE* hAdvancedReport;
  /*!
   * \var lTestsInDatalog
   * \brief Keeps track of tests in part dataloged
   */
  int lTestsInDatalog;
  /*!
   * \var iProcessBins
   * \brief Type of parts to process
   */
  int iProcessBins;
  /*!
   * \var m_nGrossDieCount
   * \brief Keep the gross die count
   */
  int m_nGrossDieCount;
  /*!
   * \var m_lSamplesInSublot
   * \brief Keep track of the number of samples in this sub-lot
   */
  long m_lSamplesInSublot;
  /*!
   * \var m_datasetConfig
   * \brief File setup
   */
  CGexDatasetConfig m_datasetConfig;
  /*!
   * \var m_oSBinHBinMap
   * \brief iMap that store the association between SBIN/HBIN
   */
  QMap<int, int> m_oSBinHBinMap;
  /*!
   * \var m_iInvalidPartFoundCount
   */
  QMap<int, int> m_iInvalidPartFoundCount;
  /*!
   * \enum TestSortOrder
   */
  enum TestSortOrder {
    TEST_SORT_ORDER_BY_NUM,
    TEST_SORT_ORDER_BY_FLOW
  };
  /*!
   * \var mSortOrder
   */
  TestSortOrder mTestSortOrder;

public:

  /*!
   * \brief The Limits enum
   */
  enum Limits { SPEC_LIMIT_IF_ANY, STANDART_LIMIT_ONLY, SPEC_LIMIT_ONLY } mUsedLimits;

  /*!
   * \enum m_eStdfCompliancy
   * \brief For ReportOptions.GetOption("dataprocessing", "stdf_compliancy")
   */
  enum StdfCompliancy { STRINGENT, FLEXIBLE} m_eStdfCompliancy;

  /*!
   * \enum m_eFailCountMode
   */
  enum FailCountMode { FAILCOUNT_ALL, FAILCOUNT_FIRST } m_eFailCountMode;

  /*!
   * \enum m_eBinComputation
   */
  enum BinComputation { WAFER_MAP, SUMMARY, SAMPLES } m_eBinComputation;
  /*!
   * \enum m_eStatsComputation
   */
  enum StatsComputation {
      FROM_SAMPLES_ONLY,
      FROM_SUMMARY_ONLY,
      FROM_SAMPLES_THEN_SUMMARY,
      FROM_SUMMARY_THEN_SAMPLES }
  m_eStatsComputation;
  /*!
   * \enum mPartIdentifiation
   */
  enum PartIdentification { XY, PARTID, AUTO } mPartIdentifiation;

  /*!
   * \fn clearSBinHBinMap
   */
  void clearSBinHBinMap() {
    m_oSBinHBinMap.clear();
  }
  /*!
   * \fn getInvalidPartFoundCount
   */
  QMap<int, int> getInvalidPartFoundCount() const
  {
    return m_iInvalidPartFoundCount;
  }

private:
  Q_DISABLE_COPY(CGexFileInGroup)
  /*!
   * \fn formatSequencerAndPinTestName
   */
  void formatSequencerAndPinTestName(QString& strTestName);
  /*!
   * \var m_bRemoveSeqName
   */
  bool m_bRemoveSeqName;
  /*!
   * \var m_bRemovePinName
   */
  bool m_bRemovePinName;



protected:
  /*!
   * \var WaferMapData
   * \brief Wafermap data resulting from STDF file analysis
   */
  CWaferMap WaferMapData;

public:
  /*!
   * \fn getWaferMapData
   */
  CWaferMap& getWaferMapData();
  const CWaferMap& getWaferMapData() const;
  /*!
   * \var m_iWIRCount
   */
  int m_iWIRCount;
  /*!
   * \var m_iWRRCount
   */
  int m_iWRRCount;
  /*!
   * \fn getPIRCount
   */
public:
  int getWIRCount() const ;
  /*!
   * \fn getWRRCount
   */
  int getWRRCount() const ;


  CTest*                getTestList          () { return ptTestList;}
  QMap<int, QString>&   getTestCorrespondance() { return mTestCorrespondance; }
  CSiteBlockMap&        getMappingSiteHead   () { return lMappingSiteHead;}
  StdfCompliancy        getStdCompliancy     () { return m_eStdfCompliancy;}
  CPartBinning&         getPartProcessed     () { return PartProcessed;}
  FailCountMode         getFailCountMode     () { return m_eFailCountMode;}
  CFailTestCell&        getMapFailTestCell   () { return m_map_ptFailTestCell;}
  bool                  getUsePassFailFlag   () { return m_bUsePassFailFlag;}
  int                   getTestMergeRule     () { return mTestMergeRule;}
  BinComputation        getBinComputation    () { return m_eBinComputation;}

  CGexGroupOfFiles*     getParentGroup       ()        { return m_ParentGroup;}
  CListTestCellMap&     getMapFailedTestGoodResult() { return m_map_ptFailedTestGoodResult;}
  CListTestCellMap&     getMapPassedTestBadResult () { return m_map_ptPassedTestBadResult;}
  bool                  getFullWafermap           () { return m_bFullWafermap;}

  CGexDatasetConfig&   getDataSetConfig() { return m_datasetConfig;}
  Limits                getUsedLimits()             { return mUsedLimits;}

  QList<CPartInfo*>&    getPartInfoList()            { return pPartInfoList;}

  std::map< unsigned short, unsigned char >& getMapFailingSiteDetails() { return m_bFailingSiteDetails;}
  QMap<int, int>&       getSBinHBinMap() { return m_oSBinHBinMap;}
  StatsComputation      getStatsComputation() { return m_eStatsComputation;}

   QMap<QString, long>& getMapPartId() { return m_mapPartID;}

  GS::StdLib::StdfRecordReadInfo& getStdfRecordHeader() { return StdfRecordHeader;}

  //CMir&   getMireData() { return MirData;}

   PartIdentification getPartIdentification() { return mPartIdentifiation;}

  QMap<QString, long>&  getFirstInstanceDies () { return m_First_InstanceDies;}
  QMap<QString, long>&  getLastInstanceDies  () { return m_Last_InstanceDies;}
  QList<long>&          getFirstInstance     () { return m_First_Instance;}
  QList<long>&          getLastInstance      () { return m_Last_Instance;}

  void initPtrFromParentGroup();
  void initPtrParentGroup();
private:
  /*!
   * \fn dumpTempDataLog
   */
  void dumpTempDatalog(int iSite, int iHead, FILE *poHandle);

  /*!
   * \brief mTestCorrespondance : Correspondance between test number and test name
   */
  QMap<int, QString> mTestCorrespondance;
};

#endif  // GEX_FILE_IN_GROUP_H
