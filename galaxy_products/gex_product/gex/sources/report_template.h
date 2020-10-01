/******************************************************************************!
 * \file report_template.h
 ******************************************************************************/
#ifndef REPORT_TEMPLATE_H
#define REPORT_TEMPLATE_H

#include <QList>
#include <QString>
#include <QColor>
#include <QDate>
#include "gexdb_plugin_base.h"

namespace GS
{
    namespace QtLib
    {
        class Range;
    }
namespace Gex
{

// Widgets stacked in the edit box
#define GEX_CRPT_WIDGET_WELCOME     0
#define GEX_CRPT_WIDGET_AGGREGATE   1
#define GEX_CRPT_WIDGET_WAFMAP      2
#define GEX_CRPT_WIDGET_BINNING     3
#define GEX_CRPT_WIDGET_PARETO      4
#define GEX_CRPT_WIDGET_PEARSON     5
#define GEX_CRPT_WIDGET_CORR_GB     6
#define GEX_CRPT_WIDGET_PRODUCTION  7
#define GEX_CRPT_WIDGET_FILE_AUDIT  8
#define GEX_CRPT_WIDGET_GLOBALINFO  9
#define GEX_CRPT_WIDGET_DATALOG     10
// Always keep it last ID in list
// (Enterprise Report section: mySQL/Oracle query)
#define GEX_CRPT_WIDGET_ER          11

// Selecting Type of test
#define GEX_CRPT_TEST_TYPE_ALL        0  // All tests
#define GEX_CRPT_TEST_TYPE_FAIL       1  // Failing tests only
#define GEX_CRPT_TEST_TYPE_OUTLIERS   2  // Tests with outliers
#define GEX_CRPT_TEST_TYPE_LIST       3  // Test list
#define GEX_CRPT_TEST_TYPE_BADCP      4  // Test with Cp less than ...
#define GEX_CRPT_TEST_TYPE_BADCPK     5  // Test with Cpk less than ...
#define GEX_CRPT_TEST_TYPE_TOP_N_FAIL 6  // Top N Failing tests

// Select Marker size & color
#define GEX_CRPT_MARKER_STYLE_MEAN    0
#define GEX_CRPT_MARKER_STYLE_LIMITS  1
#define GEX_CRPT_MARKER_STYLE_MIN     2
#define GEX_CRPT_MARKER_STYLE_MAX     3
#define GEX_CRPT_MARKER_STYLE_MEDIAN  4
#define GEX_CRPT_MARKER_STYLE_LOT     5
#define GEX_CRPT_MARKER_STYLE_SUBLOT  6
#define GEX_CRPT_MARKER_STYLE_GROUP   7
#define GEX_CRPT_MARKER_STYLE_2SIGMA  8
#define GEX_CRPT_MARKER_STYLE_3SIGMA  9
#define GEX_CRPT_MARKER_STYLE_6SIGMA  10
#define GEX_CRPT_MARKER_STYLE_12SIGMA 11
#define GEX_CRPT_MARKER_TOTALSIZE     12  // Total number of markers available

// Define Charting mode of aggregate chart
// One test per chart (all layers on chart)
#define GEX_CRPT_CHART_MODE_SINGLE_TEST    0
// One test per chart (one layer on chart) =>
// generate as many charts per test as layers
#define GEX_CRPT_CHART_MODE_SINGLE_LAYER   1
// Overlay tests & layers on same chart
#define GEX_CRPT_CHART_MODE_SINGLE_OVERLAY 2

// List of possible filter types
#define GEX_AUDIT_FILTER_TNAME      0
#define GEX_AUDIT_FILTER_TNUMBER    1

class ReportTemplateSection;
class CGexCustomReport_WafMap_Section;

typedef QList<ReportTemplateSection*>::iterator
    ReportTemplateSectionIterator;

/******************************************************************************!
 * \class ReportTemplate
 ******************************************************************************/
class ReportTemplate : public QObject
{
public:
    /*!
     * \fn ReportTemplate
     * \brief Constructor
     */
    ReportTemplate() { }
    /*!
     * \fn ~ReportTemplate
     * \brief Destructor
     */
    virtual ~ReportTemplate();
    /*!
     * \fn updateAllSectionId
     */
    void UpdateAllSectionId();
    /*!
     * \fn getWafmapSectionDetails
     */
    CGexCustomReport_WafMap_Section* GetWafmapSectionDetails();
    /*!
     * \fn append
     */
    void Append(ReportTemplateSection* s) { mSections.append(s); }
    /*!
     * \fn insert
     */
    void Insert(int i, ReportTemplateSection* s) { mSections.insert(i, s); }
    /*!
     * \fn replace
     */
    void Replace(int i, ReportTemplateSection* s) { mSections.replace(i, s); }
    /*!
     * \fn at
     */
    ReportTemplateSection* At(int i) const { return mSections.at(i); }
    /*!
     * \fn takeAt
     */
    ReportTemplateSection* TakeAt(int i) { return mSections.takeAt(i); }
    /*!
     * \fn begin
     */
    ReportTemplateSectionIterator Begin() { return mSections.begin(); }
    /*!
     * \fn end
     */
    ReportTemplateSectionIterator End() { return mSections.end(); }
    /*!
     * \fn clear
     */
    void Clear() { mSections.clear(); }
    /*!
     * \fn setFrontPageText
     */
    void SetFrontPageText(const QString& s) { mFrontPageText = s; }
    /*!
     * \fn setFrontPageImage
     */
    void SetFrontPageImage(const QString& s) { mFrontPageImage = s; }
    /*!
     * \fn getFrontPageText
     */
    QString GetFrontPageText() const { return mFrontPageText; }
    /*!
     * \fn getFrontPageImage
     */
    QString GetFrontPageImage() const { return mFrontPageImage; }

private:
    Q_DISABLE_COPY(ReportTemplate)

