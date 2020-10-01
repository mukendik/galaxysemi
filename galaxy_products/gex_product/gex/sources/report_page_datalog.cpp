/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Datalog' page (Advanced report)
/////////////////////////////////////////////////////////////////////////////

#include <QLabel>
#include "report_build.h"
#include "report_options.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "cpart_info.h"
#include "engine.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

extern double			ScalingPower(int iPower);

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_Datalog(bool bValidSection)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Datalog' page & header
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- DATALOG report ----\n");
        if(bValidSection == false)
            fprintf(hReportFile,"No Datalog data available !\n");
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased())
        //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        if(bValidSection == false)
        {
            fprintf(hReportFile,"<p align=\"left\">&nbsp;</p>\n");
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Datalog data available !<br>\n",iHthmNormalFontSize);
        }
    }

    if(bValidSection == true)
    {
        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup	= getGroupsList().isEmpty()?NULL:getGroupsList().first();
        CGexFileInGroup *pFile		= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if(m_pReportOptions->isReportOutputHtmlBased())
            //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");

        if(m_pReportOptions->iFiles == 1)
        {
            WriteInfoLine(hReportFile, "Program", pFile->getMirDatas().szJobName);
            WriteInfoLine(hReportFile, "Product", pFile->getMirDatas().szPartType);
            WriteInfoLine(hReportFile, "Lot", pFile->getMirDatas().szLot);
            WriteInfoLine(hReportFile, "Datalog date", TimeStringUTC(pFile->getMirDatas().lStartT));
        }

        QString lMessage;
        switch(m_pReportOptions->getAdvancedReportSettings())
        {
            case GEX_ADV_DATALOG_ALL: // list tests
            default:
                lMessage = "list all tests";
                break;
            case GEX_ADV_DATALOG_FAIL: // list FAIL tests only
                lMessage = "Failing tests only";
                break;
            case GEX_ADV_DATALOG_OUTLIER:
                lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Outliers for tests: ");
                break;
            case GEX_ADV_DATALOG_LIST:// datalog specific tests
            case GEX_ADV_DATALOG_RAWDATA:// datalog specific tests : raw datalog
                lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("list tests: ");
                break;
        }

        WriteInfoLine(hReportFile, "Datalog mode", lMessage.toLatin1().constData());

        if(strOutputFormat=="CSV")
            fprintf(hReportFile,"\n");
        else
        {
            // Standard multi-pages HTML file
            fprintf(hReportFile,"</table>\n");
            fprintf(hReportFile,"<br>\n");

            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,"","Datalog");

            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n");
        }
    }

    // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
    m_pReportOptions->lAdvancedHtmlPages = 1;	// incremented as we generate the HTML datalog pages...
    // Reset line counter...used to insert page breaks in Advanced HTML pages that get too long
    m_pReportOptions->lAdvancedHtmlLinesInPage=0;

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Datalog(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    if(strOutputFormat=="HTML")
    {
        // Standard multi-paes HTML file
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file not created.

        // In case No tests in the last datalog run...then fill table with only empty line (workaround QT bug)
        fprintf(hReportFile,"<tr>\n<td> </td>\n</tr>\n");

        fprintf(hReportFile,"</table>\n");
        if(m_pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">Datalog report is disabled!<br>\n",iHthmNormalFontSize);
        else
            fprintf(hReportFile,"<p align=\"center\"> [----End Of Datalog----]<br>\n");
        fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
        fprintf(hReportFile,"</body>\n");
        fprintf(hReportFile,"</html>\n");

        // Close report file...unlerss flat HTML with multiple sections (eg: MyReport)
        if(m_pReportOptions->strTemplateFile.isEmpty())
            CloseReportFile();
    }

    return GS::StdLib::Stdf::NoError;
}


