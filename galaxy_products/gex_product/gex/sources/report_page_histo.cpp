/////////////////////////////////////////////////////////////////////////////
// Creates HTML Histogram page.
/////////////////////////////////////////////////////////////////////////////

#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "interactive_charts.h"		// Layer classes, etc
#include "patman_lib.h"				// List of '#define' to return type of distribution (gaussian, lognormal, etc...)
#include "gexperformancecounter.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include <gqtl_log.h>
#include "engine.h"
#include "csl/csl_engine.h"
//#include "pat_info.h"
#include "pat_engine.h"
#include <math.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

extern QString			formatHtmlImageFilename(const QString& strImageFileName);

int	CGexReport::PrepareSection_Histo(BOOL /*bValidSection*/)
{
    // Creates the Histogram page & header
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV")
    {
        if(m_pReportOptions->iHistogramType == GEX_HISTOGRAM_DISABLED)
            return GS::StdLib::Stdf::NoError;

        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Histograms ----");

        if(m_pReportOptions->iGroups == 1)
            fprintf(hReportFile,"\n\nHistogram,Cell1,Cell2,Cell3,Cell4,Cell5,Cell6,Cell7,Cell8,Cell9,Cell10,etc\n");
        else
            fprintf(hReportFile,"\n\nHistogram:\nWARNING, Correlation histograms only supported in HTML format.\n\n");
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // If this section has to be created, update steps in progress bar
        if((m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_HISTOGRAM) == 0)
        {
            // Let's update the progressbar size...
            int iTotalSteps = ((2*m_pReportOptions->iFiles)+6) + (mTotalHistogramPages*MAX_HISTOGRAM_PERPAGE) + mTotalWafermapPages + m_pReportOptions->lAdvancedHtmlPages;
            // Shows process bar...
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,iTotalSteps,-1);	// Show process bar...updates steps count, add one new step 1
        }

        // Will be used while creating all Statistic pages: only used if HTML pages created !
        mTotalHtmlPages = mTotalHistogramPages;
    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Histo(qtTestListStatistics	*pqtStatisticsList)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if	(strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        static char	szString[2048];

        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_STATISTICS) == true)
            && (strOutputFormat=="HTML")
            )
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }
        else
        if(hReportFile!=NULL)
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            if (strOutputFormat=="HTML")
                CloseReportFile();	// Close report file
        }

        // Create Test index page
        if (strOutputFormat=="HTML")
        {
            // Open <stdf-filename>/report/histogram.htm
            sprintf(szString,"%s/pages/histogram.htm", m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;
        }

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(strOutputFormat=="HTML")
        {
            // Create Test index page
            WriteTestListIndexPage(GEX_INDEXPAGE_HISTO,true,pqtStatisticsList);

            CloseReportFile();	// Close report file
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Checks if current Histogram column is PASS (green) or FAIL (red)
/////////////////////////////////////////////////////////////////////////////
char	*CGexReport::CheckColumnPassFail(CTest *ptTestCell,double fValueL,double fValueH)
{
    // This routine checks if the whole histogram is pass or fail. If part of the
    // histogram cell is in the fail, the whole bar will be red !

    // bit0=1 (no low limit), bit1=1 (no high limit)
    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL)
    {
        // No Low limit
        if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL)
          return GEX_T("p");	// No limits at all
        // High limit only
        if(fValueH <= ptTestCell->GetCurrentLimitItem()->lfHighLimit)
            return GEX_T("p");
        else
          return GEX_T("f");

    }
    else
    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL)
    {
        // No High limit, low limit only
        if(fValueL >= ptTestCell->GetCurrentLimitItem()->lfLowLimit)
            return GEX_T("p");
        else
            return GEX_T("f");
    }

    // Test has low AND high limits!
    if(fValueL < ptTestCell->GetCurrentLimitItem()->lfLowLimit)
        return GEX_T("f");
    if(fValueH > ptTestCell->GetCurrentLimitItem()->lfHighLimit)
        return GEX_T("f");
    return GEX_T("p");
}

void	CGexReport::WriteHtmlPageLayerLegends(CGexChartOverlays *pChartsInfo)
{
    static char	szString[300];

    // just in case!
    if(hReportFile == NULL)
        return;

    // HTML code to open table, Width = 98%, cell spacing=0
    WriteHtmlOpenTable(98,0);

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"90%%\" bgcolor=%s>Parameter</td>\n",szFieldColor);
    fprintf(hReportFile,"</tr>\n");

    QString	strParameterName;
    CGexSingleChart *pChart ;
    CTest *ptTestCell;
    CGexGroupOfFiles *pTestGroup;
    CGexFileInGroup *pTestFile;
    int	iRed,iGreen,iBlue;

    QString strAdvHistogramFieldOptions = (m_pReportOptions->GetOption(QString("adv_histogram"), QString("field"))).toString();
    QStringList qslAdvHistogramFieldOptionList = strAdvHistogramFieldOptions.split(QString("|"));

    for (QList<CGexSingleChart*>::const_iterator
         iter  = pChartsInfo->chartsList().begin();
         iter != pChartsInfo->chartsList().end(); ++iter) {
        pChart = *iter;
        iRed = pChart->cColor.red();
        iGreen = pChart->cColor.green();
        iBlue = pChart->cColor.blue();
        sprintf(szString,"%02x%02x%02x",iRed,iGreen,iBlue);

        // Get pointer to paameter data structure.
        pTestGroup = (pChart->iGroupX < 0 || pChart->iGroupX >= getGroupsList().size()) ? NULL : getGroupsList().at(pChart->iGroupX);
        pTestFile  = (pTestGroup->pFilesList.isEmpty()) ? NULL : pTestGroup->pFilesList.first();
        pTestFile->FindTestCell(pChart->iTestNumberX,pChart->iPinMapX,&ptTestCell,false,false,
                                pChart->strTestNameX.toLatin1().data());

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"10%%\" bgcolor=#%s style=\"border-style: solid; border-width: 1\"> </td>\n",szString);

        // If multiple groups exist, write the group name
        if(getGroupsList().count() > 1)
            strParameterName = pTestGroup->strGroupName + " - Test " + QString::number(pChart->iTestNumberX) + " : ";
        else
            strParameterName = "";

        strParameterName += "<b>" + pChart->strChartName + "</b>";


        // Add few statistics for each layer
        //strParameterName += "<font size=\"3\">";	// Use small fonts
        strParameterName += "<font >";

        //-- limits
        if(qslAdvHistogramFieldOptionList.contains("limits"))
        {
            strParameterName += "<br>Low L.: ";
            strParameterName += ptTestCell->GetCurrentLimitItem()->szLowL;
            strParameterName += "<br>High L.: ";
            strParameterName += ptTestCell->GetCurrentLimitItem()->szHighL;
        }

        //-- min, max
        if(qslAdvHistogramFieldOptionList.contains("min") && qslAdvHistogramFieldOptionList.contains("max"))
        {
            strParameterName += "<br>Min / Max: ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfMin,ptTestCell->res_scal);
            strParameterName += " / ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfMax,ptTestCell->res_scal);
        }
        else if(qslAdvHistogramFieldOptionList.contains("min") )
        {
            strParameterName += "<br>Min : ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfMin,ptTestCell->res_scal);
        }
        else if(qslAdvHistogramFieldOptionList.contains("max") )
        {
            strParameterName += "<br>Max : ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfMax,ptTestCell->res_scal);
        }

        //-- mean
        if(qslAdvHistogramFieldOptionList.contains("mean"))
        {
            strParameterName += "<br>Mean: ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal);
        }

        // -- sigma
        if(qslAdvHistogramFieldOptionList.contains("sigma"))
        {
            strParameterName += "<br>Sigma: ";
            strParameterName += pTestFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal);
        }

        //-- cp, cpk
        if(qslAdvHistogramFieldOptionList.contains("cp") && qslAdvHistogramFieldOptionList.contains("cpk"))
        {
            strParameterName += "<br>Cp / Cpk: ";
            strParameterName += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp);
            strParameterName += " / ";
            strParameterName += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk);
        }
        else if(qslAdvHistogramFieldOptionList.contains("cp") )
        {
            strParameterName += "<br>Cp: ";
            strParameterName += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp);
        }
        else if(qslAdvHistogramFieldOptionList.contains("cpk") )
        {
            strParameterName += "<br>Cpk: ";
            strParameterName += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk);
        }

        //-- shape
        if(qslAdvHistogramFieldOptionList.contains("shape"))
        {
            strParameterName += "<br>Shape: ";
            strParameterName += patlib_GetDistributionName(patlib_GetDistributionType(ptTestCell));
        }
        strParameterName += "</font><br>";

        fprintf(hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,strParameterName.toLatin1().constData());
        fprintf(hReportFile,"</tr>\n");
    }

    fprintf(hReportFile,"</table>\n");
}

