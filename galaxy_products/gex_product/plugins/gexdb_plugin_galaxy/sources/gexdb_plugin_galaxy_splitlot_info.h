#ifndef GEXDB_PLUGIN_GALAXY_SPLITLOT_INFO_H
#define GEXDB_PLUGIN_GALAXY_SPLITLOT_INFO_H

#include <QString>
#include <QMap>
#include <QVariant>

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Galaxy_SplitlotInfo
// Holds info on 1 splitlot
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Galaxy_SplitlotInfo
{
public:
  GexDbPlugin_Galaxy_SplitlotInfo()
  {
    m_lSplitlotID              = -1;
    m_uiSetupTime              = 0;
    m_uiStartTime              = 0;
    m_uiFinishTime             = 0;
    m_uiStationNb              = 0;
    m_uiFlags                  = 0;
    m_uiNbParts                = 0;
    m_uiNbParts_Good           = 0;
    m_uiNbPartsSamples         = 0;
    m_uiNbPartsSamples_Good    = 0;
    m_uiNbPartsSummary         = 0;
    m_uiNbPartsSummary_Good    = 0;
    m_uiRetestIndex            = 0;
    m_uiReworkCode             = 0;
    m_uiNbSites                = 0;
    m_uiHeadNum                = 0;
    m_uiFileHostID             = 0;
    m_cValidSplitlot           = 'N';
    m_uiInsertionTime          = 0;
    m_cProdData                = 'Y';
    m_cModeCod                 = ' ';
    m_cRtstCod                 = ' ';
    m_cProtCod                 = ' ';
    m_uiBurnTim                = 65535;
    m_cCmodCod                 = ' ';
    m_lSyaID                   = -1;
    m_bConsolidatedExtraction  = false;
    m_uiWeekNb                 = 0;
    m_uiMonthNb                = 0;
    m_uiQuarterNb              = 0;
    m_uiYearNb                 = 0;
    m_nWaferNb                 = -1;
  }

  // Fields from Galaxy DB (et_splitlot, wt_splitlot or ft_splitlot table)
  qint64          m_lSplitlotID;             // SPLITLOT_ID
  QString         m_strLotID;                // LOT_ID
  QString         m_strSublotID;             // SUBLOT_ID
  unsigned int    m_uiSetupTime;             // SETUP_T
  unsigned int    m_uiStartTime;             // START_T
  unsigned int    m_uiFinishTime;            // FINISH_T
  unsigned int    m_uiStationNb;             // STAT_NUM
  QString         m_strTesterName;           // TESTER_NAME
  QString         m_strTesterType;           // TESTER_TYPE
  unsigned int    m_uiFlags;                 // SPLITLOT_FLAGS
  unsigned int    m_uiNbParts;               // NB_PARTS
  unsigned int    m_uiNbParts_Good;          // NB_PARTS_GOOD
  unsigned int    m_uiNbPartsSamples;        // NB_PARTS_SAMPLES
  unsigned int    m_uiNbPartsSamples_Good;   // NB_PARTS_SAMPLES_GOOD
  unsigned int    m_uiNbPartsSummary;        // NB_PARTS_SUMMARY
  unsigned int    m_uiNbPartsSummary_Good;   // NB_PARTS_SUMMARY_GOOD
  QString         m_strDataProvider;         // DATA_PROVIDER
  QString         m_strDataType;             // DATA_TYPE
  char            m_cProdData;               // PROD_DATA
  QString         m_strRetestPhase;          // TEST_INSERTION
  unsigned int    m_uiRetestIndex;           // RETEST_INDEX
  QString         m_strRetestedHbins;        // RETEST_HBINS
  unsigned int    m_uiReworkCode;            // REWORK_CODE
  QString         m_strJobName;              // JOB_NAME
  QString         m_strJobRev;               // JOB_REV
  QString         m_strOperator;             // OPER_NAM
  QString         m_strExecType;             // EXEC_TYP
  QString         m_strExecVer;              // EXEC_VER
  QString         m_strTestCode;             // TEST_COD
  QString         m_strFacilID;              // FACIL_ID
  QString         m_strTestTemp;             // TST_TEMP
  char            m_cModeCod;                // MODE_COD
  char            m_cRtstCod;                // RTST_COD
  char            m_cProtCod;                // PROT_COD
  unsigned int    m_uiBurnTim;               // BURN_TIM
  char            m_cCmodCod;                // CMOD_COD
  QString         m_strPartTyp;              // PART_TYP
  QString         m_strUserTxt;              // USER_TXT
  QString         m_strAuxFile;              // AUX_FILE
  QString         m_strPkgTyp;               // PKG_TYP
  QString         m_strFamlyID;              // FAMLY_ID
  QString         m_strDateCod;              // DATE_COD
  QString         m_strFloorID;              // FLOOR_ID
  QString         m_strProcID;               // PROC_ID
  QString         m_strOperFrq;              // OPER_FRQ
  QString         m_strSpecNam;              // SPEC_NAM
  QString         m_strSpecVer;              // SPEC_VER
  QString         m_strFlowID;               // FLOW_ID
  QString         m_strSetupID;              // SETUP_ID
  QString         m_strDsgnRev;              // DSGN_REV
  QString         m_strEngID;                // ENG_ID
  QString         m_strRomCod;               // ROM_COD
  QString         m_strSerlNum;              // SERL_NUM
  QString         m_strSuprNam;              // SUPR_NAM
  unsigned int    m_uiNbSites;               // NB_SITES
  unsigned int    m_uiHeadNum;               // HEAD_NUM
  QString         m_strHandlerType;          // HANDLER_TYP
  QString         m_strHandlerID;            // HANDLER_ID
  QString         m_strCardType;             // CARD_TYP
  QString         m_strCardID;               // CARD_ID
  QString         m_strLoadboardType;        // LOADBOARD_TYP
  QString         m_strLoadboardID;          // LOADBOARD_ID
  QString         m_strDibType;              // DIB_TYP
  QString         m_strDibID;                // DIB_ID
  QString         m_strCableType;            // CABLE_TYP
  QString         m_strCableID;              // CABLE_ID
  QString         m_strContactorType;        // CONTACTOR_TYP
  QString         m_strContactorID;          // CONTACTOR_ID
  QString         m_strLaserType;            // LASER_TYP
  QString         m_strLaserID;              // LASER_ID
  QString         m_strExtraType;            // EXTRA_TYP
  QString         m_strExtraID;              // EXTRA_ID
  unsigned int    m_uiFileHostID;            // FILE_HOST_ID
  QString         m_strFilePath;             // FILE_PATH
  QString         m_strFileName;             // FILE_NAME
  char            m_cValidSplitlot;          // VALID_SPLITLOT
  unsigned int    m_uiInsertionTime;         // INSERTION_TIME
  QString         m_strSubconLotID;          // SUBCON_LOT_ID
  QString         m_strWaferID;              // WAFER_ID
  qint64          m_lSyaID;                  // SYA_ID
  QString         m_strDay;                  // DAY
  unsigned int    m_uiWeekNb;                // WEEK_NB
  unsigned int    m_uiMonthNb;               // MONTH_NB
  unsigned int    m_uiQuarterNb;             // QUARTER_NB
  unsigned int    m_uiYearNb;                // YEAR_NB
  QString         m_strYearAndWeek;          // YEAR_AND_WEEK
  QString         m_strYearAndMonth;         // YEAR_AND_MONTH
  QString         m_strYearAndQuarter;       // YEAR_AND_QUARTER
  QString         m_strIncremental_Update;   // INCREMENTAL_UPDATE
  QString         m_strProductName;          // PRODUCT.PRODUCT_NAME
  int             m_nWaferNb;                // WAFER_NB (-1 if none found)
  QString         m_strEtestSiteConfig;      // ETEST_SITE_CONFIG

  // Filtering
  QString         m_strSiteFilterValue;         // Site filter value
  QVector<int>    m_vHbinsToExclude;            // Hbins to exclude when retrieving data (default empty)
  bool            m_bConsolidatedExtraction;    // True if extracting consolidated data

  // ToDo : move all attributes to this map
  QMap< QString, QVariant > mAttributes;

};

#endif // GEXDB_PLUGIN_GALAXY_SPLITLOT_INFO_H
