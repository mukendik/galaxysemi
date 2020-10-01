#include <gqtl_sysutils.h>

#include "gexabstractdatalog.h"
#include "report_build.h"
#include "cpart_info.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "engine.h"

extern CGexReport*		gexReport;
extern CReportOptions	ReportOptions;

GexAbstractDatalog *	GexAbstractDatalog::m_pDatalog = NULL;

GexAbstractDatalog::GexAbstractDatalog(bool bValidSection, QString strOutputFormat)
    : m_bValidSection(bValidSection), m_strOutputFormat(strOutputFormat), m_uiTestsInDatalog(0), m_eFields(fieldNone), m_pEngine(NULL)
{
    QStringList listFieldStandard = ReportOptions.GetOption("adv_datalog", "field").toString().split("|");
    QStringList listFieldAdvanced = ReportOptions.GetOption("adv_datalog", "advanced_field").toString().split("|");

    if (listFieldStandard.contains("comment"))
        m_eFields |= fieldComments;
    if (listFieldStandard.contains("test_number"))
        m_eFields |= fieldTestNumber;
    if (listFieldStandard.contains("test_name"))
        m_eFields |= fieldTestName;
    if (listFieldStandard.contains("limits"))
        m_eFields |= fieldTestLimits;
    if (listFieldStandard.contains("die_loc"))
        m_eFields |= fieldDieXY;
    if (listFieldAdvanced.contains("site"))
        m_eFields |= fieldSite;
    if (listFieldAdvanced.contains("test_time"))
        m_eFields |= fieldTestTime;
    if (listFieldAdvanced.contains("pattern"))
        m_eFields |= fieldPattern;
    if (listFieldAdvanced.contains("cycle_count"))
        m_eFields |= fieldCycleCount;
    if (listFieldAdvanced.contains("rel_vector_addr"))
        m_eFields |= fieldRelVAddr;
    if (listFieldAdvanced.contains("pins_failed_count"))
        m_eFields |= fieldPinsFailed;
    if (listFieldAdvanced.contains("pin_nb"))
        m_eFields |= fieldPinNumber;
    if (listFieldAdvanced.contains("pin_name"))
        m_eFields |= fieldPinName;
}

GexAbstractDatalog::~GexAbstractDatalog()
{
    if (m_pEngine)
    {
        delete m_pEngine;
        m_pEngine = NULL;
    }
}

bool GexAbstractDatalog::existInstance()
{
    return (m_pDatalog != NULL);
}

bool GexAbstractDatalog::createInstance(bool bValidSection)
{
    bool bRes = false;

    if (m_pDatalog == NULL)
    {
        QString strOutputFormat = ReportOptions.GetOption("output", "format").toString();

        try
        {
            if (strOutputFormat == "CSV")
                m_pDatalog = new GexCSVDatalog(bValidSection, strOutputFormat);
            else
                m_pDatalog = new GexHTMLDatalog(bValidSection, strOutputFormat);

            // Open advanced report file
            if (m_pDatalog && m_pDatalog->open())
            {
                m_pDatalog->writeGenericHeader();
                m_pDatalog->writeBeginDatalog();

                // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
                ReportOptions.lAdvancedHtmlPages = 1;	// incremented as we generate the HTML datalog pages...

                // Reset line counter...used to insert page breaks in Advanced HTML pages that get too long
                ReportOptions.lAdvancedHtmlLinesInPage=0;

                bRes = true;
            }
        }
        catch(const std::bad_alloc& exceptionAlloc)
        {
            GSLOG(SYSLOG_SEV_ERROR, exceptionAlloc.what());
        }
        catch(const QString& strException)
        {
            GSLOG(SYSLOG_SEV_ERROR, strException.toLatin1().constData());
        }
    }

    return bRes;
}

bool GexAbstractDatalog::releaseInstance()
{
    bool bRes = false;

    if (m_pDatalog)
    {
        if (gexReport && gexReport->advancedReportHandle())
        {
            try
            {
                m_pDatalog->writeEndDatalog();
                m_pDatalog->writeFooter();
                m_pDatalog->close();

                bRes = true;
            }
            catch(const QString& strException)
            {
                GSLOG(SYSLOG_SEV_ERROR, strException.toLatin1().constData());
            }
        }

        delete m_pDatalog;
        m_pDatalog = NULL;
    }

    return bRes;
}

void GexAbstractDatalog::writeGenericHeader()
{
    // Write header section
    writeHeader();

    if (m_bValidSection == true)
    {
        QString lMessage;

        // Open File info section
        writeBeginFileInfo();

        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *	pGroup = gexReport->getGroupsList().isEmpty()	? NULL : gexReport->getGroupsList().first();
        CGexFileInGroup *	pFile  = (pGroup->pFilesList.isEmpty())		? NULL : pGroup->pFilesList.first();

        if(ReportOptions.iFiles == 1)
        {
            gexReport->WriteInfoLine(gexReport->advancedReportHandle(), "Program",			pFile->getMirDatas().szJobName);
            gexReport->WriteInfoLine(gexReport->advancedReportHandle(), "Product",			pFile->getMirDatas().szPartType);
            gexReport->WriteInfoLine(gexReport->advancedReportHandle(), "Lot",				pFile->getMirDatas().szLot);
            gexReport->WriteInfoLine(gexReport->advancedReportHandle(), "Datalog date",	TimeStringUTC(pFile->getMirDatas().lStartT));
        }

        if(ReportOptions.getAdvancedReport() != GEX_ADV_DATALOG)
            lMessage = "Datalog report is disabled!";
        else
        {
            switch(ReportOptions.getAdvancedReportSettings())
            {
                case GEX_ADV_DATALOG_ALL:		// list tests
                default:
                    lMessage = "list all tests";
                    break;

                case GEX_ADV_DATALOG_FAIL:		// list FAIL tests only
                    lMessage = "Failing tests only";
                    break;

                case GEX_ADV_DATALOG_OUTLIER:
                    lMessage = ReportOptions.pGexAdvancedRangeList->BuildTestListString("Removed Data points for tests: ");
                    break;

                case GEX_ADV_DATALOG_LIST:		// datalog specific tests
                case GEX_ADV_DATALOG_RAWDATA:	// datalog specific tests : raw datalog
                    lMessage = ReportOptions.pGexAdvancedRangeList->BuildTestListString("list tests: ");
                    break;
            }
        }

        // Write datalog mode
        gexReport->WriteInfoLine(gexReport->advancedReportHandle(),
                                 "Datalog mode", lMessage.toLatin1().constData());

        // Close File information section
        writeEndFileInfo();
    }
}

