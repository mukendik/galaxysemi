/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Trend' page.
/////////////////////////////////////////////////////////////////////////////

#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "interactive_charts.h"		// Layer classes, etc
#include "report_classes_sorting.h"
#include "cstats.h"
#include "cpart_info.h"
#include <gqtl_log.h>
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gextrendchart.h"
#include "engine.h"
#include "gex_undo_command.h"
#include "browser_dialog.h"
#include "csl/csl_engine.h"
#include "gqtl_global.h"

// cstats.cpp
extern double			ScalingPower(int iPower);

// report_page_adv_histo.cpp
extern bool				isSingleTestAllLayers(CGexChartOverlays *pChartsInfo);
extern QString			formatHtmlImageFilename(const QString& strImageFileName);
extern bool				CTestFailCountLessThan(CTest* s1, CTest* s2);
extern GexMainwindow	*pGexMainWindow;

int	CGexReport::PrepareSection_AdvTrend(BOOL bValidSection)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" %1 section").arg( bValidSection?"Valid":"Unvalid").toLatin1().constData());

    CTest *ptTestCell=NULL;	// Pointer to test cell to receive STDF info.
    mAdvancedTestsListToReport.clear();

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Trend' page & header
    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report: Trend data ----\n\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased())
            //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // Generating HTML report file.
        long	iTestsTrendInPage=0;
        int		iTotalTestsInReport=0;
        mTotalAdvancedPages = 1; // Number of Histogram HTML pages that will be generated
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();

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

        while(ptTestCell != NULL)
        {
            // We create one ADV_Histogram HTML page per X tests
            if((iTestsTrendInPage >= MAX_HISTOGRAM_PERPAGE) && (htmlpb=="true"))
            {
                // Once X tests are in the page, reset counter, increment page count
                iTestsTrendInPage=0;
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
                if(m_pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex))
                    mAdvancedTestsListToReport.append(ptTestCell);
                else
                    goto NextTestCell;
            }

            // Saves page# where histogram will be
            ptTestCell->iHtmlAdvancedPage = mTotalAdvancedPages;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
                ptTestCell->iHtmlAdvancedPage = 1;

            // Total number of tests indexed in current page
            iTestsTrendInPage++;
            iTotalTestsInReport++;
            if (m_pReportOptions->strAdvancedTestList.startsWith("top") )
            {
                iTestsTrendInPage--;
                //iTotalTestsInReport++;
            }
            NextTestCell:
            // Point to next test cell
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

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvTrend(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Trend page created (if any)...so we can now build the index.
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (strOutputFormat=="HTML"))
        {
            // Writes HTML footer Application name/web site string
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
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
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVTREND, true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(strOutputFormat=="HTML")
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes a Binning trend chart page (Soft or Hard Binning)
/////////////////////////////////////////////////////////////////////////////
void
CGexReport::
WriteAdvTrendBinningChartPageEx(CGexChartOverlays* /*pChartsInfo*/,
                                int iChartSize)
{
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    const char *		ptTitle		= NULL;
    const char *		szBinType	= NULL;
    char	szString[300];
    int		iIndex;
    int		iBin;
    long	lTotalMatchingBins;
    long	lSubLotTotalMatchingBins;
    long	lTotalBins;
    long	lSubTotalBins;
    double	lfBinPercentage;
    double	lfSubLotBinPercentage;
    int		i,iBinChartSize;
    int		iCurvePenSize = 1;				// curve pen size 1 or 2 pixels.
    double	lfPos;
    int		iSizeX=0,iSizeY=0;
    double	*x=NULL;					// buffer to holds X data: Binning events
    double	*y=NULL;					// buffer to holds Y data: Binning Level (%)
    double	*x_rolling=NULL;			// buffer to holds X data: Binning events for Rolling yield
    double	*y_rolling=NULL;			// buffer to holds Y data: Binning Level (%) for rolling yield
    long	lRollingYieldParts = m_pReportOptions->GetOption("adv_trend","rolling_yield").toInt();
    long	lTotalMatchingRollingBins=0;// Used to compute the total of good bins over 'lRollingYieldParts' consecutive bins.
    long	lRollingDataPoint=0;		// Index to rolling yield data points
    long	lRollingDataOffset=0;		// Index Offset to rolling yield of each file to process
    double	lfExtra;
    double	lfChartBottom=C_INFINITE,lfChartTop=-C_INFINITE;
    int		iGroup=1;					// Counter used to know on which group we are (#1 or other)
    bool	bSubLotsPlot=false;			// true if plots based on SUMMARY info (Sub-Lots) instead of Parts
    QString lMessage;


    QString strAdvTrendMarkerOptions = (m_pReportOptions->GetOption(
                                            QString("adv_trend"),
                                            QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.
            split(QString("|"));

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if (m_pReportOptions->isReportOutputHtmlBased())
            //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Create single page : advanced1.htm
        CheckForNewHtmlPage(NULL,SECTION_ADV_TREND,1,"Trend chart of Binning");

        // Image size to create for the trend chart
        switch(iChartSize)
        {
        case GEX_CHARTSIZE_SMALL:
            iSizeX = GEX_CHARTSIZE_SMALL_X;
            iSizeY = GEX_CHARTSIZE_SMALL_Y;
            break;
        case GEX_CHARTSIZE_MEDIUM:
            iSizeX = GEX_CHARTSIZE_MEDIUM_X;
            iSizeY = GEX_CHARTSIZE_MEDIUM_Y;
            break;
        case GEX_CHARTSIZE_LARGE:
            iSizeX = GEX_CHARTSIZE_LARGE_X;
            iSizeY = GEX_CHARTSIZE_LARGE_Y;
            break;
        }

        // Writes HTML code to display the Trend chart...image file created later!
        QString strImage = BuildImageUniqueName("advbinning");	// image to create: 'advbinning.png'

        fprintf(hReportFile,"<td width=\"74%%\" bgcolor=%s><a name=\"Tbinning\"></a></td><br>\n",szDataColor);
        fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\"><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());
    }

    // Get background color
    unsigned int iBackgroundColor = (m_pReportOptions->cBkgColor.rgb() & 0xFFFFFF);

    // Create chart window
    m_pChartDirector->allocateXYChart(iSizeX, iSizeY, iBackgroundColor);
    int nGridColor = m_pChartDirector->xyChart()->dashLineColor(0xc0c0c0, DashLine);
    m_pChartDirector->xyChart()->setPlotArea(60, 25, iSizeX-80, iSizeY-65)->setGridColor(nGridColor, nGridColor);
    m_pChartDirector->xyChart()->setClipping();

    //Set the font for the y axis labels to Arial
    m_pChartDirector->xyChart()->yAxis()->setLabelStyle("arial.ttf");

    switch(m_pReportOptions->getAdvancedReportSettings())
    {
        case GEX_ADV_TREND_SOFTBIN_PARTS:
            ptTitle			= "Software Bin (all parts): ";
            szBinType		= "Software";
            bSubLotsPlot	= false;
            break;
        case GEX_ADV_TREND_SOFTBIN_SBLOTS:
            ptTitle			= "Software Bin (Sub-Lots): ";
            szBinType		= "Software";
            bSubLotsPlot	= true;
            break;
        case GEX_ADV_TREND_HARDBIN_PARTS:
            ptTitle			= "Hardware Bin (all parts): ";
            szBinType		= "Hardware";
            bSubLotsPlot	= false;
            break;
        case GEX_ADV_TREND_HARDBIN_SBLOTS:
            ptTitle			= "Hardware Bin (Sub-Lots): ";
            szBinType		= "Hardware";
            bSubLotsPlot	= true;
            break;
        case GEX_ADV_TREND_SOFTBIN_ROLLING:
            ptTitle			= "Soft Bin + rolling yield: ";
            szBinType		= "Software";
            bSubLotsPlot	= false;
            break;
        case GEX_ADV_TREND_HARDBIN_ROLLING:
            ptTitle			= "Hard Bin + rolling yield: ";
            szBinType		= "Hardware";
            bSubLotsPlot	= false;
            break;
    }

    // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
    if (m_pReportOptions->isReportOutputHtmlBased())
         //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        if((iChartSize != GEX_CHARTSIZE_SMALL) && (qslAdvTrendMarkerOptionList.contains(QString("test_name"))) && (iGroup==1))
        {
            // Creates sub-string listing Bin# list to Trend chart.
            lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString(ptTitle);
            m_pChartDirector->xyChart()->addTitle(lMessage.toLatin1().constData());
        }
    }
    else
    {
        // CSV dump...
        // Creates sub-string listing Bin# list to Trend chart.
        lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString(ptTitle);
        fprintf(hReportFile,"\n%s\n",lMessage.toLatin1().constData());
    }

    // Never reset during analysis if processing Summary data...will hold
    // the largest number of summary events of all groups. USed to define
    // viewport X width.
    lTotalBins=0;

    // Stack all charts of all groups for each same test
    for (int nCount = 0; nCount < getGroupsList().count(); nCount++)
    {
        pGroup = getGroupsList().at(nCount);

        // Reset variables for each group.
        lTotalMatchingBins	= 0;
        lSubTotalBins		= 0;
        lfBinPercentage		= 0.0;

        // Plotting Sub-Lots result
        if(bSubLotsPlot == true)
            lTotalBins = 0;

        // Table with Numerical data
        if(m_pReportOptions->iGroups == 1)	// Single group: do not show group name.
        {
            if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                fprintf(hReportFile,"<br><h1 align=\"left\"><font color=\"#006699\">Trend Details:</font></h1><br>\n");
            else
                fprintf(hReportFile,"Trend Details\n");
        }
        else
        {
            if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                fprintf(hReportFile,"<br><h1 align=\"left\"><font color=\"#006699\">Trend Details:</font><font color=\"%s\"> %s</h1><br>\n",GetChartingHtmlColor(1+nCount),pGroup->strGroupName.toLatin1().constData());
            else
                fprintf(hReportFile,"Trend Details:%s\n",pGroup->strGroupName.toLatin1().constData());
        }

        if ( m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br>\n");
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"left\"><b>Start date</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>LotID</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>SubLot<br>WaferID</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>SubLot Yield<br>Bins selected</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>SubLot Yield<br>Other Bins</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"40%%\" bgcolor=%s align=\"center\"><b>%s Binning Chart</b></td>\n",szFieldColor,szBinType);
            fprintf(hReportFile,"</tr>\n");
        }

        // Plotting Sub-Lots result
        if(bSubLotsPlot == true)
        {
            // Allocate Buffers to hold the binning curve for all files in group.
            lTotalBins = pGroup->pFilesList.count();
            x = new double[lTotalBins+1];
            if(x == NULL)
                return;
            y = new double[lTotalBins+1];
            if(y == NULL)
            {
                delete x;
                return;
            }
        }

        // For each group, process files in sorted order
        QDateTime dStartLot;

        for(int nItem = 0; nItem < pGroup->pFilesList.count(); ++nItem)
        {
            pFile = pGroup->pFilesList.at(nItem);

            // Reset variables for each file.
            lSubLotTotalMatchingBins = 0;
            lfSubLotBinPercentage=0.0;

            // Show verticale marker at position X
            if(pGroup->pFilesList.count() < 6)
                sprintf(szString,"Lot: %s",pFile->getMirDatas().szLot);
            else
            if(pGroup->pFilesList.count() < 12)
                sprintf(szString,"%s",pFile->getMirDatas().szLot);
            else
                *szString = 0;	// No Marker name if over 12 markers

            // Plotting Sub-Lots result
            if(bSubLotsPlot == true)
                lfPos = lSubTotalBins;	// Summary Event (File#) is the Market X Offset
            else
            {
                lfPos = lTotalBins;		// Part# is the Marker X Offset

                // Check if we have at least one data result!
                if(pFile->PartProcessed.hasResultsCount() == 0)
                    continue;
            }

            // Only show markers if 2 groups or less.
            if( m_pReportOptions->isReportOutputHtmlBased() ) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            {
                if(getGroupsList().count() <= 2)
                {
                    if(nCount == 0)
                        m_pChartDirector->addMarker(m_pChartDirector->xyChart()->xAxis(), lfPos, QColor(Qt::darkGreen).rgb() & 0xffffff, szString, 1, Chart::TopRight);
                    else
                        m_pChartDirector->addMarker(m_pChartDirector->xyChart()->xAxis(), lfPos, QColor(Qt::blue).rgb() & 0xffffff, szString, 1, Chart::BottomRight);
                }
            }

            // Plotting Sub-Lots result
            if(bSubLotsPlot == true)
            {
                // We build plots for Summary bin info
                lSubLotTotalMatchingBins = pFile->lAdvBinningTrendTotalMatch;
                if(pFile->lAdvBinningTrendTotal > 0)
                    lfBinPercentage = lfSubLotBinPercentage = 100.0*((double)pFile->lAdvBinningTrendTotalMatch)/(double)pFile->lAdvBinningTrendTotal;
                else
                    lfBinPercentage=0;	// Yield not available...show 0%!
                x[lSubTotalBins] = nItem;
                y[lSubTotalBins] = lfBinPercentage;
                lSubTotalBins++;
                lTotalBins = gex_max(lTotalBins,lSubTotalBins);		// Keeps track of largest number of bin plots
                // Update Min/Max Y values.
                lfChartBottom = gex_min(lfChartBottom,lfBinPercentage);
                lfChartTop    = gex_max(lfChartTop,lfBinPercentage);
            }
            else
            {
                // Parts in trend chart
                lSubTotalBins=pFile->PartProcessed.partNumber();

                // Allocate Buffers to hold the binning results.
                x = new double[lSubTotalBins+1];
                if(x == NULL)
                    return;
                y = new double[lSubTotalBins+1];
                if(y == NULL)
                {
                    delete x;
                    return;
                }

                if((m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_TREND_SOFTBIN_ROLLING) ||
                    (m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_TREND_HARDBIN_ROLLING))
                {
                    // Allocate Buffers to hold the binning results.
                    x_rolling = new double[(lSubTotalBins/lRollingYieldParts)+1];
                    if(x_rolling == NULL)
                        return;
                    y_rolling = new double[(lSubTotalBins/lRollingYieldParts)+1];
                    if(y_rolling == NULL)
                    {
                        delete x_rolling;
                        return;
                    }

                    // Clear variables.
                    lTotalMatchingRollingBins = 0;
                    lRollingDataPoint = 1;	// First datapoint (offset 0) starts at initial yield and is not computed.
                }

                // Compute list of binning matching criteria
                for(iIndex=1;iIndex<=lSubTotalBins;iIndex++)
                {
                    // X value = event#
                    x[iIndex] = lTotalBins;
                    lTotalBins++;
                    if((m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_TREND_SOFTBIN_PARTS) ||
                        (m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_TREND_SOFTBIN_ROLLING))
                        iBin = pFile->PartProcessed.resultsAt(iIndex).iSoftBin;
                    else
                        iBin = pFile->PartProcessed.resultsAt(iIndex).iHardBin;

                    if(m_pReportOptions->pGexAdvancedRangeList->IsTestInList(iBin,GEX_PTEST))
                    {
                        lTotalMatchingBins++;
                        lSubLotTotalMatchingBins++;
                        // Yield value over ALL files selected.
                        lfBinPercentage = (100.0*(double)lTotalMatchingBins)/(double)lTotalBins;
                        // Yield value over THIS file.
                        lfSubLotBinPercentage = (100.0*(double)lSubLotTotalMatchingBins)/(double)iIndex;

                        // If rolling yield charting enabled, compute rolling yield (total good bins over 'lRollingYieldParts' parts)!
                        lTotalMatchingRollingBins++;
                    }
                    // Y value = Binning Current Percentage
                    y[iIndex] = lfBinPercentage;

                    // If rolling yield enabled, compute it...
                    if((x_rolling != NULL) && ((iIndex % lRollingYieldParts) == 0))
                    {
                        x_rolling[lRollingDataPoint] = iIndex + lRollingDataOffset;
                        lfBinPercentage = (100.0*(double)lTotalMatchingRollingBins)/(double)lRollingYieldParts;
                        y_rolling[lRollingDataPoint++] = lfBinPercentage;

                        // Reset counter
                        lTotalMatchingRollingBins = 0;
                    }

                    // Update Min/Max Y values.
                    lfChartBottom = gex_min(lfChartBottom,lfBinPercentage);
                    lfChartTop    = gex_max(lfChartTop,lfBinPercentage);
                }

                // As cell0 is not used, load it with same data as cell1!
                x[0] = x[1];
                y[0] = y[1];

                // If rolling yield enabled, do same thing: copy cell1 into cell0
                if(x_rolling != NULL)
                {
                    x_rolling[0]= lRollingDataOffset;
                    y_rolling[0]= y_rolling[1];
                }

                // Keep track of plotting offset (in case multiple files trend (merged).
                lRollingDataOffset += lSubTotalBins;
            }

            // Insert new curves
            if (m_pReportOptions->isReportOutputHtmlBased())     //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            {
                // Set curve styles: if curve is a straight line, use 2pixels size so Mean line doesn't hide it!
                if(lTotalBins<2)
                    iCurvePenSize = 2;	// 2pixel wide curve
                else
                    iCurvePenSize = 1;	// 1pixel wide curve

                int nFillColor, nLineColor;
                Chart::SymbolType layerSymbol;

                // Set relevant layer style & symbol
                SetLayerStyles(iGroup, NULL, nFillColor, nLineColor, layerSymbol, SECTION_ADV_TREND);

                // Plotting all parts result
                if(bSubLotsPlot == false)
                {
                    //SetLayerStyles(iGroup, NULL, nFillColor, nLineColor, layerSymbol, )
                    LineLayer * pLineLayer = m_pChartDirector->xyChart()->addLineLayer(DoubleArray(y, lSubTotalBins), nLineColor);

                    // Plot the yield curve
                    pLineLayer->setLineWidth(iCurvePenSize);
                    pLineLayer->setXData(DoubleArray(x, lSubTotalBins));

                    if (layerSymbol != Chart::NoSymbol)
                        pLineLayer->getDataSet(0)->setDataSymbol(layerSymbol, 6, nLineColor, nLineColor);

                    // If Rolling yield enabled, also insert Rolling yield curve!
                    if(x_rolling != NULL)
                    {
                        LineLayer * pRollingLine = m_pChartDirector->xyChart()->addLineLayer(DoubleArray(y_rolling, lRollingDataPoint), QColor(Qt::magenta).rgb() & 0xffffff);
                        pRollingLine->setXData(DoubleArray(x_rolling, lRollingDataPoint));

                        // Overwrite symbol to include triangle
                        pRollingLine->setLineWidth(2);
                        pRollingLine->getDataSet(0)->setDataSymbol(Chart::TriangleSymbol, 7, QColor(Qt::magenta).rgb() & 0xffffff, QColor(Qt::black).rgb() & 0xffffff);
                    }


                    delete x; x=0;	// delete array as now a copy is in the 'plot' object.
                    delete y; y=0;	// delete array as now a copy is in the 'plot' object.
                    if(x_rolling != NULL)
                        delete x_rolling;	// delete array as now a copy is in the 'plot' object.
                    if(y_rolling != NULL)
                        delete y_rolling;	// delete array as now a copy is in the 'plot' object.
                }
            }
            else
            {
                // CSV file: dump data to CSV file.
                fprintf(hReportFile,"Bin/EventID,");
                for(i=0;i<lSubTotalBins;i++)
                    fprintf(hReportFile,"%f,",x[i]);
                fprintf(hReportFile,"\nBin Percentage,");
                for(i=0;i<lSubTotalBins;i++)
                    fprintf(hReportFile,"%.2lf %%,",y[i]);
                fprintf(hReportFile,"\n");

                delete x;	// delete array as now a copy is in the 'plot' object.
                delete y;	// delete array as now a copy is in the 'plot' object.
            }

            // Create date string : DD-Month-YYYY HH:MM:SS
            dStartLot.setTime_t(pFile->getMirDatas().lStartT);
            sprintf(szString,"%s",dStartLot.toString("dd-MMM-yyyy hh:mm:ss").toLatin1().constData());
            // Write info into HTML table
            if(m_pReportOptions->isReportOutputHtmlBased())  //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s align=\"left\"><b>%s</b></td>\n",szDataColor,szString);
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,pFile->getMirDatas().szLot);
            }
            else
            {
                // Label line.
                fprintf(hReportFile,"\nStart date,LotID,WaferID,SubLot Yield (Bins selected),SubLot Yield (Other Bins)\n");
                // <Date>,<LotID>
                fprintf(hReportFile,"%s,%s,",szString,pFile->getMirDatas().szLot);
            }

            if(*pFile->getMirDatas().szSubLot)
                strcpy(szString,pFile->getMirDatas().szSubLot);
            else
                strcpy(szString,pFile->getWaferMapData().szWaferID);

            if(m_pReportOptions->isReportOutputHtmlBased())  //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szString);
            else
                fprintf(hReportFile,"%s,",szString);

            if(pFile->lAdvBinningTrendTotal > 0)
            {
                // We have a valid yield info
                if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                {
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%.2lf %%</td>\n",szDataColor,lfSubLotBinPercentage);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">%.2lf %%</td>\n",szDataColor,100.0-lfSubLotBinPercentage);
                }
                else
                {
                    fprintf(hReportFile,"%.2lf %%,",lfSubLotBinPercentage);
                    fprintf(hReportFile,"%.2lf %%,",100.0-lfSubLotBinPercentage);
                }
            }
            else
            {
                // No valid yield info...
                if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                {
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">n/a</td>\n",szDataColor);
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\">n/a</td>\n",szDataColor);
                }
                else
                    fprintf(hReportFile,",,");
            }

            // Green + Red bars
            iBinChartSize = (int)(3.0*lfSubLotBinPercentage);
            // This to make sure none of the 2 bars is 0pixel wide (QT bug)!
            if(iBinChartSize == 0)
                iBinChartSize=1;
            if(iBinChartSize == 300)
                iBinChartSize=299;

            if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            {
                fprintf(hReportFile,"<td width=\"40%%\" bgcolor=%s align=\"left\"><img border=\"1\" src=\"../images/bar1.png\" width=\"%d\" height=\"10\"><img border=\"1\" src=\"../images/bar3.png\" width=\"%d\" height=\"10\"></td>\n",
                    szDataColor,iBinChartSize,300-iBinChartSize);
                fprintf(hReportFile,"</tr>\n");
            }
            else
                fprintf(hReportFile,"\n");
        };

        // Close table with list of Trend data info.
        if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            fprintf(hReportFile,"</table>\n");

        // Plotting Sub-Lots result
        if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            if(bSubLotsPlot == true)
            {
                int nFillColor, nLineColor;
                Chart::SymbolType layerSymbol;

                // Set relevant layer style & symbol
                SetLayerStyles(iGroup, NULL, nFillColor, nLineColor, layerSymbol, SECTION_ADV_TREND);

                // Add last segment
                x[lSubTotalBins] = lSubTotalBins;
                y[lSubTotalBins] = y[lSubTotalBins-1];

                //SetLayerStyles(iGroup, NULL, nFillColor, nLineColor, layerSymbol, )
                LineLayer * pLineLayer = m_pChartDirector->xyChart()->addLineLayer(DoubleArray(y, lSubTotalBins+1), nLineColor);

                // Plot the yield curve
                pLineLayer->setLineWidth(iCurvePenSize);
                pLineLayer->setXData(DoubleArray(x, lSubTotalBins+1));

                if (layerSymbol != Chart::NoSymbol)
                    pLineLayer->getDataSet(0)->setDataSymbol(layerSymbol, 6, nLineColor, nLineColor);

                delete x;	// delete array as now a copy is in the 'plot' object.
                delete y;	// delete array as now a copy is in the 'plot' object.
            }
        }

        iGroup++;	// Keep track of group index processed.
    };	// Read histogram of a given test in all groups so to stack all charts.

    if(of=="CSV")
        return;

    // X axis scale.
    m_pChartDirector->xyChart()->xAxis()->setLinearScale(0, lTotalBins);

    // Y axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    lfExtra = (lfChartTop-lfChartBottom)*0.01;
    m_pChartDirector->xyChart()->yAxis()->setLinearScale(lfChartBottom-lfExtra,lfChartTop+lfExtra);

    // We have a chart legend unless the chart size in only 200 pixel wide (SMALL chart mode)!
    if(iChartSize != GEX_CHARTSIZE_SMALL)
    {
        // Plotting Sub-Lots result
        if(bSubLotsPlot == true)
            sprintf(szString,"Binning summary found: %ld",lTotalBins);
        else
            sprintf(szString,"Binning/Parts events found: %ld",lTotalBins);

        m_pChartDirector->xyChart()->xAxis()->setTitle(szString);

        // compute a string <value> units...just to extract the units!
        sprintf(szString,"Binning Percentage (%%)");	// No units!
        m_pChartDirector->xyChart()->yAxis()->setTitle(szString);
    }

    // Save Binning Trend chart image into .PNG file! (256 colors palette)
    sprintf(szString,"%s/images/advbinning.png",m_pReportOptions->strReportDirectory.toLatin1().constData());
    m_pChartDirector->drawChart(szString, GetImageCopyrightString());
}

