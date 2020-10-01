///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/stat.h>

#include <QDesktopWidget>

#include <gqtl_log.h>
#include <gqtl_sysutils.h>
#include <gex_shared.h>

#include "browser.h"
#include "browser_dialog.h"
#include "engine.h"
#include "report_options.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "gex_database_entry.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_taskdata.h"
#include "reporting/reporting_taskdata.h"
#include "mo_email.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_taskdata.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "admin_engine.h"
#include "command_line_options.h"

extern bool             UpdateGexScriptEngine(CReportOptions * pReportOptions);
extern GexScriptEngine* pGexScriptEngine;
extern void             WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *  pGexMainWindow;
extern CReportOptions   ReportOptions;      // Holds options (report_build.h)

void GS::Gex::SchedulerEngine::OnCheckRenameFiles(void)

{
    // If none,
    if(mFilesToRename.count() <= 0)
        return;

    // Try to delete destination file + rename temporary file
    QStringList strFailures;
    QString strSource,strDest;
    QDir    cDir;
    int     iIndex;
    bool    bSuccess;
    for ( QStringList::Iterator it = mFilesToRename.begin(); it != mFilesToRename.end(); ++it )
    {
        strSource = strDest = *it;
        bSuccess = false;

        if(strDest.endsWith(GEX_TEMPORARY_HTML))
        {
            // This is a valid temporary file name.
            iIndex = strDest.lastIndexOf(GEX_TEMPORARY_HTML);
            strDest = strDest.left(iIndex);

            // Make sure destination can be erased
#if defined unix || __MACH__
            chmod(strDest.toLatin1().constData(),0777);
#else
            _chmod(strDest.toLatin1().constData(),_S_IREAD | _S_IWRITE);
#endif

            if(QFile::exists(strSource))
            {
                // Erase destination
                cDir.remove(strDest);

                // Rename source to destination
                if(cDir.rename(strSource,strDest) == true)
                    bSuccess = true;    // Success: remove entry from the list
            }
            else
                bSuccess = true;    // Source doesn't exist!...maybe we already processed it.
        }

        // Check if successful rename...if not, add failing file to the new list.
        if(bSuccess == false)
            strFailures += strSource;
    }

    // If some files couldn't be renamed, update the new list of files to rename.
    mFilesToRename = strFailures;

    // If need to reshedule a 'rename' session, trigger timer,
    if(mFilesToRename.count() > 0)
    {
        timerRenameFiles.setSingleShot(true);
        timerRenameFiles.start(500);
    }
}

