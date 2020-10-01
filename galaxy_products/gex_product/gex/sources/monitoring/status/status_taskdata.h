#ifndef STATUS_TASKDATA_H
#define STATUS_TASKDATA_H

#include <time.h>
#include <stdio.h>
#include <QList>
#include <QString>
#include <QDir>
#include <QTime>
#include <QMap>
#include <QVariant>
#include "task_properties.h"

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;
class CGexMoTaskStatus;
class CGexMoLotsProcessed;
class GexMoStatusTaskData : public TaskProperties
{
public:
    GexMoStatusTaskData(QObject* parent);	// Constructor
    ~GexMoStatusTaskData();

    GexMoStatusTaskData& operator= (const GexMoStatusTaskData &copy);

    void  ExecuteStatusTask(CGexMoTaskStatus *ptTask,QStringList *ptFilesToRename);

    // Getters
    const QString title() const          {return m_strTitle;}
    bool  isOneWebPerDatabase() const    {return m_bOneWebPerDatabase;}
    const QString intranetPath() const   { return m_strIntranetPath; }
    const QString homePage() const       {return m_strHomePage;}
    const QString reportURL() const      {return m_strReportURL;}
    const QString reportHttpURL() const  {return m_strReportHttpURL;}

    // Setters
    void setTitle(const QString &strTitle);
    void setOneWebPerDatabase(bool bOneWebPerDB);
    void setIntranetPath(const QString &strIntranetPath);
    void setHomePage(const QString &strHomePage);
    void setReportURL(const QString &strReportURL);
    void setReportHttpURL(const QString &strReportHttpURL);

private:
    void	CreateStatusWebSite(CGexMoTaskStatus *ptTask,QString strDatabaseName,QString strWebChildName,QString strDateString);
    void	CreateStatusWebMasterHome(CGexMoTaskStatus *ptTask);
    bool	CreateReportPage(GexMoStatusTaskData *pStatusTask,QString strWebChildFolder,QString strWebChildName,int iPageType,QString strChildPage="");
    void	CloseReportPage(int iPageType,bool bIndexPages,int iHistoryLink);
    void	UpdatePastHistoryPage(GexMoStatusTaskData *pStatusTask,QString strWebChildFolder,QString strWebChildName,bool bIndexPages,int iHistoryPageType,QString strFolder);
    void	ReadLotsInserted(QString strDatabaseMonitoringLotsFolder,QString strDateString);

    // List of Lots processed during the day.
    QList<CGexMoLotsProcessed*>	m_lstGexMoLotsList;
    FILE *						m_hReportPage;			// Handle to HTML page to create.
    QString						m_strHtmlPageFolder;		// Folder where the HTML page is created.

    // Holds pointer to parent's list of temporary files to rename.
    QStringList *				m_pFilesToRename;

private:
    QString	m_strTitle;			// Task title.
    bool	m_bOneWebPerDatabase;// true if create one child-web site per database, false otherwise.
    QString	m_strIntranetPath;	// where to publish the HTML status pages.
    QString	m_strHomePage;		// Home page to create in Intranet.
    QString	m_strReportURL;		// Report's URL name, use to display in Emails if strReportHTTPURL is empty (hyperlink)
    QString	m_strReportHttpURL;	// Http Report's URL name to display in Emails (hyperlink)

public:
    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

// Hold all data for a given Lot+Sublot data file processed.
class CGexMoLotsProcessed
{
public:
    time_t	tStartTime;
    QString strProductID;
    QString	strLot;
    QString	strSubLot;
    QString	strWafer;
    QString	strNodeName;
    QString	strOperator;
    QString	strJobName;
    QString	strJobRev;
    long	lTotalGoodBins;
    long	lTotalParts;
};

#endif // STATUS_TASKDATA_H
