/******************************************************************************!
 * \file report_all_pages.cpp
 * \brief Miscelaneous functions used to create report pages
 ******************************************************************************/
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined unix || __MACH__
#include <unistd.h>
#include <errno.h>
#elif defined(WIN32)
  #include <io.h>
#endif

#include <QMessageBox>
#include <QPrinter>
#include <QWebView>

#include "pat_info.h"
#include "pat_engine.h"
#include "patman_lib.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "drill_chart.h"
#include "interactive_charts.h"  // Layer classes, etc.
#include "cbinning.h"
#include "gex_word_report.h"
#include "gex_pdf_report.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gex_advanced_enterprise_report.h"
#include "gexperformancecounter.h"
#include "db_engine.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "report_template.h"
#include "report_template_io.h"
#include "message.h"
#include "report_classes_sorting.cpp"
#include "report_log_unit.h"
#include "command_line_options.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

// main.cpp
extern GexMainwindow* pGexMainWindow;

// report_build.cpp
extern CReportOptions ReportOptions;  // Holds options (report_build.h)

// In csl/ZcGexLib.cpp
extern CGexTestRange* createTestsRange(QString strParameterList,
                                       bool bAcceptRange,
                                       bool bIsAdvancedReport);

extern int RetrieveTopNWorstTests(int N, CTest* start, QList<CTest*>& output);

typedef bool (* ptrFunction)(CTest*, CTest*);

//bool OneTestisNull(CTest* test1, CTest *test2)
//{
//    if( (!test1) && (!test2) )
//    {
//        GEX_ASSERT(false);
//        return false;
//    }
//    else if (!test1)
//    {
//        GEX_ASSERT(false);
//        return false;
//    }
//    else if (!test2)
//    {
//        GEX_ASSERT(false);
//        return true;
//    }
//}

/******************************************************************************!
 * \fn SortTestNumber
 * \brief Sorting: test number
 ******************************************************************************/
bool SortTestNumber(CTest* test1, CTest* test2)
{
    bool ret;
    if (test1->lTestNumber < test2->lTestNumber)
    {
        ret = true;
    }
    else if (test1->lTestNumber == test2->lTestNumber)
    {
        // Check if test includes pinmap index
        if ((test1->lPinmapIndex == GEX_PTEST) &&
            (test2->lPinmapIndex == GEX_PTEST))
        {
            ret = true;
        }
        else if ((test1->lPinmapIndex == GEX_PTEST) &&
                 (test2->lPinmapIndex != GEX_PTEST))
        {
            ret = true;
        }
        else if ((test1->lPinmapIndex != GEX_PTEST) &&
                 (test2->lPinmapIndex == GEX_PTEST))
        {
            ret = false;
        }
        else if (test1->lPinmapIndex < test2->lPinmapIndex)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}

/******************************************************************************!
 * \fn SortTestName
 * \brief Sorting: test name
 ******************************************************************************/
bool SortTestName(CTest* test1, CTest* test2)
{
    return(qstricmp(test1->strTestName.toLatin1().constData(),
                    test2->strTestName.toLatin1().constData()) < 0);
}

/******************************************************************************!
 * \fn SortTestFlowId
 * \brief Sorting: Test flow ID (execution order)
 ******************************************************************************/
bool SortTestFlowId(CTest* test1, CTest* test2)
{
    bool ret(false);
    // Flow ID is -1 for custom tests (added by GEX)
    // Those tests will appear at th eend of the list
    if (test1->lTestFlowID == -1)
    {
        ret = false;
    }
    else if (test2->lTestFlowID == -1)
    {
        ret = true;
    }
    else if (test1->lTestFlowID < test2->lTestFlowID)
    {
        ret = true;
    }

    return ret;
}

/******************************************************************************!
 * \fn SortTestFailCount
 * \brief Sorting: Fail Count (highest to lowest), Compare fail count
 ******************************************************************************/
bool SortTestFailCount(CTest* test1, CTest* test2)
{
    return(test1->GetCurrentLimitItem()->ldFailCount > test2->GetCurrentLimitItem()->ldFailCount);
}

/******************************************************************************!
 * \fn SortTestMean
 * \brief Sorting: Mean, Compare mean (highest to lowest)
 ******************************************************************************/
bool SortTestMean(CTest* test1, CTest* test2)
{
    return(test1->lfMean > test2->lfMean);
}

/******************************************************************************!
 * \fn SortTestMeanShift
 * \brief Sorting: Mean-Shift, Compare mean_shift (highest to lowest)
 ******************************************************************************/
bool SortTestMeanShift(CTest* test1, CTest* test2)
{
    // GCORE-8337 code review:
    // This sorting is done in static reports on the reference group.
    // The mean shift for the reference group used to be always 0.
    // So this sorting never worked. GCORE-9986 has been created to reflect this bug.
    // To fix this, we would need to position a max mean shift, as we do for the range.
    // We also need to take into account now the selecte limits set in case of multi-limits, as the mean range
    // may depend on the limits if the option to compute the mean shift using the limits space is semected.

    //return(test1->lfMeanShift->Value() > test2->lfMeanShift->Value());
    return true;
}

/******************************************************************************!
 * \fn SortTestSigma
 * \brief Sorting: Sigma (highest to lowest), Compare Sigma
 ******************************************************************************/
bool SortTestSigma(CTest* test1, CTest* test2)
{
    return(test1->lfSigma > test2->lfSigma);
}

/******************************************************************************!
 * \fn SortTestCp
 * \brief Sorting: Cp, Compare Cp (lowest to highest)
 ******************************************************************************/
bool SortTestCp(CTest* test1, CTest* test2)
{
    return(test1->GetCurrentLimitItem()->lfCp < test2->GetCurrentLimitItem()->lfCp);
}

/******************************************************************************!
 * \fn SortTestCpk
 * \brief Sorting: Cpk, Compare Cpk (lowest to highest)
 ******************************************************************************/
bool SortTestCpk(CTest* test1, CTest* test2)
{
    return(test1->GetCurrentLimitItem()->lfCpk < test2->GetCurrentLimitItem()->lfCpk);
}

/******************************************************************************!
 * \fn SortTestRandR
 * \brief R&R (highest % to lowest %), Compare R&R
 ******************************************************************************/
bool SortTestRandR(CTest* test1, CTest* test2)
{
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
        return (test1->pGage->lfRR_percent > test2->pGage->lfRR_percent);
    }
}

/******************************************************************************!
 * \fn SortFieldFuntion
 * \brief Choose the appropriate sorting function occording to SortField
 ******************************************************************************/
ptrFunction SortFieldFuntion(qtTestListStatistics::SortField sortOnField
                             /*, CTest* test1, CTest* test2*/)
{
    ptrFunction returnedFunction;
    //if (! test1 || ! test2)
    //    return OneTestisNull;

    switch (sortOnField)
    {
    case qtTestListStatistics::SortOnTestNumber:
        returnedFunction = &SortTestNumber;
        break;
    case qtTestListStatistics::SortOnTestName:
        returnedFunction = &SortTestName;
        break;
    case qtTestListStatistics::SortOnTestFlowID:
        returnedFunction = &SortTestFlowId;
        break;
    case qtTestListStatistics::SortOnFailCount:
        returnedFunction = &SortTestFailCount;
        break;
    case qtTestListStatistics::SortOnMean:
        returnedFunction = &SortTestMean;
        break;
    case qtTestListStatistics::SortOnMeanShift:
        returnedFunction = &SortTestMeanShift;
        break;
    case qtTestListStatistics::SortOnSigma:
        returnedFunction = &SortTestSigma;
        break;
    case qtTestListStatistics::SortOnCP:
        returnedFunction = &SortTestCp;
        break;
    case qtTestListStatistics::SortOnCPK:
        returnedFunction = &SortTestCpk;
        break;
    case qtTestListStatistics::SortOnRandR:
        returnedFunction = &SortTestRandR;
        break;
    default:
        GEX_ASSERT(false);
        return 0;
        break;
    }
    return returnedFunction;
}

/******************************************************************************!
 * \fn formatHtmlImageFilename
 ******************************************************************************/
QString formatHtmlImageFilename(const QString& strImageFileName)
{
    QString of = ReportOptions.GetOption("output", "format").toString();
    // For htmpl report only, add "?" + timestamp
    // to force the web engine to reload the image
    if (of == "HTML")  //(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_HTML)
    {
        return QString("%1?%2").arg(strImageFileName).arg(
            QDateTime::currentDateTime().toTime_t());
    }

    return strImageFileName;
}

/******************************************************************************!
 * \fn DeleteTestEntry
 * \brief Delete given (or all) tests into one (or all) groups
 ******************************************************************************/
/*bool CGexReport::DeleteTestEntry(CGexGroupOfFiles* pGroup  ,
                                 CTest* ptTestCell  )
{
    // Check if delete ALL groups & test entries
    bool bDeleteAllGroups = false;
    if (pGroup == NULL)
    {
        bDeleteAllGroups = true;
        pGroup = getGroupsList().isEmpty() ? NULL : getGroupsList().first();
    }

    // Scan test list for the given group and delete each entry
    CTest* ptTest, * ptPrevTest, * ptNextTest;
    ptTest = ptPrevTest = pGroup->cMergedData.ptMergedTestList;

    // If only one file in group, the test list is at the test file level
    if (ptTest == NULL)
    {
        ptTest = ptPrevTest =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first()->ptTestList);
    }

    // Scan all groups (unless specified otherwise)
    for (int nIndex = getGroupsList().indexOf(pGroup);
         nIndex < getGroupsList().count(); ++nIndex)
    {
        pGroup = getGroupsList().at(nIndex);

        // Scan test list and delete the specified test entries
        while (ptTest != NULL)
        {
            // save pointer to next cell before we delete this one
            ptNextTest = ptTest->GetNextTest();

            // We have to delete one specific test entry
            if ((ptTestCell != NULL) && (ptTest == ptTestCell))
            {
                // Delete test entry
                delete ptTest; ptTest = 0;

                // Connect previous test entry to this new one
                if (ptPrevTest == pGroup->cMergedData.ptMergedTestList)
                {
                    // We have deleted the first entry in list (head)
                    // So next entry in list now becomes list's head
                    pGroup->cMergedData.ptMergedTestList = ptNextTest;
                }
                else
                {
                    // Have previous entry in list now point to the
                    // test entry following the one just deleted
                    ptPrevTest->ptNextTest = ptNextTest;
                }

                // Job done
                return true;
            }

            // We have to delete ALL entries in this group
            if (ptTestCell == NULL)
            {
                // We have to delete ALL entries ...
                // so do it one entry at a time
                delete ptTest;
            }

            // Move to next cell
            ptTest = ptNextTest;
        }
        ;

        // reset pointer (as we've an empty list)
        pGroup->cMergedData.ptMergedTestList = NULL;
        if (! pGroup->pFilesList.isEmpty())
        {
            pGroup->pFilesList.first()->ptTestList = NULL;
        }

        // If we had to only work on this group, then exit now
        if (bDeleteAllGroups == false)
        {
            return true;
        }
    }

    // Job done
    return true;
}
*/
/******************************************************************************!
 * \fn CreateResultStringCpCpk
 * \brief Format Cp/Cpk string
 ******************************************************************************/
const char* CGexReport::CreateResultStringCpCrCpk(double lfValue)
{
    GEX_BENCHMARK_METHOD(QString(""));

    // WT: why only 20 chars ? dangerous.
    static char szBuffer[20]="";  // used to build a result string

    if (lfValue == C_NO_CP_CPK)
    {
        strcpy(szBuffer, GEX_NA);
    }
    else if (fabs(lfValue) <= 1)
    {
        sprintf(szBuffer, "%.4f", lfValue);
        QString strFormatDouble =
            getNumberFormat()->formatNumericValue(lfValue, false,
                                                  QString(szBuffer));
        sprintf(szBuffer, "%s", strFormatDouble.toLatin1().constData());
    }
    else if (fabs(lfValue) <= 100)
    {
        sprintf(szBuffer, "%.2f", lfValue);
        QString strFormatDouble =
            getNumberFormat()->formatNumericValue(lfValue, false,
                                                  QString(szBuffer));
        sprintf(szBuffer, "%s", strFormatDouble.toLatin1().constData());
    }
    else if (fabs(lfValue) <= 10000)
    {
        sprintf(szBuffer, "%.1f", lfValue);
        QString strFormatDouble =
            getNumberFormat()->formatNumericValue(lfValue, false,
                                                  QString(szBuffer));
        sprintf(szBuffer, "%s", strFormatDouble.toLatin1().constData());
    }
    else
    {
        sprintf(szBuffer, "%.0g", lfValue);
        QString strFormatDouble =
            getNumberFormat()->formatNumericValue(lfValue, false,
                                                  QString(szBuffer));
        sprintf(szBuffer, "%s", strFormatDouble.toLatin1().constData());
    }
    return szBuffer;
}

const char* CGexReport::CreateTestDrilTL(CTest *lTestCell)
{
    static char lTempString[GEX_LIMIT_LABEL]="";
    double	lfLimitSpace(0), lData(0);
    QString strFormatDouble;

    strcpy(lTempString, GEX_NA);
    if(((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
        ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
        ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0) &&
        ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
    {
        lfLimitSpace = (lTestCell->GetCurrentLimitItem()->lfHighLimit - lTestCell->GetCurrentLimitItem()->lfLowLimit);
        lData = lTestCell->lfHighSpecLimit-lTestCell->lfLowSpecLimit;
        if(lData)
            lData = (lfLimitSpace / lData)*100.0;
        if(lData >1000)
            lData = 999.99;	// Clamp % value in case very high!
        sprintf(lTempString,"%.2lf %%",lData);
        strFormatDouble = getNumberFormat()->formatNumericValue(lData, true,QString(lTempString));
        sprintf(lTempString,"%s",strFormatDouble.toLatin1().constData());
    }
    return lTempString;
}

// LOW Test/Spec limits drift %
const char* CGexReport::CreateTestDrilFlow(CTest *lTestCell)
{
    static char lTempString[GEX_LIMIT_LABEL]="";
    double	lData(0);
    QString strFormatDouble;

    strcpy(lTempString, GEX_NA);
    if(((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
        ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0))
    {
        lData = 999.99;
        if(lTestCell->lfLowSpecLimit)
            lData = (lTestCell->GetCurrentLimitItem()->lfLowLimit / lTestCell->lfLowSpecLimit)*100.0;
        if(lData >1000)
            lData = 999.99;	// Clamp % value in case very high!
        sprintf(lTempString,"%.2lf %%",lData);
        strFormatDouble = getNumberFormat()->formatNumericValue(lData, true,QString(lTempString));
        sprintf(lTempString,"%s",strFormatDouble.toLatin1().constData());
    }
    return lTempString;
}

// HIGH Test/Spec limits drift %
const char* CGexReport::CreateTestDriftHigh(CTest *lTestCell)
{
    static char lTempString[GEX_LIMIT_LABEL]="";
    double	lData(0);
    QString strFormatDouble;

    strcpy(lTempString, GEX_NA);
    if(((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
        ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
    {
        lData = 999.99;
        if(lTestCell->lfHighSpecLimit)
            lData = (lTestCell->GetCurrentLimitItem()->lfHighLimit / lTestCell->lfHighSpecLimit)*100.0;
        if(lData >1000)
            lData = 999.99;	// Clamp % value in case very high!
        sprintf(lTempString,"%.2lf %%",lData);
        strFormatDouble = getNumberFormat()->formatNumericValue(lData, true,QString(lTempString));
        sprintf(lTempString,"%s",strFormatDouble.toLatin1().constData());
    }
    return lTempString;
}

// Fail %
void CGexReport::CreateTestFailPercentage(CTest *lTestCell, QString& outputString)
{
    float	lData(0);
    if(lTestCell->ldSamplesValidExecs > 0)
    {
        lData = (100.0*lTestCell->GetCurrentLimitItem()->ldFailCount)/lTestCell->ldSamplesValidExecs;
        outputString = QString::number(lData, 'g', 2);  // sprintf(outputString,"%.2f",lData);
        outputString = gexReport->getNumberFormat()->formatNumericValue(lData, false,QString(outputString));
    }
    else
    {
        outputString = "0";
    }
}

/******************************************************************************!
 * \fn CreateResultStringPercent
 * \brief Format string
 ******************************************************************************/
const char* CGexReport::CreateResultStringPercent(double lfValue)
{
    static char szBuffer[20];  // used to build a result string

    if (lfValue >= 10000.0)
    {
        sprintf(szBuffer, "%.0g %%", lfValue);
    }
    else if (lfValue >= 1000.0)
    {
        sprintf(szBuffer, "%.1f %%", lfValue);
    }
    else if (lfValue >= 0)
    {
        sprintf(szBuffer, "%.2f %%", lfValue);
    }
    else
    {
        strcpy(szBuffer, GEX_NA);
    }
    return szBuffer;
}

/******************************************************************************!
 * \fn CreateResultStringCpCpkShift
 * \brief Format Cp/Cpk string
 ******************************************************************************/
const char* CGexReport::CreateResultStringCpCrCpkShift(double lfValue)
{
    static char szBuffer[20];  // used to build a result string

    if (fabs(lfValue) >= 10000.0)
    {
        sprintf(szBuffer, "%.0g", lfValue);
    }
    else if (fabs(lfValue) >= 1000.0)
    {
        sprintf(szBuffer, "%.1f", lfValue);
    }
    else if (fabs(lfValue) >= 0)
    {
        sprintf(szBuffer, "%.2f", lfValue);
    }
    else
    {
        sprintf(szBuffer, "%.2f", lfValue);
    }

    return szBuffer;
}

/******************************************************************************!
 * \fn CreateResultString
 * \brief Converts value to string
 ******************************************************************************/
const char* CGexReport::CreateResultString(long lValue)
{
    static char szString[20];  // used to build a result string

    if (lValue < 0)
    {
        return GEX_NA;
    }
    sprintf(szString, "%ld", lValue);

    QString strFormatDouble =
        getNumberFormat()->formatNumericValue(lValue, false,
                                              QString(szString));
    sprintf(szString, "%s", strFormatDouble.toLatin1().constData());

    return szString;
}

/******************************************************************************!
 * \fn reportFormat
 ******************************************************************************/
QString CGexReport::reportFormat(const QString& strReportFile)
{
    QString strReportFormat = "";

    QFileInfo reportFileInfo(strReportFile);

    QString strFileExtension = reportFileInfo.suffix();

    strFileExtension = strFileExtension.toLower();

    if (strFileExtension == "htm")
    {
        strReportFormat = "HTML";
    }
    else if (strFileExtension == "csv")
    {
        strReportFormat = "CSV";
    }
    else if (strFileExtension == "doc")
    {
        strReportFormat = "DOC";
    }
    else if (strFileExtension == "ppt")
    {
        strReportFormat = "PPT";
    }
    else if (strFileExtension == "pdf")
    {
        strReportFormat = "PDF";
    }
    else if (strFileExtension == "odt")
    {
        strReportFormat = "ODT";
    }
    else  //if (strFileExtension == "interactive")
    {
        strReportFormat = "INTERACTIVE";
    }

    return strReportFormat;
}

/******************************************************************************!
 * \fn GetChartingHtmlColor
 * \brief Returns the HTML string of the RGB color for a specific groupID
 ******************************************************************************/
const char* CGexReport::GetChartingHtmlColor(int iGroupIndex)
{
    int nIndex = iGroupIndex - 1;

    // Check if index is valid
    if (nIndex >= 0 && nIndex < m_pReportOptions->pLayersStyleList.count())
    {
        // If color customized under Interactive chart, use it
        CGexSingleChart* pLayerStyle =
            m_pReportOptions->pLayersStyleList.at(nIndex);
        if (pLayerStyle != NULL)
        {
            static char szColor[8];
            sprintf(szColor, "#%02x%02x%02x",
                    pLayerStyle->cColor.red(),
                    pLayerStyle->cColor.green(), pLayerStyle->cColor.blue());
            return szColor;
        }
    }

    // No custom color defined for this gorup, then use default colors
    switch (nIndex % 12)
    {
    case 0:
        return "#AAFF7F";  // light green;
    case 1:
        return "#FF00FF";  // Magenta;
    case 2:
        return "#000080";  // Blue-grey
    case 3:
        return "#FFFF00";  // Yellow;
    case 4:
        return "#00FFFF";  // Cyan;
    case 5:
        return "#800080";  // Violet;
    case 6:
        return "#800000";  // Brown
    case 7:
        return "#FF9900";  // brighter Brown-orange
    case 8:
        return "#0000FF";  // Full blue
    case 9:
        return "#00CCFF";  // Light blue lagoon
    case 10:
        return "#000000";  // Black
    case 11:
        return "#008000";  // Green;
    }
    // Should never happen
    return "#00FF00";  //Qt::blue;
}

/******************************************************************************!
 * \fn GetReportSortingMode
 * \brief Tells what sorting mode was selected
 ******************************************************************************/
const char* CGexReport::GetReportSortingMode(QString strSortingMode)
{
    const char* szSorting = NULL;

    if (strSortingMode == "test_number")
    {
        szSorting = "Test number (from lowest number to highest)";
    }
    else if (strSortingMode == "test_name")
    {
        szSorting = "Test name (ascending order)";
    }
    else if (strSortingMode == "test_flow_id")
    {
        szSorting = "Test flow ID (test execution order)";
    }
    else if (strSortingMode == "fail_count")
    {
        szSorting = "Fail count (highest to lowest)";
    }
    else if (strSortingMode == "mean")
    {
        if (m_pReportOptions->iGroups == 1)
        {
            szSorting = "Mean (highest to lowest)";
        }
        else
        {
            szSorting = "Mean shift (highest shift to lowest)";
        }
    }
    else if (strSortingMode == "mean_shift")
    {
        szSorting = "Mean shift (highest shift to lowest)";
    }
    else if (strSortingMode == "sigma")
    {
        if (m_pReportOptions->iGroups == 1)
        {
            szSorting = "Sigma (highest to lowest)";
        }
        else
        {
            szSorting = "Sigma shift (highest shift to lowest)";
        }
    }
    else if (strSortingMode == "cp")
    {
        if (m_pReportOptions->iGroups == 1)
        {
            szSorting = "Cp (lowest to highest)";
        }
        else
        {
            szSorting = "Cp shift (highest shift to lowest)";
        }
    }
    else if (strSortingMode == "cpk")
    {
        if (m_pReportOptions->iGroups == 1)
        {
            szSorting = "Cpk (lowest to highest)";
        }
        else
        {
            szSorting = "Cpk shift (highest shift to lowest)";
        }
    }
    else if (strSortingMode == "r&r")
    {
        szSorting = "Gage R&R value (from highest to lowest)";
    }
    else
    {
        GEX_ASSERT(false);
    }
    return szSorting;

}

/******************************************************************************!
 * \fn setReportFile
 * \brief Force handle used when creating reports
 ******************************************************************************/
void CGexReport::setReportFile(FILE* hHtmlReportFile)
{
    hReportFile = hHtmlReportFile;
}

/******************************************************************************!
 * \fn getReportFile
 ******************************************************************************/
FILE* CGexReport::getReportFile()
{
    return hReportFile;
}

/******************************************************************************!
 * \fn setAdvancedReportFile
 * \brief Force handle used when creating reports
 ******************************************************************************/
void CGexReport::setAdvancedReportFile(FILE* hHtmlAdvancedReportFile)
{
    hAdvancedReport = hHtmlAdvancedReportFile;
}

/******************************************************************************!
 * \fn getAdvancedReportFile
 ******************************************************************************/
FILE* CGexReport::getAdvancedReportFile()
{
    return hAdvancedReport;
}

/******************************************************************************!
 * \fn GetImageCopyrightString
 ******************************************************************************/
QString CGexReport::GetImageCopyrightString()
{
    switch (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode())
    {
    case GEX_RUNNINGMODE_EVALUATION:
        // In Evaluation mode, write on image
        // "Free copy. Not for commercial use"
        return GEX_EVALCOPY_SHORT_NOTICE;
    default:
        // In registered copy, write on image "www.mentor.com"
        return "www.mentor.com";
    }
}

/******************************************************************************!
 * \fn GetPartFilterReportType
 * \brief Prepares the report section to be written (.CSV & .HTML)
 ******************************************************************************/
QString CGexReport::GetPartFilterReportType(int processBins)
{
    QString strMessage;

    switch (processBins)
    {
    case GEX_PROCESSPART_ALL:
        strMessage = "All parts";
        break;
    case GEX_PROCESSPART_EXPARTLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("All parts except parts: ");
        break;
    case GEX_PROCESSPART_GOOD:
        strMessage = "Good parts only";
        break;
    case GEX_PROCESSPART_FAIL:
        strMessage = "Failing parts only (all but Bin1)";
        break;
    case GEX_PROCESSPART_PARTLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("Specific part, or parts range: ");
        break;
    case GEX_PROCESSPART_SBINLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("Specific Soft bin, or bin range: ");
        break;
    case GEX_PROCESSPART_EXSBINLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("All Soft bins, except bins: ");
        break;
    case GEX_PROCESSPART_HBINLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("Specific Hard bin, or bin range: ");
        break;
    case GEX_PROCESSPART_EXHBINLIST:
        // List of Ranges to process (parts/bins,...)
        strMessage = m_pReportOptions->
            pGexRangeList->BuildListString("All Hard bins, except bins: ");
        break;
    case GEX_PROCESSPART_ODD:
        strMessage = "Odd parts (1,3,5,...)";
        break;
    case GEX_PROCESSPART_EVEN:
        strMessage = "Even parts (2,4,6,...)";
        break;
    case GEX_PROCESSPART_FIRSTINSTANCE:
        strMessage = "Wafersort: First Test instances (ignore retests)";
        break;
    case GEX_PROCESSPART_LASTINSTANCE:
        strMessage = "Wafersort: Last Test instances (only last in retests)";
        break;
    case GEX_PROCESSPART_NO_SAMPLES:
        strMessage = "Bin data only (ignore samples)";
        break;
    default:
        strMessage = GEX_NA;
    }
    return strMessage;
}

/******************************************************************************!
 * \fn UpdateUserTestNameAndTestNumber
 ******************************************************************************/
void CGexReport::UpdateUserTestNameAndTestNumber()
{
    // GSCurrentTest
    if (! m_pScriptEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("No script engine").toLatin1().constData());
        return;
    }
    CTest lCurrentTest;
    QScriptValue lObj = m_pScriptEngine->newQObject(&lCurrentTest);
    if (lObj.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Cannot add new object to script engine").
              toLatin1().constData());
        return;
    }
    m_pScriptEngine->globalObject().setProperty("GSCurrentTest", lObj);

    // Options
    QString lTestNameExp(ReportOptions.
                         GetOption("dataprocessing",
                                   "test_name").toString());
    if (lTestNameExp.isEmpty())
    {
        lTestNameExp =
            ReportOptions.GetOptionsHandler().GetOptionsTypeDefinition().
            getDefaultOptionValue("dataprocessing", "test_name");
    }
    QString lTestNumberExp(ReportOptions.
                           GetOption("dataprocessing",
                                     "test_number").toString());
    if (lTestNumberExp.isEmpty())
    {
        lTestNumberExp =
            ReportOptions.GetOptionsHandler().GetOptionsTypeDefinition().
            getDefaultOptionValue("dataprocessing", "test_number");
    }
    QString lPinNameExp(ReportOptions.
                        GetOption("dataprocessing",
                                  "pin_name").toString());
    if (lPinNameExp.isEmpty())
    {
        lPinNameExp =
            ReportOptions.GetOptionsHandler().GetOptionsTypeDefinition().
            getDefaultOptionValue("dataprocessing", "pin_name");
    }

    CGexFileInGroup* lFile;
    CTest* lTest;
    CTest* lMasterTest;
    CPinmap* lPinmap;
    QString lResult;

    QList<CGexGroupOfFiles*>::iterator lIter;
    for (lIter = pGroupsList.begin();
         lIter != pGroupsList.end(); ++lIter)
    {
        lFile = (*lIter)->pFilesList.first();
        lTest = lFile->ptTestList;
        while (lTest)
        {
            lMasterTest = NULL;
            if (lTest->lPinmapIndex >= 0)
            {
                // GCORE-4758 HTH
                // When looking for a test, the test name must be always provided. It is up to the method
                // to decide whether it should be taken into account to match the test base on the options
                int lR=lFile->FindTestCell(lTest->lTestNumber, GEX_MPTEST, &lMasterTest, true,
                                           false, lTest->strTestName);
                if (lR==-1)
                {
                    GSLOG(3, "Failed to find test cell");
                    //return; // ?
                }
            }
            lCurrentTest.lTestNumber = lTest->lTestNumber;
            lCurrentTest.strTestName = lTest->strTestName;
            lCurrentTest.lPinmapIndex = lTest->lPinmapIndex;
            if (lMasterTest != NULL &&
                lMasterTest->ptResultArrayIndexes != NULL &&
                lFile->FindPinmapCell(&lPinmap, lMasterTest->
                               ptResultArrayIndexes[lTest->lPinmapIndex]) == 1)
            {
                lCurrentTest.SetPinPhysicName(lPinmap->strPhysicName);
                lCurrentTest.SetPinLogicName(lPinmap->strLogicName);
                lCurrentTest.SetPinChannelName(lPinmap->strChannelName);
            }
            else
            {
                lResult = "";
                lCurrentTest.SetPinPhysicName(lResult);
                lCurrentTest.SetPinLogicName(lResult);
            }
            // User test name
            if (m_pScriptEngine->canEvaluate(lTestNameExp))
            {
                lResult = m_pScriptEngine->evaluate(lTestNameExp).toString();
            }
            if (m_pScriptEngine->hasUncaughtException())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Error evaluating: %1 : %2").
                      arg(lTestNameExp).
                      arg(m_pScriptEngine->uncaughtException().toString()).
                      toLatin1().constData());
                m_pScriptEngine->globalObject().
                    setProperty("GSCurrentTest", QScriptValue());
                return;
            }
            lTest->SetUserTestName(lResult);
            // User test number
            if (m_pScriptEngine->canEvaluate(lTestNumberExp))
            {
                lResult = m_pScriptEngine->evaluate(lTestNumberExp).toString();
            }
            if (m_pScriptEngine->hasUncaughtException())
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error evaluating: %1 : %2").arg(lTestNumberExp).
                      arg(m_pScriptEngine->uncaughtException().toString()).
                      toLatin1().constData());
                m_pScriptEngine->globalObject().setProperty("GSCurrentTest", QScriptValue());
                // GCORE-499
                m_pScriptEngine->collectGarbage();
                return;
            }
            lTest->SetUserTestNumber(lResult);
            // User pin name
            if (m_pScriptEngine->canEvaluate(lPinNameExp))
            {
                lResult =
                    m_pScriptEngine->evaluate(lPinNameExp).toString();
            }
            if (m_pScriptEngine->hasUncaughtException())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Error evaluating: %1 : %2").
                      arg(lPinNameExp).
                      arg(m_pScriptEngine->uncaughtException().toString()).
                      toLatin1().constData());
                m_pScriptEngine->globalObject().setProperty("GSCurrentTest", QScriptValue());
                m_pScriptEngine->collectGarbage();
                return;
            }
            lTest->SetUserPinName(lResult);
            lTest = lTest->GetNextTest();
        }
    }
    m_pScriptEngine->
        globalObject().setProperty("GSCurrentTest", QScriptValue());
}

