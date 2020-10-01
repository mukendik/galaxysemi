///////////////////////////////////////////////////////////
// gex_web: ExaminatorWeb related functions...
///////////////////////////////////////////////////////////

#include "gex_web.h"
#include "db_engine.h"
#include "report_options.h"
#include "gex_report.h"
#include "gex_database_entry.h"
#include "engine.h"

#include <QProgressBar>
#include <QListIterator>

// main
extern GexMainwindow *  pGexMainWindow;
extern CGexReport*		gexReport;			// Handle to report class
extern QProgressBar *	GexProgressBar;

#define GEXWEB_ASP_PATH		"../../../"
#define GEXWEB_ASP_IMG		"../../../help/images/"

///////////////////////////////////////////////////////////
// Content of the 'gexweb_status' status file. Can be any
// of the following lines:
//
// Reports the report generation progress:
//   gexweb_progress <Visible> <Percentage> <Progress> <TotalSteps>
//
// Reports the HTML page to display (once report is built):
//   gexweb_url <Url>
//
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexWeb::~GexWeb()
{
    // Empty list of reports.
    while(!pReportListEntries.isEmpty())
        delete pReportListEntries.takeFirst();
}

///////////////////////////////////////////////////////////
// Initializes the Status file to write into...
///////////////////////////////////////////////////////////
void GexWeb::setStatusFile(QString strFile)
{
    // Save status file that will be read by client Browser
    strStatusFile = strFile;
    // Build name of temporary status file name used when building new status file
    strNewStatusFile = strFile + ".new";
}

