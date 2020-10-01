/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Wafermap' page.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
  #include <windows.h>
#endif

#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_shared.h"
#include "gex_constants.h"			// Constants shared in modules
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "cbinning.h"
#include "report_classes_sorting.h"	// Classes to sort lists
#include "patman_lib.h"
#include "chart_director.h"
#include "pick_export_wafermap_dialog.h"
#include "gex_algorithms.h"
#include "gexperformancecounter.h"
#include "gex_algorithms.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "waf_bin_mismatch.h"
#include "csl/csl_engine.h"

#include <QPainter>

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"
#include <sstream>

// main.cpp
extern CGexReport *		gexReport;				// Handle to report class

// cstats.cpp
extern double			ScalingPower(int iPower);

// csl/ZcGexLib.cpp
extern CGexTestRange *	createTestsRange(QString strParameterList,bool bAcceptRange, bool bIsAdvancedReport);

extern QString			formatHtmlImageFilename(const QString& strImageFileName);

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

static bool	bTiledDisplay;	// set to 'true' when creating a tiled display with 4 wafers per line for quick overview.

extern bool CompareBinning(CBinning* bin1, CBinning* bin2);


/////////////////////////////////////////////////////////////////////////////
// Writes the STACKED wafermap for a given group.
/////////////////////////////////////////////////////////////////////////////
bool	CGexReport::WriteStackedWaferMap(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport,int iGroupID)
{
    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return false;

    // If no STACKED wafermap...quietly return
    if((pGroup->cStackedWaferMapData.iTotalWafermaps <= 1) || (pGroup->cStackedWaferMapData.cWafMap == NULL))
        return false;

    QStringList strLstWafermapShow = m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|");

    // If STACKED wafer is disabled, quietly return.
    if (strLstWafermapShow.contains("stacked") == false)
        return false;

    // If Examinator Characterization STD edition....do not allow this report!
    bool bShow = true;
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        bShow = false;
    }

    QString of								= m_pReportOptions->GetOption("output", "format").toString();
    QString	strWafermapBinStacked			= m_pReportOptions->GetOption("wafer","bin_stacked").toString();
    QString	strWafermapParametricStacked	= m_pReportOptions->GetOption("wafer","parametric_stacked").toString();

    if(bShow == false)
    {
        // Display message that Stacked wafer is not available under this module!
        if (of=="CSV")
        {
            fprintf(hReportFile,"Stacked wafers: Not available under this release.\n");
      QString m=m_pReportOptions->GetOption("messages", "upgrade").toString();
      fprintf(hReportFile, "%s", m.toLatin1().data());
        }
        else
        //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        if(m_pReportOptions->isReportOutputHtmlBased())
        {
            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,"all_stackedwafers","Stacked Wafer maps");
            fprintf(hReportFile,"<br>Feature not available: <br>\n");
      fprintf(hReportFile,"For this report, you need to <b>upgrade</b> your license.<br>\n");
            fprintf(hReportFile,"Contact %s for more details.<br><br>\n",GEX_EMAIL_SALES);
        }
        return true;
    }

    // Creates the 'Wafermap' page & header for given file.
    QString			strBinningListToStack;
    QString			strString;
    char			szString[2048];
    CTest *			ptTestCell;	// Pointer to test cell to receive STDF info.
    const char *	ptMessage;
    int				iDays		= 0;
    int				iHours		= 0;
    int				iMinutes	= 0;

    iDays		= (pGroup->cStackedWaferMapData.iTotaltime) / (24*3600);
    iHours		= ((pGroup->cStackedWaferMapData.iTotaltime) - iDays*(24*3600)) / 3600;
    iMinutes	= ((pGroup->cStackedWaferMapData.iTotaltime) - iDays*(24*3600) - iHours*3600)/60;

    if (of=="CSV")
    {
        fprintf(hReportFile,"Stacked wafers\n");

        // Binning list to stack
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:
                if (strWafermapBinStacked == "bin_count")
                {
                    strBinningListToStack = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("");
                    fprintf(hReportFile,
                            "Binning list to stack:,%s\n",
                            strBinningListToStack.toLatin1().constData());
                }
                break;

            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_STACK_HARDBIN:
                strBinningListToStack = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("");
                fprintf(hReportFile,
                        "Binning list to stack:,%s\n",
                        strBinningListToStack.toLatin1().constData());
                break;

            case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
            case GEX_WAFMAP_TEST_PASSFAIL:
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                long lZoningTest=-1,lZoningPinmapIndex=-1;
                if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                {
                    lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                    lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                }

                if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
                {
                    // No wafermap data available for this test
                    fprintf(hReportFile,"No Wafermap data available for parametric test: %s\n",
                            m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                    fprintf(hReportFile,"Either test missing low and high limits, or no data found (do you filter test results?)\n");
                    break;	// Error
                }
                else
                    fprintf(hReportFile,"Characterization Wafermap for test: %s\n",
                            m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS)
                    fprintf(hReportFile,"Characterization mode: Check spread over test limits\n");
                else if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL || m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL)
                    fprintf(hReportFile,"Characterization mode: Check spread test pass/fail\n");
                else
                    fprintf(hReportFile,"Characterization mode: Check spread over test values range\n");
                break;
        }

        // total wafers stacked
        fprintf(hReportFile,"Total Wafers stacked:,%d\n",pGroup->cStackedWaferMapData.iTotalWafermaps);

        // Testing time
        if(pGroup->cStackedWaferMapData.iTotaltime)
        {
            fprintf(hReportFile,"Testing time:,");
            if(iDays)
                fprintf(hReportFile,"%d days ",iDays);
            fprintf(hReportFile,"%d Hours ",iHours);
            fprintf(hReportFile,"%d Min\n",iMinutes);
        }
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // STACKED Wafermap section...
        QString strBookmarkTitle, strBookmark;

        // Standard HTML: draw horizontal line (flat HTML use page breaks instead)
        if (of=="HTML")
            fprintf(hReportFile,"<hr><br>\n");

        if(m_pReportOptions->iGroups > 1)
            strBookmarkTitle = "Stacked Wafer maps: " + pGroup->strGroupName;
        else
            strBookmarkTitle = "Stacked Wafer maps";
        strBookmark = "all_stackedwafers";

        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,strBookmark,strBookmarkTitle);

        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0

        // Binning list.
        strBinningListToStack = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("");
        strBinningListToStack += "    (for a different Bin list, see the <a href=\"_gex_file_settings.htm\">'Settings'</a> page, section 'wafermap / Stacked wafers')";
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_STACK_HARDBIN:
                if (strWafermapBinStacked == "bin_count")
                    //iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT)
                    WriteInfoLine("Binning list to stack", strBinningListToStack.toLatin1().constData());
                break;
            case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
            case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                long lZoningTest=-1,lZoningPinmapIndex=-1;
                if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                {
                    lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                    lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                }

                // Zoning wafer map on specific test.
                if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
                {
                    // Zoning wafer map: No zoning wafermap data available for this test
                    fprintf(hReportFile,
                            "<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Wafermap data available for parametric test: %s\n",
                            iHthmNormalFontSize,
                            m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                    fprintf(hReportFile,
                            "<p align=\"left\"><font color=\"#000000\" size=\"%d\">Either test doesn't have low and high limits, or no data found\n",
                            iHthmNormalFontSize);
                    break;	// Error
                }
                // Load Wafermap array with relevant data
                gexReport->FillWaferMap(pGroup,pFile,ptTestCell,m_pReportOptions->iWafermapType);

                if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS)
                    ptMessage = "(Check spread over test limits)";
                else if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL || m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL)
                    ptMessage = "(Check spread test pass/fail)";
                else
                    ptMessage = "(Check spread over test values range)";

                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s><b>Characterization of Test</b><br>%s</td>\n",
                        szFieldColor, ptMessage);

                // Bookmark: are in same page if FLAT HTML page is generated
                if (of=="HTML")
                    strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
                else
                    strBookmark = "#StatT";	// Test Statistics bookmark header string.

                                fprintf(hReportFile,"<td width=\"77%%\" bgcolor=%s><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></td>\n",szDataColor,
                    ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                fprintf(hReportFile,"</tr>\n");
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s>Name</td>\n",szFieldColor);
                fprintf(hReportFile,"<td width=\"77%%\" bgcolor=%s>%s</td>\n",szDataColor,buildDisplayName(ptTestCell).toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
                fprintf(hReportFile,"<tr>\n");
                break;
        }

        // Number of wafers.
        WriteInfoLine("Total Wafers stacked",pGroup->cStackedWaferMapData.iTotalWafermaps);

        // Total testing time.
        if(pGroup->cStackedWaferMapData.iTotaltime)
        {
            if(iDays)
                sprintf(szString,"%d days %d Hours %d Min",iDays,iHours,iMinutes);
            else
                sprintf(szString,"%d Hours %d Min",iHours,iMinutes);
            WriteInfoLine("Testing time",szString);
        }

        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n");

        // Standard HTML: skip 2 line)
        if (of=="HTML")
            fprintf(hReportFile,"<br><br>\n");

        bool bZoningScale = true;

        // Stacked Binning wafermap is using the same gadient color scale as Parametric test wafermap...
        // Show colored scale.
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_STACK_HARDBIN:
                if (strWafermapBinStacked == "bin_count")
                    //iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT)
                    fprintf(hReportFile,"Color scale: Total matching Bins stacked per die<br>\n");
                else
                    bZoningScale = false;
                break;
            case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
                if (strWafermapParametricStacked == "mean")
                    fprintf(hReportFile,"Color scale: Test/Parameter 'mean' value per stacked die<br>\n");
                else if (strWafermapParametricStacked == "median")
                    fprintf(hReportFile,"Color scale: Test/Parameter 'median' value per stacked die<br>\n");
                break;
            case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                fprintf(hReportFile,"Color scale: Test/Parameter failure count per stacked die<br>\n");
                break;
        }

        if (bZoningScale)
        {
            fprintf(hReportFile,"<img border=\"0\"  width=\"300\" src=\"../images/zoning.png\"><br>\n");
            fprintf(hReportFile,"<table border=\"0\" width=\"300\" cellspacing=\"0\">\n");
            fprintf(hReportFile,"<tr>\n");
            switch(m_pReportOptions->iWafermapType)
            {
                case GEX_WAFMAP_STACK_SOFTBIN:
                case GEX_WAFMAP_SOFTBIN:
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_HARDBIN:
                    {
                        // Few labels on scale.
                        int nTotalWidth = 0;
                        int nWidth		= 0;
                        int nValue		= 0;
                        int nNextValue	= 0;

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount / 4;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount / 2;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = 3 * pGroup->cStackedWaferMapData.iHighestDieCount/4;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = 270 - nTotalWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                        }

                        // write last value
                        fprintf(hReportFile,"<td width=\"30\" align=\"right\">%d</td>\n",pGroup->cStackedWaferMapData.iHighestDieCount);
                    }
                    break;

                case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    {
                        // Few labels on scale.
                        double lfValue;
                        double	lfWindowSize = pGroup->cStackedWaferMapData.lfHighWindow-pGroup->cStackedWaferMapData.lfLowWindow;
                        if(lfWindowSize == 0) lfWindowSize = 1e-50;
                        lfValue = pGroup->cStackedWaferMapData.lfLowWindow;
                        fprintf(hReportFile,"<td width=\"75\" align=\"left\">%s</td>\n",pFile->FormatTestResult(ptTestCell,lfValue,ptTestCell->res_scal));
                        lfValue += (lfWindowSize/4);
                        fprintf(hReportFile,"<td width=\"75\" align=\"left\">%s</td>\n",pFile->FormatTestResult(ptTestCell,lfValue,ptTestCell->res_scal));
                        lfValue += (lfWindowSize/4);
                        fprintf(hReportFile,"<td width=\"75\" align=\"left\">%s</td>\n",pFile->FormatTestResult(ptTestCell,lfValue,ptTestCell->res_scal));
                        lfValue += (lfWindowSize/4);
                        fprintf(hReportFile,"<td width=\"36\" align=\"left\">%s</td>\n",pFile->FormatTestResult(ptTestCell,lfValue,ptTestCell->res_scal));
                        lfValue = pGroup->cStackedWaferMapData.lfHighWindow;
                        fprintf(hReportFile,"<td width=\"39%%\" align=\"right\">%s</td>\n",pFile->FormatTestResult(ptTestCell,lfValue,ptTestCell->res_scal));
                    }
                break;

                case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    {
                        // Few labels on scale.
                        int nTotalWidth = 0;
                        int nWidth		= 0;
                        int nValue		= 0;
                        int nNextValue	= 0;

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount / 4;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount / 2;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = 3 * pGroup->cStackedWaferMapData.iHighestDieCount/4;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = ((300 * nNextValue) / pGroup->cStackedWaferMapData.iHighestDieCount) - nTotalWidth;
                            nTotalWidth += nWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                            nValue = nNextValue;
                        }

                        // compute next position to determinate the width
                        nNextValue = pGroup->cStackedWaferMapData.iHighestDieCount;

                        // if next value differs from the current, compute the width of the current value
                        if (nNextValue != nValue)
                        {
                            // compute next position to determinate the width
                            nWidth = 270 - nTotalWidth;
                            fprintf(hReportFile,"<td width=\"%d\" align=\"left\">%d</td>\n", nWidth, nValue);
                        }

                        // write last value
                        fprintf(hReportFile,"<td width=\"30\" align=\"right\">%d</td>\n",pGroup->cStackedWaferMapData.iHighestDieCount);
                    }
                    break;
            }

            fprintf(hReportFile,"</tr>\n");
            fprintf(hReportFile,"</table>\n");
        }
        else
        {
            int nFailCount = 0;
            int nPassCount = 0;

            // Merge this wafermap to the stacked array.
            for(int nIndex = 0; pGroup && nIndex < pGroup->cStackedWaferMapData.SizeX * pGroup->cStackedWaferMapData.SizeY; nIndex++)
            {
                // Get die value (Bin# or Parametric % value)
                if (pGroup->cStackedWaferMapData.cWafMap[nIndex].lStatus == GEX_WAFMAP_PASS_CELL)
                    nPassCount++;
                else if (pGroup->cStackedWaferMapData.cWafMap[nIndex].lStatus == GEX_WAFMAP_FAIL_CELL)
                    nFailCount++;
            }

            WritePassFailLegend(nPassCount, nFailCount);
        }

        // Show hyperlink to export compare-wafermap data
        QString	strDrillArgument;
        QString strExportWafer;
        strDrillArgument= "drill_3d=wafer_hbin";
        strDrillArgument += "--g=";
        strDrillArgument += QString::number(0);	// GroupID (0=1st group, etc...)
        strDrillArgument += "--f=";
        strDrillArgument += QString::number(-1);	// FileID = -1, means 'stacked wafer' structure to dump.
        strDrillArgument += "--stacked=";
        strDrillArgument += (bZoningScale) ? "BinCount" : "PassFailAll";
        strExportWafer = "#_gex_export_wafmap.htm#" + strDrillArgument;
        WriteHtmlToolBar(GetWaferSizeRequested(GetWaferImageSize()),false,strDrillArgument,
            "Export to file","../images/save.png",strExportWafer);
    }

    // Creates the STACKED wafermap PNG file: 'stackwaf<group#>.png'
    QString		strImageName ;
    QString		strImageFile;
    strImageName = "stackwaf";
    strImageName += QString::number(iGroupID);
    strImageName += "-T";

    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
        strImageName += QString::number(m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest);
    else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_SOFTBIN || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_SOFTBIN)
        strImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_SBIN);
    else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_HARDBIN || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_HARDBIN)
        strImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_HBIN);

    strImageName += ".png";

    strImageFile = m_pReportOptions->strReportDirectory;
    strImageFile += "/images/";
    strImageFile += strImageName;
    CreateWaferMapImage(CGexReport::stackedWafermap, pGroup, pFile, bWriteReport, strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());

    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // If STACKED wafer exists and is enabled and INDIVIDUAL wafermap disabled, show Info message on HOW to also display individual wafermaps.
        if((strLstWafermapShow.contains("all_individual") == false) && (pGroup->cStackedWaferMapData.iTotalWafermaps > 1))
            fprintf(hReportFile,"<br>Note: To see ALL individual wafermaps, check the <a href=\"_gex_options.htm\">'Options'</a> page, section 'wafermap / Wafermaps to include in report'<br>\n");
    }

    // Standard HTML: draw horizontal line (flat HTML use page breaks instead)
    if (of=="HTML")
        fprintf(hReportFile,"<br><hr><br>\n");

    ////////////////// Failing Patterns wafermap (only if stacked SOFT or HAD bins, not parametric wafermaps)
    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:
        case GEX_WAFMAP_HARDBIN:
            break;

        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_STACK_SOFTBIN:
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            return true;	// Exit now.
            break;
    }

    WritePageBreak();	// Write page break (ignored if not writing a flat HTML document)

    // Rebuild Binary wafer showing all dies with yield Below a threshold
    bool bOptionConversionRslt;
    double lfPatternYieldLevel = m_pReportOptions->GetOption("wafer","low_yield_pattern").toDouble(&bOptionConversionRslt);
    GEX_ASSERT( (bOptionConversionRslt) && !(lfPatternYieldLevel<0) && !(lfPatternYieldLevel>100) );


    WriteHtmlSectionTitle(hReportFile,"all_stacked_patterns","Stacked wafers: Low-Yield patterns");
    if (of=="CSV")
    {
        // Binning list
        fprintf(hReportFile,"Binning list to stack:,%s\n", strBinningListToStack.toLatin1().constData());
        // Number of wafers.
        fprintf(hReportFile,"Total Wafers stacked:,%d\n",pGroup->cStackedWaferMapData.iTotalWafermaps);
        // Display low yield threshold defined.
        fprintf(hReportFile,"Low-Yield pattern threshold,%g\n",lfPatternYieldLevel);
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Open table
        if (of=="HTML")
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
        else
            WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
        // Binning list.
        WriteInfoLine("Binning list to stack:", strBinningListToStack.toLatin1().constData());
        // Number of wafers.
        WriteInfoLine("Total Wafers stacked",pGroup->cStackedWaferMapData.iTotalWafermaps);
        // Display low yield threshold defined.
        strString.sprintf("%g%% ( Customizable from <a href=\"_gex_options.htm\">'Options'</a> page, section 'Wafer map, Strip map / Low-Yield Patterns detection' )",lfPatternYieldLevel);
        WriteInfoLine("Low yield pattern threshold", strString.toLatin1().constData());
    }

    // Green dies: over yield threshold
    // Blue dies: under yield threshold
    int	iIndex, iDieCount;
    int	iTotalFailBinPattern = 0;	// Use to count how many dies are failing threshold
    int	iTotalBinsStacked = 0;		// Use to count how many dies are on the resulting stacked wafer.

    // case 6410: Compute directly the yield for each die rather than a die count threshold from the low yield pattern.
    // It will avoid some rounding error unsing integer.
    double dieYield = 0.0;

    for(iIndex =0; iIndex < pGroup->cStackedWaferMapData.SizeX*pGroup->cStackedWaferMapData.SizeY; iIndex++)
    {
        // Get total number of matching dies for this cell (stacked dies)
        iDieCount = pGroup->cStackedWaferMapData.cWafMap[iIndex].ldCount;

        // If die location tested, check if over / under alarm threshold
        if(iDieCount >= 0)
        {
            // Keep track of dies detected on stacked wafer.
            iTotalBinsStacked++;

            // Compute the yield for this die
            if (pGroup->cStackedWaferMapData.iHighestDieCount)
                dieYield = (double) (iDieCount * 100.0) / pGroup->cStackedWaferMapData.iHighestDieCount;
            else
                dieYield = 0.0;

            if (dieYield < lfPatternYieldLevel)
            {
                // Force it to BLUE color
                pGroup->cStackedWaferMapData.cWafMap[iIndex].lStatus = GEX_WAFMAP_FAIL_CELL;

                // Keep track of dies under threshold
                iTotalFailBinPattern++;
            }
            else
                pGroup->cStackedWaferMapData.cWafMap[iIndex].lStatus = GEX_WAFMAP_PASS_CELL;
        }
    }

    // Compute percentage of dies under yield alarm threshold
    float fPercentage=0;
    if(iTotalBinsStacked)
        fPercentage = ((float)iTotalFailBinPattern*100.0)/(float)iTotalBinsStacked;
    strString.sprintf("%.2f %% ( %d dies over wafer of %d dies )",fPercentage,iTotalFailBinPattern,iTotalBinsStacked);
    if (of=="CSV")
    {
        // Number of low-yield dies
        strString.sprintf("%.2f %% ( %d dies over wafer of %d dies )",fPercentage,iTotalFailBinPattern,iTotalBinsStacked);
        fprintf(hReportFile,"Total Low-Yield dies,%s\n",strString.toLatin1().constData());
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Number of low-yield dies
        strString.sprintf("<b>%.2f %%</b> ( %d dies over wafer of %d dies )",fPercentage,iTotalFailBinPattern,iTotalBinsStacked);
        WriteInfoLine("Total Low-Yield dies", strString.toLatin1().constData());
        // Close table.
        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n");
        fprintf(hReportFile,"Note: <font color=\"#0000FF\"><b>Blue</b></font> dies are failing patterns (stacked bins with yield below threshold)<br><br>\n");
    }

    strImageName = "stackwaf_pattern";
    strImageName += QString::number(iGroupID);
    strImageName += "-T";

    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
        strImageName += QString::number(m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest);
    else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_SOFTBIN || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_SOFTBIN)
        strImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_SBIN);
    else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_HARDBIN || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_HARDBIN)
        strImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_HBIN);

    strImageName += ".png";

    strImageFile = m_pReportOptions->strReportDirectory;
    strImageFile += "/images/";
    strImageFile += strImageName;
    CreateWaferMapImage(CGexReport::lowYieldPattern, pGroup, pFile, bWriteReport, strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());

    // Standard HTML: draw horizontal line (flat HTML use page breaks instead)
    if (of=="HTML")
        fprintf(hReportFile,"<br><hr><br>\n");

    return true;
}

