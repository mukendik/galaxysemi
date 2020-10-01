///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined unix || __MACH__
#include <unistd.h>
#include <errno.h>
#elif defined(WIN32)
#include <io.h>
#endif

#include <QRadioButton>
#include <QRegExp>
#include <QApplication>
#include <QDesktopWidget>
#include <QProgressBar>
#include <QSqlError>
#include <QListIterator>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "datapump/olddatapump_task.h"
#include "datapump/datapump_task.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "reporting/reporting_task.h"
#include "reporting/reporting_taskdata.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_task.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_task.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_task.h"
#include "converter/converter_taskdata.h"
#include "spm/spm_task.h"
#include "sya/sya_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include "pat/pat_task.h"
#include "trigger/trigger_task.h"
#include "scheduler_engine.h"
#include "tasks_manager_engine.h"
#include "mo_task.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "sya_task.h"

#include "mo_email.h"
#include "admin_engine.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// report_build.cpp
extern CReportOptions   ReportOptions;      // Holds options (report_build.h)

// in main.cpp
extern void             WriteDebugMessageFile(const QString & strMessage);
extern GexScriptEngine* pGexScriptEngine;

extern QProgressBar *           GexProgressBar;         // Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Make sure the qApplication->processEvent function is not called too often
//////////////////////////////////////////////////////////////////////
QTime   g_clLastProcessEvents;       // Time used to store last time ProcessEvents was executed
#define EVENTS_INTERVAL   500    // Min interval between processEvents calls (milliseconds)
void ProcessEvents(int ProgressBarValue)
{
    // Check interval
    if(g_clLastProcessEvents.elapsed() > EVENTS_INTERVAL)
    {
        QCoreApplication::processEvents();
        g_clLastProcessEvents.start();

        if(GexProgressBar && !GexProgressBar->isHidden())
            GexProgressBar->setValue(ProgressBarValue);
    }
}

bool GS::Gex::SchedulerEngine::HasTasksRunning()
{
    return (GS::Gex::Engine::GetInstance().GetTaskManager().RunningTasksCount() > 0);
}

bool GS::Gex::SchedulerEngine::TimersTraceReset(QString TimerName)
{
    if(TimerName.isEmpty() || (TimerName.toUpper() == "ALL"))
        mTimersTrace.clear();
    else
    {
        mTimersTrace[TimerName] = 0;
        mTimersTrace[TimerName + " count"] = 0;
        mTimersTrace[TimerName + " status"] = 0;
    }
    mTime.start();
    return true;
}

bool GS::Gex::SchedulerEngine::TimersTraceStart(QString TimerName)
{
    if(!mTimersTrace.contains(TimerName))
    {
        mTimersTrace[TimerName] = 0;
        mTimersTrace[TimerName + " count"] = 0;
        mTimersTrace[TimerName + " status"] = 0;
    }

    if(mTimersTrace[TimerName + " status"] == 1)
        return false;

    mTimersTrace[TimerName + " status"] = 1;
    mTimersTrace[TimerName] -= mTime.elapsed();
    return true;
}

bool GS::Gex::SchedulerEngine::TimersTraceStop(QString TimerName)
{
    if(!mTimersTrace.contains(TimerName))
        return false;

    if(mTimersTrace[TimerName + " status"] == 0)
        return false;

    mTimersTrace[TimerName + " status"] = 0;
    mTimersTrace[TimerName] += mTime.elapsed();
    mTimersTrace[TimerName + " count"]++;
    return true;
}

bool GS::Gex::SchedulerEngine::TimersTraceDump(QString TimerName)
{
    if(TimerName.isEmpty() || (TimerName.toUpper() == "ALL"))
    {
        foreach(const QString &Name,mTimersTrace.keys())
        {
            if(Name.endsWith(" status"))
                continue;
            if(Name.endsWith(" count"))
                continue;
            if(mTimersTrace[Name + " status"] != 0)
                continue;
            if(mTimersTrace[Name] == 0)
                continue;

            GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  %1 processing cumul time %2 and count %3")
                  .arg(Name.toLatin1().constData())
                  .arg(mTimersTrace[Name])
                  .arg(mTimersTrace[Name + " count"]).toLatin1().constData());
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  %1 processing cumul time %2 and count %3")
              .arg(TimerName.toLatin1().constData())
              .arg(mTimersTrace[TimerName])
              .arg(mTimersTrace[TimerName + " count"]).toLatin1().constData());
    }
    return true;
}

bool GS::Gex::SchedulerEngine::GetNextTaskOptionValue(int TaskId, QString& Field, QString& Value)
{
    Field = Value = "";

    if(m_QueryTaskOptions.next())
    {
        while(m_QueryTaskOptions.value(0).isValid() && m_QueryTaskOptions.value(0).toInt() < TaskId)
            m_QueryTaskOptions.next();
        if(m_QueryTaskOptions.value(0).isValid() && m_QueryTaskOptions.value(0).toInt() == TaskId)
        {
            Field = m_QueryTaskOptions.value(1).toString();
            Value = m_QueryTaskOptions.value(2).toString().trimmed();
            return true;
        }
        m_QueryTaskOptions.previous();
    }
    return false;
}

bool GS::Gex::SchedulerEngine::LoadTasksList(bool bForceReload/*=false*/)
{
    QMap<QString,QVariant> lMI=CGexSystemUtils::GetMemoryInfo(false,false);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Load Tasks List begin: currently %1 Ko of mem used").arg(lMI.value("MemUsedByProcessInKo").toInt())
          .toLatin1().data() );

    if(!mAllowed)
        return true;

    // Do not reload task if have task running
    if(Engine::GetInstance().HasTasksRunning())
        return true;

    QCoreApplication::processEvents();

    if(!GS::Gex::Engine::GetInstance().HasNodeValidLicense())
        return true;

    // Task Edited is true during the update of the mTasksList
    if(mTaskBeingEdited)
        return true;

    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadTasksList - bForceReload = %1").arg( bForceReload?"true":"false")
          .toLatin1().constData());

    // Check if yieldman is connected

    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            &&  !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
    {
        // Examinator-PRO
        // User not connected
        // Nothing TODO
        // Tasks not loaded
        GSLOG(SYSLOG_SEV_DEBUG, "LoadTasksList - No user connected DONE");
        return true;
    }

    // Before loading all tasks, have to load databases information
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty();

    // Check if all databases are loaded
    if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().m_bDatabasesLoaded)
        return true;


    bool ForceReloadAllTasks = bForceReload;
    if((mTasksList.count() == 0))
        ForceReloadAllTasks = true;

    mTaskBeingEdited = true;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "LoadTasksList - Load tasks");

    ProcessEvents(0);
    TimersTraceReset();
    // Load Tasks from YieldManDb
    qlonglong DbTasksChecksum = mLastDbTasksIdChecksum + mLastDbTasksUpdateCheckSum;
    QDateTime LocalTasksLoaded = mLastLocalTasksLoaded;

    if(ForceReloadAllTasks || (mLastTimeTasksLoaded.elapsed() >= 10000))
    {
        emit sUpdateListView(QStringList()<<"Wait");

        // if already checked 10s ago - do not reload from DB/XML
        LoadUploadedTasks();

        // AND
        // Load Tasks from Local File
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            LoadLocalTasks(ForceReloadAllTasks);

        mLastTimeTasksLoaded.start();
    }

    // If Checksum is the same, no tasks updated or added or deleted
    if((DbTasksChecksum != (mLastDbTasksIdChecksum + mLastDbTasksUpdateCheckSum))
            || (LocalTasksLoaded != mLastLocalTasksLoaded))
        ForceReloadAllTasks = true;

    mTaskBeingEdited = false;

    // Update GUI
    // Check Tasks Status
    CheckTasksList(ForceReloadAllTasks);

    TimersTraceDump();
    emit sUpdateListView(QStringList());

    if(!mAllTasksLoaded) // first execution
    {
        // For GexEmail.exe configuration
        // Save gex_email section if TaskStatus is uploaded and not present in Local tasks
        CGexMoTaskStatus* ptTaskItem = GetStatusTask();
        if(ptTaskItem != NULL)
        {
            // If already uploaded
            if(ptTaskItem->IsUploaded())
            {
                SaveLocalTasks();
            }
        }
    }

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().m_bFirstActivation
            && !mAllTasksLoaded)
        GS::Gex::Engine::GetInstance().GetAdminEngine().SummaryDialog();

    mAllTasksLoaded = true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("LoadTasksList - Load all %1 tasks DONE").arg(mTasksList.count()).toLatin1().data());


    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("");

    lMI=CGexSystemUtils::GetMemoryInfo(false,false);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Load Tasks List begin: currently %1 Ko of mem used").arg(lMI.value("MemUsedByProcessInKo").toInt())
          .toLatin1().data() );

    return true;
}