/////////////////////////////////////////////////////////////////////////////
// Writes a Difference trend chart page (TestX-TestY)
/////////////////////////////////////////////////////////////////////////////
void
CGexReport::
WriteAdvTrendDifferenceChartPageEx(CGexChartOverlays* /*pChartsInfo*/,
                                   int iChartSize)
{
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    CTest *				ptTest1Cell;
    CTest *				ptTest2Cell;
    char				szTestName[2*GEX_MAX_STRING];
    char				szXscaleLabel[50];
    char				szString[300];
    int					i;
    int					iCurvePenSize;	// curve pen size 1 or 2 pixels.
    int					iSizeX=0,iSizeY=0;
    double				lfValue1,lfValue2;
    double *			x;				// buffer to holds X data.
    double *			y;				// buffer to holds X data.
    double				lfExtra,lfChartBottom,lfChartTop,lfHighL,lfLowL;
    double				lStartID,lEndID,lStep;

    // Pointer to label strings...may vary depending of the chart size!
    const char *		ptChar;
    const char *		szLabelLL	= "Low Limit";
    const char *		szLabelHL	= "High Limit";
    int					lTest1Number,lPinmap1Index;
    int					lTest2Number,lPinmap2Index;
    int					iGroup		= 1;	// Counter used to know on which group we are (#1 or other)
    S_gexTestRange*		pTestRangeList;

    // OPTIONS
    QString strAdvTrendMarkerOptions = (m_pReportOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized")?true:false;

    // Check if valid tests# specified
    if(m_pReportOptions->pGexAdvancedRangeList == NULL)
        return;

    pTestRangeList = m_pReportOptions->pGexAdvancedRangeList->pTestRangeList;
    if(pTestRangeList == NULL)
        return;

    // Extract Test#1
    lTest1Number = pTestRangeList->lFromTest;
    lPinmap1Index = pTestRangeList->lFromPinmap;

    // Extract Test#2
    pTestRangeList = pTestRangeList->pNextTestRange;
    if(pTestRangeList == NULL)
        return;
    lTest2Number = pTestRangeList->lFromTest;
    lPinmap2Index = pTestRangeList->lFromPinmap;

    // Find the 2 Tests to compare
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Test 1 Always from group1
    if(pFile->FindTestCell(lTest1Number,lPinmap1Index,&ptTest1Cell,false,false)!=1)
        return;


    // Test 2 from Group1 if only 1 group, or group2 otherwise.
    if(m_pReportOptions->iGroups > 1)
    {
        pGroup = (getGroupsList().size() < 2) ? NULL : getGroupsList().at(1);
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    }

    if(pFile->FindTestCell(lTest2Number,lPinmap2Index,&ptTest2Cell,false,false)!=1)
        return;

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    //if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Check if need to create new HTML Trend. page
        CheckForNewHtmlPage(NULL,SECTION_ADV_TREND,ptTest1Cell->iHtmlAdvancedPage);

        // Image size to create for the trend chart
        switch(iChartSize)
        {
        case GEX_CHARTSIZE_SMALL:
            // Shortest label possible
            szLabelHL = "HL";
            szLabelLL = "LL";
            iSizeX = GEX_CHARTSIZE_SMALL_X;
            iSizeY = GEX_CHARTSIZE_SMALL_Y;
            break;
        case GEX_CHARTSIZE_MEDIUM:
            // Shortest label possible
            szLabelHL = "HL";
            szLabelLL = "LL";
            iSizeX = GEX_CHARTSIZE_MEDIUM_X;
            iSizeY = GEX_CHARTSIZE_MEDIUM_Y;
            break;
        case GEX_CHARTSIZE_LARGE:
            iSizeX = GEX_CHARTSIZE_LARGE_X;
            iSizeY = GEX_CHARTSIZE_LARGE_Y;
            break;
        }
    }

    // Get background color
    unsigned int iBackgroundColor = (m_pReportOptions->cBkgColor.rgb() & 0xFFFFFF);

    // Create chart window
    m_pChartDirector->allocateXYChart(iSizeX, iSizeY, iBackgroundColor);
    int nGridColor = m_pChartDirector->xyChart()->dashLineColor(0xc0c0c0, DashLine);
    m_pChartDirector->xyChart()->setPlotArea(60, 25, iSizeX-80, iSizeY-65)->setGridColor(nGridColor, nGridColor);
    m_pChartDirector->xyChart()->setClipping();

    //Set the font for the y axis labels to Arial
    m_pChartDirector->xyChart()->yAxis()->setLabelStyle("arial.ttf");

    // count samples to compare
    long lTotalValidSamples=0;
    long lTotalSamples = gex_min(ptTest1Cell->m_testResult.count(),ptTest2Cell->m_testResult.count());

    // Create X scale.
    x = new double[lTotalSamples];
    if(x == NULL)
        return;

    // Compute Y (Test1-Test2)
    y = new double[lTotalSamples];
    if(y == NULL)
    {
        delete x;
        return;
    }

    lfChartTop = -C_INFINITE;
    lfChartBottom = C_INFINITE;

    for(i=0,lTotalValidSamples=0;i<lTotalSamples;i++)
    {
        if( (ptTest1Cell->m_testResult.isValidResultAt(i)) && (ptTest2Cell->m_testResult.isValidResultAt(i)) )
        {

            // Compute Test1-Test2
            lfValue1 = ptTest1Cell->m_testResult.resultAt(i);
            lfValue2 = ptTest2Cell->m_testResult.resultAt(i);

            y[lTotalValidSamples] = lfValue1-lfValue2;

            // Update Min/Max window tracking
            lfChartTop = gex_max(lfChartTop,y[lTotalValidSamples]);
            lfChartBottom = gex_min(lfChartBottom,y[lTotalValidSamples]);
            lTotalValidSamples++;
        }
    }

    //if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
        if((iChartSize != GEX_CHARTSIZE_SMALL) && (qslAdvTrendMarkerOptionList.contains(QString("test_name"))) && (iGroup==1))
            sprintf(szString,"Difference between tests:%s",
                    m_pReportOptions->pGexAdvancedRangeList->BuildTestListString(" ").toLatin1().constData());
        else
            *szString = 0;	// No title!
        m_pChartDirector->xyChart()->addTitle(szString);
    }
    else
    {
        // CSV dump...
        fprintf(hReportFile,"\nDifference between tests:%s, Group name: %s\n",
            m_pReportOptions->pGexAdvancedRangeList->BuildTestListString(" ").toLatin1().constData(),
            pGroup->strGroupName.toLatin1().constData());
    }

    // Init. X values to sample#.
    lStartID = lEndID = ptTest1Cell->lFirstPartResult;
    if(ptTest1Cell->ldSamplesValidExecs > 0)
    {
        // First compute the difference of PartID...
        lStep = ((double)1+lTotalValidSamples);
        if(lStep < ptTest1Cell->ldSamplesExecs)
        {
            // We probably have a mix of files, so we do not have enough PartIDs
            // ..so X axis will simply show Part count starting from ID=1....to N
            lStartID = 1;
            lStep = 1;
            strcpy(szXscaleLabel,"Data count");
        }
        else
        {
            lStep /= (double)ptTest1Cell->ldSamplesExecs;
            strcpy(szXscaleLabel,"Data ID");
        }
    }
    else
    {
        lStep = 0;
        strcpy(szXscaleLabel,"No data");
    }
    for(i=0;i<lTotalValidSamples;i++)
    {
        x[i] = lEndID;
        lEndID += lStep;
    }

    // Insert new curves
    if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Set curve styles: if curve is a straight line, use 2pixels size so Mean line doesn't hide it!
        if(ptTest1Cell->lfSamplesMax == ptTest1Cell->lfSamplesMin)
            iCurvePenSize = 2;	// 2pixel wide curve
        else
            iCurvePenSize = 1;	// 1pixel wide curve

        int nLineColor, nFillColor;
        Chart::SymbolType layerSymbol;

        SetLayerStyles(iGroup, NULL, nFillColor, nLineColor, layerSymbol, SECTION_ADV_TREND);

        LineLayer * pLineLayer = m_pChartDirector->xyChart()->addLineLayer(DoubleArray(y, lTotalValidSamples), nLineColor);

        pLineLayer->setLineWidth(iCurvePenSize);
        pLineLayer->setXData(DoubleArray(x, lTotalValidSamples));

        if (layerSymbol != Chart::NoSymbol)
            pLineLayer->getDataSet(0)->setDataSymbol(layerSymbol, 6, nLineColor, nLineColor);

        delete x;	// delete array as now a copy is in the 'plot' object.
        delete y;	// delete array as now a copy is in the 'plot' object.
    }
    else
    {
        // CSV file: dump data to CSV file.
        fprintf(hReportFile,"Step/EventID,");
        for(i=0;i<lTotalValidSamples;i++)
            fprintf(hReportFile,"%f,",x[i]);
        fprintf(hReportFile,"\nValue,");
        for(i=0;i<lTotalValidSamples;i++)
            fprintf(hReportFile,"%f,",y[i]);
        fprintf(hReportFile,"\n");

        delete x;	// delete array as now a copy is in the 'plot' object.
        delete y;	// delete array as now a copy is in the 'plot' object.
        return;
    }

    // Draw '0' line: Difference=0
    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), 0.0f, QColor(Qt::red).rgb() & 0xffffff, "Diff=0", 1, Chart::TopLeft);

    // Draw limit markers (done only once: when charting Plot for group#1)
    if(((ptTest1Cell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)  && (iGroup==1))
    {
        lfLowL = ptTest1Cell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
        // If we have to keep values in normalized format, do not rescale!
        if (!isScalingNormalized)
        {
            // convert LowLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&lfLowL,ptTest1Cell->llm_scal);
            lfLowL *=  ScalingPower(ptTest1Cell->llm_scal);	// normalized
            lfLowL /=  ScalingPower(ptTest1Cell->res_scal);	// normalized
        }
        // LowLimit Marker
        if(qslAdvTrendMarkerOptionList.contains(QString("limits")))
            m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfLowL, QColor(Qt::red).rgb() & 0xffffff, szLabelLL, 1, Chart::TopLeft);
    }
    if(((ptTest1Cell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (iGroup==1))
    {
        lfHighL = ptTest1Cell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
        // If we have to keep values in normalized format, do not rescale!
        if (!isScalingNormalized)
        {
            // convert HighLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&lfHighL,ptTest1Cell->hlm_scal);
            lfHighL *=  ScalingPower(ptTest1Cell->hlm_scal);	// normalized
            lfHighL /=  ScalingPower(ptTest1Cell->res_scal);	// normalized
        }
        // High limit Marker
        if(qslAdvTrendMarkerOptionList.contains(QString("limits")))
            m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfHighL, QColor(Qt::red).rgb() & 0xffffff, szLabelHL, 1, Chart::BottomLeft);
    }

    // Y axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    lfExtra = (lfChartTop-lfChartBottom)*0.01;

    m_pChartDirector->xyChart()->yAxis()->setLinearScale(lfChartBottom-lfExtra,lfChartTop+lfExtra);

    // X axis scale.
    m_pChartDirector->xyChart()->xAxis()->setLinearScale(lStartID-1,lEndID-0.5);

    // We have a chart legend unless the chart size in only 200 pixel wide (SMALL chart mode)!...done only once at 1st group processing
    if((iChartSize != GEX_CHARTSIZE_SMALL) && (iGroup==1))
    {
        if(iChartSize != GEX_CHARTSIZE_SMALL)
            sprintf(szString,"%s (%ld samples found)",szXscaleLabel,lTotalValidSamples);
        else
            *szString = 0;	// No legend in X

        m_pChartDirector->xyChart()->xAxis()->setTitle(szString);

        // compute a string <value> units...just to extract the units!
        if(iChartSize != GEX_CHARTSIZE_SMALL)
        {
            ptChar = pFile->FormatTestResult(ptTest1Cell,ptTest1Cell->lfMin,ptTest1Cell->res_scal);
            if(sscanf(ptChar,"%*f %s",szTestName)==1)
                sprintf(szString,"Tests difference (%s)",szTestName);
            else
                sprintf(szString,"Tests difference");	// No units!
        }
        else
          *szString = 0;	// No legend in Y

        m_pChartDirector->xyChart()->yAxis()->setTitle(szString);
    }

    // Plot vertical Lots# markers if any.
    plotLotMarkersEx(ptTest1Cell, m_pReportOptions);

    // Save file into .PNG file! (256 colors palette)
    sprintf(szString,"%s/images/advdiff.png",m_pReportOptions->strReportDirectory.toLatin1().constData());

    m_pChartDirector->drawChart(szString, GetImageCopyrightString());

    // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
    QString strImage = BuildImageUniqueName("advdiff");	// image to create: 'advdiff.png'

    fprintf(hReportFile,"<td width=\"74%%\" bgcolor=%s><a name=\"Tdifference\"></a></td><br>\n",szDataColor);
    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\"><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());
    fprintf(hReportFile,"<hr>\n");
    fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"></font></p>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Dumps all the Data points into the QTable
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::ListAdvScatterData(
        CGexChartOverlays *pChartsInfo, QTableWidget * pTableWidget,CGexFileInGroup *pFile,
        CTest *ptTestReferenceCellX,CTest *ptTestReferenceCellY)
{
  CGexGroupOfFiles *pGroupX=0,*pGroupY=0;
    CGexFileInGroup *pFileX=NULL;
    CGexFileInGroup *pFileY=NULL;
    char	szTestName[1024];
    long	lTestNumber, lPinmapIndex;
    double	fCustomScaleFactor;
    CTest	*ptTestCell,*ptPartIdCell;;
    long	iRows=0;       //long	iRows=50;
    long	lDataOffset;
    double	lfValue	= GEX_C_DOUBLE_NAN;
    CGexSingleChart	*pChart=NULL;	// Handle to Parameter Layer info.
    unsigned iLayerIndex=0;
    unsigned iTotalLayers = pChartsInfo->chartsList().count();
    QString	strUnits;
    QString strString;
    QTableWidgetItem * pTableWidgetItem = NULL;
    QColor colorGreen	= QColor(192,255,192);
    QColor colorItem;
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");

    pTableWidget->clear();
    // Clear the table

    // If no layers...exit!
    if(iTotalLayers <= 0)
        return;

    // X column per group (as many as layers)....and double if list PartID too!
    long	lTotalColsPerLayer;
    int		iPartXColOffset;
    int		iPartYColOffset;
    int		iDataXColOffset;
    int		iDataYColOffset;
    bool	bShowPartID = (m_pReportOptions->GetOption("dataprocessing", "part_id").toString() == "show") ? true : false;

    if (bShowPartID)
    {
        lTotalColsPerLayer = 4;	// 4 columns per result: <PartID> , <ResultX>, <PartID> , <ResultY>
        iPartXColOffset=0;
        iDataXColOffset=1;
        iPartYColOffset=2;
        iDataYColOffset=3;

    }
    else
    {
        lTotalColsPerLayer = 2;	// 2 column per result (no partID) : <ResultX>,<ResultY>
        iPartXColOffset=0;
        iDataXColOffset=0;	// Not used
        iPartYColOffset=1;
        iDataYColOffset=1;	// Not used

    }

    // Total columns in table
    pTableWidget->setColumnCount(iTotalLayers*lTotalColsPerLayer);

    // Stack all charts of all groups for each same test
    for(iLayerIndex = 0; iLayerIndex < iTotalLayers; iLayerIndex++)
    {
        pChart			= pChartsInfo->chartsList().at(iLayerIndex);
        lTestNumber		= pChart->iTestNumberX;
        lPinmapIndex	= pChart->iPinMapX;

        // Get pointer to relevant group
        pGroupX	= (pChart->iGroupX < 0 || pChart->iGroupX >= getGroupsList().size()) ? NULL : getGroupsList().at(pChart->iGroupX);
        pGroupY	= (pChart->iGroupY < 0 || pChart->iGroupY >= getGroupsList().size()) ? NULL : getGroupsList().at(pChart->iGroupY);

        if(pGroupX  == NULL || pGroupY  == NULL)
            goto NextGroup;

        pFileX = (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();
        pFileY = (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

        if(pFileX->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, pChart->strTestNameX.toLatin1().data())!=1)
            goto NextGroup;

        // Update number of rows
        iRows = gex_max(iRows,ptTestCell->ldSamplesExecs);
        pTableWidget->setRowCount(iRows);

        // If to display PartID, do it now!
        if (bShowPartID)
        {
            // Build header title for this given group.
            strString = pGroupX->strGroupName + " - Part ID";

            if (pTableWidget->horizontalHeaderItem(iPartXColOffset+(lTotalColsPerLayer*iLayerIndex)) == NULL)
                pTableWidget->setHorizontalHeaderItem(iPartXColOffset+(lTotalColsPerLayer*iLayerIndex), new QTableWidgetItem(strString));
            else
                pTableWidget->horizontalHeaderItem(iPartXColOffset+(lTotalColsPerLayer*iLayerIndex))->setText(strString);

            CGexFileInGroup *	pFileTmp	= NULL;
            int					nBegin		= 0;
            int					nEnd		= 0;
            int					nPartID		= 0;

            // Loop on each file in order to find the right partID string
            for (int nFile = 0; nFile < pGroupX->pFilesList.count(); nFile++)
            {
                pFileTmp = pGroupX->pFilesList.at(nFile);

                // Load all values in the table.
                if (pFileTmp->FindTestCell(GEX_TESTNBR_OFFSET_EXT_PARTID,GEX_PTEST,&ptPartIdCell,false,false) == 1)
                {
                    // Find out begin and end index for this sublot
                    ptPartIdCell->findSublotOffset(nBegin, nEnd, nFile);

                    for(lDataOffset = nBegin; lDataOffset < nEnd; lDataOffset++)
                    {
                        nPartID	= lDataOffset-nBegin;

                        if (nPartID < iRows)
                        {
                            if(ptPartIdCell->m_testResult.isValidResultAt(lDataOffset))
                            {
                                strString = pFileTmp->pPartInfoList.at(nPartID)->getPartID();
                                colorItem = Qt::white;
                            }
                            else
                            {
                                strString = "n/a";
                                colorItem = colorGreen;
                            }

                            pTableWidgetItem = pTableWidget->item(lDataOffset,iPartXColOffset+(lTotalColsPerLayer*iLayerIndex));
                            if (pTableWidgetItem == NULL)
                            {
                                pTableWidgetItem = new QTableWidgetItem();
                                pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                                pTableWidget->setItem(lDataOffset,iPartXColOffset+(lTotalColsPerLayer*iLayerIndex), pTableWidgetItem);
                            }

                            pTableWidgetItem->setText(strString);
                            pTableWidgetItem->setBackground(colorItem);
                            pTableWidgetItem->setTextAlignment(Qt::AlignRight);
                        }
                    }
                }
            }
        }

        // Build header title for this 1st group (X axis)
        strString = "X: " + pGroupX->strGroupName;
        strString += " - T";
        strString += ptTestCell->szTestLabel;
        strString += QString(" : ");
        BuildTestNameString(pFile,ptTestCell,szTestName);
        strString += szTestName;

        if (pTableWidget->horizontalHeaderItem(iDataXColOffset+(lTotalColsPerLayer*iLayerIndex)) == NULL)
            pTableWidget->setHorizontalHeaderItem(iDataXColOffset+(lTotalColsPerLayer*iLayerIndex), new QTableWidgetItem(strString));
        else
            pTableWidget->horizontalHeaderItem(iDataXColOffset+(lTotalColsPerLayer*iLayerIndex))->setText(strString);

        // Load all values in the table (X axis)
        if(ptTestCell->m_testResult.count() > 0 && (ptTestCell->ldSamplesValidExecs > 0))
        {
            // Get scaled units (as data samples in the array are not normalized, but in res_scal format)
            strUnits = ptTestCell->GetScaledUnits(&fCustomScaleFactor, scaling);

            for(lDataOffset=0;lDataOffset < ptTestCell->m_testResult.count(); lDataOffset++)
            {
                colorItem = Qt::white;
                strString.clear();

                if(ptTestCell->m_testResult.isValidResultAt(lDataOffset))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(lDataOffset))
                    {
                        CTestResult::PassFailStatus ePassFailStatus = ptTestCell->m_testResult.passFailStatus(lDataOffset);

                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lDataOffset)->count(); ++nCount)
                        {
                            if (nCount > 0)
                                strString += " | ";

                            lfValue		= ptTestCell->m_testResult.at(lDataOffset)->multiResultAt(nCount);
                            strString	+= getNumberFormat()->formatNumericValue(lfValue*fCustomScaleFactor, true,QString::number(lfValue*fCustomScaleFactor)) + strUnits;

                            // Valid sample value.
                            if (!isScalingNormalized)
                                lfValue *= ScalingPower(ptTestCell->res_scal);

                            // Check if this cell is holding a test failure
                            if(ptTestCell->isFailingValue(lfValue, ePassFailStatus))
                                colorItem = Qt::red;
                        }
                    }
                    else
                    {
                        lfValue		= ptTestCell->m_testResult.resultAt(lDataOffset);
                        strString	+= getNumberFormat()->formatNumericValue(lfValue*fCustomScaleFactor, true, QString::number(lfValue*fCustomScaleFactor)) + strUnits;

                        // Valid sample value.
                        if (!isScalingNormalized)
                            lfValue *= ScalingPower(ptTestCell->res_scal);

                        // Check if this cell is holding a test failure
                        if(ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(lDataOffset)))
                            colorItem = Qt::red;
                    }
                }
                else
                {
                    strString = "-";
                    colorItem = colorGreen;
                }

                pTableWidgetItem = pTableWidget->item(lDataOffset,iDataXColOffset+(lTotalColsPerLayer*iLayerIndex));
                if (pTableWidgetItem == NULL)
                {
                    pTableWidgetItem = new QTableWidgetItem();
                    pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                    pTableWidget->setItem(lDataOffset,iDataXColOffset+(lTotalColsPerLayer*iLayerIndex), pTableWidgetItem);
                }

                pTableWidgetItem->setText(strString);
                pTableWidgetItem->setBackground(colorItem);
                pTableWidgetItem->setTextAlignment(Qt::AlignLeft);
            }
        }

        lTestNumber		= pChart->iTestNumberY;
        lPinmapIndex	= pChart->iPinMapY;

        if(pFileY->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,
                                pChart->strTestNameY.toLatin1().data())!=1)
            goto NextGroup;

        // Update number of rows
        iRows = gex_max(iRows,ptTestCell->ldSamplesExecs);
        pTableWidget->setRowCount(iRows);

        // If to display PartID, do it now!
        if (bShowPartID)
        {
            // Build header title for this given group.
            strString = pGroupY->strGroupName + " - Part ID";

            if (pTableWidget->horizontalHeaderItem(iPartYColOffset+(lTotalColsPerLayer*iLayerIndex)) == NULL)
                pTableWidget->setHorizontalHeaderItem(iPartYColOffset+(lTotalColsPerLayer*iLayerIndex), new QTableWidgetItem(strString));
            else
                pTableWidget->horizontalHeaderItem(iPartYColOffset+(lTotalColsPerLayer*iLayerIndex))->setText(strString);

            CGexFileInGroup *	pFileTmp	= NULL;
            int					nBegin		= 0;
            int					nEnd		= 0;
            int					nPartID		= 0;

            // Loop on each file in order to find the right partID string
            for (int nFile = 0; nFile < pGroupY->pFilesList.count(); nFile++)
            {
                pFileTmp = pGroupY->pFilesList.at(nFile);

                // Load all values in the table.
                if (pFileTmp->FindTestCell(GEX_TESTNBR_OFFSET_EXT_PARTID,GEX_PTEST,&ptPartIdCell,false,false) == 1)
                {
                    // Find out begin and end index for this sublot
                    ptPartIdCell->findSublotOffset(nBegin, nEnd, nFile);

                    for(lDataOffset = nBegin; lDataOffset < nEnd; lDataOffset++)
                    {
                        nPartID	= lDataOffset-nBegin;

                        if (nPartID < iRows)
                        {
                            if(ptPartIdCell->m_testResult.isValidResultAt(lDataOffset))
                            {
                                strString = pFileTmp->pPartInfoList.at(nPartID)->getPartID();
                                colorItem = Qt::white;
                            }
                            else
                            {
                                strString = "n/a";
                                colorItem = colorGreen;
                            }

                            pTableWidgetItem = pTableWidget->item(lDataOffset,iPartYColOffset+(lTotalColsPerLayer*iLayerIndex));
                            if (pTableWidgetItem == NULL)
                            {
                                pTableWidgetItem = new QTableWidgetItem();
                                pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                                pTableWidget->setItem(lDataOffset,iPartYColOffset+(lTotalColsPerLayer*iLayerIndex), pTableWidgetItem);
                            }

                            pTableWidgetItem->setText(strString);
                            pTableWidgetItem->setBackground(colorItem);
                            pTableWidgetItem->setTextAlignment(Qt::AlignRight);
                        }
                    }
                }
            }
        }

        // Build header title for the Y axis
        strString = "Y: " + pGroupY->strGroupName;
        strString += " - T";
        strString += ptTestCell->szTestLabel;
        strString += QString(" : ");
        strString += ptTestCell->strTestName;

        if (pTableWidget->horizontalHeaderItem(iDataYColOffset+(lTotalColsPerLayer*iLayerIndex)) == NULL)
            pTableWidget->setHorizontalHeaderItem(iDataYColOffset+(lTotalColsPerLayer*iLayerIndex), new QTableWidgetItem(strString));
        else
            pTableWidget->horizontalHeaderItem(iDataYColOffset+(lTotalColsPerLayer*iLayerIndex))->setText(strString);

        // Load all values in the table (X axis)
        if(ptTestCell->m_testResult.count() > 0 && (ptTestCell->ldSamplesValidExecs > 0))
        {
            // Get scaled units (as data samples in the array are not normalized, but in res_scal format)
            strUnits = ptTestCell->GetScaledUnits(&fCustomScaleFactor, scaling);

            for(lDataOffset=0;lDataOffset < ptTestCell->m_testResult.count(); lDataOffset++)
            {
                colorItem = Qt::white;
                strString.clear();

                if(ptTestCell->m_testResult.isValidResultAt(lDataOffset))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(lDataOffset))
                    {
                        CTestResult::PassFailStatus ePassFailStatus = ptTestCell->m_testResult.passFailStatus(lDataOffset);

                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lDataOffset)->count(); ++nCount)
                        {
                            if (nCount > 0)
                                strString += " | ";

                            lfValue		= ptTestCell->m_testResult.at(lDataOffset)->multiResultAt(nCount);
                            strString	= getNumberFormat()->formatNumericValue(lfValue*fCustomScaleFactor, true, QString::number(lfValue*fCustomScaleFactor)) + strUnits;

                            // Valid sample value.
                            if(!isScalingNormalized)
                                lfValue *= ScalingPower(ptTestCell->res_scal);

                            // Check if this cell is holding a test failure
                            if(ptTestCell->isFailingValue(lfValue, ePassFailStatus))
                                colorItem = Qt::red;
                        }
                    }
                    else
                    {
                        lfValue		= ptTestCell->m_testResult.resultAt(lDataOffset);
                        strString	= getNumberFormat()->formatNumericValue(lfValue*fCustomScaleFactor, true, QString::number(lfValue*fCustomScaleFactor)) + strUnits;

                        // Valid sample value.
                        if(!isScalingNormalized)
                            lfValue *= ScalingPower(ptTestCell->res_scal);

                        // Check if this cell is holding a test failure
                        if(ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(lDataOffset)))
                            colorItem = Qt::red;
                    }
                }
                else
                {
                    strString = "-";
                    colorItem = colorGreen;
                }

                pTableWidgetItem = pTableWidget->item(lDataOffset,iDataYColOffset+(lTotalColsPerLayer*iLayerIndex));
                if (pTableWidgetItem == NULL)
                {
                    pTableWidgetItem = new QTableWidgetItem();
                    pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                    pTableWidget->setItem(lDataOffset,iDataYColOffset+(lTotalColsPerLayer*iLayerIndex), pTableWidgetItem);
                }

                pTableWidgetItem->setText(strString);
                pTableWidgetItem->setBackground(colorItem);
                pTableWidgetItem->setTextAlignment(Qt::AlignLeft);
            }
        }
        NextGroup:;
    };	// Read Trend of a given test in all groups so to stack all charts.

    // Update the HTML Info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_SCATTER,0);

    // Writes HTML table with global test in X info + chart (name, limits, Cp,cp shift,...)
    WriteHistoHtmlTestInfo(pChartsInfo,"adv_scatter",pFileX,ptTestReferenceCellX,NULL,NULL,true,"",GEX_CHARTSIZE_LARGE);

    // Writes HTML table with global test in Y info + chart (name, limits, Cp,cp shift,...)
    WriteHistoHtmlTestInfo(pChartsInfo,"adv_scatter",pFileY,ptTestReferenceCellY,NULL,NULL,true,"",GEX_CHARTSIZE_LARGE);

    // Close Info file created in $home (or Windows) folder
    CloseReportFile();	// Close report file
}