class clBinCount
{
public:
    clBinCount();			// Constructor

    long	lTotalBins;		// Total bins
};

clBinCount::clBinCount()
{
    lTotalBins = 0;
}

typedef QMap<int, clBinCount*> CountBinMap;

/////////////////////////////////////////////////////////////////////////////
// Returns pointer to the relevant bin cell
/////////////////////////////////////////////////////////////////////////////
CBinning*	CGexReport::findBinningCell(CBinning	*ptBinList,int iBinNumber)
{
    CBinning *ptBinCell=ptBinList;
    while(ptBinCell != NULL)
    {
        if(ptBinCell->iBinValue == iBinNumber)
            return ptBinCell;

        // Move to next cell
        ptBinCell=ptBinCell->ptNextBin;
    };

    // Bin not in list!
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the BINNING wafermap color legend & Yield per Bin class: for a complete group
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteBinningLegend(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport)
{
    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return;

    // This wafer must not be plotted (filtered)		(case 3935)
    if((pFile->strWaferToExtract.isEmpty() == false) &&
        (pFile->isMatchingWafer(pFile->strWaferToExtract,pFile->getWaferMapData().szWaferID) == false))
        return;

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // Only available for HTML & flat HTML
    if (!m_pReportOptions->isReportOutputHtmlBased())
        return;

    // If display of Individual wafers is disabled, do not show any legend!
    if (m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|").contains("all_individual", Qt::CaseSensitive) == false)
        return;

    CountBinMap	cCountBinMap;
    CountBinMap::Iterator it;
    clBinCount	*pBinCell;

    CBinning	*ptBinList=NULL;	// Points to the binning list to show (HARD or SOFT)
    CBinning	*ptBinCell;
    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
        case GEX_WAFMAP_STACK_SOFTBIN:
            ptBinList= pGroup->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure
            break;
        case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
        case GEX_WAFMAP_STACK_HARDBIN:
            ptBinList= pGroup->cMergedData.ptMergedHardBinList; // Points first HARD binning structure
            break;

        default:
            return;	// Parametric wafermap....no gobal scale to sho! Custom scale is per wafer.
    }

    // Scan all the wafer and count binnings!
    int	iLine,iCol,iBinCode;
    int	iTotalBins=0;
    for(iLine = 0; iLine < pFile->getWaferMapData().SizeY; iLine++)
    {
        // Processing a wafer line.
        for(iCol = 0; iCol < pFile->getWaferMapData().SizeX; iCol++)
        {
            // Get PAT-Man binning at location iRow,iCol.
            iBinCode = pFile->getWaferMapData().getWafMap()[(iCol+(iLine*pFile->getWaferMapData().SizeX))].getBin();
            switch(iBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                    break;

                default:	// Die tested, check if PASS or FAIL bin
                    if(!cCountBinMap.contains(iBinCode))
                    {
                        // Entry doesn't exist, create it & add entry to QMap!
                        pBinCell = new clBinCount;
                        cCountBinMap[iBinCode] = pBinCell;
                    }
                    else
                        pBinCell = cCountBinMap[iBinCode];
                    iTotalBins++;
                    pBinCell->lTotalBins++;
                    break;
            }
        }
    }

    // Write Bin list.
    WriteHtmlSectionTitle(hReportFile,"individual_binning","Individual Map");

    // Show List of binnings
    // Get pointer to wafermap data...if any!

    if (of=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
    else
        WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0

    /////////////////////////////////
    // Bin# list
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Bin#</b></td>\n",szFieldColor);
    for(it = cCountBinMap.begin(); it != cCountBinMap.end(); ++it)
    {
        // Get binning#
        iBinCode = it.key();
        ptBinCell = findBinningCell(ptBinList,iBinCode);
        if(ptBinCell != NULL)
            fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%d<br>%s</td>\n",szFieldColor,iBinCode,ptBinCell->strBinName.toLatin1().constData());
    }
    fprintf(hReportFile,"</tr>\n");

    /////////////////////////////////
    // Percentage in bin
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Percentage</b></td>\n",szFieldColor);
    double lfPercentage;
    for(it = cCountBinMap.begin(); it != cCountBinMap.end(); ++it)
    {
        pBinCell = *it;
        if(iTotalBins)
            lfPercentage = (double)100.0*pBinCell->lTotalBins/(double)iTotalBins;
        else
            lfPercentage = 0;
        fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
    }
    fprintf(hReportFile,"</tr>\n");

    // If gross die count ernabled, also show percentage based on gross die
    if(pFile->grossDieCount() > 0)
    {
        /////////////////////////////////
        // Percentage in bin (based on gross die count)
        /////////////////////////////////
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Percentage (based on gross die)</b></td>\n",szFieldColor);
        double lfPercentage;
        for(it = cCountBinMap.begin(); it != cCountBinMap.end(); ++it)
        {
            pBinCell = *it;
            lfPercentage = (double)100.0*pBinCell->lTotalBins/(double)pFile->grossDieCount();
            fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%.1lf %%</td>\n",szDataColor,lfPercentage);
        }
        fprintf(hReportFile,"</tr>\n");
    }

    /////////////////////////////////
    // Count in bin
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Count</b></td>\n",szFieldColor);
    for(it = cCountBinMap.begin(); it != cCountBinMap.end(); ++it)
    {
        pBinCell = *it;
        fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%ld</td>\n",szDataColor,pBinCell->lTotalBins);
    }
    fprintf(hReportFile,"</tr>\n");


    fprintf(hReportFile,"</table>\n<br>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Writes the BINNING wafermap color legend & Yield per Bin class: for a complete group
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteBinningLegend(CGexGroupOfFiles *pGroup,BOOL bWriteReport)
{
    // Report must not be created yet...then quietly return
    if(bWriteReport == false)
        return;

    // Individual wafermap disabled.
    if (m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|").contains("all_individual", Qt::CaseSensitive) == false)
        return;

    // Get pointer to wafermap data...if any!
    CBinning *		ptBinList	= NULL;	// Points to the binning list to show (HARD or SOFT)
    const char *	ptMessage	= NULL;
    long			lTotalBins	= 0;
    bool			bSoftBin	= false;

    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_ZONAL_SOFTBIN:
            ptMessage	= "Software";
            lTotalBins	= pGroup->cMergedData.lTotalSoftBins;
            ptBinList	= pGroup->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure
            bSoftBin	= true;
            break;
        case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            ptMessage = "Hardware";
            lTotalBins = pGroup->cMergedData.lTotalHardBins;
            ptBinList= pGroup->cMergedData.ptMergedHardBinList; // Points first HARD binning structure
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            return;	// Parametric wafermap....no gobal scale to sho! Custom scale is per wafer.
    }

    char		cChar;
    int			iMaxBin;
    double		fPercentage,fCumulPercentage;
    QString		strBinningBookmark;

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    if
    (
        (m_pReportOptions->isReportOutputHtmlBased())                       &&
        (	(m_pReportOptions->iWafermapType==GEX_WAFMAP_SOFTBIN)           ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_STACK_SOFTBIN)     ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_HARDBIN)           ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_STACK_HARDBIN)     ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_ZONAL_SOFTBIN)     ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_ZONAL_HARDBIN)
        )
    )
    {
        // If showing wafers in TILED mode, do not display any text, only compute color maps.
        if(bTiledDisplay == false)
        {
            QString strBookmarkTitle, strBookmark;

            fprintf(hReportFile,"<br>\n");

            if(m_pReportOptions->iGroups > 1 && (ReportOptions.getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL))
                strBookmarkTitle = "Group name: " + pGroup->strGroupName;
            else
                strBookmarkTitle = "List of Individual Maps";

            strBookmark = "all_individualwafers";

            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,strBookmark,strBookmarkTitle);

            // Show List of binnings, colors, Percentage for this group.
            // Get pointer to wafermap data...if any!

            if (of=="HTML")
            {
                fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
                strBinningBookmark.sprintf("binning.htm#%s",ptMessage);
            }
            else
            {
                WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0
                strBinningBookmark.sprintf("#%s",ptMessage);
            }
            fprintf(hReportFile,"<tr>\n");

                        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b><a name=\"Binning\"></a> <a href=\"%s\">Top 10 %s Binning</a></b></td>\n",szFieldColor,strBinningBookmark.toLatin1().constData(),ptMessage);
        }
        // Create list of Binnings so we can sort them and know the top 10 counts
        qtTestListBinning	qtBinningList;
        CBinning	*ptBinCell;	// Pointer to Bin cell

        // Create the list of Binning counts
        ptBinCell= ptBinList;
        while(ptBinCell != NULL)
        {
            qtBinningList.append(ptBinCell);
            ptBinCell = ptBinCell->ptNextBin;
        };
        // Sort list of binning
        qSort(qtBinningList.begin(), qtBinningList.end(), CompareBinning);

        // Display Top 10 biggest binnnig in list
        // If showing wafers in TILED mode, do not display any text, only compute color maps.
        if(bTiledDisplay == false)
        {
            iMaxBin = GEX_WAFMAP_BINCOLORS-2;
            QString lBinName;
            foreach(ptBinCell, qtBinningList)
            {
                if(ptBinCell->strBinName.isEmpty())
                    lBinName="";
                else
                    lBinName = ptBinCell->strBinName;

                fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%d<br>%s</td>\n",
                    szDataColor,ptBinCell->iBinValue,lBinName.toLatin1().constData());
                iMaxBin--;
                if(iMaxBin == 0)
                    break;
            };
            // If we have more than 10 binnings, add a 'Others' column.
            if(iMaxBin == 0)
                fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">Others</td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");
        }

        // Display top 10 Wafermap colors
        // If showing wafers in TILED mode, do not display any text, only compute color maps.
        if(bTiledDisplay == false)
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Color</b></td>\n",szFieldColor);
        }

        QString		strString;
        iMaxBin = GEX_WAFMAP_BINCOLORS-2;
        foreach(ptBinCell, qtBinningList)
        {
            // If showing wafers in TILED mode, do not display any text, only compute color maps.
            if(bTiledDisplay == false)
            {
                // If WORD file to be created, then duplicate the image color 4 times as the HTML to WORD conversion doesn't accept image stretching
                if (of=="DOC"||of=="ODT")
                {
                    strString = "<img height=\"21\" border=\"0\" src=\"../images/" + cDieColor.GetBinNumberImageName(ptBinCell->iBinValue, bSoftBin);
                    strString += "\">";
                    strString += strString;	// Have 2 bin images side by side
                    strString += strString;	// Have 4 bin images side by side
                }
                else
                {
                    strString = "<img width=\"60\" height=\"21\" border=\"0\" src=\"../images/" + cDieColor.GetBinNumberImageName(ptBinCell->iBinValue, bSoftBin);
                    strString += "\">";
                }
                fprintf(hReportFile,"<td align=center width=\"7%%\" height=\"21\">%s</td>\n",strString.toLatin1().constData());
            }
            iMaxBin--;
            if(iMaxBin == 0)
                break;
        };
        // If showing wafers in TILED mode, do not display any text, only compute color maps.
        if(bTiledDisplay == true)
            return;

        // If we have more than 10 binnings, add a 'Others' column.
        if(iMaxBin == 0)
            fprintf(hReportFile,"<td align=center width=\"7%%\" height=\"21\">-</td>\n");
        fprintf(hReportFile,"</tr>\n");

        // Display Top 10 binning: Pass/fail info
        ptBinCell = ptBinList;	// Points the Soft or Hard bin list
        iMaxBin = GEX_WAFMAP_BINCOLORS-2;
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Pass/Fail</b></td>\n",szFieldColor);
        foreach(ptBinCell, qtBinningList)
        {
            cChar = ptBinCell->cPassFail;
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%c</td>\n",szDataColor,cChar);

            iMaxBin--;
            if(iMaxBin == 0)
                break;
        };
        // If we have more than 10 binnings, add a 'Others' column.
        if(iMaxBin == 0)
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">-</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");

        // Top 10 binning: Percentage
        ptBinCell = ptBinList;	// Points the Soft or Hard bin list
        iMaxBin = GEX_WAFMAP_BINCOLORS-2;
        fCumulPercentage=0;	// Reset count
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Percentage</b></td>\n",szFieldColor);
        char		szPercentage[20];
        foreach(ptBinCell, qtBinningList)
        {
              if(lTotalBins)
              {
                  fPercentage = (100.0*ptBinCell->ldTotalCount)/lTotalBins;
                  fCumulPercentage += fPercentage;	// Cumulates total of percentage of first 15 bins.
                  sprintf(szPercentage,"%.1f%%",fPercentage);
              }
              else
                strcpy(szPercentage,"-");
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPercentage);
            iMaxBin--;
            if(iMaxBin == 0)
                break;
        };
        // If we have more than 10 binnings, add a 'Others' column.
        if(iMaxBin == 0)
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%.1f%%</td>\n",szDataColor,100.0-fCumulPercentage);

        // If gross die count ernabled, also show percentage based on gross die
        if(pGroup->cMergedData.grossDieCount() > 0)
        {
            /////////////////////////////////
            // Percentage in bin (based on gross die count)
            /////////////////////////////////
            // Top 10 binning: Percentage
            ptBinCell = ptBinList;	// Points the Soft or Hard bin list
            iMaxBin = GEX_WAFMAP_BINCOLORS-2;
            fCumulPercentage=0;	// Reset count
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Percentage (based on Gross Die count)</b></td>\n",szFieldColor);
            foreach(ptBinCell, qtBinningList)
            {
                fPercentage = (100.0*ptBinCell->ldTotalCount)/(double)pGroup->cMergedData.grossDieCount();
                fCumulPercentage += fPercentage;	// Cumulates total of percentage of first 15 bins.
                sprintf(szPercentage,"%.1f%%",fPercentage);
                fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPercentage);
                iMaxBin--;
                if(iMaxBin == 0)
                    break;
            };
            // If we have more than 10 binnings, add a 'Others' column.
            if(iMaxBin == 0)
                fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%.1f%%</td>\n",szDataColor,100.0-fCumulPercentage);
        }

        // Top 10 binning: Total count
        ptBinCell = ptBinList;	// Points the Soft or Hard bin list
        iMaxBin = GEX_WAFMAP_BINCOLORS-2;
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Total count</b></td>\n",szFieldColor);
        foreach(ptBinCell, qtBinningList)
        {
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,ptBinCell->ldTotalCount);
            iMaxBin--;
            if(iMaxBin == 0)
                break;
        };
        // Cumlate all bin counts not in the top 10 list.
        int	iTotalBinUnderTop10=0;
        foreach(ptBinCell, qtBinningList)
        {
            iTotalBinUnderTop10 += ptBinCell->ldTotalCount;
        };
        // If we have more than 10 binnings, add a 'Others' column.
        if(iMaxBin == 0)
            fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor,iTotalBinUnderTop10);

        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n<br>\n");
    }
    else
    if
    (
        (m_pReportOptions->isReportOutputHtmlBased())
            &&
        (
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_TESTOVERLIMITS) ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_STACK_TESTOVERLIMITS) ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_TESTOVERDATA) ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_STACK_TESTOVERDATA) ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_TEST_PASSFAIL) ||
            (m_pReportOptions->iWafermapType==GEX_WAFMAP_STACK_TEST_PASSFAIL)
        )
    )
    {
        fprintf(hReportFile,"<br>\n");
        QString strBookmarkTitle, strBookmark;

        if(m_pReportOptions->iGroups > 1)
            strBookmarkTitle = "Group name: " + pGroup->strGroupName;
        else
            strBookmarkTitle = "Individual Maps";
        strBookmark = "all_individualwafers";
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,strBookmark,strBookmarkTitle);
    }

    // If creating PowerPoint slides, then change Slide name to "Wafer Color Legends"
    SetPowerPointSlideName("Wafer Color Legends");

    // Write page break (ignored if not writing a flat HTML document)
    WritePageBreak();
}