void GS::Gex::SchedulerEngine::CheckTasksList(bool bForceReload)
{

    if(mTasksList.isEmpty())
        return;

    if(mTaskBeingEdited)
        return;

    if(GexProgressBar && !GexProgressBar->isHidden())
    {
        GexProgressBar->setMaximum(mTasksList.count()*2);
        GexProgressBar->setValue(mTasksList.count());
    }

    TimersTraceStart("Checking tasks");

    // Then Check Status
    // Check the Status before to update the view
    CGexMoTaskItem* ptTaskItem = NULL;
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    lstIteratorTask.toFront();
    QString strMessage = "Checking tasks...";
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(strMessage);

    // It is not necessary to check all tasks when the Scheduler is paused
    // and when the view is not visible
    // LoadTasksList always call when SelectTab changes
    int TaskTypeToCheck = -1;
    if(!bForceReload && !!isSchedulerStopped())
    {
        TaskTypeToCheck = emit sGetCurrentTableType();
    }

    //    bool bFoundNewStatus = false;
    int TotalTasks = mTasksList.count();
    int Progress = TotalTasks;
    int TasksChecked = 0;
    int DebugInfo = 0;
    QTime timer;
    timer.start();

    while(lstIteratorTask.hasNext())
    {
        ProcessEvents(++Progress);

        ptTaskItem = lstIteratorTask.next();

        if(ptTaskItem == NULL)
            continue;

        if((TaskTypeToCheck>0)
                && (ptTaskItem->GetTaskType() != TaskTypeToCheck))
            continue;

        //QDateTime StatusUpdated = ptTaskItem->m_LastStatusUpdate;
        // Then Check if the Database is referenced
        CheckTaskStatus(ptTaskItem,bForceReload?"Folder":"");

        // For first execution (list view is empty)
        // Automatically upload all tasks
        // only if Db is uploaded or if no Db
        if(!mAllTasksLoaded && ptTaskItem->IsLocal() && (ptTaskItem->m_iStatus != MO_TASK_STATUS_ERROR))
        {
            if(ptTaskItem->GetDatabaseName().isEmpty() || (ptTaskItem->m_iDatabaseId >= 0))
            {
                OnUploadTask(ptTaskItem);
                if(ptTaskItem->IsUploaded())
                {
                    CheckTaskStatus(ptTaskItem,"Folder");
                }
            }
        }
        //        if(StatusUpdated != ptTaskItem->m_LastStatusUpdate)
        //            bFoundNewStatus = true;

        ++TasksChecked;
        ++DebugInfo;
        if(((DebugInfo>77) || (timer.elapsed()>500))
                && (DebugInfo*2 < TotalTasks))
        {
            DebugInfo = 0;
            timer.start();
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(
                        "Checking tasks... "+QString::number(TasksChecked)+" tasks");
        }

    }
    TimersTraceStop("Checking tasks");

    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("");

    TimersTraceStart("Displaying tasks");
    // Update the view
    emit sUpdateListView(QStringList()<<"Load"<<(bForceReload?"Force":""));

    TimersTraceStop("Displaying tasks");
}


