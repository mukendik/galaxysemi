/////////////////////////////////////////////////////////////////////////////
// Creates report page 'Multi-chart'
/////////////////////////////////////////////////////////////////////////////

#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_group_of_files.h"
#include "engine.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "csl/csl_engine.h"
#include "product_info.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern bool CTestFailCountLessThan(CTest* s1, CTest* s2);

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvMultiChart(BOOL /*bValidSection*/)
{
    CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.

    // Creates the 'adv_histo' page & header
    long	iTestsMultiChartInPage = 0;

    mAdvancedTestsListToReport.clear();

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
            //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating HTML report file.
        mTotalAdvancedPages = 1;// Number of Histogram HTML pages that will be generated
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();

        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();

        int N=0;
        if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
        {
            bool ok=false;
            N=QString( m_pReportOptions->strAdvancedTestList ).section(' ',1,1).toInt(&ok);
            if (!ok)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString(" failed to understand '%1'").arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData() );
                N=5;
            }
            GSLOG(SYSLOG_SEV_NOTICE, QString(" will consider only top %1 failed tests ").arg(N).toLatin1().constData());
        }

        QString htmlpb=m_pReportOptions->GetOption("output", "html_page_breaks").toString();
        ptTestCell = pGroup->cMergedData.ptMergedTestList;

        while(ptTestCell != NULL)
        {
            // We create one ADV_Histogram HTML page per X tests
            if((iTestsMultiChartInPage >= MAX_HISTOGRAM_PERPAGE)
                && (htmlpb=="true"))
            {
                // Once X tests are in the page, reset counter, increment page count
                iTestsMultiChartInPage = 0;
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
                if((ptTestCell->bTestType == '-') && (strOptionStorageDevice == "hide"))		// !pReportOptions->bIncludeGenericTests)
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
            ptTestCell->iHtmlAdvancedPage = mTotalAdvancedPages;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
                ptTestCell->iHtmlAdvancedPage= 1;

            // Total number of tests indexed in current page
            iTestsMultiChartInPage++;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
            {
                iTestsMultiChartInPage--;
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
int	CGexReport::CloseSection_AdvMultiChart(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
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
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data());	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();

        // Open Advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
            return GS::StdLib::Stdf::ReportFile;

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVMULTICHART,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if	(of=="HTML") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

void CGexReport::WriteAdvMultiChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int iChartSize)
{
        // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_MULTICHART, ptTestReferenceCell->iHtmlAdvancedPage);

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if (pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = (pChartsInfo->chartsList().first())->iGroupX;

    QString strImagePath = m_pReportOptions->strReportDirectory;
    strImagePath += "/images/";

    // Create Histogram Plot Chart (paint into Widget)
    QString strImageHisto =
        BuildImageUniqueName(strImagePath +
                             "adv_mch_", ptTestReferenceCell, iGroup);
    int     nHistoChartType;

    // Convert multichart viewport type into Histogram viewport
    switch(m_pReportOptions->getAdvancedReportSettings())
    {
        case GEX_ADV_MULTICHART_OVERLIMITS  :
        default                             :   nHistoChartType = GEX_ADV_HISTOGRAM_OVERLIMITS;
                                                break;

        case GEX_ADV_MULTICHART_DATALIMITS  :   nHistoChartType = GEX_ADV_HISTOGRAM_DATALIMITS;
                                                break;

        case GEX_ADV_MULTICHART_OVERDATA    :   nHistoChartType = GEX_ADV_HISTOGRAM_OVERDATA;
                                                break;
    }

    CreateAdvHistoChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,nHistoChartType, strImagePath + strImageHisto);

    // Create trend Chart (paint into Widget)
    QString strImageTrend =
        BuildImageUniqueName(strImagePath +
                             "adv_mct_", ptTestReferenceCell, iGroup);
    CreateAdvTrendChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize, strImagePath + strImageTrend, false);

    // Create Box Plot Chart (paint into Widget)
    QString strImageBoxPlot =
        BuildImageUniqueName(strImagePath +
                             "adv_mcb_", ptTestReferenceCell, iGroup);
    CreateAdvBoxPlotChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings(),strImagePath + strImageBoxPlot);

    // Create Probability Plot Chart (paint into Widget)
    QString strImageProbability =
        BuildImageUniqueName(strImagePath +
                             "adv_mcp_", ptTestReferenceCell, iGroup);
    CreateAdvProbabilityPlotChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings(),strImagePath + strImageProbability);

    // Write chart into Image (unless we are in Interactive mode).
    if(pChartsInfo == NULL || m_pChartsInfo->bFileOutput)
    {
        QStringList lstImageName;

        lstImageName << strImageHisto << strImageTrend << strImageBoxPlot << strImageProbability;

        // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_multichart",pFile,ptTestReferenceCell,NULL,NULL,true, lstImageName,GEX_CHARTSIZE_LARGE_X);
        fprintf(hReportFile,"<br></font></p>\n");
    }
    else
    {
        // Close Info file created in $home (or Windows) folder
        CloseReportFile();	// Close report file
    }
}

