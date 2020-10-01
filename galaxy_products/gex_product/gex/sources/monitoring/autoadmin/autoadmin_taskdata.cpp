#include <gqtl_log.h>

#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "autoadmin_taskdata.h"
#include "autoadmin_task.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "engine.h"
#include "product_info.h"
#include "message.h"
#include "admin_engine.h"

GexMoAutoAdminTaskData::GexMoAutoAdminTaskData(QObject* parent): TaskProperties(parent)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "new GexMoAutoAdminTaskData...");
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        mEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        mEmailFrom = GEX_EMAIL_YIELD_MAN;
    mSendInsertionLogs = false;
    mLastInsertionLogsSent = QDate::currentDate().addDays(-1);
}

void GexMoAutoAdminTaskData::UpdatePrivateAttributes()
{
    // Insert new values
    SetPrivateAttribute("Title",mTitle);
    SetPrivateAttribute("StartTime", QString::number(mStartTime.hour()) + "," + QString::number(mStartTime.minute()));
    SetPrivateAttribute("Emails",mEmailNotify);
    SetPrivateAttribute("EmailFrom",mEmailFrom);
    SetPrivateAttribute("KeepReportDuration",QString::number(mKeepReportDuration));
    SetPrivateAttribute("SendInsertionLogs",QString::number(mSendInsertionLogs));
    SetPrivateAttribute("LastInsertionLogsSent",QString::number(QDateTime(mLastInsertionLogsSent).toTime_t()));
    SetPrivateAttribute("LogContents",QString::number(mLogContents));
}

GexMoAutoAdminTaskData &GexMoAutoAdminTaskData::operator=(const GexMoAutoAdminTaskData &copy)
{
    if(this != &copy)
    {
        mTitle                = copy.mTitle;
        mKeepReportDuration     = copy.mKeepReportDuration;
        mLogContents            = copy.mLogContents;
        mStartTime              = copy.mStartTime;
        mEmailFrom            = copy.mEmailFrom;
        mEmailNotify          = copy.mEmailNotify;
        mSendInsertionLogs    = copy.mSendInsertionLogs;
        mLastInsertionLogsSent= copy.mLastInsertionLogsSent;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();

    }
    return *this;
}

QString GS::Gex::SchedulerEngine::ExecuteAutoAdminTask(CGexMoTaskAutoAdmin *ptTask)
{
    if (ptTask->GetProperties()->mSendInsertionLogs)
    {
        // Check if we have logs to send
        if ((ptTask->m_tLastExecuted == 1) ||
                (mLastCheckScheduler.isValid() && (mLastCheckScheduler != ptTask->GetProperties()->mLastInsertionLogsSent)))
        {
            sendInsertionLogsMail(ptTask->GetProperties(), mLastCheckScheduler.addDays(-1));
            if(!SaveDbTasks(ptTask))
            {
                QString strError;
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                        && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
                    GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(strError);
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && strError.isEmpty())
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
                GSLOG(4, QString("SaveDbTasks() returns an error [%1]").arg(strError).toLatin1().constData());
            }
        }
    }

    QDateTime	cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();

    // Debug message
    QString strString;
    strString = "Executing AutoAdmin Task ";
    strString += ptTask->GetProperties()->mTitle;
    strString += " (";
    strString += cCurrentDateTime.toString("hh:mm:ss")+"): ";
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    onStartTask(ptTask);

    // Build Path to Monitoring Reports folder
    QString strReportFolder;
    QString	strReportURL;
    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask)
        strReportURL = ptStatusTask->GetProperties()->reportURL();

    if(strReportURL.isEmpty() || strReportURL.toLower() == "default" || strReportURL.toLower() == "(default)")
    {
        // No Web task defined or no custom report folder defined : use default report folder
        strReportFolder = ReportOptions.GetServerDatabaseFolder(true);
        // Make sure folder path doesn't end with a '/' or '\\'
        if((strReportFolder.endsWith("/") == true) || (strReportFolder.endsWith("\\")==true))
            strReportFolder.truncate(strReportFolder.length()-1);
        strReportFolder += GEX_DATABASE_REPORTS_MO;
    }
    else
    {
        // Web status task exists: use report URL
        strReportFolder = strReportURL;
        if((strReportFolder.endsWith("/") == false) && (strReportFolder.endsWith("\\")==false))
            strReportFolder += "/";
    }

    strString += " in "+strReportFolder;
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    // If we're in a sub-folder, check if we have any files in it (if so, we can't erase it).
    QDir d;
    QString		strFilePath;
    QStringList strDatabaseEntries;
    QStringList::Iterator it;
    QFileInfo cInfo;
    QDateTime	cFileCreation;
    int			iDays;
    bool		bErase;

    // Find All files & folders present....and check creation date
    d.setPath(strReportFolder);
    d.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
    strDatabaseEntries = d.entryList(QDir::nameFiltersFromString("*"));

    for(it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        // If files in this sub-folder, can't erase it...
        strFilePath = strReportFolder + "/" + *it;
        if((*it != ".") && (*it != ".."))
        {
            cInfo.setFile(strFilePath);
            cFileCreation = cInfo.lastModified();
            // Compute number of days elapsed since this file/folder creation.
            iDays = cFileCreation.daysTo(GS::Gex::Engine::GetInstance().GetClientDateTime());
        }
        else
            iDays = 0;

        // Reset Erase flag
        bErase = false;

        // Erase any file & folder older than the specified number of days/weeks/months
        switch(ptTask->GetProperties()->mKeepReportDuration)
        {
        case GEXMO_RPTERASE_FREQ_1DAY:			// Erase reports older than: 1 day
            if(iDays > 1)
                bErase = true;
            break;

        case GEXMO_RPTERASE_FREQ_2DAY:			// Erase reports older than: 2 days
            if(iDays > 2)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_3DAY:		// Erase reports older than: 3 days
            if(iDays > 3)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_4DAY:		// Erase reports older than: 4 days
            if(iDays > 4)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_5DAY:		// Erase reports older than: 5 days
            if(iDays > 5)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_6DAY:		// Erase reports older than: 6 days
            if(iDays > 6)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_1WEEK:		// Erase reports older than: 1 week
            if(iDays > 7)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_2WEEK:		// Erase reports older than: 2 weeks
            if(iDays > 14)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_3WEEK:		// Erase reports older than: 3 weeks
            if(iDays > 21)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_1MONTH:		// Erase reports older than: 1 month
            if(iDays > 31)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_2MONTH:		// Erase reports older than: 2 month
            if(iDays > 62)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_3MONTH:		// Erase reports older than: 3 month
            if(iDays > 93)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_4MONTH:		// Erase reports older than: 4 month
            if(iDays > 124)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_5MONTH:		// Erase reports older than: 5 month
            if(iDays > 156)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_6MONTH:		// Erase reports older than: 6 month
            if(iDays > 187)
                bErase = true;
            break;

        case	GEXMO_RPTERASE_FREQ_NEVER:		// Never erase reports on the server
            break;
        }

        // If this file/folder is too old, we remove it
        if(bErase)
        {
            // Remove folder (if this is a folder)
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strFilePath);
            // Remove file (if it wasn't a folder!)
            GS::Gex::Engine::RemoveFileFromDisk(strFilePath);
        }
    }

    onStopTask(ptTask);

    return "";
}