bool GS::Gex::SchedulerEngine::LoadUploadedTasks(CGexMoTaskItem* ptTask)
{

    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUploadedTasks - for all tasks = %1").arg( (ptTask==NULL)?"true":"false")
          .toLatin1().constData());

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return true;

    // Check if yieldman is connected
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    // For the first run (YieldMan node must be updated and referenced in the ym_nodes table)
    QString strClauseWhere;
    if(ptTask)
    {
        if(ptTask->IsLocal())
            return true;

        strClauseWhere= " WHERE ym_tasks.task_id="+QString::number(ptTask->GetID());
    }
    else
    {
        // LOAD-BALANCING
        // YieldMan/PatMan
        //      isPAT load only PAT tasks else all except PAT tasks
        // Examinator-PRO/Examinator_PAT
        //      isPAT load only PAT tasks else all except PAT tasks
        // NOT LOAD-BALACING
        // YieldMan/PatMan
        //      load only accredited tasks (node_id or null)
        strClauseWhere = " WHERE ym_tasks.type ";
        strClauseWhere+= "IN (";
        strClauseWhere+= CGexMoTaskItem::GetTypeListFor(GS::LPPlugin::ProductInfo::getInstance()->isPATMan()).join(",");
        strClauseWhere+= ") ";
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsLoadBalancingMode())
            strClauseWhere+= " AND (ym_tasks.node_id IS NULL OR(ym_tasks.node_id="
                    +QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId())
                    +"))";
        // hide lines where the user is not the owner
        if (! GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
                !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin())
        {
            QString value;
            // Check if the user can read all other tasks
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(true))
            {
                strClauseWhere+= " AND ym_tasks.user_id="+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId);
            }
            else
            {
                if (GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsValue("SHOW_ALL_USERS_RULES", value)
                        && value.toUpper() != "TRUE")
                    strClauseWhere+= " AND ym_tasks.user_id="+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId);
            }
            if (GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsValue("SHOW_ONLY_ENABLED_RULES", value)
                    && value.toUpper() == "TRUE")
                strClauseWhere+= " AND ym_tasks.enabled=1";
        }
    }

    // Load Tasks from YieldManDb
    bool        ReloadTasksFromDb = false;
    QString     strQuery;
    QSqlQuery   clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    clQuery.setForwardOnly(true);


    if(ptTask == NULL)
    {
        // Check for all tasks
        // Save timestamp of Task file loaded
        // Case 7200: use CHECKSUM on last_update rather than MAX
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
            strQuery = "SELECT SUM(CRC32(ym_tasks.task_id)), SUM(CRC32(ym_tasks.last_update)) FROM ym_tasks";
        else
            strQuery = "SELECT SUM(ORA_HASH(ym_tasks.task_id, POWER(2,16))) , SUM(ORA_HASH(ym_tasks.last_update, POWER(2,16))) FROM ym_tasks";
        strQuery+= strClauseWhere;
        clQuery.exec(strQuery);
        if(clQuery.first())
        {
            if(mLastDbTasksIdChecksum != clQuery.value(0).toLongLong())
                ReloadTasksFromDb = true;
            mLastDbTasksIdChecksum = clQuery.value(0).toLongLong();
            if(mLastDbTasksUpdateCheckSum != clQuery.value(1).toLongLong())
                ReloadTasksFromDb = true;
            mLastDbTasksUpdateCheckSum = clQuery.value(1).toLongLong();
        }
        else
        {
            ReloadTasksFromDb = true;
            mLastDbTasksIdChecksum = 0;
            mLastDbTasksUpdateCheckSum = 0;
        }
    }
    else
    {
        // Check for one task
        strQuery = "SELECT last_update FROM ym_tasks";
        strQuery+= strClauseWhere;
        clQuery.exec(strQuery);
        if(clQuery.first())
        {
            if(ptTask->m_clLastUpdate != clQuery.value(0).toDateTime())
                ReloadTasksFromDb = true;
        }
    }

    if(!ReloadTasksFromDb)
        return true;

    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUploadedTasks - New mLastDbTasksChecksum = %1")
          .arg(mLastDbTasksIdChecksum).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUploadedTasks - New mLastDbTasksUpdateCheckSum = %1")
          .arg(mLastDbTasksUpdateCheckSum).toLatin1().constData());

    if(ptTask == NULL)
    {
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("Loading tasks...");
        TimersTraceStart("Loading tasks");
    }

    strQuery = "SELECT ym_tasks.task_id,ym_tasks.type,ym_tasks.node_id,ym_tasks.user_id,ym_tasks.group_id,ym_tasks.database_id,ym_tasks.name,";
    strQuery+= "ym_tasks.enabled,ym_tasks.permisions,ym_tasks.creation_date,ym_tasks.expiration_date,ym_tasks.last_update FROM ym_tasks";
    strQuery+= strClauseWhere;
    strQuery+= " ORDER BY ym_tasks.task_id";

    if(clQuery.exec(strQuery))
    {
        // Load Tasks from YieldManDb
        int     iIndex;
        bool    bStatus = false;
        int     TaskId;
        int     TaskType;

        // Get the list of tasks already created
        QMap<int,CGexMoTaskItem*> TasksFromList;
        QStringList               TasksFromDB;
        QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
        CGexMoTaskItem* ptTaskItem = NULL;
        QDateTime   LastDbTaskLoaded;

        lstIteratorTask.toFront();
        while (lstIteratorTask.hasNext())
        {
            ptTaskItem = lstIteratorTask.next();

            if(ptTask)
            {
                // only check the current task
                if(ptTask->GetID() != ptTaskItem->GetID())
                    continue;
            }

            if (!ptTaskItem || ptTaskItem->IsLocal())
                continue;
            TasksFromList[ptTaskItem->GetID()] = ptTaskItem;
        }


        // Load task list
        QTime timerLoad;
        timerLoad.start();
        int TasksLoaded = 0;
        int DebugInfo = 0;
        QTime timer;
        timer.start();

        m_QueryTaskOptions = QSqlQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
        m_QueryTaskOptions.setForwardOnly(false);

        // Load task options list
        strQuery = "SELECT ym_tasks_options.task_id, ym_tasks_options.field, ym_tasks_options.value FROM ym_tasks_options ";
        strQuery+= " RIGHT OUTER JOIN ym_tasks ON ym_tasks_options.task_id=ym_tasks.task_id";
        strQuery+= strClauseWhere;
        strQuery+=" ORDER BY task_id";

        if(!m_QueryTaskOptions.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(
                        GS::Gex::AdminEngine::eDB_Query,
                        strQuery,
                        QString::number(m_QueryTaskOptions.lastError().number()),
                        m_QueryTaskOptions.lastError().text());
            return false;
        }

        // pre-load flyweight definitions of the SPM and SYA tasks
        mSYALightWeightOptions.clear();
        mSPMLightWeightOptions.clear();
        QMap<QString, QString> lOption;
        QSqlQuery lMonitoringQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

        strQuery =
            "SELECT"
            "    s.task_id,"
            "    s.testing_stage,"
            "    s.product_regexp,"
            "    s.stats_to_monitor,"
            "    s.min_lots,"
            "    s.default_algorithm,"
            "    dpstd.param_value AS n1,"
            "    dpcrt.param_value AS n2,"
            "    s.emails,"
            "    active_prod.version_id AS active_version_id,"
            "    active_prod.expiration_date AS active_expiration_date,"
            "    active_prod.expiration_warning_date AS active_expiration_warning_date,"
            "    active_prod.expiration_warning_done AS active_expiration_warning_done,"
            "    last_prod.version_id AS last_version_id,"
            "    last_prod.start_date AS last_start_date "
            "FROM"
            "    ym_tasks "
            "INNER JOIN"
            "    ym_tasks_options top"
            "    ON ym_tasks.task_id = top.task_id"
            "    AND top.field = 'TaskId' "
            "INNER JOIN"
            "    ym_sya s"
            "    ON s.task_id = top.value "
            "INNER JOIN"
            "    ym_sya_default_params dpstd"
            "    ON s.task_id = dpstd.task_id"
            "    AND dpstd.criticity_level = 1"
            "    AND dpstd.param_name = 'N' "
            "INNER JOIN"
            "    ym_sya_default_params dpcrt"
            "    ON s.task_id = dpcrt.task_id"
            "    AND dpcrt.criticity_level = 2"
            "    AND dpcrt.param_name = 'N' "
            "LEFT OUTER JOIN ("
            "    SELECT"
            "        task_id,"
            "        MAX(version_id) AS active_version_id"
            "    FROM"
            "        ym_sya_version"
            "    WHERE"
            "        draft_version = 0"
            "        AND start_date <= NOW()"
            "    GROUP BY"
            "        task_id"
            ") active_prod_version"
            "    ON s.task_id = active_prod_version.task_id "
            "LEFT OUTER JOIN"
            "    ym_sya_version active_prod"
            "    ON active_prod_version.task_id = active_prod.task_id"
            "    AND active_prod_version.active_version_id = active_prod.version_id "
            "LEFT OUTER JOIN ("
            "    SELECT"
            "        task_id,"
            "        MAX(version_id) AS active_version_id"
            "    FROM"
            "        ym_sya_version"
            "    WHERE"
            "        draft_version = 0"
            "    GROUP BY"
            "        task_id"
            ") last_prod_version"
            "    ON s.task_id = last_prod_version.task_id "
            "LEFT OUTER JOIN"
            "    ym_sya_version last_prod"
            "    ON last_prod_version.task_id = last_prod.task_id"
            "    AND last_prod_version.active_version_id = last_prod.version_id "
            + strClauseWhere +
            " ORDER BY"
            "    s.task_id;";

        if(!lMonitoringQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(
                        GS::Gex::AdminEngine::eDB_Query,
                        strQuery,
                        QString::number(lMonitoringQuery.lastError().number()),
                        lMonitoringQuery.lastError().text());
            return false;
        }

        while(lMonitoringQuery.next())
        {
            lOption.clear();
            lOption.insert("testing_stage", lMonitoringQuery.value("testing_stage").toString());
            lOption.insert("product_regexp", lMonitoringQuery.value("product_regexp").toString());
            lOption.insert("stats_to_monitor", lMonitoringQuery.value("stats_to_monitor").toString());
            lOption.insert("min_lots", lMonitoringQuery.value("min_lots").toString());
            lOption.insert("default_algorithm", lMonitoringQuery.value("default_algorithm").toString());
            lOption.insert("n1", lMonitoringQuery.value("n1").toString());
            lOption.insert("n2", lMonitoringQuery.value("n2").toString());
            lOption.insert("emails", lMonitoringQuery.value("emails").toString());
            lOption.insert("active_version_id", lMonitoringQuery.value("active_version_id").isNull() ? "-1" : lMonitoringQuery.value("active_version_id").toString());
            lOption.insert("active_expiration_date", lMonitoringQuery.value("active_expiration_date").toString());
            lOption.insert("active_expiration_warning_date", lMonitoringQuery.value("active_expiration_warning_date").toString());
            lOption.insert("active_expiration_warning_done", lMonitoringQuery.value("active_expiration_warning_done").toString());
            lOption.insert("last_version_id", lMonitoringQuery.value("last_version_id").isNull() ? "-1" : lMonitoringQuery.value("active_version_id").toString());
            lOption.insert("last_start_date", lMonitoringQuery.value("last_start_date").toString());

            mSYALightWeightOptions.insert(lMonitoringQuery.value("task_id").toInt(), lOption);
        }

        strQuery =
            "SELECT"
            "    s.task_id,"
            "    s.testing_stage,"
            "    s.product_regexp,"
            "    s.stats_to_monitor,"
            "    s.min_lots,"
            "    s.default_algorithm,"
            "    dpstd.param_value AS n1,"
            "    dpcrt.param_value AS n2,"
            "    s.emails,"
            "    active_prod.version_id AS active_version_id,"
            "    active_prod.expiration_date AS active_expiration_date,"
            "    active_prod.expiration_warning_date AS active_expiration_warning_date,"
            "    active_prod.expiration_warning_done AS active_expiration_warning_done,"
            "    last_prod.version_id AS last_version_id,"
            "    last_prod.start_date AS last_start_date "
            "FROM"
            "    ym_tasks "
            "INNER JOIN"
            "    ym_tasks_options top"
            "    ON ym_tasks.task_id = top.task_id"
            "    AND top.field = 'TaskId' "
            "INNER JOIN"
            "    ym_spm s"
            "    ON s.task_id = top.value "
            "INNER JOIN"
            "    ym_spm_default_params dpstd"
            "    ON s.task_id = dpstd.task_id"
            "    AND dpstd.criticity_level = 1"
            "    AND dpstd.param_name = 'N' "
            "INNER JOIN"
            "    ym_spm_default_params dpcrt"
            "    ON s.task_id = dpcrt.task_id"
            "    AND dpcrt.criticity_level = 2"
            "    AND dpcrt.param_name = 'N' "
            "LEFT OUTER JOIN ("
            "    SELECT"
            "        task_id,"
            "        MAX(version_id) AS active_version_id"
            "    FROM"
            "        ym_spm_version"
            "    WHERE"
            "        draft_version = 0"
            "        AND start_date <= NOW()"
            "    GROUP BY"
            "        task_id"
            ") active_prod_version"
            "    ON s.task_id = active_prod_version.task_id "
            "LEFT OUTER JOIN"
            "    ym_spm_version active_prod"
            "    ON active_prod_version.task_id = active_prod.task_id"
            "    AND active_prod_version.active_version_id = active_prod.version_id "
            "LEFT OUTER JOIN ("
            "    SELECT"
            "        task_id,"
            "        MAX(version_id) AS active_version_id"
            "    FROM"
            "        ym_spm_version"
            "    WHERE"
            "        draft_version = 0"
            "    GROUP BY"
            "        task_id"
            ") last_prod_version"
            "    ON s.task_id = last_prod_version.task_id "
            "LEFT OUTER JOIN"
            "    ym_spm_version last_prod"
            "    ON last_prod_version.task_id = last_prod.task_id"
            "    AND last_prod_version.active_version_id = last_prod.version_id "
            + strClauseWhere +
            " ORDER BY"
            "    s.task_id;";

        if(!lMonitoringQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(
                        GS::Gex::AdminEngine::eDB_Query,
                        strQuery,
                        QString::number(lMonitoringQuery.lastError().number()),
                        lMonitoringQuery.lastError().text());
            return false;
        }

        while(lMonitoringQuery.next())
        {
            lOption.clear();
            lOption.insert("testing_stage", lMonitoringQuery.value("testing_stage").toString());
            lOption.insert("product_regexp", lMonitoringQuery.value("product_regexp").toString());
            lOption.insert("stats_to_monitor", lMonitoringQuery.value("stats_to_monitor").toString());
            lOption.insert("min_lots", lMonitoringQuery.value("min_lots").toString());
            lOption.insert("default_algorithm", lMonitoringQuery.value("default_algorithm").toString());
            lOption.insert("n1", lMonitoringQuery.value("n1").toString());
            lOption.insert("n2", lMonitoringQuery.value("n2").toString());
            lOption.insert("emails", lMonitoringQuery.value("emails").toString());
            lOption.insert("active_version_id", lMonitoringQuery.value("active_version_id").isNull() ? "-1" : lMonitoringQuery.value("active_version_id").toString());
            lOption.insert("active_expiration_date", lMonitoringQuery.value("active_expiration_date").toString());
            lOption.insert("active_expiration_warning_date", lMonitoringQuery.value("active_expiration_warning_date").toString());
            lOption.insert("active_expiration_warning_done", lMonitoringQuery.value("active_expiration_warning_done").toString());
            lOption.insert("last_version_id", lMonitoringQuery.value("last_version_id").isNull() ? "-1" : lMonitoringQuery.value("active_version_id").toString());
            lOption.insert("last_start_date", lMonitoringQuery.value("last_start_date").toString());

            mSPMLightWeightOptions.insert(lMonitoringQuery.value("task_id").toInt(), lOption);
        }

        int TotalTasks = clQuery.size();
        if(GexProgressBar && !GexProgressBar->isHidden())
        {
            GexProgressBar->setMaximum(TotalTasks*3);
            GexProgressBar->setValue(0);
        }
        while(clQuery.next())
        {
            ProcessEvents(TasksLoaded);
            ptTaskItem = NULL;
            bStatus = false;

            iIndex = 0;
            TaskId  = clQuery.value(iIndex++).toInt();
            TaskType = clQuery.value(iIndex++).toInt();

            LastDbTaskLoaded = clQuery.value(11).toDateTime();

            TasksFromDB << QString::number(TaskId);

            // Check if existing task needs to be reloaded
            if(TasksFromList.contains(TaskId))
            {
                ptTaskItem = TasksFromList[TaskId];

                if(ptTaskItem && ptTaskItem->m_clLastUpdate == LastDbTaskLoaded)
                    continue;
            }
            else
            {
                switch(TaskType)
                {
                    case GEXMO_TASK_OLD_DATAPUMP:
                        ptTaskItem = new CGexMoTaskOldDataPump;
                    break;
                    case GEXMO_TASK_DATAPUMP:
                        ptTaskItem = new CGexMoTaskDataPump;
                    break;
                    case GEXMO_TASK_TRIGGERPUMP:
                        ptTaskItem = new CGexMoTaskTriggerPump;
                    break;
                    case GEXMO_TASK_PATPUMP:
                        ptTaskItem = new CGexMoTaskPatPump;
                    break;
                    case GEXMO_TASK_YIELDMONITOR:
                        ptTaskItem = new CGexMoTaskYieldMonitor;
                    break;
                    case GEXMO_TASK_YIELDMONITOR_RDB:
                        ptTaskItem = new CGexMoTaskSYA;
                    break;
                    case GEXMO_TASK_REPORTING:
                        ptTaskItem = new CGexMoTaskReporting;
                    break;
                    case GEXMO_TASK_STATUS:
                        ptTaskItem = new CGexMoTaskStatus;
                    break;
                    case GEXMO_TASK_CONVERTER:
                        ptTaskItem = new CGexMoTaskConverter;
                    break;
                    case GEXMO_TASK_OUTLIER_REMOVAL:
                        ptTaskItem = new CGexMoTaskOutlierRemoval;
                    break;
                    case GEXMO_TASK_AUTOADMIN:
                        ptTaskItem = new CGexMoTaskAutoAdmin;
                    break;
                    case GEXMO_TASK_SPM:
                        ptTaskItem = new CGexMoTaskSPM;
                    break;
                    case GEXMO_TASK_SYA:
                        ptTaskItem = new CGexMoTaskSYA;
                    break;
                    default:
                    break;
                }

                if(!ptTaskItem)
                {
                    GSLOG(SYSLOG_SEV_CRITICAL, QString("Cannot allocate new task!!").toLatin1().constData());
                    return false;
                }
            }

            // Update the task from ym_tasks
            ptTaskItem->SetID(TaskId);
            ptTaskItem->m_iNodeId = clQuery.value(iIndex++).toInt();
            ptTaskItem->m_iUserId = clQuery.value(iIndex++).toInt();
            ptTaskItem->m_iGroupId = clQuery.value(iIndex++).toInt();
            ptTaskItem->m_iDatabaseId = clQuery.value(iIndex++).toInt();
            ptTaskItem->m_strName = clQuery.value(iIndex++).toString();
            ptTaskItem->SetEnabledState(clQuery.value(iIndex++).toInt() == 1);
            ptTaskItem->m_iPermissions = clQuery.value(iIndex++).toInt();
            ptTaskItem->m_clCreationDate = clQuery.value(iIndex++).toDateTime();
            ptTaskItem->m_clExpirationDate = clQuery.value(iIndex++).toDateTime();

            // Case 7195 - 7202: LoadDbTaskSection... will not remove internal pointer
            // Update the task from ym_tasks_options
            bStatus = ptTaskItem->LoadTaskDataFromDb();

            ++TasksLoaded;
            ++DebugInfo;
            if(((DebugInfo>77) || (timer.elapsed()>500))
                    && (DebugInfo*2 < TotalTasks))
            {
                DebugInfo = 0;
                timer.start();
                GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUploadedTasks - %1 tasks (re)loaded in %2")
                      .arg(TasksLoaded)
                      .arg(timerLoad.elapsed()).toLatin1().constData());
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(
                            QString("Loading tasks... %1 tasks").arg(TasksLoaded));
            }

            if(!bStatus)
            {
                // If error during task update
                // Case 7195: do not remove task, just keep previous definition
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("LoadUploadedTasks - can't load task id %1, keeping previous task definition")
                      .arg(TaskId).toLatin1().constData());
                // Check if it is a new creation
                if(!TasksFromList.contains(TaskId))
                {
                    delete ptTaskItem;
                    ptTaskItem=NULL;
                }
            }
            else
            {
                // Load OK
                // Update the PrivateAttributes for READ-ONLY access
                ptTaskItem->UpdatePrivateAttributes();
                ptTaskItem->m_clLastUpdate = LastDbTaskLoaded;

                // Check the updated task is a new task
                if(!TasksFromList.contains(TaskId))
                {
                    // Add task to internal structure
                    mTasksList.append(ptTaskItem);
                }
            }
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUploadedTasks - %1 tasks (re)loaded in %2")
              .arg(TasksLoaded)
              .arg(timerLoad.elapsed()).toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG, "LoadUploadedTasks - Load Query DONE");


        // Check if have tasks removed
        // and check the tasks status
        foreach(int key, TasksFromList.keys())
        {
            if(TasksFromDB.contains(QString::number(key)))
                continue;

            ptTaskItem = TasksFromList[key];
            if(ptTaskItem == NULL)
                continue;

            if(ptTask)
            {
                // only check the current task
                if(ptTask->GetID() != ptTaskItem->GetID())
                    continue;
            }
            // Remove this task from the task list
            DeleteTaskInList(ptTaskItem);
        }
        TasksFromDB.clear();
        TasksFromList.clear();
    }
    else
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                     strQuery,
                                                                     QString::number(clQuery.lastError().number()),
                                                                     clQuery.lastError().text());
        return false;
    }

    if(ptTask == NULL)
        TimersTraceStop("Loading tasks");
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("");

    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: Data Pump
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionDataPump(CGexMoTaskDataPump * ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;
    int     nValue;
    double  lfValue;

    GexMoDataPumpTaskData* ptDataPump = ptTask->GetProperties();
    bool bAllocate = false;
    if(ptDataPump == NULL)
    {
        bAllocate = true;
        ptDataPump = new GexMoDataPumpTaskData(ptTask);
        ptTask->SetProperties(ptDataPump);
    }

    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptDataPump->strTitle = strValue;

        // Read data path
        else if(strString == "DataPath")
            ptDataPump->strDataPath = strValue;

        // Scan sub-folder flag
        else if(strString == "ScanSubFolders")
        {
            if(strValue == "YES")
                ptDataPump->bScanSubFolders = true;
            else
                ptDataPump->bScanSubFolders = false;
        }

        // Sort By flag
        else if(strString == "SortBy")
        {
            ptDataPump->eSortFlags = QDir::Name;
            if(strValue.startsWith("Time"))
            {
                ptDataPump->eSortFlags = QDir::Time;
                if(strValue.endsWith("Desc"))
                    ptDataPump->eSortFlags |= QDir::Reversed;
            }
        }

        // List of files extensions to import
        else if(strString == "ImportFileExtensions")
            ptDataPump->strImportFileExtensions = strValue;

        // Read Database targetted
        else if(strString == "Database")
            ptTask->m_strDatabaseName = ptDataPump->strDatabaseTarget = strValue;

        // Read Data Type targetted
        else if(strString == "DataType")
            ptDataPump->uiDataType = strValue.toUInt();

        // Read Testing stage (if WYR data type)
        else if(strString == "TestingStage")
            ptDataPump->strTestingStage = strValue;

        // Read Priority
        else if(strString == "Priority")
            ptDataPump->mPriority = strValue.toInt();

        // Read Task frequency
        else if(strString == "Frequency")
            ptDataPump->iFrequency = strValue.toLong();
        // Read Task Day of Week execution
        else if(strString == "DayOfWeek")
            ptDataPump->iDayOfWeek = strValue.toLong();
        // Execution window flag
        else if(strString == "ExecWindow")
        {
            if(strValue == "YES")
                ptDataPump->bExecutionWindow = true;
            else
                ptDataPump->bExecutionWindow = false;
        }

        // Read Start-time
        else if(strString == "StartTime")
        {
            strString = strValue;
            strValue = strString.section(',',0,0);
            int iHour = strValue.toInt();
            strValue = strString.section(',',1,1);
            int iMinute = strValue.toInt();
            QTime tStartTime(iHour,iMinute);
            ptDataPump->cStartTime = tStartTime;
        }

        // Read Stop-time
        else if(strString == "StopTime")
        {
            strString = strValue;
            strValue = strString.section(',',0,0);
            int iHour = strValue.toInt();
            strValue = strString.section(',',1,1);
            int iMinute = strValue.toInt();
            QTime tStopTime(iHour,iMinute);
            ptDataPump->cStopTime = tStopTime;
        }

        // Read PostImport task: Rename, Move or Delete files.
        else if(strString == "PostImport")
            ptDataPump->iPostImport= strValue.toLong();

        // Read Move/FTP folder
        else if(strString == "PostImportFolder")
            ptDataPump->strPostImportMoveFolder = strValue;

        // Read PostImport task: Rename, Move or Delete files.
        else if(strString == "PostImportFailure")
            ptDataPump->iPostImportFailure= strValue.toLong();

        // Read Move/FTP folder
        else if(strString == "PostImportFailureFolder")
            ptDataPump->strPostImportFailureMoveFolder = strValue;

        // Read PostImport task (delayed files): Rename, Move or Delete files.
        else if(strString == "PostImportDelay")
            ptDataPump->iPostImportDelay= strValue.toLong();

        // Read Move/FTP folder (delayed files)
        else if(strString == "PostImportDelayFolder")
            ptDataPump->strPostImportDelayMoveFolder = strValue;

        // check Yield window flag
        else if(strString == "CheckYield")
        {
            if(strValue == "YES")
                ptDataPump->bCheckYield = true;
            else
                ptDataPump->bCheckYield = false;
        }

        // Read Good bin list
        else if(strString == "YieldBins")
            ptDataPump->strYieldBinsList = strValue;

        // Read Alarm level (0-100%)
        else if(strString == "AlarmLevel")
            ptDataPump->iAlarmLevel = strValue.toInt();

        // Read Minimum parts to have a valid file
        else if(strString == "MinimumParts")
            ptDataPump->lMinimumyieldParts = strValue.toLong();

        // Read Email 'From'
        else if((strString == "EmailFrom") || (strString == "EmailsFrom")) // bug from SaveDb
            ptDataPump->strEmailFrom = strValue;

        // Read Email notification list
        else if(strString == "Emails")
            ptDataPump->strEmailNotify = strValue;

        // Read Email format: HTML or TXT
        else if(strString == "EmailFormat")
        {
            if(strValue == "HTML")
                ptDataPump->bHtmlEmail = true;
            else
                ptDataPump->bHtmlEmail = false;
        }

        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }

        // Other options
        else if(strString == "RejectFilesWithFewerParts")
        {
            // Old option where only 1 mode was supported (abs nb of parts, -1 for disabled)
            nValue = strValue.toInt();
            if(nValue > 0)
            {
                ptDataPump->bRejectSmallSplitlots_NbParts = true;
                ptDataPump->uiRejectSmallSplitlots_NbParts = (unsigned int)nValue;
            }
        }

        else if(strString == "RejectSmallSplitlots_NbParts")
        {
            // New option, nb_parts mode (abs nb of parts, -1 for disabled)
            nValue = strValue.toInt();
            if(nValue > 0)
            {
                ptDataPump->bRejectSmallSplitlots_NbParts = true;
                ptDataPump->uiRejectSmallSplitlots_NbParts = (unsigned int)nValue;
            }
        }

        else if(strString == "RejectSmallSplitlots_Gdpw_Percent")
        {
            // New option, gdpw_percent mode (percentage of GDPW, -1 for disabled)
            lfValue = strValue.toDouble();
            if(lfValue > 0.0)
            {
                ptDataPump->bRejectSmallSplitlots_GdpwPercent = true;
                ptDataPump->lfRejectSmallSplitlots_GdpwPercent = lfValue;
            }
        }

        else if(strString == "ExecuteBatchAfterInsertion")
        {
            if(strValue == "YES")
                ptDataPump->bExecuteBatchAfterInsertion = true;
            else
                ptDataPump->bExecuteBatchAfterInsertion = false;
        }

        else if(strString == "BatchToExecuteAfterInsertion")
            ptDataPump->strBatchToExecuteAfterInsertion = strValue;

        else if(strString == "MaxPartsForTestResultInsertion")
            ptDataPump->nMaxPartsForTestResultInsertion = strValue.toInt();

        else if(strString == "RejectFilesOnPassBinlist")
        {
            if(strValue == "YES")
                ptDataPump->bRejectFilesOnPassBinlist = true;
            else
                ptDataPump->bRejectFilesOnPassBinlist = false;
        }
        else if(strString == "PassBinlistForRejectTest")
            ptDataPump->strPassBinListForRejectTest = strValue;
        else
        {
            // any non hard coded option
            if (strString=="PreInsertionJavaScript") // deprecated
                strString=GexMoDataPumpTaskData::sPreScriptAttrName;
            if (strString=="PostInsertionJavaScript") // deprecated
                strString=GexMoDataPumpTaskData::sPostScriptAttrName;

            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }

    // Old frequency
    // < 30mn [7] => High
    // < 1h   [8] => Medium
    // else       => Low
    if((!ptTask->GetAttributes().contains("Priority"))
            && (ptTask->GetAttributes().contains("Frequency")))
    {
        if(ptDataPump->iFrequency <= 7)
            ptDataPump->mPriority = 2;
        else if(ptDataPump->iFrequency <= 8)
            ptDataPump->mPriority = 1;
        else
            ptDataPump->mPriority = 0;
    }

    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptDataPump;
            ptTask->SetProperties(NULL);
        }
        return false;
    }

    // for 7.2 to 7.3 migration
    if (!ptTask->GetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName).isValid())
        ptTask->SetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName, false);
    if (!ptTask->GetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName).isValid())
        ptTask->SetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName, false);

    if (!ptTask->GetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName).isValid())
        ptTask->SetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName, GexMoDataPumpTaskData::sDefaultPostScript );
    if (!ptTask->GetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName).isValid())
    {
        if (ptTask->GetTaskType()==GEXMO_TASK_DATAPUMP)
            ptTask->SetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName, GexMoDataPumpTaskData::sDefaultDataPumpPreScript);
        else
            ptTask->SetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName, GexMoDataPumpTaskData::sDefaultPreScript);
    }
    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: YIELD MONITORING RDB  (SYL/SBL)
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionYieldMonitoring_OneRule(CGexMoTaskYieldMonitor *  ptTask)
{

    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    // Allocate buffer to store information read from disk.
    bool bAllocate = false;
    GexMoYieldMonitoringTaskData* ptYield = ptTask->GetProperties();
    if(ptYield == NULL)
    {
        bAllocate = true;
        ptYield = new GexMoYieldMonitoringTaskData(ptTask);
        ptTask->SetProperties(ptYield);
        ptTask->SetAttribute("ExpirationWarned", false);
    }


    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptYield->strTitle = strValue;

        // Read ProductID : 7170
        else if(strString == "ProductID")
            ptYield->strProductID = strValue;

        // Read Yield Bin list
        else if(strString == "YieldBins")
            ptYield->strYieldBinsList = strValue;

        // Bin list type: 0=Good bins, 1=Failing bins.
        else if(strString == "BiningType")
            ptYield->iBiningType = strValue.toInt();

        // Read Alarm level (0-100%)
        else if(strString == "AlarmLevel")
            ptYield->iAlarmLevel = strValue.toInt();

        // Read Flag: Check if Yield OVER or UNDER the limit.
        else if(strString == "AlarmDirection")
            ptYield->iAlarmIfOverLimit = strValue.toInt();

        // Read Minimum parts to have a valid file
        else if(strString == "MinimumParts")
            ptYield->lMinimumyieldParts = strValue.toLong();

        // Read SBL/YBL data file (if exists)
        else if((strString == "SblFile")) //&& QFile::exists(strValue))
        {
            GSLOG(SYSLOG_SEV_WARNING, "SBL file no more supported.");  //ptYield->strSblFile = strValue;
        }
        else if(strString == "EmailFrom")
            ptYield->strEmailFrom = strValue;

        // Read Email notification list
        else if(strString == "Emails")
            ptYield->strEmailNotify = strValue;

        // Read Email format: HTML or TXT
        else if(strString == "EmailFormat")
        {
            if(strValue == "HTML")
                ptYield->bHtmlEmail = true;
            else
                ptYield->bHtmlEmail = false;
        }

        // Read Email message contents type to send
        else if(strString == "EmailReportType")
            ptYield->iEmailReportType = strValue.toInt();

        // Read Email report notification type: send as attachment or leave on server
        else if(strString == "NotificationType")
            ptYield->iNotificationType = strValue.toInt();

        // Read Alarm type: Standard, Critical...
        else if(strString == "ExceptionLevel")
            ptYield->iExceptionLevel = strValue.toInt();

        /////////////////////////////////////////////////////////////////////////////
        // SYL-SBL specifics
        /////////////////////////////////////////////////////////////////////////////
        else if(strString == "ActiveOnDatafileInsertion")
            ptYield->bSYA_active_on_datafile_insertion = strValue == "1";

        else if(strString == "ActiveOnTriggerFile")
            ptYield->bSYA_active_on_trigger_file = strValue == "1";

        else if(strString == "BinRule")
        {
            int nBinNo;
            int nRuleType;

            bool bIsNumber;
            QStringList lRules = strValue.split(";");
            while(!lRules.isEmpty())
            {
                strValue = lRules.takeFirst();

                nBinNo = strValue.section("|",0,0).toInt(&bIsNumber);

                // If no error
                if(bIsNumber)
                {
                    nRuleType = strValue.section("|",1,1).toInt(&bIsNumber);

                    // Add the rule bin
                    ptYield->mapBins_rules[nBinNo]["RuleType"] = nRuleType;
                    if(nRuleType == eManual)
                    {
                        ptYield->mapBins_rules[nBinNo]["LL1"] = strValue.section("|",2,2).toFloat();
                        ptYield->mapBins_rules[nBinNo]["HL1"] = strValue.section("|",3,3).toFloat();
                        ptYield->mapBins_rules[nBinNo]["LL2"] = strValue.section("|",4,4).toFloat();
                        ptYield->mapBins_rules[nBinNo]["HL2"] = strValue.section("|",5,5).toFloat();
                    }
                    else
                    {
                        ptYield->mapBins_rules[nBinNo]["N1"] = strValue.section("|",2,2).toFloat();
                        ptYield->mapBins_rules[nBinNo]["N2"] = strValue.section("|",3,3).toFloat();
                    }
                }
            }
        }

        else if(strString == "Database")
            ptTask->m_strDatabaseName = ptYield->strDatabase = strValue;

        else if(strString == "TestingStage")
            ptYield->strTestingStage = strValue;

        else if(strString == "RuleType")
        {
            bool ok=false;
            ptYield->eSYA_Rule = (OutlierRule)strValue.toInt(&ok);      // Rule: 0=N*Sigma, 1=N*IQR
        }

        else if(strString == "RuleTypeString")       // Rule string: N*Sigma, N*IQR
            ptYield->strSYA_Rule = strValue;

        else if(strString == "N_Parameter")
            ptYield->fSYA_N1_value = strValue.toFloat();    // N parameter (compatibility, new fields are N1, N2)

        else if(strString == "N1_Parameter")
            ptYield->fSYA_N1_value = strValue.toFloat();    // N1 parameter

        else if(strString == "N2_Parameter")
            ptYield->fSYA_N2_value = strValue.toFloat();    // N2 parameter

        else if(strString == "MinimumLotsRequired")
            ptYield->iSYA_LotsRequired = strValue.toInt();// Minimum Total lots required for computing new SYL-SBL

        else if(strString == "ValidityPeriod")
        {
            ptYield->iSYA_Period = strValue.toInt();// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...
        }
        else if(strString == "ExpirationDate")
        {
            // Historically, dates where stored in French format d M yyyy
            // While mutating ExpirationDate to Attribute, the QVariant defaulty stringify date in ISO format : yyyy-MM-dd
            // datetime in yyyy-MM-ddThh:mm:ss
            // Consequently, for the moment, lets support all formats.
            QDateTime d;
            d=d.fromString(strValue, "yyyy-MM-dd"); // ISO
            if (!d.isValid())
            {
                d=d.fromString(strValue, "yyyy-MM-ddThh:mm:ss");
                if (!d.isValid())
                {
                    // French parsing ?
                    int iDay,iMonth,iYear;
                    iDay = strValue.section(' ',0,0).trimmed().toInt();
                    iMonth = strValue.section(' ',1,1).trimmed().toInt();
                    iYear = strValue.section(' ',2,2).trimmed().toInt();
                    //ptYield->cExpiration.setDate(iYear,iMonth,iDay);
                    d=QDateTime(QDate(iYear,iMonth,iDay));
                }
            }
            ptYield->SetAttribute("ExpirationDate",d.date());
        }

        else if(strString == "SBL_LL_Disabled")
        {
            ptYield->strSYA_SBL1_LL_Disabled = strValue;
            ptYield->strSYA_SBL2_LL_Disabled = strValue;
        }

        else if(strString == "SBL1_LL_Disabled")
            ptYield->strSYA_SBL1_LL_Disabled = strValue;

        else if(strString == "SBL2_LL_Disabled")
            ptYield->strSYA_SBL2_LL_Disabled = strValue;

        else if(strString == "SBL_HL_Disabled")
        {
            ptYield->strSYA_SBL1_HL_Disabled = strValue;
            ptYield->strSYA_SBL2_HL_Disabled = strValue;
        }

        else if(strString == "SBL1_HL_Disabled")
            ptYield->strSYA_SBL1_HL_Disabled = strValue;

        else if(strString == "SBL2_HL_Disabled")
            ptYield->strSYA_SBL2_HL_Disabled = strValue;

        else if(strString == "SYL_LL_Disabled")
        {
            ptYield->bSYA_SYL1_LL_Disabled = strValue == "1";
            ptYield->bSYA_SYL2_LL_Disabled = strValue == "1";
        }

        else if(strString == "SYL1_LL_Disabled")
            ptYield->bSYA_SYL1_LL_Disabled = strValue == "1";

        else if(strString == "SYL2_LL_Disabled")
            ptYield->bSYA_SYL2_LL_Disabled = strValue == "1";

        else if(strString == "SYL_HL_Disabled")
        {
            ptYield->bSYA_SYL1_HL_Disabled = strValue == "1";
            ptYield->bSYA_SYL2_HL_Disabled = strValue == "1";
        }

        else if(strString == "SYL1_HL_Disabled")
            ptYield->bSYA_SYL1_HL_Disabled = strValue == "1";

        else if(strString == "SYL2_HL_Disabled")
            ptYield->bSYA_SYL2_HL_Disabled = strValue == "1";

        else if(strString == "IgnoreDataPointsWithNullSigma")
            ptYield->bSYA_IgnoreDataPointsWithNullSigma = strValue == "1";

        else if(strString == "IgnoreOutliers")
            ptYield->bSYA_IgnoreOutliers = (strValue == "1");

        else if(strString == "UseGrossDie")
            ptYield->bSYA_UseGrossDie = (strValue == "1");

        else if(strString == "MinDataPoints")
            ptYield->iSYA_MinDataPoints = strValue.toInt();
        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // Generic option
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }

    if((ptTask->GetAttribute("CheckType").isNull())
            || (ptTask->GetAttribute("CheckType").toString() == "Unknown"))
    {
        // Check if the CheckType is setted
        if(ptTask->GetTaskType() == GEXMO_TASK_YIELDMONITOR)
            ptTask->SetAttribute("CheckType", QVariant("FixedYieldTreshold"));
    }

    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptYield;
            ptTask->SetProperties(NULL);
        }
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: REPORTING
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionReporting(CGexMoTaskReporting *    ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    bool bAllocate = false;
    GexMoReportingTaskData* ptReporting = ptTask->GetProperties();
    if(ptReporting == NULL)
    {
        bAllocate = true;
        ptReporting = new GexMoReportingTaskData(ptTask);
        ptTask->SetProperties(ptReporting);
    }

    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptReporting->strTitle = strValue;

        // Read Script path
        else if(strString == "ScriptPath")
            ptReporting->strScriptPath = strValue;

        // Read Task frequency
        else if(strString == "Frequency")
            ptReporting->iFrequency = strValue.toLong();

        // Read Task Day of Week execution
        else if(strString == "DayOfWeek")
            ptReporting->iDayOfWeek = strValue.toLong();

        // Execution window flag
        else if(strString == "ExecWindow")
        {
            if(strValue == "YES")
                ptReporting->bExecutionWindow = true;
            else
                ptReporting->bExecutionWindow = false;
        }

        // Read Start-time
        else if(strString == "StartTime")
        {
            strString = strValue;
            strValue = strString.section(',',0,0);
            int iHour = strValue.toInt();
            strValue = strString.section(',',1,1);
            int iMinute = strValue.toInt();
            QTime tStartTime(iHour,iMinute);
            ptReporting->cStartTime = tStartTime;
        }

        // Read Stop-time
        else if(strString == "StopTime")
        {
            strString = strValue;
            strValue = strString.section(',',0,0);
            int iHour = strValue.toInt();
            strValue = strString.section(',',1,1);
            int iMinute = strValue.toInt();
            QTime tStopTime(iHour,iMinute);
            ptReporting->cStopTime = tStopTime;
        }

        // Read Email Notification type
        else if(strString == "NotificationType")
            ptReporting->iNotificationType = strValue.toInt();

        // Read Email from
        else if (strString == "EmailFrom") {
            ptReporting->strEmailFrom = strValue;
        }

        // Read Email notification list
        else if(strString == "Emails")
            ptReporting->strEmailNotify = strValue;

        // Read Email format: HTML or TXT
        else if(strString == "EmailFormat")
        {
            if(strValue == "HTML")
                ptReporting->bHtmlEmail = true;
            else
                ptReporting->bHtmlEmail = false;
        }

        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }
    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptReporting;
            ptTask->SetProperties(NULL);
        }
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: STATUS
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionStatus(CGexMoTaskStatus *   ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    bool bAllocate = false;
    GexMoStatusTaskData* ptStatus = ptTask->GetProperties();
    if(ptStatus == NULL)
    {
        bAllocate = true;
        ptStatus = new GexMoStatusTaskData(ptTask);
        ptTask->SetProperties(ptStatus);
    }

    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptStatus->setTitle(strValue);

        // Read Web organization type
        else if(strString == "WebOrganization")
        {
            int iOneWebPerDatabase;
            iOneWebPerDatabase = strValue.toLong();
            if(iOneWebPerDatabase)
                ptStatus->setOneWebPerDatabase(true);
            else
                ptStatus->setOneWebPerDatabase(false);
        }

        // Read Intranet path
        else if(strString == "IntranetPath")
            ptStatus->setIntranetPath(strValue);

        // Read Home page name
        else if(strString == "HomePage")
            ptStatus->setHomePage(strValue);

        // Report's URL name to display in Emails (hyperlink)
        else if(strString == "ReportURL")
            ptStatus->setReportURL(strValue);

        // Report's http URL name to display in Emails (hyperlink)
        else if(strString == "ReportHttpURL")
            ptStatus->setReportHttpURL(strValue);

        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }
    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptStatus;
            ptTask->SetProperties(NULL);
        }
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: FileConverter
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionFileConverter(CGexMoTaskConverter *    ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    bool bAllocate = false;
    GexMoFileConverterTaskData* ptConverter = ptTask->GetProperties();
    if(ptConverter == NULL)
    {
        bAllocate = true;
        ptConverter = new GexMoFileConverterTaskData(ptTask);
        ptTask->SetProperties(ptConverter);
    }

    int nField = 0;
    bool bHavePriority = false;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptConverter->strTitle = strValue;

        // Input Folder
        else if(strString == "InputFolder")
            ptConverter->strInputFolder = strValue;

        // Input file extensions
        else if(strString == "ImportFileExtensions")
            ptConverter->strFileExtensions = strValue;

        // Read Task frequency
        else if(strString == "Frequency")
            ptConverter->iFrequency = strValue.toLong();
        // Read Task frequency
        else if(strString == "Priority")
            ptConverter->mPriority = strValue.toInt(&bHavePriority);
        // Read Task Day of Week execution
        else if(strString == "DayOfWeek")
            ptConverter->iDayOfWeek = strValue.toLong();
        // Output Folder
        else if(strString == "OutputFolder")
            ptConverter->strOutputFolder = strValue;

        // Output Format: STDF (0) or CSV (1)
        else if(strString == "OutputFormat")
            ptConverter->iFormat = strValue.toInt();

        // Include Timestamp info in file name to create?
        else if(strString == "TimeStampFile")
            ptConverter->bTimeStampName = (strValue.toInt() != 0) ? true : false;

        // What to to file file successfuly converted
        else if(strString == "SuccessMode")
            ptConverter->iOnSuccess = strValue.toInt();

        // Folder where to move source files (if successfuly converted)
        else if(strString == "SuccessFolder")
            ptConverter->strOutputSuccess = strValue;

        // What to to file file that failed conversion
        else if(strString == "FailMode")
            ptConverter->iOnError = strValue.toInt();

        // Folder where to move source files (if failed conversion)
        else if(strString == "FailFolder")
            ptConverter->strOutputError = strValue;

        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }
    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptConverter;
            ptTask->SetProperties(NULL);
        }
        return false;
    }

    // Old frequency
    // < 30mn [7] => High
    // < 1h   [8] => Medium
    // else       => Low
    if(!bHavePriority)
    {
        if(ptConverter->iFrequency <= 7)
            ptConverter->mPriority = 2;
        else if(ptConverter->iFrequency <= 8)
            ptConverter->mPriority = 1;
        else
            ptConverter->mPriority = 0;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: OUTLIER REMOVAL
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionOutlierRemoval(CGexMoTaskOutlierRemoval *   ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    bool bAllocate = false;
    GexMoOutlierRemovalTaskData* ptOutlier = ptTask->GetProperties();
    if(ptOutlier == NULL)
    {
        bAllocate = true;
        ptOutlier = new GexMoOutlierRemovalTaskData(ptTask);
        ptTask->SetProperties(ptOutlier);
    }

    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptOutlier->strTitle = strValue;

        else if(strString == "Database")
            ptOutlier->strDatabase = strValue;

        else if(strString == "TestingStage")
            ptOutlier->strTestingStage = strValue;

        // Read ProductID
        else if(strString == "ProductID")
            ptOutlier->strProductID = strValue;

        // Read Alarm level
        else if(strString == "AlarmLevel")
            ptOutlier->lfAlarmLevel = strValue.toDouble();

        // Read Alarm Type: % (0) or #parts (1)
        else if(strString == "AlarmType")
            ptOutlier->iAlarmType = strValue.toInt();

        // Read Minimum parts to have a valid file
        else if(strString == "MinimumParts")
            ptOutlier->lMinimumyieldParts = strValue.toLong();

        // Notify if distribution shape changes compared to historical data
        else if(strString == "NotifyShapeChange")
            ptOutlier->bNotifyShapeChange = (bool) strValue.toLong();
        // Read Maximum number of Die mismatch between E-Test & STDF wafermaps

        else if(strString == "CompositeEtestAlarm")
            ptOutlier->lCompositeEtestAlarm = strValue.toLong();

        // Read Maximum number of Die to reject on the exclusion zone stacked wafer.
        else if(strString == "CompositeExclusionZoneAlarm")
            ptOutlier->lCompositeExclusionZoneAlarm = strValue.toLong();

        // Read Email notification list
        else if(strString == "Emails")
            ptOutlier->strEmailNotify = strValue;

        // Read Email format: HTML or TXT
        else if(strString == "EmailFormat")
        {
            if(strValue == "HTML")
                ptOutlier->bHtmlEmail = true;
            else
                ptOutlier->bHtmlEmail = false;
        }

        // Read Email message contents type to send
        else if(strString == "EmailReportType")
            ptOutlier->iEmailReportType = strValue.toInt();

        // Read Email report notification type: send as attachment or leave on server
        else if(strString == "NotificationType")
            ptOutlier->iNotificationType = strValue.toInt();

        // Read Alarm type: Standard, Critical...
        else if(strString == "ExceptionLevel")
            ptOutlier->iExceptionLevel = strValue.toInt();

        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Task[%1]: empty option '%2'").arg(ptTask->GetID()).arg(strString).toLatin1().constData());
        }
    }
    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptOutlier;
            ptTask->SetProperties(NULL);
        }
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: Auto Admin
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::LoadDbTaskSectionAutoAdmin(CGexMoTaskAutoAdmin *    ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;

    QString strString;
    QString strValue;

    bool bAllocate = false;
    GexMoAutoAdminTaskData* ptAutoAdmin = ptTask->GetProperties();
    if(ptAutoAdmin == NULL)
    {
        bAllocate = true;
        ptAutoAdmin = new GexMoAutoAdminTaskData(ptTask);
        ptTask->SetProperties(ptAutoAdmin);
    }

    int nField = 0;
    while(GetNextTaskOptionValue(ptTask->GetID(),strString,strValue))
    {
        ++nField;

        // Read Title
        if(strString == "Title")
            ptAutoAdmin->mTitle = strValue;

        // Time of day to start auto-admin
        else if(strString == "StartTime")
        {
            strString = strValue;
            strValue = strString.section(',',0,0);
            int iHour = strValue.toInt();
            strValue = strString.section(',',1,1);
            int iMinute = strValue.toInt();
            QTime tStartTime(iHour,iMinute);
            ptAutoAdmin->mStartTime = tStartTime;
        }

        // Read Web organization type
        else if(strString == "KeepReportDuration")
            ptAutoAdmin->mKeepReportDuration = strValue.toLong();

        // Read log file contents type
        else if(strString == "LogContents")
            ptAutoAdmin->mLogContents = strValue.toLong();

        // Read Email notification list
        else if(strString == "Emails")
            ptAutoAdmin->mEmailNotify = strValue;

        else if((strString == "EmailFrom") || (strString == "EmailsFrom")) // bug from SaveDb
            ptAutoAdmin->mEmailFrom = strValue;

        else if(strString == "SendInsertionLogs")
            ptAutoAdmin->mSendInsertionLogs = strValue.toInt();

        else if(strString == "LastInsertionLogsSent")
            ptAutoAdmin->mLastInsertionLogsSent = QDateTime::fromTime_t(strValue.toInt()).date();

        // Read Last time task was executed...
        else if(strString == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm:ss");
            ptTask->m_tLastExecuted = lDateTime.toTime_t();
        }
        else
        {
            // any non hard coded option
            if(!QVariant(strValue).isNull())
                ptTask->SetAttribute(strString, QVariant(strValue));
        }
    }
    if(nField < 3)
    {
        // Corrupted task
        if(bAllocate)
        {
            delete ptAutoAdmin;
            ptTask->SetProperties(NULL);
        }
        return false;
    }

    // If at the end Yield shell is set but not Sya
    // we are in the upgrade and must copy yield to sya
    if (ptTask->GetProperties()->GetAttribute(C_ShellSyaAlarm).isNull())
    {
        ptTask->GetProperties()->SetAttribute(C_ShellSyaAlarm,
                            ptTask->GetProperties()->GetAttribute(C_ShellYieldAlarm));
    }
    if (ptTask->GetProperties()->GetAttribute(C_ShellSyaAlarmCritical).isNull())
    {
        ptTask->GetProperties()->SetAttribute(C_ShellSyaAlarmCritical,
                            ptTask->GetProperties()->GetAttribute(C_ShellYieldAlarmCritical));
    }

    return true;
}

