#include <QTextBlock>
#include "snapshot_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "stdf.h"
#include "gex_report.h"
#include "engine.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

// main.cpp
extern CGexReport*		gexReport;			// Handle to report class

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
SnapshotDialog::SnapshotDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()), this, SLOT(reject()));

    // Set focus on title field.
    snapshotTitle->setFocus();
}


///////////////////////////////////////////////////////////
// Create DRILL home page if it doesn't exist (drill.htm)
///////////////////////////////////////////////////////////
int SnapShotReport::CreateHomePage(void)
{
    char	szString[2048];

    // No data analyzed yet!
    if(gexReport == NULL)
        return GS::StdLib::Stdf::ReportFile;

    // Open <stdf-filename>/drill/drill.htm
    sprintf(szString,"%s/drill/drill.htm",gexReport->getReportOptions()->strReportDirectory.toLatin1().constData());
    hSnapShotReport = fopen(szString,"r");
    if(hSnapShotReport != NULL)
    {
        // SnapShot page already exists...no need to create it!
        fclose(hSnapShotReport);
        hSnapShotReport = NULL;
        return GS::StdLib::Stdf::NoError;
    }

    // SnapShot page doesn't exist...must create it!
    hSnapShotReport = fopen(szString,"wt");
    if(hSnapShotReport == NULL)
        return GS::StdLib::Stdf::ReportFile;

    gexReport->WriteHeaderHTML(hSnapShotReport,"#000000");	// Default: Text is Black
    fprintf(hSnapShotReport,"<h1 align=\"left\"><font color=\"#006699\">Interactive drill: Snapshots Gallery...</font></h1><br>\n");
    fprintf(hSnapShotReport,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br></p>\n");
    fprintf(hSnapShotReport,"<p align=\"left\">&nbsp;</p>\n");
    fprintf(hSnapShotReport,"<table border=\"0\" width=\"750\" height=\"74\" cellspacing=\"1\">\n");
    fprintf(hSnapShotReport,"<tr><td width=\"300\" bgcolor=%s><b>Snapshot</b></td>\n",szFieldColor);
    fprintf(hSnapShotReport,"<td width=\"450\" bgcolor=%s><b>Notes</b></td></tr>\n",szDataColor);
    // Write HTML comment marker 'GEX_DRILL_END_TABLE %PageID' . This marker is used for dynamically adding links in this page
    fprintf(hSnapShotReport,"<!-- [GEX_DRILL_END_TABLE 1 ] -->\n");
    fprintf(hSnapShotReport,"<tr><td width=\"300\" bgcolor=%s><b>-</b></td>\n",szFieldColor);
    fprintf(hSnapShotReport,"<td width=\"450\" bgcolor=%s>-</td></tr>\n",szDataColor);
    fprintf(hSnapShotReport,"</table>\n");
    fprintf(hSnapShotReport,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">The Snapshots Gallery is empty!<br>\n",3);
    fprintf(hSnapShotReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hSnapShotReport,"</body>\n");
    fprintf(hSnapShotReport,"</html>\n");
    fclose(hSnapShotReport);
    hSnapShotReport = NULL;


    return GS::StdLib::Stdf::NoError;
}

///////////////////////////////////////////////////////////
// Add a link entry to the snapshot home page (drill.htm)
///////////////////////////////////////////////////////////
int SnapShotReport::AddLink(QString strLinkName, QTextEdit* ptNotes)
{
    char	szString[2048]="";
    char	szFileOld[2048]="";
    char	szFileNew[2048]="";
    char	*ptChar=0;
    FILE	*hNewHomePage=0;
    int		iLine=0;

    // Will include the Drill Page ID to use for creating the snapshot page.
    iSnapShotPageID = -1;

    // No data analyzed yet!
    if(gexReport == NULL)
        return GS::StdLib::Stdf::ReportFile;

    // Create Home page if it doesn't exist.
    if(CreateHomePage() != GS::StdLib::Stdf::NoError)
        return GS::StdLib::Stdf::ReportFile;

    // Open <stdf-filename>/drill/drill.htm
    sprintf(szFileOld,"%s/drill/drill.htm",gexReport->getReportOptions()->strReportDirectory.toLatin1().constData());
    hSnapShotReport = fopen(szFileOld,"rt");
    if(hSnapShotReport == NULL)
        return GS::StdLib::Stdf::ReportFile;

    // Create <stdf-filename>/drill/drill_new.htm
    sprintf(szFileNew,"%s/drill/drill_new.htm",gexReport->getReportOptions()->strReportDirectory.toLatin1().constData());
    hNewHomePage = fopen(szFileNew,"wt");
    while(!feof(hSnapShotReport))
    {
        // Line counter...used to decide the snapshot pageID to create!
        iLine++;

        // Get 1 line from original Drill home page until market 'GEX_DRILL_END_TABLE' is found
        if(fgets(szString,2047,hSnapShotReport) == NULL)
            break;	// end of file reached...
        ptChar = strstr(szString,"[GEX_DRILL_END_TABLE");
        if(ptChar != NULL)
        {
            // Save PageID that this snaphot link will point to.
            if(sscanf(ptChar,"%*s %d",&iSnapShotPageID) != 1)
                iSnapShotPageID = 1;

            // Write link name+hyperlink to its page + Notes string
            fprintf(hNewHomePage,"<tr><td width=\"300\" bgcolor=%s><b><a href=\"d%d.htm\">%s</a></b></td>\n",szFieldColor,iSnapShotPageID,strLinkName.toLatin1().constData());
            fprintf(hNewHomePage,"<td width=\"450\" bgcolor=%s>",szDataColor);
            // Write the Notes lines
            int iTotalLines = ptNotes->document()->blockCount();
            for(int iLine=0; iLine < iTotalLines; iLine++)
            {
                fprintf(hNewHomePage,"<align=\"left\"><font color=\"#000000\" size=\"%d\">%s<br>\n",3, ptNotes->document()->findBlockByNumber(iLine).text().toLatin1().constData());
            }
            fprintf(hNewHomePage,"</td></tr>\n");


            // Write end of list marker + end of HTML page.
            fprintf(hNewHomePage,"<!-- [GEX_DRILL_END_TABLE %d ] -->\n",iSnapShotPageID+1);
            fprintf(hNewHomePage,"</table>\n");
            fprintf(hNewHomePage,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hNewHomePage,"</body>\n");
            fprintf(hNewHomePage,"</html>\n");
            break;
        }

        // Copy line from old Drill home page to new one...
        fputs(szString,hNewHomePage);
    };

    fclose(hSnapShotReport);
    fclose(hNewHomePage);
    hSnapShotReport = NULL;

    // rename 'drill_new.htm' into 'drill.htm', then remove 'drill_new.htm'
    unlink(szFileOld);
    if(rename(szFileNew,szFileOld))
        return GS::StdLib::Stdf::ReportFile;
    return GS::StdLib::Stdf::NoError;
}

///////////////////////////////////////////////////////////
// Add a page for a snapshot
///////////////////////////////////////////////////////////
FILE *SnapShotReport::AddPage(QString strLinkName)
{
    // No data analyzed yet!
    if(gexReport == NULL)
        return NULL;

    // No Link created on Home page...wrong calling sequence!
    if(iSnapShotPageID == -1)
        return NULL;

    char	szString[2048];
    // Open <stdf-filename>/drill/dXXX.htm
    sprintf(szString,"%s/drill/d%d.htm",gexReport->getReportOptions()->strReportDirectory.toLatin1().constData(),iSnapShotPageID);
    hSnapShotReport = fopen(szString,"wt");
    if(hSnapShotReport == NULL)
        return NULL;

    gexReport->WriteHeaderHTML(hSnapShotReport,"#000000");	// Default: Text is Black
    fprintf(hSnapShotReport,"<h1 align=\"left\"><font color=\"#006699\">Snapshot: %s</font></h1><br>\n",strLinkName.toLatin1().constData());
    fprintf(hSnapShotReport,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br></p>\n");
    fprintf(hSnapShotReport,"<p align=\"left\">&nbsp;</p>\n");

    return hSnapShotReport;
}

///////////////////////////////////////////////////////////
// Close a page for a snapshot
///////////////////////////////////////////////////////////
void SnapShotReport::ClosePage(QTextEdit* ptNotes)
{
    if(hSnapShotReport == NULL)
        return;

    fprintf(hSnapShotReport,"<br>\n<br>\n<hr>\n");
    fprintf(hSnapShotReport,"<br>\n<p><b>Notes:</b></p>\n");
    // Write the Notes lines
    int iTotalLines = ptNotes->document()->blockCount();
    for(int iLine=0; iLine < iTotalLines; iLine++)
    {
        fprintf(hSnapShotReport,"<align=\"left\"><font color=\"#000000\" size=\"%d\">%s<br>\n",3, ptNotes->document()->findBlockByNumber(iLine).text().toLatin1().constData());
    }

    // Close HTML section
    fprintf(hSnapShotReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hSnapShotReport,"</body>\n");
    fprintf(hSnapShotReport,"</html>\n");

    // Close file
    fclose(hSnapShotReport);
    hSnapShotReport= NULL;
}