/////////////////////////////////////////////////////////////////////////////
// Remove ONE data point (sample index specified). Called multiple times when removing cells from table
// Examinator PRO only (not OEM releases)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::RemoveTableDataPoint(CTest *ptTestCell,int iIndex,bool bComputeStats)
{
    int	iFrom;
    double lfValue;
    double lfTotal=0, lfTotalSquare=0;
    double lfMin = C_INFINITE,lfMax = -C_INFINITE;
    int	iOutlierCount=0;

    // Check if we have samples!
    if(ptTestCell->ldSamplesValidExecs <= 0)
        return 0;
    if(ptTestCell->m_testResult.count() == 0)
        return 0;

    // Compute exponent to have value normalized.
    float lfExponent = ScalingPower(ptTestCell->res_scal);

    // If we have to keep values in normalized format, do not rescale!
    if (m_pReportOptions->GetOption("dataprocessing","scaling").toString()=="normalized")
        lfExponent = 1.0;

    if(iIndex >=0 && iIndex < ptTestCell->ldSamplesExecs)
    {
        // If datapoint to remove is a valid samples, update counter...
        if(ptTestCell->m_testResult.isValidResultAt(iIndex))
        {
            ptTestCell->ldSamplesValidExecs--;

            // Remove selection from the list
            ptTestCell->m_testResult.invalidateResultAt(iIndex);
        }
    }

    if(bComputeStats == false)
        return 0;

    QString orm=m_pReportOptions->GetOption("dataprocessing","outlier_removal").toString();
    // Compute/update all statistics as we've removed datapoint(s): usually called only once ALL datapoints selected are removed from the table.

    for(iFrom = 0; iFrom < ptTestCell->m_testResult.count(); iFrom++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(iFrom))
        {
            lfValue = ptTestCell->m_testResult.resultAt(iFrom);

            // Update Sum(X) and Sum^2(X) so we can compute new Mean & Sigma
            lfValue *= lfExponent;	// Adjust scale to match internal storage convention
            lfTotal += lfValue;
            lfTotalSquare += (lfValue*lfValue);
            lfMin = gex_min(lfMin,lfValue);
            lfMax = gex_max(lfMax,lfValue);

            // Test if test is an outlierl/Inlier...
            if(orm=="exclude_n_sigma")
            {
                // Inlier
                if((lfValue > ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) && (lfValue < ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
                    iOutlierCount++;
            }
            else
            {
                // Outlier
                if((lfValue < ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) || (lfValue>ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
                    iOutlierCount++;
            }
        }
    }


    // Update new data samples stats
    ptTestCell->lfSamplesMin = lfMin;
    ptTestCell->lfSamplesMax = lfMax;
    ptTestCell->lfSamplesTotal = lfTotal;
    ptTestCell->lfSamplesTotalSquare = lfTotalSquare;
    ptTestCell->GetCurrentLimitItem()->ldOutliers += iOutlierCount;	// Add all new outliers identified.

    // Force all statistics to be computed again (Cp, Cpk, etc...)
    m_cStats.UpdateOptions(m_pReportOptions);
    m_cStats.ComputeBasicTestStatistics(ptTestCell,true);
    m_cStats.RebuildHistogramArray(ptTestCell,GEX_HISTOGRAM_OVERDATA);
    QString pf=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();

    // Ensure we use latest options set
    m_cStats.UpdateOptions(m_pReportOptions);

    // Compute advanced statistics
    m_cStats.ComputeAdvancedDataStatistics(ptTestCell,true, pf=="percentile"?true:false);

    // return total outliers detected
    return iOutlierCount;
}

/////////////////////////////////////////////////////////////////////////////
// Remove all data points lower (or higher) than a given value+ recompute stats.
// Examinator PRO only (not OEM releases)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::RemoveDataPoints(
        CGexChartOverlays *pChartsInfo, CGexGroupOfFiles *pGroup, CTest *ptTestCell,
        double fLowLimit,double fHighLimit, int iRemoveMode,bool bForceStatsUpdate/*=false*/,
        bool bRemoveAssociatedParts /*=false*/, QList<TestRemoveResultCommand::TestRemovedResult *> *poRunIdxRemoved)
{
    CGexSingleChart *pChart;
    CGexFileInGroup *pFile;
    int		lTestNumber;
    int		lPinmapIndex;

    int		iFrom;
    double	lfValue;
    double	lfTotal, lfTotalSquare;
    double	lfMin,lfMax;
    int		iOutlierCount=0;
    bool	bValidData;
    int		iTotalLayers;
    double	lfExponent;
    int iInitOutliers = 0;
    TestRemoveResultCommand::TestRemovedResult  *poRemovedResult = 0;

    // Create list of parts to remove
    QList <long> cPartList;

    if((pChartsInfo == NULL) && (ptTestCell == NULL))
        return 0;

    if((pChartsInfo != NULL) && (ptTestCell != NULL))
        return 0;

    QString orm=m_pReportOptions->GetOption("dataprocessing","data_cleaning_mode").toString();
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");

    if(pChartsInfo != NULL)
    {
        // Removing samples from layers
        iTotalLayers = pChartsInfo->chartsList().count();
    }
    else
    {
        // Removing samples from one sepcific test (not dealing with layers)
        iTotalLayers = 1;
    }

    // Remove data points from all layers
    for(int iLayerIndex = 0; iLayerIndex < iTotalLayers; iLayerIndex++)
    {
        poRemovedResult = 0;
        iInitOutliers = 0;

        if(pChartsInfo != NULL)
        {
            // Processing layers
            pChart			= pChartsInfo->chartsList().at(iLayerIndex);
            lTestNumber		= pChart->iTestNumberX;
            lPinmapIndex	= pChart->iPinMapX;

            // Get pointer to relevant group
            pGroup	= (pChart->iGroupX < 0 || pChart->iGroupX >= getGroupsList().size()) ? NULL : getGroupsList().at(pChart->iGroupX);
            pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            if(pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false, false,
                                   pChart->strTestNameX.toLatin1().data())!=1)
                goto next_layer;

            // Data are going to change, reset data already computed.
            pChart->dataProbabilityPlot().clean();
        }
        iInitOutliers = ptTestCell->GetCurrentLimitItem()->ldOutliers;
        // Check if we have samples!
        if(ptTestCell->ldSamplesValidExecs <= 0)
            goto next_layer;
        if(ptTestCell->m_testResult.count() == 0)
            goto next_layer;

        // Compute exponent to have value normalized.
        lfExponent = ScalingPower(ptTestCell->res_scal);

        // If we have to keep values in normalized format, do not rescale!
        if(isScalingNormalized)
            lfExponent = 1.0;

        // Clear variables
        lfTotal=0;
        lfTotalSquare=0;
        lfMin = C_INFINITE;
        lfMax = -C_INFINITE;
        iOutlierCount=0;

        cPartList.clear();

        for(iFrom = 0; iFrom < ptTestCell->m_testResult.count(); iFrom++)
        {
      /// TO REVIEW : try to use better isValidResultAt() method, this loop should be simplifyed.

            bool isMultiResult = ptTestCell->m_testResult.isMultiResultAt(iFrom);
            for (int nCount = 0; nCount < (isMultiResult ? ptTestCell->m_testResult.at(iFrom)->count() : 1); nCount++){
                bValidData = false;
                lfValue = isMultiResult ? (ptTestCell->m_testResult.at(iFrom)->multiResultAt(nCount)) : (ptTestCell->m_testResult.resultAt(iFrom));
                lfValue *= lfExponent;	// Adjust scale to match internal storage convention
                if(lfValue == GEX_C_DOUBLE_NAN || qIsInf(lfValue)
                        ||  (!isMultiResult && !ptTestCell->m_testResult.isValidResultAt(iFrom))
                        || (isMultiResult && !ptTestCell->m_testResult.at(iFrom)->isValidResultAt(nCount)))
                    bValidData = false;
                else
                {
                    switch(iRemoveMode)
                    {
                    case GEX_REMOVE_HIGHER_THAN: // Remove values higher than 'fHighLimit', keep all lower values
                        if(lfValue <= fHighLimit)
                            bValidData= true;
                        break;
                    case GEX_REMOVE_LOWER_THAN:	// Remove values lower than 'fLowLimit', keep all higher values
                        if(lfValue >= fLowLimit)
                            bValidData= true;
                        break;
                    case GEX_REMOVE_DATA_RANGE: // Remove range of values
                        if((lfValue < fLowLimit) || (lfValue > fHighLimit))
                            bValidData= true;
                        break;
                    }
                }

                // If this data sample is kept, then save it + update min/max etc...
                if(bValidData)
                {
                    // Update Sum(X) and Sum^2(X) so we can compute new Mean & Sigma


                    lfTotal += lfValue;
                    lfTotalSquare += (lfValue*lfValue);
                    lfMin = gex_min(lfMin,lfValue);
                    lfMax = gex_max(lfMax,lfValue);


                    // Outlier
                    switch(iRemoveMode)
                    {
                        case GEX_REMOVE_HIGHER_THAN: // Remove values higher than 'fHighLimit', keep all lower values
                            if(lfValue>ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier)
                                iOutlierCount++;
                            break;
                        case GEX_REMOVE_LOWER_THAN:	// Remove values lower than 'fLowLimit', keep all higher values
                            if(lfValue < ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier)
                                iOutlierCount++;
                            break;
                        case GEX_REMOVE_DATA_RANGE: // Remove range of values
                            // Test if test is an outlierl/Inlier...
                            if(orm=="exclude_n_sigma")

                            {
                                // Inlier
                                if((lfValue > ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) && (lfValue < ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
                                    iOutlierCount++;
                            }
                            else if((lfValue < ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) || (lfValue>ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
                                iOutlierCount++;
                            break;
                    }
                }
                else
                {
                bool bIsValidValue = isMultiResult ? ptTestCell->m_testResult.at(iFrom)->isValidResultAt(nCount) : ptTestCell->m_testResult.isValidResultAt(iFrom);

                if(poRunIdxRemoved && !poRemovedResult)
                {
                    poRemovedResult = new TestRemoveResultCommand::TestRemovedResult(ptTestCell, pGroup, bRemoveAssociatedParts);
                    poRemovedResult->setOutlierRemoval(iInitOutliers);
                    poRunIdxRemoved->append(poRemovedResult);
                }
                if(!isMultiResult ){
                    ptTestCell->m_testResult.invalidateResultAt(iFrom);
                    if(bIsValidValue && poRemovedResult){
                        poRemovedResult->addRemovedIdx(iFrom);
                    }
                }
                else{
                    ptTestCell->m_testResult.at(iFrom)->invalidateResultAt(nCount);
                    if(bIsValidValue && poRemovedResult){
                        poRemovedResult->addRemovedIdx(iFrom, nCount);
                    }

                }
                if(lfValue != GEX_C_DOUBLE_NAN && !qIsInf(lfValue) && bIsValidValue )
                    {
                        iOutlierCount++;	// This removed data is now counted as an outlier.
                        ptTestCell->ldSamplesValidExecs--;

                        cPartList.append(iFrom);
                    }
                    // Value to mark as 'removed'

                }
            }
        }

        // All data points filtered, update statistics
        ptTestCell->lfSamplesMin = lfMin;
        ptTestCell->lfSamplesMax = lfMax;
        ptTestCell->lfSamplesTotal = lfTotal;
        ptTestCell->lfSamplesTotalSquare = lfTotalSquare;
        ptTestCell->GetCurrentLimitItem()->ldOutliers += iOutlierCount;

        // No longer keep summary info since data manually filtered.
        ptTestCell->ldExecs = ptTestCell->ldSamplesValidExecs;
        ptTestCell->lfTotal = ptTestCell->lfSamplesTotal;
        ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare;
        ptTestCell->lfMax = lfMax;
        ptTestCell->lfMin = lfMin;

        // If dealing with multi-layers: Force all statistics to be computed again (Cp, Cpk, etc...)
        if(bForceStatsUpdate || pChartsInfo != NULL)
        {
            m_cStats.ComputeBasicTestStatistics(ptTestCell,true);
            m_cStats.RebuildHistogramArray(ptTestCell,GEX_HISTOGRAM_OVERDATA);
            QString pf=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();
            m_cStats.ComputeAdvancedDataStatistics(ptTestCell,true,pf=="percentile"?true:false);
        }

        if (bRemoveAssociatedParts && pGroup)
            pGroup->removeParts(cPartList);

next_layer:;
    }

    // Return total outliers detected
    return iOutlierCount;
}

/////////////////////////////////////////////////////////////////////////////
// Dumps all the Data points into the QTable
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::ListAdvTrendData(CGexChartOverlays *pChartsInfo, QTableWidget * pTableWidget,
                                     CGexFileInGroup *pFile, CTest *ptTestReferenceCell)
{
    CTest *		ptTestCell,*ptPartIdCell;
    long		lTestNumber, lPinmapIndex;
    char		szTestName[1024];
    double		lfValue;
    long		iRows=0;
    long		lDataOffset;
    unsigned	iLayerIndex=0;
    unsigned	iTotalLayers = pChartsInfo->chartsList().count();
    QString		strUnits;
    QString		strString;
    CGexSingleChart	*	pChart = NULL;	// Handle to Parameter Layer info.
    CGexGroupOfFiles *	pGroup;
    QTableWidgetItem *	pTableWidgetItem = NULL;
    QColor colorGreen	= QColor(192,255,192);
    QColor colorItem;

    // Clear the table
    pTableWidget->clear();

    // If no layers...exit!
    if(iTotalLayers <= 0)
        return;

    // X column per group (as many as layers)....and double if list PartID too!
    long	lTotalColsPerLayer;
    int		iDataColOffset;
    double	lfCustomScaleFactor;
    bool	bShowPartID = (m_pReportOptions->GetOption("dataprocessing", "part_id").toString() == "show") ? true : false;
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");

    if (bShowPartID)
    {
        lTotalColsPerLayer = 2;	// 2 columns per result: <PartID> , <Result>
        iDataColOffset = 1;		// Offset to <Result>

    }
    else
    {
        lTotalColsPerLayer = 1;	// 1 column per result (no partID) : <Result>
        iDataColOffset = 0;		// Offset to <Result>
    }

    pTableWidget->setColumnCount(iTotalLayers*lTotalColsPerLayer);

    // Stack all charts of all groups for each same test
    for(iLayerIndex = 0; iLayerIndex < iTotalLayers; iLayerIndex++)
    {
        pChart			= pChartsInfo->chartsList().at(iLayerIndex);
        lTestNumber		= pChart->iTestNumberX;
        lPinmapIndex	= pChart->iPinMapX;

        // Get pointer to relevant group
        pGroup	= (pChart->iGroupX < 0 || pChart->iGroupX >= getGroupsList().size()) ? NULL : getGroupsList().at(pChart->iGroupX);
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false, pChart->strTestNameX.toLatin1().data())!=1)
            goto NextLayer;

        // Update number of rows
        iRows = gex_max(iRows,ptTestCell->ldSamplesExecs);
        pTableWidget->setRowCount(iRows);

        // If to display PartID, do it now!
        if (bShowPartID)
        {
            // Build header title for this given group.
            strString = pGroup->strGroupName + " - Part ID";

            if (pTableWidget->horizontalHeaderItem(lTotalColsPerLayer*iLayerIndex) == NULL)
                pTableWidget->setHorizontalHeaderItem(lTotalColsPerLayer*iLayerIndex, new QTableWidgetItem(strString));
            else
                pTableWidget->horizontalHeaderItem(lTotalColsPerLayer*iLayerIndex)->setText(strString);

            CGexFileInGroup *	pFileTmp	= NULL;
            int					nBegin		= 0;
            int					nEnd		= 0;
            int					nPartID		= 0;

            // Loop on each file in order to find the right partID string
            for (int nFile = 0; nFile < pGroup->pFilesList.count(); nFile++)
            {
                pFileTmp = pGroup->pFilesList.at(nFile);

                // Load all values in the table.
                if (pFileTmp->FindTestCell(GEX_TESTNBR_OFFSET_EXT_PARTID,GEX_PTEST,&ptPartIdCell,false,false) == 1)
                {
                    // Find out begin and end index for this sublot
                    ptPartIdCell->findSublotOffset(nBegin, nEnd, nFile);

                    for(lDataOffset = nBegin; lDataOffset < nEnd; lDataOffset++)
                    {
                        nPartID	= lDataOffset-nBegin;

                        if (nPartID < iRows)
                        {
                            if(ptPartIdCell->m_testResult.isValidResultAt(lDataOffset))
                            {
                                strString = pFileTmp->pPartInfoList.at(nPartID)->getPartID();
                                colorItem = Qt::white;
                            }
                            else
                            {
                                strString = "n/a";
                                colorItem = colorGreen;
                            }

                            pTableWidgetItem = pTableWidget->item(lDataOffset,lTotalColsPerLayer*iLayerIndex);
                            if (pTableWidgetItem == NULL)
                            {
                                pTableWidgetItem = new QTableWidgetItem();
                                pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                                pTableWidget->setItem(lDataOffset, lTotalColsPerLayer*iLayerIndex, pTableWidgetItem);
                            }

                            pTableWidgetItem->setText(strString);
                            pTableWidgetItem->setBackground(colorItem);
                            pTableWidgetItem->setTextAlignment(Qt::AlignRight);
                        }
                    }
                }
            }

        }


        // Build header title for this given group.
        strString = pGroup->strGroupName + " - T";
        strString += ptTestCell->szTestLabel;
        strString += ": ";
        BuildTestNameString(pFile,ptTestCell,szTestName);
        strString += szTestName;

        if (pTableWidget->horizontalHeaderItem(iDataColOffset+(lTotalColsPerLayer*iLayerIndex)) == NULL)
            pTableWidget->setHorizontalHeaderItem(iDataColOffset+(lTotalColsPerLayer*iLayerIndex), new QTableWidgetItem(strString));
        else
            pTableWidget->horizontalHeaderItem(iDataColOffset+(lTotalColsPerLayer*iLayerIndex))->setText(strString);

        // Load all values in the table.
        if(ptTestCell->m_testResult.count() > 0 && (ptTestCell->ldSamplesValidExecs > 0))
        {
            // Get scaled units (as data samples in the array are not normalized, but in res_scal format)
            strUnits = ptTestCell->GetScaledUnits(&lfCustomScaleFactor, scaling);

            for(lDataOffset=0;lDataOffset < ptTestCell->m_testResult.count(); lDataOffset++)
            {
                if(ptTestCell->m_testResult.isValidResultAt(lDataOffset))
                {
                    colorItem	= Qt::white;
                    strString.clear();

                    if(ptTestCell->m_testResult.isMultiResultAt(lDataOffset))
                    {
                        CTestResult::PassFailStatus ePassFailStatus = ptTestCell->m_testResult.passFailStatus(lDataOffset);

                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lDataOffset)->count(); ++nCount)
                        {
                            if(!ptTestCell->m_testResult.at(lDataOffset)->isValidResultAt(nCount)) continue;
                            if (nCount > 0)
                                strString += " | ";

                            lfValue		= ptTestCell->m_testResult.at(lDataOffset)->multiResultAt(nCount);
                            strString	+= getNumberFormat()->formatNumericValue(lfValue * lfCustomScaleFactor, true, QString::number(lfValue * lfCustomScaleFactor, 'g', 15)) + strUnits;

                            // Valid sample value.
                            if(!isScalingNormalized)
                                lfValue *= ScalingPower(ptTestCell->res_scal);

                            // Check if this cell is holding a test failure
                            if(ptTestCell->isFailingValue(lfValue, ePassFailStatus))
                                colorItem = Qt::red;
                        }
                    }
                    else
                    {
                        lfValue		= ptTestCell->m_testResult.resultAt(lDataOffset);
                        strString	+= getNumberFormat()->formatNumericValue(lfValue * lfCustomScaleFactor, true, QString::number(lfValue * lfCustomScaleFactor, 'g', 15)) + strUnits;

                        // Valid sample value.
                        if(!isScalingNormalized)
                            lfValue *= ScalingPower(ptTestCell->res_scal);

                        // Check if this cell is holding a test failure
                        if(ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(lDataOffset)))
                            colorItem = Qt::red;
                    }
                }
                else
                {
                    strString = "";
                    colorItem = colorGreen;
                }

                pTableWidgetItem = pTableWidget->item(lDataOffset,iDataColOffset+(lTotalColsPerLayer*iLayerIndex));
                if (pTableWidgetItem == NULL)
                {
                    pTableWidgetItem = new QTableWidgetItem();
                    pTableWidgetItem->setFlags(pTableWidgetItem->flags() & ~Qt::ItemIsEditable);
                    pTableWidget->setItem(lDataOffset, iDataColOffset+(lTotalColsPerLayer*iLayerIndex), pTableWidgetItem);
                }

                pTableWidgetItem->setText(strString);
                pTableWidgetItem->setBackground(colorItem);
                pTableWidgetItem->setTextAlignment(Qt::AlignLeft);
            }
        }

        NextLayer:;
    };	// Read Trend of a given test in all groups so to stack all charts.

    // Update the HTML Info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_TREND,0);

    // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
    WriteHistoHtmlTestInfo(pChartsInfo,"adv_trend",pFile,ptTestReferenceCell,NULL,NULL,true,"",GEX_CHARTSIZE_LARGE);

    // Close Info file created in $home (or Windows) folder
    CloseReportFile();	// Close report file
}