bool GexAbstractDatalog::isDataloggable(CTest* ptTestCell, bool bFailResult, bool bIsOutlier)
{
    bool bIsDataloggable = true;

    switch(ReportOptions.getAdvancedReportSettings())
    {
        // ALL tests
        case GEX_ADV_DATALOG_ALL	:
        default						:	if(bIsOutlier == true)
                                            bIsDataloggable = false;
                                        break;					// Just continue in this function !

        // Only FAIL tests
        case GEX_ADV_DATALOG_FAIL	:	if((bFailResult == false) || (bIsOutlier == true))
                                            bIsDataloggable = false;				// Test is PASS: do not Datalog !
                                        break;

        // Swap Outlier flag so we can fall into next case.
        case GEX_ADV_DATALOG_OUTLIER:	if(bIsOutlier == false)
                                            bIsOutlier = true;
                                        else
                                            bIsOutlier = false;

        // Tests between given range
        // Tests between given range: raw data
        case GEX_ADV_DATALOG_LIST	:
        case GEX_ADV_DATALOG_RAWDATA:	if(bIsOutlier == true)
                                            bIsDataloggable = false;
                                        if(ReportOptions.pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex))
                                            break;
                                        else
                                            bIsDataloggable = false;	// Test out of limits ! or Test PinmapIndex# out of limit!
    }

    return bIsDataloggable;
}

bool GexAbstractDatalog::pushParametricResult(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID)
{
    if (m_pDatalog)
    {
        if (gexReport && gexReport->advancedReportHandle())
        {
            try
            {
                m_pDatalog->writeParametricDatalog(pFile, ptTestCell, pPartInfo, fValue, bFailResult, bAlarmResult, iSiteID);
                return true;
            }
            catch(const QString& strException)
            {
                GSLOG(SYSLOG_SEV_ERROR, strException.toLatin1().constData());
            }
        }
    }

    return false;
}

bool GexAbstractDatalog::pushFunctionalResult(CGexFileInGroup *pFile, CTest *ptTestCell, const CPartInfo *pPartInfo, float fValue, bool bFailResult, const QString &strVectorName, const GexVectorFailureInfo &failureInfo, int iSiteID)
{
    if (m_pDatalog)
    {
        if (gexReport && gexReport->advancedReportHandle())
        {
            try
            {
                m_pDatalog->writeFunctionalDatalog(pFile, ptTestCell, pPartInfo, fValue, bFailResult, strVectorName, failureInfo, iSiteID);
                return true;
            }
            catch(const QString& strException)
            {
                GSLOG(SYSLOG_SEV_ERROR, strException.toLatin1().constData());
            }
        }
    }

    return false;
}

bool GexAbstractDatalog::pushPartInfo(const CPartInfo * pPartInfo)
{
    if (m_pDatalog)
    {
        if (gexReport && gexReport->advancedReportHandle())
        {
            try
            {
                m_pDatalog->writeGenericPartFooter(pPartInfo);
                return true;
            }
            catch(const QString& strException)
            {
                GSLOG(SYSLOG_SEV_ERROR, strException.toLatin1().constData());
            }
        }
    }

    return false;
}

void GexAbstractDatalog::writeGenericPartFooter(const CPartInfo *pPartInfo)
{
    // Part footer
    writePartFooter(pPartInfo);

    // Reset test counter.
    m_uiTestsInDatalog = 0;
}

GexHTMLDatalog::GexHTMLDatalog(bool bValidSection, QString strOutputFormat)
    : GexAbstractDatalog(bValidSection, strOutputFormat)
{

}

bool GexHTMLDatalog::open()
{
    if(gexReport && gexReport->OpenFile_Advanced() == GS::StdLib::Stdf::NoError)
        return true;

    return false;
}

void GexHTMLDatalog::close()
{
    if(m_strOutputFormat == "HTML" && gexReport)
        gexReport->CloseReportFile(gexReport->advancedReportHandle());
}

void GexHTMLDatalog::writeHeader()
{
    gexReport->WriteHeaderHTML(gexReport->advancedReportHandle(),"#000080");	// Default: Text is Dark Blue

    // Title + bookmark
    gexReport->WriteHtmlSectionTitle(gexReport->advancedReportHandle(),"all_advanced","More Reports: Datalog");
}

void GexHTMLDatalog::writeBeginFileInfo()
{
    fprintf(gexReport->advancedReportHandle(),"<p align=\"left\">&nbsp;</p>\n");
    fprintf(gexReport->advancedReportHandle(),"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Datalog data available !<br>\n", gexReport->iHthmNormalFontSize);
    fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
}