/******************************************************************************!
 * \fn BuildTestNumberString
 * \brief Builds a string that include the Test# and PinmapIndex# if any
 *        (Multi-result param)
 ******************************************************************************/
void CGexReport::BuildTestNumberString(CTest* ptTestCell)
{
    switch (ptTestCell->lPinmapIndex)
    {
    case GEX_FTEST:  // Functional test
    case GEX_PTEST:  // Standard Parametric test: no PinmapIndex#
    case GEX_INVALIDTEST:
    case GEX_UNKNOWNTEST:
    case GEX_MPTEST:
        // Multi-result parametric test, but no PinmapIndex defined
        sprintf(ptTestCell->szTestLabel, "%u", ptTestCell->lTestNumber);
        break;
    default:  // Multi-result parametric test, PinmapIndex# defined
        QString lString = ptTestCell->GetUserTestNumber();
        if (lString.isEmpty())
        {
            this->UpdateUserTestNameAndTestNumber();
            lString = ptTestCell->GetUserTestName();
        }
        sprintf(ptTestCell->szTestLabel, "%s", lString.toLatin1().constData());
    }
}

/******************************************************************************!
 * \fn BuildTestNameString
 * \brief Builds a string that include the Test name and PinmapIndex#:name
 *        if any (Multi-result param)
 ******************************************************************************/
void CGexReport::BuildTestNameString(CGexFileInGroup* pFile,
                                     CTest* ptTestCell,
                                     char* szString)
{
    QString strTestName;

    BuildTestNameString(pFile, ptTestCell, strTestName);

    strcpy(szString, strTestName.toLatin1().constData());
}

/******************************************************************************!
 * \fn BuildTestNameString
 * \brief Builds a string that include the Test name and PinmapIndex#:name
 *        if any (Multi-result param)
 ******************************************************************************/
void CGexReport::BuildTestNameString(CGexFileInGroup* /*pFile*/,
                                     CTest* ptTestCell,
                                     QString& strString)
{
    switch (ptTestCell->lPinmapIndex)
    {
    case GEX_FTEST:  // Functional test
    case GEX_PTEST:  // Standard Parametric test: no PinmapIndex#
    case GEX_INVALIDTEST:
    case GEX_UNKNOWNTEST:
    case GEX_MPTEST:
        // Multi-result parametric test, but no PinmapIndex defined
        strString = ptTestCell->strTestName;
        break;
    default:  // Multi-result parametric test, PinmapIndex# defined
        strString = ptTestCell->GetUserTestName();
        if (strString.isEmpty())
        {
            this->UpdateUserTestNameAndTestNumber();
            strString = ptTestCell->GetUserTestName();
        }
    }
}

/******************************************************************************!
 * \fn BuildPinNameString
 * \brief Gets the Pin name if any (Multi-result param)
 ******************************************************************************/
void CGexReport::BuildPinNameString(CGexFileInGroup* /*pFile*/,
                                    CTest* ptTestCell,
                                    QString& strPinName)
{
    if (ptTestCell->lPinmapIndex >= 0)
    {
        strPinName = ptTestCell->GetUserPinName();
    }
    else
    {
        strPinName = "";
    }
}

/******************************************************************************!
 * \fn BuildTestTypeString
 * \brief Builds a string that include the Test type, and info if any
 *        (Multi-result param)
 ******************************************************************************/
void CGexReport::BuildTestTypeString(CGexFileInGroup* pFile,
                                     CTest* ptTestCell,
                                     char* szString,
                                     BOOL bShortType)
{
    switch (ptTestCell->bTestType)
    {
    case '-':  // Parameter created by Examinator (eg: Die location, etc.)
        strcpy(szString, "-");
        break;

    case 'F':  // Functional test
        if (bShortType)
        {
            strcpy(szString, "F");
        }
        else
        {
            strcpy(szString, "Functional");
        }
        break;

    case 'P':  // Standard Parametric test
        if (bShortType)
        {
            strcpy(szString, "P");
        }
        else
        {
            strcpy(szString, "Parametric");
        }
        break;

    case ' ':  // unknown test type
        if (bShortType)
        {
            strcpy(szString, "Unk");
        }
        else
        {
            strcpy(szString, "Unknown");
        }
        break;

    case 'I':  // Invalid test type
        if (bShortType)
        {
            strcpy(szString, "Inv");
        }
        else
        {
            strcpy(szString, "Invalid");
        }
        break;

    case 'M':
    default:  // Multi-result parametric test, PinmapIndex# defined
        CPinmap* ptPinmapCell = 0;
        char szTemp[50];
        if (bShortType)
        {
            strcpy(szString, "MP");
            break;
        }

        // Multi-parameteric, but all pinmaps merged,
        // therefore can't say that much
        if (ptTestCell->lPinmapIndex < 0)
        {
            strcpy(szString, "(Parametric, Multiple results");  //FIXME: no ")"?
            break;
        }

        if (pFile->FindPinmapCell(&ptPinmapCell, ptTestCell->lPinmapIndex) == 1)
        {
            // Found pinmap details, show them
            if (ptTestCell->lPinmapIndex >= 0)
            {
                sprintf(szString,
                        "Parametric, Multiple results "
                        "(Test %d, Pinmap index %d",
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex);
            }
            else
            {
                strcpy(szString, "(Parametric, Multiple results");
            }

            if (ptPinmapCell->strChannelName.isEmpty() == false)
            {
                // Channel name exists, add it to string
                sprintf(szTemp,
                        ", channel:%s",
                        ptPinmapCell->strChannelName.toLatin1().constData());
                strcat(szString, szTemp);
            }

            if (ptPinmapCell->strPhysicName.isEmpty() == false)
            {
                // Physical pin name exists, add it to string
                sprintf(szTemp,
                        ", Physical pin:%s",
                        ptPinmapCell->strPhysicName.toLatin1().constData());
                strcat(szString, szTemp);
            }

            if (ptPinmapCell->strLogicName.isEmpty() == false)
            {
                // Logical pin name exists, add it to string
                sprintf(szTemp,
                        ", Logical pin:%s",
                        ptPinmapCell->strLogicName.toLatin1().constData());
                strcat(szString, szTemp);
            }

            strcat(szString, ")");  // end of test/pin description
        }
        else
        {
            // Failed finding information about the pinmap ...
            // so only list pin index
            sprintf(szString,
                    "Parametric, Multiple results (Test %d, Pinmap index %d)",
                    ptTestCell->lTestNumber,
                    ptTestCell->lPinmapIndex);
        }
    }
}

/******************************************************************************!
 * \fn BuildParameterNameFromURL
 * \brief Extract Parameter name from the URL link field
 ******************************************************************************/
QString CGexReport::BuildParameterNameFromURL(QString strName)
{
    QString strParameterName = strName;
    strParameterName.replace("&32", " ");  // Spaces
    strParameterName.replace("&33", "!");  // !
    strParameterName.replace("&34", "\"");  // "
    strParameterName.replace("&35", "#");  // #
    strParameterName.replace("&36", "$");  // $
    strParameterName.replace("&37", "%");  // %
    strParameterName.replace("&38", "&");  // &
    strParameterName.replace("&39", "'");  // '
    strParameterName.replace("&40", "(");  // (
    strParameterName.replace("&41", ")");  // )
    strParameterName.replace("&42", "*");  // *
    strParameterName.replace("&43", "+");  // +
    strParameterName.replace("&44", ",");  // ,
    strParameterName.replace("&45", "-");  // -
    strParameterName.replace("&46", ".");  // .
    strParameterName.replace("&58", ":");  // :
    strParameterName.replace("&59", ";");  // ;
    strParameterName.replace("&60", "<");  // <
    strParameterName.replace("&61", "=");  // =
    strParameterName.replace("&62", ">");  // >
    strParameterName.replace("&63", "?");  // ?
    strParameterName.replace("&64", "@");  // @
    strParameterName.replace("&91", "[");  // [
    strParameterName.replace("&92", "\\");  // "\"
    strParameterName.replace("&93", "]");  // ]
    strParameterName.replace("&94", "^");  // ^
    strParameterName.replace("&96", "`");  // `
    strParameterName.replace("&123", "{");  // {
    strParameterName.replace("&124", "|");  // |
    strParameterName.replace("&125", "}");  // }
    strParameterName.replace("&126", "~");  // ~

    return strParameterName;
}

/******************************************************************************!
 * \fn BuildParameterNameToURL
 * \brief Convert Parameter name to be URL complient (space = &20, etc.
 ******************************************************************************/
QString CGexReport::BuildParameterNameToURL(QString strName)
{
    QString strUrlField = strName;

    strUrlField.replace(" ", "&32");  // Spaces
    strUrlField.replace("!", "&33");  // !
    strUrlField.replace("\"", "&34");  // "
    strUrlField.replace("#", "&35");  // #
    strUrlField.replace("$", "&36");  // $
    strUrlField.replace("%", "&37");  // %
    strUrlField.replace("&", "&38");  // &
    strUrlField.replace("'", "&39");  // '
    strUrlField.replace("(", "&40");  // (
    strUrlField.replace(")", "&41");  // )
    strUrlField.replace("*", "&42");  // *
    strUrlField.replace("+", "&43");  // +
    strUrlField.replace(",", "&44");  // ,
    strUrlField.replace("-", "&45");  // -
    strUrlField.replace(".", "&46");  // .
    strUrlField.replace(":", "&58");  // :
    strUrlField.replace(";", "&59");  // ;
    strUrlField.replace("<", "&60");  // <
    strUrlField.replace("=", "&61");  // =
    strUrlField.replace(">", "&62");  // >
    strUrlField.replace("?", "&63");  // ?
    strUrlField.replace("@", "&64");  // @
    strUrlField.replace("[", "&91");  // [
    strUrlField.replace("\\", "&92");  // "\"
    strUrlField.replace("]", "&93");  // ]
    strUrlField.replace("^", "&94");  // ^
    strUrlField.replace("`", "&96");  // `
    strUrlField.replace("{", "&123");  // {
    strUrlField.replace("|", "&124");  // |
    strUrlField.replace("}", "&125");  // }
    strUrlField.replace("~", "&126");  // ~

    return strUrlField;
}

/******************************************************************************!
 * \fn CheckForNewHtmlPage
 * \brief If current HTML page includes enough data,
 *        close page and create new one
 ******************************************************************************/
void CGexReport::CheckForNewHtmlPage(CGexChartOverlays* pChartsInfo,
                                     int iSectionType,
                                     int iTestPage,
                                     const char* szTitle  /* = NULL*/)
{
    char szString[2048];
    // static QString strPropertiesPath;
    // Root name of the HTML loage name to create
    const char* szPageName = NULL;
    QString strPageTitle = "";  // Holds page title
    QString strBookmark = "all_";  // Bookmark, used in HTML flat page only

    // If script running creating a 'My Report' or 'Report Center',
    // all pages are in flat mode, so simply ignore this call
    if ((ReportOptions.strTemplateFile.isEmpty() == false ||
         ReportOptions.strReportCenterTemplateFile.isEmpty() == false) &&
        (GS::Gex::CSLEngine::GetInstance().IsRunning()))
    {
        return;
    }

    QString strOutputFormat =
        ReportOptions.GetOption("output", "format").toString();

    if ((strOutputFormat == "CSV") && (pChartsInfo == NULL))
    {
        return;
    }

    if (pChartsInfo != NULL)
    {
        // Creating HTML page when in Interactive Charting
        CloseReportFile();  // Close report file
    }
    else
    {
        // Creating HTML report pages
        if (iCurrentHtmlPage == iTestPage)
        {
            return;  // Still okay
        }
    }

    // Starting a new HTML page
    switch (iSectionType)
    {
    case SECTION_STATS:
        szPageName = "stats";
        // "Tests Statistics"
        strPageTitle =
            ReportOptions.GetOption("statistics", "section_name").toString();
        break;
    case SECTION_HISTO:
        szPageName = "histogram";
        // "Histogram of Tests"
        strPageTitle =
            ReportOptions.GetOption("histogram", "section_name").toString();
        break;
    case SECTION_ADV_HISTO:
        szPageName = "advanced";
        strPageTitle = "Advanced Histogram chart of Tests";
        break;
    case SECTION_ADV_TREND:
        szPageName = "advanced";
        strPageTitle = "Trend chart of Tests";
        break;
    case SECTION_ADV_SCATTER:
        szPageName = "advanced";
        strPageTitle = "Correlation chart of Tests";
        break;
    case SECTION_ADV_BOXPLOT:
        szPageName = "advanced";
        strPageTitle = "Gage R&R, Boxplot/Candle chart of Tests";
        break;
    case SECTION_ADV_BOXPLOT_EX:
        szPageName = "advanced";
        strPageTitle = "Advanced Box-Plot chart of tests";
        break;
    case SECTION_ADV_MULTICHART:
        szPageName = "advanced";
        strPageTitle = "Advanced Multi-chart of tests";
        break;
    case SECTION_ADV_PROBABILITY_PLOT:
        szPageName = "advanced";
        strPageTitle = "Advanced Probability Plot chart of tests";
        break;
    case SECTION_ADV_HISTOFUNCTIONAL:
        szPageName = "advanced";
        strPageTitle = "Advanced Histogram chart of Functional tests";
        break;
    }

    // If title specified by caller, use it instead
    if (szTitle != NULL)
    {
        strPageTitle = szTitle;  // Interactive mode

    }
    if (pChartsInfo == NULL)
    {
        // Normal HTML report generation (not an interactive Charting session)

        // Bookmark (used in HTML flat mode only)
        strBookmark += szPageName;

        if (strOutputFormat == "HTML")
        // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        {
            // If prevous page includes reports and is not an index table,
            // append Prev/Next buttons
            if (hReportFile != NULL)
            {
                fprintf(hReportFile, "</table>\n");
                // If previous page was the Home page, point to it
                if (iCurrentHtmlPage)
                {
                    WriteNavigationButtons(szPageName);
                }
                // Close previous page
                // Writes HTML footer Application name/web site string
                fprintf(hReportFile,
                        C_HTML_FOOTER,
                        GS::Gex::Engine::GetInstance().
                        Get("AppFullName").toString().toLatin1().data());
                fprintf(hReportFile, "</body>\n");
                fprintf(hReportFile, "</html>\n");
                CloseReportFile();  // Close report file
            }
        }
        else
        {
            // If creating a flat HTML file (so to convert to word),
            // then ignore page breaks into a given data section
        }

        // Updates the current page being built
        iCurrentHtmlPage = iTestPage;

        // Open <stdf-filename>/report/<szPageName>.htm
        sprintf(szString,
                "%s/pages/%s%d.htm",
                m_pReportOptions->strReportDirectory.toLatin1().constData(),
                szPageName,
                iCurrentHtmlPage);
    }
    else
    {
        // Create HTML Info page (Interactive charting session)
        // Get Path to HTML info file to create
        if (pGexMainWindow->mWizardsHandler.ChartWizards().isEmpty())
        {
            return;
        }
        strcpy(szString, pGexMainWindow->mWizardsHandler.ChartWizards()[0]->strPropertiesPath.toLatin1().constData());
    }

    // If creating a flat HTML file (so to convert to word),
    // then ignore page breaks into a given data section
    // if in interactive mode, open file
    if ((pChartsInfo != NULL) ||
        (strOutputFormat == "HTML")
        // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        )
    {
        // Open file
        hReportFile = fopen(szString, "wt");
        if (hReportFile == NULL)
        {
            return;
        }
    }

    WriteHeaderHTML(hReportFile, "#000000");  // Default: Text is Black
    if (pChartsInfo != NULL)
    {
        return;  // Interactive mode
    }
    // If standard HTML report pages repeat page type on each page
    // If flat HTML (for Word or PDF),
    // only show page type on 1st page of the section
    if ((iTestPage <= 1) ||
        (strOutputFormat == "HTML")
        // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        )
    {
        WriteHtmlSectionTitle(hReportFile, strBookmark, strPageTitle);
    }

    // Keep track of total HTML pages written
    lReportPageNumber++;

    // Write Interactive link if available
    switch (iSectionType)
    {
    case SECTION_STATS:
        WriteHtmlToolBar(0, true, "drill_table=stats");
        break;
    case SECTION_ADV_BOXPLOT:
        // Write legend...unless we're writting a flat file,
        // and already wrote it one in first page
        if ((iTestPage > 1) &&
            ((strOutputFormat == "DOC") || (strOutputFormat == "PDF") ||
             (strOutputFormat == "PPT") || (strOutputFormat == "ODT"))
            // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            )
        {
            break;
        }

        QStringList qslAdvBoxplotFieldOptionsList =
            m_pReportOptions->GetOption("adv_boxplot",
                                        "field").toString().split(QString("|"));

        if (qslAdvBoxplotFieldOptionsList.contains("boxplot_chart"))
        {
            // Display legend (except for PowerPoint output
            // because formatting is too difficult)
            // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
            // if (strOutputFormat == "HTML" ||
            //     strOutputFormat == "DOC" ||
            //     strOutputFormat == "PDF" ||
            //     strOutputFormat == "PPT" ||
            //     strOutputFormat == "INTERACTIVE")
            if (ReportOptions.isReportOutputHtmlBased())
            {
                fprintf(hReportFile,
                        "<p>For each test or parameter listed, "
                        "the box-plot charts multiple information:<br>\n");
                fprintf(hReportFile,
                        "<img border=\"0\" src=\"../images/gage_legend.png\">"
                        "<br>\n");
                fprintf(hReportFile,
                        "To display Gage info: Repeatability, Reproducibility, "
                        "R&R, %%R&R...check the <a href=\"_gex_options.htm\">"
                        "Options</a> tab, section Gage R&R</p>\n");
                fprintf(hReportFile,
                        "<p>You need more than 1 trial per appraiser to run a Gage R&R report<br>\n");
            }

            // If powerpoint slides, insert page break now
            // if (strOutputFormat == "PPT")
            // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
            WritePageBreak();  // Write this page as a slide (image)
        }
        break;
    }

    // If creating a WORD file (out from a flat HTML),
    // do not include HTML page navigation buttons, otherwise do it
    if (strOutputFormat == "HTML")
    // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
    {
        fprintf(hReportFile, "<table border=\"0\" width=\"100%%\">\n");
        fprintf(hReportFile, "<tr>\n");
        if (iCurrentHtmlPage == 1)
        {
            strcpy(szString, szPageName);
        }
        else
        {
            sprintf(szString, "%s%d", szPageName, iCurrentHtmlPage - 1);
        }
        fprintf(hReportFile,
                "<td width=\"33%%\"><a href=\"%s.htm\">"
                "<img src=\"../images/prev.png\" alt=\"Previous report page\" "
                "border=\"0\" width=\"58\" height=\"31\"></a></td>\n",
                szString);
        fprintf(hReportFile, "<td></td>\n");

        // Middle table cell (empty in header)
        fprintf(hReportFile, "<td width=\"33%%\">&nbsp;</td>\n");

        // If pointing on last HTML report page,
        // 'next' button rollsback to the Index page
        if (iTestPage == mTotalHtmlPages)
        {
            strcpy(szString, szPageName);
        }
        else
        {
            sprintf(szString, "%s%d", szPageName, iCurrentHtmlPage + 1);
        }
        fprintf(hReportFile,
                "<td width=\"33%%\"><p align=\"right\"><a href=\"%s.htm\">"
                "<img src=\"../images/next.png\" alt=\"Next report page\" "
                "border=\"0\" width=\"51\" height=\"30\"></a></td>\n",
                szString);
        fprintf(hReportFile, "</tr>\n</table>\n");
    }

    // Open table at beginning of new page if needed
    switch (iSectionType)
    {
    case SECTION_STATS:
        if (strOutputFormat == "HTML")
        // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        {
            fprintf(hReportFile,
                    "<table border=\"0\" cellspacing=\"1\" width=\"98%%\">\n");
        }
        else
        {
            // Flat HTML: do not recreate a new table on
            // following pages as it wasn't closed in previous one
            if (iTestPage <= 1)
            {
                WriteHtmlOpenTable(98, 0);  // Table Widt= 98%, Cell spacing = 0
            }
        }
        break;
    case SECTION_HISTO:
        break;
    case SECTION_ADV_HISTO:
    case SECTION_ADV_TREND:
    case SECTION_ADV_SCATTER:
    case SECTION_ADV_PROBABILITY_PLOT:
    case SECTION_ADV_BOXPLOT_EX:
        // Update process bar
        // Increment to next step
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, -1, -1);
        break;
    case SECTION_ADV_BOXPLOT:
        // Update process bar
        // Increment to next step
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, -1, -1);
        if (! ReportOptions.isReportOutputHtmlBased()
            // (strOutputFormat != "HTML" &&
            //  strOutputFormat != "DOC" &&
            //  strOutputFormat != "PDF" &&
            //  strOutputFormat != "PPT" &&
            //  strOutputFormat != "INTERACTIVE")
            // (pReportOptions->iOutputFormat &
            //  GEX_OPTION_OUTPUT_HTML_BASED) == 0
            )
        {
            break;
        }
        // Close previous table
        if (iCurrentHtmlPage > 1)
        {
            fprintf(hReportFile, "</table>\n");
        }

        fprintf(hReportFile,
                "<table border=\"0\" cellspacing=\"0\" width=\"98%%\">\n");
        break;
    }
}

