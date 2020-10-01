#include <QLabel>
#include "browser.h"
#include "gex_report.h"
#include "gex_group_of_files.h"
#include "shift_report_unit.h"
#include "charac1_report_unit.h"
#include "charac2_report_unit.h"
#include "ftr_correlation_report.h"
#include "ws_pat_report_unit.h"
#include "ft_pat_report_unit.h"
#include <gqtl_log.h>
#include "engine.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

// in report_page_adv_boxplot.cpp
extern bool compareCTest( CTest *test1, CTest *test2);

int	CGexReport::OpenFile_Advanced(const QString& strFile /*= QString()*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" html = '%1'").arg( strOutputFormat).toLatin1().constData() );
    // Open <stdf-filename>/report/advanced.htm
    if(strOutputFormat=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
    {
        QString strAdvancedReportFile = strFile;

        if (strFile.isNull() || strFile.isEmpty())
            strAdvancedReportFile = m_pReportOptions->strReportDirectory + "/pages/advanced.htm";

        hReportFile		= NULL;
        hAdvancedReport	= fopen(strAdvancedReportFile.toLatin1().constData(), "wt");
    }
    else
    {
        // Creating a flat HTML file, so we do not close HTML pages between section, all pages are merged
        hAdvancedReport = hReportFile;
    }
    if(hAdvancedReport == NULL)
        return GS::StdLib::Stdf::ReportFile;
    else
        return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_Advanced(BOOL bValidSection)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("output format : '%1'").arg( strOutputFormat).toLatin1().constData() );
    // Creates the 'Advanced Histogram' page & header
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        hAdvancedReport = hReportFile;
    }
    else
        if( m_pReportOptions->isReportOutputHtmlBased())
            //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            // Generating HTML report file.
            // Total number of HTML pages to create
            m_pReportOptions->lAdvancedHtmlPages=0;
        }

    switch(m_pReportOptions->getAdvancedReport())
    {
    case GEX_ADV_DISABLED:
    case GEX_ADV_GUARDBANDING:
        if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Generating .CSV report file.
            fprintf(hReportFile,"\nNo advanced report requested!\n");
        }
        else
            if( m_pReportOptions->isReportOutputHtmlBased() )
                //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
            {
                if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
                    return GS::StdLib::Stdf::ReportFile;
                hReportFile = hAdvancedReport;
                // Write page break (ignored if not writing a flat HTML document)
                WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black
                WriteHtmlSectionTitle(hReportFile,"morereports","More Reports...");
                fprintf(hReportFile,"<p align=\"left\">&nbsp;</p>\n");
                fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No advanced report requested!<br>\n",iHthmNormalFontSize);
            }
        break;
    case GEX_ADV_HISTOGRAM:
        PrepareSection_AdvHisto(bValidSection);
        break;
    case GEX_ADV_TREND:
        PrepareSection_AdvTrend(bValidSection);
        break;
    case GEX_ADV_CORRELATION:
        PrepareSection_AdvScatter(bValidSection);
        break;
    case GEX_ADV_PROBABILITY_PLOT:
        PrepareSection_AdvProbabilityPlot(bValidSection);
        break;
    case GEX_ADV_BOXPLOT:
        PrepareSection_AdvBoxplotEx(bValidSection);
        break;
    case GEX_ADV_MULTICHART:
        PrepareSection_AdvMultiChart(bValidSection);
        break;
    case GEX_ADV_CANDLE_MEANRANGE:
        PrepareSection_AdvBoxPlot(bValidSection);
        break;
    case GEX_ADV_DATALOG:
        PrepareSection_AdvDatalog(bValidSection);
        break;
    case GEX_ADV_PEARSON:
        PrepareSection_AdvPearson(bValidSection);
        break;
    case GEX_ADV_PAT_TRACEABILITY:
//        PrepareSection_AdvPatTraceability(bValidSection);	// Displays PAT-Man report created into STDF file
        break;
    case GEX_ADV_PROD_YIELD:
        PrepareSection_AdvProductionYield(bValidSection);	// Production reports (Yield trend)
        break;

    case GEX_ADV_SHIFT:
        // Do we have to create the ShiftReportUnit now ?
        //PrepareSection_AdvShift(bValidSection); // will be done later
        break;

    case GEX_ADV_CHARAC_BOXWHISKER_CHART:
        //PrepareSection_AdvShift(bValidSection); // will be done later
        break;

    case GEX_ADV_OUTLIER_REMOVAL:
        PrepareSection_AdvOutlierRemoval(bValidSection);
        break;

    case GEX_ADV_GO_DIAGNOSTICS:
        PrepareSection_AdvOptimizerDiags(bValidSection);
        break;
    case GEX_ADV_FUNCTIONAL:
        PrepareSection_AdvHistoFunctional(bValidSection);
        break;
    }

    foreach(const QString &k, m_pReportOptions->GetReportUnits().keys())
    {
        if (k=="adv_shift")
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating the Shift ReportUnit...");
            GS::Gex::ShiftReportUnit* sru=new GS::Gex::ShiftReportUnit(this, k);
            mReportUnits.insert(k, sru);
            sru->PrepareSection(true);
        }
        else if (k=="adv_charac1")
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating the %1 ReportUnit...").arg( k).toLatin1().constData());
            GS::Gex::Charac1ReportUnit* ru = new GS::Gex::Charac1ReportUnit(this, k);
            mReportUnits.insert(k, ru);
            QString r=ru->PrepareSection(true);
            GSLOG(SYSLOG_SEV_NOTICE, QString("PrepareSection %1 : %2")
                  .arg(k).arg(r).toLatin1().constData());
        }
        else if (k=="adv_charac2")
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating the %1 ReportUnit...").arg( k).toLatin1().constData());
            GS::Gex::Charac2ReportUnit* ru = new GS::Gex::Charac2ReportUnit(this, k);
            mReportUnits.insert(k, ru);
            QString r=ru->PrepareSection(true);
            GSLOG(SYSLOG_SEV_NOTICE, QString("PrepareSection %1 : %2")
                  .arg(k).arg(r).toLatin1().constData());
        }
        else if (k=="adv_ftr_correlation")
        {
            //GEX_ADV_FTR_CORRELATION
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating the %1 ReportUnit...").arg( k).toLatin1().constData());
            GS::Gex::FTRCorrelationReport* ru=new GS::Gex::FTRCorrelationReport(this, k);
            mReportUnits.insert(k, ru);
            QString r=ru->PrepareSection(true);
            GSLOG(SYSLOG_SEV_NOTICE, QString("PrepareSection %1 : %2")
                  .arg(k).arg(r).toLatin1().constData());
        }
        else if (k=="adv_pat")
        {
            //GEX_ADV_PAT_TRACEABILITY
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating the %1 ReportUnit...").arg( k).toLatin1().constData());

            GS::Gex::ReportUnit * ru = NULL;

            if (ReportOptions.GetPATTraceabilityFile().isEmpty())
                ru = new GS::Gex::WSPATReportUnit(this, k);
            else
                ru = new GS::Gex::FTPATReportUnit(this, k);

            mReportUnits.insert(k, ru);
            QString r=ru->PrepareSection(true);
            GSLOG(SYSLOG_SEV_NOTICE, QString("PrepareSection %1 : %2")
                  .arg(k).arg(r).toLatin1().constData());
        }

    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Advanced(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" output format = %1").arg( strOutputFormat).toLatin1().constData() );

  foreach(GS::Gex::ReportUnit* ru, mReportUnits.values())
  {
     if (!ru)
           continue;
     ru->CloseSection();
  }

    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !

        if(m_pReportOptions->getAdvancedReport() == GEX_ADV_DATALOG)
        {
                CloseSection_AdvDatalog();
        }
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
        //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        switch(m_pReportOptions->getAdvancedReport())
        {
            case GEX_ADV_DISABLED:
            case GEX_ADV_GUARDBANDING:
                if(hAdvancedReport == NULL)
                    break;	// Just in case file was closed on memory allocation error while collecting trend data!
                // close page...if this section was created.
                if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true)
                    && (strOutputFormat=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
                    )
                {
                    // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
                    fprintf(hAdvancedReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
                    fprintf(hAdvancedReport,"</body>\n");
                    fprintf(hAdvancedReport,"</html>\n");
                    CloseReportFile(hAdvancedReport);
                }
                // Write page break (ignored if not writing a flat HTML document)
                WritePageBreak();
                break;
            case GEX_ADV_HISTOGRAM:
                CloseSection_AdvHisto();
                break;
            case GEX_ADV_TREND:
                CloseSection_AdvTrend();
                break;
            case GEX_ADV_CORRELATION:
                CloseSection_AdvScatter();
                break;
            case GEX_ADV_PROBABILITY_PLOT:
                CloseSection_AdvProbabilityPlot();
                break;
            case GEX_ADV_BOXPLOT:
                CloseSection_AdvBoxplotEx();
                break;
            case GEX_ADV_MULTICHART:
                CloseSection_AdvMultiChart();
                break;
            case GEX_ADV_CANDLE_MEANRANGE:
                CloseSection_AdvBoxPlot();
                break;
            case GEX_ADV_DATALOG:
                CloseSection_AdvDatalog();
                break;
            case GEX_ADV_PEARSON:
                CloseSection_AdvPearson();
                break;
            case GEX_ADV_SHIFT:
                // Do we have to delete the ShiftReportUnit now ?
                GSLOG(SYSLOG_SEV_WARNING, "check me : GEX_ADV_SHIFT");
                //CloseSection_AdvShift();
                break;
            case GEX_ADV_PAT_TRACEABILITY:
//                CloseSection_AdvPatTraceability();
                break;
            case GEX_ADV_PROD_YIELD:
                CloseSection_AdvProductionYield();	// Production reports (Yield trend)
                break;

            case GEX_ADV_OUTLIER_REMOVAL:
                CloseSection_AdvOutlierRemoval();
                break;
            case GEX_ADV_GO_DIAGNOSTICS:
                CloseSection_AdvOptimizerDiags();
                break;
            case GEX_ADV_FUNCTIONAL:
                CloseSection_AdvFunctionalHisto();
                break;
        }
    }
    return GS::StdLib::Stdf::NoError;
}

int	CGexReport::GetChartImageSize(QString sChartSize, int iTotalCharts)
{
    int iChartSize=GEX_CHARTSIZE_AUTO;
    if (sChartSize=="auto")
    {
        if(iTotalCharts > GEX_CHARTSIZEAUTO_MEDIUM)
            iChartSize = GEX_CHARTSIZE_SMALL;		// if over 40 charts to build
        else
        if(iTotalCharts > GEX_CHARTSIZEAUTO_LARGE)
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// if between 10 and 40 charts to build
        else
            iChartSize = GEX_CHARTSIZE_LARGE;		// if upto 10 charts to build
    }
    else if (sChartSize=="small")
        iChartSize=GEX_CHARTSIZE_SMALL;
    else if (sChartSize=="medium")
        iChartSize=GEX_CHARTSIZE_MEDIUM;
    else if (sChartSize=="large")
        iChartSize=GEX_CHARTSIZE_LARGE;
    else if (sChartSize=="banner")
        iChartSize=GEX_CHARTSIZE_BANNER;


    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // If creating a flat file (for WORD, PDF,...), then only support Medium (default)
    if (strOutputFormat!="HTML")
    {
        if ( (sChartSize!="medium") && (sChartSize!="large") && (sChartSize!="banner") )
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// Creating Flat HTML for Word, PDF,...output
    }

    return iChartSize;
}

int	CGexReport::GetChartImageSize(int iChartSize, int iTotalCharts)
{
    if(iChartSize == GEX_CHARTSIZE_AUTO)
    {
        if(iTotalCharts > GEX_CHARTSIZEAUTO_MEDIUM)
            iChartSize = GEX_CHARTSIZE_SMALL;		// if over 40 charts to build
        else
        if(iTotalCharts > GEX_CHARTSIZEAUTO_LARGE)
            iChartSize = GEX_CHARTSIZE_MEDIUM;		// if between 10 and 40 charts to build
        else
            iChartSize = GEX_CHARTSIZE_LARGE;		// if upto 10 charts to build
    }

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // If creating a flat file (for WORD, PDF,...), then only support Medium (default)
    if(strOutputFormat!="HTML")	//(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
    {
        switch(iChartSize)
        {
            case GEX_CHARTSIZE_MEDIUM:
            case GEX_CHARTSIZE_LARGE:
            case GEX_CHARTSIZE_BANNER:
                break;
            default:
                iChartSize = GEX_CHARTSIZE_MEDIUM;		// Creating Flat HTML for Word, PDF,...output
                break;
        }
    }
    return iChartSize;
}

extern bool CTestFailCountLessThan(CTest* s1, CTest* s2);

// Retrieve the Top N failing Tests from the start Test
// push CTest pointers into output
// returns number of Worst Test found
int RetrieveTopNWorstTests(int N, CTest* start, QList<CTest*> &output )
{
    if (!start || N<1)
        return -1;
    CTest * ptTestCell              = start;
    QString lOptionStorageDevice    = ReportOptions.GetOption("statistics","generic_galaxy_tests").toString();

    int nWorstTest=0;
    while(ptTestCell != NULL)
    {
        if ( ptTestCell->GetCurrentLimitItem()->ldFailCount <=0
             ||(ptTestCell->bTestType == '-' && lOptionStorageDevice == "hide"))
        {
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        if (output.size()< N )
        {
            output.insert(0, ptTestCell);
            qSort(output.begin(), output.end(), CTestFailCountLessThan);
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        if (ptTestCell->GetCurrentLimitItem()->ldFailCount > output.last()->GetCurrentLimitItem()->ldFailCount)
        {
            output.removeLast();
            output.insert(0, ptTestCell);
            qSort(output.begin(), output.end(), CTestFailCountLessThan);
        }

        ptTestCell = ptTestCell->GetNextTest();
    }

    return nWorstTest;
}

int	CGexReport::CreatePages_Advanced(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("format = '%1' iAdvancedReport = %2 TestsList = '%3' ")
         .arg(m_pReportOptions->GetOption("output","format").toString().toLatin1().data())
         .arg(m_pReportOptions->getAdvancedReport())
         .arg(m_pReportOptions->strAdvancedTestList).toLatin1().data());

    CGexGroupOfFiles *pGroup=0;
    CGexFileInGroup *pFile=0;

    mAdvancedTestsListToReport.clear();

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Advanced reports...");

    QString of=m_pReportOptions->GetOption("output", "format").toString();
    QString pf=m_pReportOptions->GetOption("output", "paper_format").toString();
    QString ps=m_pReportOptions->GetOption("output", "paper_size").toString();

    // Ensure we use latest options set
    if (!m_cStats.UpdateOptions(m_pReportOptions))
        GSLOG(SYSLOG_SEV_WARNING, " warning : failed to update options !");

    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    if(	((m_pReportOptions->getAdvancedReport() == GEX_ADV_DISABLED)
         && (of!="HTML") //(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
         )
            ||
            (
                m_pReportOptions->isReportOutputHtmlBased()
                //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
                //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE"||of=="ODT")
                && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_ADVANCED))
            )
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Section must be disabled for given output format : %1 ").arg( of).toLatin1().constData());
        if (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_ADVANCED)
            GSLOG(SYSLOG_SEV_DEBUG, "GEX_HTMLSECTION_ADVANCED must be skiped.");
        int iStatus=CloseSection_Advanced();
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("CloseSection_Advanced returned %1").arg( iStatus).toLatin1().constData());

        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
        return GS::StdLib::Stdf::NoError;
    }

    // Makes the 'Advanced Reports' page become 'advanced.htm'
    // Unless datalog, create pages...
    // NO NEED to Open section 'SECTION_ADVANCED' because it
    // is done automatically during pass1!
    int	iStatus;
    int iTotalSteps;
    CTest	*ptTestCell=0;	// Pointer to test cell to receive STDF info.
    //QString	strString;
    //QStringList strTests;	// Used in Scatter plpot to extract list of couples to plot (X,Y)

    //QList<CTest*>		sortedWorstFailingTestsList;	// list containing the worst tests
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Error : GroupList does not contain any Group !");
        return GS::StdLib::Stdf::ErrorMemory;
    }
    /*
 if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
 {
  bool ok=false;
  int N=QString( m_pReportOptions->strAdvancedTestList ).section(' ',1,1).toInt(&ok);
  if (!ok)
   GSLOG(SYSLOG_SEV_WARNING, QString(" failed ot understand '%1'").arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData() );
  else
  {	GSLOG(SYSLOG_SEV_NOTICE, QString(" will consider only top %1 failed tests ").arg( N));
   RetrieveTopNWorstTests(N, pGroup->cMergedData.ptMergedTestList, sortedWorstFailingTestsList);
   GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 tests succesfully sorted.").arg( sortedWorstFailingTestsList.size() );
  }
 }
 */

    /*
 // List of CTest* to write in generated reports :
 // will be filled now but could be filled as soos as possible
 // should nt be empty
 // can be build from : all tests, Tests list, Top N failed tests, PAT tests,....
 QList<CTest*>		TestsListToReport;

 if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
 {
  bool ok=false;
  int N=QString( m_pReportOptions->strAdvancedTestList ).section(' ',1,1).toInt(&ok);
  if (!ok)
   GSLOG(SYSLOG_SEV_WARNING, QString(" failed to understand '%1'").arg( m_pReportOptions->strAdvancedTestList).toLatin1().constData() );
  else
  {
   GSLOG(SYSLOG_SEV_NOTICE, QString(" will consider only top %1 failed tests ").arg( N));
   RetrieveTopNWorstTests(N, pGroup->cMergedData.ptMergedTestList, TestsListToReport);
   GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 tests succesfully sorted.").arg( TestsListToReport.size() );
  }
 }
 else if (m_pReportOptions->strAdvancedTestList=="all")
 {
  ptTestCell = pGroup->cMergedData.ptMergedTestList;
  while(ptTestCell != NULL)
  {
   TestsListToReport.append(ptTestCell);
   ptTestCell = ptTestCell->GetNextTest();
  }
  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" All : %1 tests").arg( TestsListToReport.size() );
 }
 else // assuming it is a TestList
 {
  ptTestCell = pGroup->cMergedData.ptMergedTestList;
  while(ptTestCell != NULL)
  {
   if(m_pReportOptions->pGexAdvancedRangeList
      ->IsTestInList(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex))
   {
    TestsListToReport.append(ptTestCell);
   }
   ptTestCell = ptTestCell->GetNextTest();
  }
  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" TestsList : %1 tests saved.").arg( TestsListToReport.size() );
 }
 */


    // Reset HTML page counter
    iCurrentHtmlPage=0;
    if(m_pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
    {
        // In case no data match the filter, ensure first page is erased (hyperlink always exists on home page!)
        char	szString[2048];
        sprintf(szString,"%s/pages/advanced1.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
        unlink(szString);

        // Datalog is section openend at start of pass1: DO NOT RE-OPEN here!
        // But if not datalog...open section now !
        iStatus = PrepareSection_Advanced(true);
        if(iStatus != GS::StdLib::Stdf::NoError)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("PrepareSection_Advanced() returned '%1'. Quiting.")
                  .arg(iStatus).toLatin1().constData());
            return iStatus;
        }

        if ((m_pReportOptions->getAdvancedReport() != GEX_ADV_DISABLED) &&
                (m_pReportOptions->getAdvancedReport() != GEX_ADV_SHIFT) &&
                (m_pReportOptions->getAdvancedReport() != GEX_ADV_PROD_YIELD) &&
                (m_pReportOptions->getAdvancedReport() != GEX_ADV_PEARSON) &&
                (m_pReportOptions->getAdvancedReport() != GEX_ADV_PAT_TRACEABILITY) &&
                (m_pReportOptions->getAdvancedReport() != GEX_ADV_CORRELATION))
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
        }

    }

    // If this section has to be created, update steps in progress bar
    if((m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_ADVANCED) == 0)
    {
        // Let's update the progressbar size...
        iTotalSteps = ((2*m_pReportOptions->iFiles)+6) + (mTotalHistogramPages*MAX_HISTOGRAM_PERPAGE) + mTotalWafermapPages + m_pReportOptions->lAdvancedHtmlPages;
        // Shows process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,iTotalSteps,-1);	// Show process bar...updates steps count, add one new step 1
    }

    QString result;
    //int ntestcell=0; // count number of test found/handled

    foreach(GS::Gex::ReportUnit* ru, mReportUnits.values())
    {
        if (!ru)
            continue;
        //if (ru->GetKey()=="adv_shift")
        ru->CreatePages();
    }

    switch(m_pReportOptions->getAdvancedReport())
    {
    case GEX_ADV_DATALOG:
        break;
    case GEX_ADV_DISABLED:
        break;
    case GEX_ADV_GUARDBANDING:
        break;
    case GEX_ADV_FUNCTIONAL:
        result=WriteAdvHistoFunctional(m_pReportOptions);
        break;
    case GEX_ADV_HISTOGRAM:
        result=WriteAdvHisto(m_pReportOptions);
        if (result=="break")
            break;
        if (result=="GS::StdLib::Stdf::NoError")
            return GS::StdLib::Stdf::NoError;
        break;
    case GEX_ADV_PEARSON:			// Test to test correlation check (Pearson's correlation formula)
        result=WriteAdvPersonReport(m_pReportOptions);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Write Pearson report : %1").arg( result).toLatin1().constData());
        break;
    case GEX_ADV_PAT_TRACEABILITY:
//        WriteAdvPatTraceability();	// STDF.DTR record holding PAT-Man outlier report
        break;
    case GEX_ADV_PROD_YIELD:
        WriteAdvProductionYield();	// Production reports (Yield trend)
        break;
    case GEX_ADV_OUTLIER_REMOVAL:
        WriteAdvOutlierRemovalReport();	// Writes Post-Processing outlier report (not available under GUI, only thru scripting!)
        break;
    case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostics creates Trend Charts too!
        WriteAdvOptimizerDiagsPage();
        break;
    case GEX_ADV_SHIFT:
    case GEX_ADV_CHARAC_BOXWHISKER_CHART:
        break;

    case GEX_ADV_TREND:
        result=WriteAdvTrend(m_pReportOptions);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("WriteAdvTrend() : %1").arg( result).toLatin1().constData());
        break;
    case GEX_ADV_CORRELATION:	// Scatter
        WriteAdvScatter(m_pReportOptions);
        break;
    case GEX_ADV_PROBABILITY_PLOT:
        result=WriteAdvProbabilityPlot(m_pReportOptions);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" WriteAdvProbabilityPlot() : %1").arg( result).toLatin1().constData() );
        break;

    case GEX_ADV_MULTICHART:
        result=WriteAdvMultiChart(m_pReportOptions);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" WriteAdvMultiChart() : %1").arg( result).toLatin1().constData() );
        break;

    case GEX_ADV_BOXPLOT:
        result=WriteAdvBoxPlotEx(m_pReportOptions);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" WriteAdvBoxPlotEx() : %1").arg( result).toLatin1().constData() );
        break;

    case GEX_ADV_CANDLE_MEANRANGE: // BoxPlot

        int iMaxLinesPerPage=15;

        if(of=="PPT")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
            iMaxLinesPerPage = 11;	// Max of 11 charts per page under PowerPoint
        else if (of=="INTERACTIVE" || of=="HTML")
            iMaxLinesPerPage = 15;	// Max of 15 chart per page
        else if(of=="DOC"||of=="PDF"||of=="ODT")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            if (pf == "landscape")
                iMaxLinesPerPage = 12;	// Max of 12 chart per page
            else
                iMaxLinesPerPage = 21;	// Max of 21 chart per page
        }

        // Fill the sorting-list object with Statistics table...
        QList<CTest*> qtStatisticsList;
        // Move back to first group.
        pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        // Create statistics report for each test#...
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while(ptTestCell != NULL)
        {
            // Check if this test is part of the report...if valid add to the list!
            if(ptTestCell->iHtmlAdvancedPage > 0)
                qtStatisticsList.append(ptTestCell);

            // Next Test
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.

        // Have the list sorted!
        qSort(qtStatisticsList.begin(), qtStatisticsList.end(), compareCTest);

        // Read list based on sorting filter (defined by user in 'Options')
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        int iBoxPlotLine=0;
        foreach(ptTestCell, qtStatisticsList)
        {
            // If single group: table header is written every 15 lines ; if comparing files (group#>1): header is after EACH test block.
            if(iBoxPlotLine != 0 &&
               iBoxPlotLine + m_pReportOptions->iGroups > iMaxLinesPerPage &&
               m_pReportOptions->isReportOutputHtmlBased())
            {
                iBoxPlotLine = 0;	// Ensure we do not have more than about X lines before new line header.

                // Close table
                fprintf(hReportFile,"</table>\n");

                // Write this page as a slide (image)
                WritePageBreak();

                fprintf(hReportFile,"<table border=\"0\" cellspacing=\"0\" width=\"98%%\">\n");
            }

            // Write all box-plot lines for all groups of given test
            WriteAdvBoxPlotLines(pFile,ptTestCell,iBoxPlotLine);
        };	// Loop until all test cells read.
        break;
    }

    // Clear slide title...in case an empty slide is created
    SetPowerPointSlideName("End Of Advanced Reports");

    // Advanced (for Datalog) file must be closed LAST.
    iStatus = CloseSection_Advanced();
    return iStatus;
}