void GexHTMLDatalog::writeEndFileInfo()
{
    // Wrting HTML file
    if((m_strOutputFormat=="DOC") ||(m_strOutputFormat=="PDF") || (m_strOutputFormat=="PPT") || (m_strOutputFormat=="ODT"))
    {
        // Flat HTML file (for Word, PDF generation): simply close the table, but do not close the file.
        fprintf(gexReport->advancedReportHandle(),"</table>\n");
    }
    else if(m_strOutputFormat == "HTML")
    {
        // add link to advanced1.htm
        QByteArray lFileData;
        QString lFilePath(ReportOptions.strReportDirectory + "/index.htm");
        QFile lFile(lFilePath);
        if (lFile.exists() && lFile.open(QIODevice::ReadWrite))
        {
            lFileData = lFile.readAll();
            QString lText(lFileData);
            lText.replace(QString("href=\"pages/advanced.htm\">skip"), QString("href=\"pages/advanced1.htm\">skip"));
            lFile.seek(0);
            lFile.write(lText.toLatin1());
            lFile.close();
        }

        // Standard multi-pages HTML file
        fprintf(gexReport->advancedReportHandle(),"<tr>\n");
        fprintf(gexReport->advancedReportHandle(),"<td bgcolor=%s>Link to pages:</td>", szFieldColor);
        fprintf(gexReport->advancedReportHandle(),"<td bgcolor=%s><b><a href=\"advanced1.htm\">See Datalog pages</a> </b></td>", szDataColor);
        fprintf(gexReport->advancedReportHandle(),"</tr>\n");
        fprintf(gexReport->advancedReportHandle(),"</table>\n");

        // Close Datalog HTML file.
        writeFooter();
        close();
    }
}

void GexHTMLDatalog::writeParametricDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID)
{
    QString strTestResult = writeCommonTestInfo(pFile, ptTestCell, pPartInfo, bFailResult);

    strTestResult += "</td>\n";
    strTestResult += writeCommonTestResult(pFile, ptTestCell, fValue, bFailResult, bAlarmResult);

    writeTestResult(pFile, strTestResult, iSiteID);
}

QString GexHTMLDatalog::writeCommonTestInfo(CGexFileInGroup *pFile, CTest *ptTestCell, const CPartInfo *pPartInfo, bool bFailResult)
{
    Q_UNUSED(pPartInfo);

    // Bookmark: are in same page if FLAT HTML page is generated
    QString	strBookmark;

    if(m_strOutputFormat == "HTML")
        strBookmark.sprintf("stats%d.htm#StatT", ptTestCell->iHtmlStatsPage);
    else
        strBookmark = "#StatT";	// Test Statistics bookmark header string.

    QString strTestInfoCell;

    // Write test info cell.
    if(m_uiTestsInDatalog % 2 == 0)
        strTestInfoCell = "<td width=\"10%%\"></td>\n";
    else
        strTestInfoCell = "<td width=\"5%%\"></td>\n";


    // Add TEST_NAME
    if (m_eFields & fieldTestNumber)
    {
        gexReport->BuildTestNumberString(ptTestCell);

        if((bFailResult == true) && (ptTestCell->iHtmlStatsPage > 0))
        {
            strTestInfoCell += "<td width=\"25%\"><b><a name=\"T";
            strTestInfoCell += ptTestCell->szTestLabel;
            strTestInfoCell += "\"></a> <a href=\"";
            strTestInfoCell += strBookmark;
            strTestInfoCell += ptTestCell->szTestLabel;
            strTestInfoCell += "\">T";
            strTestInfoCell += ptTestCell->szTestLabel;
            strTestInfoCell += "</a>: ";
        }
        else
        {
            strTestInfoCell += "<td width=\"25%%\">T";
            strTestInfoCell += ptTestCell->szTestLabel;
            strTestInfoCell += ": ";
        }
    }
    else
        strTestInfoCell += "<td width=\"25%%\">";

    if (m_eFields & fieldTestName)
    {
        QString strTestName;
        gexReport->BuildTestNameString(pFile, ptTestCell, strTestName);
        strTestInfoCell += strTestName;
    }

    // Check if have to display limits
    if (m_eFields & fieldTestLimits)
    {
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            pFile->FormatTestLimit(ptTestCell,
                                   ptTestCell->GetCurrentLimitItem()->szLowL ,
                                   ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                   ptTestCell->llm_scal);

            strTestInfoCell += "<br>LowL: ";
            strTestInfoCell += ptTestCell->GetCurrentLimitItem()->szLowL;
        }

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            pFile->FormatTestLimit(ptTestCell,
                                   ptTestCell->GetCurrentLimitItem()->szHighL,
                                   ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                   ptTestCell->hlm_scal);

            strTestInfoCell += "<br>HighL: ";
            strTestInfoCell += ptTestCell->GetCurrentLimitItem()->szHighL;
        }
    }

    return strTestInfoCell;
}

QString GexHTMLDatalog::writeCommonTestResult(CGexFileInGroup *pFile, CTest *ptTestCell, float fValue, bool bFailResult, bool bAlarmResult)
{
    // Write test result.
    QString strTestValueCell = "<td width=\"15%%\" align=\"right\">";

    if(bFailResult == true || bAlarmResult == true)
        strTestValueCell += "<font color=\"#FF0000\"><b>";

    strTestValueCell += pFile->FormatTestResult(ptTestCell, fValue, ptTestCell->res_scal);

    if(bFailResult == true)
        strTestValueCell += " *F</b></font>";
    else if(bAlarmResult == true)
        strTestValueCell += " *A</b></font>";

    strTestValueCell +="</td>\n";

    return strTestValueCell;
}

