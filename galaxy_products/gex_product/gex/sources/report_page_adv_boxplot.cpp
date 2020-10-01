/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'BoxPlot' page: Gage R&R
/////////////////////////////////////////////////////////////////////////////

#include "browser_dialog.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "cstats.h"
#include "eval_exp_cexev.h"
#include "gexboxplotchart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_xbar_r_chart.h"
#include "gex_report.h"
#include "product_info.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "gqtl_global.h"
#include "gexperformancecounter.h"

#include <limits>
#include <QPainter>
#include <gqtl_log.h>

// main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// report_page_adv_histo.cpp
extern bool isSingleTestAllLayers(CGexChartOverlays *pChartsInfo);
//
extern bool CTestFailCountLessThan(CTest* s1, CTest* s2);

// cstats.cpp
extern double	ScalingPower(int iPower);

extern QString	formatHtmlImageFilename(const QString& strImageFileName);

// Define the best font that works under Windows or Unix for text labels, scales.
#if defined unix || __MACH__
    #define	GEXPLOT_GAGE_FONTNAME "Helvetica"		// Under Unix
#else
    #define	GEXPLOT_GAGE_FONTNAME "Times New Roman"	// Under Windows
#endif

double	table_d2[] =
{
0.0 , 1.41,     1.91,   2.24,   2.48,   2.67,   2.83,   2.96,   3.08,   3.18,   3.27,   3.35,   3.42,   3.49,   3.55,
0.0, 1.28,  1.81,   2.15,   2.40,   2.60,   2.77,   2.91,   3.02,   3.13,   3.22,   3.30,   3.38,   3.45,   3.51,
0.0, 1.23,  1.77,   2.12,   2.38,   2.58,   2.75,   2.89,   3.01,   3.11,   3.21,   3.29,   3.37,   3.43,   3.50,
0.0, 1.21,  1.75,   2.11,   2.37,   2.57,   2.74,   2.88,   3.00,   3.10,   3.20,   3.28,   3.36,   3.43,    3.49,
0.0, 1.19,  1.74,   2.10,   2.36,   2.56,   2.78,   2.87,   2.99,   3.10,   3.19,   3.28,   3.36,   3.42,   3.49,
0.0, 1.18,  1.73,   2.09,   2.35,   2.56,   2.73,   2.87,   2.99,   3.10,   3.19,   3.27,   3.35,   3.42,   3.49,
0.0, 1.17,  1.73,   2.09,   2.35,   2.55,   2.72,   2.87,   2.99,   3.10,   3.19,   3.27,   3.35,   3.42,   3.48,
0.0, 1.17,  1.72,   2.08,   2.35,   2.55,   2.72,   2.87,   2.98,   3.09,   3.19,   3.27,   3.35,   3.42,   3.48,
0.0, 1.16,  1.72,   2.08,   2.34,   2.55,   2.72,   2.86,   2.98,   3.09,   3.19,   3.27,   3.35,   3.42,   3.48,
0.0, 1.16,  1.72,   2.08,   2.34,   2.55,   2.72,   2.86,   2.98,   3.09,   3.18,   3.27,   3.34,   3.42,   3.48,
0.0, 1.15,  1.71,   2.08,   2.34,   2.55,   2.72,   2.86,   2.98,   3.09,   3.18,   3.27,   3.34,   3.41,   3.48,
0.0, 1.15,  1.71,   2.07,   2.34,   2.55,   2.72,   2.85,   2.98,   3.09,   3.18,   3.27,   3.34,   3.41,   3.48,
0.0, 1.15,  1.71,   2.07,   2.34,   2.55,   2.71,   2.85,   2.98,   3.09,   3.18,   3.27,   3.34,   3.41,   3.48,
0.0, 1.15,  1.71,   2.07,   2.34,   2.54,   2.71,   2.85,   2.98,   3.09,   3.18,   3.27,   3.34,   3.41,   3.48,
0.0, 1.15,  1.71,   2.07,   2.34,   2.54,   2.71,   2.85,   2.98,   3.08,   3.18,   3.26,   3.34,   3.41,   3.48,
0.0, 1.128, 1.693,  2.059,  2.326,  2.534,  2.704,  2.847,  2.97,   3.078,  3.173,  3.258,  3.336,  3.407,  3.472
};

// Constant table extracted from minitab doc
double	table_d2_unbiaised[] =
{
    0, 1, 1.128, 1.693, 2.059, 2.326, 2.534, 2.704, 2.847, 2.97, 3.078, 3.173, 3.258,
    3.336, 3.407, 3.472, 3.532, 3.588, 3.64, 3.689, 3.735, 3.778, 3.819, 3.858, 3.895,
    3.931, 3.965, 3.997, 4.028, 4.058, 4.086, 4.113, 4.139, 4.164, 4.189, 4.213, 4.236,
    4.258, 4.28, 4.301, 4.322, 4.342, 4.361, 4.38, 4.398, 4.415, 4.432, 4.449, 4.466,
    4.482, 4.498
};

/////////////////////////////////////////////////////////////////////////////
// Returns D2 value used to compute Repeatability
// iZ = Total number of samples (parts  Appraisers)
// iW = Number of trials (files in groups)
/////////////////////////////////////////////////////////////////////////////
double	lfD2_Value(int iZ,int iW)
{
    if(iZ < 1)
    {
        iZ = 1;
    }
    else if(iZ > 16)
    {
        iZ = 16;
    }

    if(iW < 1)
    {
        iW = 1;
    }
    // if more than 15 trials lets switch to second constant table
    else if ((iW > 15) && (iW < 51))
    {
        // source: minitab doc
        return table_d2_unbiaised[iW];
    }
    // if more than 50 trials lets compute it!
    else if (iW > 50)
    {
        // source: minitab doc
        return (3.4873 + (0.0250141 * iW) - (0.00009823*(GS_POW(iW, 2.0))));
    }

    return table_d2[(iZ-1)*15 + (iW-1)];
}

/////////////////////////////////////////////////////////////////////////////
// Truncate resolution for visual fitting
/////////////////////////////////////////////////////////////////////////////
static double AdaptResolution(double lfValue)
{
    if(lfValue >= 10)
        lfValue = (0.1 * double((int)( lfValue * 10)));		// Only one digit after the .
    else
    if(lfValue >= 1)
        lfValue = (0.01 * double((int)( lfValue * 100)));	// Only two digit after the .

    return lfValue;
}

/////////////////////////////////////////////////////////////////////////////
// Call-back Function used by the 'sort' function to create BoxPlot statistics table
/////////////////////////////////////////////////////////////////////////////
bool compareCTest( CTest *test1, CTest *test2)
{
    bool ret = false;
    QString strAdvBoxplotSortingOption = (ReportOptions.GetOption("adv_boxplot","sorting")).toString();

    if(strAdvBoxplotSortingOption == "test_number")
    {
        // Sorting: test number
        if (test1->lTestNumber < test2->lTestNumber)
            ret = true;
        else if(test1->lTestNumber == test2->lTestNumber)
        {
            // Check if test includes pinmap index...
            if((test1->lPinmapIndex == GEX_PTEST) && (test2->lPinmapIndex == GEX_PTEST))
                ret = true;
            else if((test1->lPinmapIndex == GEX_PTEST) && (test2->lPinmapIndex != GEX_PTEST))
                ret = true;
            else if((test1->lPinmapIndex != GEX_PTEST) && (test2->lPinmapIndex == GEX_PTEST))
                ret = false;
            else if(test1->lPinmapIndex < test2->lPinmapIndex )
                ret = true;
            else
                return false;
        }
        else
            ret = false;
    }
    else if (strAdvBoxplotSortingOption == "test_name")
    {
        // Sorting: test name
        ret = (test1->strTestName.compare(test2->strTestName, Qt::CaseInsensitive) < 0);
    }
    else if (strAdvBoxplotSortingOption == "test_flow_id")
    {
        // Sorting: Test flow ID (execution order)
        // Flow ID is -1 for custom tests (added by GEX). Those tests will appear at th eend of the list.
        if(test1->lTestFlowID == -1)
            ret = false;
        else if(test2->lTestFlowID == -1)
            ret = true;
        else if(test1->lTestFlowID < test2->lTestFlowID)
            ret = true;
    }
    else if (strAdvBoxplotSortingOption == "mean")
    {
        // Sorting: Mean
        // Compare mean (highest to lowest)
        if(test1->lfMean > test2->lfMean)
            ret = true;
    }
    else if (strAdvBoxplotSortingOption == "sigma")
    {
        // Sorting: Sigma (highest to lowest)
        // Compare Sigma
        if(test1->lfSigma > test2->lfSigma)
            ret = true;
    }
    else if (strAdvBoxplotSortingOption == "cp")
    {
        // Sorting: Cp
        // Compare Cp (lowest to highest)
        if(test1->GetCurrentLimitItem()->lfCp < test2->GetCurrentLimitItem()->lfCp)
            ret = true;
    }
    else if (strAdvBoxplotSortingOption == "cpk")
    {
        // Sorting: Cpk
        // Compare Cpk (lowest to highest)
        if(test1->GetCurrentLimitItem()->lfCpk < test2->GetCurrentLimitItem()->lfCpk)
            ret = true;
    }
    else if (strAdvBoxplotSortingOption == "r&r")
    {
        // Compare R&R
        if (test1->pGage == NULL || isnan(test1->pGage->lfRR_percent))
        {
            return false;
        }
        else if (test2->pGage == NULL || isnan(test2->pGage->lfRR_percent))
        {
            return true;
        }
        else
        {
            return (test1->pGage->lfRR_percent >= test2->pGage->lfRR_percent);
        }
    }
    else
    {
        // Error!
        GEX_ASSERT(false);
        ret = false;
    }
    return ret;
}




/////////////////////////////////////////////////////////////////////////////
// Gage  R&R statistical information
/////////////////////////////////////////////////////////////////////////////
QString	CGexReport::ValueAndPercentageString(double lfValue, double lfLimitSpace,double &lfPercent)
{
    GEX_BENCHMARK_METHOD(QString(""));

    QString strString;
    lfPercent=-1.0;

    // Compute value as % of the test limits (if applicable)
    if(lfLimitSpace > 0.0)
    {
        lfPercent = 100.0*lfValue/lfLimitSpace;
        if(lfPercent < 1e-3)
            lfPercent = 0;
        else
        if(lfPercent >= 100.0)
            lfPercent = 100.0;
    }
    // Write Value + percentage (if applicable)
    double	lfResult = AdaptResolution(lfValue);
    if(lfPercent >= 0){
        //strString.sprintf("%g (%.2f %%)",lfResult,lfPercent);
        QString strTemp1, strTemp2;
        strTemp1.sprintf("%g",lfResult);
        strTemp2.sprintf("%.2f",lfPercent);
        strString.sprintf("%s (%s %%)",getNumberFormat()->formatNumericValue(lfResult, true,strTemp1).toLatin1().constData()
                                      ,getNumberFormat()->formatNumericValue(lfPercent, false, strTemp2).toLatin1().constData());
    } else {
        strString.sprintf("%g",lfResult);
        strString.sprintf("%s",getNumberFormat()->formatNumericValue(lfResult, true, strString).toLatin1().constData());
    }

    return strString;
}

//	Class to evaluate R&R alarm conditions (over R_R and Cpk expressions)
using namespace variant;
using namespace cexev;
class CMyExEvCtx : public CExEvCtx
{
public:
    CMyExEvCtx() : CExEvCtx() {}
    virtual ~CMyExEvCtx() {}

    // Override to return values for variables
    virtual bool getValue(const char *cIdent, Variant& result);

    double	m_lfR_R_Percent;	// To hold R&R% value for the test under focus
    double	m_lfCpk;			// To hold Cpk value for the test under focus
};

bool CMyExEvCtx::getValue(const char *cIdent, Variant& result)
{
    if(!qstricmp(cIdent,"Cpk"))
    {
        result.setReal(m_lfCpk);
        return true;
    }
    if(!qstricmp(cIdent,"R_R"))
    {
        result.setReal(m_lfR_R_Percent);
        return true;
    }

    return CExEvCtx::getValue(cIdent, result);
}

