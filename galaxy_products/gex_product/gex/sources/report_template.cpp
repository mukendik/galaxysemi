/******************************************************************************!
 * \file report_template.cpp
 ******************************************************************************/

#include "report_template.h"
#include "gqtl_utils.h"
#include "gex_constants.h"

#if ! defined(C_INFINITE)
#define C_INFINITE (double) 9e99
#endif

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn ~ReportTemplate
 * \brief Destructor
 ******************************************************************************/
ReportTemplate::~ReportTemplate()
{
    while (! mSections.isEmpty())
    {
        delete mSections.takeFirst();
    }
}

/******************************************************************************!
 * \fn UpdateAllSectionId
 * \brief  Set Section ID: can be used by each
 *         report section to create unique Image name
 ******************************************************************************/
void ReportTemplate::UpdateAllSectionId()
{
    ReportTemplateSection* pSection = NULL;
    for (int iSectionID = 0; iSectionID < mSections.count(); ++iSectionID)
    {
        pSection = mSections.at(iSectionID);
        pSection->iSection_ID = iSectionID;
    }
}

/******************************************************************************!
 * \fn AuditParameterFilter
 * \brief Constructor: File Audit parameter filter class
 ******************************************************************************/
AuditParameterFilter::AuditParameterFilter()
{
    strProduct  = "";  // Holds product name
    // Holds filter type (ignore test name, ignore test number)
    iFilterType = GEX_AUDIT_FILTER_TNAME;
    QString strFilter = "";  // Filter string (eg: continuity*)
    // List of tests to filter (if FilterType = filter over test numbers)
    pTestList = NULL;
}

/******************************************************************************!
 * \fn ~AuditParameterFilter
 * \brief Destructor
 ******************************************************************************/
AuditParameterFilter::~AuditParameterFilter()
{
    if (pTestList != NULL)
    {
        delete pTestList;
    }
}

/******************************************************************************!
 * \fn ReportTemplateSection
 * \brief Constructor: Section entry
 ******************************************************************************/
ReportTemplateSection::ReportTemplateSection()
{
    iSectionType = -1;
    pAggregate   = NULL;
    pWafmap      = NULL;
    pBinning     = NULL;
    pPareto      = NULL;
    pPearson     = NULL;
    pProduction  = NULL;
    pGlobalInfo  = NULL;
    pFileAudit   = NULL;
    pTesterCorrelationGB = NULL;
    pEnterpriseReport    = NULL;
    pDatalog = NULL;
}

/******************************************************************************!
 * \fn ~ReportTemplateSection
 * \brief Destructor: Section entry
 ******************************************************************************/
ReportTemplateSection::~ReportTemplateSection()
{
    if (pAggregate)
    {
        delete pAggregate;
    }
    pAggregate = 0;
    if (pWafmap)
    {
        delete pWafmap;
    }
    pWafmap = 0;
    if (pBinning)
    {
        delete pBinning;
    }
    pBinning = 0;
    if (pPareto)
    {
        delete pPareto;
    }
    pPareto = 0;
    if (pPearson)
    {
        delete pPearson;
    }
    pPearson = 0;
    if (pProduction)
    {
        delete pProduction;
    }
    pProduction = 0;
    if (pGlobalInfo)
    {
        delete pGlobalInfo;
    }
    pGlobalInfo = 0;
    if (pFileAudit)
    {
        delete pFileAudit;
    }
    pFileAudit = 0;
    if (pTesterCorrelationGB)
    {
        delete pTesterCorrelationGB;
    }
    pTesterCorrelationGB = 0;
    if (pEnterpriseReport)
    {
        delete pEnterpriseReport;
    }
    pEnterpriseReport = 0;
    if (pDatalog)
    {
        delete pDatalog;
    }
    pDatalog = 0;
}

/******************************************************************************!
 * \fn CustomReportTestAggregateSection
 * \brief Constructor: Aggregate details
 ******************************************************************************/