QString GexHTMLDatalog::writeFunctionalTestInfo(const QString &strVectorName, const GexVectorFailureInfo &failureInfo)
{
    QString strFunctionalTestInfo;

    // add field PATTERN
    if (m_eFields & fieldPattern)
    {
        strFunctionalTestInfo = "<br>Vector module pattern name: ";
        strFunctionalTestInfo += strVectorName;
    }

    // add field CYCLE_COUNT
    if (m_eFields & fieldCycleCount)
    {
        strFunctionalTestInfo += "<br>Cycle count of vector: ";
        strFunctionalTestInfo += QString::number(failureInfo.vectorCycleCount());
    }

    // add field VADDR
    if (m_eFields & fieldRelVAddr)
    {
        strFunctionalTestInfo += "<br>Relative vector address: ";
        strFunctionalTestInfo += QString::number(failureInfo.relativeVectorAddress());
    }

    // add field FAIL_PINS
    if (m_eFields & fieldPinsFailed)
    {
        strFunctionalTestInfo += "<br>Failing pins (";
        strFunctionalTestInfo += QString::number(failureInfo.pinmapCount());
        strFunctionalTestInfo += "):";
    }

    // Add fields PIN_NB and PIN_NAME
    if ((m_eFields & fieldPinNumber) || (m_eFields & fieldPinName))
    {
        for(int nPin = 0; nPin < failureInfo.pinmapCount(); nPin++)
        {
            // add field PIN_NAME (PIN_NB)
            strFunctionalTestInfo += "<br>";

            if (m_eFields & fieldPinName)
                strFunctionalTestInfo += failureInfo.pinmapAt(nPin)->strLogicName;

            if (m_eFields & fieldPinNumber)
            {
                strFunctionalTestInfo += "(";
                strFunctionalTestInfo += QString::number(failureInfo.pinmapAt(nPin)->iPinmapIndex);
                strFunctionalTestInfo += ")";
            }
        }
    }

    return strFunctionalTestInfo;
}

void GexHTMLDatalog::writeFunctionalDatalog(CGexFileInGroup *pFile, CTest *ptTestCell, const CPartInfo *pPartInfo, float fValue, bool bFailResult, const QString &strVectorName, const GexVectorFailureInfo &failureInfo, int iSiteID)
{
    QString strTestResult = writeCommonTestInfo(pFile, ptTestCell, pPartInfo, bFailResult);

    // Add functional details only if test is fail
    if (bFailResult)
        strTestResult += writeFunctionalTestInfo(strVectorName, failureInfo);

    strTestResult += "</td>\n";
    strTestResult += writeCommonTestResult(pFile, ptTestCell, fValue, bFailResult, false);

    writeTestResult(pFile, strTestResult, iSiteID);
}

void GexHTMLDatalog::writeTestResult(CGexFileInGroup *pFile, const QString& strTestResult, int iSiteID)
{
    // Add page break if need be
    writePageBreak();

    // First test datalogged in part, and not RAW datalog.
    if(m_uiTestsInDatalog == 0)
    {
        writePartHeader(pFile, iSiteID);

        // Open table for data
        fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n");
    }

    // Test is in first column
    if(m_uiTestsInDatalog % 2 == 0)
        fprintf(gexReport->advancedReportHandle(), "<tr>\n");

    fprintf(gexReport->advancedReportHandle(), "%s", strTestResult.toLatin1().data());

    // Test is in second column
    if(m_uiTestsInDatalog % 2)
        fprintf(gexReport->advancedReportHandle(),"</tr>\n");

    // Update HTML line count.
    if(m_uiTestsInDatalog % 2)
        ReportOptions.lAdvancedHtmlLinesInPage++;

    // Keeps track of the number of tests datalogged in part
    m_uiTestsInDatalog++;
}

void GexHTMLDatalog::writeBeginDatalog()
{
    if(m_strOutputFormat == "HTML")
    {
        // Create first datalog page...
        QString strReportFileName = ReportOptions.strReportDirectory + "/pages/advanced1.htm";

        if (gexReport->OpenFile_Advanced(strReportFileName) == GS::StdLib::Stdf::NoError)
        {
            fprintf(gexReport->advancedReportHandle(),"<html>\n");
            fprintf(gexReport->advancedReportHandle(),"<head>\n");
            fprintf(gexReport->advancedReportHandle(),"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
            fprintf(gexReport->advancedReportHandle(),"</head>\n");

            // Sets default background color = white, text color given in argument.
            fprintf(gexReport->advancedReportHandle(),"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!
        }
        else
            throw(QString("Unable to open advanced report file (%1)").arg(strReportFileName));
    }

    // Keep track of total HTML pages written
    gexReport->lReportPageNumber++;

    // Title + bookmark
    gexReport->WriteHtmlSectionTitle(gexReport->advancedReportHandle(),"","Datalog");
}

void GexHTMLDatalog::writeEndDatalog()
{
    // In case No tests in the last datalog run...then fill table with only empty line (workaround QT bug)
    fprintf(gexReport->advancedReportHandle(),"<tr>\n<td> </td>\n</tr>\n");
    fprintf(gexReport->advancedReportHandle(),"</table>\n");

    if(ReportOptions.getAdvancedReport() != GEX_ADV_DATALOG)
        fprintf(gexReport->advancedReportHandle(),"<p align=\"left\"><font color=\"#000000\" size=\"%d\">Datalog report is disabled!<br>\n", gexReport->iHthmNormalFontSize);
    else
        fprintf(gexReport->advancedReportHandle(),"<p align=\"center\"> [----End Of Datalog----]<br>\n");
}

void GexHTMLDatalog::writeFooter()
{
    fprintf(gexReport->advancedReportHandle(), C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(gexReport->advancedReportHandle(),"</body>\n");
    fprintf(gexReport->advancedReportHandle(),"</html>\n");
}

void GexHTMLDatalog::writePartHeader(CGexFileInGroup * pFile, int iSiteID)
{
    // Open table for part header
    fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n");

    fprintf(gexReport->advancedReportHandle(),"<tr>\n");
    fprintf(gexReport->advancedReportHandle(),"<td width=\"10%%\"><b>Datalog#:</b></td>\n");
    fprintf(gexReport->advancedReportHandle(),"<td width=\"5%%\"><b>%d</b></td>\n", pFile->PartProcessed.partNumber(iSiteID));

    if(*(pFile->getMirDatas().szPartType))
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\"><b>Product: %s</b></td>\n", pFile->getMirDatas().szPartType);
    else
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\"></td>\n");

    if(*(pFile->getMirDatas().szLot))
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\"><b>Lot: %s</b></td>\n", pFile->getMirDatas().szLot);
    else
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\"></td>\n");

    if(*(pFile->getMirDatas().szSubLot))
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\"><b>Sublot: %s</b></td>\n", pFile->getMirDatas().szSubLot);
    else
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\"></td>\n");

    if(*(pFile->getWaferMapData().szWaferID))
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\"><b>Wafer:</b> %s</td>\n", pFile->getWaferMapData().szWaferID);
    else
        fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\"></td>\n");

    fprintf(gexReport->advancedReportHandle(),"<td width=\"5%%\" align=\"left\"></td>\n");
    fprintf(gexReport->advancedReportHandle(),"</tr>\n");
    fprintf(gexReport->advancedReportHandle(),"</table>\n");

    // Next test result to datalog will start on a even column.
    m_uiTestsInDatalog	+= 2;
    ReportOptions.lAdvancedHtmlLinesInPage++;	// Update HTML line count.
}