/////////////////////////////////////////////////////////////////////////////
// Gage  R&R: Check background color to use
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::getR_R_AlarmColor(CTest *ptTestCell,double lfPercent, QString& strDataColor,QColor &cColor)
{
    // If no custom expression defined, keep using default Warning & Alarm thresholds
    if(m_lstGageWarning.count() == 0)
    {
        if(lfPercent < 5)
        {
            // HTML color
            strDataColor = szDataColor;	// FINE...no alarm

            // RGB color
            cColor = QColor(0xff,0xff,0xcc);
        }
        else
        if(lfPercent < 10)
        {
            // HTML color
            strDataColor = "\"#FFFF66\"";	// Warning

            // RGB color
            cColor = QColor(0xff,0xff,0x66);
        }
        else
        {
            // HTML color
            strDataColor = szAlarmColor;	// ALARM!

            // RGB color
            cColor = QColor(0xfc,0x6e,0x42);
        }
        return;
    }

    static char	szHtmlColor[11];
    QString strString;
    CMyExEvCtx eectx;
    CExEv cExpression;
    Variant vres;

    // Save Cpk & R&R% values for this test
    eectx.m_lfCpk = ptTestCell->GetCurrentLimitItem()->lfCpk;
    eectx.m_lfR_R_Percent = lfPercent;

    int iStatus,iResult;
    QString	strExpression;

    QList<CGageWarning>::iterator		itBegin = m_lstGageWarning.begin();
    QList<CGageWarning>::iterator		itEnd	= m_lstGageWarning.end();

    while(itBegin != itEnd)
    {
        // Get expression to evaluate
        strExpression = (*itBegin).expression();

        cExpression.setContext(&eectx);
        iStatus = cExpression.evalExpression(strExpression, vres);

        // Expression verified and Passes...then return its color
        vres.asInt(iResult);	// floating point result of expression evaluation
        if((iStatus == CExEv::E_OK) && (iResult))
        {
            // Return color
            cColor = (*itBegin).color();

            // HTML code code
            sprintf(szHtmlColor,"\"#%02x%02x%02x\"",cColor.red(),cColor.green(),cColor.blue());
            strDataColor = szHtmlColor;
        }
        if(iStatus != CExEv::E_OK)
        {
            // Expression evaluation failed...inform user unless we're running in background mode, or error already flagged
            if((*itBegin).syntaxError() == false)
            {
                strString = "Gage R&R alarm expression error (Line# " + QString::number((*itBegin).fileLine()) + ")\nExpression: ";
                strString += strExpression + "\nError type: ";
                strString += cExpression.getErrorMessage();

                GS::Gex::Message::information("", strString);

                // Flag this expression as invalid so we do not display the error message again!
                (*itBegin).setSyntaxError(true);
            }
        }

        // Check next expression in list
        ++itBegin;
    };
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrintValueAndPercentage(int iGroup,FILE *hReportFile,CTest *ptTestCell, const char * pDataColor, double lfValue, double lfLimitSpace,bool bCheckAlarm)
{
    double	lfPercent=-1;
    QString strColor = pDataColor;
    QColor	cColor;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Information only printed one: when listing info for Appraiser#1
    if(iGroup != 0)
    {
        if(strOutputFormat=="CSV")
        {
            if (lfLimitSpace > 0.0)
                fprintf(hReportFile,",,");
            else
                fprintf(hReportFile,",");
        }
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;</td>\n", strColor.toLatin1().constData());
        return;
    }

    // Compute value as % of the test limits (if applicable)
    if(lfLimitSpace > 0.0 && lfValue != GEX_C_DOUBLE_NAN)
    {
        lfPercent = 100.0*lfValue/lfLimitSpace;
        if(lfPercent < 1e-3)
            lfPercent = 0;
        else
        if(lfPercent >= 100.00)
            lfPercent = 100.00;
    }

    // If check for alarm (and use custom background color)
    if(bCheckAlarm)
        getR_R_AlarmColor(ptTestCell, lfPercent, strColor, cColor);

    // Write Value
    QString lFormatedValue;
    if (lfValue < std::numeric_limits<double>::epsilon())
    {
        lFormatedValue = "0";
    }
    else if (m_pReportOptions->
        GetOption("output", "scientific_notation").toString() == "turn_on")
    {
        lFormatedValue =
            this->getNumberFormat()->formatNumericValue(lfValue, true);
    }
    else
    {
        lFormatedValue =
            QString("%1").arg(AdaptResolution(lfValue));
    }
    if (lFormatedValue == "nan")
    {
        lFormatedValue = "n/a";
        lfPercent = -1.0;
    }


    if (strOutputFormat == "CSV")
    {
        fprintf(hReportFile,"%s,", lFormatedValue.toLatin1().constData());
    }
    else
    {
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s %s",
                strColor.toLatin1().constData(),
                lFormatedValue.toLatin1().constData(),
                ptTestCell->szTestUnits);
    }

    // Write percentage (if applicable)
    if(lfPercent >= 0)
    {
        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"%.2f %%,",lfPercent);
        else
            fprintf(hReportFile,"<br>(%.2f %%)",lfPercent);
    }
    else if(strOutputFormat=="CSV" && lfLimitSpace > 0.0)
        fprintf(hReportFile, ",");


    if (m_pReportOptions->isReportOutputHtmlBased()) //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        fprintf(hReportFile,"</td>\n");
}