void CGexReport::WritePassFailLegend(int nPassCount, int nFailCount)
{
    QString strString;
    QString of=m_pReportOptions->GetOption("output", "format").toString();

    if (of=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"30%%\">\n");
    else
        WriteHtmlOpenTable(30,0);	// HTML code to open table, 30%, cell spacing=0

    /////////////////////////////////
    // Header
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Status</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">Pass</td>\n", szDataColor);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">Fail</td>\n", szDataColor);
    fprintf(hReportFile,"</tr>\n");

    /////////////////////////////////
    // Pass/Fail color
    /////////////////////////////////

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td bgcolor=%s align=left width=\"30%%\" height=\"21\"><b>Color</b></td>\n",szFieldColor);

    // Pass Color
    // If WORD file to be created, then duplicate the image color 4 times as the HTML to WORD conversion doesn't accept image stretching
    if (of=="DOC"||of=="ODT")
    {
        strString = "<img height=\"21\" border=\"0\" src=\"../images/rgb_86f286.png\">";
        strString += strString;	// Have 2 bin images side by side
        strString += strString;	// Have 4 bin images side by side
    }
    else
        strString = "<img width=\"60\" height=\"21\" border=\"0\" src=\"../images/../images/rgb_86f286.png\">";

    fprintf(hReportFile,"<td bgcolor=%s align=center width=\"35%%\" height=\"21\">%s</td>\n",szDataColor, strString.toLatin1().constData());

    // >Fail Color
    // If WORD file to be created, then duplicate the image color 4 times as the HTML to WORD conversion doesn't accept image stretching
    if (of=="DOC"||of=="ODT")
    {
        strString = "<img height=\"21\" border=\"0\" src=\"../images/rgb_fc5a5a.png\">";
        strString += strString;	// Have 2 bin images side by side
        strString += strString;	// Have 4 bin images side by side
    }
    else
        strString = "<img width=\"60\" height=\"21\" border=\"0\" src=\"../images/../images/rgb_fc5a5a.png\">";

    fprintf(hReportFile,"<td bgcolor=%s align=center width=\"35%%\" height=\"21\">%s</td>\n",szDataColor, strString.toLatin1().constData());

    fprintf(hReportFile,"</tr>\n");

    /////////////////////////////////
    // Percentage
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Percentage</b></td>\n",szFieldColor);
    double lfPercentage;

    // Pass percentage
    lfPercentage = (double)100.0 * nPassCount / (double)(nPassCount + nFailCount);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%.1lf %%</td>\n", szDataColor, lfPercentage);

    // Fail percentage
    lfPercentage = (double)100.0 * nFailCount / (double)(nPassCount + nFailCount);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%.1lf %%</td>\n", szDataColor, lfPercentage);
    fprintf(hReportFile,"</tr>\n");

    /////////////////////////////////
    // Pass/Fail count
    /////////////////////////////////
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"left\"><b>Count</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor, nPassCount);
    fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor, nFailCount);
    fprintf(hReportFile,"</tr>\n");

    fprintf(hReportFile,"</table>\n<br>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Writes the BINNING wafermap color legend & Yield per Bin class: for a complete group
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteBinToBinColorCorrelationLegend()
{
    if(gexReport == NULL)
        return;

    // Tell if focus is on hard or soft bin
    bool	bSoftBin = false;

    switch(ReportOptions.iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_ZONAL_SOFTBIN:
            bSoftBin = true;
            break;

        case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Software Binning
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            bSoftBin = false;
            break;

        default:
            return;	// Parametric wafermap....no gobal scale to sho! Custom scale is per wafer.
    }

    // Fill a map containing all binning detected on wafermaps
    QMap<int, CBinning *>	mapBinCell;
    CBinning *				ptBinCell;	// Pointer to Bin cell
    CGexGroupOfFiles *		pGroup	= NULL;

    for (int nGroup = 0; nGroup < gexReport->getGroupsList().count(); nGroup++)
    {
        pGroup = gexReport->getGroupsList().at(nGroup);

        // Create the list of Binning counts
        if(bSoftBin)
            // Working on SOFT bins
            ptBinCell= pGroup->cMergedData.ptMergedSoftBinList;
        else
            // Working on HARD bins
            ptBinCell= pGroup->cMergedData.ptMergedHardBinList;

        while(ptBinCell != NULL)
        {
            // Keep only binning that have been detected
            if (ptBinCell->ldTotalCount > 0)
                mapBinCell.insert(ptBinCell->iBinValue, ptBinCell);

            ptBinCell = ptBinCell->ptNextBin;
        };
    }

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if (of=="HTML")
        fprintf(hReportFile,"<br><table border=\"0\" width=\"98%%\">\n");
    else
        WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0

    CBinningColor						cBinColor;
    QString								strString;
    QColor								cColor;
    QMap<int, CBinning *>::iterator		itBinCell;

    // Fill the binning number and name
    fprintf(hReportFile,"<tr>\n");
    if (bSoftBin)
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Software binning</b></td>\n",szFieldColor);
    else
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Hardware binning</b></td>\n",szFieldColor);

    for (itBinCell = mapBinCell.begin(); itBinCell != mapBinCell.end(); itBinCell++)
    {
        if((*itBinCell)->strBinName.isEmpty())
            strString = "";
        else
            strString = (*itBinCell)->strBinName;

        fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%d<br>%s</td>\n",	szDataColor, (*itBinCell)->iBinValue, strString.toLatin1().constData());
    };

    fprintf(hReportFile,"</tr>\n");

    // Fill the binning color
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Color</b></td>\n",szFieldColor);

    for (itBinCell = mapBinCell.begin(); itBinCell != mapBinCell.end(); itBinCell++)
    {
        if (of=="DOC"||of=="ODT")
        {
            strString = "<img height=\"21\" border=\"0\" src=\"../images/" + cDieColor.GetBinNumberImageName((*itBinCell)->iBinValue, bSoftBin);
            strString += "\">";
            strString += strString;	// Have 2 bin images side by side
            strString += strString;	// Have 4 bin images side by side
        }
        else
        {
            strString = "<img width=\"60\" height=\"21\" border=\"0\" src=\"../images/" + cDieColor.GetBinNumberImageName((*itBinCell)->iBinValue, bSoftBin);
            strString += "\">";
        }

        fprintf(hReportFile,"<td align=center width=\"7%%\" height=\"21\">%s</td>\n",strString.toLatin1().constData());
    };

    fprintf(hReportFile,"</tr>\n");

    // Fill the binning Pass/Fail status
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Pass/Fail</b></td>\n",szFieldColor);

    for (itBinCell = mapBinCell.begin(); itBinCell != mapBinCell.end(); itBinCell++)
    {
        fprintf(hReportFile,"<td width=\"7%%\" height=\"21\" bgcolor=%s align=\"center\">%c</td>\n", szDataColor, (char) (*itBinCell)->cPassFail);
    };

    fprintf(hReportFile,"</tr>\n");

    fprintf(hReportFile,"</table>\n<br>\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
bool CGexReport::WriteIndividualWafer(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport,int iGroupID,int iFileID,QString strImageName/*=""*/,bool bAllowWaferArrayReload/*=true*/)
{
    int iMean=0;

    // No report to create...only scanning wafers to build Binning summary.
    if(bWriteReport == false)
    {
        CreateWaferMapImage(CGexReport::individualWafermap, pGroup, pFile, bWriteReport, NULL, NULL);
        return false;
    }

    // This wafer must not be plotted (filtered)
    if((pFile->strWaferToExtract.isEmpty() == false) &&
        (pFile->isMatchingWafer(pFile->strWaferToExtract,pFile->getWaferMapData().szWaferID) == false))
        return false;

    // Display of individual wafers is disabled.
    if(m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|").contains("all_individual", Qt::CaseSensitive) == false)
        return false;

    // Get Visual options
    QStringList lstVisualOptions = m_pReportOptions->GetOption("wafer", "visual_options").toString().split("|");

    // Display Wafermap.
    char			szString[2048];
    const char *	ptMessage;
    const char *	ptComment;
    const char *	ptChar;
    CTest *			ptTestCell;	// Pointer to test cell to receive STDF info.
    QString			strBookmark;
    QString			strString;

    QString		of						= m_pReportOptions->GetOption("output", "format").toString();
    QStringList	strLstWafermapAlarms	= m_pReportOptions->GetOption("wafer", "alarms").toString().split("|");
    QString		strOptionPositiveX		= m_pReportOptions->GetOption("wafer", "positive_x").toString();
    QString		strOptionPositiveY		= m_pReportOptions->GetOption("wafer", "positive_y").toString();
    QString		strOptionNL				= (gexReport->getReportOptions()->GetOption("wafer", "notch_location")).toString();
    QString		strOptionDN				= (gexReport->getReportOptions()->GetOption("wafer", "default_notch")).toString();


    if (of=="CSV")
    {
        // Generating .CSV report file.
        if(pFile->getWaferMapData().bPartialWaferMap == true)
            fprintf(hReportFile,"Warning: Incomplete wafermap\n");
        if(pFile->getWaferMapData().getWafMap() == NULL)
        {
            fprintf(hReportFile,"No wafermap data !\n");
            // If database SUMMARY used instead of data samples, this could be why we didn't find data!

            if(m_pReportOptions->bSpeedUseSummaryDB)
                fprintf(hReportFile,"Possible cause: Only Database SUMMARY records were used (see 'Options' tab, section 'Speed optimization')");
        }
        else
        {
            // Wafer info.
            if(pFile->grossDieCount()<= 0)
                fprintf(hReportFile,"Total physical parts tested: %d\n",pFile->getWaferMapData().iTotalPhysicalDies);
            else
                fprintf(hReportFile,"Gross die per wafer: %d\n", pFile->grossDieCount());

            fprintf(hReportFile,"LowX  = %d\n",pFile->getWaferMapData().iLowDieX);
            fprintf(hReportFile,"HighX = %d\n",pFile->getWaferMapData().iHighDieX);
            fprintf(hReportFile,"LowY  = %d\n",pFile->getWaferMapData().iLowDieY);
            fprintf(hReportFile,"HighY = %d\n\n",pFile->getWaferMapData().iHighDieY);
            fprintf(hReportFile,"Bin1     = '.'\n");
            fprintf(hReportFile,"Bin > 15 = '+'\n");
            fprintf(hReportFile,"\nWafer Map:\n\n");
            fprintf(hReportFile,"\nMap type: %s", GetWaferMapReportType().toLatin1().constData());
            if(*pFile->getMirDatas().szPartType)
                fprintf(hReportFile,"\nProduct:%s\n",pFile->getMirDatas().szPartType);
            if(*pFile->getMirDatas().szLot)
                fprintf(hReportFile,"\nLot:%s\n",pFile->getMirDatas().szLot);
            if(*pFile->getMirDatas().szSubLot)
                fprintf(hReportFile,"\nSubLot:%s",pFile->getMirDatas().szSubLot);
            if(*pFile->getWaferMapData().szWaferID)
                fprintf(hReportFile,"\nWafer/Strip ID:%s",pFile->getWaferMapData().szWaferID);
            fprintf(hReportFile,"\nWafer/Strip size: %s",	pFile->BuildWaferMapSize(szString));
            fprintf(hReportFile,"\nDie size: %s",	pFile->BuildDieSize(szString));
            fprintf(hReportFile,"\nWafer/Strip started: %s",TimeStringUTC(pFile->getWaferMapData().lWaferStartTime));
            fprintf(hReportFile,"\nWafer/Strip ended: %s",TimeStringUTC(pFile->getWaferMapData().lWaferEndTime));
            int lWaferTestedIn;
            if(pFile->getWaferMapData().lWaferEndTime <= 0 || pFile->getWaferMapData().lWaferStartTime <= 0)
                lWaferTestedIn = -1;
            else
                lWaferTestedIn = pFile->getWaferMapData().lWaferEndTime-pFile->getWaferMapData().lWaferStartTime;
            fprintf(hReportFile,"\nWafer/Strip tested in: %s sec.\n",HMSString(lWaferTestedIn,0,szString));

            switch(m_pReportOptions->iWafermapType)
            {
                case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_ZONAL_HARDBIN:
                    CreateWaferMapImage(CGexReport::individualWafermap, pGroup, pFile, bWriteReport, NULL, NULL);
                    break;

                case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    long lZoningTest=-1,lZoningPinmapIndex=-1;
                    if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                    {
                        lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                        lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                    }

                    // Wafer map zoning
                    if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
                    {
                        // Zoning wafer map: No zoning wafermap data available for this test
                        fprintf(hReportFile,"No Wafermap data available for test: %s\n",
                                m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                        fprintf(hReportFile,"Either test missing low and high limits, or no data found (do you filter test results?)\n");
                        break;	// Error
                    }
                    else
                        fprintf(hReportFile,
                                "Characterization Wafermap for test: %s\n",
                                m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());

                    // Load Wafermap array with relevant data
                    gexReport->FillWaferMap(pGroup,pFile,ptTestCell,m_pReportOptions->iWafermapType);
                    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS)
                        fprintf(hReportFile,"Characterization mode: Check spread over test limits\n");
                    else if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL || m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL)
                        fprintf(hReportFile,"Characterization mode: Check spread test pass/fail\n");
                    else
                        fprintf(hReportFile,"Characterization mode: Check spread over test values range\n");
                    break;
            } // Switch
        }
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        bool	bCompactLayout=false;
        bool	bBinningWafermap=true;
        int		lWaferTestedIn;
        int		iChartSize;
        CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.
        long lZoningTest=-1, lZoningPinmapIndex=-1;

        // If this group holds multiple file, and its a database Query...then do not tell "no wafermap"...otherwise such message may appear many times (as many as files processed!).
        if((pGroup->pFilesList.count() > 1) && (iFileID > 1) &&
            (pFile->getWaferMapData().getWafMap() == NULL) &&
            (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
             GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() ||
             GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()))
        {
            // No wafermap for this file...and its the result of a query...do not show any message!
            return false;	// No wafermap data for this file...move on!
        }

        // Wafermap report page...: insert horizontal line between each wafer
        // (unless wafer gallery or flat HTML for Word, PDF, etc...)
        if	((bTiledDisplay == false) && (of=="HTML") )
            fprintf(hReportFile,"<hr>\n<br>\n");

        // See if we create a binning wafermap or a Parametric wafermap
        switch(m_pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_DISABLED:	// Wafermap disabled
            case GEX_WAFMAP_SOFTBIN:	// Software bin
            case GEX_WAFMAP_HARDBIN:	// Software bin
            case GEX_WAFMAP_STACK_SOFTBIN:	// Software bin (stacking wafers)
            case GEX_WAFMAP_STACK_HARDBIN:	// Hardware bin (stacking wafers)
            case GEX_WAFMAP_ZONAL_SOFTBIN:
            case GEX_WAFMAP_ZONAL_HARDBIN:
                bBinningWafermap=true;
                break;

            case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
            case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                bBinningWafermap=false;
                break;
        }

        // Show wafer details (unless display is wafers gallery)
        if(bTiledDisplay == false)
        {
            // Get the image size to create (depends on the number of images to create)
            iChartSize = GetWaferImageSize();

            switch(iChartSize)
            {
                case GEX_CHARTSIZE_SMALL_X:
                case GEX_CHARTSIZE_SMALL:
                case GEX_CHARTSIZE_MEDIUM_X:
                case GEX_CHARTSIZE_MEDIUM:
                    // 7131 : let's try the autoresize of html
                    // Do not allow compact layout if wafer is going to be big (ie: at least 50 dies in X or Y)
                    //if(gex_min(pFile->getWaferMapData().SizeX,pFile->getWaferMapData().SizeY) >= 50)
                    //    break;

                    // Force compact layout: image is in the right column of a table
                    bCompactLayout = true;
                    if(bBinningWafermap == false)
                        break;
                    // Parametric wafermap will create a 2 cells table...but later, when listing tests statistics!
                    // Small or medium Binning wafmap mages: then, create table and first cell is the wafer details,
                    // 2nd cell is the wafermap image.
                    fprintf(hReportFile,
                      "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" "
                            " bordercolor=\"#111111\" width=\"100%%\">\n");
                    fprintf(hReportFile,"<tr>");
                    // 7131 : lets try auto resize
                        //fprintf(hReportFile,"<td width=\"50%%\">");
                    fprintf(hReportFile,"<td>"); // we dont want to impose a size to the wafermap to auto resize
                    break;
                case GEX_CHARTSIZE_LARGE:
                case GEX_CHARTSIZE_LARGE_X:
                    // Large images: then, show the stats info., then next line is the big image.
                    break;
            }

            // Check if overwrite layout style: if wafer has many
            if (of=="HTML")
                fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
            else
                WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0

            // Write File name if Examinator software
            if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
                WriteWaferMapTableLine("File", pFile->strFileName.toLatin1().constData());

            // Write map type: Wafer map or Strip map
            if(pFile->getWaferMapData().bStripMap)
                WriteWaferMapTableLine("Map style","STRIP map ( parts tested are PACKAGED DEVICES! )");
            else
                WriteWaferMapTableLine("Map style","WAFER map ( parts tested are DIES )");

            // Total parts tested + bookmark to allow fast jump from TILED view (page header)
            fprintf(hReportFile,"<tr>\n");
            if(pFile->grossDieCount() <= 0)
            {
                fprintf(hReportFile,
                  "<td width=\"21%%\" bgcolor=%s size=\"%d\"><b><a name=\"waf%d-%d\">Total physical parts tested</a></b></td>\n",
                  szFieldColor,iHthmNormalFontSize,iGroupID,iFileID);
                sprintf(szString,"%d",pFile->getWaferMapData().iTotalPhysicalDies);
            }
            else
            {
                fprintf(hReportFile,
                  "<td width=\"21%%\" bgcolor=%s size=\"%d\"><b><a name=\"waf%d-%d\">Gross die per wafer</a></b></td>\n",
                  szFieldColor,iHthmNormalFontSize,iGroupID,iFileID);
                sprintf(szString,"%d", pFile->grossDieCount());
            }
            fprintf(hReportFile,"<td width=\"77%%\" bgcolor=%s size=\"%d\">%s</td>\n",szDataColor,iHthmNormalFontSize,szString);
            fprintf(hReportFile,"</tr>\n");

            // Parts to process
            WriteWaferMapTableLine("Parts processed",(pFile->BuildPartsToProcess()).toLatin1().data());

            // Testing on sites
            if(ReportOptions.getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL)
                WriteWaferMapTableLine("Data from Sites",pFile->BuildSitesToProcess().toLatin1().data());

            // if no wafermap for this file
            if(pFile->getWaferMapData().getWafMap() == NULL)
            {
                WriteWaferMapTableLine("*Warning*","No wafermap data available !");
                // If database SUMMARY used instead of data samples, this could be why we didn't find data!

                if(m_pReportOptions->bSpeedUseSummaryDB)
                    WriteWaferMapTableLine("*Possible cause*",
                      "Only Database SUMMARY records were used (see 'Options' tab, section 'Speed optimization')");

                fprintf(hReportFile,"</tr>\n");
                fprintf(hReportFile,"</table>\n");
                return true;	// No wafermap data for this file...move on!
            }

            // If creating PowerPoint slides, then change Slide name to "Wafer ID: XXXXX"
            if(pFile->getWaferMapData().bStripMap)
                strString = "Strip ID: ";
            else
                strString = "Wafer ID: ";
            strString += pFile->getWaferMapData().szWaferID;
            SetPowerPointSlideName(strString);

            WriteWaferMapTableLine("Product ",pFile->getMirDatas().szPartType);
            WriteWaferMapTableLine("Lot ",pFile->getMirDatas().szLot);
            WriteWaferMapTableLine("SubLot ",pFile->getMirDatas().szSubLot);

            // Wafer / Strip ID
            if(pFile->getWaferMapData().bStripMap)
                ptChar = "Strip ID: ";
            else
                ptChar = "Wafer ID: ";
            WriteWaferMapTableLine(ptChar,pFile->getWaferMapData().szWaferID);

            // Wafermap
            if(!pFile->getWaferMapData().bStripMap)
            {
                bool	bWaferAlarm = false;
                // GetWaferNotch() HAS TO BE CALLED TO MAKE SURE getWaferMapData().cWaferFlat_xxxx are set!!!!!
                pFile->getWaferMapData().GetWaferNotch();

                // 1. Wafer orientation options
                strString = "";

                // Wafer notch location option
                strString += "Notch=";
                if(strOptionNL == "file_or_detected")
                    strString += "File or detected flat";
                else if(strOptionNL == "file_or_default")
                    strString += "File or default";
                else if(strOptionNL == "default")
                    strString += "Default";
                else // default is detected
                    strString += "Detected flat";

                // Wafer default notch option
                strString += ", Default notch=" + strOptionDN;

                // Wafer probing directions
                strString += ", PosX=";
                if (strOptionPositiveX == "right")
                    ptChar = "Right";
                else if (strOptionPositiveX == "left")
                    ptChar = "Left";
                else
                    ptChar = "From file";

                strString += ptChar;

                strString += ", PosY=";
                if (strOptionPositiveY == "up")
                    ptChar = "Up";
                else if (strOptionPositiveY == "down")
                    ptChar = "Down";
                else
                    ptChar = "From file";

                strString += ptChar;
                WriteWaferMapTableLine("Wafer orientation options", strString.toLatin1().constData());

                // 2. Original wafer orientation from STDF file
                // Wafer Notch
                strString = "Notch=";
                ptChar = NULL;
                switch(toupper(pFile->getWaferMapData().cWaferFlat_Stdf))
                {
                    case 'U':
                        ptChar = "Up";
                        break;
                    case 'D':
                        ptChar = "Down";
                        break;
                    case 'R':
                        ptChar = "Right";
                        break;
                    case 'L':
                        ptChar = "Left";
                        break;
                    default:
                        ptChar = "n/a";
                        break;
                }
                strString += ptChar;

                // Wafer probing direction info
                strString += ", PosX=";
                switch(toupper(pFile->getWaferMapData().cPos_X))
                {
                    case 'R':
                        ptChar = "Right";
                        break;
                    case 'L':
                        ptChar = "Left";
                        break;
                    default:
                        ptChar = "n/a";
                }
                strString += ptChar;
                strString += ", PosY=";
                switch(toupper(pFile->getWaferMapData().cPos_Y))
                {
                    case 'U':
                        ptChar = "Up";
                        break;
                    case 'D':
                        ptChar = "Down";
                        break;
                    default:
                        ptChar = "n/a";
                }
                strString += ptChar;
                strString += ", CenterDie=";
                if(pFile->getWaferMapData().GetCenterDie().IsValid())
                {
                    strString += "(" + QString::number(pFile->getWaferMapData().GetCenterDie().GetX());
                    strString += "," + QString::number(pFile->getWaferMapData().GetCenterDie().GetY()) + ")";
                }
                else
                    strString += "n/a";
                WriteWaferMapTableLine("Wafer orientation (from data file)", strString.toLatin1().constData());

                // 3. Wafer notch location
                // Flat/Notch from file
                strString = "From file=";
                switch(toupper(pFile->getWaferMapData().cWaferFlat_Stdf))
                {
                    case 'U':
                        strString += "Up";
                        break;

                    case 'D':
                        strString += "Down";
                        break;

                    case 'R':
                        strString += "Right";
                        break;

                    case 'L':
                        strString += "Left";
                        break;
                    default:
                        strString += "n/a";
                        break;
                }

                // Detected flat
                strString += ", Detected flat=";
                switch(toupper(pFile->getWaferMapData().cWaferFlat_Detected))
                {
                    case 'U':
                        strString += "Up";
                        break;

                    case 'D':
                        strString += "Down";
                        break;

                    case 'R':
                        strString += "Right";
                        break;

                    case 'L':
                        strString += "Left";
                        break;
                    default:
                        strString += "n/a";
                        break;
                }

                // Active notch
                strString += ", Active notch=";
                switch(toupper(pFile->getWaferMapData().cWaferFlat_Active))
                {
                    case 'U':
                        strString += "Up";
                        break;

                    case 'D':
                        strString += "Down";
                        break;

                    case 'R':
                        strString += "Right";
                        break;

                    case 'L':
                        strString += "Left";
                        break;
                    default:
                        strString += "n/a";
                        break;
                }

                if (strLstWafermapAlarms.contains("flat_differs")
                    && (pFile->getWaferMapData().isValidWaferFlatOrigin() == false
                        || toupper(pFile->getWaferMapData().cWaferFlat_Stdf) != toupper(pFile->getWaferMapData().cWaferFlat_Active)))
                    bWaferAlarm = true;
                else if (strLstWafermapAlarms.contains("no_flat") && pFile->getWaferMapData().isValidWaferFlatOrigin() == false)
                    bWaferAlarm = true;

                WriteWaferMapTableLine("Wafer notch location", strString.toLatin1().constData(), bWaferAlarm);

                // 4. Wafer visual options
                // Mirror X
                strString = "MirrorX=";
                if (lstVisualOptions.contains("mirror_x"))
                    strString += "yes";
                else
                    strString += "no";
                strString += ", MirrorY=";
                if (lstVisualOptions.contains("mirror_y"))
                    strString += "yes";
                else
                    strString += "no";
                WriteWaferMapTableLine("Wafer visual options", strString.toLatin1().constData());

                WriteWaferMapTableLine("Wafer size",pFile->BuildWaferMapSize(szString));
                WriteWaferMapTableLine("Die size",	pFile->BuildDieSize(szString));
            }

            // Wafer / Strip start time
            if(pFile->getWaferMapData().bStripMap)
                ptChar = "Strip started";
            else
                ptChar = "Wafer started";
            WriteWaferMapTableLine(ptChar,TimeStringUTC(pFile->getWaferMapData().lWaferStartTime));
            if(pFile->getWaferMapData().lWaferStartTime < pFile->getWaferMapData().lWaferEndTime)
            {
                // If execution time  > 0, give details about testing period & time per device

                // Wafer / Strip start time
                if(pFile->getWaferMapData().bStripMap)
                    ptChar = "Strip ended";
                else
                    ptChar = "Wafer ended";
                WriteWaferMapTableLine(ptChar,TimeStringUTC(pFile->getWaferMapData().lWaferEndTime));
                if(pFile->getWaferMapData().lWaferEndTime <= 0 || pFile->getWaferMapData().lWaferStartTime <= 0)
                    lWaferTestedIn = -1;
                else
                    lWaferTestedIn = pFile->getWaferMapData().lWaferEndTime-pFile->getWaferMapData().lWaferStartTime;
                WriteWaferMapTableLine("Wafer tested in",HMSString(lWaferTestedIn,0,szString));
                float fData = (float) (pFile->getWaferMapData().lWaferEndTime-pFile->getWaferMapData().lWaferStartTime) / pFile->getPcrDatas().lPartCount;
                if((lWaferTestedIn >= 0) && (pFile->getPcrDatas().lPartCount > 0))
                    sprintf(szString,"%.3f sec.",fData);
                else
                    strcpy(szString,GEX_NA);
                WriteWaferMapTableLine("Average device test time",szString);
            }

            // Wafermap size.
            sprintf(szString,"LowX=%d, LowY=%d, HighX=%d, HighY=%d",pFile->getWaferMapData().iLowDieX,pFile->getWaferMapData().iLowDieY,pFile->getWaferMapData().iHighDieX,pFile->getWaferMapData().iHighDieY);
            WriteWaferMapTableLine("Map dimensions",szString);
            if(pFile->iProcessBins)//if(m_pReportOptions->iProcessBins)
                WriteWaferMapTableLine("*Warning*","Map may be incorrect because your settings do not show to process 'all parts'");

            fprintf(hReportFile,"</table>\n");

            QString pf=m_pReportOptions->GetOption("output", "paper_format").toString();
            // If not HTML report (PPT, Word, PDF)
            if ( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" )
            {
                // If NOT Compact layout (we have a very big wafermap)
                // OR PPT output format defined
                // OR Landscape orientation defined
                // then insert a page break.
                if (!bCompactLayout)
                    WritePageBreak();
                else if (
                            (
                                (of=="PPT")
                                || (pf!="portrait") )
                        && !bBinningWafermap
                        )
                    WritePageBreak();
            }

            // Compact layout for a Binning wafermap: now we go to the second cell of the table that will hold the wafermap image
            if(bCompactLayout && bBinningWafermap)
                // 7131 :
                //fprintf(hReportFile,"</td>\n<td width=\"50%%\" align=\"left\" valign=\"top\">");
                fprintf(hReportFile,"</td>\n<td align=\"left\" valign=\"top\">");
            else
                fprintf(hReportFile,"<br>\n");	// else, simply skip a line.

            switch(m_pReportOptions->iWafermapType)
            {
                case GEX_WAFMAP_DISABLED:  // Wafermap disabled
                    break;
                case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                    {
                        lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                        lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                    }

                    // Zoning wafer map on specific test.
                    if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
                    {
                        // Zoning wafer map: No zoning wafermap data available for this test
                        fprintf(hReportFile,
                                "<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Wafermap data available for parametric test: %s\n",
                                iHthmNormalFontSize,
                                m_pReportOptions->pGexWafermapRangeList->BuildTestListString("").toLatin1().constData());
                        fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">Either test doesn't have low and high limits, or no data found\n",iHthmNormalFontSize);
                        break;	// Error
                    }
                    // Load Wafermap array with relevant data
                    gexReport->FillWaferMap(pGroup,pFile,ptTestCell,m_pReportOptions->iWafermapType);

                    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS
                            || m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS)
                        ptMessage = "(Check spread over test limits)";
                    else
                        ptMessage = "(Check spread over test values range)";

                    // If standard HTML, write horizontal line.
                    if (of=="HTML")
                        fprintf(hReportFile,"<br><hr><br>\n");

                    // Small or medium Parametric wafmap images:
                    // then, create table and first cell is the wafer details, 2nd cell is the wafermap image.
                    if(bCompactLayout)
                    {
                        fprintf(hReportFile,
                          "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"100%%\">\n");
                        fprintf(hReportFile,"<tr>");
                        // 7131 : let s try a smart layout
                        //fprintf(hReportFile,"<td width=\"50%%\" align=\"left\" valign=\"top\">");
                        fprintf(hReportFile,"<td align=\"center\" valign=\"top\">");
                    }

                    // Bookmark: are in same page if FLAT HTML page is generated
                    if (of=="HTML")
                        strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
                    else
                        strBookmark = "#StatT";	// Test Statistics bookmark header string.

                    // 7131 : let s try to let html decide width
                    //fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
                    fprintf(hReportFile,"<table align=\"center\" border=\"0\">\n");
                    fprintf(hReportFile," <tr>\n");
                    // 7131
                    fprintf(hReportFile,"  <td width=\"\" bgcolor=%s><b>Characterization of Test</b><br>%s</td>\n",
                      szFieldColor,ptMessage);
                    fprintf(hReportFile,"  <td width=\"\" bgcolor=%s><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></td>\n",
                      szDataColor,
                      ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),
                      ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                    fprintf(hReportFile," </tr>\n");
                    fprintf(hReportFile," <tr>\n");
                    fprintf(hReportFile,"  <td width=\"\" bgcolor=%s>Name</td>\n",szFieldColor);
                    fprintf(hReportFile,"  <td width=\"\" bgcolor=%s>%s</td>\n",
                      szDataColor,buildDisplayName(ptTestCell).toLatin1().constData());
                    fprintf(hReportFile," </tr>\n");
                    fprintf(hReportFile," <tr>\n");

                    // If the limit is custom (from a 'what-if', then changed its background color.
                    // // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
                    if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
                    {
                        ptChar = szDataColor;	// Standard color (not a custom limit)
                        ptComment = "";
                    }
                    else
                    {
                        ptChar = szDrillColorIf;// Custom color (custom limit from 'what-if')
                        ptComment = "(custom limit - Guard Banding simulation)";
                    }
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Low L.</td>\n",szFieldColor);
                    fprintf(hReportFile,
                            "<td width=\"\" bgcolor=%s>%s    %s</td>\n",
                            ptChar,
                            ptTestCell->GetCurrentLimitItem()->szLowL,
                            ptComment);
                    fprintf(hReportFile,"</tr>\n");

                    fprintf(hReportFile,"<tr>\n");
                    if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
                    {
                        ptChar = szDataColor;	// Standard color (not a custom limit)
                        ptComment = "";
                    }
                    else
                    {
                        ptChar = szDrillColorIf;// Custom color (custom limit from 'what-if')
                        ptComment = "(custom limit - Guard Banding simulation)";
                    }
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>High Limit</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s    %s</td>\n",ptChar,
                            ptTestCell->GetCurrentLimitItem()->szHighL,ptComment);
                    fprintf(hReportFile,"</tr>\n");

                    // Mean value
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Mean</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>",szDataColor,
                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal));
                    fprintf(hReportFile,"</tr>\n");

                    // Minimum
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Min.</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>",szDataColor,
                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfMin,ptTestCell->res_scal));
                    fprintf(hReportFile,"</tr>\n");

                    // Maximum
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Max.</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>",szDataColor,
                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfMax,ptTestCell->res_scal));
                    fprintf(hReportFile,"</tr>\n");

                    // Sigma
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Sigma</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>",szDataColor,
                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal));
                    fprintf(hReportFile,"</tr>\n");

                    // Range
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Range</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>",szDataColor,
                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfRange,ptTestCell->res_scal));
                    fprintf(hReportFile,"</tr>\n");

                    // Cp
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Cp</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>\n",szDataColor,
                            CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
                    fprintf(hReportFile,"</tr>\n");

                    // Cpk
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>Cpk</td>\n",szFieldColor);
                    fprintf(hReportFile,"<td width=\"\" bgcolor=%s>%s</td>\n",szDataColor,
                            CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
                    fprintf(hReportFile,"</tr>\n");
                    fprintf(hReportFile,"</table>\n");

                    // If we're in compact layout of a Patrametric wafermap,
                    // then we have the image + wafer info wrapped in a table with 2 horizontal cells
                    if(bCompactLayout)
                        // 7131
                        //fprintf(hReportFile,"<td width=\"50%%\">\n");
                        fprintf(hReportFile,"<td>\n");
                    else
                    {
                        fprintf(hReportFile,"<p align=\"left\"></p>\n");	// Skip one line.

                        // If Parametric wafermap for Word or PDF file,
                        // make sure the colored legend is on the same page as the image (if big or medium)
                        if (of=="PDF"||of=="DOC"||of=="ODT")
                        {
                            switch(iChartSize)
                            {
                                case GEX_CHARTSIZE_SMALL_X:
                                case GEX_CHARTSIZE_SMALL:
                                    break;
                                case GEX_CHARTSIZE_MEDIUM_X:
                                case GEX_CHARTSIZE_MEDIUM:
                                case GEX_CHARTSIZE_LARGE:
                                case GEX_CHARTSIZE_LARGE_X:
                                    // Medium or large image: need to skip a page.
                                    WritePageBreak();
                                    break;
                            }

                        }
                    }

                    // right table
                    fprintf(hReportFile,"<table border=\"0\" width=\"300\" cellspacing=\"0\">\n");
                    fprintf(hReportFile,"<tr>\n");
                    switch(m_pReportOptions->iWafermapType)
                    {
                        case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
                        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) // Low limit exists
                                fprintf(hReportFile,"<td width=\"150\" align=\"left\">Low Limit: %s</td>\n",
                                        ptTestCell->GetCurrentLimitItem()->szLowL);
                            else
                                fprintf(hReportFile,"<td width=\"150\" align=\"left\">Min. Value: %s</td>\n",
                                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfMin,ptTestCell->res_scal));
                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) // High limit exists
                                fprintf(hReportFile,"<td width=\"150\" align=\"right\">High Limit: %s</td>\n",
                                        ptTestCell->GetCurrentLimitItem()->szHighL);
                            else
                                fprintf(hReportFile,"<td width=\"150\" align=\"right\">Max. Value: %s</td>\n",
                                        pFile->FormatTestResult(ptTestCell,ptTestCell->lfMax,ptTestCell->res_scal));
                            break;
                        case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
                        case GEX_WAFMAP_STACK_TESTOVERDATA:
                            fprintf(hReportFile,"<td width=\"150\" align=\"left\">Min. Value: %s</td>\n",
                                pFile->FormatTestResult(ptTestCell,ptTestCell->lfMin,ptTestCell->res_scal));
                            fprintf(hReportFile,"<td width=\"150\" align=\"right\">Max. Value: %s</td>\n",
                                pFile->FormatTestResult(ptTestCell,ptTestCell->lfMax,ptTestCell->res_scal));
                            break;
                    }
                    fprintf(hReportFile,"</tr>\n");
                    fprintf(hReportFile,"</table>\n");

                    fprintf(hReportFile,"<table border=\"0\" width=\"300\" cellspacing=\"0\">\n");
                    fprintf(hReportFile," <tr>\n");
                    fprintf(hReportFile,"  <td><img border=\"0\" src=\"../images/zoning.png\"></td>\n");
                    fprintf(hReportFile," </tr>\n");
                    fprintf(hReportFile,"</table>\n");

                    // Compute Mean position...so we display cursor at correct position.
                    switch(m_pReportOptions->iWafermapType)
                    {
                        case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
                        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                            double	fLowLimit,fHighLimit;

                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) // Low limit exists
                                fLowLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
                            else
                                fLowLimit = ptTestCell->lfMin;

                            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) // High limit exists
                                fHighLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
                            else
                                fHighLimit = ptTestCell->lfMax;
                            if(fHighLimit != fLowLimit)
                                iMean = (int)( (100.0*(ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs - fLowLimit)) / (fHighLimit - fLowLimit));
                            else
                                iMean = 0;
                            break;
                        case GEX_WAFMAP_TESTOVERDATA:// Zoning on test values range
                        case GEX_WAFMAP_STACK_TESTOVERDATA:
                            iMean = (int)( (100.0*(ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs - ptTestCell->lfMin)) / (ptTestCell->lfMax - ptTestCell->lfMin));
                            break;
                    }

                    char *ptMeanCursor;	// Points to cusor name to display...
                    if(iMean < 0)
                    {
                        ptMeanCursor = GEX_T("lcursor.png");	// Mean less than LowLimit !!!
                        iMean = 0;
                    }
                    else
                    if(iMean > 100)
                    {
                        ptMeanCursor = GEX_T("rcursor.png");	// Mean less than LowLimit !!!
                        iMean = 100;
                    }
                    else
                        ptMeanCursor = GEX_T("upcursor.png");	// Mean is valid, use default cursor!!!

                    int	iLeft,iRight;
                    char	*ptAlign;

                    // Ensure we do not reach either [0-100%] limits....HTML doesn't like it !
                    if(iMean < 1)
                        iMean = 1;
                    else
                    if(iMean > 99)
                        iMean = 99;

                    if(iMean <= 50)
                    {
                        iLeft  = iMean;
                        iRight = 100-iMean-20;
                        ptAlign = GEX_T("left");
                    }
                    else
                    {
                        iLeft  = iMean-20;;
                        iRight = 100-iMean;
                        ptAlign = GEX_T("right");
                    }
                    fprintf(hReportFile,"<table border=\"0\" width=\"300\" cellspacing=\"0\">\n");
                    fprintf(hReportFile," <tr>\n");
                    fprintf(hReportFile,
                      "  <td width=\"%d\"><img border=\"0\" src=\"../images/binff.png\" width=\"%d\" height=\"12\"></td>\n",
                      iLeft,3*iLeft);
                    fprintf(hReportFile,"  <td width=\"50\"><p align=\"%s\"><img src=\"../images/%s\"><br>\n",
                      ptAlign, formatHtmlImageFilename(ptMeanCursor).toLatin1().constData());
                    fprintf(hReportFile,"Mean</td>\n");

                    fprintf(hReportFile,"  <td width=\"%d\" align=\"right\"></td>\n",3*iRight);
                    fprintf(hReportFile," </tr>\n");
                    fprintf(hReportFile,"</table>\n");
                    // Skip one line.
                    fprintf(hReportFile,"<p align=\"left\"></p>\n");
                    break;

                case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    {
                        int nFailCount = 0;
                        int nPassCount = 0;

                        // Merge this wafermap to the stacked array.
                        for(int nIndex = 0; pFile && nIndex < pFile->getWaferMapData().SizeX * pFile->getWaferMapData().SizeY; nIndex++)
                        {
                            // Get die value (Bin# or Parametric % value)
                            if (pFile->getWaferMapData().getWafMap()[nIndex].getBin() == GEX_WAFMAP_FAIL_CELL)
                                nFailCount++;
                            else if (pFile->getWaferMapData().getWafMap()[nIndex].getBin() == GEX_WAFMAP_PASS_CELL)
                                nPassCount++;
                        }

                        WritePassFailLegend(nPassCount, nFailCount);
                    }
                    break;

                case GEX_WAFMAP_SOFTBIN:		// Standard Wafer map: Software Binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                case GEX_WAFMAP_HARDBIN:		// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_ZONAL_HARDBIN:
                    break;
            }	// end switch.

            QString strDrillDownType;
            switch(m_pReportOptions->iWafermapType)
            {
                case GEX_WAFMAP_DISABLED:
                    break;

                case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_STACK_SOFTBIN:
                    strDrillDownType ="_sbin";
                    break;
                case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_ZONAL_HARDBIN:
                case GEX_WAFMAP_STACK_HARDBIN:
                    strDrillDownType ="_hbin";
                    break;
                case GEX_WAFMAP_TESTOVERLIMITS:
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                    strDrillDownType = "_param_limits";
                    break;
                case GEX_WAFMAP_TESTOVERDATA:
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    strDrillDownType = "_param_range";
                    break;
                case GEX_WAFMAP_TEST_PASSFAIL:
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    strDrillDownType = "_test_passfail";
                    break;
            }

            // Have the ToolBar line written in the HTML file
            switch(m_pReportOptions->iWafermapType)
            {
                case GEX_WAFMAP_DISABLED:
                    break;

                case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_TESTOVERLIMITS:
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                case GEX_WAFMAP_TEST_PASSFAIL:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_ZONAL_HARDBIN:
                    QString	strDrillArgument;
                    QString strExportWafer;
                    strDrillArgument= "drill_3d=wafer" + strDrillDownType;
                    strDrillArgument += "--g=";
                    strDrillArgument += QString::number(iGroupID-1);	// GroupID (0=1st group, etc...)
                    strDrillArgument += "--f=";
                    strDrillArgument += QString::number(iFileID-1);	// FileID (0=1st file, etc...)
                    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS ||
                        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
                        m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA ||
                        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
                        m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL ||
                        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                    {
                        // Parametric wafermap: include test# and pinmap#
                        long lZoningTest=-1, lZoningPinmapIndex=-1;
                        if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                        {
                            lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                            lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                        }

                        if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) ==1)
                        {
                            strDrillArgument += "--Test=";
                            strDrillArgument += QString::number(ptTestCell->lTestNumber);	// Test#
                            strDrillArgument += "--Pinmap=";
                            strDrillArgument += QString::number(ptTestCell->lPinmapIndex);	// Pinmap#
                        }
                    }
                    strExportWafer = "#_gex_export_wafmap.htm#" + strDrillArgument;
                    WriteHtmlToolBar(GetWaferSizeRequested(iChartSize),true,strDrillArgument,
                        "Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm",
                        "Export to file","../images/save.png",strExportWafer);
                    break;
            }
        }	// Displaying details about wafer (size, lot name, etc...)

        // Creates the wafermap PNG file: 'waf<group#>-<File#>-<Test#>.png'
        QString		strWaferImageName ;
        QString		strImageFile;
        if(strImageName.isEmpty())
        {
            strWaferImageName = "waf";
            strWaferImageName += QString::number(iGroupID);
            strWaferImageName += "-";
            strWaferImageName += QString::number(iFileID);
            strWaferImageName += "-T";

            if(m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS
                    || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
                m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA
                    ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
                m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL
                    ||	m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                strWaferImageName += QString::number(m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest);
            else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_SOFTBIN
                     || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_SOFTBIN)
                strWaferImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_SBIN);
            else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_HARDBIN
                     || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_HARDBIN)
                strWaferImageName += QString::number(GEX_TESTNBR_OFFSET_EXT_HBIN);

            if(bTiledDisplay)
                strWaferImageName += "-tiled";
            strWaferImageName += ".png";
        }
        else
            strWaferImageName = strImageName;

        strImageFile = m_pReportOptions->strReportDirectory;
        strImageFile += "/images/";
        strImageFile += strWaferImageName;

        // Tiled image is a hyperlink to the full sized wafer
        if(bTiledDisplay == true)
        {
            fprintf(hReportFile,"<a href=\"#waf%d-%d\">",iGroupID,iFileID);
        }

        // will actually create and write the WaferMap into the report if desired
        CreateWaferMapImage(CGexReport::individualWafermap, pGroup, pFile, bWriteReport,
           strImageFile.toLatin1().constData(),
           strWaferImageName.toLatin1().constData(), false, bAllowWaferArrayReload);

        if(bTiledDisplay == false)
        {
            // If we're in compact layout of a Binning or Parametric wafermap,
            // then we have the image + wafer info wrapped in a table with 2 horizontal cells
            if(bCompactLayout)
                fprintf(hReportFile,"</td>\n</tr>\n</table><br>\n");
        }
        else
        {
            // Tiled display: then show WaferID + hyperlink to FULL size wafer image.
            QString strWafer;
            if(*pFile->getMirDatas().szLot)
                strWafer = "Lot: " + QString(pFile->getMirDatas().szLot);
            if(*pFile->getMirDatas().szSubLot)
                strWafer += " SubLot: " + QString(pFile->getMirDatas().szSubLot);
            if(*pFile->getWaferMapData().szWaferID)
                strWafer += "<br> Wafer ID: " + QString(pFile->getWaferMapData().szWaferID);
            // In case no WaferID available, make sure the string to display is not empty! (would crash under Unix)
            if(strWafer.isEmpty())
                strWafer = "Wafer";
            fprintf(hReportFile,"%s</a>",strWafer.toLatin1().constData());
        }
    }

    // Wafermap page has some info (not empty)
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the wafermaps tables and charts
// Note: Wafermap image name can be specified if only one wafer to be create at
// a time (when caller is MyReport routines)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::WriteWaferMapCharts(BOOL bWriteReport,QString strImageName/*=""*/)
{
    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // If wafermap report disabled, quietly return.
    if(m_pReportOptions->iWafermapType == GEX_WAFMAP_DISABLED)
        return GS::StdLib::Stdf::NoError;

    int					iGroupID	= 1;
    int					iFileID		= 1;
    CGexGroupOfFiles *	pGroup		= NULL;
    CGexFileInGroup *	pFile		= NULL;

    QString		strOuputFormat		= m_pReportOptions->GetOption("output", "format").toString();
    QStringList	strLstWaferMapShow	= m_pReportOptions->GetOption("wafer", "chart_show").toString().split("|");

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // If multiple wafers, then first show the Tiled version (4 wafers per line) with hyperlinks
    // to full sized wafers.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    int		iWaferCol;
    bool	bOpenLine=false;

    // Iterator on Groups list
    QListIterator<CGexGroupOfFiles*> itGroupsList(getGroupsList());

    if(strLstWaferMapShow.contains("all_individual") && (m_pReportOptions->lAdvancedHtmlPages > 1))
    {
        // Entering in TILED display section : Wafer map Gallery
        bTiledDisplay = true;

        // section Title + bookmark
        if(bWriteReport)
            WriteHtmlSectionTitle(hReportFile,"wafmap_gallery","Wafer maps gallery");

        // Loop at ALL groups
        while(itGroupsList.hasNext())
        {
            pGroup = itGroupsList.next();

            // Init variables
            iWaferCol=0;
            iFileID=1;

            // Open table to list all wafers found in this group:
            if(bWriteReport)
            {
                if (strOuputFormat == "HTML")
                    fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
                else
                    WriteHtmlOpenTable(98,0);	// HTML code to open table, 98%, cell spacing=0
            }

            // Initializes Binning colors.
            WriteBinningLegend(pGroup,bWriteReport);

            // Loop at Files in group groups
            QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

            while(itFilesList.hasNext())
            {
                pFile = itFilesList.next();

                // if first wafer in line, open table line
                if(bWriteReport && ((iWaferCol % 4) == 0))
                {
                    fprintf(hReportFile,"<tr>\n");

                    // Keep track that we have a table line currently opened...
                    bOpenLine = true;
                }

                // If this wafer to be plotted...
                if((pFile->strWaferToExtract.isEmpty() == true) ||
                (pFile->isMatchingWafer(pFile->strWaferToExtract,pFile->getWaferMapData().szWaferID) == true))
                {
                    // Display individual wafermaps in TILED format.
                    if(bWriteReport)
                        fprintf(hReportFile,"<td width=\"20%%\" align=\"center\">");
                    WriteIndividualWafer(pGroup,pFile,bWriteReport,iGroupID,iFileID);
                    iWaferCol++;
                    if(bWriteReport)
                        fprintf(hReportFile,"</td>\n");
                }

                // We allow 4 wafermaps per line in Tile mode.
                if(bWriteReport && ((iWaferCol % 4) == 0))
                {
                    fprintf(hReportFile,"</tr>\n");

                    // Keep track that we have closed the table line
                    bOpenLine = true;
                }
                if( iWaferCol && (iWaferCol % 8 ) == 0){
                    if (strOuputFormat == "PPT"){
                        if(bWriteReport){
                           if(bOpenLine)
                                fprintf(hReportFile,"</tr>\n");
                          fprintf(hReportFile,"</table>\n");
                          WritePageBreak();
                        }
                    }
                }

                iFileID++;
            };

            // Close table used for this group
            if(bWriteReport)
            {
                // Check if table line is not closed yet...and do it if needed!
                if(bOpenLine)
                    fprintf(hReportFile,"</tr>\n");

                // Close table
                fprintf(hReportFile,"</table>\n");
            }

            iGroupID++;
        };

        // Write page break (ignored if not writing a flat HTML document)
        if(bWriteReport)
            WritePageBreak();
    }


    // Clear flag as we enter into normal wafer display (full sized).
    bTiledDisplay = false;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Now , If comparing datasets with one wafermap in each dataset, then show Differential wafermap, if option enabled!
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    if ( strLstWaferMapShow.contains(QString("bin_mismatch"))|| strLstWaferMapShow.contains(QString("bin_to_bin")) )
    {
        if(WriteCompareWaferMap(bWriteReport))
            WritePageBreak();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Now , show each individual wafer in its default size.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    int iTotalWafersInGroup=0;
    int	iTotalWafersInFile=0;

    iGroupID = 1;
    itGroupsList.toFront();

    // If PAT-Man report with multi-sites: do not show individual wafers, only show combined one (all sites merged)
    if((getGroupsList().count() > 1) && (m_pReportOptions->getAdvancedReport() == GEX_ADV_OUTLIER_REMOVAL))
        itGroupsList.toBack();

    while(itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();

        // Rebuild the stacked wafer map
        pGroup->BuildStackedWaferMap(m_pReportOptions);

        // Get pointer to first file in group
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        iFileID=1;

        // Display Zonal report (if requested)
        if(WriteZonalWaferMap(pGroup,pFile,bWriteReport,iGroupID,strImageName))
            WritePageBreak();

        // If STACKED wafer is enabled, draw it.
        if (strLstWaferMapShow.contains("stacked"))
        {
            // If multiple wafers in this group, first show the Stacked wafer
            if(WriteStackedWaferMap(pGroup,pFile,bWriteReport,iGroupID))
                WritePageBreak();
        }

        // If BINNING wafermap, write Bin colors legend & Yield per Bin.
        WriteBinningLegend(pGroup,bWriteReport);

        // Loop at Files in group groups
        iTotalWafersInGroup = 0;

        // Loop at Files in group groups
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while(itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            // Write individual wafer binning list.
            if(bWriteReport && pGroup->pFilesList.count() > 1)
                WriteBinningLegend(pGroup,pFile,bWriteReport);

            // Display individual wafermaps.
            if(WriteIndividualWafer(pGroup,pFile,bWriteReport,iGroupID,iFileID,strImageName))
            {
                iTotalWafersInGroup++;
                if(bWriteReport)
                    WritePageBreak();
            }

            iFileID++;
        }

        // Keep track of total wafers found
        iTotalWafersInFile += iTotalWafersInGroup;

        // Write page break between groups (ignored if not writing a flat HTML document)...
        if((iTotalWafersInGroup == 0) && (pGroup != NULL) && bWriteReport)
            WritePageBreak();

        iGroupID++;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // LAST: If PAT-Man report built and multi-sites, then show combined wafermap (merging all sites/groups)
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    if(WriteStackedGroupsWaferMap(iTotalWafersInFile,bWriteReport))
        WritePageBreak();

    // If only one wafermap in group and 'individual wafermap' is disabled, report a message!
    if((iTotalWafersInGroup == 0) && (getGroupsList().count() == 1)
        //&& (strOuputFormat=="HTML" || strOuputFormat=="DOC" || strOuputFormat=="PDF" || strOuputFormat=="PPT" || strOuputFormat=="INTERACTIVE")
        && m_pReportOptions->isReportOutputHtmlBased()
        && bWriteReport)
    {
        fprintf(hReportFile,"<h1><font color=\"#006699\">*Info* Wafermap image may be disabled!</h1><br>\n");
        fprintf(hReportFile,"To <b>enable</b> it, go to the <a href=\"_gex_options.htm\">'Options'</a> page, section 'Wafer map, Strip map / Wafermaps to include in report'<br>\n");
    }


    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Wafermap()
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if (of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // close table...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_WAFERMAP) == true)
            && (of=="HTML")
            )
        {
            if (iCurrentHtmlPage)
                WriteNavigationButtons(GEX_T("wafermap"));

            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");

            // Close report file (unless flat HTML with multiple sections. eg: MyReport)
            if(m_pReportOptions->strTemplateFile.isEmpty())
                CloseReportFile();
        }
    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Write HTML line into WaferMap table
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteWaferMapTableLine(const char *szLabel, const char *szData, bool bAlarm /*= false*/)
{
    if(*szData == 0)
        return;
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"21%%\" height=\"21\" bgcolor=%s size=\"%d\"><b>%s</b></td>\n",szFieldColor,iHthmNormalFontSize,szLabel);

    if (bAlarm)
            fprintf(hReportFile,"<td width=\"77%%\" height=\"21\" bgcolor=%s size=\"%d\">%s</td>\n",szAlarmColor,iHthmNormalFontSize,szData);
    else
        fprintf(hReportFile,"<td width=\"77%%\" height=\"21\" bgcolor=%s size=\"%d\">%s</td>\n",szDataColor,iHthmNormalFontSize,szData);

    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
QString CGexReport::GetWaferMapReportType(void)
{
    QString strMessage;
    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_DISABLED:  // disabled
            strMessage = "wafermap is disabled!";
            break;
        case GEX_WAFMAP_SOFTBIN:  // Soft bin
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_ZONAL_SOFTBIN:
            strMessage = "Show Software bins";
            break;
        case GEX_WAFMAP_HARDBIN:  // Hardbin
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            strMessage = "Show Hardware bins";
            break;
        case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            strMessage = "Parametric test (over test limits).";
            break;
        case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            strMessage = "Parametric test (over test results)";
            break;
        case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            strMessage = "Parametric/Functional test (pass/fail)";
            break;
        default:
            strMessage = GEX_NA;
            break;
    }

    return strMessage;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Wafermap image size to use (depends of # of images to create)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::GetWaferImageSize(bool bStackedWafer)
{
    int iChartSize = 0;

    // If chart size=Auto...compute it's real size (unless it's a STACKED wafer, in which case keep computed size.)
    QString strChartSizeOption = (m_pReportOptions->GetOption("wafer","chart_size")).toString();
    if(strChartSizeOption == "small")
    {
        iChartSize = GEX_CHARTSIZE_SMALL;
    }
    else if(strChartSizeOption == "medium")
    {
        iChartSize = GEX_CHARTSIZE_MEDIUM;
    }
    else if(strChartSizeOption == "large")
    {
        iChartSize = GEX_CHARTSIZE_LARGE;
    }
    else if(strChartSizeOption == "auto")
    {
        iChartSize = GEX_CHARTSIZE_AUTO;
    }
    else
    {
        GEX_ASSERT(false);
    }

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // IF displaying Tiled waferes (4 per line) then show in SMALL size!
    if(bTiledDisplay)
        iChartSize = GEX_CHARTSIZE_SMALL;
    else
    if((iChartSize == GEX_CHARTSIZE_AUTO) && (bStackedWafer == true))
    {
        // Stacked wafers...Auto = LARGE size!...unless creating flat file (then show it Medium size to fit in page!)
        if (of=="HTML")
            iChartSize = GEX_CHARTSIZE_LARGE;		// Creating standard HTML file
        else
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// Creating Flat HTML for Word, PDF,...output
    }
    else
    if((iChartSize == GEX_CHARTSIZE_AUTO) && (bStackedWafer == false))
    {
        if(m_pReportOptions->lAdvancedHtmlPages > (GEX_CHARTSIZEAUTO_MEDIUM/2))
            iChartSize = GEX_CHARTSIZE_SMALL;		// if over 20 charts to build
        else
        if(m_pReportOptions->lAdvancedHtmlPages > (GEX_CHARTSIZEAUTO_LARGE/2))
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// if between 5 and 20 charts to build
        else
            iChartSize = GEX_CHARTSIZE_LARGE;		// if upto 5 charts to build

        // If creating FLAT HTML file (Word, PDF: Force size to Medium in all cases).
        if (of!="HTML")
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// Creating Flat HTML for Word, PDF,...output
    }

    return iChartSize;
}

/////////////////////////////////////////////////////////////////////////////
// Tell image size to be created
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::GetWaferSizeRequested(int iChartSize)
{
    int iWaferSizeRequested=200;

    QString of					= m_pReportOptions->GetOption("output", "format").toString();
    QString pf					= m_pReportOptions->GetOption("output", "paper_format").toString();
    QStringList strLstWafermapMarker= m_pReportOptions->GetOption("wafer", "marker").toString().split("|");
    bool	bWafermapMarkerBin	= strLstWafermapMarker.contains("bin", Qt::CaseSensitive);
    bool	bWafermapMarkerPercentage = strLstWafermapMarker.contains("percent", Qt::CaseSensitive);

    switch(iChartSize)
    {
    case GEX_CHARTSIZE_SMALL:	// 200x200 wafermap size
        iWaferSizeRequested = 200;
        break;
    case GEX_CHARTSIZE_MEDIUM:
        iWaferSizeRequested = 400;
        break;
    case GEX_CHARTSIZE_LARGE:
        if (bWafermapMarkerBin || bWafermapMarkerPercentage)
            iWaferSizeRequested = 1400; // 1000
        else
        {
            if (of=="PPT")
                iWaferSizeRequested = 640;
            else if ( (of=="DOC"||of=="ODT")
                     && pf!="portrait" )
                iWaferSizeRequested = 680;
            else
                iWaferSizeRequested = 800;
        }
        break;
    }

    // Return image size in pixels
    return iWaferSizeRequested;
}


QString CGexReport::CreateWaferMapImage(int eWafermapMode,
                                        int lGroupIndex,
                                        int lFileIndex,
                                        const QString &lWafermapImagePath)

{
    if (pGroupsList.size()<lGroupIndex+1)
        return "error: no such group";

    CGexGroupOfFiles* lGroup=pGroupsList.at(lGroupIndex);

    if (!lGroup)
        return "error: null group at this index";

    //if (lGroup->GetTotalValidFiles()<lFileIndex+1)
    if (lGroup->pFilesList.size()<lFileIndex+1)
        return "error: no such file in this group";

    CGexFileInGroup* lFile=lGroup->pFilesList.at(lFileIndex);
    if (!lFile)
        return "error: null file at this index";

    bool pbBreakPage=false;

    CreateWaferMapImage((CGexReport::wafermapMode)eWafermapMode,
                        lGroup,
                        lFile,
                        false,
                        lWafermapImagePath.toLatin1().constData(),
                        "wafermap",
                        true,
                        true,
                        &pbBreakPage);

    return "ok";
}

bool isPassFailWafermapType( int aWaferMapType )
{
    return ( aWaferMapType == GEX_WAFMAP_STACK_TEST_PASSFAIL ) || ( aWaferMapType == GEX_WAFMAP_TEST_PASSFAIL );
}

int getImageOffsetForPassFailWafermapInHtmlBasedReport()
{
    return 30;
}

/////////////////////////////////////////////////////////////////////////////
// Creates a wafermap image, saves in a .png file...or .CSV depends or report type!
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateWaferMapImage(CGexReport::wafermapMode eWafermapMode,
                                        CGexGroupOfFiles* pGroup,
                                        CGexFileInGroup* pFile,
                                        BOOL bWriteReport,
                                        const char* szWafermapImagePath,
                                        const char * szWafermapImage,
                                        bool bSaveImage/*=false*/,
                                        bool bAllowWaferArrayReload/*=true*/,
                                        bool *pbBreakPage/*= NULL*/,
                                        int waferMapType/*= -1*/,
                                        bool noReport/*=false*/,
                                        CTest* testCell/*=NULL*/,
                                        QStringList deactivatedBinsList/*=QStringList()*/)
{
    GSLOG(5, QString("CreateWaferMapImage into %1 %2")
          .arg(szWafermapImagePath?szWafermapImagePath:"?").arg(szWafermapImage?szWafermapImage:"?")
          .toLatin1().data());

    int	iCode=0,iLoopX=0,iLoopY=0;
    int lDieStatus = GEX_WAFMAP_EMPTY_CELL;
    int	iChartSize=0;
    int	iIndex=0;
    int	iCurrentDieX=0,iCurrentDieY=0;
    int	iSmallestDieAllowed=0;		// Smallest Die size allowed.
    int	iFirstDieX=0,iFirstDieY=0;		// Tells which first die location is at up-left corner
    int	iLastDieX=0,iLastDieY=0;		// Tells which last die location is at low-right corner
    int	iXstart=0,iXcount=0,iXstep=0;
    int	iYstart=0,iYcount=0,iYstep=0;
    int	ImageSizeX=0;				// Wafermap image X size to create
    int	ImageSizeY=0;				// Wafermap image Y size to create
    CBinning	**ptBinList=NULL;
    CTest		*ptTestCell=NULL;			// Pointer to test cell to receive STDF info.
    int			iDieSizeX=0;
    int			iDieSizeY=0;
    int			iWaferSizeRequested=0;		// Once computed, holds the wafermap size requested in the settings
    QColor		*sColor=NULL;				// 50 different pens, as we have 16 different colors classes, or 50 if Zoning
    QColor		cTempColor;
    QPainter	qpWaferMapPainter;							// To draw into wafermap image
    QPixmap		*qpmWafermapPixmap=NULL;					// wafermap image pixmap
    bool        bLocalBreak = false;
    int			iLowDieX=0,iHighDieX=0,SizeX=0;
    int			iLowDieY=0,iHighDieY=0,SizeY=0;
    int			iDieX=0,iDieY=0;
    int			iHighestDieCount=0;
    int			iBinToBinDie=0;

    int lWafermapType;
    if (waferMapType == -1)
        lWafermapType = m_pReportOptions->iWafermapType;
    else
        lWafermapType = waferMapType;

    // We are using the global option to be synchronize between the waferamp and the data analysed
    QStringList	strLstWafermapMarker	= m_pReportOptions->GetOption("wafer", "marker").toString().split("|");
    bool		bWafermapMarkerBinInDie	= strLstWafermapMarker.contains("bin", Qt::CaseSensitive);
    bool		bWafermapMarkerRetest	= strLstWafermapMarker.contains("retest", Qt::CaseSensitive);
    bool		bWafermapMarkerPercentage	= strLstWafermapMarker.contains("percent", Qt::CaseSensitive);

    QStringList	strLstVisualOptions		= m_pReportOptions->GetOption("wafer", "visual_options").toString().split("|");
    bool		bWafmapAlwaysRound		= strLstVisualOptions.contains("shape_round", Qt::CaseSensitive);

    CStackedWaferMap * pStackedWafermap = NULL;

    if (testCell != NULL)
        ptTestCell = testCell;
    // Individual wafermap
    if(eWafermapMode == individualWafermap)
    {
        // Check if we have wafermap data.
        if(pFile->getWaferMapData().bWaferMapExists == false)
            return;
        if(pFile->getWaferMapData().getWafMap() == NULL)
            return;

        iLowDieX=pFile->getWaferMapData().iLowDieX;
        iHighDieX=pFile->getWaferMapData().iHighDieX;
        SizeX = pFile->getWaferMapData().SizeX;
        iLowDieY=pFile->getWaferMapData().iLowDieY;
        iHighDieY=pFile->getWaferMapData().iHighDieY;
        SizeY = pFile->getWaferMapData().SizeY;
    }
    else	//	compare or stacked wafermap)
    {
        if (eWafermapMode == compareBinMismatch || eWafermapMode == compareBinToBinCorrelation)
            pStackedWafermap = &pGroup->cDieMismatchWaferMapData;
        else
            pStackedWafermap = &pGroup->cStackedWaferMapData;

        // Check if we have STACKED wafermap data.
        if(pStackedWafermap->bWaferMapExists == false)
            return;

        iLowDieX			= pStackedWafermap->iLowDieX;
        iHighDieX			= pStackedWafermap->iHighDieX;
        SizeX				= pStackedWafermap->SizeX;
        iLowDieY			= pStackedWafermap->iLowDieY;
        iHighDieY			= pStackedWafermap->iHighDieY;
        SizeY				= pStackedWafermap->SizeY;
        iHighestDieCount	= pStackedWafermap->iHighestDieCount;
        if(iHighestDieCount == 0)
            iHighestDieCount = 1;	// avoid division by 0 if the stacked wafermap is empty.
    }

    // Sets pointer to correct Bin list.
    switch(lWafermapType)
    {
        case GEX_WAFMAP_DISABLED:  // Wafermap disabled
            return;

        case GEX_WAFMAP_SOFTBIN:		// Standard wafermap: SOFT Bin
        case GEX_WAFMAP_ZONAL_SOFTBIN:	// Zonal wafermap: SOFT Bin
            ptBinList= &(pGroup->cMergedData.ptMergedSoftBinList); // Points first SOFT binning structure
            break;
        case GEX_WAFMAP_HARDBIN:// Standard wafermap: HARD Bin
        case GEX_WAFMAP_ZONAL_HARDBIN:	// Zonal wafermap: HARD Bin
            ptBinList= &(pGroup->cMergedData.ptMergedHardBinList); // Points first HARD binning structure
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            if (testCell == NULL)
            {
                long lZoningTest=-1, lZoningPinmapIndex=-1;
                if(m_pReportOptions->pGexWafermapRangeList && m_pReportOptions->pGexWafermapRangeList->pTestRangeList)
                {
                    lZoningTest = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromTest;
                    lZoningPinmapIndex = m_pReportOptions->pGexWafermapRangeList->pTestRangeList->lFromPinmap;
                }

                // Zoning wafermap on specific test.
                if(pFile->FindTestCell(lZoningTest,lZoningPinmapIndex,&ptTestCell,false,false) !=1)
                    return;
            }
            break;
    }

    // Load Wafermap array with relevant data (unless disabled: eg PATMAN has loaded the array with merged sites)
    if(bAllowWaferArrayReload)
        gexReport->FillWaferMap(pGroup,pFile,ptTestCell,lWafermapType);

    // bWriteReport, bSaveImage
    // 1) True, X	:	o PNG image saved
    //					o Wafermap report written
    //					o No Binning table created
    // 2) False, False:	o PNG image not saved
    //					o No Wafermap report written
    //					o Binning table created by reading the wafermap data
    // 3) False, True:	o PNG image saved
    //					o No Wafermap report written
    //					o No Binning table created

    // Checks if display must include mirror conversions!
    if(pFile->getWaferMapData().GetPosXDirection() == true)
    {
        // Positive X as we go from LEFT to RIGHT direction.
        iXstart = 0;
        iFirstDieX = iLowDieX;
        iLastDieX = iHighDieX;
        iLoopX = 1+iHighDieX-iLowDieX;
        iXstep = 1;
    }
    else
    {
        // Mirror in X : reading from RIGHT to LEFT!
        iXstart = iHighDieX-iLowDieX;
        iFirstDieX = iHighDieX;
        iLastDieX = iLowDieX;
        iLoopX = 1+iXstart;
        iXstep = -1;
    }
    // Check for Y mirroring
    if(pFile->getWaferMapData().GetPosYDirection() == true)
    {
        // Positive Y as we go to the DOWN direction.
        iYstart = 0;
        iFirstDieY = iLowDieY;
        iLastDieY = iHighDieY;
        iLoopY = iYcount = 1+iHighDieY-iLowDieY;
        iYstep = 1;
    }
    else
    {
        // Mirror in Y : reading from BOTTOM to TOP!
        iYstart = iHighDieY-iLowDieY;
        iFirstDieY = iHighDieY;
        iLastDieY = iLowDieY;
        iLoopY = iYcount = 1+iYstart;
        iYstep = -1;
    }
    // Define default die size...may be adjusted to smaller value later if too many dies to show
    int iMaxDiesInLine = gex_max(iLoopX,iLoopY);

    // If write value in die / % , then don't care it's not square dies!
    if (bWafermapMarkerBinInDie || bWafermapMarkerPercentage)
    {
        iDieSizeX = 1400 / iLoopX;	// Rectangular dies!
        if(iDieSizeX < 14)
            iDieSizeX = 14;		// Minimum is 14 pixels
        if(bTiledDisplay)
        {
            if(iDieSizeX > 35)
                iDieSizeX = 35;	// Maximum is 35 pixels. (as showing tiled small wafers in the gallery table)
        }
        else
        if(iDieSizeX > 150)
        {
                iDieSizeX = 150;	// Maximum is 150 pixels.
        }
        iDieSizeY = 14;				// Fixed size...
    }
    else
    {
        iDieSizeX = iDieSizeY = 800 / iMaxDiesInLine;	// Square dies!
        if(iDieSizeX < 2)
            iDieSizeX = iDieSizeY = 2;		// Minimum is 3 pixels (was 14 pixels)
        if(bTiledDisplay)
        {
            if(iDieSizeX > 35)
                iDieSizeX = iDieSizeY = 35;	// Maximum is 35 pixels. (as showing tiled small wafers in the gallery table)
        }
        else
        if(iDieSizeX > 150)
            iDieSizeX = iDieSizeY = 150;	// Maximum is 150 pixels.
    }
    // In tiled mode, do not allow each die to be too big...
    if(bTiledDisplay)
    {
        if(iDieSizeX > 150)
            iDieSizeX = 150;	// Maximum is 150 pixels. (unless tiled mode...then not such big dies)
        if(iDieSizeX > 150)
            iDieSizeX = 150;	// Maximum is 150 pixels. (unless tiled mode...then not such big dies)

        // Set lowest die size for the tiled-gallery
        iSmallestDieAllowed = 2;
    }
    else
        iSmallestDieAllowed = 2;	// Was 5

    double lfValue;

    QString of							= m_pReportOptions->GetOption("output", "format").toString();
    QString	strWaferBinStackedOption	= m_pReportOptions->GetOption("wafer","bin_stacked").toString();

    if (of=="CSV")
    {
        // Generating .CSV report file.
        iCurrentDieY = iYstart;
        while(iYcount >0)
        {
            iCurrentDieX = iXstart;
            iXcount = iLoopX;
            while(iXcount > 0)
            {
                if(eWafermapMode == individualWafermap)
                {
                    // Get bin
                    iCode	= pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getBin();
                    lfValue = pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getValue();
                    // Compute absolute X,Y loc.
                    iDieX = pFile->getWaferMapData().iLowDieX + iXcount;
                    iDieY = pFile->getWaferMapData().iLowDieY + iYcount;
                }
                else
                {
                    // Get bin
                    iCode       = pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].ldCount;
                    lfValue     = pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].dValue;
                    lDieStatus  = pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].lStatus;
                    // Compute absolute X,Y loc.
                    iDieX = pStackedWafermap->iLowDieX + iXcount;
                    iDieY = pStackedWafermap->iLowDieY + iYcount;
                }

                switch(lWafermapType)
                {
                    case GEX_WAFMAP_STACK_SOFTBIN:
                    case GEX_WAFMAP_STACK_HARDBIN:	// Only called if report MUST be created.
                        if(bWriteReport)
                        {
                            if (eWafermapMode == stackedWafermap)
                            {
                                if (strWaferBinStackedOption == "bin_count")
                                {
                                    if(iCode != GEX_WAFMAP_EMPTY_CELL)
                                        fprintf(hReportFile,"%3d",iCode);	// BIN count for this die.
                                    else
                                        fprintf(hReportFile,"   ");		// Empty Cell.
                                }
                                else if (strWaferBinStackedOption == "pf_pass_if_all_pass")
                                {
                                    if (lDieStatus == GEX_WAFMAP_FAIL_CELL)
                                        fprintf(hReportFile, "0");
                                    else if (lDieStatus == GEX_WAFMAP_PASS_CELL)
                                        fprintf(hReportFile, ".");
                                    else
                                        fprintf(hReportFile, " ");
                                }
                            }
                            else if (eWafermapMode == lowYieldPattern)
                            {
                                if (lDieStatus == GEX_WAFMAP_FAIL_CELL)
                                    fprintf(hReportFile, "0");
                                else if (lDieStatus == GEX_WAFMAP_PASS_CELL)
                                    fprintf(hReportFile, ".");
                                else
                                    fprintf(hReportFile, " ");
                            }
                            else
                            {
                                if(iCode != GEX_WAFMAP_EMPTY_CELL)
                                    fprintf(hReportFile,"%3d",iCode);	// BIN count for this die.
                                else
                                    fprintf(hReportFile,"   ");		// Empty Cell.
                            }
                        }
                        break;

                    case GEX_WAFMAP_SOFTBIN:// Standard wafermap: SOFT Bin
                    case GEX_WAFMAP_HARDBIN:// Standard wafermap: HARD Bin
                    case GEX_WAFMAP_ZONAL_SOFTBIN:// Zonal wafermap: SOFT Bin
                    case GEX_WAFMAP_ZONAL_HARDBIN:// Zonal wafermap: HARD Bin
                        if (eWafermapMode == stackedWafermap)
                        {
                            if (strWaferBinStackedOption == "bin_count")
                            {
                                if(iCode != GEX_WAFMAP_EMPTY_CELL)
                                    fprintf(hReportFile,"%3d",iCode);	// BIN count for this die.
                                else
                                    fprintf(hReportFile,"   ");		// Empty Cell.
                            }
                            else if (strWaferBinStackedOption == "pf_pass_if_all_pass")
                            {
                                if (lDieStatus == GEX_WAFMAP_FAIL_CELL)
                                    fprintf(hReportFile, "0");
                                else if (lDieStatus == GEX_WAFMAP_PASS_CELL)
                                    fprintf(hReportFile, ".");
                                else
                                    fprintf(hReportFile, " ");
                            }
                        }
                        else if (eWafermapMode == lowYieldPattern)
                        {
                            if (lDieStatus == GEX_WAFMAP_FAIL_CELL)
                                fprintf(hReportFile, "0");
                            else if (lDieStatus == GEX_WAFMAP_PASS_CELL)
                                fprintf(hReportFile, ".");
                            else
                                fprintf(hReportFile, " ");
                        }
                        else
                        {
                            if(iCode == 1)
                            {
                                // If Bin table to be created...
                                if(bWriteReport)
                                    fprintf(hReportFile,".");		// BIN 1 = '.'
                                else if(!bSaveImage)
                                    // Add Bin inf to list
                                    pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,'P',1,true,false,"");
                            }
                            else
                            if((iCode >= 0) && (iCode < MAX_WAFMAP_BIN))
                            {
                                // If Bin table to be created...
                                if(bWriteReport)
                                    fprintf(hReportFile,"%x",iCode);	// BIN0, BIN2-BIN#(MAX_WAFMAP_BIN-1)
                                else if(!bSaveImage)
                                    pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,' ',1,true,false,"");	// Add Bin inf to list
                            }
                            else
                            if(iCode != GEX_WAFMAP_EMPTY_CELL)
                            {
                                if(bWriteReport)
                                    fprintf(hReportFile,"+");	// BIN >= MAX_WAFMAP_BIN
                                else if(!bSaveImage)
                                    pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,' ',1,true,false,"");	// Add Bin inf to list
                            }
                            else
                            {
                                if(bWriteReport)
                                    fprintf(hReportFile," ");			// Empty Cell.
                            }
                        }
                        break;

                    case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                    case GEX_WAFMAP_STACK_TESTOVERDATA:
                    case GEX_WAFMAP_TESTOVERLIMITS:  // Zoning on test limits
                    case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
                    case GEX_WAFMAP_TEST_PASSFAIL:
                    case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                        if((iCode >= 0) && (iCode < GEX_WAFMAP_EMPTY_CELL))
                        {
                            if(bWriteReport)
                            {
                                // Keep it last: print Parameter value in die?
                                if (bWafermapMarkerBinInDie)
                                {
                                    if (lWafermapType == GEX_WAFMAP_TEST_PASSFAIL
                                            || lWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                                    {
                                        if (eWafermapMode == individualWafermap)
                                        {
                                            if (iCode == GEX_WAFMAP_FAIL_CELL)
                                                fprintf(hReportFile, "F");
                                            else
                                                fprintf(hReportFile, "P");
                                        }
                                        else
                                            fprintf(hReportFile,"%d ",iCode);	// count when stacked or pass/fail value
                                    }
                                    else
                                    {
                                        // Write Bin#/Count in die if: option set+wafermap big enough!
                                        pFile->FormatTestResultNoUnits(&lfValue,ptTestCell->res_scal);
                                        fprintf(hReportFile,"%g ",lfValue);	// Value result
                                    }
                                }
                                else
                                {
                                    if (lWafermapType == GEX_WAFMAP_TEST_PASSFAIL || lWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                                    {
                                        if (eWafermapMode == individualWafermap)
                                        {
                                            if (iCode == GEX_WAFMAP_FAIL_CELL)
                                                fprintf(hReportFile, "F");
                                            else
                                                fprintf(hReportFile, "P");
                                        }
                                        else
                                            fprintf(hReportFile,"%d ",iCode);	// count when stacked or pass/fail value
                                    }
                                    else
                                        fprintf(hReportFile,"%d%% ",iCode);	// Distance from low limit in %
                                }
                            }
                            else if(!bSaveImage)
                                pFile->AddBinCell(&(pGroup->cMergedData.ptMergedSoftBinList),-1,iDieX,iDieY,iCode,iCode,'P',iCode,true,false,"");	// Add Bin inf to list
                        }
                        else
                        {
                            if(bWriteReport)
                                fprintf(hReportFile," ");			// Empty Cell.
                        }
                        break;
                }// end switch

                iCurrentDieX += iXstep;
                iXcount--;
            }
            if(bWriteReport)
                fprintf(hReportFile,"\n");

            iCurrentDieY += iYstep;
            iYcount--;
        }
    }
    else
    //if (of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        if(bWriteReport || bSaveImage)
        {
            // Create Wafermap image.
            double fDieX=iDieSizeX;
            double fDieY=iDieSizeY;

            // Get the image size to create (depends on the number of images to create)
            // Modifier la taille ici
            iChartSize = GetWaferImageSize(eWafermapMode != individualWafermap);

            // If only few dies...act as if WaferMap is LARGE
            if(gex_max(iLoopX,iLoopY) < 10)
            {
                iChartSize = GEX_CHARTSIZE_LARGE;
            }

            // GCORE-14380 - only large map allows title with test name and test number
            if( isPassFailWafermapType( lWafermapType ) )
            {
                iChartSize = GEX_CHARTSIZE_LARGE;
            }

            // Get image size requested (in pixels)
            // Modifier la taille ici
            iWaferSizeRequested = GetWaferSizeRequested(iChartSize);
            do
            {
                fDieX *= 0.97;		// reduce sizeX by 3%
                fDieY *= 0.97;		// reduce sizeY by 3%
                ImageSizeX=(int)(SizeX*fDieX);
                ImageSizeY=(int)(SizeY*fDieY);
            }
            while((ImageSizeX > iWaferSizeRequested) || (ImageSizeY > iWaferSizeRequested));

            // Save new die size into integer buffers. Also ensures die rectangle is not too small.
            iDieSizeX= (int)(fDieX <= iSmallestDieAllowed ? iSmallestDieAllowed: fDieX);
            iDieSizeY= (int)(fDieY <= iSmallestDieAllowed ? iSmallestDieAllowed: fDieY);

            if(iDieSizeX <=2 || iDieSizeY <=2)
            {
                if(pbBreakPage)
                {
                    (*pbBreakPage) = true;
                }
                bLocalBreak = true;
            }

            // Recompute final image size to ensure we will fit ALL dies+space for scales!
            ImageSizeX=GEX_WAFMAP_ORGX + (SizeX*iDieSizeX) + 2;
            ImageSizeY=GEX_WAFMAP_ORGY + (SizeY*iDieSizeY) + 2;


            // If option to keep wafermap round...
            if(bWafmapAlwaysRound && (SizeX*iDieSizeX*SizeY*iDieSizeY))
            {
                float fImageRatio = ((float)SizeX*iDieSizeX)/((float)SizeY*iDieSizeY);
                if(fImageRatio > 1.2)
                {
                    // Y axis too small
                    iDieSizeY = (int)((float)iDieSizeY*fImageRatio);

                }
                else
                if(fImageRatio < 0.8)
                {
                    // X axis too small
                    iDieSizeX = (int)((float)iDieSizeX/fImageRatio);
                }

                // Update image size to use.
                ImageSizeX=GEX_WAFMAP_ORGX + (SizeX*iDieSizeX) + 2;
                ImageSizeY=GEX_WAFMAP_ORGY + (SizeY*iDieSizeY) + 2;
            }

            // GCORE-14380
            if( isPassFailWafermapType( lWafermapType ) )
            {
                ImageSizeY += getImageOffsetForPassFailWafermapInHtmlBasedReport();
            }

            // Add more space to write wafer information (Product ID, Lot ID, Sublot ID, Wafer ID)
            if (bSaveImage)
                ImageSizeY += GEX_WAFMAP_TITLE;

            // Create the Pixmap where to draw the wafermap.
            qpmWafermapPixmap = new QPixmap( ImageSizeX, ImageSizeY );

            // Check if successfuly created pixmap (may fail if size is way too big)
            if(qpmWafermapPixmap == NULL || qpmWafermapPixmap->width() != ImageSizeX
                    || qpmWafermapPixmap->height () != ImageSizeY)
                goto exit_wafmap;

            // 50 colors for Zoning results, or stacked wafers.
            // Modifier la couleur ici
            sColor = CreateColorSpectrum();

            // fills with White color
            qpmWafermapPixmap->fill(QColor(255,255,255));

            // Begin painting task
            qpWaferMapPainter.begin(qpmWafermapPixmap);
        }

        // Starting location to draw the wafermap.
        int X = GEX_WAFMAP_ORGX;	// Column counter (in image pixels)
        int Y = GEX_WAFMAP_ORGY;	// Line counter (in image pixels)

        if(bWriteReport || bSaveImage)
        {
            int nLegendPosY = 0;

            // Draw X scale
            qpWaferMapPainter.setPen(QColor(Qt::black));
            qpWaferMapPainter.setFont(QFont("Helvetica", 8));

            if (bSaveImage && !noReport)
            {
                QString strTitle = QString("Image created with %1 - www.mentor.com")
                        .arg(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
                qpWaferMapPainter.drawText(X, 0, ImageSizeX, 20, Qt::AlignLeft | Qt::AlignVCenter, strTitle);

                switch (eWafermapMode)
                {
                    case stackedWafermap			:	strTitle = "Stacked wafermap";
                                                        break;
                    case compareBinMismatch			:	strTitle = "Bin mismatch wafermap";
                                                        break;
                    case compareBinToBinCorrelation	:	strTitle = "Bin mismatch wafermap";
                                                        break;
                    case individualWafermap			:	strTitle = QString("Product= %1, Lot= %2, Sublot= %3, Wafer= %4").arg(pFile->getMirDatas().szPartType, pFile->getMirDatas().szLot, pFile->getMirDatas().szSubLot, pFile->getWaferMapData().szWaferID);
                                                        break;
                    default							:	GEX_ASSERT("Unknown wafermap mode");
                                                        break;
                }

                qpWaferMapPainter.drawText(X, 30, ImageSizeX, 20, Qt::AlignLeft | Qt::AlignVCenter, strTitle);

                Y += GEX_WAFMAP_TITLE;
                nLegendPosY += GEX_WAFMAP_TITLE;
            }

            // GCORE-14380
            if( isPassFailWafermapType( lWafermapType ) )
            {
                int lOffset = getImageOffsetForPassFailWafermapInHtmlBasedReport();
                qpWaferMapPainter.setFont(QFont("Helvetica", 18));

                std::stringstream lStringStream;
                lStringStream << ptTestCell->GetTestNumber().toStdString()
                              << " : "
                              << ptTestCell->GetTestName().toStdString();

                qpWaferMapPainter.drawText( 0, nLegendPosY,
                                            ImageSizeX, lOffset,
                                            Qt::AlignHCenter | Qt::AlignTop,
                                            QString::fromStdString( lStringStream.str() ) );

                qpWaferMapPainter.setFont(QFont("Helvetica", 8));
                nLegendPosY += lOffset;
                Y += lOffset;
            }

            // Write legend if at least 6 dies hight!
            if(iLoopX >= 6)
                qpWaferMapPainter.drawText(0,nLegendPosY,ImageSizeX,20,Qt::AlignHCenter | Qt::AlignTop ,"Die X locations");

            qpWaferMapPainter.setPen(QColor(Qt::darkGray));
            qpWaferMapPainter.drawLine(X-2, Y-2, ImageSizeX-(iDieSizeX/2), Y-2);

            char	szText[10];		// Used to build the scale graduation values.
            int		Xloc;			// X location to draw scale lines
            for(iIndex = iFirstDieX; iIndex != (iLastDieX + iXstep);iIndex += iXstep)
            {
                Xloc = X+(iDieSizeX/2)+abs(iIndex-iFirstDieX)*iDieSizeX;
                if(iIndex % 10 == 0)
                {
                    // Die X location is multiple of 10
                    qpWaferMapPainter.drawLine(Xloc, Y-2, Xloc, Y-8);

                    sprintf(szText,"%d",iIndex);
                    qpWaferMapPainter.drawText(Xloc-30,Y-20,60,20,Qt::AlignHCenter | Qt::AlignTop ,szText);
                }
                else
                if(iIndex % 5 == 0)
                {
                    // Die X location is multiple of 5, but not 10
                    qpWaferMapPainter.drawLine(Xloc, Y-2, Xloc, Y-6);
                }
                else
                {
                    // Die X location is increment but NOT multiple of 5 or 10
                    qpWaferMapPainter.drawLine(Xloc, Y-2, Xloc, Y-3);
                }
            }

            // Draw Y scale
            qpWaferMapPainter.rotate(-90.0);
            qpWaferMapPainter.setPen(QColor(Qt::black));
            // Write legend if at least 6 dies hight!
            if(iLoopY >= 6)
                qpWaferMapPainter.drawText(-ImageSizeY,0,ImageSizeY,20,Qt::AlignHCenter | Qt::AlignTop ,"Die Y locations");
            qpWaferMapPainter.rotate(+90.0);	// Return to standard coordinate

            qpWaferMapPainter.setPen(QColor(Qt::darkGray));
            qpWaferMapPainter.drawLine(X-2, Y-2, X-2, ImageSizeY-(iDieSizeY/2));

            int	Yloc;			// Y location to draw scale lines
            for(iIndex = iFirstDieY; iIndex != (iLastDieY + iYstep);iIndex += iYstep)
            {
                Yloc = Y+(iDieSizeY/2)+abs(iIndex-iFirstDieY)*iDieSizeY;
                if(iIndex % 10 == 0)
                {
                    // Die Y location is multiple of 10
                    qpWaferMapPainter.drawLine(X-2, Yloc, X-8, Yloc);
                    sprintf(szText,"%d",iIndex);
                    qpWaferMapPainter.rotate(-90.0);
                    qpWaferMapPainter.drawText(-Yloc-30,X-20,60,20,Qt::AlignHCenter | Qt::AlignTop ,szText);
                    //p.drawText(-Yloc-8,X-10,szText);
                    qpWaferMapPainter.rotate(+90.0);	// Return to standard coordinate
                }
                else
                if(iIndex % 5 == 0)
                {
                    // Die Y location is multiple of 5, but not 10
                    qpWaferMapPainter.drawLine(X-2, Yloc, X-6, Yloc);
                }
                else
                {
                    // Die Y location is increment but NOT multiple of 5 or 10
                    qpWaferMapPainter.drawLine(X-2, Yloc, X-3, Yloc);
                }
            }
            // Grey is the default outline color for each die box.
            qpWaferMapPainter.setPen(QColor(Qt::darkGray));
            // Small font in case we have to write Bin# in die...
            qpWaferMapPainter.setFont(QFont(GEXCHART_FONTNAME, 6));
        }

        BOOL	bDrawOutline;
        int		iOriginalBin;
        QString strDieValue;

        // Die rectangle to draw: if die too smal, do not draw a white outline.
        int iRectSizeX = (iDieSizeX <= iSmallestDieAllowed ? iSmallestDieAllowed: iDieSizeX);
        int iRectSizeY = (iDieSizeY <= iSmallestDieAllowed ? iSmallestDieAllowed: iDieSizeY);

        if((iRectSizeX == iSmallestDieAllowed) ||(iRectSizeY == iSmallestDieAllowed))
        {
            // Die is too small do show a grey outline
            bDrawOutline = false;
            // If either of the die dimension is too small, consider the other one the same way.
            iRectSizeX = iDieSizeX;
            iRectSizeY = iDieSizeY;
        }
        else
            bDrawOutline = true;

        bool bDisplaySoftBin=true;
        // modifier ici
        switch(lWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:		// Standard Wafermap: SOFT BIN
            case GEX_WAFMAP_ZONAL_SOFTBIN:	// Zonal Wafermap: SOFT BIN
            case GEX_WAFMAP_STACK_SOFTBIN:
                bDisplaySoftBin = true;
                break;
            case GEX_WAFMAP_HARDBIN:		// Standard Wafermap: HARD BIN
            case GEX_WAFMAP_ZONAL_HARDBIN:	// Zonal Wafermap: HARD BIN
            case GEX_WAFMAP_STACK_HARDBIN:
                bDisplaySoftBin = false;
                break;
        }

        iCurrentDieY = iYstart;
        while(iYcount >0)
        {
            iCurrentDieX = iXstart;
            iXcount = iLoopX;
            while(iXcount > 0)
            {
                if (eWafermapMode == individualWafermap)
                {
                    // Get bin#.
                    iCode	= pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getBin();
                    lfValue	= pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getValue();

                    // Compute absolute X,Y loc.
                    iDieX	= pFile->getWaferMapData().iLowDieX + iXcount;
                    iDieY	= pFile->getWaferMapData().iLowDieY + iYcount;
                }
                else
                {
                    // Get bin#
                    iCode			= pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].ldCount;
                    lfValue			= pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].dValue;
                    lDieStatus      = pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].lStatus;
                    iBinToBinDie	= pStackedWafermap->cWafMap[(iCurrentDieX+(iCurrentDieY*SizeX))].nBinToBinDie;
                    // Compute absolute X,Y loc.
                    iDieX = pStackedWafermap->iLowDieX + iXcount;
                    iDieY = pStackedWafermap->iLowDieY + iYcount;
                }
                if(!deactivatedBinsList.contains(QString::number((iCode))))
                {

                    switch(lWafermapType)
                    {
                    case GEX_WAFMAP_SOFTBIN:// Standard Wafermap: SOFT BIN
                    case GEX_WAFMAP_HARDBIN:// Standard Wafermap: HARD BIN
                        // Collapse some binnings in same classes...typically 16 classes.
                        if(iCode == GEX_WAFMAP_EMPTY_CELL)
                            break;	// ignore empty die!

                        // If die tested...
                        if(!bWriteReport && !bSaveImage)
                            pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,' ',1,true,false,"");	// Add Bin inf to list
                        else
                        {
                            // Special process for bin to bin mismatch wafermap.
                            // The 1st test is the triangle on the upper left of the die, and the retest would be the triangle on the bottom right part of the die
                            if (eWafermapMode == compareBinToBinCorrelation)
                            {
                                int nBinDieInitTest = iBinToBinDie >> 16;
                                int nBinDieRetest	= iBinToBinDie & 0x00FF;

                                if (iCode == 0)
                                {
                                    //cTempColor = cDieColor.GetWafmapDieColor(nBinDieOne,bDisplaySoftBin);
                                    qpWaferMapPainter.setBrush(Qt::white);
                                    // Draw in darker (50% darker) color than rectangle.
                                    /*cTempColor.setRgb((int)(0.9*cTempColor.red()),(int)(0.9*cTempColor.green()),(int)(0.9*cTempColor.blue()));
                                    p.setPen(cTempColor);		// Die is too small: don't draw an outline.*/
                                    qpWaferMapPainter.setPen(Qt::gray);
                                    qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                                }
                                else
                                {
                                    QPolygon diePolygon(3);
                                    cTempColor = cDieColor.GetWafmapDieColor(nBinDieInitTest,bDisplaySoftBin);
                                    qpWaferMapPainter.setBrush(cTempColor);

                                    // Draw in darker (50% darker) color than rectangle.
                                    cTempColor.setRgb((int)(0.9*cTempColor.red()),(int)(0.9*cTempColor.green()),(int)(0.9*cTempColor.blue()));
                                    qpWaferMapPainter.setPen(cTempColor);		// Die is too small: don't draw an outline.

                                    diePolygon.setPoint(0,X,Y);
                                    diePolygon.setPoint(1,X, Y+iRectSizeY);
                                    diePolygon.setPoint(2,X+iRectSizeX, Y);
                                    qpWaferMapPainter.drawPolygon(diePolygon);

                                    cTempColor = cDieColor.GetWafmapDieColor(nBinDieRetest,bDisplaySoftBin);

                                    qpWaferMapPainter.setBrush(cTempColor);

                                    // Draw in darker (50% darker) color than rectangle.
                                    cTempColor.setRgb((int)(0.9*cTempColor.red()),(int)(0.9*cTempColor.green()),(int)(0.9*cTempColor.blue()));
                                    qpWaferMapPainter.setPen(cTempColor);		// Die is too small: don't draw an outline.

                                    diePolygon.setPoint(0,X+iRectSizeX, Y);
                                    diePolygon.setPoint(1,X, Y+iRectSizeY);
                                    diePolygon.setPoint(2,X+iRectSizeX, Y+iRectSizeY);
                                    qpWaferMapPainter.drawPolygon(diePolygon);
                                }
                            }
                            else
                            {
                                // Use correct color to fill Die rectangle
                                if (eWafermapMode == individualWafermap)
                                    cTempColor = cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin);
                                else if (eWafermapMode == compareBinMismatch)
                                    cTempColor = sColor[(50*iCode)/iHighestDieCount];	// Stacked wafer: use gradiant color
                                else if (eWafermapMode == lowYieldPattern)
                                {
                                    cTempColor = (lDieStatus == GEX_WAFMAP_PASS_CELL) ? sColor[25] : sColor[0];
                                }
                                else
                                {
                                    if (strWaferBinStackedOption == "bin_count")
                                    {
                                        cTempColor = sColor[(50*iCode)/iHighestDieCount];	// Stacked wafer: use gradiant color
                                    }
                                    else if (strWaferBinStackedOption == "pf_pass_if_all_pass")
                                    {
                                        cTempColor = (lDieStatus == GEX_WAFMAP_PASS_CELL) ? QColor(0x86, 0xF2, 0x86) : QColor(0xFC, 0x5A, 0x5A);
                                    }
#if 0 // waiting for new options
                                    else if (strWaferBinStackedOption == "pf_pass_if_one_pass")
                                    {
                                        cTempColor = (iCode == GEX_WAFMAP_PASS_CELL) ? QColor(0x86, 0xF2, 0x86) : QColor(0xFC, 0x5A, 0x5A);
                                    }
                                    else if (strWaferBinStackedOption == "highest_bin")
                                    {
                                        cTempColor = cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin);
                                    }
                                    else if (strWaferBinStackedOption == "highest_fail_bin")
                                    {
                                        cTempColor = cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin);
                                    }
#endif
                                }

                                qpWaferMapPainter.setBrush(cTempColor);

                                // Draw in darker (50% darker) color than rectangle.
                                cTempColor.setRgb((int)(0.9*cTempColor.red()),(int)(0.9*cTempColor.green()),(int)(0.9*cTempColor.blue()));
                                qpWaferMapPainter.setPen(cTempColor);		// Die is too small: don't draw an outline.
                                qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                            }

                            // Disable brush so we can write into the die rectangle.
                            qpWaferMapPainter.setBrush(Qt::NoBrush);

                            // Die retested?
                            if(bWafermapMarkerRetest && (eWafermapMode == individualWafermap))
                            {
                                if(pFile->getWaferMapData().getCellTestCounter()[(iCurrentDieX+(iCurrentDieY*SizeX))] > 1){
                                    // Draw a think border if die retested...
                                    iOriginalBin = pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getOrgBin();
                                    if(iOriginalBin != GEX_WAFMAP_EMPTY_CELL)
                                    {
                                        // If die size is big enough, make the outline 2 pixels wide.
                                        if(bDrawOutline==true)
                                        {
                                            qpWaferMapPainter.setBrush(QBrush(cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin), Qt::Dense2Pattern));
                                            qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                                            qpWaferMapPainter.setBrush(Qt::NoBrush);
                                        }
                                        // Draw a black outline
                                        qpWaferMapPainter.setPen(QColor(Qt::black));
                                        qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                                    }
                                }
                            }

                            // Keep it last: print bin#/Bin count in die?
                            if(bWafermapMarkerBinInDie && eWafermapMode != compareBinMismatch && eWafermapMode != compareBinToBinCorrelation)
                            {
                                if((eWafermapMode == stackedWafermap) || ((iCode != 1) && (bDrawOutline)))
                                    //								((iCode != 1) && (iChartSize == GEX_CHARTSIZE_LARGE) && (bDrawOutline)))
                                {
                                    // Write Bin#/Count in die if: option set+wafermap big enough!
                                    qpWaferMapPainter.setPen(QColor(Qt::black));
                                    qpWaferMapPainter.drawText(X+2,Y,iRectSizeX-2,iRectSizeY,Qt::AlignVCenter | Qt::AlignHCenter,QString::number(iCode));
                                }
                            }
                        }
                        break;
                    case GEX_WAFMAP_STACK_SOFTBIN:
                    case GEX_WAFMAP_ZONAL_SOFTBIN:
                    case GEX_WAFMAP_STACK_HARDBIN:
                    case GEX_WAFMAP_ZONAL_HARDBIN:
                        // Collapse some binnings in same classes...typically 16 classes.
                        if(iCode == GEX_WAFMAP_EMPTY_CELL)
                            break;	// ignore empty die!

                        // If die tested...
                        if(!bWriteReport && !bSaveImage)
                            pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,' ',1,true,false,"");	// Add Bin inf to list
                        else
                        {
                            // Use correct color to fill Die rectangle
                            if(eWafermapMode == individualWafermap)
                                cTempColor = cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin);
                            else if (eWafermapMode == lowYieldPattern)
                            {
                                cTempColor = (lDieStatus == GEX_WAFMAP_PASS_CELL) ? sColor[25] : sColor[0];
                            }
                            else
                            {
                                if (strWaferBinStackedOption == "pf_pass_if_all_pass")
                                    cTempColor = (lDieStatus == GEX_WAFMAP_PASS_CELL) ? QColor(0x86, 0xF2, 0x86) : QColor(0xFC, 0x5A, 0x5A);
                                else
                                    cTempColor = sColor[(50*iCode)/iHighestDieCount];	// Stacked wafer: use gradiant color
                            }

                            qpWaferMapPainter.setBrush(cTempColor);

                            // Draw in darker (50% darker) color than rectangle.
                            cTempColor.setRgb((int)(0.9*cTempColor.red()),(int)(0.9*cTempColor.green()),(int)(0.9*cTempColor.blue()));
                            qpWaferMapPainter.setPen(cTempColor);		// Die is too small: don't draw an outline.
                            qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);

                            // Disable brush so we can write into the die rectangle.
                            qpWaferMapPainter.setBrush(Qt::NoBrush);

                            // Die retested?
                            if(bWafermapMarkerRetest && (eWafermapMode == individualWafermap))
                            {
                                // Draw a think border if die retested...
                                if(pFile->getWaferMapData().getCellTestCounter()[(iCurrentDieX+(iCurrentDieY*SizeX))] > 1){

                                    iOriginalBin = pFile->getWaferMapData().getWafMap()[(iCurrentDieX+(iCurrentDieY*SizeX))].getOrgBin();
                                    if(iOriginalBin != GEX_WAFMAP_EMPTY_CELL)
                                    {
                                        // If die size is big enough, make the outline 2 pixels wide.
                                        if(bDrawOutline==true)
                                        {
                                            qpWaferMapPainter.setBrush(QBrush(cDieColor.GetWafmapDieColor(iCode,bDisplaySoftBin),Qt::Dense2Pattern));
                                            qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                                            qpWaferMapPainter.setBrush(Qt::NoBrush);
                                        }
                                        // Draw a black outline
                                        qpWaferMapPainter.setPen(QColor(Qt::black));
                                        qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                                    }
                                }
                            }


                            // Stacking wafer & write yield % value in die?
                            if(bWafermapMarkerPercentage &&
                                    (eWafermapMode == stackedWafermap || eWafermapMode == lowYieldPattern))
                            {
                                // Get yield for a given die location (scans die location for all wafers in group)
                                strDieValue = "";
                                if(pGroup->GetDieYieldValue(pStackedWafermap->iLowDieX+iCurrentDieX,pStackedWafermap->iLowDieY+iCurrentDieY,lfValue))
                                    strDieValue = strDieValue.setNum(lfValue,'f',0)+"%";

                                // Write Yield% in die!
                                qpWaferMapPainter.setPen(QColor(Qt::black));
                                qpWaferMapPainter.drawText(X+2,Y,iRectSizeX-2,iRectSizeY,Qt::AlignVCenter | Qt::AlignHCenter,strDieValue);
                            }

                            // Keep it last: print bin#/Bin count in die?
                            if(bWafermapMarkerBinInDie)
                            {
                                if((eWafermapMode == stackedWafermap) || ((iCode != 1) && (bDrawOutline)))
                                    //								((iCode != 1) && (iChartSize == GEX_CHARTSIZE_LARGE) && (bDrawOutline)))
                                {
                                    // Write Bin#/Count in die if: option set+wafermap big enough!
                                    qpWaferMapPainter.setPen(QColor(Qt::black));

                                    if (strWaferBinStackedOption == "pf_pass_if_all_pass")
                                        qpWaferMapPainter.drawText(X+2,Y,iRectSizeX-2,iRectSizeY,Qt::AlignVCenter | Qt::AlignHCenter,QString::number(lDieStatus));
                                    else
                                        qpWaferMapPainter.drawText(X+2,Y,iRectSizeX-2,iRectSizeY,Qt::AlignVCenter | Qt::AlignHCenter,QString::number(iCode));
                                }
                            }
                        }
                        break;
                    case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
                    case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                    case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
                    case GEX_WAFMAP_STACK_TESTOVERDATA:
                    case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
                    case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                        // Zoning test wafermap.
                        if(iCode > 100 || iCode == GEX_WAFMAP_EMPTY_CELL || iCode < 0)
                            break;	// ignore invalid values!

                        // Note: devide value by 2 because we only have 50 images for the 0-100% Limits space.
                        if(!bWriteReport && !bSaveImage)
                            pFile->AddBinCell(ptBinList,-1,iDieX,iDieY,iCode,iCode,' ',1,true,false,"");	// Add Bin inf to list if valid !
                        else
                        {
                            if (lWafermapType == GEX_WAFMAP_TEST_PASSFAIL || lWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                            {
                                if(eWafermapMode == individualWafermap)
                                    cTempColor = (iCode == GEX_WAFMAP_PASS_CELL) ? QColor(0x86, 0xF2, 0x86) : QColor(0xFC, 0x5A, 0x5A);
                                else
                                    cTempColor = sColor[(50*iCode)/iHighestDieCount];	// Stacked wafer: use gradiant color
                            }
                            else
                                cTempColor = sColor[iCode/2];

                            // Use correct color to fill Die rectangle
                            qpWaferMapPainter.setBrush(cTempColor);

                            //  If very small dies plotted, do not outline in grey but keep same color as die.
                            if(bDrawOutline==false)
                                qpWaferMapPainter.setPen(cTempColor);

                            qpWaferMapPainter.drawRect(X,Y,iRectSizeX,iRectSizeY);
                        }

                        // Keep it last: print Parameter value in die?
                        if(bWafermapMarkerBinInDie)
                        {
                            if (lWafermapType == GEX_WAFMAP_TEST_PASSFAIL || lWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                            {
                                if (eWafermapMode == individualWafermap)
                                {
                                    if (iCode == GEX_WAFMAP_FAIL_CELL)
                                        strDieValue = "F";
                                    else
                                        strDieValue = "P";
                                }
                                else
                                    strDieValue = QString::number(iCode);	// count when stacked or pass/fail value
                            }
                            else
                            {
                                // Write Bin#/Count in die if: option set+wafermap big enough!
                                pFile->FormatTestResultNoUnits(&lfValue, ptTestCell->res_scal);
                                if(fabs(lfValue) >= 100)
                                    strDieValue.sprintf("%ld",(long)lfValue);
                                else
                                    if(fabs(lfValue) >= 10)
                                    {
                                        if(ceil(lfValue) == lfValue)
                                            strDieValue.sprintf("%d",(int)lfValue);	// integer value!
                                        else
                                            strDieValue.sprintf("%.1lf",lfValue);
                                    }
                                    else
                                    {
                                        if(ceil(lfValue) == lfValue)
                                            strDieValue.sprintf("%d",(int)lfValue);	// integer value!
                                        else
                                            strDieValue.sprintf("%.2lf",lfValue);
                                    }
                            }

                            qpWaferMapPainter.setPen(QColor(Qt::black));
                            qpWaferMapPainter.drawText(X+2,Y,iRectSizeX-2,iRectSizeY,Qt::AlignVCenter | Qt::AlignHCenter,strDieValue);
                        }
                        break;
                    }
                }
                iCurrentDieX += iXstep;
                X+=iDieSizeX;
                iXcount--;
            }
            iCurrentDieY += iYstep;
            iYcount--;
            X=GEX_WAFMAP_ORGX;	// Reset column counter
            Y+=iDieSizeY;		// Move to next line
        }

        if(bWriteReport || bSaveImage)
        {
            // Write copyright info on image! (only if evaluation mode)
            if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
            {
                qpWaferMapPainter.setFont(QFont("Helvetica", 12,QFont::Bold));
                qpWaferMapPainter.setPen(QColor(0xff,0x66,0));
                qpWaferMapPainter.drawText( 5,ImageSizeY-4-qpWaferMapPainter.fontMetrics().height(),
                                            ImageSizeX,20, Qt::AlignCenter | Qt::AlignTop, GEX_EVALCOPY_NOTICE);
            }
            // Save to disk.
            qpWaferMapPainter.end();
            if (!qpmWafermapPixmap->save(szWafermapImagePath,"PNG"))
                GSLOG(SYSLOG_SEV_ERROR, QString("Failed to save WaferMap image to '%1'")
                      .arg(szWafermapImagePath).toLatin1().data() );
            #ifdef QT_DEBUG
                qpmWafermapPixmap->save(GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator()
                                    +QFileInfo(szWafermapImagePath).fileName());
            #endif
            if(bWriteReport)
            {
                if(
                     (bLocalBreak || (iChartSize == GEX_CHARTSIZE_AUTO || iChartSize == GEX_CHARTSIZE_LARGE))
                      &&
                     (m_pReportOptions->GetOption("output", "format").toString() == "PPT")
                      &&
                      (iChartSize != GEX_CHARTSIZE_SMALL )
                  )
                  WritePageBreak(); // Will actually flush a PPT/ODP new page

                // if noReport: only need an image
                if (!noReport)
                {
                    // 7131 : let s try to adapt height to max available to always enter inside a pres (ppt, odp,...) page
                    if ( (of=="PPT"||of=="ODP") ) //&&(iChartSize==GEX_CHARTSIZE_LARGE) )
                    {
                        // if we try to center the img, then the wafer is cut at the right !
                        // strangely when the wafer is in a separated page (not compact layout) the hlml is crapy:
                        // <img height="20%" border="0" src="../images/waf1-1-T1.png"><br><br>
                        // </td>
                        // </tr>
                        // </table>
                        //</body>
                        //</html>
                        if (iChartSize==GEX_CHARTSIZE_LARGE)
                            fprintf(hReportFile,"<html><body><table width=\"100%%\" height=\"80%%\">\n"
                              "<tr align=\"center\"><td>"
                              "<img height=\"80%%\" border=\"0\" src=\"../images/%s\"><br><br>\n",
                            formatHtmlImageFilename(szWafermapImage).toLatin1().constData());
                        else
                            fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\"><br><br>\n",
                                formatHtmlImageFilename(szWafermapImage).toLatin1().constData());
                    }
                    else
                    {
                        QString lFormatString;

                        // GCORE-14380 - rendering is different for stacked wafermap
                        if( isPassFailWafermapType( lWafermapType ) )
                            lFormatString = "<img border=\"0\" src=\"../images/%s\">";
                        else
                            lFormatString = "<img border=\"0\" src=\"../images/%s\"><br><br>\n";

                        fprintf(hReportFile,
                                lFormatString.toStdString().c_str(),
                                formatHtmlImageFilename(szWafermapImage).toLatin1().constData());
                    }

    //                 If Strip map, then show reading direction(1st device is bottom left)
                    if(pFile->getWaferMapData().bStripMap)
                      fprintf(hReportFile,"<img border=\"0\" src=\"../images/stripmap_direction.png\"><br><br>\n");
                }
            }
        }
    }// HTML report