    /*!
     * \var mSections
     */
    QList<ReportTemplateSection*> mSections;
    /*!
     * \var mFrontPageText
     * \brief HTML text to display on report's home page
     */
    QString mFrontPageText;
    /*!
     * \var mFrontPageImage
     * \brief Logo to display on report's home page
     */
    QString mFrontPageImage;
};

/******************************************************************************!
 * \class CustomReportTestAggregateSection
 ******************************************************************************/
class CustomReportTestAggregateSection
{
public:
    /*!
     * \fn CustomReportTestAggregateSection
     * \brief Constructor
     */
    CustomReportTestAggregateSection();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
    /*!
     * \var iTestType
     * \brief xx_ALL , xx_FAIL, etc.
     */
    int iTestType;
    /*!
     * \var strRangeList
     */
    QString strRangeList;
    /*!
     * \var bStats
     * \brief Display test statistics
     */
    bool bStats;
    /*!
     * \var iChartMode
     * \brief Overlay layers, or one chart per layer, or both
     */
    int iChartMode;
    /*!
     * \var iHistogramType
     * \brief Disabled or type of histogram to generate
     *        (over data, limits, adaptive)
     */
    int iHistogramType;
    /*!
     * \var iTrendType
     * \brief Disabled or type of trend to generate
     *        (over data, limits, adaptive)
     */
    int iTrendType;
    /*!
     * \var iScatterType
     * \brief Disabled or type of scatter to generate
     *        (over data, limits, adaptive)
     */
    int iScatterType;
    /*!
     * \var strRangeListY
     */
    QString strRangeListY;
    /*!
     * \var iBoxPlotType
     * \brief Disabled or type of boxplot
     */
    int iBoxPlotType;
    /*!
     * \var iProbabilityPlotType
     * \brief Disabled or type of probability plot
     */
    int iProbabilityPlotType;
    /*!
     * \var iBoxplotExType
     * \brief Disabled or type of boxplot extended
     */
    int iBoxplotExType;
    /*!
     * \var iMultiChartsType
     * \brief Disabled or type of MultiCharts
     */
    int iMultiChartsType;
    /*!
     * \var strChartTitle
     * \brief Label for this chart
     */
    QString strChartTitle;
    /*!
     * \var bChartTitle
     * \brief true if ChartTitle is valid
     */
    bool bChartTitle;
    /*!
     * \var strAxisLegendX
     * \brief Legend for X axis
     */
    QString strAxisLegendX;
    /*!
     * \var bLegendX
     * \brief true if Legend for X axis is valid
     */
    bool bLegendX;
    /*!
     * \var bLogScaleX
     * \brief true if X scale to use Logarithmic resolution
     *        (false for linear scale)
     */
    bool bLogScaleX;
    /*!
     * \var strAxisLegendY
     * \brief Legend for Y axis
     */
    QString strAxisLegendY;
    /*!
     * \var bLegendY
     * \brief true if Legend for Y axis is valid
     */
    bool bLegendY;
    /*!
     * \var bLogScaleY
     * \brief true if X scale to use Logarithmic resolution
     *        (false for linear scale)
     */
    bool bLogScaleY;
    /*!
     * \var lfLowX
     * \brief Custom Viewport rectangle to focus on
     */
    double lfLowX;
    /*!
     * \var lfLowY
     */
    double lfLowY;
    /*!
     * \var lfHighX
     */
    double lfHighX;
    /*!
     * \var lfHighY
     */
    double lfHighY;
    /*!
     * \var bBoxBars
     * \brief true if draw with BARS (histogram charting only)
     */
    bool bBoxBars;
    /*!
     * \var bFittingCurve
     * \brief true if draw fitting curve / spin
     */
    bool bFittingCurve;
    /*!
     * \var bBellCurve
     * \brief true if draw the Guaussian Bell-curve shape
     */
    bool bBellCurve;
    /*!
     * \var bLines
     * \brief true if connect points with a line
     */
    bool bLines;
    /*!
     * \var bSpots
     * \brief true if draw a spot at each data point
     */
    bool bSpots;
    /*!
     * \var iLineWidth
     * \brief Plotting line width
     */
    int iLineWidth;
    /*!
     * \var cColor
     * \brief Line color
     */
    QColor cColor;
    /*!
     * \var iLineStyle
     * \brief Line style: solid, dashed, etc.
     */
    int iLineStyle;
    /*!
     * \var iSpotStyle
     * \brief Spot style: circle, diamond, rectangle, etc.
     */
    int iSpotStyle;
    /*!
     * \var icMarkerWidth
     * \brief Markers: Line width (0=hide)
     */
    int icMarkerWidth[GEX_CRPT_MARKER_TOTALSIZE];
    /*!
     * \var cMarkerColor
     * \brief Markers: color
     */
    QColor cMarkerColor[GEX_CRPT_MARKER_TOTALSIZE];
};

/******************************************************************************!
 * \class CGexCustomReport_WafMap_Section
 ******************************************************************************/
class CGexCustomReport_WafMap_Section
{
public:
    /*!
     * \fn CGexCustomReport_WafMap_Section
     * \brief Constructor
     */
    CGexCustomReport_WafMap_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
    /*!
     * \var iWafermapType
     * \brief disabled, or type of report to generate
     */
    int iWafermapType;
    /*!
     * \var strRangeList
     */
    QString strRangeList;
};

/******************************************************************************!
 * \class CGexCustomReport_Binning_Section
 ******************************************************************************/
class CGexCustomReport_Binning_Section
{
public:
    /*!
     * \fn CGexCustomReport_Binning_Section
     * \brief Constructor
     */
    CGexCustomReport_Binning_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
};

/******************************************************************************!
 * \class CGexCustomReport_Pareto_Section
 ******************************************************************************/
class CGexCustomReport_Pareto_Section
{
public:
    /*!
     * \fn CGexCustomReport_Pareto_Section
     * \brief Constructor
     */
    CGexCustomReport_Pareto_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
};

/******************************************************************************!
 * \class CGexCustomReport_Pearson_Section
 ******************************************************************************/
class CGexCustomReport_Pearson_Section
{
public:
    /*!
     * \fn CGexCustomReport_Pearson_Section
     * \brief Constructor
     */
    CGexCustomReport_Pearson_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
};

/******************************************************************************!
 * \class CGexCustomReport_TesterCorrelationGB_Section
 ******************************************************************************/
class CGexCustomReport_TesterCorrelationGB_Section
{
public:
    /*!
     * \fn CGexCustomReport_TesterCorrelationGB_Section
     * \brief Constructor
     */
    CGexCustomReport_TesterCorrelationGB_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
    /*!
     * \var strGuardBandFile
     */
    QString strGuardBandFile;
};

/******************************************************************************!
 * \class CGexCustomReport_Production_Section
 ******************************************************************************/
class CGexCustomReport_Production_Section
{
public:
    /*!
     * \fn CGexCustomReport_Production_Section
     * \brief Constructor
     */
    CGexCustomReport_Production_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
    /*!
     * \var iChartingType
     */
    int iChartingType;
};

/******************************************************************************!
 * \class CGexCustomReport_GlobalInfo_Section
 ******************************************************************************/
class CGexCustomReport_GlobalInfo_Section
{
public:
    /*!
     * \fn CGexCustomReport_GlobalInfo_Section
     * \brief Constructor
     */
    CGexCustomReport_GlobalInfo_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
};

/******************************************************************************!
 * \class CGexCustomReport_Datalog_Section
 ******************************************************************************/
class CGexCustomReport_Datalog_Section
{
public:
    /*!
     * \fn CGexCustomReport_Datalog_Section
     * \brief Constructor
     */
    CGexCustomReport_Datalog_Section();