/******************************************************************************!
 * \fn EndDataCollection
 * \brief Final work after reading ALL data. Last step before creating reports
 ******************************************************************************/
int CGexReport::EndDataCollection()
{
    int iStatus;
    int iTtmpWafermapType;

    // Get pointer to first group & first file (we always have them exist)
    CGexFileInGroup* pFileG0 = NULL;
    CTest* ptTestCell, * ptG0_TestCell;
    CBinning* ptBinCell;  // Pointer to Bin cell
    CBinning* hBinList = NULL;
    int iGroup = 0;
    int iFile = 0;
    bool bBinningUseWafermapOnly;
    QString strBinningComputationOption =
        (m_pReportOptions->GetOption("binning", "computation")).toString();

    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup* pFile = NULL;

    // Compute max multi-limit item for the whole report
    mMaxMLItems = 0;

    for (iGroup = 0; iGroup < (int) getGroupsList().count(); iGroup++)
    {
        pGroup = getGroupsList().at(iGroup);

        // In case no binning data (SBR) were found in the STDF file,
        // we take a last chance to create them from the wafermap.
        // Then the wafermap data are scanned, but no report is generated yet
        switch (m_pReportOptions->iWafermapType)
        {
        case GEX_WAFMAP_SOFTBIN:  // Standard Wafer map: Software Binning
        case GEX_WAFMAP_STACK_SOFTBIN:
            // Compute bin count for all wafers in ALL groups
            // (onyl call this once)
            // if ((pGroup->cMergedData.ptMergedSoftBinList == NULL ||
            //      pReportOptions->bBinningUseWafermapOnly) && (iGroup == 0))
            if ((pGroup->cMergedData.ptMergedSoftBinList == NULL ||
                 (strBinningComputationOption == "wafer_map")) &&
                (iGroup == 0))
            {
                // Force to merge SOFTBIN data
                iTtmpWafermapType = m_pReportOptions->iWafermapType;
                m_pReportOptions->iWafermapType = GEX_WAFMAP_SOFTBIN;
                WriteWaferMapCharts(FALSE);
                m_pReportOptions->iWafermapType = iTtmpWafermapType;
            }
            hBinList = pGroup->cMergedData.ptMergedSoftBinList;

            break;

        case GEX_WAFMAP_HARDBIN:  // Standard Wafer map: Hardware Binning
        case GEX_WAFMAP_STACK_HARDBIN:
            // Compute bin count for all wafers in ALL groups
            // (onyl call this once)
            // if ((pGroup->cMergedData.ptMergedHardBinList == NULL ||
            //      pReportOptions->bBinningUseWafermapOnly) && (iGroup == 0))
            if ((pGroup->cMergedData.ptMergedHardBinList == NULL ||
                 (strBinningComputationOption == "wafer_map")) &&
                (iGroup == 0))
            {
                // Force to merge HARDBIN data
                iTtmpWafermapType = m_pReportOptions->iWafermapType;
                m_pReportOptions->iWafermapType = GEX_WAFMAP_HARDBIN;
                WriteWaferMapCharts(FALSE);
                m_pReportOptions->iWafermapType = iTtmpWafermapType;
            }
            hBinList = pGroup->cMergedData.ptMergedHardBinList;

            break;
        }

        // Force to recompute the bin tables
        pGroup->UpdateBinTables(true);
        pGroup->UpdateBinTables(false);

        // If build binning from wafermap/stripmap only,
        // then Overwrite count/Yield info
        for (iFile = 0; iFile < (int) pGroup->pFilesList.count(); iFile++)
        {
            pFile = pGroup->pFilesList.at(iFile);

            if (pFile->GetMaxMultiLimitItems() > mMaxMLItems)
                mMaxMLItems = pFile->GetMaxMultiLimitItems();

            // Update the active notch if not defined
            // (from file, detected or forced to a default value)
            pFile->getWaferMapData().UpdateActiveOrientation();

            // Force computing yield over wafer map if option
            // set OR 'What-if' performed over wafer-sort data
            // bBinningUseWafermapOnly =
            //     pReportOptions->bBinningUseWafermapOnly;
            bBinningUseWafermapOnly =
                (strBinningComputationOption == "wafer_map");
            if (m_pReportOptions->getAdvancedReport() == GEX_ADV_GUARDBANDING)
            {
                if (pFile->getWaferMapData().bStripMap == false)
                {
                    bBinningUseWafermapOnly = true;
                }
            }

            if (bBinningUseWafermapOnly)
            {
                // Compute total dies
                pFile->getPcrDatas().lPartCount =
                    pFile->getWaferMapData().iTotalPhysicalDies;

                // Rebuild Production/trend yield info to be accurate
                // (and support retest)
                if (m_pReportOptions->getAdvancedReport() == GEX_ADV_PROD_YIELD)
                {
                    pFile->lAdvBinningTrendTotal =
                        pFile->getWaferMapData().iTotalPhysicalDies;
                    pFile->lAdvBinningTrendTotalMatch = 0;
                }

                // Count good dies
                if ((hBinList != NULL) &&
                    (pFile->getWaferMapData().iTotalPhysicalDies))
                {
                    // Scan all the wafer and count good dies
                    int iLine, iCol, iBinCode;
                    CBinning* hScanBinList;
                    int iGoodBins = 0;  // Used to compute good bins
                    for (iLine = 0;
                         iLine < pFile->getWaferMapData().SizeY;
                         iLine++)
                    {
                        // Processing a wafer line
                        for (iCol = 0;
                             iCol < pFile->getWaferMapData().SizeX;
                             iCol++)
                        {
                            // Get PAT-Man binning at location iRow,iCol
                            iBinCode =
                                pFile->getWaferMapData().
                                getWafMap()[(iCol +
                                             (iLine * pFile->
                                              getWaferMapData().SizeX))].
                                getBin();
                            switch (iBinCode)
                            {
                            case GEX_WAFMAP_EMPTY_CELL:  // -1: Die not tested
                                break;

                            default:  // Die tested, check if PASS or FAIL bin
                                for (hScanBinList = hBinList;
                                     hScanBinList != NULL;
                                     hScanBinList = hScanBinList->ptNextBin)
                                {
                                    if (hScanBinList->iBinValue == iBinCode)
                                    {
                                        // We've found to which family belonngs
                                        // this bin, check if Pass die
                                        if ((hScanBinList->cPassFail == 'P') ||
                                            (hScanBinList->cPassFail != 'F' &&
                                             iBinCode == 1))
                                        {
                                            iGoodBins++;
                                        }
                                    }
                                }

                                // In case production reports enabled,
                                // make sure to recompute yield info
                                if (m_pReportOptions->getAdvancedReport() ==
                                    GEX_ADV_PROD_YIELD)
                                {
                                    // Keep track of exact Good bins tested
                                    // (good bin or bin list selected)
                                    if (m_pReportOptions->pGexAdvancedRangeList
                                        ->IsTestInList(iBinCode, GEX_PTEST))
                                    {
                                        pFile->lAdvBinningTrendTotalMatch++;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    pFile->getPcrDatas().lGoodCount = iGoodBins;
                }
            }

            // Check if GEX accepts STDF file of this tester platform
            iStatus = CheckForTesterPlatform(pFile->getMirDatas().szExecType,
                                             pFile->getMirDatas().szExecVer,
                                             pFile->getMirDatas().szTesterType,
                                             pFile->iStdfAtrTesterBrand);
            if (iStatus != GS::StdLib::Stdf::NoError)
            {
                return iStatus;
            }

            // If wafer info, while MIR info missing, use Wafer info.
            if(pFile->getMirDatas().lStartT <= 0 && pFile->getWaferMapData().lWaferStartTime > 0)
                pFile->getMirDatas().lStartT = pFile->getWaferMapData().lWaferStartTime;
            if(pFile->getMirDatas().lEndT <= 0 && pFile->getWaferMapData().lWaferEndTime > 0)
                pFile->getMirDatas().lEndT = pFile->getWaferMapData().lWaferStartTime;

            // If MIR data exist, wafer exist, but time data missing, use MIR data
            if(pFile->getWaferMapData().bWaferMapExists == true)
            {
                if(pFile->getMirDatas().lStartT > 0 && pFile->getWaferMapData().lWaferStartTime <= 0)
                    pFile->getWaferMapData().lWaferStartTime = pFile->getMirDatas().lStartT;
                if(pFile->getMirDatas().lEndT > 0 && pFile->getWaferMapData().lWaferEndTime <= 0)
                    pFile->getWaferMapData().lWaferEndTime = pFile->getMirDatas().lEndT;
            }
        }

        // Compute number of parts (in soft bin and in hardbin)
        ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
        pGroup->cMergedData.lTotalSoftBins = 0;

        // Loop to list all SOFT Bin cells
        while (ptBinCell != NULL)
        {
            pGroup->cMergedData.lTotalSoftBins += ptBinCell->ldTotalCount;
            ptBinCell = ptBinCell->ptNextBin;
        }

        // Create Hardware Binning Summary report
        ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
        pGroup->cMergedData.lTotalHardBins = 0;
        // Loop to list all HARD Bin cells
        while (ptBinCell != NULL)
        {
            pGroup->cMergedData.lTotalHardBins += ptBinCell->ldTotalCount;
            ptBinCell = ptBinCell->ptNextBin;
        }

        // If Group#2 or higher, check if test limits missing for testing site
        if (iGroup == 0)
        {
            // Save pointer to first test cell (used for other groups)
            pFileG0 =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
        }
        if (iGroup > 0)
        {
            // Scan all tests, and check tests missing limits
            ptTestCell = pGroup->cMergedData.ptMergedTestList;
            while (ptTestCell != NULL)
            {
                if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag
                     & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) ==
                    (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                {
                    // No limits defined, check if limits exist in 1st site
                    if (pFileG0->FindTestCell(
                            ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                            &ptG0_TestCell, FALSE, FALSE) == 1)
                    {
                        ptTestCell->GetCurrentLimitItem()->bLimitFlag = ptG0_TestCell->GetCurrentLimitItem()->bLimitFlag;
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit = ptG0_TestCell->GetCurrentLimitItem()->lfHighLimit;
                        ptTestCell->hlm_scal = ptG0_TestCell->hlm_scal;
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit = ptG0_TestCell->GetCurrentLimitItem()->lfLowLimit;
                        ptTestCell->llm_scal = ptG0_TestCell->llm_scal;
                        strcpy(ptTestCell->szTestUnits,
                               ptG0_TestCell->szTestUnits);
                    }
                }

                // Next test cell
                ptTestCell = ptTestCell->GetNextTest();
            }
        }
    }

    // Get pointer to first group & first file
    if (iGroup > 0)
    {
        pGroup = getGroupsList().isEmpty() ? NULL : getGroupsList().first();
        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    }

    return GS::StdLib::Stdf::NoError;
}

/******************************************************************************!
 * \fn CTestFailCountLessThan
 ******************************************************************************/
bool CTestFailCountLessThan(CTest* s1, CTest* s2)
{
    return s1->GetCurrentLimitItem()->ldFailCount > s2->GetCurrentLimitItem()->ldFailCount;
}

void CGexReport::ComputeStatisticStat( qtTestListStatistics* pqtStatisticsList_Stats)
{
    CTest* ptTestCell;  // Pointer to test cell to receive STDF info
    CGexGroupOfFiles* pGroup;
    // List containing the worst tests
    QList<CTest*> sortedWorstFailingTestsList;

    lTestsInProgram = 0;

    // STATISTICS
    // Get pointer to first group & first file (we always have them exist)
    pGroup = getGroupsList().isEmpty() ? NULL : getGroupsList().first();
    if (pGroup)
    {
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
    }
    else
    {
        ptTestCell = 0;
        GSLOG(SYSLOG_SEV_WARNING, "undefined group !");
    }
    sortedWorstFailingTestsList.clear();

    while (ptTestCell != 0)
    {
        // IF Muti-result parametric test, do not show master test record
        if (ptTestCell->lResultArraySize > 0)
        {
            goto NextTestCell;
        }

        // Ignore Generic Galaxy Parameters
        {
            QString strOptionStorageDevice =
                (ReportOptions.GetOption("statistics",
                                         "generic_galaxy_tests")).toString();
            GEX_ASSERT((strOptionStorageDevice == "hide") ||
                       (strOptionStorageDevice == "show"));
            if ((ptTestCell->bTestType == '-') &&
                (strOptionStorageDevice == "hide")
                )
            {
                goto NextTestCell;
            }
        }

        switch (m_pReportOptions->iStatsType)
        {
        case GEX_STATISTICS_DISABLED:  // No test
            goto NextTestCell;

        case GEX_STATISTICS_ALL:  // All tests
            break;

        case GEX_STATISTICS_FAIL:  // Failing tests only
            if (ptTestCell->GetCurrentLimitItem()->ldFailCount <= 0)
            {
                goto NextTestCell;
            }
            break;

        case GEX_STATISTICS_OUTLIERS:  // Tests with outliers only
            if (ptTestCell->GetCurrentLimitItem()->ldOutliers <= 0)
            {
                goto NextTestCell;
            }
            break;

        case GEX_STATISTICS_LIST:  // Test or test range
            if (m_pReportOptions->pGexStatsRangeList->
                IsTestInList(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex))
            {
                // Good we include this test in the report
                break;
            }
            else
            {
                goto NextTestCell;
            }
            break;

        case GEX_STATISTICS_BADCP:  // Tests with Cp <= value
            if ((ptTestCell->GetCurrentLimitItem()->lfCp < 0) ||
                (ptTestCell->GetCurrentLimitItem()->lfCp > m_pReportOptions->lfStatsLimit))
            {
                goto NextTestCell;
            }
            break;
        case GEX_STATISTICS_BADCPK:  // Tests with Cpk <= value
            if ((ptTestCell->GetCurrentLimitItem()->lfCpk < 0) ||
                (ptTestCell->GetCurrentLimitItem()->lfCpk > m_pReportOptions->lfStatsLimit))
            {
                goto NextTestCell;
            }
            break;

        case GEX_STATISTICS_TOP_N_FAILTESTS:
            // We have first to find which are the top N failing tests,
            // insert them in the list, and append at the whole end
            if (ptTestCell->GetCurrentLimitItem()->ldFailCount <= 0)
            {
                goto NextTestCell;
            }
            if (sortedWorstFailingTestsList.size() <
                (int) m_pReportOptions->lfStatsLimit)
            {
                sortedWorstFailingTestsList.insert(0, ptTestCell);
                qSort(
                    sortedWorstFailingTestsList.begin(),
                    sortedWorstFailingTestsList.end(), CTestFailCountLessThan);
                goto NextTestCell;
            }
            if (ptTestCell->GetCurrentLimitItem()->ldFailCount >
                sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount)
            {
                sortedWorstFailingTestsList.removeLast();
                sortedWorstFailingTestsList.insert(0, ptTestCell);
                qSort(
                    sortedWorstFailingTestsList.begin(),
                    sortedWorstFailingTestsList.end(), CTestFailCountLessThan);
            }
            goto NextTestCell;
            break;
        }

        // Valid entry, add it to the list
        pqtStatisticsList_Stats->append(ptTestCell);

        // Point to next test cell
NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();

    }  // Loop until all test cells read

    if (m_pReportOptions->iStatsType == GEX_STATISTICS_TOP_N_FAILTESTS)
    {
        if (sortedWorstFailingTestsList.size() > 0)
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("successfuly retrieve the top %1 failing tests ! "
                          "(from %2 to %3 failcount)").
                  arg(sortedWorstFailingTestsList.size()).
                  arg(sortedWorstFailingTestsList.first()->GetCurrentLimitItem()->ldFailCount).
                  arg(sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount).
                  toLatin1().constData());
        }

        foreach(CTest * t, sortedWorstFailingTestsList)
        {
            pqtStatisticsList_Stats->append(t);
        }
    }

    // Have the list sorted by test number (default)
    // but this should be configurable in the options
    if (m_pReportOptions->iStatsType != GEX_STATISTICS_TOP_N_FAILTESTS)
    {
        qSort(pqtStatisticsList_Stats->begin(), pqtStatisticsList_Stats->end(),
              SortFieldFuntion(pqtStatisticsList_Stats->getSortField()));
    }
}



void CGexReport::ComputePagesHyperlinks(
    qtTestListStatistics* pqtStatisticsList_Stats,
    qtTestListStatistics* pqtStatisticsList_Histo,
    qtTestListStatistics* qtStatisticsList_Wafermap)
{
    CTest* ptTestCell;  // Pointer to test cell to receive STDF info
    CGexGroupOfFiles* pGroup;
    CGexFileInGroup* pFile;
    long iTestsStatisticsInPage = 0;
    long iTestsHistogramsInPage = 0;
    // List containing the worst tests
    QList<CTest*> sortedWorstFailingTestsList;

    // Number of Statistics HTML pages that will be generated
    mTotalStatisticsPages = 1;
    // Number of Histogram HTML pages that will be generated
    mTotalHistogramPages = 1;
    // Number of wafermap HTML pages that will be generated
    mTotalWafermapPages = 0;
    lTestsInProgram = 0;

    //__________________________________________________________________________
    //
    // STATISTICS
    ComputeStatisticStat(pqtStatisticsList_Stats);

    // Read list based on sorting filter (defined by user in 'Options')
    QString htmlpb = m_pReportOptions->GetOption("output", "html_page_breaks").toString();

    foreach(ptTestCell, *pqtStatisticsList_Stats)
    {
        // We create one Statistics HTML page per X test lines (it's X tests
        // if processing one group, X/2 if processing 2 gruops, etc.)
        if ((iTestsStatisticsInPage >= MAX_STATS_PERPAGE)
            && (htmlpb == "true")
            )
        {
            // Once X tests are in the page, reset counter, increment page count
            iTestsStatisticsInPage = 0;
            mTotalStatisticsPages++;
        }

        lTestsInProgram++;
        ptTestCell->iHtmlStatsPage = mTotalStatisticsPages;
        // We have as many statistics lines per test as groups to compare
        iTestsStatisticsInPage += m_pReportOptions->iGroups;

        // Move to next Test in sorted list
    }  // Loop until all test cells read

    //__________________________________________________________________________
    //
    // HISTOGRAM
    // Get pointer to first group & first file (we always have them exist)
    pGroup = getGroupsList().isEmpty() ? NULL : getGroupsList().first();
    if (pGroup)
    {
        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
            (pGroup->pFilesList.first());
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
    }
    else
    {
        pFile = NULL;
        ptTestCell = NULL;
        GSLOG(SYSLOG_SEV_WARNING, "undefined group !");
    }

    bool bValidTest = false;
    CTest* ptTestCellTmp = NULL;

    sortedWorstFailingTestsList.clear();

    while (ptTestCell != NULL)
    {
        // Histograms disabled
        if (m_pReportOptions->iHistogramType == GEX_HISTOGRAM_DISABLED)
        {
            goto NextTestCellHisto;
        }

        // Ignore Generic Galaxy Parameters
        {
            QString strOptionStorageDevice =
                (m_pReportOptions->
                 GetOption("statistics", "generic_galaxy_tests")).toString();
            if ((ptTestCell->bTestType == '-')
                && (strOptionStorageDevice == "hide"))
            {
                goto NextTestCellHisto;
            }
        }

        bValidTest = false;

        if (getGroupsList().count() == 1)
        {
            // Only test with samples valid execs
            // AND NOT Muti-result parametric test,
            // do not show master test record
            // AND NOT functional test in list, only Parametric tests
            // Display this test
            if ((ptTestCell->ldSamplesValidExecs > 0) &&
                (ptTestCell->lResultArraySize <= 0) &&
                (ptTestCell->bTestType != 'F'))
            {
                bValidTest = true;
            }
        }
        else
        {
            // Check if samples valid each at least in one of all groups
            for (int nGroup = 0;
                 nGroup < getGroupsList().count() && bValidTest == false;
                 nGroup++)
            {
                pGroup = getGroupsList().at(nGroup);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());

                if (pFile->
                    FindTestCell(ptTestCell->lTestNumber,
                                 ptTestCell->lPinmapIndex,
                                 &ptTestCellTmp, true, false,
                                 ptTestCell->
                                 strTestName.toLatin1().constData()) == 1)
                {
                    // Only test with samples valid execs
                    if (ptTestCellTmp->ldSamplesValidExecs <= 0)
                    {
                        continue;
                    }

                    // IF Muti-result parametric test,
                    // do not show master test record
                    if (ptTestCellTmp->lResultArraySize > 0)
                    {
                        continue;
                    }

                    // Do not display functional test in list,
                    // only Parametric tests
                    if (ptTestCellTmp->bTestType == 'F')
                    {
                        continue;
                    }

                    // Data samples found
                    bValidTest = true;
                }
            }
        }

        // Check if we should create a histogram for this test
        if (bValidTest == false)
        {
            goto NextTestCellHisto;
        }

        // Histograms Page counter
        switch (m_pReportOptions->iHistogramTests)
        {
        case GEX_HISTOGRAM_ALL:  // All tests
            break;
        case GEX_HISTOGRAM_LIST:  // test or test range
            if (! m_pReportOptions->pGexHistoRangeList->
                IsTestInList(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex))
            {
                goto NextTestCellHisto;
            }
            break;
        case GEX_HISTOGRAM_TOP_N_FAIL_TESTS:
            if (ptTestCell->GetCurrentLimitItem()->ldFailCount <= 0)
            {
                goto NextTestCellHisto;
            }

            if (sortedWorstFailingTestsList.size() <
                (int) ReportOptions.iHistogramNumberOfTests)
            {
                sortedWorstFailingTestsList.insert(0, ptTestCell);
                qSort(
                    sortedWorstFailingTestsList.begin(),
                    sortedWorstFailingTestsList.end(), CTestFailCountLessThan);
                goto NextTestCellHisto;
            }
            if (ptTestCell->GetCurrentLimitItem()->ldFailCount >
                sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount)
            {
                sortedWorstFailingTestsList.removeLast();
                sortedWorstFailingTestsList.insert(0, ptTestCell);
                qSort(
                    sortedWorstFailingTestsList.begin(),
                    sortedWorstFailingTestsList.end(), CTestFailCountLessThan);
            }
            goto NextTestCellHisto;
            break;
        }

        // Valid entry, add it to the list
        pqtStatisticsList_Histo->append(ptTestCell);

        // Point to next test cell
NextTestCellHisto:
        ptTestCell = ptTestCell->GetNextTest();

    }  // Loop until all test cells read

    if (m_pReportOptions->iHistogramTests == GEX_HISTOGRAM_TOP_N_FAIL_TESTS)
    {
        if (sortedWorstFailingTestsList.size() > 0)
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("successfuly retrieve the top %1 failing tests ! "
                          "(from %2 to %3 failcount)").
                  arg(sortedWorstFailingTestsList.size()).
                  arg(sortedWorstFailingTestsList.first()->GetCurrentLimitItem()->ldFailCount).
                  arg(sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount).
                  toLatin1().constData());
        }

        foreach(CTest * t, sortedWorstFailingTestsList)
        {
            pqtStatisticsList_Histo->append(t);
        }
    }

    // Have the list sorted
    if (m_pReportOptions->iHistogramTests != GEX_HISTOGRAM_TOP_N_FAIL_TESTS)
    {
        qSort(pqtStatisticsList_Histo->begin(), pqtStatisticsList_Histo->end(),
              SortFieldFuntion(pqtStatisticsList_Histo->getSortField()));
    }

    // Read list based on sorting filter (defined by user in 'Options')
    foreach(ptTestCell, *pqtStatisticsList_Histo)
    {
        // We create one Histogram HTML page per X tests
        if ((iTestsHistogramsInPage >= MAX_HISTOGRAM_PERPAGE) &&
            (htmlpb == "true"))
        {
            // Once X tests are in the page, reset counter, increment page count
            iTestsHistogramsInPage = 0;
            mTotalHistogramPages++;
        }

        // Saves page# where histogram will be
        ptTestCell->iHtmlHistoPage = mTotalHistogramPages;
        // Total number of tests indexed in current page
        iTestsHistogramsInPage++;

    }  // Loop until all test cells read

    //__________________________________________________________________________
    //
    // WAFERMAPS tests
    //
    // Get pointer to first group & first file (we always have them exist)
    //
    pGroup = getGroupsList().isEmpty() ? NULL : getGroupsList().first();
    if (pGroup)
    {
        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
            (pGroup->pFilesList.first());
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
    }
    else
    {
        pFile = NULL;
        ptTestCell = NULL;
        GSLOG(SYSLOG_SEV_WARNING, "undefined group !");
    }

    sortedWorstFailingTestsList.clear();

    GSLOG(SYSLOG_SEV_DEBUG, " wafermaps tests...");

    // Check if we should create a wafermap for this test
    if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL ||
        m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
    {
        // Get options about generic galaxy tests
        QString strOptionStorageDevice =
            (m_pReportOptions->GetOption("statistics",
                                         "generic_galaxy_tests")).toString();
        QList<BYTE> lstTestType;

        if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL ||
            m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
        {
            lstTestType << 'P' << 'M' << 'F';
        }
        else
        {
            lstTestType << 'P' << 'M';
        }

        // if generic test are displayed, include authorized testtype
        if (strOptionStorageDevice == "show")
        {
            lstTestType << '-';
        }

        while (ptTestCell != NULL)
        {
            bValidTest = false;

            if (getGroupsList().count() == 1)
            {
                // Only test with samples valid execs
                // AND NOT Muti-result parametric test,
                // do not show master test record
                // AND NOT functional test in list, only Parametric tests
                // Display this test
                if (ptTestCell->ldSamplesValidExecs > 0 &&
                    ptTestCell->lResultArraySize <= 0 &&
                    lstTestType.contains(ptTestCell->bTestType))
                {
                    bValidTest = true;
                }
            }
            else
            {
                // Check if samples valid each at least in one of all groups
                for (int nGroup = 0;
                     nGroup < getGroupsList().count() && bValidTest == false;
                     nGroup++)
                {
                    pGroup = getGroupsList().at(nGroup);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());

                    if (pFile->FindTestCell(ptTestCell->lTestNumber,
                                            ptTestCell->lPinmapIndex,
                                            &ptTestCellTmp,
                                            true, false,
                                            ptTestCell->strTestName.
                                            toLatin1().constData()) == 1)
                    {
                        // Only test with samples valid execs
                        if (ptTestCellTmp->ldSamplesValidExecs <= 0)
                        {
                            continue;
                        }

                        // IF Muti-result parametric test,
                        // do not show master test record
                        if (ptTestCellTmp->lResultArraySize > 0)
                        {
                            continue;
                        }

                        // Do not display functional test in list,
                        // only Parametric tests
                        if (lstTestType.contains(ptTestCell->bTestType) ==
                            false)
                        {
                            continue;
                        }

                        // Data samples found
                        bValidTest = true;
                    }
                }
            }

            // Check if we should create a histogram for this test
            if (bValidTest == false)
            {
                goto NextTestCellWafermap;
            }

            // Histograms Page counter
            switch (m_pReportOptions->iWafermapTests)
            {
            case GEX_WAFMAP_ALL:  // All tests
                break;

            case GEX_WAFMAP_LIST:  // Test or test range
                if (! m_pReportOptions->pGexWafermapRangeList->
                    IsTestInList(ptTestCell->lTestNumber,
                                 ptTestCell->lPinmapIndex))
                {
                    goto NextTestCellWafermap;
                }
                break;
            case GEX_WAFMAP_TOP_N_FAILTESTS:
                if (ptTestCell->GetCurrentLimitItem()->ldFailCount <= 0)
                {
                    goto NextTestCellWafermap;
                }
                if (sortedWorstFailingTestsList.size() <
                    (int) ReportOptions.iWafermapNumberOfTests)
                {
                    sortedWorstFailingTestsList.insert(0, ptTestCell);
                    qSort(
                        sortedWorstFailingTestsList.begin(),
                        sortedWorstFailingTestsList.end(),
                        CTestFailCountLessThan);
                    goto NextTestCellWafermap;
                }
                if (ptTestCell->GetCurrentLimitItem()->ldFailCount >
                    sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount)
                {
                    sortedWorstFailingTestsList.removeLast();
                    sortedWorstFailingTestsList.insert(0, ptTestCell);
                    qSort(
                        sortedWorstFailingTestsList.begin(),
                        sortedWorstFailingTestsList.end(),
                        CTestFailCountLessThan);
                }
                goto NextTestCellWafermap;
                break;
            }

            // Valid entry, add it to the list
            qtStatisticsList_Wafermap->append(ptTestCell);

            // Point to next test cell
NextTestCellWafermap:
            ptTestCell = ptTestCell->GetNextTest();

        }  // Loop until all test cells read
    }

    if (m_pReportOptions->iWafermapTests == GEX_HISTOGRAM_TOP_N_FAIL_TESTS)
    {
        if (sortedWorstFailingTestsList.size() > 0)
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString(" successfuly retrieve the top %1 failing tests ! "
                          "(from %2 to %3 failcount)").
                  arg(sortedWorstFailingTestsList.size()).
                  arg(sortedWorstFailingTestsList.first()->GetCurrentLimitItem()->ldFailCount).
                  arg(sortedWorstFailingTestsList.last()->GetCurrentLimitItem()->ldFailCount).
                  toLatin1().constData());
        }
        foreach(CTest * t, sortedWorstFailingTestsList)
        {
            qtStatisticsList_Wafermap->append(t);
        }
    }

    // Have the list sorted
    // Warning : there was perhaps a bug here :
    // the line was 'pqtStatisticsList_Histo->sort();'
    // Should not it be 'qtStatisticsList_Wafermap->sort();' ?
    if (m_pReportOptions->iWafermapTests != GEX_HISTOGRAM_TOP_N_FAIL_TESTS)
    {
        qSort(
            qtStatisticsList_Wafermap->begin(),
            qtStatisticsList_Wafermap->end(),
            SortFieldFuntion(qtStatisticsList_Wafermap->getSortField()));
    }

    // Read list based on sorting filter (defined by user in 'Options')
    foreach(ptTestCell, *qtStatisticsList_Wafermap)
    {
        mTotalWafermapPages++;
        // Saves page# where histogram will be
        ptTestCell->iHtmlWafermapPage = mTotalWafermapPages;

    }  // Loop until all test cells read

    GSLOG(SYSLOG_SEV_DEBUG, " wafermaps tests - Exit");
}

