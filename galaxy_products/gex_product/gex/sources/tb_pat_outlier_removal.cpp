//#undef QT3_SUPPORT

///////////////////////////////////////////////////////////
// Toolbox: PAT (Part Average Testing) Outlier Removal
// Support for the Automotive Electronics Concil specs:
// AEC-Q001, JEDEC JESD62, and more rules too!
///////////////////////////////////////////////////////////

#include <QFileDialog>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qcolordialog.h>
#include <qbuttongroup.h>
#include <qvalidator.h>
#include <QPainter>
#include <QMessageBox>
#include <math.h>
#include <gqtl_log.h>

#include "message.h"
#include "gex_shared.h"
#include "browser.h"
#include "browser_dialog.h"
#include "tb_pat_outlier_removal.h"
#include "tb_toolbox.h"	// Examinator ToolBox
#include "report_build.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "patman_lib.h"
#include "cstats.h"
#include "report_options.h"
#include "product_info.h"
#include "engine.h"
#include "pat_engine.h"
#include "pat_definition.h"
#include "pat_rules.h"
#include "pat_global.h"
#include "pat_recipe_io.h"
#include "gex_pat_constants_extern.h"
#include <QHash>

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

////////
// main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CGexReport *		gexReport;				// Handle to report class
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

//extern CPatInfo	*lPatInfo;			// Holds all global pat info required to create & build PAT reports for a given station (real-time final test), or given file (post-processing/wafer-sort)

///////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

bool FillComboBox(QComboBox * pCombo, const char * szTextTab[])
{
    if (!pCombo)
    {
        GSLOG(3, "the combobox to fill is NULL");
        return false;
    }

    if (!szTextTab)
    {
        GSLOG(3, "the given text table to fill combobox is NULL");
        return false;
    }

    int nItem = 0;

    while (szTextTab[nItem])
        pCombo->addItem(szTextTab[nItem++]);
    return true;
}

// Sort by descending Severity score
bool ComparePatResultScore(CPatDefinition* test1, CPatDefinition* test2)
{
    return (test1->m_lSeverityScore > test2->m_lSeverityScore);
}

// Sort by ascending test number
bool ComparePatResultNum(CPatDefinition* test1, CPatDefinition* test2)
{
    return(test1->m_lTestNumber < test2->m_lTestNumber);
}

// Sort by descending outlier count
bool ComparePatNNRTestSummary(const GS::Gex::PATNNRTestSummary& test1, const GS::Gex::PATNNRTestSummary& test2)
{
    return (test1.GetOutlierCount() > test2.GetOutlierCount());
}


// Sort by descending outlier count
bool ComparePatNNRTestRule(const QPair<QString, int>& rule1, const QPair<QString,int>& rule2)
{
    return (rule1.second > rule2.second);
}

///////////////////////////////////////////////////////////
// Save PAT Report to disk...
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_SavePatReport(void)
{
    // Enable/disable some features...
    bool bRefuse = false;

    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()) && !(GS::LPPlugin::ProductInfo::getInstance()->isPATMan()))
        bRefuse = true;

    // Reject Drill?
    if(bRefuse)
    {
    QString m=ReportOptions.GetOption("messages", "upgrade").toString();
    GS::Gex::Message::information("", m);
      //"Your Examinator release doesn't allow this function.\nyou need to upgrade to 'Examinator-PAT'.\n\nPlease contact "+QString(GEX_EMAIL_SALES)
        return;
    }

    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;

    QString strFileName = QString(pFile->getMirDatas().szLot) + "_" + QString(pFile->getWaferMapData().szWaferID) + ".csv";
    QString strSaveAs = QFileDialog::getSaveFileName(this, "Export PAT Report to disk...",
                                                     strFileName, "PAT report (*.csv)");

    // If no file selected, ignore command.
    if(strSaveAs.isEmpty())
        return;

    gexReport->CreateReportFile(strSaveAs);
    gexReport->BuildPP_OutlierRemovalReport("excel",false);
    gexReport->CloseReportFile();
}