exit_wafmap:
    // Free colors.
    if(sColor != NULL)
        delete []sColor;

    // Free pixmap.
    if(qpmWafermapPixmap != NULL)
        delete qpmWafermapPixmap;
    qpmWafermapPixmap=0;
}

/////////////////////////////////////////////////////////////////////////////
// Constructor: sets wafermap die colors for top 10 binnings
/////////////////////////////////////////////////////////////////////////////
CBinningColor::CBinningColor()
{
    // Fill the pass color list
    m_lstPassColor.append(QColor(0,0xa6,0));
    m_lstPassColor.append(QColor(0,0x86,0));
    m_lstPassColor.append(QColor(0,0x66,0));
    m_lstPassColor.append(QColor(0,0x46,0));

    // Fill the fail color list
    m_lstFailColor.append(QColor(0xff,0,0));
    m_lstFailColor.append(QColor(0xff,0xff,0));
    m_lstFailColor.append(QColor(0xff,0,0xff));
    m_lstFailColor.append(QColor(0x84,0x84,0xff));
    m_lstFailColor.append(QColor(0,0x84,0x84));
    m_lstFailColor.append(QColor(0xff,0xc6,0xc6));
    m_lstFailColor.append(QColor(0xc6,0xff,0xc6));
    m_lstFailColor.append(QColor(0x84,0x84,0));
    m_lstFailColor.append(QColor(0,0,0x84));
}

