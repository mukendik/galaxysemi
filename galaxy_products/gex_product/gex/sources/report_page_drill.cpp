/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Snapshots Gallery' pages
/////////////////////////////////////////////////////////////////////////////

#include "report_build.h"
#include "report_options.h"
#include "snapshot_dialog.h"
#include "gex_report.h"

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_Drill(BOOL /*bValidSection*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Snapshot gallery' home page & header
    if (strOutputFormat=="CSV")	//if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Snapshot Gallery ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
    if (m_pReportOptions->isReportOutputHtmlBased())
            //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating .HTML report file: create empty DRILL page if it doesn't exist yet!
          SnapShotReport cSnapReport;
          cSnapReport.CreateHomePage();
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Drill(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased())
        //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        // Generating .HTML report file: no specific task to perform when section written !
    }
    return GS::StdLib::Stdf::NoError;
}