/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvOutlierRemoval(BOOL /*bValidSection*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Adv report' page & header
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n\nOutlier Identification & Removal\n");
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Generating HTML report file.
        // Open advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError) return GS::StdLib::Stdf::ReportFile;

        // Most of functions keep writing to the 'hReportFile' file.
        hReportFile = hAdvancedReport;

        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"all_advanced","More Reports: Outlier Identification & Removal");
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvOutlierRemoval(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting trend data!

        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (strOutputFormat=="HTML"))
        {
            fprintf(hReportFile,C_HTML_FOOTER,
              GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() ); // Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill: Holds all outlier tests detected (all parts)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildDPAT_SummaryHtmlTable(bool bStdfDestination)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("List of Outliers identified (test failing Static or Dynamic Outlier limits)\n",bStdfDestination);
        WritePP_OutlierReportLine("Legend:\n",bStdfDestination);
        WritePP_OutlierReportLine("S-Low,Number of failures under Low Static outlier limit\n",bStdfDestination);
        WritePP_OutlierReportLine("S-High,Number of failures over High Static outlier limit\n",bStdfDestination);
        WritePP_OutlierReportLine("Test,Name,Samples to ignore,Outliers to Keep,Outlier Limits set,N factor,T factor,Severity score,S-Low,S-High,Dynamic: Low fails: Far - Medium - Near,Dynamic: High fails: Near - Medium - Far,Total Fails\n",bStdfDestination);
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        char	szString[255];
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\"><b>Test</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\"><b>Name</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Samples<br>to ignore</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outliers<br>to keep</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outlier<br>Limits set</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Severity<br>score</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Static Fails<br>Low / High</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Dynamic<br>Low fails</b><br>Far-Medium-Near</td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Dynamic<br>High fails</b><br>Near-Medium-Far</td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Total<br>PAT Fails</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("Outlier: Tests");
    }
}

void CGexReport::BuildNNRSummaryPage(const QString &lPageName, bool lSplitPatReportPages, bool lStdfDestination)
{
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    SplitPatReportWebPage(lPageName);

    // Title + bookmark
    if(!lStdfDestination)
    {
        WritePageBreak();

        if (lOutputFormat=="CSV")
            WritePP_OutlierReportLine("\n\nSummary: Tests failing NNR\n", lStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile, "nnr_summary","Summary: Tests failing NNR");

        SetPowerPointSlideName("Summary: NNR Outliers");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (lOutputFormat=="CSV")
            WritePP_OutlierReportLine("\n\nSummary: Tests failing NNR\n", lStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"nnr_summary\"><font color=\"#006699\">Summary: Tests failing NNR<br></font></h1>\n\n", lStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a><br>\n", lStdfDestination);

    // Get PAT context pointer
    CPatInfo * lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    // Build NNR summary table if any NNR outliers found
    if (lPatInfo && lPatInfo->pNNR_OutlierTests.count())
    {
        // NNR variables
        QMap<QPair<int,int>, GS::Gex::PATNNRTestSummary>    lNNRTestSummary;

        // Read list of NNR tests, and count failing rate for each (using a Qmap)
        CPatOutlierNNR * ptNNR_TestFailure = NULL;
        for (int lIdx = 0; lIdx < lPatInfo->pNNR_OutlierTests.count(); ++lIdx)
        {
            ptNNR_TestFailure = lPatInfo->pNNR_OutlierTests.at(lIdx);

            GS::Gex::PATNNRTestSummary  lTestSummary(ptNNR_TestFailure->mTestNumber,
                                                     ptNNR_TestFailure->mPinmap,
                                                     ptNNR_TestFailure->mTestName);
            QPair<int,int>              lTestID(ptNNR_TestFailure->mTestNumber, ptNNR_TestFailure->mPinmap);

            if (lNNRTestSummary.contains(lTestID))
                lTestSummary = lNNRTestSummary.value(lTestID);

            lTestSummary.AddOutlier(ptNNR_TestFailure->mNNRRule.GetRuleName(), 1);

            lNNRTestSummary.insert(lTestID, lTestSummary);
        };

        // Sort (make pareto) of NNR results: top of table is highest NNR fail count
        QList<GS::Gex::PATNNRTestSummary> lNNRTestList = lNNRTestSummary.values();

        qSort(lNNRTestList.begin(), lNNRTestList.end(), ComparePatNNRTestSummary);

        // Build NNR summary table
        BuildNNRSummaryTable(lNNRTestList, lStdfDestination);

        if(!lStdfDestination)
        {
            WritePageBreak();

            if (lOutputFormat=="CSV")
                WritePP_OutlierReportLine("\n\nDetails: Rules failing NNR per test\n", lStdfDestination);
            else
            {
                WriteHtmlSectionTitle(hReportFile, "nnr_details","Details: Rules failing NNR per test");
                WritePP_OutlierReportLine("<br>\n");
            }
        }
        else
        {
            // Creating report buffer to store into STDF.DTR records
            if (lOutputFormat=="CSV")
                WritePP_OutlierReportLine("\n\nDetails: Rules failing NNR per test\n", lStdfDestination);
            else
                WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"nnr_details\"><font color=\"#006699\">Details: Rules failing NNR per test<br></font></h1>\n\n", lStdfDestination);
        }

        // Build NNR Detailed table
        BuildNNRDetailedTable(lNNRTestList, lStdfDestination);
    }
    else
    {
        if(lOutputFormat=="CSV")
            WritePP_OutlierReportLine("\nNo test failed any of the NNR rules!\n", lStdfDestination);
        else
            WritePP_OutlierReportLine("\n<br>No test failed any of the NNR rules!\n<br>", lStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n", lStdfDestination);
}

/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill: Holds all NNR outliers (pareto)
/////////////////////////////////////////////////////////////////////////////
void    CGexReport::BuildNNRSummaryTableHeader(bool lStdfDestination)
{
    QString lOutputFormat =   m_pReportOptions->GetOption("output", "format").toString();

    if (lOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("List of NNR Outliers\n", lStdfDestination);
        WritePP_OutlierReportLine("Legend:\n", lStdfDestination);
        WritePP_OutlierReportLine("Test,Name,NNR Fails\n", lStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        static char	szString[255];
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,lStdfDestination);

        WritePP_OutlierReportLine("<tr>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"15%%\"><b>Test</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"70%%\"><b>Name</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\" width=\"15%%\"><b>Total<br>NNR Fails</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("</tr>\n", lStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!lStdfDestination)
            SetPowerPointSlideName("NNR Outliers");
    }
}

void CGexReport::BuildNNRSummaryTable(const QList<GS::Gex::PATNNRTestSummary> &lNNRTestList, bool bStdfDestination)
{
    QString     lHyperlink;
    QString     lHyperLinkDetail;
    QString     lCell;
    // Get output format
    QString     lOutputFormat =   m_pReportOptions->GetOption("output", "format").toString();

    // Create table header
    BuildNNRSummaryTableHeader(bStdfDestination);

    GS::Gex::PATNNRTestSummary  lNNRTest;
    for (int lIdx = 0; lIdx < lNNRTestList.count(); ++lIdx)
    {
        lNNRTest = lNNRTestList.at(lIdx);
        if (lOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Test#
            WritePP_OutlierReportLine(QString::number(lNNRTest.GetTestNumber()) + QString(","), bStdfDestination);
            // Test name
            WritePP_OutlierReportLine(lNNRTest.GetTestName() + QString(","), bStdfDestination);
            // NNR fail count
            WritePP_OutlierReportLine(QString::number(lNNRTest.GetOutlierCount()) + "\n", bStdfDestination);
        }
        else if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);

            // Hyperlink name
            if (lOutputFormat=="DOC"||lOutputFormat=="PDF"||lOutputFormat=="PPT"||lOutputFormat=="ODT")
            {
                // Flat html to endup in one file
                lHyperlink = "href=\"#HistoT";
                lHyperlink += QString::number(lNNRTest.GetTestNumber());
                lHyperlink += "\"";
            }
            else
            {
                // Regular html page
                lHyperlink = "href=\"#_gex_drill--drill_chart=adv_multichart--data=";
                lHyperlink += QString::number(lNNRTest.GetTestNumber());
                lHyperlink += "\"";
            }

            lHyperLinkDetail = "<a href=\"#DetailsT";
            lHyperLinkDetail += QString::number(lNNRTest.GetTestNumber());
            lHyperLinkDetail += "\"><img src=\"../images/link.png\"  border=\"0\"></a>";

            // Test# + hyperlink
            lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"15%%\">%1 <b><a %2>%3</a></b></td>\n")
                    .arg(lHyperLinkDetail).arg(lHyperlink).arg(lNNRTest.GetTestNumber());
            WritePP_OutlierReportLine(lCell, bStdfDestination);

            // Test name
            lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"70%%\">%1</td>\n").arg(lNNRTest.GetTestName());
            WritePP_OutlierReportLine(lCell, bStdfDestination);

            // NNR fail count
            if (lOutputFormat=="DOC"||lOutputFormat=="PDF"||lOutputFormat=="PPT"||lOutputFormat=="ODT")
            {
                // Flat html to endup in one file
                lHyperlink = "href=\"#HistoT" + QString::number(lNNRTest.GetTestNumber());
                lHyperlink += "\"";
            }
            else
            {
                // Regular html page
                lHyperlink = "href=\"#_gex_drill--drill_3d=wafer_nnr";
                lHyperlink += "--g=0" ;		// group#
                lHyperlink += "--f=0";		// file#
                lHyperlink += "--Test=" + QString::number(lNNRTest.GetTestNumber());	// test#
                lHyperlink += "--Pinmap=" + QString::number(lNNRTest.GetPinmap());	// Pinmap#
                lHyperlink += "\"";
            }

            // NNR count + hyperlink
            lCell = QString("<td bgcolor=\"#F8F8F8\" align=\"center\" width=\"15%%\"<b><a %1> %2</a></b></td>\n")
                    .arg(lHyperlink).arg(lNNRTest.GetOutlierCount());
            WritePP_OutlierReportLine(lCell, bStdfDestination);
        }
    }

    // close HTML codes.
    if(m_pReportOptions->isReportOutputHtmlBased())
        WritePP_OutlierReportLine("</table>\n<br>",bStdfDestination);
}

void CGexReport::BuildNNRDetailedTableHeader(bool lStdfDestination)
{
    QString lOutputFormat =   m_pReportOptions->GetOption("output", "format").toString();

    if (lOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("Detailed List of NNR Outliers\n", lStdfDestination);
        WritePP_OutlierReportLine("Legend:\n", lStdfDestination);
        WritePP_OutlierReportLine("Test,Name,NNR Rule,NNR Algo,NNR Factor, NNR Fails\n", lStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        QString lCell = QString("<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %1pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n").arg(iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(lCell,lStdfDestination);

        WritePP_OutlierReportLine("<tr>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"10%%\"><b>Test</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"35%%\"><b>Name</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"15%%\"><b>NNR Rule</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"20%%\"><b>NNR Algo</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"10%%\"><b>NNR Factor</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"10%%\"><b>NNR Fails</b></td>\n", lStdfDestination);
        WritePP_OutlierReportLine("</tr>\n", lStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!lStdfDestination)
            SetPowerPointSlideName("NNR Outliers per test");
    }
}

void CGexReport::BuildNNRDetailedTable(const QList<GS::Gex::PATNNRTestSummary> &lNNRTestList, bool lStdfDestination)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
        return;

    // Create table header
    BuildNNRDetailedTableHeader(lStdfDestination);

    GS::Gex::PATNNRTestSummary      lNNRTest;
    QList<QPair<QString, int> >     lNNRRules;
    QString                         lCell;
    QString                         lOutputFormat =   m_pReportOptions->GetOption("output", "format").toString();

    for (int lIdx = 0; lIdx < lNNRTestList.count(); ++lIdx)
    {
        lNNRTest = lNNRTestList.at(lIdx);
        lNNRRules.clear();

        QMap<QString, int>::const_iterator    itBegin;

        for (itBegin = lNNRTest.GetRuleSummary().constBegin(); itBegin != lNNRTest.GetRuleSummary().constEnd(); ++itBegin)
        {
            lNNRRules.append(QPair<QString, int>(itBegin.key(), itBegin.value()));
        }

        qSort(lNNRRules.begin(), lNNRRules.end(), ComparePatNNRTestRule);

        for (int lRule = 0; lRule < lNNRRules.count(); ++lRule)
        {
            int lIdx = lPatInfo->GetRecipeOptions().FindNNRRuleByName(lNNRRules.at(lRule).first);

            if (lIdx >= 0)
            {
                const CNNR_Rule& lNNRRUle = lPatInfo->GetRecipeOptions().GetNNRRules().at(lIdx);

                if (lOutputFormat=="CSV")
                {
                    // Test#
                    WritePP_OutlierReportLine(QString("%1,").arg(lNNRTest.GetTestNumber()), lStdfDestination);

                    // Test name
                    WritePP_OutlierReportLine(QString("%1,").arg(lNNRTest.GetTestName()), lStdfDestination);

                    // Rule Name
                    WritePP_OutlierReportLine(QString("%1,").arg(lNNRRUle.GetRuleName()), lStdfDestination);

                    // Rule Algo
                    WritePP_OutlierReportLine(QString("%1,").arg(GS::Gex::PAT::GetNNRAlgorithmName(lNNRRUle.GetAlgorithm())),
                                              lStdfDestination);

                    // Rule Factor
                    WritePP_OutlierReportLine(QString("%1,").arg(lNNRRUle.GetNFactor()), lStdfDestination);

                    // Fail count
                    WritePP_OutlierReportLine(QString("%1\n").arg(lNNRRules.at(lRule).second), lStdfDestination);
                }
                else
                {
                    WritePP_OutlierReportLine("<tr>\n", lStdfDestination);

                    if (lRule == 0)
                    {
                        // Test# + hyperlink
                        lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"10%%\"><a name=\"DetailsT%1\"</a> %2</td>\n")
                                .arg(lNNRTest.GetTestNumber()).arg(lNNRTest.GetTestNumber());
                        WritePP_OutlierReportLine(lCell, lStdfDestination);

                        // Test name
                        lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"35%%\">%1</td>\n").arg(lNNRTest.GetTestName());
                        WritePP_OutlierReportLine(lCell, lStdfDestination);
                    }
                    else
                    {
                        WritePP_OutlierReportLine("<td bgcolor=\"#F8F8F8\" width=\"10%%\"></td>\n", lStdfDestination);
                        WritePP_OutlierReportLine("<td bgcolor=\"#F8F8F8\" width=\"35%%\"></td>\n", lStdfDestination);
                    }

                    // Rule Name
                    lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"15%%\">%1</td>\n").arg(lNNRRUle.GetRuleName());
                    WritePP_OutlierReportLine(lCell, lStdfDestination);

                    // Rule Algo
                    lCell = QString("<td bgcolor=\"#F8F8F8\" width=\"20%%\">%1</td>\n")
                            .arg(GS::Gex::PAT::GetNNRAlgorithmName(lNNRRUle.GetAlgorithm()));
                    WritePP_OutlierReportLine(lCell, lStdfDestination);

                    // Rule Factor
                    lCell = QString("<td align=\"center\" bgcolor=\"#F8F8F8\" width=\"10%%\">%1</td>\n").arg(lNNRRUle.GetNFactor());
                    WritePP_OutlierReportLine(lCell, lStdfDestination);

                    // Fail count
                    lCell = QString("<td align=\"center\" bgcolor=\"#F8F8F8\" width=\"10%%\">%1</td>\n").arg(lNNRRules.at(lRule).second);
                    WritePP_OutlierReportLine(lCell, lStdfDestination);
                    WritePP_OutlierReportLine("</tr>\n", lStdfDestination);
                }
            }
            else
            {
                // NNR Rule not found in the recipe
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("NNR Rule %1 not found while generating the report").arg(lNNRRules.at(lIdx).first)
                      .toLatin1().constData());
            }
        }
    }

    // close HTML codes.
    if(m_pReportOptions->isReportOutputHtmlBased())
        WritePP_OutlierReportLine("</table>\n<br>", lStdfDestination);
}

/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill: Holds all IDDQ-Delta outliers (pareto)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildIDDQ_Delta_SummaryHtmlTable(bool bStdfDestination)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("List of IDDQ-Delta Outliers\n",bStdfDestination);
        WritePP_OutlierReportLine("Legend:\n",bStdfDestination);
        WritePP_OutlierReportLine("Test,Name,Total Fails\n",bStdfDestination);
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        char	szString[255];
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"15%%\"><b>Test</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\" width=\"75%%\"><b>Name</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\" width=\"15%%\"><b>Total<br>Fails</b></td>\n",bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("IDDQ-Delta Outliers");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill: Reports reticle yield levels
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildReticleHtmlTable(bool bStdfDestination)
{
    QString	strString;
    QString strOutputFormat = m_pReportOptions->GetOption("output", "format").toString();
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
        return;

    // Retrieve wafer map where Reticle information are located
    const CWaferMap * lWaferReticle = lPatInfo->GetReticleStepInformation();
    if (lWaferReticle == NULL && lPatInfo->GetRecipeOptions().GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FILE)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Unable to find Reticle Information from data file");
        return;
    }

    unsigned int lReticleSizeX;

    switch (lPatInfo->GetRecipeOptions().GetReticleSizeSource())
    {
        case PATOptionReticle::RETICLE_SIZE_FILE:
            lReticleSizeX = lWaferReticle->GetReticleWidth();
            break;

        default:
            lReticleSizeX = lPatInfo->GetRecipeOptions().GetReticleSizeX();
    }

    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("Line   \\   Row",bStdfDestination);
        for(unsigned int lIdx = 1; lIdx <= lReticleSizeX; lIdx++)
        {
            strString = "," + QString::number(lIdx);
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }
        WritePP_OutlierReportLine("\n",bStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, cell spacing=1 (or 2 if creating PDF file)
        char	szString[255];
        int		iTableSize = 80*lReticleSizeX;	// Each cell is 80 pixels
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"%d\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iTableSize,iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\"><b>Line \\ Row</b></td>\n",bStdfDestination);

        for(unsigned int lIdx = 1; lIdx <= lReticleSizeX; lIdx++)
        {
            strString = "<td align=\"center\" bgcolor=\"#CCECFF\"><b>" + QString::number(lIdx) + "</b></td>\n";
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("Reticle Yield");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill: Holds details about each part
// detected as an outlier
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildDPAT_PartsHtmlTable(bool bStdfDestination)
{
    char	szString[255];
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("LotID,WaferID,PartID,PatBin,DieX,DieY,Site,Tests failing PAT\n",bStdfDestination);
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>LotID</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>PartID</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>PatBin</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieX</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieY</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Site</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"60%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Tests failing PAT Limits (Outliers)</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("Outliers: Parts");
    }
}

void CGexReport::BuildMVPATRulesChartsPage(const QStringList& lRules,
                                              bool lSplitPatReportPages,
                                              bool lStdfDestination)
{
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();
    QString lPageCharts;
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    // No MVPAT charts if CSV report or html report stored in STDF file
    if (lPatInfo == NULL || lStdfDestination || lOutputFormat == "CSV")
        return;

    // Create a map of the mv charts page to create
    QMap<int, QPair<int, QString> > lMVChartRules;
    int                             lRuleIndex = 0;
    for (int lIdx = 0; lIdx < lPatInfo->GetMultiVariateRules().count(); ++lIdx)
    {
        lRuleIndex = lRules.indexOf(lPatInfo->GetMultiVariateRules().at(lIdx).GetName());
        if (lRuleIndex != -1)
            lMVChartRules.insert(lIdx, QPair<int, QString>(lRuleIndex, lPatInfo->GetMultiVariateRules().at(lIdx).GetName()));
    }

    QMapIterator<int, QPair<int, QString> > itChartRules(lMVChartRules);
    int lPreviousIndex = -1;
    while(itChartRules.hasNext())
    {
        itChartRules.next();

        QStringList lCharts = lPatInfo->GetMVPATRuleChartsPath(itChartRules.value().second);

        // Build the page name
        lPageCharts = "advanced_mvpat_charts" + QString("_rule%1").arg(itChartRules.value().first) + ".htm";

        SplitPatReportWebPage(lPageCharts);

        // Title + bookmark
        WritePageBreak();

        if (ReportOptions.isReportOutputHtmlBased())
            WriteHtmlSectionTitle(hReportFile, lPageCharts, "Charts: Rules failing MV-PAT");

        if (lOutputFormat == "HTML")
        {
            QString lLink;

            WritePP_OutlierReportLine("<table border=\"0\" width=\"98%%\">\n", lStdfDestination);
            WritePP_OutlierReportLine("<tr>\n", lStdfDestination);

            if (lPreviousIndex >= 0)
            {
                lLink = "<td width=\"50%%\"><a href=\"";
                lLink += QString("advanced_mvpat_charts_rule%1.htm").arg(lPreviousIndex);
                lLink += "\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n";
                WritePP_OutlierReportLine(lLink, lStdfDestination);
            }
            else
                WritePP_OutlierReportLine("<td width=\"50%%\">&nbsp;</td>\n", lStdfDestination);

            if (itChartRules.hasNext())
            {
                lLink = "<td width=\"50%%\"><p align=\"right\"><a href=\"";
                lLink += QString("advanced_mvpat_charts_rule%1.htm").arg(itChartRules.peekNext().value().first);
                lLink += "\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"31\"></a></td>\n";
                WritePP_OutlierReportLine(lLink, lStdfDestination);
            }
            else
                WritePP_OutlierReportLine("<td width=\"50%%\">&nbsp;</td>\n", lStdfDestination);

            WritePP_OutlierReportLine("</tr>\n</table>\n", lStdfDestination);

            // Keep last rule index used
            lPreviousIndex = itChartRules.value().first;
        }

        // Create bookmark
        QString lTestHeader;

        lTestHeader = "<br><h2 align=\"left\"><font color=\"#006699\">";
        lTestHeader += itChartRules.value().second;
        lTestHeader += "</font></h2><br>\n";
        WritePP_OutlierReportLine(lTestHeader, lStdfDestination);

        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        QString lLine = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" ";
        lLine += "style=\"font-size: " + QString::number(iHthmSmallFontSizePixels);
        lLine += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
        WritePP_OutlierReportLine(lLine.toLatin1().constData(), lStdfDestination);

        SetPowerPointSlideName("Charts: Failing rules");

        for (int lIdx = 0; lIdx < lCharts.count(); ++lIdx)
        {
            if (lIdx % 2 == 0)
                WritePP_OutlierReportLine("<tr>\n", lStdfDestination);

            QFileInfo   lFileInfo(lCharts.at(lIdx));
            QString     lToFile = ReportOptions.strReportDirectory + "/images/" + lFileInfo.fileName();

            // Remove destination file if already exists
            if (QFile::exists(lToFile))
                QFile::remove(lToFile);

            // Copy chart file from temp folder to report folder
            QFile::copy(lCharts.at(lIdx), lToFile);

            // Remove chart file from temp folder
            QFile::remove(lCharts.at(lIdx));

            lLine = QString("<td width=\"50%%\"><img src=\"../images/%1\"></img></td>\n").arg(formatHtmlImageFilename(lFileInfo.fileName()));
            WritePP_OutlierReportLine(lLine, lStdfDestination);

            if (lIdx % 2 == 1 || lIdx == lCharts.count()-1)
                WritePP_OutlierReportLine("</tr>\n", lStdfDestination);
        }

        // Close HTML table
        WritePP_OutlierReportLine("</table><br>\n", lStdfDestination);

        // If standard Web page (splitted): Write 'Return' hyperlink
        if(lSplitPatReportPages)
            WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n", lStdfDestination);
    }
}

void CGexReport::BuildMVPATRulesDetailsPage(const QString &lPageName, bool lSplitPatReportPages, bool lStdfDestination)
{
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    SplitPatReportWebPage(lPageName);

    // Title + bookmark
    if(!lStdfDestination)
    {
        WritePageBreak();
        if (lOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: MV-PAT Rules definitions\n",
                                      lStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"mvpat_definitions","Summary: MV-PAT Rules definitions");

        SetPowerPointSlideName("MV Rules definitions");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (lOutputFormat=="CSV")
            WritePP_OutlierReportLine("\n\nSummary: MV-PAT Rules definitions\n", lStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"mvpat_definitions\"><font color=\"#006699\">Summary: MV-PAT Rules definitions<br></font></h1>\n\n", lStdfDestination);
    }
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(lPatInfo && lPatInfo->GetMultiVariateRules().count())
        BuildMVPATRulesDetailsTable(lStdfDestination);
    else
        WritePP_OutlierReportLine("No MV-PAT rules defined in the recipe file\n\n",
                                  lStdfDestination);

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n", lStdfDestination);
}

void CGexReport::BuildMVPATRulesDetailsTableHeader(bool lStdfDestination)
{
    QString lOutputFormat = m_pReportOptions->GetOption("output", "format").toString();

    if (lOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("List of Outliers identified (rules failing MV PAT Outlier limits)\n",lStdfDestination);
        WritePP_OutlierReportLine("Rule name,Type,Bin,Outlier Distance,Custom Distance,Test list,Enabled,Notes\n",lStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        QString lLine = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" ";
        lLine += "style=\"font-size: " + QString::number(iHthmSmallFontSizePixels);
        lLine += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
        WritePP_OutlierReportLine(lLine.toLatin1().constData(), lStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"10%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Rule name</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" bgcolor=\"#CCECFF\"><b>Type</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" bgcolor=\"#CCECFF\"><b>Bin</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Outlier<br>Distance</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Custom<br>Distance</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"60%%\" bgcolor=\"#CCECFF\"><b>Test List</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" bgcolor=\"#CCECFF\"><b>Enabled</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" bgcolor=\"#CCECFF\"><b>Notes</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!lStdfDestination)
            SetPowerPointSlideName("MV-Rule definition: Detail");
    }
}

void CGexReport::BuildMVPATRulesDetailsTable(bool lStdfDestination)
{
    int     lLineCount      = 0;
    int		lTableLine      = 15;
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();
    QString lLabel;
    QString lCell;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
        return;
    QList<GS::Gex::PATMultiVariateRule>::ConstIterator itBegin  = lPatInfo->GetMultiVariateRules().constBegin();
    QList<GS::Gex::PATMultiVariateRule>::ConstIterator itEnd    = lPatInfo->GetMultiVariateRules().end();

    for(; itBegin < itEnd; ++itBegin)
    {
        if (lOutputFormat == "CSV")
        {
            for (int lIdx = 0; lIdx < (*itBegin).GetMVTestData().count(); ++lIdx)
            {
                if (lIdx == 0)
                {
                    WritePP_OutlierReportLine((*itBegin).GetName() + ",", lStdfDestination);	// Rule Name
                    WritePP_OutlierReportLine((*itBegin).GetTypeString() + ",", lStdfDestination);	// Type
                    WritePP_OutlierReportLine(QString("%1,").arg((*itBegin).GetBin()), lStdfDestination); // Associated bin
                    WritePP_OutlierReportLine((*itBegin).GetOutlierDistanceModeString() + ",", lStdfDestination);      // Outlier distance set
                    WritePP_OutlierReportLine(QString("%1,").arg((*itBegin).GetCustomDistance()), lStdfDestination);	// Custom distance

                    lLabel = QString::number((*itBegin).GetMVTestData().at(lIdx).GetTestNumber());
                    if ((*itBegin).GetMVTestData().at(lIdx).GetPinIdx() != -1)
                        lLabel += "." + QString::number((*itBegin).GetMVTestData().at(lIdx).GetPinIdx());

                    lLabel += " " + (*itBegin).GetMVTestData().at(lIdx).GetTestName();
                    WritePP_OutlierReportLine(lLabel + ",", lStdfDestination); // Test

                    if ((*itBegin).GetEnabled())
                        WritePP_OutlierReportLine("Enabled,", lStdfDestination);	// Enabled
                    else
                        WritePP_OutlierReportLine("Disabled,", lStdfDestination);	// Disabled
                    /// TODO HTH
                    WritePP_OutlierReportLine(",",lStdfDestination);            // MV PAT Notes
                }
                else
                {
                    WritePP_OutlierReportLine(",", lStdfDestination);	// Rule Name
                    WritePP_OutlierReportLine(",", lStdfDestination);	// Type
                    WritePP_OutlierReportLine(",", lStdfDestination);   // Associated bin
                    WritePP_OutlierReportLine(",", lStdfDestination);   // Outlier distance set
                    WritePP_OutlierReportLine(",", lStdfDestination);	// Custom distance

                    lLabel = QString::number((*itBegin).GetMVTestData().at(lIdx).GetTestNumber());
                    if ((*itBegin).GetMVTestData().at(lIdx).GetPinIdx() != -1)
                        lLabel += "." + QString::number((*itBegin).GetMVTestData().at(lIdx).GetPinIdx());

                    lLabel += " " + (*itBegin).GetMVTestData().at(lIdx).GetTestName();
                    WritePP_OutlierReportLine(lLabel + ",", lStdfDestination); // Test
                    WritePP_OutlierReportLine(",", lStdfDestination);	// Enabled
                    WritePP_OutlierReportLine(",",lStdfDestination);    // MV PAT Notes
                }
            }
        }
        else if (m_pReportOptions->isReportOutputHtmlBased())
        {
            if (lLineCount == 0)
                BuildMVPATRulesDetailsTableHeader(lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", lStdfDestination);
            lCell = QString("<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg((*itBegin).GetName());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg((*itBegin).GetTypeString());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg((*itBegin).GetBin());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg((*itBegin).GetOutlierDistanceModeString());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg((*itBegin).GetCustomDistance());
            WritePP_OutlierReportLine(lCell, lStdfDestination);

            lCell = "<td width=\"40%%\" bgcolor=\"#F8F8F8\" valign=\"top\">";
            for (int lIdx = 0; lIdx < (*itBegin).GetMVTestData().count(); ++lIdx)
            {
                lLabel = QString::number((*itBegin).GetMVTestData().at(lIdx).GetTestNumber());
                if ((*itBegin).GetMVTestData().at(lIdx).GetPinIdx() != -1)
                    lLabel += "." + QString::number((*itBegin).GetMVTestData().at(lIdx).GetPinIdx());

                lLabel += " " + (*itBegin).GetMVTestData().at(lIdx).GetTestName();
                lCell += lLabel + "<br>";
            }
            lCell += "</td>\n";
            WritePP_OutlierReportLine(lCell, lStdfDestination);

            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg(((*itBegin).GetEnabled())?"Enabled":"Disabled");
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            /// TODO HTH
            lCell = QString("<td width=\"20%%\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n")
                    .arg("");
            WritePP_OutlierReportLine(lCell, lStdfDestination);

            ++lLineCount;

            // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
            if (!lStdfDestination && (lOutputFormat=="PPT")
                && ((lLineCount % lTableLine) == 0))
            {
                // close table & goto next page
                fprintf(hReportFile,"</table>\n");
                WritePageBreak();

                // Reset line counter in page (so will reopen table if needed)
                lLineCount = 0;

                // Number of lines in table (page 2 and following) is 20
                lTableLine = 20;
            }



        }
    }

}

void CGexReport::BuildMVPATRulesSummaryPage(const QString &lPageName,
                                            bool lSplitPatReportPages,
                                            bool lStdfDestination)
{
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    SplitPatReportWebPage(lPageName);

    // Title + bookmark
    if(!lStdfDestination)
    {
        WritePageBreak();
        if (lOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Rules failing MV-PAT\n",
                                      lStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"mvpat_summary","Summary: Rules failing MV-PAT");

        SetPowerPointSlideName("Outliers: Failing tests");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (lOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Rules failing MV-PAT\n", lStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"mvpat_summary\"><font color=\"#006699\">Summary: Rules failing MV-PAT<br></font></h1>\n\n", lStdfDestination);
    }

    QStringList lSummaryRules;
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(lPatInfo && lPatInfo->GetMVOutliers().count())
        lSummaryRules = BuildMVPATRulesSummaryTable(lStdfDestination);
    else
        WritePP_OutlierReportLine("No rule failed any of the Outlier limits!!\n\n",
                                  lStdfDestination);

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n", lStdfDestination);

    if (lSummaryRules.isEmpty() == false)
        BuildMVPATRulesChartsPage(lSummaryRules, lSplitPatReportPages, lStdfDestination);

}

void CGexReport::BuildMVPATRulesSummaryTableHeader(bool lStdfDestination)
{
    QString lOutputFormat = m_pReportOptions->GetOption("output", "format").toString();

    if (lOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("List of Outliers identified (rules failing MV PAT Outlier limits)\n",lStdfDestination);
        WritePP_OutlierReportLine("Rule,Outlier Distance,Severity Score,MV-PAT fails: Near - Medium - Far,Total MV-PAT Fails\n",lStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        QString lLine = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" ";
        lLine += "style=\"font-size: " + QString::number(iHthmSmallFontSizePixels);
        lLine += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
        WritePP_OutlierReportLine(lLine.toLatin1().constData(), lStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\"><b>Rule</b></td>\n",lStdfDestination);
//        WritePP_OutlierReportLine("<td bgcolor=\"#CCECFF\"><b>Test List</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Outlier<br>Distance</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Severity<br>score</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>MV-PAT fails</b><br>Near-Medium-Far</td>\n",lStdfDestination);
        WritePP_OutlierReportLine("<td align=\"center\" bgcolor=\"#CCECFF\"><b>Total<br>MV-PAT Fails</b></td>\n",lStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!lStdfDestination)
            SetPowerPointSlideName("Outlier: Tests");
    }
}

QStringList CGexReport::BuildMVPATRulesSummaryTable(bool lStdfDestination)
{
    int     lLineCount      = 0;
    int		lTableLine      = 15;
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
        return QStringList();
    tdIterMVPATOutliers     itMVOutlier(lPatInfo->GetMVOutliers());
    QHash<QString, GS::Gex::PATMVRuleSummary>   lMVRulesSummary;

    // Create the MV failing rules summary
    while (itMVOutlier.hasNext())
    {
        itMVOutlier.next();

        for (int lIdx = 0; lIdx < itMVOutlier.value().GetFailingRules().count(); ++lIdx)
        {
            QString lRuleName = itMVOutlier.value().GetFailingRules().at(lIdx).GetRuleName();

            GS::Gex::PATMVRuleSummary lRuleSummary(lRuleName);

            if (lMVRulesSummary.contains(lRuleName))
                lRuleSummary = lMVRulesSummary.value(lRuleName);

            switch(itMVOutlier.value().GetFailingRules().at(lIdx).GetSeverity())
            {
                case GS::Gex::PAT::Near:
                    lRuleSummary.AddNearOutlier(1);
                    break;

                case GS::Gex::PAT::Medium:
                    lRuleSummary.AddMediumOutlier(1);
                    break;

                case GS::Gex::PAT::Far:
                    lRuleSummary.AddFarOutlier(1);
                    break;

                default:
                    GSLOG(SYSLOG_SEV_WARNING, "Unknown outlier distance set found for the MV PAT rule");
                    break;
            }

            lMVRulesSummary.insert(lRuleName, lRuleSummary);
        }
    }

    QList<GS::Gex::PATMVRuleSummary>                    lSummary;
    QHash<QString, GS::Gex::PATMVRuleSummary>::Iterator itSummary = lMVRulesSummary.begin();

    for (; itSummary != lMVRulesSummary.end(); ++itSummary)
        lSummary.append(itSummary.value());

    // order list by severity score
    qSort(lSummary);

    QString lCell;
    QString lPageCharts;
    QString lOutlierSet;
    QStringList lSummaryRules;
    for (int lIdx = 0; lIdx < lSummary.count(); ++lIdx)
    {
        lOutlierSet = "Unknown";

        // keep the ordered rules
        lSummaryRules.append(lSummary.at(lIdx).GetRuleName());

        // build the charts page corresponding
        lPageCharts = "advanced_mvpat_charts" + QString("_rule%1").arg(lIdx) + ".htm";

        for (int lRule = 0; lRule < lPatInfo->GetMultiVariateRules().count(); ++lRule)
        {
            if (lPatInfo->GetMultiVariateRules().at(lRule).GetName() == lSummary.at(lIdx).GetRuleName())
            {
                lOutlierSet = lPatInfo->GetMultiVariateRules().at(lRule).GetOutlierDistanceModeString();

                if (lPatInfo->GetMultiVariateRules().at(lRule).GetOutlierDistanceMode() == GS::Gex::PAT::Custom)
                {
                    lOutlierSet += " (";
                    lOutlierSet += QString::number(lPatInfo->GetMultiVariateRules().at(lRule).GetCustomDistance());
                    lOutlierSet += ")";
                }
            }
        }

        if (lOutputFormat == "CSV")
        {
            WritePP_OutlierReportLine(lSummary.at(lIdx).GetRuleName() + ",", lStdfDestination);	// Rule Name
//            WritePP_OutlierReportLine(",", bStdfDestination);
            WritePP_OutlierReportLine(lOutlierSet + ",", lStdfDestination);	// Outlier distance set
            WritePP_OutlierReportLine(QString::number(lSummary.at(lIdx).GetSeverityScore()) + ",",
                                      lStdfDestination);	// Severity score

            lCell = QString::number(lSummary.at(lIdx).GetNearOutlierCount()) + " - ";
            lCell += QString::number(lSummary.at(lIdx).GetMediumOutlierCount()) + " - ";
            lCell += QString::number(lSummary.at(lIdx).GetFarOutlierCount());
            WritePP_OutlierReportLine(lCell + ",", lStdfDestination);	// Outlier found
            WritePP_OutlierReportLine(QString::number(lSummary.at(lIdx).GetOutlierCount()) + ",",
                                      lStdfDestination);	// Total outliers
        }
        else if(m_pReportOptions->isReportOutputHtmlBased())
        {
            if (lLineCount == 0)
                BuildMVPATRulesSummaryTableHeader(lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", lStdfDestination);
            lCell = QString("<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\" valign=\"top\"><a href=\"%1\">%2</a></td>\n")
                    .arg(lPageCharts).arg(lSummary.at(lIdx).GetRuleName());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(lOutlierSet);
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(lSummary.at(lIdx).GetSeverityScore());
            WritePP_OutlierReportLine(lCell, lStdfDestination);

            lCell = "<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">";
            lCell += QString::number(lSummary.at(lIdx).GetNearOutlierCount()) + " - ";
            lCell += QString::number(lSummary.at(lIdx).GetMediumOutlierCount()) + " - ";
            lCell += QString::number(lSummary.at(lIdx).GetFarOutlierCount()) + "</td>\n";
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(lSummary.at(lIdx).GetOutlierCount());
            WritePP_OutlierReportLine(lCell, lStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", lStdfDestination);

            ++lLineCount;

            // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
            if (!lStdfDestination && (lOutputFormat=="PPT")
                && ((lLineCount % lTableLine) == 0))
            {
                // close table & goto next page
                fprintf(hReportFile,"</table>\n");
                WritePageBreak();

                // Reset line counter in page (so will reopen table if needed)
                lLineCount = 0;

                // Number of lines in table (page 2 and following) is 20
                lTableLine = 20;
            }
        }
    }

    if(lLineCount > 0)
        WritePP_OutlierReportLine("</table><br>\n", lStdfDestination);

    return lSummaryRules;
}

void CGexReport::BuildMVPATFailingPartsPage(const QString& lPageName,
                                            bool lSplitPatReportPages,
                                            bool lStdfDestination)
{
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    SplitPatReportWebPage(lPageName);

    if (lOutputFormat=="CSV")
        WritePP_OutlierReportLine("\nParts failing MV-PAT rules:\n", lStdfDestination);
    else
    {
        // Section Title + bookmark
        if(!lStdfDestination)
            WriteHtmlSectionTitle(hReportFile,"part_mvpat_fails","Parts failing MV-PAT rules");
        else
            WritePP_OutlierReportLine(
               "<h1 align=\"left\"><a name=\"part_mvpat_fails\"></a><font color=\"#006699\">Parts failing MV-PAT rules<br></font></h1>\n\n",
               lStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
    {
        WritePP_OutlierReportLine(
                    "<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",
                    lStdfDestination);
    }

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(lPatInfo && lPatInfo->GetMVOutliers().count())
        BuildMVPATFailingPartsTable(lStdfDestination);
    else
        WritePP_OutlierReportLine("No Part failed any of the MV Outlier limits!\n\n",
                                  lStdfDestination);

    // close HTML codes.
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        if(lPatInfo && lPatInfo->GetMVOutliers().count())
        {
            // Legend
            // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
            QString lLine;
            lLine = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" ";
            lLine += "style=\"font-size: " + QString::number(iHthmSmallFontSizePixels);
            lLine += "dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
            WritePP_OutlierReportLine(lLine,lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>Legend</b></td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"90%%\" bgcolor=\"#CCECFF\">Outlier type / Description</td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>N</b></td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Near</b> outlier (between 'Near' and 'Medium' MV outlier distances)</td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>M</b></td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Medium</b> outlier (between 'Medium' and 'Far' MV outlier distances)</td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>F</b></td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Far</b> outlier (Over the 'Far' MV outlier distances)</td>\n",
                                      lStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",lStdfDestination);

            WritePP_OutlierReportLine("</table>\n",lStdfDestination);
            WritePP_OutlierReportLine("<br><br>\n",lStdfDestination);
        }
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(lSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",
                                  lStdfDestination);

}

void CGexReport::BuildMVPATFailingPartsTableHeader(bool bStdfDestination)
{
    QString strOutputFormat = m_pReportOptions->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("LotID,Wafer,PartID,PatBin,DieX,DieY,Site,Rules failing MV-PAT limits\n",
                                  bStdfDestination);
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        QString lLine = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" ";
        lLine += "style=\"font-size: " + QString::number(iHthmSmallFontSizePixels);
        lLine += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
        WritePP_OutlierReportLine(lLine.toLatin1().constData(), bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>LotID</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>PartID</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>PatBin</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieX</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieY</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Site</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("<td width=\"50%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Rules failing MV-PAT limits (Outliers)</b></td>\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("Outliers: MV-PAT Parts");
    }
}

void CGexReport::BuildMVPATFailingPartsTable(bool bStdfDestination)
{
    int     lLineCount      = 0;
    int		lTableLine      = 15;
    QString lOutputFormat   = m_pReportOptions->GetOption("output", "format").toString();

    CGexGroupOfFiles *  lGroup = getGroupsList().isEmpty()? NULL : getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
        return;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
        return;
    tdIterMVPATOutliers     itMVOutlier(lPatInfo->GetMVOutliers());
    QString                 lFailureDetails;
    QString                 lLotID      = lFile->getMirDatas().szLot;
    QString                 lWaferID    = lFile->getWaferMapData().szWaferID;
//    qSort(lPatInfo->mMVOutliers.begin(), lPatInfo->mMVOutliers.end(), CPatOutlierPart::lessThan);

    while (itMVOutlier.hasNext())
    {
        itMVOutlier.next();

        if (lLineCount == 0)
            BuildMVPATFailingPartsTableHeader(bStdfDestination);

        if (lOutputFormat == "CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            for (int lIdx = 0; lIdx < itMVOutlier.value().GetFailingRules().count(); ++lIdx)
            {
                QString lLine;

                WritePP_OutlierReportLine(lLotID + ",",bStdfDestination);	// LotID
                WritePP_OutlierReportLine("'" + lWaferID + "',",bStdfDestination);	// WaferID
                WritePP_OutlierReportLine(itMVOutlier.value().GetPartID() + ",",
                                          bStdfDestination);	// PartID
                WritePP_OutlierReportLine(QString::number(itMVOutlier.value().GetSoftBin()) + ",",
                                          bStdfDestination);				// PAT Bin
                WritePP_OutlierReportLine(QString::number(itMVOutlier.value().GetCoordinate().GetX()) + ",",
                                          bStdfDestination);					// Die X
                WritePP_OutlierReportLine(QString::number(itMVOutlier.value().GetCoordinate().GetY()) + ",",
                                          bStdfDestination);					// Die Y
                WritePP_OutlierReportLine(QString::number(itMVOutlier.value().GetSite()) + ",",
                                          bStdfDestination);					// Testing site


                lLine = itMVOutlier.value().GetFailingRules().at(lIdx).GetRuleName();
                lLine += " ";

                switch (itMVOutlier.value().GetFailingRules().at(lIdx).GetSeverity())
                {
                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:
                        lLine += "N";
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:
                        lLine += "M";
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:
                        lLine += "F";
                        break;

                    default:
                        lFailureDetails += "?";
                        break;
                }

                lLine += " ( ";
                lLine += QString::number(itMVOutlier.value().GetFailingRules().at(lIdx).GetZScore());
                lLine += " )";

                WritePP_OutlierReportLine(lLine, bStdfDestination);	// List of tests
                WritePP_OutlierReportLine("\n", bStdfDestination);
            }
        }
        else if(m_pReportOptions->isReportOutputHtmlBased())
        {
            QString lCell;

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            lCell = QString("<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(lLotID);
            WritePP_OutlierReportLine(lCell, bStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(lWaferID);
            WritePP_OutlierReportLine(lCell, bStdfDestination);

            // Hyperlink ot SBIN-Wafermap (only created is standard HTML report)
            lCell = "<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">";
            if (lOutputFormat == "HTML")
            {
                // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
                int iGroupID = gexReport->getGroupForSite(itMVOutlier.value().GetSite());
                if(iGroupID < 0)
                    iGroupID=0;

                // Regular html page: Build Link to SoftBin wafermap
                lCell += "<a href=\"#_gex_drill--drill_3d=wafer_sbin";
                lCell += "--g=" + QString::number(iGroupID);		// group#
                lCell += "--f=0";		// file#
                lCell += "--Test=0";	// test#: not used
                lCell += "--Pinmap=0";		// pinmap#: not used
                lCell += "--DieX=" + QString::number(itMVOutlier.value().GetCoordinate().GetX());	// DieX position
                lCell += "--DieY=" + QString::number(itMVOutlier.value().GetCoordinate().GetY());	// DieY position
                lCell += "\">";
                lCell += itMVOutlier.value().GetPartID();
                lCell += "</a>";
            }
            else
                lCell += itMVOutlier.value().GetPartID();

            lCell += "</td>\n";

            WritePP_OutlierReportLine(lCell, bStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(itMVOutlier.value().GetSoftBin());
            WritePP_OutlierReportLine(lCell, bStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(itMVOutlier.value().GetCoordinate().GetX());
            WritePP_OutlierReportLine(lCell, bStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(itMVOutlier.value().GetCoordinate().GetY());
            WritePP_OutlierReportLine(lCell, bStdfDestination);
            lCell = QString("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>\n").arg(itMVOutlier.value().GetSite());
            WritePP_OutlierReportLine(lCell,bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"50%%\" align=\"left\" bgcolor=\"#F8F8F8\">",
                                      bStdfDestination);

            for (int lIdx = 0; lIdx < itMVOutlier.value().GetFailingRules().count(); ++lIdx)
            {
                lCell = itMVOutlier.value().GetFailingRules().at(lIdx).GetRuleName();
                lCell += " ";

                switch (itMVOutlier.value().GetFailingRules().at(lIdx).GetSeverity())
                {
                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:
                        lCell += "N";
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:
                        lCell += "M";
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:
                        lCell += "F";
                        break;

                    default:
                        lFailureDetails += "?";
                        break;
                }

                lCell += " ( ";

            // Build hyperlink for wafermap interactive
//            if (lOutputFormat == "HTML")
//            {
//                // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
//                iGroupID = gexReport->getGroupForSite(pOutlierPart->iSite);
//                if(iGroupID < 0)
//                    iGroupID=0;

//                // Wafer map link
//                lLine	= "<a href=\"#_gex_drill--drill_3d=wafer_param_range";
//                lLine	+= "--g=" + QString::number(iGroupID);		// group#
//                lLine	+= "--f=0";         // file#
//                lLine	+= "--Test=";		// test#
//                lLine	+= QString::number(itMVOutlier.value().get);
//                lLine	+= "--Pinmap=";     // pin#
//                lLine	+= QString::number(cFailTest.mPinIndex);
//                lLine	+= "--DieX=" + QString::number(pOutlierPart->iDieX);	// DieX position
//                lLine	+= "--DieY=" + QString::number(pOutlierPart->iDieY);	// DieY position
//                lLine	+= "\">";
//                lLine   += QString::number(itMVOutlier.value().GetZScore());
//                lLine	+= "</a>";
//            }
//            else
                    lCell   += QString::number(itMVOutlier.value().GetFailingRules().at(lIdx).GetZScore());

                lCell += " )";

                // Write outlier test
                WritePP_OutlierReportLine(lCell, bStdfDestination);

                // Insert line break if multiple failing tests in part.
                if(lIdx > 1)
                    WritePP_OutlierReportLine("<br>",bStdfDestination);
            }

            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ++lLineCount;

            // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
            if (!bStdfDestination && (lOutputFormat=="PPT")
                && ((lLineCount % lTableLine) == 0))
            {
                // close table & goto next page
                fprintf(hReportFile,"</table>\n");
                WritePageBreak();

                // Reset line counter in page (so will reopen table if needed)
                lLineCount = 0;

                // Number of lines in table (page 2 and following) is 20
                lTableLine = 20;
            }
        }
    }

    if(lLineCount > 0)
        WritePP_OutlierReportLine("</table><br>\n", bStdfDestination);
}

bool sortStaticPatDefinition(const CPatDefinition * pLeft, const CPatDefinition * pRight)
{
    return (pLeft->m_lSeverityScore > pRight->m_lSeverityScore);
}

///////////////////////////////////////////////////////////
// Implement lessThan method to sort object
///////////////////////////////////////////////////////////
bool CPatOutlierPart::lessThan(const CPatOutlierPart * pItem1, const CPatOutlierPart * pItem2)
{
    return (*pItem1) < (*pItem2);
}


/////////////////////////////////////////////////////////////////////////////
// Build Post-Processing Outier Removal report
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WritePP_OutlierReportLine(QString strLine,bool bStdfDestination/*=true*/)
{
    if(bStdfDestination)
    {
        // Writing line for STDF record....so save it into buffer, do not write in file yet!
        strPatTraceabilityReport += strLine;
    }
    else
    {
        // Write line to file
        if(hReportFile)
            fprintf(hReportFile,"%s",strLine.toLatin1().constData());
    }
}

