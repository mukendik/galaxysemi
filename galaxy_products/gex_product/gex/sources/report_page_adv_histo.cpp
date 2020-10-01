/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'adv_histogram' page.
/////////////////////////////////////////////////////////////////////////////
#include <qregexp.h>
#include <QHash>

#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "interactive_charts.h"		// Layer classes, etc
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gexhistogramchart.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "engine.h"
#include "csl/csl_engine.h"
#include "product_info.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"

// main.cpp
extern bool CTestFailCountLessThan(CTest* s1, CTest* s2);
extern CGexReport*		gexReport;			// Handle to report class

// cstats.cpp
extern	double				ScalingPower(int iPower);
extern	Chart::SymbolType	convertToChartDirectorSpot(int nSpotIndex);

extern int RetrieveTopNWorstTests(int N, CTest* start, QList<CTest*> &output );

int	CGexReport::PrepareSection_AdvHisto(bool bValidSection)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" for tests '%1' section %2").arg( bValidSection?"valid":"invalid")
          .arg(m_pReportOptions->strAdvancedTestList).toLatin1().constData());

    CTest	*ptTestCell=0;	// Pointer to test cell to receive STDF info.
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();

    mAdvancedTestsListToReport.clear();

    // Creates the 'adv_histo' page & header
    long	iTestsHistogramsInPage=0;
    int		iTotalTestsInReport=0;

    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
    if (m_pReportOptions->isReportOutputHtmlBased())
        //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating HTML report file.
        mTotalAdvancedPages = 1;// Number of Histogram HTML pages that will be generated

        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        if (!pGroup)
        {
            GSLOG(SYSLOG_SEV_WARNING, " error : cant retrieve first pGroup in getGroupsList() !");
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
                      .arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData());
                N=5;
            }
            else
            {
                GSLOG(SYSLOG_SEV_NOTICE,
                      QString(" will consider only top %1 failed tests ")
                      .arg( N).toLatin1().constData());
            }
        }

        ptTestCell = pGroup->cMergedData.ptMergedTestList;

        int i=0;

        QString htmlpb=m_pReportOptions->GetOption("output", "html_page_breaks").toString();

        while(ptTestCell != NULL)
        {
            // We create one ADV_Histogram HTML page per X tests
            if((iTestsHistogramsInPage >= MAX_HISTOGRAM_PERPAGE) && (htmlpb=="true"))
            {
                // Once X tests are in the page, reset counter, increment page count
                iTestsHistogramsInPage=0;
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
                if ((ptTestCell->bTestType == '-')
                        && (strOptionStorageDevice=="hide")
                    )
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
            iTestsHistogramsInPage++;
            iTotalTestsInReport++;

            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
            {
                iTestsHistogramsInPage--;
                //iTotalTestsInReport++;
            }
            NextTestCell:
            i++;
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.

        // Will be used while creating all Histogram pages
        mTotalHtmlPages = mTotalAdvancedPages;

        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("%1 adv pages to create.").arg( mTotalHtmlPages).toLatin1().constData());

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

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("%1 tests marked.").arg( iTotalTestsInReport).toLatin1().constData());

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvHisto(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting trend data!
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (strOutputFormat=="HTML"))
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
        if (OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "OpenFile_Advanced() failed.");
            return GS::StdLib::Stdf::ReportFile;
        }

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVHISTO,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(strOutputFormat=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Set layer style & symbol style (if any)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::SetLayerStyles(int iGroup, CGexSingleChart *pChart, int& nFillColor, int& nLineColor, Chart::SymbolType& layerSymbol, int iReportType /* = -1*/)
{
    int nPatternCode = 0;

    // Set curve color: one different color per plot to allow easy stacking.
    if(pChart)
    {
        if(iGroup == 1)
            nFillColor = pChart->cColor.rgb() & 0xffffff;				// Interactive Histogram: user defined color
        else
            nFillColor = GetChartingColor(iGroup).rgb() & 0xffffff;	// Group#2 and higher can't be customized by users.

        switch(pChart->iLineStyle)
        {
            case 0:	// Solid line
                nPatternCode = 0;
                break;
            case 1:	// Dash line
                nPatternCode = DashLine;
                break;
            case 2:	// Dot line
                nPatternCode = DotLine;
                break;
            case 3:	// DashDot line
                nPatternCode = DotDashLine;
                break;
            case 4:	// DashDotDot line
                nPatternCode = AltDashLine;
                break;
        }

        if (nPatternCode)
            nLineColor  = m_pChartDirector->xyChart()->dashLineColor(nFillColor, nPatternCode);
        else
            nLineColor  = nFillColor;

        // Symbol?
        if(pChart->bSpots == false)
            layerSymbol = Chart::NoSymbol;
        else
            layerSymbol = convertToChartDirectorSpot(pChart->iSpotStyle);
    }
    else
    {
        CGexSingleChart	*pLayerStyle = NULL;

        // Check if index is valid
        if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
            pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);

        // Non-interactive chart: either use default color, or custom color if defined under intercative mode
        if(pLayerStyle == NULL)
        {
            // No color defined under interactive mode for this group...then use default color
            nFillColor	= GetChartingColor(iGroup).rgb() & 0xffffff;
            nLineColor  = nFillColor;
            layerSymbol = Chart::NoSymbol;
        }
        else
        {
            QString ct=m_pReportOptions->GetOption("adv_trend","chart_type").toString();
            // Get charting line style for PNG image to create.
            switch(iReportType)
            {
                case SECTION_ADV_HISTO:
                    break;
                case SECTION_ADV_TREND:
                    // Lines?
                    if ( (ct=="lines")|| (ct=="lines_spots") )
                    /*
                    if((pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINES) ||
                        (pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINESSPOTS))
                    */
                        pLayerStyle->iLineStyle = 0;	// Solid Line
                    else
                        pLayerStyle->iLineStyle = -1;	// No line.

                    // Spots?
                    if ( (ct=="spots")||(ct=="lines_spots") )
                    /*
                    if((pReportOptions->iTrendChartType == GEX_CHARTTYPE_SPOTS) ||
                    (pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINESSPOTS))
                    */
                        pLayerStyle->bSpots = true;
                    else
                        pLayerStyle->bSpots = false;
                    break;

                case SECTION_ADV_SCATTER:
                    // Lines?
                    if((m_pReportOptions->iScatterChartType == GEX_CHARTTYPE_LINES) ||
                        (m_pReportOptions->iScatterChartType == GEX_CHARTTYPE_LINESSPOTS))
                        pLayerStyle->iLineStyle = 0;	// Solid Line
                    else
                        pLayerStyle->iLineStyle = -1;	// No line.

                    // Spots?
                    if((m_pReportOptions->iScatterChartType == GEX_CHARTTYPE_SPOTS) ||
                    (m_pReportOptions->iScatterChartType == GEX_CHARTTYPE_LINESSPOTS))
                        pLayerStyle->bSpots = true;
                    else
                        pLayerStyle->bSpots = false;
                    break;
            }

            // Use interactive color
            nFillColor = pLayerStyle->cColor.rgb() & 0xffffff;

            switch(pLayerStyle->iLineStyle)
            {
                case 0:	// Solid line
                    nPatternCode = 0;
                    break;
                case 1:	// Dash line
                    nPatternCode = DashLine;
                    break;
                case 2:	// Dot line
                    nPatternCode = DotLine;
                    break;
                case 3:	// DashDot line
                    nPatternCode = DotDashLine;
                    break;
                case 4:	// DashDotDot line
                    nPatternCode = AltDashLine;
                    break;
            }

            if (nPatternCode)
                nLineColor  = m_pChartDirector->xyChart()->dashLineColor(nFillColor, nPatternCode);
            else
                nLineColor  = nFillColor;

            // Symbol?
            if(pLayerStyle->bSpots == false)
                layerSymbol = Chart::NoSymbol;
            else
                layerSymbol = convertToChartDirectorSpot(pLayerStyle->iSpotStyle);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// Tells if all layers relate to same test...
/////////////////////////////////////////////////////////////////////////////
bool isSingleTestAllLayers(CGexChartOverlays *pChartsInfo)
{
    if(pChartsInfo == NULL)
        return true;

    CGexSingleChart	*	pChart	= NULL;	// Handle to Parameter Layer info.
    int					nLayerIndex;
    unsigned int		iTest	= 0;
    int					iPinmap	= 0;

    for(nLayerIndex=0;nLayerIndex<pChartsInfo->chartsList().count();nLayerIndex++)
    {
        pChart = pChartsInfo->chartsList().at(nLayerIndex);
        if(nLayerIndex == 0)
        {
            iTest	= pChart->iTestNumberX;
            iPinmap = pChart->iPinMapX;
        }

        if((pChart->iTestNumberX != iTest) || (pChart->iPinMapX != iPinmap))
            return false;	// Mismatch: not all layers showing same test#
    }

    // All layers showing same test#
    return true;
}

void CGexReport::CreateAdvHistoChartImageEx(CGexChartOverlays *pChartsInfo,CTest *ptTestReferenceCell,
                                            int iChartSize,bool bStandardHisto,int iChartType,QString strImage)
{
    GexHistogramChart histogramChart(bStandardHisto, iChartSize, 0, pChartsInfo);
    histogramChart.setIsAdvHistoOnFile(true);
    histogramChart.setViewportModeFromChartMode(iChartType);
    histogramChart.computeData(m_pReportOptions, ptTestReferenceCell);
    histogramChart.buildChart();
    histogramChart.drawChart(strImage, GetImageCopyrightString());
}

void CGexReport::CreateAdvHistoFunctionaChartImageEx(const QMap<uint,int> &oListHistoFuncTest, CGexChartOverlays *pChartsInfo,CTest */*ptTestReferenceCell*/,int iChartSize,bool bStandardHisto,int iChartType,QString strImage,QString strPattern,int iPart)
{
    QList<uint> oListKey = oListHistoFuncTest.keys();
    int iValuesCount = oListKey.count();
    if(!iValuesCount)
        return;

    int iMaxBarPerChart = m_pReportOptions->GetOption("adv_functional","split_image").toInt();

    bool bSplit = (iMaxBarPerChart!=0);
    int iStart =0;
    int iEnd = iValuesCount -1;

    if(bSplit){
        if(iMaxBarPerChart >0 && iValuesCount>iMaxBarPerChart){
            iStart = iPart * iMaxBarPerChart;
            iEnd = (iPart+1) * iMaxBarPerChart -1  ;
            if(iEnd>iValuesCount)
                iEnd = iValuesCount - 1;
        }else
            bSplit = false;
    }

    int iBufferSize = iEnd-iStart+1;
    double *dY = new double[iBufferSize];
    char **pszXLegend = new char *[iBufferSize];
    QString strXTitle;
    if(m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_FUNCTIONAL_REL_VAD)
        strXTitle = "Relative vector address";
    else
        strXTitle = "Cycle count of vector";
    QString strYTitle("Count of Parts");
    QString strChartTitle = strPattern;
    if(bSplit)
        strChartTitle += QString(" Part(%1)").arg(iPart+1);

    for(int iIdx = iStart; iIdx <=iEnd; iIdx++){
        uint uiKey = oListKey[iIdx];
        dY[iIdx-iStart]=oListHistoFuncTest[uiKey];
        QString strLegend = QString::number(uiKey);
        pszXLegend[iIdx-iStart] = new char[strLegend.size()+1];
        strcpy(pszXLegend[iIdx-iStart], strLegend.toLatin1().constData());
    }


    GexHistogramChart histogramChart(bStandardHisto, iChartSize, 0, pChartsInfo);
    histogramChart.setIsAdvHistoOnFile(true);
    histogramChart.setViewportModeFromChartMode(iChartType);
    histogramChart.setReportOptions(m_pReportOptions);
    histogramChart.buildChartFromData(pszXLegend, dY, iBufferSize, strChartTitle, strXTitle, strYTitle);
    histogramChart.drawChart(strImage, GetImageCopyrightString());
    delete []dY;
    for(int iIdx=0; iIdx<iBufferSize;iIdx++){
        delete [] (pszXLegend[iIdx]);
    }
    delete [] pszXLegend;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
QString	CGexReport::BuildImageUniqueName(QString strPrefix,CTest *ptTestCell/* = NULL */,int iGroup, QString strPattren, int iPart/*=-1*/)
{
    QRegExp regExp("[^A-Za-z0-9_.]");
    QString strImage;
    QFileInfo fileInfo;
    unsigned int hash;

    if(ptTestCell != NULL)
        strImage += ptTestCell->szTestLabel;

    // Add group ID if any
    if(iGroup >= 0)
        strImage += "_" + QString::number(iGroup);

    // Add Parameter name
    if(ptTestCell != NULL)
        strImage += "_" + ptTestCell->strTestName;
    if(!strPattren.isEmpty())
        strImage += "_" + strPattren;
    if(iPart!=-1)
        strImage += "_part" + QString::number(iPart);

    hash = qHash(strImage);
    do {
        strImage = QString("%1_%2.png").arg(strPrefix).arg(hash);
        fileInfo.setFile(strImage);
        --hash;
    } while (fileInfo.exists());

    strImage = strImage.section('/', -1);
    strImage = strImage.replace(regExp, "_");
    return strImage;
}

/////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Histo report page.
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvHistoChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell, int iChartSize)
{
    // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_HISTO,ptTestReferenceCell->iHtmlAdvancedPage);

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if(pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = pChartsInfo->chartsList().first()->iGroupX;

    QString strImage =
        BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                             "/images/adv_h_", ptTestReferenceCell, iGroup);
    QString strImagePath	= m_pReportOptions->strReportDirectory;
    strImagePath			+= "/images/";
    strImagePath			+= strImage;

    //if (GetOption("output","format") : could slow down !
    if(m_strOutputFormat=="ODT")
        iChartSize=GEX_CHARTSIZE_LARGE; // For the moment it is the only supported size

    // Create Chart (paint into Widget)
    CreateAdvHistoChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings(),strImagePath);

    // See what image size was built...make HTML table that wide!
    int	iSizeX=0;
    switch(iChartSize)
    {
        case GEX_CHARTSIZE_SMALL: iSizeX = GEX_CHARTSIZE_SMALL_X;
        break;
        case GEX_CHARTSIZE_MEDIUM: iSizeX = GEX_CHARTSIZE_MEDIUM_X;
        break;
        case GEX_CHARTSIZE_LARGE: iSizeX = GEX_CHARTSIZE_LARGE_X;
        break;
        case GEX_CHARTSIZE_BANNER: iSizeX = GEX_CHARTSIZE_BANNER_X;
        break;
    }

    // Write chart into Image (unless we are in Interactive mode).
    if(pChartsInfo == NULL || m_pChartsInfo->bFileOutput)
    {
        // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_histo",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX);
        fprintf(hReportFile,"<br></font></p>\n");
    }
    else
    {
        // Close Info file created in $home (or Windows) folder
        CloseReportFile();	// Close report file
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PlotScriptingCustomMarkersEx(CTest *ptTestCell,CGexFileInGroup *pFile,QColor &cLayerRGB,int iLayer,double fCustomScaleFactor,bool bVerticalMarker,double *plfChartBottom/*=NULL*/,double *plfChartTop/*=NULL*/)
{
    // Ignore if no test currently pointed
    if(ptTestCell == NULL)
        return;

    // Plot all markers defined for this test
    int 	nMarkerColor;
    double	lfMarkerPos;
    Axis *	pAxis = NULL;

    QList<TestMarker*>::iterator itBegin	= ptTestCell->ptCustomMarkers.begin();
    QList<TestMarker*>::iterator itEnd		= ptTestCell->ptCustomMarkers.end();

    while(itBegin != itEnd)
    {
        // Check if marker is visible: If marker is sticky to a layer, ignore color, and use same color as layer colr!
        if(((*itBegin)->iLayer != iLayer-1) && ((*itBegin)->iLayer >= 0))
            goto next_marker;	// This marker must only be charted for a specific layer!

        // Select marker color: either defined thru script, or left to invalid so to use same color as layer charting color
        // If marker is sticky to a layer, ignore color, and use same color as layer colr!
        if(((*itBegin)->iLayer == iLayer-1) && ((*itBegin)->bForceColor == false))
            nMarkerColor = m_pChartDirector->xyChart()->dashLineColor(cLayerRGB.rgb() & 0xffffff, DotDashLine);				// Use color of layer for a leyer-specific marker
        else
            nMarkerColor = m_pChartDirector->xyChart()->dashLineColor((*itBegin)->cColor.rgb() & 0xffffff, DotDashLine);		// If marker visible on all layers....

        // Scale position from normalized to viewing scale
        lfMarkerPos = fCustomScaleFactor * (*itBegin)->lfPos;
        pFile->FormatTestResultNoUnits(&lfMarkerPos,ptTestCell->res_scal);

        // check if Vertical or Horizontal
        if (!bVerticalMarker)
        {
            pAxis = m_pChartDirector->xyChart()->yAxis();

            // Update Y viewport start-ending points, based on marker position
            if(plfChartBottom)
                *plfChartBottom = gex_min(*plfChartBottom,lfMarkerPos);
            if(plfChartTop)
                *plfChartTop = gex_max(*plfChartTop,lfMarkerPos);
        }
        else
            pAxis = m_pChartDirector->xyChart()->xAxis();

        // Insert  marker
        m_pChartDirector->addMarker(pAxis, lfMarkerPos, nMarkerColor, (*itBegin)->strLabel, (*itBegin)->iLine, Chart::TopLeft);

        // Move to next marker
next_marker:
        itBegin++;
    };
}

QString	CGexReport::WriteAdvHisto(CReportOptions* ro)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString(" iAdvancedReport = GEX_ADV_HISTOGRAM AdvTestList = %1")
          .arg( mAdvancedTestsListToReport.size()).toLatin1().constData());

    QString of=ro->GetOption("output", "format").toString();
    if	(of=="CSV") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        return "warning : Adv Hsto not available in csv format !";

    QString pf=ro->GetOption("output", "paper_format").toString();
    QString ps=ro->GetOption("output", "paper_size").toString();

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	m_pReportOptions->GetOption("adv_histogram","chart_size").toString(), //pReportOptions->iHistoChartSize,
                                   m_pReportOptions->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);

    int	iChartNumber=0;	// Keeps track of the number of chart inserted (so to insert appropriate page breaks)

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";
    CGexFileInGroup *pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    CTest	*ptTestCell=NULL;	// Pointer to test cell to receive STDF info.

    //ptTestCell = pGroup->cMergedData.ptMergedTestList;
    int		ntestcell=0; // count number of test found/handled
    int		iTestsPerPage=0;

    QList<CTest*>		sortedWorstFailingTestsList;	// list containing the worst tests

    if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
    {
        bool ok=false;
        int N=QString( m_pReportOptions->strAdvancedTestList ).section(' ',1,1).toInt(&ok);
        if (!ok)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString(" failed ot understand '%1'")
                  .arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData());
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString(" will consider only top %1 failed tests ").arg( N).toLatin1().constData());

            RetrieveTopNWorstTests(N, pGroup->cMergedData.ptMergedTestList, sortedWorstFailingTestsList);
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString(" %1 tests successfully sorted.")
                  .arg( sortedWorstFailingTestsList.size()).toLatin1().constData());

            if (sortedWorstFailingTestsList.size()>0)
                ptTestCell = sortedWorstFailingTestsList.first();
        }
    }

    int i=0;

    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();
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
        if( (ptTestCell->iHtmlAdvancedPage > 0))
        {
            ntestcell++;

            // ******** Check for user 'Abort' signal
            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
               return "GS::StdLib::Stdf::NoError"; //return GS::StdLib::Stdf::NoError;

            // Draw chart into image file.
            WriteAdvHistoChartPage(NULL,pFile,ptTestCell,iChartSize);

            // Update chart count
            iChartNumber++;

            // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
            switch(iChartSize)
            {
                case GEX_CHARTSIZE_MEDIUM:
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Histogram",iChartNumber,2,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if	(	(of=="PDF") || (of=="DOC") || of=="ODT"	)
                    {
                        // In portrait mode: allow 5 medium histograms per page
                        if (pf=="portrait") // if(pReportOptions->bPortraitFormat)
                        {
                            // A4: allow 6 tests per page. Letter: allow 5 tests per page.
                            iTestsPerPage = (ps=="A4") ? 6: 5; //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 6: 5;

                            if((iChartNumber % iTestsPerPage) == 0)
                                WritePageBreak();
                        }
                        else
                        {
                            // Landscape A4: allow 2 tests per page. Letter: allow 3 tests per page.
                            iTestsPerPage = (ps=="A4") ? 2: 3;
                             //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 2: 3;

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
                    RefreshPowerPointSlideName("Histogram",1,1,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if ( (of=="PDF") || (of=="DOC")	|| of=="ODT" )
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
                    break;

                case GEX_CHARTSIZE_BANNER:
                    // Dynamically build the PowerPoint slide name (as name includes the 5 (or 4) tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Histogram",iChartNumber,4,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF")	|| (of=="DOC") || of=="ODT" )
                    {
                        // In portrait mode: allow 8 banner histograms per page
                        if (pf=="portrait") //if(pReportOptions->bPortraitFormat)
                        {
                            // A4: allow 9 tests per page. Letter: allow 8 tests per page.
                            //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 9: 8;
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

        i++;
        it++;

    };	// Loop until all test cells read.

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 TestCells writed to report.").arg(ntestcell).toLatin1().constData());

    return "ok";
}

int CGexReport::PrepareSection_AdvHistoFunctional(BOOL bValidSection){
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" for tests '%1' section %2")
          .arg( bValidSection?"valid":"invalid")
          .arg(m_pReportOptions->strAdvancedTestList).toLatin1().data());

    CTest	*ptTestCell=0;	// Pointer to test cell to receive STDF info.
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();

    mAdvancedTestsListToReport.clear();

    // Creates the 'adv_histo' page & header
    long	iTestsHistogramsInPage=0;
    int		iTotalTestsInReport=0;
    bool bFirst = true;

    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
    if (m_pReportOptions->isReportOutputHtmlBased())
        //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating HTML report file.
        mTotalAdvancedPages = 1;// Number of Histogram HTML pages that will be generated

        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        if (!pGroup)
        {
            GSLOG(SYSLOG_SEV_WARNING, " error : cant retrieve first pGroup in getGroupsList() !");
            return GS::StdLib::Stdf::ErrorMemory;
        }

        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        int i=0;
        QString htmlpb=m_pReportOptions->GetOption("output", "html_page_breaks").toString();

        while(ptTestCell != NULL)
        {
            bFirst = true;
            // If  a parametric / multiparametric  test, and not   functional  ignore!
            if(ptTestCell->bTestType != 'F')
                goto NextTestCell;
            if(ptTestCell->mVectors.keys().isEmpty())
                goto NextTestCell;

            foreach(const QString &strPattern, ptTestCell->mVectors.keys()){
                if(!ptTestCell->mVectors[strPattern].m_lstVectorInfo.count())
                    continue;
                // We create one ADV_Histogram HTML page per X tests
                if((iTestsHistogramsInPage >= MAX_HISTOGRAM_PERPAGE) && (htmlpb=="true"))
                {
                    // Once X tests are in the page, reset counter, increment page count
                    iTestsHistogramsInPage=0;
                    mTotalAdvancedPages++;
                }

                if (m_pReportOptions->strAdvancedTestList=="all"){
                    if(!mAdvancedTestsListToReport.contains(ptTestCell))
                        mAdvancedTestsListToReport.append(ptTestCell);
                }
                else
                {
                    // assuming we are in given TestsList	mode
                    if (m_pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex)){
                        if(!mAdvancedTestsListToReport.contains(ptTestCell))
                            mAdvancedTestsListToReport.append(ptTestCell);
                    }
                    else
                        goto NextTestCell;
                }

                // Saves page# where histogram will be
                if(bFirst){
                    ptTestCell->iHtmlAdvancedPage= mTotalAdvancedPages;
                    bFirst = false;
                }
                // Total number of tests indexed in current page
                iTestsHistogramsInPage++;
                iTotalTestsInReport++;
            }
            NextTestCell:
            i++;
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.

        // Will be used while creating all Histogram pages
        mTotalHtmlPages = mTotalAdvancedPages;

        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("%1 adv pages to create.").arg( mTotalHtmlPages).toLatin1().constData());

        // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
        m_pReportOptions->lAdvancedHtmlPages = mTotalAdvancedPages;
    }

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("%1 tests marked.").arg( iTotalTestsInReport).toLatin1().constData());

    return GS::StdLib::Stdf::NoError;

}