    /*!
     * \var strSectionTitle
     */
    QString strSectionTitle;
    /*!
     * \var iDatalogType
     * \brief disabled, or type of report to generate
     */
    int iDatalogType;
    /*!
     * \var strTestList
     */
    QString strTestList;
};

/******************************************************************************!
 * \class AuditParameterFilter
 * \brief Filter cell
 ******************************************************************************/
class AuditParameterFilter
{
public:
    /*!
     * \fn AuditParameterFilter
     * \brief Constructor
     */
    AuditParameterFilter();
    /*!
     * \fn ~AuditParameterFilter
     * \brief Destructor
     */
    ~AuditParameterFilter();

    /*!
     * \var strProduct
     * \brief Holds product name
     */
    QString strProduct;
    /*!
     * \var iFilterType
     * \brief Holds filter type (ignore test name, ignore test number)
     */
    int iFilterType;
    /*!
     * \var strFilter
     * \brief Filter string (eg: continuity*)
     */
    QString strFilter;
    /*!
     * \var pTestList
     * \brief List of tests to filter (if FilterType = filter over test numbers)
     */
    GS::QtLib::Range* pTestList;
};

/******************************************************************************!
 * \class CustomReportFileAuditSection
 ******************************************************************************/
class CustomReportFileAuditSection
{
public:
    /*!
     * \fn CustomReportFileAuditSection
     * \brief Constructor
     */
    CustomReportFileAuditSection();
    /*!
     * \fn ~CustomReportFileAuditSection
     * \brief Destructor
     */
    ~CustomReportFileAuditSection();

