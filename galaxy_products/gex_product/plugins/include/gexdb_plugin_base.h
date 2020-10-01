// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_BASE_HEADER_
#define _GEXDB_PLUGIN_BASE_HEADER_

// Standard includes
#ifdef _WIN32
#include <windows.h>
#endif
#include <math.h>

// Qt includes
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QSqlDatabase>
#include <QLibrary>
#include <QRegExp>
#include <QSqlQuery>
#include <QTextStream>
#include <QMainWindow>
#include <QHash>
#include <QMap>
#include <QList>
#include <QThread>
#include <QDomElement>
#include <QObject>

// Galaxy modules includes
#include <gstdl_errormgr.h>
//#include <gqtl_sysutils.h>
#include <gqtl_skin.h>
#include <gex_scriptengine.h>
#include "statistical_monitoring_monitored_item_unique_key_rule.h"

class QProgressDialog;

// Fix Me : use QTSRCDIR inc path ?
// On windows, the compiler working dir is qt/qmake
#if defined(__WIN32__) || defined(__CYGWIN32__)
#include <qtbase/src/3rdparty/sqlite/sqlite3.h>
#else
#include <qtbase/src/3rdparty/sqlite/sqlite3.h>
#endif

// Local includes
#include "gexdb_plugin_datafile.h"
#include <QList>

// mainly used by SYA/SBL
// See GexDbPlugin_Base::GetRuleNameFromRuleType( OutlierRule );
enum OutlierRule
{
    eNone=-1,           // None
    eMeanNSigma,        // Mean +/- N*Sigma
    eMedianNRobustSigma,// Median +/- N*RobustSigma
    eMedianNIQR,        // Median +/- N*IQR : IQR=RobustSigma
    ePercentil_0N100,   // Percentile(N) Percentile(100-N)
    eQ1Q3NIQR,          // LL=Q1-N*IQR HL=Q3+N*IQR
    eDefault,           // the one in parent object
    eManual             // a custom dealed by user
};

#define GET_OULIER_RULE_NAME(x, s)  char* OutlierRuleNames[]={(char*)"None", (char*)"Mean +/- N*Sigma", (char*)"Median +/- N*RobustSigma",  \
    (char*)"Median +/- N*IQR", (char*)"Percentile(N) Percentile(100-N)", (char*)"LL=Q1-N*IQR HL=Q3+N*IQR", (char*)"Default", (char*)"Manual" }; \
    if (x>=0 && x<=7) s=OutlierRuleNames[x];

// Used for SPM for now, but could be used for all modules coping with outlier algorithms
// Coding values for outlier removal algorithms
const QString C_OUTLIERRULE_MEAN_N_SIGMA            = "mean_n_sigma";
const QString C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA    = "median_n_robustsigma";
const QString C_OUTLIERRULE_MEDIAN_N_IQR            = "median_n_iqr";
const QString C_OUTLIERRULE_PERCENTILE_N            = "percentile_n";
const QString C_OUTLIERRULE_Q1Q3_N_IQR              = "q1_q3_n_iqr";
const QString C_OUTLIERRULE_DEFAULT                 = "defaut";
const QString C_OUTLIERRULE_MANUAL                  = "manual";
// Display values for outlier removal algorithms
const QString C_OUTLIERRULE_MEAN_N_SIGMA_D          = "Mean +- N*Sigma";
const QString C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA_D  = "Median +- N*RobustSigma";
const QString C_OUTLIERRULE_MEDIAN_N_IQR_D          = "Median +- N*IQR";
const QString C_OUTLIERRULE_PERCENTILE_N_D          = "Percentile(N) - Percentile(100-N)";
const QString C_OUTLIERRULE_Q1Q3_N_IQR_D            = "Q1/Q3 +- N*IQR";
const QString C_OUTLIERRULE_DEFAULT_D               = "Default";
const QString C_OUTLIERRULE_MANUAL_D                = "Manual";
// Coding values for statistics
const QString C_STATS_MEAN            = "mean";
const QString C_STATS_P0              = "p0";
const QString C_STATS_P25             = "p25";
const QString C_STATS_P50             = "p50";
const QString C_STATS_P75             = "p75";
const QString C_STATS_P100            = "p100";
const QString C_STATS_RANGE           = "range";
const QString C_STATS_CP              = "cp";
const QString C_STATS_CPK             = "cpk";
const QString C_STATS_CR              = "cr";
const QString C_STATS_SIGMA           = "sigma";
const QString C_STATS_YIELD           = "yield";
const QString C_STATS_FAIL_COUNT      = "fail_count";
const QString C_STATS_EXEC_COUNT      = "exec_count";
const QString C_STATS_RATIO           = "ratio";
// Display values for statistics
const QString C_STATS_MEAN_D          = "Mean";
const QString C_STATS_P0_D            = "Min value";
const QString C_STATS_P25_D           = "Q1";
const QString C_STATS_P50_D           = "Median";
const QString C_STATS_P75_D           = "Q3";
const QString C_STATS_P100_D          = "Max value";
const QString C_STATS_RANGE_D         = "Range";
const QString C_STATS_CP_D            = "Cp";
const QString C_STATS_CPK_D           = "Cpk";
const QString C_STATS_CR_D            = "Cr";
const QString C_STATS_SIGMA_D         = "Sigma";
const QString C_STATS_YIELD_D         = "Yield";
const QString C_STATS_FAIL_COUNT_D    = "Fail count";
const QString C_STATS_EXEC_COUNT_D    = "Exec count";
const QString C_STATS_RATIO_D         = "Ratio";

// Used by TranslateUnixTimeStampToSqlDateTime
enum SqlUnixDateFormat
{
    eEmpty,
    eYear,
    eQuarter,
    eWeek,
    eWYear,
    eMonth,
    eDate
};

#include "gexdb_plugin_sya.h"
#include "gexdb_plugin_er_dataset.h"
#include "query_engine.h"

// Forward declarations
class QFile;
class QSqlDatabase;
class QWidget;
class QLibrary;
class QMainWindow;
class GexDbPlugin_XYChart_Data;
struct GsData;

namespace GS
{
namespace QtLib
{
class DatakeysEngine;
}
namespace DbPluginBase
{

class DbArchitecture;
class QueryEngine;
class AbstractQueryProgress;
}
}

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within a plugin DLL are compiled with the GEXDB_PLUGIN_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// GEXDB_PLUGIN_API functions as being imported from a DLL, wheras the plugin DLL sees symbols
// defined with this macro as being exported.
#if defined(WIN32)
#ifdef GEXDB_PLUGIN_EXPORTS
#define GEXDB_PLUGIN_API __declspec(dllexport)
#else
#define GEXDB_PLUGIN_API __declspec(dllimport)
#endif
#else
#define GEXDB_PLUGIN_API
#endif
///////////////////////////////////////////////////////////
// Defines...
///////////////////////////////////////////////////////////
// Name of factory function (to get ptr on plugin object)
#define GEXDB_PLUGIN_GETOBJECT_NAME                      "gexdb_plugin_getobject"
#define GEXDB_PLUGIN_RELEASEOBJECT_NAME "gexdb_plugin_releaseobject"

#define GEXDB_INVALID_SCALING_FACTOR                      9999

// Insertion validation checker options
// List of all checkers to do

#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALREADYINSERTED           0x000001
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRCOUNT            0x000002
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRMULTIHEAD        0x000004
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRCOUNT         0x000008
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMRRCOUNT            0x000010
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDSDRCOUNT            0x000020
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID                0x000040
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDPARTLOCATION         0x000080
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION     0x000100
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION      0x000200
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTRESULTOUTSIDE         0x000400
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTWAFERNO          0x000800
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME         0x001000
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMISMATCHCOUNT       0x002000

// WARNING BEFORE ERROR
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMIRCOUNT            0x010000
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRWAFERID       0x020000
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDPCRCOUNT            0x040000
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDTSRMISMATCH         0x080000

// FIRST FOR YIELDMAN
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FAILON_YIELDMAN           0x00ffff
#define GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALL                       0xffffff

// Database fields used by GEXDB, and mapped to External database fields
#define GEXDB_PLUGIN_DBFIELD_PRODUCTIONSTAGE                     "Production Stage"
#define GEXDB_PLUGIN_DBFIELD_DATATYPE                            "Data Type"
#define GEXDB_PLUGIN_DBFIELD_TESTNAME                            "TestName"
#define GEXDB_PLUGIN_DBFIELD_TESTNUM                             "TestNumber"
#define GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR                        "MPR TestName"
#define GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR                         "MPR TestNumber"
#define GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR                        "FTR TestName"
#define GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR                         "FTR TestNumber"
#define GEXDB_PLUGIN_DBFIELD_STARTDATETIME                       "Batch Start date/time"
#define GEXDB_PLUGIN_DBFIELD_WYR_SITE                            "WYR_Site"
#define GEXDB_PLUGIN_DBFIELD_WYR_YEAR                            "WYR_Year"
#define GEXDB_PLUGIN_DBFIELD_WYR_WEEK                            "WYR_Week"
#define GEXDB_PLUGIN_DBFIELD_HBIN                                "Hbin#"
#define GEXDB_PLUGIN_DBFIELD_SBIN                                "Sbin#"
#define GEXDB_PLUGIN_DBFIELD_HBIN_NAME                           "Hbin Name"
#define GEXDB_PLUGIN_DBFIELD_SBIN_NAME                           "Sbin Name"
#define GEXDB_PLUGIN_DBFIELD_HBIN_PF                             "Hbin P/F status"
#define GEXDB_PLUGIN_DBFIELD_SBIN_PF                             "Sbin P/F status"
#define GEXDB_PLUGIN_DBFIELD_USER_TXT                            "User Text"
#define GEXDB_PLUGIN_DBFIELD_TRACKING_LOT                        "Tracking Lot ID"
#define GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT                      "Valid Splitlot"
#define GEXDB_PLUGIN_DBFIELD_PROD_DATA                           "Production Data"
#define GEXDB_PLUGIN_DBFIELD_RETESTPHASE                         "Retest Phase"
#define GEXDB_PLUGIN_DBFIELD_TESTINSERTION                       "Test Insertion"
#define GEXDB_PLUGIN_DBFIELD_TESTFLOW                            "Test Flow"

#define GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_PRODUCT           "WS Product"
#define GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_LOT_ID            "WS Lot ID"
#define GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_WAFER_ID          "WS Wafer ID"

#define GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_DIE_ID          "Wafer Die ID"
#define GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_PRODUCT_ID      "Wafer Product ID"
#define GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_SUBLOT_ID       "Wafer Sublot ID"
#define GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_TRACKING_LOT_ID "Wafer Tracking Lot ID"
#define GEXDB_PLUGIN_DBFIELD_DAY                                 "Day"
#define GEXDB_PLUGIN_DBFIELD_WEEK                                "Week"
#define GEXDB_PLUGIN_DBFIELD_MONTH                               "Month"
#define GEXDB_PLUGIN_DBFIELD_QUARTER                             "Quarter"
#define GEXDB_PLUGIN_DBFIELD_YEAR                                "Year"
#define GEXDB_PLUGIN_DBFIELD_YEAR_WEEK                           "Year/Week"
#define GEXDB_PLUGIN_DBFIELD_YEAR_MONTH                          "Year/Month"
#define GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER                        "Year/Quarter"
#define GEXDB_PLUGIN_DBFIELD_PARTS                               "Parts"
#define GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE                        "Parts Sample"
#define GEXDB_PLUGIN_DBFIELD_GROSS_DIE                           "Gross Die"
#define GEXDB_PLUGIN_DBFIELD_PARTS_GOOD                          "Good Parts"
#define GEXDB_PLUGIN_DBFIELD_LOT_PARTS                           "Lot Parts"
#define GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS                        "Sublot Parts"
#define GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD                      "Lot Good Parts"
#define GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD                   "Sublot Good Parts"
#define GEXDB_PLUGIN_DBFIELD_WAFER_PARTS                         "Wafer Parts"
#define GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE                     "Wafer Gross Die"
#define GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD                    "Wafer Good Parts"
#define GEXDB_PLUGIN_DBFIELD_WAFER_NB                            "Wafer Nb"
#define GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE              "Consolidated Data Type"
#define GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME                  "Consolidation Name"
#define GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW                  "Consolidation Flow"
#define GEXDB_PLUGIN_DBFIELD_ETEST_SITE_CONFIG                   "Etest Site Config"

// Delimiters for internal use in expressions
#define GEXDB_PLUGIN_DELIMITER_FROMTO       "#GEXDB#TO#"
#define GEXDB_PLUGIN_DELIMITER_OR           "#GEXDB#OR#"

// Alarm FLAGS (for the Flags field in table XX_PROD_ALARM)
#define GEXDB_PLUGIN_PRODALARM_LCL_VALID    0x0001
#define GEXDB_PLUGIN_PRODALARM_UCL_VALID    0x0002

// Markers to identify the origin of stats written into the TSR (summary, samples)
#define GEXDB_PLUGIN_TSR_STATS_SOURCE_SAMPLES           "[GEXDB_TSR_STATS_FROM_SAMPLES]"
#define GEXDB_PLUGIN_TSR_STATS_SOURCE_SUMMARY           "[GEXDB_TSR_STATS_FROM_SUMMARY]"

// SYA rule options
#define GEXDB_PLUGIN_SYA_IGNORENULLSIGMA     0x01
#define GEXDB_PLUGIN_SYA_REMOVEOUTLIERS      0x02
#define GEXDB_PLUGIN_SYA_USEGROSSDIE         0x04
#define GEXDB_PLUGIN_SYA_EXCLUDE_BINS        0x08

// SYA flags
#define GEXDB_PLUGIN_SYA_EXPIRATIONEMAILSENT 0x01

class GexDbPlugin_Base;

typedef GexDbPlugin_Base* (*FP_gexdb_plugin_getobject)(const QString & strHostName,
                                                       const QString & strApplicationPath,
                                                       const QString & strUserProfile,
                                                       const QString & strLocalFolder,
                                                       const char *gexLabelFilterChoices[],
                                                       CGexSkin * pGexSkin,
                                                       GexScriptEngine* gse,
                                                       const bool bCustomerDebugMode);
typedef void (*FP_gexdb_plugin_releaseobject)(GexDbPlugin_Base *pObject);