CustomReportTestAggregateSection::CustomReportTestAggregateSection()
{
    bStats = false;  // Disable statistics
    strRangeList   = "";
    strRangeListY  = "";
    iTestType      = -1;  // xx_ALL , xx_FAIL, etc.
    // Disabled or type of histogram to generate (over data, limits, adaptive)
    iHistogramType = -1;
    // Disabled or type of trend to generate (over data, limits, adaptive)
    iTrendType     = -1;
    // Disabled or type of scatter to generate (over data, limits, adaptive)
    iScatterType   = -1;
    iBoxPlotType   = -1;        // Disabled or type of boxplot
    iProbabilityPlotType = -1;  // Disabled or type of probability plot
    iBoxplotExType       = -1;  // Disabled or type of box plot extended
    iMultiChartsType     = -1;  // Disabled or type of MultiCharts
    iChartMode           = GEX_CRPT_CHART_MODE_SINGLE_TEST;

    // Titles & viewport
    strChartTitle = "";     // Label for this chart
    bChartTitle   = false;  // true if ChartTitle is valid

    strAxisLegendX = "";    // Legend for X axis
    bLegendX       = false; // 'true' if Legend for X axis is valid
    // 'true' if X scale to use Logarithmic resolution
    // ('false' for linear scale)
    bLogScaleX     = false;

    strAxisLegendY = "";    // Legend for Y axis
    bLegendY       = false; // true if Legend for Y axis is valid
    // 'true' if X scale to use Logarithmic resolution
    // ('false' for linear scale)
    bLogScaleY     = false;

    // Custom Viewport rectangle to focus on
    lfLowX  = -C_INFINITE;
    lfLowY  = -C_INFINITE;
    lfHighX = C_INFINITE;
    lfHighY = C_INFINITE;

    // Plotting style
    bBoxBars      = true;   // true if draw with BARS (histogram charting only)
    bFittingCurve = false;  // true if draw fitting curve / spin.
    bBellCurve    = false;  // true if draw Guaussian Bell-curve shape
    bLines        = true;   // true if connect points with a line
    bSpots        = false;  // true if draw a spot at each data point.
    iSpotStyle    = 0;      // Spot style: circle, rectangle, diamond, etc.
    iLineWidth    = 1;      // Plotting line width
    iLineStyle    = 0;      // Line style: solid
    cColor        = QColor(0, 85, 0);  // Line color

    // Markers
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_MEAN]    = 1;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_MEAN]     = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_MIN]     = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_MIN]      = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_MAX]     = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_MAX]      = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_LIMITS]  = 1;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_LIMITS]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_MEDIAN]  = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_MEDIAN]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_LOT]     = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_LOT]      = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_SUBLOT]  = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_SUBLOT]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_GROUP]   = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_GROUP]    = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_2SIGMA]  = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_2SIGMA]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_3SIGMA]  = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_3SIGMA]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_6SIGMA]  = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_6SIGMA]   = Qt::red;
    icMarkerWidth[GEX_CRPT_MARKER_STYLE_12SIGMA] = 0;
    cMarkerColor[GEX_CRPT_MARKER_STYLE_12SIGMA]  = Qt::red;
}

/******************************************************************************!
 * \fn CGexCustomReport_WafMap_Section
 * \brief Constructor: Wafermap details
 ******************************************************************************/
CGexCustomReport_WafMap_Section::CGexCustomReport_WafMap_Section()
{
    iWafermapType = -1;  // disabled, or type of report to generate
    strRangeList  = "";
}

/******************************************************************************!
 * \fn CGexCustomReport_Binning_Section
 * \brief Constructor: Binning details
 ******************************************************************************/
CGexCustomReport_Binning_Section::CGexCustomReport_Binning_Section()
{
}

/******************************************************************************!
 * \fn CGexCustomReport_Pareto_Section
 * \brief Constructor: Pareto details
 ******************************************************************************/
CGexCustomReport_Pareto_Section::CGexCustomReport_Pareto_Section()
{
}

/******************************************************************************!
 * \fn CGexCustomReport_Pearson_Section
 * \brief Constructor: Pearson details
 ******************************************************************************/
CGexCustomReport_Pearson_Section::CGexCustomReport_Pearson_Section()
{
}

/******************************************************************************!
 * \fn CGexCustomReport_Production_Section
 * \brief Constructor: Production details
 ******************************************************************************/
CGexCustomReport_Production_Section::CGexCustomReport_Production_Section()
{
}