/////////////////////////////////////////////////////////////////////////////
// Splits PAT report Web page in smaller pages if needed
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::SplitPatReportWebPage(const QString &strPage)
{
    // If no page name specified, simply quietly return!
    if(strPage.isEmpty())
        return;
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    // If not standard Web page, simply return
    if (strOutputFormat=="CSV") // if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        return;
    if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
        return;	// Flat HTML (to end-up as PDF, Word, etc...do NOT split page!)
    if(hReportFile == NULL)
        return;	// Not currently writing a report (maybe writing string into STDF file!).

    // Close current page
    fprintf(hReportFile,"</body>\n</html>\n");
    fclose(hReportFile);

    // Open new HTML page
    QString strPath;
    strPath.sprintf("%s/pages/%s",
                    m_pReportOptions->strReportDirectory.toLatin1().constData(),
                    strPage.toLatin1().constData());
    hReportFile = fopen(strPath.toLatin1().constData(),"w");
    hAdvancedReport = hReportFile;

    // Write page break (ignored if not writing a flat HTML document)
    WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black
}

static int GetTotalFailures(unsigned long lTestNumber,
                            int lPinIndex,
                            const QString& lTestName,
                            tdListPatOutlierParts lOutlierParts,
                            int iSiteID=-1,
                            bool bCountDPAT=false)
{
    int	iTotalFailures=0;
    QList<CPatFailingTest>::iterator		itPart;
    CPatFailingTest							cFailTest;
    tdIterPatOutlierParts					itOutlierParts(lOutlierParts);
    CPatOutlierPart *						pOutlierPart = NULL;
    QString                                 lTestKey = ReportOptions.GetOption("dataprocessing", "duplicate_test").toString();
    int                                     lTestKeyOption;
    bool                                    lMatch = false;

    if (lTestKey == "merge")
        lTestKeyOption = TEST_MERGE_NUMBER;
    else if (lTestKey == "merge_name")
        lTestKeyOption = TEST_MERGE_NAME;
    else if (lTestKey == "no_merge")
        lTestKeyOption = TEST_NO_MERGE;
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Invalid Test key option detected: %1").arg(lTestKey).toLatin1().constData());

        return iTotalFailures;
    }

    // Scan all failing parts (SPAT & DPAT)
    while(itOutlierParts.hasNext())
    {
        pOutlierPart = itOutlierParts.next();

        // Focus on given site only
        if((iSiteID < 0) || (pOutlierPart->iSite == iSiteID))
        {
            for(itPart = pOutlierPart->cOutlierList.begin(); itPart != pOutlierPart->cOutlierList.end(); ++itPart )
            {
                cFailTest = *itPart;

                // match the failing test depending on test key option
                switch (lTestKeyOption)
                {
                    case TEST_MERGE_NUMBER:
                    default:
                        lMatch = (lTestNumber == cFailTest.mTestNumber && lPinIndex == cFailTest.mPinIndex);
                        break;

                    case TEST_MERGE_NAME:
                        lMatch = (lPinIndex == cFailTest.mPinIndex &&
                                  lTestName.compare(cFailTest.mTestName) == 0);
                        break;

                    case TEST_NO_MERGE:
                        lMatch =(lTestNumber == cFailTest.mTestNumber && lPinIndex == cFailTest.mPinIndex &&
                                 lTestName.compare(cFailTest.mTestName) == 0);
                        break;
                }

                // Check if given test failed
                if(lMatch)
                {
                    // Check failure type.
                    switch(cFailTest.mFailureMode)
                    {
                        // SPAT failure
                        case GEX_TPAT_FAILMODE_STATIC_L:	// Low Static
                        default:	// Static failure.
                        case GEX_TPAT_FAILMODE_STATIC_H:	// High static
                            // If we're looking at SPAT failures, then update fail count
                            if(!bCountDPAT)
                                iTotalFailures++;
                            break;

                        // DPAT failure
                        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:	// Failure on 'Near' outlier limits
                        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:// Failure on 'Medium' outlier limits
                        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:// Failure on 'Far' outlier limits
                            // If we're looking at DPAT failures, then update fail count
                            if(bCountDPAT)
                                iTotalFailures++;
                            break;
                    }

                }
            }
        }
    };

    // Return total failures for given test & site
    return iTotalFailures;
}