void CGexReport::ComputeRAndRTestList()
{
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *  lGroup      = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    CGexFileInGroup *   lFile       = NULL;
    CTest *             lTest       = NULL;
    CTest *             lTmpTest    = NULL;

    // Fill the sorting-list object with Statistics table...
    QString lOptionStorageDevice    = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();
    QString lOutputFormat           = m_pReportOptions->GetOption("output", "format").toString();
    QString lHtmlPageBreak          = m_pReportOptions->GetOption("output", "html_page_breaks").toString();

    // Clear current test list
    mAdvancedTestsListToReport.clear();

    // Enumerate all tests to list in BoxPlot HTML report section.
    lTest = lGroup->cMergedData.ptMergedTestList;
    while(lTest != NULL)
    {
        // If neither PTR values, this test was not executed !
        if((lTest->ldExecs == 0) && (lTest->GetCurrentLimitItem()->ldOutliers == 0))
            goto NextTestCell;

        // IF Muti-result parametric test, do not show master test record
        if(lTest->lResultArraySize > 0)
            goto NextTestCell;

        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(lTest->bTestType == 'F')
            goto NextTestCell;

        // Ignore Generic Galaxy Parameters
        if((lTest->bTestType == '-') && (lOptionStorageDevice == "hide"))
            goto NextTestCell;

        // Check this test# also exists in ALL other groups (from group#2 and higher)
        for (int nCount = 1; nCount < getGroupsList().count(); ++nCount)
        {
            lGroup	= getGroupsList().at(nCount);
            lFile	= (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(0);

            if(lFile == NULL)
                goto NextTestCell;

            if(lFile->FindTestCell(lTest->lTestNumber,lTest->lPinmapIndex, &lTmpTest,false,false,
                                   lTest->strTestName.toLatin1().data())!=1)
                goto NextTestCell;
        };

        switch(m_pReportOptions->getAdvancedReportSettings())
        {
            case GEX_ADV_ALL:// All tests
            default:
                mAdvancedTestsListToReport.append(lTest);
            break;

            case GEX_ADV_LIST:// List of tests
                // Create Boxplot chart for the selected files...
                if(m_pReportOptions->pGexAdvancedRangeList->IsTestInList(lTest->lTestNumber,lTest->lPinmapIndex))
                {
                    mAdvancedTestsListToReport.append(lTest);
                }
            break;
            case GEX_ADV_TOPNFAILTESTS:
                // For the moment let take all then we will sort them and then keep only worst
                GSLOG(SYSLOG_SEV_NOTICE, "Error : feature not yet implemented.");
                mAdvancedTestsListToReport.append(lTest);
            break;
        }

        // Point to next test cell
        NextTestCell:
        lTest = lTest->GetNextTest();
    };	// Loop until all test cells read.

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 tests saved").arg(mAdvancedTestsListToReport.count()).toLatin1().constData());

    // Have the list sorted!
    qSort(mAdvancedTestsListToReport.begin(), mAdvancedTestsListToReport.end(), compareCTest);

    long	lTestsBoxplotInPage = 0;
    bool	lForceBreak;

    mTotalAdvancedPages = 1;	// Number of boxplot HTML pages that will be generated
    foreach(lTest, mAdvancedTestsListToReport)
    {
        // We create one ADV box-plot HTML page per X tests (only for PowerPoint slides that require physical page breaks
        lForceBreak = false;
        if(lHtmlPageBreak=="true")
        {
            if((lOutputFormat == "PPT") && (lTestsBoxplotInPage > 11 ))
                lForceBreak = true;
            if((lOutputFormat == "HTML") && (lTestsBoxplotInPage > MAX_STATS_PERPAGE))
                lForceBreak = true;
        }

        if(lForceBreak)
        {
            // Once X tests are in the page, reset counter, increment page count
            lTestsBoxplotInPage=0;
            mTotalAdvancedPages++;
        }

        lTest->iHtmlAdvancedPage= mTotalAdvancedPages;
        // We have as many statistics lines per test as groups to compare!
        lTestsBoxplotInPage+=m_pReportOptions->iGroups;
    };	// Loop until all sorted test cells read.

    // Will be used while creating all Histogram pages
    mTotalHtmlPages = mTotalAdvancedPages;

    // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
    m_pReportOptions->lAdvancedHtmlPages = mTotalAdvancedPages;
}

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvBoxPlot(BOOL bValidSection)
{
    // Generating HTML report file.
    if(bValidSection == false)
        return GS::StdLib::Stdf::NoError;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report: Gage R&R Box-plot ----\n\n");
    }

    ComputeRAndRTestList();

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvBoxPlot(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Boxplot page created (if any)...so we can now build the index.
        // close table...if this section was created.
        if(IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true)
            fprintf(hReportFile,"</table>\n");

        if ((hReportFile != NULL) && (of=="HTML"))
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
    if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
        return GS::StdLib::Stdf::ReportFile;

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVBOXPLOT,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(of=="HTML")
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes header line of labels in statistics page.
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteBoxPlotLabelLine(bool bChartOverLimits,int &iTotalRows)
{
    const char	*szMean;	// holds 'Mean' column label, built at run time
    const char	*szSigma;	// holds 'Sigma' column label, built at run time
    const char	*szMedian;	// holds 'Median' column label, built at run time
    const char	*szCp;		// holds 'Cp' column label, built at run time
    const char	*szCpk;		// holds 'Cpk' column label, built at run time
    const char	*szLegend;	// Gage R&R legend image
    QString	strGlobalInfoBookmark;

    // ('adv_boxplot', 'field') flag option
    QString		strAdvBoxplotFieldOptions = (m_pReportOptions->GetOption(QString("adv_boxplot"), QString("field"))).toString();
    QStringList qslAdvBoxplotFieldOptionsList = strAdvBoxplotFieldOptions.split(QString("|"));

    // Count total rows in table
    iTotalRows = 0;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        fprintf(hReportFile,"Test,Name,");
    }
    else
    {
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Test</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Name</b></td>\n",szFieldColor);
    }
    // Keep track of total rows
    iTotalRows += 2;

    // Hyperlink to Global info page.
    if(strOutputFormat=="HTML")
        strGlobalInfoBookmark = "global.htm";
    else
        strGlobalInfoBookmark = "#all_globalinfo";

    // If multiple groups, show group names
    if(m_pReportOptions->iGroups > 1)
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Group Appraiser,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b><a href=\"%s\">Group<br>Appraiser</a></b></td>",szFieldColor,strGlobalInfoBookmark.toLatin1().constData());
    }

    // Check if display Test Limits
    if(qslAdvBoxplotFieldOptionsList.contains(QString("limits")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Low.L.,,High.L.,,");
        else
        {
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Low.L.</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>High.L.</b></td>\n",szFieldColor);
        }
    }

    if(qslAdvBoxplotFieldOptionsList.contains(QString("tolerance")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Tol,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,"Tol");
    }

    if(qslAdvBoxplotFieldOptionsList.contains(QString("global_min")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Min,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,"Min");
    }

    if(qslAdvBoxplotFieldOptionsList.contains(QString("global_max")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Max,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>", szFieldColor, "Max");
    }

    if(qslAdvBoxplotFieldOptionsList.contains(QString("global_mean")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Mean,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>", szFieldColor, "Mean");
    }

    // Check if display Mean/Shift
    if(qslAdvBoxplotFieldOptionsList.contains(QString("mean")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
        {
            szMean = (m_pReportOptions->iGroups == 1)? "Mean,," : "Mean,,Shift,";
            fprintf(hReportFile,"%s",szMean);
        }
        else
        {
            szMean = (m_pReportOptions->iGroups == 1)? "Mean" : "Mean / Shift";
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,szMean);
        }
    }

    // Check if display Sigma/Shift
    if(qslAdvBoxplotFieldOptionsList.contains(QString("sigma")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
        {
            szSigma= (m_pReportOptions->iGroups == 1)? "Sigma,," : "Sigma,,Shift,";
            fprintf(hReportFile,"%s",szSigma);
        }
        else
        {
            szSigma= (m_pReportOptions->iGroups == 1)? "Sigma" : "Sigma / Shift";
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,szSigma);
        }
    }

    // Check if display Median/Shift
    if(qslAdvBoxplotFieldOptionsList.contains(QString("median")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
        {
            szMedian = (m_pReportOptions->iGroups == 1)? "Median,," : "Median,,";
            fprintf(hReportFile,"%s",szMedian);
        }
        else
        {
            szMedian = (m_pReportOptions->iGroups == 1)? "Median" : "Median";
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,szMedian);
        }
    }

    // Check if display Cp/Shift
    if(qslAdvBoxplotFieldOptionsList.contains(QString("cp")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
        {
            szCp = (m_pReportOptions->iGroups == 1)? "Cp," : "Cp,Shift,";
            fprintf(hReportFile,"%s",szCp);
        }
        else
        {
            szCp = (m_pReportOptions->iGroups == 1)? "Cp" : "Cp / Shift";
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,szCp);
        }
    }

    // Check if display Cpk/Shift
    if(qslAdvBoxplotFieldOptionsList.contains(QString("cpk")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
        {
            szCpk = (m_pReportOptions->iGroups == 1)? "Cpk," : "Cpk,Shift,";
            fprintf(hReportFile,"%s",szCpk);
        }
        else
        {
            szCpk = (m_pReportOptions->iGroups == 1)? "Cpk" : "Cpk / Shift";
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>",szFieldColor,szCpk);
        }
    }

    // Check if display dataset Repeatability
    if(qslAdvBoxplotFieldOptionsList.contains(QString("repeatability")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Repeatability,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Repeat.</b></td>",szFieldColor);
    }

    // Check if display #
    if(qslAdvBoxplotFieldOptionsList.contains(QString("parts_in_control")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"#,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>%s</b></td>", szFieldColor, "Parts IC");
    }

    // Check if display Equipment Variation / repetability
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("ev"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"(EV),,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>(EV)</b></td>",szFieldColor);
    }

    // Check if display Appraiser Equipment Variation / repetability
    if(qslAdvBoxplotFieldOptionsList.contains(QString("local_ev")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"(Local EV),,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>(Local EV)</b></td>",szFieldColor);
    }

    // Check if display Reproducibility
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("av"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"(AV),,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>(AV)</b></td>",szFieldColor);
    }

    // Check if display R&R
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("r&r"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"R&R,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>R&R</b></td>",szFieldColor);
    }

    // Check if display #R&R
    if(qslAdvBoxplotFieldOptionsList.contains(QString("#r&r")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"#R&R,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>#R&R</b></td>",szFieldColor);
    }

    // Check if display AV Worst Case
    if(qslAdvBoxplotFieldOptionsList.contains(QString("avwc")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"AVwc,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>AVwc</b></td>",szFieldColor);
    }

    // Check if display R&R Worst Case
    if(qslAdvBoxplotFieldOptionsList.contains(QString("r&rwc")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"R&Rwc,,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>R&Rwc</b></td>",szFieldColor);
    }

    // Check if display Max XDiff
    if(qslAdvBoxplotFieldOptionsList.contains(QString("max_xdiff")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"Max Xdiff,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Max Xdiff</b></td>",szFieldColor);
    }

    // Check if display GuardBand (%RNR*Tolerance
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("gb"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"GB,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>GB</b></td>",szFieldColor);
    }

    // Check if display PV
    if((m_pReportOptions->iGroups > 1) && qslAdvBoxplotFieldOptionsList.contains(QString("pv")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"(PV),,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>(PV)</b></td>",szFieldColor);
    }

    // Check if display TV
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("tv"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"(TV),,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>(TV)</b></td>",szFieldColor);
    }

    // Check if display P/T Ratio
    if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("p_t"))))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"P/T,");
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P/T</b></td>",szFieldColor);
    }

    if (strOutputFormat!="CSV" && qslAdvBoxplotFieldOptionsList.contains(QString("boxplot_chart")))
    {
        // Keep track of total rows
        iTotalRows ++;

        if(bChartOverLimits)
            szLegend="gage_limits.png";	// Drawing Range is test limits
        else
            szLegend="gage_range.png";	// Drawing range is min-max space
        // Boxplot chart image
        fprintf(hReportFile,"<td bgcolor=%s > <img border=\"0\" src=\"../images/%s\"></td>\n",szFieldColor, formatHtmlImageFilename(szLegend).toLatin1().constData());
    }

    if(strOutputFormat=="CSV")
    {
        fprintf(hReportFile,"\n");
    }
    else
    {
        fprintf(hReportFile,"</tr>\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Computes limits space
/////////////////////////////////////////////////////////////////////////////
void CGexReport::ComputeLimitsSpace(CGexFileInGroup* /*pFile*/,
                                    CTest* ptTestCell,
                                    double& lfLimitSpace)
{
    lfLimitSpace=0.0;
    // Compute limit space (used in R&R)
    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
    {
        double fData = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
#if 0
        // If we have to keep values in normalized format, do not rescale!
        if(pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
        {
            // convert HighLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&fData,ptTestCell->hlm_scal);
            fData *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
            fData /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }
#endif
        lfLimitSpace = fData;

        fData = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists

#if 0
        // If we have to keep values in normalized format, do not rescale!
        if(pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
        {
            // convert LowLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&fData,ptTestCell->llm_scal);
            fData *=  ScalingPower(ptTestCell->llm_scal);	// normalized
            fData /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }
#endif

        lfLimitSpace -= fData;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Compute the gage R&R values for a given test (in group#1)
/////////////////////////////////////////////////////////////////////////////
void CGexReport::ComputeTestGage(CTest *ptReferenceTestCell)
{
    // Ignore gage R&R for tests that are not paramteric!
    if(ptReferenceTestCell->bTestType == 'F')
        return;

    CTest	*			ptTestCell	= NULL;
    CGexGroupOfFiles *	pGroup		= NULL;
    CGexFileInGroup *	pFile		= NULL;
    double          lfMin,lfMax;
    int             iDataOffset;
    int             ldAppraisers=0;			// total appraisers involved
    QList<double>   lAppraiserAverage; // holds average by appraiser
    unsigned        lTrials=0;             // total trials involved
    unsigned        uTrialID;				// index counter
    int             ldMaxSamplesPerTrial=0;	// Holds maximum samples to consider in each trial (is in fact the minimum common number to all trials!)
    double          lfD2;
    double          lfSumRanges=0;			// Holds sum of ranges
    int             ldTotalSamples=0;		// holds total number of samples (parts * Appraisers)
    double          lfMaxMean;
    double          lfMinMean;
    double          lfTmpMean=0;
    int             iIndex;
    int             iGroupID=0;	// Counter used to know on which group we are (#1 or other)
    double          lfLimitSpace;
    double          lfPercent;

    bool bR_R_Nsigma=false;
    double lfR_R_Nsigma = ReportOptions.GetOption("adv_boxplot","r&r_sigma").toDouble(&bR_R_Nsigma);
    if (!bR_R_Nsigma)
    {
        GSLOG(SYSLOG_SEV_WARNING,
              "cant retrieve option 'adv_boxplot','r&r_sigma' !");
    }

    // No data samples for this test!
    if(ptReferenceTestCell->ldSamplesValidExecs <= 0)
        return;

    int nSamples = ptReferenceTestCell->ldSamplesExecs;

    // Samples execs must be equal for each group
    for (iGroupID = 0; iGroupID < getGroupsList().count(); iGroupID++)
    {
        pGroup = getGroupsList().at(iGroupID);
        if (!pGroup->pFilesList.isEmpty())
        {
            // Handle to files in group.
            pFile = pGroup->pFilesList.at(0);
            // Get pointer to test structure
            pFile->FindTestCell(ptReferenceTestCell->lTestNumber,
                                ptReferenceTestCell->lPinmapIndex,&ptTestCell,false,false,
                                ptReferenceTestCell->strTestName.toLatin1().data());

            // Ensure test# exists in each dataset
            if(ptTestCell == NULL || ptTestCell->ldSamplesExecs != nSamples)
                return;
        }
    }

    // Variables for GAGE R&R computations
    CGageR_R	*ptGage = ptReferenceTestCell->pGage = new CGageR_R();
    if(ptGage == NULL)
        return;

    // ***************************
    // GAGE R&R computations
    // ***************************
    // Gage: "EV" Repeatability............
    double	lValue;
    ldAppraisers = getGroupsList().count();

    // Check if each group only holds one data file.
    // If so: then split each file onto two sub-data sets to virtually create 2 trials.
    int iMaxFilesPerGroup = 0;
    for (iGroupID = 0; iGroupID < getGroupsList().count(); iGroupID++)
    {
        pGroup = getGroupsList().at(iGroupID);
        iMaxFilesPerGroup = gex_max(iMaxFilesPerGroup, pGroup->pFilesList.count());
    }

    // Rewind.
    iGroupID=0;

    ///// Computing R&R for a group with multiple files.
    // Rewind.
    lTrials = pGroup->pFilesList.count();
    lAppraiserAverage.clear();
    for (iGroupID = 0; iGroupID < getGroupsList().count(); iGroupID++)
    {
        pGroup = getGroupsList().at(iGroupID);
        if (!pGroup->pFilesList.isEmpty())
        {
            double lAppraiserSum = 0;
            // Handle to files in group.
            pFile = pGroup->pFilesList.at(0);
            // Get pointer to test structure
            pFile->FindTestCell(ptReferenceTestCell->lTestNumber,ptReferenceTestCell->lPinmapIndex,&ptTestCell,
                                false,false, ptReferenceTestCell->strTestName.toLatin1().data());

            // Ensure test# exists in each dataset
            if(ptTestCell == NULL)
                return;

            // Update maximum number of commun samples in all trials
            ldMaxSamplesPerTrial = ptTestCell->pSamplesInSublots[0];
            for(uTrialID=1;uTrialID < lTrials; uTrialID++)
                ldMaxSamplesPerTrial = gex_min(ldMaxSamplesPerTrial, ptTestCell->pSamplesInSublots[uTrialID]);

            // Compute sum of ranges (so to compute Mean of ranges RR value):
            // this is the range of results for a same partID over the trials  (files in group)
            for(int iIndex = 0; iIndex <ldMaxSamplesPerTrial; iIndex++)
            {
                // Compute range of result for the same partID in all trials in this group.
                lfMin = C_INFINITE;
                lfMax = -C_INFINITE;
                iDataOffset = 0;

                for(uTrialID=0;uTrialID < lTrials; uTrialID++)
                {
                    int nRsltIndex = iIndex + iDataOffset;
                    if( !(ptTestCell->m_testResult.isValidIndex(nRsltIndex)) )
                    {
                        GEX_ASSERT(false);
                        continue;
                    }

                    if(ptTestCell->m_testResult.isValidResultAt(nRsltIndex))
                    {
                        // Get trial value
                        lValue = ptTestCell->m_testResult.resultAt(nRsltIndex);

                        lValue *= ScalingPower(ptTestCell->res_scal);	// normalized
                        lAppraiserSum += lValue;
                        // Update Min/Max
                        lfMin = gex_min(lfMin,lValue);
                        lfMax = gex_max(lfMax,lValue);
                    }
                    iDataOffset += ptTestCell->pSamplesInSublots[uTrialID];
                }

                // Update sum of range
                if( (lfMax!=-C_INFINITE) && (lfMin!=C_INFINITE) )		// PYC, 24/05/2011
                    lfSumRanges += lfMax - lfMin;
            }

            // Keep track of number of samples computed
            ldTotalSamples += ldMaxSamplesPerTrial;
            lAppraiserAverage.append(lAppraiserSum / (ldMaxSamplesPerTrial*lTrials));
        }
    };

    lfD2 = lfD2_Value(ldTotalSamples,lTrials);

    // Compute Equipment Variation : 5.15x Mean_of-ranges / d2
    ptGage->lfEV = (lfR_R_Nsigma*lfSumRanges/ldTotalSamples)/lfD2;

    // ***************************
    // Gage: "AV" Reproducibility
    qSort(lAppraiserAverage.begin(), lAppraiserAverage.end());
    // Compute diff between appraiser with max average and
    // appraiser with min average
    double lXbarDiff = lAppraiserAverage.last() - lAppraiserAverage.first();

    // Compute d2 for Reproducibility
    lfD2 = lfD2_Value(1,ldAppraisers);
    ptGage->lfAV = NAN;
    if ((ldMaxSamplesPerTrial*lTrials) > 0)
    {
        double lSquaredAV = GS_POW((lfR_R_Nsigma*lXbarDiff)/lfD2, 2) -
                (ptGage->lfEV*ptGage->lfEV)/(ldMaxSamplesPerTrial*lTrials);
        if (lSquaredAV >= 0)
            ptGage->lfAV = sqrt(lSquaredAV);
    }

    if ((ptGage->lfAV < 1e-20) || (lXbarDiff == 0))
        ptGage->lfAV = 0;

    // R&R
    //GCORE-10965 - GRR calculation, set AV to 0 so that GRR = EV and not 'nan'
    ptGage->lfRR = sqrt(GS_POW(isnan(ptGage->lfEV) ? 0 : ptGage->lfEV, 2) +
                            GS_POW(isnan(ptGage->lfAV) ? 0 : ptGage->lfAV, 2));

    if(ptGage->lfRR < 1e-20)
        ptGage->lfRR = 0;

    // Compute limit space (used in R&R)
    ComputeLimitsSpace(pFile,ptReferenceTestCell,lfLimitSpace);

    // Part Variation (PV)
    // Find the lowest mean & highest mean of trials per part.
    lfMaxMean= -C_INFINITE;
    lfMinMean= C_INFINITE;
    for(iIndex = 0; iIndex <ldMaxSamplesPerTrial; iIndex++)
    {
        for (iGroupID = 0; iGroupID < getGroupsList().count(); iGroupID++)
        {
            pGroup = getGroupsList().at(iGroupID);
            if (pGroup->pFilesList.isEmpty() == false)
            {
                // Handle to files in group.
                pFile = pGroup->pFilesList.at(0);
                // Get pointer to test structure
                pFile->FindTestCell(ptReferenceTestCell->lTestNumber,ptReferenceTestCell->lPinmapIndex,&ptTestCell,
                                    false,false, ptReferenceTestCell->strTestName.toLatin1().data());

                // Ensure test# exists in each dataset
                if(ptTestCell == NULL)
                    return;

                iDataOffset = 0;
                for(uTrialID=0;uTrialID < lTrials; uTrialID++)
                {
                    int nRsltIndex = iIndex + iDataOffset;
                    iDataOffset += ptTestCell->pSamplesInSublots[uTrialID];

                    if(!(ptTestCell->m_testResult.isValidIndex(nRsltIndex)))
                    {
                        GEX_ASSERT(false);
                        continue;
                    }

                    if(ptTestCell->m_testResult.isValidResultAt(nRsltIndex))
                    {
                        lValue = ptTestCell->m_testResult.resultAt(nRsltIndex);
                        lfTmpMean += lValue*ScalingPower(ptTestCell->res_scal);     // normalized
                    }
                }
            }
        };

        // Keep track of the highest mean of trials over the parts (required later for PartVariation (PV)
        lfMaxMean = gex_max(lfMaxMean, lfTmpMean);
        lfMinMean = gex_min(lfMinMean, lfTmpMean);
        lfTmpMean=0;
    }
    // Compute maximum mean over all trials and appraisers
    lfMaxMean /= (lTrials*ldAppraisers);
    lfMinMean /= (lTrials*ldAppraisers);

    lfD2 = lfD2_Value(1,ldMaxSamplesPerTrial);
    ptGage->lfPV = (lfR_R_Nsigma*(lfMaxMean - lfMinMean))/lfD2;

    // Total Variation (TV)
    ptGage->lfTV = sqrt((ptGage->lfRR*ptGage->lfRR) + (ptGage->lfPV*ptGage->lfPV) );
    if(ptGage->lfTV < 1e-20)
        ptGage->lfTV = 0;

    // Compute value as % of the test limits (if applicable)
    if (ReportOptions.GetOption("adv_boxplot", "delta").toString() == "over_tv")
    {
        double lRangeSpace = (ptGage->lfTV) ? ptGage->lfTV : 1;
        lfPercent = 100.0 * ptGage->lfRR / lRangeSpace;
        if (lfPercent < 1e-3)
        {
            lfPercent = 0;
        }
        else if (lfPercent >= 99.99)
        {
            lfPercent = 99.99;
        }
    }
    else if (lfLimitSpace > 0.0)
    {
        lfPercent = 100.0 * ptGage->lfRR / lfLimitSpace;
        if (lfPercent < 1e-3)
        {
            lfPercent = 0;
        }
        else if (lfPercent >= 99.99)
        {
            lfPercent = 99.99;
        }
    }
    else
    {
        lfPercent = 0;
    }
    ptGage->lfRR_percent = lfPercent;

    // GB (Guard Band): %RNR*Tolerance/100
    ptGage->lfGB = ptGage->lfRR_percent * lfLimitSpace / 100.0;

    // P/T ratio
    if(lfLimitSpace)
        ptGage->lfP_T = (100.0 * ptGage->lfRR) / lfLimitSpace;
    else
        ptGage->lfP_T = 0.0;	// lfLimitSpace=0, would make division by 0!

}

int CGexReport::GetResultPosition(CGexGroupOfFiles *aGroup, int aDiePos, int aTrialId, int aPartsCount)
{
    static bool lWarning = false;

    // iterate over pGroup to build parts list of the group
    QList<int> lTrialPartIdList;
    CGexFileInGroup* lTrialFile = aGroup->pFilesList.at(aTrialId);
    if (!lTrialFile)
        return -1;

    QStringList lRuns = lTrialFile->strRangeList.split(";");
    for (int lDieIt = 0; lDieIt < lRuns.size(); ++lDieIt)
    {
        if (lRuns.at(lDieIt) == QString("0 to 2147483647"))
        {
            if (! lWarning)
            {
                GS::Gex::Message::warning("", "No part list defined");
                lWarning = true;
            }
            return aDiePos + (aTrialId * aPartsCount);
        }
        lTrialPartIdList.append(lRuns.at(lDieIt).toInt());
    }

    // extract partID from die position
    int lPartId = lTrialPartIdList.at(aDiePos);
    // Find the position of partId in sorted partId list (read order)
    qSort(lTrialPartIdList.begin(), lTrialPartIdList.end(), qLess<int>());
    return (lTrialPartIdList.indexOf(lPartId) + (aTrialId * aPartsCount));
}

void CGexReport::ComputeTestXBarR(CTest * pReferenceTest)
{
    if (pReferenceTest->m_pXbarRDataset)
    {
        delete pReferenceTest->m_pXbarRDataset;
        pReferenceTest->m_pXbarRDataset = NULL;
    }

    CGexGroupOfFiles *	pGroup				= NULL;
    CGexFileInGroup *	pFile				= NULL;
    CTest *				pTest				= NULL;
    QString				strScalingOption	= m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    uint				uiAppraisersCount	= getGroupsList().count();
    uint				uiTrialsCount		= 0;
    uint				uiDevicesCount		= 0;
    bool				bValidTest			= false;

    // No data samples for this test!
    if(pReferenceTest->ldSamplesValidExecs > 0)
    {
        int nSamples = pReferenceTest->ldSamplesExecs;

        bValidTest = true;

        // Samples execs must be equal for each group
        for (int iGroupID = 0; iGroupID < getGroupsList().count() && bValidTest == true; iGroupID++)
        {
            pGroup			= getGroupsList().at(iGroupID);
            uiTrialsCount	= qMax((int) uiTrialsCount, pGroup->pFilesList.count());

            if (!pGroup->pFilesList.isEmpty())
            {
                // Handle to files in group.
                pFile = pGroup->pFilesList.first();

                // Get pointer to test structure
                pFile->FindTestCell(pReferenceTest->lTestNumber, pReferenceTest->lPinmapIndex, &pTest, false, false,
                                    pReferenceTest->strTestName.toLatin1().data());

                // Ensure test# exists in each dataset
                if(pTest == NULL || pTest->ldSamplesExecs != nSamples)
                    bValidTest = false;
            }
        }

        uiDevicesCount	= pReferenceTest->m_testResult.count() / uiTrialsCount;
    }

    if (bValidTest)
    {
        QString			strUnits;
        int				nPinmapIndex		= 0;
        int				nRefScaleFactor		= 0;
        int				nCustomScaleFactor	= 0;
        double			dCustomScaleFactor	= 0.0;
        QVector<double>	vecDataPoint(uiTrialsCount);

        pGroup		= (getGroupsList().isEmpty() == false) ? getGroupsList().first() : NULL;
        pFile		= (pGroup && pGroup->pFilesList.isEmpty() == false) ? pGroup->pFilesList.first() : NULL;
        strUnits	= pReferenceTest->GetScaledUnits(&dCustomScaleFactor, strScalingOption);

        // Allocate memory for XBar R dataset
        pReferenceTest->m_pXbarRDataset = GexXBarRDataset::create(uiTrialsCount, uiDevicesCount, uiAppraisersCount, strUnits);

        if (pReferenceTest->m_pXbarRDataset)
        {
            // Calculate XBar per appraiser
            for(uint uiAppraiser = 0; uiAppraiser < uiAppraisersCount; ++uiAppraiser)
            {
                pGroup	= getGroupsList().at(uiAppraiser);
                pFile	= pGroup->pFilesList.first();

                if(pReferenceTest->lPinmapIndex >= 0)
                    nPinmapIndex = pReferenceTest->lPinmapIndex;
                else
                    nPinmapIndex = GEX_PTEST;

                // Find the test cell for the current appraiser
                if (pFile && pFile->FindTestCell(pReferenceTest->lTestNumber, nPinmapIndex, &pTest, true, false,
                                                 pReferenceTest->strTestName.toLatin1().data()) == 1)
                {
                    if (uiAppraiser == 0)
                        nRefScaleFactor = pTest->res_scal;

                    nCustomScaleFactor = pTest->res_scal - nRefScaleFactor;
                    dCustomScaleFactor = 1/GS_POW(10.0, nCustomScaleFactor);

                    // Calculate XBar R valuefor each device
                    for(uint uiDevice = 0; uiDevice < uiDevicesCount; ++uiDevice)
                    {
                        vecDataPoint.clear();

                        // Retrieve all trial values for each device
                        for(uint uiTrial = 0; uiTrial < uiTrialsCount; ++uiTrial)
                        {
                            // Compute run index in the test result
                            int lRsltIndex = GetResultPosition(pGroup, uiDevice, uiTrial, uiDevicesCount);
                            if(pTest->m_testResult.isValidIndex(lRsltIndex))
                            {
                                if (pTest->m_testResult.isValidResultAt(lRsltIndex))
                                {
                                    vecDataPoint.append(pTest->m_testResult.resultAt(lRsltIndex) * dCustomScaleFactor);
                                }
                            }
                        }

                        // Add the datapoint for this device to the dataset
                        pReferenceTest->m_pXbarRDataset->addDataPoint(uiAppraiser, uiDevice, vecDataPoint);
                    }

                    // Set low limit
                    if((pTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    {
                        double dLimit = pTest->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists

                        // If we have to keep values in normalized format, do not rescale!
                        if (strScalingOption != "normalized")
                        {
                            // convert LowLimit to same scale as results:
                            pFile->FormatTestResultNoUnits(&dLimit, pTest->llm_scal);
                            dLimit *=  ScalingPower(pTest->llm_scal);	// normalized
                            dLimit /=  ScalingPower(pTest->res_scal);	// normalized
                        }

                        if(nCustomScaleFactor)
                            dLimit *= dCustomScaleFactor;

                        pReferenceTest->m_pXbarRDataset->setLowTestLimit(uiAppraiser, dLimit);
                    }

                    // Set High limit
                    if((pTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    {
                        double dLimit = pTest->GetCurrentLimitItem()->lfHighLimit;		// Low limit exists

                        // If we have to keep values in normalized format, do not rescale!
                        if (strScalingOption != "normalized")
                        {
                            // convert LowLimit to same scale as results:
                            pFile->FormatTestResultNoUnits(&dLimit, pTest->hlm_scal);
                            dLimit *=  ScalingPower(pTest->hlm_scal);	// normalized
                            dLimit /=  ScalingPower(pTest->res_scal);	// normalized
                        }

                        if(nCustomScaleFactor)
                            dLimit *= dCustomScaleFactor;

                        pReferenceTest->m_pXbarRDataset->setHighTestLimit(uiAppraiser, dLimit);
                    }
                }

                // Set appraiser name
                pReferenceTest->m_pXbarRDataset->setAppraiserName(uiAppraiser, pGroup->strGroupName);
            }

            // When all data have been added to the dataset, compute the control limits
            pReferenceTest->m_pXbarRDataset->computeControlLimits();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Writes boxplot line of ONE test (may be multiple lines if comparing files)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvBoxPlotLines(CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int& iLine)
{
    CGexGroupOfFiles *	pGroup;
    CTest			 *	ptTestCell;	// Pointer to test cell of groups2 or higher
    char				szString[300];
    int					iSizeX,iSizeY,iMiddleY;
    double				lfMean,lfDrawMean=0.0,lfHighL,lfLowL;

    // Variables to compute where to draw the box plot - sigma line
    double				lfQuartile1,lfQuartile2,lfQuartile3;
    double				lfDrawQ1=0.0,lfDrawQ2=0.0,lfDrawQ3=0.0;
    double				lfMin,lfMax;
    double				lfPosition;
    double				aCoeff=0.0;
    double				bCoeff=0.0;
    double				lfLowWindowMarker;
    double				lfHighWindowMarker;
    const char *		szBackgroundColor;	// Holds std or RED color depends if test drift alarm
    const char *		pChar;
    const char *		ptChar;
    double				fData;
    int					lPinmapIndex;
    int					iGroup=1;	// Counter used to know on which group we are (#1 or other)
    int					iPenSize;
    int					iFontSize;
    bool				bAllDataIdentical;
    QString				strImage,strImagePath;
    QString				strBookmark;
    bool                firstGroup(false);

    // Variables for GAGE R&R computations
    double	lfLimitSpace,lfRangeSpace;
    double	lfRepetability=0.0;	// dataset repeatability
    static  int	iTotalRows;

    // Image size to create for the Gage R&R chart
    iSizeX=350;	// Painting X area size;
    iSizeY=45;	// Painting Y area size;
    iMiddleY = iSizeY/2;
    lfLowWindowMarker = 0.1*iSizeX;	// Low Value marker position (either low limit if one group, or SamplesMin if multiple groups)
    lfHighWindowMarker=0.9*iSizeX;	// High value marker position (either high limit if one group, or SamplesMax if multiple groups)
    if(m_pReportOptions->iGroups == 1)
        iPenSize = 1;
    else
        iPenSize = 2;

#if defined unix || __MACH__
    iFontSize = 10;	// Unix Font size for writing legends on chart
#else
    iFontSize = 9;	// PC Font size for writing legends on chart
#endif

    // Create boxplot pixmap image...
    QString	strLowL,strHighL,strText;
    QPainter p;
    QPixmap  pm( iSizeX, iSizeY );
    QFont	 penFont(QFont(GEXPLOT_GAGE_FONTNAME, iFontSize,QFont::Normal));
    QPen     penBoxOutline(Qt::black,1,Qt::SolidLine );
    QPen     penSecondary(Qt::blue,1,Qt::DotLine );
    QPen     penPrimery(Qt::blue,1,Qt::SolidLine );
    QPen     penDrawingColor(Qt::green,iPenSize,Qt::SolidLine);;
    QColor	 cBackgroundColor(248,248,248);
    QColor	 cBrightColor;

    // Check if need to create new HTML Boxplot. page
    CheckForNewHtmlPage(NULL,SECTION_ADV_BOXPLOT,ptTestReferenceCell->iHtmlAdvancedPage);

    // 'adv_boxplot', 'field' flag option list
    QString strAdvBoxplotFieldOptions = (m_pReportOptions->GetOption(QString("adv_boxplot"), QString("field"))).toString();
    QStringList qslAdvBoxplotFieldOptionsList = strAdvBoxplotFieldOptions.split(QString("|"));

    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");

    double lfR_R_Nsigma			= ReportOptions.GetOption("adv_boxplot","r&r_sigma").toDouble();
    QString strOutputFormat		= m_pReportOptions->GetOption("output", "format").toString();
    QString strPageFormat		= m_pReportOptions->GetOption("output", "paper_format").toString();
    QString strBoxplotShiftOver = ReportOptions.GetOption("adv_boxplot", "delta").toString();
    bool bBoxplotShiftOverTV = (strBoxplotShiftOver == "over_tv");
    bool bChartOverLimits=true;

    // new ReportOption
    {
        QString strOptionStorageDevice;
        strOptionStorageDevice = (m_pReportOptions->GetOption("adv_boxplot","chart_type")).toString();

        if (strOptionStorageDevice == QString("range"))
            bChartOverLimits = false;
        else if (strOptionStorageDevice == QString("adaptive"))
        {
            if(m_pReportOptions->iGroups == 1)
                bChartOverLimits = true;
            else
                bChartOverLimits = false;
        }
        else			// strOptionStorageDevice == QString("limits") (default value)
            bChartOverLimits = true;
    }

    // Insert the line labels every 15 lines...eases reading reports!
    if(iLine == 0)
        WriteBoxPlotLabelLine(bChartOverLimits,iTotalRows);

    // Create one Gage R&R line per test of each group...
    for (iGroup = 0; iGroup < getGroupsList().count(); iGroup++)
    {
        iLine++;

        pGroup = getGroupsList().at(iGroup);

        // Define drawing color
        penDrawingColor.setColor(GetChartingColor(iGroup));
        // List of tests in group#1, then 2, etc...
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // List of tests in this group.
        ptTestCell = pGroup->cMergedData.ptMergedTestList;

        // Find test cell: RESET list to ensure we scan list of the right group!
        // Check if pinmap index...
        if(ptTestReferenceCell->lPinmapIndex >= 0)
            lPinmapIndex = ptTestReferenceCell->lPinmapIndex;
        else
            lPinmapIndex = GEX_PTEST;
        if(pFile->FindTestCell(ptTestReferenceCell->lTestNumber,lPinmapIndex,&ptTestCell,false,false,
                               ptTestReferenceCell->strTestName.toLatin1().data())!=1)
            continue;

        // Chart over limits.
        if(bChartOverLimits)
        {
            // Define LowLimit
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
                // legend for low limit value
                strLowL = pFile->FormatTestLimit(ptTestCell,szString,lfLowL,ptTestCell->llm_scal);
                // If we have to keep values in normalized format, do not rescale!
                if(!isScalingNormalized)
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->llm_scal);
                    lfLowL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                    lfLowL /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }
            }
            else
            {
                // Low limit doesn't exist, assume one.
                lfLowL = ptTestCell->lfSamplesMin;	// No low limit...create one artificial
                if(lfLowL > 0)
                    lfLowL /= 2;
                else
                    lfLowL *= 2;
                // legend for low limit value
                strLowL = pFile->FormatTestResult(ptTestCell,lfLowL,ptTestCell->res_scal);
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->res_scal);//scale
            }

            // Define HighLimit
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
                // legend for high limit value
                strHighL = pFile->FormatTestLimit(ptTestCell,szString,lfHighL,ptTestCell->hlm_scal);
                // If we have to keep values in normalized format, do not rescale!
                if(!isScalingNormalized)
                {
                    // convert HighLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->hlm_scal);
                    lfHighL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                    lfHighL /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }
            }
            else
            {
                // High limit doesn't exist, assume one.
                lfHighL = ptTestCell->lfSamplesMax;	// No High limit...create one artificial
                if(lfHighL > 0)
                    lfHighL *= 2;
                else
                    lfHighL /= 2;
                // legend for high limit value
                strHighL = pFile->FormatTestResult(ptTestCell,lfHighL,ptTestCell->res_scal);
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->res_scal);//scale
            }
        }
        else
        {
            // Chart over range of data
            if(iGroup == 0)
            {
                lfLowL = C_INFINITE;
                lfHighL = -C_INFINITE;

                QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

                while(itGroupsList.hasNext())
                {
                    // Hanlde to files in group.
                    pGroup	= itGroupsList.next();
                    pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                    pFile->FindTestCell(ptTestReferenceCell->lTestNumber,lPinmapIndex,&ptTestCell,false,false,
                                        ptTestReferenceCell->strTestName.toLatin1().data());

                    if(ptTestCell && ptTestCell->lfSamplesMin < lfLowL)
                    {
                        lfLowL = ptTestCell->lfSamplesMin;
                        // legend for low limit value
                        strLowL = pFile->FormatTestResult(ptTestCell,lfLowL,ptTestCell->res_scal);
                    }

                    if(ptTestCell && ptTestCell->lfSamplesMax > lfHighL)
                    {
                        lfHighL = ptTestCell->lfSamplesMax;
                        // legend for high limit value
                        strHighL = pFile->FormatTestResult(ptTestCell,lfHighL,ptTestCell->res_scal);
                    }
                };

                // Rewind to first group!
                pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();	// Group#1
                pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
                pFile->FindTestCell(ptTestReferenceCell->lTestNumber,lPinmapIndex,&ptTestCell,false,false,
                                    ptTestReferenceCell->strTestName.toLatin1().data());

                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->res_scal);//scale
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->res_scal);//scale

            }
        }

        // Compute limit space (used in R&R)
        ComputeLimitsSpace(pFile,ptTestCell,lfLimitSpace);

        ////////////////////////////////////////
        // Draw Gage R&R markers.
        ////////////////////////////////////////

        m_cStats.ComputeAdvancedDataStatistics_Quartiles(ptTestCell);

        // Scale Mean to correct scale
        lfMean = ptTestCell->lfMean;
        pFile->FormatTestResultNoUnits(&lfMean,ptTestCell->res_scal);

        // Scale Quartile1 to correct scale
        lfQuartile1 = ptTestCell->lfSamplesQuartile1;
        pFile->FormatTestResultNoUnits(&lfQuartile1,ptTestCell->res_scal);

        // Scale Quartile2 (Median) to correct scale
        lfQuartile2 = ptTestCell->lfSamplesQuartile2;
        pFile->FormatTestResultNoUnits(&lfQuartile2,ptTestCell->res_scal);

        // Scale Quartile3 to correct scale
        lfQuartile3 = ptTestCell->lfSamplesQuartile3;
        pFile->FormatTestResultNoUnits(&lfQuartile3,ptTestCell->res_scal);

        // Checl if the space size to paint is 0.
        if(lfHighL == lfLowL)
        {
            bAllDataIdentical = true;
        }
        else
        {
            // We have a non-null space to chart
            bAllDataIdentical = false;
            // DrawingPint = a.DataValue + b
            aCoeff = (lfHighWindowMarker-lfLowWindowMarker)/(lfHighL-lfLowL);
            bCoeff = lfLowWindowMarker - aCoeff*lfLowL;
            // Compute the Mean position on graph.
            lfDrawMean = (aCoeff*lfMean) + bCoeff;
            // Compute the Quartiles position on graph.
            lfDrawQ1 = (aCoeff*lfQuartile1) + bCoeff;
            lfDrawQ2 = (aCoeff*lfQuartile2) + bCoeff;
            lfDrawQ3= (aCoeff*lfQuartile3) + bCoeff;
        }

        if(strOutputFormat!="CSV")
        {
#if 0
            // Create ChartDirector BoxPlot image.
            XYChart *pBoxPlot = new XYChart(iSizeX,iSizeY,0xF8F8F8);

            //Set the plot area Enable both horizontal and vertical grids by setting their colors to grey (0xc0c0c0)
            pBoxPlot->setPlotArea(0, 0, iSizeX,(2*iSizeY)/3);

            // Make Boxplot horizontal
            pBoxPlot->swapXY();

            //Set the font for the x axis labels to Arial (remember: Axis SWAP flag set)
            pBoxPlot->yAxis()->setLabelStyle("arial.ttf");

            int iTotalVisibleLayers=1;
            double	lfIQR,lfLowWhisker,lfHighWhisker;
            double	lfLayerIndex;
            double Q0Data;	// Low Whisker.
            double Q1Data;	// Q1
            double Q2Data;	// Q2: Median
            double Q3Data;	// Q3
            double Q4Data;	// High Whisker.
            int	   iQColors;// Layer color
            Mark *pMark;								// Handle to lines / Markers

            // Plot all the layers
            QString			strLabelY;
            int				iLineWidth;
            int				iTestReferenceScaleFactor=0,iCustomScaleFactor=0;
            double			fCustomScaleFactor;
            int				iIndex;
            int				iColor;
            int				iPenWidth;
            double			lfValue;
            double			lfLowL;
            double			lfHighL;
            double			lfChartBottom=C_INFINITE;
            double			lfChartTop=-C_INFINITE;
            double			lfMin,lfMax,lfMean;

            if((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                // Draw LL limit markers (done only once: when charting Plot for group#1)
                lfLowL = ptTestCell->lfLowLimit;		// Low limit exists
                // If we have to keep values in normalized format, do not rescale!
                if(pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->llm_scal);
                    lfLowL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                    lfLowL /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }
                // LowLimit Marker
                iColor = 0xff0000;
                iPenWidth = 1;
                // Requested to show limits.
                if(iPenWidth)
                {
                    //Add marker
                    pMark = pBoxPlot->yAxis()->addMark(lfLowL, iColor,"Low L.");
                    //Set the mark line width to x pixels
                    pMark->setLineWidth(iPenWidth);

                    //Align label
                    pMark->setAlignment(Chart::Left);
                }
            }
            else
                lfLowL = -C_INFINITE;	// Say: no Low limit!

            if((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                // Draw HL limit markers (done only once: when charting Plot for group#1)
                lfHighL = ptTestCell->lfHighLimit;		// High limit exists
                // If we have to keep values in normalized format, do not rescale!
                if(pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
                {
                    // convert HighLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->hlm_scal);
                    lfHighL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                    lfHighL /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }
                // High limit Marker
                iColor = 0xff0000;
                iPenWidth = 1;

                // Request to show limits
                if(iPenWidth)
                {
                    //Add marker
                    pMark = pBoxPlot->yAxis()->addMark(lfHighL, iColor,"High L.");
                    //Set the mark line width to x pixels
                    pMark->setLineWidth(iPenWidth);

                    //Align label
                    pMark->setAlignment(Chart::Right);
                }
            }
            else
                lfHighL = C_INFINITE;	// Say: no High limit!


            lfIQR = ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile1;
            lfLowWhisker = ptTestCell->lfSamplesQuartile2 - 1.5*lfIQR;
            lfHighWhisker = ptTestCell->lfSamplesQuartile2 + 1.5*lfIQR;
            lfMean = ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs;
            pFile->FormatTestResultNoUnits(&lfMean,ptTestCell->res_scal);

            Q0Data = ptTestCell->lfSamplesMin;	// Low Whisker.
            Q1Data = ptTestCell->lfSamplesQuartile1;				// Q1
            pFile->FormatTestResultNoUnits(&Q1Data,ptTestCell->res_scal);
            Q2Data = ptTestCell->lfSamplesQuartile2;				// Q2: Median
            pFile->FormatTestResultNoUnits(&Q2Data,ptTestCell->res_scal);
            Q3Data = ptTestCell->lfSamplesQuartile3;				// Q3
            pFile->FormatTestResultNoUnits(&Q3Data,ptTestCell->res_scal);
            Q4Data = ptTestCell->lfSamplesMax;	// High Whisker.

            // Get Box color
            iQColors= 0x00ff00;			// Boxplot color

            // Insert Mean marker
            lfLayerIndex = 0;				// Layer index
            pBoxPlot->addScatterLayer(DoubleArray(&lfLayerIndex, 1),DoubleArray(&lfMean, 1),"",Chart::CrossShape(0.2),20,0x008000,0x008000);

            // Show outliers!
#if 0
            for(iIndex = 0; iIndex < ptTestCell->ldSamplesExecs;iIndex++)
            {
                lfValue = ptTestCell->m_testResult.resultAt(iIndex);
                if((lfValue != GEX_C_DOUBLE_NAN) && (lfValue < lfLowWhisker || lfValue > lfHighWhisker))
                    pBoxPlot->addScatterLayer(DoubleArray(&lfValue, 1),DoubleArray(&lfLayerIndex, 1),"",Chart::Cross2Shape(0.3),10,0xFFC62A);
            }
#endif
            // Update viewport space if required
            switch(pReportOptions->iBoxplotCharting)
            {
                case GEX_BOXPLOTTYPE_LIMITS:
                    // Chart has to be done over limits...unless they do not exist!
                    if(lfLowL != -C_INFINITE)
                            lfChartBottom = lfLowL;	// If chart over limits, update charting
                    else
                        lfChartBottom = gex_min(lfChartBottom,ptTestCell->lfSamplesMin);

                    if(lfHighL != C_INFINITE)
                            lfChartTop = lfHighL;	// If chart over limits, update charting
                    else
                            lfChartTop = gex_max(lfChartTop,ptTestCell->lfSamplesMax);
                    break;

                case GEX_BOXPLOTTYPE_RANGE:
                    // Chart has to be done over data (min & max)
                    lfChartBottom = gex_min(lfChartBottom,ptTestCell->lfSamplesMin);
                    lfChartTop = gex_max(lfChartTop,ptTestCell->lfSamplesMax);
                    break;

                case GEX_BOXPLOTTYPE_ADAPTIVE:
                    // Chart has to be done over maxi of both datapoints & limits
                    if(lfLowL != -C_INFINITE)
                            lfChartBottom = gex_min(lfChartBottom,lfLowL);
                    if(lfHighL != C_INFINITE)
                            lfChartTop = gex_max(lfChartTop,lfHighL);
                    lfChartBottom = gex_min(lfChartBottom,ptTestCell->lfSamplesMin);
                    lfChartTop = gex_max(lfChartTop,ptTestCell->lfSamplesMax);
                    break;
            }



            // Viewport: add 5% extra space at each end (remember: Axis SWAP flag set)
            double	lfExtra = (lfChartTop-lfChartBottom) * 0.05;	// Extra space +/- 5%
            pBoxPlot->xAxis()->setLinearScale(lfChartBottom-lfExtra,lfChartTop+lfExtra);

            // Set X axis format (because of Axis SWAP flag)
            pBoxPlot->yAxis()->setLabelFormat("{value|g}");

            // Next layer: Box Whisker layer. Set the line width to 2 pixels
            BoxWhiskerLayer *pWhiskerLayer = pBoxPlot->addBoxWhiskerLayer2(
                DoubleArray(&Q3Data, 1),
                DoubleArray(&Q1Data, 1),
                DoubleArray(&Q4Data, 1),
                DoubleArray(&Q0Data, 1),
                DoubleArray(&Q2Data, 1),
                IntArray(&iQColors,1),
                0.5);

            // Set whisker line size
            pWhiskerLayer->setLineWidth(1);

            QImage cImage;
            // Convert ChartDirector image to Bitmap
            MemBlock m = pBoxPlot->makeChart(BMP);
            cImage.loadFromData((unsigned char*) m.data,m.len);

            // Draw image
            pm.convertFromImage(cImage);

#else
            pm.fill(cBackgroundColor);  // fills pm with color
            p.begin(&pm);

            // Draw grid
            p.setPen(penSecondary);
            // Middle horizontal line
            p.drawLine(0, iMiddleY, iSizeX, iMiddleY);
            // Middle Low-High limits vertical line
            p.drawLine(iSizeX/2, 0, iSizeX/2, iSizeY);

            p.setPen(penPrimery);
            // LowLimit vertical Marker
            p.drawLine((int)(lfLowWindowMarker-1.0), iMiddleY, (int)(lfLowWindowMarker-1.0), iMiddleY+6);

            // HighLimit vertical line
            p.drawLine((int)lfHighWindowMarker, iMiddleY, (int)lfHighWindowMarker, iMiddleY+6);

            // Check if data are all exactly the same...
            if(bAllDataIdentical)
            {
                // Daw small box centered on Mean
                penBoxOutline = penDrawingColor;
                penBoxOutline.setColor(penBoxOutline.color().darker());
                penBoxOutline.setWidth(1);
                p.setPen(penBoxOutline);

                // Fill with bright color
                cBrightColor = penDrawingColor.color();
                cBrightColor = cBrightColor.lighter();
                p.setBrush(QBrush(cBrightColor));
                p.drawRect(iSizeX/2,iMiddleY-5,2,10);
            }
            else
            {
                // Daw small box centered on Mean
                penBoxOutline = penDrawingColor;
                penBoxOutline.setColor(penBoxOutline.color().darker());
                penBoxOutline.setWidth(1);
                p.setPen(penBoxOutline);

                // Fill with bright color
                cBrightColor = penDrawingColor.color();
                cBrightColor = cBrightColor.lighter();
                p.setBrush(QBrush(cBrightColor));
                p.drawRect((int)lfDrawQ1,iMiddleY-5,(int)(lfDrawQ3-lfDrawQ1+1.0),10);

                // Draw Minimum (low-end) of the samples.
                lfMin = ptTestCell->lfSamplesMin;
                pFile->FormatTestResultNoUnits(&lfMin,ptTestCell->res_scal);
                lfPosition = (aCoeff*lfMin) + bCoeff;
                p.drawLine((int)(lfPosition-1.0), iMiddleY-5, (int)(lfPosition-1.0), iMiddleY+5);
                penDrawingColor.setWidth(1);
                p.setPen(penDrawingColor);

                // Connect 'Minimum' marker to Box edge
                p.drawLine((int)lfPosition, iMiddleY, (int)lfDrawQ1, iMiddleY);

                // Draw Maximum (high-end) of the samples.
                penDrawingColor.setWidth(1);
                p.setPen(penDrawingColor);
                lfMax = ptTestCell->lfSamplesMax;
                pFile->FormatTestResultNoUnits(&lfMax,ptTestCell->res_scal);
                lfPosition = (aCoeff*lfMax) + bCoeff;

                p.drawLine((int)lfPosition, iMiddleY-5, (int)lfPosition, iMiddleY+5);
                penDrawingColor.setWidth(1);
                p.setPen(penDrawingColor);

                // Connect 'Maximum' marker to Box edge
                p.drawLine((int)lfPosition, iMiddleY, (int)lfDrawQ3, iMiddleY);

                // Draw Q2: Median (Quartile2)
                p.drawLine((int)lfDrawQ2, iMiddleY-7, (int)lfDrawQ2, iMiddleY+7);

                // Draw Mean as a '+'
                p.drawLine((int)lfDrawMean, iMiddleY-3, (int)lfDrawMean, iMiddleY+4);
                p.drawLine((int)(lfDrawMean-3.0), iMiddleY, (int)(lfDrawMean+4.0), iMiddleY);
            }

            // Write legend for Lowest value in viewport
            p.setPen(penPrimery);
            p.setFont(penFont);
            p.drawText((int)(lfLowWindowMarker-20.0),iSizeY,strLowL);

            // Write legend for Highest value in viewport
            p.setFont(penFont);
            p.drawText((int)(lfHighWindowMarker-20.0),iSizeY,strHighL);

            // End of pixmap writing
            p.end();
#endif
            // Save file into .PNG file!
            strImage =
                BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                     "/images/adv", ptTestCell, iGroup);
            strImagePath = m_pReportOptions->strReportDirectory + "/images/";
            strImagePath += strImage;
            pm.save(strImagePath,"PNG");

            // if Histogram acivtated, point to histogram for that test
            if (ptTestReferenceCell->iHtmlHistoPage)
            {
                // Bookmark: are in same page if FLAT HTML page is generated
                if (strOutputFormat=="HTML")
                    strBookmark.sprintf("histogram%d.htm#HistoT",ptTestReferenceCell->iHtmlHistoPage);
                else
                    strBookmark = "#HistoT";	// Histogram of Test bookmark header string.
            }
            else if (ptTestReferenceCell->iHtmlStatsPage)
            {
                // Bookmark: are in same page if FLAT HTML page is generated
                if(strOutputFormat=="HTML")
                    strBookmark.sprintf("stats%d.htm#StatT",ptTestReferenceCell->iHtmlStatsPage);
                else
                    strBookmark = "#StatT";	// Test Statistics bookmark header string.
            }
            else
                strBookmark = "";
        }	// HTML output format.

        // Generating Test header table.
        if(strOutputFormat=="CSV")
        {
            // Test name
            fprintf(hReportFile,"%s,%s,",ptTestCell->szTestLabel, ptTestCell->strTestName.toLatin1().constData());
        }
        else
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td bgcolor=%s>",szDataColor);

            if (strBookmark.isEmpty())
                fprintf(hReportFile,"%s</td>\n", ptTestCell->szTestLabel);
            else
                fprintf(hReportFile,"<a name=\"AdvT%s\"></a> <a href=\"%s%s\">%s</a></td>\n",
                        ptTestCell->szTestLabel, strBookmark.toLatin1().constData(),
                        ptTestCell->szTestLabel, ptTestCell->szTestLabel);

            fprintf(hReportFile,"<td bgcolor=%s>%s</td>\n",szDataColor, ptTestCell->strTestName.toLatin1().constData());
        }

        // If multiple groups, show group names
        if(m_pReportOptions->iGroups > 1)
        {
            if(strOutputFormat=="CSV")
                fprintf(hReportFile,"%s,",pGroup->strGroupName.toLatin1().constData());
            else
                fprintf(hReportFile,"<td bgcolor=%s><font color=\"%s\"><b>%s</font></td>",szDataColor,GetChartingHtmlColor(iGroup),pGroup->strGroupName.toLatin1().constData());
        }

        // Check if display Test Limits
        if(qslAdvBoxplotFieldOptionsList.contains(QString("limits")))
        {
            if(strOutputFormat=="CSV")
            {
                fprintf(hReportFile,"%s,",ptTestCell->GetCurrentLimitItem()->szLowL);
                fprintf(hReportFile,"%s,",ptTestCell->GetCurrentLimitItem()->szHighL);
            }
            else
            {
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,ptTestCell->GetCurrentLimitItem()->szLowL);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,ptTestCell->GetCurrentLimitItem()->szHighL);
            }
        }

        // Check if display Min
        if(qslAdvBoxplotFieldOptionsList.contains(QString("tolerance")))
        {
            char			szTolerance[512];
            double			dTolerance = 0.0;
            CGexFileInGroup cFile(NULL,0,"",0,0,"","","");	// in order to use FormatTestResultNoUnits(...)

            // Compute limit space
            ComputeLimitsSpace(NULL, ptTestCell, dTolerance);

            cFile.FormatTestLimit(ptTestCell, szTolerance, dTolerance, ptTestCell->res_scal);

            if(strOutputFormat=="CSV")
                fprintf(hReportFile,"%s,", szTolerance);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor, szTolerance);
        }

        // Check if display #Min
        if(qslAdvBoxplotFieldOptionsList.contains(QString("global_min")))
        {
            if (iGroup == 0)
            {
                ptChar = (ptTestReferenceCell->m_pXbarRDataset) ? pFile->FormatTestResult(ptTestReferenceCell, ptTestReferenceCell->m_pXbarRDataset->minSample(), ptTestCell->res_scal) : "n/a";

                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,", ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor, ptChar);
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,",,");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ></td>",szDataColor);
            }
        }

        // Check if display #Max
        if(qslAdvBoxplotFieldOptionsList.contains(QString("global_max")))
        {
            if (iGroup == 0)
            {
                ptChar = (ptTestReferenceCell->m_pXbarRDataset) ? pFile->FormatTestResult(ptTestReferenceCell, ptTestReferenceCell->m_pXbarRDataset->maxSample(), ptTestCell->res_scal) : "n/a";

                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,", ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor, ptChar);
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,",,");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ></td>",szDataColor);
            }
        }

        // Check if display #Mean
        if(qslAdvBoxplotFieldOptionsList.contains(QString("global_mean")))
        {
            if (iGroup == 0)
            {
                ptChar = (ptTestReferenceCell->m_pXbarRDataset) ? pFile->FormatTestResult(ptTestReferenceCell, ptTestReferenceCell->m_pXbarRDataset->meanSample(), ptTestCell->res_scal) : "n/a";
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,", ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor, ptChar);
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,",,");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ></td>",szDataColor);
            }
        }

        TestShift lTestShift;
        GS::Core::MLShift lMLShift;
        if (ptTestCell->mTestShifts.size() > 0)
        {
            lTestShift = ptTestCell->mTestShifts.first();
            lMLShift = lTestShift.GetMlShift(ptTestCell->GetCurrentLimitItem());
        }

        // Check if display Mean/Shift
        if(qslAdvBoxplotFieldOptionsList.contains(QString("mean")))
        {
            ptChar = pFile->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal);
            if(m_pReportOptions->iGroups == 1)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,",ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
            }
            else
            {
                if(iGroup == 0)
                {
                    szBackgroundColor = szDataColor;	// Reference group: no alarm possible!
                    fData = 0;	// No Mean drift since it is the reference!
                }
                else
                {
                    // Group#2 and higher, check for drift alarm!
                    if(ptTestCell->iAlarm & GEX_ALARM_MEANSHIFT)
                        szBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
                    else
                        szBackgroundColor = szDataColor;	// FINE...no alarm
                    fData = lMLShift.mMeanShiftPct;
                }
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,%.2f %%,",ptChar,fData);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s<br>%.2f %%</td>\n",szBackgroundColor,ptChar,fData);
            }
        }
        // Check if display Sigma/Shift
        if(qslAdvBoxplotFieldOptionsList.contains(QString("sigma")))
        {
            ptChar = pFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal);

            if(m_pReportOptions->iGroups == 1)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,",ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
            }
            else
            {
                if(iGroup == 0)
                {
                    szBackgroundColor = szDataColor;	// Reference group: no alarm possible!
                    fData = 0;	// No Mean drift since it is the reference!
                }
                else
                {
                    // Group#2 and higher, check for drift alarm!
                    if(ptTestCell->iAlarm & GEX_ALARM_SIGMASHIFT)
                        szBackgroundColor = szAlarmColor;	// ALARM!: Sigma shift over limit.
                    else
                        szBackgroundColor = szDataColor;	// FINE...no alarm
                    fData = lTestShift.mSigmaShiftPercent;
                }
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,%.2f %%,",ptChar,fData);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s<br>%.2f %%</td>\n",szBackgroundColor,ptChar,fData);
            }
        }
        // Check if display Median/Shift
        if(qslAdvBoxplotFieldOptionsList.contains(QString("median")))
        {
            ptChar = pFile->FormatTestResult(ptTestCell,ptTestCell->lfSamplesQuartile2,ptTestCell->res_scal);
            if(strOutputFormat=="CSV")
                fprintf(hReportFile,"%s,",ptChar);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
        }
        // Check if display Cp/Shift
        if(qslAdvBoxplotFieldOptionsList.contains(QString("cp")))
        {
            ptChar = CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp);
            if(m_pReportOptions->iGroups == 1)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,",ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
            }
            else
            {
                if(iGroup == 0)
                    pChar = "0.00";	// No Cp drift since it is the reference!
                else
                    pChar = CreateResultStringCpCrCpkShift(lMLShift.mCpShift);

                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,%s %%,",ptChar,pChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s<br>%s %%</td>\n",szDataColor,ptChar,pChar);
            }
        }
        // Check if display Cpk/Shift
        if(qslAdvBoxplotFieldOptionsList.contains(QString("cpk")))
        {
            ptChar = CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk);
            if(m_pReportOptions->iGroups == 1)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,",ptChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
            }
            else
            {
                if(iGroup == 0)
                {
                    firstGroup = true;
                    szBackgroundColor = szDataColor;	// Reference group: no alarm possible!
                    fData = 0;	// No Cpk drift since it is the reference!
                    pChar = "0.00";	// No Cpk drift since it is the reference!
                }
                else
                {
                    // Group#2 and higher, check for drift alarm!
                    if(ptTestCell->iAlarm & GEX_ALARM_CPKSHIFT)
                        szBackgroundColor = szAlarmColor;	// ALARM!: Cpk shift over limit.
                    else
                        szBackgroundColor = szDataColor;	// FINE...no alarm
                    if((iGroup == 1) && (firstGroup == false))
                    {
                        firstGroup = true;
                        pChar = "0.00";	// No Cpk drift since it is the reference!
                    }
                    else
                        pChar = CreateResultStringCpCrCpkShift(lMLShift.mCpkShift);
                }
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,%s %%,",ptChar,pChar);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s<br>%s %%</td>\n",szBackgroundColor,ptChar,pChar);
            }
        }

        // Show dataset Repeatability
        if(qslAdvBoxplotFieldOptionsList.contains(QString("repeatability")))
        {
            lfRepetability = lfR_R_Nsigma*ptTestCell->lfSigma;

            ptChar = pFile->FormatTestResult(ptTestCell,lfRepetability,ptTestCell->res_scal);

            if(strOutputFormat=="CSV")
                fprintf(hReportFile,"%s,",ptChar);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>",szDataColor,ptChar);
        }

        // ***************************
        // GAGE R&R computations
        // ***************************

        // Check if % computed over TV or Limit space
        if(bBoxplotShiftOverTV)
            lfRangeSpace = (ptTestReferenceCell->pGage && ptTestReferenceCell->pGage->lfTV) ? ptTestReferenceCell->pGage->lfTV : 1;
        else
            lfRangeSpace = (lfLimitSpace) ? lfLimitSpace : 1;

        // Check if display Parts In Control
        if(qslAdvBoxplotFieldOptionsList.contains(QString("parts_in_control")))
        {
            if (iGroup == 0)
            {
                if (ptTestReferenceCell->m_pXbarRDataset)
                {
                    if(strOutputFormat=="CSV")
                        fprintf(hReportFile,"%d,", ptTestReferenceCell->m_pXbarRDataset->goodDeviceCount());
                    else
                        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%d</td>",szDataColor, ptTestReferenceCell->m_pXbarRDataset->goodDeviceCount());
                }
                else
                {
                    if(strOutputFormat=="CSV")
                        fprintf(hReportFile,"n/a,");
                    else
                        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >n/a</td>",szDataColor);
                }
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,",");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;</td>",szDataColor);
            }
        }

        // Gage: "EV" Repeatability............
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfEV;
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("ev"))))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,lfRangeSpace,false);

        // Check if display Appraiser EV
        if(qslAdvBoxplotFieldOptionsList.contains(QString("local_ev")))
        {
            double dPercent = -1;
            double dLocalEV = (ptTestReferenceCell->m_pXbarRDataset) ? ptTestReferenceCell->m_pXbarRDataset->EVAppraiser(iGroup) : std::numeric_limits<double>::quiet_NaN();

            // Compute value as % of the test limits (if applicable)
            if(lfLimitSpace > 0.0 && !isnan(dLocalEV))
            {
                dPercent = 100.0 * dLocalEV / lfLimitSpace;

                if(dPercent < 1e-3)
                    dPercent = 0;
                else if(dPercent >= 100.00)
                    dPercent = 100.00;
            }

            // Write Value
            if (isnan(dLocalEV))
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"n/a,,");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >n/a",szDataColor);
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%g,", dLocalEV);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%g %s",szDataColor, dLocalEV, ptTestCell->szTestUnits);

                // Write percentage (if applicable)
                if(dPercent >= 0)
                {
                    if(strOutputFormat=="CSV")
                        fprintf(hReportFile,"%.2f %%,", dPercent);
                    else
                        fprintf(hReportFile,"<br>(%.2f %%)</td>", dPercent);
                }
                else
                {
                    if(strOutputFormat=="CSV")
                        fprintf(hReportFile,",");
                    else
                        fprintf(hReportFile,"</td>");
                }
            }
        }

        // Gage: "AV" Reproducibility
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfAV;
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("av"))))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,lfRangeSpace,false);

        // R&R
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfRR;
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("r&r"))))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,lfRangeSpace,true);

        // Check if display #R&R
        fData = (ptTestReferenceCell->m_pXbarRDataset == NULL || isnan(ptTestReferenceCell->m_pXbarRDataset->ICRandR()) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->m_pXbarRDataset->ICRandR());
        if(qslAdvBoxplotFieldOptionsList.contains(QString("#r&r")))
            PrintValueAndPercentage(iGroup, hReportFile, ptTestReferenceCell, szDataColor, fData, lfRangeSpace, false);

        // Check if display AV Worst Case
        fData = (ptTestReferenceCell->m_pXbarRDataset == NULL || isnan(ptTestReferenceCell->m_pXbarRDataset->WCAppraiserVariation()) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->m_pXbarRDataset->WCAppraiserVariation());
        if(qslAdvBoxplotFieldOptionsList.contains(QString("avwc")))
            PrintValueAndPercentage(iGroup, hReportFile, ptTestReferenceCell, szDataColor, fData, lfRangeSpace, false);

        // Check if display R&R Worst Case
        fData = (ptTestReferenceCell->m_pXbarRDataset == NULL || isnan(ptTestReferenceCell->m_pXbarRDataset->WCRAndR()) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->m_pXbarRDataset->WCRAndR());
        if(qslAdvBoxplotFieldOptionsList.contains(QString("r&rwc")))
            PrintValueAndPercentage(iGroup, hReportFile, ptTestReferenceCell, szDataColor, fData, lfRangeSpace, false);

        // Check if display Max XDiff
        if(qslAdvBoxplotFieldOptionsList.contains(QString("max_xdiff")))
        {
            // Keep track of total rows
            iTotalRows ++;

            if (ptTestReferenceCell->m_pXbarRDataset)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%s,", ptTestReferenceCell->m_pXbarRDataset->WCAppraisers().toLatin1().data());
                else
                {
                    QString strAppraiserOne = ptTestReferenceCell->m_pXbarRDataset->WCAppraisers().section("|", 0, 0);
                    QString strAppraiserTwo = ptTestReferenceCell->m_pXbarRDataset->WCAppraisers().section("|", 1, 1);

                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s<br>%s</td>",szDataColor, strAppraiserOne.toLatin1().data(), strAppraiserTwo.toLatin1().data());
                }
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"n/a,");
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >n/a",szDataColor);
            }
        }

        // GB
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfGB;
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("gb"))))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,-1,true);

        // Part Variation (PV)
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfPV;
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("pv"))))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,lfRangeSpace,false);


        // Total Variation (TV)
        fData = (ptTestReferenceCell->pGage == NULL) ? GEX_C_DOUBLE_NAN : ptTestReferenceCell->pGage->lfTV;
        if((m_pReportOptions->iGroups > 1) && qslAdvBoxplotFieldOptionsList.contains(QString("tv")))
            PrintValueAndPercentage(iGroup,hReportFile,ptTestReferenceCell,szDataColor,fData,lfRangeSpace,false);

        // P/T Ratio
        if((m_pReportOptions->iGroups > 1) && (qslAdvBoxplotFieldOptionsList.contains(QString("p_t"))))
        {
            fData = (ptTestReferenceCell->pGage == NULL) ? 99.9: ptTestReferenceCell->pGage->lfP_T;
            // Use scientific notation when value is lower than 100 %
            if (fData >= 100.00f)
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%.2f %%,",fData);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%.2f %%</td>",szDataColor,fData);
            }
            else
            {
                if(strOutputFormat=="CSV")
                    fprintf(hReportFile,"%.2g %%,",fData);
                else
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%.2g %%</td>",szDataColor,fData);
            }

        }

        // Box-Plot Chart
        if (strOutputFormat!="CSV" && qslAdvBoxplotFieldOptionsList.contains(QString("boxplot_chart")))
        {
            // Display boxplot image.
            fprintf(hReportFile,"<td bgcolor=%s> <img border=\"0\" src=\"../images/%s\" >", szDataColor, formatHtmlImageFilename(strImage).toLatin1().constData());

            // If Advanced Examinator, HTML output (not flat HTML), add 'Interactive 2D/3D link' on first group instance
            if(	(strOutputFormat=="HTML")
         && iGroup==1)
            {
                QString strDrillArgument= "drill_chart=adv_boxplot--data=";
                strDrillArgument += ptTestReferenceCell->szTestLabel;

                fprintf(hReportFile," <a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\">",strDrillArgument.toLatin1().constData());
            }

            fprintf(hReportFile,"</td>\n");
        }

        if(strOutputFormat=="CSV")
        {
            fprintf(hReportFile,"\n");
        }
        else
            fprintf(hReportFile,"</tr>\n");

    };	// Build Boxplot for a given test in all groups so to stack all charts.

    int		iNbControlChart = 0;

    if (m_pReportOptions->GetOption("adv_boxplot", "control_charts").toString().split("|").contains("xbar"))
        iNbControlChart++;

    if (m_pReportOptions->GetOption("adv_boxplot", "control_charts").toString().split("|").contains("range"))
        iNbControlChart++;

    if (strOutputFormat != "CSV" && iNbControlChart > 0)
    {
        // Close table
        fprintf(hReportFile,"</table>\n");

        bool bAddBreakPage	= false;

        if (strOutputFormat == "PDF" || strOutputFormat == "DOC" || strOutputFormat=="ODT")
        {
            if (strPageFormat == "landscape")
            {
                if ((iLine >= 5 && iNbControlChart == 2) || iLine >= 8)
                    bAddBreakPage = true;
            }
            else
            {
                if ((iLine >= 8 && iNbControlChart == 2) || (iLine >= 14) )
                    bAddBreakPage = true;
            }
        }
        else if (strOutputFormat == "PPT")
        {
            bAddBreakPage = true;
        }

        // Add separator between Table and Chart (Empty line or break page)
        if (bAddBreakPage)
            // Write this page as a slide (image)
            WritePageBreak();
        else
            // Add an empty row between table and chart
            fprintf(hReportFile, "<br>");

        // Open table

        fprintf(hReportFile,"<table border=\"0\" cellspacing=\"0\" width=\"98%%\">\n");
        // If Advanced Examinator, HTML output (not flat HTML), add 'Interactive 2D/3D link'
        if (strOutputFormat == "HTML" &&
            GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GS::LPPlugin::LicenseProvider::eLtxcOEM)
        {
            QString strDrillArgument= "drill_chart=adv_xbarr--data=";
            strDrillArgument += ptTestReferenceCell->szTestLabel;

            fprintf(hReportFile, "<tr><td bgcolor=%s align=center>\n", szDataColor);
            fprintf(hReportFile, "<a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\"><b>Data Explorer-2D/3D</b></a> (Interactive Zoom, Drill,...)\n",strDrillArgument.toLatin1().constData());
            fprintf(hReportFile, "</tr></td>");
        }

        // Insert the XBar-R Chart here.
        if (m_pReportOptions->GetOption("adv_boxplot", "control_charts").toString().split("|").contains("xbar"))
        {
            strImage =
                BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                     "/images/adv_xbar_", ptTestReferenceCell);
            strImagePath	= m_pReportOptions->strReportDirectory;
            strImagePath	+= "/images/";
            strImagePath	+= strImage;

            GexXBarChart XBarRChart(GEX_CHARTSIZE_AUTO, 0, NULL);
            XBarRChart.setViewportMode(GexXBarChart::viewportAdaptiveControl);
            XBarRChart.computeData(m_pReportOptions, ptTestReferenceCell);
            XBarRChart.buildChart(900, 250);
            XBarRChart.drawChart(strImagePath, GetImageCopyrightString());

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td bgcolor=%s align=center>\n", szDataColor);
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(strImage).toLatin1().constData());
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        if (m_pReportOptions->GetOption("adv_boxplot", "control_charts").toString().split("|").contains("range"))
        {
            strImage =
                BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                                     "/images/adv_r_", ptTestReferenceCell);
            strImagePath	= m_pReportOptions->strReportDirectory;
            strImagePath	+= "/images/";
            strImagePath	+= strImage;

            GexRChart RChart(GEX_CHARTSIZE_AUTO, 0, NULL);
            RChart.setViewportMode(GexRChart::viewportAdaptiveControl);
            RChart.computeData(m_pReportOptions, ptTestReferenceCell);
            RChart.buildChart(900, 200);
            RChart.drawChart(strImagePath, GetImageCopyrightString());

            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td bgcolor=%s align=center>\n", szDataColor);
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n", formatHtmlImageFilename(strImage).toLatin1().constData());
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        // Close table
        fprintf(hReportFile,"</table>\n");

        if (strOutputFormat=="HTML")
            fprintf(hReportFile, "<br>");

        // Write this page as a slide (image)
        WritePageBreak();

        fprintf(hReportFile,"<table border=\"0\" cellspacing=\"0\" width=\"98%%\">\n");

        iLine = 0;
    }
    else if((m_pReportOptions->iGroups > 1)
            && m_pReportOptions->isReportOutputHtmlBased()
            //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            )
        {
            // Insert one white line separator between test blocs (if multiple groups
            int i;
            fprintf(hReportFile,"<tr>\n");
            for(i=0;i<iTotalRows;i++)
                fprintf(hReportFile,"<td bgcolor=\"#FFFFFF\" height=\"2\" style=\"font-size: 2pt\">&nbsp;</td>\n");
            fprintf(hReportFile,"</tr>\n");

        }
}


