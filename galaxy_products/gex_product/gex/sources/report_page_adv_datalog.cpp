/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Datalog' page (Advanced report)
/////////////////////////////////////////////////////////////////////////////

#include <QLabel>

#include "report_build.h"
#include "report_options.h"
#include "gexabstractdatalog.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "engine.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvDatalog(bool bValidSection)
{
    QString strDatalogOptions = m_pReportOptions->GetOption("adv_datalog", "format").toString();

    if (strDatalogOptions == "advanced")
    {
        if (GexAbstractDatalog::createInstance(bValidSection) == false)
            return GS::StdLib::Stdf::ErrorOpen;
    }
    else
    {
        QString of=m_pReportOptions->GetOption("output", "format").toString();

        // Creates the 'Datalog' page & header
        if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Generating .CSV report file.
            fprintf(hReportFile,"\n---- DATALOG report ----\n");
            if(bValidSection == false)
                fprintf(hAdvancedReport,"No Datalog data available !\n");
        }
        else
            if  (m_pReportOptions->isReportOutputHtmlBased())
            //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
            //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            // Generating HTML report file.
            // Open advanced.htm file
            if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
                return GS::StdLib::Stdf::ReportFile;

            WriteHeaderHTML(hAdvancedReport,"#000080");	// Default: Text is Dark Blue
            // Title + bookmark
            WriteHtmlSectionTitle(hAdvancedReport,"all_advanced","More Reports: Datalog");

            if(bValidSection == false)
            {
                fprintf(hAdvancedReport,"<p align=\"left\">&nbsp;</p>\n");
                fprintf(hAdvancedReport,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Datalog data available !<br>\n",iHthmNormalFontSize);
            }
        }

        if(bValidSection == true)
        {
            // Get pointer to first group & first file (we always have them exist)
            CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
            CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            if (m_pReportOptions->isReportOutputHtmlBased())
                //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
                //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                fprintf(hAdvancedReport,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");

            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hAdvancedReport, "Program",pFile->getMirDatas().szJobName);
                WriteInfoLine(hAdvancedReport, "Product",pFile->getMirDatas().szPartType);
                WriteInfoLine(hAdvancedReport, "Lot",pFile->getMirDatas().szLot);
                WriteInfoLine(hAdvancedReport, "Datalog date",TimeStringUTC(pFile->getMirDatas().lStartT));
            }

            QString lMessage;
            if(m_pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
                lMessage = "Datalog report is disabled!";
            else
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
            WriteInfoLine(hAdvancedReport, "Datalog mode", lMessage.toLatin1().constData());
            if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                fprintf(hAdvancedReport,"\n");
            else
            {
                // Writing HTML file
                if( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" ) 	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                {
                    // Flat HTML file (for Word, PDF generation): simply close the table, but do not close the file.
                    fprintf(hAdvancedReport,"</table>\n");
                }
                else
                if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
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
                    fprintf(hAdvancedReport,"<tr>\n");
                    fprintf(hAdvancedReport,"<td bgcolor=%s>Link to pages:</td>",szFieldColor);
                    fprintf(hAdvancedReport,"<td bgcolor=%s><b><a href=\"advanced1.htm\">See Datalog pages</a> </b></td>",szDataColor);
                    fprintf(hAdvancedReport,"</tr>\n");
                    fprintf(hAdvancedReport,"</table>\n");
                    fprintf(hAdvancedReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
                    fprintf(hAdvancedReport,"</body>\n");
                    fprintf(hAdvancedReport,"</html>\n");
                    CloseReportFile(hAdvancedReport); // Close Datalog HTML file.

                    // Create first datalog page...
                    char szString[2048];
                    sprintf(szString,"%s/pages/advanced1.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
                    hReportFile = hAdvancedReport = fopen(szString,"wt");
                    if(hAdvancedReport == NULL)
                        return GS::StdLib::Stdf::ReportFile;

                    fprintf(hAdvancedReport,"<html>\n");
                    fprintf(hAdvancedReport,"<head>\n");
                    fprintf(hAdvancedReport,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
                    fprintf(hAdvancedReport,"</head>\n");
                    // Sets default background color = white, text color given in argument.
                    fprintf(hAdvancedReport,"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!

                }

                // Keep track of total HTML pages written
                lReportPageNumber++;

                // Title + bookmark
                WriteHtmlSectionTitle(hAdvancedReport,"","Datalog");

                fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n");
            }
        }

        // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
        m_pReportOptions->lAdvancedHtmlPages = 1;	// incremented as we generate the HTML datalog pages...
        // Reset line counter...used to insert page breaks in Advanced HTML pages that get too long
        m_pReportOptions->lAdvancedHtmlLinesInPage=0;
    }

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvDatalog(void)
{
    if (GexAbstractDatalog::existInstance())
        GexAbstractDatalog::releaseInstance();
    else
    {
        QString of=m_pReportOptions->GetOption("output", "format").toString();
        if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Generating .CSV report file: no specific task to perform when section written !
        }
        else
        if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        {
            // Standard multi-paes HTML file
            if(hAdvancedReport == NULL)
                return GS::StdLib::Stdf::NoError;	// Just in case file not created.

            // In case No tests in the last datalog run...then fill table with only empty line (workaround QT bug)
            fprintf(hAdvancedReport,"<tr>\n<td> </td>\n</tr>\n");

            fprintf(hAdvancedReport,"</table>\n");
            if(m_pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
                fprintf(hAdvancedReport,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">Datalog report is disabled!<br>\n",iHthmNormalFontSize);
            else
                fprintf(hAdvancedReport,"<p align=\"center\"> [----End Of Datalog----]<br>\n");
            // Writes HTML footer Application name/web site string
            fprintf(hAdvancedReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
            fprintf(hAdvancedReport,"</body>\n");
            fprintf(hAdvancedReport,"</html>\n");

            CloseReportFile(hAdvancedReport); // Close Datalog HTML file.
        }
    }
    return GS::StdLib::Stdf::NoError;
}