/////////////////////////////////////////////////////////////////////////////
// Build Post-Processing Outier Removal report
/////////////////////////////////////////////////////////////////////////////
void CGexReport::BuildPP_OutlierRemovalReport(QString strReportType,bool bStdfDestination/*=true*/)
{
    char			szString[1024];
    const char*     ptColor=0;		// To keep handle to a HTML colot string.
    long			ldValue=0, ldTotalCount=0;
    double			lfValue=0;
    bool			bSplitPatReportPages=false;	// Set to 'true' if the report created is standard Web HTML (so to split report in 4 pages: one per PAT report section)
    QString			strStatsDetails;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
    {
         GSLOG(3, "PatInfo NULL");
         return;
    }

    if(bStdfDestination)
    {
        // Prepare report to write in STDF file; format is either CSV or HTML!
        if(strReportType.startsWith("excel", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output","format","CSV"); //pReportOptions->iOutputFormat = GEX_OPTION_OUTPUT_CSV;
        else
            m_pReportOptions->SetOption("output","format","HTML"); //pReportOptions->iOutputFormat = GEX_OPTION_OUTPUT_HTML;
    }
    else
    {
        // Decode the report output format to create
        m_pReportOptions->SetOption("output", "format", "HTML");
        if(strReportType.startsWith("excel", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output", "format", "CSV");
        if(strReportType.startsWith("html", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output", "format", "HTML");
        if(strReportType.startsWith("word", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output", "format", "DOC");
        if(strReportType.startsWith("ppt", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output", "format", "PPT");
        if(strReportType.startsWith("pdf", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output", "format", "PDF");
        if(strReportType.startsWith("odt", Qt::CaseInsensitive))
            m_pReportOptions->SetOption("output","format","ODT");
    }

    // Reset buffer
    strPatTraceabilityReport = "";

    // Get pointer to first group & first file (we always have them exist)
//    const CWaferMap * lWaferReticle = NULL;
    CTest *ptTestCell=0;
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;

    QString strString;
    QString strValue;
    QString strPatPage_FailingParts="";
    QString strPatPage_MVPAT_FailingParts="";
    QString strPatPage_NNR_FailingParts="";
    QString strPatPage_IDDQ_Delta_FailingParts="";
    QString strPatPage_SpatLimits="";
    QString strPatPage_DpatLimits="";
    QString strPatPage_Summary="";
    QString	strPatPage_MVPAT_Summary="";
    QString	strPatPage_MVPAT_Details="";
    QString	strPatPage_NNR_Summary="";
    QString strPatPage_IDDQ_Delta_Summary="";
    QString strPatPage_Reticle_Summary="";
    QString strPatPage_detailed_reticle ="";
    QString strPatPage_WarningLog="";

    // Global info
    QString strProduct = pFile->getMirDatas().szPartType;
    QString strLotID   = pFile->getMirDatas().szLot;
    QString strSubLot  = pFile->getMirDatas().szSubLot;
    QString strWaferID = pFile->getWaferMapData().szWaferID;
    strProduct = strProduct.trimmed();

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        WritePP_OutlierReportLine("############################################################\n",
                                  bStdfDestination);
        WritePP_OutlierReportLine("\n# Recipe file," + lPatInfo->GetRecipeFilename(),
                                  bStdfDestination);

        if(lPatInfo->strDataSources.count() > 0)
        {
            // Show input data file(s) used
            if(lPatInfo->strDataSources.count() > 1)
            {
                // Multiple files merged...
                strString = "# Test data files (merged),";
                int nIndex;
                for(nIndex = 0; nIndex < lPatInfo->strDataSources.count(); nIndex++)
                    strString += lPatInfo->strDataSources[nIndex] + ",";
            }
            else
            {
                strString = lPatInfo->strDataSources.isEmpty()? "" : lPatInfo->strDataSources.first();
                strString = "# Test data files (merged)," + strString;
            }
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().bCustomPatLib)
            WritePP_OutlierReportLine("\n# Custom PAT Lib," + lPatInfo->GetRecipeOptions().strCustomPatLibName,bStdfDestination);
        if(strProduct.isEmpty()==false)
            WritePP_OutlierReportLine("\n# Product," + strProduct,bStdfDestination);
        if(strLotID.isEmpty()==false)
            WritePP_OutlierReportLine("\n# Lot," + strLotID,bStdfDestination);
        if(strSubLot.isEmpty()==false)
            WritePP_OutlierReportLine("\n# SubLot," + strSubLot,bStdfDestination);
        if(strWaferID.isEmpty()==false)
            WritePP_OutlierReportLine("\n# Wafer,'" + strWaferID,bStdfDestination);
        WritePP_OutlierReportLine("\n# Ignore test with few samples," + QString::number(lPatInfo->GetRecipeOptions().iMinimumSamples),bStdfDestination);
        if(lPatInfo->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled && lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk >= 0)
            WritePP_OutlierReportLine("\n# Remove outliers until Test Cpk," + QString::number(lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk),bStdfDestination);

        if(lPatInfo->GetRecipeOptions().bScanGoodPartsOnly)
            WritePP_OutlierReportLine("\n# Bin outliers,Found in GOOD parts only",bStdfDestination);
        else
            WritePP_OutlierReportLine("\n# Bin outliers,Found in All  parts",bStdfDestination);

        WritePP_OutlierReportLine("\n# Minimum outliers to fail a part," + QString::number(lPatInfo->GetRecipeOptions().iMinimumOutliersPerPart),bStdfDestination);

        // Total parts tested
        if(pFile->getWaferMapData().bStripMap)
            ldValue = pFile->ldTotalPartsSampled;				// Final test data
        else
            ldValue = pFile->getWaferMapData().iTotalPhysicalDies;	// Wafersort data
        if(ldValue <=0)
            ldValue = 1;

        WritePP_OutlierReportLine("\n# Total parts tested," + QString::number(ldValue),bStdfDestination);

        // DPAT fail count + yield loss
        ldTotalCount = lPatInfo->m_lstOutlierParts.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%), (including retests)",ldTotalCount,lfValue);
        // 6179
        WritePP_OutlierReportLine("\n# Parts failing PAT," + QString(szString), bStdfDestination);

        // MVPAT fail count + yield loss
        ldTotalCount = lPatInfo->GetMVOutliers().count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100)
            lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%),",ldTotalCount, lfValue);
        // 6179
        WritePP_OutlierReportLine("\n# Parts failing MV-PAT," + QString(szString), bStdfDestination);

        // NNR fail count + yield loss
        ldTotalCount = lPatInfo->mNNROutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing NNR," + QString(szString),bStdfDestination);

        // IDDQ-Delta fail count + yield loss
        ldTotalCount = lPatInfo->mIDDQOutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing IDDQ-Delta," + QString(szString),bStdfDestination);

        // Reticle fail count + yield loss
        ldTotalCount = lPatInfo->mReticleOutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing Reticle," + QString(szString),bStdfDestination);

        // Clustering fail count + yield loss
        ldTotalCount = lPatInfo->mClusteringOutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing Clustering," + QString(szString),bStdfDestination);

        // GDBN fail count + yield loss
        ldTotalCount = lPatInfo->mGDBNOutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing GDBN," + QString(szString),bStdfDestination);

        // ZPAT fail count + yield loss
        ldTotalCount = lPatInfo->mZPATOutliers.count();
        lfValue = (100.0*ldTotalCount)/(double)ldValue;
        if(lfValue > 100) lfValue = 100;
        sprintf(szString,"%ld (%.2lf%%)",ldTotalCount,lfValue);
        WritePP_OutlierReportLine("\n# Parts failing ZPAT," + QString(szString),bStdfDestination);

        // Total Test processed in PAT
        bool lAddNewLog =
            (QString(qgetenv("GS_SHAPEDETECTION_LEGACY")) != "1") ?
            true : false;

        if (lAddNewLog)
        {
            ldTotalCount = 0;
            QHash<QString, CPatDefinition*>::iterator   lItPATDef;
            CPatDefinition *                            lPatDef = NULL;
            for(lItPATDef = lPatInfo->GetRecipe().GetUniVariateRules().begin();
                lItPATDef != lPatInfo->GetRecipe().GetUniVariateRules().end(); ++lItPATDef)
            {
                lPatDef = *lItPATDef;
                if (!lPatDef->IsTestDisabled())
                    ++ldTotalCount;
            }
            WritePP_OutlierReportLine("\n# Total tests PAT processed," + QString::number(ldTotalCount),bStdfDestination);
        }

        WritePP_OutlierReportLine("\n-----------------------------------------------------------\n",bStdfDestination);
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Global information
        sprintf(szString,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\">\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Recipe file</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",
                lPatInfo->GetRecipeFilename().toLatin1().constData());
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // Show input data file(s) used
        if(lPatInfo->strDataSources.count() > 1)
        {
            // Multiple files merged...
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Test data files (merged)</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">");
            WritePP_OutlierReportLine(szString,bStdfDestination);

            strString.clear();

            for(int nIndex = 0; nIndex < lPatInfo->strDataSources.count(); nIndex++)
                strString += lPatInfo->strDataSources[nIndex] + "<br>";

            WritePP_OutlierReportLine(strString,bStdfDestination);

            sprintf(szString,"</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }
        else if (lPatInfo->strDataSources.count() > 0)
        {
            strString = lPatInfo->strDataSources.isEmpty()? "" : lPatInfo->strDataSources.first();
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Test data file</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",strString.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().bCustomPatLib)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Custom PAT Lib</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",lPatInfo->GetRecipeOptions().strCustomPatLibName.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        if(strProduct.isEmpty()==false)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Product</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",strProduct.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        if(strLotID.isEmpty()==false)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Lot</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",strLotID.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        if(strSubLot.isEmpty()==false)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>SubLot</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",strSubLot.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        if(strWaferID.isEmpty()==false)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%s</td>\n",strWaferID.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Ignore tests with few samples</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%ld</td>\n",lPatInfo->GetRecipeOptions().iMinimumSamples);
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        if(lPatInfo->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled && lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk >= 0)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Remove outliers until Test Cpk &gt;=</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%g</td>\n",lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Bin outliers</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        if(lPatInfo->GetRecipeOptions().bScanGoodPartsOnly)
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">Found in GOOD parts only</td>\n");
        else
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">Found in ALL parts</td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Minimum outliers to fail a part</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%ld</td>\n",lPatInfo->GetRecipeOptions().iMinimumOutliersPerPart);
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // Total parts tested
        if(pFile->getWaferMapData().bStripMap)
            ldValue = pFile->ldTotalPartsSampled;				// Final test data
        else
            ldValue = pFile->getWaferMapData().iTotalPhysicalDies;	// Wafersort data
        if(ldValue <=0)
            ldValue = 1;

        // When writing into STDF records, we do not have the correct die count...so do not save info in file!
        if(!bStdfDestination)
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Total parts tested</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\">%ld</td>\n",ldValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
        }

        // Total PAT failures (and yield loss associated)
        if(!bStdfDestination)
        {
            // When writing into STDF records, we do not have the correct die count...so do not save info in file!
            ldTotalCount = lPatInfo->m_lstOutlierParts.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            // 6179
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing PAT</b></td>\n");
            WritePP_OutlierReportLine(szString, bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b> (including retests)</td>\n",
                    ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            // MV-PAT
            ldTotalCount = lPatInfo->GetMVOutliers().count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100)
                lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,
                    "<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing MV-PAT</b></td>\n");
            WritePP_OutlierReportLine(szString, bStdfDestination);
            sprintf(szString,
                    "<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",
                    ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mNNROutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing NNR</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mIDDQOutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing IDDQ-Delta</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mReticleOutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing Reticle</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mClusteringOutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing Clustering</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mGDBNOutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing GDBN</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            ldTotalCount = lPatInfo->mZPATOutliers.count();
            lfValue = (100.0*ldTotalCount)/(double)ldValue;
            if(lfValue > 100) lfValue = 100;
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>Parts failing ZPAT</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"70%%\" bgcolor=\"#F8F8F8\"><b>%ld (%.2lf%%)</b></td>\n",ldTotalCount,lfValue);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        WritePP_OutlierReportLine("</table>\n",bStdfDestination);
        WritePP_OutlierReportLine("<br><br>\n",bStdfDestination);
        WritePP_OutlierReportLine("<b>Sections created:</b>\n",bStdfDestination);

        // Create shortcuts to sections:
        if (
            (m_pReportOptions->isReportOutputHtmlBased())
            //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            //if((pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                && (strOutputFormat!="DOC"&&strOutputFormat!="PDF"&&strOutputFormat!="PPT"&&strOutputFormat!="ODT") //&& ((pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML) == 0) &&
            && (bStdfDestination == 0))
        {
            bSplitPatReportPages = true;	// Report format is standard Web HTML: so split web pages to make them smaller!
        }
        if(bSplitPatReportPages)
        {
            strPatPage_FailingParts="advanced_fail.htm";
            strPatPage_MVPAT_FailingParts="advanced_mvpat.htm";
            strPatPage_NNR_FailingParts="advanced_nnr.htm";
            strPatPage_IDDQ_Delta_FailingParts="advanced_iddq_delta.htm";
            strPatPage_SpatLimits="advanced_spat.htm";
            strPatPage_DpatLimits="advanced_dpat.htm";
            strPatPage_Summary="advanced_summary.htm";
            strPatPage_MVPAT_Summary="advanced_mvpat_summary.htm";
            strPatPage_MVPAT_Details="advanced_mvpat_definitions.htm";
            strPatPage_NNR_Summary = "advanced_nnr_summary.htm";
            strPatPage_IDDQ_Delta_Summary = "advanced_iddq_delta_summary.htm";
            strPatPage_Reticle_Summary="advanced_reticle.htm";
            strPatPage_detailed_reticle = "advanced_detailed_reticle.htm";
            strPatPage_WarningLog="advanced_log.htm";
        }
        WritePP_OutlierReportLine("<ul>\n",bStdfDestination);

        strString.sprintf("  <li><a href=\"%s#part_fails\">Parts failing PAT limits</a></li>\n",strPatPage_FailingParts.toLatin1().constData());
        WritePP_OutlierReportLine(strString,bStdfDestination);

        // MV-PAT enabled ?
        if (lPatInfo->GetRecipeOptions().GetMVPATEnabled())
        {
            strString.sprintf("  <li><a href=\"%s#part_mvpat_fails\">Parts failing MV-PAT rules</a></li>\n",
                              strPatPage_MVPAT_FailingParts.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        // NNR D-PAT?
        if(lPatInfo->GetRecipeOptions().IsNNREnabled())
        {
            strString.sprintf("  <li><a href=\"%s#part_nnr_fails\">Parts failing NNR rule</a></li>\n",strPatPage_NNR_FailingParts.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        // IDDQ-Delta D-PAT?
        if(lPatInfo->GetRecipeOptions().mIsIDDQ_Delta_enabled)
        {
            strString.sprintf("  <li><a href=\"%s#part_iddq_delta_fails\">Parts failing IDDQ-Delta rule</a></li>\n",strPatPage_IDDQ_Delta_FailingParts.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().bReport_SPAT_Limits)
        {
            strString.sprintf("  <li><a href=\"%s#all_stat_pat_limits\">Static-PAT Test Limits</a></li>\n",
                              strPatPage_SpatLimits.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_Outliers || lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_NoOutliers)
        {
            strString.sprintf("  <li><a href=\"%s#all_dyn_pat_limits\">Dynamic-PAT Test Limits</a></li>\n",
                              strPatPage_DpatLimits.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        // 5909 : add PAT reload in these links too ?
        strString.sprintf("  <li><a href=\"%s#all_tests_dyn_fails\">Summary: Tests failing PAT limits</a></li>\n",
                          strPatPage_Summary.toLatin1().constData());
        WritePP_OutlierReportLine(strString,bStdfDestination);

        // MV-PAT enabled ?
        if (lPatInfo->GetRecipeOptions().GetMVPATEnabled())
        {
            strString.sprintf("  <li><a href=\"%s#mvpat_summary\">Summary: Rules failing MV-PAT</a></li>\n",
                              strPatPage_MVPAT_Summary.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);

            strString.sprintf("  <li><a href=\"%s#mvpat_definitions\">Summary: MV-PAT Rules definitions</a></li>\n",
                              strPatPage_MVPAT_Details.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().IsNNREnabled())
        {
            strString.sprintf("  <li><a href=\"%s#nnr_summary\">Summary: Tests failing NNR</a></li>\n",
                              strPatPage_NNR_Summary.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().mIsIDDQ_Delta_enabled)
        {
            strString.sprintf("  <li><a href=\"%s#iddq_delta_summary\">Summary: Tests failing IDDQ-Delta</a></li>\n",
                              strPatPage_IDDQ_Delta_Summary.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }

        if(lPatInfo->GetRecipeOptions().GetReticleEnabled())
        {
            strString.sprintf("  <li><a href=\"%s#reticle_summary\">Summary: Reticle</a></li>\n",
                              strPatPage_Reticle_Summary.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
            strString.sprintf(" <li><a href=\"%s#advanced_detailed_reticle\">Reticle: Parts failing reticle</a></li>",
                            strPatPage_detailed_reticle.toLatin1().constData());
            WritePP_OutlierReportLine(strString,bStdfDestination);
        }



        WritePP_OutlierReportLine("</ul>\n<br>",bStdfDestination);
        if(lPatInfo->m_strLogWarnings.count() <= 0)
            strString.sprintf("<a href=\"%s#warning_log\"><b>Warning log</b>: Empty</a><br><br>\n",
                              strPatPage_WarningLog.toLatin1().constData());
        else
        if(lPatInfo->m_strLogWarnings.count() == 1)
            strString.sprintf("<a href=\"%s#warning_log\"><b>Warning log</b>: 1 line</a><br><br>\n",
                              strPatPage_WarningLog.toLatin1().constData());
        else
            strString.sprintf("<a href=\"%s#warning_log\"><b>Warning log</b>: %d lines</a><br><br>\n",
                              strPatPage_WarningLog.toLatin1().constData(), lPatInfo->m_strLogWarnings.count());
        WritePP_OutlierReportLine(strString,bStdfDestination);

        WritePP_OutlierReportLine(
                    "<a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"><b>Export to file</b></a>\n",bStdfDestination);
    }


    // Create new Web page (if required)
    WritePageBreak();

    SplitPatReportWebPage(strPatPage_FailingParts);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Detail Parts that failed the outlier limits and show tests names+hyperlinks.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        WritePP_OutlierReportLine("\nParts failing PAT limits:\n", bStdfDestination);
    }
    else
    {
        // Section Title + bookmark
        if(!bStdfDestination)
            WriteHtmlSectionTitle(hReportFile,"part_fails","Parts failing PAT limits");
        else
            WritePP_OutlierReportLine(
               "<h1 align=\"left\"><a name=\"part_fails\"></a><font color=\"#006699\">Parts failing PAT limits<br></font></h1>\n\n",
               bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine(
            "<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // If PowerPoint: Number of lines in table on first page (normally it's 20, but first page also lists global settings!).
    int             iTableLine = 15;
    long            lTotalFailures = 0;	// Keeps track of total tests that failed the PAT limits.
    unsigned int    lLineInTable = 0;	// Keeps track of total tests in current table & page (cleared at each new page)
//    unsigned int    lColInTable = 0;
    int             iTestCount;

    // Write lines
    int		iGroupID;
    QString	strTestNumber;
    QString strHyperlink;
    QString	strFailureType;
    QString strFailureValue;
    QList<CPatFailingTest>::iterator            itPart;
    CPatFailingTest                             cFailTest;
    CPatDefinition *                            ptPatDef = NULL;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;

    // Make sure we reset the outlier count for good parts (since the report is called twice: once for the STDF content, 2nd for HTM output!)
    lPatInfo->clearOutlierCount(false);

    ///////
    QList<CPatOutlierPart*>	lstPatDeviceResults;
    tdIterPatOutlierParts	itOutlierParts(lPatInfo->m_lstOutlierParts);
    CPatOutlierPart *		pOutlierPart = NULL;

    while(itOutlierParts.hasNext())
    {
        pOutlierPart = itOutlierParts.next();

        lstPatDeviceResults.append(pOutlierPart);
    };

    // If sorting by Device PAT Severity score
    qSort(lstPatDeviceResults.begin(), lstPatDeviceResults.end(), CPatOutlierPart::lessThan);

    //////
    QListIterator<CPatOutlierPart*> itPatDeviceResults(lstPatDeviceResults);
    double	lfLowLimit,lfHighLimit;
    int	iSeverityLimits;		// Tells which outlier limits to use (near, medium, far)

    while(itPatDeviceResults.hasNext())
    {
        pOutlierPart = itPatDeviceResults.next();

        // Write the table header to fill
        if(lLineInTable == 0)
            BuildDPAT_PartsHtmlTable(bStdfDestination);

        // Keep track of total failing Parts
        lTotalFailures++;
        lLineInTable++;

        if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            QString lLine;

            WritePP_OutlierReportLine(strLotID + ",",bStdfDestination);	// LotID
            WritePP_OutlierReportLine("'" + strWaferID + "',",bStdfDestination);	// WaferID
            WritePP_OutlierReportLine(pOutlierPart->strPartID + ",",bStdfDestination);	// PartID
            WritePP_OutlierReportLine(QString::number(pOutlierPart->iPatSBin) + ",",bStdfDestination);				// PAT Bin
            WritePP_OutlierReportLine(QString::number(pOutlierPart->iDieX) + ",",bStdfDestination);					// Die X
            WritePP_OutlierReportLine(QString::number(pOutlierPart->iDieY) + ",",bStdfDestination);					// Die Y
            WritePP_OutlierReportLine(QString::number(pOutlierPart->iSite) + ",",bStdfDestination);					// Testing site
            iTestCount = 0;
            for ( itPart = pOutlierPart->cOutlierList.begin(); itPart != pOutlierPart->cOutlierList.end(); ++itPart )
            {
                cFailTest = *itPart;

                ptPatDef = lPatInfo->GetPatDefinition(cFailTest.mTestNumber, cFailTest.mPinIndex,
                                                      cFailTest.mTestName);

                if (ptPatDef == NULL)
                {
                    QString lMessage;

                    lMessage = "No PAT definitions found for test ";
                    lMessage += QString::number(cFailTest.mTestNumber);

                    if (cFailTest.mPinIndex >= 0)
                        lMessage += "." + QString::number(cFailTest.mPinIndex);

                    GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().constData());
                    continue;
                }

                switch(cFailTest.mFailureMode)
                {
                    case GEX_TPAT_FAILMODE_STATIC_L:	// Low Static
                    default:	// Static failure.
                        strFailureType = "Static";
                        ptPatDef->m_lStaticFailuresLow++;
                        break;

                    case GEX_TPAT_FAILMODE_STATIC_H:	// High static
                        strFailureType = "Static";
                        ptPatDef->m_lStaticFailuresHigh++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:	// Failure on 'Near' outlier limits
                        strFailureType = "N";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:// Failure on 'Medium' outlier limits
                        strFailureType = "M";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:// Failure on 'Far' outlier limits
                        strFailureType = "F";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]++;
                        break;
                }
                if(cFailTest.mFailureDirection < 0)
                    strFailureType += "-";	// PAT Failure by being under the PAT low limit
                else
                    strFailureType += "+";	// PAT Failure by being over the PAT high limit


                // Keep track of total failures for this test test
                ptPatDef->m_TotalFailures++;

                // Prepare line to write
                // Write test number
                lLine = "T" + QString::number(cFailTest.mTestNumber);

                // Concatenate Pin Index if valid
                if (cFailTest.mPinIndex >= 0)
                    lLine += "." + QString::number(cFailTest.mPinIndex);

                // Concatenate failure type
                lLine += strFailureType + ",";

                WritePP_OutlierReportLine(lLine, bStdfDestination);	// List of tests
            }

            WritePP_OutlierReportLine("\n",bStdfDestination);
        }
        else if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\" valign=\"top\">%s</td>\n",strLotID.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%s</td>\n",strWaferID.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);

            // Hyperlink ot SBIN-Wafermap (only created is standard HTML report)
            QString strHyperLinkBegin="";
            QString strHyperLinkEnd="";
            if (strOutputFormat=="HTML") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML)
            {
                // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
                iGroupID = gexReport->getGroupForSite(pOutlierPart->iSite);
                if(iGroupID < 0)
                    iGroupID=0;

                // Regular html page: Build Link to SoftBin wafermap
                strHyperLinkBegin = "<a href=\"#_gex_drill--drill_3d=wafer_sbin";
                strHyperLinkBegin += "--g=" + QString::number(iGroupID);		// group#
                strHyperLinkBegin += "--f=0";		// file#
                strHyperLinkBegin += "--Test=0";	// test#: not used
                strHyperLinkBegin += "--Pinmap=0";		// pinmap#: not used
                strHyperLinkBegin += "--DieX=" + QString::number(pOutlierPart->iDieX);	// DieX position
                strHyperLinkBegin += "--DieY=" + QString::number(pOutlierPart->iDieY);	// DieY position
                strHyperLinkBegin += "\">";

                strHyperLinkEnd = "</a>";
            }

            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%s%s%s</td>\n",strHyperLinkBegin.toLatin1().constData(),pOutlierPart->strPartID.toLatin1().constData(),strHyperLinkEnd.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%d</td>\n",pOutlierPart->iPatSBin);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%d</td>\n",pOutlierPart->iDieX);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%d</td>\n",pOutlierPart->iDieY);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%d</td>\n",pOutlierPart->iSite);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"50%%\" align=\"left\" bgcolor=\"#F8F8F8\">");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            iTestCount = 0;
            for ( itPart = pOutlierPart->cOutlierList.begin(); itPart != pOutlierPart->cOutlierList.end(); ++itPart )
            {
                // Keeps track of total outlier tests in this part
                iTestCount++;

                // Get handle to faulty test.
                cFailTest = *itPart;

                ptPatDef = lPatInfo->GetPatDefinition(cFailTest.mTestNumber, cFailTest.mPinIndex,
                                                      cFailTest.mTestName);

                if (ptPatDef == NULL)
                {
                    QString lMessage;

                    lMessage = "No PAT definitions found for test ";
                    lMessage += QString::number(cFailTest.mTestNumber);

                    if (cFailTest.mPinIndex >= 0)
                        lMessage += "." + QString::number(cFailTest.mPinIndex);

                    GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().constData());
                    continue;
                }

                switch(cFailTest.mFailureMode)
                {
                    case GEX_TPAT_FAILMODE_STATIC_L:	// Low Static
                    default:	// Static failure.
                        strFailureType = "Static";
                        ptPatDef->m_lStaticFailuresLow++;
                        break;

                    case GEX_TPAT_FAILMODE_STATIC_H:	// High static
                        strFailureType = "Static";
                        ptPatDef->m_lStaticFailuresHigh++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:	// Failure on 'Near' outlier limits
                        strFailureType = "N";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:// Failure on 'Medium' outlier limits
                        strFailureType = "M";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]++;
                        break;

                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:// Failure on 'Far' outlier limits
                        strFailureType = "F";
                        // Update outlier count
                        if(cFailTest.mFailureDirection < 0)
                            ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]++;
                        else
                            ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]++;
                        break;
                }
                if(cFailTest.mFailureDirection < 0)
                    strFailureType += "-";	// PAT Failure by being under the PAT low limit
                else
                    strFailureType += "+";	// PAT Failure by being over the PAT high limit


                // Keep track of total failures for this test test
                ptPatDef->m_TotalFailures++;

                // Build hyperlink for wafermap interactive
                if (strOutputFormat=="HTML") //if (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML)
                {
                    // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
                    iGroupID = gexReport->getGroupForSite(pOutlierPart->iSite);
                    if(iGroupID < 0)
                        iGroupID=0;

                    // Wafer map link
                    strHyperLinkBegin	= "<a href=\"#_gex_drill--drill_3d=wafer_param_range";
                    strHyperLinkBegin	+= "--g=" + QString::number(iGroupID);		// group#
                    strHyperLinkBegin	+= "--f=0";         // file#
                    strHyperLinkBegin	+= "--Test=";		// test#
                    strHyperLinkBegin	+= QString::number(cFailTest.mTestNumber);
                    strHyperLinkBegin	+= "--Pinmap=";     // pin#
                    strHyperLinkBegin	+= QString::number(cFailTest.mPinIndex);
                    strHyperLinkBegin	+= "--DieX=" + QString::number(pOutlierPart->iDieX);	// DieX position
                    strHyperLinkBegin	+= "--DieY=" + QString::number(pOutlierPart->iDieY);	// DieY position
                    strHyperLinkBegin	+= "\">";

                    strHyperLinkEnd		= "</a>";

                    strFailureValue		= strHyperLinkBegin + QString::number(cFailTest.mValue) + strHyperLinkEnd;
                }
                else
                    strFailureValue		= QString::number(cFailTest.mValue);

                // Display outlier value
                strFailureType += " ( " + strFailureValue + " ";

                // Returns pointer to correct cell. If cell doesn't exist ; its created. Test# mapping enabled
                if(pFile->FindTestCell(cFailTest.mTestNumber, cFailTest.mPinIndex, &ptTestCell, true, false,
                                       cFailTest.mTestName) == 1)
                    strFailureType += ptTestCell->szTestUnits;

                strFailureType += " )";

                // Display Outlier
                GS::PAT::DynamicLimits lDynLimits = ptPatDef->GetDynamicLimits(pOutlierPart->iSite);

                // Site exists, then list the dynamic limits used  (check which severity level must be used!)
                iSeverityLimits = ptPatDef->m_iOutlierLimitsSet;

                // Get relevant PAT limits set, as the STDF only allows one limit set, we need to specify the range of the two limits sets
                lfLowLimit = gex_min(lDynLimits.mLowDynamicLimit1[iSeverityLimits], lDynLimits.mLowDynamicLimit2[iSeverityLimits]);
                if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
                    lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;
                lfHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[iSeverityLimits], lDynLimits.mHighDynamicLimit2[iSeverityLimits]);
                if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
                    lfHighLimit = GEX_TPAT_FLOAT_INFINITE;
                if(cFailTest.mFailureDirection < 0)
                {
                    // '-' outlier: PAT Failure by being under the PAT low limit
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    {
                        // PAT limit - OriginalTestLimit
                        lfValue = ptPatDef->m_lfLowLimit-lfLowLimit;
                        if(lfValue)
                            lfValue = fabs((cFailTest.mValue-ptPatDef->m_lfLowLimit)/lfValue);
                    }
                }
                else
                {
                    // '+' outlier : PAT Failure by being over the PAT high limit
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    {
                        // PAT limit - OriginalTestLimit
                        lfValue = ptPatDef->m_lfHighLimit-lfHighLimit;
                        if(lfValue)
                            lfValue = fabs((cFailTest.mValue-ptPatDef->m_lfHighLimit)/lfValue);
                    }
                }
                strFailureType += " - Limit Proximity: ";
                if(lfValue != GEX_C_DOUBLE_NAN)
                    strFailureType += QString::number(100*lfValue,'f',2)+ "%";
                else
                    strFailureType += "n/a";

                if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
                    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                {
                    // Flat html to endup in one file
                    strHyperlink = "href=\"#HistoT";
                    strHyperlink += QString::number(cFailTest.mTestNumber);
                    if (cFailTest.mPinIndex >= 0)
                        strHyperlink += "." + QString::number(cFailTest.mPinIndex);
                    strHyperlink += "\"";
                }
                else
                {
                    // Regular html page
                    strHyperlink = "href=\"#_gex_drill--drill_chart=adv_multichart--data=";
                    strHyperlink += QString::number(cFailTest.mTestNumber);
                    if (cFailTest.mPinIndex >= 0)
                        strHyperlink += "." + QString::number(cFailTest.mPinIndex);
                    strHyperlink += "--marker=" + QString::number(pOutlierPart->iSite) + " ";
                    strHyperlink += QString::number(pOutlierPart->lRunID) + " ";
                    strHyperlink += QString::number(pOutlierPart->iDieX) + " ";
                    strHyperlink += QString::number(pOutlierPart->iDieY) + " ";
                    strHyperlink += QString::number(cFailTest.mValue) + " Outlier";
                    strHyperlink += "\"";
                }

                // Insert line break if multiple failing tests in part.
                if(iTestCount > 1)
                    WritePP_OutlierReportLine("<br>",bStdfDestination);
                sprintf(szString,"<a %s> <b>T%ld</b></a> %s &nbsp;  &nbsp; %s &nbsp; &nbsp;",strHyperlink.toLatin1().constData(),cFailTest.mTestNumber,buildDisplayName(ptTestCell).toLatin1().constData(),strFailureType.toLatin1().constData());	// List of tests
                // Write outlier test
                WritePP_OutlierReportLine(szString,bStdfDestination);
            }

            // Yield tip (not written if report goes into STDF.DTR records)
            if(!bStdfDestination)
                sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#F8F8F8\"><a href=\"#YieldTipNbr%ld\">#%ld</a></td>\n",lTotalFailures,lTotalFailures);
            else
                sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#F8F8F8\">.</td>\n");

            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
            if (!bStdfDestination
                && (strOutputFormat=="PPT") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
                && ((lTotalFailures % iTableLine) == 0))
            {
                // close table & goto next page
                fprintf(hReportFile,"</table>\n");
                WritePageBreak();

                // Reset line counter in page (so will reopen table if needed)
                lLineInTable = 0;

                // Number of lines in table (page 2 and following) is 20
                iTableLine = 20;
            }
        }
    };

    // If no failure at all, just say it!
    if(lTotalFailures == 0)
        WritePP_OutlierReportLine("No Part failed any of the Outlier limits (Static and Dynamic)!\n\n",bStdfDestination);

    // close HTML codes.
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        if(lLineInTable > 0)
        {
            WritePP_OutlierReportLine("</table><br>\n",bStdfDestination);
        }
        if(lTotalFailures)
        {
            // Legend
            // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
            sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
            WritePP_OutlierReportLine(szString,bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>Legend</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#CCECFF\">Outlier type / Description</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>Static</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Static</b> outlier (Static PAT)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>N</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Near</b> outlier (between 'Near' and 'Medium' dynamic outlier limits)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>M</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Medium</b> outlier (between 'Medium' and 'Far' dynamic outlier limits)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>F</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\"><b>Far</b> outlier (Over the 'Far' dynamic outlier limits)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>-</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\">Left outlier (under the outlier <b>low</b> limit)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\"  align=\"center\" bgcolor=\"#CCECFF\"><b>+</b></td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"90%%\" bgcolor=\"#F8F8F8\">Right outlier (over the outlier <b>high</b> limit)</td>\n");
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
            WritePP_OutlierReportLine("</table>\n",bStdfDestination);
            WritePP_OutlierReportLine("<br><br>\n",bStdfDestination);
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Write Yield Tips (only if creating HTML/HTML_FLAT non CSV report, not STDF.DTR report records)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(!bStdfDestination && lTotalFailures &&
       (strOutputFormat!="CSV") //(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_CSV)
       )
    {
        // Title + bookmark
        WritePageBreak();
        WriteHtmlSectionTitle(hReportFile,"pat_yield_tips","Yield Tips: Minimizing PAT yield loss");
        SetPowerPointSlideName("Outliers: Minimizing yield loss");

        // Table
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Yield tips</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>PartID</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"80%%\" align=\"left\" bgcolor=\"#CCECFF\"><b>Tips to minimize your PAT yield loss...</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        lTotalFailures = 0;
        int                     lDistributionShape;
        QString					strPartYieldTips;	// Holds all tips for a given part, flushed to the file at the end of the part review
        QString					strYieldTip;		// Holds the tip for one failing test in part.
        CPatOutlierPart *		pOutlierPart = NULL;

        itPatDeviceResults.toFront();
        while(itPatDeviceResults.hasNext())
        {
            pOutlierPart = itPatDeviceResults.next();

            // For each part, Write HTML line with Yield tips.
            ++lTotalFailures;

            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#F8F8F8\"><a name=\"YieldTipNbr%ld\"></a>#%ld</td>\n",lTotalFailures,lTotalFailures);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td width=\"10%%\" align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",pOutlierPart->strPartID.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);

            // Point to relevant dataset group for this device.
            pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
            pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            // Analyze the type of PAT failure....so to display the appropriate tip.
            strPartYieldTips = "";
            for ( itPart = pOutlierPart->cOutlierList.begin(); itPart != pOutlierPart->cOutlierList.end(); ++itPart )
            {
                strYieldTip = "";
                cFailTest = *itPart;

                if(pFile->FindTestCell(cFailTest.mTestNumber, cFailTest.mPinIndex, &ptTestCell,
                                       true, false, cFailTest.mTestName) != 1)
                    ptTestCell = NULL;

                // If the tip line already includes some text, add a line break
                if(strPartYieldTips.isEmpty() == false)
                    strPartYieldTips += "<br>";

                // build Test# and it's hyperlink.
                if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
                    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                {
                    // Flat html to endup in one file
                    strHyperlink = "<a href=\"#HistoT";
                    strHyperlink += QString::number(cFailTest.mTestNumber);
                    if (cFailTest.mPinIndex >= 0)
                        strHyperlink += "." + QString::number(cFailTest.mPinIndex);
                    strHyperlink += "\">";
                }
                else
                {
                    // Regular html page
                    strHyperlink = "<a href=\"#_gex_drill--drill_chart=adv_histo--data=";
                    strHyperlink += QString::number(cFailTest.mTestNumber);
                    if (cFailTest.mPinIndex >= 0)
                        strHyperlink += "." + QString::number(cFailTest.mPinIndex);
                    strHyperlink += "\">";
                }

                strHyperlink += " <b>T" + QString::number(cFailTest.mTestNumber);
                if (cFailTest.mPinIndex >= 0)
                    strHyperlink += "." + QString::number(cFailTest.mPinIndex);
                strHyperlink += "</b></a>";

                // If cell doesn't exist ; its created. Test# mapping enabled
                if(ptTestCell)
                {
                    if (lPatInfo)
                        lDistributionShape = patlib_GetDistributionType(ptTestCell,
                                                                        lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                                        lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                                        lPatInfo->GetRecipeOptions().mMinConfThreshold);
                    else
                        lDistributionShape = patlib_GetDistributionType(ptTestCell);

                    switch(lDistributionShape)
                    {
                        case PATMAN_LIB_SHAPE_GAUSSIAN:
                        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                        case PATMAN_LIB_SHAPE_BIMODAL:
                        case PATMAN_LIB_SHAPE_MULTIMODAL:
                        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                            break;

                        case PATMAN_LIB_SHAPE_CATEGORY:
                            strYieldTip += " Distribution shape is 'Categories' (only few different values). You should <b>disable</b> this test from PAT";
                            goto next_failing_test;

                        case PATMAN_LIB_SHAPE_UNKNOWN:
                            strYieldTip += " Distribution shape is 'Unknown'. You should <b>disable</b> this test from PAT";
                            goto next_failing_test;

                        case PATMAN_LIB_SHAPE_ERROR:
                        default:
                            strYieldTip += " <b>Error</b> while computing distribution shape. Contact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+" for more details.";
                            goto next_failing_test;
                    }

                    // Check what's the test Cpk for this testing site.
                    if((ptTestCell->GetCurrentLimitItem()->lfCpk >= 10) && (lPatInfo->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled || lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk < 0))
                    {
                        strYieldTip += " High Cpk of <b>" + QString::number(ptTestCell->GetCurrentLimitItem()->lfCpk,'f',2) +" </b> (outliers remain concentrated and not too remote). Yield can be improved forcing the PAT global options to <b>'Only remove outliers until Cpk > xx'</b>, where 'xx' is your threshold limit (E.g.: 20)";
                        goto next_failing_test;
                    }

                    // Low yield, so see if other setting could do!
                    if(cFailTest.mFailureMode == GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR)
                    {
                        strYieldTip += " Outlier is 'Near'. You may keep it to improve your yield, forcing the test settings to <b>'Outlier limits set = Medium'</b>";
                        goto next_failing_test;
                    }

                    // Medium yield, so see if other setting could do!
                    if(cFailTest.mFailureMode == GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM)
                    {
                        strYieldTip += " Outlier is 'Medium'. You may keep it to improve your yield, forcing the test settings to <b>'Outlier limits set = Far'</b>";
                        goto next_failing_test;
                    }

                }

next_failing_test:
                // Concatenate tips for all failing tests in part (except the one with nothing to say about!)
                if(strYieldTip.isEmpty() == false)
                    strPartYieldTips  += strHyperlink + strYieldTip;
            }
            if(pOutlierPart->cOutlierList.count() >= 2)
                strPartYieldTips += "<br>Note: If acceptable, you may request multiple outliers in a part to fail it (see PAT global options dialog box).";


            // Write tip messages!
            if(strPartYieldTips.isEmpty())
                strPartYieldTips = "-";
            WritePP_OutlierReportLine("<td width=\"80%\" align=\"left\" bgcolor=\"#F8F8F8\">",bStdfDestination);
            WritePP_OutlierReportLine(strPartYieldTips,bStdfDestination);
            WritePP_OutlierReportLine("</td>\n",bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        };

        WritePP_OutlierReportLine("</table>\n",bStdfDestination);
        WritePP_OutlierReportLine("<br><br>\n",bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // Create new Web page (if required)
    WritePageBreak();

    // Detail Parts that failed the MV PAT rules
    BuildMVPATFailingPartsPage(strPatPage_MVPAT_FailingParts, bSplitPatReportPages, bStdfDestination);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // If NNR enabled, report NNR failures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create new Web page (if required)
    WritePageBreak();
    SplitPatReportWebPage(strPatPage_NNR_FailingParts);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Detail Parts that failed the NNR outlier algorithm show tests names+hyperlinks.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tdIterGPATOutliers itNNR(lPatInfo->mNNROutliers);

    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        WritePP_OutlierReportLine("\nParts failing NNR algorithm:\n",bStdfDestination);
    }
    else
    {
        // Section Title + bookmark
        if(!bStdfDestination)
            WriteHtmlSectionTitle(hReportFile,"part_nnr_fails","Parts failing NNR algorithm");
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"part_nnr_fails\"></a><font color=\"#006699\">Parts failing NNR algorithm<br></font></h1>\n\n",bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n<br>\n",bStdfDestination);

    // If no NNR result, go to next page!
    if(lPatInfo->mNNROutliers.count() == 0)
    {
        if(strOutputFormat==QString("CSV"))
            WritePP_OutlierReportLine("No test failed the NNR algorithm!\n\n",bStdfDestination);		// case 3765, pyc, 13/04/2011
        else
            WritePP_OutlierReportLine("<br>No test failed the NNR algorithm!<br>\n\n",bStdfDestination);


        goto end_nnr_details;
    }

    // Create table header
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("LotID,WaferID,DieX,DieY,Site,Tests failing NNR algorithm\n",bStdfDestination);
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>LotID</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieX</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieY</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Site</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"70%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Tests failing NNR algorithm (NNR Outliers)</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("NNR Outliers: Parts");
    }

    // List all dies identified with NNR exception
    int iSite;
    while (itNNR.hasNext())
    {
        itNNR.next();

        // Scan list of NNR outlier tests...and find the one matching this die location.
        // Keep track of NNR test failures in same die.
        iTestCount = 0;
        foreach(CPatOutlierNNR * ptNRR_OutlierPart, lPatInfo->pNNR_OutlierTests)
        {
            if(ptNRR_OutlierPart->mDieX == itNNR.value().mDieX &&
               ptNRR_OutlierPart->mDieY == itNNR.value().mDieY)
            {
                // Keep track of total NNR failures in same die
                iTestCount++;

                // Get site# used for testing given die.
                iSite = gexReport->getSiteForDie(ptNRR_OutlierPart->mDieX, ptNRR_OutlierPart->mDieY);

                // Report test details failing NNR for that die
                if(iTestCount == 1)
                {
                    // First NNR test exception in die.
                    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                    {
                        WritePP_OutlierReportLine(strLotID + ",",bStdfDestination);	// LotID
                        WritePP_OutlierReportLine("'" + strWaferID + "',",bStdfDestination);	// WaferID
                        WritePP_OutlierReportLine(QString::number(ptNRR_OutlierPart->mDieX) + ",",bStdfDestination);					// Die X
                        WritePP_OutlierReportLine(QString::number(ptNRR_OutlierPart->mDieY) + ",",bStdfDestination);					// Die Y
                        WritePP_OutlierReportLine(QString::number(iSite) + ",",bStdfDestination); // Testing site
                    }
                    else
                    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                    if(m_pReportOptions->isReportOutputHtmlBased())
                    {
                        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
                        sprintf(szString,"<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\">%s</td>\n",strLotID.toLatin1().constData());
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",strWaferID.toLatin1().constData());
                        WritePP_OutlierReportLine(szString,bStdfDestination);

                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",ptNRR_OutlierPart->mDieX);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",ptNRR_OutlierPart->mDieY);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",iSite);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"50%%\" align=\"left\" bgcolor=\"#F8F8F8\">");
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                    }
                }
                else
                {
                    // Multi-lines result for same die location
                    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                        WritePP_OutlierReportLine(",,,,,",bStdfDestination);
                }

                // Write test info
                if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                    WritePP_OutlierReportLine("T" + QString::number(ptNRR_OutlierPart->mTestNumber) + ",\n",bStdfDestination);							// List of tests
                else
                //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                if(m_pReportOptions->isReportOutputHtmlBased())
                {
                    // Hyperlink strings
                    QString strHyperLinkBegin="";
                    QString strHyperLinkEnd="";
                    QString lDisplayTestName;

                    // Build hyperlink for wafermap interactive
                    if (strOutputFormat=="HTML") //if (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML)
                    {
                        // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
                        iGroupID = gexReport->getGroupForSite(iSite);
                        if(iGroupID < 0)
                            iGroupID=0;

                        // Wafer map link
                        strHyperLinkBegin	= "<a href=\"#_gex_drill--drill_3d=wafer_param_range_nnr";
                        strHyperLinkBegin	+= "--g=" + QString::number(iGroupID);		// group#
                        strHyperLinkBegin	+= "--f=0";			// file#
                        strHyperLinkBegin	+= "--Test=";		// test#
                        strHyperLinkBegin	+= QString::number(ptNRR_OutlierPart->mTestNumber);
                        strHyperLinkBegin	+= "--Pinmap=-1";	// pin#
                        strHyperLinkBegin	+= "--DieX=" + QString::number(ptNRR_OutlierPart->mDieX);	// DieX position
                        strHyperLinkBegin	+= "--DieY=" + QString::number(ptNRR_OutlierPart->mDieY);	// DieY position
                        strHyperLinkBegin	+= "--AlgoType=" + QString::number(ptNRR_OutlierPart->mNNRRule.GetAlgorithm());	// Algo type
                        strHyperLinkBegin	+= "--NFactor=" + QString::number(ptNRR_OutlierPart->mNNRRule.GetNFactor());	// N factor
                        strHyperLinkBegin	+= "--Size=" + QString::number(ptNRR_OutlierPart->mNNRRule.GetClusterSize());	// Cluster size
                        strHyperLinkBegin	+= "--LA=" + QString::number(ptNRR_OutlierPart->mNNRRule.GetLA());	// Cluster LA (location averaging)
                        strHyperLinkBegin	+= "\">";

                        strHyperLinkEnd		= "</a>";

                        strFailureValue		= strHyperLinkBegin + QString::number(ptNRR_OutlierPart->mValue) + strHyperLinkEnd;
                    }
                    else
                        strFailureValue		= QString::number(ptNRR_OutlierPart->mValue);

                    // Display outlier value
                    strFailureType = " ( Residual: " + strFailureValue + " )";

                    // Returns pointer to correct cell. If cell doesn't exist ; its created. Test# mapping enabled
                    if (pFile->FindTestCell(ptNRR_OutlierPart->mTestNumber, ptNRR_OutlierPart->mPinmap, &ptTestCell,
                                            true, false, ptNRR_OutlierPart->mTestName) == 1)
                    {
                        lDisplayTestName = buildDisplayName(ptTestCell);
                    }
                    else
                        lDisplayTestName = buildDisplayName(ptNRR_OutlierPart->mTestName);

                    if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
                    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                    {
                        // Flat html to endup in one file
                        strHyperlink = "href=\"#HistoT";
                        strHyperlink += QString::number(ptNRR_OutlierPart->mTestNumber);
                        strHyperlink += "\"";
                    }
                    else
                    {
                        // Regular html page
                        strHyperlink = "href=\"#_gex_drill--drill_chart=adv_multichart--data=";
                        strHyperlink += QString::number(ptNRR_OutlierPart->mTestNumber);
                        strHyperlink += "--marker=" + QString::number(iSite) + " ";
                        strHyperlink += QString::number(0) + " ";
                        strHyperlink += QString::number(ptNRR_OutlierPart->mDieX) + " ";
                        strHyperlink += QString::number(ptNRR_OutlierPart->mDieY) + " ";
                        strHyperlink += QString::number(ptNRR_OutlierPart->mValue) + " NRR Outlier";
                        strHyperlink += "\"";
                    }

                    // Insert line break if multiple failing tests in part.
                    if(iTestCount > 1)
                        WritePP_OutlierReportLine("<br>",bStdfDestination);
                    sprintf(szString,"<a %s> <b>T%ld</b></a> %s &nbsp;  &nbsp; %s &nbsp; &nbsp;",
                            strHyperlink.toLatin1().constData(),
                            ptNRR_OutlierPart->mTestNumber,
                            lDisplayTestName.toLatin1().constData(),
                            strFailureType.toLatin1().constData());	// List of tests
                    // Write outlier test
                    WritePP_OutlierReportLine(szString,bStdfDestination);
                }
            }
        }

    }

    // Close table
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        WritePP_OutlierReportLine("</table>\n",bStdfDestination);
        WritePP_OutlierReportLine("<br><br>\n",bStdfDestination);
    }

end_nnr_details:;
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n<br>\n",bStdfDestination);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // If IDDQ-Delta enabled, report IDDQ failures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create new Web page (if required)
    WritePageBreak();
    SplitPatReportWebPage(strPatPage_IDDQ_Delta_FailingParts);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Detail Parts that failed the IDDQ-Delta outlier algorithm show tests names+hyperlinks.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tdIterGPATOutliers itIDDQ(lPatInfo->mIDDQOutliers);
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        WritePP_OutlierReportLine("\nParts failing IDDQ-Delta algorithm:\n",bStdfDestination);
    }
    else
    {
        // Section Title + bookmark
        if(!bStdfDestination)
            WriteHtmlSectionTitle(hReportFile,"part_iddq_delta_fails","Parts failing IDDQ-Delta algorithm");
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"part_iddq_delta_fails\"></a><font color=\"#006699\">Parts failing IDDQ-Delta algorithm<br></font></h1>\n\n",bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n<br>\n",bStdfDestination);

    // If no NNR result, go to next page!
    if(lPatInfo->mIDDQOutliers.count() == 0)
    {
        if(strOutputFormat==QString("CSV"))
            WritePP_OutlierReportLine("No test failed the IDDQ-Delta algorithm!\n\n",bStdfDestination);		// case 3765, pyc, 13/04/2011
        else
            WritePP_OutlierReportLine("<br>No test failed the IDDQ-Delta algorithm!<br>\n\n",bStdfDestination);

        goto end_iddq_delta_details;
    }

    // Create table header
    if (strOutputFormat=="CSV")  //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        WritePP_OutlierReportLine("LotID,WaferID,DieX,DieY,Site,Tests failing IDDQ-Delta algorithm\n",bStdfDestination);
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open HTML table with correlation results
        // HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
        WritePP_OutlierReportLine(szString,bStdfDestination);

        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
        sprintf(szString,"<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>LotID</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieX</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieY</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Site</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        sprintf(szString,"<td width=\"70%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Tests failing IDDQ-Delta algorithm</b></td>\n");
        WritePP_OutlierReportLine(szString,bStdfDestination);
        WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

        // If PowerPoint slide to create, set its name
        if(!bStdfDestination)
            SetPowerPointSlideName("IDDQ-Delta Outliers: Parts");
    }

    // List all dies identified with IDDQ-Delta exception
    CPatOutlierIDDQ_Delta * ptIDDQ_Delta_OutlierPart;
    while (itIDDQ.hasNext())
    {
        itIDDQ.next();

        // Scan list of IDDQ-Delta outlier tests...and find the one matching this die location.
        // Keep track of IDDQ-Delta test failures in same die.
        iTestCount = 0;
        foreach(ptIDDQ_Delta_OutlierPart, lPatInfo->pIDDQ_Delta_OutlierTests)
        {
            if(ptIDDQ_Delta_OutlierPart->iDieX == itIDDQ.value().mDieX &&
               ptIDDQ_Delta_OutlierPart->iDieY == itIDDQ.value().mDieY)
            {
                // Keep track of total IDDQ-Delta failures in same die
                iTestCount++;

                // Get site# used for testing given die.
                iSite = gexReport->getSiteForDie(ptIDDQ_Delta_OutlierPart->iDieX,
                                                 ptIDDQ_Delta_OutlierPart->iDieY);

                // Report test details failing IDDQ-Delta for that die
                if(iTestCount == 1)
                {
                    // First IDDQ-Delta test exception in die.
                    if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                    {
                        WritePP_OutlierReportLine(strLotID + ",",bStdfDestination);	// LotID
                        WritePP_OutlierReportLine("'" + strWaferID + "',",bStdfDestination);	// WaferID
                        WritePP_OutlierReportLine(QString::number(ptIDDQ_Delta_OutlierPart->iDieX) + ",",bStdfDestination);					// Die X
                        WritePP_OutlierReportLine(QString::number(ptIDDQ_Delta_OutlierPart->iDieY) + ",",bStdfDestination);					// Die Y
                        WritePP_OutlierReportLine(QString::number(iSite) + ",",bStdfDestination); // Testing site
                    }
                    else
                    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                    if(m_pReportOptions->isReportOutputHtmlBased())
                    {
                        WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
                        sprintf(szString,"<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\">%s</td>\n",strLotID.toLatin1().constData());
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",strWaferID.toLatin1().constData());
                        WritePP_OutlierReportLine(szString,bStdfDestination);

                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",ptIDDQ_Delta_OutlierPart->iDieX);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",ptIDDQ_Delta_OutlierPart->iDieY);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\">%d</td>\n",iSite);
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                        sprintf(szString,"<td width=\"50%%\" align=\"left\" bgcolor=\"#F8F8F8\">");
                        WritePP_OutlierReportLine(szString,bStdfDestination);
                    }
                }
                else
                {
                    // Multi-lines result for same die location
                    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                        WritePP_OutlierReportLine(",,,,,",bStdfDestination);
                }

                // Write test info
                if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                {
                    strString = "T" + QString::number(ptIDDQ_Delta_OutlierPart->lTestNumber1);
                    strString += "/ T" + QString::number(ptIDDQ_Delta_OutlierPart->lTestNumber2) + " ,\n";
                    WritePP_OutlierReportLine(strString,bStdfDestination);							// List of tests
                }
                else
                //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                if(m_pReportOptions->isReportOutputHtmlBased())
                {
                    strFailureValue		= QString::number(ptIDDQ_Delta_OutlierPart->lfValue);

                    // Display outlier value
                    strFailureType = " ( " + strFailureValue + " ";

                    // Returns pointer to correct cell. If cell doesn't exist ; its created. Test# mapping enabled
                    if(pFile->FindTestCell(ptIDDQ_Delta_OutlierPart->lTestNumber1,GEX_PTEST,&ptTestCell) == 1)
                        strFailureType += ptTestCell->szTestUnits;

                    strFailureType += " )";

                    // Insert line break if multiple failing tests in part.
                    if(iTestCount > 1)
                        WritePP_OutlierReportLine("<br>",bStdfDestination);
                                        sprintf(szString,
                                                "T%lu / T%lu &nbsp;  &nbsp; %s &nbsp; &nbsp;",
                                                ptIDDQ_Delta_OutlierPart->lTestNumber1,
                                                ptIDDQ_Delta_OutlierPart->lTestNumber2,
                                                strFailureType.toLatin1().constData());	// List of tests
                    // Write outlier test
                    WritePP_OutlierReportLine(szString,bStdfDestination);
                }
            }
        }
    }

    // Close table
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        WritePP_OutlierReportLine("</table>\n",bStdfDestination);
        WritePP_OutlierReportLine("<br><br>\n",bStdfDestination);
    }

end_iddq_delta_details:;
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n<br>\n",bStdfDestination);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For all sites: display Static-PAT test limits applied to each test
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QList<CPatDefinition*>			cPatResults;
    QList<int>                      lDynSites;

    // Create SPAT section (unless disabled)
    if(lPatInfo->GetRecipeOptions().bReport_SPAT_Limits == false)
        goto end_report_spat;

    SplitPatReportWebPage(strPatPage_SpatLimits);
    if(!bStdfDestination)
    {
        WritePageBreak();
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nStatic-PAT Test Limits\n",bStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"all_stat_pat_limits","Static-PAT Test Limits");
        SetPowerPointSlideName("Outliers: Static-PAT limits");
    }
    else
    {
        if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nStatic-PAT Test Limits\n",bStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"all_stat_pat_limits\"></a><font color=\"#006699\">Static-PAT Test Limits<br></font></h1>\n\n",bStdfDestination);
    }

    // list all sites, all test results.
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    // Sort Pat results: By increasing test number
    bool	bIgnoreTest;
    for(itPATDefinifion = lPatInfo->GetUnivariateRules().begin();
        itPATDefinifion != lPatInfo->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Only include tests that are StaticPAT enabled
        bIgnoreTest = false;
        if(ptPatDef == NULL)
            bIgnoreTest = true;
        if (ptPatDef->m_lFailStaticBin == -1)
            bIgnoreTest = true;
        if(bIgnoreTest)
            goto next_test_static_cell;

        cPatResults.append(ptPatDef);
next_test_static_cell:;
    }

    // Sort results in descending order
    qSort(cPatResults.begin(), cPatResults.end(), ComparePatResultNum);

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // Display table titles (if data available)
    if(cPatResults.count() > 0)
    {
        // Table titles
        if (strOutputFormat == "CSV") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_CSV)
        {
            WritePP_OutlierReportLine("Test number,Test name,SPAT Fail count,Data shape,Rule,N Factor,T Factor,Low Limit,High Limit,Low S.PAT Limit,High S.PAT Limit\n",bStdfDestination);
            WritePP_OutlierReportLine("-----------,-------------------------,----------,--------,--------,----------,----------,-------------,--------------,---------------,----------------\n",bStdfDestination);
        }
        else
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<table border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" width=\"100%\">\n",bStdfDestination);
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Test#</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"33%\" bgcolor=\"#CCFFFF\"><b>Test name</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#CCFFFF\"><b>SPAT<br>Fails</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#CCFFFF\"><b>Shape<br>(without outliers)</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#CCFFFF\"><b>Rule</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#CCFFFF\"><b>N</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#CCFFFF\"><b>T</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Original<br>Low Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Original<br>High Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Static PAT<br>Low Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Static PAT<br>High Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }
    }
    else
    {
        if(strOutputFormat==QString("CSV"))
            WritePP_OutlierReportLine("\nNo Static-PAT rule defined for any test.\n\n",bStdfDestination);		// case 3765, pyc, 13/04/2011
        else
            WritePP_OutlierReportLine("\n<br><br>No Static-PAT rule defined for any test.<br>\n\n",bStdfDestination);
    }

    // Review all Static-PAT tests definitions
    foreach(ptPatDef, cPatResults)
    {
        // Get pointer to test cell with data
        if(pFile->FindTestCell(ptPatDef->m_lTestNumber, ptPatDef->mPinIndex, &ptTestCell, true, false,
                               ptPatDef->m_strTestName) != 1)
        {
            continue;
        }

        // Get relevant limits set, as the STDF only allows one limit set, we need to specify the range of the two limits sets
        lfLowLimit = ptPatDef->m_lfLowStaticLimit;
        if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
            lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;
        lfHighLimit = ptPatDef->m_lfHighStaticLimit;
        if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
            lfHighLimit = GEX_TPAT_FLOAT_INFINITE;

        // Compute total SPAT failures for this test
        ldValue = GetTotalFailures(ptPatDef->m_lTestNumber, ptPatDef->mPinIndex,
                                   ptPatDef->m_strTestName, lPatInfo->m_lstOutlierParts);

        // Write Test limits
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_CSV)
        {
            WritePP_OutlierReportLine(	QString::number(ptPatDef->m_lTestNumber) + ",",bStdfDestination);
            WritePP_OutlierReportLine(	ptPatDef->m_strTestName + ",",bStdfDestination);
            // SPAT fail count
            WritePP_OutlierReportLine(QString::number(ldValue)+ ",",bStdfDestination);
            WritePP_OutlierReportLine(patlib_GetDistributionName(ptPatDef->m_iDistributionShape) + ",",bStdfDestination);
            WritePP_OutlierReportLine( QString(gexSpatRuleSetItemsGUI[ptPatDef->m_SPATRuleType]) + ",",bStdfDestination);
            switch(ptPatDef->m_SPATRuleType)
            {
                default:
                    // SPAT Relax rule: N*Sigma, or custom Limits
                    WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + QString(","), bStdfDestination);
                    WritePP_OutlierReportLine(QString(","), bStdfDestination);
                    break;

                case  GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA:
                    // SPAT AEC rule: 6*RobustSigma
                    WritePP_OutlierReportLine(QString(","), bStdfDestination);
                    WritePP_OutlierReportLine(QString(","), bStdfDestination);
                    break;

                case GEX_TPAT_SPAT_ALGO_NEWLIMITS:
                    WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + QString(","), bStdfDestination);
                    WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfSpatOutlierTFactor, 'g', 3) + QString(","), bStdfDestination);
                    break;

                case GEX_TPAT_SPAT_ALGO_RANGE:
                        // N factor: Range
                        WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + QString(","), bStdfDestination);
                        // T factor not relevant.
                        WritePP_OutlierReportLine(QString(","), bStdfDestination);
                    break;
            }

            // Original program: Low Limit
            if((ptPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
                WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfLowLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + ",", bStdfDestination);
            else
                WritePP_OutlierReportLine("-,",bStdfDestination);

            // Original program: High Limit
            if((ptPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfHighLimit, 'g', 6)  + QString(" ") + ptPatDef->m_strUnits + ",",bStdfDestination);
            else
                WritePP_OutlierReportLine("-,",bStdfDestination);

            // Static-PAT limits
            WritePP_OutlierReportLine(	QString::number(lfLowLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + ",",bStdfDestination);
            WritePP_OutlierReportLine(	QString::number(lfHighLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + "\n",bStdfDestination);
        }
        else
        //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);

            // Write Test# + hyperlink
            strTestNumber = QString::number(ptTestCell->lTestNumber);
            if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            {
                // Flat html to endup in one file
                strHyperlink = "href=\"#HistoT" + strTestNumber;
                strHyperlink += "\"";
            }
            else
            {
                // Regular html page
                strHyperlink = "href=\"#_gex_drill--drill_chart=adv_histo--data=";
                strHyperlink += strTestNumber;
                strHyperlink += "\"";
            }
            sprintf(szString,"<td width=\"10%%\" bgcolor=\"#F8F8F8\"><b><a %s> %s</a></b></td>\n",strHyperlink.toLatin1().constData(),strTestNumber.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);

            WritePP_OutlierReportLine("<td width=\"33%\" bgcolor=\"#F8F8F8\">" + ptPatDef->m_strTestName + "</td>\n",bStdfDestination);

            // SPAT fail count
            if(ldValue)
                ptColor = "FF0000";	// Alarm (fail count > 0)
            else
                ptColor = "FFFFCC";	// Fine (fail count = 0)
            strString.sprintf("<td align=\"center\" width=\"5%%\" bgcolor=\"#%s\">%ld</td>\n",ptColor,ldValue);
            WritePP_OutlierReportLine(strString,bStdfDestination);

            // Distribution shape
            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">" + QString(patlib_GetDistributionName(ptPatDef->m_iDistributionShape)) + "</td>\n",bStdfDestination);

            switch(ptPatDef->m_SPATRuleType)
            {
                default:
                    // SPAT Relax rule: N*Sigma or custom limits
                    WritePP_OutlierReportLine("<td width=\"8%\" bgcolor=\"#FFFF00\">" + QString(gexSpatRuleSetItemsGUI[ptPatDef->m_SPATRuleType]) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#FFFF00\">" + QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#FFFF00\">-</td>\n",bStdfDestination);
                        break;

                case  GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA:
                    // SPAT AEC rule: 6*RobustSigma
                    WritePP_OutlierReportLine("<td width=\"8%\" bgcolor=\"#F8F8F8\">" + QString(gexSpatRuleSetItemsGUI[ptPatDef->m_SPATRuleType]) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#F8F8F8\">-</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#F8F8F8\">-</td>\n",bStdfDestination);
                    break;

                case GEX_TPAT_SPAT_ALGO_NEWLIMITS:
                    WritePP_OutlierReportLine("<td width=\"8%\" bgcolor=\"#FFFF00\">" + QString(gexSpatRuleSetItemsGUI[ptPatDef->m_SPATRuleType]) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#FFFF00\">" + QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#FFFF00\">" + QString::number(ptPatDef->m_lfSpatOutlierTFactor, 'g', 3) + "</td>\n",bStdfDestination);
                    break;

                case GEX_TPAT_SPAT_ALGO_RANGE:
                    // Range: Only report N factor, T factor not relevant.
                    WritePP_OutlierReportLine("<td width=\"8%\" bgcolor=\"#FFFF00\">" + QString(gexSpatRuleSetItemsGUI[ptPatDef->m_SPATRuleType]) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#FFFF00\">" + QString::number(ptPatDef->m_lfSpatOutlierNFactor, 'g', 3) + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td align=\"center\" width=\"7%\" bgcolor=\"#F8F8F8\">-</td>\n",bStdfDestination);
                    break;
            }

            // Original program Low Limit
            if((ptPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
                WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(ptPatDef->m_lfLowLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
            else
                WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">n/a</td>\n",bStdfDestination);

            // Original program: High Limit
            if((ptPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(ptPatDef->m_lfHighLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
            else
                WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">n/a</td>\n",bStdfDestination);

            // Static-PAT limits
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(lfLowLimit)  + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(lfHighLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        };
    }

    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // If tests displayed in table, now is time to close it.
        if((cPatResults.count() > 0) && (lPatInfo->GetRecipeOptions().bReport_SPAT_Limits))
            WritePP_OutlierReportLine("</table>\n",bStdfDestination);
    }
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);
end_report_spat:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For all sites: display Dynamic-PAT test limits applied to each test
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create SPAT section (unless disabled)
    if((lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_Outliers == false) && (lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_NoOutliers == false))
        goto end_report_dpat;

    // Create new Web page (if required)
    SplitPatReportWebPage(strPatPage_DpatLimits);
    if(!bStdfDestination)
    {
        WritePageBreak();
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nDynamic-PAT Test Limits\n",bStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"all_dyn_pat_limits","Dynamic-PAT Test Limits");
        SetPowerPointSlideName("Outliers: Dynamic-PAT limits");
    }
    else
    {
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nDynamic-PAT Test Limits\n",bStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"all_dyn_pat_limits\"></a><font color=\"#006699\">Dynamic-PAT Test Limits<br></font></h1>\n\n",bStdfDestination);
    }

    // list all sites, all test results.
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Sort Pat results: By increasing test number
    cPatResults.clear();
    for(itPATDefinifion = lPatInfo->GetUnivariateRules().begin();
        itPATDefinifion != lPatInfo->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Only include tests that are Dynamic-PAT enabled
        bIgnoreTest = false;
        if(ptPatDef == NULL)
            bIgnoreTest = true;
        if (ptPatDef->m_lFailDynamicBin == -1 )
            bIgnoreTest = true;
        if(ptPatDef->mOutlierRule == GEX_TPAT_RULETYPE_IGNOREID)
            bIgnoreTest = true;

        // If outlier, check if we have to report outliers...
        if((ptPatDef->m_lSeverityScore > 0) && (lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_Outliers == false))
            bIgnoreTest = true;

        // If not outlier, check if we have to report tests without outlier...
        ptPatDef->computeSeverity();
        if((ptPatDef->m_lSeverityScore <= 0) && (lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_NoOutliers == false))
            bIgnoreTest = true;

        // See if ignore this test...
        if(bIgnoreTest)
            goto next_test_dynamic_cell;

        cPatResults.append(ptPatDef);
next_test_dynamic_cell:;
    }

    // Sort results in descending order
    qSort(cPatResults.begin(), cPatResults.end(), ComparePatResultNum);

    // Display table titles (if data avialable)
    if(cPatResults.count() > 0)
    {
        // If standard Web page (splitted): Write 'Return' hyperlink
        if(bSplitPatReportPages)
            WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

        // Table titles
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_CSV)
        {
            WritePP_OutlierReportLine("Test number,Test name,Site,DPAT Fails, Historical shape (all sites),Current shape / site,Outlier rule, N Factor,T Factor,Low Limit,High Limit,Low PAT Limit,High PAT Limit,Misc.\n",bStdfDestination);
            WritePP_OutlierReportLine("-----------,--------------------------,----,-------------,--------------,-------------,--------------,--------------,--------------,--------------,--------------,-------------,--------------,--------------\n",bStdfDestination);
        }
        else
        //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            sprintf(szString,"<table border=\"1\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"6%\" bgcolor=\"#CCFFFF\"><b>Test#</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"21%\" bgcolor=\"#CCFFFF\"><b>Test name</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"3%\" bgcolor=\"#CCFFFF\"><b>Site#</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"3%\" bgcolor=\"#CCFFFF\"><b>DPAT<br>Fails</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"5%\" bgcolor=\"#CCFFFF\"><b>Historical<br>Shape (all sites)</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"5%\" bgcolor=\"#CCFFFF\"><b>Current<br>Shape (per site)</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"5%\" bgcolor=\"#CCFFFF\"><b>Outlier<br>rule</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"4%\" bgcolor=\"#CCFFFF\"><b>N Factor</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"4%\" bgcolor=\"#CCFFFF\"><b>T Factor</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#CCFFFF\"><b>Original<br>Low Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#CCFFFF\"><b>Original<br>High Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"10%\" bgcolor=\"#CCFFFF\"><b>Dynamic PAT<br>Low Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td align=\"center\" width=\"10%\" bgcolor=\"#CCFFFF\"><b>Dynamic PAT<br>High Limit</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"16%\" bgcolor=\"#CCFFFF\"><b>-N / +N<br>-M / +M<br>-F / +F</b></td>\n",bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#CCFFFF\"><b>Misc. stats.</b>\n",bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }
    }
    else
    {
        WritePP_OutlierReportLine("\nNo Dynamic-PAT rule defined for any test.\n\n",bStdfDestination);
    }

    //////////////////////////////////////////////////////////////////////////
    // Review all Dynamic-PAT tests definitions
    //////////////////////////////////////////////////////////////////////////
    bool        bFirstVisibleSite;
    bool        bIgnoreThisSite;
    double      lfFactor;

    foreach(ptPatDef, cPatResults)
    {
        // Get pointer to test cell with data
        if(pFile->FindTestCell(ptPatDef->m_lTestNumber, ptPatDef->mPinIndex, &ptTestCell, true,
                               false, ptPatDef->m_strTestName) != 1)
            continue;

        lDynSites           = ptPatDef->GetDynamicSites();
        bFirstVisibleSite   = true;

        int lSiteNumber = -1;
        for (int lIdx = 0; lIdx < lDynSites.count(); ++lIdx)
        {
            lSiteNumber = lDynSites.at(lIdx);

            // See if site 255 must be ignored or not?
            if(lSiteNumber == 255)
            {
                // Site 255: only ignore if multiple sites are available; if only one site in file and is site255, then consider it valid!
                if(ptPatDef->mDynamicLimits.count() > 1)
                    bIgnoreThisSite = true;
                else
                    bIgnoreThisSite = false;

            }
            else
                bIgnoreThisSite = false;	// Not site255: always valid

            // Compute total DPAT fails
            ldValue = GetTotalFailures(ptPatDef->m_lTestNumber, ptPatDef->mPinIndex,
                                       ptPatDef->m_strTestName, lPatInfo->m_lstOutlierParts, lSiteNumber, true);

            // Check if this test has no outliers and if only report outliers!
            if (bIgnoreThisSite == false && (ldValue > 0 || lPatInfo->GetRecipeOptions().bReport_DPAT_Limits_NoOutliers == true))
            {
                // Retrieve the dynamic limits and stats computed for this site
                GS::PAT::DynamicLimits lDynLimits = ptPatDef->GetDynamicLimits(lSiteNumber);

                // Site exists, then list the dynamic limits used  (check which severity level must be used!)
                iSeverityLimits = ptPatDef->m_iOutlierLimitsSet;

                // Get relevant limits set, as the STDF only allows one limit set, we need to specify the range of the two limits sets
                lfLowLimit = gex_min(lDynLimits.mLowDynamicLimit1[iSeverityLimits], lDynLimits.mLowDynamicLimit2[iSeverityLimits]);
                if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
                    lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;
                lfHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[iSeverityLimits], lDynLimits.mHighDynamicLimit2[iSeverityLimits]);
                if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
                    lfHighLimit = GEX_TPAT_FLOAT_INFINITE;

                // Write Test limits
                if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_CSV)
                {
                    WritePP_OutlierReportLine(	QString::number(ptPatDef->m_lTestNumber) + ",",bStdfDestination);
                    WritePP_OutlierReportLine(	ptPatDef->m_strTestName + ",",bStdfDestination);

                    // If sites merged (-1), then display 'All' instead of site number
                    if (lSiteNumber == -1)
                        WritePP_OutlierReportLine("All,",bStdfDestination);
                    else
                        WritePP_OutlierReportLine(	QString::number(lSiteNumber) + ",",bStdfDestination);

                    // DPAT fail count
                    WritePP_OutlierReportLine(QString::number(ldValue) + ",",bStdfDestination);
                    // Historical data shape
                    WritePP_OutlierReportLine(patlib_GetDistributionName(ptPatDef->m_iDistributionShape) + ",",bStdfDestination);
                    // Dynamic data shape detected
                    WritePP_OutlierReportLine(patlib_GetDistributionName(ptPatDef->mDynamicLimits[lSiteNumber].mDistributionShape) + ",",bStdfDestination);
                    // Outlier rule
                    WritePP_OutlierReportLine(QString(gexRuleSetItemsGUI[ptPatDef->mOutlierRule]) + ",",bStdfDestination);

                    // N & T factors
                    sprintf(szString,"%g,",ptPatDef->m_lfOutlierNFactor);
                    WritePP_OutlierReportLine(szString,bStdfDestination);
                    sprintf(szString,"%g,",ptPatDef->m_lfOutlierTFactor);
                    WritePP_OutlierReportLine(szString,bStdfDestination);

                    // Original program: Low Limit
                    if((ptPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
                        WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfLowLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + ",", bStdfDestination);
                    else
                        WritePP_OutlierReportLine("-,",bStdfDestination);

                    // Original program: High Limit
                    if((ptPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                        WritePP_OutlierReportLine(QString::number(ptPatDef->m_lfHighLimit, 'g', 6)  + QString(" ") + ptPatDef->m_strUnits + ",",bStdfDestination);
                    else
                        WritePP_OutlierReportLine("-,",bStdfDestination);

                    // PAT limits
                    WritePP_OutlierReportLine(	QString::number(lfLowLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + ",",bStdfDestination);
                    WritePP_OutlierReportLine(	QString::number(lfHighLimit, 'g', 6) + QString(" ") + ptPatDef->m_strUnits + "\n",bStdfDestination);

                    // Misc.
                    WritePP_OutlierReportLine("-,",bStdfDestination);
                }
                else if(m_pReportOptions->isReportOutputHtmlBased())
                {
                    WritePP_OutlierReportLine("<tr>\n",bStdfDestination);

                    // Write Test# + hyperlink
                    strTestNumber = QString::number(ptTestCell->lTestNumber);
                    if(ptTestCell->lPinmapIndex >= 0)
                    {
                        strTestNumber+= "." + QString::number(ptTestCell->lPinmapIndex);
                    }
                    if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT") //if (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                    {
                        // Flat html to endup in one file
                        strHyperlink = "href=\"#HistoT" + strTestNumber;
                        strHyperlink += "\"";
                    }
                    else
                    {
                        // Regular html page
                        strHyperlink = "href=\"#_gex_drill--drill_chart=adv_histo--data=";
                        strHyperlink += strTestNumber;
                        strHyperlink += "\"";
                    }
                    sprintf(szString,"<td width=\"6%%\" bgcolor=\"#F8F8F8\"><b><a %s> %s</a></b></td>\n",strHyperlink.toLatin1().constData(),strTestNumber.toLatin1().constData());
                    WritePP_OutlierReportLine(szString,bStdfDestination);

                    // Only display Test name on first visible site#
                    if(bFirstVisibleSite)
                        WritePP_OutlierReportLine("<td width=\"21%\" bgcolor=\"#F8F8F8\">" + ptPatDef->m_strTestName + "</td>\n",bStdfDestination);
                    else
                        WritePP_OutlierReportLine("<td width=\"21%\" bgcolor=\"#F8F8F8\">&nbsp;</td>\n",bStdfDestination);

                    // If sites merged (-1), then display 'All' instead of site number
                    if (lSiteNumber == -1)
                        WritePP_OutlierReportLine("<td align=\"center\" width=\"3%\" bgcolor=\"#F8F8F8\">All</td>\n",bStdfDestination);
                    else
                        WritePP_OutlierReportLine("<td align=\"center\" width=\"3%\" bgcolor=\"#F8F8F8\">" + QString::number(lSiteNumber) + "</td>\n",bStdfDestination);

                    // DPAT fail count (for all sites)
                    if(ldValue)
                        ptColor = "FF0000";	// Alarm (fail count > 0)
                    else
                        ptColor = "FFFFCC";	// Fine (fail count = 0)
                    strString.sprintf("<td align=\"center\" width=\"3%%\" bgcolor=\"#%s\">%ld</td>\n",ptColor,ldValue);
                    WritePP_OutlierReportLine(strString,bStdfDestination);

                    // Historical data shape
                    WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#F8F8F8\">" + QString(patlib_GetDistributionName(ptPatDef->m_iDistributionShape)) + "</td>\n",bStdfDestination);

                    // Dynamic data shape detected (highlight with RED background if not matching historical shape...)
                    if(patlib_IsDistributionMatchingHistory(ptPatDef->m_iDistributionShape,ptPatDef->mDynamicLimits[lSiteNumber].mDistributionShape))
                    {
                        // No mismatch, simply display regular shape text
                        WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#F8F8F8\">" + QString(patlib_GetDistributionName(ptPatDef->mDynamicLimits[lSiteNumber].mDistributionShape)) + "</td>\n",bStdfDestination);
                    }
                    else
                    {
                        // Distribution MISMATCH: Force light red/pink background + Bold text
                        WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#FEDCD0\"><b>" + QString(patlib_GetDistributionName(ptPatDef->mDynamicLimits[lSiteNumber].mDistributionShape)) + "</b></td>\n",bStdfDestination);

                        lPatInfo->lPatShapeMismatch++;	// Update total number of distributions that mismatch from historical shapes.
                    }

                    // Outlier rule
                    if(bFirstVisibleSite)
                        WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#F8F8F8\">" + QString(gexRuleSetItemsGUI[ptPatDef->mOutlierRule]) + "</td>\n",bStdfDestination);
                    else
                        WritePP_OutlierReportLine("<td width=\"5%\" bgcolor=\"#F8F8F8\"></td>\n",bStdfDestination);

                    // N & T factors
                    lfFactor = patlib_GetOutlierFactor(lPatInfo,ptPatDef,lSiteNumber,true);
                    if(lfFactor)
                        sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%.2g</td>\n",lfFactor);
                    else
                        sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">-</td>\n");
                    WritePP_OutlierReportLine(szString,bStdfDestination);

                    lfFactor = patlib_GetOutlierFactor(lPatInfo,ptPatDef,lSiteNumber,false);
                    if(lfFactor)
                        sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%.2g</td>\n",lfFactor);
                    else
                        sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">-</td>\n");
                    WritePP_OutlierReportLine(szString,bStdfDestination);

                    // Original program limits (Only display Test name on first visible site#)
                    if(bFirstVisibleSite)
                    {
                        // Low Limit
                        if((ptPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
                            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">" + QString::number(ptPatDef->m_lfLowLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
                        else
                            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">n/a</td>\n",bStdfDestination);

                        // Original program: High Limit
                        if((ptPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">" + QString::number(ptPatDef->m_lfHighLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
                        else
                            WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">n/a</td>\n",bStdfDestination);

                        // Clear flag
                        bFirstVisibleSite = false;
                    }
                    else
                    {
                        WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">&nbsp;</td>\n",bStdfDestination);
                        WritePP_OutlierReportLine("<td width=\"7%\" bgcolor=\"#F8F8F8\">&nbsp;</td>\n",bStdfDestination);
                    }

                    // PAT limits
                    WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(lfLowLimit)  + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);
                    WritePP_OutlierReportLine("<td width=\"10%\" bgcolor=\"#F8F8F8\">" + QString::number(lfHighLimit) + QString(" ") + ptPatDef->m_strUnits + "</td>\n",bStdfDestination);

                    // Write Near, Medium and Far limits (HTML report only, not CSV)
                    strString = "<td width=\"16%\" bgcolor=\"#F8F8F8\">";
                    for(ldValue=0;ldValue < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES; ldValue++)
                    {
                        lfLowLimit = gex_min(lDynLimits.mLowDynamicLimit1[ldValue], lDynLimits.mLowDynamicLimit2[ldValue]);
                        if(lfLowLimit < -GEX_TPAT_FLOAT_INFINITE)
                            lfLowLimit = -GEX_TPAT_FLOAT_INFINITE;
                        lfHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[ldValue], lDynLimits.mHighDynamicLimit2[ldValue]);
                        if(lfHighLimit > GEX_TPAT_FLOAT_INFINITE)
                            lfHighLimit = GEX_TPAT_FLOAT_INFINITE;
                        strString += QString::number(lfLowLimit);
                        strString += " / " + QString::number(lfHighLimit);
                        if(ldValue < 2)
                            strString += "<br>";	// Line feed between line
                        else
                            strString += "</td>\n";	// Last line (FAR imits)
                    }
                    WritePP_OutlierReportLine(strString,bStdfDestination);

                    // Misc: display low-level statitics used for computing DPAT limits
                    strStatsDetails="<td>";
                    switch(ptPatDef->mOutlierRule)
                    {
                        case GEX_TPAT_RULETYPE_SIGMAID:			// Rule 'sigma' ID (offset in combo)
                            strStatsDetails += "Mean=" + QString::number(lDynLimits.mDynMean);
                            strStatsDetails += "<br>Sigma=" + QString::number(lDynLimits.mDynSigma);
                            break;
                        case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:			// Rule 'N*RobustSigma' ID (offset in combo)
                            strStatsDetails += "Median=" + QString::number(lDynLimits.mDynQ2);
                            strStatsDetails += "<br>RSigma=" + QString::number((lDynLimits.mDynQ3 - lDynLimits.mDynQ1)/1.35);
                            break;
                        case GEX_TPAT_RULETYPE_Q1Q3IQRID:			// Rule 'Q1/Q3 IQR' ID (offset in combo)
                            strStatsDetails += "Q1=" + QString::number(lDynLimits.mDynQ1);
                            strStatsDetails += "<br>Q3=" + QString::number(lDynLimits.mDynQ3);
                            strStatsDetails += "<br>IQR=" + QString::number(lDynLimits.mDynQ3 - lDynLimits.mDynQ1);
                            break;
                        case GEX_TPAT_RULETYPE_LIMITSID:			// Rule '%limit' ID (offset in combo)
                        case GEX_TPAT_RULETYPE_NEWLIMITSID:			// Rule 'newlimit' ID (offset in combo)
                        case GEX_TPAT_RULETYPE_RANGEID:			// Rule 'range' ID (offset in combo)
                        case GEX_TPAT_RULETYPE_SMARTID:			// Rule 'smart' ID (offset in combo)
                        case GEX_TPAT_RULETYPE_IGNOREID:			// Rule 'disabled' ID (offset in combo)
                        case GEX_TPAT_RULETYPE_SEPARATORID:			// Rule separator
                        case GEX_TPAT_RULETYPE_GAUSSIANID:			// Rule: Gaussian distribution
                        case GEX_TPAT_RULETYPE_GAUSSIANTAILID:			// Rule: Gaussian+Tail distribution
                        case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:		// Rule: Gaussian+Tail distribution
                        case GEX_TPAT_RULETYPE_LOGNORMALID:					// Rule: LogNormal distribution
                        case GEX_TPAT_RULETYPE_BIMODALID:			// Rule: Bi-Modal distribution, with each mode clearly apart from the other
                        case GEX_TPAT_RULETYPE_MULTIMODALID:			// Rule: Multi-Modal distribution
                        case GEX_TPAT_RULETYPE_CLAMPEDID:			// Rule: Clamped distribution
                        case GEX_TPAT_RULETYPE_DUALCLAMPEDID:			// Rule: DoubleClamped distribution
                        case GEX_TPAT_RULETYPE_CATEGORYID:			// Rule: Categories distribution
                        case GEX_TPAT_RULETYPE_CUSTOMLIBID:
                            strStatsDetails += "Mean=" + QString::number(lDynLimits.mDynMean);
                            strStatsDetails += "Median=" + QString::number(lDynLimits.mDynQ2);
                            strStatsDetails += "<br>Sigma=" + QString::number(lDynLimits.mDynSigma);
                            strStatsDetails += "Q1=" + QString::number(lDynLimits.mDynQ1);
                            strStatsDetails += "<br>Q3=" + QString::number(lDynLimits.mDynQ3);
                            strStatsDetails += "<br>IQR=" + QString::number(lDynLimits.mDynQ3 - lDynLimits.mDynQ1);
                            break;
                        default:
                            break;
                    }
                    strStatsDetails += "</td>\n";
                    WritePP_OutlierReportLine(strStatsDetails,bStdfDestination);
                    WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
                } // HTML report
            }
        }
    }

    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // If tests displayed in table, now is time to close it.
        if(cPatResults.count() > 0)
            WritePP_OutlierReportLine("</table>\n",bStdfDestination);
    }
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

end_report_dpat:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Outlier Failures (summary) found in the file (on selected/good parts only)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SplitPatReportWebPage(strPatPage_Summary);
    // Title + bookmark
    if(!bStdfDestination)
    {
        WritePageBreak();
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Tests failing PAT limits\n",bStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"all_tests_dyn_fails","Summary: Tests failing PAT limits");
        SetPowerPointSlideName("Outliers: Failing tests");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Tests failing PAT limits\n",bStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"all_tests_dyn_fails\"><font color=\"#006699\">Summary: Tests failing PAT limits<br></font></h1>\n\n",bStdfDestination);
    }
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // If PowerPoint: Number of lines in table on first page (normally it's 20, but first page also lists global settings!).
    iTableLine = 15;
    lTotalFailures = 0;	// Keeps track of total tests that failed the PAT limits.
    lLineInTable = 0;	// Keeps track of total tests in current table & page (cleared at each new page)

    // Sort (make pareto) of Pat results: top of table is highest fail count
    cPatResults.clear();
    for(itPATDefinifion = lPatInfo->GetUnivariateRules().begin();
        itPATDefinifion != lPatInfo->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Only include tests that are Dynamic-PAT enabled
        bIgnoreTest = false;
        if(ptPatDef == NULL)
            bIgnoreTest = true;
        if(!ptPatDef->m_TotalFailures)
            bIgnoreTest = true;
        if(bIgnoreTest)
            goto next_test_cell_pareto;

        cPatResults.append(ptPatDef);
next_test_cell_pareto:;
    }

    // Sort results in descending order
    qSort(cPatResults.begin(), cPatResults.end(), ComparePatResultScore);

    // Write lines
    foreach(ptPatDef, cPatResults)
    {
        // Write the table header to fill
        if(lLineInTable == 0)
            BuildDPAT_SummaryHtmlTable(bStdfDestination);

        // Build test# string
        if(ptPatDef->m_lTestNumber != (unsigned) -1)
        {
            strTestNumber = QString::number(ptPatDef->m_lTestNumber);
            if(ptPatDef->mPinIndex >= 0)
            {
                strTestNumber += ".";
                strTestNumber += ptPatDef->mPinIndex;
            }
        }
        else
            strTestNumber="";

        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            sprintf(szString,"%s,",strTestNumber.toLatin1().constData());	// No test number defined
            WritePP_OutlierReportLine(szString,bStdfDestination);

            if(ptPatDef->m_strTestName.isEmpty() == false)
            {
                sprintf(szString,"%s,",ptPatDef->m_strTestName.toLatin1().constData());
                WritePP_OutlierReportLine(szString,bStdfDestination);
            }
            else
                WritePP_OutlierReportLine(",,",bStdfDestination);	// No test name defined

            WritePP_OutlierReportLine(gexIgnoreSamplesSetItemsGUI[ptPatDef->m_SamplesToIgnore] + QString(","),bStdfDestination);
            WritePP_OutlierReportLine(gexKeepOutliersSetItemsGUI[ptPatDef->m_OutliersToKeep] + QString(","),bStdfDestination);
            WritePP_OutlierReportLine(gexOutlierLimitsSetItemsGUI[ptPatDef->m_iOutlierLimitsSet] + QString(","),bStdfDestination);

            sprintf(szString,"%ld,",ptPatDef->m_lSeverityScore);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"%ld,",ptPatDef->m_lStaticFailuresLow);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"%ld,",ptPatDef->m_lStaticFailuresHigh);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"%ld - %ld - %ld,",
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"%ld - %ld - %ld,",
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"%ld,\n",ptPatDef->m_TotalFailures);
            WritePP_OutlierReportLine(szString,bStdfDestination);
        }
        else
        //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);

            if (strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
                //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            {
                // Flat html to endup in one file
                strHyperlink = "href=\"#HistoT" + strTestNumber;
                strHyperlink += "\"";
            }
            else
            {
                // Regular html page
                strHyperlink = "href=\"#_gex_drill--drill_chart=adv_histo--data=";
                strHyperlink += strTestNumber;
                strHyperlink += "\"";
            }
            sprintf(szString,"<td bgcolor=\"#F8F8F8\"><b><a %s> %s</a></b></td>\n",strHyperlink.toLatin1().constData(),strTestNumber.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td bgcolor=\"#F8F8F8\">%s</td>\n",ptPatDef->m_strTestName.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",gexIgnoreSamplesSetItemsGUI[ptPatDef->m_SamplesToIgnore]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",gexKeepOutliersSetItemsGUI[ptPatDef->m_OutliersToKeep]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%s</td>\n",gexOutlierLimitsSetItemsGUI[ptPatDef->m_iOutlierLimitsSet]);
            WritePP_OutlierReportLine(szString,bStdfDestination);

            // Severity score
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\"><b>%ld</b></td>\n",ptPatDef->m_lSeverityScore);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%ld &nbsp;&nbsp;/&nbsp;&nbsp; %ld</td>\n",ptPatDef->m_lStaticFailuresLow, ptPatDef->m_lStaticFailuresHigh);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%ld - %ld - %ld</td>\n",
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],
                ptPatDef->m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\">%ld - %ld - %ld</td>\n",
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],
                ptPatDef->m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            sprintf(szString,"<td align=\"center\" bgcolor=\"#F8F8F8\"><b>%ld</b></td>\n",ptPatDef->m_TotalFailures);
            WritePP_OutlierReportLine(szString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
        }

        // Keep track of total failing tests
        lTotalFailures++;
        lLineInTable++;

        // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
        if(!bStdfDestination &&
           (strOutputFormat=="PPT") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
           && ((lTotalFailures % iTableLine) == 0))
        {
            // close table & goto next page
            fprintf(hReportFile,"</table>\n");
            WritePageBreak();

            // Reset line counter in page (so will reopen table if needed)
            lLineInTable = 0;

            // Number of lines in table (page 2 and following) is 20
            iTableLine = 20;
        }
    };

    cPatResults.clear();

    // If no failure at all, just say it!
    if(lTotalFailures == 0)
        WritePP_OutlierReportLine("No test failed any of the Outlier limits!\n\n",bStdfDestination);

    // close HTML codes.
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        if(lLineInTable > 0)
            WritePP_OutlierReportLine("</table>\n",bStdfDestination);
    }
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // MV PAT summary
    BuildMVPATRulesSummaryPage(strPatPage_MVPAT_Summary, bSplitPatReportPages, bStdfDestination);

    // MV PAT Rule details
    BuildMVPATRulesDetailsPage(strPatPage_MVPAT_Details, bSplitPatReportPages, bStdfDestination);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NNR Summary
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Check if NNR report to be created
    BuildNNRSummaryPage(strPatPage_NNR_Summary, bSplitPatReportPages, bStdfDestination);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IDDQ-Delta Summary
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // IDDQ variables
    QMap <QString,int> cmapIDDQ_Summary;
    QMap <QString,int>::Iterator it_iddq_delta_list;

    SplitPatReportWebPage(strPatPage_IDDQ_Delta_Summary);

    // Check if IDDQ-Delta report to be created
    if(lPatInfo->GetRecipeOptions().mIsIDDQ_Delta_enabled == false)
        goto end_iddq_delta_summary;

    // Title + bookmark
    if(!bStdfDestination)
    {
        WritePageBreak();
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Tests failing IDDQ-Delta\n",bStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"iddq_delta_summary","Summary: Tests failing IDDQ-Delta");
        SetPowerPointSlideName("Summary: IDDQ-Delta Outliers");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nSummary: Tests failing IDDQ-Delta\n",bStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"iddq_delta_summary\"><font color=\"#006699\">Summary: Tests failing IDDQ-Delta<br></font></h1>\n\n",bStdfDestination);
    }
    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a><br>\n",bStdfDestination);

    // Read list of IDDQ-Delta tests, and count failing rate for each (using a Qmap)
    foreach(CPatOutlierIDDQ_Delta * ptIDDQ_Delta_TestFailure, lPatInfo->pIDDQ_Delta_OutlierTests)
    {
        strString = QString::number(ptIDDQ_Delta_TestFailure->lTestNumber1) + "." + QString::number(ptIDDQ_Delta_TestFailure->lPinmapIndex1);
        if(cmapIDDQ_Summary.contains(strString))
            cmapIDDQ_Summary[strString] = cmapIDDQ_Summary[strString] + 1;
        else
            cmapIDDQ_Summary[strString] = 1;
    };

    // Sort (make pareto) of IDDQ-Delta results: top of table is highest fail count
    it_iddq_delta_list = cmapIDDQ_Summary.begin();

    for( ; it_iddq_delta_list != cmapIDDQ_Summary.end(); ++it_iddq_delta_list )
    {
        strString = it_iddq_delta_list.key();

        ptPatDef = new CPatDefinition;
        // Extract Test#.Pinmap from key string
        ptPatDef->m_lTestNumber = strString.section('.',0,0).toInt();
        ptPatDef->mPinIndex = strString.section('.',1,1).toInt();
        ptPatDef->m_lSeverityScore = it_iddq_delta_list.value();

        cPatResults.append(ptPatDef);
    }
    qSort(cPatResults.begin(), cPatResults.end(), ComparePatResultScore);

    // Write NNR summary section
    if(cPatResults.count() <= 0)
        goto end_iddq_delta_summary;

    // Pointer to first PAT definition