void GS::Gex::SchedulerEngine::OnCheckScheduler()
{
    CGexMoTaskItem *ptTask=NULL;
    QString strMessage;
    QString strError;
    bool    bStatus=false;

    QMap<QString, QVariant> lMI=CGexSystemUtils::GetMemoryInfo(false,false);
    GSLOG(SYSLOG_SEV_NOTICE, QString("On check scheduler: currently %1Ko used by process...")
          .arg(lMI.value("MemUsedByProcessInKo").toInt()).toLatin1().data() );

    // Check if node fully ready. If not, give it some time to get ready.
    if(GS::Gex::Engine::GetInstance().GetClientState() != GS::Gex::Engine::eState_NodeReady)
    {
        strMessage = "Waiting for client to be ready (ClientState = " + GS::Gex::Engine::GetInstance().GetClientStateName()+")";
        GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());

        // Stop Monitoring timer, to avoid cumulating OnCheckScheduler timers
        GSLOG(SYSLOG_SEV_NOTICE, "GS Scheduler Engine: stopping scheduler timer");
        timerScheduler.stop();
        mSchedulerTimerStarted = false;

        QTimer::singleShot(1000, this, SLOT(OnCheckScheduler(void)));
        return;
    }

    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
    {
        mSchedulerStopped=true;
        GSLOG(SYSLOG_SEV_WARNING, "On check scheduler impossible : bad license ");
        emit sDisplayStatusMessage("On check scheduler impossible : bad license ");
        GS::Gex::Message::information("", "On check scheduler impossible : bad license.");
        emit sPopupMessage("Your License has expired, you need to renew it...\nPlease contact "+QString(GEX_EMAIL_SALES) );
        return;
    }

    // Now that the client node is fully ready, make sure the monitoring timer has been started, else start it
    if(!mSchedulerTimerStarted)
    {
        // SCHEDULER_INTERVAL_DEFINITION (use this tag to retrieve all linked codes)
        int lSchedulerInterval = 60000;
        // Check for undocumented env var...
        // REMOVE THIS CODE
        // NOT USED BY APPS
        //char * lSI = getenv("GS_SCHEDULERENGINE_INTERVAL");
        //if(lSI)
        //{
        //    bool ok=false;
        //    int  lSIValue = QString(lSI).toInt(&ok);
        //    if (ok && lSIValue>=1)
        //        lSchedulerInterval=lSIValue*1000;
        //}
        //        if(GS::LPPlugin::ProductInfo::getInstance()->isY123WebMode())
        //            timerScheduler.start(1000);    // 1 second (1,000 Msec.)
        //        else
        GSLOG(SYSLOG_SEV_NOTICE, QString("GS Scheduler Engine: starting scheduler timer (%1 ms)")
              .arg(lSchedulerInterval).toLatin1().constData());
        timerScheduler.start(lSchedulerInterval);   // 1 minute (60,000 Msec.)
        mSchedulerTimerStarted = true;
    }

    if (GS::Gex::CSLEngine::GetInstance().IsRunning())
        return;

    //return; // 1611 test

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        // Check if the node is ready
        // A node can be paused by another process 'PAUSE_REQUESTED'
        // then the scheduler must be paused
        // If a pause is requested during a loop insertion/converter
        // The update will be done each 1 minute
        // DO THIS CHECK BEFORE THE CHECK bProcessingTaskList
        GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateNodeStatus();
    }

    // If User PAUSE, do not process next task until PAUSE released!

    if(isSchedulerStopped())
    {
        // Ensure user knows scheduler is paused (Red text)
        emit sDisplayStatusMessage("");
        // Load tasks if not already loaded
        if(!mAllTasksLoaded)
            LoadTasksList(false);
        return;
    }

    if(mSchedulerProcessing == true)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "On check scheduler : Processing Task List...");
        return;   // ignore timer calls until tasks under execution not completely finished!
    }

    // Enter into 'task processing' section
    mSchedulerProcessing = true;
    // To check the duration of this loop
    QTime lSchedulerDuration;
    lSchedulerDuration.start();

    // 6320 : temporary implementation untill good reasons to use a QTimer or a QThread
    /*
    Solution 1 : create 1 QTimer per node timed out each N mn :
        Exple : OnTimout() { runScript("recurrent_script.js"); }
        - si synchrone : pas de bug de mutex
        - si assynchrone : risque de bug, similaire a la solition 2

    Solution 2 : create 1 QThread per node :
        Exple : while(true) { runScript("recurrent_script.js"); Sleep(N mn); }
        - ne bloque pas l app
        - gex n est pas encore thread safe
    */

    ExecRecurrentScripts(GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/scripts/recurrent");
    ExecRecurrentScripts(GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/GalaxySemi/scripts/recurrent");

    GSLOG(SYSLOG_SEV_NOTICE, "On Check Scheduler...");
    // Each database associated with a task was checked
    // Check database status (can be updated or in lock mode)
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabasesList();


    // Reload tasks if file timestamp has changed and is old enough( keep 5 secs. delay to void sharing issues)!
    // Check if the tasks list is loaded
    if(!LoadTasksList(false))
    {
        GSLOG(SYSLOG_SEV_ERROR, "LoadDbTasks failed");
        mSchedulerProcessing=false;
        return;
    }

    emit sDisplayStatusMessage("");
    // If we start a new day, make sure HTML intranet web page is created, even if no files processed yet.
    static QDate cLastIntranetUpdate(1900,1,1);
    QDate cCurrentDate = QDate::currentDate();
    if((cLastIntranetUpdate.daysTo(cCurrentDate) > 1) || (cLastIntranetUpdate.dayOfYear() != cCurrentDate.dayOfYear()))
    {
        // Force to create the Web pages of the day.
        ExecuteStatusTask();
        cLastIntranetUpdate = cCurrentDate;
    }

    //////////////////////////////
    // LoadBalancing
    // while(GetNextAction == true)
    //   ExecuteAction(ActionId)
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        int         lActionId;
        QStringList lActionTypeName = CGexMoTaskItem::GetNameListFor(GS::LPPlugin::ProductInfo::getInstance()->isPATMan());

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetMasterRole())
        {
            // When all tasks were executed
            // Check all database for autoincremental update

            // If have MasterRole
            //   Loop On Tasks (excluded PAT tasks or only on PAT tasks)
            //     No more Execution Windows
            //     For DataPump, DataPumpPat, Converter Insert 0 or 1 or N file (Task Priority)
            //     If Enabled => Insert Action with FileList (GetListOfFiles)

            // First Check if there is no incremental update to do
            // Call DB incremental update function
            GexDatabaseEntry    *pDatabaseEntry=0;
            int iIndex;
            for(iIndex=0; iIndex<GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.count(); iIndex++)
            {
                pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.at(iIndex);

                if(pDatabaseEntry &&
                        pDatabaseEntry->IsCompatible() &&
                        pDatabaseEntry->IsExternal() &&
                        pDatabaseEntry->m_pExternalDatabase &&
                        pDatabaseEntry->m_pExternalDatabase->IsDbConnected() &&
                        pDatabaseEntry->m_pExternalDatabase->IsAutomaticIncrementalUpdatesEnabled())
                {
                    // Insert new action when uploaded
                    strError = InsertNewActions(pDatabaseEntry);
                }
            }

            // Then update action for Tasks list
            // if LOAD BALANCING ON
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsLoadBalancingMode())
            {
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On check scheduler : update action flow for total %1 tasks...")
                      .arg( mTasksList.count()).toLatin1().constData());
                for(iIndex=0; iIndex<mTasksList.count(); iIndex++)
                {
                    ptTask = mTasksList.at(iIndex);

                    // Ignore task when local
                    if(ptTask->IsLocal())
                        continue;

                    // Ignore task when disabled
                    if(!ptTask->GetEnabledState())
                        continue;

                    if(!ptTask->IsExecutable())
                        continue;

                    CheckTaskStatus(ptTask,"Folder|DatabaseStatus");

                    // Check if the Task can be executed (check also ExecutionWindows)
                    if(!ptTask->IsUsable(true,true))
                        continue;

                    // Insert new action when uploaded
                    strError = InsertNewActions(ptTask);


                    if(strError.startsWith("Action aborted:"))
                    {
                        // Disable this task
                        ptTask->m_iStatus = MO_TASK_STATUS_ERROR;
                        ptTask->m_strLastInfoMsg =  strError;
                        ptTask->m_LastStatusUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();
                        emit sUpdateListViewItem(ptTask,"");
                    }
                }
            }
            GS::Gex::Engine::GetInstance().GetAdminEngine().ReleaseMasterRole();
        }

        QString lActionStatus;
        while(true)
        {
            bStatus = GS::Gex::Engine::GetInstance().GetAdminEngine().GetNextAction("EXECUTION",lActionTypeName,lActionId);
            if(!bStatus)
                break;
            // Check if have an action to do
            if(lActionId <= 0)
                break;

            // Execute this Task
            lActionStatus = "PASS";
            strError = ExecuteOneAction(lActionId);
            if(!strError.isEmpty())
            {
                lActionStatus = "FAIL";
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Execute new action return [%1]...").arg(strError)
                      .toLatin1().constData());
            }
            GS::Gex::Engine::GetInstance().GetAdminEngine().CloseAction(lActionId,0,lActionStatus,"",strError);

            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateNodeStatus();

            if(isSchedulerStopped())
                break;
        }
    }

    // Ensure user knows scheduler is paused (Red text)
    emit sDisplayStatusMessage("");

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateNodeStatus();

    // If User PAUSE, do not process next task until PAUSE released!
    if(isSchedulerStopped())
    {
        mSchedulerProcessing=false;
        return;
    }

    int iIndex=0;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On check scheduler : checking total %1 tasks...")
          .arg( mTasksList.count()).toLatin1().constData());
    for(iIndex=0; iIndex<mTasksList.count(); iIndex++)
    {
        ptTask = mTasksList.at(iIndex);

        // Ignore task when disabled
        if(!ptTask->GetEnabledState())
            continue;

        if(!ptTask->IsExecutable())
            continue;

        // If LoadBalancing
        // Execute task only when local
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                && GS::Gex::Engine::GetInstance().GetAdminEngine().IsLoadBalancingMode()
                && (ptTask->IsUploaded()))
            continue;

        CheckTaskStatus(ptTask,"Folder|DatabaseStatus");

        // Check if the Task can be executed (check also ExecutionWindows or Frenquency)
        if(!ptTask->IsUsable(!isSchedulerStopped(),!isSchedulerStopped()))
            continue;

        strError = ExecuteTask(ptTask);

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateNodeStatus();

        // If User PAUSE, do not process next task until PAUSE released!
        if(isSchedulerStopped())
            break;
    }

    // Ensure user knows scheduler is paused (Red text)
    emit sDisplayStatusMessage("");

    // If temporary files to erase, do it now (could have been intermediate STDF or while unziping files for .CSL scripts executions...)
    GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

    // Allow next timer tick to enter in this function again.
    mSchedulerProcessing=false;

    // Set Date of the last check scheduler
    mLastCheckScheduler = QDate::currentDate();

    lMI=CGexSystemUtils::GetMemoryInfo(false,false);

    GSLOG(SYSLOG_SEV_NOTICE, QString("On check scheduler end: currently %1Ko used by process...")
              .arg(lMI.value("MemUsedByProcessInKo").toInt()).toLatin1().data() );

    // SCHEDULER_INTERVAL_DEFINITION (use this tag to retrieve all linked codes)
    // Check if we need to execute directly a new OnScheduler
    if(lSchedulerDuration.elapsed() > timerScheduler.interval())
        QTimer::singleShot(1000, this, SLOT(OnCheckScheduler(void)));

}