    /*!
     * \var strSectionTitle
     * \brief
     */
    QString strSectionTitle;
    /*!
     * \var bCheckBadRecords
     */
    bool bCheckBadRecords;
    /*!
     * \var bCheckMissingRecords
     */
    bool bCheckMissingRecords;
    /*!
     * \var bCheckEndRecords
     */
    bool bCheckEndRecords;
    /*!
     * \var bCheckTestNames
     */
    bool bCheckTestNames;
    /*!
     * \var bCheckTestLimits
     */
    bool bCheckTestLimits;
    /*!
     * \var bCheckMissingLimits
     */
    bool bCheckMissingLimits;
    /*!
     * \var bCheckZeroRange
     */
    bool bCheckZeroRange;
    /*!
     * \var bCheckCategories
     */
    bool bCheckCategories;
    /*!
     * \var bCheckMultiModal
     */
    bool bCheckMultiModal;
    /*!
     * \var bCheckMeasurementIssue
     */
    bool bCheckMeasurementIssue;
    /*!
     * \var bCheckRangeIssue
     */
    bool bCheckRangeIssue;
    /*!
     * \var bCheckSiteCorrelation
     */
    bool bCheckSiteCorrelation;
    /*!
     * \var bShowHistogram
     */
    bool bShowHistogram;
    /*!
     * \var ldSamplesLimit
     */
    long ldSamplesLimit;
    /*!
     * \var lfCpkLimit
     */
    double lfCpkLimit;
    /*!
     * \var cTestFilters
     * \brief Filters list
     */
    QList<AuditParameterFilter*> cTestFilters;
    /*!
     * \var bCheckProductID
     */
    bool bCheckProductID;
    /*!
     * \var bCheckLotID
     */
    bool bCheckLotID;
    /*!
     * \var bCheckWaferID
     */
    bool bCheckWaferID;
    /*!
     * \var bCheckDieLoc
     */
    bool bCheckDieLoc;
    /*!
     * \var bLongReportMode
     */
    bool bLongReportMode;
};

/******************************************************************************!
 * \class CustomReportEnterpriseReportSection
 ******************************************************************************/
class CustomReportEnterpriseReportSection
{
public:
    /*!
     * \fn CustomReportEnterpriseReportSection
     * \brief Constructor
     */
    CustomReportEnterpriseReportSection();
    /*!
     * \fn ~CustomReportEnterpriseReportSection
     * \brief Destructor
     */
    ~CustomReportEnterpriseReportSection();