void GexHTMLDatalog::writePageBreak()
{
    // Add page break only for html output format
    // Check if we exceed the allowed lines# per page...if so change to new page
    if(m_strOutputFormat == "HTML" && ReportOptions.lAdvancedHtmlLinesInPage >= MAX_DATALOG_PERPAGE)
    {
        QString	strTempString;
        long	iCurrentHtmlPage = ReportOptions.lAdvancedHtmlPages;

        // Close current page, open next page.
        if(m_uiTestsInDatalog != 0)
            fprintf(gexReport->advancedReportHandle(), "</table>\n");

        fprintf(gexReport->advancedReportHandle(), "<br><br><br><table border=\"0\" width=\"98%%\">\n");
        fprintf(gexReport->advancedReportHandle(), "<tr>\n");

        if(iCurrentHtmlPage == 1)
            strTempString = "advanced";
        else
            strTempString = QString("advanced%1").arg(iCurrentHtmlPage);

        fprintf(gexReport->advancedReportHandle(),"<td width=\"50%%\"><a href=\"%s.htm\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n", strTempString.toLatin1().data());

        strTempString = QString("advanced%1").arg(iCurrentHtmlPage+1);
        fprintf(gexReport->advancedReportHandle(),"<td width=\"50%%\"><p align=\"right\"><a href=\"%s.htm\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"30\"></a></td>\n", strTempString.toLatin1().data());
        fprintf(gexReport->advancedReportHandle(),"</tr>\n</table>\n");

        // Close page
        writeFooter();
        close();

        // Remove next page...it is the only way to avoid 'Print' include dummy pages!
        strTempString = QString("%1/pages/advanced%2.htm").arg(ReportOptions.strReportDirectory).arg(iCurrentHtmlPage+2);
        remove(strTempString.toLatin1().data());

        // Open <stdf-filename>/report/<szPageName>.htm
        strTempString = QString("%1/pages/advanced%2.htm").arg(ReportOptions.strReportDirectory).arg(iCurrentHtmlPage+1);

        if (gexReport->OpenFile_Advanced(strTempString) != GS::StdLib::Stdf::NoError)
            throw(QString("Unable to open advanced report file (%1)").arg(strTempString));

        fprintf(gexReport->advancedReportHandle(),"<html>\n");
        fprintf(gexReport->advancedReportHandle(),"<head>\n");
        fprintf(gexReport->advancedReportHandle(),"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
        fprintf(gexReport->advancedReportHandle(),"</head>\n");

        // Sets default background color = white, text color given in argument.
        fprintf(gexReport->advancedReportHandle(),"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!

        fprintf(gexReport->advancedReportHandle(),"<h1 align=\"left\"><font color=\"#006699\">Datalog</font></h1><br>\n");
        fprintf(gexReport->advancedReportHandle(),"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br>\n");
        fprintf(gexReport->advancedReportHandle(),"</table>\n");
        fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"98%%\">\n");
        fprintf(gexReport->advancedReportHandle(),"<tr>\n");

        if(iCurrentHtmlPage <= 1)
            strTempString = "advanced";
        else
            strTempString = QString("advanced%1").arg(iCurrentHtmlPage);

        fprintf(gexReport->advancedReportHandle(),"<td width=\"33%%\"><a href=\"%s.htm\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n", strTempString.toLatin1().data());
        fprintf(gexReport->advancedReportHandle(),"<td></td>\n");

        // Middle table cell (empty in header)
        fprintf(gexReport->advancedReportHandle(),"<td width=\"33%%\">&nbsp;</td>\n");

        // 'next' button
        strTempString = QString("advanced%2").arg(iCurrentHtmlPage);
        fprintf(gexReport->advancedReportHandle(),"<td width=\"33%%\"><p align=\"right\"><a href=\"%s.htm\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"30\"></a></td>\n", strTempString.toLatin1().data());
        fprintf(gexReport->advancedReportHandle(),"</tr>\n</table>\n");

        // Open new data table
        if(m_uiTestsInDatalog != 0)
            fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

        // Reset counter
        ReportOptions.lAdvancedHtmlLinesInPage = 0;
        ReportOptions.lAdvancedHtmlPages++;
    }
}

void GexHTMLDatalog::writePartFooter(const CPartInfo *pPartInfo)
{
    // Test is in second column
    if(m_uiTestsInDatalog % 2)
    {
        fprintf(gexReport->advancedReportHandle(), "<td colspan=\"3\" width=\"50%%\"></td>\n");
        fprintf(gexReport->advancedReportHandle(), "</tr>\n");
    }

    // Close data table
    if(m_uiTestsInDatalog != 0)
        fprintf(gexReport->advancedReportHandle(),"</table>\n");

    //FIXME: not used ?
    /*int iBin;
    if (pPartInfo->iSoftBin != 65535)
        iBin = pPartInfo->iSoftBin;
    else
        iBin = pPartInfo->iHardBin;*/

    // If not RAW datalog!
    fprintf(gexReport->advancedReportHandle(),"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
    fprintf(gexReport->advancedReportHandle(),"<tr>\n");
    fprintf(gexReport->advancedReportHandle(),"<td width=\"10%%\"></td>\n");

    if(pPartInfo->bPass)
    {
        fprintf(gexReport->advancedReportHandle(),"<td width=\"25%%\"><b>");

        if(pPartInfo->iSoftBin != 65535)
            fprintf(gexReport->advancedReportHandle(),"SBIN %d ", pPartInfo->iSoftBin);

        fprintf(gexReport->advancedReportHandle(),"HBIN %d - PASSED</b></td>\n", pPartInfo->iHardBin);
    }
    else
    {
        fprintf(gexReport->advancedReportHandle(),"<td width=\"25%%\"><font color=\"#FF0000\"><b>");

        if(pPartInfo->iSoftBin != 65535)
            fprintf(gexReport->advancedReportHandle(),"SBIN %d ", pPartInfo->iSoftBin);

        fprintf(gexReport->advancedReportHandle(),"HBIN %d *FAILED*</b></font></td>\n", pPartInfo->iHardBin);
    }

    // Die location
    fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\">");
    if (m_eFields & fieldDieXY)
        fprintf(gexReport->advancedReportHandle(),"<b>DieX=%d, DieY=%d</b>  ",pPartInfo->iDieX, pPartInfo->iDieY);
    fprintf(gexReport->advancedReportHandle(),"</td>\n");

    // PartID
    fprintf(gexReport->advancedReportHandle(),"<td width=\"10%%\" align=\"left\">");
    fprintf(gexReport->advancedReportHandle(),"(PartID: %s)",((CPartInfo *)pPartInfo)->getPartID().toLatin1().constData());
    fprintf(gexReport->advancedReportHandle(),"</td>\n");

    // Site
    fprintf(gexReport->advancedReportHandle(),"<td width=\"10%%\" align=\"left\">");
    if (m_eFields & fieldSite)
        fprintf(gexReport->advancedReportHandle()," Site: %d", pPartInfo->m_site);
    fprintf(gexReport->advancedReportHandle(),"</td>\n");

    // Testing time
    fprintf(gexReport->advancedReportHandle(),"<td width=\"20%%\" align=\"left\">");
    if (m_eFields & fieldTestTime)
        fprintf(gexReport->advancedReportHandle()," Test Time: %.3g sec.",(float)pPartInfo->lExecutionTime/1000.0);
    fprintf(gexReport->advancedReportHandle(),"</td>\n");

    fprintf(gexReport->advancedReportHandle(),"<td width=\"5%%\"></td>\n");
    fprintf(gexReport->advancedReportHandle(),"</tr>\n");

    fprintf(gexReport->advancedReportHandle(),"</table>\n<br>\n<br>");
}

GexCSVDatalog::GexCSVDatalog(bool bValidSection, QString strOutputFormat)
    : GexAbstractDatalog(bValidSection, strOutputFormat)
{

}

bool GexCSVDatalog::open()
{
    return true;
}

void GexCSVDatalog::close()
{
    // Do nothing for CSV output
}

void GexCSVDatalog::writeHeader()
{
    // Generating .CSV report file.
    fprintf(gexReport->advancedReportHandle(), "\n---- DATALOG report ----\n");

    if(m_bValidSection == false)
        fprintf(gexReport->advancedReportHandle(), "No Datalog data available !\n");
}

void GexCSVDatalog::writeFooter()
{
    // Do nothing for CSV output
}

void GexCSVDatalog::writeBeginDatalog()
{
    // add field PRODUCT_ID
    fprintf(gexReport->advancedReportHandle(),"PRODUCT_ID,");

    // add field LOT_ID
    fprintf(gexReport->advancedReportHandle(),"LOT_ID,");

    // add field SUBLOT_ID
    fprintf(gexReport->advancedReportHandle(),"SUBLOT_ID,");

    // add field WAFER_ID
    fprintf(gexReport->advancedReportHandle(),"WAFER_ID,");

    // add field START_TIME
    fprintf(gexReport->advancedReportHandle(),"START_TIME,");

    // add field PARTS_TESTED
    fprintf(gexReport->advancedReportHandle(),"PARTS_TESTED,");

    // add field PARTS_PASSED
    fprintf(gexReport->advancedReportHandle(),"PARTS_PASSED,");

    // add field PART_ID
    fprintf(gexReport->advancedReportHandle(),"PART_ID,");

    // add field DIE_X and DIE_Y
    if (m_eFields & fieldDieXY)
    {
        fprintf(gexReport->advancedReportHandle(),"DIE_X,");
        fprintf(gexReport->advancedReportHandle(),"DIE_Y,");
    }

    // add field SITE
    if (m_eFields & fieldSite)
        fprintf(gexReport->advancedReportHandle(),"SITE,");

    // add field HBIN
    fprintf(gexReport->advancedReportHandle(),"HBIN,");

    // add field SBIN
    fprintf(gexReport->advancedReportHandle(),"SBIN,");

    // add field TEST_TIME
    if (m_eFields & fieldTestTime)
        fprintf(gexReport->advancedReportHandle(),"TEST_TIME,");

    // add field TNUM
    if (m_eFields & fieldTestNumber)
        fprintf(gexReport->advancedReportHandle(),"TNUM,");

    // add field TNAME
    if (m_eFields & fieldTestName)
        fprintf(gexReport->advancedReportHandle(),"TNAME,");

    // add field TTYPE
    fprintf(gexReport->advancedReportHandle(),"TTYPE,");

    // add field UNITS
    fprintf(gexReport->advancedReportHandle(),"UNITS,");

    // add field LL and HL
    if (m_eFields & fieldTestLimits)
    {
        fprintf(gexReport->advancedReportHandle(),"LL,");
        fprintf(gexReport->advancedReportHandle(),"HL,");
    }

    // add field VALUE
    fprintf(gexReport->advancedReportHandle(),"VALUE,");

    // add field STATUS
    fprintf(gexReport->advancedReportHandle(),"STATUS,");

    // add field PATTERN
    if (m_eFields & fieldPattern)
        fprintf(gexReport->advancedReportHandle(),"PATTERN,");

    // add field CYCLE_COUNT
    if (m_eFields & fieldCycleCount)
        fprintf(gexReport->advancedReportHandle(),"CYCLE_COUNT,");

    // add field VADDR
    if (m_eFields & fieldSite)
        fprintf(gexReport->advancedReportHandle(),"VADDR,");

    // add field FAIL_PINS
    if (m_eFields & fieldPinsFailed)
        fprintf(gexReport->advancedReportHandle(),"FAIL_PINS,");

    // add field PIN_NB
    if (m_eFields & fieldPinNumber)
        fprintf(gexReport->advancedReportHandle(),"PIN_NB,");

    // add field PIN_NAME
    if (m_eFields & fieldPinName)
        fprintf(gexReport->advancedReportHandle(),"PIN_NAME");

    fprintf(gexReport->advancedReportHandle(),"\n");
}

void GexCSVDatalog::writeCommonFields(CGexFileInGroup *pFile, CTest *ptTestCell, const CPartInfo *pPartInfo, float fValue, bool bFailResult, bool bAlarmResult)
{
    // add field PRODUCT_ID
    fprintf(gexReport->advancedReportHandle(), "%s,", pFile->getMirDatas().szPartType);

    // add field LOT_ID
    fprintf(gexReport->advancedReportHandle(), "%s,", pFile->getMirDatas().szLot);

    // add field SUBLOT_ID
    fprintf(gexReport->advancedReportHandle() ,"%s,", pFile->getMirDatas().szSubLot);

    // add field WAFER_ID
    fprintf(gexReport->advancedReportHandle() ,"%s,", pFile->getWaferMapData().szWaferID);

    // add field START_TIME
    QDateTime clDateTime;
    clDateTime.setTimeSpec(Qt::UTC);
    clDateTime.setTime_t(pFile->getMirDatas().lStartT);
    fprintf(gexReport->advancedReportHandle(), "%s,", clDateTime.toString("ddd MMM dd hh:mm:ss yyyy").toLatin1().data());

    // add field PARTS_TESTED
    if (pFile->getPcrDatas().lPartCount >= 0)
        fprintf(gexReport->advancedReportHandle(), "%ld,",pFile->getPcrDatas().lPartCount);
    else
        fprintf(gexReport->advancedReportHandle(), "n/a,");

    // add field PARTS_PASSED
    if (pFile->getPcrDatas().lGoodCount >= 0)
        fprintf(gexReport->advancedReportHandle(), "%d,", pFile->getPcrDatas().lGoodCount);
    else
        fprintf(gexReport->advancedReportHandle(), "n/a,");

    // add field PART_ID
    fprintf(gexReport->advancedReportHandle(), "%s,", ((CPartInfo *)pPartInfo)->getPartID().toLatin1().data());

    // add field DIE_X and field DIE_Y
    if (m_eFields & fieldDieXY)
    {
        fprintf(gexReport->advancedReportHandle(), "%d,", pPartInfo->iDieX);
        fprintf(gexReport->advancedReportHandle(), "%d,", pPartInfo->iDieY);
    }

    // add field SITE
    if (m_eFields & fieldSite)
        fprintf(gexReport->advancedReportHandle(), "%d,", pPartInfo->m_site);

    // add field HBIN
    fprintf(gexReport->advancedReportHandle(), "%d,", pPartInfo->iHardBin);

    // add field SBIN
    fprintf(gexReport->advancedReportHandle(), "%d,", pPartInfo->iSoftBin);

    // add field TEST_TIME
    if (m_eFields & fieldTestTime)
        fprintf(gexReport->advancedReportHandle(), "%ld,", pPartInfo->lExecutionTime);

    // add field TNUM
    if (m_eFields & fieldTestNumber)
    {
        gexReport->BuildTestNumberString(ptTestCell);
        fprintf(gexReport->advancedReportHandle(), "T%s,", ptTestCell->szTestLabel);
    }

    // add field TNAME
    if (m_eFields & fieldTestName)
    {
        QString strTestName;
        gexReport->BuildTestNameString(pFile, ptTestCell, strTestName);
        fprintf(gexReport->advancedReportHandle(), "%s,", strTestName.toLatin1().constData());
    }

    // add field TTYPE
    fprintf(gexReport->advancedReportHandle(), "%c,", ptTestCell->bTestType);

    // add field UNITS
    double factor = ptTestCell->llm_scal;
    fprintf(gexReport->advancedReportHandle(), "%s,", ptTestCell->GetScaledUnits(&factor,
            ReportOptions.GetOption("dataprocessing","scaling").toString()).toLatin1().constData() /*ptTestCell->szTestUnits*/);

    //deal with expected figures and precisions for double values before printing them
    double dVal ;
    QString lScientificNotation = gexReport->GetOption("output", "scientific_notation").toString();
    int lPrecision = gexReport->GetOption("output", "precision").toInt();
    gexReport->getNumberFormat()->setOptions((lScientificNotation == "turn_on") ? 'e' : ' ', lPrecision);

    // add field LL and HL
    if (m_eFields & fieldTestLimits)
    {
    dVal = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
    pFile->FormatTestResultNoUnits(&dVal,ptTestCell->llm_scal);
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            fprintf(gexReport->advancedReportHandle(), "%s,", gexReport->getNumberFormat()->formatNumericValue(dVal, true).toStdString().c_str());
        else
            fprintf(gexReport->advancedReportHandle(), ",");
    dVal = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
    pFile->FormatTestResultNoUnits(&dVal,ptTestCell->llm_scal);
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            fprintf(gexReport->advancedReportHandle(), "%s,", gexReport->getNumberFormat()->formatNumericValue(dVal, true).toStdString().c_str());
        else
            fprintf(gexReport->advancedReportHandle(), ",");
    }

    dVal = fValue;
    pFile->FormatTestResultNoUnits(&dVal,ptTestCell->llm_scal);
    // add field VALUE
    if (fValue == GEX_C_DOUBLE_NAN)
    fprintf(gexReport->advancedReportHandle(),"n/a,");
    else
        fprintf(gexReport->advancedReportHandle(), "%s,", gexReport->getNumberFormat()->formatNumericValue(dVal, true).toStdString().c_str());

    // add field STATUS
    if(bFailResult == true)
        fprintf(gexReport->advancedReportHandle(),"F,");
    else if(bAlarmResult == true)
        fprintf(gexReport->advancedReportHandle(),"A,");
    else
        fprintf(gexReport->advancedReportHandle(),"P,");
}

void GexCSVDatalog::writeParametricDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID)
{
    Q_UNUSED(iSiteID);

    writeCommonFields(pFile, ptTestCell, pPartInfo, fValue, bFailResult, bAlarmResult);

    fprintf(gexReport->advancedReportHandle(), "\n");
}