/******************************************************************************!
 * \fn CGexCustomReport_TesterCorrelationGB_Section
 * \brief Constructor: Tester-to-tester correlation & Guardband details
 ******************************************************************************/
CGexCustomReport_TesterCorrelationGB_Section::
CGexCustomReport_TesterCorrelationGB_Section()
{
}

/******************************************************************************!
 * \fn CGexCustomReport_GlobalInfo_Section
 * \brief Constructor: Global Info details
 ******************************************************************************/
CGexCustomReport_GlobalInfo_Section::CGexCustomReport_GlobalInfo_Section()
{
}

/******************************************************************************!
 * \fn CustomReportFileAuditSection
 * \brief Constructor: File Audit details
 ******************************************************************************/
CustomReportFileAuditSection::CustomReportFileAuditSection()
{
    // File analysis
    bCheckBadRecords     = true;
    bCheckMissingRecords = true;
    bCheckEndRecords     = true;
    bCheckTestNames      = true;
    bCheckTestLimits     = true;
    bCheckMissingLimits  = false;

    // Dataset analysis
    bCheckZeroRange  = true;
    bCheckCategories = true;
    bCheckMultiModal = true;
    bCheckMeasurementIssue = true;
    bCheckRangeIssue       = true;
    bCheckSiteCorrelation  = true;
    bShowHistogram         = true;
    ldSamplesLimit         = 30;
    lfCpkLimit = 20.0;

    // Other options
    bCheckProductID = true;;
    bCheckLotID     = true;
    bCheckWaferID   = true;
    bCheckDieLoc    = true;
    bLongReportMode = false;
}

/******************************************************************************!
 * \fn ~CustomReportFileAuditSection
 * \brief Destructor: File Audit details
 ******************************************************************************/
CustomReportFileAuditSection::~CustomReportFileAuditSection()
{
    while (! cTestFilters.isEmpty())
    {
        delete cTestFilters.takeFirst();
    }
}

/******************************************************************************!
 * \fn CGexCustomReport_Datalog_Section
 * \brief Constructor: Wafermap details
 ******************************************************************************/
CGexCustomReport_Datalog_Section::CGexCustomReport_Datalog_Section()
{
    iDatalogType = -1;  // disabled, or type of report to generate
    strTestList  = "";
}

/******************************************************************************!
 * \fn CustomReportEnterpriseReportSection
 * \brief Constructor: Enterprise Reports section
 ******************************************************************************/