/////////////////////////////////////////////////////////////////////////////
// Creates ALL pages for the Binning report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Datalog(qtTestListStatistics * pqtStatisticsList)
{
    // Create Software Binning Summary report
    int			iStatus;

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Datalog section...");

    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
    CGexGroupOfFiles *	pGroup		= NULL;
    CGexFileInGroup *	pFile		= NULL;
    CTest *				pTestCell	= NULL;
    CTest *				pTestBin	= NULL;
    CPartInfo *			pPartInfo	= NULL;

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Opens HTML Datalog page
    iStatus = PrepareSection_Datalog(true);

    if(iStatus != GS::StdLib::Stdf::NoError)
        return iStatus;

    int nBegin			= 0;
    int nEnd			= 0;
    int nTestsInDatalog = 0;
    int nBinTestNumber	= GEX_TESTNBR_OFFSET_EXT_SBIN;

    switch(m_pReportOptions->iWafermapType)
    {
        case GEX_WAFMAP_SOFTBIN:
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_ZONAL_SOFTBIN:
            // Wafer map on: SOFT  bin
            nBinTestNumber = GEX_TESTNBR_OFFSET_EXT_SBIN;
            break;
        case GEX_WAFMAP_HARDBIN:
        case GEX_WAFMAP_STACK_HARDBIN:
        case GEX_WAFMAP_ZONAL_HARDBIN:
            // Wafer map on: HARD  bin
            nBinTestNumber = GEX_TESTNBR_OFFSET_EXT_HBIN;
            break;
    }

    // Create report section for each group.
    while(itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();

        // Loop on all wafer
        for (int nIndex = 0; nIndex < pGroup->pFilesList.count(); nIndex++)
        {
            pFile = pGroup->pFilesList.at(nIndex);

            // Find out the begin and end offset of this sublot
            pFile->ptTestList->findSublotOffset(nBegin, nEnd, nIndex);

            // Get the bin testcell
            if (pFile->FindTestCell(nBinTestNumber, GEX_PTEST, &pTestBin, true, false) == 1)
            {
                // Loop on each part
                for (int nSample = nBegin; nSample < nEnd; nSample++)
                {
                    pPartInfo = pFile->pPartInfoList.at(nSample - nBegin);

                    // For each test for this part, datalog result
                    foreach(pTestCell, *pqtStatisticsList)
                    {
                        // validity is checked in WriteDatalogTestResult();
                        WriteDatalogTestResult(pTestCell, pFile, nSample, pPartInfo, nTestsInDatalog);

                    }

                    if (nTestsInDatalog > 0)
                    {
                        if (pTestBin->m_testResult.isValidIndex(nSample))
                        {
                            WriteDatalogBinResult(static_cast<int>(pTestBin->m_testResult.resultAt(nSample)),
                                                  pPartInfo,
                                                  nTestsInDatalog);
                        }
                    }
                }
            }
        }
    }

    // Close Datalog section.
    iStatus = CloseSection_Datalog();

    return iStatus;
}

