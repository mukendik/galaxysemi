/////////////////////////////////////////////////////////////////////////////
// Creates chart 'ProbabilityPlot'
/////////////////////////////////////////////////////////////////////////////

#include "browser_dialog.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include <math.h>
#include "gexprobabilityplotchart.h"
#include <gqtl_log.h>
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "gqtl_global.h"
#include "product_info.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// report_page_adv_histo.cpp
extern bool				isSingleTestAllLayers(CGexChartOverlays *pChartsInfo);
//
extern bool				CTestFailCountLessThan(CTest* s1, CTest* s2);

// cstats.cpp
extern double			ScalingPower(int iPower);

// http://home.online.no/~pjacklam/notes/invnorm/impl/natarajan/
#define A1		(-3.969683028665376e+01)
#define A2		2.209460984245205e+02
#define A3		(-2.759285104469687e+02)
#define A4		1.383577518672690e+02
#define A5		(-3.066479806614716e+01)
#define A6		2.506628277459239e+00

#define B1		(-5.447609879822406e+01)
#define B2		1.615858368580409e+02
#define B3		(-1.556989798598866e+02)
#define B4		6.680131188771972e+01
#define B5		(-1.328068155288572e+01)

#define C1		(-7.784894002430293e-03)
#define C2		(-3.223964580411365e-01)
#define C3		(-2.400758277161838e+00)
#define C4		(-2.549732539343734e+00)
#define C5		4.374664141464968e+00
#define C6		2.938163982698783e+00

#define D1		7.784695709041462e-03
#define D2		3.224671290700398e-01
#define D3		2.445134137142996e+00
#define D4		3.754408661907416e+00

#define	P_LOW   0.02425
/* P_high = 1 - p_low*/
#define P_HIGH  0.97575