void writeHtmlFile(FILE *hFile, const char *szTitle, const char *szResult)
{
    fprintf(hFile,"<tr>\n");
    fprintf(hFile,"<td width=\"24%%\" bgcolor=%s><font size=\"3\">%s</font></td>\n",szFieldColor,szTitle);
    fprintf(hFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"3\">%s</font></td>\n",szDataColor,szResult);
    fprintf(hFile,"</tr>\n");
}

 void	CGexReport::WriteHistoHtmlTestInfo(CGexChartOverlays *pChartsInfo, const char *szChartType,CGexFileInGroup *pFile, CTest *ptTestCell,CGexFileInGroup *pFileY, CTest *ptTestCellY, BOOL bAdvancedReport, const QString& strImageName /*= QString()*/
                                           , int iSizeX /*= 0*/, int bIsAdvancedFunc)
 {
     if (strImageName.isEmpty())
        WriteHistoHtmlTestInfo(pChartsInfo, szChartType, pFile, ptTestCell, pFileY, ptTestCellY, bAdvancedReport,
                               QStringList(), iSizeX,bIsAdvancedFunc);
    else
        WriteHistoHtmlTestInfo(pChartsInfo, szChartType, pFile, ptTestCell, pFileY, ptTestCellY, bAdvancedReport,
                               QStringList(strImageName), iSizeX,bIsAdvancedFunc);
 }