//    ptPatDefList = *lPatInfo->mPATDefinitions.begin();

    // Create table header
    BuildIDDQ_Delta_SummaryHtmlTable(bStdfDestination);
    foreach(ptPatDef, cPatResults)
    {
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Test#
            WritePP_OutlierReportLine(QString::number(ptPatDef->m_lTestNumber) + QString(","),bStdfDestination);

            // Test name
            WritePP_OutlierReportLine(lPatInfo->GetTestName(ptPatDef->m_lTestNumber,ptPatDef->mPinIndex) + QString(","),bStdfDestination);

            // IDDQ-Delta fail count
            WritePP_OutlierReportLine(QString::number(ptPatDef->m_lSeverityScore) + ",",bStdfDestination);
        }
        else
        //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            WritePP_OutlierReportLine("<tr>\n",bStdfDestination);

            // Hyperlink name
            strTestNumber = QString::number(ptPatDef->m_lTestNumber);


            // Test# + hyperlink
            sprintf(szString,"<td bgcolor=\"#F8F8F8\" width=\"15%%\"><b>%s</b></td>\n",strTestNumber.toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);

            // Test name
            sprintf(szString,"<td bgcolor=\"#F8F8F8\" width=\"70%%\">%s</td>\n",lPatInfo->GetTestName(ptPatDef->m_lTestNumber,ptPatDef->mPinIndex).toLatin1().constData());
            WritePP_OutlierReportLine(szString,bStdfDestination);

            // IDDQ-Delta fail count
            sprintf(szString,
                    "<td align=\"center\" bgcolor=\"#F8F8F8\" width=\"15%%\">%ld</td>\n",
                    ptPatDef->m_lSeverityScore);
            WritePP_OutlierReportLine(szString,bStdfDestination);

        }
    }

    // close HTML codes.
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if(m_pReportOptions->isReportOutputHtmlBased())
        WritePP_OutlierReportLine("</table>\n<br>",bStdfDestination);