bool GS::Gex::SchedulerEngine::DumpTaskOptions(int taskId, QMap<QString, QString>& taskOptions)
{
    QString strString;
    QString strValue;

    while(GetNextTaskOptionValue(taskId,strString,strValue))
    {
        taskOptions.insert(strString, strValue);
    }

    return true;
}


///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::SaveDbTasksById(int TaskId)
{
    CGexMoTaskItem *TaskItem = NULL;
    if(TaskId > 0)
    {
        TaskItem = FindTaskInList(TaskId);
        if(TaskItem == NULL)
            return false;
    }

    return SaveDbTasks(TaskItem);
}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::SaveDbTasks(CGexMoTaskItem *   ptTask)
{
    // Check if yieldmandb is active
    if((!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            || !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
    {
        if(mTaskBeingEdited)
            return false;

        // Save all tasks in the local folder
        SaveLocalTasks();

        return true;
    }

    QString strString;
    strString = "Save Db Tasks (";

    QList<CGexMoTaskItem*> lstTasks;
    if(ptTask)
    {
        lstTasks.append(ptTask);
        strString+= ptTask->m_strName+")";
    }
    else
    {
        lstTasks = mTasksList;
        strString+= "ALL_TASKS)";
    }

    //WriteDebugMessageFile(strString);

    QListIterator<CGexMoTaskItem*> lstIteratorTask(lstTasks);

    CGexMoTaskItem *    ptTaskItem = NULL;

    QSqlQuery   clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString     strQuery;
    bool        bStatus = true;
    int         nTaskId = 0;

    lstIteratorTask.toFront();

    // Before to save
    // check if some tasks was modified
    // if not, it is not necessary to reload all
    qlonglong lLastDbUpdateCheckSum = 0;
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
        strQuery = "SELECT SUM(CRC32(last_update)) FROM ym_tasks";
    else
        strQuery = "SELECT SUM(ORA_HASH(last_update, POWER(2,16))) FROM ym_tasks";
    clQuery.exec(strQuery);
    if(clQuery.first())
        lLastDbUpdateCheckSum = clQuery.value(0).toLongLong();

    while(lstIteratorTask.hasNext())
    {
        ptTaskItem = lstIteratorTask.next();

        if(ptTask == NULL)
        {
            // List contains all tasks
            // Save only the uploaded tasks
            if(ptTaskItem->IsLocal())
                continue;
        }
        else
        {
            // If task not already uploaded
            if(ptTaskItem->IsLocal())
            {
                // upload if DB uploaded
                if(ptTaskItem->m_iDatabaseId < 0)
                    continue;
            }
            nTaskId = ptTaskItem->GetID();
            ptTaskItem->m_clLastUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();
        }


        strQuery = "SELECT last_update FROM ym_tasks WHERE task_id="+QString::number(ptTaskItem->GetID());
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            return false;
        }

        if(!clQuery.first())
        {
            // Insert
            // Get the next AutoIncrement
            QTime clTime = QTime::currentTime();
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
            {
                strQuery = "INSERT INTO ym_tasks(task_id,name, creation_date) VALUES(";
                strQuery+="ym_tasks_sequence.nextval,'" + clTime.toString("hhmmsszz") + "',SYSDATE)";
            }
            else
            {
                strQuery = "INSERT INTO ym_tasks(name, creation_date) VALUES(";
                strQuery+= "'" + clTime.toString("hhmmsszz") + "', NOW())";
            }
            if(!clQuery.exec(strQuery))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                             strQuery,
                                                                             QString::number(clQuery.lastError().number()),
                                                                             clQuery.lastError().text());
                return false;
            }


            // Retrieve the task_id
            strQuery = "SELECT task_id FROM ym_tasks WHERE name='"+clTime.toString("hhmmsszz")+"'";
            if(!clQuery.exec(strQuery))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                             strQuery,
                                                                             QString::number(clQuery.lastError().number()),
                                                                             clQuery.lastError().text());
                return false;
            }


            clQuery.first();
            ptTaskItem->SetID(clQuery.value(0).toInt());
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Save DB tasks : new taskId = %1").arg(QString::number(ptTaskItem->GetID())).toLatin1().data() );

            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            {
                ptTaskItem->m_iUserId = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId;
                ptTaskItem->m_iGroupId = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nGroupId;
            }
        }
        else
        {
            // Check if task was modified
            if(ptTask == NULL)
            {
                QDateTime clLastUpdate = clQuery.value(0).toDateTime();
                if(ptTaskItem->m_clLastUpdate.secsTo(clLastUpdate) < 0)
                {
                    // Save this task
                }
                else
                {
                    // Nothing to do
                    continue;
                }
            }
        }

        QString strSqlDate = ptTaskItem->m_clLastUpdate.toString("yyyy-MM-dd hh:mm:ss");
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
            strSqlDate = "TO_DATE('" + strSqlDate + "','YYYY-MM-DD HH24:MI:SS')";
        else
            strSqlDate = "'" + strSqlDate + "'";

        QString     strNullField = "null";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
        {
            strNullField = "''";
        }

        QString strNodeOwner = strNullField;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId() > 0)
        {
            strNodeOwner = QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId());
        }

        // Some task must be shared with all YieldMan
        if(
            (ptTaskItem->GetTaskType() == GEXMO_TASK_YIELDMONITOR)
            || (ptTaskItem->GetTaskType() == GEXMO_TASK_YIELDMONITOR_RDB)
            || (ptTaskItem->GetTaskType() == GEXMO_TASK_OUTLIER_REMOVAL)
            || (ptTaskItem->GetTaskType() == GEXMO_TASK_SPM)
            || (ptTaskItem->GetTaskType() == GEXMO_TASK_SYA)
        )
        {
            strNodeOwner = strNullField;
        }

        // Update
        // Case 7195: do not update last_update until all options have been updated
        strQuery = "UPDATE ym_tasks SET ";
        strQuery+= "  user_id=" + QString::number(ptTaskItem->m_iUserId);
        strQuery+= "  ,node_id=" + strNodeOwner;
        if(ptTaskItem->m_iGroupId <= 0)
            strQuery+= ", group_id=" + strNullField;
        else
            strQuery+= ", group_id=" + QString::number(ptTaskItem->m_iGroupId);
        if(ptTaskItem->m_iDatabaseId <= 0)
            strQuery+= ", database_id=" + strNullField;
        else
            strQuery+= ", database_id=" + QString::number(ptTaskItem->m_iDatabaseId);
        strQuery+= ", name='" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(ptTaskItem->m_strName,false) +"'";
        strQuery+= ", type=" + QString::number(ptTaskItem->GetTaskType());
        strQuery+= ", enabled=" + QString(ptTaskItem->GetEnabledState() ? "1" : "0");
        strQuery+= ", permisions=" + QString::number(ptTaskItem->m_iPermissions);
        strQuery+= " WHERE task_id="+QString::number(ptTaskItem->GetID());
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            clQuery.exec("COMMIT"); // To unlock any tables
            // Check if have to clean the DB
            // Only for not uploaded task
            if(nTaskId < 0)
                clQuery.exec("DELETE FROM ym_tasks WHERE task_id="+QString::number(ptTaskItem->GetID()));
            // Restore the original TaskId
            ptTaskItem->SetID(nTaskId);
            return false;
        }

        bStatus = ptTaskItem->SaveTaskDataToDb();

        if(!bStatus)
        {
            // Check if have to clean the DB
            // Only for not uploaded task
            if(nTaskId < 0)
            {
                clQuery.exec("DELETE FROM ym_tasks WHERE task_id="+QString::number(ptTaskItem->GetID()));
                clQuery.exec("DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTaskItem->GetID()));
                // Restore the original TaskId
                ptTaskItem->SetID(nTaskId);
            }
            return false;
        }

        // Case 7195: update last_update now all options have been updated
        strQuery = QString("UPDATE ym_tasks SET last_update=%1 WHERE task_id=%2")
                .arg(strSqlDate).arg(ptTaskItem->GetID());
        if(!clQuery.exec(strQuery))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error executiong query: %1").arg(strQuery).toLatin1().constData());
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            clQuery.exec("COMMIT"); // To unlock any tables
            // Check if have to clean the DB
            // Only for not uploaded task
            if(nTaskId < 0)
            {
                clQuery.exec("DELETE FROM ym_tasks WHERE task_id="+QString::number(ptTaskItem->GetID()));
                clQuery.exec("DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTaskItem->GetID()));
                // Restore the original TaskId
                ptTaskItem->SetID(nTaskId);
            }
            return false;
        }
        clQuery.exec("COMMIT"); // To unlock any tables
    }

    if(ptTask && (ptTask->IsUploaded()))
    {
        if(nTaskId < 0)
        {
            // Remove the old local task
            // Task uploaded
            // Add a new line
            emit sUpdateListViewItem(ptTask,QString::number(nTaskId));
            SaveLocalTasks();
        }
        return true;
    }

    clQuery.exec("COMMIT");

    // Case 7200: use CHECKSUM on last_update rather than MAX
    if(mLastDbTasksUpdateCheckSum == lLastDbUpdateCheckSum)
    {
        // To not reloadtask list if only my task was updated
        // If before to save my task, have a diff => force the update
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
            strQuery = "SELECT SUM(CRC32(last_update)) FROM ym_tasks";
        else
            strQuery = "SELECT SUM(ORA_HASH(last_update, POWER(2,16))) FROM ym_tasks";
        clQuery.exec(strQuery);
        if(clQuery.first())
            mLastDbTasksUpdateCheckSum = clQuery.value(0).toLongLong();
    }


    // Then save all other tasks in the local folder
    SaveLocalTasks();

    return true;
}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::SaveDbTaskLastExecution(CGexMoTaskItem *   ptTaskItem)
{

    if(ptTaskItem == NULL)
        return true;

    // Save only the uploaded tasks
    if(ptTaskItem->IsLocal())
        return true;

    // Check if yieldmandb is active
    if((!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            || !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
    {
        return true;
    }

    QSqlQuery   clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;

    QString strNullField = "null";
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
    {
        strNullField = "''";
    }

    QString strNodeOwner = strNullField;
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId() > 0)
    {
        strNodeOwner = QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId());
    }

    // Some task must be shared with all YieldMan
    if(
        (ptTaskItem->GetTaskType() != GEXMO_TASK_YIELDMONITOR)
        && (ptTaskItem->GetTaskType() != GEXMO_TASK_YIELDMONITOR_RDB)
        && (ptTaskItem->GetTaskType() != GEXMO_TASK_OUTLIER_REMOVAL)
        && (ptTaskItem->GetTaskType() != GEXMO_TASK_SPM)
        && (ptTaskItem->GetTaskType() != GEXMO_TASK_SYA)
    )
    {
        // Update
        strQuery = "UPDATE ym_tasks SET ";
        strQuery+= " node_id=" + strNodeOwner;
        strQuery+= " WHERE task_id="+QString::number(ptTaskItem->GetID());
        strQuery+= " AND node_id IS NULL";
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            clQuery.exec("COMMIT"); // To unlock any tables
            return false;
        }

        clQuery.exec("COMMIT"); // To unlock any tables
    }

    QDateTime lDate;
    lDate.setTime_t(ptTaskItem->m_tLastExecuted);
    strQuery = "INSERT INTO ym_tasks_options (task_id,field,value) VALUES(";
    strQuery+= QString::number(ptTaskItem->GetID())+",";
    strQuery+= "'LastExecuted','" + lDate.toString("yyyy-MM-dd hh:mm:ss")+"'";
    strQuery+= ")";
    strQuery+= " ON DUPLICATE KEY UPDATE value=GREATEST(value,'" + lDate.toString("yyyy-MM-dd hh:mm:ss")+"')";
    if(!clQuery.exec(strQuery))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                     strQuery,
                                                                     QString::number(clQuery.lastError().number()),
                                                                     clQuery.lastError().text());
        clQuery.exec("COMMIT"); // To unlock any tables
        return false;
    }

    ptTaskItem->SetPrivateAttribute("LastExecuted",lDate.toString("yyyy-MM-dd hh:mm:ss"));
    clQuery.exec("COMMIT"); // To unlock any tables
    return true;
}