/******************************************************************************!
 * \fn CreateTestReportPages_UserFlow
 * \brief Creates the report:  Default sections as detailed from 'Settings' page
 ******************************************************************************/
int CGexReport::CreateTestReportPages_UserFlow()
{
    qtTestListStatistics qtStatisticsList_Stats;
    qtTestListStatistics qtStatisticsList_Histo;
    qtTestListStatistics qtStatisticsList_Wafermap;
    int iStartCellIndex = 0, iStatus;

    // Compute HTML hyperlink indexes between pages
    // (eg: cross links between Stats & Histograms)
    ComputePagesHyperlinks(&qtStatisticsList_Stats,
                           &qtStatisticsList_Histo,
                           &qtStatisticsList_Wafermap);

    // Compute HTML Hyperlink indexes between pages for R&R advanced report
    if (m_pReportOptions->getAdvancedReport() == GEX_ADV_CANDLE_MEANRANGE)
        ComputeRAndRTestList();

    // Creates **** TEST STATISTICS **** pages
    iStatus = CreatePages_Stats(&qtStatisticsList_Stats, iStartCellIndex);
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // * Check for user 'Abort' signal*
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return GS::StdLib::Stdf::NoError;
    }

    // Creates **** HISTOGRAMS **** pages
    iStatus = CreatePages_Histo(&qtStatisticsList_Histo);
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // * Check for user 'Abort' signal*
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return GS::StdLib::Stdf::NoError;
    }

    // Creates **** PARETO **** pages
    iStatus = CreatePages_Pareto();
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // * Check for user 'Abort' signal*
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return GS::StdLib::Stdf::NoError;
    }

    // Creates **** WAFERMAP **** pages
    iStatus = CreatePages_Wafermap(&qtStatisticsList_Wafermap);
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // * Check for user 'Abort' signal*
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return GS::StdLib::Stdf::NoError;
    }

    // Creates ****  BINNING **** pages
    iStatus = CreatePages_Binning();
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // * Check for user 'Abort' signal*
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return GS::StdLib::Stdf::NoError;
    }

    // Creates **** ADVANCED CHARTS **** pages
    iStatus = CreatePages_Advanced();
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // Close any page left open
    CloseReport();

    //This is the code to PrepareSection for ReportMessageUnit No more processing => no more messages.

    //Report message List not empty so create this report
    QString lReportName = "report_log";
    GS::Gex::ReportUnit *lLogUnit = NULL;
    if(!mReportUnits.contains(lReportName))
    {
        lLogUnit = new GS::Gex::ReportLogUnit(this, lReportName);
        lLogUnit->PrepareSection(true);
        mReportUnits.insert(lReportName, lLogUnit);
    }
    else
        lLogUnit = mReportUnits[lReportName];

    if(!lLogUnit)
    {
        lLogUnit = new GS::Gex::ReportLogUnit(this, lReportName);
        mReportUnits.insert(lReportName, lLogUnit);
    }

    lLogUnit->CreatePages();

    // Creates Global page, includes total processing time
    GSLOG(SYSLOG_SEV_DEBUG, "Launching CreatePages_Global ");
    iStatus = CreatePages_Global();
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed in CreatePages_Global !");
        return iStatus;
    }

    QString of = ReportOptions.GetOption("output", "format").toString();

    // ******** All reports created, make clean close of opened files ********
    if ((of == "DOC") || (of == "PDF") ||
        (of == "PPT") || (of == "ODT"))
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
    {
        // If flat HTML file, close it properly here
        fprintf(hReportFile, "\n</body>\n");
        fprintf(hReportFile, "</html>\n");
        CloseReportFile(hReportFile);
    }

    return GS::StdLib::Stdf::NoError;
}

/******************************************************************************!
 * \fn CreateTestReportPages
 * \brief Creates the .CSV or HTML file, with all STDF information
 ******************************************************************************/
int CGexReport::CreateTestReportPages(bool* pbNeedsPostProcessing)
{
    int iStatus=-1;

    // Boolean to indicate that the report has been fully created and
    // should be post-processed in case output format is Word, Ppt, ...
    // is set to TRUE if this function fully completes
    *pbNeedsPostProcessing = FALSE;

    // Build test limits strings to be 'report output' compliant
    // (eg: use ',' between limit and units if CSV output)
    BuildAllTestLimitsStrings();

    // check if report has to be built if not, quietly return
    bool bBuildReportOption = (ReportOptions.GetOption(QString("report"), QString("build"))).toBool();
    if (! bBuildReportOption)
    {
        qtTestListStatistics pqtStatisticsList_Stats;
        ComputeStatisticStat(&pqtStatisticsList_Stats);
        return GS::StdLib::Stdf::NoError;
    }

    // Create all BIN images (Hard & Soft)  used in the HTML reports
    CHtmlPng cWebResources;
    if (cWebResources.CreateBinImages(ReportOptions.strReportDirectory.toLatin1().constData()) != true)
    {
        QString strErrMessage =
            "Failed to prepare folder or report folder: \n" +
            ReportOptions.strReportDirectory;
        strErrMessage +=
            "\n\nPossible cause:\n- The file is still opened by another app, "
            "so you have to close it before\n- Read/Write access issue,\n"
            "- a file has the same name as the folder we have tried to create.";
        GS::Gex::Message::information("", strErrMessage);
    }


    // Build report: using user flow, or template file (MyReport)
    if (ReportOptions.strTemplateFile.isEmpty() == false && ReportOptions.getAdvancedReport() == GEX_ADV_TEMPLATE)
    {
        iStatus = CreateTestReportPages_Template();
    }
    else if (ReportOptions.strReportCenterTemplateFile.isEmpty() == false)
    {
        iStatus = CreateTestReportPages_ReportCenter();
    }
    else
    {
        iStatus = CreateTestReportPages_UserFlow();
    }

    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return iStatus;
    }

    // Creating PowerPoint slides: create very last slide in case not flush yet
    QString lRes=WritePowerPointSlide(hReportFile);
    if (lRes.startsWith("err"))
        GSLOG(SYSLOG_SEV_ERROR, lRes.toLatin1().data());

    // Ensure report file closed
    CloseReportFile();  // Close report file

    // Snapshot Gallery only applies to HTML reports
    if (ReportOptions.isReportOutputHtmlBased())
    {
        PrepareSection_Drill(TRUE);
        CloseSection_Drill();
    }

    // Report needs post-processing: convert to final output format
    // (in case it's Word, PDF, PowerPoint)
    *pbNeedsPostProcessing = TRUE;


    return GS::StdLib::Stdf::NoError;
}

/******************************************************************************!
 * \fn ReportPostProcessingConverter
 * \brief Convert report in case output requires post processing
 *        (eg: Word, PowerPoint, PDF, ...)
 ******************************************************************************/
QString CGexReport::ReportPostProcessingConverter()
{
    QString of = ReportOptions.GetOption("output", "format").toString();
    // If output is WORD, then convert current HTML page created into .doc file
    if (of == "DOC")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_WORD)
    {
        GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(" Building Word document...");
        int r = ConvertHtmlToWord();
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("ConvertHtmlToWord returned %1").arg(r).
              toLatin1().constData());
    }

    if (of == "ODT")
    {
        GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(" Building ODT document...");
        int r = ConvertHtmlToODT();
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("ConvertHtml To ODT returned %1").arg(r).
              toLatin1().constData());
        return r==0?"ok":"error";
    }

    // If output is PowerPoint slides,
    // then convert current HTML page(s) created into .ppt file
    if (of == "PPT")
    {
        // Status bar message
        GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(" Building PowerPoint document...");
        // Build PowerPoint document
        QString lRes = ConvertHtmlToPowerpoint();
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("ConvertHtmlToPowerpoint returned %1").arg(lRes).
              toLatin1().constData());
        if (lRes.startsWith("err"))
            return lRes;
    }
    // If output is PDF, then convert current HTML page created into .pdf file
    if (of == "PDF")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PDF)
    {
        // Status bar message
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(
            " Building PDF document...");

        // Build Word document
        ConvertHtmlToPDF();
    }
    return "ok";
}

/******************************************************************************!
 * \fn BuildTargetReportName
 * \brief Build target report name for a .PPT, .DOC or .PDF file
 ******************************************************************************/
QString CGexReport::BuildTargetReportName(QString& strHtmlReportFolder,
                                          const QString strExtension)
{
    QString strDestination;
    // Path: <path>/<query_folder_name>/pages/indexf.htm
    strDestination = reportFlatHtmlAbsPath();
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Build target report name : FlatHtmlAbsPath = %1").
          arg(strDestination).toLatin1().constData());
    int iFolDerPath = strDestination.lastIndexOf('/');
    // Path: <path>/<query_folder_name>/pages
    strDestination.truncate(iFolDerPath);
    iFolDerPath = strDestination.lastIndexOf('/');
    // Path: <path>/<query_folder_name>
    strDestination.truncate(iFolDerPath);
    // This is the folder to erase once the
    // Pdf/word/ppt document will be created
    strHtmlReportFolder = strDestination;
    // Check if the file name need to be changed
    bool isInServerMode =
        GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();
    if (! isInServerMode && reportGenerationMode() != "legacy" &&
        m_bUserAlreadyAsked && ! m_strUserDefRepotFileName.isEmpty())
    {
        iFolDerPath = strDestination.lastIndexOf('/');
        strDestination.truncate(iFolDerPath);
        strDestination += QDir::separator() + m_strUserDefRepotFileName;
    }
    else
    {
        // This is the full document name to create
        strDestination += strExtension;
    }
    if (reportGenerationMode() == "legacy")
    {
        strDestination = buildLegacyMonitoringReportName(strDestination,
                                                         strExtension);
    }
    return strDestination;
}

/******************************************************************************!
 * \fn BuildHeaderFooterText
 ******************************************************************************/
void CGexReport::BuildHeaderFooterText(QString& strHeader, QString& strFooter)
{
    QString strVersionText;
    QString strProductText = " ";
    char cSpace = ' ';

    strHeader = strFooter = " ";

    // Build Version and URL info
    strVersionText =
        QString("Report created with: %1 - www.mentor.com").
        arg(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());

    QString of = ReportOptions.GetOption("output", "format").toString();
    // Define space character (tab under word, etc.)
    if (of == "DOC" || of == "ODT")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_WORD)
    {
        cSpace = '\t';
    }
    else if (of == "PPT")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
    {
        cSpace = ' ';
    }
    else if (of == "PDF")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PDF)
    {
        cSpace = '\t';
    }

    // Default
    strFooter = strVersionText;

    // If multiple files only display Version text as footer
    if (ReportOptions.iFiles != 1)
    {
        return;
    }

    // Build Product & Lot ID info
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().at(0));
    if (pGroup->pFilesList.isEmpty())
    {
        return;
    }
    CGexFileInGroup* pFile = pGroup->pFilesList.at(0);
    if (*pFile->getMirDatas().szPartType)
    {
        strProductText += "Product:" +
            QString(pFile->getMirDatas().szPartType).replace('\"', '_');
        strProductText += cSpace;
    }

    if (*pFile->getMirDatas().szLot)
    {
        strProductText +=
            "  Lot:" + QString(pFile->getMirDatas().szLot).replace('\"', '_');
        strProductText += cSpace;
    }
    if (*pFile->getWaferMapData().szWaferID)
    {
        strProductText += "  Wafer:" +
            QString(pFile->getWaferMapData().szWaferID).replace('\"', '_');
    }
    else if (*pFile->getMirDatas().szSubLot)
    {
        strProductText += "  SubLot:" +
            QString(pFile->getMirDatas().szSubLot).replace('\"', '_');
    }

    // Finalize Header & Footer strings
    if (of == "DOC" || of == "ODT")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_WORD)
    {
        // Note: header line didn't look nice under Word,
        // so now added as a second line in footer
        strHeader = strProductText + QString("   |   Page:");
        strFooter = strVersionText;
    }
    else if (of == "PPT")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
    {
        strHeader = " ";
        strFooter = strVersionText + QString("\t") + strProductText;
    }
    else if (of == "PDF")
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PDF)
    {
        strHeader = " ";
        strFooter = strVersionText + QString("   |   ") + strProductText;
    }
}

/******************************************************************************!
 * \fn ConvertHtmlToPDF
 * \brief Convert HTML report into PDF
 ******************************************************************************/
int CGexReport::ConvertHtmlToPDF(const QString& pdfFileDestination,
                                 const QString &htmlFile,
                                 const QString& imageOption,
                                 const QString& imageOptionFile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("HTML origin: %1").arg(htmlFile).toLatin1().data());
    GSLOG(SYSLOG_SEV_DEBUG, QString("PDF dest: %1").arg(pdfFileDestination).toLatin1().data());
    // Convert flat HTML to PDF file
    CGexPdfReport clPdfReport;
    QString strDestination = pdfFileDestination;
    QString strHtmlReportFolder;
    QDir cDir;
    int nStatus;

    // HTML flat file is <path>/<query_folder_name>/pages/indexf.htm,
    // Pdf output must be: <path>/<query_folder_name>.pdf
    if(strDestination.isEmpty())
    {
        strDestination = BuildTargetReportName(strHtmlReportFolder, ".pdf");
        cDir.remove(strDestination);  // Make sure destination doesn't exist
        GSLOG(SYSLOG_SEV_DEBUG, QString("dest is empty lets create it: %1").arg(strDestination).toLatin1().data());
    }

    // Retrieve PDF Options
    GexPdfOptions stGexPdfOptions;

    // Show progress bar & messages?
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        stGexPdfOptions.m_bShowProgressBar = false;
    }

    // Paper size
    QString ps = ReportOptions.GetOption("output", "paper_size").toString();
    if (ps == "A4")
    {
        stGexPdfOptions.m_nPaperFormat = GexPdfOptions::ePaperFormatA4;
    }
    if (ps == "letter")
    {
        stGexPdfOptions.m_nPaperFormat = GexPdfOptions::ePaperFormatLetter;
    }

    // Paper orientation
    QString pf = ReportOptions.GetOption("output", "paper_format").toString();
    if (pf == "portrait")  // if (pReportOptions->bPortraitFormat)
    {
        stGexPdfOptions.m_nPaperOrientation = GexPdfOptions::ePaperOrientationPortrait;
    }
    else
    {
        stGexPdfOptions.m_nPaperOrientation = GexPdfOptions::ePaperOrientationLandscape;
    }

    QString efinpdf = ReportOptions.GetOption("output", "embed_fonts_in_pdf").toString();
    // stGexPdfOptions.m_bEmbedFonts=false; // Test for litepoint
    stGexPdfOptions.m_bEmbedFonts = (efinpdf == "true") ? true : false;

    // GCORE-118
    stGexPdfOptions.mPrinterType=GexPdfOptions::eHtmlDoc;
    if (ReportOptions.GetOption("output", "pdf_printer").toString()=="QTPRINTER")
        stGexPdfOptions.mPrinterType=GexPdfOptions::eQPrinter;

    // Margins
    stGexPdfOptions.m_nMarginUnits = GexPdfOptions::eMarginUnitsInches;
    stGexPdfOptions.m_lfMargin_Left = 0.5;  // in inches
    stGexPdfOptions.m_lfMargin_Right = 0.5;  // in inches
    stGexPdfOptions.m_lfMargin_Top = 0.5;  // in inches
    stGexPdfOptions.m_lfMargin_Bottom = 0.5;  // in inches

    stGexPdfOptions.m_imageFooter = imageOption;
    stGexPdfOptions.m_imageFooterFile = imageOptionFile;
    // Message box to be used if we fail to create the Pdf file (file locked)
    QString logMessage("Failed to create PDF document... " "maybe it's already in use");
#ifndef GSDAEMON
    QMessageBox mb(
        GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        logMessage + " ?\nIf so, close Acrobat reader first then try again...",
        QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No | QMessageBox::Escape,
        0,
        0);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText(QMessageBox::Yes, "&Try again");
    mb.setButtonText(QMessageBox::NoAll, "&Cancel");
#endif

#ifndef GSDAEMON
build_pdf_file:
#endif

    QString lHtmlFile = htmlFile;
    if(lHtmlFile.isEmpty())
    {
        lHtmlFile = reportFlatHtmlAbsPath();
        GSLOG(SYSLOG_SEV_DEBUG, QString("html is empty lets create it: %1").arg(lHtmlFile).toLatin1().data());
    }

    // Generate PDF document
    nStatus = clPdfReport.GeneratePdfFromHtml(pGexMainWindow,
                            stGexPdfOptions,
                            lHtmlFile,
                            strDestination);
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("GeneratePdfFromHtml retuned %1").arg(nStatus).
          toLatin1().constData());

    // gcore-118: Test: Using pdf QPrinter and QWebview
        /*
        //GexWebBrowser::capture ? no way
        QPrinter lPrinter;
        lPrinter.setOutputFormat(QPrinter::PdfFormat);
        QWebView lWebView;
        lWebView.setUrl(QUrl(reportFlatHtmlAbsPath())); // better ?
        //lWebView.load(QUrl(reportFlatHtmlAbsPath()));
        QCoreApplication::processEvents();
        QThread::currentThread()->sleep(5);
        lWebView.showMaximized();
        //while (!lWebView.showMaximized(); //loadFinished())
            QCoreApplication::processEvents();
        lPrinter.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/temp.pdf");
        for(int i=0; i<100000; i++)
            QCoreApplication::processEvents();
        lWebView.print(&lPrinter);
        */

    switch (nStatus)
    {
    case CGexPdfReport::Err_RemoveDestFile:

        if (stGexPdfOptions.m_bShowProgressBar == false)
        {
            // Running Monitoring: then do not show dialog box,
            // add message to log file instead
            GS::Gex::Message::information(
                "", "PDF-Builder already in use by another process.");
            break;
        }
#ifndef GSDAEMON
        if (mb.exec() == QMessageBox::Yes)
        {
            goto build_pdf_file;  // Try again
        }
#else
        GSLOG(SYSLOG_SEV_WARNING, logMessage.toLatin1().constData());
#endif
        break;

    case CGexPdfReport::ConversionCancelled:
        // Conversion process has been cancelled by the user
        GS::Gex::Message::
        information("", "PDF document generation cancelled.");
        break;

    case CGexPdfReport::Err_MissingDir_App:
        GS::Gex::Message::information(
            "", "Error creating PDF document: couldn't "
            "get application directory.");
        break;

    case CGexPdfReport::Err_MissingExe:
#if (defined __sun__)
        GS::Gex::Message::information(
            "", "Error creating PDF document:\ncouldn't find htmldoc "
            "executable (<application dir>/htmldoc/htmldoc_sol).");
#elif (defined __linux__)
        GS::Gex::Message::information(
            "", "Error creating PDF document:\ncouldn't find htmldoc "
            "executable (<application dir>/htmldoc/htmldoc_linux).");
#else
        GS::Gex::Message::information(
            "", "Error creating PDF document:\ncouldn't find htmldoc "
            "executable (<application dir>/htmldoc/htmldoc.exe).");
#endif
        break;

    case CGexPdfReport::Err_MissingDir_Data:
        GS::Gex::Message::information(
            "", "Error creating PDF document:\ncouldn't find htmldoc "
            "data dir (<application dir>/htmldoc/data).");
        break;

    case CGexPdfReport::Err_MissingDir_Fonts:
        GS::Gex::Message::information(
            "", "Error creating PDF document:\ncouldn't find htmldoc "
            "fonts dir (<application dir>/htmldoc/fonts).");
        break;

    case CGexPdfReport::Err_LaunchHtmlDocProcess:
#if (defined __sun__)
        GS::Gex::Message::information(
            "", "Error creating PDF document:\nerror launching htmldoc "
            "executable (<application dir>/htmldoc/htmldoc_sol).");
#elif (defined __linux__)
        GS::Gex::Message::information(
            "", "Error creating PDF document:\nerror launching htmldoc "
            "executable (<application dir>/htmldoc/htmldoc_linux).");
#else
        GS::Gex::Message::information(
            "", "Error creating PDF document:\nerror launching htmldoc "
            "executable (<application dir>/htmldoc/htmldoc.exe).");
#endif
        break;

    case CGexPdfReport::Err_ProcessError:
        GS::Gex::Message::information(
            "", "Error creating PDF document:\nno PDF file was generated.");
        break;

    case CGexPdfReport::NoError:  // No error
        break;

    default:
        GS::Gex::Message::information("", "Error creating PDF document.");
        break;
    }

    // Cleanup: erase the HTML folder created for the flat HTML file
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().
    DeleteFolderContent(strHtmlReportFolder);

    // Cleanup: erase the HTML file if exists
    if(!GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsCustomDebugMode())
    {
        GS::Gex::Engine::RemoveFileFromDisk(lHtmlFile);
    }

    // Update the report file name created
    // (from HTML file name to '.pdf' file just created
    setLegacyReportName(strDestination);

    return 1;
}