end_iddq_delta_summary:;
    // If no IDDQ-Delta failure at all, just say it!
    if(cPatResults.count() == 0)
    {
        if(strOutputFormat==QString("CSV"))
            WritePP_OutlierReportLine("\nNo test failed the IDDQ-Delta rules!\n",bStdfDestination);		// case 3765, pyc, 13/04/2011
        else
            WritePP_OutlierReportLine("\n<br>No test failed the IDDQ-Delta rules!\n<br>",bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

    // Empty temporarly IDDQ-Delta sorted list
    qDeleteAll(cPatResults);
    cPatResults.clear();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Reticle Summary
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SplitPatReportWebPage(strPatPage_Reticle_Summary);

    const QList<PATOptionReticle>& lReticleRules = lPatInfo->GetRecipeOptions().GetReticleRules();
    // Reticle rules enabled?
    if (lPatInfo->GetRecipeOptions().GetReticleEnabled())
    {
        // Title + bookmark
        if(!bStdfDestination)
        {
            WritePageBreak();
            if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                WritePP_OutlierReportLine("\n\nSummary: Reticle yield levels\n",bStdfDestination);
            else
                WriteHtmlSectionTitle(hReportFile,"reticle_summary","Summary: Reticle yield levels");
            SetPowerPointSlideName("Reticle Yield");
        }
        else
        {
            // Creating report buffer to store into STDF.DTR records
            if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                WritePP_OutlierReportLine("\n\nSummary: Reticle yield levels\n",bStdfDestination);
            else
                WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"reticle_summary\"><font color=\"#006699\">Summary: Reticle yield levels<br></font></h1>\n\n",bStdfDestination);
        }

        // If standard Web page (splitted): Write 'Return' hyperlink
        if(bSplitPatReportPages)
            WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

//        QHash<int, double> lPatternResults;

        // Begin the list items section
        if (strOutputFormat != "CSV")
        {
            WritePP_OutlierReportLine("<ul>",bStdfDestination);

            // Add all rules
            for (int lIdxRule = 0; lIdxRule < lReticleRules.count(); ++lIdxRule)
            {
                const PATOptionReticle& lReticle = lReticleRules.at(lIdxRule);

                if (lReticle.IsReticleEnabled())
                {
                    // Write reticle info:
                    if (strOutputFormat=="CSV")
                    {
                        strString = lReticle.GetRuleName() + "\n";
                        WritePP_OutlierReportLine(strString,bStdfDestination);
                    }
                    else
                    {
                        strString = "<li> <a href=\"#rule" + QString::number(lIdxRule) + "\">" + lReticle.GetRuleName() + "</a></li>\n";
                        WritePP_OutlierReportLine(strString,bStdfDestination);
                    }
                }

                // Reticle section, only created with Repeating Pattern rule
                /*if (lReticle.GetRule() == PATOptionReticle::REPEATING_PATTERNS && lReticle.IsReticleEnabled())
                {
                    lPatternResults = lPatInfo->GetReticleRepeatingPatternResults(lReticle.GetRuleName());

                    // Check if results available
                    if(lPatternResults.count() == 0)
                    {
                        WritePP_OutlierReportLine("\n\nNo Reticle results!\n",bStdfDestination);
                    }
                    else
                    {
                        // Write reticle info:
                        if (strOutputFormat=="CSV")
                        {
                            strString = "Reticle rule name: " + lReticle.GetRuleName() + "\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);
                            strString = "Reticle Bad bins alarm level: " +
                                        QString::number(lReticle.GetReticleYieldThreshold(),'f',2) + " %\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);

                            strString = "Reticle Bad bins list: " +
                                        lReticle.GetBadBinsReticleList().GetRangeList() + "\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);

                            WritePP_OutlierReportLine("\nReticle yield level (per reticle die location):\n",bStdfDestination);
                        }
                        else if(m_pReportOptions->isReportOutputHtmlBased())
                        {
                            strString = "<p>Reticle rule name: " + lReticle.GetRuleName() + "<br>\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);
                            strString = "Reticle Bad bins alarm level: " +
                                        QString::number(lReticle.GetReticleYieldThreshold(),'f',2) + " %<br>\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);

                            strString = "Reticle Bad bins list: " +
                                        lReticle.GetBadBinsReticleList().GetRangeList() + "<br>\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);

                            WritePP_OutlierReportLine("<br><b>Reticle Bad-Bins yield level (per reticle die location):</b></p>\n",bStdfDestination);
                        }

                        // Create table + write headers: ReticleSizeX * ReticleSizeY
                        BuildReticleHtmlTable(bStdfDestination);

                        // Retrieve wafer map where Reticle information are located
                        lWaferReticle = lPatInfo->GetReticleStepInformation();
                        if (lWaferReticle == NULL &&
                                lPatInfo->GetRecipeOptions().GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FILE)
                        {
                            GSLOG(SYSLOG_SEV_WARNING, "Unable to find Reticle Information from data file");
                            goto end_reticle;
                        }

                        unsigned int lReticleSizeX, lReticleSizeY;

                        switch (lPatInfo->GetRecipeOptions().GetReticleSizeSource())
                        {
                            case PATOptionReticle::RETICLE_SIZE_FILE:
                                lReticleSizeX = lWaferReticle->GetReticleWidth();
                                lReticleSizeY = lWaferReticle->GetReticleHeight();
                                break;

                            default:
                                lReticleSizeX = lPatInfo->GetRecipeOptions().GetReticleSizeX();
                                lReticleSizeY = lPatInfo->GetRecipeOptions().GetReticleSizeY();
                                break;
                        }

                        // Fill table
                        int iIndex;
                        for(lLineInTable = 0; lLineInTable < lReticleSizeY; lLineInTable++)
                        {
                            // Open table line
                            if (strOutputFormat=="CSV")
                                strString = QString::number(1+lLineInTable) + ",";
                            else if(m_pReportOptions->isReportOutputHtmlBased())
                                strString = "<tr>\n<td align=\"center\" bgcolor=\"#CCECFF\"><b>" + QString::number(1+lLineInTable) + "</b></td>\n";
                            WritePP_OutlierReportLine(strString,bStdfDestination);

                            // Fill line
                            for(lColInTable = 0; lColInTable < lReticleSizeX; lColInTable++)
                            {
                                // Get reticle die location yield level
                                iIndex = lColInTable + (lLineInTable*lReticleSizeX);

                                if (lPatternResults.contains(iIndex))
                                    lfValue = lPatternResults.value(iIndex);
                                else
                                    lfValue = -1;

                                if(lfValue < 0 || lfValue > 100)
                                    strValue = "n/a";
                                else
                                {
                                    // Convert Good bin yield value to Fail Bin yield
                                    lfValue = 100 - lfValue;

                                    strValue = strString.sprintf("%.1lf %%",lfValue);
                                }

                                // Generating .CSV report file.
                                if (strOutputFormat=="CSV")
                                {
                                    strString = strValue + ",";
                                }
                                else if(m_pReportOptions->isReportOutputHtmlBased())
                                {
                                    strString = "<td align=\"center\"";
                                    // Check which background color to be used: RED = Alram
                                    if(lfValue > lReticle.GetReticleYieldThreshold())
                                        strString += " bgcolor=\"#F4715C\">";	// Red background
                                    else
                                        strString += " bgcolor=\"#D4FED4\">";	// Green background
                                    strString +=  strValue + "</td>\n";
                                }
                                WritePP_OutlierReportLine(strString,bStdfDestination);
                            }

                            // End of line
                            if (strOutputFormat=="CSV")
                                WritePP_OutlierReportLine("\n",bStdfDestination);
                            else
                                WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
                        }

                        // close HTML codes.
                        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                            WritePP_OutlierReportLine("\n",bStdfDestination);
                        else if(m_pReportOptions->isReportOutputHtmlBased())
                            WritePP_OutlierReportLine("</table>\n",bStdfDestination);
                    }
                }
            */
            }
        }

        // Close the list items
        if (strOutputFormat != "CSV")
            WritePP_OutlierReportLine("</ul>",bStdfDestination);

        // Create the step defectivity check sections
        for (int lIdxRule = 0; lIdxRule < lReticleRules.count(); ++lIdxRule)
        {
            const PATOptionReticle& lReticle = lReticleRules.at(lIdxRule);

            if (lReticle.GetRule() == PATOptionReticle::REPEATING_PATTERNS)
                CreateRepeatingPatternSection(lReticle, *lPatInfo, lIdxRule, strOutputFormat, bStdfDestination);
            else if (lReticle.GetRule() == PATOptionReticle::STEP_DEFECTIVITY_CHECK)
                CreateStepDefectivityCheckSection(lReticle, *lPatInfo, lIdxRule, strOutputFormat, bStdfDestination);
            else if (lReticle.GetRule() == PATOptionReticle::CORNER)
                CreateCornerRuleSection(lReticle, *lPatInfo, lIdxRule, strOutputFormat, bStdfDestination);
        }

        // If standard Web page (splitted): Write 'Return' hyperlink
        if(bSplitPatReportPages)
            WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);
    }

    SplitPatReportWebPage(strPatPage_detailed_reticle);
    if (lPatInfo->GetRecipeOptions().GetReticleEnabled())
    {
        // Title + bookmark
        if(!bStdfDestination)
        {
            WritePageBreak();
            if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                WritePP_OutlierReportLine("\n\nParts failing Reticle algorithm\n",bStdfDestination);
            else
                WriteHtmlSectionTitle(hReportFile,"advanced_detailed_reticle","Parts failing Reticle algorithm");
            SetPowerPointSlideName("Reticle Yield");
        }
        else
        {
            // Creating report buffer to store into STDF.DTR records
            if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                WritePP_OutlierReportLine("\n\nParts failing Reticle algorithm\n",bStdfDestination);
            else
                WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"advanced_detailed_reticle\"><font color=\"#006699\">Parts failing Reticle algorithm<br></font></h1>\n\n",bStdfDestination);
        }

        // If standard Web page (splitted): Write 'Return' hyperlink
        if(bSplitPatReportPages)
            WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

        // If no outliers
        tdGPATOutliers lReticleOutliers = lPatInfo->mReticleOutliers;
        tdGPATOutliers::iterator lOutlier = lReticleOutliers.begin();
        tdGPATOutliers::iterator lLastOutlier = lReticleOutliers.end();
        if (lReticleOutliers.count() == 0)
            WritePP_OutlierReportLine("\n\nNo outliers detected for the reticle rules.\n",bStdfDestination);
        else
        {
            if (strOutputFormat=="CSV") //if (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            {
                // Generating .CSV report file.
                WritePP_OutlierReportLine("LotID,WaferID,PartID,PatBin,DieX,DieY,Site,Tests failing PAT\n",bStdfDestination);
            }
            else
            {
                // Open HTML table with correlation results
                sprintf(szString,"<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iHthmSmallFontSizePixels);
                WritePP_OutlierReportLine(szString,bStdfDestination);

                WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
                sprintf(szString,"<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>LotID</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Wafer</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"15%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Reticle Pos X</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"15%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Reticle Pos Y</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieX</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>DieY</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Site</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                sprintf(szString,"<td width=\"40%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>Reticle rule</b></td>\n");
                WritePP_OutlierReportLine(szString,bStdfDestination);
                WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
            }

            const CWaferMap * lWaferReticle = lPatInfo->GetReticleStepInformation();


            for (; lOutlier != lLastOutlier; ++lOutlier)
            {
                CPatDieCoordinates lCoordinate = lOutlier.value();
                int lIndex;
                // Generating .CSV report file.
                strString = "";
                if (strOutputFormat=="CSV")
                {
                    strString = strLotID + "," + strWaferID + ",";
                    if (lWaferReticle)
                    {
                        lWaferReticle->indexFromCoord(lIndex, lCoordinate.mDieX, lCoordinate.mDieY);
                        CWafMapArray lWafMapArray = lWaferReticle->getWafMapDie(lIndex);
                        strString += QString::number(lWafMapArray.GetReticlePosX()) + ",";
                        strString += QString::number(lWafMapArray.GetReticlePosY()) + ",";
                    }
                    else
                    {
                        strString += "n/a,";
                        strString += "n/a,";
                    }
                    strString += QString::number(lCoordinate.mDieX) + ",";
                    strString += QString::number(lCoordinate.mDieY) + ",";
                    strString +=  QString::number(lCoordinate.mSite) + "," + lCoordinate.mRuleName;
                }
                else if(m_pReportOptions->isReportOutputHtmlBased())
                {
                    strString = "<tr><td align=\"center\">" + strLotID + "</td>";
                    strString += "<td align=\"center\">" + strWaferID + "</td>";
                    if (lWaferReticle)
                    {
                        lWaferReticle->indexFromCoord(lIndex, lCoordinate.mDieX, lCoordinate.mDieY);
                        CWafMapArray lWafMapArray = lWaferReticle->getWafMapDie(lIndex);
                        strString += "<td align=\"center\">" + QString::number(lWafMapArray.GetReticlePosX()) + "</td>";
                        strString += "<td align=\"center\">" + QString::number(lWafMapArray.GetReticlePosY()) + "</td>";
                    }
                    else
                    {
                        strString += "<td align=\"center\">n/a</td>";
                        strString += "<td align=\"center\">n/a</td>";
                    }
                    strString += "<td align=\"center\">" + QString::number(lCoordinate.mDieX) + "</td>";
                    strString += "<td align=\"center\">" + QString::number(lCoordinate.mDieY) + "</td>";
                    strString += "<td align=\"center\">" + QString::number(lCoordinate.mSite) + "</td>";
                    strString += "<td align=\"center\">" + lCoordinate.mRuleName + "</td>\n";
                }
                WritePP_OutlierReportLine(strString,bStdfDestination);

                // End of line
                if (strOutputFormat=="CSV")
                    WritePP_OutlierReportLine("\n",bStdfDestination);
                else
                    WritePP_OutlierReportLine("</tr>\n",bStdfDestination);

            }
            // close HTML codes.
            if (strOutputFormat=="CSV")
                WritePP_OutlierReportLine("\n",bStdfDestination);
            else if(m_pReportOptions->isReportOutputHtmlBased())
                WritePP_OutlierReportLine("</table>\n",bStdfDestination);
        }
    }