bool GS::Gex::SchedulerEngine::SaveDbTaskAttributes(CGexMoTaskItem *ptTask)
{

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return false;


    QSqlQuery   clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;


    // Delete all entry from this task
    strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->GetID());
    if(!clQuery.exec(strQuery))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                     strQuery,
                                                                     QString::number(clQuery.lastError().number()),
                                                                     clQuery.lastError().text());
        return false;
    }

    QString     strAnd;
    strQuery = "INSERT INTO ym_tasks_options (task_id,field,value) VALUES";
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
        strAnd = ",";
    else
        strAnd = ";\n" + strQuery;

    // Update Private attribute from members
    ptTask->UpdatePrivateAttributes();

    // Save private attributes
    // Save public attributes
    QMap<QString, QVariant> mapAttributes = ptTask->GetAttributes();
    QMap<QString, QVariant>::const_iterator itAttribute;
    for(itAttribute = mapAttributes.begin(); itAttribute != mapAttributes.end(); itAttribute++)
    {
        if(itAttribute != mapAttributes.begin())
            strQuery += strAnd;
        strQuery += "(" + QString::number(ptTask->GetID()) + ",'"+itAttribute.key()+"',"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSqlLob(
                    itAttribute.value().toString()) + ")";
    }

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
    {
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            return false;
        }
    }
    else
    {
        if(!clQuery.exec("BEGIN\n"+strQuery+";\nEND;\n"))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Delete Task from YieldManDb
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::DeleteDbTask(CGexMoTaskItem *ptTask)
{
    bool success = true;

    if(ptTask == NULL)
        return false;

    QString strString = "Delete Db Task "+ptTask->m_strName;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().data());

    int iTaskId = ptTask->GetID();

    // Save all tasks in the local folder
    if(ptTask->IsLocal())
    {
        ptTask->DeleteTaskDetails();

        // Remove this task from the task list
        DeleteTaskInList(ptTask);

        SaveLocalTasks();
        return false;
    }
    else
    {
        // Check if yieldmandb is active
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            return false;

        // Check if yieldman is connected
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
            return false;

        QSqlQuery   clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
        QString     strQuery;

        ptTask->TraceUpdate("DELETE","START","Delete Task");

        // Delete all entry from this task
        strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(iTaskId);
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            return false;
        }

        // Delete all entry from this task
        strQuery = "DELETE FROM ym_tasks WHERE task_id="+QString::number(iTaskId);
        if(!clQuery.exec(strQuery))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(AdminEngine::eDB_Query,
                                                                         strQuery,
                                                                         QString::number(clQuery.lastError().number()),
                                                                         clQuery.lastError().text());
            ptTask->TraceUpdate("DELETE","FAIL",clQuery.lastError().text());
            return false;
        }

        // Update the Events flow
        ptTask->TraceUpdate("DELETE","PASS","Deleted");

        success = ptTask->DeleteTaskDetails();

        // Remove this task from the task list
        DeleteTaskInList(ptTask);

        // Force to reload all tasks
        mLastLocalTasksLoaded = GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-1);
        // Case 7200: use CHECKSUM on last_update rather than MAX
        mLastDbTasksUpdateCheckSum = 0;

        return success;
    }
}