/////////////////////////////////////////////////////////////////////////////
// Compute statistical aggregate data point for an array of samples
// (Mean, or Sigma, or Cp, or Cpk)
/////////////////////////////////////////////////////////////////////////////
double CGexReport::ComputeAggregateValue(CTest *ptTestCell,long lFromDataSample,long lToDataSample,int iAggregateInfo)
{
    // Step1: compute MEAN,SumX, SumX^2 as it is required for all aggregates computations
    double	lfMean,lfTotal=0,lfTotalSquare=0;
    double	lfSigma,CpkL,CpkH;
    double	lfResult;
    long	ldTotalExecs = 0;

    float		lfExponent;

    // Compute exponent to have value normalized.
    lfExponent = ScalingPower(ptTestCell->res_scal);

    // If we have to keep values in normalized format, do not rescale!
    if (m_pReportOptions->GetOption("dataprocessing","scaling").toString()=="normalized")
        lfExponent = 1.0;

    GEX_ASSERT(lToDataSample<ptTestCell->m_testResult.count());

    for(long iIndex = lFromDataSample; iIndex < lToDataSample; iIndex++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(iIndex))
        {
            if (ptTestCell->m_testResult.isMultiResultAt(iIndex))
            {
                for (int nCount = 0; nCount < ptTestCell->m_testResult.at(iIndex)->count(); ++nCount)
                {
                    if(ptTestCell->m_testResult.at(iIndex)->isValidResultAt(nCount)){
                        lfResult		= ptTestCell->m_testResult.at(iIndex)->multiResultAt(nCount);

                        lfResult		*= lfExponent;              // Data sample
                        lfTotal			+= lfResult;				// SumX
                        lfTotalSquare	+= (lfResult*lfResult);		// Sum X^2

                        // Keep track of total valid samples in array
                        ldTotalExecs++;
                    }
                }
            }
            else
            {
                lfResult		= ptTestCell->m_testResult.resultAt(iIndex);

                lfResult		*= lfExponent;                      // Data sample
                lfTotal			+= lfResult;						// SumX
                lfTotalSquare	+= (lfResult*lfResult);				// Sum X^2

                // Keep track of total valid samples in array
                ldTotalExecs++;
            }
        }
    }
    // Compute Mean.
    lfMean = lfTotal / ldTotalExecs;

    if(ldTotalExecs <= 1)
        lfSigma = 0;
    else
        lfSigma = sqrt(fabs(((ldTotalExecs*lfTotalSquare) - GS_POW(lfTotal,2.0))/
                (ldTotalExecs*(ldTotalExecs-1))));

    // Step2: compute aggregate statistical value.
    switch(iAggregateInfo)
    {
        case GEX_ADV_TREND_AGGREGATE_MEAN:
        default:
            return lfMean;

        case GEX_ADV_TREND_AGGREGATE_SIGMA:
            return lfSigma;

        case GEX_ADV_TREND_AGGREGATE_CP:
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) || (lfSigma <= 0))
                return C_NO_CP_CPK;
            lfResult = fabs(ptTestCell->GetCurrentLimitItem()->lfHighLimit - ptTestCell->GetCurrentLimitItem()->lfLowLimit)/(6.0*lfSigma);
            return lfResult;


        case GEX_ADV_TREND_AGGREGATE_CPK:
            if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL)
                CpkL = C_NO_CP_CPK;	// No Low Limit
            else
            {
                if(lfSigma == 0)
                    CpkL = C_INFINITE;
                else
                  CpkL = (lfMean-ptTestCell->GetCurrentLimitItem()->lfLowLimit)/(3.0*lfSigma);
                if(CpkL > 1e6) CpkL = C_INFINITE;
            }

            if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL)
                CpkH = C_NO_CP_CPK;	// No High Limit.
            else
            {
                if(lfSigma == 0)
                    CpkH = C_INFINITE;
                else
                    CpkH = (ptTestCell->GetCurrentLimitItem()->lfHighLimit-lfMean)/(3.0*lfSigma);
                if(CpkH > 1e6) CpkH = C_INFINITE;
                if(CpkH < -1e6) CpkH = -C_INFINITE;
            }
            lfResult = gex_min(CpkL,CpkH);
            if(lfResult > 1e6) lfResult = C_INFINITE;
            if(lfResult < -1e6) lfResult = -C_INFINITE;
            return lfResult;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Trend report page for Aggregate data:
