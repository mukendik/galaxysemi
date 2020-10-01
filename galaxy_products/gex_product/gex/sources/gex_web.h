///////////////////////////////////////////////////////////
// ExaminatorWeb class (handles I/O stream between
// server and client
///////////////////////////////////////////////////////////

#ifndef GEXWEB_H
#define GEXWEB_H

#include <time.h>
#include "browser_dialog.h"
#include "report_build.h"
#include "gex_constants.h"

// in main.cpp
extern QProgressBar	*	GexProgressBar;	// Handle to progress bar in status bar

class GexReportListEntry
{
public:
    time_t	cCreationDate;
    QString	strReportTitle;
    QString	strReportFolder;
};

class GexWeb
{
public:

    ~GexWeb();

    void	setStatusFile(QString);
    void	ProgressBarStatus(void);
    bool	RemoveReportEntry(QString strReportName);
    bool	UpdateReportsInfoFile(void);
    void	UrlStatus(QString strUrl);
    bool	htmlImportFailure(QString strShowPage,QStringList *pCorruptedFiles); // Build HTML page that reports files that failed going to the database.
    bool	htmlBuildAdminDatabases(QString strPage);
    bool	htmlBuildAdminReports(QString strPage);
private:
    QString	strStatusFile;		// Holds full path to the Status file to create/update.
    QString	strNewStatusFile;	// Holds next status file being built to replace previous one...
    QString	strStatusLine;		// Used to build string to dump into status file
    void	WriteMouseHooverSequence(FILE *hHtmlPage,QString strImageName);
    void	WriteStatusFile();	// Dumps strStatusLine into status file.
    QList<GexReportListEntry*>	pReportListEntries;	// List of Reports found
};



#endif