/////////////////////////////////////////////////////////////////////////////
// Assign a color to the given bin number
/////////////////////////////////////////////////////////////////////////////
QColor CBinningColor::assignBinColor(int nBinNumber, bool bPassBin)
{
    // 1/ Check if the given bin has already been assigned to a color, if it do then do nothing
    // 2/ It is a pass bin, try to pick a pass color from the pass color list if it remains at leats one.
    // 3/ It is a fail bin or not pass color are left, try to pick a fail color from the fail color list if it remains at least one.
    // 4/ Both list are empty, then don't assign any colors to this bin. Default color will be use
    QColor rgbColor = m_colorDefault;

    if (m_mapBinColors.contains(nBinNumber) == false)
    {
        // Assign the reserved green color to the bin One
        if (nBinNumber == 1)
            rgbColor = m_colorBinOne;
        // Assign a pass color to this pass bin if the list is not empty
        else if (bPassBin && m_lstPassColor.count() > 0)
            rgbColor = m_lstPassColor.takeFirst();
        // Assign a fail color to this bin if the list is not empty
        else if (m_lstFailColor.count() > 0)
            rgbColor = m_lstFailColor.takeFirst();

        m_mapBinColors.insert(nBinNumber, rgbColor);
    }
    else
        rgbColor = m_mapBinColors.value(nBinNumber);

    return rgbColor;
}