CustomReportEnterpriseReportSection::
CustomReportEnterpriseReportSection()
{
    // Reports using std GUI (UPH, Yield, Consolidated Yield)
    // Charting mode: Line, Bars, ...
    m_strChartingMode     = "Bars";
    // 2D, 3D, etc.
    m_strBarStyle         = "3D Bar";
    // 'false' if using plain color, 'true' if using gradient color instead
    m_bGradientBarColor   = true;
    // 'false' if plain color, 'true' if semi-transparent
    m_bSemiTransparentBar = true;
    // Line, spline, steps, ...
    m_strLineStyle       = "Line";
    // 'false' if drawing full line, 'true' if drawing dashed line
    m_bDashedLine        = false;
    // Type of line spots (if any)
    m_strLineSpots       = "Square";
    // Type of chart background
    m_strBackgroundStyle = "Gradient color...";
    // Default Charting color (for 1st layer only)
    m_cDefaultColor     = QColor(103, 94, 199);
    // Background color if a custom mode defined in 'strBackgroundStyle'
    m_cBackgroundColor  = QColor(255, 255, 127);
    // 'true' if must write value on top of each bar/data point
    m_bShowValueMarkers = true;
    // 'true' if dump the SQL table into a CSV file after each query processed
    m_bDumpRawData      = true;

    // Report using Yield Wizard GUI
    // Data source for yield computation (raw data, consolidated data)
    m_YieldWizard_strDataSource = "Raw data";
    // Color management option (auto, manual)
    m_YieldWizard_strColorMgt = "Manual";
    // Set to true if Sof Binnings should be used, false else
    m_YieldWizard_bSoftBin    = true;
    // Global style
    // Global style: bar style (2D, 3D, ...)
    m_YieldWizard_Global_strBarStyle = "3D Bar";
    // Global style: set to true if bars should use gradient color
    m_YieldWizard_Global_bGradientBarColor   = true;
    // Global style: set to true if bars should be semi-transparent
    m_YieldWizard_Global_bSemiTransparentBar = true;
    // Global style: bars overlapping ratio
    m_YieldWizard_Global_nOverlappingRatio   = 50;
    // Global style: Type of chart background
    m_YieldWizard_Global_strBackgroundStyle  = "Gradient color...";
    // Global style: Background color if a custom mode defined
    // in 'strBackgroundStyle'
    m_YieldWizard_Global_cBackgroundColor    = QColor(255, 255, 127);
    // Global style: Layer policy (dark to light, light to dark)
    m_YieldWizard_Global_strLayerPolicy      = "Dark to Light";
    // Volume style
    // Volume style: charting mode (bars, lines)
    m_YieldWizard_Volume_strChartingMode = "Lines";
    // Volume style: data labels
    m_YieldWizard_Volume_strDataLabels   = "Top";
    // Volume style: line style (line, spline, steps)
    m_YieldWizard_Volume_strLineStyle    = "Line";
    // Volume style: line spots (none, triangle, circle, ...)
    m_YieldWizard_Volume_strLineSpots    = "Square";
    // Volume style: line property (solid, dashed, ...)
    m_YieldWizard_Volume_strLineProperty = "SolidLine";
    // Volume style: color (used if bar or line charting mode)
    m_YieldWizard_Volume_cColor = QColor(103, 94, 199);
    // Binning pareto options
    // Binning pareto title
    m_YieldWizard_BinPareto_strTitle = "Default";
    // Binnings to use in Binning pareto (all, pass, fail)
    m_YieldWizard_BinPareto_strBinnings     = "All";
    // Max categories to display in Binning pareto
    m_YieldWizard_BinPareto_nMaxCategories  = 10;
    // Binning pareto style: charting mode (pie, bars, lines)
    m_YieldWizard_BinPareto_strChartingMode = "Pie";
    // Binning pareto style: data labels
    m_YieldWizard_BinPareto_strDataLabels   = "Top";
    // Binning pareto style: line style (line, spline, steps)
    m_YieldWizard_BinPareto_strLineStyle    = "Line";
    // Binning pareto style: line spots (none, triangle, circle, ...)
    m_YieldWizard_BinPareto_strLineSpots    = "Square";
    // Binning pareto style: line property (solid, dashed, ...)
    m_YieldWizard_BinPareto_strLineProperty = "SolidLine";
    // Binning pareto style: color (used if bar or line charting mode)
    m_YieldWizard_BinPareto_cColor          = QColor(0, 85, 0);
    // Maverick definition
    // For maverick wafers, lots, show the Binning Pareto chart
    m_YieldWizard_bShowBinParetoOverMaverick      = true;
    // Alarm type to use to define a maverick wafer:
    // standard alarm, critical alarm or any alarm type
    m_YieldWizard_EtWt_MaverickWafer_strAlarmType = "Critical alarms only";
    // Total alarms required in a wafer to show it 'Maverick'
    m_YieldWizard_EtWt_MaverickWafer_nAlarmCount  = 1;
    // Total Maverick wafers required in a lot to show it 'Maverick'
    m_YieldWizard_EtWt_MaverickLot_nWaferCount    = 1;
    // Genealogy
    m_Genealogy_Options_strGranularity = "lot";
}

/******************************************************************************!
 * \fn ~CustomReportEnterpriseReportSection
 * \brief Destructor: Enterprise Reports section
 ******************************************************************************/
CustomReportEnterpriseReportSection::
~CustomReportEnterpriseReportSection()
{
}

/******************************************************************************!
 * \fn getSectionTitle
 * \brief Get section title for given section structure
 ******************************************************************************/