long double normsinv(long double p)
{
    long double x	= GEX_C_DOUBLE_NAN;
    long double q	= 0.0;
    long double r	= 0.0;

#if defined unix || __MACH__
    long double u, e;
#endif
    if ((0 < p )  && (p < P_LOW)){
       q = sqrt(-2*log(p));
       x = (((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
    }
    else{
            if ((P_LOW <= p) && (p <= P_HIGH)){
               q = p - 0.5;
               r = q*q;
               x = (((((A1*r+A2)*r+A3)*r+A4)*r+A5)*r+A6)*q /(((((B1*r+B2)*r+B3)*r+B4)*r+B5)*r+1);
            }
            else{
                    if ((P_HIGH < p)&&(p < 1)){
                       q = sqrt(-2*log(1-p));
                       x = -(((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
                    }
            }
    }

#if defined unix || __MACH__
    // If you are compiling this under UNIX OR LINUX, you may uncomment this block for better accuracy.
    if(( 0 < p)&&(p < 1)){
       e = 0.5 * erfc(-x/sqrt(2)) - p;
       u = e * sqrt(2*M_PI) * exp(x*x/2);
       x = x - u/(1 + x*u/2);
    }
#endif

    return x;
}

double uniformOrderStatisticMedian(int nIndex, int nSize)
{
    double dPercentPoint = 0.0;

    if (nIndex == 1)
        dPercentPoint = 1 - uniformOrderStatisticMedian(nSize, nSize);
    else if (nIndex == nSize)
        dPercentPoint = GS_POW(0.5, (1.0/(double)nSize));
    else
        dPercentPoint = ((double) nIndex - 0.3165) / ((double) nSize + 0.365);

    return dPercentPoint;
}

double approximationOrderStatistic(int nIndex, int nSize)
{
    double dPercentPoint = (double) nIndex / (double) (nSize+1);

    return dPercentPoint;
}

/////////////////////////////////////////////////////////////////////////////
// Retrieve the chart director symbol for plot accorind to the Gex combo
/////////////////////////////////////////////////////////////////////////////
Chart::SymbolType convertToChartDirectorSpot(int nSpotIndex)
{
    Chart::SymbolType spotType = Chart::SquareSymbol;

    switch(nSpotIndex)
    {
        case 0	:	spotType = Chart::CircleSymbol;
                    break;

        case 1	:	spotType = Chart::SquareSymbol;
                    break;

        case 2	:	spotType = Chart::DiamondSymbol;
                    break;

        case 3	:	spotType = Chart::CrossSymbol;
                    break;

        case 4	:	spotType = Chart::Cross2Symbol;
                    break;

        case 5	:	spotType = Chart::TriangleSymbol;
                    break;

        case 6	:	spotType = Chart::InvertedTriangleSymbol;
                    break;

        case 7	:	spotType = Chart::LeftTriangleSymbol;
                    break;

        case 8	:	spotType = Chart::RightTriangleSymbol;
                    break;

        default :	break;
    }

    return spotType;
}

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvProbabilityPlot(BOOL /*bValidSection*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,"");

    mAdvancedTestsListToReport.clear();

    CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'adv_histo' page & header
    long	iTestsBoxPlotExInPage=0;
    if(of=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Generating HTML report file.
        mTotalAdvancedPages = 1;// Number of Histogram HTML pages that will be generated

        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        if (!pGroup)
        {
            GSLOG(SYSLOG_SEV_WARNING, " error : first pGroup in getGroupsList() is NULL or inexistant !");
            return GS::StdLib::Stdf::ErrorMemory;
        }

        int N=0;
        if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
        {
            bool ok=false;
            N=QString( m_pReportOptions->strAdvancedTestList ).section(' ',1,1).toInt(&ok);
            if (!ok)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString(" failed to understand '%1'")
                      .arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData() );
                N=5;
            }

            GSLOG(SYSLOG_SEV_NOTICE,
                  QString(" will consider only top %1 failed tests ")
                  .arg( N).toLatin1().constData());
        }

        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        QString htmlpb=m_pReportOptions->GetOption("output", "html_page_breaks").toString();
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();

        while(ptTestCell != NULL)
        {
            // We create one ADV_Histogram HTML page per X tests
            if((iTestsBoxPlotExInPage >= MAX_HISTOGRAM_PERPAGE) && (htmlpb=="true"))
            {
                // Once X tests are in the page, reset counter, increment page count
                iTestsBoxPlotExInPage=0;
                mTotalAdvancedPages++;
            }

            // Check if valid test (samples and not master test of a MultiParametric)
            if((CGexReport::validSamplesExecsOverGroups(ptTestCell->lTestNumber,
                                                        ptTestCell->lPinmapIndex,
                                                        ptTestCell->strTestName,
                                                        getGroupsList()) <= 0)
                    || (ptTestCell->lResultArraySize > 0))
                goto NextTestCell;

            // If not a parametric / multiparametric (eg: functional) test, ignore!
            if(ptTestCell->bTestType == 'F')
                goto NextTestCell;

            // Ignore Generic Galaxy Parameters
            {
                if((ptTestCell->bTestType == '-')
                    && (strOptionStorageDevice == "hide"))
                    goto NextTestCell;
            }

            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
                mAdvancedTestsListToReport.append(ptTestCell);	// we will sort and trim later
            else if (m_pReportOptions->strAdvancedTestList=="all")
                mAdvancedTestsListToReport.append(ptTestCell);
            else
            {
                // assuming we are in given TestsList	mode
                if (m_pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex))
                    mAdvancedTestsListToReport.append(ptTestCell);
                else
                    goto NextTestCell;
            }

            // Saves page# where histogram will be
            ptTestCell->iHtmlAdvancedPage= mTotalAdvancedPages;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
                ptTestCell->iHtmlAdvancedPage= 1;
            // Total number of tests indexed in current page
            iTestsBoxPlotExInPage++;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
            {
                iTestsBoxPlotExInPage--;
                //iTotalTestsInReport++;
            }
            NextTestCell:
            // Point to next test cell
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.

        // Will be used while creating all Histogram pages
        mTotalHtmlPages = mTotalAdvancedPages;

        // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
        m_pReportOptions->lAdvancedHtmlPages = mTotalAdvancedPages;

        if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
        {
            qSort(mAdvancedTestsListToReport.begin(), mAdvancedTestsListToReport.end(), CTestFailCountLessThan);
            // forget undesired Tests
            while(mAdvancedTestsListToReport.size()>N)
                mAdvancedTestsListToReport.removeLast();
            // put all top tests in the same page
            foreach(ptTestCell, mAdvancedTestsListToReport)
            {
                ptTestCell->iHtmlAdvancedPage = 1;
            }
        }
    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvProbabilityPlot(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting trend data!
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (of=="HTML"))
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();

        // Open Advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError) return GS::StdLib::Stdf::ReportFile;

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVPROBABILITY_PLOT,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(of=="HTML")
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

void CGexReport::WriteAdvProbabilityPlotChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int iChartSize)
{
        // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_PROBABILITY_PLOT, ptTestReferenceCell->iHtmlAdvancedPage);

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if (pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = (pChartsInfo->chartsList().first())->iGroupX;

    QString strImage =
        BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                             "/images/adv_p_", ptTestReferenceCell, iGroup);
    QString strImagePath = m_pReportOptions->strReportDirectory;
    strImagePath += "/images/";
    strImagePath += strImage;

    // Create Chart (paint into Widget)
    CreateAdvProbabilityPlotChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings(),strImagePath);

    // See what image size was built...make HTML table that wide!
    int	iSizeX=0;
    switch(iChartSize)
    {
    case GEX_CHARTSIZE_SMALL:
        iSizeX = GEX_CHARTSIZE_SMALL_X;
        break;
    case GEX_CHARTSIZE_MEDIUM:
        iSizeX = GEX_CHARTSIZE_MEDIUM_X;
        break;
    case GEX_CHARTSIZE_LARGE:
        iSizeX = GEX_CHARTSIZE_LARGE_X;
        break;
    case GEX_CHARTSIZE_BANNER:
        iSizeX = GEX_CHARTSIZE_BANNER_X;
        break;
    }

    // Write chart into Image (unless we are in Interactive mode).
    if(pChartsInfo == NULL || m_pChartsInfo->bFileOutput)
    {
        // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_probabilityplot",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX);
        fprintf(hReportFile,"<br></font></p>\n");
    }
    else
    {
        // Close Info file created in $home (or Windows) folder
        CloseReportFile();	// Close report file
    }
}

/////////////////////////////////////////////////////////////////////////////
// Create the advanced probability plot image, and save it to disk
/////////////////////////////////////////////////////////////////////////////
void
CGexReport::
CreateAdvProbabilityPlotChartImageEx(CGexChartOverlays* pChartsInfo,
                                     CTest* ptTestReferenceCell,
                                     int iChartSize,
                                     bool /*bStandardHisto*/,
                                     int iChartType,
                                     QString strImage)
{
    GexProbabilityPlotChart probabilityPlotChart(iChartSize, 0, pChartsInfo);

    probabilityPlotChart.setViewportModeFromChartMode(iChartType);
    probabilityPlotChart.computeData(m_pReportOptions, ptTestReferenceCell);
    probabilityPlotChart.buildChart();
    probabilityPlotChart.drawChart(strImage, GetImageCopyrightString());
}

QString CGexReport::WriteAdvProbabilityPlot(CReportOptions* ro)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
                GS::LPPlugin::ProductInfo::advProbabilityPlot)) /*== GS::LPPlugin::LicenseProvider::eLtxcOEM*/
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        WriteText(m);
        return "error : Your licence doesn't allow this function";
    }

    QString of=ro->GetOption("output", "format").toString();

    if	(of=="CSV")
        return "error : csv not available";

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	ro->GetOption("adv_probabilityplot","chart_size").toString(),
                                   ro->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();
    CTest	*ptTestCell = NULL;	//pGroup->cMergedData.ptMergedTestList;
    int iChartNumber=0;
    int iTestsPerPage=0;
    QString pf=ro->GetOption("output", "paper_format").toString();
    QString ps=ro->GetOption("output", "paper_size").toString();

    //while(ptTestCell != NULL)
    while(it!=mAdvancedTestsListToReport.end())
    {
        ptTestCell=*it;
        if (!ptTestCell)
        {
            it++;
            continue;
        }

        // If no samples available...no chart!
        if(ptTestCell->iHtmlAdvancedPage > 0)
        {
            // ******** Check for user 'Abort' signal
            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
               return "ok"; //return GS::StdLib::Stdf::NoError;

            // Draw chart into image file.
            WriteAdvProbabilityPlotChartPage(NULL,pFile,ptTestCell,iChartSize);

            // Update chart count
            iChartNumber++;

            // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
            switch(iChartSize)
            {
                case GEX_CHARTSIZE_MEDIUM:
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Probability-Plot",iChartNumber,2,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if ( (of=="PDF") || (of=="DOC") || of=="ODT" )
                    {
                        // In portrait mode: allow 5 medium histograms per page
                        if (pf=="portrait")
                        {
                            // A4: allow 6 tests per page. Letter: allow 5 tests per page.
                            iTestsPerPage = (ps=="A4") ? 6: 5;
                            if((iChartNumber % iTestsPerPage) == 0)
                                WritePageBreak();
                        }
                        else
                        {
                            // Landscape A4: allow 2 tests per page. Letter: allow 3 tests per page.
                            iTestsPerPage = (ps=="A4") ? 2: 3;

                        if((iChartNumber % iTestsPerPage) == 0)
                            WritePageBreak();	// Landscape mode: only two medium histogram per page.
                        }
                    }
                    else
                    if((iChartNumber % 2) == 0)
                        WritePageBreak();
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name. Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Probability-Plot",1,1,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF") || (of=="DOC") || of=="ODT" )
                    {
                        // In portrait mode: allow 2 large histograms per page
                        if (pf=="portrait")
                        {
                            if((iChartNumber % 2) == 0)
                                WritePageBreak();
                        }
                        else
                            WritePageBreak();	// Landscape mode: only one large histogram per page.
                    }
                    else
                        WritePageBreak();
                    break;

                case GEX_CHARTSIZE_BANNER:
                    // Dynamically build the PowerPoint slide name (as name includes the 5 (or 4) tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Probability-Plot",iChartNumber,4,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF")||  (of=="DOC") || of=="ODT" )
                    {
                        // In portrait mode: allow 8 banner histograms per page
                        if (pf=="portrait")
                        {
                            // A4: allow 9 tests per page. Letter: allow 8 tests per page.
                            iTestsPerPage = (ps=="A4") ? 9: 8;
                            if((iChartNumber % iTestsPerPage) == 0)
                                WritePageBreak();
                        }
                        else
                        if((iChartNumber % 4) == 0)
                            WritePageBreak();	// Landscape mode: only 4 banner histograms per page.
                    }
                    else
                    if((iChartNumber % 4) == 0)
                        WritePageBreak();
                    break;
            }
        }

        // Point to next test cell
        it++;
        //ptTestCell = ptTestCell->ptNextTest;
    };	// Loop until all test cells read.

    return "ok";
}