QString	CGexReport::WriteAdvMultiChart(CReportOptions* ro)
{
    if (!ro)
        return "error";

    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        WriteText(m);
        return "error : Your licence doesn't allow this function";
    }

    QString of=ro->GetOption("output", "format").toString();
    if (of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        return "ok";

    // If chart size=Auto...compute it's real size
    //	iChartSize = GetChartImageSize(pReportOptions->iProbPlotChartSize,pReportOptions->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles* pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";

    QString pf=ro->GetOption("output", "paper_format").toString();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    CTest	*ptTestCell = NULL;	//pGroup->cMergedData.ptMergedTestList;
    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();

    int iChartNumber=0;
    while(it!=mAdvancedTestsListToReport.end()) //while(ptTestCell != NULL)
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
               return "ok";	//"GS::StdLib::Stdf::NoError";

            // Draw chart into image file.
            WriteAdvMultiChartPage(NULL, pFile, ptTestCell, GEX_CHARTSIZE_MEDIUM);

            // Update chart count
            iChartNumber++;

//					// When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
//					switch(iChartSize)
//					{
//						case GEX_CHARTSIZE_MEDIUM:
//							// Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
//							RefreshPowerPointSlideName("Multi-chart",iChartNumber,2,ptTestCell);
//
//							// If PDF or Word, we support Portrait & Landscape formats
//							if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PDF || pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_WORD)
//							{
//								// In portrait mode: allow 5 medium histograms per page
//								if(pReportOptions->bPortraitFormat)
//								{
//									// A4: allow 6 tests per page. Letter: allow 5 tests per page.
//									iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 6: 5;
//
//									if((iChartNumber % iTestsPerPage) == 0)
//										WritePageBreak();
//								}
//								else
//								{
//									// Landscape A4: allow 2 tests per page. Letter: allow 3 tests per page.
//									iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 2: 3;
//
//								if((iChartNumber % iTestsPerPage) == 0)
//									WritePageBreak();	// Landscape mode: only two medium histogram per page.
//								}
//							}
//							else
//							if((iChartNumber % 2) == 0)
//								WritePageBreak();
//							break;

//						case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name. Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Multi-chart",1,1,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF") || (of=="DOC")|| of=="ODT" )
                    {
                        // In portrait mode: allow 2 large histograms per page
                        if (pf=="portrait") //if(pReportOptions->bPortraitFormat)
                        {
                            if((iChartNumber % 2) == 0)
                                WritePageBreak();
                        }
                        else
                            WritePageBreak();	// Landscape mode: only one large histogram per page.
                    }
                    else
                        WritePageBreak();
//							break;
//					}
        }

        // Point to next test cell
        it++; //ptTestCell = ptTestCell->GetNextTest();
    };	// Loop until all test cells read.

    return "ok";
}