QString ReportTemplateSection::getSectionTitle()
{
    // Check which widget is visible
    switch (iSectionType)
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        return pAggregate->strSectionTitle;

    case GEX_CRPT_WIDGET_WAFMAP:
        return pWafmap->strSectionTitle;

    case GEX_CRPT_WIDGET_BINNING:
        return pBinning->strSectionTitle;

    case GEX_CRPT_WIDGET_PARETO:
        return pPareto->strSectionTitle;

    case GEX_CRPT_WIDGET_PEARSON:
        return pPearson->strSectionTitle;

    case GEX_CRPT_WIDGET_CORR_GB:
        return pTesterCorrelationGB->strSectionTitle;

    case GEX_CRPT_WIDGET_PRODUCTION:
        return pProduction->strSectionTitle;

    case GEX_CRPT_WIDGET_GLOBALINFO:
        return pGlobalInfo->strSectionTitle;

    case GEX_CRPT_WIDGET_FILE_AUDIT:
        return pFileAudit->strSectionTitle;

    case GEX_CRPT_WIDGET_DATALOG:
        return pDatalog->strSectionTitle;

    default:
        return "Unknown";
    }
}

/******************************************************************************!
 * \fn getTestList
 * \brief Get Test list for given section structure
 ******************************************************************************/
QString ReportTemplateSection::getTestList()
{
    // Check which widget is visible
    QString strTestList;

    switch (iSectionType)
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        switch (pAggregate->iTestType)
        {
        default:
        case GEX_CRPT_TEST_TYPE_ALL:  // All tests
            strTestList = "All tests";
            break;
        case GEX_CRPT_TEST_TYPE_FAIL:  // Failing tests only
            strTestList = "Failing tests only";
            break;
        case GEX_CRPT_TEST_TYPE_OUTLIERS:  // Tests with outliers
            strTestList = "Tests with Data Cleaning";
            break;
        case GEX_CRPT_TEST_TYPE_LIST:  // Test list
            strTestList = pAggregate->strRangeList;
            break;
        case GEX_CRPT_TEST_TYPE_BADCP:  // Test with Cp less than ...
            strTestList = "Cp < " + pAggregate->strRangeList;
            break;
        case GEX_CRPT_TEST_TYPE_BADCPK:  // Test with Cpk less than ...
            strTestList = "Cpk < " + pAggregate->strRangeList;
            break;
        case GEX_CRPT_TEST_TYPE_TOP_N_FAIL:  // Top N fail count
            strTestList = "Top " + pAggregate->strRangeList + " failing tests";
            break;
        }
        break;

    case GEX_CRPT_WIDGET_WAFMAP:
        switch (pWafmap->iWafermapType + 1)
        {
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_SOFTBIN:  // Standard Wafer map: Software Binning
        case GEX_WAFMAP_ZONAL_SOFTBIN:
        case GEX_WAFMAP_HARDBIN:  // Standard Wafer map: Hardware Binning
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            strTestList = "Bin " + pWafmap->strRangeList;
            break;
        case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            strTestList = "T" + pWafmap->strRangeList;
            break;
        }
        break;

    case GEX_CRPT_WIDGET_DATALOG:
        switch (pDatalog->iDatalogType)
        {
        default:
        case GEX_ADV_DATALOG_ALL:  // All tests
            strTestList = "All tests";
            break;
        case GEX_ADV_DATALOG_FAIL:  // Failing tests only
            strTestList = "Failing tests only";
            break;
        case GEX_ADV_DATALOG_LIST:  // Test list
            strTestList = pDatalog->strTestList;
            break;
        case GEX_ADV_DATALOG_RAWDATA:  // Values only
            strTestList = "Raw data : " + pDatalog->strTestList;
            break;
        }
        break;

    case GEX_CRPT_WIDGET_BINNING:
    case GEX_CRPT_WIDGET_PARETO:
    case GEX_CRPT_WIDGET_PEARSON:
    case GEX_CRPT_WIDGET_CORR_GB:
    case GEX_CRPT_WIDGET_PRODUCTION:
    case GEX_CRPT_WIDGET_GLOBALINFO:
    case GEX_CRPT_WIDGET_FILE_AUDIT:
    default:
        strTestList = "-";
        break;
    }

    return strTestList;
}

/******************************************************************************!
 * \fn getCharts
 * \brief Get Charts details for a given section structure
 ******************************************************************************/