/******************************************************************************!
 * \fn CloseReport
 * \brief Close current report page opened
 ******************************************************************************/
void CGexReport::CloseReport()
{
    if (hReportFile == NULL)
    {
        return;  // May happen if was closed previously for unxpected reasons
    }
    QString of = ReportOptions.GetOption("output", "format").toString();
    if (of == "CSV")
    // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file
        fprintf(hReportFile, "\n");
        // CSV: do not close file as we still need to append the global info
        return;
    }
    // Close index page (unless creating one flat HTML file with all sections
    // (eg: when creating a Word document)
    else if (of == "HTML")
    {
        CloseReportFile();  // Close report file
    }
}

/******************************************************************************!
 * \fn CreateTestReportPages_ReportCenter
 * \brief Create Advanced Enterprise Report page
 ******************************************************************************/
int CGexReport::CreateTestReportPages_ReportCenter()
{
    QString of = ReportOptions.GetOption("output", "format").toString();
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Create Test Report Pages for ReportsCenter in format %1...").
          arg(of).toLatin1().constData());
    // If output report is standard HTML,
    // then force it to FLAT HTML and in sub-folder <pages>
    if (of == "HTML")
    // (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
    {
        // Get path to report home folder
        QFileInfo cFileInfo(reportAbsFilePath());

        // build report page to be in specific sub-folder
        setLegacyReportRootFolder(cFileInfo.absolutePath());
        setLegacyReportName(cFileInfo.absolutePath() + "/pages/index.htm");

        // Create report file(delete if already exists)
        GS::Gex::Engine::RemoveFileFromDisk(reportAbsFilePath());
        hReportFile = fopen(reportAbsFilePath().toLatin1().constData(), "wt");
        if (hReportFile == NULL)
        {
            return GS::StdLib::Stdf::ErrorOpen;
        }
    }

    // If HTML based report, write HTML header block + Table of Contents
    if (ReportOptions.isReportOutputHtmlBased())
    {
        // (of == "HTML" || of == "DOC" ||
        //  of == "PDF" || of == "PPT" || of == "INTERACTIVE")
        // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        WriteHeaderHTML(hReportFile, "#000000", "#FFFFFF", "", false, false);
    }

    // Allocate advanced enterprise report
    QTextStream txtStream(hReportFile);

    if (m_pAdvancedEnterpriseReport)
    {
        m_pAdvancedEnterpriseReport->generate(txtStream);
    }
    else
    {
        m_pAdvancedEnterpriseReport = new GexAdvancedEnterpriseReport;
        if (! m_pAdvancedEnterpriseReport->load(m_pReportOptions->
                                                strReportCenterTemplateFile))
        {
            //FIXME: Herve' : cancel ?
            GSLOG(SYSLOG_SEV_WARNING, "AdvancedEnterpriseReport load failed");
        }
        m_pAdvancedEnterpriseReport->generate(txtStream);
    }

    // AEReports::Report aerReport;
    // aerReport.generate(txtStream,
    //                    pReportOptions->strReportCenterTemplateFile);

    // All reports created, make clean close of opened files
    if ((of == "DOC") || (of == "PDF") ||
        (of == "PPT") || (of == "ODT"))
    // (pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
    {
        // If flat HTML file, close it properly here
        txtStream << endl << "</body>" << endl;
        txtStream << "</html>" << endl;
    }

    // Close file
    CloseReportFile();

    return GS::StdLib::Stdf::NoError;
}

/******************************************************************************!
 * \fn CreateTestReportPages_Template
 * \brief Creates the report: sections defined in Template (MyReport) file
 ******************************************************************************/
int CGexReport::CreateTestReportPages_Template()
{
    // Load template file
    GS::Gex::ReportTemplate reportTemplate;
    GS::Gex::ReportTemplateIO reportTemplateIO;
    reportTemplateIO.ReadTemplateFromDisk(&reportTemplate,
                                          m_pReportOptions->strTemplateFile);

    QString of = ReportOptions.GetOption("output", "format").toString();

    // If output report is standard HTML, then force it to
    // FLAT HTML and in sub-folder <pages>
    if (of == "HTML")
    {
        // Get path to report home folder
        QFileInfo cFileInfo(reportAbsFilePath());
        // build report page to be in specific sub-folder
        setLegacyReportRootFolder(cFileInfo.absolutePath());
        setLegacyReportName(cFileInfo.absolutePath() + "/pages/index.htm");

        // Create report file(delete if already exists)
        GS::Gex::Engine::RemoveFileFromDisk(reportAbsFilePath());
        hReportFile = fopen(reportAbsFilePath().toLatin1().constData(), "wt");
        if (hReportFile == NULL)
        {
            return GS::StdLib::Stdf::ErrorOpen;
        }
    }

    // If HTML based report, write HTML header block + Table of Contents
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        WriteHeaderHTML(hReportFile, "#000000", "#FFFFFF", "", false, false);
        WriteTableOfContents_Template(hReportFile, reportTemplate);
    }

    // Save user options (so not to overwrite them with
    // MyReport's without a backup!)
    CReportOptions cOptionsBackup = ReportOptions;

    // Create report sections
    QString strLinkText;
    QString strBookmark;

    GS::Gex::ReportTemplateSection* pSection;
    GS::Gex::ReportTemplateSectionIterator iter;
    for (iter = reportTemplate.Begin();
         iter != reportTemplate.End(); ++iter)
    {
        // Get section data
        pSection = *iter;
        switch (pSection->iSectionType)
        {
        case GEX_CRPT_WIDGET_AGGREGATE:
            strLinkText = pSection->pAggregate->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_WAFMAP:
            strLinkText = pSection->pWafmap->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_BINNING:
            strLinkText = pSection->pBinning->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_PARETO:
            strLinkText = pSection->pPareto->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_PEARSON:
            strLinkText = pSection->pPearson->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_CORR_GB:
            strLinkText = pSection->pTesterCorrelationGB->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_PRODUCTION:
            strLinkText = pSection->pProduction->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_GLOBALINFO:
            strLinkText = pSection->pGlobalInfo->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_FILE_AUDIT:
            strLinkText = pSection->pFileAudit->strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_ER:
            strLinkText = pSection->pEnterpriseReport->m_strSectionTitle;
            break;
        case GEX_CRPT_WIDGET_DATALOG:
            strLinkText = pSection->pDatalog->strSectionTitle;
            break;
        default:
            break;
        }

        // Write Section's title(including bookmark)
        strBookmark = "section_" + QString::number(pSection->iSection_ID);
        if (of == "CSV")
        {
            fprintf(hReportFile, "\n\n%s\n",
                    strLinkText.toLatin1().constData());
        }
        else
        {
            WriteHtmlSectionTitle(hReportFile, strBookmark, strLinkText);
        }

        // Write section + page break
        WriteSection_Template(pSection, hReportFile);
    }

    // If HTML based report, write Report generation duration
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Wtrite Report generation duration
        char szString[256];
        // Compute elapsed time to create report
        int iElapsedProcessTime = m_tProcessTime.elapsed();
        HMSString(iElapsedProcessTime / 1000,
                  iElapsedProcessTime % 1000,
                  szString);  // Convert processing time (in seconds) to string
        fprintf(hReportFile,
                "<align=\"left\"><font color=\"#000000\" size=\"%d\">"
                "<b>Report generated in: </b>%s<br><br><br></font>\n",
                iHthmNormalFontSize,
                szString);
    }

    // All reports created, make clean close of opened files
    if (of == "DOC" || of == "PDF" || of == "PPT")
    {
        // If flat HTML file, close it properly here
        fprintf(hReportFile, "\n</body>\n");
        fprintf(hReportFile, "</html>\n");
    }

    // Close file
    CloseReportFile();

    // Restore user options
    ReportOptions = cOptionsBackup;

    return GS::StdLib::Stdf::NoError;
}

/******************************************************************************!
 * \fn WriteTableOfContents_Template
 * \brief Writes the Header page +
 *        "Table Of Contents" in the HTML page (as defined in Template)
 ******************************************************************************/