    /*!
     * \var m_strSectionTitle
     * \brief Section title (Report title)
     */
    QString m_strSectionTitle;
    /*!
     * \var m_strReportType
     * \brief Report type (UPH, Yield, Consolidated Yield)
     */
    QString m_strReportType;
    /*!
     * \var m_strDatabaseLogicalName
     * \brief Database name
     */
    QString m_strDatabaseLogicalName;
    /*!
     * \var m_strDatabaseType
     * \brief Selected data type (E-Test, Wafer-Sort, Final-Test)
     */
    QString m_strDatabaseType;
    /*!
     * \var m_strTimePeriod
     * \brief Selected time period (last n days, from calendar)
     */
    QString m_strTimePeriod;
    /*!
     * \var m_clFromDate
     * \brief If using calendar: period begin
     */
    QDate m_clFromDate;
    /*!
     * \var m_clToDate
     * \brief If using calendar: period end
     */
    QDate m_clToDate;
    /*!
     * \var m_strFiltersList
     * \brief Filters
     */
    QStringList m_strFiltersList;
    /*!
     * \var m_strAggregateField
     * \brief Field to use to aggregate results
     */
    QString m_strAggregateField;
    /*!
     * \var m_bDumpRawData
     * \brief true if dump the SQL table into a
     *        CSV file after each query processed
     */
    bool m_bDumpRawData;
    /*!
     * \var m_strLeftAxis
     * \brief Data on left axis (Yield, Volume, none)
     */
    QString m_strLeftAxis;
    /*!
     * \var m_strRightAxis
     * \brief Data on right axis (Yield, Volume, none)
     */
    QString m_strRightAxis;
    /*!
     * \var m_plistSeries
     * \brief List of series definitions
     */
    GexDbPlugin_ER_Parts_Series m_plistSeries;
    /*!
     * \var m_strBinList
     * \brief Binnings (eg: pass OR HBIN:7 OR SBIN:22 etc.)
     */
    QString m_strBinList;
    /*!
     * \var m_strChartingMode
     * \brief Charting mode: Line, Bars, ...
     */
    QString m_strChartingMode;
    /*!
     * \var m_strSplitField
     * \brief Split field
     */
    QString m_strSplitField;
    /*!
     * \var m_strLayerMode
     * \brief Layer mode if split mode (eg: Mult-layer or multi-charts)
     */
    QString m_strLayerMode;
    /*!
     * \var m_strBarStyle
     * \brief 2D, 3D, etc.
     */
    QString m_strBarStyle;
    /*!
     * \var m_bGradientBarColor
     * \brief false if using plain color, true if using gradient color instead
     */
    bool m_bGradientBarColor;
    /*!
     * \var m_bSemiTransparentBar
     * \brief false if plain color, true if semi-transparent
     */
    bool m_bSemiTransparentBar;
    /*!
     * \var m_strLineStyle
     * \brief Line, spline, steps, ...
     */
    QString m_strLineStyle;
    /*!
     * \var m_bDashedLine
     * \brief false if drawing full line, true if drawing dashed line
     */
    bool m_bDashedLine;
    /*!
     * \var m_strLineSpots
     * \brief Type of line spots (if any)
     */
    QString m_strLineSpots;
    /*!
     * \var m_strBackgroundStyle
     * \brief Type of chart background
     */
    QString m_strBackgroundStyle;
    /*!
     * \var m_cDefaultColor
     * \brief Default Charting color (for 1st layer only)
     */
    QColor m_cDefaultColor;
    /*!
     * \var m_cBackgroundColor
     * \brief Background color if a custom mode defined in 'strBackgroundStyle'
     */
    QColor m_cBackgroundColor;
    /*!
     * \var m_bShowValueMarkers
     * \brief true if must write value on top of each bar/data point
     */
    bool m_bShowValueMarkers;
    /*!
     * \var m_YieldWizard_strDataSource
     * \brief Data source for yield computation (raw data, consolidated data)
     */
    QString m_YieldWizard_strDataSource;
    /*!
     * \var m_YieldWizard_strColorMgt
     * \brief Color management option (auto, manual)
     */
    QString m_YieldWizard_strColorMgt;
    /*!
     * \var m_YieldWizard_bSoftBin
     * \brief Set to true if Sof Binnings should be used, false else
     */
    bool m_YieldWizard_bSoftBin;
    /*!
     * \var m_YieldWizard_strlFields_GraphSplit
     * \brief List of fields for splitting result data in several graphs
     */
    QStringList m_YieldWizard_strlFields_GraphSplit;
    /*!
     * \var m_YieldWizard_strlFields_LayerSplit
     * \brief List of fields for splitting result data
     *        in several layers for each graph
     */
    QStringList m_YieldWizard_strlFields_LayerSplit;
    /*!
     * \var m_YieldWizard_Global_strBarStyle
     * \brief Global style: bar style (2D, 3D, ...)
     */
    QString m_YieldWizard_Global_strBarStyle;
    /*!
     * \var m_YieldWizard_Global_bGradientBarColor
     * \brief Global style: set to true if bars should use gradient color
     */
    bool m_YieldWizard_Global_bGradientBarColor;
    /*!
     * \var m_YieldWizard_Global_bSemiTransparentBar
     * \brief Global style: set to true if bars should be semi-transparent
     */
    bool m_YieldWizard_Global_bSemiTransparentBar;
    /*!
     * \var m_YieldWizard_Global_nOverlappingRatio
     * \brief Global style: bars overlapping ratio
     */
    int m_YieldWizard_Global_nOverlappingRatio;
    /*!
     * \var m_YieldWizard_Global_strBackgroundStyle
     * \brief Global style: Type of chart background
     */
    QString m_YieldWizard_Global_strBackgroundStyle;
    /*!
     * \var m_YieldWizard_Global_cBackgroundColor
     * \brief Global style: Background color if a custom mode
     *        defined in 'strBackgroundStyle'
     */
    QColor m_YieldWizard_Global_cBackgroundColor;
    /*!
     * \var m_YieldWizard_Global_strLayerPolicy
     * \brief Global style: Layer policy (dark to light, light to dark)
     */
    QString m_YieldWizard_Global_strLayerPolicy;
    /*!
     * \var m_YieldWizard_Volume_strChartingMode
     * \brief Volume style: charting mode (bars, lines)
     */
    QString m_YieldWizard_Volume_strChartingMode;
    /*!
     * \var m_YieldWizard_Volume_strDataLabels
     * \brief Volume style: data labels
     */
    QString m_YieldWizard_Volume_strDataLabels;
    /*!
     * \var m_YieldWizard_Volume_strLineStyle
     * \brief Volume style: line style (line, spline, steps)
     */
    QString m_YieldWizard_Volume_strLineStyle;
    /*!
     * \var m_YieldWizard_Volume_strLineSpots
     * \brief Volume style: line spots (none, triangle, circle, ...)
     */
    QString m_YieldWizard_Volume_strLineSpots;
    /*!
     * \var m_YieldWizard_Volume_strLineProperty
     * \brief Volume style: line property (solid, dashed, ...)
     */
    QString m_YieldWizard_Volume_strLineProperty;
    /*!
     * \var m_YieldWizard_Volume_cColor
     * \brief Volume style: color (used if bar or line charting mode)
     */
    QColor m_YieldWizard_Volume_cColor;
    /*!
     * \var m_YieldWizard_BinPareto_strTitle
     * \brief Binning pareto title
     */
    QString m_YieldWizard_BinPareto_strTitle;
    /*!
     * \var m_YieldWizard_BinPareto_strBinnings
     * \brief Binnings to use in Binning pareto (all, pass, fail)
     */
    QString m_YieldWizard_BinPareto_strBinnings;
    /*!
     * \var m_YieldWizard_BinPareto_nMaxCategories
     * \brief Max categories to display in Binning pareto
     */
    int m_YieldWizard_BinPareto_nMaxCategories;
    /*!
     * \var m_YieldWizard_BinPareto_strChartingMode
     * \brief Binning pareto style: charting mode (pie, bars, lines)
     */
    QString m_YieldWizard_BinPareto_strChartingMode;
    /*!
     * \var m_YieldWizard_BinPareto_strDataLabels
     * \brief Binning pareto style: data labels
     */
    QString m_YieldWizard_BinPareto_strDataLabels;
    /*!
     * \var m_YieldWizard_BinPareto_strLineStyle
     * \brief Binning pareto style: line style (line, spline, steps)
     */
    QString m_YieldWizard_BinPareto_strLineStyle;
    /*!
     * \var m_YieldWizard_BinPareto_strLineSpots
     * \brief Binning pareto style: line spots (none, triangle, circle, ...)
     */
    QString m_YieldWizard_BinPareto_strLineSpots;
    /*!
     * \var m_YieldWizard_BinPareto_strLineProperty
     * \brief Binning pareto style: line property (solid, dashed, ...)
     */
    QString m_YieldWizard_BinPareto_strLineProperty;
    /*!
     * \var m_YieldWizard_BinPareto_cColor
     * \brief Binning pareto style: color (used if bar or line charting mode)
     */
    QColor m_YieldWizard_BinPareto_cColor;
    /*!
     * \var m_YieldWizard_bShowBinParetoOverMaverick
     * \brief For maverick wafers, lots, show the Binning Pareto chart
     */
    bool m_YieldWizard_bShowBinParetoOverMaverick;
    /*!
     * \var m_YieldWizard_EtWt_MaverickWafer_strAlarmType
     * \brief Alarm type to use to define a maverick wafer:
     *        standard alarm, critical alarm or any alarm type
     */
    QString m_YieldWizard_EtWt_MaverickWafer_strAlarmType;
    /*!
     * \var m_YieldWizard_EtWt_MaverickWafer_nAlarmCount
     * \brief Total alarms required in a wafer to show it 'Maverick'
     */
    int m_YieldWizard_EtWt_MaverickWafer_nAlarmCount;
    /*!
     * \var m_YieldWizard_EtWt_MaverickLot_nWaferCount
     * \brief Total Maverick wafers required in a lot to show it 'Maverick'
     */
    int m_YieldWizard_EtWt_MaverickLot_nWaferCount;
    /*!
     * \var m_Genealogy_strProduct
     * \brief Product selected for Genealogy report
     */
    QString m_Genealogy_strProduct;
    /*!
     * \var m_Genealogy_Options_strGranularity
     * \brief Granularity (lot, wafer) for Genealogy Yield vs Yield scatter
     */
    QString m_Genealogy_Options_strGranularity;
};

/******************************************************************************!
 * \class ReportTemplateSection
 * \brief Structure holding info for one report section
 ******************************************************************************/
class ReportTemplateSection
{
public:
    /*!
     * \fn ReportTemplateSection
     * \brief Constructor
     */
    ReportTemplateSection();
    /*!
     * \fn ~GexReportTemplateSection
     * \brief Destructor
     */
    ~ReportTemplateSection();
    /*!
     * \fn getSectionTitle
     */
    QString getSectionTitle();
    /*!
     * \fn getTestList
     */
    QString getTestList();
    /*!
     * \fn getCharts
     */
    QString getCharts();