// Mean, or Sigma, or Cp, or Cpk
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvTrendChartPageAggregateEx(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                                      CTest *ptTestReferenceCell,int iChartSize,
                                                      bool bPluginCall,int iAggregateInfo)
{
    CTest *			ptTestCell;
    CTest *			ptScaleTestCellReference = ptTestReferenceCell;	// Pointer to Test used a reference for scaling if multiple layers involved
    char			szTestName[2*GEX_MAX_STRING];
    char			szXscaleLabel[50];
    char			szString[300];
    int				i;
    int				iCurvePenSize;			// curve pen size 1 or 2 pixels.
    int				iSizeX=0,iSizeY=0;
    double *		x;						// buffer to holds X data.
    double *		y;						// Buffer to hold trend data in case we have multiple layers and need to rescale datalog results.
    double			lfMean,lfMin,lfMax,lfMedian,lfSigma,lfExtra,lfChartBottom=C_INFINITE,lfChartTop=-C_INFINITE,lfHighL,lfLowL;
    double			lfDataStart=C_INFINITE,lfDataEnd=-C_INFINITE;
    double			lStartID=0.0,lEndID=0.0;
    double			lfData;
    double			lfExponent ;

    // Pointer to label strings...may vary depending of the chart size!
    const char *	szLabelLL	= "Low Limit";
    const char *	szLabelHL	= "High Limit";
    int				lTestNumber;
    int				lPinmapIndex;
    int				iGroup		= 1;					// Counter used to know on which group we are (#1 or other)
    int				iTestReferenceScaleFactor	= 0;	// Scale factor for 1st test (Chart scales based on it)
    int				iCustomScaleFactor			= 0;	// Correction custom Scale factor to apply on all but 1st chart so all scales match!
    double			fCustomScaleFactor			= 0.0;	// Scale ratio to apply multiplyer)
    QString			strLabel;
    long			lTotalSublots;
    long			lSubLotID;
    long			lFromDataSample=0,lToDataSample=0;

    // OPTIONS
    QString strAdvTrendMarkerOptions = (m_pReportOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");

    if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Check if need to create new HTML Trend. page
        if(!bPluginCall)
            CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_TREND,ptTestReferenceCell->iHtmlAdvancedPage);

        // Image size to create for the trend chart
        switch(iChartSize)
        {
        case GEX_CHARTSIZE_SMALL:
            // Shortest label possible
            szLabelHL = "HL";
            szLabelLL = "LL";
            // No grid
            iSizeX = GEX_CHARTSIZE_SMALL_X;
            iSizeY = GEX_CHARTSIZE_SMALL_Y;
            break;
        case GEX_CHARTSIZE_MEDIUM:
            // Shortest label possible
            szLabelHL = "HL";
            szLabelLL = "LL";
            iSizeX = GEX_CHARTSIZE_MEDIUM_X;
            iSizeY = GEX_CHARTSIZE_MEDIUM_Y;
            break;
        case GEX_CHARTSIZE_LARGE:
            iSizeX = GEX_CHARTSIZE_LARGE_X;
            iSizeY = GEX_CHARTSIZE_LARGE_Y;
            break;
        }
    }

    // Get background color
    unsigned int iBackgroundColor = (m_pReportOptions->cBkgColor.rgb() & 0xFFFFFF);

    // Create chart window
    m_pChartDirector->allocateXYChart(iSizeX, iSizeY, iBackgroundColor);
    int nGridColor = m_pChartDirector->xyChart()->dashLineColor(0xc0c0c0, DashLine);
    m_pChartDirector->xyChart()->setPlotArea(60, 25, iSizeX-80, iSizeY-65)->setGridColor(nGridColor, nGridColor);
    m_pChartDirector->xyChart()->setClipping();

    //Set the font for the y axis labels to Arial
    m_pChartDirector->xyChart()->yAxis()->setLabelStyle("arial.ttf");

    // Index used to keep track of Chart layers being charted.
    CGexSingleChart	*pChart=NULL;	// Handle to Parameter Layer info.
    int				iLineWidth=1;
    int				iLayerIndex=0;
    bool			bMultiLayers;
    QColor			cPenColor;
    int				cPenWidth;
    QColor			cLayerColor;	// Color used to plot a layer
    QString         lParameterName;

    // Check if charting multi-layers chart...
    if(pChartsInfo && pChartsInfo->chartsList().count() > 1)
        bMultiLayers = true;
    else
        bMultiLayers = false;

    // Stack all charts of all groups for each same test
    QList<CGexGroupOfFiles*>::iterator	itGroupList = getGroupsList().begin();
    CGexGroupOfFiles *					pGroup		= (itGroupList != getGroupsList().end()) ? (*itGroupList) : NULL;

    while(pGroup!=NULL)
    {
        ptTestCell = NULL;

        // List of tests in group#1, then 2, etc...
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if(pChartsInfo)
            pChart = pChartsInfo->chartsList().at(iLayerIndex);
        if(pChart)
        {
            // We have multiple charts (layers) to plot!
            lTestNumber = pChart->iTestNumberX;
            lPinmapIndex = pChart->iPinMapX;
            lParameterName = pChart->strTestNameX;

            // Get ploting details
            iLineWidth = pChart->iLineWidth;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            lTestNumber     = ptTestReferenceCell->lTestNumber;
            lParameterName  = ptTestReferenceCell->strTestName;
            if(ptTestReferenceCell->lPinmapIndex >= 0)
                lPinmapIndex = ptTestReferenceCell->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;
        }

        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,
                               lParameterName.toLatin1().data())!=1)
            goto NextGroup;

        // Check if we have data samples
        lTotalSublots = ptTestCell->pSamplesInSublots.count();
        if(lTotalSublots <= 0)
        {
            strcpy(szXscaleLabel,"No data");
            goto NextGroup;
        }
        else
            strcpy(szXscaleLabel,"Sublots");


        // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
        if(iGroup == 1)
        {
            // Multi-layer: Chart scales will be of 1st Parameter in 1st layer
            if(pChart)
            {
                if(iLayerIndex == 1)
                {
                    iTestReferenceScaleFactor = ptTestCell->res_scal;
                    ptScaleTestCellReference = ptTestCell;
                }
            }
            else
            {
                iTestReferenceScaleFactor = ptTestCell->res_scal;// Not multi layer (but maybe multi-groups to overlay), then chart scales will be of 1st group.
                ptScaleTestCellReference = ptTestCell;
            }
        }
        // For tests such as binning type = '-'), do not rescale, align to first layer scale.
        if(ptTestCell->bTestType == '-')
            fCustomScaleFactor = 1;
        else
        {
            iCustomScaleFactor = ptTestCell->res_scal - iTestReferenceScaleFactor;
            fCustomScaleFactor = 1/GS_POW(10.0,iCustomScaleFactor);
        }
        lfExponent = ScalingPower(ptTestCell->res_scal);
        // Allocate buffer to receive the data points.
        x = new double[lTotalSublots];
        if(x == NULL)
            return;
        y = new double[lTotalSublots];
        if(y == NULL)
            return;

        // Compute aggregate value for each sublot data samples.
        lFromDataSample = lToDataSample=0;
        lStartID = 1;
        lfData = 0;	// Used to store aggregate value for a given sublot.
        lEndID = lStartID+lTotalSublots;
        for(lSubLotID=0; lSubLotID < lTotalSublots; lSubLotID++)
        {
            // Total samples in sublot.
            lToDataSample += ptTestCell->pSamplesInSublots[lSubLotID];

            // X axis is 1 steps increments starting from 1
            x[lSubLotID] = lSubLotID+1;

            // If a sublot doesn't have any data, we simply keep the value of the previous sub-lot!
            // and if the first sublot doesn't have data, we force a '0' value.
            if(lFromDataSample != lToDataSample)
                 lfData = ComputeAggregateValue(ptTestCell,lFromDataSample,lToDataSample,iAggregateInfo)/lfExponent;
            y[lSubLotID] = lfData;

            // Update min/max values so chart's scale will be accurate
            lfDataStart = gex_min(lfDataStart,lfData);
            lfDataEnd = gex_max(lfDataEnd,lfData);

            // Update starting point in data samples array.
            lFromDataSample = lToDataSample;
        }

        if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
            if((iChartSize != GEX_CHARTSIZE_SMALL) && (qslAdvTrendMarkerOptionList.contains(QString("test_name"))) && (iGroup==1))
            {
                if(bMultiLayers && (isSingleTestAllLayers(pChartsInfo) == false))
                {
                    // In multi-layer mode and with multiple layers shown, title is generic:
                    strcpy(szString,"Overlay Tests/Parameters");
                }
                else
                {
                    BuildTestNameString(pFile,ptTestCell,szTestName);
                    sprintf(szString,"Test %s: %s",ptTestCell->szTestLabel,szTestName);
                }
            }
            else
                *szString = 0;	// No title!

            // Check if we have a custom title to overwrite default one
            if(pChartsInfo)
            {
                // Multilayrs, check if we have assigned it a title...
                if(pChartsInfo)
                {
                    // Multilayrs, check if we have assigned it a title...
                    if(pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.bChartTitle)
                        m_pChartDirector->xyChart()->addTitle(pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.strChartTitle.toLatin1().constData());	// custom title.
                    else
                        m_pChartDirector->xyChart()->addTitle(szString);	// Non-custom title
                }
            }
            else
                m_pChartDirector->xyChart()->addTitle(szString);	// Non-custom title
        }
        else
        {
            // CSV dump...
            BuildTestNameString(pFile,ptTestCell,szTestName);
            fprintf(hReportFile,"\nTest %s: %s, Group name: %s\n",
                ptTestCell->szTestLabel,szTestName,
                pGroup->strGroupName.toLatin1().constData());
        }

        // Insert new curves
        if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            int nFillColor, nLineColor;
            Chart::SymbolType layerSymbol;
            SetLayerStyles(iGroup, pChart, nFillColor, nLineColor, layerSymbol, SECTION_ADV_TREND);

            LineLayer * pLineLayer = m_pChartDirector->xyChart()->addLineLayer(DoubleArray(y, lTotalSublots), nLineColor);
            pLineLayer->setXData(DoubleArray(x, lTotalSublots));

            iCurvePenSize = iLineWidth;	// Xpixel wide curve (1 is default)
            pLineLayer->setLineWidth(iCurvePenSize);

            if (layerSymbol != Chart::NoSymbol)
                pLineLayer->getDataSet(0)->setDataSymbol(layerSymbol, 6, nLineColor, nLineColor);

            // Delete temporary Xand Y buffers
            delete x;
            delete y;
        }
        else
        {
            // CSV file: dump data to CSV file.
            fprintf(hReportFile,"Sublots,");
            for(i=0;i<lTotalSublots;i++)
                fprintf(hReportFile,"%f,",x[i]);
            fprintf(hReportFile,"\nValue,");
            for(i=0;i<lTotalSublots;i++)
                fprintf(hReportFile,"%f,",y[i]);
            fprintf(hReportFile,"\n");

            // Delete temporary Xand Y buffers
            delete x;
            delete y;

            goto NextGroup;
        }

        // Draw limit markers (done only once: when charting Plot for group#1)
        if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)  && (iGroup==1))
        {
            lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
            // If we have to keep values in normalized format, do not rescale!
            if(!isScalingNormalized)
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->llm_scal);
                lfLowL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                lfLowL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfLowL *= fCustomScaleFactor;
            // LowLimit Marker
            if((qslAdvTrendMarkerOptionList.contains(QString("limits"))) || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bMultiLayers);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfLowL, cPenColor.rgb() & 0xffffff, szLabelLL, cPenWidth, Chart::TopLeft);
            }
        }
        if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (iGroup==1))
        {
            lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
            // If we have to keep values in normalized format, do not rescale!
            if (!isScalingNormalized)
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->hlm_scal);
                lfHighL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                lfHighL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfHighL *= fCustomScaleFactor;
            // High limit Marker
            if((qslAdvTrendMarkerOptionList.contains(QString("limits"))) || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bMultiLayers);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfHighL, cPenColor.rgb() & 0xffffff, szLabelHL, cPenWidth, Chart::TopLeft);
            }
        }

        // Update Y viewport start-ending points, based on data
        lfChartTop = gex_max(lfChartTop,lfDataEnd);
        lfChartBottom = gex_min(lfChartBottom,lfDataStart);

        // Update Y viewport start-ending points, based on limits
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            lfChartBottom = gex_min(lfLowL,lfChartBottom);		// Low limit exists
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            lfChartTop = gex_max(lfHighL,lfChartTop);			// High limit exists

        // Insert markers (horizontal lines): Mean, LowL, HighL
        // Scale Mean & Sigma to correct scale
        lfMean = fCustomScaleFactor*ptTestCell->lfMean;
        pFile->FormatTestResultNoUnits(&lfMean,ptTestCell->res_scal);
        lfSigma = fCustomScaleFactor*ptTestCell->lfSigma;
        pFile->FormatTestResultNoUnits(&lfSigma,ptTestCell->res_scal);

        // If request to show the Mean
        if((qslAdvTrendMarkerOptionList.contains(QString("mean"))) || (pChart != NULL))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->meanColor(bMultiLayers);
                cPenWidth = pChart->meanLineWidth();
            }
            else
            {
                cPenColor = Qt::blue;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nPenColor;

                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && (pChart == NULL))
                    nPenColor = GetChartingColor(iGroup).rgb() & 0xffffff;
                else
                    nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean, nPenColor, "Mean", cPenWidth, Chart::TopRight);
                else
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean, nPenColor, "Mean", cPenWidth, Chart::BottomRight);

                // Update Y viewport start-ending points, based on Mean marker
                lfChartTop = gex_max(lfChartTop,lfMean);
                lfChartBottom = gex_min(lfChartBottom,lfMean);
            }
        }

        // If request to show the Median
        if(((qslAdvTrendMarkerOptionList.contains(QString("median"))) || (pChart != NULL)) &&
                (ptTestCell->lfSamplesQuartile2 != -C_INFINITE))
        {
            lfMedian = fCustomScaleFactor*ptTestCell->lfSamplesQuartile2;
            pFile->FormatTestResultNoUnits(&lfMedian,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->medianColor(bMultiLayers);
                cPenWidth = pChart->medianLineWidth();
            }
            else
            {
                cPenColor = Qt::blue;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nPenColor;

                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && (pChart == NULL))
                    nPenColor = GetChartingColor(iGroup).rgb() & 0xffffff;
                else
                    nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMedian, nPenColor, "Median", cPenWidth, Chart::TopRight);
                else
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMedian, nPenColor, "Median", cPenWidth, Chart::BottomRight);

                // Update Y viewport start-ending points, based on Median marker
                lfChartTop = gex_max(lfChartTop,lfMedian);
                lfChartBottom = gex_min(lfChartBottom,lfMedian);
            }
        }

        // If request to show the Min
        if((m_pReportOptions->bTrendMarkerMin) || (pChart != NULL))
        {
            lfMin = fCustomScaleFactor*ptTestCell->lfSamplesMin;
            pFile->FormatTestResultNoUnits(&lfMin,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->minColor(bMultiLayers);
                cPenWidth = pChart->minLineWidth();
            }
            else
            {
                cPenColor = Qt::blue;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nPenColor;

                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && (pChart == NULL))
                    nPenColor = GetChartingColor(iGroup).rgb() & 0xffffff;
                else
                    nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMin, nPenColor, "Min.", cPenWidth, Chart::TopLeft);
                else
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMin, nPenColor, "Min.", cPenWidth, Chart::TopRight);
            }
        }

        // If request to show the Max
        if((m_pReportOptions->bTrendMarkerMax) || (pChart != NULL))
        {
            lfMax = fCustomScaleFactor*ptTestCell->lfSamplesMax;
            pFile->FormatTestResultNoUnits(&lfMax,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->maxColor(bMultiLayers);
                cPenWidth = pChart->maxLineWidth();
            }
            else
            {
                cPenColor = Qt::blue;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nPenColor;

                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && (pChart == NULL))
                    nPenColor = GetChartingColor(iGroup).rgb() & 0xffffff;
                else
                    nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis2(), lfMax, nPenColor, "Max.", cPenWidth, Chart::TopLeft);
                else
                    m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis2(), lfMax, nPenColor, "Max.", cPenWidth, Chart::TopRight);
            }
        }

        // If request to show the 2sigma space
        if(( (qslAdvTrendMarkerOptionList.contains(QString("2sigma"))) && (iGroup==1))  || (pChart != NULL))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma2Color(bMultiLayers);
                cPenWidth = pChart->sigma2LineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pChartDirector->xyChart()->dashLineColor(nTextColor, DotDashLine);

                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean-lfSigma, nLineColor, "-1s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean+lfSigma, nLineColor, "+1s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                // Update Y viewport start-ending points, based on 2Sigma marker
                lfChartTop = gex_max(lfChartTop,lfMean+lfSigma);
                lfChartBottom = gex_min(lfChartBottom,lfMean-lfSigma);
            }
        }

        // If request to show the 3sigma space
        if(( (qslAdvTrendMarkerOptionList.contains(QString("3sigma"))) && (iGroup==1)) || (pChart != NULL))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma3Color(bMultiLayers);
                cPenWidth = pChart->sigma3LineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pChartDirector->xyChart()->dashLineColor(nTextColor, DotDashLine);

                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean-1.5*lfSigma, nLineColor, "-1.5s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean+1.5*lfSigma, nLineColor, "+1.5s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                // Update Y viewport start-ending points, based on 3Sigma marker
                lfChartTop		= gex_max(lfChartTop,lfMean+1.5*lfSigma);
                lfChartBottom	= gex_min(lfChartBottom,lfMean-1.5*lfSigma);
            }
        }

        // If request to show the 6sigma space
        if(( (qslAdvTrendMarkerOptionList.contains(QString("6sigma"))) && (iGroup==1)) || (pChart != NULL))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma6Color(bMultiLayers);
                cPenWidth = pChart->sigma6LineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pChartDirector->xyChart()->dashLineColor(nTextColor, DotDashLine);

                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean-3*lfSigma, nLineColor, "-3s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean+3*lfSigma, nLineColor, "+3s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);

                // Update Y viewport start-ending points, based on 6Sigma marker
                lfChartTop		= gex_max(lfChartTop,lfMean+3*lfSigma);
                lfChartBottom	= gex_min(lfChartBottom,lfMean-3*lfSigma);
            }
        }

        // If request to show the 12sigma space
        if(( (qslAdvTrendMarkerOptionList.contains(QString("12sigma"))) && (iGroup==1)) || (pChart != NULL))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma12Color(bMultiLayers);
                cPenWidth = pChart->sigma12LineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pChartDirector->xyChart()->dashLineColor(nTextColor, DotDashLine);

                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean-6*lfSigma, nLineColor, "-6s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->yAxis(), lfMean+6*lfSigma, nLineColor, "+6s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);

                // Update Y viewport start-ending points, based on 12Sigma marker
                lfChartTop = gex_max(lfChartTop,lfMean+6*lfSigma);
                lfChartBottom = gex_min(lfChartBottom,lfMean-6*lfSigma);
            }
        }

        // We have a chart legend unless the chart size in only 200 pixel wide (SMALL chart mode)!...done only once at 1st group processing
        if((iChartSize != GEX_CHARTSIZE_SMALL) && (iGroup==1))
        {
            if(iChartSize != GEX_CHARTSIZE_SMALL)
                sprintf(szString,"%s (%ld)",szXscaleLabel,lTotalSublots);
            else
                *szString = 0;	// No legend in X
            // Check if we have a custom legend to overwrite default one
            if(pChartsInfo && pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.bLegendX)
                m_pChartDirector->xyChart()->xAxis()->setTitle(pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.strAxisLegendX.toLatin1().constData());
            else
                m_pChartDirector->xyChart()->xAxis()->setTitle(szString);

            // compute a string <value> units...just to extract the units!
            if(iChartSize != GEX_CHARTSIZE_SMALL)
            {
                QString strUnits = ptScaleTestCellReference->GetScaledUnits(&fCustomScaleFactor, scaling);

                // compute a string <value> units...just to extract the units!
                switch(iAggregateInfo)
                {
                    case GEX_ADV_TREND_AGGREGATE_MEAN:
                    default:
                        strLabel = "Mean trend ";
                        if(strUnits.length() > 0)
                            strLabel += "(" + strUnits + ")";
                        break;

                    case GEX_ADV_TREND_AGGREGATE_SIGMA:
                        strLabel = "Sigma trend ";
                        if(strUnits.length() > 0)
                            strLabel += "(" + strUnits + ")";
                        break;

                    case GEX_ADV_TREND_AGGREGATE_CP:
                        strLabel = "Cp trend ";
                        break;

                    case GEX_ADV_TREND_AGGREGATE_CPK:
                        strLabel = "Cpk trend ";
                        break;
                }
                // Save Y scale label
                strcpy(szString,strLabel.toLatin1().constData());
            }
            else
              *szString = 0;	// No legend in Y
            // Check if we have a custom legend to overwrite default one
            if(pChartsInfo && pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.bLegendY)
                m_pChartDirector->xyChart()->yAxis()->setTitle(pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].cChartOptions.strAxisLegendY.toLatin1().constData());
            else
                m_pChartDirector->xyChart()->yAxis()->setTitle(szString);
        }

        // Add custom markers - Horizontal line
        PlotScriptingCustomMarkersEx(ptTestCell,pFile,cLayerColor,iGroup,fCustomScaleFactor,true,&lfChartBottom,&lfChartTop);

        // If custom viewport in Y, overwrite defaults window viewing
        if((ptTestReferenceCell->ptChartOptions != NULL) && (ptTestReferenceCell->ptChartOptions->customViewportY()))
        {
            lfChartBottom = ptTestReferenceCell->ptChartOptions->lowY();
            lfChartTop = ptTestReferenceCell->ptChartOptions->highY();
        }

        NextGroup:
        // Make sure ALL layers in the group#1 have been ploted before going to next group.
        if(pChartsInfo && (pChartsInfo->chartsList().count() == iLayerIndex))
        {
            ++itGroupList;

            pGroup = (itGroupList != getGroupsList().end()) ? (*itGroupList) : NULL;

            iGroup++;			// Keep track of group index processed.
            iLayerIndex = 0;	// Reset layer counter.
        }
        else if (pChartsInfo == NULL)
        {
            ++itGroupList;

            pGroup = (itGroupList != getGroupsList().end()) ? (*itGroupList) : NULL;

            iGroup++;	// Keep track of group index processed.
        }

    };	// Read trend of a given test in all groups so to stack all charts.

    // Define scales: if in Interactive charting, consider zoomin factor
    if(pChartsInfo != NULL)
    {
        double	lfOffsetX,lfOffsetY;

        if(pChartsInfo->lfZoomFactorX < 0)
        {
            // Reset all
            pChartsInfo->lfZoomFactorX = pChartsInfo->lfZoomFactorY = 1.0;
            lfOffsetX = 0.0;
            lfOffsetY = 0.0;
            pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowX = lStartID;
            pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighX = lEndID;
            pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowY = lfChartBottom;
            pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighY = lfChartTop;
        }
        else
        {
            // Keep viewport data
            lStartID = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowX;
            lEndID = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighX;
            lfChartBottom = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowY;
            lfChartTop = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighY;
            // Clear Viewport offset values.
            lfOffsetX = 0.0;
            lfOffsetY = 0.0;
        }// Zoom or Drag action.

        // Update Viewport coordinates (or reset if zoom=1.0)
        lStartID = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowX = lStartID - lfOffsetX;
        lEndID = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighX = lEndID - lfOffsetX;
        lfChartBottom = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfLowY = lfChartBottom - lfOffsetY;
        lfChartTop = pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].lfHighY = lfChartTop - lfOffsetY;
    }

    // Y axis scale. Add 10% of scale over and under chart window...
    // to ensure markers will be seen
    lfExtra = (lfChartTop-lfChartBottom)*0.10;
    m_pChartDirector->xyChart()->yAxis()->setLinearScale(lfChartBottom-lfExtra,lfChartTop+lfExtra);

    // X axis scale.
    m_pChartDirector->xyChart()->xAxis()->setLinearScale(lStartID-1,lEndID-0.5);

    if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Plot vertical Lots# markers if any.
        plotLotMarkersEx(ptTestCell, m_pReportOptions, true);

        QString strImage =
            BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                 "/images/adv_t_", ptTestReferenceCell);

        // Save file into .PNG file! (unless we are ploting chart in a dialog box)
        if(pChartsInfo == NULL)
        {
            sprintf(szString,"%s/images/%s",m_pReportOptions->strReportDirectory.toLatin1().constData(), strImage.toLatin1().constData());

            // Save image to disk (256 colors palette)
            m_pChartDirector->drawChart(szString, GetImageCopyrightString());
        }

        WriteHistoHtmlTestInfo(pChartsInfo,"adv_trend",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX);

        if(pChartsInfo == NULL)
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"></font></p>\n");
        else
        {
            // Interactive Chart: Close Info file created in $home (or Windows) folder
            CloseReportFile();	// Close report file
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Trend report into an image (.png file).
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateAdvTrendChartImageEx(CGexChartOverlays *pChartsInfo,CTest *ptTestReferenceCell,int iChartSize, QString strImage,bool bPluginCall)
{
    GexTrendChart trendChart(bPluginCall, iChartSize, 0, pChartsInfo);

    trendChart.setViewportModeFromChartMode(m_pReportOptions->getAdvancedReportSettings());
    trendChart.computeData(m_pReportOptions, ptTestReferenceCell);
    trendChart.buildChart();
    trendChart.drawChart(strImage, GetImageCopyrightString());
}

/////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Trend report page.
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvTrendChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int iChartSize,bool bPluginCall)
{
    int	iSizeX=0;

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Check if need to create new HTML Trend. page
        if(!bPluginCall)
            CheckForNewHtmlPage(pChartsInfo, SECTION_ADV_TREND, ptTestReferenceCell->iHtmlAdvancedPage);
    }

    // Image size to create for the trend chart
    switch(iChartSize)
    {
        case GEX_CHARTSIZE_SMALL:
            // In SMALL chart mode, no scale shown in X
            iSizeX = GEX_CHARTSIZE_SMALL_X;
            break;
        case GEX_CHARTSIZE_MEDIUM:
            // Shortest label possible
            iSizeX = GEX_CHARTSIZE_MEDIUM_X;
            break;
        case GEX_CHARTSIZE_LARGE:
            iSizeX = GEX_CHARTSIZE_LARGE_X;
            break;
        case GEX_CHARTSIZE_BANNER:
            iSizeX = GEX_CHARTSIZE_BANNER_X;
            break;
    }

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if(pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = pChartsInfo->chartsList().first()->iGroupX;

    QString strImage =
        BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                             "/images/adv_t_", ptTestReferenceCell, iGroup);
    QString strImagePath = m_pReportOptions->strReportDirectory;
    strImagePath += "/images/";
    strImagePath += strImage;

    // Create Chart (paint into Widget)
    CreateAdvTrendChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,strImagePath,bPluginCall);

    // Write statistics & close HTML page.
    if(m_pReportOptions->isReportOutputHtmlBased()) //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_trend",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX);

        if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
        {
            // Writing Trend chart to disk
            fprintf(hReportFile,"<br></font></p>\n");
        }
        else
        {
            // Interactive Chart: Close Info file created in $home (or Windows) folder
            CloseReportFile();	// Close report file
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Add plot markers for each lot (if multi files merge)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::plotLotMarkersEx(CTest *ptTestCell, CReportOptions * pOptions, bool bAggregateData)
{
    if (ptTestCell == NULL)
        return;

    if((ptTestCell->ptChartOptions != NULL) && (ptTestCell->ptChartOptions->lotMarker() == false))
        return;	// Do not show Lots markers.

    CGexGroupOfFiles *pGroup;
    CGexFileInGroup *pFile;
    double			lfPosition,lfPreviousPosition=-1;
    char			szString[200];
    long			lDataOffset=0;
    int				iMarkerIndex=0;
    QString			strCurrentLot;
    int				nMarkerColor = QColor(Qt::blue).rgb() & 0xffffff;
    Mark *			pPreviousMarker = NULL;

    // OPTIONS
    QString strAdvTrendMarkerOptions = (pOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));

    if(pOptions->iGroups == 1)
    {
        // If a single group with multiple files, show vertical delimiters for each file (lot)
        pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while (itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            if(bAggregateData)
                lfPosition = iMarkerIndex+0.5;
            else
            {
                // If marker defines a samples count#, then show marker between last data of previous lot and next data of new lot
                lfPosition = lDataOffset;
                lfPosition -= 0.5;
            }

            // Show verticale marker at position X (only one per Lot)
            if( (qslAdvTrendMarkerOptionList.contains(QString("lot"))) && (strCurrentLot != pFile->getMirDatas().szLot))
            {
                // Write lotID with a 90 Deg. angle
                strCurrentLot = pFile->getMirDatas().szLot;
                sprintf(szString,"Lot: %s",pFile->getMirDatas().szLot);
                // Aggregate data: only one point per sub-lot
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->xAxis(), lfPosition, nMarkerColor, szString, 1, Chart::TopRight);

                // If previous marker inserted is at same position as new one, then remove previous maker!
                if(lfPreviousPosition == lfPosition && pPreviousMarker)
                    pPreviousMarker->setMarkColor(0xFFFFFFFF); // Transparent, marker won't appear

                // Update marker status
                lfPreviousPosition = lfPosition;
            }

            // If display marker at each sub-lot (file)
            if(qslAdvTrendMarkerOptionList.contains(QString("sublot")))
            {
                // If valid sublot & wafer ID, or Only WaferID: show WaferID
                if((*pFile->getMirDatas().szSubLot && *pFile->getWaferMapData().szWaferID) || *pFile->getWaferMapData().szWaferID)
                {
                    if(pGroup->pFilesList.count() <= 10)
                        sprintf(szString,"Waf:%s",pFile->getWaferMapData().szWaferID);
                    else
                    if(pGroup->pFilesList.count() <= 26)
                        sprintf(szString,"W:%s",pFile->getWaferMapData().szWaferID);
                    else
                        sprintf(szString,"%s",pFile->getWaferMapData().szWaferID);	// Shaortest strinng possible!
                }
                else
                if(*pFile->getMirDatas().szSubLot)
                    sprintf(szString,"SubLot: %s",pFile->getMirDatas().szSubLot);

                // Write sub-lots with a -20 Deg. angle...
                Mark * pMarker = m_pChartDirector->addMarker(m_pChartDirector->xyChart()->xAxis(), lfPosition, nMarkerColor, szString, 1, Chart::BottomRight);
                pMarker->setFontAngle(20);
            }

            // If display marker for each file (dataset name)
            if( (qslAdvTrendMarkerOptionList.contains(QString("group_name"))) && (pFile->strDatasetName.isEmpty() == false))
                m_pChartDirector->addMarker(m_pChartDirector->xyChart()->xAxis(), lfPosition, nMarkerColor, pFile->strDatasetName, 1, Chart::BottomRight);

            // Compute offset position to next data array (next file in group)...
            if(ptTestCell!= NULL)
            {
                // Find test cell: RESET list to ensure we scan list of the right group!
                pFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTestCell,false,false, ptTestCell->strTestName.toLatin1().data());
                if(ptTestCell->ldSamplesValidExecs > 0)
                    lDataOffset += ptTestCell->pSamplesInSublots[iMarkerIndex];
            }
            else
                lDataOffset = pFile->getMirDatas().lFirstPartResult;	// Using PartID as the offset.

            // Keep track of marker# plotted
            iMarkerIndex++;
        };
    }
}