void CGexReport::WriteTableOfContents_Template(
    FILE* hHtmlReportFile,
    GS::Gex::ReportTemplate& reportTemplate)
{
    // Table of content is only for HTML based files (HTML, WORD, PDF, etc.)
    QString strOutputFormat =
        m_pReportOptions->GetOption("output", "format").toString();
    // if((pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED) == 0)
    if (! m_pReportOptions->isReportOutputHtmlBased())
    {
        return;
    }

    QString strTitle = "Quantix Reports";

    // Set Slide Title
    SetPowerPointSlideName(strTitle);

    // Open table to fit all the Table of Content text
    // HTML code to open table, Width 98%, cell spacing=0
    WriteHtmlOpenTable(98, 0);
    fprintf(hHtmlReportFile, "<tr>\n");
    fprintf(hHtmlReportFile, "<td align=");

    // If PPT file, center all text in the table, otherwise, left justified
    if (strOutputFormat == "PPT")
    {
        fprintf(hHtmlReportFile, "\"center\">\n");
    }
    else
    {
        fprintf(hHtmlReportFile, "\"left\">\n");
    }

    // Write slide title (900 pixel wide)
    fprintf(hHtmlReportFile,
            "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" "
            "style=\"border-collapse: collapse\" bordercolor=\"#111111\" "
            "width=\"900\" bgcolor=\"#F8F8F8\">\n");
    fprintf(hHtmlReportFile, "<tr>\n");
    fprintf(hHtmlReportFile,
            " <td width=\"100%%\"><img border=\"0\" "
            "src=\"../images/quantix.png\"></td>\n");
    fprintf(hHtmlReportFile, "</tr>\n");
    fprintf(hHtmlReportFile, "<tr>\n");
    fprintf(hHtmlReportFile,
            "     <td><p align=\"center\"><b>"
            "<font face=\"Arial\" size=\"6\" color=\"#0070C0\">"
            "Welcome To %s!</font></b></p></td>\n",
            strTitle.toLatin1().constData());
    fprintf(hHtmlReportFile, "</tr>\n");
    fprintf(hHtmlReportFile, "</table>\n");

    // Sets default background color = white, text color given in argument
    time_t iTime = time(NULL);
    fprintf(hHtmlReportFile,
            "<align=\"left\"><font color=\"#000000\" size=\"%d\">"
            "<b>Date: %s</b><br></font>\n",
            iHthmNormalFontSize, ctime(&iTime));

    // Write Examinator release (except if PDF file which already
    // writes each page header with this string)
    if (strOutputFormat != "PDF")
    {
        fprintf(hHtmlReportFile,
                "<align=\"left\"><font color=\"#000000\" size=\"%d\">"
                "<b>Report created with: </b>"
                "%s - www.mentor.com<br></font>\n",
                iHthmNormalFontSize,
                GS::Gex::Engine::GetInstance().Get("AppFullName").
                toString().toLatin1().data());
    }

    // Wtrite Data processing duration
    char szString[256];
    // Compute elapsed time to create report
    int iElapsedProcessTime = m_tProcessTime.elapsed();
    // Convert processing time (in seconds) to string
    HMSString(iElapsedProcessTime / 1000, iElapsedProcessTime % 1000, szString);
    fprintf(hHtmlReportFile,
            "<align=\"left\"><font color=\"#000000\" size=\"%d\">"
            "<b>Data processed in: </b>%s<br><br><br></font>\n",
            iHthmNormalFontSize, szString);

    // Reset timer to measure report generation time
    m_tProcessTime.start();

    // User Template Text + Logo (if defined)
    bool bUserLogo = QFile::exists(reportTemplate.GetFrontPageImage());
    bool bUserText = ! reportTemplate.GetFrontPageText().isEmpty();

    // HTML code to open table, Width 98%, cell spacing=8
    WriteHtmlOpenTable(98, 8);
    fprintf(hHtmlReportFile, "<tr>\n");

    if (bUserLogo)  // Logo exists, write it
    {
        fprintf(hHtmlReportFile, "<td align=\"left\">\n");
        QImage qiImage(reportTemplate.GetFrontPageImage());
        fprintf(hHtmlReportFile,
                "<img border=\"0\" src=\"%s\" width=\"%d\" height=\"%d\">\n",
                formatHtmlImageFilename(
                    reportTemplate.GetFrontPageImage()).toLatin1().constData(),
                qiImage.width(),
                qiImage.height());
        fprintf(hHtmlReportFile, "</td>\n<tr align=\"left\">\n");
    }

    // Text
    if (bUserText)
    {
        // Text exists, write it
        fprintf(hHtmlReportFile, "<td align=\"left\" valign=\"top\">\n");
        fprintf(hHtmlReportFile,
                "<font color=\"#000000\" size=\"%d\">",
                iHthmSmallFontSize);
        fprintf(hHtmlReportFile,
                "%s",
                reportTemplate.GetFrontPageText().toLatin1().constData());
        fprintf(hHtmlReportFile, "</font></td></tr>\n<tr>");
    }

    // Close Template table
    fprintf(hHtmlReportFile, "</tr>\n</table>\n<br>\n");

    // Next table cell
    fprintf(hHtmlReportFile, "</td>\n");
    fprintf(hHtmlReportFile, "</tr>\n");
    fprintf(hHtmlReportFile, "<tr>\n");
    fprintf(hHtmlReportFile, "<td align=left>");

    // Add a save button for Enterprise Report
    if ((*reportTemplate.Begin())->iSectionType == GEX_CRPT_WIDGET_ER)
    {
        fprintf(hHtmlReportFile,
                "<p><a href=\"_gex_save.htm\">"
                "<img src=\"../images/save.png\" "
                "border=\"0\" width=\"14\" height=\"14\"></a> :");
        fprintf(hHtmlReportFile,
                "<a href=\"_gex_save.htm\">Save</a> this report's script for ");
        fprintf(hHtmlReportFile,
                "future playback or auto-report under Yield-Man!</p>");
    }

    // Table of contents (hyperlinks to template sections)
    fprintf(hHtmlReportFile,
            "<font color=\"#000000\" size=\"%d\"><br>"
            "<b>Table of contents</b><br></font>\n",
            iHthmNormalFontSize);
    fprintf(hHtmlReportFile, "<blockquote>\n");
    fprintf(hHtmlReportFile, "<p align=\"left\">\n");

    // Write Tables of contents hyperlinks
    QString strLinkText;
    QString strBookmark;

    GS::Gex::ReportTemplateSection* pSection;
    GS::Gex::ReportTemplateSectionIterator iter;
    for (iter = reportTemplate.Begin();
         iter != reportTemplate.End(); ++iter)
    {
        // Get section
        pSection = *iter;
        switch (pSection->iSectionType)
        {
        case GEX_CRPT_WIDGET_AGGREGATE:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pAggregate->strSectionTitle.replace("<", "&lt;");
            pSection->pAggregate->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pAggregate->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_WAFMAP:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pWafmap->strSectionTitle.replace("<", "&lt;");
            pSection->pWafmap->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pWafmap->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_BINNING:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pBinning->strSectionTitle.replace("<", "&lt;");
            pSection->pBinning->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pBinning->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_PARETO:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pPareto->strSectionTitle.replace("<", "&lt;");
            pSection->pPareto->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pPareto->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_PEARSON:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pPearson->strSectionTitle.replace("<", "&lt;");
            pSection->pPearson->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pPearson->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_CORR_GB:
            // Make necessary substitutions in string to be HTML compliant
            pSection->
            pTesterCorrelationGB->strSectionTitle.replace("<", "&lt;");
            pSection->
            pTesterCorrelationGB->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pTesterCorrelationGB->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_PRODUCTION:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pProduction->strSectionTitle.replace("<", "&lt;");
            pSection->pProduction->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pProduction->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_GLOBALINFO:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pGlobalInfo->strSectionTitle.replace("<", "&lt;");
            pSection->pGlobalInfo->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pGlobalInfo->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_FILE_AUDIT:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pFileAudit->strSectionTitle.replace("<", "&lt;");
            pSection->pFileAudit->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pFileAudit->strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_ER:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pEnterpriseReport->m_strSectionTitle.replace("<", "&lt;");
            pSection->pEnterpriseReport->m_strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pEnterpriseReport->m_strSectionTitle;
            break;

        case GEX_CRPT_WIDGET_DATALOG:
            // Make necessary substitutions in string to be HTML compliant
            pSection->pDatalog->strSectionTitle.replace("<", "&lt;");
            pSection->pDatalog->strSectionTitle.replace(">", "&gt;");
            strLinkText = pSection->pDatalog->strSectionTitle;
            break;
        }

        // Write hyperlink
        strBookmark = "section_" + QString::number(pSection->iSection_ID);
        fprintf(hHtmlReportFile,
                "<img border=\"0\" src=\"../images/action.png\" "
                "align=\"center\" width=\"17\" height=\"17\"> "
                "<a href=\"#%s\">%s</a><br>\n",
                strBookmark.toLatin1().constData(),
                strLinkText.toLatin1().constData());
    }

    // Close table (used to have all tables of contents Centered)
    fprintf(hHtmlReportFile, "</blockquote>\n<br>");
    fprintf(hHtmlReportFile, "</td>\n</tr>\n</table>\n");

    // Write page break
    WritePageBreak(hHtmlReportFile);
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Statistics
 * \brief Write Aggregate report 'Test Statistics' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Statistics(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check if Test Statistics to be listed in report
    if (pAggregate->bStats == false)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Prepare section (and write table header if CSV output)
    PrepareSection_Stats(true);

    QString strOutputFormat = m_pReportOptions->GetOption("output", "format").toString();
    QString strStatisticsFieldsOptions =
            (ReportOptions.GetOption(QString("statistics"),QString("fields"))).toString();

    // Open table + write header
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // HTML code to open table, Width 98%, cell spacing=0
        WriteHtmlOpenTable(98, 0);
    }
    int iLineInNewPage = 0;
    int iStatsLine = 0;  // Keeps track of number of line written
    // When creating flat HTML, must insert page break at smart position
    // (before too many lines are written)
    int iLinesInPage = 0;
    // Scan list of tests matching criteria
    foreach(CTest* ptTestCell, qtStatisticsList)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if (iLinesInPage && (iLinesInPage >= iLinesPerFlatPage) &&
            (strOutputFormat == "DOC" || strOutputFormat == "PDF" ||
             strOutputFormat == "PPT" || strOutputFormat == "ODT"))
        {
            // Insert page break (only for PowerPoint as to avoid writing
            // outside of visible image window; PDF & Word: let the
            // application manage page breaks in table
            if (strOutputFormat == "PPT")
            {
                // close table
                fprintf(hReportFile, "</table>\n");
                WritePageBreak();
                // Reopen table + write header
                // HTML code to open table, Width 98%, cell spacing=0
                WriteHtmlOpenTable(98, 0);
            }

            // Ensure we do not have more than about 15 lines
            // before new line header
            iStatsLine = 0;
            iLinesInPage = 0;
        }

        // Compute page# (as sorting may have affected indexes)
        iLineInNewPage++;
        QString htmlpb = m_pReportOptions->GetOption("output", "html_page_breaks").toString();
        if ((iLineInNewPage >= MAX_STATS_PERPAGE) &&
            htmlpb == "true" &&
            strOutputFormat == "HTML")
        {
            // Once X tests are in the page, reset counter,
            // increment page count. Only applies to standad HTML pages,
            // not flat HTML file
            iLineInNewPage = 0;
        }

        // Function writes test statistics for each test
        // (list data for all groups)
        WriteStatsLines(ptTestCell, iStatsLine, strStatisticsFieldsOptions);

        // If single group: table header is written every 15 lines
        // If comparing files (group#>1): header is after EACH test block
        if (m_pReportOptions->iGroups == 1)
        {
            // 15 lines blocks maximum before writting again
            // the table field names, each group has one line per test
            iStatsLine++;

            // Keep track of the line count
            // (never reset, incremented at each line written)
            iLinesInPage++;
        }
        else
        {
            // If writting multi-groups, then each test requires:
            // 1 line for the field names + X lines for the X groups
            iLinesInPage += 1 + m_pReportOptions->iGroups;
        }
        if ((iStatsLine > 0xf) && strOutputFormat == "HTML")
        {
            // Ensure we do not have more than about 15 lines
            // before new line header.Only applies to standad HTML pages,
            // not flat HTML file
            iStatsLine = 0;

        }

    }  // Loop until all test cells read

    // Close table
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // close table + skip a line
        fprintf(hReportFile, "</table>\n");

        // If HTML output, make sure to skip a line
        if (strOutputFormat == "HTML")
        {
            fprintf(hReportFile, "<br><br>\n");
        }
    }
    else
    {
        // CSV output
        fprintf(hReportFile, "\n\n");
    }

    // Write page break
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Histogram
 * \brief Write Aggregate report 'Histograms' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Histogram(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Histograms to be listed in report
    if (pAggregate->iHistogramType < 0)
    {
        return;
    }

    // No Histograms to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(pAggregate->iHistogramType + 1);

    // If chart size=Auto, compute it's real size
    int iChartSize =
        GetChartImageSize(m_pReportOptions->GetOption("adv_histogram",
                                                      "chart_size").toString(),
                          qtStatisticsList.count() * MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    QString strBookmark;
    QString strLinkText;
    bool bWritePageBreak = false;
    bool bFlushLayer = true;
    int iGroupID = 0;
    int iChartLayer = 0;
    int iChartNumber = 0;
    int iMaxChartsPerPage;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);
    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.bChartTitle = pAggregate->bChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.bLegendX = pAggregate->bLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.bLegendY = pAggregate->bLegendY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.bLogScaleX = pAggregate->bLogScaleX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.bLogScaleY = pAggregate->bLogScaleY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.strChartTitle = pAggregate->strChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.strAxisLegendX = pAggregate->strAxisLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeHistogram].
    cChartOptions.strAxisLegendY = pAggregate->strAxisLegendY;

    // Scan list of tests matching criteria
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());
    CGexSingleChart* pLayer = NULL;
    CTest* ptTestCell = qtStatisticsList.first();
    int indexTestCell = 0;
    CTest* ptRefTest = NULL;

    while (ptTestCell != NULL)
    {
        // Clear variables
        bWritePageBreak = false;

        // If no samples available OR functional test, no chart
        if (ptTestCell->bTestType == 'F' ||
            ptTestCell->m_testResult.count() == 0)
        {
            indexTestCell++;
            if (indexTestCell < qtStatisticsList.count())
            {
                ptTestCell = qtStatisticsList.at(indexTestCell);
            }
            else
            {
                ptTestCell = NULL;
            }
        }
        else
        {
            // flush any layer already in memory
            if (bFlushLayer)
            {
                // Empty layer list
                m_pChartsInfo->removeCharts();

                // Define viewport
                if (pAggregate->lfHighX == C_INFINITE &&
                    pAggregate->lfHighY == C_INFINITE &&
                    pAggregate->lfLowX == -C_INFINITE &&
                    pAggregate->lfLowY == -C_INFINITE)
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = -1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    bForceViewport = false;
                }
                else
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = 1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    bForceViewport = true;
                }

                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeHistogram].
                lfHighX =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.lfHighX = pAggregate->lfHighX;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeHistogram].
                lfHighY =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.lfHighY = pAggregate->lfHighY;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeHistogram].
                lfLowX =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.lfLowX = pAggregate->lfLowX;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeHistogram].
                lfLowY =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.lfLowY = pAggregate->lfLowY;

                bFlushLayer = false;
            }

            // If All test layer to chart together, merge them now
            ptRefTest = NULL;
            if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_TEST)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iGroupID);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iGroupID + 1);
                        m_pChartsInfo->addChart(pLayer);
                    }
                }  // stack all groups for this test

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
            {
                // Chart one layer at a time for each test
                pGroup = getGroupsList().at(iGroupID);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());
                pFile->FindTestCell(
                    ptTestCell->lTestNumber,
                    ptTestCell->lPinmapIndex,
                    &ptTestCell,
                    FALSE,
                    FALSE,
                    NULL);

                // If valid dataset
                if (ptTestCell)
                {
                    // Update ref test ptr
                    if (ptRefTest == NULL)
                    {
                        ptRefTest = ptTestCell;
                    }

                    // If split layers, then display
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1) && (iGroupID == 0))
                    {
                        strLinkText.sprintf("T%d : ", ptTestCell->lTestNumber);
                        strLinkText += ptTestCell->strTestName;

                        WriteHtmlSectionTitle(hReportFile,
                                              strBookmark,
                                              strLinkText);
                    }

                    pLayer = new CGexSingleChart(0);
                    pLayer->iGroupX = iGroupID;
                    pLayer->iTestNumberX = ptTestCell->lTestNumber;
                    pLayer->iPinMapX = ptTestCell->lPinmapIndex;

                    // Chart title
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1))
                    {
                        // Banner mode
                        pLayer->strTestLabelX = pGroup->strGroupName;
                    }
                    else
                    {
                        pLayer->strTestLabelX.sprintf(
                            "T%d - %s : ",
                            ptTestCell->lTestNumber,
                            pGroup->strGroupName.
                            toLatin1().constData());
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                    }
                    pLayer->strTestNameX = ptTestCell->strTestName;
                    pLayer->cColor = GetChartingColor(iGroupID + 1);
                    m_pChartsInfo->addChart(pLayer);

                    // Ensure chart title shows the layer name:
                    // use custom title just built
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.bChartTitle = true;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeHistogram].
                    cChartOptions.strChartTitle =
                        pLayer->strTestLabelX;
                }

                // Next chart to be on this same test, but next group
                // (if exists)
                iGroupID++;

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode ==
                     GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iChartLayer);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iChartLayer + 1);
                        m_pChartsInfo->addChart(pLayer);

                        iChartLayer++;
                    }

                }  // stack all groups for this test
            }

            // If no overlay, draw one chart at a time
            pGroup =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            if (pAggregate->iChartMode != GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                if (ptRefTest)
                {
                    WriteAdvHistoChartPage(m_pChartsInfo,
                                           pFile,
                                           ptRefTest,
                                           iChartSize);
                }

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file),
                // insert page break every chart (large images) or
                // every 2 charts (medium images)
                switch (iChartSize)
                {
                case GEX_CHARTSIZE_MEDIUM:
                    iMaxChartsPerPage = 2;

                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 2 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Histogram",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Histogram", 1, 1, ptTestCell);
                    bWritePageBreak = true;
                    break;

                case GEX_CHARTSIZE_BANNER:
                    QString pf = m_pReportOptions->GetOption("output",
                                                             "paper_format").
                        toString();
                    if (pf == "portrait")
                    {
                        iMaxChartsPerPage = 7;  // Portrait format
                    }
                    else
                    {
                        iMaxChartsPerPage = 4;  // Landscapeformat

                    }
                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 4 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Histogram",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    // Page break ?
                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;
                }
            }

            // Point to next test cell, unless each group
            // to be charted idividually
            if (iGroupID >= getGroupsList().count())
            {
                indexTestCell++;
                if (indexTestCell < qtStatisticsList.count())
                {
                    ptTestCell = qtStatisticsList.at(indexTestCell);
                }
                else
                {
                    ptTestCell = NULL;
                }

                // Clear variables
                iGroupID = 0;

                // If banner display mode & each layer in individual chart,
                // force page break after each test
                if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                    (getGroupsList().count() > 1) &&
                    pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
                {
                    bWritePageBreak = true;
                }
            }

            if (bWritePageBreak)
            {
                WritePageBreak();
                iChartNumber = 0;  // Charts in page
            }
        }

    }  // Loop until all test cells read

    // If must overlay ALL tests into one signle chart
    if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
    {
        ptTestCell = qtStatisticsList.first();
        WriteAdvHistoChartPage(m_pChartsInfo, pFile, ptTestCell, iChartSize);
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    RefreshPowerPointSlideName("End of Histograms");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Trend
 * \brief Write Aggregate report 'Trend' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Trend(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Trend to be listed in report
    if (pAggregate->iTrendType < 0)
    {
        return;
    }

    // No Trend to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(pAggregate->iTrendType + 1);

    // If chart size=Auto, compute it's real size
    int iChartSize =
        GetChartImageSize(m_pReportOptions->GetOption("adv_trend",
                                                      "chart_size").toString(),
                          qtStatisticsList.count() * MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    QString strBookmark;
    QString strLinkText;
    bool bWritePageBreak = false;
    bool bFlushLayer = true;
    int iGroupID = 0;
    int iChartLayer = 0;
    int iChartNumber = 0;
    int iMaxChartsPerPage;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);

    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.bChartTitle = pAggregate->bChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.bLegendX = pAggregate->bLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.bLegendY = pAggregate->bLegendY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.bLogScaleX = pAggregate->bLogScaleX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.bLogScaleY = pAggregate->bLogScaleY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.strChartTitle = pAggregate->strChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.strAxisLegendX = pAggregate->strAxisLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeTrend].
    cChartOptions.strAxisLegendY = pAggregate->strAxisLegendY;

    // Scan list of tests matching criteria
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());
    CGexSingleChart* pLayer = NULL;
    CTest* ptTestCellStatistic = qtStatisticsList.first();
    int indexTestCell = 0;
    CTest* ptTestCell = NULL;
    CTest* ptRefTest = NULL;


    qtTestListStatistics::iterator lIterBegin(qtStatisticsList.begin()), lIterEnd(qtStatisticsList.end());
    for(;lIterBegin!=lIterEnd; ++lIterBegin)
    {
        ptTestCellStatistic = *lIterBegin;
        // Clear variables
        bWritePageBreak = false;

        // If no samples available, no chart
        // If no samples available OR functional test, no chart
        if (ptTestCellStatistic->bTestType == 'F' ||
            ptTestCellStatistic->m_testResult.count() == 0)
        {
            indexTestCell++;
            if (indexTestCell < qtStatisticsList.count())
            {
                ptTestCell = qtStatisticsList.at(indexTestCell);
            }
            else
            {
                ptTestCell = NULL;
            }
        }
        else
        {
            // flush any layer already in memory
            if (bFlushLayer)
            {
                // Empty layer list
                m_pChartsInfo->removeCharts();

                // Define viewport
                if (pAggregate->lfHighX == C_INFINITE &&
                    pAggregate->lfHighY == C_INFINITE &&
                    pAggregate->lfLowX == -C_INFINITE &&
                    pAggregate->lfLowY == -C_INFINITE)
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = -1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    bForceViewport = false;
                }
                else
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = 1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    bForceViewport = true;
                }

                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeTrend].lfHighX =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.lfHighX = pAggregate->lfHighX;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeTrend].lfHighY =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.lfHighY = pAggregate->lfHighY;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeTrend].lfLowX =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.lfLowX = pAggregate->lfLowX;
                m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                      chartTypeTrend].lfLowY =
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.lfLowY = pAggregate->lfLowY;

                bFlushLayer = false;
            }

            ptRefTest = NULL;
            // If All test layer to chart together, merge them now
            if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_TEST)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCellStatistic->lTestNumber,
                        ptTestCellStatistic->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iGroupID);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iGroupID + 1);
                        m_pChartsInfo->addChart(pLayer);
                    }

                }  // stack all groups for this test

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
            {
                // Chart one layer at a time for each test
                pGroup = getGroupsList().at(iGroupID);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());
                pFile->FindTestCell(
                    ptTestCellStatistic->lTestNumber,
                    ptTestCellStatistic->lPinmapIndex,
                    &ptTestCell,
                    FALSE,
                    FALSE,
                    NULL);

                // If valid dataset
                if (ptTestCell)
                {
                    // Update ref test ptr
                    if (ptRefTest == NULL)
                    {
                        ptRefTest = ptTestCell;
                    }

                    // If split layers, then display
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1) && (iGroupID == 0))
                    {
                        strLinkText.sprintf("T%d : ", ptTestCell->lTestNumber);
                        strLinkText += ptTestCell->strTestName;

                        WriteHtmlSectionTitle(hReportFile,
                                              strBookmark,
                                              strLinkText);
                    }

                    pLayer = new CGexSingleChart(0);
                    pLayer->iGroupX = iGroupID;
                    pLayer->iTestNumberX = ptTestCell->lTestNumber;
                    pLayer->iPinMapX = ptTestCell->lPinmapIndex;

                    // Chart title
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1))
                    {
                        // Banner mode
                        pLayer->strTestLabelX = pGroup->strGroupName;
                    }
                    else
                    {
                        pLayer->strTestLabelX.sprintf(
                            "T%d - %s : ",
                            ptTestCell->lTestNumber,
                            pGroup->strGroupName.
                            toLatin1().constData());
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                    }
                    pLayer->strTestNameX = ptTestCell->strTestName;
                    pLayer->cColor = GetChartingColor(iGroupID + 1);
                    m_pChartsInfo->addChart(pLayer);

                    // Ensure chart title shows the layer name:
                    // use custom title just built
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.bChartTitle = true;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeTrend].
                    cChartOptions.strChartTitle =
                        pLayer->strTestLabelX;
                }

                // Next chart to be on this same test, but next group
                // (if exists)
                iGroupID++;

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode ==
                     GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCellStatistic->lTestNumber,
                        ptTestCellStatistic->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iChartLayer);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iChartLayer + 1);
                        m_pChartsInfo->addChart(pLayer);

                        iChartLayer++;
                    }

                }  // stack all groups for this test
            }

            // If no overlay, draw one chart at a time
            pGroup =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            if (pAggregate->iChartMode != GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                if (ptRefTest)
                {
                    WriteAdvTrendChartPage(m_pChartsInfo,
                                           pFile,
                                           ptRefTest,
                                           iChartSize);
                }

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file),
                // insert page break every chart (large images) or
                // every 2 charts (medium images)
                switch (iChartSize)
                {
                case GEX_CHARTSIZE_MEDIUM:
                    iMaxChartsPerPage = 2;

                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 2 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Trend",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Trend", 1, 1, ptTestCell);
                    bWritePageBreak = true;
                    break;

                case GEX_CHARTSIZE_BANNER:
                    QString pf = m_pReportOptions->GetOption("output",
                                                             "paper_format").
                        toString();
                    if (pf == "portrait")
                    {
                        iMaxChartsPerPage = 7;  // Portrait format
                    }
                    else
                    {
                        iMaxChartsPerPage = 4;  // Landscapeformat

                    }
                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 4 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Trend",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    // Page break ?
                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;
                }
            }

            // Point to next test cell,
            // unless each group to be charted idividually
            if (iGroupID >= getGroupsList().count())
            {
                indexTestCell++;
                if (indexTestCell < qtStatisticsList.count())
                {
                    ptTestCell = qtStatisticsList.at(indexTestCell);
                }
                else
                {
                    ptTestCell = NULL;
                }

                // Clear variables
                iGroupID = 0;

                // If banner display mode & each layer in individual chart,
                //force page break after each test
                if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                    (getGroupsList().count() > 1) &&
                    pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
                {
                    bWritePageBreak = true;
                }
            }

            if (bWritePageBreak)
            {
                WritePageBreak();
                iChartNumber = 0;  // Charts in page
            }
        }


    }  // Loop until all test cells read

    // If must overlay ALL tests into one signle chart
    if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
    {
        ptTestCell = qtStatisticsList.first();
        WriteAdvTrendChartPage(m_pChartsInfo, pFile, ptTestCell, iChartSize);
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    RefreshPowerPointSlideName("End of Trend charts");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Scatter
 * \brief Write Aggregate report 'Scatter' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Scatter(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Scatter to be listed in report
    if (pAggregate->iScatterType < 0)
    {
        return;
    }

    // No Scatter to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(pAggregate->iScatterType + 1);

    // Get pointer to first group & first file
    // (and 2d group if two goups or more are present)
    CGexGroupOfFiles* pGroup = NULL;
    CGexGroupOfFiles* pGroupY = NULL;
    CGexGroupOfFiles* pFirstGroupX = NULL;
    CGexGroupOfFiles* pFirstGroupY = NULL;
    CGexFileInGroup* pFile = NULL;
    CGexFileInGroup* pFileY = NULL;
    CGexFileInGroup* pFirstFileX = NULL;
    CGexFileInGroup* pFirstFileY = NULL;

    if (m_pReportOptions->iGroups == 1)
    {
        pGroup = pGroupY =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
        pFile = pFileY =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
    }
    else
    {
        pGroup = (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(0);
        pGroupY = (getGroupsList().size() < 2) ? NULL : getGroupsList().at(1);

        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
            (pGroup->pFilesList.first());
        pFileY =
            (pGroupY->pFilesList.isEmpty()) ? NULL :
            (pGroupY->pFilesList.first());
    }

    // Extract all tests to scatter
    GS::QtLib::Range* pRangeListY =
        new GS::QtLib::Range(pAggregate->strRangeListY.toLatin1().constData());

    // Compute total Y tests to plot
    CTest* ptTestCell = NULL;
    CTest* ptTestCellX = NULL;
    CTest* ptTestCellY = NULL;
    CTest* pFirstTestX = NULL;
    CTest* pFirstTestY = NULL;
    int iTotalTests = 0;
    ptTestCellY = pGroupY->cMergedData.ptMergedTestList;
    while (ptTestCellY != NULL)
    {
        // Check if this test belongs to the Y list
        if (pRangeListY->Contains(ptTestCellY->lTestNumber) == true)
        {
            iTotalTests++;
        }

        // Check next test cell
        ptTestCellY = ptTestCellY->GetNextTest();
    }

    // If chart size=Auto, compute it's real size
    int iChartSize =
        GetChartImageSize(m_pReportOptions->GetOption("adv_correlation",
                                                      "chart_size").toString(),
                          iTotalTests * qtStatisticsList.count());
    // was GetChartImageSize(pReportOptions->iScatterChartSize,
    //                       iTotalTests*qtStatisticsList.count());

    // Overlay flag set ?
    bool bNoOverlay = (pAggregate->iChartMode == 0) ? true : false;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);

    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.bChartTitle = pAggregate->bChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.bLegendX = pAggregate->bLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.bLegendY = pAggregate->bLegendY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.bLogScaleX = pAggregate->bLogScaleX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.bLogScaleY = pAggregate->bLogScaleY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.strChartTitle = pAggregate->strChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.strAxisLegendX = pAggregate->strAxisLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeScatter].
    cChartOptions.strAxisLegendY = pAggregate->strAxisLegendY;

    // Define viewport
    if (pAggregate->lfHighX == C_INFINITE &&
        pAggregate->lfHighY == C_INFINITE &&
        pAggregate->lfLowX == -C_INFINITE &&
        pAggregate->lfLowY == -C_INFINITE)
    {
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].bForceViewport = false;
    }
    else
    {
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].bForceViewport = true;
    }

    m_pChartsInfo->getViewportRectangle()
    [GexAbstractChart::chartTypeScatter].lfHighX =
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].cChartOptions.lfHighX =
            pAggregate->lfHighX;
    m_pChartsInfo->getViewportRectangle()
    [GexAbstractChart::chartTypeScatter].lfHighY =
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].cChartOptions.lfHighY =
            pAggregate->lfHighY;
    m_pChartsInfo->getViewportRectangle()
    [GexAbstractChart::chartTypeScatter].lfLowX =
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].cChartOptions.lfLowX =
            pAggregate->lfLowX;
    m_pChartsInfo->getViewportRectangle()
    [GexAbstractChart::chartTypeScatter].lfLowY =
        m_pChartsInfo->getViewportRectangle()
        [GexAbstractChart::chartTypeScatter].cChartOptions.lfLowY =
            pAggregate->lfLowY;
    m_pChartsInfo->lfZoomFactorX = m_pChartsInfo->lfZoomFactorY = 1.0;

    QString of = ReportOptions.GetOption("output", "format").toString();
    QString strString;
    int iChartNumber = 0;
    CGexSingleChart* pLayer = NULL;

    // Scan list of tests matching criteria
    foreach(ptTestCell, qtStatisticsList)
    {
        // Test to plot in X
        ptTestCellX = ptTestCell;

        // List of tests to plot in Y
        ptTestCellY = pGroupY->cMergedData.ptMergedTestList;
        while (ptTestCellY != NULL)
        {
            // Check if this test belongs to the Y list
            if (pRangeListY->Contains(ptTestCellY->lTestNumber) == false)
            {
                goto next_entry;
            }

            // Ignore functional tests
            // If not a parametric / multiparametric
            // (eg: functional) test, ignore
            if (ptTestCellX && ptTestCellX->bTestType == 'F')
            {
                goto next_entry;
            }
            // If not a parametric / multiparametric
            // (eg: functional) test, ignore
            if (ptTestCellY && ptTestCellY->bTestType == 'F')
            {
                goto next_entry;
            }

            // If found tests, create the plot
            if (ptTestCellX && ptTestCellY)
            {
                // If overlay, then save the first pair of tests inserted
                if (! bNoOverlay)
                {
                    pFirstGroupX = pGroup;
                    pFirstFileX = pFile;
                    pFirstTestX = ptTestCellX;
                    pFirstGroupY = pGroupY;
                    pFirstFileY = pFileY;
                    pFirstTestY = ptTestCellY;
                }

                // If overlay disabled, or first overlay:
                // flush any layer already in memory
                if (bNoOverlay || (iChartNumber == 0))
                {
                    m_pChartsInfo->removeCharts();
                }

                // Dynamically build the PowerPoint slide name
                strString = "Scatter : ";
                strString += buildDisplayName(ptTestCellX);
                strString += " vs " + buildDisplayName(ptTestCellY);
                SetPowerPointSlideName(strString);

                // Check color palette to use
                if (bNoOverlay)
                {
                    pLayer = new CGexSingleChart(0);
                }
                else
                {
                    pLayer = new CGexSingleChart(iChartNumber);
                }
                pLayer->iGroupX = 0;
                pLayer->iGroupY = 0;
                pLayer->iTestNumberX = ptTestCellX->lTestNumber;
                pLayer->iPinMapX = ptTestCellX->lPinmapIndex;
                pLayer->iTestNumberY = ptTestCellY->lTestNumber;
                pLayer->iPinMapY = ptTestCellY->lPinmapIndex;
                pLayer->strTestLabelX.sprintf("T%d : ",
                                              ptTestCellX->lTestNumber);
                pLayer->strTestLabelX += ptTestCellX->strTestName;
                pLayer->strTestNameX = ptTestCellX->strTestName;
                pLayer->strTestLabelY.sprintf("T%d : ",
                                              ptTestCellY->lTestNumber);
                pLayer->strTestLabelY += ptTestCellY->strTestName;
                pLayer->strTestNameY = ptTestCellY->strTestName;
                pLayer->bSpots = true;
                pLayer->bLines = pLayer->bFittingCurve =
                        pLayer->bBellCurve =
                            pLayer->bBoxBars = false;
                m_pChartsInfo->addChart(pLayer);

                // If no overlay, draw one scatter plot at a time
                if (bNoOverlay)
                {
                    WriteAdvScatterChartPage(
                        m_pChartsInfo,
                        pGroup,
                        pFile,
                        ptTestCellX,
                        pGroupY,
                        pFileY,
                        ptTestCellY,
                        iChartNumber,
                        iChartSize);

                    // Insert page break if flat HTML
                    // (for Word, PPT or PDF file)
                    if (of == "DOC" || of == "PDF" ||
                        of == "PPT" || of == "ODT")
                    {
                        WritePageBreak();
                    }
                }

                // Update chart count
                iChartNumber++;
            }
next_entry:
            // Move to next test in Y
            ptTestCellY = ptTestCellY->GetNextTest();
        }
    }  // Loop until all test cells read

    // If overlay enabled, write now all layers at once
    if (! bNoOverlay)
    {
        iChartNumber = 1;
        WriteAdvScatterChartPage(
            m_pChartsInfo,
            pFirstGroupX,
            pFirstFileX,
            pFirstTestX,
            pFirstGroupY,
            pFirstFileY,
            pFirstTestY,
            iChartNumber,
            iChartSize);

        // Insert page break if flat HTML (for Word, PPT or PDF file)
        if (of == "DOC" || of == "PDF" || of == "PPT" || of == "ODT")
        {
            WritePageBreak();
        }
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Probabilityplot
 * \brief Write Aggregate report 'Probability Plot' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Probabilityplot(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Histograms to be listed in report
    if (pAggregate->iProbabilityPlotType < 0)
    {
        return;
    }

    // No Histograms to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(
        pAggregate->iProbabilityPlotType + 1);

    // If chart size=Auto, compute it's real size
    int iChartSize =
        GetChartImageSize(m_pReportOptions->GetOption("adv_probabilityplot",
                                                      "chart_size").toString(),
                          qtStatisticsList.count() * MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    QString strBookmark;
    QString strLinkText;
    bool bWritePageBreak = false;
    bool bFlushLayer = true;
    int iGroupID = 0;
    int iChartLayer = 0;
    int iChartNumber = 0;
    int iMaxChartsPerPage;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);

    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.bChartTitle = pAggregate->bChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.bLegendX = pAggregate->bLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.bLegendY = pAggregate->bLegendY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.bLogScaleX = pAggregate->bLogScaleX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.bLogScaleY = pAggregate->bLogScaleY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.strChartTitle = pAggregate->strChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.strAxisLegendX = pAggregate->strAxisLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                          chartTypeProbabilityPlot].
    cChartOptions.strAxisLegendY = pAggregate->strAxisLegendY;

    // Scan list of tests matching criteria
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());
    CGexSingleChart* pLayer = NULL;
    CTest* ptTestCell = qtStatisticsList.first();
    int indexTestCell = 0;
    CTest* ptRefTest = NULL;

    while (ptTestCell != NULL)
    {
        // Clear variables
        bWritePageBreak = false;

        // If no samples available OR functional test, no chart
        if (ptTestCell->bTestType == 'F' ||
            ptTestCell->m_testResult.count() == 0)
        {
            indexTestCell++;
            if (indexTestCell < qtStatisticsList.count())
            {
                ptTestCell = qtStatisticsList.at(indexTestCell);
            }
            else
            {
                ptTestCell = NULL;
            }
        }
        else
        {
            // flush any layer already in memory
            if (bFlushLayer)
            {
                // Empty layer list
                m_pChartsInfo->removeCharts();

                // Define viewport
                if (pAggregate->lfHighX == C_INFINITE &&
                    pAggregate->lfHighY == C_INFINITE &&
                    pAggregate->lfLowX == -C_INFINITE &&
                    pAggregate->lfLowY == -C_INFINITE)
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = -1.0;
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].
                    bForceViewport = false;
                }
                else
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = 1.0;
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].
                    bForceViewport = true;
                }

                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeProbabilityPlot].lfHighX =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].cChartOptions.
                    lfHighX = pAggregate->lfHighX;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeProbabilityPlot].lfHighY =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].cChartOptions.
                    lfHighY = pAggregate->lfHighY;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeProbabilityPlot].lfLowX =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].cChartOptions.
                    lfLowX = pAggregate->lfLowX;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeProbabilityPlot].lfLowY =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].cChartOptions.
                    lfLowY = pAggregate->lfLowY;

                bFlushLayer = false;
            }

            // If All test layer to chart together, merge them now
            ptRefTest = NULL;
            if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_TEST)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iGroupID);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iGroupID + 1);
                        m_pChartsInfo->addChart(pLayer);
                    }

                }  // stack all groups for this test

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
            {
                // Chart one layer at a time for each test
                pGroup = getGroupsList().at(iGroupID);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());
                pFile->FindTestCell(
                    ptTestCell->lTestNumber,
                    ptTestCell->lPinmapIndex,
                    &ptTestCell,
                    FALSE,
                    FALSE,
                    NULL);

                // If valid dataset
                if (ptTestCell)
                {
                    // Update ref test ptr
                    if (ptRefTest == NULL)
                    {
                        ptRefTest = ptTestCell;
                    }

                    // If split layers, then display
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1) && (iGroupID == 0))
                    {
                        strLinkText.sprintf("T%d : ", ptTestCell->lTestNumber);
                        strLinkText += ptTestCell->strTestName;

                        WriteHtmlSectionTitle(hReportFile,
                                              strBookmark,
                                              strLinkText);
                    }

                    pLayer = new CGexSingleChart(0);
                    pLayer->iGroupX = iGroupID;
                    pLayer->iTestNumberX = ptTestCell->lTestNumber;
                    pLayer->iPinMapX = ptTestCell->lPinmapIndex;

                    // Chart title
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1))
                    {
                        // Banner mode
                        pLayer->strTestLabelX = pGroup->strGroupName;
                    }
                    else
                    {
                        pLayer->strTestLabelX.sprintf(
                            "T%d - %s : ",
                            ptTestCell->lTestNumber,
                            pGroup->strGroupName.
                            toLatin1().constData());
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                    }
                    pLayer->strTestNameX = ptTestCell->strTestName;
                    pLayer->cColor = GetChartingColor(iGroupID + 1);
                    m_pChartsInfo->addChart(pLayer);

                    // Ensure chart title shows the layer name:
                    // use custom title just built
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].
                    cChartOptions.bChartTitle = true;
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeProbabilityPlot].
                    cChartOptions.strChartTitle = pLayer->strTestLabelX;
                }

                // Next chart to be on this same test, but next group
                // (if exists)
                iGroupID++;

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode ==
                     GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iChartLayer);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iChartLayer + 1);
                        m_pChartsInfo->addChart(pLayer);

                        iChartLayer++;
                    }

                }  // stack all groups for this test
            }

            // If no overlay, draw one chart at a time
            pGroup =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            if (pAggregate->iChartMode != GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                if (ptRefTest)
                {
                    WriteAdvProbabilityPlotChartPage(m_pChartsInfo,
                                                     pFile,
                                                     ptRefTest,
                                                     iChartSize);
                }

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file),
                // insert page break every chart (large images) or
                // every 2 charts (medium images)
                switch (iChartSize)
                {
                case GEX_CHARTSIZE_MEDIUM:
                    iMaxChartsPerPage = 2;

                    // Dynamically build the PowerPoint slide name
                    //(as name includes the 2 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Probability plot",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Probability plot", 1, 1,
                                               ptTestCell);
                    bWritePageBreak = true;
                    break;

                case GEX_CHARTSIZE_BANNER:
                    QString pf = ReportOptions.GetOption("output",
                                                         "paper_format").
                        toString();
                    if (pf == "portrait")
                    {
                        iMaxChartsPerPage = 7;  // Portrait format
                    }
                    else
                    {
                        iMaxChartsPerPage = 4;  // Landscapeformat

                    }
                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 4 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Probability plot",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    // Page break ?
                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;
                }
            }

            // Point to next test cell,
            // unless each group to be charted idividually
            if (iGroupID >= getGroupsList().count())
            {
                indexTestCell++;
                if (indexTestCell < qtStatisticsList.count())
                {
                    ptTestCell = qtStatisticsList.at(indexTestCell);
                }
                else
                {
                    ptTestCell = NULL;
                }

                // Clear variables
                iGroupID = 0;

                // If banner display mode & each layer in individual chart,
                // force page break after each test
                if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                    (getGroupsList().count() > 1) &&
                    pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
                {
                    bWritePageBreak = true;
                }
            }

            if (bWritePageBreak)
            {
                WritePageBreak();
                iChartNumber = 0;  // Charts in page
            }

        }

    }  // Loop until all test cells read

    // If must overlay ALL tests into one signle chart
    if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
    {
        ptTestCell = qtStatisticsList.first();
        WriteAdvProbabilityPlotChartPage(m_pChartsInfo,
                                         pFile,
                                         ptTestCell,
                                         iChartSize);
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    RefreshPowerPointSlideName("End of Probability plots");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_BoxplotEx
 * \brief Write Aggregate report 'Box Plot' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_BoxplotEx(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Histograms to be listed in report
    if (pAggregate->iBoxplotExType < 0)
    {
        return;
    }

    // No Histograms to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(pAggregate->iBoxplotExType + 1);

    // If chart size=Auto, compute it's real size
    int iChartSize =
        GetChartImageSize(m_pReportOptions->GetOption("adv_boxplot_ex",
                                                      "chart_size").toString(),
                          qtStatisticsList.count() * MAX_HISTOGRAM_PERPAGE);

    // Get pointer to first group & first file (we always have them exist)
    QString strBookmark;
    QString strLinkText;
    bool bWritePageBreak = false;
    bool bFlushLayer = true;
    int iGroupID = 0;
    int iChartLayer = 0;
    int iChartNumber = 0;
    int iMaxChartsPerPage;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);

    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.bChartTitle = pAggregate->bChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.bLegendX = pAggregate->bLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.bLegendY = pAggregate->bLegendY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.bLogScaleX = pAggregate->bLogScaleX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.bLogScaleY = pAggregate->bLogScaleY;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.strChartTitle = pAggregate->strChartTitle;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.strAxisLegendX = pAggregate->strAxisLegendX;
    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].
    cChartOptions.strAxisLegendY = pAggregate->strAxisLegendY;

    // Scan list of tests matching criteria
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());
    CGexSingleChart* pLayer = NULL;
    CTest* ptTestCell = qtStatisticsList.first();
    int indexTestCell = 0;
    CTest* ptRefTest = NULL;

    while (ptTestCell != NULL)
    {
        // Clear variables
        bWritePageBreak = false;

        // If no samples available OR functional test, no chart
        if (ptTestCell->bTestType == 'F' ||
            ptTestCell->m_testResult.count() == 0)
        {
            indexTestCell++;
            if (indexTestCell < qtStatisticsList.count())
            {
                ptTestCell = qtStatisticsList.at(indexTestCell);
            }
            else
            {
                ptTestCell = NULL;
            }
        }
        else
        {
            // flush any layer already in memory
            if (bFlushLayer)
            {
                // Empty layer list
                m_pChartsInfo->removeCharts();

                // Define viewport
                if (pAggregate->lfHighX == C_INFINITE &&
                    pAggregate->lfHighY == C_INFINITE &&
                    pAggregate->lfLowX == -C_INFINITE &&
                    pAggregate->lfLowY == -C_INFINITE)
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = -1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeBoxPlot].
                    bForceViewport = false;
                }
                else
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = 1.0;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeBoxPlot].
                    bForceViewport = true;
                }

                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeBoxPlot].lfHighX =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeBoxPlot].cChartOptions.lfHighX =
                        pAggregate->lfHighX;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeBoxPlot].lfHighY =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeBoxPlot].cChartOptions.lfHighY =
                        pAggregate->lfHighY;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeBoxPlot].lfLowX =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeBoxPlot].cChartOptions.lfLowX =
                        pAggregate->lfLowX;
                m_pChartsInfo->getViewportRectangle()
                [GexAbstractChart::chartTypeBoxPlot].lfLowY =
                    m_pChartsInfo->getViewportRectangle()
                    [GexAbstractChart::chartTypeBoxPlot].cChartOptions.lfLowY =
                        pAggregate->lfLowY;

                bFlushLayer = false;
            }

            // If All test layer to chart together, merge them now
            ptRefTest = NULL;
            if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_TEST)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iGroupID);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iGroupID + 1);
                        m_pChartsInfo->addChart(pLayer);
                    }

                }  // stack all groups for this test

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
            {
                // Chart one layer at a time for each test
                pGroup = getGroupsList().at(iGroupID);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());
                pFile->FindTestCell(
                    ptTestCell->lTestNumber,
                    ptTestCell->lPinmapIndex,
                    &ptTestCell,
                    FALSE,
                    FALSE,
                    NULL);

                // If valid dataset
                if (ptTestCell)
                {
                    // Update ref test ptr
                    if (ptRefTest == NULL)
                    {
                        ptRefTest = ptTestCell;
                    }

                    // If split layers, then display
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1) && (iGroupID == 0))
                    {
                        strLinkText.sprintf("T%d : ", ptTestCell->lTestNumber);
                        strLinkText += ptTestCell->strTestName;

                        WriteHtmlSectionTitle(hReportFile,
                                              strBookmark,
                                              strLinkText);
                    }

                    pLayer = new CGexSingleChart(0);
                    pLayer->iGroupX = iGroupID;
                    pLayer->iTestNumberX = ptTestCell->lTestNumber;
                    pLayer->iPinMapX = ptTestCell->lPinmapIndex;

                    // Chart title
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1))
                    {
                        // Banner mode
                        pLayer->strTestLabelX = pGroup->strGroupName;
                    }
                    else
                    {
                        pLayer->strTestLabelX.sprintf(
                            "T%d - %s : ",
                            ptTestCell->lTestNumber,
                            pGroup->strGroupName.
                            toLatin1().constData());
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                    }
                    pLayer->strTestNameX = ptTestCell->strTestName;
                    pLayer->cColor = GetChartingColor(iGroupID + 1);
                    m_pChartsInfo->addChart(pLayer);

                    // Ensure chart title shows the layer name:
                    // use custom title just built
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeBoxPlot].
                    cChartOptions.bChartTitle = true;
                    m_pChartsInfo->getViewportRectangle()[GexAbstractChart::
                                                          chartTypeBoxPlot].
                    cChartOptions.strChartTitle = pLayer->strTestLabelX;
                }

                // Next chart to be on this same test, but next group
                // (if exists)
                iGroupID++;

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode ==
                     GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iChartLayer);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iChartLayer + 1);
                        m_pChartsInfo->addChart(pLayer);

                        iChartLayer++;
                    }

                }  // stack all groups for this test
            }

            // If no overlay, draw one chart at a time
            pGroup =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            if (pAggregate->iChartMode != GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                if (ptRefTest)
                {
                    WriteAdvBoxplotExChartPage(m_pChartsInfo,
                                               pFile,
                                               ptRefTest,
                                               iChartSize);
                }

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file),
                // insert page break every chart (large images) or
                // every 2 charts (medium images)
                switch (iChartSize)
                {
                case GEX_CHARTSIZE_MEDIUM:
                    iMaxChartsPerPage = 2;

                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 2 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Box plot",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;

                case GEX_CHARTSIZE_LARGE:
                    // Dynamically build the PowerPoint slide name
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Box plot", 1, 1, ptTestCell);
                    bWritePageBreak = true;
                    break;

                case GEX_CHARTSIZE_BANNER:
                    QString pf =
                        m_pReportOptions->GetOption("output",
                                                    "paper_format").toString();
                    if (pf == "portrait")
                    {
                        iMaxChartsPerPage = 7;  // Portrait format
                    }
                    else
                    {
                        iMaxChartsPerPage = 4;  // Landscapeformat

                    }
                    // Dynamically build the PowerPoint slide name
                    // (as name includes the 4 tests writtent per page)
                    // Ignored if not generating a PPT file
                    RefreshPowerPointSlideName("Box plot",
                                               iChartNumber,
                                               iMaxChartsPerPage,
                                               ptTestCell);

                    // Page break ?
                    if ((iChartNumber % iMaxChartsPerPage) == 0)
                    {
                        bWritePageBreak = true;
                    }
                    break;
                }
            }

            // Point to next test cell,
            // unless each group to be charted individually
            if (iGroupID >= getGroupsList().count())
            {
                indexTestCell++;
                if (indexTestCell < qtStatisticsList.count())
                {
                    ptTestCell = qtStatisticsList.at(indexTestCell);
                }
                else
                {
                    ptTestCell = NULL;
                }

                // Clear variables
                iGroupID = 0;

                // If banner display mode & each layer in individual chart,
                // force page break after each test
                if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                    (getGroupsList().count() > 1) &&
                    pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
                {
                    bWritePageBreak = true;
                }
            }

            if (bWritePageBreak)
            {
                WritePageBreak();
                iChartNumber = 0;  // Charts in page
            }
        }
    }  // Loop until all test cells read

    // If must overlay ALL tests into one signle chart
    if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
    {
        ptTestCell = qtStatisticsList.first();
        WriteAdvBoxplotExChartPage(m_pChartsInfo,
                                   pFile, ptTestCell, iChartSize);
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    RefreshPowerPointSlideName("End of box plots");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_MultiCharts
 * \brief Write Aggregate report 'MultiChart' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_MultiCharts(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check Histograms to be listed in report
    if (pAggregate->iMultiChartsType < 0)
    {
        return;
    }

    // No Histograms to plot
    if (qtStatisticsList.count() <= 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->
    setAdvancedReportSettings(pAggregate->iMultiChartsType + 1);

    // If chart size=Auto, compute it's real size
    int iChartSize = GEX_CHARTSIZE_MEDIUM;

    // Get pointer to first group & first file (we always have them exist)
    QString strBookmark;
    QString strLinkText;
    bool bWritePageBreak = false;
    bool bFlushLayer = true;
    int iGroupID = 0;
    int iChartLayer = 0;
    int iChartNumber = 0;

    // Prepare layers setup
    m_pChartsInfo->clear();
    m_pChartsInfo->InitGlobal(m_pReportOptions);

    m_pChartsInfo->bFileOutput = true;  // Chart to be saved to disk
    foreach(int iChartType, m_pChartsInfo->getViewportRectangle().keys()) {
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        bChartTitle = pAggregate->bChartTitle;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        bLegendX = pAggregate->bLegendX;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        bLegendY = pAggregate->bLegendY;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        bLogScaleX = pAggregate->bLogScaleX;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        bLogScaleY = pAggregate->bLogScaleY;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        strChartTitle = pAggregate->strChartTitle;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        strAxisLegendX = pAggregate->strAxisLegendX;
        m_pChartsInfo->getViewportRectangle()[iChartType].cChartOptions.
        strAxisLegendY = pAggregate->strAxisLegendY;
    }
    // Scan list of tests matching criteria
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());
    CGexSingleChart* pLayer = NULL;
    CTest* ptTestCell = qtStatisticsList.first();
    int indexTestCell = 0;
    CTest* ptRefTest = NULL;

    while (ptTestCell != NULL)
    {
        // Clear variables
        bWritePageBreak = false;

        // If no samples available OR functional test, no chart
        if (ptTestCell->bTestType == 'F' ||
            ptTestCell->m_testResult.count() == 0)
        {
            indexTestCell++;
            if (indexTestCell < qtStatisticsList.count())
            {
                ptTestCell = qtStatisticsList.at(indexTestCell);
            }
            else
            {
                ptTestCell = NULL;
            }
        }
        else
        {
            // flush any layer already in memory
            if (bFlushLayer)
            {
                // Empty layer list
                m_pChartsInfo->removeCharts();

                // Define viewport
                if (pAggregate->lfHighX == C_INFINITE &&
                    pAggregate->lfHighY == C_INFINITE &&
                    pAggregate->lfLowX == -C_INFINITE &&
                    pAggregate->lfLowY == -C_INFINITE)
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = -1.0;
                    foreach(int iChartType,
                            m_pChartsInfo->getViewportRectangle().keys()) {
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        bForceViewport = false;
                    }
                }
                else
                {
                    m_pChartsInfo->lfZoomFactorX =
                        m_pChartsInfo->lfZoomFactorY = 1.0;
                    foreach(int iChartType,
                            m_pChartsInfo->getViewportRectangle().keys()) {
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        bForceViewport = true;
                    }
                }

                foreach(int iChartType,
                        m_pChartsInfo->getViewportRectangle().keys()) {
                    m_pChartsInfo->getViewportRectangle()[iChartType].lfHighX =
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.lfHighX = pAggregate->lfHighX;
                    m_pChartsInfo->getViewportRectangle()[iChartType].lfHighY =
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.lfHighY = pAggregate->lfHighY;
                    m_pChartsInfo->getViewportRectangle()[iChartType].lfLowX =
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.lfLowX = pAggregate->lfLowX;
                    m_pChartsInfo->getViewportRectangle()[iChartType].lfLowY =
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.lfLowY = pAggregate->lfLowY;
                }

                bFlushLayer = false;
            }

            // If All test layer to chart together, merge them now
            ptRefTest = NULL;
            if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_TEST)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iGroupID);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iGroupID + 1);
                        m_pChartsInfo->addChart(pLayer);
                    }

                }  // stack all groups for this test

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
            {
                // Chart one layer at a time for each test
                pGroup = getGroupsList().at(iGroupID);
                pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                    (pGroup->pFilesList.first());
                pFile->FindTestCell(
                    ptTestCell->lTestNumber,
                    ptTestCell->lPinmapIndex,
                    &ptTestCell,
                    FALSE,
                    FALSE,
                    NULL);

                // If valid dataset
                if (ptTestCell)
                {
                    // Update ref test ptr
                    if (ptRefTest == NULL)
                    {
                        ptRefTest = ptTestCell;
                    }

                    // If split layers, then display
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1) && (iGroupID == 0))
                    {
                        strLinkText.sprintf("T%d : ", ptTestCell->lTestNumber);
                        strLinkText += ptTestCell->strTestName;

                        WriteHtmlSectionTitle(hReportFile,
                                              strBookmark,
                                              strLinkText);
                    }

                    pLayer = new CGexSingleChart(0);
                    pLayer->iGroupX = iGroupID;
                    pLayer->iTestNumberX = ptTestCell->lTestNumber;
                    pLayer->iPinMapX = ptTestCell->lPinmapIndex;

                    // Chart title
                    if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                        (getGroupsList().count() > 1))
                    {
                        // Banner mode
                        pLayer->strTestLabelX = pGroup->strGroupName;
                    }
                    else
                    {
                        pLayer->strTestLabelX.sprintf(
                            "T%d - %s : ",
                            ptTestCell->lTestNumber,
                            pGroup->strGroupName.
                            toLatin1().constData());
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                    }
                    pLayer->strTestNameX = ptTestCell->strTestName;
                    pLayer->cColor = GetChartingColor(iGroupID + 1);
                    m_pChartsInfo->addChart(pLayer);

                    // Ensure chart title shows the layer name:
                    // use custom title just built
                    foreach(int iChartType,
                            m_pChartsInfo->getViewportRectangle().keys()) {
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.bChartTitle = true;
                        m_pChartsInfo->getViewportRectangle()[iChartType].
                        cChartOptions.strChartTitle = pLayer->strTestLabelX;
                    }
                }

                // Next chart to be on this same test, but next group
                // (if exists)
                iGroupID++;

                bFlushLayer = true;
            }
            else if (pAggregate->iChartMode ==
                     GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                for (iGroupID = 0;
                     iGroupID < getGroupsList().count();
                     iGroupID++)
                {
                    // Get pointer to test cell
                    pGroup = getGroupsList().at(iGroupID);
                    pFile =
                        (pGroup->pFilesList.isEmpty()) ? NULL :
                        (pGroup->pFilesList.first());
                    pFile->FindTestCell(
                        ptTestCell->lTestNumber,
                        ptTestCell->lPinmapIndex,
                        &ptTestCell,
                        FALSE,
                        FALSE,
                        NULL);

                    // If valid dataset
                    if (ptTestCell)
                    {
                        // Update ref test ptr
                        if (ptRefTest == NULL)
                        {
                            ptRefTest = ptTestCell;
                        }

                        // Check color palette to use
                        pLayer = new CGexSingleChart(iChartLayer);
                        pLayer->iGroupX = iGroupID;
                        pLayer->iTestNumberX = ptTestCell->lTestNumber;
                        pLayer->iPinMapX = ptTestCell->lPinmapIndex;
                        pLayer->strTestLabelX.sprintf("T%d : ",
                                                      ptTestCell->lTestNumber);
                        pLayer->strTestLabelX += ptTestCell->strTestName;
                        pLayer->strTestNameX = ptTestCell->strTestName;
                        pLayer->cColor = GetChartingColor(iChartLayer + 1);
                        m_pChartsInfo->addChart(pLayer);

                        iChartLayer++;
                    }

                }  // stack all groups for this test
            }

            // If no overlay, draw one chart at a time
            pGroup =
                (getGroupsList().isEmpty()) ? NULL :
                (getGroupsList().first());
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            if (pAggregate->iChartMode != GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
            {
                if (ptRefTest)
                {
                    WriteAdvMultiChartPage(m_pChartsInfo,
                                           pFile,
                                           ptRefTest,
                                           iChartSize);
                }

                // Update chart count
                iChartNumber++;

                // When writing flat HTML (for Word or PDF file),
                // insert page break every chart (large images) or
                // every 2 charts (medium images)
                // Dynamically build the PowerPoint slide name
                // Ignored if not generating a PPT file
                RefreshPowerPointSlideName("MultiCharts", 1, 1, ptTestCell);
                bWritePageBreak = true;
            }

            // Point to next test cell,
            // unless each group to be charted idividually
            if (iGroupID >= getGroupsList().count())
            {
                indexTestCell++;
                if (indexTestCell < qtStatisticsList.count())
                {
                    ptTestCell = qtStatisticsList.at(indexTestCell);
                }
                else
                {
                    ptTestCell = NULL;
                }

                // Clear variables
                iGroupID = 0;

                // If banner display mode & each layer in individual chart,
                // force page break after each test
                if ((iChartSize == GEX_CHARTSIZE_BANNER) &&
                    (getGroupsList().count() > 1) &&
                    pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_LAYER)
                {
                    bWritePageBreak = true;
                }
            }

            if (bWritePageBreak)
            {
                WritePageBreak();
                iChartNumber = 0;  // Charts in page
            }

        }
    }  // Loop until all test cells read

    // If must overlay ALL tests into one signle chart
    if (pAggregate->iChartMode == GEX_CRPT_CHART_MODE_SINGLE_OVERLAY)
    {
        ptTestCell = qtStatisticsList.first();
        WriteAdvMultiChartPage(m_pChartsInfo, pFile, ptTestCell, iChartSize);
    }

    // Erase layers from memory
    m_pChartsInfo->clear();

    // Write page break
    RefreshPowerPointSlideName("End of MultiCharts");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate_Boxplot
 * \brief Write Aggregate report 'BoxPlot' section
 ******************************************************************************/