//end_reticle:;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // WARNING Log
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SplitPatReportWebPage(strPatPage_WarningLog);
    // Title + bookmark
    if(!bStdfDestination)
    {
        WritePageBreak();
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nWarning Log\n",bStdfDestination);
        else
            WriteHtmlSectionTitle(hReportFile,"warning_log","Warning Log");
        SetPowerPointSlideName("Warning Log");
    }
    else
    {
        // Creating report buffer to store into STDF.DTR records
        if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            WritePP_OutlierReportLine("\n\nWarning Log\n",bStdfDestination);
        else
            WritePP_OutlierReportLine("<h1 align=\"left\"><a name=\"warning_log\"><font color=\"#006699\">Warning Log<br></font></h1>\n\n",bStdfDestination);
    }

    // If standard Web page (splitted): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a><br><br>\n",bStdfDestination);


    // Check if results available
    if(lPatInfo->m_strLogWarnings.count() <= 0)
    {
        WritePP_OutlierReportLine("\n\nNo Warning detected during PAT processing!\n",bStdfDestination);
        goto end_warning_log;
    }
    for(int iIndex = 0; iIndex < lPatInfo->m_strLogWarnings.count(); iIndex++)
    {
        strString = lPatInfo->m_strLogWarnings.at(iIndex);
        if (strOutputFormat!="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            strString += "<br>";
        strString += "\n";
        WritePP_OutlierReportLine(strString,bStdfDestination);
    }
    // Creating report buffer to store into STDF.DTR records
    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        WritePP_OutlierReportLine("\n",bStdfDestination);
    else
        WritePP_OutlierReportLine("<br>\n",bStdfDestination);

    // If standard Web page (split): Write 'Return' hyperlink
    if(bSplitPatReportPages)
        WritePP_OutlierReportLine("<br>Back to <a href=\"advanced.htm\">Table of contents</a>  |  <a href=\"#_gex_pat_report.htm\"><img src=\"../images/save.png\"> Export to file</a>\n",bStdfDestination);

end_warning_log:;
    ///////////////////////////////////////
    // End of PAT report.
    ///////////////////////////////////////
}