void GexCSVDatalog::writeFunctionalDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iSiteID)
{
    Q_UNUSED(iSiteID);

    // Datalog functional details when
    //	* Test result is fail AND
    //	* Pin failed exists AND
    //	* Field PIN_NAME OR PIN_NUMBER should be displayed
    if (failureInfo.pinmapCount() > 0 && bFailResult && ((m_eFields & fieldPinNumber) || (m_eFields & fieldPinName)))
    {
        for(int iPinIndex = 0; iPinIndex < failureInfo.pinmapCount(); iPinIndex++)
        {
            // Write common fields
            writeCommonFields(pFile, ptTestCell, pPartInfo, fValue, bFailResult, false);

            // write functional fields
            writeFunctionalFields(strVectorName, failureInfo, iPinIndex);

            fprintf(gexReport->advancedReportHandle(), "\n");
        }
    }
    else
    {
        // Write common fields
        writeCommonFields(pFile, ptTestCell, pPartInfo, fValue, bFailResult, false);

        // write functional fields
        if (bFailResult)
            writeFunctionalFields(strVectorName, failureInfo, -1);

        fprintf(gexReport->advancedReportHandle(), "\n");
    }
}

void GexCSVDatalog::writeFunctionalFields(const QString &strVectorName, const GexVectorFailureInfo &failureInfo, int iPinIndex)
{
    // add field PATTERN
    if (m_eFields | fieldPattern)
        fprintf(gexReport->advancedReportHandle(), "%s,", strVectorName.toLatin1().data());

    // add field CYCLE_COUNT
    if (m_eFields | fieldCycleCount)
    fprintf(gexReport->advancedReportHandle(), "%d,", failureInfo.vectorCycleCount());

    // add field VADDR
    if (m_eFields | fieldRelVAddr)
        fprintf(gexReport->advancedReportHandle(), "%d,", failureInfo.relativeVectorAddress());

    // add field FAIL_PINS
    if (m_eFields | fieldPinsFailed)
        fprintf(gexReport->advancedReportHandle(), "%d,", failureInfo.pinmapCount());

    // add field PIN_NB
    if ((m_eFields | fieldPinNumber) && iPinIndex >= 0)
        fprintf(gexReport->advancedReportHandle(), "%d,",	failureInfo.pinmapAt(iPinIndex)->iPinmapIndex);

    // add field PIN_NAME
    if ((m_eFields | fieldPinName) && iPinIndex >= 0)
        fprintf(gexReport->advancedReportHandle(), "%s,",	failureInfo.pinmapAt(iPinIndex)->strLogicName.toLatin1().data());
}

void GexCSVDatalog::writeEndDatalog()
{
    // Do nothing for CSV output
}

void GexCSVDatalog::writeBeginFileInfo()
{
    // Do nothing for CSV output
}

void GexCSVDatalog::writeEndFileInfo()
{
    fprintf(gexReport->advancedReportHandle(),"\n");
}

void GexCSVDatalog::writePartFooter(const CPartInfo *pPartInfo)
{
    Q_UNUSED(pPartInfo);

    // Do nothing for CSV output
}