/******************************************************************************!
 * \fn ExecRecurrentScripts
 ******************************************************************************/
void GS::Gex::SchedulerEngine::ExecRecurrentScripts(const QString &lFolder) const
{
    QDir dir;
    //dir.setPath(GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/scripts/recurrent");
    dir.setPath(lFolder);
    QStringList filters("*.js");
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty())
        return;

    QFileInfoList::const_iterator iter;
    for (iter  = list.begin();
         iter != list.end(); ++iter)
    {
        QFile scriptFile((*iter).absoluteFilePath());
        if (scriptFile.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&scriptFile);
            QString     contents = stream.readAll();
            scriptFile.close();
            if (!contents.isEmpty())
            {
                // GS::Gex::Engine::GetInstance().
                //     GetScriptEngineDebbugger().detach();
                if (!UpdateGexScriptEngine(&ReportOptions))
                {
                    GSLOG(SYSLOG_SEV_WARNING,"Failed to update Gex Script Engine");
                }
                QScriptValue v = pGexScriptEngine->evaluate(contents);
                if (v.isError() || pGexScriptEngine->hasUncaughtException())
                {
                    QString m = QString("%1 script evaluation : '%2'").
                        arg((*iter).fileName()).
                        arg(pGexScriptEngine->uncaughtException().toString());
                    if (m.startsWith("error"))
                    {
                        GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
                    }
                    // TODO : send an email about this error ?

                }
                // Let s remove any debuuger unless JavaScriptCenter
                // GS::Gex::Engine::GetInstance().
                //     GetScriptEngineDebbugger().attachTo(pGexScriptEngine);

                unsigned int loglevel = SYSLOG_SEV_WARNING;
                if (v.toString().startsWith("ok"))
                {
                    loglevel = SYSLOG_SEV_NOTICE;
                }
                GSLOG(loglevel, QString("Execution of %1 script returned : %2").
                      arg((*iter).fileName()).
                      arg(v.toString()).toLatin1().constData());
            }
        }
    }
    // GCORE-499
    pGexScriptEngine->collectGarbage();
}