/////////////////////////////////////////////////////////////////////////////
// Get Bin number color image name
/////////////////////////////////////////////////////////////////////////////
QString CBinningColor::GetBinNumberImageName(int iBinningValue,bool bSoftBin)
{
    return GetBinColorImageName(GetWafmapDieColor(iBinningValue,bSoftBin));
}

/////////////////////////////////////////////////////////////////////////////
// Get Bin Color image name
/////////////////////////////////////////////////////////////////////////////
QString CBinningColor::GetBinColorImageName(const QColor& cColor)
{
    QString strImage;

    int iR = cColor.red();
    int iG = cColor.green();
    int iB = cColor.blue();

    // build RGB image color string
    strImage = strImage.sprintf("rgb_%02x%02x%02x.png",iR,iG,iB);
    return strImage;
}

/////////////////////////////////////////////////////////////////////////////
// Get Bin Color HTML code name
/////////////////////////////////////////////////////////////////////////////
QString CBinningColor::GetBinColorHtmlCode(int iBinningValue,bool bSoftBin)
{
    QString strHtmlColor;
    QColor	rgbColor = GetWafmapDieColor(iBinningValue,bSoftBin);

    int iR = rgbColor.red();
    int iG = rgbColor.green();
    int iB = rgbColor.blue();

    // build HTML color string
    strHtmlColor = strHtmlColor.sprintf("#%02x%02x%02x",iR,iG,iB);

    return strHtmlColor;
}