void CGexReport::WriteDatalogTestResult(CTest * ptTestCell, CGexFileInGroup * pFile, int nSample, CPartInfo * pPartInfo, int& lTestsInDatalog)
{
    QString strAdvDatalogFieldOptions = (m_pReportOptions->GetOption(QString("adv_datalog"), QString("field"))).toString();
    QStringList qslAdvDatalogFieldOptionsList = strAdvDatalogFieldOptions.split(QString("|"));


    // Handle may be NULL if a datalog is requested, while Report output is 'interactive only'!
    if(hReportFile == NULL)
        return;

    if( !ptTestCell->m_testResult.isValidIndex(nSample) )
        return;
    if( !ptTestCell->m_testResult.isValidResultAt(nSample) )
        return;

    double	fValue	= ptTestCell->m_testResult.resultAt(nSample);

    // Scale result to be normalized as test limits
    fValue *= ScalingPower(ptTestCell->res_scal);

    bool	bFailResult = ptTestCell->isFailingValue(fValue, ptTestCell->m_testResult.passFailStatus(nSample));
    bool	bIsOutlier	= false;

    switch(m_pReportOptions->getAdvancedReportSettings())
    {
    case GEX_ADV_DATALOG_ALL:	// ALL tests
    default:
        if(bIsOutlier == true)
            return;
        break;					// Just continue in this function !
    case GEX_ADV_DATALOG_FAIL:	// Only FAIL tests
        if((bFailResult == false) || (bIsOutlier == true))
            return;				// Test is PASS: do not Datalog !
        break;
    case GEX_ADV_DATALOG_OUTLIER:
        // Swap Outlier flag so we can fall into next case.
        if(bIsOutlier == false)
            bIsOutlier = true;
        else
            bIsOutlier = false;
        // MUST FALL INTO NEXT CASES!
    case GEX_ADV_DATALOG_LIST:	// Tests between given range
    case GEX_ADV_DATALOG_RAWDATA:// Tests between given range: raw data
        if(bIsOutlier == true)
            return;

        if(m_pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex))
            break;
        else
            return;
    }

    if(lTestsInDatalog == 0)
    {
        // First test datalogged in part, and not RAW datalog.
        WriteDatalogPartHeader(pPartInfo, pFile, lTestsInDatalog);
    }

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        // Check if have to display testlabel
        if(qslAdvDatalogFieldOptionsList.contains(QString("test_number")))
            fprintf(hReportFile,"T%s,",ptTestCell->szTestLabel);

        // Check if have to display testlabel
        if( (qslAdvDatalogFieldOptionsList.contains(QString("test_name"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
        {
            QString strTestName;
            BuildTestNameString(pFile, ptTestCell, strTestName);
            fprintf(hReportFile,"%s,", strTestName.toLatin1().constData());
        }

        // Check if have to display limits
        if( (qslAdvDatalogFieldOptionsList.contains(QString("limits"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
        {
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                pFile->FormatTestLimit(ptTestCell,
                                       ptTestCell->GetCurrentLimitItem()->szLowL,
                                       ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                       ptTestCell->llm_scal);
                if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                {
                    // Display 'strict' or 'not strict' Low limit
                    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_LTLNOSTRICT)
                        fprintf(hReportFile,"%s, <=,",ptTestCell->GetCurrentLimitItem()->szLowL);	// Not a strict limit (i.e: '<=')
                    else
                        fprintf(hReportFile,"%s, <,",ptTestCell->GetCurrentLimitItem()->szLowL);	// strict limit (i.e: '<=')
                }
                else
                    fprintf(hReportFile,"LowL: %s,",ptTestCell->GetCurrentLimitItem()->szLowL);
            }
            else
            {
                if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                {
                    fprintf(hReportFile,",,");
                }
                else
                    fprintf(hReportFile,"LowL: na,");
            }
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                pFile->FormatTestLimit(ptTestCell,
                                       ptTestCell->GetCurrentLimitItem()->szHighL,
                                       ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                       ptTestCell->hlm_scal);
                if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="2rows")
                    fprintf(hReportFile,"HighL: %s,",ptTestCell->GetCurrentLimitItem()->szHighL);
            }
        }

        fprintf(hReportFile,"%s,", pFile->FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
        if(bFailResult == true)
            fprintf(hReportFile," *F,");
//      else if(bAlarmResult == true)
//          fprintf(hReportFile," *A,");
        else
            fprintf(hReportFile," ,");

        // Single row mode: display high limit now.
        if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
        {
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                // Display 'strict' or 'not strict' Hightlimit
                if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_HTLNOSTRICT)
                    fprintf(hReportFile,"<=, %s,",ptTestCell->GetCurrentLimitItem()->szHighL);	// Not a strict limit (i.e: '<=')
                else
                    fprintf(hReportFile,"<, %s,",ptTestCell->GetCurrentLimitItem()->szHighL);	// strict limit (i.e: '<=')
            }
            else
            {
                fprintf(hReportFile,",,");	// strict limit (i.e: '<=')
            }
        }

        // If Test is in second column...or we must only create one row
        if((lTestsInDatalog % 2) ||	(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                )
            fprintf(hReportFile,"\n");
    }
    else if( m_pReportOptions->isReportOutputHtmlBased())
            //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        QString	strBookmark;

            // Check if page break needed
            //		WriteDatalogCheckPageBreak();

        // Bookmark: are in same page if FLAT HTML page is generated
        if(strOutputFormat=="HTML")
            strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
        else
            strBookmark = "#StatT";	// Test Statistics bookmark header string.

        // Datalog current part in the HTML datalog file.
        if(lTestsInDatalog % 2)
        {

            // Test in in second column
            fprintf(hReportFile,"<td width=\"10%%\"></td>\n");

            // Check if have to display test number
            if(qslAdvDatalogFieldOptionsList.contains(QString("test_number")))
            {
                if((bFailResult == true) && (ptTestCell->iHtmlStatsPage>0))
                    fprintf(hReportFile,"<td width=\"15%%\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">T%s</a>: ",
                            ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                else
                    fprintf(hReportFile,"<td width=\"15%%\">T%s: ",ptTestCell->szTestLabel);
            }
            else
                fprintf(hReportFile,"<td width=\"15%%\">");

            // Check if have to display testlabel
            if( (qslAdvDatalogFieldOptionsList.contains(QString("test_name"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                QString strTestName;
                BuildTestNameString(pFile, ptTestCell, strTestName);
                fprintf(hReportFile,"%s", strTestName.toLatin1().constData());
            }

            // Check if have to display limits
            if( (qslAdvDatalogFieldOptionsList.contains(QString("limits"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    pFile->FormatTestLimit(ptTestCell,
                                           ptTestCell->GetCurrentLimitItem()->szLowL,
                                           ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                           ptTestCell->llm_scal);
                    fprintf(hReportFile,"<br>LowL: %s",ptTestCell->GetCurrentLimitItem()->szLowL);
                }
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    pFile->FormatTestLimit(ptTestCell,
                                           ptTestCell->GetCurrentLimitItem()->szHighL,
                                           ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                           ptTestCell->hlm_scal);
                    fprintf(hReportFile,"<br>HighL: %s",ptTestCell->GetCurrentLimitItem()->szHighL);
                }
            }
            fprintf(hReportFile,"</td>\n");

            fprintf(hReportFile,"<td width=\"25%%\" align=\"right\">");
            if(bFailResult == true /*|| bAlarmResult == true*/)
                fprintf(hReportFile,"<font color=\"#FF0000\"><b>");
            fprintf(hReportFile,"%s", pFile->FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
            if(bFailResult == true)
                fprintf(hReportFile," *F</b></font>");
            /*			else
   if(bAlarmResult == true)
    fprintf(hReportFile," *A</b></font>");*/
            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"</tr>\n");
        }
        else
        {
            if(lTestsInDatalog == 0)
            {
                // First test datalogged in part, and not RAW datalog.
                WriteDatalogPartHeader(pPartInfo, pFile, lTestsInDatalog);
            }

            // Test is in first column
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"10%%\"></td>\n");

            // Check if have to display test number
            if(qslAdvDatalogFieldOptionsList.contains("test_number"))
            {
                if((bFailResult == true) && (ptTestCell->iHtmlStatsPage>0))
                    fprintf(hReportFile,"<td width=\"15%%\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">T%s</a></b>: ",
                            ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                else
                    fprintf(hReportFile,"<td width=\"15%%\">T%s: ",ptTestCell->szTestLabel);
            }
            else
                fprintf(hReportFile,"<td width=\"15%%\">");

            // Check if have to display testlabel
            if( (qslAdvDatalogFieldOptionsList.contains(QString("test_name"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                    fprintf(hReportFile,"</td><td>\n");
                QString strTestName;
                BuildTestNameString(pFile, ptTestCell, strTestName);
                fprintf(hReportFile,"%s", strTestName.toLatin1().constData());
            }
            if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
            {
                // In one column mode, use more space to display info!
                fprintf(hReportFile,"</td>\n");
                fprintf(hReportFile,"<td width=\"25%%\" align=\"right\">");
            }

            // Check if have to display limits
            if( (qslAdvDatalogFieldOptionsList.contains(QString("limits"))) && (m_pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    pFile->FormatTestLimit(ptTestCell,
                                           ptTestCell->GetCurrentLimitItem()->szLowL,
                                           ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                           ptTestCell->llm_scal);
                    if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="2rows")
                    {
                        // In 2 column mode, use less space to display info (display limits with test name)!
                        fprintf(hReportFile,"<br>LowL: ");
                    }
                    fprintf(hReportFile,"%s  ",ptTestCell->GetCurrentLimitItem()->szLowL);
                    if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                    {
                        // Display 'strict' or 'not strict' Low limit
                        if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_LTLNOSTRICT)
                            fprintf(hReportFile,"&lt;= &nbsp;&nbsp; </td><td align=\"center\">");	// Not a strict limit (i.e: '<=')
                        else
                            fprintf(hReportFile,"&lt; &nbsp;&nbsp; </td><td align=\"center\">");	// strict limit (i.e: '<=')
                    }
                }
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    // to be sure to update the limit string
                    pFile->FormatTestLimit(ptTestCell,
                                           ptTestCell->GetCurrentLimitItem()->szHighL,
                                           ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                           ptTestCell->hlm_scal);
                    if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="2rows")
                    {
                        // In 2 column mode, use less space to display info (display limits with test name)!
                        fprintf(hReportFile,"<br>HighL: %s",ptTestCell->GetCurrentLimitItem()->szHighL);
                    }
                }
            }
            else
                if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
                {
                    // In one column mode, use more space to display info!
                    fprintf(hReportFile,"</td>\n");
                    fprintf(hReportFile,"<td align=\"center\">");
                }

            if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="2rows")
            {
                fprintf(hReportFile,"</td>\n");
                fprintf(hReportFile,"<td width=\"25%%\" align=\"right\">");
            }

            if(bFailResult == true /*|| bAlarmResult == true*/)
                fprintf(hReportFile,"<font color=\"#FF0000\"><b>");
            fprintf(hReportFile,"%s", pFile->FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
            if(bFailResult == true)
                fprintf(hReportFile," *F</b></font>");
            /*	else
   if(bAlarmResult == true)
    fprintf(hReportFile," *A</b></font>");*/
            fprintf(hReportFile,"</td>\n");

            // Single row mode: display high limit now.
            if(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
            {
                if( (qslAdvDatalogFieldOptionsList.contains(QString("limits"))) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                {
                    // Display 'strict' or 'not strict' Hightlimit
                    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_HTLNOSTRICT)
                        fprintf(hReportFile,"</td><td>&lt;= %s</td>\n",ptTestCell->GetCurrentLimitItem()->szHighL);	// Not a strict limit (i.e: '<=')
                    else
                        fprintf(hReportFile,"</td><td>&lt; %s</td>\n",ptTestCell->GetCurrentLimitItem()->szHighL);	// strict limit (i.e: '<=')
                }
            }
        }
    }

    // Update HTML line count.
    if(lTestsInDatalog % 2)
        m_pReportOptions->lAdvancedHtmlLinesInPage++;

    // Keeps track of the number of tests datalogged in part
    lTestsInDatalog++;

    // Check if we must only fill a single ROW.
    if(	(m_pReportOptions->GetOption("adv_datalog","format").toString()=="1row")
            && (lTestsInDatalog % 2)
            && (m_pReportOptions->isReportOutputHtmlBased())    //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            )
    {
        // Make sure we only fill one row if this is the setting...
        // Test in in second column...so tab until on next line
        fprintf(hReportFile,"</tr>\n");
        lTestsInDatalog++;
    }
}

////////////////////////////////////////////////////////////////////////////
// Write Datalog Binning Result (HTML mode only).
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteDatalogBinResult(int iBin, CPartInfo *pPartInfo, int& lTestsInDatalog)
{
    // Convert from 'unsigned int' to 'int'
    if(pPartInfo->iDieX >= 32768) pPartInfo->iDieX -= 65536;
    if(pPartInfo->iDieY >= 32768) pPartInfo->iDieY -= 65536;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    QString strAdvDatalogFieldOptions = (m_pReportOptions->GetOption(QString("adv_datalog"), QString("field"))).toString();
    QStringList qslAdvDatalogFieldOptionsList = strAdvDatalogFieldOptions.split(QString("|"));


    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.

        // Current Test curror is in second column: skip remaining table cells
        if(lTestsInDatalog % 2)
            fprintf(hReportFile,"\n");

        if(m_pReportOptions->getAdvancedReportSettings() != GEX_ADV_DATALOG_RAWDATA)
        {
            // If not RAW datalog!
            fprintf(hReportFile,"BIN %d",iBin);

            if(pPartInfo->bPass)
                fprintf(hReportFile," PASSED");
            else
                fprintf(hReportFile," *FAILED*");

            // Die location
            if(qslAdvDatalogFieldOptionsList.contains(QString("die_loc")))
                fprintf(hReportFile,"  [DieX=%d ; DieY=%d]",pPartInfo->iDieX,pPartInfo->iDieY);

            // PartID
            if(pPartInfo->getPartID().isEmpty() == false)
                fprintf(hReportFile,"  - PartID: %s",pPartInfo->getPartID().toLatin1().constData());

            // Testing time
            if(pPartInfo->lExecutionTime > 0)
                fprintf(hReportFile,"  Test Time: %.3g sec.",(float)pPartInfo->lExecutionTime/1000.0);

            fprintf(hReportFile,"\n\n");
        }
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        if(lTestsInDatalog % 2)
        {
            // Current Test cusror is in second column: skip remaining table cells
            fprintf(hReportFile,"</tr>\n");
        }

        if(m_pReportOptions->getAdvancedReportSettings() != GEX_ADV_DATALOG_RAWDATA)
        {
            // If not RAW datalog!
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"10%%\"></td>\n");

            if(pPartInfo->bPass)
                fprintf(hReportFile,"<td width=\"15%%\"><b>BIN %d - PASSED</b></td>\n",iBin);
            else
                fprintf(hReportFile,"<td width=\"15%%\"><font color=\"#FF0000\"><b>BIN %d *FAILED*</b></font></td>\n",iBin);

            // Die location
            fprintf(hReportFile,"<td width=\"25%%\" align=\"left\">");
            if(qslAdvDatalogFieldOptionsList.contains(QString("die_loc")))
                fprintf(hReportFile,"<b>DieX=%d, DieY=%d</b>  ",pPartInfo->iDieX,pPartInfo->iDieY);

            // PartID
            if(pPartInfo->getPartID().isEmpty() == false)
                fprintf(hReportFile,"(PartID: %s)",pPartInfo->getPartID().toLatin1().constData());

            // Testing time
            if(pPartInfo->lExecutionTime > 0)
                fprintf(hReportFile,"  Test Time: %.3g sec.",(float)pPartInfo->lExecutionTime/1000.0);


            fprintf(hReportFile,"</td>\n");
            fprintf(hReportFile,"<td width=\"10%%\"></td>\n");
            fprintf(hReportFile,"<td width=\"15%%\"></td>\n");
            fprintf(hReportFile,"<td width=\"25%%\" align=\"right\"></td>\n");
            fprintf(hReportFile,"</tr>\n");
        }

        fprintf(hReportFile,"</table>\n<br>\n<br>");
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
    }

    // Reset test counter.
    lTestsInDatalog = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Datalog par header: tells which part currently dataloged
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteDatalogPartHeader(CPartInfo * pPartInfo, CGexFileInGroup * pFile, int& lTestsInDatalog)
{
    // Handle may be NULL if a datalog is requested, while Report output is 'interactive only'!
    if(hReportFile == NULL)
        return;

    // Check if page break needed (HTML only)
//	WriteDatalogCheckPageBreak();

    // If RAW datalog, do not write header!
    if(m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_DATALOG_RAWDATA)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,
                "Datalog# %ld,Lot: %s,Sublot: %s,Wafer: %s\n",
                pPartInfo->lPartNumber,
                pFile->getMirDatas().szLot,
                pFile->getMirDatas().szSubLot,
                pFile->getWaferMapData().szWaferID);
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"10%%\"><b>Datalog#:</b></td>\n");
        fprintf(hReportFile,
                "<td width=\"15%%\"><b>%ld</b></td>\n",
                pPartInfo->lPartNumber);

        if(*pFile->getMirDatas().szLot)
            fprintf(hReportFile,"<td width=\"25%%\" align=\"right\"><b>Lot: %s</b></td>\n", pFile->getMirDatas().szLot);
        else
            fprintf(hReportFile,"<td width=\"25%%\"></td>\n");

        if(*pFile->getMirDatas().szSubLot)
            fprintf(hReportFile,"<td width=\"10%%\" align=\"right\"><b>Sublot: %s</b></td>\n", pFile->getMirDatas().szSubLot);
        else
            fprintf(hReportFile,"<td width=\"10%%\"></td>\n");

        if(*pFile->getWaferMapData().szWaferID)
            fprintf(hReportFile,"<td width=\"15%%\" align=\"right\"><b>Wafer:</b> %s</td>\n", pFile->getWaferMapData().szWaferID);
        else
            fprintf(hReportFile,"<td width=\"15%%\"></td>\n");
        fprintf(hReportFile,"<td width=\"25%%\" align=\"right\"></td>\n");
        fprintf(hReportFile,"</tr>\n");
    }

    lTestsInDatalog+=2;	// Next test result to datalog will start on a even column.
    m_pReportOptions->lAdvancedHtmlLinesInPage++;	// Update HTML line count.
}