    /*!
     * \var iSectionType
     */
    int iSectionType;
    /*!
     * \var iSection_ID
     * \brief holds section# in report when building pages
     *        (can be used to index images)
     */
    int iSection_ID;
    /*!
     * \var pAggregate
     */
    CustomReportTestAggregateSection* pAggregate;
    /*!
     * \var pWafmap
     */
    CGexCustomReport_WafMap_Section* pWafmap;
    /*!
     * \var pBinning
     */
    CGexCustomReport_Binning_Section* pBinning;
    /*!
     * \var pPareto
     */
    CGexCustomReport_Pareto_Section* pPareto;
    /*!
     * \var pPearson
     */
    CGexCustomReport_Pearson_Section* pPearson;
    /*!
     * \var pTesterCorrelationGB
     */
    CGexCustomReport_TesterCorrelationGB_Section* pTesterCorrelationGB;
    /*!
     * \var pProduction
     */
    CGexCustomReport_Production_Section* pProduction;
    /*!
     * \var pGlobalInfo
     */
    CGexCustomReport_GlobalInfo_Section* pGlobalInfo;
    /*!
     * \var pFileAudit
     */
    CustomReportFileAuditSection* pFileAudit;
    /*!
     * \var pEnterpriseReport
     */
    CustomReportEnterpriseReportSection* pEnterpriseReport;
    /*!
     * \var pDatalog
     */
    CGexCustomReport_Datalog_Section* pDatalog;
};

}
}

#endif