/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvBoxplotEx(BOOL /*bValidSection*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 ").arg( strOutputFormat).toLatin1().constData());

    // Creates the 'adv_histo' page & header
    long	iTestsBoxPlotExInPage=0;
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased())
        //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        CTest	*ptTestCell=NULL;	// Pointer to test cell to receive STDF info.

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
                      .arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData() );
                N=5;
            }
            else
            {
                GSLOG(SYSLOG_SEV_NOTICE,
                      QString(" will consider only top %1 failed tests ")
                      .arg( N).toLatin1().constData());
            }
        }

        QString htmlpb=m_pReportOptions->GetOption("output", "html_page_breaks").toString();
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","generic_galaxy_tests")).toString();
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while(ptTestCell != NULL)
        {
            // We create one ADV_Histogram HTML page per X tests
            if((iTestsBoxPlotExInPage >= MAX_HISTOGRAM_PERPAGE)
                && (htmlpb=="true"))
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
                GEX_ASSERT( (strOptionStorageDevice == "show") || (strOptionStorageDevice == "hide") );
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
int	CGexReport::CloseSection_AdvBoxplotEx(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
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
    if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
        return GS::StdLib::Stdf::ReportFile;

        // As many functions write to 'hReportFile' as the output.
        hReportFile = hAdvancedReport;

        // Create Test index page
        WriteTestListIndexPage(GEX_INDEXPAGE_ADVBOXPLOT_EX,true);

        // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
        if(strOutputFormat=="HTML")
            CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}

void CGexReport::WriteAdvBoxplotExChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestReferenceCell,int iChartSize)
{
    // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_BOXPLOT_EX,ptTestReferenceCell->iHtmlAdvancedPage);

    // Build Image full path where to save the chart plot.
    // Note: if overlay layers specified, see if a group# can be specified too...
    int	iGroup = -1;
    if (pChartsInfo && pChartsInfo->chartsList().count() == 1)
        iGroup = (pChartsInfo->chartsList().first())->iGroupX;

    QString strImage =
        BuildImageUniqueName(m_pReportOptions->strReportDirectory +
                             "/images/adv_b_", ptTestReferenceCell, iGroup);
    QString strImagePath = m_pReportOptions->strReportDirectory;
    strImagePath += "/images/";
    strImagePath += strImage;

    // Create Chart (paint into Widget)
    CreateAdvBoxPlotChartImageEx(pChartsInfo,ptTestReferenceCell,iChartSize,false,m_pReportOptions->getAdvancedReportSettings(),strImagePath);

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
        WriteHistoHtmlTestInfo(pChartsInfo,"adv_boxplot",pFile,ptTestReferenceCell,NULL,NULL,true, strImage.toLatin1().constData(),iSizeX);
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
CreateAdvBoxPlotChartImageEx(CGexChartOverlays* pChartsInfo,
                             CTest* ptTestReferenceCell,
                             int iChartSize,
                             bool /*bStandardHisto*/,
                             int iChartType,
                             QString strImage)
{
    GexBoxPlotChart boxPlotChart(iChartSize, 0, pChartsInfo);

    boxPlotChart.setViewportModeFromChartMode(iChartType);
    boxPlotChart.computeData(m_pReportOptions, ptTestReferenceCell);
    boxPlotChart.buildChart();
    boxPlotChart.drawChart(strImage, GetImageCopyrightString());
}

