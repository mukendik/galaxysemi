/******************************************************************************!
 * \file report_template_io.cpp
 ******************************************************************************/

#include "report_template_io.h"
#include "report_template.h"
#include "report_template_gui.h"
#include "report_options.h"
#include "gex_constants.h"
#include "classes.h"
#include <gqtl_log.h>

#if ! defined(C_INFINITE)
#define C_INFINITE (double) 9e99
#endif

// In report_build.h
extern CReportOptions ReportOptions;

// In csl/ZcGexLib.cpp
extern CGexTestRange* createTestsRange(QString strParameterList,
                                       bool    bAcceptRange,
                                       bool    bIsAdvancedReport);

namespace GS {
namespace Gex {

/******************************************************************************!
 * \fn ReportTemplateIO
 * \brief Constructor
 ******************************************************************************/
ReportTemplateIO::ReportTemplateIO()
    : mReportTemplate(NULL),
      mReportTemplateGui(NULL)
{
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk
 * \brief Read Template file from disk. Load in memory
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk(ReportTemplate* reportTemplate,
                                       QString& strTemplateName)
{
    mReportTemplate = reportTemplate;

    QString strString;

    // Build path to the 'Tasks' list
    QFile file(strTemplateName);  // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        return false;  // Failed opening Tasks file
    }
    // Read config File
    QTextStream hTemplate(&file);  // Assign file handle to data stream

    // Check if valid header, or empty
    strString = hTemplate.readLine();
    if (strString != "<galaxy_template>")
    {
        file.close();
        return false;
    }

    do
    {
        // Read one line from file
        QString strString = hTemplate.readLine();

        // Section type: Home page details
        if (strString.startsWith("<section_home_page>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_HomePage(hTemplate);
        }

        // ######### SECTIONS for STDF files analysis ONLY #########
        // Section type: Aggregate
        if (strString.startsWith("<section_aggregate>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Aggregate(hTemplate);
        }

        // Section type: Wafermap
        if (strString.startsWith("<section_wafmap>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Wafmap(hTemplate);
        }

        // Section type: Binning
        if (strString.startsWith("<section_binning>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Binning(hTemplate);
        }

        // Section type: Pareto
        if (strString.startsWith("<section_pareto>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Pareto(hTemplate);
        }

        // Section type: Pearson's correlation
        if (strString.startsWith("<section_pearson>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Pearson(hTemplate);
        }

        // Section type: Tester-to-tester: Correlation & Guard Band
        if (strString.startsWith("<section_tester_correlation_gb>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_TesterCorrelationGB(hTemplate);
        }

        // Section type: Production reports
        if (strString.startsWith("<section_production>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Production(hTemplate);
        }

        // Section type: Global Info
        if (strString.startsWith("<section_global_info>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_GlobalInfo(hTemplate);
        }

        // Section type: File Audit
        if (strString.startsWith("<section_file_audit>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_FileAudit(hTemplate);
        }

        // Section type: Datalog
        if (strString.startsWith("<section_datalog>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_Datalog(hTemplate);
        }

        // ######### SECTION for SQL-ONLY Database Queries #########
        // Section type: SQL specific query
        if (strString.startsWith("<section_sql>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_SQL(hTemplate);
        }

        // Section type: Enterprise Report
        if (strString.startsWith("<section_enterprise_report>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER(hTemplate);
        }
    }
    while (hTemplate.atEnd() == false);
    file.close();

    // Set Section ID: can be used by each
    // report section to create unique Image name
    mReportTemplate->UpdateAllSectionId();

    return true;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_HomePage
 * \brief Read Template file from disk: Home page details
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_HomePage(QTextStream& hTemplate)
{
    QString strString;
    QString strParameter;

    do
    {
        // Read one line from file
        QString strString = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Home page text
        if (strString.startsWith("HomeText=", Qt::CaseInsensitive) == true)
        {
            mReportTemplate->SetFrontPageText(strParameter);
        }

        // Home page logo
        if (strString.startsWith("HomeLogo=", Qt::CaseInsensitive) == true)
        {
            mReportTemplate->SetFrontPageImage(strParameter);
        }

        // End of section reached
        if (strString.startsWith("</section_home_page>") == true)
        {
            return true;
        }
    }
    while (hTemplate.atEnd() == false);

    // Didn't find closing marker
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Aggregate
 * \brief Read Template file from disk: Aggregate section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Aggregate(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_AGGREGATE;
    GS::Gex::CustomReportTestAggregateSection* pAggregate =
        new GS::Gex::CustomReportTestAggregateSection();
    pNewSection->pAggregate = pAggregate;

    QString strString;
    QString strParameter;
    int     iRed, iGreen, iBlue;
    int     iMakerIndex, iWidth;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strSectionTitle = strParameter;
        }

        // Testlist type (all tests, failing tests only, etc.)
        if (strString.startsWith("TestType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iTestType = strParameter.toInt();
        }

        // Testlist
        if (strString.startsWith("TestList=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strRangeList = strParameter;
        }

        // Include statistics ?
        if (strString.startsWith("bStats=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bStats = (strParameter.toInt()) ? true : false;
        }

        // Charting mode: one chart per test, overlay all, both
        if (strString.startsWith("ChartMode=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iChartMode =
                (strParameter.toInt() < 0) ? 0 : strParameter.toInt();
        }

        // Histogram type (over data over limits)
        if (strString.startsWith("HistoType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iHistogramType = strParameter.toInt();
        }

        // Trend type (over data over limits)
        if (strString.startsWith("TrendType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iTrendType = strParameter.toInt();
        }

        // Scatter type (over data over limits)
        if (strString.startsWith("ScatterType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iScatterType = strParameter.toInt();
        }

        // Testlist in Y for scatter plot
        if (strString.startsWith("TestListY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strRangeListY = strParameter;
        }

        // Probability plot type (over data over limits)
        if (strString.startsWith("ProbabilityPlotType=",
                                 Qt::CaseInsensitive) == true)
        {
            pAggregate->iProbabilityPlotType = strParameter.toInt();
        }

        // Box plot extended type (over data over limits)
        if (strString.startsWith("BoxplotExType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iBoxplotExType = strParameter.toInt();
        }

        // MultiCharts type (over data over limits)
        if (strString.startsWith("MultiChartsType=",
                                 Qt::CaseInsensitive) == true)
        {
            pAggregate->iMultiChartsType = strParameter.toInt();
        }

        // BoxPlot type (over data over limits)
        if (strString.startsWith("BoxPlotType=", Qt::CaseInsensitive) == true)
        {
            pAggregate->iBoxPlotType = strParameter.toInt();
        }

        // Titles & viewport
        if (strString.startsWith("bChartTitle=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bChartTitle = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("ChartTitle=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strChartTitle = strParameter;
        }

        // X-axis Legend & scale
        if (strString.startsWith("bLegendX=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bLegendX = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("LegendX=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strAxisLegendX = strParameter;
        }
        if (strString.startsWith("bLogScaleX=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bLogScaleX = (strParameter.toInt()) ? true : false;
        }

        // Y-axis Legend & scale
        if (strString.startsWith("bLegendY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bLegendY = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("LegendY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->strAxisLegendY = strParameter;
        }
        if (strString.startsWith("bLogScaleY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bLogScaleY = (strParameter.toInt()) ? true : false;
        }

        // Viewport size
        if (strString.startsWith("lfLowX=", Qt::CaseInsensitive) == true)
        {
            pAggregate->lfLowX = strParameter.toDouble();
        }
        if (strString.startsWith("lfLowY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->lfLowY = strParameter.toDouble();
        }
        if (strString.startsWith("lfHighX=", Qt::CaseInsensitive) == true)
        {
            pAggregate->lfHighX = strParameter.toDouble();
        }
        if (strString.startsWith("lfHighY=", Qt::CaseInsensitive) == true)
        {
            pAggregate->lfHighY = strParameter.toDouble();
        }

        // Plotting style
        if (strString.startsWith("bBoxBars=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bBoxBars = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("bFittingCurve=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bFittingCurve = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("bBellCurve=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bBellCurve = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("bLines=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bLines = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("bSpots=", Qt::CaseInsensitive) == true)
        {
            pAggregate->bSpots = (strParameter.toInt()) ? true : false;
        }
        if (strString.startsWith("iLineWidth=", Qt::CaseInsensitive) == true)
        {
            // Plotting line width
            pAggregate->iLineWidth = strParameter.toInt();
        }
        if (strString.startsWith("iSpotStyle=", Qt::CaseInsensitive) == true)
        {
            // Spot style: circle, rectangle, diamond, etc.
            pAggregate->iSpotStyle = strParameter.toInt();
        }
        if (strString.startsWith("iLineStyle=", Qt::CaseInsensitive) == true)
        {
            // Line style: solid
            pAggregate->iLineStyle = strParameter.toInt();
        }
        if (strString.startsWith("cColor=", Qt::CaseInsensitive) == true)
        {
            sscanf(strParameter.toLatin1().constData(), "%d %d %d",
                   &iRed, &iGreen, &iBlue);
            pAggregate->cColor = QColor(iRed, iGreen, iBlue);
        }
        if (strString.startsWith("cMarkerColor=", Qt::CaseInsensitive) == true)
        {
            sscanf(strParameter.toLatin1().constData(), "%d %d %d %d %d",
                   &iMakerIndex, &iWidth, &iRed, &iGreen, &iBlue);
            pAggregate->cMarkerColor[iMakerIndex] = QColor(iRed, iGreen, iBlue);
            pAggregate->icMarkerWidth[iMakerIndex] = iWidth;
        }

        // Section type: Aggregate
        if (strString.startsWith("</section_aggregate>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Wafmap
 * \brief Read Template file from disk: Wafer map section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Wafmap(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_WAFMAP;
    CGexCustomReport_WafMap_Section* pWafmap =
        new CGexCustomReport_WafMap_Section();
    pNewSection->pWafmap = pWafmap;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pWafmap->strSectionTitle = strParameter;
        }

        // Wafermap type (SOFT BIN, HARDBIN, PARAMTERIC, STACKED, ETC.
        if (strString.startsWith("WaferType=", Qt::CaseInsensitive) == true)
        {
            pWafmap->iWafermapType = strParameter.toInt();
        }

        // Test/Bin list
        if (strString.startsWith("TestList=", Qt::CaseInsensitive) == true)
        {
            pWafmap->strRangeList = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_wafmap>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Binning
 * \brief Read Template file from disk: Binning section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Binning(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_BINNING;
    CGexCustomReport_Binning_Section* pBinning =
        new CGexCustomReport_Binning_Section();
    pNewSection->pBinning = pBinning;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pBinning->strSectionTitle = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_binning>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Pareto
 * \brief Read Template file from disk: Pareto section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Pareto(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_PARETO;
    CGexCustomReport_Pareto_Section* pPareto =
        new CGexCustomReport_Pareto_Section();
    pNewSection->pPareto = pPareto;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pPareto->strSectionTitle = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_pareto>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Pearson
 * \brief Read Template file from disk: Pearson's correlation section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Pearson(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_PEARSON;
    CGexCustomReport_Pearson_Section* pPearson =
        new CGexCustomReport_Pearson_Section();
    pNewSection->pPearson = pPearson;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pPearson->strSectionTitle = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_pearson>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}


/******************************************************************************!
 * \fn ReadTemplateFromDisk_TesterCorrelationGB
 * \brief Read Template file from disk:
 *        Tester-to-tester correlation & guardband section
 ******************************************************************************/
bool ReportTemplateIO::ReadTemplateFromDisk_TesterCorrelationGB(
    QTextStream& hTemplate)
{
    // Create  object to receive information extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_CORR_GB;
    CGexCustomReport_TesterCorrelationGB_Section* pTesterCorrelationGB =
        new CGexCustomReport_TesterCorrelationGB_Section();
    pNewSection->pTesterCorrelationGB = pTesterCorrelationGB;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pTesterCorrelationGB->strSectionTitle = strParameter;
        }

        // Section GuardBand file
        if (strString.startsWith("GuardBandFile=", Qt::CaseInsensitive) == true)
        {
            pTesterCorrelationGB->strGuardBandFile = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_tester_correlation_gb>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}


/******************************************************************************!
 * \fn ReadTemplateFromDisk_Production
 * \brief Read Template file from disk: Production reports
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Production(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_PRODUCTION;
    CGexCustomReport_Production_Section* pProduction =
        new CGexCustomReport_Production_Section();
    pNewSection->pProduction = pProduction;

    // Force some options to have GEX collect binning info when reading files
    ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
    if (ReportOptions.pGexAdvancedRangeList)
    {
        delete ReportOptions.pGexAdvancedRangeList;
    }
    ReportOptions.pGexAdvancedRangeList =
        createTestsRange(QString("1"), true, true);

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pProduction->strSectionTitle = strParameter;
        }

        // Chart type (GEX_ADV_PRODYIELD_SBLOT, GEX_ADV_PRODYIELD_LOT, etc.)
        if (strString.startsWith("ChartingType=", Qt::CaseInsensitive) == true)
        {
            pProduction->iChartingType = strParameter.toInt();
        }

        // End of section
        if (strString.startsWith("</section_production>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_GlobalInfo
 * \brief Read Template file from disk: Global info section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_GlobalInfo(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_GLOBALINFO;
    CGexCustomReport_GlobalInfo_Section* pGlobalInfo =
        new CGexCustomReport_GlobalInfo_Section();
    pNewSection->pGlobalInfo = pGlobalInfo;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pGlobalInfo->strSectionTitle = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_global_info>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_FileAudit
 * \brief Read Template file from disk: File Audit section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_FileAudit(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection = new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_FILE_AUDIT;
    GS::Gex::CustomReportFileAuditSection* pFileAudit = new GS::Gex::CustomReportFileAuditSection();
    pNewSection->pFileAudit = pFileAudit;

    QString strString;
    QString strParameter;
    GS::Gex::AuditParameterFilter* pFilter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pFileAudit->strSectionTitle = strParameter;
        }

        // File Audit options
        if (strString.startsWith("BadRecords=", Qt::CaseInsensitive) == true)
        {
            // Find: bad records
            pFileAudit->bCheckBadRecords = (bool) strParameter.toInt();
        }
        if (strString.startsWith("MissingRecords=", Qt::CaseInsensitive) == true)
        {
            // Find: missing records
            pFileAudit->bCheckMissingRecords = (bool) strParameter.toInt();
        }
        if (strString.startsWith("UnEndRecords=", Qt::CaseInsensitive) == true)
        {
            // Find: missing records
            pFileAudit->bCheckEndRecords = (bool) strParameter.toInt();
        }
        if (strString.startsWith("DuplicateTest=", Qt::CaseInsensitive) == true)
        {
            // Find: test names issues
            pFileAudit->bCheckTestNames = (bool) strParameter.toInt();
        }
        if (strString.startsWith("DuplicateLimits=", Qt::CaseInsensitive) == true)
        {
            // Find: test limits issue
            pFileAudit->bCheckTestLimits = (bool) strParameter.toInt();
        }
        if (strString.startsWith("MissingLimits=", Qt::CaseInsensitive) == true)
        {
            // Find: missing test limit(s)
            pFileAudit->bCheckMissingLimits = (bool) strParameter.toInt();
        }
        // Data Audit options
        if (strString.startsWith("ZeroRange=", Qt::CaseInsensitive) == true)
        {
            // Find: zero range
            pFileAudit->bCheckZeroRange = (bool) strParameter.toInt();
        }
        if (strString.startsWith("Categories=", Qt::CaseInsensitive) == true)
        {
            // Find: categories
            pFileAudit->bCheckCategories = (bool) strParameter.toInt();
        }
        if (strString.startsWith("MultiModal=", Qt::CaseInsensitive) == true)
        {
            // Find: multi-modal
            pFileAudit->bCheckMultiModal = (bool) strParameter.toInt();
        }
        if (strString.startsWith("MeasurementIssue=", Qt::CaseInsensitive) == true)
        {
            // Find: measurement issue
            pFileAudit->bCheckMeasurementIssue = (bool) strParameter.toInt();
        }
        if (strString.startsWith("RangeIssue=", Qt::CaseInsensitive) == true)
        {
            // Find: incorrect range scale
            pFileAudit->bCheckRangeIssue = (bool) strParameter.toInt();
        }
        if (strString.startsWith("SiteToSite=", Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bCheckSiteCorrelation = (bool) strParameter.toInt();
        }
        if (strString.startsWith("ShowHistogram=", Qt::CaseInsensitive) == true)
        {
            // Include Histogram in report
            pFileAudit->bShowHistogram = (bool) strParameter.toInt();
        }
        if (strString.startsWith("SamplesLimit=", Qt::CaseInsensitive) == true)
        {
            // Ignore tests with too few samples
            pFileAudit->ldSamplesLimit = strParameter.toLong();
        }
        if (strString.startsWith("CpkLimit=", Qt::CaseInsensitive) == true)
        {
            // Ignore tests with Cpk high enough
            pFileAudit->lfCpkLimit = strParameter.toDouble();
        }
        // Parameter filters
        if (strString.startsWith("<ParamFilter>", Qt::CaseInsensitive) == true)
        {
            pFilter = new GS::Gex::AuditParameterFilter;
            do
            {
                // Read filter bloc
                strString    = hTemplate.readLine();
                strParameter = strString.section('=', 1);

                if (strString.startsWith("FilterProduct=", Qt::CaseInsensitive) == true)
                {
                    //  Product name to filter on
                    pFilter->strProduct = strParameter;
                }
                if (strString.startsWith("FilterType=", Qt::CaseInsensitive) == true)
                {
                    //  Filter type (0=test names, 1= test num ...)
                    pFilter->iFilterType = strParameter.toInt();
                }
                if (strString.startsWith("FilterString=", Qt::CaseInsensitive) == true)
                {
                    pFilter->strFilter = strParameter;  // Filter string
                }
                if (strString.startsWith("</ParamFilter>", Qt::CaseInsensitive) == true)
                {
                    // Save filter entry just found
                    if (pFilter->iFilterType == GEX_AUDIT_FILTER_TNUMBER)
                    {
                        // Build test list structure from test list string
                        pFilter->pTestList = new GS::QtLib::Range(
                                pFilter->strFilter.toLatin1().constData());
                    }
                    pFileAudit->cTestFilters.append(pFilter);
                    pFilter = NULL;
                    break;  // Exit from filter reading loop
                }

            }
            while (hTemplate.atEnd() == false);
            // Unexpected end of file
            if (pFilter != NULL)
            {
                delete  pFilter;
            }
            pFilter = 0;
        }

        // Other options
        if (strString.startsWith("ProductID=", Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bCheckProductID = (bool) strParameter.toInt();
        }
        if (strString.startsWith("LotID=", Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bCheckLotID = (bool) strParameter.toInt();
        }
        if (strString.startsWith("WaferID=", Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bCheckWaferID = (bool) strParameter.toInt();
        }
        if (strString.startsWith("DieLoc=", Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bCheckDieLoc = (bool) strParameter.toInt();
        }
        if (strString.startsWith("LongReportMode=",
                                 Qt::CaseInsensitive) == true)
        {
            // Find: site-to-site correlation issue
            pFileAudit->bLongReportMode = (bool) strParameter.toInt();
        }
        // End of section
        if (strString.startsWith("</section_file_audit>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_Datalog
 * \brief Read Template file from disk: Datalog section
 ******************************************************************************/
bool
ReportTemplateIO::ReadTemplateFromDisk_Datalog(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information
    // extracted from Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    CGexCustomReport_Datalog_Section* pDatalog =
        new CGexCustomReport_Datalog_Section();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_DATALOG;
    pNewSection->pDatalog     = pDatalog;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pDatalog->strSectionTitle = strParameter;
        }

        // Datalog type (All tests, failing test, etc.)
        if (strString.startsWith("DatalogType=", Qt::CaseInsensitive) == true)
        {
            pDatalog->iDatalogType = strParameter.toInt();
        }

        // Test list
        if (strString.startsWith("TestList=", Qt::CaseInsensitive) == true)
        {
            pDatalog->strTestList = strParameter;
        }

        // End of section
        if (strString.startsWith("</section_datalog>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }

    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk
 * \brief Write Template to disk
 ******************************************************************************/
bool
ReportTemplateIO::WriteTemplateToDisk(
    ReportTemplate*    reportTemplate,
    QString&           strTemplateName,
    ReportTemplateGui* gui)
{
    mReportTemplate    = reportTemplate;
    mReportTemplateGui = gui;

    QFile file(strTemplateName);  // Write the template file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        return false;
    }
    // Read Template File
    QTextStream hTemplate(&file);  // Assign file handle to data stream

    // Fill file with Database details...
    hTemplate << "<galaxy_template>" << endl;  // Start template marker

    // Write Home page info
    WriteTemplateToDisk_HomePage(hTemplate);

    ReportTemplateSection* pNewSection;
    ReportTemplateSectionIterator iter;
    for (iter  = reportTemplate->Begin();
         iter != reportTemplate->End(); ++iter)
    {
        // Get section data
        pNewSection = *iter;
        switch (pNewSection->iSectionType)
        {
        case GEX_CRPT_WIDGET_AGGREGATE:
            WriteTemplateToDisk_Aggregate(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_WAFMAP:
            WriteTemplateToDisk_Wafmap(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_BINNING:
            WriteTemplateToDisk_Binning(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_PARETO:
            WriteTemplateToDisk_Pareto(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_PEARSON:
            WriteTemplateToDisk_Pearson(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_CORR_GB:
            WriteTemplateToDisk_TesterCorrelationGB(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_PRODUCTION:
            WriteTemplateToDisk_Production(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_GLOBALINFO:
            WriteTemplateToDisk_GlobalInfo(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_FILE_AUDIT:
            WriteTemplateToDisk_FileAudit(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_ER:
            WriteTemplateToDisk_ER(hTemplate, pNewSection);
            break;
        case GEX_CRPT_WIDGET_DATALOG:
            WriteTemplateToDisk_Datalog(hTemplate, pNewSection);
            break;
        default:
            break;
        }
    }

    hTemplate << "</galaxy_template>" << endl;  // End template marker
    file.close();

    return true;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_HomePage
 * \brief Write Template to disk: Home page
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_HomePage(QTextStream& hTemplate)
{
    // Format text to be one line only, and HTML compliant
    QString frontPageText = mReportTemplateGui->textHtml->toPlainText();
    frontPageText.replace("\n", "<br/>");
    frontPageText.replace("<br /><br />", "<br/>");
    mReportTemplate->SetFrontPageText(frontPageText);

    hTemplate << "<section_home_page>" << endl;
    hTemplate << "HomeText=" << mReportTemplate->GetFrontPageText() << endl;
    hTemplate << "HomeLogo=" << mReportTemplate->GetFrontPageImage() << endl;
    hTemplate << "</section_home_page>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Aggregate
 * \brief Write Template to disk: Aggregate section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Aggregate(
    QTextStream&              hTemplate,
    ReportTemplateSection* pNewSection)
{
    GS::Gex::CustomReportTestAggregateSection* pAggregate =
        pNewSection->pAggregate;

    // Start template marker
    hTemplate << "<section_aggregate>" << endl;
    // Section title
    hTemplate << "Title=" << pAggregate->strSectionTitle << endl;
    // Testlist type (all tests, failing tests only, etc.)
    hTemplate << "TestType=" << pAggregate->iTestType << endl;
    // Testlist
    hTemplate << "TestList=" << pAggregate->strRangeList << endl;
    // Include statistics ?
    hTemplate << "bStats=" << QString::number((int) pAggregate->bStats) << endl;
    // Charting mode: one chart per test, overlay all, both
    hTemplate << "ChartMode=" << pAggregate->iChartMode << endl;
    // Histogram type (over data over limits)
    hTemplate << "HistoType=" << pAggregate->iHistogramType << endl;
    // Trend type (over data over limits)
    hTemplate << "TrendType=" << pAggregate->iTrendType << endl;
    // Scatter type (over data over limits)
    hTemplate << "ScatterType=" << pAggregate->iScatterType << endl;
    // Testlist in Y for scatter plot
    hTemplate << "TestListY=" << pAggregate->strRangeListY << endl;
    // BoxPlottype (over data over limits)
    hTemplate << "BoxPlotType=" << pAggregate->iBoxPlotType << endl;
    // Histogram type (over data over limits)
    hTemplate << "ProbabilityPlotType=" << pAggregate->iProbabilityPlotType <<
    endl;
    // Histogram type (over data over limits)
    hTemplate << "BoxplotExType=" << pAggregate->iBoxplotExType << endl;
    // MultiCharts type (over data over limits)
    hTemplate << "MultiChartsType=" << pAggregate->iMultiChartsType << endl;

    // Titles & viewport
    if (pAggregate->bChartTitle)
    {
        hTemplate << "bChartTitle=1" << endl;  // Chart's title
        hTemplate << "ChartTitle=" << pAggregate->strChartTitle << endl;
    }

    if (pAggregate->bLegendX)
    {
        hTemplate << "bLegendX=1" << endl;  // X-axis Legend
        hTemplate << "LegendX=" << pAggregate->strAxisLegendX << endl;
    }
    if (pAggregate->bLogScaleX)
    {
        hTemplate << "bLogScaleX=1" << endl;  // X-axis: Log scale
    }
    if (pAggregate->bLegendY)
    {
        hTemplate << "bLegendY=1" << endl;  // Y-axis Legend
        hTemplate << "LegendY=" << pAggregate->strAxisLegendY << endl;
    }
    if (pAggregate->bLogScaleY)
    {
        hTemplate << "bLogScaleY=1" << endl;  // Y-axis: Log scale
    }
    // Custom Viewport size (if defined)
    if (pAggregate->lfLowX  != -C_INFINITE &&
        pAggregate->lfHighX !=  C_INFINITE)
    {
        hTemplate << "lfLowX=" << QString::number(pAggregate->lfLowX) << endl;
        hTemplate << "lfHighX=" << QString::number(pAggregate->lfHighX) << endl;
    }
    if (pAggregate->lfLowY != -C_INFINITE && pAggregate->lfHighY != C_INFINITE)
    {
        hTemplate << "lfLowY=" << QString::number(pAggregate->lfLowY) << endl;
        hTemplate << "lfHighY=" << QString::number(pAggregate->lfHighY) << endl;
    }

    // Plotting style
    hTemplate << "bBoxBars=" << QString::number((int) pAggregate->bBoxBars) <<
        endl;  // true if draw with BARS (histogram charting only)
    hTemplate << "bFittingCurve=" << QString::number(
        (int) pAggregate->bFittingCurve) <<
        endl;  // true if draw fitting curve / spin
    hTemplate << "bBellCurve=" <<
        QString::number((int) pAggregate->bBellCurve) <<
        endl;  // true if draw Guaussian Bell-curve shape
    hTemplate << "bLines=" << QString::number((int) pAggregate->bLines) <<
        endl;  // true if connect points with a line
    hTemplate << "bSpots=" << QString::number((int) pAggregate->bSpots) <<
        endl;  // true if draw a spot at each data point.
    hTemplate << "iLineWidth=" << pAggregate->iLineWidth <<
        endl;  // Plotting line width
    hTemplate << "iSpotStyle=" << pAggregate->iSpotStyle <<
        endl;  // Spot style: circle, rectangle, diamond, etc.
    hTemplate << "iLineStyle=" << pAggregate->iLineStyle <<
        endl;  // Line style: solid
    hTemplate << "cColor=" << pAggregate->cColor.red() << " " <<
        pAggregate->cColor.green() << " " << pAggregate->cColor.blue() <<
        endl;  // Line color

    int iMakerIndex;
    for (iMakerIndex = 0;
         iMakerIndex < GEX_CRPT_MARKER_TOTALSIZE;
         iMakerIndex++)
    {
        // Marker index & marker width
        hTemplate << "cMarkerColor=" << iMakerIndex << " ";
        hTemplate << pAggregate->icMarkerWidth[iMakerIndex] << " ";
        // Marker color
        hTemplate << pAggregate->cMarkerColor[iMakerIndex].red() << " ";
        hTemplate << pAggregate->cMarkerColor[iMakerIndex].green() << " ";
        hTemplate << pAggregate->cMarkerColor[iMakerIndex].blue() << endl;
    }

    hTemplate << "</section_aggregate>" << endl;  // End template marker
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Wafmap
 * \brief Write Template to disk: Wafer map section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Wafmap(
    QTextStream& hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_WafMap_Section* pWafmap = pNewSection->pWafmap;

    hTemplate << "<section_wafmap>" << endl;
    hTemplate << "Title=" << pWafmap->strSectionTitle << endl;
    hTemplate << "WaferType=" << pWafmap->iWafermapType << endl;
    hTemplate << "TestList=" << pWafmap->strRangeList << endl;
    hTemplate << "</section_wafmap>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Binning
 * \brief Write Template to disk: Binning section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Binning(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_Binning_Section* pBinning = pNewSection->pBinning;

    hTemplate << "<section_binning>" << endl;
    hTemplate << "Title=" << pBinning->strSectionTitle << endl;
    hTemplate << "</section_binning>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Pareto
 * \brief Write Template to disk: Pareto section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Pareto(
    QTextStream& hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_Pareto_Section* pPareto = pNewSection->pPareto;

    hTemplate << "<section_pareto>" << endl;
    hTemplate << "Title=" << pPareto->strSectionTitle << endl;
    hTemplate << "</section_pareto>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Pearson
 * \brief Write Template to disk: Pearson section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Pearson(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_Pearson_Section* pPearson = pNewSection->pPearson;

    hTemplate << "<section_pearson>" << endl;
    hTemplate << "Title=" << pPearson->strSectionTitle << endl;
    hTemplate << "</section_pearson>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_TesterCorrelationGB
 * \brief Write Template to disk: Tester-to-tester Correlation &
 *        GuardBand section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_TesterCorrelationGB(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_TesterCorrelationGB_Section* pTesterCorrelationGB =
        pNewSection->pTesterCorrelationGB;

    hTemplate << "<section_tester_correlation_gb>" << endl;
    hTemplate << "Title=" << pTesterCorrelationGB->strSectionTitle << endl;
    hTemplate << "GuardBandFile=" << pTesterCorrelationGB->strGuardBandFile <<
    endl;
    hTemplate << "</section_tester_correlation_gb>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Production
 * \brief Write Template to disk: Production section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Production(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_Production_Section* pProduction = pNewSection->pProduction;

    hTemplate << "<section_production>" << endl;
    hTemplate << "Title=" << pProduction->strSectionTitle << endl;
    hTemplate << "ChartingType=" << pProduction->iChartingType << endl;
    hTemplate << "</section_production>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_GlobalInfo
 * \brief Write Template to disk: Global Info section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_GlobalInfo(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_GlobalInfo_Section* pGlobalInfo = pNewSection->pGlobalInfo;

    hTemplate << "<section_global_info>" << endl;
    hTemplate << "Title=" << pGlobalInfo->strSectionTitle << endl;
    hTemplate << "</section_global_info>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_FileAudit
 * \brief Write Template to disk: File Audit section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_FileAudit(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    GS::Gex::CustomReportFileAuditSection* pFileAudit = pNewSection->pFileAudit;

    hTemplate << "<section_file_audit>" << endl;
    hTemplate << "Title=" << pFileAudit->strSectionTitle << endl;

    // File Audit options
    hTemplate << "BadRecords=" << pFileAudit->bCheckBadRecords << endl;
    hTemplate << "MissingRecords=" << pFileAudit->bCheckMissingRecords << endl;
    hTemplate << "UnEndRecords=" << pFileAudit->bCheckEndRecords << endl;
    hTemplate << "DuplicateTest=" << pFileAudit->bCheckTestNames << endl;
    hTemplate << "DuplicateLimits=" << pFileAudit->bCheckTestLimits << endl;
    hTemplate << "MissingLimits=" << pFileAudit->bCheckMissingLimits << endl;

    // Data Audit options
    hTemplate << "ZeroRange=" << pFileAudit->bCheckZeroRange << endl;
    hTemplate << "Categories=" << pFileAudit->bCheckCategories << endl;
    hTemplate << "MultiModal=" << pFileAudit->bCheckMultiModal << endl;
    hTemplate << "MeasurementIssue=" << pFileAudit->bCheckMeasurementIssue <<
    endl;
    hTemplate << "RangeIssue=" << pFileAudit->bCheckRangeIssue << endl;
    hTemplate << "SiteToSite=" << pFileAudit->bCheckSiteCorrelation << endl;
    hTemplate << "ShowHistogram=" << pFileAudit->bShowHistogram << endl;
    hTemplate << "SamplesLimit=" << pFileAudit->ldSamplesLimit << endl;
    hTemplate << "CpkLimit=" << pFileAudit->lfCpkLimit << endl;

    // Filter
    QList<GS::Gex::AuditParameterFilter*>::iterator itBegin =
        pFileAudit->cTestFilters.begin();
    QList<GS::Gex::AuditParameterFilter*>::iterator itEnd =
        pFileAudit->cTestFilters.end();

    while (itBegin != itEnd)
    {
        hTemplate << "<ParamFilter>"  << endl;
        hTemplate << "FilterProduct=" << (*itBegin)->strProduct  << endl;
        hTemplate << "FilterType="    << (*itBegin)->iFilterType << endl;
        hTemplate << "FilterString="  << (*itBegin)->strFilter   << endl;
        hTemplate << "</ParamFilter>" << endl;
        itBegin++;
    }

    // Other options
    hTemplate << "ProductID="      << pFileAudit->bCheckProductID << endl;
    hTemplate << "LotID="          << pFileAudit->bCheckLotID     << endl;
    hTemplate << "WaferID="        << pFileAudit->bCheckWaferID   << endl;
    hTemplate << "DieLoc="         << pFileAudit->bCheckDieLoc    << endl;
    hTemplate << "LongReportMode=" << pFileAudit->bLongReportMode << endl;

    hTemplate << "</section_file_audit>" << endl;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_Datalog
 * \brief Write Template to disk: Datalog section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_Datalog(
    QTextStream&                     hTemplate,
    ReportTemplateSection* pNewSection)
{
    CGexCustomReport_Datalog_Section* pDatalog = pNewSection->pDatalog;

    hTemplate << "<section_datalog>"  << endl;
    hTemplate << "Title="             << pDatalog->strSectionTitle << endl;
    hTemplate << "DatalogType="       << pDatalog->iDatalogType    << endl;
    hTemplate << "TestList="          << pDatalog->strTestList     << endl;
    hTemplate << "</section_datalog>" << endl;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_SQL_Filters
 * \brief Read Return SQL Query filters bloc from SQL section
 ******************************************************************************/
QStringList ReportTemplateIO::ReadTemplateFromDisk_SQL_Filters(
    QTextStream& hTemplate)
{
    QString     strString;
    QStringList strFilters;
    do
    {
        // Read one line from file
        strString = hTemplate.readLine();
        strString = strString.trimmed();

        // End of Filters bloc
        if (strString.startsWith("</filters>") == true)
        {
            break;
        }

        // Only process lines with valid data
        if ((strString.isEmpty() == false) &&
            (strString.startsWith("#", Qt::CaseInsensitive) == false))
        {
            strFilters += strString;
        }
    }
    while (hTemplate.atEnd() == false);

    // Return list of filters identified
    return strFilters;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_SQL_Options
 * \brief Read SQL report charting options
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_SQL_Options(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Charting options block
        if (strString.startsWith("</options>"))
        {
            break;
        }

        // Get Bar style
        if (strString.startsWith("BarStyle"))
        {
            pEnterpriseReport->m_strBarStyle =
                pEnterpriseReport->m_YieldWizard_Global_strBarStyle =
                    strParameter;
        }

        // Bar Gradient color ?
        if (strString.startsWith("BarColorGradient"))
        {
            pEnterpriseReport->m_bGradientBarColor =
                pEnterpriseReport->m_YieldWizard_Global_bGradientBarColor =
                    (strParameter.toInt() == 1) ? true : false;
        }

        // Bar color semi-stransparent ?
        if (strString.startsWith("BarColorSemiTransparent"))
        {
            pEnterpriseReport->m_bSemiTransparentBar =
                pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar =
                    (strParameter.toInt() == 1) ? true : false;
        }

        // Get Line style
        if (strString.startsWith("LineStyle"))
        {
            pEnterpriseReport->m_strLineStyle =
                pEnterpriseReport->m_YieldWizard_Volume_strLineStyle =
                    strParameter;
        }

        // Dashed line ?
        if (strString.startsWith("DashedLine"))
        {
            pEnterpriseReport->m_bDashedLine =
                (strParameter.toInt() == 1) ? true : false;
            if (pEnterpriseReport->m_bDashedLine)
            {
                pEnterpriseReport->m_YieldWizard_Volume_strLineProperty =
                    "DashLine";
            }
            else
            {
                pEnterpriseReport->m_YieldWizard_Volume_strLineProperty =
                    "SolidLine";
            }
        }

        // Line spots (if any)
        if (strString.startsWith("LineSpots"))
        {
            pEnterpriseReport->m_strLineSpots =
                pEnterpriseReport->m_YieldWizard_Volume_strLineSpots =
                    strParameter;
        }

        // Background style
        if (strString.startsWith("BackgroundStyle"))
        {
            pEnterpriseReport->m_strBackgroundStyle =
                pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle =
                    strParameter;
        }

        // Default charting color
        if (strString.startsWith("DefaultColor"))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.toLatin1().constData(), "%d %d %d",
                       &iR, &iG, &iB);
            }

            pEnterpriseReport->m_cDefaultColor =
                pEnterpriseReport->m_YieldWizard_Volume_cColor =
                QColor(iR, iG, iB);;
            QString strMessage = "Read Volume color: R=";
            strMessage += QString::number(iR);
            strMessage += " G=";
            strMessage += QString::number(iG);
            strMessage += " B=";
            strMessage += QString::number(iB);
            GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());
        }

        // Background color
        if (strString.startsWith("BackgroundColor"))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.toLatin1().constData(), "%d %d %d",
                       &iR, &iG, &iB);
            }

            pEnterpriseReport->m_cBackgroundColor =
                pEnterpriseReport->m_YieldWizard_Global_cBackgroundColor =
                QColor(iR, iG, iB);;
        }

        // Dislay value markers on charts ?
        if (strString.startsWith("ValueMarkers"))
        {
            pEnterpriseReport->m_bShowValueMarkers =
                (strParameter.toInt() == 1) ? true : false;
            if (pEnterpriseReport->m_bShowValueMarkers)
            {
                pEnterpriseReport->m_YieldWizard_Volume_strDataLabels = "Top";
            }
            else
            {
                pEnterpriseReport->m_YieldWizard_Volume_strDataLabels =
                    "No label";
            }
        }

        // Dump SQL raw data to disk ?
        if (strString.startsWith("DumpRawData"))
        {
            pEnterpriseReport->m_bDumpRawData =
                (strParameter.toInt() == 1) ? true : false;
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_SQL
 * \brief Read Template file from disk: Section SQL
 *        (MySQL/Oracle query & report)
 ******************************************************************************/
bool ReportTemplateIO::ReadTemplateFromDisk_SQL(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information extracted from
    // Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_ER;
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport =
        new GS::Gex::CustomReportEnterpriseReportSection();
    pNewSection->pEnterpriseReport = pEnterpriseReport;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strSectionTitle = strParameter;
        }

        // Get database name
        if (strString.startsWith("DatabaseName=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strDatabaseLogicalName = strParameter;
        }

        // Get report type
        if (strString.startsWith("ReportFunction=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strReportType = strParameter;
        }

        // Get Database type (e-test, f-test, w-sort)
        if (strString.startsWith("DatabaseType=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strDatabaseType = strParameter;
        }

        // Get Time Period and calendar dates
        if (strString.startsWith("TimePeriod=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strTimePeriod = strParameter;
        }
        if (strString.startsWith("CalendarFrom=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_clFromDate =
                QDate::fromString(strParameter, Qt::ISODate);
        }
        if (strString.startsWith("CalendarTo=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_clToDate =
                QDate::fromString(strParameter, Qt::ISODate);
        }

        // Get X-axis type
        if (strString.startsWith("X-AxisField=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strAggregateField = strParameter;
        }

        // Get Y-axis type (Yield report)
        if (strString.startsWith("Y-AxisField=", Qt::CaseInsensitive) == true)
        {
            if (strParameter == "yield only")
            {
                pEnterpriseReport->m_strLeftAxis  = "Serie - Yield";
                pEnterpriseReport->m_strRightAxis = "Disabled (hide axis)";
            }
            else if (strParameter == "volume only")
            {
                pEnterpriseReport->m_strLeftAxis  = "Disabled (hide axis)";
                pEnterpriseReport->m_strRightAxis = "Total - Volume";
            }
            else
            {
                pEnterpriseReport->m_strLeftAxis  = "Serie - Yield";
                pEnterpriseReport->m_strRightAxis = "Total - Volume";
            }
        }

        // Get Bin List
        if (strString.startsWith("BinList=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strBinList = strParameter;
        }

        // Get Charting mode
        if (strString.startsWith("ChartingMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strChartingMode = strParameter;
        }

        // Get split mode info
        if (strString.startsWith("SplitField=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strSplitField = strParameter;
        }
        if (strString.startsWith("LayerMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLayerMode = strParameter;
        }

        //////////////////////////////////////////////////////////////////////
        // Advanced Yield (Yield Wizard)
        //////////////////////////////////////////////////////////////////////

        // AdvancedYield: X-axis field aggregate
        if (strString.startsWith("FieldAggregate=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strAggregateField = strParameter;
        }

        // AdvancedYield: Left-axis type
        if (strString.startsWith("Left-AxisField=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLeftAxis = strParameter;
        }

        // AdvancedYield: Right-axis type
        if (strString.startsWith("Right-AxisField=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strRightAxis = strParameter;
        }

        // Show Binning pareto over maverick lots?
        if (strString.startsWith("ShowBinParetoForMaverick=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_bShowBinParetoOverMaverick =
                (bool) strParameter.toInt();
        }

        // Binning Pareto style
        if (strString.startsWith("BinningParetoStyle"))
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strChartingMode =
                strParameter;
        }

        // Binning Pareto color
        if (strString.startsWith("BinningParetoColor"))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.toLatin1().constData(), "%d %d %d",
                       &iR, &iG, &iB);
            }

            pEnterpriseReport->m_YieldWizard_BinPareto_cColor =
                QColor(iR, iG, iB);;
        }

        // AdvancedYield: Maverick wafer alarm type
        if (strString.startsWith("MaverickWafer_AlarmType=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_EtWt_MaverickWafer_strAlarmType =
                strParameter;
        }

        // AdvancedYield: Maverick wafer alarm count
        if (strString.startsWith("MaverickWafer_AlarmCount=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_EtWt_MaverickWafer_nAlarmCount =
                strParameter.toInt();
        }

        // AdvancedYield: Maverick lot wafer count
        if (strString.startsWith("MaverickLot_WaferCount=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_EtWt_MaverickLot_nWaferCount =
                strParameter.toInt();
        }

        // AdvancedYield: Using softbin?
        if (strString.startsWith("UseSoftBin=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_bSoftBin =
                (bool) strParameter.toInt();
        }


        // AdvancedYield: Serie to chart
        if (strString.startsWith("Serie=", Qt::CaseInsensitive) == true)
        {
            GexDbPlugin_ER_Parts_SerieDef* pSerieDef =
                new GexDbPlugin_ER_Parts_SerieDef(strParameter);
            pEnterpriseReport->m_plistSeries.append(pSerieDef);
        }

        // AdvancedYield: Chart split
        if (strString.startsWith("GraphSplit=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->
                m_YieldWizard_strlFields_GraphSplit.append(strParameter);
        }

        // AdvancedYield: Layer split
        if (strString.startsWith("LayerSplit=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->
                m_YieldWizard_strlFields_LayerSplit.append(strParameter);
        }

        //////////////////////////////////////////////////////////////////////
        // Get Query filters
        //////////////////////////////////////////////////////////////////////
        if (strString.startsWith("<filters>", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strFiltersList =
                ReadTemplateFromDisk_SQL_Filters(hTemplate);
        }

        // Get charting Style & options
        if (strString.startsWith("<options>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_SQL_Options(hTemplate, pEnterpriseReport);
        }

        // End of section
        if (strString.startsWith("</section_sql>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }
    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_DataFilters
 * \brief Read Enterprise report data filters section
 ******************************************************************************/
QStringList ReportTemplateIO::ReadTemplateFromDisk_ER_DataFilters(
    QTextStream& hTemplate)
{
    QString     strString;
    QStringList strFilters;
    do
    {
        // Read one line from file
        strString = hTemplate.readLine();
        strString = strString.trimmed();

        // End of Filters bloc
        if (strString.startsWith("</filters>") == true)
        {
            break;
        }

        // Only process lines with valid data
        if ((strString.isEmpty() == false) &&
            (strString.startsWith("#", Qt::CaseInsensitive) == false))
        {
            strFilters += strString;
        }
    }
    while (hTemplate.atEnd() == false);

    // Return list of filters identified
    return strFilters;
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Data
 * \brief Read Enterprise report data section
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Data(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</data>"))
        {
            break;
        }

        // Get database name
        if (strString.startsWith("DatabaseName=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strDatabaseLogicalName = strParameter;
        }

        // Get Database type (e-test, f-test, w-sort)
        if (strString.startsWith("DatabaseType=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strDatabaseType = strParameter;
        }

        // Get Time Period and calendar dates
        if (strString.startsWith("TimePeriod=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strTimePeriod = strParameter;
        }
        if (strString.startsWith("CalendarFrom=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_clFromDate =
                QDate::fromString(strParameter, Qt::ISODate);
        }
        if (strString.startsWith("CalendarTo=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_clToDate =
                QDate::fromString(strParameter, Qt::ISODate);
        }

        // Get query filters
        if (strString.startsWith("<filters>", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strFiltersList =
                ReadTemplateFromDisk_ER_DataFilters(hTemplate);
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_Std
 * \brief Read Enterprise report specific section for std GUI:
 *        UPH, Yield, Consolidated yield
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_Std(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if ((strString.startsWith("</prod_uph>", Qt::CaseInsensitive) == true) ||
            (strString.startsWith("</prod_yield>",
                                  Qt::CaseInsensitive) == true) ||
            (strString.startsWith("</prod_consolidatedyield>", Qt::CaseInsensitive) == true))
        {
            break;
        }

        // Get X-Axis field
        if (strString.startsWith("Aggregate=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strAggregateField = strParameter;
        }

        // Get Left-Axis field
        if (strString.startsWith("LeftAxis=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLeftAxis = strParameter;
        }

        // Get Right-Axis field
        if (strString.startsWith("RightAxis=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strRightAxis = strParameter;
        }

        // Get ChartingMode field
        if (strString.startsWith("ChartingMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strChartingMode = strParameter;
        }

        // Get BinList field
        if (strString.startsWith("BinList=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strBinList = strParameter;
        }

        // Get SplitField field
        if (strString.startsWith("SplitField=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strSplitField = strParameter;
        }

        // Get LayerMode field
        if (strString.startsWith("LayerMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLayerMode = strParameter;
        }

        // Style section ?
        if (strString.startsWith("<style>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Style(hTemplate, pEnterpriseReport);
        }

        // Advanced section ?
        if (strString.startsWith("<advanced>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Advanced(hTemplate, pEnterpriseReport);
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Std_Style
 * \brief Read Enterprise report style section for std GUI:
 *        UPH, Yield, Consolidated yield
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Std_Style(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</style>", Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Get BarStyle field
        if (strString.startsWith("BarStyle=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strBarStyle = strParameter;
        }

        // Get BarColorGradient field
        if (strString.startsWith("BarColorGradient=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_bGradientBarColor =
                (bool) strParameter.toInt();
        }

        // Get BarColorSemiTransparent field
        if (strString.startsWith("BarColorSemiTransparent=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_bSemiTransparentBar =
                (bool) strParameter.toInt();
        }

        // Get LineStyle field
        if (strString.startsWith("LineStyle=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLineStyle = strParameter;
        }

        // Get Line Property field
        if (strString.startsWith("DashedLine=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_bDashedLine = (bool) strParameter.toInt();
        }

        // Get LineSpots field
        if (strString.startsWith("LineSpots=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLineSpots = strParameter;
        }

        // Get Background style field
        if (strString.startsWith("BackgroundStyle=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strBackgroundStyle = strParameter;
        }

        // Get Background color
        if (strString.startsWith("BackgroundColor="))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.mid(2).toLatin1().constData(),
                       "%02x%02x%02x", &iR, &iG, &iB);
            }
            pEnterpriseReport->m_cBackgroundColor = QColor(iR, iG, iB);;
        }

        // Get ValueMarkers field
        if (strString.startsWith("ValueMarkers=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_bShowValueMarkers =
                (bool) strParameter.toInt();
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Std_Advanced
 * \brief Read Enterprise report advanced section for std GUI:
 *        UPH, Yield, Consolidated yield
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Std_Advanced(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</advanced>", Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Get DumpRawData field
        if (strString.startsWith("DumpRawData=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_bDumpRawData = (bool) strParameter.toInt();
        }

        // Get Default charting color
        if (strString.startsWith("DefaultColor"))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.mid(2).toLatin1().constData(),
                       "%02x%02x%02x", &iR, &iG, &iB);
            }
            pEnterpriseReport->m_cDefaultColor = QColor(iR, iG, iB);;
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard
 * \brief Read Enterprise report specific section for wizard GUI:
 *        Yield wizard
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_YieldWizard(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</prod_advancedyield>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Get X-Axis field
        if (strString.startsWith("Aggregate=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strAggregateField = strParameter;
        }

        // Get LeftAxis field
        if (strString.startsWith("LeftAxis=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strLeftAxis = strParameter;
        }

        // Get RightAxis field
        if (strString.startsWith("RightAxis=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strRightAxis = strParameter;
        }

        // Get DataSource field
        if (strString.startsWith("DataSource=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_strDataSource = strParameter;
        }

        // Get ColorMgt field
        if (strString.startsWith("ColorMgt=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_strColorMgt = strParameter;
        }

        // Get GraphSplit field
        if (strString.startsWith("GraphSplit=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_strlFields_GraphSplit.append(
                strParameter);
        }

        // Get LayerSplit field
        if (strString.startsWith("LayerSplit=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_strlFields_LayerSplit.append(
                strParameter);
        }

        // Get UsSoftBin field
        if (strString.startsWith("UseSoftBin=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_bSoftBin = (strParameter == "1");
        }

        // Global style
        if (strString.startsWith("<style_global>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_YieldWizard_GlobalStyle(
                hTemplate,
                pEnterpriseReport);
        }

        // Volume style
        if (strString.startsWith("<style_volume>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_YieldWizard_VolumeStyle(
                hTemplate,
                pEnterpriseReport);
        }

        // BinPareto style
        if (strString.startsWith("<style_binpareto>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_YieldWizard_BinparetoStyle(
                hTemplate,
                pEnterpriseReport);
        }

        // Serie
        if (strString.startsWith("<serie>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_YieldWizard_Serie(hTemplate,
                                                           pEnterpriseReport);
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_GlobalStyle
 * \brief Read Enterprise report specific section for wizard GUI:
 *        Global style
 ******************************************************************************/
void
ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_YieldWizard_GlobalStyle(
    QTextStream& hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</style_global>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Bar style
        if (strString.startsWith("BarStyle=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_strBarStyle = strParameter;
        }

        // Bar color gradient
        if (strString.startsWith("BarColorGradient=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_bGradientBarColor =
                (strParameter == "1");
        }

        // Bar color semitransparent
        if (strString.startsWith("BarColorSemiTransparent=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar =
                (strParameter == "1");
        }

        // Bar overlapping ratio
        if (strString.startsWith("BarOverlappingRatio=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_nOverlappingRatio =
                strParameter.toInt();
        }

        // Background style
        if (strString.startsWith("BackgroundStyle=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle =
                strParameter;
        }

        // Background color
        if (strString.startsWith("BackgroundColor="))
        {
            // White background in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.mid(2).toLatin1().constData(),
                       "%02x%02x%02x", &iR, &iG, &iB);
            }
            pEnterpriseReport->m_YieldWizard_Global_cBackgroundColor =
                QColor(iR, iG, iB);;
        }

        // Layer policy
        if (strString.startsWith("LayerPolicy=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Global_strLayerPolicy =
                strParameter;
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_VolumeStyle
 * \brief Read Enterprise report specific section for wizard GUI:
 *        Volume style
 ******************************************************************************/
void
ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_YieldWizard_VolumeStyle(
    QTextStream& hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</style_volume>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Charting mode
        if (strString.startsWith("ChartingMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Volume_strChartingMode =
                strParameter;
        }

        // Data labels
        if (strString.startsWith("DataLabels=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Volume_strDataLabels =
                strParameter;
        }

        // Line style
        if (strString.startsWith("LineStyle=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Volume_strLineStyle = strParameter;
        }

        // Line spots
        if (strString.startsWith("LineSpots=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Volume_strLineSpots = strParameter;
        }

        // Line property
        if (strString.startsWith("LineProperty=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_Volume_strLineProperty =
                strParameter;
        }

        // Color
        if (strString.startsWith("Color="))
        {
            // White color in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.mid(2).toLatin1().constData(),
                       "%02x%02x%02x", &iR, &iG, &iB);
            }
            pEnterpriseReport->m_YieldWizard_Volume_cColor =
                QColor(iR, iG, iB);;
            QString strMessage = "Read Volume color (<serie>): R=";
            strMessage += QString::number(iR);
            strMessage += " G=";
            strMessage += QString::number(iG);
            strMessage += " B=";
            strMessage += QString::number(iB);
            GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn
 * \brief Read Enterprise report specific section for wizard GUI:
 *        Binning Pareto style
 ******************************************************************************/
void ReportTemplateIO::
ReadTemplateFromDisk_ER_Prod_YieldWizard_BinparetoStyle(
    QTextStream& hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</style_binpareto>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Pareto title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strTitle = strParameter;
        }

        // Binnings
        if (strString.startsWith("Binnings=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings =
                strParameter;
        }

        // Max categories
        if (strString.startsWith("MaxCategories=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_nMaxCategories =
                strParameter.toInt();
        }

        // Charting mode
        if (strString.startsWith("ChartingMode=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strChartingMode =
                strParameter;
        }

        // Data labels
        if (strString.startsWith("DataLabels=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strDataLabels =
                strParameter;
        }

        // Line style
        if (strString.startsWith("LineStyle=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strLineStyle =
                strParameter;
        }

        // Line spots
        if (strString.startsWith("LineSpots=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots =
                strParameter;
        }

        // Line Property
        if (strString.startsWith("LineProperty=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_YieldWizard_BinPareto_strLineProperty =
                strParameter;
        }

        // Color
        if (strString.startsWith("Color="))
        {
            // White color in case fails reading from file
            int iR = 0xff, iG = 0xff, iB = 0xff;
            if (strParameter.isEmpty() == false)
            {
                sscanf(strParameter.mid(2).toLatin1().constData(),
                       "%02x%02x%02x", &iR, &iG, &iB);
            }
            pEnterpriseReport->m_YieldWizard_BinPareto_cColor =
                QColor(iR, iG, iB);;
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_Serie
 * \brief Read Enterprise report specific section for wizard GUI:
 *        Serie definition
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_YieldWizard_Serie(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    GexDbPlugin_ER_Parts_SerieDef* pSerie = new GexDbPlugin_ER_Parts_SerieDef();
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</serie>", Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Pareto title
        if (strString.startsWith("Name=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strSerieName = strParameter;
        }

        // Binnings
        if (strString.startsWith("Binnings=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strBinnings = strParameter;
        }

        // Plot serie
        if (strString.startsWith("PlotSerie=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_bPlotSerie = (strParameter == "1");
        }

        // Yield exception
        if (strString.startsWith("YieldException=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie->SetYieldException(strParameter);
        }

        // Table data
        if (strString.startsWith("TableData=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strTableData = strParameter;
        }

        // Charting mode
        if (strString.startsWith("ChartingMode=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strChartingMode = strParameter;
        }

        // Data labels
        if (strString.startsWith("DataLabels=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strDataLabels = strParameter;
        }

        // Line style
        if (strString.startsWith("LineStyle=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strLineStyle = strParameter;
        }

        // Line spots
        if (strString.startsWith("LineSpots=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strLineSpots = strParameter;
        }

        // Line Property
        if (strString.startsWith("LineProperty=", Qt::CaseInsensitive) == true)
        {
            pSerie->m_strLineProperty = strParameter;
        }

        // Color
        if (strString.startsWith("Color="))
        {
            bool bOK;
            // Skip '0x' string so to read the RGB color
            pSerie->m_nColor = strParameter.mid(2).toInt(&bOK, 16);
        }
    }
    while (hTemplate.atEnd() == false);

    pEnterpriseReport->m_plistSeries.append(pSerie);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Prod_WyrStandard
 * \brief Read Enterprise report specific section for WYR standard
 ******************************************************************************/
void
ReportTemplateIO::ReadTemplateFromDisk_ER_Prod_WyrStandard(
    QTextStream& hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection*  /*pEnterpriseReport*/)
{
    QString strString, strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</wyr_standard>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }
    }
    while (hTemplate.atEnd() == false);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Genealogy_YieldVsYield
 * \brief Read Enterprise report specific section for Genealogy
 *        Yield vs Yield report
 ******************************************************************************/
void ReportTemplateIO::ReadTemplateFromDisk_ER_Genealogy_YieldVsYield(
    QTextStream&                               hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    GexDbPlugin_ER_Parts_SerieDef* pSerie_X =
        new GexDbPlugin_ER_Parts_SerieDef();
    GexDbPlugin_ER_Parts_SerieDef* pSerie_Y =
        new GexDbPlugin_ER_Parts_SerieDef();

    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</genealogy_yieldvsyield>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Product
        if (strString.startsWith("Product=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_Genealogy_strProduct = strParameter;
        }

        // X-Axis: Testing stage
        if (strString.startsWith("XAxis_TestingStage=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strTestingStage = strParameter;
        }

        // X-Axis: Product
        if (strString.startsWith("XAxis_Product=", Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strProduct = strParameter;
        }

        // X-Axis: Binnings (Pass, Fail, list)
        if (strString.startsWith("XAxis_Binnings=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strBinnings = strParameter;
        }

        // Y-Axis: Testing stage
        if (strString.startsWith("YAxis_TestingStage=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strTestingStage = strParameter;
        }

        // Y-Axis: Product
        if (strString.startsWith("YAxis_Product=", Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strProduct = strParameter;
        }

        // Y-Axis: Binnings (Pass, Fail, list)
        if (strString.startsWith("YAxis_Binnings=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strBinnings = strParameter;
        }

        // Options: Opions_Granularity
        if (strString.startsWith("Opions_Granularity=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_Genealogy_Options_strGranularity =
                strParameter;
        }

        // Style section ?
        if (strString.startsWith("<style>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Style(hTemplate, pEnterpriseReport);
        }

        // Advanced section ?
        if (strString.startsWith("<advanced>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Advanced(hTemplate, pEnterpriseReport);
        }
    }
    while (hTemplate.atEnd() == false);

    // Check if we read required parameters
    if (pSerie_X->m_strTestingStage.isEmpty() ||
        pSerie_X->m_strProduct.isEmpty() ||
        pSerie_Y->m_strTestingStage.isEmpty() ||
        pSerie_Y->m_strProduct.isEmpty())
    {
        delete pSerie_X;
        delete pSerie_Y;
        return;
    }

    // Set serie names
    pSerie_X->m_strSerieName = "X-Axis";
    pSerie_Y->m_strSerieName = "Y-Axis";

    // Add sections
    pEnterpriseReport->m_plistSeries.append(pSerie_X);
    pEnterpriseReport->m_plistSeries.append(pSerie_Y);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER_Genealogy_YieldVsParameter
 * \brief Read Enterprise report specific section for Genealogy
 *        Yield vs Parameter report
 ******************************************************************************/
void ReportTemplateIO::
ReadTemplateFromDisk_ER_Genealogy_YieldVsParameter(
    QTextStream& hTemplate,
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
    QString strString, strParameter;
    GexDbPlugin_ER_Parts_SerieDef* pSerie_X =
        new GexDbPlugin_ER_Parts_SerieDef();
    GexDbPlugin_ER_Parts_SerieDef* pSerie_Y =
        new GexDbPlugin_ER_Parts_SerieDef();

    // X-Axis is a parameter
    pSerie_X->m_bParameterSerie = true;

    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strString    = strString.trimmed();
        strParameter = strString.section('=', 1);

        // End of Data block
        if (strString.startsWith("</genealogy_yieldvsparameter>",
                                 Qt::CaseInsensitive) == true)
        {
            break;
        }

        // Product
        if (strString.startsWith("Product=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_Genealogy_strProduct = strParameter;
        }

        // X-Axis: Testing stage
        if (strString.startsWith("XAxis_TestingStage=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strTestingStage = strParameter;
        }

        // X-Axis: Product
        if (strString.startsWith("XAxis_Product=", Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strProduct = strParameter;
        }

        // X-Axis: Parameter
        if (strString.startsWith("XAxis_Parameter=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_X->m_strParameter = strParameter;
        }

        // Y-Axis: Testing stage
        if (strString.startsWith("YAxis_TestingStage=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strTestingStage = strParameter;
        }

        // Y-Axis: Product
        if (strString.startsWith("YAxis_Product=", Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strProduct = strParameter;
        }

        // Y-Axis: Binnings (Pass, Fail, list)
        if (strString.startsWith("YAxis_Binnings=",
                                 Qt::CaseInsensitive) == true)
        {
            pSerie_Y->m_strBinnings = strParameter;
        }

        // Options: Opions_Granularity
        if (strString.startsWith("Opions_Granularity=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_Genealogy_Options_strGranularity =
                strParameter;
        }

        // Style section ?
        if (strString.startsWith("<style>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Style(hTemplate, pEnterpriseReport);
        }

        // Advanced section ?
        if (strString.startsWith("<advanced>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Std_Advanced(hTemplate, pEnterpriseReport);
        }
    }
    while (hTemplate.atEnd() == false);

    // Check if we read required parameters
    if (pSerie_X->m_strTestingStage.isEmpty() ||
        pSerie_X->m_strProduct.isEmpty() ||
        pSerie_Y->m_strTestingStage.isEmpty() ||
        pSerie_Y->m_strProduct.isEmpty())
    {
        delete pSerie_X;
        delete pSerie_Y;
        return;
    }

    // Set serie names
    pSerie_X->m_strSerieName = "X-Axis";
    pSerie_Y->m_strSerieName = "Y-Axis";

    // Add sections
    pEnterpriseReport->m_plistSeries.append(pSerie_X);
    pEnterpriseReport->m_plistSeries.append(pSerie_Y);
}

/******************************************************************************!
 * \fn ReadTemplateFromDisk_ER
 * \brief Read Template file from disk: Section Enterprise Report
 *        (MySQL/Oracle query & report)
 ******************************************************************************/
bool ReportTemplateIO::ReadTemplateFromDisk_ER(QTextStream& hTemplate)
{
    // Create Aggreate object to receive information extracted from
    // Template file
    ReportTemplateSection* pNewSection =
        new ReportTemplateSection();
    pNewSection->iSectionType = GEX_CRPT_WIDGET_ER;
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport =
        new GS::Gex::CustomReportEnterpriseReportSection();
    pNewSection->pEnterpriseReport = pEnterpriseReport;

    QString strString;
    QString strParameter;
    do
    {
        // Read one line from file
        strString    = hTemplate.readLine();
        strParameter = strString.section('=', 1);

        // Section title
        if (strString.startsWith("Title=", Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strSectionTitle = strParameter;
        }

        // Get report type
        if (strString.startsWith("ReportFunction=",
                                 Qt::CaseInsensitive) == true)
        {
            pEnterpriseReport->m_strReportType = strParameter;
        }

        // Data section ?
        if (strString.startsWith("<data>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Data(hTemplate, pEnterpriseReport);
        }

        // Specific report section for UPH, Yield, Consolidated yield
        if ((strString.startsWith("<prod_uph>",
                                  Qt::CaseInsensitive) == true) ||
            (strString.startsWith("<prod_yield>",
                                  Qt::CaseInsensitive) == true) ||
            (strString.startsWith("<prod_consolidatedyield>",
                                  Qt::CaseInsensitive) == true))
        {
            ReadTemplateFromDisk_ER_Prod_Std(hTemplate, pEnterpriseReport);
        }

        // Specific report section for WYR standard
        if (strString.startsWith("<wyr_standard>", Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_WyrStandard(hTemplate,
                                                     pEnterpriseReport);
        }

        // Specific report section for Yield wizard
        if (strString.startsWith("<prod_advancedyield>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Prod_YieldWizard(hTemplate,
                                                     pEnterpriseReport);
        }

        // Specific report section for Genealogy Yield vs. Yield
        if (strString.startsWith("<genealogy_yieldvsyield>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Genealogy_YieldVsYield(hTemplate,
                                                           pEnterpriseReport);
        }

        // Specific report section for Genealogy Yield vs. Parameter
        if (strString.startsWith("<genealogy_yieldvsparameter>",
                                 Qt::CaseInsensitive) == true)
        {
            ReadTemplateFromDisk_ER_Genealogy_YieldVsParameter(
                hTemplate,
                pEnterpriseReport);
        }

        // End of section
        if (strString.startsWith("</section_enterprise_report>") == true)
        {
            // Append new section to the list
            mReportTemplate->Append(pNewSection);
            return true;
        }
    }
    while (hTemplate.atEnd() == false);

    // Bad escape: section missing closing marker
    delete pNewSection;
    return false;
}

/******************************************************************************!
 * \fn WriteTemplateToDisk_ER
 * \brief Write Template to disk: Enterprise Report section
 ******************************************************************************/
void ReportTemplateIO::WriteTemplateToDisk_ER(
    QTextStream& hTemplate,
    ReportTemplateSection* pNewSection)
{
    GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport =
        pNewSection->pEnterpriseReport;

    hTemplate << "<section_enterprise_report>" << endl;
    hTemplate << "Title=" << pEnterpriseReport->m_strSectionTitle << endl;
    hTemplate << "DatabaseName=" <<
    pEnterpriseReport->m_strDatabaseLogicalName << endl;
    hTemplate << "</section_enterprise_report>" << endl;
}

}
}