void CGexReport::WriteSection_Template_Aggregate_Boxplot(
    GS::Gex::CustomReportTestAggregateSection* pAggregate,
    qtTestListStatistics& qtStatisticsList)
{
    // Check BoxPlot to be listed in report
    if (pAggregate->iBoxPlotType < 0)
    {
        return;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return;
    }

    QString of = ReportOptions.GetOption("output", "format").toString();

    int iMaxLinesPerPage = 15;
    if (of == "PPT")
    {
        iMaxLinesPerPage = 11;  // Max of 11 charts per page under PowerPoint
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        iMaxLinesPerPage = 15;  // Max of 15 chart per page

    }
    // Open table + write header
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // HTML code to open table, Width 98%, cell spacing=0
        WriteHtmlOpenTable(98, 0);
    }
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles* pGroup = 0;
    CGexFileInGroup* pFile = 0;
    int iBoxPlotLine = 0;
    pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());

    // Scan list of tests matching criteria
    foreach(CTest * ptTestCell, qtStatisticsList)
    {
        // If no samples available or functional test, no chart
        if (ptTestCell->m_testResult.count() > 0 ||
            ptTestCell->bTestType != 'F')
        {
            // Write all box-plot lines for all groups of given test
            WriteAdvBoxPlotLines(pFile, ptTestCell, iBoxPlotLine);

            // If single group: table header is written every 15 lines
            // If comparing files (group#>1): header is after EACH test block
            // X lines blocks maximum, each group has one line per test
            iBoxPlotLine += m_pReportOptions->iGroups;
            if (iBoxPlotLine > iMaxLinesPerPage &&
                m_pReportOptions->isReportOutputHtmlBased())
            {
                // Ensure we do not have more than about X lines
                // before new line header
                iBoxPlotLine = 0;
            }
        }
    }  // Loop until all test cells read

    // Close table
    if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // close table + skip a line
        fprintf(hReportFile, "</table>\n");

        // If HTML output, make sure to skip a line
        if (of == "HTML")
        {
            fprintf(hReportFile, "<br><br>\n");
        }
    }
    else
    {
        // CSV output
        fprintf(hReportFile, "\n\n");
    }

    // Write page break
    RefreshPowerPointSlideName("End of BoxPlots");
    if (qtStatisticsList.count() > 0)
    {
        WritePageBreak();
    }
}