QString	CGexReport::WriteAdvBoxPlotEx(CReportOptions* ro)
{
    if (!ro)
        return "error";

    QString of=ro->GetOption("output", "format").toString();
    if(of=="CSV")
        return "ok";

    // If chart size=Auto...compute it's real size
    int iChartSize = GetChartImageSize(	m_pReportOptions->GetOption("adv_boxplot_ex","chart_size").toString(),
                                   m_pReportOptions->lAdvancedHtmlPages*MAX_HISTOGRAM_PERPAGE);


    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
        return "error";

    QString pf=ro->GetOption("output", "paper_format").toString();
    QString ps=ro->GetOption("output", "paper_size").toString();

    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    CTest	*ptTestCell = pGroup->cMergedData.ptMergedTestList;
    int iChartNumber=0;
    int iTestsPerPage=0;

    QList<CTest*>::iterator it=mAdvancedTestsListToReport.begin();
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
               return "GS::StdLib::Stdf::NoError";

            // Draw chart into image file.
            WriteAdvBoxplotExChartPage(NULL,pFile,ptTestCell,iChartSize);

            // Update chart count
            iChartNumber++;

            // When writing flat HTML (for Word or PDF file), insert page break every chart (large images) or every 2 charts (medium images)
            switch(iChartSize)
            {
                case GEX_CHARTSIZE_MEDIUM:
                    // Dynamically build the PowerPoint slide name (as name includes the 2 tests writtent per page). Ignored if not generating a PPT file.
                    RefreshPowerPointSlideName("Box-Plot",iChartNumber,2,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF") || (of=="DOC") || of=="ODT" )
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
                    RefreshPowerPointSlideName("Box-Plot",1,1,ptTestCell);

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
                    RefreshPowerPointSlideName("Box-Plot",iChartNumber,4,ptTestCell);

                    // If PDF or Word, we support Portrait & Landscape formats
                    if(	(of=="PDF") || (of=="DOC") || of=="ODT")
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
        it++; //ptTestCell = ptTestCell->ptNextTest;
    };	// Loop until all test cells read.


    return "ok";
}