/////////////////////////////////////////////////////////////////////////////////////////
// Classes to store results for Enterprise Reports: Parts
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_SerieDef
// Store definition for one serie of datapoints
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_SerieDef
{
public:
    // Default constructor
    GexDbPlugin_ER_Parts_SerieDef();

    // Constructor using legacy serie definition string
    GexDbPlugin_ER_Parts_SerieDef(QString & strSerieDefinition);
    ~GexDbPlugin_ER_Parts_SerieDef();

    GexDbPlugin_ER_Parts_SerieDef& operator=(const GexDbPlugin_ER_Parts_SerieDef& source)           // assignment operator
    {
        m_strSerieName                   = source.m_strSerieName;
        m_strBinnings                    = source.m_strBinnings;
        m_bWorkOnBinnings                = source.m_bWorkOnBinnings;
        m_bPlotSerie                     = source.m_bPlotSerie;
        m_strChartingMode                = source.m_strChartingMode;
        m_nColor                         = source.m_nColor;
        m_strDataLabels                  = source.m_strDataLabels;
        m_strLineStyle                   = source.m_strLineStyle;
        m_strLineSpots                   = source.m_strLineSpots;
        m_strLineProperty                = source.m_strLineProperty;
        m_strTableData                   = source.m_strTableData;
        m_fYieldExceptionLimit           = source.m_fYieldExceptionLimit;
        m_uiYieldExceptionLimit_Type     = source.m_uiYieldExceptionLimit_Type;
        m_bYieldExceptionLimit_Strict    = source.m_bYieldExceptionLimit_Strict;
        m_strProduct                     = source.m_strProduct;
        m_strTestingStage                = source.m_strTestingStage;
        m_strYieldExceptionRule          = source.m_strYieldExceptionRule;
        m_bParameterSerie                = source.m_bParameterSerie;
        m_strParameter                   = source.m_strParameter;
        m_strParameterName               = source.m_strParameterName;

        return *this;
    }
    void SetYieldException(const QString & strYieldException)
    {
        QString strLimit;
        QRegExp           clRegExp("[<>=a-z%]");
        m_strYieldExceptionRule = strYieldException;

        // Set yield limits variables
        if(m_strYieldExceptionRule == "-")
            m_uiYieldExceptionLimit_Type = 0;
        else if(m_strYieldExceptionRule.indexOf("<") != -1)
            m_uiYieldExceptionLimit_Type = 1;
        else
            m_uiYieldExceptionLimit_Type = 2;
        if(m_strYieldExceptionRule.indexOf("=") != -1)
            m_bYieldExceptionLimit_Strict = false;
        else
            m_bYieldExceptionLimit_Strict = true;
        strLimit = m_strYieldExceptionRule.simplified().toLower();
        strLimit = strLimit.remove(" ");
        strLimit = strLimit.remove(clRegExp);
        m_fYieldExceptionLimit = strLimit.toFloat();
    }
    QString               m_strSerieName;                   // Name affected to this serie
    QString               m_strBinnings;                    // Binnings corresponding to this data serie
    bool                  m_bWorkOnBinnings;                // True if the serie is defined through a list of binnings (not "pass bins" or "fail bins")
    bool                  m_bPlotSerie;                     // True if serie should be plotted, false else
    QString               m_strChartingMode;                // Serie charting mode (Bars or Lines)
    int                   m_nColor;                         // Serie color code
    QString               m_strDataLabels;                  // Serie data labels (no label, top, left...)
    QString               m_strLineStyle;                   // Line style (line, spline...)
    QString               m_strLineSpots;                   // Spots to use on line (none, square...)
    QString               m_strLineProperty;                // Line properties (solid, dahsed...)
    QString               m_strTableData;                   // Serie table data option (Yield, Volume, Yield & volume,none...)
    float                 m_fYieldExceptionLimit;
    unsigned int          m_uiYieldExceptionLimit_Type;     // 0=No limit, 1=HighLimit, 2=LowLimit
    bool                  m_bYieldExceptionLimit_Strict;    // True if limit is strict
    QString               m_strProduct;                     // Product filter for this serie (used for Genealogy reports)
    QString               m_strTestingStage;                // Testing stage filter for this serie (used for Genealogy reports)
    bool                  m_bParameterSerie;                // Set to true if the serie is a parameter
    QString               m_strParameter;                   // Parameter number (if serie is a parameter)
    QString               m_strParameterName;               // Parameter name (if serie is a parameter)

private:
    QString               m_strYieldExceptionRule;          // Rule to use to detect a yield exception ("-", "Yield <= 100 %"....)
    static int s_numInstances;
};