int	CGexReport::CloseSection_AdvFunctionalHisto(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting trend data!
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (strOutputFormat=="HTML"))
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data()  );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();

        // Open Advanced.htm file
        if (OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "OpenFile_Advanced() failed.");
            return GS::StdLib::Stdf::ReportFile;
        }

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVHISTOFUNCTIONAL,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(strOutputFormat=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

QString	CGexReport::WriteAdvHistoFunctional(CReportOptions* ro)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString(" iAdvancedReport = GEX_ADV_HISTOGRAM AdvTestList = %1")
          .arg( mAdvancedTestsListToReport.size()).toLatin1().constData());

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
      QString m=ReportOptions.GetOption("messages", "upgrade").toString();
      fprintf(hReportFile, "*ERROR* %s", m.toLatin1().data() );
      return "error : Your licence doesn't allow this function";
    }

    QString of=ro->GetOption("output", "format").toString();
    if	(of=="CSV") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        return "warning : Adv Hsto not available in csv format !";

    QString pf=ro->GetOption("output", "paper_format").toString();
    QString ps=ro->GetOption("output", "paper_size").toString();

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	m_pReportOptions->GetOption("adv_functional","chart_size").toString(), //pReportOptions->iHistoChartSize,
                                   m_pReportOptions->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);
    int iMaxBarPerChart = m_pReportOptions->GetOption("adv_functional","split_image").toInt();

    int	iChartNumber=0;	// Keeps track of the number of chart inserted (so to insert appropriate page breaks)

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";
    CGexFileInGroup *pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    CTest	*ptTestCell=NULL;	// Pointer to test cell to receive STDF info.

    //ptTestCell = pGroup->cMergedData.ptMergedTestList;
    int		ntestcell=0; // count number of test found/handled
    int		iTestsPerPage=0;

    int i=0;

    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();
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
        if( (ptTestCell->iHtmlAdvancedPage > 0))
        {
            ntestcell++;

            // ******** Check for user 'Abort' signal
            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
               return "GS::StdLib::Stdf::NoError"; //return GS::StdLib::Stdf::NoError;

            // Draw chart into image file.
            WriteAdvHistoFunctionalChartPage(NULL,pFile,ptTestCell,iChartSize);

            // Update chart count
            int iCountFuncChart = 0;
            foreach(const QString &strChartPattern, ptTestCell->mVectors.keys()){
                int iValuesCount = ptTestCell->mVectors[strChartPattern].m_lstVectorInfo.count();
                if(iValuesCount){
                    if(iMaxBarPerChart >0 && iValuesCount>iMaxBarPerChart){
                        iCountFuncChart += iValuesCount/iMaxBarPerChart;
                        if(iValuesCount%iMaxBarPerChart)
                            iCountFuncChart +=1;
                    }else
                        ++iCountFuncChart;
                }
            }
            iChartNumber+=iCountFuncChart;

            // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
            switch(iChartSize)
            {
                case GEX_CHARTSIZE_MEDIUM:
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Histogram",iChartNumber,2,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if	(	(of=="PDF") || (of=="DOC")	|| of=="ODT" )
                    {
                        // In portrait mode: allow 5 medium histograms per page
                        if (pf=="portrait") // if(pReportOptions->bPortraitFormat)
                        {
                            // A4: allow 6 tests per page. Letter: allow 5 tests per page.
                            iTestsPerPage = (ps=="A4") ? 6: 5; //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 6: 5;

                            if((iChartNumber % iTestsPerPage) == 0)
                                WritePageBreak();
                        }
                        else
                        {
                            // Landscape A4: allow 2 tests per page. Letter: allow 3 tests per page.
                            iTestsPerPage = (ps=="A4") ? 2: 3;
                             //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 2: 3;

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
                    RefreshPowerPointSlideName("Histogram",1,1,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if ( (of=="PDF") || (of=="DOC") || of=="ODT" )
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
                    break;

                case GEX_CHARTSIZE_BANNER:
                    // Dynamically build the PowerPoint slide name (as name includes the 5 (or 4) tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Histogram",iChartNumber,4,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF")	|| (of=="DOC") || of=="ODT" )
                    {
                        // In portrait mode: allow 8 banner histograms per page
                        if (pf=="portrait") //if(pReportOptions->bPortraitFormat)
                        {
                            // A4: allow 9 tests per page. Letter: allow 8 tests per page.
                            //iTestsPerPage = (pReportOptions->iPaperSize == GEX_OPTION_PAPER_SIZE_A4) ? 9: 8;
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

        i++;
        it++;

    };	// Loop until all test cells read.

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 TestCells writed to report.").arg(ntestcell).toLatin1().constData());

    return "ok";
}

void	CGexReport::WriteAdvHistoFunctionalChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int iChartSize)
{
    // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_HISTOFUNCTIONAL,ptTestReferenceCell->iHtmlAdvancedPage);

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if(pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = pChartsInfo->chartsList().first()->iGroupX;
    int iPartsNumber = 1;
    int iMaxBarPerChart = m_pReportOptions->GetOption("adv_functional","split_image").toInt();
    foreach(const QString &strPattern, ptTestReferenceCell->mVectors.keys()){
        if(!ptTestReferenceCell->mVectors[strPattern].m_lstVectorInfo.count())
            continue;

        QMap<QString, QMap<uint,int> >   oListHistoFuncTest = ptTestReferenceCell->buildFunctionalHistoData(m_pReportOptions->getAdvancedReportSettings(), strPattern);
        int iValuesCount = oListHistoFuncTest[strPattern].keys().count();

        if(iMaxBarPerChart >0 && iValuesCount>iMaxBarPerChart){
            iPartsNumber = iValuesCount/iMaxBarPerChart;
            if(iValuesCount%iMaxBarPerChart)
                iPartsNumber +=1;
        }

        for(int iPartIdx=0; iPartIdx<iPartsNumber; iPartIdx++){
            QString strImage =
                BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                     "/images/adv_h_", ptTestReferenceCell,
                                     iGroup,strPattern,
                                     (iPartIdx >= 1 ? iPartIdx : -1));
            QString strImagePath	= m_pReportOptions->strReportDirectory;
            strImagePath			+= "/images/";
            strImagePath			+= strImage;

            // Create Chart (paint into Widget)
            CreateAdvHistoFunctionaChartImageEx(oListHistoFuncTest[strPattern], pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings()
                                                ,strImagePath,strPattern,iPartIdx);

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
                WriteHistoHtmlTestInfo(pChartsInfo,"adv_histo",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX,true);
                fprintf(hReportFile,"<br></font></p>\n");
            }
            else
            {
                // Close Info file created in $home (or Windows) folder
                CloseReportFile();	// Close report file
            }
        }
    }
}