void
CGexReport::
WriteHistoHtmlTestInfo(CGexChartOverlays* pChartsInfo,
                       const char* szChartType,
                       CGexFileInGroup* pFile,
                       CTest* ptTestCell,
                       CGexFileInGroup* /*pFileY*/,
                       CTest* ptTestCellY,
                       BOOL bAdvancedReport,
                       const QStringList& lstImageName /*= QStringList()*/,
                       int iSizeX, int bIsAdvancedFunc)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString("Write html test(s) info for %1 tests/groups for chart '%2'").arg(
      //       m_pReportOptions->iGroups, szChartType?szChartType:"?");

    if((hReportFile == NULL) || (ptTestCell == NULL))
        return;

    QString				strDrillArgument;
    char				szString[300];
    char				szTempString[GEX_LIMIT_LABEL];
    const char *		ptSlash			= NULL;
    QString				strBookmark;		// Hyperlink to test statistics table
    QString				strTestBookmark;	// Bookmark for this test statistics we write
    QString             strDistributionType;
    bool				bCompactLayout	= true;
    CGexFileInGroup *	pTestFile		= NULL;
    bool				bAlarmColor		= false;
    bool				bMyReport		= false;
    bool				bBannerChart	= false;
    bool				bMultiChart		= (lstImageName.count() == 4) ? true : false;
    CGexGroupOfFiles *	pGroup=0;
    CGexGroupOfFiles *	pTestGroup=0;
    CTest *				ptTmpTestCell=0;
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    // OPTIONS
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString().toUpper();
    QString strStatisticsAdvFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("adv_fields"))).toString();
    QStringList qslStatisticsAdvFieldsList = strStatisticsAdvFieldsOptions.split(QString("|"));
    QString strAdvHistogramFieldOptions = (m_pReportOptions->GetOption(QString("adv_histogram"), QString("field"))).toString();
    QStringList qslAdvHistogramFieldOptionList = strAdvHistogramFieldOptions.split(QString("|"));
    if(m_pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL && bIsAdvancedFunc)
    {
        strStatisticsAdvFieldsOptions.clear();
        strAdvHistogramFieldOptions.clear();
        qslAdvHistogramFieldOptionList.clear();
        qslStatisticsAdvFieldsList.clear();
    }

    QString lfOptionShiftAnlzMethod = m_pReportOptions->GetOption( "statistics", "shift_with" ).toString();

    // If the Histogram chart is one image (and not oe image per bar), then consider different layout, depending of the image size...
    if(bAdvancedReport)
    {
        switch(iSizeX)
        {
            case GEX_CHARTSIZE_SMALL_X:
            case GEX_CHARTSIZE_SMALL:
            case GEX_CHARTSIZE_MEDIUM_X:
            case GEX_CHARTSIZE_MEDIUM:
                // Small or medium images: then, create table and first cell is the image, 2nd cell is the stats info.
                WriteHtmlOpenTable(100,0);	// HTML code to open table, Width = 100%, cell spacing=0

                fprintf(hReportFile,"<tr>");
                fprintf(hReportFile,"<td width=\"50%%\">");
                bCompactLayout = true;
                break;
            case GEX_CHARTSIZE_LARGE:
            case GEX_CHARTSIZE_LARGE_X:
                // Large images: then, sho the stats info., then next line is the big image.
                bCompactLayout = false;
                break;

            case GEX_CHARTSIZE_BANNER:		// In Banner mode, no HTML text included so to maximize # of charts per page.
            case GEX_CHARTSIZE_BANNER_X:	// In Banner mode, no HTML text included so to maximize # of charts per page.
                bBannerChart = true;
                goto write_image;
        }
    }

    // Check if this is Test statitcis for Advanced reports....then define the bookmark accordingly.
    strDrillArgument = szChartType;
    if(strDrillArgument.startsWith("adv_"))
        strTestBookmark = "Adv";
    else
        strTestBookmark = "Histo";	// Standard Histogram page


    // HTML code to open table, Width = 98%, Cell spacing=0
    WriteHtmlOpenTable(98,0);

    // If multi-layer chart, output for MyReport (flat HTML), then display colors per layer!
    if(pChartsInfo != NULL && pChartsInfo->chartsList().count() >= 1 && pChartsInfo->bFileOutput)
    {
        CGexSingleChart *pChart=0;
        QColor cColor;

        // Flag that in this code we write test# AND test name!
        bMyReport = true;

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Tests</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);
        for(int i=0;i< (int)pChartsInfo->chartsList().count();i++)
        {
            pChart = pChartsInfo->chartsList().at(i);
            cColor = pChart->cColor;
            sprintf(szString,"#%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());

            // Html color string.
            fprintf(hReportFile,"<font color=\"%s\"><b>%s</b></font><br>",szString,pChart->strTestLabelX.toLatin1().constData());
        }
        fprintf(hReportFile,"</td>\n");
        fprintf(hReportFile,"</tr>\n");
        ptSlash = "/  ";	// Normal text separator in HTML text bellow.
    //FIXME: not used ?
    //ptComma = ",";	// Normal text separator in HTML text bellow.
    }
    else
    {
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Test</font></td>\n",szFieldColor, iHthmSmallFontSize);

        // Hyperlink only exists if page is regular HTML, not Interactive Charting.
        if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
        {

            if(ptTestCell->iHtmlStatsPage>0)
            {
                // Bookmark: are in same page if FLAT HTML page is generated
                if (strOutputFormat=="HTML")
                    strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
                else
                    strBookmark = "#StatT";	// Test Statistics bookmark header string.

                fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><a name=\"%sT%s\"></a> <a href=\"%s%s\">%s</a></td>\n",szDataColor,strTestBookmark.toLatin1().constData(),ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            }
            else if (m_pReportOptions->getAdvancedReport() == GEX_ADV_CANDLE_MEANRANGE &&
                     ptTestCell->iHtmlAdvancedPage && ptTestCell->bTestType != 'F')
            {
                // Bookmark: are in same page if FLAT HTML page is generated
                if (strOutputFormat=="HTML")
                    strBookmark.sprintf("advanced%d.htm#AdvT", ptTestCell->iHtmlAdvancedPage);
                else
                    strBookmark = "#AdvT";	// Advanced section of Test bookmark header string.

                // If this test has a histogram page, create the hyperlink!
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\" ><b><a name=\"StatT%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",
                        szFieldColor,ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),
                        ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            }
            else
                fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",szDataColor, iHthmSmallFontSize, ptTestCell->szTestLabel);

            ptSlash = "/  ";	// Normal text separator in HTML text bellow.
      //FIXME: not used ?
      //ptComma = ",";	// Normal text separator in HTML text bellow.
        }
        else
        {
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",szDataColor, iHthmSmallFontSize, ptTestCell->szTestLabel);
      //FIXME: not used ?
      //ptComma =
            ptSlash = "<br>";	// Line-Feed text separator used in HTML text bellow.
        }
        fprintf(hReportFile,"</tr>\n");
    }

    char				szTestName[1024];
    const char *		szBackgroundColor;	// Holds the background color, set to 'red' if drift alarm
    int					iIndex;

    if(m_pReportOptions->iGroups > 1)
    {
        // Tells which Color assigned to each chart...unless we're in interactive mode!
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Groups colors</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

        for(int i = 1; i <= m_pReportOptions->iGroups; i++)
        {
            pGroup = ((i-1) < 0 || (i-1) >= getGroupsList().size()) ? NULL : getGroupsList().at(i-1);

            if((pChartsInfo != NULL) && (i == 1))
            {
                // This page is created for a Interactive mode...so color of the Reference can be custom...
                CGexSingleChart *	pChart = pChartsInfo->chartsList().at(0);
                QColor				cColor = pChart->cColor;

                sprintf(szString,"#%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
            }
            else
                strcpy(szString,GetChartingHtmlColor(i));

            // Html color string.
            fprintf(hReportFile,"<font color=\"%s\"><b>%s</b> </font>",szString,pGroup->strGroupName.toLatin1().constData());
        }

        fprintf(hReportFile,"</td>\n");
        fprintf(hReportFile,"</tr>\n");
    }

    // If test name not already written...do it now!
    if(!bMyReport)
    {
        BuildTestNameString(pFile,ptTestCell,szTestName);
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Name</font></td>\n",szFieldColor, iHthmSmallFontSize);
        if(m_pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL && bIsAdvancedFunc)
        {
            //remocve this when called not from adv func histo
            QString strTestName = "Test Name " + buildDisplayName(szTestName) + "<br>";
            strTestName += " Pattern :";

            foreach(const QString &strPattern, ptTestCell->mVectors.keys()){
                if(!ptTestCell->mVectors[strPattern].m_lstVectorInfo.count())
                    continue;
                strTestName += QString(" %1 <br>").arg(strPattern);
            }

            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",szDataColor, iHthmSmallFontSize, strTestName.toLatin1().constData());
        }
        else
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",szDataColor, iHthmSmallFontSize, buildDisplayName(szTestName).toLatin1().constData());

        fprintf(hReportFile,"</tr>\n");
    }

    if(qslAdvHistogramFieldOptionList.contains(QString("test_type")))
    {
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Test type</font></td>\n",szFieldColor, iHthmSmallFontSize);
        // Build Test type string
        BuildTestTypeString(pFile,ptTestCell,szString,false);
        fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",szDataColor, iHthmSmallFontSize, szString);
        fprintf(hReportFile,"</tr>\n");
    }

    if(qslAdvHistogramFieldOptionList.contains(QString("shape")))
    {
        if (lPatInfo)
        {
            strDistributionType = patlib_GetDistributionName(patlib_GetDistributionType(ptTestCell,
                                                                                        lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                                                        lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                                                        lPatInfo->GetRecipeOptions().mMinConfThreshold));
        }
        else
            strDistributionType = patlib_GetDistributionName(patlib_GetDistributionType(ptTestCell));
    }

    // If multiple groups: we will create a chart image using the AdvancedHisto function.
    if(m_pReportOptions->iGroups > 1)
    {
        // Display test limits
        if(qslAdvHistogramFieldOptionList.contains(QString("limits")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>High Limit</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL  || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    char* lLimit;
                    if((ptTmpTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                        lLimit = pFile->FormatTestLimit(ptTmpTestCell, szTempString, ptTmpTestCell->GetCurrentLimitItem()->lfHighLimit, ptTmpTestCell->hlm_scal, false);
                    else
                        lLimit = QString(GEX_NA).toLatin1().data();
                    fprintf(hReportFile,"%s   ", lLimit);
                }
                else
                    fprintf(hReportFile,"0   ");	// No test cell entry for this group!

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
                else // a separator is required
                    fprintf(hReportFile,"%s  ", ptSlash);
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Low Limit</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL  || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    char* lLimit;
                    if((ptTmpTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                        lLimit = pFile->FormatTestLimit(ptTmpTestCell, szTempString, ptTmpTestCell->GetCurrentLimitItem()->lfLowLimit, ptTmpTestCell->llm_scal, false);
                    else
                        lLimit = QString(GEX_NA).toLatin1().data();
                    fprintf(hReportFile,"%s   ", lLimit);
                }
                else
                    fprintf(hReportFile,"0   ");	// No test cell entry for this group!

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
                else // a separator is required
                    fprintf(hReportFile,"%s  ", ptSlash);
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Exec count' information
        if(qslAdvHistogramFieldOptionList.contains(QString("exec_count")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Samples</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL  || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                    fprintf(hReportFile,"%d   ",ptTmpTestCell->ldSamplesValidExecs);
                else
                    fprintf(hReportFile,"0   ");	// No test cell entry for this group!

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
                else // a separator is required
                    fprintf(hReportFile,"%s  ", ptSlash);
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        TestShift lTestShift;
        if (ptTestCell->mTestShifts.size() > 0)
        {
            lTestShift = ptTestCell->mTestShifts.first();
        }

        // GCORE-17411 - when comparing all groups together, shift are in the tests of the reference group of files
        const CGexGroupOfFiles * const lReferenceGroupOfFiles = getGroupsList().first();
        CGexFileInGroup * const lReferenceGroupFile = lReferenceGroupOfFiles->GetFilesList().first();
        CTest *lReferenceTest = NULL;

        lReferenceGroupFile->FindTestCell( ptTestCell->lTestNumber,
                                           ptTestCell->lPinmapIndex,
                                           &lReferenceTest,
                                           false,
                                           false,
                                           ptTestCell->strTestName.toLatin1().data() );

        const GS::Core::MLShift &lReferenceTestShift = lTestShift.GetMlShift( lReferenceTest->GetCurrentLimitItem() );

        // 'Mean-MeanShift' information
        if(qslAdvHistogramFieldOptionList.contains(QString("mean")))
        {
            QString strMeanString;
            float fMean = ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs;
            float fMaxMeanShift = 0.;

            double lfOptionStorageDevice;
            bool bGetOptionRslt=false;
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            // Build mean result string, background color not known until all means scanned!
            strMeanString = "";
            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    if (ptTmpTestCell->ldSamplesValidExecs)
                        fMean = ptTmpTestCell->lfSamplesTotal/ptTmpTestCell->ldSamplesValidExecs;
                    else
                        fMean = C_INFINITE;
                }
                else
                    fMean = 0;	// No test cell for this group

                // Highlight in red if mean outside of limits!
                if(ptTmpTestCell)
                {
                    if (ptTmpTestCell->ldSamplesValidExecs > 0 &&
                        ((((ptTmpTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (fMean < ptTmpTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                        (((ptTmpTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (fMean > ptTmpTestCell->GetCurrentLimitItem()->lfHighLimit))) )
                        szBackgroundColor = szAlarmColor;

                    strMeanString += pFile->FormatTestResult(ptTmpTestCell,fMean,ptTmpTestCell->res_scal, false);
                }
                strMeanString += "   " + QString(ptSlash) + "  ";

                if (ptTmpTestCell->mTestShifts.size() > 0)
                {
                    // GCORE-17411 - when comparing all group of files with each other, calculation is different, see below
                    if(lfOptionShiftAnlzMethod != "shift_to_all")
                    {
                        TestShift lTmpTestShift;
                        GS::Core::MLShift ltmpMLShift;

                        lTmpTestShift = ptTmpTestCell->mTestShifts.first();
                        ltmpMLShift = lTmpTestShift.GetMlShift(ptTmpTestCell->GetCurrentLimitItem());
                        if(fabs(ltmpMLShift.mMeanShiftPct) > fabs(fMaxMeanShift))
                            fMaxMeanShift = ltmpMLShift.mMeanShiftPct;
                    }
                }

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }

            // grab shift values in the reference test shifts
            if(lfOptionShiftAnlzMethod == "shift_to_all")
                fMaxMeanShift = lReferenceTestShift.GetMeanShiftPct();

            bGetOptionRslt = (fabs(fMaxMeanShift) >= lfOptionStorageDevice)&& (lfOptionStorageDevice >= 0);
            if(bGetOptionRslt)
                szBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
            else
                szBackgroundColor = szDataColor;
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Mean  %sMax.shift</td>\n",szFieldColor,ptSlash);

            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szBackgroundColor);
            fprintf(hReportFile,"%s",strMeanString.toLatin1().constData());
            fprintf(hReportFile," (%.2f%%)</td>\n", fMaxMeanShift);
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Median-Median shift' information
        if(qslAdvHistogramFieldOptionList.contains(QString("quartile2")))
        {
            float fMedian = ptTestCell->lfSamplesQuartile2;
            float fMaxMeanShift = 0.;

            double lfOptionStorageDevice;
            bool bGetOptionRslt;

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    fMedian = ptTmpTestCell->lfSamplesQuartile2;
                    fprintf(hReportFile,"%s   %s  ",
                        pFile->FormatTestResult(ptTmpTestCell,fMedian,ptTmpTestCell->res_scal, false),ptSlash);
                }
                else
                {
                    fprintf(hReportFile,"0   %s  ",ptSlash);
                }

                if (ptTmpTestCell->mTestShifts.size() > 0)
                {
                    // GCORE-17411 - when comparing all group of files with each other, calculation is different, see below
                    if(lfOptionShiftAnlzMethod != "shift_to_all")
                    {
                        TestShift lTmpTestShift;
                        GS::Core::MLShift ltmpMLShift;

                        lTmpTestShift = ptTmpTestCell->mTestShifts.first();
                        ltmpMLShift = lTmpTestShift.GetMlShift(ptTmpTestCell->GetCurrentLimitItem());
                        if(fabs(ltmpMLShift.mMeanShiftPct) > fabs(fMaxMeanShift))
                            fMaxMeanShift = ltmpMLShift.mMeanShiftPct;
                    }
                }

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }

            // grab shift values in the reference test shifts
            if(lfOptionShiftAnlzMethod == "shift_to_all")
                fMaxMeanShift = lReferenceTestShift.GetMeanShiftPct();

            bGetOptionRslt = (fabs(fMaxMeanShift) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0);
            if(bGetOptionRslt)
                szBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
            else
                szBackgroundColor = szDataColor;
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Median  %sMax.shift</td>\n",szFieldColor,ptSlash);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szBackgroundColor);

            fprintf(hReportFile," (%.2f%%)</td>\n", fMaxMeanShift);
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Sigma-SigmaShift' information
        if(qslAdvHistogramFieldOptionList.contains(QString("sigma")))
        {
            double lfOptionStorageDevice;
            bool bGetOptionRslt;
            float fMaxSigmaShift = 0.;//lTestShift.mSigmaShiftPercent;
            QString lTmpReportValuesStr;

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    lTmpReportValuesStr += QString(pFile->FormatTestResult(ptTmpTestCell,ptTmpTestCell->lfSigma,ptTmpTestCell->res_scal, false));
                }
                else
                {
                    lTmpReportValuesStr += "0";
                }
                lTmpReportValuesStr += " ";
                lTmpReportValuesStr += ptSlash;

                if (ptTmpTestCell->mTestShifts.size() > 0)
                {
                    // GCORE-17411 - when comparing all group of files with each other, calculation is different, see below
                    if(lfOptionShiftAnlzMethod != "shift_to_all")
                    {
                        float lTmpSigmaShift = ptTmpTestCell->mTestShifts.first().mSigmaShiftPercent;

                        if(fabs(lTmpSigmaShift) > fabs(fMaxSigmaShift))
                            fMaxSigmaShift = lTmpSigmaShift;
                    }
                }

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }

            // grab shift values in the reference test shifts
            if(lfOptionShiftAnlzMethod == "shift_to_all")
                fMaxSigmaShift = lReferenceTestShift.GetSigmaShiftPct();

            bGetOptionRslt = (fabs(fMaxSigmaShift) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0);
            if(bGetOptionRslt)
                szBackgroundColor = szAlarmColor;	// ALARM!: Sigma shift over limit.
            else
                szBackgroundColor = szDataColor;
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Sigma %sMax.shift</td>\n",szFieldColor,ptSlash);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szBackgroundColor);
            fprintf(hReportFile,"%s", lTmpReportValuesStr.toStdString().c_str());

            fprintf(hReportFile," (%.2f%%)</td>\n",fMaxSigmaShift);
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Min' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("min")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Min</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                    fprintf(hReportFile,"%s %s",pFile->FormatTestResult(ptTmpTestCell,ptTmpTestCell->lfMin,ptTestCell->res_scal, false),ptSlash);
                else
                    fprintf(hReportFile,"0 %s",ptSlash);

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Max' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("max")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Max</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                    fprintf(hReportFile,"%s %s",pFile->FormatTestResult(ptTmpTestCell,ptTmpTestCell->lfMax,ptTestCell->res_scal, false),ptSlash);
                else
                    fprintf(hReportFile,"0 %s",ptSlash);

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Cp' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("cp")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Cp</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                    fprintf(hReportFile,"%s ",CreateResultStringCpCrCpk(ptTmpTestCell->GetCurrentLimitItem()->lfCp));
                else
                    fprintf(hReportFile,"0 ");

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
                else // a separator is required
                    fprintf(hReportFile,"%s  ", ptSlash);
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Cpk-CpkShift' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("cpk")))
        {
            double lfOptionStorageDevice;
            bool bGetOptionRslt;
            float fMaxCpkShift = 0.;

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            QString lTmpReportValuesStr;
            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false, ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    lTmpReportValuesStr += QString(CreateResultStringCpCrCpk(ptTmpTestCell->GetCurrentLimitItem()->lfCpk));
                }
                else
                {
                    lTmpReportValuesStr += "0";
                }
                lTmpReportValuesStr += " ";
                lTmpReportValuesStr += ptSlash;

                if (ptTmpTestCell->mTestShifts.size() > 0)
                {
                    // GCORE-17411 - when comparing all group of files with each other, calculation is different, see below
                    if(lfOptionShiftAnlzMethod != "shift_to_all")
                    {
                        TestShift lTmpTestShift;
                        GS::Core::MLShift ltmpMLShift;

                        lTmpTestShift = ptTmpTestCell->mTestShifts.first();
                        ltmpMLShift = lTmpTestShift.GetMlShift(ptTmpTestCell->GetCurrentLimitItem());

                        if(fabs(ltmpMLShift.mCpkShift) > fabs(fMaxCpkShift))
                            fMaxCpkShift = ltmpMLShift.mCpkShift;
                    }
                }

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }

            // grab shift values in the reference test shifts
            if(lfOptionShiftAnlzMethod == "shift_to_all")
                fMaxCpkShift = lReferenceTestShift.GetCpkShiftPct();

            if(!bGetOptionRslt)
                lfOptionStorageDevice = 33;
            bGetOptionRslt = (fabs(fMaxCpkShift) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0);
            if(bGetOptionRslt)
                szBackgroundColor = szAlarmColor;	// ALARM!: Sigma shift over limit.
            else
                szBackgroundColor = szDataColor;
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Cpk %sMax.shift</td>\n",szFieldColor,ptSlash);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szBackgroundColor);

            fprintf(hReportFile,"%s", lTmpReportValuesStr.toStdString().c_str());
            fprintf(hReportFile,"(%.2f%%)</td>\n", fMaxCpkShift);
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Failures' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("fail_count")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Failures</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false,ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                    fprintf(hReportFile,"%d ",ptTmpTestCell->GetCurrentLimitItem()->ldFailCount);
                else
                    fprintf(hReportFile,"0 ");

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
                else // a separator is required
                    fprintf(hReportFile,"%s  ", ptSlash);
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // 'Outlier' imformation
        if(qslAdvHistogramFieldOptionList.contains(QString("removed_count")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s>Removed Count</td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\">",szDataColor);

            int	ldOutliers;
            for(iIndex=0;iIndex<m_pReportOptions->iGroups;iIndex++)
            {
                if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
                {
                    // Creating HTML report
                    pTestGroup = (iIndex >= getGroupsList().size()) ? NULL : getGroupsList().at(iIndex);
                    if (!pTestGroup->pFilesList.isEmpty())
                    {
                        pTestFile  = pTestGroup->pFilesList.at(0);
                        pTestFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTmpTestCell,
                                                false,false,ptTestCell->strTestName.toLatin1().data());
                    }
                }
                else
                    ptTmpTestCell = ptTestCell;	// Interactive mode: only display currently pointed layer!
                if(ptTmpTestCell)
                {
                    ldOutliers = ptTmpTestCell->GetCurrentLimitItem()->ldOutliers;
                    if(ldOutliers <=  0)
                        ldOutliers= 0;
                }
                else
                    ldOutliers=0;
                fprintf(hReportFile,"%d %s",ldOutliers,ptSlash);

                // If interactive mode: only display stats for current layer selected.
                if(pChartsInfo != NULL && (pChartsInfo->bFileOutput == false))
                    break;
            }
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        if (!qstricmp(szChartType,"adv_scatter") && m_pReportOptions->iGroups==2 && ptTestCellY)
        {
            // Let's compute Pearson
            // Statistics are for a scatter plot, so display the Pearson's correlation coefficient.
            int count=0;
            double lfPearson = ComputePearsonValue(ptTestCell, ptTestCellY, count);
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Corr.(r)</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s><font size=\"%d\">%.6lf</td>\n",
                    szDataColor, iHthmSmallFontSize, lfPearson);
            fprintf(hReportFile,"</tr>\n");
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Corr.(r^2)</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s><font size=\"%d\">%.6lf</td>\n",
                    szDataColor,iHthmSmallFontSize, lfPearson*lfPearson);
            fprintf(hReportFile,"</tr>\n");
        }


        if(pChartsInfo != NULL && strDistributionType.isEmpty() == false)
        {
            // Interactive mode: Tell distribution type:
            //FIXME: not used ?
            /*
            double lfQ1,lfQ2,lfQ3,lfQ4;	// Percentage of samples belonging to Q1,Q2,Q3 and Q4 spaces
            lfQ1 = (ptTestCell->lfSamplesQuartile1-ptTestCell->lfSamplesMin) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ2 = (ptTestCell->lfSamplesQuartile2-ptTestCell->lfSamplesQuartile1) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ3 = (ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile2) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ4 = (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesQuartile3) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            */

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Shape</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",
                    szDataColor,iHthmSmallFontSize, strDistributionType.toLatin1().constData());
            fprintf(hReportFile,"</tr>\n");
        }

        fprintf(hReportFile,"</table>\n");

        if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
        {
            // Have the ToolBar line written in the HTML file
            strDrillArgument= "drill_chart=";
            strDrillArgument += szChartType;
            strDrillArgument += "--data=";
            strDrillArgument += ptTestCell->szTestLabel;

            // If advanced Histogram, the image must be added now!
            if(bAdvancedReport)
            {
                // If we're in compact layout, then we have the image + test statistics wrapped in a table with 2 horizontal cells
                if(bCompactLayout)
                    fprintf(hReportFile,"</td>\n<td width=\"50%%\">");
                WriteHtmlToolBar(iSizeX, true, strDrillArgument);

                if (bMultiChart)
                {
                    WriteHtmlOpenTable(100, 1);
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(0)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(1)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"</tr>\n");

                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(2)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(3)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"</tr>\n");

                    fprintf(hReportFile,"</table><br>\n");
                }
                else if (lstImageName.count() > 0)
                {
                     QString strTempBookmark = "Histo";	// Test Statistics bookmark header string.

                     fprintf(hReportFile,
                        "<img border=\"0\" alt=\"?\" src=\"../images/%s\"> <a name=\"%sT%s\"></a> <a href=\"#StatT%s\"></a> </img>\n",
                        formatHtmlImageFilename(lstImageName.at(0)).toLatin1().constData(),
                        strTempBookmark.toLatin1().constData(),
                        ptTestCell->szTestLabel,
                        ptTestCell->szTestLabel);
                }

                if(bCompactLayout)
                    fprintf(hReportFile,"</td>\n</tr>\n</table><br>\n");
                else
                    fprintf(hReportFile,"<br><hr>\n");
            }
            else
                WriteHtmlToolBar(iSizeX,true,strDrillArgument);
        }
    } // Multiple groups (correlation analysis)
    else
    {
        // Only 1 group : Histogram chart is made of prebuilt image bars!
        // limits
        if(qslAdvHistogramFieldOptionList.contains(QString("limits")))
        {
            QString lValue;
            QString lDataColor = szDataColor;

            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                lValue = pFile->FormatTestLimit(ptTestCell, szTempString, ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                                 ptTestCell->llm_scal, false);

                // If the limit is custom (from a 'what-if', then changed its background color.
                // // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
                if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    lDataColor = szDrillColorIf;	// Custom color (custom limit from 'what-if')
                    lValue     += " (custom limit - Guard Banding simulation)";
                }
            }
            else
                lValue =  QString(GEX_NA).toLatin1().data();

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Low limit</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s",
                    lDataColor.toLatin1().constData(), iHthmSmallFontSize, lValue.toLatin1().constData());
            fprintf(hReportFile,"</font></td>\n</tr>\n");

            // High limit
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                lValue = pFile->FormatTestLimit(ptTestCell, szTempString, ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                                 ptTestCell->hlm_scal, false);

                // If the limit is custom (from a 'what-if', then changed its background color.
                // // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
                if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    lDataColor = szDrillColorIf;	// Custom color (custom limit from 'what-if')
                    lValue     += " (custom limit - Guard Banding simulation)";
                }
            }
            else
                lValue =  QString(GEX_NA).toLatin1().data();

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">High limit</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s",
                    lDataColor.toLatin1().constData(), iHthmSmallFontSize, lValue.toLatin1().constData());
            fprintf(hReportFile,"</font></td>\n</tr>\n");
        }

        // Mean, Min, Max from SUMMARY
        if(qslAdvHistogramFieldOptionList.contains(QString("mean")) ||
           qslAdvHistogramFieldOptionList.contains(QString("min")) ||
           qslAdvHistogramFieldOptionList.contains(QString("max")) )
        {
            QString lHeader;
            QString lData;

            if(pChartsInfo != NULL)
                lHeader += "Summary:";

            if(qslAdvHistogramFieldOptionList.contains(QString("mean")))
            {
                // Mean value (Summary)
                if(pChartsInfo == NULL)
                {
                    if (lHeader.isEmpty() == false)
                        lHeader += " / ";
                    lHeader += "Mean";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMean, ptTestCell->res_scal, false);
                }
                else
                {
                    lHeader += "<br>Mean";
                    lData   += "<br>";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMean, ptTestCell->res_scal, false);
                }
            }

            if(qslAdvHistogramFieldOptionList.contains(QString("min")))
            {
                // Minimum (summary)
                if(pChartsInfo == NULL)
                {
                    if (lHeader.isEmpty() == false)
                        lHeader += " / ";
                    lHeader += "Min";

                    if (lData.isEmpty() == false)
                        lData   += " / ";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMin, ptTestCell->res_scal, false);
                }
                else
                {
                    lHeader += "<br>Min";
                    lData   += "<br>";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMin, ptTestCell->res_scal, false);
                }
            }

            if(qslAdvHistogramFieldOptionList.contains(QString("max")))
            {
                // Maximum (summary)
                if(pChartsInfo == NULL)
                {
                    if (lHeader.isEmpty() == false)
                        lHeader += " / ";
                    lHeader += "Max";

                    if (lData.isEmpty() == false)
                        lData   += " / ";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMax, ptTestCell->res_scal, false);

                }
                else
                {
                    lHeader += "<br>Max";
                    lData   += "<br>";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfMax, ptTestCell->res_scal, false);
                }
            }

            if (pChartsInfo == NULL)
                lData   += " from summary";

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,
                    "<td width=\"24%%\" bgcolor=%s><font size=\"%d\">",
                    szFieldColor, iHthmSmallFontSize);
            fprintf(hReportFile, "%s", lHeader.toLatin1().constData());
            fprintf(hReportFile, "</font></td>\n");

            fprintf(hReportFile,
                    "<td width=\"24%%\" bgcolor=%s><font size=\"%d\">",
                    szDataColor, iHthmSmallFontSize);
            fprintf(hReportFile,"%s", lData.toLatin1().constData());
            fprintf(hReportFile, "</font></td>\n");

            fprintf(hReportFile,"</tr>\n");
            fprintf(hReportFile,"<tr>\n");

            // Mean, Min, Max from Data samples
            if(pChartsInfo == NULL)
            {
                lHeader.clear();
                lData.clear();
            }
            else
            {
                lHeader = "<br>Samples:";
                lData = "<br>";
                // Fails / Outliers
                if( (qslAdvHistogramFieldOptionList.contains(QString("removed_count"))) || (qslAdvHistogramFieldOptionList.contains(QString("fail_count"))) )
                {

                    lHeader += "<br>Exec<br>Fails";
                    double lFails = 0;
                    if(ptTestCell->ldSamplesValidExecs)
                        lFails = 100.0*(ptTestCell->GetCurrentLimitItem()->ldSampleFails)/(double)ptTestCell->ldSamplesValidExecs;


                    lData += QString().sprintf("<br>%d", ptTestCell->ldSamplesValidExecs);


                    lData += QString().sprintf("<br>%d (%.2f %%)",
                                              ptTestCell->GetCurrentLimitItem()->ldSampleFails, lFails);
                }
            }

            // Mean value (samples)
            if(qslAdvHistogramFieldOptionList.contains(QString("mean")))
            {
                // Mean
                double lMean=0;
                if(ptTestCell->ldSamplesValidExecs)
                {
                    lMean = ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs;

                    // Highlight in red if mean outside of limits!
                    if( (((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (lMean < ptTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                            (((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (lMean > ptTestCell->GetCurrentLimitItem()->lfHighLimit)) )
                        bAlarmColor = true;
                    else
                        bAlarmColor = false;
                }

                if(pChartsInfo == NULL)
                {
                    lHeader += "Mean";

                    if(bAlarmColor)
                        lData += "<font color=\"#FF0000\"><b>";

                    lData   += pFile->FormatTestResult(ptTestCell, lMean, ptTestCell->res_scal, false);

                    if (bAlarmColor)
                        lData += "</b></font>";
                }
                else
                {
                    lHeader += "<br>Mean";
                    lData   += "<br>";

                    if(bAlarmColor)
                        lData += "<font color=\"#FF0000\"><b>";

                    lData   += pFile->FormatTestResult(ptTestCell, lMean, ptTestCell->res_scal, false);

                    if (bAlarmColor)
                        lData += "</b></font>";
                }
            }

            // Minimum (samples)
            if(qslAdvHistogramFieldOptionList.contains(QString("min")))
            {
                if(pChartsInfo == NULL)
                {
                    if (lHeader.isEmpty() == false)
                        lHeader += " / ";
                    lHeader += "Min";

                    if (lData.isEmpty() == false)
                        lData   += " / ";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfSamplesMin, ptTestCell->res_scal, false);
                }
                else
                {
                    lHeader += "<br>Min";
                    lData   += "<br>";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfSamplesMin, ptTestCell->res_scal, false);
                }
            }

            // Maximum (samples)
            if(qslAdvHistogramFieldOptionList.contains(QString("max")))
            {
                if(pChartsInfo == NULL)
                {
                    if (lHeader.isEmpty() == false)
                        lHeader += " / ";
                    lHeader += "Max";

                    if (lData.isEmpty() == false)
                        lData   += " / ";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfSamplesMax, ptTestCell->res_scal, false);

                }
                else
                {
                    lHeader += "<br>Max";
                    lData   += "<br>";
                    lData   += pFile->FormatTestResult(ptTestCell, ptTestCell->lfSamplesMax, ptTestCell->res_scal, false);
                }
            }

            if (pChartsInfo == NULL)
                lData   += " from " + QString::number(ptTestCell->ldSamplesValidExecs) + " samples";

            fprintf(hReportFile,
                    "<td width=\"24%%\" bgcolor=%s><font size=\"%d\">",
                    szFieldColor, iHthmSmallFontSize);
            fprintf(hReportFile, "%s", lHeader.toLatin1().constData());
            fprintf(hReportFile, "</font></td>\n");

            fprintf(hReportFile,
                    "<td width=\"24%%\" bgcolor=%s><font size=\"%d\">",
                    szDataColor, iHthmSmallFontSize);
            fprintf(hReportFile, "%s", lData.toLatin1().constData());
            fprintf(hReportFile, "</font></td>\n");

            fprintf(hReportFile,"</tr>\n");
        }

        // Median (if exists)
        if((qslAdvHistogramFieldOptionList.contains(QString("quartile2"))) && ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
        {
            // Highlight in red if median outside of limits!
            if( (((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (ptTestCell->lfSamplesQuartile2 < ptTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                (((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (ptTestCell->lfSamplesQuartile2 > ptTestCell->GetCurrentLimitItem()->lfHighLimit)) )
                bAlarmColor=true;
            else
                bAlarmColor=false;

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Median</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">",szDataColor,iHthmSmallFontSize);
            if(bAlarmColor)
                fprintf(hReportFile,"<font color=\"#FF0000\"><b>");
            fprintf(hReportFile,"%s",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile2,ptTestCell->res_scal, false));
            if(bAlarmColor)
                fprintf(hReportFile,"</b>");
            fprintf(hReportFile,"</font></td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // Sigma / Range
        if(qslAdvHistogramFieldOptionList.contains(QString("sigma")))
        {
            fprintf(hReportFile,"<tr>\n");
            if(pChartsInfo == NULL)
                fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Sigma / Range</font></td>\n",szFieldColor,iHthmSmallFontSize);
            else
                fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Sigma<br>Range</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s",szDataColor,iHthmSmallFontSize,pFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal, false));
            if(pChartsInfo == NULL)
                fprintf(hReportFile," / ");
            else
                fprintf(hReportFile,"<br>");

            fprintf(hReportFile,"%s</font></td>\n",pFile->FormatTestResult(ptTestCell,ptTestCell->lfRange,ptTestCell->res_scal, false));
            fprintf(hReportFile,"</tr>\n");
        }

        // Cp & Cpk
        if(qslAdvHistogramFieldOptionList.contains(QString("cp")) ||
           qslAdvHistogramFieldOptionList.contains(QString("cpk")))
        {
            fprintf(hReportFile,"<tr>\n");

            QString lLabel;
            QString lValue;

            if(qslAdvHistogramFieldOptionList.contains(QString("cp")))
            {
                lLabel += "Cp";
                lValue += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp);
            }

            if(qslAdvHistogramFieldOptionList.contains(QString("cpk")))
            {
                if (lLabel.isEmpty() == false)
                    lLabel += " / ";
                lLabel += "Cpk";

                if (lValue.isEmpty() == false)
                    lValue += " / ";
                lValue += CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk);

                if(ptTestCell->lfSigma && ptTestCell->GetCurrentLimitItem()->lfCpkLow < 0 &&
                   ptTestCell->GetCurrentLimitItem()->lfCpkLow != C_NO_CP_CPK)
                {
                    lValue += " ....=> Warning: Process is under the low limit";
                }
                else if(ptTestCell->lfSigma && ptTestCell->GetCurrentLimitItem()->lfCpkHigh < 0 &&
                        ptTestCell->GetCurrentLimitItem()->lfCpkHigh != C_NO_CP_CPK)
                {
                    lValue += " ....=> Warning: Process is over the high limit";
                }
            }

            fprintf(hReportFile,
                    "<td width=\"24%%\" bgcolor=%s><font size=\"%d\">%s</font></td>\n",
                    szFieldColor, iHthmSmallFontSize, lLabel.toLatin1().data());
            fprintf(hReportFile,
                    "<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s",
                    szDataColor,iHthmSmallFontSize, lValue.toLatin1().data());
            fprintf(hReportFile,"</font></td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // Skewness & Kurtosis
        if(pChartsInfo != NULL  && !pChartsInfo->bFileOutput)
        {
            if(qslStatisticsAdvFieldsList.contains(QString("skew")))
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Skew</font></td>\n",szFieldColor,iHthmSmallFontSize);
                if(ptTestCell->lfSamplesSkew != -C_INFINITE)
                    fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%g</font></td>\n",szDataColor,iHthmSmallFontSize,ptTestCell->lfSamplesSkew);
                else
                    fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">n/a</font></td>\n",szDataColor,iHthmSmallFontSize);
                fprintf(hReportFile,"</tr>\n");
            }

            if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Kurtosis</font></td>\n",szFieldColor,iHthmSmallFontSize);
                if(ptTestCell->lfSamplesKurt != -C_INFINITE)
                    fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%g</font></td>\n",szDataColor,iHthmSmallFontSize,ptTestCell->lfSamplesKurt);
                else
                    fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">n/a</font></td>\n",szDataColor,iHthmSmallFontSize);
                fprintf(hReportFile,"</tr>\n");
            }

            // Tell distribution type:
            //FIXME: not used ?
            /*double lfQ1,lfQ2,lfQ3,lfQ4;	// Percentage of samples belonging to Q1,Q2,Q3 and Q4 spaces
            lfQ1 = (ptTestCell->lfSamplesQuartile1-ptTestCell->lfSamplesMin) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ2 = (ptTestCell->lfSamplesQuartile2-ptTestCell->lfSamplesQuartile1) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ3 = (ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile2) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);
            lfQ4 = (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesQuartile3) * 100 / (ptTestCell->lfSamplesMax-ptTestCell->lfSamplesMin);*/

            if(strDistributionType.isEmpty() == false)
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Shape</font></td>\n",szFieldColor,iHthmSmallFontSize);
                fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%s</font></td>\n",
                        szDataColor,iHthmSmallFontSize,strDistributionType.toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
            }

#if 0
            // FOR DEBUG
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Skew No-noise</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%g</font></td>\n",szDataColor,iHthmSmallFontSize,ptTestCell->lfSamplesSkewWithoutNoise);
            fprintf(hReportFile,"</tr>\n");

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Q1 Q1%%<br>Q2 Q2%%<br>Q3 Q3%%<br>Q4%%<br></font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%g %.1lf<br>%g %.1lf<br>%g %.1lf<br>%.1lf<br></font></td>\n",szDataColor,iHthmSmallFontSize,
                ptTestCell->lfSamplesQuartile1,lfQ1,
                ptTestCell->lfSamplesQuartile2,lfQ2,
                ptTestCell->lfSamplesQuartile3,lfQ3,
                lfQ4);
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Range/IQR</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%g</font></td>\n",szDataColor,iHthmSmallFontSize,ptTestCell->lfRange/ptTestCell->lfSamplesSigmaInterQuartiles);
            fprintf(hReportFile,"</tr>\n");
#endif

            if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
                writeHtmlFile(hReportFile,"P0.5%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP0_5,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
                writeHtmlFile(hReportFile,"P2.5%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP2_5,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("P10")))
                writeHtmlFile(hReportFile,"P10%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP10,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
                writeHtmlFile(hReportFile,"P25%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile1,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
                writeHtmlFile(hReportFile,"P50%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile2,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
                writeHtmlFile(hReportFile,"P75%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile3,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("P90")))
                writeHtmlFile(hReportFile,"P90%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP90,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
                writeHtmlFile(hReportFile,"P97.5%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP97_5,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
                writeHtmlFile(hReportFile,"P99.5%",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesP99_5,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
                writeHtmlFile(hReportFile,"IQR",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile1,ptTestCell->res_scal, false));
            if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
                writeHtmlFile(hReportFile,"IQR SD",pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesSigmaInterQuartiles,ptTestCell->res_scal, false));
        } // if(pChartsInfo != NULL  && !pChartsInfo->bFileOutput)


        // Exec count
        if(qslAdvHistogramFieldOptionList.contains(QString("exec_count")))
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Samples</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%d</font></td>\n",szDataColor,iHthmSmallFontSize,
                ptTestCell->ldSamplesValidExecs);
            fprintf(hReportFile,"</tr>\n");
        }

        // Fails / Outliers
        if( (qslAdvHistogramFieldOptionList.contains(QString("removed_count"))) || (qslAdvHistogramFieldOptionList.contains(QString("fail_count"))) )
        {
            int	ldOutliers = ptTestCell->GetCurrentLimitItem()->ldOutliers;
            if(ldOutliers <=  0)
                ldOutliers= 0;
            fprintf(hReportFile,"<tr>\n");

            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Fails / Removed</font></td>\n",szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s align=\"left\"><font size=\"%d\">%d / %d</font></td>\n",szDataColor,iHthmSmallFontSize,
                 ptTestCell->GetCurrentLimitItem()->ldFailCount, ldOutliers);
            fprintf(hReportFile,"</tr>\n");
        }

        if(!qstricmp(szChartType,"adv_scatter"))
        {
            // Statistics are for a scatter plot, so display the Pearson's correlation coefficient.
            int count=0;
            double lfPearson = ComputePearsonValue(ptTestCell, ptTestCellY, count);
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Corr. (r)</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s><font size=\"%d\">%.6lf</td>\n",
                    szDataColor, iHthmSmallFontSize, lfPearson);
            fprintf(hReportFile,"</tr>\n");
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"24%%\" bgcolor=%s><font size=\"%d\">Corr. (r^2)</font></td>\n",
                    szFieldColor,iHthmSmallFontSize);
            fprintf(hReportFile,"<td width=\"76%%\" bgcolor=%s><font size=\"%d\">%.6lf</td>\n",
                    szDataColor,iHthmSmallFontSize, lfPearson*lfPearson);
            fprintf(hReportFile,"</tr>\n");
        }

        fprintf(hReportFile,"</table>\n");

        // If regular HTML page created (not Interactive Charting info).
write_image:
        if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
        {
            // Have the ToolBar line written in the HTML file
            strDrillArgument= "drill_chart=";
            strDrillArgument += szChartType;
            strDrillArgument += "--data=";
            strDrillArgument += ptTestCell->szTestLabel;

            // If advanced Histogram, the image must be added now!
            if(bAdvancedReport)
            {
                // If we're in compact layout, then we have the image + test statistics wrapped in a table with 2 horizontal cells
                if(bCompactLayout && (bBannerChart == false))
                    fprintf(hReportFile,"</td>\n<td width=\"50%%\">");

                if(m_pReportOptions->getAdvancedReport()!=GEX_ADV_FUNCTIONAL)
                    WriteHtmlToolBar(iSizeX,true,strDrillArgument);
                else
                    WriteHtmlToolBar(iSizeX,false,strDrillArgument);

                // If not compact mode...
                if(!bCompactLayout)
                {
                    // PDF file has enough room to leave space between statistics and the chart.
                    if(strOutputFormat=="PDF")
                        fprintf(hReportFile,"<br>\n");
                }

                if (bMultiChart)
                {
                    WriteHtmlOpenTable(100, 2);
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(0)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(1)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"</tr>\n");

                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(2)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"<td align=\"center\" width=\"50%%\">\n");
                    fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(lstImageName.at(3)).toLatin1().constData());
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"</tr>\n");

                    fprintf(hReportFile,"</table><br>\n");
                }
                else if (lstImageName.count() > 0)
                {
                    QString strTempBookmark = "Histo";	// Test Statistics bookmark header string.

                    if (strOutputFormat=="ODT")
                        fprintf(hReportFile,"<!-- odt --><table width=\"90%%\"><tr width2=\"99%%\"><td width2=\"99%%\">\n");
                        // odt : adding width=\"99%%\" does not work for odt writer : images disappear
                        // odt : adding width=\"640\" fit perfectly !
                     fprintf(hReportFile,
                       "<img %s  border=\"0\"  src=\"../images/%s\"> <a name=\"%sT%s\"></a> <a href=\"#StatT%s\"></a> </img>\n",
                         strOutputFormat=="ODT"?"width=\"640\"":"",
                         formatHtmlImageFilename(lstImageName.at(0)).toLatin1().constData(),
                         strTempBookmark.toLatin1().constData(),
                         ptTestCell->szTestLabel,
                         ptTestCell->szTestLabel);

                     if (strOutputFormat=="ODT")
                        fprintf(hReportFile, "<!-- odt --></td></tr></table>\n");
                }

                if(bCompactLayout && (bBannerChart == false))
                    fprintf(hReportFile,"</td>\n</tr>\n</table><br>\n");

                // Insert horizontal line if regular HTML, if flat HTML (word pdf PT report), do NOT insert such line!
                if(! ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" ) )
                    fprintf(hReportFile,"<br><hr>\n");
            }
            else
                WriteHtmlToolBar(iSizeX,true,strDrillArgument);
        }
    }

    if(pChartsInfo == NULL || pChartsInfo->bFileOutput)
    {
        // If creating PowerPoint slides, add more space between each chart
        if (strOutputFormat=="PPT")
        {
            if(bBannerChart)
                fprintf(hReportFile,"<br>\n");
            else if (getGroupsList().count() > 1)				// add more space when comparing groups
                fprintf(hReportFile,"<br><br><br><br>\n");
            else
                fprintf(hReportFile,"<br><br>\n");
        }
        else
            fprintf(hReportFile,"<br>\n");
    }
    else
    {
        // Interactive Drill HTML info window, end test info with a horizontal line (in case multiple tests are listed. eg: Scatter plot)
        fprintf(hReportFile,"<hr>\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Writes one test Histogram: Create chart + Call function to write table
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteHistoLine(CGexFileInGroup *pFile, CTest *ptTestCell,bool bPluginCall)
{
    int		iIndex;
    float fRealPercent;

    GEX_BENCHMARK_METHOD(QString("unknown"));

#if 0
    // Force Histogram to be advanced format.
    pReportOptions->bForceAdvancedHisto = true;
    pReportOptions->iAdvancedReportSettings = GEX_ADV_HISTOGRAM_OVERDATA;
#endif

    if(!bPluginCall)
    {
        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
    }

    if(m_pReportOptions->iHistogramType == GEX_HISTOGRAM_DISABLED)
        return;	// Histogram is disabled !

    QString strOutputFormat		= m_pReportOptions->GetOption("output", "format").toString();
    QString strOptionHistoYAxis = m_pReportOptions->GetOption("adv_histogram", "y_axis").toString();

    if(strOutputFormat=="CSV")
    {
        int lTotBars;
        lTotBars = ptTestCell->mHistogramData.GetTotalBars();
        if(m_pReportOptions->iGroups == 1)
        {
            // Generating .CSV report file...only if one group.
            fprintf(hReportFile,"\n,");
            // First line is cells position
            for(iIndex=0;iIndex<lTotBars;iIndex++)
            {
//                double fCell = ptTestCell->lfHistogramMin + iIndex*(ptTestCell->lfHistogramMax-ptTestCell->lfHistogramMin)/TEST_HISTOSIZE;
                fprintf(hReportFile,"%g,",ptTestCell->mHistogramData.GetClassRange(iIndex));
            }
            // Second line is : test name, then % per cell
            fprintf(hReportFile,"\n");
            fprintf(hReportFile,"T%s : %s,",ptTestCell->szTestLabel,ptTestCell->strTestName.toLatin1().constData());
            fRealPercent=0;
            for(iIndex=0;iIndex<lTotBars;iIndex++)
            {
                if((m_pReportOptions->iHistogramType == GEX_HISTOGRAM_CUMULLIMITS) || (m_pReportOptions->iHistogramType == GEX_HISTOGRAM_CUMULDATA))
                {
                    // Under cumulative histogram, add % results.
                    if(strOptionHistoYAxis == "hits")
                        fRealPercent += ptTestCell->mHistogramData.GetClassCount(iIndex);												// Y scale: Frequency count
                    else
                        fRealPercent += (100.0*ptTestCell->mHistogramData.GetClassCount(iIndex))/(ptTestCell->ldSamplesValidExecs);	// Y scale: Percentage
                }
                else
                {
                    if(strOptionHistoYAxis == "hits")
                        fRealPercent = ptTestCell->mHistogramData.GetClassCount(iIndex);												// Y scale: Frequency count
                    else
                        fRealPercent = (100.0*ptTestCell->mHistogramData.GetClassCount(iIndex))/(ptTestCell->ldSamplesValidExecs);		// Y scale: Percentage
                }

                if(strOptionHistoYAxis == "hits")
                    fprintf(hReportFile,"%f,",fRealPercent);
                else
                    fprintf(hReportFile,"%.2f%%,",fRealPercent);
            }
            fprintf(hReportFile,"\n");
        }

    }
    else
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Check if need to create new HTML Histo. page
        if(!bPluginCall)
            CheckForNewHtmlPage(NULL,SECTION_HISTO,ptTestCell->iHtmlHistoPage);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Herve Thomy : Change since case 2712, always use chart generated with ChartDirector
        //////////////////////////////////////////////////////////////////////////////////////////
        // If chart size=Auto...compute it's real size
        int iChartSize = GetChartImageSize( m_pReportOptions->GetOption("adv_histogram","chart_size").toString(),
                                           mTotalHistogramPages*MAX_HISTOGRAM_PERPAGE);

        // Image name
        QString strImage =
            BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                 "/images/his", ptTestCell, 1);

        // Full image path
        QString strImagePath = m_pReportOptions->strReportDirectory;
        strImagePath += "/images/";
        strImagePath += strImage;

        // Since we call the advanced histogram, we need to map the report type
        int iChartType;
        if(m_pReportOptions->bForceAdvancedHisto)
            iChartType = m_pReportOptions->getAdvancedReportSettings();	// User forcing to use Advanced Histo, then use AdvHisto charting style.
        else
        switch(m_pReportOptions->iHistogramType)
        {
            default:
            case	GEX_HISTOGRAM_OVERLIMITS:
                iChartType = GEX_ADV_HISTOGRAM_OVERLIMITS;
                break;
            case	GEX_HISTOGRAM_CUMULLIMITS:
                iChartType = GEX_ADV_HISTOGRAM_CUMULLIMITS;
                break;
            case	GEX_HISTOGRAM_OVERDATA:
                iChartType = GEX_ADV_HISTOGRAM_OVERDATA;
                break;
            case	GEX_HISTOGRAM_CUMULDATA:
                iChartType = GEX_ADV_HISTOGRAM_CUMULDATA;
                break;
            case	GEX_HISTOGRAM_DATALIMITS:
                iChartType = GEX_ADV_HISTOGRAM_DATALIMITS;
                break;
        }

        CreateAdvHistoChartImageEx(NULL,ptTestCell,iChartSize,false,iChartType,strImagePath);

        // Writes HTML table with global test info + chart (name, limits, Cp,cp shift,...)
        WriteHistoHtmlTestInfo(NULL,"histo",pFile,ptTestCell,NULL,NULL,true, strImage,iChartSize);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL pages for the Histogram report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Histo(qtTestListStatistics *pqtStatisticsList_Histo)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating Histograms page...");
    int					iStatus;
    BOOL				bValidSection;
    CGexFileInGroup		*pFile;

    // Generate Histogram info for each test.
    CTest *ptTestCell;	// Pointer to test cell to receive STDF info.

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Histograms...");

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    QString pf=m_pReportOptions->GetOption("output", "paper_format").toString();
    QString ps=m_pReportOptions->GetOption("output", "paper_size").toString();

    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    if(	((m_pReportOptions->iHistogramType == GEX_HISTOGRAM_DISABLED)
            && (of!="HTML")
        )
        ||
        ( m_pReportOptions->isReportOutputHtmlBased()  //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_HISTOGRAM))
        )
    {
        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
        return GS::StdLib::Stdf::NoError;
    }

    // In case no data match the filter, ensure first page is erased (hyperlink always exists on home page!)
    char	szString[2048];
    sprintf(szString,"%s/pages/histogram1.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
    unlink(szString);

    // Get pointer to first file in first group (we always have them exist)
    pFile = NULL;
    if (!getGroupsList().isEmpty())
    {
        if (!getGroupsList().first()->pFilesList.isEmpty())
            pFile = getGroupsList().first()->pFilesList.first();
    }

    // Check if the list is not empty !
    if(pqtStatisticsList_Histo->count() > 0)
        bValidSection = true;
    else
        bValidSection = false;

    iStatus = PrepareSection_Histo(bValidSection);
    if(iStatus != GS::StdLib::Stdf::NoError) return iStatus;

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	m_pReportOptions->GetOption("adv_histogram","chart_size").toString(),
                                       mTotalHistogramPages*MAX_HISTOGRAM_PERPAGE);

    // Reset HTML page counter
    iCurrentHtmlPage=0;

    int             iChartNumber	= 0;		// Keeps track of the number of chart inserted (so to insert appropriate page breaks)
    int             iTestsPerPage	= 1;
    QElapsedTimer   elapsedTimer;
    elapsedTimer.start();


    // Read list based on sorting filter (defined by user in 'Options')
    foreach(ptTestCell, *pqtStatisticsList_Histo)
    {
        // The Histogram generation may be long...so check for 'Abort' every 2 seconds.
        if(elapsedTimer.elapsed() > 2000)
        {
            // At least 2 seconds since last check...so check again now!
            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
               return GS::StdLib::Stdf::NoError;

            // Reset timer.
            elapsedTimer.restart();
        }

        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Building Histograms %1...").arg(iChartNumber) );

        switch(m_pReportOptions->iHistogramTests)
        {
            case GEX_HISTOGRAM_TOP_N_FAIL_TESTS:
                GSLOG(SYSLOG_SEV_WARNING, "GEX_HISTOGRAM_TOP_N_FAIL_TESTS : not implemented");
            case GEX_HISTOGRAM_ALL:		// All tests
            case GEX_HISTOGRAM_LIST:	// test or test range. Tests not part of the list are not in the sorting list (pqtStatisticsList_Histo)

                // Write histogram
                WriteHistoLine(pFile,ptTestCell);

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
                if ( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" )
                {
                    switch(iChartSize)
                    {
                        case GEX_CHARTSIZE_MEDIUM:
                        default:
                            // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                            RefreshPowerPointSlideName("Histogram",iChartNumber,2,ptTestCell);

                            // If PDF or Word, we support Portrait & Landscape formats
                            if(
                                    (of=="PDF") || (of=="DOC") || of=="ODT"
                               )
                            {
                                // In portrait mode: allow X medium histograms per page
                                if (pf=="portrait")
                                {
                                    // A4: allow X tests per page. Letter: allow Y tests per page.
                                    iTestsPerPage = (ps=="A4") ? 6: 5;
                                }
                                else
                                {
                                    // Landscape A4: allow X tests per page. Letter: allow Y tests per page.
                                    iTestsPerPage = (ps=="A4") ? 2: 3;
                                }
                            }
                            else
                                iTestsPerPage = 2;
                            break;

                        case GEX_CHARTSIZE_BANNER:
                            // Dynamically build the PowerPoint slide name (as name includes the 5 (or 4) tests writtent per page). Ignored if not generating a PPT file.
                            RefreshPowerPointSlideName("Histogram",iChartNumber,4,ptTestCell);

                            // If PDF or Word, we support Portrait & Landscape formats
                            if(
                                    (of=="PDF") || (of=="DOC") || of=="ODT"
                               )
                            {
                                // In portrait mode: allow 8 banner histograms per page
                                if (pf=="portrait")
                                {
                                    // A4: allow 9 tests per page. Letter: allow 8 tests per page.
                                    iTestsPerPage = (ps=="A4") ? 9: 8;
                                }
                                else
                                    iTestsPerPage = 4;	// Landscape mode: only 4 banner histograms per page.
                            }
                            else
                              iTestsPerPage = 4;
                            break;

                        case GEX_CHARTSIZE_LARGE:
                            // Dynamically build the PowerPoint slide name. Ignored if not generating a PPT file.
                            RefreshPowerPointSlideName("Histogram",1,1,ptTestCell);
                            // If PDF or Word, we support Portrait & Landscape formats
                            if(
                                    (of=="PDF") || (of=="DOC") || of=="ODT"
                               )
                            {
                                // In portrait mode: allow 2 large histograms per page
                                if (pf=="portrait")
                                {
                                    iTestsPerPage = 2;
                                }
                                else
                                    iTestsPerPage = 1;	// Landscape mode: only one large histogram per page.
                            }
                            else
                                iTestsPerPage = 1;
                            break;
                    }
                    if((iChartNumber % iTestsPerPage) == 0)
                        WritePageBreak();
                }
                else
                {
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Histogram",iChartNumber,2,ptTestCell);
                }

                break;
        }

    };	// Loop until all test cells read.

    // If at least one Histogram page was generated, close it.
    if(iCurrentHtmlPage)
        WriteNavigationButtons("histogram");
    iStatus = CloseSection_Histo(pqtStatisticsList_Histo);

    // When writing flat HTML (for Word or PDF file), insert page break (unless nothing written so far) every chart (large images) or every 2 charts (medium images)
    if(iCurrentHtmlPage && ( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" )
        )
    {
        switch(iChartSize)
        {
            case GEX_CHARTSIZE_MEDIUM:
            default:
                if(iChartNumber % iTestsPerPage)
                    WritePageBreak();
                break;

            case GEX_CHARTSIZE_LARGE:
                    WritePageBreak();
                break;
        }
    }
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "End of creating Histograms page...");
    return iStatus;
}