///////////////////////////////////////////////////////////
// Writes HTML sequence to manage the Mouse Hoover.
///////////////////////////////////////////////////////////
void GexWeb::WriteMouseHooverSequence(FILE *hHtmlPage,QString strImageName)
{
    fprintf(hHtmlPage,"onMouseOut =\"this.src='%s%s.png';\" ",GEXWEB_ASP_IMG,strImageName.toLatin1().constData());
    fprintf(hHtmlPage,"onMouseOver=\"this.src='%s%sbig.png';\" ",GEXWEB_ASP_IMG,strImageName.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Update the Status file with relevant string.
///////////////////////////////////////////////////////////
void GexWeb::WriteStatusFile(void)
{
    QFile fNew(strNewStatusFile);
    if(!fNew.open( QIODevice::WriteOnly ))
        return;	// Failed creating new status file.

    // Assign file I/O stream
    QTextStream hStatusFile(&fNew);

    // Fill file with status line previously built...
    hStatusFile << strStatusLine << endl;
    fNew.close();

    // Delete old status file (if it exists!)
    QFile fOld(strStatusFile);
    QTime	cTime;
    BOOL	bExit=false;
    int		iMaximumTry=3;
    if(fOld.exists() == true)
    {
        // Try up to 3 times to erase the file with little delay between each try.
        bExit = fOld.remove();
        while(bExit==false)
        {
            // Wait 3ms before trying again to erase the old status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = fOld.remove();
            iMaximumTry--;
            if(iMaximumTry == 0)
                return;	// Failed 3 times to erase old status file...then give up.
        };
    }

    // Success removing previous status file...so replace it with the new one!
    QDir cDir;
    bExit=false;
    iMaximumTry=3;
    if(cDir.rename(strNewStatusFile,strStatusFile) == false)
    {
        // Failed renaming file...wait a little and try again...up to 3 times, then give-up!
        while(bExit==false)
        {
            // Wait 3ms before trying again to rename the status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = cDir.rename(strStatusFile,strNewStatusFile);
            iMaximumTry--;
            if(iMaximumTry == 0)
                return;	// Failed 3 times to rename status file...then give up.
        };
    }
}

///////////////////////////////////////////////////////////
// Remove a report entry from the reports list
///////////////////////////////////////////////////////////
bool GexWeb::RemoveReportEntry(QString strReportName)
{
    // Update Report list file in <report_folder>
    QString strReportFile = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE + GEXWEB_RPTENTRY_DEFINITION;
    QString strNewReportFile = strReportFile + ".new";
    QString strString,strSection;
    bool bOriginalFileExists;

    // Open existing reports list file
    QFile fOldListFile(strReportFile);
    if(!fOldListFile.open(QIODevice::ReadOnly))
        bOriginalFileExists = false;	// First report to create, no reports list exist!
    else
        bOriginalFileExists = true;

    // Create new temporary list file.
    QFile fNewListFile(strNewReportFile);
    if(!fNewListFile.open( QIODevice::ReadOnly ))
    {
        fOldListFile.close();
        return false;	// Failed creating new report list file....quietly return.
    }

    // Copy original report list file until relevant report section found (if exists)
    QTextStream hOldListFile(&fOldListFile);	// Assign file I/O stream
    QTextStream hNewListFile(&fNewListFile);	// Assign file I/O stream

    // If reports list file exists...read it.
    if(bOriginalFileExists == true)
    {
        do
        {
            // Read one line from original reports list
            strString = hOldListFile.readLine();


            if(strString.startsWith("<report>") == true)
            {
                // Read next line (report 'Title'
                strString = hOldListFile.readLine();
                strSection = strString.section('=',1);
                strSection = strSection.trimmed();

                // if this section is matching the section to remove, skip its content in original file
                if(strSection == strReportName)
                {
                    do
                    {
                        strString = hOldListFile.readLine();
                    }
                    while((hOldListFile.atEnd() == false) && (strString.startsWith("</report>") == false));
                }
                else
                {
                    hNewListFile << "<report>" << endl;	// Copy it to new reports list under construction
                    hNewListFile << strString << endl;	// Copy it to new reports list under construction
                }
            }
            else
                hNewListFile << strString << endl;	// Copy it to new reports list under construction
        }
        while(hOldListFile.atEnd() == false);
    }

    fNewListFile.close();
    fOldListFile.close();

    // Erase destination file so we can rename temporary reports list to default name
    // Delete old status file (if it exists!)
    QTime	cTime;
    BOOL	bExit=false;
    int		iMaximumTry=3;
    if(bOriginalFileExists == true)
    {
        // Try up to 3 times to erase the file with little delay between each try.
        bExit = fOldListFile.remove();
        while(bExit==false)
        {
            // Wait 3ms before trying again to erase the old status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = fOldListFile.remove();
            iMaximumTry--;
            if(iMaximumTry == 0)
                return false;	// Failed 3 times to erase old status file...then give up.
        };
    }

    // Success removing previous reports list file...so replace it with the new one!
    QDir cDir;
    bExit=false;
    iMaximumTry=3;
    if(cDir.rename(strNewReportFile,strReportFile) == false)
    {
        // Failed renaming file...wait a little and try again...up to 3 times, then give-up!
        while(bExit==false)
        {
            // Wait 3ms before trying again to rename the status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = cDir.rename(strNewReportFile,strReportFile);
            iMaximumTry--;
            if(iMaximumTry == 0)
                return false;	// Failed 3 times to rename status file...then give up.
        };
    }

    return true;
}

///////////////////////////////////////////////////////////
// Create/Update the reports list file in the 'reports' home folder
///////////////////////////////////////////////////////////
bool GexWeb::UpdateReportsInfoFile(void)
{
    // Update Report list file in <report_folder>
    QString strReportFile = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE + GEXWEB_RPTENTRY_DEFINITION;
    QString strNewReportFile = strReportFile + ".new";
    QString strString,strSection;
    bool bOriginalFileExists;

    // Open existing reports list file
    QFile fOldListFile(strReportFile);
    if(!fOldListFile.open( QIODevice::ReadOnly ))
        bOriginalFileExists = false;	// First report to create, no reports list exist!
    else
        bOriginalFileExists = true;

    // Create new temporary list file.
    QFile fNewListFile(strNewReportFile);
    if(!fNewListFile.open( QIODevice::WriteOnly ))
    {
        fOldListFile.close();
        return false;	// Failed creating new report list file....quietly return.
    }

    // Copy original report list file until relevant report section found (if exists)
    QTextStream hOldListFile(&fOldListFile);	// Assign file I/O stream
    QTextStream hNewListFile(&fNewListFile);	// Assign file I/O stream

    // If reports list file exists...read it.
    if(bOriginalFileExists == true)
    {
        do
        {
            // Read one line from original reports list
            strString = hOldListFile.readLine();


            if(strString.startsWith("<report>") == true)
            {
                // Read next line (report 'Title'
                strString = hOldListFile.readLine();
                strSection = strString.section('=',1);
                strSection = strSection.trimmed();

                // if this section is matching the section to update, skip its content in original file
                if(strSection == gexReport->getReportOptions()->strReportTitle)
                {
                    do
                    {
                        strString = hOldListFile.readLine();
                    }
                    while((hOldListFile.atEnd() == false) && (strString.startsWith("</report>") == false));
                }
                else
                {
                    hNewListFile << "<report>" << endl;	// Copy it to new reports list under construction
                    hNewListFile << strString << endl;	// Copy it to new reports list under construction
                }
            }
            else
                hNewListFile << strString << endl;	// Copy it to new reports list under construction
        }
        while(hOldListFile.atEnd() == false);
    }

    // Append current report entry to the new reports list being created.
    hNewListFile << "<report>" << endl;
    hNewListFile << "Title=" << gexReport->getReportOptions()->strReportTitle << endl;
    hNewListFile << "Folder=" << gexReport->getReportOptions()->strReportNormalizedTitle << endl;
    time_t CurrentTime = time(NULL);
    hNewListFile << "Created=" << QString::number(CurrentTime) << endl;		// Creation date
    hNewListFile << "</report>" << endl;			// Start definition marker

    fNewListFile.close();
    fOldListFile.close();

    // Erase destination file so we can rename temporary reports list to default name
    // Delete old status file (if it exists!)
    QTime	cTime;
    BOOL	bExit=false;
    int		iMaximumTry=3;
    if(bOriginalFileExists == true)
    {
        // Try up to 3 times to erase the file with little delay between each try.
        bExit = fOldListFile.remove();
        while(bExit==false)
        {
            // Wait 3ms before trying again to erase the old status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = fOldListFile.remove();
            iMaximumTry--;
            if(iMaximumTry == 0)
                return false;	// Failed 3 times to erase old status file...then give up.
        };
    }

    // Success removing previous reports list file...so replace it with the new one!
    QDir cDir;
    bExit=false;
    iMaximumTry=3;
    if(cDir.rename(strNewReportFile,strReportFile) == false)
    {
        // Failed renaming file...wait a little and try again...up to 3 times, then give-up!
        while(bExit==false)
        {
            // Wait 3ms before trying again to rename the status file.
            cTime.start();
            while(cTime.elapsed() < 3) ;
            bExit = cDir.rename(strNewReportFile,strReportFile);
            iMaximumTry--;
            if(iMaximumTry == 0)
                return false;	// Failed 3 times to rename status file...then give up.
        };
    }

    return true;
}

///////////////////////////////////////////////////////////
// Updates status file to tell current Progress bar value.
// Status line: "gexweb_progress <Visible> <Percentage> <Progress> <TotalSteps>"
// Note: <Visible> = 1 if progress bar is visible, 0 if hidden
// <Percentage> is the percentage of the bar done (number in 0-100)
// <Progress> is the current step#, an integer number from 0...TotalSteps
// <TotalSteps> is the total steps to reach 100% of the steps
///////////////////////////////////////////////////////////
void GexWeb::ProgressBarStatus(void)
{
    if (!GexProgressBar)
        return;

    int iTotalSteps = GexProgressBar->maximum();
    int iProgress = GexProgressBar->value();
    int iVisible = (GexProgressBar->isVisible() == true)? 1: 0;
    int	iPercentage;
    if(iTotalSteps>0)
    {
        iPercentage = (int)((100.0F*(float)iProgress)/(float)iTotalSteps);
        if(iPercentage > 100)
            iPercentage = 100;
    }
    else
        iPercentage = 0;

    strStatusLine = strStatusLine.sprintf("gexweb_progress %d %d %d %d",
        iVisible,iPercentage, iProgress, iTotalSteps);

    // Writes StatusLine into the status file, so client Browser call read it.
    WriteStatusFile();
}

///////////////////////////////////////////////////////////
// Updates status file to tell current URL to show.
// Status line: "gexweb_url <Url>"
///////////////////////////////////////////////////////////
void GexWeb::UrlStatus(QString strUrl)
{
    if(strUrl.isEmpty() == true)
        return;

    // Make URL relative to the user $home folder!
    QString strPageUrl = QDir::toNativeSeparators(strUrl);
    if(strPageUrl.startsWith(pGexMainWindow->strExaminatorWebUserHome) == true)
        strPageUrl = strPageUrl.mid(1+pGexMainWindow->strExaminatorWebUserHome.length());

    // Build status line.
    strStatusLine = "gexweb_url " + strPageUrl;

    // Writes StatusLine into the status file, so client Browser call read it.
    WriteStatusFile();
}

///////////////////////////////////////////////////////////
// Import file to database: Build HTML page with list of failues.
///////////////////////////////////////////////////////////
bool GexWeb::htmlImportFailure(QString strShowPage,QStringList *pCorruptedFiles)
{
    QDir cDir;
    FILE *hImportReport = fopen(strShowPage.toLatin1().constData(),"w");
    if(hImportReport == NULL)
        return false;	// Failed creating report page...quietly exit.

    gexReport->WriteHeaderHTML(hImportReport,"#006699","#F8F8F8");	// Default: Text is blue, background is light yellow.
    fprintf(hImportReport,"<h1 align=\"left\"><font color=\"#006699\">ExaminatorWeb: Import data</font></h1><br>\n");
    fprintf(hImportReport,"<HR><br>\n");

    fprintf(hImportReport,"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"600\">\n");
    fprintf(hImportReport,"<tr>\n");
    fprintf(hImportReport,"<td width=\"100\">&nbsp;</td>\n");
    fprintf(hImportReport,"<td width=\"50\"><b>\n");
    fprintf(hImportReport,"<img border=\"0\" src=\"%shelp/images/sad_face.png\"></b></td>\n",GEXWEB_ASP_IMG);
    fprintf(hImportReport,"<td width=\"300\">\n");
    fprintf(hImportReport,"<p align=\"center\"><b>FAILED importing data files</b></td>\n");
    fprintf(hImportReport,"<td width=\"50\"><b>\n");
    fprintf(hImportReport,"<img border=\"0\" src=\"%shelp/images/sad_face.png\"></b></td>\n",GEXWEB_ASP_IMG);
    fprintf(hImportReport,"<td width=\"100\">&nbsp;</td>\n");
    fprintf(hImportReport,"</tr>\n");
    fprintf(hImportReport,"</table>\n<p>&nbsp;<p>\n");

    // Report list of files that didn't import
    fprintf(hImportReport,"Data import <b><font color=\"#FF0000\">FAILURE</font></b>. The following files couldn't be imported to the database:<br>&nbsp;\n");
    fprintf(hImportReport,"<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" width=\"80%%\" bordercolor=\"#000000\">\n");
    fprintf(hImportReport,"<tr>\n");
    fprintf(hImportReport,"<td width=\"30%%\" bgcolor=\"#FFFF99\" align=\"center\"><b>File name</b></td>\n");
    fprintf(hImportReport,"<td width=\"70%%\" bgcolor=\"#FFFF99\" align=\"center\"><b>Possible root cause</b></td>\n");
    fprintf(hImportReport,"</tr>\n");
    for ( QStringList::Iterator it = pCorruptedFiles->begin(); it != pCorruptedFiles->end(); ++it )
    {
        fprintf(hImportReport,"<tr>\n");
        cDir.setPath(*it);
        fprintf(hImportReport,"<td width=\"30%%\" align=\"center\">%s</td>",cDir.dirName().toLatin1().constData());
        fprintf(hImportReport,"<td width=\"70%%\">cause...unknown!</td>\n");
        fprintf(hImportReport,"</tr>\n");
    }
    fprintf(hImportReport,"</table>\n");
    // Close HTML page
    fprintf(hImportReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hImportReport,"</body>\n");
    fprintf(hImportReport,"</html>\n");
    fclose(hImportReport);

    return true;
}

///////////////////////////////////////////////////////////
// Build Database Admin HTML page with list of existing databases!
///////////////////////////////////////////////////////////
bool GexWeb::htmlBuildAdminDatabases(QString strPage)
{
    FILE *hHtmlPage = fopen(strPage.toLatin1().constData(),"w");
    QString strHeaderLines;

    if(hHtmlPage == NULL)
        return false;	// Failed creating HTML page...quietly exit.

    strHeaderLines =  "<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n";
    strHeaderLines += "<META HTTP-EQUIV=\"Expires\" CONTENT=\"-1\">\n";
    strHeaderLines += "<!--#include file=\"../../../_gex_top_else.asp\"-->\n";

    gexReport->WriteHeaderHTML(hHtmlPage,"#006699","#F8F8F8",strHeaderLines);	// Default: Text is Blue, light yellow background.
    fprintf(hHtmlPage,"<h1 align=\"left\"><font color=\"#006699\">Manage your Databases!</font></h1><br>\n");
    fprintf(hHtmlPage,"<HR><br>\n");

    fprintf(hHtmlPage,"&nbsp;<br>&nbsp;<br>\n");
    fprintf(hHtmlPage,"<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"95%%\">\n");
    // Column titles
    fprintf(hHtmlPage,"<tr>\n");
    fprintf(hHtmlPage,"<td width=\"31%%\" bgcolor=\"#FFCC99\"><b>&nbsp;&nbsp; Database name</b></td>\n");
    fprintf(hHtmlPage,"<td width=\"11%%\" bgcolor=\"#FFCC99\" align=\"center\">\n");
    fprintf(hHtmlPage,"<p align=\"center\"><b>Size</b></td>\n");
    fprintf(hHtmlPage,"<td width=\"46%%\" bgcolor=\"#FFCC99\"><b>&nbsp;&nbsp; Description</b></td>\n");
    fprintf(hHtmlPage,"<td width=\"12%%\" bgcolor=\"#FFCC99\" align=\"center\">\n");
    fprintf(hHtmlPage,"<p align=\"center\"><b>Actions</b></td>\n");
    fprintf(hHtmlPage,"</tr>\n");

    // For each existing database, report its details:
    QString strString;
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty();

    QList<GexDatabaseEntry*>::iterator itBegin	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();

    while(itBegin != itEnd)
    {
        // Write database entry details.
        fprintf(hHtmlPage,"<tr>\n");
        fprintf(hHtmlPage,"<td width=\"31%%\"><b>&nbsp;&nbsp;%s</b></td>\n", (*itBegin)->LogicalName().toLatin1().constData());

        strString = GS::Gex::Engine::GetInstance().GetDatabaseEngine().BuildDatabaseSize((*itBegin)->CacheSize(), (*itBegin)->IsExternal());
        fprintf(hHtmlPage,"<td width=\"11%%\" align=\"center\">%s</td>\n",strString.toLatin1().constData());

        strString = "<td width=\"46%%\"><b>&nbsp;&nbsp;" + (*itBegin)->Description();
        strString += "</b></td>\n";
        fprintf(hHtmlPage,"%s",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<td width=\"12%%\" align=\"center\">\n");
        fprintf(hHtmlPage,"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"100%%\">\n");
        fprintf(hHtmlPage,"<tr>\n");
        fprintf(hHtmlPage,"<td width=\"50%%\" align=\"center\">\n");

        // Build hyperlink name
        strString = GEXWEB_ASP_PATH;
        strString += "dispatch.asp?task=db_import&name=" + (*itBegin)->LogicalName();
        fprintf(hHtmlPage,"<a href=\"%s\">\n",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<img border=\"0\" src=\"%sfile_open.png\" ",GEXWEB_ASP_IMG);

        WriteMouseHooverSequence(hHtmlPage,"file_open");

        fprintf(hHtmlPage,"width=\"26\" height=\"25\" alt=\"Add data files to the database\"></a></td>\n");
        fprintf(hHtmlPage,"<td width=\"50%%\" align=\"center\">\n");

        // Build hyperlink name
        strString = GEXWEB_ASP_PATH;
        strString += "dispatch.asp?task=db_delete&name=" + (*itBegin)->LogicalName();
        fprintf(hHtmlPage,"<a href=\"%s\">\n",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<img border=\"0\" src=\"%sfile_remove.png\" ",GEXWEB_ASP_IMG);

        WriteMouseHooverSequence(hHtmlPage,"file_remove");

        fprintf(hHtmlPage,"width=\"26\" height=\"25\" alt=\"Delete database\"></a></td>\n");
        fprintf(hHtmlPage,"</tr>\n");
        fprintf(hHtmlPage,"</table>\n");
        fprintf(hHtmlPage,"</td>\n");
        fprintf(hHtmlPage,"</tr>\n");

        // Move to next entry.
        itBegin++;
    };

    fprintf(hHtmlPage,"</table>\n");

    // Text + Link to create an additional database.
    fprintf(hHtmlPage,"&nbsp;<p>To <b>create a new database,</b> use the following link:<br>\n");
    fprintf(hHtmlPage,"<table border=\"0\" width=\"95%%\">\n");
    fprintf(hHtmlPage,"<tr>\n");
    fprintf(hHtmlPage,"<td width=\"15%%\">\n");
    fprintf(hHtmlPage,"<a target=\"_blank\" href=\"http://www.mentor.com/examinator\">\n");
    fprintf(hHtmlPage,"<img border=\"0\" src=\"%sassistant.png\" alt=\"ExaminatorWeb: Online help!\"></a></td>\n",GEXWEB_ASP_IMG);
    fprintf(hHtmlPage,"<td width=\"85%%\" align=\"left\" valign=\"top\"><br>\n");
    fprintf(hHtmlPage,"<br>\n");
    fprintf(hHtmlPage,"<img alt src=\"%srarrow.png\" border=\"0\" width=\"6\" height=\"9\">&nbsp;\n",GEXWEB_ASP_IMG);
    fprintf(hHtmlPage,"<b><a href=\"%sgexweb_create_db.asp\">Create new database</a></b>\n",GEXWEB_ASP_PATH);
    fprintf(hHtmlPage,"<br>&nbsp; - Create a database and post as many data files as you want!</td>\n");
    fprintf(hHtmlPage,"</tr>\n");
    fprintf(hHtmlPage,"<tr>\n");
    fprintf(hHtmlPage,"<td width=\"15%%\">\n");
    fprintf(hHtmlPage,"<p align=\"center\"><a target=\"_blank\" href=\"http://support.galaxysemi.com\">Need a hint?</a></td>\n");
    fprintf(hHtmlPage,"<td width=\"85%%\"></td>\n");
    fprintf(hHtmlPage,"</tr>\n");
    fprintf(hHtmlPage,"</table>\n");

    // Close HTML page
    fprintf(hHtmlPage,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hHtmlPage,"</body>\n");
    fprintf(hHtmlPage,"</html>\n");
    fclose(hHtmlPage);

    return true;
}

///////////////////////////////////////////////////////////
// Build Reports Admin HTML page with list of existing reports!
///////////////////////////////////////////////////////////
bool GexWeb::htmlBuildAdminReports(QString strPage)
{
    QString strHeaderLines,strString,strSection;

    // Read list of reports file and fill structures...
    QString strReportFile = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE + GEXWEB_RPTENTRY_DEFINITION;
    QFile fReportsListFile(strReportFile);
    if(!fReportsListFile.open( QIODevice::ReadOnly ))
        return false;	// Failed reading list...

    QTextStream hReportsListFile(&fReportsListFile);	// Assign file I/O stream
    GexReportListEntry cReportListEntry,*pReportListEntry;

    // Empty list of reports.
    while(!pReportListEntries.isEmpty())
        delete pReportListEntries.takeFirst();

    do
    {
        // Read one line from original reports list
        strString = hReportsListFile.readLine();

        if(strString.startsWith("<report>") == true)
        {
            do
            {
                strString = hReportsListFile.readLine();
                strSection = strString.section('=',1);
                strSection = strSection.trimmed();

                if(strString.startsWith("Title=")==true)
                    cReportListEntry.strReportTitle = strSection;
                if(strString.startsWith("Folder=")==true)
                    cReportListEntry.strReportFolder = strSection;
                if(strString.startsWith("Created=")==true)
                    cReportListEntry.cCreationDate = strSection.toInt();
            }
            while((hReportsListFile.atEnd() == false) && (strString.startsWith("</report>") == false));
            if(strString.startsWith("</report>") == true)
            {
                // Add report entry to list.
                pReportListEntry = new GexReportListEntry();
                *pReportListEntry = cReportListEntry;
                pReportListEntries.append(pReportListEntry);
            }
        }
    }
    while(hReportsListFile.atEnd() == false);
    fReportsListFile.close();

    // Create HTML Report table:
    // <Creation date> <Report title> <Actions>
    // Actions include: Download (.zip the folder), Delete

    FILE *hHtmlPage = fopen(strPage.toLatin1().constData(),"w");
    if(hHtmlPage == NULL)
        return false;	// Failed creating HTML page...quietly exit.

    strHeaderLines =  "<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n";
    strHeaderLines += "<META HTTP-EQUIV=\"Expires\" CONTENT=\"-1\">\n";
    strHeaderLines += "<!--#include file=\"../../../_gex_top_else.asp\"-->\n";

    gexReport->WriteHeaderHTML(hHtmlPage,"#006699","#F8F8F8",strHeaderLines);	// Default: Text is Blue, light yellow background.
    fprintf(hHtmlPage,"<h1 align=\"left\"><font color=\"#006699\">Manage your Reports!</font></h1><br>\n");
    fprintf(hHtmlPage,"<HR><br>\n");

    fprintf(hHtmlPage,"&nbsp;<br>&nbsp;<br>\n");
    fprintf(hHtmlPage,"<table border=\"1\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"80%%\">\n");
    // Column titles
    fprintf(hHtmlPage,"<tr>\n");
    fprintf(hHtmlPage,"<td width=\"30%%\" bgcolor=\"#FFCC99\" align=\"center\">\n");
    fprintf(hHtmlPage,"<p align=\"center\"><b>Creation date</b></td>\n");
    fprintf(hHtmlPage,"<td width=\"60%%\" bgcolor=\"#FFCC99\"><b>&nbsp;&nbsp; Report name</b></td>\n");
    fprintf(hHtmlPage,"<td width=\"10%%\" bgcolor=\"#FFCC99\" align=\"center\">\n");
    fprintf(hHtmlPage,"<p align=\"center\"><b>Actions</b></td>\n");
    fprintf(hHtmlPage,"</tr>\n");

    // For each existing report entry, report its details:
    QDateTime cReportDataTime;

    QListIterator<GexReportListEntry*> lstIteratorReportListEntries(pReportListEntries);

    while(lstIteratorReportListEntries.hasNext())
    {
        pReportListEntry = lstIteratorReportListEntries.next();

        // Write database entry details.
        fprintf(hHtmlPage,"<tr>\n");
        // Created...
        cReportDataTime.setTime_t(pReportListEntry->cCreationDate);
        strString = cReportDataTime.toString("d MMM yyyy h:mm:ss ap");
        fprintf(hHtmlPage,"<td width=\"30%%\" align=\"center\">%s</td>\n",strString.toLatin1().constData());

        // Report name + link
        strString = "../reports/" + pReportListEntry->strReportFolder + "/index.htm";
        fprintf(hHtmlPage,"<td width=\"60%%\">&nbsp;&nbsp;<a href=\"%s\">\n",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<b>%s</b></a></td>\n",pReportListEntry->strReportTitle.toLatin1().constData());

        // Actions
        fprintf(hHtmlPage,"<td width=\"10%%\" align=\"center\">\n");
        fprintf(hHtmlPage,"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"100%%\">\n");
        fprintf(hHtmlPage,"<tr>\n");
        fprintf(hHtmlPage,"<td width=\"50%%\" align=\"center\">\n");
        // Build hyperlink name
        strString = GEXWEB_ASP_PATH;
        strString += "dispatch.asp?task=rpt_zip&name=" + pReportListEntry->strReportTitle;
        fprintf(hHtmlPage,"<a href=\"%s\">\n",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<img border=\"0\" src=\"%szip.png\" ",GEXWEB_ASP_IMG);
        WriteMouseHooverSequence(hHtmlPage,"zip");
        fprintf(hHtmlPage,"width=\"26\" height=\"25\" alt=\"Zip report and download it now!\"></a></td>\n");
        fprintf(hHtmlPage,"<td width=\"50%%\" align=\"center\">\n");
        // Build hyperlink name
        strString = GEXWEB_ASP_PATH;
        strString += "dispatch.asp?task=rpt_delete&name=" + pReportListEntry->strReportTitle;
        fprintf(hHtmlPage,"<a href=\"%s\">\n",strString.toLatin1().constData());
        fprintf(hHtmlPage,"<img border=\"0\" src=\"%sfile_remove.png\" ",GEXWEB_ASP_IMG);
        WriteMouseHooverSequence(hHtmlPage,"file_remove");
        fprintf(hHtmlPage,"width=\"26\" height=\"25\" alt=\"Delete report\"></a></td>\n");
        fprintf(hHtmlPage,"</tr>\n");
        fprintf(hHtmlPage,"</table>\n");
        fprintf(hHtmlPage,"</td>\n");
        fprintf(hHtmlPage,"</tr>\n");
    };

    fprintf(hHtmlPage,"</table>\n");

    // Close HTML page
    fprintf(hHtmlPage,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hHtmlPage,"</body>\n");
    fprintf(hHtmlPage,"</html>\n");
    fclose(hHtmlPage);

    return true;
}