///////////////////////////////////////////////////////////
// When a task start : send a signal to get a task id from
// the task manager
int GS::Gex::SchedulerEngine::onStartTask(CGexMoTaskItem *ptTask,QString Command)
{
    if(!mAllowed) return 0;

    if(ptTask == NULL)
        return 0;

    ptTask->TraceExecution(Command,"START",Command);

    ptTask->iTaskMngId =  onStartTask(ptTask->GetTaskType());
    return ptTask->iTaskMngId;
}

///////////////////////////////////////////////////////////
// When a task start : send a signal to get a task id from
// the task manager
int GS::Gex::SchedulerEngine::onStopTask(CGexMoTaskItem *ptTask, QString Error)
{
    if(!mAllowed)
        return 0;

    if(ptTask == NULL)
        return 0;

    ptTask->TraceExecution("",Error,Error);

    // Update the 'Last executed' timestamp
    ptTask->m_tLastExecuted = GS::Gex::Engine::GetInstance().GetServerDateTime().toTime_t();
    SaveDbTaskLastExecution(ptTask);

    emit sStoppedTask(ptTask->iTaskMngId);
    ptTask->iTaskMngId = 0;
    ptTask->mActionMngAttributes.clear();

    return ptTask->iTaskMngId;
}
///////////////////////////////////////////////////////////
// When a task start : send a signal to get a task id from
// the task manager
int GS::Gex::SchedulerEngine::onStartTask(int iType)
{
    int iTaskId             = 0;
    mLastTaskIdReceived   = 0;
    mTaskIdReceived       = false;

    // Emit signal for task manager
    emit sStartTask(iType);

    while (!mTaskIdReceived)
    {
        // waiting for task manager answer
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    iTaskId = mLastTaskIdReceived;
    mTaskIdReceived = false;

    return iTaskId;
}

/////////////////////////////////////////////////////
// stop insertion on DBname
// if DBname empty, stop for all DB
/////////////////////////////////////////////////////
// CALL StopProcess of plugin Galaxy
/////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::StopProcess(const QString &DBname)
{
    if(!isSchedulerRunning())
    {
        // Nothing to do
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Nothing to abort").toLatin1().constData() );
        return "ok";
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Trying to abort current task(s) on DB %1").arg( DBname).toLatin1().constData() );

    QList<GexDatabaseEntry*>::iterator itBegin	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();
    GexDatabaseEntry* pDatabaseEntry;
    while(itBegin != itEnd)
    {
        pDatabaseEntry = *itBegin;

        // Move to next enry.
        itBegin++;

        if(!DBname.isEmpty()
                && pDatabaseEntry->LogicalName() != DBname)
            continue;
        if(!pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
            continue;
        if(!pDatabaseEntry->StatusFlags() & STATUS_INSERTION)
                continue;

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Found a datapump on DB target %1").arg( pDatabaseEntry->LogicalName()).toLatin1().constData() );
        if(pDatabaseEntry->m_pExternalDatabase)
            pDatabaseEntry->m_pExternalDatabase->StopProcess();
    };

    return "error : check me";
}

///////////////////////////////////////////////////////////
// Require all running task to stop
void GS::Gex::SchedulerEngine::OnStopAllRunningTask()
{
    GSLOG(SYSLOG_SEV_NOTICE, "onStopAllRunningTask...");

    if(isSchedulerRunning())
        RunTaskScheduler(false);
}

///////////////////////////////////////////////////////////
// Require all pending task to restart
void GS::Gex::SchedulerEngine::OnRestartAllPendingTask()
{
    GSLOG(SYSLOG_SEV_NOTICE, "OnRestartAllPendingTask clicked");
    RunTaskScheduler(true);
}

///////////////////////////////////////////////////////////
// Upload given task in YieldManDb
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::OnUploadTask(CGexMoTaskItem *ptTask)
{

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return;

    QString strString;
    if( ptTask && (ptTask->IsLocal()))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" Uploading Task %1")
              .arg( ptTask?ptTask->m_strName:"NULL").toLatin1().constData());
    }
    else
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" Saving Task %1")
              .arg( ptTask?ptTask->m_strName:"NULL").toLatin1().constData());

    // Check if yieldman is connected
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    CGexMoTaskItem      *ptTaskItem = ptTask;
    int iTaskId;
    /* No upload from GUI
    if(ptTaskItem == NULL)
    {
        QList<CGexMoTaskItem*> lstTasksSelected = GetSelectedTasks();

        if(!lstTasksSelected.isEmpty())
            ptTaskItem = lstTasksSelected.first();
    }
    */

    if(ptTaskItem)
    {
        iTaskId = ptTaskItem->GetID();

        strString = "Task "+ptTaskItem->m_strName+"["+QString::number(iTaskId)+"]";
        //WriteDebugMessageFile(strString);
        strString = "Database["+QString::number(ptTaskItem->m_iDatabaseId)+"]";
        //WriteDebugMessageFile(strString);
        strString = "Status["+ptTaskItem->m_strLastInfoMsg+"]";
        //WriteDebugMessageFile(strString);

        // ALLOW ALL TYPE OF DATABASES
        if((ptTaskItem->IsLocal()) // if task already uploaded, don't check error
                && (ptTaskItem->m_iDatabaseId < 0))
        {
            QString strError;
            QString strMessage = ptTaskItem->m_strLastInfoMsg;
            if(strMessage.isEmpty())
                strMessage = "Task "+ptTaskItem->m_strName+" with Database not uploaded cannot be uploaded";
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(strError, strMessage);
            WriteDebugMessageFile(strMessage);

            // Task already loaded and comes from Local folder
            // Upload call by system, not the user
            // Save in Local Folder
            //SaveLocalTasks();
            //LoadDbTasks(false);

            return;
        }

        // Save new task list on disk
        if(!SaveDbTasks(ptTaskItem))
        {
            QString strError;
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
                GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(strError);
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && strError.isEmpty())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
            WriteDebugMessageFile("SaveDbTasks() return an error ["+strError+"]");

            /*
            if(iTaskId < 0)
            {
                // Save in Local Folder
                SaveLocalTasks();
            }
            */
        }
        // Check if the task correctly uploaded
        if((iTaskId < 0) && (ptTaskItem->IsUploaded()))
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On upload task : task %1[%2] uploaded").arg(ptTaskItem->m_strName)
                  .arg(ptTaskItem->GetID()).toLatin1().constData());
            // Remove the old line in the list view
            // Line is referenced with the old TaskId
            // Remove by the SaveDbTask
            //emit sUpdateListViewItem(ptTaskItem,QString::number(iTaskId));
        }

        LoadTasksList(false);
    }
}