/////////////////////////////////////////////////////////////////////////////
// Get die color
/////////////////////////////////////////////////////////////////////////////
QColor CBinningColor::GetWafmapDieColor(int iBinningValue,bool bSoftBin/*=true*/)
{
    static	QColor cNoColorAssigned = Qt::black;

    // Check if use default colors or custom colors
    if(ReportOptions.bUseCustomBinColors)
    {
        GEX_BENCHMARK_METHOD("Custom bin colors");

        QList <CBinColor>::iterator itBegin;
        QList <CBinColor>::iterator itEnd;

        // check if we read the Softbin or Hard bin list.
        // Find to which Bin list this binning belongs...
        if (bSoftBin == true)
        {
            itBegin = ReportOptions.softBinColorList.begin();
            itEnd	= ReportOptions.softBinColorList.end();
        }
        else
        {
            itBegin = ReportOptions.hardBinColorList.begin();
            itEnd	= ReportOptions.hardBinColorList.end();
        }

        while(itBegin != itEnd)
        {
            if((*itBegin).cBinRange->Contains(iBinningValue))
            {
                // found bin entry, return color !
                return (*itBegin).cBinColor;
            }

            // Move to next BinColor object
            itBegin++;
        };

        // Bin doesn't belong to any valid entry...then force it!
        return cNoColorAssigned;
    }

    // Use Examinator default colors...
    if (m_mapBinColors.contains(iBinningValue))
        return m_mapBinColors.value(iBinningValue);

    // Binning in not part of top 10, belongs to 'others' class.
    return m_colorDefault;
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL pages for the Wafermap report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Wafermap(qtTestListStatistics *pqtStatisticsList_Wafermap, QString strImageName/*=""*/)
{
    int	iStatus=GS::StdLib::Stdf::NoError;
    if (GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
               GS::LPPlugin::ProductInfo::waferMap))
    {
        return iStatus;
    }

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Wafer maps...");

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    if(	((m_pReportOptions->iWafermapType == GEX_WAFMAP_DISABLED)
        && (of!="HTML")
        )
        ||
        (	m_pReportOptions->isReportOutputHtmlBased() //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_WAFERMAP)))
    {
        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
        return GS::StdLib::Stdf::NoError;
    }

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Reset HTML page counter
    iCurrentHtmlPage = 0;

    // Create Test index page
    if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS
            || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA
            || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL
            || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
    {
        int				iOrgWafermapType;
        CGexTestRange	*ptOrgTestRange = m_pReportOptions->pGexWafermapRangeList;		// Backup value;
        QElapsedTimer   elapsedTimer;
        elapsedTimer.start();

        // Will be used while creating all Statistic pages: only used if HTML pages created !
        mTotalHtmlPages = mTotalWafermapPages;

        // Read list based on sorting filter (defined by user in 'Options')
        foreach(CTest *ptTestCell, *pqtStatisticsList_Wafermap)
        {
            // Increase HTML page counter
            iCurrentHtmlPage++;

            // The Histogram generation may be long...so check for 'Abort' every 2 seconds.
            if(elapsedTimer.elapsed() > 2000)
            {
                // At least 2 seconds since last check...so check again now!
                if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
                   return GS::StdLib::Stdf::NoError;

                // Reset timer.
                elapsedTimer.restart();
            }

            // Overload Wafer test range list
            m_pReportOptions->pGexWafermapRangeList = new CGexTestRange(ptTestCell);

            // Save the original wafermap type; todo before processing wafermap !
            iOrgWafermapType = m_pReportOptions->iWafermapType;

            switch(iOrgWafermapType)
            {
                case GEX_WAFMAP_TEST_PASSFAIL			:
                case GEX_WAFMAP_TESTOVERLIMITS			:
                case GEX_WAFMAP_TESTOVERDATA			:
                if (ptTestCell->lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
                    m_pReportOptions->iWafermapType = GEX_WAFMAP_SOFTBIN;
                else if (ptTestCell->lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
                    m_pReportOptions->iWafermapType = GEX_WAFMAP_HARDBIN;
                else if (ptTestCell->bTestType == '-')
                    m_pReportOptions->iWafermapType = GEX_WAFMAP_TESTOVERDATA;
                break;

                case GEX_WAFMAP_STACK_TEST_PASSFAIL		:
                case GEX_WAFMAP_STACK_TESTOVERLIMITS	:
                case GEX_WAFMAP_STACK_TESTOVERDATA		:
                    if (ptTestCell->lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
                        m_pReportOptions->iWafermapType = GEX_WAFMAP_STACK_SOFTBIN;
                    else if (ptTestCell->lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
                        m_pReportOptions->iWafermapType = GEX_WAFMAP_STACK_HARDBIN;
                    else if (ptTestCell->bTestType == '-')
                        m_pReportOptions->iWafermapType = GEX_WAFMAP_STACK_TESTOVERDATA;
                    break;

                default									:
                    break;
            }

            // Create Wafer Map report
            iStatus = PrepareSection_Wafermap(true, ptTestCell->iHtmlWafermapPage);
            if(iStatus != GS::StdLib::Stdf::NoError)
                return iStatus;

            WriteWaferMapCharts(true, strImageName);

            // Restore originam wafermape type
            m_pReportOptions->iWafermapType = iOrgWafermapType;

            delete m_pReportOptions->pGexWafermapRangeList;
            m_pReportOptions->pGexWafermapRangeList = NULL;

            iStatus = CloseSection_Wafermap();
        }

        // Reload original Test list!
        m_pReportOptions->pGexWafermapRangeList = ptOrgTestRange;

        // Write index test list...unless we're creating a flat HTML file with other sections (eg: MyReport)
        if ( (of=="HTML")
            && m_pReportOptions->strTemplateFile.isEmpty())
        {
            char	szString[2048];

            // Open <stdf-filename>/report/wafermap.htm
            sprintf(szString,"%s/pages/wafermap.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;

            WriteTestListIndexPage(GEX_INDEXPAGE_WAFERMAP, true, pqtStatisticsList_Wafermap);

            CloseReportFile();
        }
    }
    else
    {
        // Create Wafer Map report
        iStatus = PrepareSection_Wafermap(true);
        if(iStatus != GS::StdLib::Stdf::NoError) return iStatus;

        WriteWaferMapCharts(true,strImageName);

        iStatus = CloseSection_Wafermap();
    }

    return iStatus;
}

CWafBinMismatchPareto::CWafBinMismatchPareto() : m_uiTotalCount(0)
{

}

CWafBinMismatchPareto::~CWafBinMismatchPareto()
{
    while (m_lstWafBinMismatch.isEmpty() == false)
        delete m_lstWafBinMismatch.takeFirst();
}

void CWafBinMismatchPareto::insert(CBinning* pBinGroup1, CBinning* pBinGroup2)
{
    QString				strKey;
    CWafBinMismatch*	pWafBinMismatch;

    // Create key: B<binning group1>-B<binning group2>
    strKey	=	"B";
    if(pBinGroup1)
        strKey	+=	QString::number(pBinGroup1->iBinValue);
    strKey	+=	"-";
    if(pBinGroup2)
        strKey	+=	QString::number(pBinGroup2->iBinValue);

    // If item already inserted, just update counter, else insert it
    pWafBinMismatch = m_hashWafBinMismatch.value(strKey, NULL);
    if(pWafBinMismatch)
        pWafBinMismatch->m_uiCount++;
    else
    {
        pWafBinMismatch = new CWafBinMismatch(pBinGroup1, pBinGroup2);
        m_hashWafBinMismatch.insert(strKey, pWafBinMismatch);
        m_lstWafBinMismatch.append(pWafBinMismatch);
    }

    // Update total counter
    m_uiTotalCount++;
}