void CGexReport::CreateStepDefectivityCheckSection(const PATOptionReticle& lReticle,
                                                   const CPatInfo &lPatInfo,
                                                   int lIdxRule,
                                                   const QString& strOutputFormat,
                                                   bool bStdfDestination)
{
    QString         lString;
    QJsonObject     lResults = lPatInfo.GetReticleResults(lReticle.GetRuleName());

    if (lReticle.IsReticleEnabled() && lReticle.GetRule() == PATOptionReticle::STEP_DEFECTIVITY_CHECK)
    {
        QString lFieldSelection("All Reticle Fields");
        if (lReticle.GetFieldSelection() == PATOptionReticle::LIST_RETICLE_FIELDS)
            lFieldSelection = PATOptionReticle::FieldCoordinatesToString(lReticle.GetFieldCoordinates());
        else if(lReticle.GetFieldSelection() == PATOptionReticle::EDGE_RETICLE_FIELDS)
            lFieldSelection = "Edge Reticle Fields";

        // Write reticle info:
        if (strOutputFormat == "CSV")
        {
            lString = "Rule Name :" + lReticle.GetRuleName() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Rule Type: Reticle Step Defectivity Check \n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Bad Bins Alarm: " + QString::number(lReticle.GetFieldThreshold()) + " %\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Bad Bins List: " + lReticle.GetBadBinsReticleList().GetRangeList() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Fields selected: " + lFieldSelection + "\n \n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Reticle Field yield level (per reticle field location):\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            QJsonArray lSteps = lResults["values"].toArray();

            if (lSteps.isEmpty() == false)
            {
                // create the header
                WritePP_OutlierReportLine("Reticle PosX,Reticle Pos Y, failure,status\n",bStdfDestination);
                for (int i=0; i<lSteps.size(); ++i)
                {
                    QJsonObject lStep = lSteps[i].toObject();
                    lString =   QString::number(lStep["PosX"].toInt()) + ","
                                + QString::number(lStep["PosY"].toInt()) + ","
                                + lStep["FailYield"].toString() + " %,"
                                + lStep["Status"].toString() + "\n";
                    WritePP_OutlierReportLine(lString,bStdfDestination);
                }
            }
            else
            {
                lString = "No reticle fields analyzed. Please check your filters.\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }

            WritePP_OutlierReportLine("\n \n",bStdfDestination);
        }
        else
        {
            lString = "<h2 id=\"rule" + QString::number(lIdxRule) + "\">" + lReticle.GetRuleName() + ":</h2>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // create the table
            lString = "<table border=\"0\" cellspacing=\"2\" width=\"98%\" style=\"border-collapse: collapse\"";
            lString += " cellpadding=\"0\">\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Rule Type:</td>\n", bStdfDestination);
            lString = "<td>Reticle Step Defectivity Check</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Bad Bins Alarm:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetFieldThreshold(), 'f', 2)  + " %</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Bad Bins List:</td>\n", bStdfDestination);
            lString = "<td>" + lReticle.GetBadBinsReticleList().GetRangeList()  + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Fields selected:</td>\n", bStdfDestination);
            lString = "<td>" + lFieldSelection + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("</table>\n",bStdfDestination);

            lString = "<br><b>Reticle Field yield level (per reticle field location):</b><br>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            QJsonArray lSteps = lResults["values"].toArray();

            if (lSteps.isEmpty() == false)
            {
                // create the table
                lString = "<table border=\"0\" cellspacing=\"2\" width=\"60%\" style=\"font-size: ";
                lString += QString::number(iHthmSmallFontSizePixels);
                lString += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
                // create the header
                WritePP_OutlierReportLine("<tr>\n",bStdfDestination);
                lString = "<td width=\"25%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Reticle PosX</b></td>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
                lString = "<td width=\"25%\"  bgcolor=\"#CCECFF\" align=\"center\"><b>Reticle Pos Y</b></td>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
                lString = "<td width=\"25%\" bgcolor=\"#CCECFF\" align=\"center\"><b> failure</b></td>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
                lString = "<td width=\"25%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Status</b></td>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
                WritePP_OutlierReportLine("</tr>\n",bStdfDestination);
                for (int i=0; i<lSteps.size(); ++i)
                {
                    QJsonObject lStep = lSteps[i].toObject();
                    lString = "<tr><td align=\"center\">" + QString::number(lStep["PosX"].toInt()) + "</td>";
                    lString += "<td align=\"center\">" + QString::number(lStep["PosY"].toInt()) + "</td>";
                    lString += "<td align=\"center\">" + lStep["FailYield"].toString() + " %</td>";
                    lString += "<td align=\"left\">" + lStep["Status"].toString() + "</td></tr>";
                    WritePP_OutlierReportLine(lString,bStdfDestination);
                }
                WritePP_OutlierReportLine("</table>\n",bStdfDestination);
            }
            else
            {
                lString = "<p style=\"color:red\"><b>No reticle fields anayzed. Please check your filters.</b></p>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }
        }
    }
}

void CGexReport::CreateRepeatingPatternSection(const PATOptionReticle &lReticle, const CPatInfo &lPatInfo,
                                               int lIdxRule, const QString &strOutputFormat, bool bStdfDestination)
{
    QString         lString;
    QJsonObject     lResults = lPatInfo.GetReticleResults(lReticle.GetRuleName());

    // fill the repeating pattern sections
    if (lReticle.IsReticleEnabled() && lReticle.GetRule() == PATOptionReticle::REPEATING_PATTERNS)
    {
        // Write reticle info:
        if (strOutputFormat == "CSV")
        {
            lString = "Rule Name: " + lReticle.GetRuleName() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Rule Type: Repeating Pattern \n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Bad Bins Alarm: " + QString::number(lReticle.GetReticleYieldThreshold(), 'f', 2) + " %\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Bad Bins List: " + lReticle.GetBadBinsReticleList().GetRangeList() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Mask Name: " + lReticle.GetReticleMaskName() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Reticle Bad-Bins yield level (per reticle die location):\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // Create CSV Heat Map
            if (CreateReticleHeatmapCSV(lResults.value("values").toArray(), lPatInfo, bStdfDestination) == false)
                WritePP_OutlierReportLine("Unable to create reticle heat map\n",bStdfDestination);

            WritePP_OutlierReportLine("\n\n",bStdfDestination);
        }
        else
        {
            lString = "<h2 id=\"rule" + QString::number(lIdxRule) + "\">" + lReticle.GetRuleName() + ":</h2>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // create the table
            lString = "<table border=\"0\" cellspacing=\"2\" width=\"98%\" style=\"border-collapse: collapse\"";
            lString += " cellpadding=\"0\">\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Rule Type:</td>\n", bStdfDestination);
            lString = "<td>Repeating Pattern</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Bad Bins Alarm:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetReticleYieldThreshold(), 'f', 2)  + " %</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Bad Bins List:</td>\n", bStdfDestination);
            lString = "<td>" + lReticle.GetBadBinsReticleList().GetRangeList()  + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Mask Name:</td>\n", bStdfDestination);
            lString = "<td>" + lReticle.GetReticleMaskName() + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("</table>\n", bStdfDestination);

            lString = "<br><b>Reticle Bad-Bins yield level (per reticle die location):</b><br>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // Create CSV Heat Map
            QString lFilename   = QString("reticle-heatmap-%1").arg(lIdxRule);
            QString lImagePath  = m_pReportOptions->strReportDirectory + "/images/" + lFilename;
            if (CreateReticleHeatmapImage(lResults.value("values").toArray(), lPatInfo,
                                          lImagePath, lReticle.GetReticleYieldThreshold()) == false)
            {
                GSLOG(SYSLOG_SEV_WARNING, "Unable to create reticle heat map");

                lString = "<p style=\"color:red\"><b>Unable to create reticle heat map image</b></p>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }
            else
            {
                lString = "<img src=\"../images/" + formatHtmlImageFilename(lFilename) + "\">\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }
        }
    }
}

void CGexReport::CreateCornerRuleSection(const PATOptionReticle &lReticle, const CPatInfo &lPatInfo, int lIdxRule, const QString &strOutputFormat, bool bStdfDestination)
{
    QString         lString;
    QJsonObject     lResults = lPatInfo.GetReticleResults(lReticle.GetRuleName());

    // fill the repeating pattern sections
    if (lReticle.IsReticleEnabled() && lReticle.GetRule() == PATOptionReticle::CORNER)
    {
        QStringList lCorners;
        if (lReticle.IsActivatedCorner(PATOptionReticle::CORNER_TOP_LEFT))
            lCorners.append("TOP_LEFT");
        if (lReticle.IsActivatedCorner(PATOptionReticle::CORNER_TOP_RIGHT))
            lCorners.append("TOP_RIGHT");
        if (lReticle.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_LEFT))
            lCorners.append("BOTTOM_LEFT");
        if (lReticle.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_RIGHT))
            lCorners.append("BOTTOM_RIGHT");

        if (lReticle.IsActivatedCorner(PATOptionReticle::TOP))
            lCorners.append("TOP_EDGE");
        if (lReticle.IsActivatedCorner(PATOptionReticle::RIGHT))
            lCorners.append("RIGHT_EDGE");
        if (lReticle.IsActivatedCorner(PATOptionReticle::BOTTOM))
            lCorners.append("BOTTOM_EDGE");
        if (lReticle.IsActivatedCorner(PATOptionReticle::LEFT))
            lCorners.append("LEFT_EDGE");

        // Write reticle info:
        if (strOutputFormat == "CSV")
        {
            lString = "Rule Name: " + lReticle.GetRuleName() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Rule Type: Corner Rule \n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Bad Bins List: " + lReticle.GetBadBinsReticleList().GetRangeList() + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Activated Corners: ";
            lString += lCorners.join("|") + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "X ink: " + QString::number(lReticle.GetXInk()) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Y ink: " + QString::number(lReticle.GetYInk()) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Diagonal ink: " + QString::number(lReticle.GetDiagInk()) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Diagonal X ink: " + QString::number(lReticle.GetXOffDiag()) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Diagonal Y ink: " + QString::number(lReticle.GetYOffDiag()) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Ignore Diagonal bad: " + QString((lReticle.IgnoreDiagonalBadDies() ? "True" : "False")) + "\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            lString = "Reticle outliers yield level (per reticle die location):\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // Create CSV Heat Map
            if (CreateReticleHeatmapCSV(lResults.value("values").toArray(), lPatInfo, bStdfDestination) == false)
                WritePP_OutlierReportLine("Unable to create reticle heat map\n",bStdfDestination);

            WritePP_OutlierReportLine("\n \n",bStdfDestination);
        }
        else
        {
            lString = "<h2 id=\"rule" + QString::number(lIdxRule) + "\">" + lReticle.GetRuleName() + ":</h2>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // create the table
            lString = "<table border=\"0\" cellspacing=\"2\" width=\"98%\" style=\"border-collapse: collapse\"";
            lString += " cellpadding=\"0\">\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Rule Type:</td>\n", bStdfDestination);
            lString = "<td>Corner Rule</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Bad Bins List:</td>\n", bStdfDestination);
            lString = "<td>" + lReticle.GetBadBinsReticleList().GetRangeList()  + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Activated Corners:</td>\n", bStdfDestination);
            lString = "<td>" + lCorners.join("|") + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">X ink:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetXInk()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Y ink:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetYInk()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Diagonal ink:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetDiagInk()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Diagonal X ink:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetXOffDiag()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Diagonal Y ink:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.GetYOffDiag()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("<tr>\n", bStdfDestination);
            WritePP_OutlierReportLine("<td width=\"15%\">Ignore Diagonal bad:</td>\n", bStdfDestination);
            lString = "<td>" + QString::number(lReticle.IgnoreDiagonalBadDies()) + "</td>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);
            WritePP_OutlierReportLine("</tr>\n", bStdfDestination);

            WritePP_OutlierReportLine("</table>\n", bStdfDestination);

            lString = "<br><b>Reticle outliers yield level (per reticle die location):</b><br>\n";
            WritePP_OutlierReportLine(lString,bStdfDestination);

            // Create PNG Heat Map
            QString lFilename   = QString("reticle-heatmap-%1").arg(lIdxRule);
            QString lImagePath  = m_pReportOptions->strReportDirectory + "/images/" + lFilename;
            if (CreateReticleHeatmapImage(lResults.value("values").toArray(), lPatInfo, lImagePath) == false)
            {
                GSLOG(SYSLOG_SEV_WARNING, "Unable to create reticle heat map");

                lString = "<p style=\"color:red\"><b>Unable to create reticle heat map image</b></p>\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }
            else
            {
                lString = "<img src=\"../images/" + formatHtmlImageFilename(lFilename) + "\">\n";
                WritePP_OutlierReportLine(lString,bStdfDestination);
            }
        }
    }
}

bool CGexReport::CreateReticleHeatmapImage(const QJsonArray &data, const CPatInfo& lPatInfo, const QString &filename,
                                           double threshold)
{
    unsigned int    lReticleSizeX;
    unsigned int    lReticleSizeY;

    // Retrieve wafer map where Reticle information are located
    const CWaferMap * lWaferReticle = lPatInfo.GetReticleStepInformation();
    if (lWaferReticle == NULL && lPatInfo.GetRecipeOptions().GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FILE)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Unable to find Reticle Information from data file");
        return false;
    }

    switch (lPatInfo.GetRecipeOptions().GetReticleSizeSource())
    {
        case PATOptionReticle::RETICLE_SIZE_FILE:
            lReticleSizeX = lWaferReticle->GetReticleWidth();
            lReticleSizeY = lWaferReticle->GetReticleHeight();
            break;

        default:
            lReticleSizeX = lPatInfo.GetRecipeOptions().GetReticleSizeX();
            lReticleSizeY = lPatInfo.GetRecipeOptions().GetReticleSizeY();
            break;
    }

    if (lReticleSizeX == 0 || lReticleSizeY == 0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Retile Height or Width is equal to 0. Unable to render the reticle heat map.");
        return false;
    }

    int lCellSize = qMax(420/lReticleSizeX, 330/lReticleSizeY);

    // Set the chatr size and plot area size based on the number of cells and cell size
    CChartDirector lChartDirectorEngine;
    if (lChartDirectorEngine.allocateXYChart((lReticleSizeX + 1) * lCellSize + 160,
                                            (lReticleSizeY + 1) * lCellSize + 50,
                                            Chart::Transparent) == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Unable to instantiate Chart engine.");
        return false;
    }
    XYChart * lChart = lChartDirectorEngine.xyChart();

    lChart->setPlotArea(30, 30,(lReticleSizeX + 1) * lCellSize, (lReticleSizeY + 1) * lCellSize,
                        -1, -1, -1, Chart::Transparent, -1);

    // Set up the x-axis and y-axis
    lChart->setXAxisOnTop(true);
    lChart->xAxis()->setLinearScale(1, lReticleSizeX, 1);
    lChart->yAxis()->setLinearScale(1, lReticleSizeY, 1);
    lChart->xAxis()->setMargin(lCellSize, lCellSize);
    lChart->yAxis()->setMargin(lCellSize, lCellSize);
    lChart->xAxis()->setLabelStyle("arialbd.ttf");
    lChart->yAxis()->setLabelStyle("arialbd.ttf");
    lChart->yAxis()->setReverse(true);

    // Create a dummy contour layer to use the color axis
    ContourLayer *  lLayer      = lChart->addContourLayer(DoubleArray(0, 0), DoubleArray(0, 0), DoubleArray(0, 0));
    ColorAxis *     lColorAxis  = lLayer->setColorAxis(lChart->getPlotArea()->getLeftX() + lChart->getPlotArea()->getWidth() + 30,
                                                       lChart->getPlotArea()->getTopY(), Chart::TopLeft,
                                                       lChart->getPlotArea()->getHeight(), Chart::Right);

    // Find max z value
    double lZMax = 0.0;
    for (int lIdx = 0; lIdx < data.count(); ++lIdx)
    {
        lZMax = qMax(lZMax, data.at(lIdx).toObject().value("z").toDouble());
    }

    // Specify the color and scale used and the threshold position
    const int colors[] = { 0x0000ff, 0x008080, 0x00ff00, 0xcccc00, 0xff0000 };
    lColorAxis->setColorGradient(true, IntArray(colors, (int)(sizeof(colors) / sizeof(colors[0]))));
    lColorAxis->setLinearScale(0, ceil(lZMax), ceil(lZMax));

    // Add Threshold marker on color scale
    if (threshold != GEX_C_DOUBLE_NAN)
        lColorAxis->addMark(threshold, 0x000000, "Threshold", "arialbd.ttf", 8);

    // Set color axis labels to use Arial Bold font
    lColorAxis->setLabelStyle("arialbd.ttf");
    lColorAxis->setLabelFormat("{value}%");

    // Create a dummy chart with the color axis and call BaseChart::layout so that the
    // Axis.getColor can be used.
    CChartDirector lDummyChartDirectorEngine;
    if (lDummyChartDirectorEngine.allocateXYChart(10,10, Chart::Transparent) == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Unable to instantiate Chart engine.");
        return false;
    }
    XYChart *   lDummyChart     = lDummyChartDirectorEngine.xyChart();
    ColorAxis * lDummyColorAxis = lDummyChart->addContourLayer(DoubleArray(0, 0),
                                                               DoubleArray(0, 0), DoubleArray(0, 0))->colorAxis();
    lDummyColorAxis->setColorGradient(true, IntArray(colors, (int)(sizeof(colors) / sizeof(colors[0]))));
    lDummyColorAxis->setLinearScale(0, ceil(lZMax), 5);
    lDummyChart->layout();

    // Add the cells as scatter symbols
    for (int lIdx = 0; lIdx < data.count(); ++lIdx)
    {
        double lXCoord  = 1 + data.at(lIdx).toObject().value("x").toDouble();
        double lYCoord  = 1 + data.at(lIdx).toObject().value("y").toDouble();
        double lZCoord  = data.at(lIdx).toObject().value("z").toDouble();
        int    lColor   = lDummyColorAxis->getColor(lZCoord);

        if (threshold != GEX_C_DOUBLE_NAN && lZCoord >= threshold)
        {
            lChart->addScatterLayer(DoubleArray(&lXCoord, 1), DoubleArray(&lYCoord, 1), "", Chart::CircleSymbol,
                                    lCellSize/2, 0x00000, 0x888888);
        }

        lChart->addScatterLayer(DoubleArray(&lXCoord, 1), DoubleArray(&lYCoord, 1), "", Chart::SquareSymbol,
                                lCellSize, lColor, 0x888888)->addExtraField(DoubleArray(&lZCoord, 1));
    }

    // Output the chart
    lChart->makeChart(filename.toLatin1().constData());

    return true;
}

bool CGexReport::CreateReticleHeatmapCSV(const QJsonArray &data, const CPatInfo& lPatInfo, bool bStdfDestination)
{
    unsigned int    lReticleSizeX;
    unsigned int    lReticleSizeY;
    QString         lString;

    // Retrieve wafer map where Reticle information are located
    const CWaferMap * lWaferReticle = lPatInfo.GetReticleStepInformation();
    if (lWaferReticle == NULL && lPatInfo.GetRecipeOptions().GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FILE)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Unable to find Reticle Information from data file");
        return false;
    }

    switch (lPatInfo.GetRecipeOptions().GetReticleSizeSource())
    {
        case PATOptionReticle::RETICLE_SIZE_FILE:
            lReticleSizeX = lWaferReticle->GetReticleWidth();
            lReticleSizeY = lWaferReticle->GetReticleHeight();
            break;

        default:
            lReticleSizeX = lPatInfo.GetRecipeOptions().GetReticleSizeX();
            lReticleSizeY = lPatInfo.GetRecipeOptions().GetReticleSizeY();
            break;
    }

    // Transform jsonarray to hash table with x and y pair as key and z as value
    QHash<QPair<int,int>, double> lValues;
    int     lX;
    int     lY;
    for (int lIdxData = 0; lIdxData < data.count(); ++lIdxData)
    {
        lX = data.at(lIdxData).toObject().value("x").toInt();
        lY = data.at(lIdxData).toObject().value("y").toInt();

        lValues.insert(QPair<int,int>(lX,lY), data.at(lIdxData).toObject().value("z").toDouble());
    }

    // create the header
    lString = "Line   \\   Row,";
    for (int lIdx = 0; lIdx < (int) lReticleSizeX; ++lIdx)
    {
        lString += QString::number(lIdx+1) + ",";
    }
    lString += "\n";
    WritePP_OutlierReportLine(lString,bStdfDestination);

    for (int lYIndex = 0; lYIndex < (int) lReticleSizeY; ++lYIndex)
    {
        lString = QString::number(lYIndex+1) + ",";
        for (int lXIndex = 0; lXIndex < (int) lReticleSizeX; ++lXIndex)
        {
            if (lValues.contains(QPair<int,int>(lXIndex, lYIndex)))
                lString += QString::number(lValues.value(QPair<int,int>(lXIndex, lYIndex)), 'f', 2) + " %,";
            else
                lString += "n/a,";
        }
        lString += "\n";
        WritePP_OutlierReportLine(lString,bStdfDestination);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Writes Outier Removal 'advanced report' section (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvOutlierRemovalReport()
{
    QString strReportType;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="DOC") strReportType = "word";
    else if (strOutputFormat=="CSV") strReportType = "excel";
    else if (strOutputFormat=="HTML") strReportType = "html";
    else if (strOutputFormat=="PDF") strReportType = "pdf";
    else if (strOutputFormat=="PPT") strReportType = "ppt";
    else if (strOutputFormat=="ODT") strReportType = "odt";

    /*
    switch(pReportOptions->iOutputFormat)
    {
        case	GEX_OPTION_OUTPUT_CSV:
        default:
            strReportType = "excel";
            break;
        case	GEX_OPTION_OUTPUT_HTML:
            strReportType = "html";
            break;
        case	GEX_OPTION_OUTPUT_WORD:
            strReportType = "word";
            break;
        case	GEX_OPTION_OUTPUT_PDF:
            strReportType = "pdf";
            break;
        case	GEX_OPTION_OUTPUT_PPT:
            strReportType = "ppt";
            break;
    }
    */

    // Write the Outlier Report pages into the file, not into the buffer string (reserved when dumping report into STDF.DTR records)
    BuildPP_OutlierRemovalReport(strReportType,false);

    // Empty buffer
    strPatTraceabilityReport = "";
}

#ifdef GCORE15334

/////////////////////////////////////////////////////////////////////////////
// Writes Outier Removal 'advanced report' section into STDF created by Post-Processing PAT
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WritePP_OutlierRemovalReport(GS::StdLib::Stdf *pStdf,
                                                 GS::Gex::PATProcessing &cFields,
                                                 QString strString/*=""*/)
{
    // Do not embed PAT-Man report in STDF file if this feature is disabled
    if(cFields.bEmbedStdfReport == false)
        return;

    // If string specified, then use ito insted of internal PAT report string buffer
    if(strString.isEmpty() == false)
        strPatTraceabilityReport = strString;

    // Write DTRs (maximum of 255 characters per DTR)
    QString	strDtrLine;
    long	lOffset=0;	// Starting offset in buffer to copy into DTR
    long	lLength = strPatTraceabilityReport.length();
    int		iSize;	// Buffer size to copy.
    while(lLength-lOffset > 0)
    {
        GS::StdLib::StdfRecordReadInfo StdfRecordHeader;
        StdfRecordHeader.iRecordType = 50;
        StdfRecordHeader.iRecordSubType = 30;
        pStdf->WriteHeader(&StdfRecordHeader);

        // Write DTR prefix string (13 characters)
        strDtrLine = "<cmd> logPAT ";
        iSize = lLength - lOffset;
        if(iSize > 230) iSize = 230;

        // Write upto 230 characters of the string to dump
        strDtrLine += strPatTraceabilityReport.mid(lOffset,iSize);

        // Write String into STDF.DTR record
        pStdf->WriteString(strDtrLine.toLatin1().constData());
        pStdf->WriteRecord();

        // Updates remaining buffer size to dump in DTRs
        lOffset += iSize;
    };
}
#endif