QString ReportTemplateSection::getCharts()
{
    // Check which widget is visible
    QString strCharts;

    switch (iSectionType)
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        if (pAggregate->bStats)
        {
            strCharts += "Statistics, ";
        }
        if (pAggregate->iHistogramType >= 0)
        {
            strCharts += "Histograms, ";
        }
        if (pAggregate->iTrendType >= 0)
        {
            strCharts += "Trend, ";
        }
        if (pAggregate->iScatterType >= 0)
        {
            strCharts += "Scatter, ";
        }
        if (pAggregate->iProbabilityPlotType >= 0)
        {
            strCharts += "ProbabilityPlot, ";
        }
        if (pAggregate->iBoxplotExType >= 0)
        {
            strCharts += "BoxplotExtended, ";
        }
        if (pAggregate->iMultiChartsType >= 0)
        {
            strCharts += "MultiCharts, ";
        }
        if (pAggregate->iBoxPlotType >= 0)
        {
            strCharts += "BoxPlot, ";
        }
        break;

    case GEX_CRPT_WIDGET_WAFMAP:
        switch (pWafmap->iWafermapType + 1)
        {
        // Standard Wafer map: Software Binning
        case GEX_WAFMAP_SOFTBIN:
            strCharts = "Soft Bin";
            break;
        case GEX_WAFMAP_STACK_SOFTBIN:
            strCharts = "Soft Bin (Stacked wafers)";
            break;
        case GEX_WAFMAP_ZONAL_SOFTBIN:
            strCharts = "Soft Bin (Zonal)";
            break;
        case GEX_WAFMAP_HARDBIN:
            // Standard Wafer map: Hardware Binning
            strCharts = "Hard Bin";
            break;
        case GEX_WAFMAP_STACK_HARDBIN:
            strCharts = "Hard Bin (Stacked wafers)";
            break;
        case GEX_WAFMAP_ZONAL_HARDBIN:
            strCharts = "Hard Bin (Zonal)";
            break;
        // Zoning on test limits
        case GEX_WAFMAP_TESTOVERLIMITS:
            strCharts = "Parametric Test over limits";
            break;
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            strCharts = "Parametric Test over limits (Stacked wafers)";
            break;
        // Zoning on test values range
        case GEX_WAFMAP_TESTOVERDATA:
            strCharts = "Parametric Test over data";
            break;
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            strCharts = "Parametric Test over data (Stacked wafers)";
            break;
        case GEX_WAFMAP_TEST_PASSFAIL:
            strCharts = "Parametric/Functional Test pass/fail";
            break;
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            strCharts = "Parametric/Functional Test pass/fail (Stacked wafers)";
            break;
        }
        break;

    case GEX_CRPT_WIDGET_BINNING:
        strCharts = " Soft / Hard Bins";
        break;
    case GEX_CRPT_WIDGET_PARETO:
        strCharts = " Cp, Cpk, Failures, Soft Bin, Hard Bin.";
        break;
    case GEX_CRPT_WIDGET_PEARSON:
        strCharts = "Correlation level, Scatter plots";
        break;
    case GEX_CRPT_WIDGET_CORR_GB:
        strCharts = "Tester-to-tester Correlation & Guardband";
        break;
    case GEX_CRPT_WIDGET_PRODUCTION:
        strCharts = "Production reports (Yield)...";
        break;
    case GEX_CRPT_WIDGET_GLOBALINFO:
        strCharts = "Data processed, Testing time, ...";
        break;
    case GEX_CRPT_WIDGET_FILE_AUDIT:
        strCharts = "Data File Audit...";
        break;
    case GEX_CRPT_WIDGET_DATALOG:
        strCharts = "Datalog";
        break;
    default:   strCharts = "-";
        break;
    }

    return strCharts;
}

/******************************************************************************!
 * \fn GetWafmapSectionDetails
 * \brief Return handle to Wafermap section (only one allowed per MyReport)
 ******************************************************************************/
CGexCustomReport_WafMap_Section*
ReportTemplate::GetWafmapSectionDetails()
{
    ReportTemplateSection* pNewSection;
    ReportTemplateSectionIterator iter;
    for (iter  = mSections.begin();
         iter != mSections.end(); ++iter)
    {
        // Get section
        pNewSection = *iter;

        // Check if wafermap section found
        if (pNewSection->iSectionType == GEX_CRPT_WIDGET_WAFMAP)
        {
            return pNewSection->pWafmap;
        }
    }

    // Wafermap section, not found
    return NULL;
}

}
}