/******************************************************************************!
 * \fn WriteSection_Template_Aggregate
 * \brief Write Aggregate report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Aggregate(
    GS::Gex::ReportTemplateSection* pSection,
    FILE*  /*hReportFile*/)
{
    GS::Gex::CustomReportTestAggregateSection* pAggregate =
        pSection->pAggregate;
    // List of Test Ranges to process
    CGexTestRange* pGexStatsRangeList = NULL;
    double lfLimit = 0;
    int nTopNFail = 0;

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Status bar message
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(
        " My Reports: Building Aggregate section...");

    // Note: iTestType needs to be offset by one to compensate first
    // combo-selection that is removed for MyReports (xx_DISABLED)
    switch (pAggregate->iTestType)
    {
    case GEX_CRPT_TEST_TYPE_LIST:  // test or test range
        if (pGexStatsRangeList)
        {
            delete pGexStatsRangeList;
        }
        pGexStatsRangeList = createTestsRange(pAggregate->strRangeList,
                                              true,
                                              false);
        break;

    case GEX_CRPT_TEST_TYPE_BADCP:  // Tests with Cp <= value
    case GEX_CRPT_TEST_TYPE_BADCPK:  // Tests with Cpk <= value
        lfLimit = pAggregate->strRangeList.toDouble();
        break;

    case GEX_CRPT_TEST_TYPE_TOP_N_FAIL:  // Top N failing tests
        nTopNFail = pAggregate->strRangeList.toInt();
        break;
    }

    // Fill the sorting-list object with Statistics aggregate table
    qtTestListStatistics qtStatisticsList;
    // Move back to first group
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());

    if (pAggregate->iTestType == GEX_CRPT_TEST_TYPE_TOP_N_FAIL)
    {
        QList<CTest*> sortedWorstFailingTestsList;

        // Retrieve the Top N Failings Tests
        RetrieveTopNWorstTests(nTopNFail,
                               pGroup->cMergedData.ptMergedTestList,
                               sortedWorstFailingTestsList);

        // Fill the test list
        while (sortedWorstFailingTestsList.isEmpty() == false)
        {
            qtStatisticsList.append(sortedWorstFailingTestsList.takeFirst());
        }
    }
    else
    {
        QString lOptionStorageDevice =
            ReportOptions.GetOption("statistics",
                                    "generic_galaxy_tests").toString();

        // Create aggregate report for each test#
        CTest* ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while (ptTestCell != NULL)
        {
            // If neither PTR values, this test was not executed
            if (ptTestCell->ldExecs == 0 && ptTestCell->GetCurrentLimitItem()->ldOutliers == 0)
            {
                goto NextTestCell2;
            }

            // IF Muti-result parametric test, do not show master test record
            if (ptTestCell->lResultArraySize > 0)
            {
                goto NextTestCell2;
            }

            if ((ptTestCell->bTestType == '-' &&
                 lOptionStorageDevice == "hide"))
            {
                goto NextTestCell2;
            }

            // Check which tests to include in aggregate report
            switch (pAggregate->iTestType)
            {
            case GEX_CRPT_TEST_TYPE_ALL:  // All tests
                break;

            case GEX_CRPT_TEST_TYPE_FAIL:  // Failing tests only
                if (ptTestCell->GetCurrentLimitItem()->ldFailCount <= 0)
                {
                    goto NextTestCell2;
                }
                break;

            case GEX_CRPT_TEST_TYPE_OUTLIERS:  // Tests with outliers only
                if (ptTestCell->GetCurrentLimitItem()->ldOutliers <= 0)
                {
                    goto NextTestCell2;
                }
                break;

            case GEX_CRPT_TEST_TYPE_LIST:  // test or test range
                if (pGexStatsRangeList->IsTestInList(ptTestCell->lTestNumber,
                                                     ptTestCell->lPinmapIndex))
                {
                    // Good we include this test in the report
                    break;
                }
                else
                {
                    goto NextTestCell2;
                }
                break;

            case GEX_CRPT_TEST_TYPE_BADCP:  // Tests with Cp <= value
                if ((ptTestCell->GetCurrentLimitItem()->lfCp < 0) || (ptTestCell->GetCurrentLimitItem()->lfCp > lfLimit))
                {
                    goto NextTestCell2;
                }
                break;
            case GEX_CRPT_TEST_TYPE_BADCPK:  // Tests with Cpk <= value
                if ((ptTestCell->GetCurrentLimitItem()->lfCpk < 0) || (ptTestCell->GetCurrentLimitItem()->lfCpk > lfLimit))
                {
                    goto NextTestCell2;
                }
                break;
            }

            // Valid entry, add it to the list
            qtStatisticsList.append(ptTestCell);

            // Next Test
NextTestCell2:
            ptTestCell = ptTestCell->GetNextTest();
        }  // Loop until all test cells read

        // Have the list sorted
        qSort(qtStatisticsList.begin(),
              qtStatisticsList.end(),
              SortFieldFuntion(qtStatisticsList.getSortField()));
    }

    // Write Test statistics tables (if enabled)
    WriteSection_Template_Aggregate_Statistics(pAggregate, qtStatisticsList);

    // Write Test Histogram charts (if enabled)
    WriteSection_Template_Aggregate_Histogram(pAggregate, qtStatisticsList);

    // Write Trend charts (if enabled)
    WriteSection_Template_Aggregate_Trend(pAggregate, qtStatisticsList);

    // Write Scatter charts (if enabled)
    WriteSection_Template_Aggregate_Scatter(pAggregate, qtStatisticsList);

    // Write Probability Plot charts (if enabled)
    WriteSection_Template_Aggregate_Probabilityplot(pAggregate,
                                                    qtStatisticsList);

    // Write box Plot charts (if enabled)
    WriteSection_Template_Aggregate_BoxplotEx(pAggregate, qtStatisticsList);

    // Write Multi charts (if enabled)
    WriteSection_Template_Aggregate_MultiCharts(pAggregate, qtStatisticsList);

    // Write Box-Plot charts (if enabled)
    WriteSection_Template_Aggregate_Boxplot(pAggregate, qtStatisticsList);

    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_Wafmap
 * \brief Write Wafer map report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Wafmap(
    GS::Gex::ReportTemplateSection* pSection,
    FILE*  /*hReportFile*/)
{
    // Check BoxPlot to be listed in report
    GS::Gex::CGexCustomReport_WafMap_Section* pWafmap = pSection->pWafmap;
    if (pWafmap->iWafermapType < 0)
    {
        return 1;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Reset handles
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());  // First group
    CGexFileInGroup* pFile =
        (pGroup->pFilesList.isEmpty()) ? NULL :
        (pGroup->pFilesList.first());  // First file
    qtTestListStatistics qtStatisticsList;

    // Define Wafermap settings
    // Wafermap type (increment to match #define values)
    ReportOptions.iWafermapType = pWafmap->iWafermapType + 1;

    switch (pSection->pWafmap->iWafermapType + 1)
    {
    default:
        pSection->pWafmap->iWafermapType = GEX_WAFMAP_STACK_SOFTBIN - 1;

    case GEX_WAFMAP_STACK_SOFTBIN:
    case GEX_WAFMAP_SOFTBIN:  // Standard Wafer map: Software Binning
    case GEX_WAFMAP_ZONAL_SOFTBIN:
        if (ReportOptions.pGexWafermapRangeList)
        {
            delete ReportOptions.pGexWafermapRangeList;
        }
        ReportOptions.pGexWafermapRangeList =
            createTestsRange(QString::number(GEX_TESTNBR_OFFSET_EXT_SBIN),
                             true, false);

        // Load Wafermap array with relevant Binning data
        FillWaferMap(pGroup, pFile, NULL, m_pReportOptions->iWafermapType);

        // Create Soft-Binning wafermap
        CreatePages_Wafermap(&qtStatisticsList);
        break;

    case GEX_WAFMAP_HARDBIN:  // Standard Wafer map: Hardware Binning
    case GEX_WAFMAP_STACK_HARDBIN:
    case GEX_WAFMAP_ZONAL_HARDBIN:
        if (ReportOptions.pGexWafermapRangeList)
        {
            delete ReportOptions.pGexWafermapRangeList;
        }
        ReportOptions.pGexWafermapRangeList =
            createTestsRange(QString::number(GEX_TESTNBR_OFFSET_EXT_HBIN),
                             true, false);

        // Load Wafermap array with relevant Binning data
        FillWaferMap(pGroup, pFile, NULL, m_pReportOptions->iWafermapType);

        // Create Hard-Binning wafermap
        CreatePages_Wafermap(&qtStatisticsList);
        break;

    case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
    case GEX_WAFMAP_STACK_TESTOVERLIMITS:
    case GEX_WAFMAP_TESTOVERDATA:  // zoning on test values range
    case GEX_WAFMAP_STACK_TESTOVERDATA:
    case GEX_WAFMAP_TEST_PASSFAIL:  // zoning on test values range
    case GEX_WAFMAP_STACK_TEST_PASSFAIL:
        // Create all parametric wafermaps requested
        if (ReportOptions.pGexWafermapRangeList)
        {
            delete ReportOptions.pGexWafermapRangeList;
        }
        ReportOptions.pGexWafermapRangeList =
            createTestsRange(pWafmap->strRangeList, true, false);

        QString strImageName;
        CTest* ptTestCell = pGroup->cMergedData.ptMergedTestList;  // Test list

        // Fill the sorting-list object with Statistics aggregate table
        while (ptTestCell != NULL)
        {
            // Check if test in list to plot
            if (ReportOptions.pGexWafermapRangeList->
                IsTestInList(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex))
            {
                // Valid entry, add it to the list
                qtStatisticsList.append(ptTestCell);
            }

            // Build Image name to use
            // image to create: 'grt_waf_XXXX_name.png'
            // strImageName = BuildImageUniqueName("grt_waf",ptTestCell,1);

            // Move to next test
            ptTestCell = ptTestCell->GetNextTest();
        }

        // Have the list sorted
        qSort(qtStatisticsList.begin(),
              qtStatisticsList.end(),
              SortFieldFuntion(qtStatisticsList.getSortField()));

        // Create wafermap
        CreatePages_Wafermap(&qtStatisticsList, strImageName);

        break;
    }

    return 1;
}

/******************************************************************************!
 * \fn
 * \brief Write Wafer map report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Datalog(
    GS::Gex::ReportTemplateSection* pSection,
    FILE*  /*hReportFile*/)
{
    // Check BoxPlot to be listed in report
    GS::Gex::CGexCustomReport_Datalog_Section* pDatalog = pSection->pDatalog;

    if (pDatalog->iDatalogType < 0)
    {
        return 1;
    }

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Make sure to overload Histogram settings
    // (increment to adjust line#1 comment offset)
    m_pReportOptions->setAdvancedReportSettings(pDatalog->iDatalogType);

    qtTestListStatistics qtStatisticsList;

    // Note: iTestType needs to be offset by one to compensate
    // first combo-selection that is removed for MyReports (xx_DISABLED)
    switch (pDatalog->iDatalogType)
    {
    case GEX_ADV_DATALOG_LIST:  // test or test range
    case GEX_ADV_DATALOG_RAWDATA:  // test or test range
        if (m_pReportOptions->pGexAdvancedRangeList)
        {
            delete m_pReportOptions->pGexAdvancedRangeList;
        }
        m_pReportOptions->pGexAdvancedRangeList =
            createTestsRange(pDatalog->strTestList, true, false);
        break;
    }

    // Move back to first group
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CTest* ptTestCell = pGroup->cMergedData.ptMergedTestList;

    // Create aggregate report for each test#
    while (ptTestCell != NULL)
    {
        switch (pDatalog->iDatalogType)
        {
        case GEX_ADV_DATALOG_LIST:  // test or test range
        case GEX_ADV_DATALOG_RAWDATA:  // test or test range
            // Valid entry, add it to the list
            if (m_pReportOptions->pGexAdvancedRangeList->
                IsTestInList(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex))
            {
                qtStatisticsList.append(ptTestCell);
            }
            break;

        default:
            qtStatisticsList.append(ptTestCell);
            break;
        }

        // Next Test
        ptTestCell = ptTestCell->GetNextTest();
    }  // Loop until all test cells read

    CreatePages_Datalog(&qtStatisticsList);

    if (m_pReportOptions->pGexAdvancedRangeList)
    {
        delete m_pReportOptions->pGexAdvancedRangeList;
        m_pReportOptions->pGexAdvancedRangeList = NULL;
    }

    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_Binning
 * \brief Write Binning report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Binning(
    GS::Gex::ReportTemplateSection*  /*pSection*/,
    FILE*  /*hReportFile*/)
{
    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Write Binning Summary section
    CreatePages_Binning();

    return 1;
}

/******************************************************************************!
 * \fn
 * \brief Write Pareto report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Pareto(
    GS::Gex::ReportTemplateSection*  /*pSection*/,
    FILE*  /*hReportFile*/)
{
    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Write pareto section
    CreatePages_Pareto();

    return 1;
}

/******************************************************************************!
 * \fn
 * \brief Write Pearson report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_Pearson(
    GS::Gex::ReportTemplateSection*  /*pSection*/,
    FILE* hReportFile)
{
    Q_UNUSED(hReportFile);

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Build Pearson's report
    QString r = WriteAdvPersonReport(m_pReportOptions);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" %1").arg(r).toLatin1().constData());
    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_TesterCorrelationGB
 * \brief Write Tester-to-tester Correlation & Guardband report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_TesterCorrelationGB(
    GS::Gex::ReportTemplateSection* pSection,
    FILE* hReportFile)
{
    GS::Gex::CGexCustomReport_TesterCorrelationGB_Section*
    pTesterCorrelationGB = pSection->pTesterCorrelationGB;

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Enable/disable some features
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        fprintf(hReportFile, "%s", m.toLatin1().data());
        return 1;
    }

    // Check that datasets are loaded
    QString strString;
    QString strLinkText;
    QString strBookmark;
    int iIndex;
    CGexGroupOfFiles* pGroup =
        (getGroupsList().isEmpty()) ? NULL :
        (getGroupsList().first());
    CGexFileInGroup* pFile;
    if (pGroup == NULL || getGroupsList().count() < 2)
    {
        // Error. No dataset selected yet
        fprintf(hReportFile, "You must have at least two datasets selected!\n");
        return 1;
    }

    // We must always have an even number of groups:
    // one half per tester / loadboard
    if (getGroupsList().count() % 2)
    {
        // Error. No dataset selected yet
        fprintf(hReportFile, "You must always have a even number of groups!\n");
        return 1;
    }

    // Get user select the Released Guard band file
    QString strGuardBandfile = pTesterCorrelationGB->strGuardBandFile;

    // Load guard band info
    QFile file(strGuardBandfile);  // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        fprintf(hReportFile,
                "Failed to open Guard band file: %s!\n",
                strGuardBandfile.toLatin1().constData());
        return 1;  // Failed opening file
    }

    // Read config File
    QString strSection;
    int iLineIndex = 1;
    bool bFlag;
    int iTestNumber;
    double lfGuardBandReleased;
    QMap <int, double> cMapTestGuardBand;
    QTextStream hData(&file);  // Assign file handle to data stream
    do
    {
        // Read one line from file
        strString = hData.readLine();
        strString = strString.trimmed();

        // Only process lines with valid data
        if ((strString.isEmpty() == false) &&
            (strString.startsWith("#", Qt::CaseInsensitive) == false))
        {
            // Line format: <text#>,<Test name>,<guard band released>
            strSection = strString.section(',', 0, 0);
            iTestNumber = strSection.toInt(&bFlag);
            if (bFlag == false)
            {
                fprintf(hReportFile,
                        "Line %d: Failed parsing Test#\n",
                        iLineIndex);
                return 1;  // Failed parsing file
            }

            strSection = strString.section(',', 2, 2);
            lfGuardBandReleased = strSection.toDouble(&bFlag);
            if (bFlag == false)
            {
                fprintf(hReportFile,
                        "Line %d: Failed parsing Guard Band value\n",
                        iLineIndex);
                return 1;  // Failed parsing file
            }

            // Save guard band value associated with this test#
            cMapTestGuardBand[iTestNumber] = lfGuardBandReleased;
        }

        // Keep track of line#
        iLineIndex++;
    }
    while (hData.atEnd() == false);
    file.close();

    // Compute number of units tested per tester
    int iUnits = getGroupsList().count() / 2;

    // Extract the two tester/loadboard names to compare
    pGroup = (getGroupsList().isEmpty()) ? NULL : getGroupsList().at(0);
    QString strTester1 = pGroup->strGroupName.section('-', -1).trimmed();
    pGroup = getGroupsList().at(iUnits);
    QString strTester2 = pGroup->strGroupName.section('-', -1).trimmed();

    // Table of contents (hyperlinks to template sections)
    fprintf(hReportFile,
            "<font color=\"#000000\" size=\"%d\"><br>"
            "<b>Direct links to:</b><br></font>\n",
            iHthmNormalFontSize);
    fprintf(hReportFile, "<blockquote>\n");
    fprintf(hReportFile, "<p align=\"left\">\n");
    for (iIndex = 0; iIndex < iUnits; iIndex++)
    {
        // Get pointer to each group
        pGroup = getGroupsList().at(iIndex);

        // Extract Unit name from group name
        strLinkText = pGroup->strGroupName.section('-', 0, 0).trimmed();

        // Write hyperlink
        strBookmark = "unit_" + QString::number(iIndex);
        fprintf(hReportFile,
                "<a href=\"#%s\">%s</a><br>\n",
                strBookmark.toLatin1().constData(),
                strLinkText.toLatin1().constData());
    }

    // Close table of contents
    fprintf(hReportFile, "</blockquote>\n<br>");

    // Write Tables for each DUT
    CTest* ptTestCellTester1;
    CTest* ptTestCellTester2;
    double lfValue, lfValue1, lfValue2, lfDeltaGB;
    bool bTester1Pass, bTester2Pass;
    for (iIndex = 0; iIndex < iUnits; iIndex++)
    {
        // Get pointer to each group
        pGroup = getGroupsList().at(iIndex);

        // Extract Unit name from group name
        strLinkText = pGroup->strGroupName.section('-', 0, 0).trimmed();

        // Section title (& bookmark)
        strBookmark = "unit_" + QString::number(iIndex);
        fprintf(hReportFile, "<a name=\"%s\"></a>",
                strBookmark.toLatin1().constData());
        fprintf(hReportFile, "<hr><h1 align=\"left\"><font color=\"#006699\">");
        fprintf(hReportFile,
                "%s: Correlation / Guardbands",
                strLinkText.toLatin1().constData());
        fprintf(hReportFile, "</h1><br>");

        // Write Statistics table
        fprintf(hReportFile, "<table border=\"0\">\n");

        // Write Labels
        fprintf(hReportFile, "<tr>\n");
        fprintf(hReportFile, "<td bgcolor=%s>Test#</td>", szFieldColor);
        fprintf(hReportFile, "<td bgcolor=%s>Name</td>", szFieldColor);
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Mean %s</td>",
                szFieldColor,
                strTester1.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s>Mean %s</td>",
                szFieldColor,
                strTester2.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Delta Mean<br>[x]</td>",
                szFieldColor);
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Released GB<br>[y]</td>",
                szFieldColor);
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">[x]/[y]</td>",
                szFieldColor);
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Std.Dev(Max)<br>%s</td>",
                szFieldColor,
                strTester1.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Std.Dev(Max)<br>%s</td>",
                szFieldColor,
                strTester2.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">GB<br>%s</td>",
                szFieldColor,
                strTester1.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">GB<br>%s</td>",
                szFieldColor,
                strTester2.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">Delta GB</td>",
                szFieldColor);
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">P/F<br>%s</td>",
                szFieldColor,
                strTester1.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">P/F<br>%s</td>",
                szFieldColor,
                strTester2.toLatin1().constData());
        fprintf(hReportFile,
                "<td bgcolor=%s align=\"center\">P/F</td>",
                szFieldColor);
        fprintf(hReportFile, "</tr>\n");

        // List of tests in this group
        pGroup = getGroupsList().at(iIndex);
        ptTestCellTester1 = pGroup->cMergedData.ptMergedTestList;
        pGroup =
            (getGroupsList().size() < (iIndex + iUnits + 1)) ? NULL :
            (getGroupsList().at(iIndex + iUnits));
        ptTestCellTester2 = pGroup->cMergedData.ptMergedTestList;
        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
            (pGroup->pFilesList.first());

        while (ptTestCellTester1 != NULL)
        {
            // If neither PTR values, this test was not executed
            if (ptTestCellTester1->ldExecs == 0)
            {
                goto next_cell;
            }

            // If a special parameter( die X,Y, Binning) ignore
            if (ptTestCellTester1->bTestType == '-')
            {
                goto next_cell;
            }

            // If not a parametric /
            // multiparametric (eg: functional) test, ignore
            if (ptTestCellTester1->bTestType == 'F')
            {
                goto next_cell;
            }

            // Get handle to same Test cell but in Tester2 group
            if (pFile->FindTestCell(ptTestCellTester1->lTestNumber,
                                    ptTestCellTester1->lPinmapIndex,
                                    &ptTestCellTester2, FALSE, FALSE,
                                    NULL) != 1)
            {
                goto next_cell;
            }

            // Only display tests that are present in the
            // 'GuardBand released' file
            if (cMapTestGuardBand.contains(ptTestCellTester1->lTestNumber) ==
                false)
            {
                goto next_cell;
            }

            // Start line
            fprintf(hReportFile, "<tr>\n");

            // Test #
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%d</td>",
                    szDataColor,
                    ptTestCellTester1->lTestNumber);

            // Test name
            fprintf(hReportFile,
                    "<td bgcolor=%s>%s</td>",
                    szDataColor,
                    ptTestCellTester1->strTestName.toLatin1().constData());

            // Mean (tester 1)
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1,
                                            ptTestCellTester1->lfMean,
                                            ptTestCellTester1->res_scal));

            // Mean (tester 2)
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester2,
                                            ptTestCellTester2->lfMean,
                                            ptTestCellTester2->res_scal));

            // [x] = Delta mean
            lfValue = fabs(
                    ptTestCellTester1->lfMean - ptTestCellTester2->lfMean);
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1, lfValue,
                                            ptTestCellTester1->res_scal));

            // [y] = Released GB
            lfValue1 = lfGuardBandReleased =
                    cMapTestGuardBand[ptTestCellTester1->lTestNumber];
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1, lfValue1,
                                            ptTestCellTester1->res_scal));

            // [x]/[y]
            if (lfValue1)
            {
                lfValue2 = lfValue / lfValue1;
                if (lfValue2 > 1)
                {
                    // Alarm: If [x]/[y] >1 Then the mean shift between
                    // platforms does not meet the guardband requirements
                    fprintf(
                        hReportFile,
                        "<td bgcolor=%s align=\"center\"><b>%.4lf</b></td>\n",
                        szAlarmColor,
                        lfValue2);
                }
                else
                {
                    fprintf(hReportFile,
                            "<td bgcolor=%s align=\"center\">%.4lf</td>\n",
                            szDataColor,
                            lfValue2);
                }
            }
            else
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\">n/a</td>\n",
                        szDataColor);
            }

            // Sigma (tester 1)
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1,
                                            ptTestCellTester1->lfSigma,
                                            ptTestCellTester1->res_scal));

            // Sigma (tester 2)
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester2,
                                            ptTestCellTester2->lfSigma,
                                            ptTestCellTester2->res_scal));

            // GB (tester 1) = 5.5 * Sigma_of_5040 + DeltaMean
            lfValue1 = (5.5 * ptTestCellTester1->lfSigma) + fabs(
                    ptTestCellTester1->lfMean - ptTestCellTester2->lfMean);
            if (lfValue1 > lfGuardBandReleased)
            {
                bTester1Pass = false;  // Fail correclation
            }
            else
            {
                bTester1Pass = true;  // Pass correclation
            }
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1, lfValue1,
                                            ptTestCellTester1->res_scal));

            // GB (tester 2)
            lfValue2 = (5.5 * ptTestCellTester2->lfSigma) +
                fabs(ptTestCellTester1->lfMean - ptTestCellTester2->lfMean);
            if (lfValue2 > lfGuardBandReleased)
            {
                bTester2Pass = false;  // Fail correclation
            }
            else
            {
                bTester2Pass = true;  // Pass correclation
            }
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester2, lfValue2,
                                            ptTestCellTester2->res_scal));

            // Delta GB
            lfDeltaGB = lfValue1 - lfValue2;
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\">%s</td>\n",
                    szDataColor,
                    pFile->FormatTestResult(ptTestCellTester1, lfDeltaGB,
                                            ptTestCellTester1->res_scal));

            // Pass/Fail (tester 1)
            if (bTester1Pass)
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\">PASS</td>\n",
                        szDataColor);
            }
            else
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\"><b>FAIL</b></td>\n",
                        szAlarmColor);
            }

            // Pass/Fail (tester 2)
            if (bTester2Pass)
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\">PASS</td>\n",
                        szDataColor);
            }
            else
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\"><b>FAIL</b></td>\n",
                        szAlarmColor);
            }

            // Overall Pass/Fail info
            if (lfDeltaGB > 0)
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\">PASS</td>\n",
                        szDataColor);
            }
            else
            {
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\"><b>FAIL</b></td>\n",
                        szAlarmColor);
            }

            // End of line
            fprintf(hReportFile, "</tr>\n");

            // Next cell
next_cell:;
            ptTestCellTester1 = ptTestCellTester1->GetNextTest();
        }

        // Close table
        fprintf(hReportFile, "</table>\n");

        // Write legend
        fprintf(hReportFile,
                "<p>If <font color=\"#FF0000\"><b>[x]/[y] > 1</b></font>: "
                "The mean shift between platforms does not meet "
                "the guardband requirements<br>\n");
        fprintf(hReportFile,
                "If 'P/F' field is <font color=\"#FF0000\"><b>FAIL</b></font>: "
                "Means the GB of the new platform is wider than "
                "the GB of the old platform</p>\n");

        // Write page break
        WritePageBreak(hReportFile);
    }

    fclose(hReportFile);
    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_Production
 * \brief Write Production report section
 ******************************************************************************/
int CGexReport:: WriteSection_Template_Production(
    GS::Gex::ReportTemplateSection* pSection,
    FILE*  /*hReportFile*/)
{
    GS::Gex::CGexCustomReport_Production_Section* pProduction =
        pSection->pProduction;

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Make sure to overload Production reports settings
    // (increment to adjust line#1 comment offset)
    // Production rendering mode (increment to match #define values)
    m_pReportOptions->setAdvancedReportSettings(pProduction->iChartingType + 1);

    // Build Production report
    WriteAdvProductionYield();
    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_GlobalInfo
 * \brief Write Global Info report section
 ******************************************************************************/
int CGexReport::WriteSection_Template_GlobalInfo(
    GS::Gex::ReportTemplateSection*  /*pSection*/,
    FILE*  /*hReportFile*/)
{
    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Build global info report
    int iStatus = CreatePages_Global();
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return 0;
    }
    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template_FileAudit
 * \brief Write File Audit section
 ******************************************************************************/
int CGexReport::WriteSection_Template_FileAudit(
    GS::Gex::ReportTemplateSection* pSection,
    FILE*  /*hReportFile*/)
{
    GS::Gex::CustomReportFileAuditSection* pFileAudit = pSection->pFileAudit;

    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Build File Audit report
    int iStatus = CreatePages_FileAudit(pFileAudit);
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return 0;
    }

    return 1;
}

/******************************************************************************!
 * \fn WriteSection_Template
 * \brief Write section + page break
 ******************************************************************************/
int
CGexReport::WriteSection_Template(GS::Gex::ReportTemplateSection* pSection,
                                  FILE*
                                  hReportFile)
{
    // Section dispatcher
    int iStatus = 1;
    switch (pSection->iSectionType)
    {
    case GEX_CRPT_WIDGET_AGGREGATE:
        iStatus = WriteSection_Template_Aggregate(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_WAFMAP:
        iStatus = WriteSection_Template_Wafmap(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_BINNING:
        iStatus = WriteSection_Template_Binning(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_PARETO:
        iStatus = WriteSection_Template_Pareto(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_PEARSON:
        iStatus = WriteSection_Template_Pearson(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_CORR_GB:
        iStatus = WriteSection_Template_TesterCorrelationGB(pSection,
                                                            hReportFile);
        break;
    case GEX_CRPT_WIDGET_PRODUCTION:
        iStatus = WriteSection_Template_Production(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_GLOBALINFO:
        iStatus = WriteSection_Template_GlobalInfo(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_FILE_AUDIT:
        iStatus = WriteSection_Template_FileAudit(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_ER:
        iStatus = WriteSection_Template_ER(pSection, hReportFile);
        break;
    case GEX_CRPT_WIDGET_DATALOG:
        iStatus = WriteSection_Template_Datalog(pSection, hReportFile);
        break;
    }

    // Write page break
    WritePageBreak(hReportFile);

    return iStatus;
}

/******************************************************************************!
 * \fn
 * \brief Write Enterprise Report section
 ******************************************************************************/
int
CGexReport::WriteSection_Template_ER(GS::Gex::ReportTemplateSection* pSection,
                                     FILE*  /*hReportFile*/)
{
    // Check for user 'Abort' signal
    if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
    {
        return 1;
    }

    // Build Enterprise specific report
    int iStatus = CreatePages_ER(pSection);
    if (iStatus != GS::StdLib::Stdf::NoError)
    {
        return 0;
    }
    return 1;
}
