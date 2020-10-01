/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Scatter' Correlation page.
/////////////////////////////////////////////////////////////////////////////

#include <gqtl_log.h>
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "interactive_charts.h"		// Layer classes, etc
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gexscatterchart.h"
#include "gex_box_whisker_chart.h"
#include "engine.h"

// main.cpp
extern int ExtractTestRange(const char *szValue,unsigned long *lFromTest,long *lFromPinmapIndex,unsigned long *lToTest,long *lToPinmapIndex);

// cstats.cpp
extern double			ScalingPower(int iPower);
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvScatter(BOOL /*bValidSection*/)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'scatter' page & header
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
    if (m_pReportOptions->isReportOutputHtmlBased())
       //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating HTML report file.

        // Open advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError) return GS::StdLib::Stdf::ReportFile;

        // Most of functions keep writing to the 'hReportFile' file.
        hReportFile = hAdvancedReport;

        // Will be used while creating all scatter pages
        mTotalHtmlPages = mTotalAdvancedPages;

    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvScatter(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        if(hAdvancedReport == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting scatter data!
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
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void
CGexReport::
WriteScatterHtmlTestInfo(CGexChartOverlays* pChartsInfo,
                         CGexGroupOfFiles* /*pGroupX*/,
                         CGexFileInGroup* /*pFileX*/,
                         CTest* ptTestCellX,
                         CGexGroupOfFiles* /*pGroupY*/,
                         CGexFileInGroup* /*pFileY*/,
                         CTest* ptTestCellY)
{
    CGexGroupOfFiles *	pGroup	= NULL;
    CGexFileInGroup *	pFile	= NULL;
    CTest *				pTestX	= NULL;
    CTest *				pTestY	= NULL;

    double	lfPearson;
    QString	strBookmark;

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // Check if charting multi-layers chart...
    bool bMyReportMultiLayer;
    if(pChartsInfo && pChartsInfo->chartsList().count() > 1)
        bMyReportMultiLayer = true;
    else
        bMyReportMultiLayer = false;

    // HTML code to open table, Width = 98% cell spacing=1
    WriteHtmlOpenTable(98,2);

    // Generating .HTML report file.
    // Test #
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s><b>Fields</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><b>Test in X</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><b>Test in Y</b></td>\n",szFieldColor);
    fprintf(hReportFile,"</tr>\n");

    // If 'MyReport' multi-layers, only list test names & colors
    if(bMyReportMultiLayer)
    {
        int		i;
        char	szString[15];
        CGexSingleChart *pChart;
        QColor cColor;

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Test#</td>\n",szFieldColor);

        // Tests in X
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>",szDataColor);
        for(i=0;i< (int)pChartsInfo->chartsList().count();i++)
        {
            pChart = pChartsInfo->chartsList().at(i);
            cColor = pChart->cColor;
            sprintf(szString,"#%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());

            // Html color string.
            fprintf(hReportFile,"<font color=\"%s\"><b>%s</b></font><br>",szString,pChart->strTestNameX.toLatin1().constData());
        }
        fprintf(hReportFile,"</td>\n");

        // Tests in Y
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>",szDataColor);
        for(i=0;i< (int)pChartsInfo->chartsList().count();i++)
        {
            pChart = pChartsInfo->chartsList().at(i);
            cColor = pChart->cColor;
            sprintf(szString,"#%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());

            // Html color string.
            fprintf(hReportFile,"<font color=\"%s\"><b>%s</b></font><br>",szString,pChart->strTestNameY.toLatin1().constData());
        }
        fprintf(hReportFile,"</td>\n");

        fprintf(hReportFile,"</tr>\n");

        // End of HTML table.
        goto end_of_table;
    }

    // Test#
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Test#</td>\n",szFieldColor);
    // Bookmark: are in same page if FLAT HTML page is generated
    if((ptTestCellX != NULL)
        &&(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        )
        strBookmark.sprintf("stats%d.htm#StatT",ptTestCellX->iHtmlStatsPage);
    else
        strBookmark = "#StatT";	// Test Statistics bookmark header string.
    if(ptTestCellX != NULL)
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></td>\n",szDataColor,ptTestCellX->szTestLabel,strBookmark.toLatin1().constData(), ptTestCellX->szTestLabel,ptTestCellX->szTestLabel);
    else
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>**ERROR** Test doesn't exist!</td>\n",szDataColor);

    // Bookmark: are in same page if FLAT HTML page is generated
    if((ptTestCellY != NULL)
        && (of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        )
        strBookmark.sprintf("stats%d.htm#StatT",ptTestCellY->iHtmlStatsPage);
    else
        strBookmark = "#StatT";	// Test Statistics bookmark header string.
    if(ptTestCellY != NULL)
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></td>\n",szDataColor,ptTestCellY->szTestLabel,strBookmark.toLatin1().constData(), ptTestCellY->szTestLabel,ptTestCellY->szTestLabel);
    else
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>**ERROR** Test doesn't exist!</td>\n",szDataColor);
    fprintf(hReportFile,"</tr>\n");
    // Test name
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Name</td>\n",szFieldColor);
    if(ptTestCellX != NULL)
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,buildDisplayName(ptTestCellX).toLatin1().constData());
    else
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
    if(ptTestCellY != NULL)
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,buildDisplayName(ptTestCellY).toLatin1().constData());
    else
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
    fprintf(hReportFile,"</tr>\n");

    // Write informations for each group
    for (int nGroupID = 0; nGroupID < getGroupsList().count(); nGroupID++)
    {
        if((nGroupID % 2 )== 0 && nGroupID != 0 )
        {
              QString lRes=WritePowerPointSlide(hReportFile,"",true);
              if (lRes.startsWith("err"))
                  GSLOG(3, lRes.toLatin1().data());
        }

        pGroup	= getGroupsList().at(nGroupID);
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if (pFile->FindTestCell(ptTestCellX->lTestNumber, ptTestCellX->lPinmapIndex, &pTestX, true, false, ptTestCellX->strTestName.toLatin1().data()) != 1)
            continue;

        if (pFile->FindTestCell(ptTestCellY->lTestNumber, ptTestCellY->lPinmapIndex, &pTestY, true, false, ptTestCellY->strTestName.toLatin1().data()) != 1)
            continue;

        // Show Group name (if multiple groups)
        if(m_pReportOptions->iGroups > 1)
        {
            CGexSingleChart *	pLayerStyle = NULL;
            QString				strGlobalInfoBookmark;
            QColor				cLayerColor;
            QString				strColor;

            // Hyperlink to Global info page.
            if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
                strGlobalInfoBookmark = "global.htm";
            else
                strGlobalInfoBookmark = "#all_globalinfo";

            if (nGroupID < m_pReportOptions->pLayersStyleList.count())
                pLayerStyle = m_pReportOptions->pLayersStyleList.at(nGroupID);

            // Non-interactive chart: either use default color, or custom color if defined under intercative mode
            if(pLayerStyle == NULL)
                cLayerColor = GetChartingColor(nGroupID);
            else
                // Use interactive color
                cLayerColor = pLayerStyle->cColor;

            strColor.sprintf("#%02x%02x%02x",cLayerColor.red(),cLayerColor.green(),cLayerColor.blue());

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"20%%\" bgcolor=%s><b>Group name</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"6%%\" bgcolor=%s><font color =%s>Color</font></td>\n",strColor.toLatin1().constData(), strColor.toLatin1().constData());

            if(pTestX != NULL)
                fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><a href=\"%s\">%s</a></td>\n",szDataColor,strGlobalInfoBookmark.toLatin1().constData(),pGroup->strGroupName.toLatin1().constData());
            else
                fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);

            if(pTestY != NULL)
                fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s><a href=\"%s\">%s</a></td>\n",szDataColor,strGlobalInfoBookmark.toLatin1().constData(),pGroup->strGroupName.toLatin1().constData());
            else
                fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");
        }

        // Low L
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Low L.</td>\n",szFieldColor);
        if(pTestX != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,pTestX->GetCurrentLimitItem()->szLowL);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        if(pTestY != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,pTestY->GetCurrentLimitItem()->szLowL);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
        // High L
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>High L.</td>\n",szFieldColor);
        if(pTestX != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,pTestX->GetCurrentLimitItem()->szHighL);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        if(pTestY != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,pTestY->GetCurrentLimitItem()->szHighL);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
        // Mean (Summary)
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Mean  /  [Min., Max.] from summary</td>\n",szFieldColor);
        if(pTestX != NULL)
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s   /   [ ",szDataColor,
                pFile->FormatTestResult(pTestX, pTestX->lfMean, pTestX->res_scal));
            fprintf(hReportFile,"%s,  ",
                pFile->FormatTestResult(pTestX, pTestX->lfMin, pTestX->res_scal));
            fprintf(hReportFile,"%s ]</td>\n",
                pFile->FormatTestResult(pTestX, pTestX->lfMax, pTestX->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }

        if(pTestY != NULL)
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s   /   [ ",szDataColor,
                pFile->FormatTestResult(pTestY,pTestY->lfMean,pTestY->res_scal));
            fprintf(hReportFile,"%s,  ",
                pFile->FormatTestResult(pTestY,pTestY->lfMin,pTestY->res_scal));
            fprintf(hReportFile,"%s ]</td>\n",
                pFile->FormatTestResult(pTestY,pTestY->lfMax,pTestY->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        fprintf(hReportFile,"</tr>\n");

        // Mean (samples)
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Mean  /  [Min., Max.] from samples</td>\n",szFieldColor);
        if(pTestX != NULL)
        {
            float fMeanX = pTestX->lfSamplesTotal/pTestX->ldSamplesValidExecs;
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s   /   [ ",szDataColor,
                pFile->FormatTestResult(pTestX,fMeanX,pTestX->res_scal));
            fprintf(hReportFile,"%s,  ",
                pFile->FormatTestResult(pTestX,pTestX->lfSamplesMin,pTestX->res_scal));
            fprintf(hReportFile,"%s ]</td>\n",
                pFile->FormatTestResult(pTestX,pTestX->lfSamplesMax,pTestX->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        if(pTestY != NULL)
        {
            float fMeanY = pTestY->lfSamplesTotal/pTestY->ldSamplesValidExecs;
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s   /   [ ",szDataColor,
                pFile->FormatTestResult(pTestY,fMeanY,pTestY->res_scal));
            fprintf(hReportFile,"%s,  ",
                pFile->FormatTestResult(pTestY,pTestY->lfSamplesMin,pTestY->res_scal));
            fprintf(hReportFile,"%s ]</td>\n",
                pFile->FormatTestResult(pTestY,pTestY->lfSamplesMax,pTestY->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        fprintf(hReportFile,"</tr>\n");

        // Median (if exists)
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Median</td>\n",szFieldColor);
        if(pTestX != NULL && (pTestX->lfSamplesQuartile2 != -C_INFINITE))
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,
                pFile->FormatTestResult(pTestX,pTestX->lfSamplesQuartile2,pTestX->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        if(pTestY != NULL && (pTestY->lfSamplesQuartile2 != -C_INFINITE))
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,
                pFile->FormatTestResult(pTestY,pTestY->lfSamplesQuartile2,pTestY->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        fprintf(hReportFile,"</tr>\n");

        // Samples
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Samples</td>\n",szFieldColor);
        if(pTestX != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%d</td>\n",szDataColor,pTestX->ldSamplesValidExecs);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        if(pTestY != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%d</td>\n",szDataColor,pTestY->ldSamplesValidExecs);
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
        // Sigma
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Sigma / Range</td>\n",szFieldColor);
        if(pTestX != NULL)
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s / ",szDataColor,
                pFile->FormatTestResult(pTestX,pTestX->lfSigma,pTestX->res_scal));
            fprintf(hReportFile,"%s</td>\n",
                pFile->FormatTestResult(pTestX,pTestX->lfRange,pTestX->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        if(pTestY != NULL)
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s / ",szDataColor,
                pFile->FormatTestResult(pTestY,pTestY->lfSigma,pTestY->res_scal));
            fprintf(hReportFile,"%s</td>\n",
                pFile->FormatTestResult(pTestY,pTestY->lfRange,pTestY->res_scal));
        }
        else
        {
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        }
        fprintf(hReportFile,"</tr>\n");
        // Cp
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Cp</td>\n",szFieldColor);
        if(pTestX != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,CreateResultStringCpCrCpk(pTestX->GetCurrentLimitItem()->lfCp));
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        if(pTestY != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,CreateResultStringCpCrCpk(pTestY->GetCurrentLimitItem()->lfCp));
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
        // Cpk
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Cpk</td>\n",szFieldColor);
        if(pTestX != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,CreateResultStringCpCrCpk(pTestX->GetCurrentLimitItem()->lfCpk));
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        if(pTestY != NULL)
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%s</td>\n",szDataColor,CreateResultStringCpCrCpk(pTestY->GetCurrentLimitItem()->lfCpk));
        else
            fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>n/a</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");

        // Write Pearson's correlation coefficient.
        int count=0;
        lfPearson = ComputePearsonValue(pTestX,pTestY, count);
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Correlation coeff. (r)</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%.6lf</td>\n",szDataColor,lfPearson);
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>&nbsp;</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td colspan=2 width=\"26%%\" bgcolor=%s>Determination coeff. (r^2)</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>%.6lf</td>\n",szDataColor,lfPearson*lfPearson);
        fprintf(hReportFile,"<td width=\"37%%\" bgcolor=%s>&nbsp;</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
    }

end_of_table:
    fprintf(hReportFile,"</table>\n");

    // Keep count of total HTML pages to create (used to compute the Progress Gauge size)
    m_pReportOptions->lAdvancedHtmlPages = 1;	// Only one Scatter report allowed!

    // If database SUMMARY used instead of data samples, this could be why we didn't find data!
    if(m_pReportOptions->bSpeedUseSummaryDB && (ptTestCellX == NULL) && (ptTestCellY == NULL))
        WriteInfoLine("Possible cause:", "Only Database SUMMARY records were used (see 'Options' tab, section 'Speed optimization')");
}

////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Scatter chart into an image (.png file).
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateAdvScatterChartImageEx(CGexChartOverlays *pChartsInfo,
    CTest *ptTestReferenceCellX, CTest *ptTestReferenceCellY, int iChartSize,QString strImage)
{
    GexScatterChart scatterChart(iChartSize, 0, pChartsInfo);
    scatterChart.setViewportModeFromChartMode(m_pReportOptions->getAdvancedReportSettings());
    scatterChart.computeData(m_pReportOptions, ptTestReferenceCellX, ptTestReferenceCellY);
    scatterChart.buildChart();
    scatterChart.drawChart(strImage, GetImageCopyrightString());
}

////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Correlation Box whisker chart into an image (.png file).
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateAdvCorrelationBoxWhiskerChartImageEx(CGexChartOverlays *pChartsInfo,
    CTest *ptTestReferenceCellX, CTest *ptTestReferenceCellY, int iChartSize,QString strImage)
{
    GexBoxWhiskerChart boxWhiskerChart(iChartSize, 0, pChartsInfo);
    boxWhiskerChart.setViewportModeFromChartMode(m_pReportOptions->getAdvancedReportSettings());
    boxWhiskerChart.computeData(m_pReportOptions, ptTestReferenceCellX, ptTestReferenceCellY);
    boxWhiskerChart.buildChart();
    boxWhiskerChart.drawChart(strImage, GetImageCopyrightString());
}

/////////////////////////////////////////////////////////////////////////////
// Writes one Advanced Scatter report page.
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvScatterChartPage(CGexChartOverlays *pChartsInfo,CGexGroupOfFiles *pGroupX,CGexFileInGroup *pFileX,CTest *ptTestCellX,CGexGroupOfFiles *pGroupY,CGexFileInGroup *pFileY,CTest *ptTestCellY,int iChartNumber,int iChartSize)
{
    int	iSizeX=0,iSizeY=0;
    switch(iChartSize)
    {
        case GEX_CHARTSIZE_SMALL:
            // In SMALL chart mode, no scale shown
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

    // Build Image full path where to save the chart plot.
    QString strImage = "adv_s_" + QString::number(iChartNumber) + ".png";	// image to create: 'adv_s_XXXX.png'
    QString strImagePath = m_pReportOptions->strReportDirectory;
    strImagePath += "/images/";
    strImagePath += strImage;

    // If first page, write section header
    if(iChartNumber == 0)
    {
        // HTML code.
        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"all_advanced","More Reports: Correlation (Scatter / BiVariate chart)");
    }

    // Write statistics
    WriteScatterHtmlTestInfo(pChartsInfo, pGroupX, pFileX, ptTestCellX, pGroupY, pFileY, ptTestCellY);

    // Create Chart (paint into Widget)
    CreateAdvScatterChartImageEx(pChartsInfo,ptTestCellX,ptTestCellY,iChartSize, strImagePath);

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    // If image created, write HTML code to show it.
    if(pChartsInfo == NULL || m_pChartsInfo->bFileOutput)
    {
        // Have the ToolBar line written in the HTML file
        QString strDrillArgument= "drill_chart=adv_scatter";
        strDrillArgument += "--data=";
        strDrillArgument += ptTestCellX->szTestLabel;	// X parameter
        strDrillArgument += "--data=";
        strDrillArgument += ptTestCellY->szTestLabel;	// Y parameter
        WriteHtmlToolBar(iSizeX,true,strDrillArgument);

        // If flat HTML, center the image as it is of MEDIUM size and add some little space between statistics & the image
        if( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" ) 	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            fprintf(hReportFile,"<p align=\"center\"><br>\n");

        // Write image
        fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(strImage).toLatin1().constData());

        if( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" ) 	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            fprintf(hReportFile,"</p>\n");
        else
            fprintf(hReportFile,"<br><br>\n");	// In standard HTML add some space between images as we don't have page breaks

        // line image separator
        fprintf(hReportFile,"<br></font></p>\n");
    }
    else
    {
        // Update the HTML Info page
        CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_SCATTER,0);

        // Write TestX and TestY info.
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_scatter",pFileX,ptTestCellX,NULL,NULL,true,"",iSizeX);
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_scatter",pFileX,ptTestCellY,NULL,NULL,true,"",iSizeY);

        // Close Info file created in $home (or Windows) folder
        CloseReportFile();	// Close report file
    }

}

QString			CGexReport::WriteAdvScatter(CReportOptions* ro)
{
    if (!ro)
        return "error";
    QString of=ro->GetOption("output", "format").toString();
    if (of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
      return "ok";

    CGexGroupOfFiles *pGroup=0,*pGroupY=0;
    CGexFileInGroup *pFile=0,*pFileY=0;

    // Get pointer to first group & first file (and 2d group if two goups or more are present)
    if(ro->iGroups == 1)
    {
      pGroup = pGroupY = getGroupsList().isEmpty()?NULL:getGroupsList().first();
      pFile  = pFileY = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    }
    else
    {
      pGroup	= (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(0);
      pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
      pGroupY = (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(1);
      pFileY  = (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();
    }

    // Extract all tests to scatter
    QString strString = ro->strAdvancedTestList;
    strString = strString.replace(',',' ');
    strString = strString.replace(';',' ');
    QStringList strTests = strString.split(" ", QString::SkipEmptyParts);

    // Only scatter a even number of tests (one in X, one in Y) x N plots
    int iTotalTests = strTests.count();
    if(iTotalTests % 2)
      iTotalTests--;

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(ro->GetOption("adv_correlation","chart_size").toString(),
                      iTotalTests); //GetChartImageSize(pReportOptions->iScatterChartSize,iTotalTests);

    unsigned long	lAdvancedTestX,lAdvancedTestY;
    long	lAdvancedTestPinmapIndexX,lAdvancedTestPinmapIndexY;
    CTest	*ptTestCellX=NULL;	// Pointer to test cell with data to chart in X
    CTest	*ptTestCellY=NULL;	// Pointer to test cell with data to chart in Y
    int	iChartNumber=0;	// Keeps track of the number of chart inserted (so to insert appropriate page breaks)

    for(int iIndex=0; iIndex < iTotalTests; )
    {
      // Get the Test X and Test Y IDs
      if(!ExtractTestRange(strTests[iIndex].toLatin1().constData(), &lAdvancedTestX, &lAdvancedTestPinmapIndexX, NULL, NULL))
        goto next_pair;

      if(!ExtractTestRange(strTests[iIndex+1].toLatin1().constData(),&lAdvancedTestY,&lAdvancedTestPinmapIndexY,NULL,NULL))
        goto next_pair;

      // Get handle to Test X and Test Y
      pFile->FindTestCell(lAdvancedTestX,lAdvancedTestPinmapIndexX,&ptTestCellX,false,false);

      // If One group only, then Test X and Test Y belong to the same group!
      if(m_pReportOptions->iGroups == 1)
        pFile->FindTestCell(lAdvancedTestY,lAdvancedTestPinmapIndexY,&ptTestCellY,false,false);
      else
        pFileY->FindTestCell(lAdvancedTestY,lAdvancedTestPinmapIndexY,&ptTestCellY,false,false);

      // Ignore functional tests
      // If not a parametric / multiparametric (eg: functional) test, ignore!
      if(ptTestCellX && ptTestCellX->bTestType == 'F')
        ptTestCellX = NULL;
      // If not a parametric / multiparametric (eg: functional) test, ignore!
      if(ptTestCellY && ptTestCellY->bTestType == 'F')
        ptTestCellY = NULL;

      // If found tests, create the plot!
      if(ptTestCellX && ptTestCellY)
      {
        // Dynamically build the PowerPoint slide name
        strString = "Scatter : ";
        strString += buildDisplayName(ptTestCellX);
        strString += " vs " + buildDisplayName(ptTestCellY);
        SetPowerPointSlideName(strString);

        // Draw chart into image file (handles page break)
        WriteAdvScatterChartPage(NULL,pGroup,pFile,ptTestCellX,pGroupY,pFileY,ptTestCellY,iChartNumber,iChartSize);

        // Insert page break if flat HTML (for Word, PPT or PDF file)
        if ( (of=="DOC")||(of=="PDF")||(of=="PPT")|| of=="ODT" ) //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
          WritePageBreak();

        // Update chart count
        iChartNumber++;
      }
next_pair:
      // Move to next test pair.
      iIndex += 2;
    }
    return "ok";
}