//typedef           tdGexDbPluginERPartSeries                      QList<GexDbPlugin_ER_Parts_SerieDef*>;

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_Series
// List of serie definitions
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_Series: public QList<GexDbPlugin_ER_Parts_SerieDef*>
{
public:
    GexDbPlugin_ER_Parts_Series()
    {
    }
    ~GexDbPlugin_ER_Parts_Series()
    {
//        while (!this->isEmpty())
//             delete this->takeFirst();
//        clear();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_DataPoint
// Store data for one datapoint
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_DataPoint
{
public:
    GexDbPlugin_ER_Parts_DataPoint()
    {
        m_uiTotalParts = 0;
        m_uiMatchingParts = 0;
    }
    GexDbPlugin_ER_Parts_DataPoint(unsigned int uiMatchingParts)
    {
        m_uiMatchingParts = uiMatchingParts;
        m_uiTotalParts = 0;
    }
    GexDbPlugin_ER_Parts_DataPoint(unsigned int uiMatchingParts, unsigned int uiTotalParts)
    {
        m_uiMatchingParts = uiMatchingParts;
        m_uiTotalParts = uiTotalParts;
    }
    GexDbPlugin_ER_Parts_DataPoint(unsigned int uiParameterExecs, double lfParameterSum)
    {
        m_uiParameterExecs = uiParameterExecs;
        m_lfParameterSum = lfParameterSum;
    }
    double                 m_lfParameterSum;
    unsigned int           m_uiParameterExecs;
    unsigned int           m_uiTotalParts;
    unsigned int           m_uiMatchingParts;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_SerieData
// Store data for one serie of datapoints
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_SerieData: public QList<GexDbPlugin_ER_Parts_DataPoint>
{
public:
    ~GexDbPlugin_ER_Parts_SerieData();

    void Append(unsigned int uiMatchingParts)
    {
        append(GexDbPlugin_ER_Parts_DataPoint(uiMatchingParts));
    }
    void Append(unsigned int uiMatchingParts, unsigned int uiTotalParts)
    {
        append(GexDbPlugin_ER_Parts_DataPoint(uiMatchingParts, uiTotalParts));
    }
    void Append(unsigned int uiParameterExecs, double lfParameterSum)
    {
        append(GexDbPlugin_ER_Parts_DataPoint(uiParameterExecs, lfParameterSum));
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_Layer
// Store data for one graph layer
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_Layer: public QList<GexDbPlugin_ER_Parts_SerieData*>
{
public:
    GexDbPlugin_ER_Parts_Layer(unsigned int uiSeries, QStringList & strlLayerSplitValues);
    ~GexDbPlugin_ER_Parts_Layer();

    // Add data for 1 specific aggregate value
    void Add(const QString & strLabel, unsigned int uiNbParts, unsigned int uiNbParts_Good,
             double lfTestTime, QList<unsigned int> & uilMatchingParts);
    // Add data for 1 specific aggregate value
    void Add(const QString & strLabel, QList<unsigned int> & uilNbParts, QList<unsigned int> & uilMatchingParts);

    // Add data for 1 specific aggregate value. This function is used for Yield vs Parameter scatter
    // Serie 0 = Parameter
    // Serie 1 = Yield
    void Add(const QString & strLabel, unsigned int uiParameterExecs, double lfParameterSum,
             unsigned int uiTotalParts, unsigned int uiMatchingParts);
    // Update data (nb splitlots, wafers, lots, Maverick wafers, Maverick lots) for 1 specific aggregate value
    void Update(const QString & strLabel, unsigned int uiNbSplitlots, unsigned int uiNbWafers,
                unsigned int uiNbLots, unsigned int uiNbMaverickWafers, unsigned int uiNbMaverickLots);

    unsigned int           m_uiSeries;
    unsigned int           m_uiDataPoints;
    QStringList            m_strlLayerSplitValues;
    QStringList            m_strlAggregateLabels;
    QStringList            m_strlAggregateLabels_Short;
    QList<unsigned int>    m_uilNbParts;
    QList<unsigned int>    m_uilNbParts_Good;
    QList<unsigned int>    m_uilNbSplitlots;
    QList<unsigned int>    m_uilNbWafers;
    QList<unsigned int>    m_uilNbLots;
    QList<unsigned int>    m_uilNbMaverickWafers;
    QList<unsigned int>    m_uilNbMaverickLots;
    QList<double>          m_lflTestTime;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts_Graph
// Store data for one graph
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts_Graph: public QList<GexDbPlugin_ER_Parts_Layer*>
{
public:
    GexDbPlugin_ER_Parts_Graph(QStringList & strlGraphSplitValues);
    ~GexDbPlugin_ER_Parts_Graph();
    QStringList      m_strlGraphSplitValues;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_ER_Parts
// Store part data for ER
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_ER_Parts: public QList<GexDbPlugin_ER_Parts_Graph*>
{
public:
    GexDbPlugin_ER_Parts();
    ~GexDbPlugin_ER_Parts();

    void Init(  QStringList & strlFields_GraphSplit, QStringList & strlFields_LayerSplit, QString & strField_Aggregate,
                bool bSoftBin, GexDbPlugin_ER_Parts_Series & plistSeries, QString & strMaverickWafer_AlarmType,
                int nMaverickWafer_AlarmCount, int nMaverickLot_WaferCount, QString & strGenealogy_Granularity);

    void Clear();

    QString                     m_strYieldQuery;
    QString                     m_strSplitCountQuery;
    QStringList                 m_strlFields_GraphSplit;
    QStringList                 m_strlFields_LayerSplit;
    QString                     m_strField_Aggregate;
    bool                        m_bSoftBin;
    bool                        m_bWorkOnBinnings;
    unsigned int                m_uiNbAxis;
    GexDbPlugin_ER_Parts_Series m_plistSerieDefs;
    unsigned int                m_uiSeries;
    QString                     m_strBinlist_Pass;
    QString                     m_strBinlist_Fail;
    QString                     m_strMaverickWafer_AlarmType;          // Alarm type to use to define a maverick wafer: standard alarm, critical alarm or any alarm type.
    int                         m_nMaverickWafer_AlarmCount;           // Total alarms required in a wafer to show it 'Maverick'
    int                         m_nMaverickLot_WaferCount;             // Total Maverick wafers required in a lot to show it 'Maverick'
    QString                     m_strGenealogy_Granularity;            // Granularity (lot, wafer) for Genealogy Yield vs Yield scatter
private :
    static int s_numInstances;
};


/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_XYChartList
// Holds a list of objects (GexDbPlugin_XYChart_Data)
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_XYChartList: public QList<GexDbPlugin_XYChart_Data*>
{
    // CONSTRUCTOR/DESTRUCTOR
public:
    GexDbPlugin_XYChartList()
    {
    }
    ~GexDbPlugin_XYChartList()
    {
//        while (!this->isEmpty())
//             delete this->takeFirst();
//        clear();
    }

    // For standard yield graphs
    QString                      m_strBinList;
    QString                      m_strCumulField;
    QString                      m_strSplitField;
    QString                      m_strQuery;

    // For Genealogy graphs
    QString                      m_strProduct;
    QString                      m_strTestingstage_X;
    QString                      m_strBinList_X;
    QString                      m_strTestingstage_Y;
    QString                      m_strBinList_Y;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_WyrFormatColumn
// Store format data for one WYR column
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_WyrFormatColumn
{
public:
    // Constructor
    GexDbPlugin_WyrFormatColumn(const QString & strSiteName, int nColumnId, int nColumnNb, const QString & strColumnName, const QString & strDataType)
    {
        m_strSiteName = strSiteName;
        m_nColumnId = nColumnId;
        m_nColumnNb = nColumnNb;
        m_strColumnName = strColumnName;
        m_strDataType = strDataType;
    }

    // Public data
    QString                m_strSiteName;
    int                    m_nColumnId;
    int                    m_nColumnNb;
    QString                m_strColumnName;
    QString                m_strDataType;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_WyrFormat
// Holds a list of objects (GexDbPlugin_WyrFormatColumn)
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_WyrFormat: public QList<GexDbPlugin_WyrFormatColumn*>
{
    // CONSTRUCTOR/DESTRUCTOR
public:
    GexDbPlugin_WyrFormat()
    {
    }
    ~GexDbPlugin_WyrFormat()
    {
//        while (!this->isEmpty())
//             delete this->takeFirst();
//        clear();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_WyrDataset
// Store 1 WYR dataset (for 1 site, 1 year, 1 week)
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_WyrDataset
{
public:
    // Constructor
    GexDbPlugin_WyrDataset(const QString & strSiteName, unsigned int uiYear, unsigned int uiWeekNb)
    {
        m_strSiteName = strSiteName;
        m_uiYear = uiYear;
        m_uiWeekNb = uiWeekNb;
    }

    // Public data
    QString              m_strSiteName;
    unsigned int         m_uiYear;
    unsigned int         m_uiWeekNb;
    QString              m_strTitleRow;
    QStringList          m_strlDataRows;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_WyrData
// Holds a list of objects (GexDbPlugin_WyrDataset)
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_WyrData: public QList<GexDbPlugin_WyrDataset*>
{
    // CONSTRUCTOR/DESTRUCTOR
public:
    GexDbPlugin_WyrData()
    {
    }

    ~GexDbPlugin_WyrData()
    {
//        while (!this->isEmpty())
//             delete this->takeFirst();
//        clear();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_SplitlotInfo
// Holds info on 1 splitlot
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_SplitlotInfo
{
public:
    GexDbPlugin_SplitlotInfo()
    {
        m_lSplitlotID       = -1;
        m_uiSetupTime       = 0;
        m_uiStartTime       = 0;
        m_uiFinishTime      = 0;
        m_uiStationNb       = 0;
        m_uiNbParts         = 0;
        m_uiNbParts_Good    = 0;
        m_uiWeekNb          = 0;
        m_uiMonthNb         = 0;
        m_uiQuarterNb       = 0;
        m_uiYearNb          = 0;
    }

    // Fields from Galaxy DB (et_splitlot, wt_splitlot or ft_splitlot table)
    qlonglong             m_lSplitlotID;          // SPLITLOT_ID
    QString               m_strProductName;       // From PART_TYP, or overloaded by dbkeys
    QString               m_strLotID;             // LOT_ID
    QString               m_strSublotID;          // SUBLOT_ID
    unsigned int          m_uiSetupTime;          // SETUP_T
    unsigned int          m_uiStartTime;          // START_T
    unsigned int          m_uiFinishTime;         // FINISH_T
    unsigned int          m_uiStationNb;          // STAT_NUM
    QString               m_strTesterName;        // TESTER_NAME
    QString               m_strTesterType;        // TESTER_TYPE
    unsigned int          m_uiNbParts;            // NB_PARTS
    unsigned int          m_uiNbParts_Good;       // NB_PARTS_GOOD
    QString               m_strJobName;           // JOB_NAME
    QString               m_strJobRev;            // JOB_REV
    QString               m_strOperator;          // OPER_NAM
    QString               m_strExecType;          // EXEC_TYP
    QString               m_strExecVer;           // EXEC_VER
    QString               m_strTestTemp;          // TST_TEMP
    QString               m_strHandlerType;       // HANDLER_TYP
    QString               m_strHandlerID;         // HANDLER_ID
    QString               m_strCardType;          // CARD_TYP
    QString               m_strCardID;            // CARD_ID
    QString               m_strLoadboardType;     // LOADBOARD_TYP
    QString               m_strLoadboardID;       // LOADBOARD_ID
    QString               m_strDibType;           // DIB_TYP
    QString               m_strDibID;             // DIB_ID
    QString               m_strCableType;         // CABLE_TYP
    QString               m_strCableID;           // CABLE_ID
    QString               m_strContactorType;     // CONTACTOR_TYP
    QString               m_strContactorID;       // CONTACTOR_ID
    QString               m_strLaserType;         // LASER_TYP
    QString               m_strLaserID;           // LASER_ID
    QString               m_strExtraType;         // EXTRA_TYP
    QString               m_strExtraID;           // EXTRA_ID
    QString               m_strDay;               // DAY
    unsigned int          m_uiWeekNb;             // WEEK_NB
    unsigned int          m_uiMonthNb;            // MONTH_NB
    unsigned int          m_uiQuarterNb;          // QUARTER_NB
    unsigned int          m_uiYearNb;             // YEAR_NB
    QString               m_strYearAndWeek;       // YEAR_AND_WEEK
    QString               m_strYearAndMonth;      // YEAR_AND_MONTH
    QString               m_strYearAndQuarter;    // YEAR_AND_QUARTER
    QString               m_strFileName;          // FILE_NAME
    QString               m_strFilePath;          // FILE_PATH
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_SplitlotList
// Holds a list of objects (GexDbPlugin_SplitlotInfo)
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_SplitlotList: public QList<GexDbPlugin_SplitlotInfo*>
{
    // CONSTRUCTOR/DESTRUCTOR
public:
    GexDbPlugin_SplitlotList();
    ~GexDbPlugin_SplitlotList();

    enum SortOn
    {
        eSortOnStart_t,       // Sorting should be done on Start_t field
        eSortOnLotID          // Sorting should be done on LotID field
    };

    // PUBLIC METHODS
public:
    void  Sort(SortOn eSortSelector, bool bAscending);

    // PROTECTED METHODS
protected:

    // PROTECTED DATA
protected:
    // For sorting
    int        mSortSelector;           // Tells on which var to sort
};


///////////////////////////////////////////////////////////
// GexDbPlugin_FtpSettings class: ftp settings...
///////////////////////////////////////////////////////////
class GexDbPlugin_FtpSettings
{
public:
    GDECLARE_ERROR_MAP(GexDbPlugin_FtpSettings)
    {
        eMarkerNotFound                                 // XML marker not found in settings file
    }
    GDECLARE_END_ERROR_MAP(GexDbPlugin_FtpSettings)

    // Constructor / destructor / operators
    GexDbPlugin_FtpSettings();                                                                                                                                               // Consturctor
    ~GexDbPlugin_FtpSettings();                                                                                                                                               // Destructor
    GexDbPlugin_FtpSettings& operator=(const GexDbPlugin_FtpSettings& source);// assignment operator

    // Read/Write settings
    bool           LoadSettings(QFile *pSettingsFile);// Load settings from file and init the calls variables
    bool           WriteSettings(QTextStream *phFile);// Write settings to file using informations loaded in the class variables

    // Get last error
    void           GetLastError(QString & strError); // Returns details about last error

    // Member variables
    QString        m_strHostName;                    // Ftp host name
    QString        m_strUserName;                    // Ftp login name
    QString        m_strPassword;                    // Ftp login password
    unsigned int   m_uiPort;                         // Ftp connection port#
    QString        m_strPath;
    bool           m_bHostnameFromDbField;           // true if hostname should be retrieved from the database
};

///////////////////////////////////////////////////////////
// OpenSqlConnectionThread class: database connection...
///////////////////////////////////////////////////////////
class OpenSqlConnectionThread : public QThread
{
    Q_OBJECT

public:

    // constructor/destructor
    OpenSqlConnectionThread(void)            {m_bConnected = m_bConnectionTimeOut = false;}
    virtual ~OpenSqlConnectionThread(void)   {}

    bool IsConnectionTimeOut()               {return m_bConnectionTimeOut;}
    void WaitForFinished();
    int  GetTimerElapsed()                   {return m_clTimer.elapsed();}

public slots:
    bool exec(QString strConnectionName,int nConnectionTimeOut);
protected:
    virtual void run();

private:
    QTime          m_clTimer;

    bool           m_bConnected;
    bool           m_bConnectionTimeOut;
    QString        m_strConnectionName;
};


///////////////////////////////////////////////////////////
// GexDbPlugin_Connector class: database connection...
///////////////////////////////////////////////////////////
class GexDbPlugin_Connector
{
public:

    GDECLARE_ERROR_MAP(GexDbPlugin_Connector)
    {
        // General
        eMarkerNotFound,         // XML marker not found in settings file
                // Database errors
                eDB_CreateConnector,     // Error creating database connector
                eDB_Connect,             // Error connecting to database
                eDB_NoConnection,        // No Database connection
                eDB_NoFields,            // No fields for specified table
                eDB_InvalidTable,        // Specified table does not exist in DB
                eDB_InvalidField,        // Specified field does not exist in DB
                eDB_Query               // Error executing SQL query
    }
    GDECLARE_END_ERROR_MAP(GexDbPlugin_Connector)

    // Constructor / destructor / operators
    GexDbPlugin_Connector(const QString & strPluginName, GexDbPlugin_Base *pPluginBase=NULL);                       // Consturctor 1
    GexDbPlugin_Connector(const QString & strPluginName, bool bCustomerDebugMode, const QString & strUserProfile);           // Consturctor 2
    GexDbPlugin_Connector(const GexDbPlugin_Connector& source);                                                     // Copy constructor
    ~GexDbPlugin_Connector();                                                                                                                                                          // Destructor
    GexDbPlugin_Connector& operator=(const GexDbPlugin_Connector& source);                                          // assignment operator

    // Database exchanges
    bool           Connect();                                                                                                                                    // Connect to remote database
    bool           EnumDrivers(QStringList & strlDrivers, QStringList & strlFriendlyNames);// Retrieves the list of database drivers available on current computer
    QString        GetDriverName(const QString& strFriendlyName);                  // Get driver name corresponding to specified friendly name
    QString        GetFriendlyName(QString & strDriverName);                       // Get friendly name corresponding to specified driver name
    bool           IsConnected();
    bool           Disconnect(void);                                               // diconnect database (closes link)
    bool           EnumTables(QStringList &strTables);                             // Retrieves the list of tables available in database
    bool           EnumFields(QString strTable,QStringList & strlFields);          // Retrieves the list of fields in a given table
    bool           UpdateMapping(int *pnItemsRemoved, int *pnItemsAdded, const QString & strTableName, const QStringList & strlGexFields, QMap<QString, QString> & cFieldMapping);           // Check fields mapping (fields in remote DB must exist)
    void           SetAdminLogin(bool bAdminLogin);                                // Set database login type (std user/admin user)
    void           SetPluginBasePtr(GexDbPlugin_Base *pPluginBase);                // Sets ptr to plugin base (useful if connector not the private plugin base connector)
    bool           IsOracleDB() const;                                                   // Returns true if connected to an Oracle DB
    bool           IsMySqlDB() const;                                                    // Returns true if connected to a MySQL DB
    bool           IsOracleDB(QString & strDatabaseDriver);                        // Returns true if connected to an Oracle DB
    bool           IsMySqlDB(QString & strDatabaseDriver);                         // Returns true if connected to a MySQL DB

    bool           IsSQLiteDB();                                                   // Returns true if "connected" to a SQlite DB
    bool           IsSQLiteDB(QString & strDatabaseDriver);                        // Returns true if strDatabaseDriver is a SQlite driver name
    QString        GetSQLiteDriverVersion();                                       // Returns the version of the SQLite DB driver. To Do : universal GetDBVersion() for all SGBD (SQLite, MySQL,...)

    // Read/Write settings
    bool           LoadSettingsFromDom(const QDomElement &node);                      // Load settings from DomElt and init the calls variables
    bool           LoadSettings(QFile *pSettingsFile);                             // Load settings from file and init the calls variables
    QDomElement    GetSettingsDom(QDomDocument &doc);
    bool           WriteSettings(QTextStream *phFile);                             // Write settings to file using informations loaded in the class variables

    // Get last error
    void           GetLastError(QString & strError);                               // Returns details about last error

    // Resolved the UserHostName with result in hostName and hostIP
    // Return the list of IP found
    QStringList    ResolveHostName(QString UserHostName,

                                   QString& hostName,
                                   QString& hostIP,
                                   bool *pbIsLocalHost);

    // Mysql/Oracle specific words list
    QStringList    SqlReservedWordList();                                          // build the list of Mysql/Oracle reserved word
    QStringList    m_lstSqlReservedWords;

    // translate string value for SQL query (ex: ' is not supported in a field)
    QString        TranslateStringToSqlVarChar(const QString &strValue, bool bAddQuote=true);
    // translate string value for SQL LOB query (ex: ORACLE = add to_clob)
    QString        TranslateStringToSqlLob(const QString &strValue);
    // apply a sql date format to translate a time
    QString TranslateUnixTimeStampToSqlDateTime(const QString strUnixTimeStamp, enum SqlUnixDateFormat eFormat, enum SqlUnixDateFormat eConcatFormat=eEmpty);

    QDateTime      ServerDateTime();
    QString        TranslateDateToSqlDateTime(QDateTime Date);




protected:
    bool           SetNlsParameters();
    bool           OpenDatabase();
    void           WriteDebugMessageFile(const QString & strMessage, bool bUpdateGlobalTrace=false);

    bool           OpenConnection(QString &strConnectionName);                  // Open a SqlDatabase connection into a thread

public:
    // Variables
    QString   m_strPluginName;            // Name of the plugin (used to differentiate different database connections in addDatabase() function)
    QString   m_strSettingsFile;          // Absolute path to the settings file (DatabaseFolder)
    QString   m_strHost_Unresolved;       // Initial unresolved Host name/IP
    QString   m_strHost_Name;             // External DB host name (resolved name of the host)
    QString   m_strHost_IP;               // External DB host name (resolved IP of the host)
    QString   m_strHost_LastUsed;         // Last Host used with success
    QString   m_strDriver;                // Driver Type (Oracle, etc...)
    QString   m_strDatabaseName;          // DB name
    QString   m_strUserName;              // DB login name (standard user)
    QString   m_strPassword;              // DB login password (standard user)
    QString   m_strUserName_Admin;        // DB login name (admin user)
    QString   m_strPassword_Admin;        // DB login password (admin user)
    QString   m_strSchemaName;            // DB schema to use to access objects
    int       m_uiPort;                   // DB connection port#
    bool      m_bUseQuotesInSqlQueries;   // Set to true if quotes should be used for schema/table names in SQL queries
    QString   m_strNlsNumericCharacters;  // Value of parameter NLS_NUMERIC_CHARACTERS retrieved from Oracle
    bool      m_bPartitioningDb;          // true if this Db have partitioning option avalaible
    bool      m_bTransactionDb;           // true if this Db can use transaction (InnoDb or Oracle)
    bool      m_bCompressionProtocol;     // true if this Db uses a COMPRESS protocole
    bool      m_bIsLocalHost;             // true if the connection is local
    bool      m_bAdminUser;               // true if we should use the Admin user, false else
    bool      m_bConnectedAsAdmin;        // true if currently connected as admin user, false else
    // (used to create temp table name for WAFER consolidation: <30 char on Oracle)
    QString   m_strConnectionName;        // Connection Name using unique connection ID
    int       GetConnectionSID();         // Unique number fron SQL(Oracle/MySql) connection
    int       m_nConnectionSID;
    QString   GetConnectionHostName();    // HostName fron SQL connection
    QString   m_strConnectionHostName;
    QString   mAdminDbHostName;
    int       mAdminDbPort;
    QString   mAdminDbDatabaseName;
    QString   mAdminDbUser;
    QString   mAdminDbPwd;
    QString   m_linkedAdrHostName;
    int       m_linkedAdrPort;
    QString   m_linkedAdrDatabaseName;
    QString   m_linkedAdrUser;
    QString   m_linkedAdrPwd;

    static int GetNumberOfInstances() { return n_instances; }

protected:
    GexDbPlugin_Base                  *m_pPluginBase;           // Ptr to parent plugin base object
    bool                              m_bCustomerDebugMode;     // Set to true if application running in customer debug mode
    QString                           m_strUserProfile;         // User profile directory
    QList<OpenSqlConnectionThread*>   m_lstOpenSqlThread;       // Open a SqlDatabase connection into a thread

private:
    bool          LoadHostFromDom(const QDomElement &node);
    bool          LoadUnresolvedHostNameFromDom(const QDomElement &node);
    bool          LoadHostNameFromDom(const QDomElement &node);
    bool          LoadHostIPFromDom(const QDomElement &node);
    bool          LoadDriverFromDom(const QDomElement &node);
    bool          LoadDatabaseNameFromDom(const QDomElement &node);
    bool          LoadUserNameFromDom(const QDomElement &node);
    bool          LoadUserPasswordFromDom(const QDomElement &node);
    bool          LoadUserName_AdminFromDom(const QDomElement &node);
    bool          LoadUserPassword_AdminFromDom(const QDomElement &node);
    bool          LoadSchemaNameFromDom(const QDomElement &node);
    bool          LoadPortFromDom(const QDomElement &node);

    QDomElement   GetHostDom(QDomDocument &doc);
    QDomElement   GetUnresolvedHostNameDom(QDomDocument &doc);
    QDomElement   GetHostNameDom(QDomDocument &doc);
    QDomElement   GetHostIPDom(QDomDocument &doc);
    QDomElement   GetDriverDom(QDomDocument &doc);
    QDomElement   GetDatabaseNameDom(QDomDocument &doc);
    QDomElement   GetUserNameDom(QDomDocument &doc);
    QDomElement   GetUserPasswordDom(QDomDocument &doc);
    QDomElement   GetUserName_AdminDom(QDomDocument &doc);
    QDomElement   GetUserPassword_AdminDom(QDomDocument &doc);
    QDomElement   GetSchemaNameDom(QDomDocument &doc);
    QDomElement   GetPortDom(QDomDocument &doc);

    static int    n_instances;
    QTime         m_LastTestConnection;
};

///////////////////////////////////////////////////////////
// GexDbPlugin_ExtractionTimers class:
// performance measurement for database extraction
///////////////////////////////////////////////////////////
class GexDbPlugin_ExtractionPerf: public QTime
{
public:

    // Constructor / destructor / operators
    GexDbPlugin_ExtractionPerf()        // Consturctor
    {
        Reset(true);
    }
    ~GexDbPlugin_ExtractionPerf()       // Destructor
    {
    }
    void Reset(bool bTotal)
    {
        m_fTime_Partial = 0.0F;
        m_fTimer_DbQuery_Partial = 0.0F;
        m_fTimer_DbIteration_Partial = 0.0F;
        m_ulNbRows_Partial = 0;
        m_ulNbRuns_Partial = 0;
        m_ulNbTestResults_Partial = 0;
        if(bTotal)
        {
            m_fTime_Total = 0.0F;
            m_fTimer_DbQuery_Total = 0.0F;
            m_fTimer_DbIteration_Total = 0.0F;
            m_ulNbRows_Total = 0;
            m_ulNbRuns_Total = 0;
            m_ulNbTestResults_Total = 0;
        }
    }
    void Start(bool bTotal=false)
    {
        Reset(bTotal);
        m_clTimer_Partial.start();
        if(bTotal)
            m_clTimer_Total.start();
    }
    void Stop()
    {
        m_fTime_Partial = (float)m_clTimer_Partial.elapsed()*1000.0F;
        m_fTime_Total = (float)m_clTimer_Total.elapsed()*1000.0F;
    }
    void Stop(unsigned long ulNbRows, float fDbQuery, float fDbIteration)
    {
        Stop();

        m_fTimer_DbQuery_Partial = fDbQuery;
        m_fTimer_DbIteration_Partial = fDbIteration;
        m_ulNbRows_Partial = ulNbRows;

        m_fTimer_DbQuery_Total += fDbQuery;
        m_fTimer_DbIteration_Total += fDbIteration;
        m_ulNbRows_Total += ulNbRows;
    }
    void Stop(unsigned long ulNbRows, float fDbQuery, float fDbIteration, unsigned int uiNbRuns, unsigned int uiNbTestResults)
    {
        Stop(ulNbRows, fDbQuery, fDbIteration);
        m_ulNbRuns_Partial = (unsigned long)uiNbRuns;
        m_ulNbTestResults_Partial = (unsigned long)uiNbTestResults;
        m_ulNbRuns_Total += (unsigned long)uiNbRuns;
        m_ulNbTestResults_Total += (unsigned long)uiNbTestResults;
    }

    float           m_fTime_Partial;               // Total execution time (us)
    float           m_fTime_Total;                 // Partial execution time (us)
    float           m_fTimer_DbQuery_Partial;      // Holds DB execution time cumul (partial) (us)
    float           m_fTimer_DbIteration_Partial;  // Holds DB iteration time cumul (partial) (us)
    float           m_fTimer_DbQuery_Total;        // Holds DB execution time cumul (total) (us)
    float           m_fTimer_DbIteration_Total;    // Holds DB iteration time cumul (total) (us)
    unsigned long   m_ulNbRows_Partial;            // Nb of extracted rows (partial)
    unsigned long   m_ulNbRows_Total;              // Nb of extracted rows (total)
    unsigned long   m_ulNbRuns_Partial;            // Nb of extracted runs (partial)
    unsigned long   m_ulNbTestResults_Partial;     // Nb of extracted test results (partial)
    unsigned long   m_ulNbRuns_Total;              // Nb of extracted runs (partial)
    unsigned long   m_ulNbTestResults_Total;       // Nb of extracted test results (partial)

protected:
    QTime           m_clTimer_Partial;
    QTime           m_clTimer_Total;
};

///////////////////////////////////////////////////////////
// GexDbPlugin_Query class: query execution/iteration with
// performance management...
///////////////////////////////////////////////////////////
class GexDbPlugin_Query: public QSqlQuery
{
public:

    // Constructor / destructor / operators
    GexDbPlugin_Query(GexDbPlugin_Base *pPluginBase, QSqlDatabase db);      // Consturctor
    GexDbPlugin_Query(const GexDbPlugin_Query& source);                     // Copy constructor
    ~GexDbPlugin_Query();                                                   // Destructor
    GexDbPlugin_Query& operator=(const GexDbPlugin_Query& source);          // assignment operator

#ifdef _WIN32
    unsigned long     m_ulPerformanceFrequency;          // Used to hold the system's performance counter frequency (counts per sec);
    LARGE_INTEGER     m_liPerformanceCounter_Ref;        // Used to hold the system's performance counter value when internal timer started
    LARGE_INTEGER     m_liPerformanceCounter_Query;      // Used to hold the system's performance counter value when query started
#endif
    bool            m_bPerformanceCounterSupported;      // Set to true if system supports a performance counter
    QString         m_strQuery;                          // Current query
    QTime           m_clQueryTimer_Ref;                  // For query execution time measurement (started when internal timer started)
    QTime           m_clQueryTimer_Query;                // For query execution time measurement (started at execution of each query)
    float           m_fTimer_DbQuery;                    // Holds DB execution time for current query (us)
    float           m_fTimer_DbIteration;                // Holds DB iteration time for current query (us)
    float           m_fTimer_DbQuery_Cumul;              // Holds DB execution time cumul (us)
    float           m_fTimer_DbIteration_Cumul;          // Holds DB iteration time cumul (us)
    unsigned long   m_ulRetrievedRows;                   // Nb of rows retrieved for current query
    unsigned long   m_ulRetrievedRows_Cumul;             // Nb of rows retrieved (cumul)

    bool  Prepare(const QString & strQuery);             // Prepare specified query
    bool  Execute();                                     // Execute prepared query
    bool  Execute(const QString & strQuery);             // Execute specified query
    // Iterate to first query result
    bool    First();
    // Iterate to next query result. Return false if no more result
    bool    Next();
    // Seek to specified query result row
    bool    Seek(int i);
    // If in debug mode, dump query performance into trace file
    void    DumpPerformance();
    // Start internal timers
    void    StartTimer(bool bQueryMainTimer=false);
    float   Elapsed(float *pfQueryMainTimer=NULL);          // Returns nb of micro-sec since internal timers started

    // The value().toChar() doesn't seem to work correctly, so we implement a workaround
    char    GetChar(int nIndex);        // Returns a char for specified index in the query result
    QChar   GetQChar(int nIndex);       // Returns a QChar for specified index in the query result

protected:
    GexDbPlugin_Base  *m_pPluginBase;   // Ptr on plugin base

private :
    bool canExecuteQuery() const;
};

///////////////////////////////////////////////////////////
// GexDbPlugin_Mapping_Field class: class to hold mapping
// fields
///////////////////////////////////////////////////////////
class GexDbPlugin_Mapping_Field
{
public:
    GexDbPlugin_Mapping_Field();
    GexDbPlugin_Mapping_Field(const GexDbPlugin_Mapping_Field& other);
    GexDbPlugin_Mapping_Field(QString strMetaDataName,
                              QString strGexName,
                              QString strSqlTable,
                              QString strSqlFullField,
                              QString strSqlLinkName,
                              bool bDisplayInQueryGui,
                              QString sBinType,
                              bool bTime,
                              bool bCustom,
                              bool bFact,
                              bool bConsolidated,
                              bool bDisplayInERGui,
                              bool bNumeric,
                              bool bAZ,
                              bool bStaticMetaData=true);
    QString   m_strMetaDataName;      // ex1 "Business Unit"              ex2 "Lot number"
    QString   m_strGexName;           // ex1 "",                          ex2 "LotID"
    QString   m_strNormalizedName;    // ex1 "business_unit"              ex2 "lot_number"
    QString   m_strSqlTable;          // ex1 "wm_lotpde",                 ex2 "ft_lot"
    QString   m_strSqlFullField;      // ex1 "wm_lotpde.pde_businessunit",ex2 "ft_lot.lot_id"
    QString   m_strSqlLinkName;       // ex1 "wm_lotpde-ft_lot",          ex2 "ft_lot-ft_splitlot"
    bool      m_bDisplayInQueryGui;   // set to true if this meta-data should be displayed in the query GUI page
    bool      m_bNumeric;             // set to true if the DB field is a numeric field (string otherwise)
    QString   m_sBinType;             // tells if the field is bin relative (bin_no, bin_cat,...) : 'H', 'S' or 'N'
    bool      m_bTime;                // tells if the field speaks of time (year, quarter, month, week, day)
    bool      m_bCustom;              // this field is linked from/to a custom client table.
    bool      m_bFact;                // this field is a fact (nb_parts,...)
    bool      m_bConsolidated;        // tells if the field should be used in the consolidated table(s)
    bool      m_bDisplayInERGui;      // set to true if this meta-data should be displayed in the ER GUI page
    bool      m_bAZ;                  // tells if the field should be used in the AZ table(s)

    // ACCESSORS
    bool      isTestCondition() const;  // Return true when the meta-data is a test condition
    bool      isStaticMetaData() const; // Return true when the meta-data is an internal Static metadata
    QString   getSqlFieldName() const;  // Return the name of the sql field

    void      setTestCondition(bool testCondition);

    // METHODS
    GexDbPlugin_Mapping_Field& operator=(const GexDbPlugin_Mapping_Field& other);

private:

    bool      mTestCondition;        // tells if the field is a test
    bool      mStaticMetaData;       // From GEX app

};

class GexDbPlugin_Mapping_FieldMap: public QMap<QString, GexDbPlugin_Mapping_Field>
{
public:
    // GexDbPlugin_Mapping_FieldMap class: Meta-data field mapping
    bool   ContainsGexField(const QString & strGexField,
                            GexDbPlugin_Mapping_Field & clFieldMapping);
    // GexDbPlugin_Mapping_FieldMap class: Meta-data field mapping
    bool   ContainsSqlFullField(const QString & strSqlFullField,
                                GexDbPlugin_Mapping_Field & clFieldMapping);
    // GexDbPlugin_Mapping_FieldMap class: Meta-data field mapping
    bool   GetSqlFullField(const QString & strMetaDataName,
                           QString & strSqlFullField);
    //GetFieldMapping()
};

class GexDbPlugin_Mapping_LinkMap;

///////////////////////////////////////////////////////////
// GexDbPlugin_Mapping_Link class: class to hold mapping
// links
///////////////////////////////////////////////////////////
class GexDbPlugin_Mapping_Link
{
public:
    GexDbPlugin_Mapping_Link()
    {
    }
    GexDbPlugin_Mapping_Link(QString strLinkName,
                             QString strSqlTable1,
                             QString strSqlFullField1,
                             QString strSqlTable2,
                             QString strSqlFullField2,
                             QString strSqlTable2Link)
    {
        m_strLinkName =       strLinkName;
        m_strSqlTable1 =      strSqlTable1;
        m_strSqlFullField1 =  strSqlFullField1;
        m_strSqlTable2 =      strSqlTable2;
        m_strSqlFullField2 =  strSqlFullField2;
        m_strSqlTable2Link =  strSqlTable2Link;
    }
    QString   m_strLinkName;       // ie "wm_lotpde-ft_lot"
    QString   m_strSqlTable1;      // ie "wm_lotpde"
    QString   m_strSqlFullField1;  // ie "wm_lotpde.lot_wm_lot_no"
    QString   m_strSqlTable2;      // ie "ft_lot"
    QString   m_strSqlFullField2;  // ie "ft_lot.tracking_lot_id"
    QString   m_strSqlTable2Link;  // ie "ft_lot-ft_splitlot"
};

class GexDbPlugin_Mapping_LinkMap: public QMap<QString, GexDbPlugin_Mapping_Link>
{
public:
    bool    IsLinkedToTable(const GexDbPlugin_Mapping_Field & clField, const QString & strTableName);
    /// \brief reccursive method to get the list of predecessor link of link
    QList<GexDbPlugin_Mapping_Link> GetLinkPredecessorList(
            const GexDbPlugin_Mapping_Link & link);
    /// \brief return the list of link which contain same two tables as link
    QList<GexDbPlugin_Mapping_Link> GetLinksWithSameTables(
            const GexDbPlugin_Mapping_Link &link) const;
};

///////////////////////////////////////////////////////////
// GexDbPlugin_Filter class: query filters...
///////////////////////////////////////////////////////////
class GexDbPlugin_Filter //: public QObject
{
    // failed to QObjectize this object : crash when accessing m_GexQueries...
    //Q_OBJECT
public:
    GexDbPlugin_Filter(QObject* parent);
    GexDbPlugin_Filter & operator=(const GexDbPlugin_Filter & source);
    QStringList   mQueryFields;             // Fields to query
    QStringList   strlQueryFilters;         // List of query filters. Each filter uses syntax <Field>=<Value>
    int           iTimePeriod;              // Time period to consider.
    int           iTimeNFactor;             // use for example in Last N * X
    enum eTimeStep { DAYS, WEEKS, MONTHS, QUARTERS, YEARS  } m_eTimeStep;           // Step used in Last N * X
    QString       strDataTypeQuery;         // Type of data to query on in SQL-DB (wafer sort, final test,...)
    QDate         calendarFrom;             // Filter: From date
    QDate         calendarTo;               // Filter: To date
    QTime         calendarFrom_Time;        // Filter: From time
    QTime         calendarTo_Time;          // Filter: To time
    time_t        tQueryFrom;               // Query: From date calculated from iTimeperiod and calenderFrom/calenderTo
    time_t        tQueryTo;                 // Query: To date calculated from iTimeperiod and calenderFrom/calenderTo
    bool          bUseTimePeriod;           // Set to true if we should use time period filter
    bool          bConsolidatedExtraction;  // Raw data extraction: true=consolidate raw data at extraction, false=extract all raw data
    QString       strSiteFilterValue;       // Contains site (hardware site# for multi-site test programs) filter value if any set
    QString       strTestList;              // List of parameters to extract
    QString       strOptionsString;         // Plug-in options string
    int           nNbQueryFields;           // Nb of fields in the Select statement (used for multi-fields filters, ie Lot-Wafer)
    bool          bConsolidatedData;        // Extract only consolidated data
    int           mMinimumPartsInFile;      // Minimum parts count for extraction
    // gexQuery stack :
    // example : db_extraction_mode, db_notch, db_exclude_wafers_not_available_at, db_granularity, db_bintype...
    QList< QList<QString> > m_gexQueries;

    void  Reset();

    void  Reset(const QStringList & strlFilters);

    // Fill filters (filter value) from stringlist (<field name>=<filter value>)
    void  SetFilters(const QStringList & strlFilters);

    // Fill fields to query (fields name) from stringlist (<field name>)
    void SetFields(const QStringList &fieldsName);
    void SetFields(const QString &fieldName);
    // informs if there are any metadata filters pointing to the given table WITH prefix : "ft_run", "ft_splitlot", ...
    // Do not forget to set TestingStage before calling this function in order to point to the desired metadata !
    // returns error on error, else ok
    QString ContainsFilterOnTable(const QString& table_name, GexDbPlugin_Base* plugin, bool &result) const;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfo_Stats
// Holds statistical info on a test
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfo_Stats
{
public:
    GexDbPlugin_TestInfo_Stats();
    GexDbPlugin_TestInfo_Stats(const GexDbPlugin_TestInfo_Stats & source);
    ~GexDbPlugin_TestInfo_Stats();

    GexDbPlugin_TestInfo_Stats & operator=(const GexDbPlugin_TestInfo_Stats & source);

    unsigned int  m_uiExecCount;
    unsigned int  m_uiFailCount;
    float         m_fMin;
    float         m_fMax;
    float         m_fSum;
    float         m_fSumSquare;
    float         m_fTestTime;
    unsigned int  m_uiOptFlag;

    void Reset();
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfo_PTR
// Holds specific info on 1 parametric test
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfo_PTR
{
public:
    GexDbPlugin_TestInfo_PTR();
    ~GexDbPlugin_TestInfo_PTR();

    GexDbPlugin_TestInfo_PTR & operator=(const GexDbPlugin_TestInfo_PTR & source);

    void ResetDynamicInfo();

    // Static info
    unsigned int  m_uiStaticFlags;
    QString       m_strTestUnit;
    bool          m_bHasLL;
    bool          m_bHasHL;
    float         m_fLL;
    float         m_fHL;
    bool          m_bHasSpecLL;
    bool          m_bHasSpecHL;
    float         m_fSpecLL;
    float         m_fSpecHL;
    int           m_nResScal;     // Result scaling factor
    int           m_nLlScal;      // LL scaling factor
    int           m_nHlScal;      // HL scaling factor
    QList<QVariantMap> mMultiLimits; // Multi limits

    // Dynamic info (may change for each run)
    QList<unsigned int> m_uiDynamicFlagsList;
    QList<float>        m_fResultList;  // Measured value
};



/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfo_MPR
// Holds specific info on 1 multi-result parametric test
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfo_MPR
{
public:
    GexDbPlugin_TestInfo_MPR();
    ~GexDbPlugin_TestInfo_MPR();

    GexDbPlugin_TestInfo_MPR & operator=(const GexDbPlugin_TestInfo_MPR & source);

    void ResetDynamicInfo();

    // Static info
    unsigned int  m_uiStaticFlags;
    QString       m_strTestUnit;
    bool          m_bHasLL;
    bool          m_bHasHL;
    float         m_fLL;
    float         m_fHL;
    bool          m_bHasSpecLL;
    bool          m_bHasSpecHL;
    float         m_fSpecLL;
    float         m_fSpecHL;
    int           m_nTpin_ArrayIndex;
    int           m_nResScal;         // Result scaling factor
    int           m_nLlScal;          // LL scaling factor
    int           m_nHlScal;          // HL scaling factor

    // Dynamic info (may change for each run)
    QList<unsigned int>  m_uiDynamicFlagsList;
    QList<float>         m_fResultList;// Measured value
    QList<int>           m_nTpin_PmrIndexList;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfo_FTR
// Holds specific info on 1 functional test
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfo_FTR
{
public:
    GexDbPlugin_TestInfo_FTR();
    ~GexDbPlugin_TestInfo_FTR();

    GexDbPlugin_TestInfo_FTR & operator=(const GexDbPlugin_TestInfo_FTR & source);

    void ResetDynamicInfo();

    // Static info

    // Dynamic info (may change for each run)
    QList<unsigned int>  m_uiDynamicFlagsList;
    QList<QString>       m_strVectorNameList;
    QList<unsigned int>  m_uiVectorOffsetList;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfo
// Holds info on 1 test
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfo
{
public:
    enum TestType
    {
        eTestAny, // Any test (PTR, MPR, FTR)
        eTestPTR, // PTR tests
        eTestMPR, // MPR tests
        eTestFTR  // FTR tests
    };

    GexDbPlugin_TestInfo();
    GexDbPlugin_TestInfo(const GexDbPlugin_TestInfo & source);
    ~GexDbPlugin_TestInfo();

    GexDbPlugin_TestInfo & operator=(const GexDbPlugin_TestInfo & source);
    bool operator==(const GexDbPlugin_TestInfo & source);
    bool operator<(const GexDbPlugin_TestInfo & source);

    void Reset();
    void ResetDynamicInfo();

    // Static info
    qint64        m_lTestID;
    unsigned int  m_uiTestNumber;
    QString       m_strTestName;
    unsigned int  m_uiTestSeq;
    char          m_cTestType;                          // 'P'=parametric, 'M'=Multi-parametric, 'F'=Functional
    bool          m_bStaticInfoWrittenToStdf;           // Set to true if static info already written to STDF file

    // Statistics
    QMap<int, GexDbPlugin_TestInfo_Stats>  m_mapStatsFromSamples;
    QMap<int, GexDbPlugin_TestInfo_Stats>  m_mapStatsFromSummary;

    // Dynamic info (may change for each run)
    bool          m_bTestExecuted;                      // true if test was executed, false else
    bool          m_bHaveSamples;
    bool          m_bStatsFromSummaryComplete;          // True if all summary stats available (ExecCnt, FailCnt, Min, Max, Sums, Sqrs)
    bool          m_bMinimumStatsFromSummaryAvailable;  // True if at least Exec_Cnt and Fail_Cnt available

    // Ptrs to test-type specific info
    GexDbPlugin_TestInfo_PTR           *m_pTestInfo_PTR;
    GexDbPlugin_TestInfo_MPR           *m_pTestInfo_MPR;
    GexDbPlugin_TestInfo_FTR           *m_pTestInfo_FTR;
};




/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoMap
// Map of <unsigned int, GexDbPlugin_TestInfo>
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfoMap: public QMap<unsigned int, GexDbPlugin_TestInfo>
{
public:
    void Reset();
    void ResetDynamicInfo();
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoContainer
// Container having ptrs on GexDbPlugin_TestInfo objects
// We need this intermediate container class rather than adding
// Next pointers to the TestInfo object itself, because we need
// a dual list:
// o 1 ordered by Test Sequence (used when writing tests to STDF)
// o 1 ordered by Test ID (used when populating the list from DB extraction)
//
// The main list is ordered by Test Sequence. It goes through all elements, and is
// accessed through m_pNextTest.
//
// The secondary list is in fact 1 list per test type (PTR, MPR, FTR). Each of those lists
// is ordered by TestID, and goes only through the tests of the concerned type. It is
// accessed through the m_pNextTest_XXX pointers.
//
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfoContainer
{
public:
    GexDbPlugin_TestInfoContainer();
    ~GexDbPlugin_TestInfoContainer();

    GexDbPlugin_TestInfo            *m_pTestInfo;               // Ptr on TestInfo object
    GexDbPlugin_TestInfoContainer   *m_pNextTest;               // Ptr on next test in the list
    GexDbPlugin_TestInfoContainer   *m_pNextTest_PTR;           // Ptr on next PTR test in the list
    GexDbPlugin_TestInfoContainer   *m_pNextTest_MPR;           // Ptr on next MPR test in the list
    GexDbPlugin_TestInfoContainer   *m_pNextTest_FTR;           // Ptr on next FTR test in the list
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoList
// Holds a list of TestInfo containers
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_TestInfoList
{
public:
    GexDbPlugin_TestInfoList();
    ~GexDbPlugin_TestInfoList();

    // Main list
    unsigned int                  m_uiNbTests;
    GexDbPlugin_TestInfoContainer *m_pFirstTest;
    GexDbPlugin_TestInfoContainer *m_pCurrentTest;
    // PTR list
    unsigned int                  m_uiNbTests_PTR;
    GexDbPlugin_TestInfoContainer *m_pFirstTest_PTR;
    GexDbPlugin_TestInfoContainer *m_pCurrentTest_PTR;
    // MPR list
    unsigned int                  m_uiNbTests_MPR;
    GexDbPlugin_TestInfoContainer *m_pFirstTest_MPR;
    GexDbPlugin_TestInfoContainer *m_pCurrentTest_MPR;
    // FTR list
    unsigned int                  m_uiNbTests_FTR;
    GexDbPlugin_TestInfoContainer *m_pFirstTest_FTR;
    GexDbPlugin_TestInfoContainer *m_pCurrentTest_FTR;

    void                 ResetData();
    void                 ClearData(bool bDeleteTestInfo = true);
    void                 Insert(GexDbPlugin_TestInfo *pTestInfo);
    GexDbPlugin_TestInfo *FindTestByNb(unsigned int uiTestNb);
    GexDbPlugin_TestInfo *FindTestByID_PTR(unsigned int uiTestID);
    GexDbPlugin_TestInfo *FindTestByID_MPR(unsigned int uiTestID);
    GexDbPlugin_TestInfo *FindTestByID_FTR(unsigned int uiTestID);
    void                 GestTestListString(QString & strTestListString);
    void                 ResetDynamicTestInfo();
    void                 ResetTestInfo();
    QString              getTestIdList_PTR()   { return m_strTestIdList_PTR; }
    QString              getTestIdList_MPR()   { return m_strTestIdList_MPR; }
    QString              getTestIdList_FTR()   { return m_strTestIdList_FTR; }

private:
    void     UpdateList_PTR(GexDbPlugin_TestInfoContainer *pNewTest);
    void     UpdateList_MPR(GexDbPlugin_TestInfoContainer *pNewTest);
    void     UpdateList_FTR(GexDbPlugin_TestInfoContainer *pNewTest);
    QString  m_strTestIdList_PTR;           // List of PTR testID included, ie "1,2,10,20,21"
    QString  m_strTestIdList_MPR;           // List of MPR testID included, ie "1,2,10,20,21"
    QString  m_strTestIdList_FTR;           // List of FTR testID included, ie "1,2,10,20,21"
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_RunInfo
// Holds info on 1 run
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_RunInfo
{
public:
    GexDbPlugin_RunInfo()           { Reset(); }
    void Reset(void)
    {
        m_nX = -32768;
        m_nY = -32768;
        m_nRunID = -1;
        m_nSiteNb = -1;
        m_nSoftBin = -1;
        m_nHardBin = -1;
        m_bPartFailed = false;
        m_bPartExcluded = false;
        m_bWrittenToStdf = false;
        m_lnTestTime = 0;
        m_tDateTimeOfTest = 0;
        m_previous=0;
        m_next=0;
        m_iTestExecuted=0;
    }

    // Fields from Database
    int           m_nRunID;             // FT
    int           m_nX;                 // WT, ET
    int           m_nY;                 // WT, ET
    int           m_nSiteNb;
    int           m_nSoftBin;
    int           m_nHardBin;
    int           m_iTestExecuted;
    QString       m_strPartID;
    bool          m_bPartFailed;
    bool          m_bPartExcluded;      // true if the part must be ignore (SITE filter, HBin filter, PART filter)
    unsigned long m_lnTestTime;
    time_t        m_tDateTimeOfTest;
    QString       m_strPartTxt;

    GexDbPlugin_RunInfo* m_next;
    GexDbPlugin_RunInfo* m_previous;

    // Additional information
    bool           m_bWrittenToStdf;    // true if this run has been written to the STDF file, false else
};


/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_BinMap
// Holds a map of GexDbPlugin_BinInfo objects
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_BinMap: public QMap<int, GexDbPlugin_BinInfo>
{
};


/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_SiteInfo
// Holds part count info for 1 site
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_SiteInfo
{
public:
    GexDbPlugin_SiteInfo()
    {
        m_nPartCount = 0;
        m_nGoodCount = 0;
        m_nRetestCount = 0;
    }
    ~GexDbPlugin_SiteInfo()
    {
        m_mapSoftBinnings.clear();
        m_mapHardBinnings.clear();
    }

    int                m_nPartCount;        // Nb of parts tested
    int                m_nGoodCount;        // Nb of Good parts tested
    int                m_nRetestCount;      // Nb of parts retested
    GexDbPlugin_BinMap m_mapSoftBinnings;   // Binning map (1 per bin nb)
    GexDbPlugin_BinMap m_mapHardBinnings;   // Binning map (1 per bin nb)
};

/////////////////////////////////////////////////////////////////////////////////////////
// statistical monitoring convenience structures
/////////////////////////////////////////////////////////////////////////////////////////

struct StatMonAlarm;
struct StatMonDataPoint;
struct MonitoredItemDesc;

enum MonitoredItemRuleType
{
    singleItem,
    range,
    all,
    groupOfItems,
    mergeOfItems
};

struct MonitoredItemRule
{
    MonitoredItemRuleType ruleType;
    QStringList ruleItems;
    bool manualRule;

    MonitoredItemRule()
    {
        ruleType = all;
        manualRule = false;
    }
};

///////////////////////////////////////////////////////////
// GexDbPlugin_Base class: database plugin base class...
///////////////////////////////////////////////////////////
class GexDbPlugin_Base : public QObject
{
    friend class GexDbPlugin_Query;
    friend class GexDbPlugin_Connector;

    Q_OBJECT

public:

    // Constructor / Destructor
    GexDbPlugin_Base(const QString & strHostName,
                     const QString & strApplicationPath,
                     const QString & strUserProfile,
                     const QString & strLocalFolder,
                     const char *gexLabelFilterChoices[],
                     const bool bCustomerDebugMode,
                     CGexSkin * pGexSkin,
                     const GexScriptEngine* gse=NULL,
                     GexDbPlugin_Connector *pclDatabaseConnector=NULL);
    virtual ~GexDbPlugin_Base();
    // change the loglevel for ALL plugins !
    //static bool SetGlobalLogLevel(int l);
    static QString GetRuleNameFromRuleType(OutlierRule eOutlierRule);
    static OutlierRule GetRuleTypeFromRuleName(QString strRuleName);
    static OutlierRule GetOutlierAlgorithmType(const QString & codingName);

    enum StatsSource
    {
        eStatsFromSummaryOnly,
        eStatsFromSummaryThenSamples,
        eStatsFromSamplesOnly,
        eStatsFromSamplesThenSummary
    };

    enum AlarmCategories
    {
        eAlarmCat_Unknown,
        eAlarmCat_PatYieldLoss_Yield,
        eAlarmCat_PatYieldLoss_Parts,
        eAlarmCat_PatDistributionMismatch,
        eAlarmCat_Yield,
        eAlarmCat_Parametric_Value,
        eAlarmCat_Parametric_Cp,
        eAlarmCat_Parametric_Cpk,
        eAlarmCat_Parametric_Mean,
        eAlarmCat_Parametric_Range,
        eAlarmCat_Parametric_Sigma,
        eAlarmCat_Parametric_PassRate,
        eAlarmCat_SYA,
        eAlarmCat_SYA_SBL,
        eAlarmCat_SYA_SYL,
        eAlarmCat_BinsPercentPerSite
    };

    enum AlarmLevels
    {
        eAlarmLevel_Unknown,
        eAlarmLevel_Standard,
        eAlarmLevel_Critical
    };

    enum QuerySort
    {
        eQuerySort_None,
        eQuerySort_Asc,
        eQuerySort_Desc
    };

    enum QueryJoinType
    {
        eQueryJoin_Inner,
        eQueryJoin_LeftOuter
    };

    // !! ALL NEW eERROR must be created at the end of a 'sub-list'
    // to preserv the enumm number !!
    GDECLARE_ERROR_MAP(GexDbPlugin_Base)
    {
                // General
                eLibraryNotFound = 1000,     // Plugin library not found
                eUnresolvedFunctions,        // Unresolved functions in library
                eFunctionNotSupported,       // Function is not supported by the plugin
                eInit,                       // Error initializing plug-in library
                eMarkerNotFound,             // XML marker not found in settings file
                eReadSettings,               // Failed reading settings file
                eWriteSettings,              // Error writing settings file
                eLicenceExpired,             // File date out of Licence window!
                eMemoryAllocation,           // Memory allocation error

                // STDF file manipulation
                eStdf_Open = 1100,                  // Failed opening STDF file
                eStdf_Read,                  // Error reading STDF file
                eStdf_Corrupted,             // Corrupted STDF file
                eStdf_DtrCommand_BadSyntax,  // Found valid DTR command but with a wrong syntax
                eStdf_DtrCommand_BadUsage,   // Found valid DTR command but with a wrong usage
                eStdf_DtrSyntricity_BadSyntax,// Found valid DTR syntricity test condiction but with a wrong syntax

                // Data validation
                eValidation_DuplicateTest = 1200,   // Test with same test nb and test name appears with different type (PTR,MPR,FTR)
                eValidation_DuplicateTestNumber,    // Test with same test nb and diff test name
                eValidation_AlreadyInserted, // Data already inserted in the DB
                eValidation_MultipleLimits,  // Multiple limits per test/site
                eValidation_MissingRecords,  // Some mandatory records are missing in the STDF file (ie MIR, MRR)
                eValidation_EmptyField,      // Some mandatory fields are empty (ie LotID)
                eValidation_InvalidField,    // Some fields are invalid (ie PartID)
                eValidation_MultiRecords,    // STDF file has data for multiple records
                eValidation_DbKeyOverload,   // DbKeys has been overloaded with invalid value
                eValidation_CountMismatch,   // Count record
                eValidation_BinMismatch,     // Bin category doesn't match with PRR flag
                eValidation_NotSupported,    // Unsupported configuration
                eValidation_InconsistentTestNumber, //Inconsistent test number detected
                eValidation_InconsistentTestResult, // GCORE-2044
                eValidation_InconsistentTestName, //Inconsistent test name detected
                eValidation_InconsistentTestFlow, //Inconsistent test name detected
                eValidation_TestConditionMultipleSources, //Multiple sources for testconditions
                eValidation_InvalidDbTypeForDTRTestcond,  //Invalid DB type for test conditions in DTR

                // Database errors
                eDB_InvalidTestingStage = 1300,     // An invalid testing stage was specified
                eDB_InvalidTransaction,      // An invalid transaction for multi insertion
                eDB_VersionMismatch,         // GEXDB version older than the version supported by the plugin
                eDB_UnsupportedDriver,       // Specified SQL driver is not supported
                eDB_Connection,              // Error connecting to the DB
                eDB_InvalidConnector,        // Database connector is NULL
                eDB_OpenSqlLoaderFile,       // Error opening file to write SQL commands for the SQL loader
                eDB_PacketSizeOverflow,      // Overflow in packet size (SQL query bigger that max allowed size)
                eDB_Query,                   // Error executing SQL query
                eDB_SqlLoader,               // Error executing SQL loader
                eDB_NoResult,                // Query returned no result
                eDB_NoResult_0,              // Query returned no result (don't display query in error message)
                eDB_InsertionValidationProcedure,   // DB Validation Procedure returned status specifying file is not valid for insertion
                eDB_NoMappingTable,          // Missing DB mapping table
                eDB_MissingMapping,          // Missing DB mapping of some required Examinator fields
                eDB_MissingLink,             // Missing Link in DB mapping
                eDB_EnumTables,              // Error retrieving list of tables
                eDB_UpdateMapping,           // Error updating DB mapping
                eDB_CustomDbUpdate,          // Error during custom DB update
                eDB_CheckDbVersion,          // Error checking DB version
                eDB_CheckDbStatus,          // Error checking DB status
                eDB_NotUptoDate,             // DB is not up-to-date
                eDB_NotIncrementalUpdatesPending,// No incremental updates are pending
                eDB_Abort,                   // Operation aborted by user
                eDB_AliasOnMultiFieldFilter, // Aliases are not supported on multi-field filters
                eDB_ValuesMismatchMultiFieldFilter,// Values mismatch on multi-field filters (different nb of values as nb of fields)
                eDB_NotTimeFieldMapping,     // Field from DB mapping is not a time field.
                // Plug-in specific errors
                eDB_NonSequential_RunID,     // Medtronic plug-in: there is a non-sequential Ut_Id in the dataset (allow a difference of 10 between 2 consecutive Ut_Id's)
                eDB_ConsolidatedFinalTestExtractionError,// see galaxy plugin extract_consolidated.cpp
                eDB_SYALotCheckError,        // the SYACheckLot failed
                eDB_Consolidation,           // Error occures during the consolidation process
                eDB_Status,                  // Error from database status
                //eDB_SYAGetLimitSetFailed   // To Do : check SYA generic error ?
                eDB_ExtractionError,
                eDB_InsertionPreProcessingProcedure,// DB Pre-processing Procedure returned status specifying file is not valid for insertion
                eDB_InsertionPostProcessingProcedure,// DB Post-processing Procedure returned status specifying file is not valid for insertion
                eDB_CustomIncrementalProcedure,// DB Custom Incremental Procedure returned status specifying file is not valid for insertion
                eDB_SPMComputeLimitsError,
                eDB_SPMCheckLimitsError,

                // Weekly Yield Report
                eWyr_FileOpen = 1400,               // Failed opening WYR file
                eWyr_FileAlreadyInserted,    // WYR file already inserted
                eWyr_MissingSite,            // Couldn't find WYR format for current site
                eWyr_MissingFormat,          // Missing date format for WYR filed DATE_IN or DATE_OUT
                eWyr_IncorrectFormat,        // Incorrect format for WYR file and WYR table
                // Internal errors
                eInternal_MissingTest = 1500,       // Couldn't find specified test in the testlist created during Pass1
                eInternal_MissingTestPin,    // Couldn't find specified test with specified pin in the testlist created during Pass1
                eInternal_MissingRun        // Couldn't find specified run in run map (initialized in InitMaps)

    }
    GDECLARE_END_ERROR_MAP(GexDbPlugin_Base)

    // Member functions
public:

    void                 SetParentWidget(QWidget *parentWidget);
    virtual bool         Init();
    // emit signal to increment progress
    void                 IncrementProgress(int prog = 1);
    // emit signal to reset progress
    void                 ResetProgress(bool forceCompleted);
    // emit signal to set max progress
    void                 SetMaxProgress(int prog);
    // emit signal to set progress
    void                 SetProgress(int prog);
    // Function to call the qApplication->processEvents function (make sure the qApplication->processEvent function is not called too often)
    virtual void         ProcessEvents();

    // Static functions
    static void          CryptPassword(const QString & strPassword, QString & strCryptedPasswordHexa);           // Crypt string
    static void          DecryptPassword(const QString & strCryptedPasswordHexa, QString & strPassword);          // Decrypt string

    // Common functions
    virtual bool         ConnectToCorporateDb();                                       // Connect to corporate DB
    void                 GetLastError(QString &strError);                              // Returns details about last error
    virtual void         GetPluginName(QString & strPluginName) const = 0;             // Returns plugin name
    virtual void         GetTdrTypeName(QString & strTdrType){strTdrType = "";}        // Returns tdr type name
    virtual bool         IsCharacTdr(){return false;}                                  // Returns true if charac tdr
    virtual bool         IsManualProdTdr(){return false;}                              // Returns true if manual production tdr
    virtual bool         IsYmProdTdr(){return false;}                                  // Returns true if ym production tdr
    virtual bool         IsAdr(){return false;}                                        // Returns true if adr
    virtual bool         IsLocalAdr(){return false;}                                   // Returns true if local adr
    virtual unsigned int GetPluginBuild(void) = 0;                                     // Returns plugin build
    virtual void         GetSupportedTestingStages(QString & strSupportedTestingStages) = 0;
    virtual bool         SetTestingStage(const QString & strTestingStage) = 0;
    virtual int          GetTestingStageEnum(const QString & strTestingStage) = 0;
    virtual bool         IsInsertionSupported() = 0;                                   // Returns true if insertion supported, false else
    virtual bool         IsUpdateSupported() = 0;                                      // Returns true if DB update supported, false else
    virtual bool         IsTestingStagesSupported() = 0;                               // Returns true if testing stages are supported, false else
    virtual bool         IsParameterSelectionSupported() = 0;                          // Returns true if parameter selection supported (for filtering), false else
    virtual bool         IsEnterpriseReportsSupported() = 0;                           // Returns true if Enterprise Reports supported, false else
    virtual bool         IsReportsCenterSupported() = 0;                               // Returns true if Reports Center supported, false else
    virtual bool         IsTestingStage_FinalTest(const QString & strTestingStage) = 0;// Returns true if the specified testing stage is Final Test
    virtual bool         IsTestingStage_Foundry(const QString & strTestingStage) = 0;  // Returns true if the specified testing stage is Foundry (E-Test)
    virtual bool         GetTestingStageName_Foundry(QString & strTestingStage) = 0;   // Returns the name of Foundry testing stage (E-Test)
    virtual void         GetWarnings(QStringList & strlWarnings);                      // Retrieve list of insertion warnings
    virtual void         GetLabelFilterChoices(const QString& /*strDataTypeQuery*/,QStringList& strlLabelFilterChoices) { strlLabelFilterChoices.clear(); }
    virtual void         GetConsolidatedLabelFilterChoices(const QString& /*strDataTypeQuery*/,QStringList& strlLabelFilterChoices) { strlLabelFilterChoices.clear(); }
    virtual void         GetLabelFilterChoicesWaferLevel(const QString& /*strDataTypeQuery*/,QStringList& strlLabelFilterChoices) { strlLabelFilterChoices.clear(); }
    virtual void         GetLabelFilterChoicesLotLevel(const QString& /*strDataTypeQuery*/,QStringList& strlLabelFilterChoices) { strlLabelFilterChoices.clear(); }
    virtual bool         LoadMetaData() = 0;

    // Return supported binning types.
    // ConsolidatedType: "Y" or "N"
    virtual bool GetSupportedBinTypes(const QString& /*TestingStage*/,
                                      QStringList& strlBinTypes,
                                      const QString& /*ConsolidatedType="Y"*/) { strlBinTypes.clear(); return true; }

    // Retrieve list of available fields for the given testing stage. output will contains the matching fields.
    virtual void GetRdbFieldsList(const QString& /*TestingStage*/,
                                  QStringList& output,
                                  const QString& /*In_GUI="Y"*/,            // Y, N or *
                                  const QString& /*BinType="N"*/,           // BinType can be H, S or N (or even *)
                                  const QString& /*Custom="*"*/,            // Custom can be Y, N or *
                                  const QString& /*TimeType="*"*/,          // TimeType can be Y, N or *
                                  const QString& /*ConsolidatedType="Y"*/,  // ConsolidatedType can be Y, N or *
                                  const QString& /*Facts="N"*/,             // Facts can be Y, N, or *
                                  bool  /*OnlyStaticMetaData=false*/
                                  ) { output.clear(); }
    virtual bool GetRdbFieldProperties(const QString& /*TestingStage*/, const QString& /*MetaDataName*/, QMap<QString,QString>& /*Properties*/){return false;}
    // Get consolidated field name corresponding to specified decorated field name
    virtual bool GetConsolidatedFieldName(const QString& /*strTestingStage*/,
                                          const QString& /*strDecoratedField*/,
                                          QString& /*strConsolidatedField*/,
                                          bool* /*pbIsNumeric = NULL*/,
                                          bool* /*pbIsBinning = NULL*/,
                                          bool* /*pbIsTime = NULL*/) { return false; }
    virtual void GetWaferIdFieldName(QString & strFieldName) { strFieldName=""; }
    virtual void GetLotIdFieldName(QString & strFieldName) { strFieldName=""; }
    void         SetAdminLogin(bool bAdminLogin);                                                                                                                                    // Set database login type (std user/admin user)
    // Dislay error message (QMessagBox)
    void                                                       DisplayErrorMessage();
    // Returns a pointer on an options GUI, if the plugin supports custom options
    virtual QWidget*  GetRdbOptionsWidget() { return NULL; }
    // Returns a pointer on a Widget if the plugin supports this kind of feature, else retun NULL
    virtual QWidget*  GetConsolidationWidget() { return NULL; }

    // Returns an options string, if the plugin supports custom options
    virtual bool GetRdbOptionsString(QString& /*strRdBOptionsString*/) { return false; }
    // Sets the plug-in options GUI according to options string passed as argument
    virtual bool SetRdbOptionsString(const QString& /*strRdBOptionsString*/) { return false; }

    // Database administration functions
    bool          IsCustomerDebugModeActivated() { return m_bCustomerDebugMode; }
    bool          IsAutomaticStartup() { return m_bAutomaticStartup; }
    bool          SetAutomaticStartup(bool bValue) {m_bAutomaticStartup=bValue;  return true; }

    virtual void          SetTdrLinkName(const QString& /*link*/) {return;}
    virtual QString       GetTdrLinkName() {return QString();}
    virtual void          SetAdrLinkName(const QString& /*link*/) {return;}
    virtual QString       GetAdrLinkName() {return QString();}
    virtual bool          MustHaveAdrLink() { return false;}

    virtual bool  IsDbUpToDate(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    virtual bool  IsDbUpToDateForInsertion(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    virtual bool  IsDbUpToDateForExtraction(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    // Update the DB following the 'command' string if any. If empty, it simply try to upgrade to the next version.
    virtual bool  UpdateDb(QString command="");
    //
    virtual bool  UpdateConsolidationProcess(int eTestingStage=0); // eTestingStage: 0 : to update all testingstage else update only one testing stage

    // Incremental Update methods
    // flag specified splitlots ready for incremental update
    virtual bool FlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey);
    virtual bool UnFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey);
    virtual bool SwitchFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalOldKey, const QString &incrementalNewKey);

    // Run a specific incremental update
    virtual bool  IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary);
    // Get/Update the total count of remaining splitlots for incremental update
    virtual bool  GetIncrementalUpdatesCount(bool checkDatabase,int &incrementalSplitlots);
    // Get the next incremental update for Schedule for ALL according to the Frequency
    virtual bool  GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & incrementalUpdatesList);
    // Get the next incremental update for Schedule for ALL or specific for manual update
    virtual bool  GetFirstIncrementalUpdatesList(QString incrementalName, QMap< QString,QMap< QString,QStringList > > & incrementalUpdatesList);
    // Incremental updates settings
    virtual bool  GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    virtual bool  SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    virtual bool  IsIncrementalUpdatesSettingsValidValue(QString &name, QString &value);
    virtual bool  IsAutomaticIncrementalUpdatesEnabled() { return false; }

    virtual int IsConsolidationInProgress(
            QString testingStage,
            QString lot,
            QString sublots,
            QString wafers,
            QString consoType,
            QString testFlow,
            QString consoLevel,
            QString testInsertion) = 0;

    // Reset the database (on SQLite, overwrite DB file gexdb.sqlite)
    virtual bool  Reset(const QString &strDB_Folder) { m_strDBFolder=strDB_Folder; return true; }
    virtual bool  PurgeSplitlots(QStringList & strlSplitlots, QString & strTestingStage, QString & strCaseTitle, QString *pstrLog=NULL);           // Purge selected splitlots
    virtual bool  ConsolidateWafer(QString & strLotID, QString & strWaferID, QString & strCaseTitle, QString *pstrLog=NULL);           // Consolidate specified wafer
    virtual bool  ConsolidateWafers(QString & strLotID, QString & strCaseTitle, QString *pstrLog=NULL);           // Consolidate specified wafer
    virtual bool  ConsolidateLot(QString & strLotID, bool bConsolidateOnlySBinTable=false, bool bCallConsolidationFunction=true);
    virtual bool  purgeDataBase(GexDbPlugin_Filter &cFilters, GexDbPlugin_SplitlotList &);
    virtual bool  exportCSVCondition(const QString &strCSVFileName, const QMap<QString, QString>& oConditions, GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList, QProgressDialog *poProgress=0);

    virtual bool  GetStorageEngineName(QString & strStorageEngine, QString & strStorageFormat);           // Get name of storage engine
    // Get the total size of the plugin DB
    virtual bool  GetTotalSize(int &size);

    // Database restriction security
    virtual bool  GetSecuredMode(QString &strSecuredMode);
    virtual bool  UpdateSecuredMode(QString strSecuredMode);

    // Get the bin counts per site for the given splitlot.
    virtual QString   GetSplitlotBinCountsPerSite(
            const QString &/*TestingStage*/,
            long /*lSplitlotID*/,
            QMap<int, GexDbPlugin_BinList> &/*mapBinsPerSite*/) { return QString("error"); }

    // Get the (raw) bin counts per group by for the given filter.
    virtual QString      GetBinCounts(GexDbPlugin_Filter & /*cFilters*/,
                                      QStringList /*lGroupBy*/,
                                      bool /*bSoftBin*/,
                                      QMap<QString, GexDbPlugin_BinList> &/*mapBinsPerKey*/) { return QString("error");}

    // Retrieve an option value simply from his name
    virtual bool  GetGlobalOptionName(int nOptionNb, QString &strValue);
    virtual bool  GetGlobalOptionTypeValue(int nOptionNb, QString &strValue);
    virtual bool  GetGlobalOptionValue(QString strOptionName, QString &strValue);
    virtual bool  GetGlobalOptionValue(int nOptionNb, QString &strValue);
    virtual bool  GetGlobalOptionValue(int nOptionNb, QString &strValue, bool &bIsDefined);
    virtual bool  GetGlobalOptionDefaultValue(int nOptionNb, QString &strValue);
    virtual bool  GetGlobalOptionDescription(int nOptionNb, QString &strValue);
    virtual bool  GetGlobalOptionReadOnly(int nOptionNb, bool &bIsReadOnly);
    virtual bool  SetGlobalOptionValue(QString strOptionName, QString &strValue);
    virtual bool  IsGlobalOptionValidValue(int nOptionNb, QString &strValue);

    // Plugin configuration
    bool          ConfigWizard(QMap<QString,QString> info, QMap<QString,QString> guiOptions);
    virtual bool  ConfigWizard() = 0;
    virtual bool  LoadSettingsFromDom(const QDomElement &node);// Load settings from DomElement and init the calls variables
    virtual bool  LoadSettings(QFile *pSettingsFile);       // Load settings from file and init the calls variables
    virtual QDomElement GetSettingsDom(QDomDocument &doc);
    virtual bool  WriteSettings(QTextStream *phFile);       // Write settings to file using informations loaded in the class variables
    virtual bool  CreateConnector();                        // Create a new connector for YieldManAdminDb

    // Data insertion
    void          SetInsertionValidationOptionFlag(unsigned int uiInsertionValidationOptionFlag){ m_uiInsertionValidationOptionFlag = uiInsertionValidationOptionFlag;}
    void          SetInsertionValidationFailOnFlag(unsigned int uiInsertionValidationFailOnFlag){ m_uiInsertionValidationFailOnFlag = uiInsertionValidationFailOnFlag;}
    unsigned int  GetInsertionValidationOptionFlag() {return m_uiInsertionValidationOptionFlag;}
    unsigned int  GetInsertionValidationFailOnFlag() {return m_uiInsertionValidationFailOnFlag;}
    /*!
     * \fn InsertDataFile
     */
    virtual bool InsertDataFile(struct GsData* lGsData,
                                int lSqliteSplitlotId,
                                const QString& strDataFileName,
                                 GS::QtLib::DatakeysEngine& dbKeysEngine,
                                bool* pbDelayInsertion,
                                long* plSplitlotID,
                                int* pnTestingStage);
    virtual bool  InsertWyrDataFile(const QString & strDataFileName, const QString & strSiteName, const QString & strTestingStage, unsigned int uiWeekNb, unsigned int uiYear, bool *pbDelayInsertion);
    // insert alarm for given splitlot and TS
    virtual bool  InsertAlarm(long lSplitlotID, int nTestingStage, GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);
    virtual bool  InsertAlarm_Wafer(long lSplitlotID, GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);

    // Generic data extraction functions
    //! \brief Execute Query and return results
    virtual bool  QuerySQL(QString & strQuery, QList<QStringList> & listResults);
    //! \brief Return all splitlots (given filters on several fields)
    //! \brief This function, if supported, must be overloaded in the derived class
    virtual bool  QuerySplitlots(GexDbPlugin_Filter & cFilters, GexDbPlugin_SplitlotList & clSplitlotList, bool bPurge=false);                       // Return all splitlots (given filters on several fields)
    //! \brief Return all binnings (given filters on other fields)
    virtual bool  QueryBinlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin=false,bool bClearQueryFirst=true,bool bIncludeBinName=false, bool bProdDataOnly=false);
    //! \brief Return all products (use only date in the filter)
    virtual bool  QueryProductList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,QString strProductName="");
    //! \brief Return all products for genealogy reports, with data for at least 2 testing stages (use only date in the filter)
    virtual bool  QueryProductList_Genealogy(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bAllTestingStages);
    //! \brief Return all test conditions corresponding to the splitlots (according to given filters)
    virtual bool  QueryTestConditionsList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues);

    //! \brief Convert REGEXP wildcards to LIKE Sql syntax
    //! \param string containing REGEXP wildcard characters
    //! \return LIKE Sql syntax if aForLIKE
    //! \return = Sql syntax if !aForLIKE
    static QString Query_BuildSqlStringExpression( const QString &aValue, bool aForSqlLIKE=false );

    // Data extraction for GEX standard reports
    //! \brief Return all valid values for a field (given filters on other fields)
    virtual bool  QueryField(GexDbPlugin_Filter & cFilters,
                             QStringList & cMatchingValues,
                             bool bSoftBin=false,
                             bool bDistinct=true,
                             QuerySort eQuerySort=GexDbPlugin_Base::eQuerySort_Asc);
    //! \brief Return all tests (given filters on other fields)
    virtual bool  QueryTestlist(GexDbPlugin_Filter & cFilters,
                                QStringList & cMatchingValues,
                                bool bParametricOnly);

    //! \brief Return all valid Data files (given filters on several fields)
    virtual bool  QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist,
                                 tdGexDbPluginDataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath,
                                 const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation,
                                 GexDbPlugin_Base::StatsSource eStatsSource) = 0;

    // Data extraction for GEX enterprise reports
    virtual bool  GetDataForProd_UPH(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList);                      // Compute data for Production - UPH graph
    virtual bool  GetDataForProd_Yield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList, int nBinning, bool bSoftBin);// Compute data for Production - Yield graph
    virtual bool  GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList);        // Compute data for Production - Consolidated Yield graph
    virtual bool  GetDataForWyr_Standard(GexDbPlugin_Filter & cFilters, GexDbPlugin_WyrData & cWyrData);                           // Compute data for WYR - standard report
    virtual bool  ER_Prod_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);                          // Compute data for Enterprise Report graphs (Yield, UPH)
    virtual bool  ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);        // Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
    virtual bool  ER_Genealogy_YieldVsParameter_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);    // Compute data for Enterprise Report graphs (Genealogy - Yield vs Parameter)
    virtual bool  ER_Prod_GetBinnings(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, GexDbPlugin_BinList & clBinList);                      // Get bin counts for Enterprise Report graphs (Yield, UPH)

    // Data extraction for GEX Advanced Enterprise Reports
    virtual bool  AER_GetDataset(const GexDbPluginERDatasetSettings& datasetSettings, GexDbPluginERDataset& datasetResult);        // Compute data for Enterprise Report graphs (Yield, UPH)

    virtual bool    SPM_GetProductList(QString testingStage,
                                       QString productRegexp,
                                       QStringList & cMatchingValues) = 0;
    virtual bool    SPM_GetFlowList(QString testingStage,
                                    QString productRegexp,
                                    QStringList & cMatchingValues) = 0;
    virtual bool    SPM_GetInsertionList(QString testingStage,
                                         QString productRegexp,
                                         QString flowRegexp,
                                         QStringList & cMatchingValues) = 0;
    virtual bool    SPM_GetItemsList(QString testingStage,
                                     QString productRegexp,
                                     QString flowRegexp,
                                     QString insertionRegexp,
                                     QString testType,
                                     QStringList & cMatchingValues) = 0;
    // Fetches a list of wafer_id from the TDR matching the provided criterion
    virtual bool SPM_FetchWaferKeysFromFilters(QString testingStage, //in
                                               QString productRegexp, //in
                                               QString lotId, //in
                                               QString sublotId, //in
                                               QString waferId, //in
                                               const QMap<QString, QString> &filtersMetaData, //in
                                               QStringList& waferKeyList) = 0; //out

    virtual bool    SPM_GetConditionsFromFilters(QString testingStage, //in
                                                 const QMap<QString,QString>& filtersMetaData, //in
                                                 QMap<QString,QStringList>& filtersConditions) = 0; //out

    virtual bool    SPM_FetchDataPointsForComputing(QString testingStage, //in
                                                    QString productRegexp, //in
                                                    QString monitoredItemType, //in
                                                    const QList<MonitoredItemRule>& monitoredItemRules, //in
                                                    MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                    QString testFlow, //in
                                                    QString consolidationType, //in
                                                    QString consolidationLevel, //in
                                                    QString testInsertion, //in
                                                    const QStringList& statsToMonitor, //in
                                                    QString siteMergeMode, //in
                                                    bool useGrossDie, //in
                                                    const QMap<QString, QStringList>& filtersConditions, //in
                                                    QDateTime dateFrom, //in
                                                    QDateTime dateTo, //in
                                                    QStringList& productsMatched, //out
                                                    int& numLotsMatched, //out
                                                    int& numDataPointsMatched, //out
                                                    QSet<int>& siteList, //out
                                                    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& testToSiteToStatToValues) = 0; //out

    // Fetches the SPM datapoints from the ADR for a check on trigger
    virtual bool    SPM_FetchDataPointsForCheckOnTrigger(QString testingStage, //in
                                                         QString productRegexp, //in
                                                         QString lotId, //in
                                                         QString sublotId, //in
                                                         QString waferId, //in
                                                         const QList<MonitoredItemDesc>& testList, //in
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int>& siteList, //in
                                                         const QList<QString> &statsList, //in
                                                         bool useGrossDie, //in
                                                         const QDateTime* dateFrom, //in
                                                         const QDateTime* dateTo, //in
                                                         const QMap<QString, QStringList>& filtersConditions, //in
                                                         QString& productList, //out
                                                         QString& lotList, //out
                                                         QString& sublotList, //out
                                                         QString& waferList, //out
                                                         int& numParts, //out
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint) = 0; //out

    // Fetches the SPM datapoints from the TDR for a check on insertion
    virtual bool    SPM_FetchDataPointsForCheckOnInsertion(QString testingStage, //in
                                                           int splitlotId, //in
                                                           const QMap<QString,QString> &filtersMetaData, //in
                                                           const QList<MonitoredItemDesc> &testList, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                           const QList<int> &siteList, //in
                                                           const QList<QString> &statsList, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint) = 0; //out

    // Fetches the SYA datapoints from the TDR for a compute
    virtual bool    SYA_FetchDataPointsForComputing(QString testingStage, //in
                                                    QString productRegexp, //in
                                                    const QMap<QString,QString> &filtersMetaData, //in
                                                    QString monitoredItemType, //in
                                                    const QList<MonitoredItemRule> &monitoredItemRules, //in
                                                    const QStringList& binsToExclude, //in
                                                    MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                    QString testFlow, //in
                                                    QString consolidationType, //in
                                                    QString consolidationLevel, //in
                                                    QString testInsertion, //in
                                                    const QStringList &statsToMonitor, //in
                                                    QString siteMergeMode, //in
                                                    bool useGrossDie, //in
                                                    QDateTime computeFrom, //in
                                                    QDateTime computeTo, //in
                                                    QStringList& productsMatched, //out
                                                    int& numLotsMatched, //out
                                                    int& numDataPointsMatched, //out
                                                    QSet<int>& siteList, //out
                                                    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues) = 0; //out

    // Fetches the SYA datapoints from the TDR for a check
    virtual bool    SYA_FetchDataPointsForCheckOnTrigger(QString testingStage, //in
                                                         QString productRegexp, //in
                                                         QString lotId, //in
                                                         QString sublotId, //in
                                                         QString waferId, //in
                                                         const QMap<QString,QString> &filtersMetaData, //in
                                                         const QList<MonitoredItemDesc> &binList, //in
                                                         const QStringList& binsToExclude, //in
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int> &siteList, //in
                                                         const QList<QString> &statsList, //in
                                                         bool useGrossDie, //in
                                                         const QDateTime* dateFrom, //in
                                                         const QDateTime* dateTo, //in
                                                         QString& productList, //out
                                                         QString& lotList, //out
                                                         QString& sublotList, //out
                                                         QString& waferList, //out
                                                         int& numParts, //out
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint) = 0; //out

    // Fetches the SYA datapoints from the TDR for a check
    virtual bool    SYA_FetchDataPointsForCheckOnInsertion(QString testingStage, //in
                                                           int splitlotId, //in
                                                           const QMap<QString,QString> &filtersMetaData, //in
                                                           const QList<MonitoredItemDesc> &binList, //in
                                                           const QStringList& binsToExclude, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                           const QList<int> &siteList, //in
                                                           const QList<QString> &statsList, //in
                                                           bool useGrossDie, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint) = 0; //out

    // Data transfer
    // Transfer remote data files to local FS
    virtual bool  TransferDataFiles(tdGexDbPluginDataFileList &cMatchingFiles, const QString & strLocalDir);

    // Product Info
    bool           IsTdrAllowed();
    bool           IsMonitoringMode();

    // Member variables
public:

    // Database Connector
    GexDbPlugin_Connector *m_pclDatabaseConnector;

    int                   m_nConnectionTimeOut;    // DB connection time out in secondes (0 is desactived)
    QString               m_strDBFolder;           // DB local folder (in /Databases/)
    const GexDbPlugin_Mapping_FieldMap* GetMappingFieldsMap() { return m_pmapFields_GexToRemote; }

public slots:
    //void CurrentActivity(const QString&);
    // Try stop the current process whatever it is : insertion, extraction,...
    // could be inefficient if the code does not listen to this request. Then the only solution is to wait.
    virtual void StopProcess() {  }

signals:
    //  emitted when the plugin would like to update a gui log with rich message
    void sLogRichMessage(const QString & strMessage, bool bPlainText);
    //  emitted when the plugin would like to update a gui log
    void sLogMessage(const QString & strMessage);
    //  emitted when the plugin would like to update the progress of a progress bar
    void sUpdateProgress(int prog);
    //  emitted when the plugin would like to reset a progress bar
    void sResetProgress(bool forceCompleted);
    //  emitted when the plugin would like to define max of a progress bar
    void sMaxProgress(int max);
    //  emitted when the plugin is busy
    void sBusy(bool isBusy);

    // Implementation
protected:
    // Holds product info attributes
    QMap<QString, QString> mGexInfoMap;
    QMap<QString, QString> mWizardGuiOptionsMap;
    GS::DbPluginBase::DbArchitecture* mDbArchitecture;
    GS::DbPluginBase::QueryEngine* mQueryEngine;

    // Global object for global extraction performance measurement
    GexDbPlugin_ExtractionPerf   m_clExtractionPerf;

    // Database mapping
    GexDbPlugin_Mapping_FieldMap *m_pmapFields_GexToRemote; // ptr on Fields mapping: GEX <-> Custom DB
    GexDbPlugin_Mapping_LinkMap  *m_pmapLinks_Remote;       // ptr on Table links mapping

    // Variables
    const GexScriptEngine* mGexScriptEngine;                    // GexScriptEngine given at constructor. Can be NULL !
    CGexSkin *    m_pGexSkin;                             // Examinator skin to be used
    QString       m_strHostName;                          // Host name
    bool          m_bAutomaticStartup;                    // False is the Db has to start manually

    QWidget       *mParentWidget;                         // Ptr to Main window
    QString       m_strApplicationPath;                   // Application path
    QString       m_strUserProfile;                       // User profile directory
    QString       m_strLocalFolder;                       // Local folder
    QString       m_strPluginName;                        // Name of the plugin (used to differentiate different database connections in addDatabase() function)
    bool          m_bPrivateConnector;                    // Set to true if Connector object allocated in this object
    QStringList   m_strlGexFields;                        // List of Examinator fields (including some plugin specific ones (hostname...))
    const char    **m_gexLabelFilterChoices;              // Original list of Examinator fields
    unsigned int  m_uiTotalRuns;                          // Total nb of runs retrieved
    unsigned int  m_uiTotalTestResults;                   // Total nb of tests results retrieved
    QStringList   m_strlQuery_LinkConditions;             // Contains list of link conditions in form "Field1|Field2"
    QStringList   m_strlQuery_ValueConditions;            // Contains list of value conditions in form "Field1|Value1[|Value2[|Value3...]]"
    QStringList   m_strlQuery_Fields;                     // Contains list of fields to query
    QStringList   m_strlQuery_OrderFields;                // Fields used to order query results
    QStringList   m_strlQuery_GroupFields;                // Fields used to group query results
    QMap<QString,QString> m_mapQuery_TableAliases;        // Contains list of table aliases
    bool          m_bProfilerON;                          // Set to true if profiler should be used, false else (default)
    unsigned int  m_uiInsertionValidationOptionFlag;      // Specifies the verifications to perform during the insertion validation function
    unsigned int  m_uiInsertionValidationFailOnFlag;      // and stop process if FailOn
    bool          m_bCustomerDebugMode;                   // Set to true if application running in customer debug mode
    StatsSource   m_eStatsSource;                         // Specifies how the statistics should be extracted (samples only, summary only, samples then summary, summary then samples)
    int           mProgress;                              // Used for progress bar
    QTime         mLastProcessEventsTime;              // Time used to store last time ProcessEvents was executed

    GS::DbPluginBase::AbstractQueryProgress *mQueryProgress;  ///< SQL query progress viewer
    QString       m_strQuery;                             // Last query executed
    QTime         m_clQueryTimer;                         // For query execution time measurement (started at execution of each query)
    unsigned int  m_uiTimer_DbQuery;                      // Holds DB execution time for current query
    unsigned int  m_uiTimer_DbIteration;                  // Holds DB iteration time for current query
    unsigned int  m_uiTimer_DbQuery_Cumul_Total;          // Holds DB execution time cumul
    unsigned int  m_uiTimer_DbIteration_Cumul_Total;      // Holds DB iteration time cumul
    unsigned int  m_uiTimer_DbQuery_Cumul_Partial;        // Holds DB execution time partial cumul
    unsigned int  m_uiTimer_DbIteration_Cumul_Partial;    // Holds DB iteration time partial cumul


    // Functions
    virtual bool  LoadDatabaseArchitecture() = 0;
    virtual bool  LoadDatabaseArchitecture(QDomDocument &document);
    bool          LoadQueryEngine();

    //virtual bool UnconnectToCorporateDb();                                 // UnConnect to corporate DB
    virtual bool  Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters);// Add condition if user selected a time period
    // Compute query date constraints. Could return "error ..." on error
    QString       Query_ComputeDateConstraints(GexDbPlugin_Filter &cFilters);
    bool          Query_Empty();                                             // Empty all lists used to build a query
    virtual int   Query_FieldsToQuery(const QString & strQueryField, QStringList & strlFields);                                                                  // return nb of fields to query
    virtual bool  Query_AddField_MultiField(QStringList & strlFields, QStringList & strlDbField, QStringList & strlDbTable, bool bUseLinkConditions=true);                                 // Add a select field to the query (multi-field)
    // Add a select field to the query
    virtual bool  Query_AddField(const QString & strQueryField,
                                 QString & strDbField, QString & strDbTable,
                                 bool bUseLinkConditions=true, bool bConsolidated =false);
    virtual bool  Query_AddFilters(GexDbPlugin_Filter &cFilters);                                                                                                                                                          // Add filters to current query
    bool          Query_AddLinkCondition(const QString & strLinkName);
    // Query: add condition string for a specified filter/value
    virtual bool  Query_AddValueCondition(const QString & strQueryFilter,
                                          const QString & strQueryValue,
                                          bool bExpressionRange=false,
                                          bool bUseLinkConditions=true);
    virtual bool   Query_AddValueCondition_MultiField(QStringList & strlFields, const QString & strQueryValue, bool bUseLinkConditions=true);
    bool           Query_NormalizeToken(const QString & strQueryToken, QString & strDbField, QString & strDbTable);           // Extract normalized field name and table name from the DB field specifier retrieved from the mapping
    void           Query_NormalizeTableName(QString & strDbTable);       // Normalize table name
    void           Query_NormalizeAlias(QString & strQueryAlias);        // Normalize alias used in SQL "AS" syntax (replace spaces, commas...)
    void           Query_BuildSqlString_NumericFilter(const QString & strDbField, const QString & strFilter, QString & strCondition, bool bNegation=false);
    void           Query_BuildSqlString_ExpressionRange(const QString & strDbField, const QString & strFilter, QString & strCondition);
    void           Query_BuildSqlString_StringFilter(const QString & strDbField, const QString & strFilter, QString & strCondition, bool bNegation=false);
    void           Query_BuildSqlString_FieldExpression_Numeric(const QString & strFieldExpression, const QString & strFilter, QString & strCondition);
    // Build SQL string using lists of fields, tables, conditions
    bool           Query_BuildSqlString(QString & strSqlString,
                                        bool bDistinct=true,
                                        QString strOptimizeOnTable="",
                                        GS::DbPluginBase::QueryEngine::BuildingRule buildingRule=GS::DbPluginBase::QueryEngine::UseFunctionalDep);
    bool           Query_BuildSqlString_AddJoin(QString & strJoins, QStringList & strlLinkConditions, QStringList & strlValueConditions, bool bIncludeFiltersInJoinCondition=true, GexDbPlugin_Base::QueryJoinType eQueryJoinType=GexDbPlugin_Base::eQueryJoin_Inner);
    bool           Query_BuildSqlString_AddJoinConditions(QString & strJoins, QString & strTable, QStringList & strlValueConditions);
    bool           Query_BuildSqlString_UsingJoins(QString & strSelect, QString & strFrom, QString & strWhere, QString & strGroup, QString & strOrder, bool bDistinct=true, QString strOptimizeOnTable="", bool bIncludeFiltersInJoinCondition=false, GexDbPlugin_Base::QueryJoinType eQueryJoinType=GexDbPlugin_Base::eQueryJoin_Inner);           // Build SQL string using lists of fields, tables, conditions
    bool           Query_BuildSqlString_UsingJoins(QString & strSqlString, bool bDistinct=true, QString strOptimizeOnTable="", bool bIncludeFiltersInJoinCondition=false, GexDbPlugin_Base::QueryJoinType eQueryJoinType=GexDbPlugin_Base::eQueryJoin_Inner);           // Build SQL string using lists of fields, tables, conditions
    bool           ConstructFieldQuery(GexDbPlugin_Filter &cFilters, QString & strQuery,bool bDistinct, QuerySort eQuerySort);                                                                             // Construct query to retrieve all values for a field, with specified filters
    virtual bool   ConstructTestlistQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, QString & strTestNumField, QString & strTestNameField, QString strTestTypeField = "");                                                                  // Construct query string to retrieve all tests matching filters
    bool           ConstructFlowListQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, QString & strFlowField);                                                                  // Construct query string to retrieve all retest phases matching filters
    bool           ConstructInsertionListQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, QString & strInsertionField);                                                                  // Construct query string to retrieve all retest phases matching filters
    // Construct query string to retrieve all binnings matching filters
    virtual bool   ConstructBinlistQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strBinType, bool bSoftBin=false, bool bClearQueryFirst=true,bool bIncludeBinName=false, bool bProdDataOnly=false);
    //! \brief For debug purpose, write message to debug trace file
    void             WriteDebugMessageFile(const QString & strMessage, bool bUpdateGlobalTrace=false) const;
    bool           Query_Execute(QSqlQuery & clQuery, const QString & strQuery);                                                                                                              // Execute query (write and cumul execution time if in debug mode)
    bool           Query_Next(QSqlQuery & clQuery);                                                                                                                                                                                           // Iterate to next query result item (cumul execution time if in debug mode)
    void           WritePartialPerformance(const char* szFunction);

    bool           IsReservedWord(QString strWordToTest);                // return true, if the word is reserved in connected db or if no db connected.
};

#endif // _GEXDB_PLUGIN_BASE_HEADER_