QString	CGexReport::WriteAdvTrend(CReportOptions* /*ro*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" iAdvancedReport = GEX_ADV_TREND AdvTestList = %1 tests ")
          .arg( mAdvancedTestsListToReport.size()).toLatin1().constData());

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	m_pReportOptions->GetOption("adv_trend","chart_size").toString(),
                                   m_pReportOptions->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";

    int	iChartNumber=0;	// Keeps track of the number of chart inserted (so to insert appropriate page breaks)

    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    CTest	*ptTestCell = NULL;	//pGroup->cMergedData.ptMergedTestList;
    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();
    int i=0;

    while(it!=mAdvancedTestsListToReport.end())	//(ptTestCell != NULL)
    {
        ptTestCell=*it;
        if (!ptTestCell)
        {
            it++;
            continue;
        }

        // If no samples available...no chart!
        // CheckMe : we should already have checked that there are samples in PrepareSection
        // Do we have to trust bTestInList ?
        // if so, it does not work because strangely, bTestInList is always true
        //if(ptTestCell->bTestInList)
        {
            i++;
            // ******** Check for user 'Abort' signal
            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
               return "abort";//return GS::StdLib::Stdf::NoError;

            // If trend of Test difference Test1-Test2, add one more trend chart: Test1-Test2
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                default:
                    WriteAdvTrendChartPage(NULL,pFile,ptTestCell,iChartSize);
                    break;
                case GEX_ADV_TREND_AGGREGATE_MEAN:
                case GEX_ADV_TREND_AGGREGATE_SIGMA:
                case GEX_ADV_TREND_AGGREGATE_CP:
                case GEX_ADV_TREND_AGGREGATE_CPK:
                    WriteAdvTrendChartPageAggregateEx(NULL,pFile,ptTestCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings());
                    break;
            }

            // Update chart count
            iChartNumber++;

            // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
            switch(iChartSize)
            {
                case GEX_CHARTSIZE_MEDIUM:
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Trend",iChartNumber,2,ptTestCell);

                    if((iChartNumber % 2) == 0)
                        WritePageBreak();
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name. Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Trend",1,1,ptTestCell);
                    WritePageBreak();
                    break;

                case GEX_CHARTSIZE_BANNER:
                    // Dynamically build the PowerPoint slide name (as name includes the 5 (or 4) tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Trend",iChartNumber,4,ptTestCell);

                    if((iChartNumber % 4) == 0)
                        WritePageBreak();
                    break;
            }
        }

        // Point to next test cell
        //ptTestCell = ptTestCell->ptNextTest;
        it++;
    };	// Loop until all test cells read.

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 tests written").arg( i).toLatin1().constData());

    // If trend of Test difference Test1-Test2, add one more trend chart: Test1-Test2
    switch(m_pReportOptions->getAdvancedReportSettings())
    {
        case GEX_ADV_TREND_DIFFERENCE:
            // Have the chart for TestX-TestY created+appended to existing charts.
            WriteAdvTrendDifferenceChartPageEx(NULL, iChartSize);
            break;
        case GEX_ADV_TREND_SOFTBIN_SBLOTS:
        case GEX_ADV_TREND_SOFTBIN_PARTS:
        case GEX_ADV_TREND_HARDBIN_SBLOTS:
        case GEX_ADV_TREND_HARDBIN_PARTS:
        case GEX_ADV_TREND_SOFTBIN_ROLLING:
        case GEX_ADV_TREND_HARDBIN_ROLLING:
            // SoftBin or HardBin trend...
            WriteAdvTrendBinningChartPageEx(NULL, iChartSize);
            break;
    }

    return "ok";
}

int CGexReport::removeDataPointsScatter(const QList<QRectF> &rAreaToRemove, bool bForceStatsUpdate, bool bRemoveAssociatedParts){

    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);

    CGexChartOverlays *poChartsInfo = m_pChartsInfo;
    CGexSingleChart *pChart = 0;
    //FIXME: not used ?
    //QList<CGexGroupOfFiles*>::iterator	itGroupList = getGroupsList().begin();
    //CGexGroupOfFiles* poGroup =
    //  (itGroupList != getGroupsList().end()) ? (*itGroupList) : NULL;
    //poGroup = 0;

    CTest *	ptTestCellX					= NULL;
    CTest *	ptTestCellY					= NULL;
    CGexFileInGroup *	pFileX	= NULL;
    CGexFileInGroup *	pFileY	= NULL;
    CGexGroupOfFiles *	pGroupX	= NULL;
    CGexGroupOfFiles *	pGroupY	= NULL;
    int		lTestNumberX;
    int		lPinmapIndexX;
    int		lTestNumberY;
    int		lPinmapIndexY;
    //FIXME: not used ?
    //int iOutlierCount=0;
    QList <long> cPartListX, cPartListY;
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;

    for(int iLayerIndex = 0; iLayerIndex < poChartsInfo->chartsList().count(); iLayerIndex++){

        pChart = poChartsInfo->chartsList().at(iLayerIndex);

        pGroupX = getGroupsList().at(pChart->iGroupX);
        pFileX =  pGroupX->pFilesList.first();
        lPinmapIndexX		= pChart->iPinMapX;
        lTestNumberX		= pChart->iTestNumberX;

        pGroupY = getGroupsList().at(pChart->iGroupY);
        pFileY	= pGroupY->pFilesList.first();
        lPinmapIndexY		= pChart->iPinMapY;
        lTestNumberY		= pChart->iTestNumberY;

        //FIXME: not used ?
        //iOutlierCount = 0;

        if(pFileX->FindTestCell(lTestNumberX,lPinmapIndexX,&ptTestCellX,false,false,
                                pChart->strTestNameX.toLatin1().data())!=1)
                return 0;
        if(pFileY->FindTestCell(lTestNumberY,lPinmapIndexY,&ptTestCellY,false,false,
                                pChart->strTestNameY.toLatin1().data())!=1)
                return 0;

        int iTotalSamples = gex_min(ptTestCellX->m_testResult.count(),ptTestCellY->m_testResult.count());
        double lfTotalX=0,lfTotalY=0;
        double lfTotalSquareX=0,lfTotalSquareY=0;
        double lfMinX = C_INFINITE,lfMinY = C_INFINITE;
        double lfMaxX = -C_INFINITE,lfMaxY = -C_INFINITE;
        int iOutlierCount=0;

        cPartListX.clear();
        cPartListY.clear();
        TestRemoveResultCommand::TestRemovedResult *poRemovedResultX = new TestRemoveResultCommand::TestRemovedResult(ptTestCellX, pGroupX, bRemoveAssociatedParts);
        TestRemoveResultCommand::TestRemovedResult *poRemovedResultY = new TestRemoveResultCommand::TestRemovedResult(ptTestCellY, pGroupY, bRemoveAssociatedParts);
        oRunIdxRemoved.append(poRemovedResultX);
        oRunIdxRemoved.append(poRemovedResultY);

        for(int iIdx =0; iIdx<iTotalSamples; iIdx++){

            if(!ptTestCellX->m_testResult.isValidResultAt(iIdx) || !ptTestCellY->m_testResult.isValidResultAt(iIdx))
                continue;
            double dX = ptTestCellX->m_testResult.resultAt(iIdx);
            double dY = ptTestCellY->m_testResult.resultAt(iIdx);

            lfTotalX += dX;
            lfTotalSquareX += (dX*dX);
            lfMinX = gex_min(lfMinX,dX);
            lfMaxX = gex_max(lfMaxX,dX);

            lfTotalY += dY;
            lfTotalSquareY += (dY*dY);
            lfMinY = gex_min(lfMinY,dY);
            lfMaxY = gex_max(lfMaxY,dY);

            for(int iRect = 0; iRect<rAreaToRemove.count(); iRect++){
                if(rAreaToRemove.at(iRect).contains(dX ,dY)){
                    ptTestCellX->m_testResult.invalidateResultAt(iIdx);
                    poRemovedResultX->addRemovedIdx(iIdx);
                    ptTestCellY->m_testResult.invalidateResultAt(iIdx);
                    poRemovedResultY->addRemovedIdx(iIdx);
                    //FIXME: not used ?
                    //iOutlierCount++;
                    ptTestCellX->ldSamplesValidExecs--;
                    ptTestCellY->ldSamplesValidExecs--;
                    cPartListX.append(iIdx);
                    cPartListY.append(iIdx);
                    break;
                }
            }
        }

        // All data points filtered, update statistics
        ptTestCellX->lfSamplesMin = lfMinX;
        ptTestCellX->lfSamplesMax = lfMaxX;
        ptTestCellX->lfSamplesTotal = lfTotalX;
        ptTestCellX->lfSamplesTotalSquare = lfTotalSquareX;
        ptTestCellX->GetCurrentLimitItem()->ldOutliers += iOutlierCount;

        // No longer keep summary info since data manually filtered.
        ptTestCellX->ldExecs = ptTestCellX->ldSamplesValidExecs;
        ptTestCellX->lfTotal = ptTestCellX->lfSamplesTotal;
        ptTestCellX->lfTotalSquare = ptTestCellX->lfSamplesTotalSquare;
        ptTestCellX->lfMax = lfMaxX;
        ptTestCellX->lfMin = lfMinX;

        // If dealing with multi-layers: Force all statistics to be computed again (Cp, Cpk, etc...)
        //TODO: update this part
        if(bForceStatsUpdate || m_pChartsInfo != NULL)
        {
                m_cStats.ComputeBasicTestStatistics(ptTestCellX,true);
                m_cStats.RebuildHistogramArray(ptTestCellX,GEX_HISTOGRAM_OVERDATA);
                QString pf=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();
                m_cStats.ComputeAdvancedDataStatistics(ptTestCellX,true,pf=="percentile"?true:false);
        }

        if (bRemoveAssociatedParts && pGroupX)
                pGroupX->removeParts(cPartListX);

        // All data points filtered, update statistics
        ptTestCellY->lfSamplesMin = lfMinY;
        ptTestCellY->lfSamplesMax = lfMaxY;
        ptTestCellY->lfSamplesTotal = lfTotalY;
        ptTestCellY->lfSamplesTotalSquare = lfTotalSquareY;
        ptTestCellY->GetCurrentLimitItem()->ldOutliers += iOutlierCount;

        // No longer keep summary info since data manually filtered.
        ptTestCellY->ldExecs = ptTestCellY->ldSamplesValidExecs;
        ptTestCellY->lfTotal = ptTestCellY->lfSamplesTotal;
        ptTestCellY->lfTotalSquare = ptTestCellY->lfSamplesTotalSquare;
        ptTestCellY->lfMax = lfMaxY;
        ptTestCellY->lfMin = lfMinY;

        // If dealing with multi-layers: Force all statistics to be computed again (Cp, Cpk, etc...)
        //TODO: update this part
        if(bForceStatsUpdate || m_pChartsInfo != NULL)
        {
                m_cStats.ComputeBasicTestStatistics(ptTestCellY,true);
                m_cStats.RebuildHistogramArray(ptTestCellY,GEX_HISTOGRAM_OVERDATA);
                QString pf=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();
                m_cStats.ComputeAdvancedDataStatistics(ptTestCellY,true,pf=="percentile"?true:false);
        }

        if (bRemoveAssociatedParts && pGroupY)
            pGroupY->removeParts(cPartListY);

    }
    if(bEnableUndoRedo && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
        pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand(QString("Removing values from scater "),oRunIdxRemoved, 0));
    }
    if(!bEnableUndoRedo){
        qDeleteAll(oRunIdxRemoved);
        oRunIdxRemoved.clear();
    }
return 1;
}


