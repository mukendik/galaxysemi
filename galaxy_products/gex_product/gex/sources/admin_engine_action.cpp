///////////////////////////////////////////////////////////
// Database admin: Create/Delete database, insert files
///////////////////////////////////////////////////////////
#include <QShortcut>
#include <QMenu>
#include <QInputDialog>
#include <QSqlError>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QRadioButton>
#include <QToolButton>

#include "browser_dialog.h"
#include "admin_engine.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "engine.h"
#include <gqtl_log.h>
#include "libgexoc.h"
#include "product_info.h"
#include "scheduler_engine.h"
#include "mo_task.h"

namespace GS
{
namespace Gex
{


bool    AdminEngine::CleanActions(QString Options)
{
    //     Delete old mutex
    //     Delete Disabled tasks

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Clean action table with Options [%1]...")
          .arg(Options)
          .toLatin1().constData());

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    if(!Options.isEmpty())
    {
        strQuery = "DELETE FROM ym_actions WHERE "+Options;
        strQuery+= " AND category='EXECUTION'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        clQuery.exec("COMMIT");
        return true;
    }

    QStringList MutexToDelete;
    // Select all mutex and check if always active
    strQuery = "SELECT DISTINCT mutex FROM ym_actions WHERE mutex IS NOT NULL AND mutex <> '' ";
    strQuery+= "AND category='EXECUTION' ";
    // To avoid some unexpected quarantine
    // Focus only on actions oldest than 1h
    // On the same Host, take the list
    // On other Host, whait 1 hour
    strQuery+= "AND (mutex like concat('%:',@@hostname) ";
    strQuery+= "  OR (start_time<DATE_SUB(now(),INTERVAL 1 hour))) ";
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    while(clQuery.next())
    {
        if(!IsMutexActive(clQuery.value(0).toString()))
            MutexToDelete << clQuery.value(0).toString();
    }

    // Delete old mutex
    // For all node crashed
    // Move file to quarantine or bad
    bool        Retry;
    QString     Status;
    QString     Mutex;
    int         TaskId;
    QString     FileName;
    QString     Command;
    QString     FileDest;
    int         ActionId;
    QString     NodeList;
    QStringList ActionsToDelete;

    QString RetryOption;
    QString RetryHouseKeeping;
    QString RetryEmail;

    // Default value is retry one time
    int RetryAfterCrash = 1;
    if(!MutexToDelete.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Clean action found invalid mutex [%1]...")
              .arg(MutexToDelete.join(","))
              .toLatin1().constData());

        if(!GetNodeSettingsValue("LOADBALANCING_QUARANTINE_RETRY_OPTION", RetryOption))
            RetryOption = "LOADBALANCING_QUARANTINE_RETRY_OPTION";
        else
            RetryAfterCrash = RetryOption.toInt();
        if(!GetNodeSettingsValue("LOADBALANCING_QUARANTINE_HOUSEKEEPING_OPTION", RetryHouseKeeping))
            RetryHouseKeeping = "LOADBALANCING_QUARANTINE_HOUSEKEEPING_OPTION";
        if(!GetNodeSettingsValue("LOADBALANCING_QUARANTINE_EMAIL_OPTION", RetryEmail))
            RetryEmail = "LOADBALANCING_QUARANTINE_EMAIL_OPTION";
    }

    while(!MutexToDelete.isEmpty())
    {
        ActionsToDelete.clear();
        Mutex = MutexToDelete.takeFirst();
        strQuery = "SELECT action_id,node_list,task_id,command,category,type,start_time,database_id,status FROM ym_actions ";
        strQuery+= "WHERE (status='RUNNING' OR status='CANCELED') AND mutex='"+Mutex+"' ";
        strQuery+= "AND category='EXECUTION'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            break;
        }
        while(clQuery.next())
        {
            Retry = false;

            ActionId = clQuery.value(0).toInt();
            NodeList = clQuery.value(1).toString();
            TaskId = clQuery.value(2).toInt();
            Command = FileName = clQuery.value(3).toString();
            Status = clQuery.value(8).toString();

            // Retrieve original command
            if(SplitCommand(Command).contains("FileName"))
                FileName = "FileName="+SplitCommand(Command)["FileName"];

            QString Summary;
            if(Status == "CANCELED")
            {
                Retry = true;
                QMap<QString,QString> info = SplitCommand(Command);
                if(info.contains("insertionstatus")
                        && info["insertionstatus"] == "")
                {
                    // insertion canceled
                    info.clear();
                    info["error"] = "Insertion was canceled";
                    info["retryoption"] = "Restart action";

                }
                else
                {
                    info["error"] = "Action was canceled but Insertion done";
                    info["retryoption"] = "Finalize action";
                }
                info["status"] = "CANCELED";
                Summary = JoinCommand(info);
            }
            else
            {
                Summary = "error=Action ended unexpectedly";
                Summary += "|retryoption="+RetryOption;
                if(RetryAfterCrash > 0)
                {
                    // Check how many time this action was executed
                    // node_list = | or |1| or |1|1| ...
                    if((NodeList.count("|")-1) < RetryAfterCrash){
                        Summary += "|retry="+QString::number(NodeList.count("|")-1);
                        Retry = true;
                    }
                    else {
                        Summary += "|retry=move to quarantine";
                        Summary += "|housekeepingoption="+RetryHouseKeeping;
                        Summary += "|emailoption="+RetryEmail;
                    }
                }

                if(FileName.startsWith("filename=",Qt::CaseInsensitive))
                    Summary += "|sourcearchive="+FileName.section("=",1);
                Summary += "|status=UNEXPECTED";
            }

            if(!Retry && FileName.startsWith("filename=",Qt::CaseInsensitive))
            {
                // Move file to quarantine
                QString Error = GS::Gex::Engine::GetInstance().GetSchedulerEngine().ProcessFileAfterExecution(-1,TaskId,FileName.section("=",1),FileDest);
                if(Error.isEmpty())
                    Summary += "|fulldestinationname="+FileDest;
                else
                    Summary += "|error="+Error;

                ActionsToDelete << QString::number(ActionId);
                GSLOG(SYSLOG_SEV_ERROR, QString("Clean action found invalid actions [%1] on %2...")
                      .arg(FileName)
                      .arg(Mutex)
                      .toLatin1().constData());

            }
            AddNewEvent(clQuery.value(4).toString(), clQuery.value(5).toString(), clQuery.value(6).toDateTime(),
                        0, TaskId, clQuery.value(7).toInt(), FileName, "UNEXPECTED",
                        Summary, "");

        }

        // Then delete line
        if(!ActionsToDelete.isEmpty())
        {
            // Do not restart this action
            strQuery = "DELETE FROM ym_actions ";
            strQuery+= "WHERE status='RUNNING' AND mutex='"+Mutex+"' ";
            strQuery+= "AND action_id IN ("+ActionsToDelete.join(",")+")";
            if(!clQuery.exec(strQuery))
                SetLastError(clQuery);
        }

        // Then restart other actions
        strQuery = "UPDATE ym_actions ";
        strQuery+= "SET status='READY' ";
        strQuery+= ", mutex=null ";
        strQuery+= ", start_time=null ";
        strQuery+= "WHERE (status='RUNNING' OR status='CANCELED') AND mutex='"+Mutex+"' ";
        if(!clQuery.exec(strQuery))
            SetLastError(clQuery);
    }
    clQuery.exec("COMMIT");
    return true;
}

// Where a stop is requested, if node execute a task
// this task must be flagged with CANCELED
bool    AdminEngine::CancelActions()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Cancel active action table...")
          .toLatin1().constData());

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));


    // Update active actions from ME
    strQuery = "UPDATE ym_actions ";
    strQuery+= "SET status='CANCELED' ";
    strQuery+= "WHERE status='RUNNING' AND mutex='"+mMutex+"' ";
    if(!clQuery.exec(strQuery))
        SetLastError(clQuery, true);

    clQuery.exec("COMMIT");
    return true;
}


bool    AdminEngine::AddNewAction(QString Category, QString Type, int TaskId, int DatabaseId, QString Command, QString& Error)
{
    // When the node got the MASTER role, he stored the active actions information
    // to be able to reject the insertion of new actions because of duplication
    // The AddNewAction will prepare the insertion of a bulck actions
    // instead of insert action after action
    // The actions list prepared during this step
    // will be inserted on the Release of the MASTER role
    // This way, this will limit the number of access/update of the actions table
    // and probably fix some TIMEOUT issue
    GSLOG(SYSLOG_SEV_DEBUG, QString("Add new action for task[%1], database[%2], command[%3]...")
          .arg(QString::number(TaskId),QString::number(DatabaseId),Command)
          .toLatin1().constData());
    Error = "";

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    QString lKey = !Command.isEmpty() ? Command : QString::number(TaskId);

    if(mActionsInfoList.contains(lKey))
    {
        QMap<QString,QString> lActionInfo;
        lActionInfo = mActionsInfoList[lKey];

        // Before inserting a new file to process
        // Check if it is not already inserted
        if(Command.startsWith("FileName="))
        {
            // The file still processed by another node
            // Ignore this file
            if(lActionInfo["Status"] != "READY")
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Add new action: File already inserted and still processed by another node. FileName[%1], Status[%2], Mutex[%3]...")
                      .arg(Command.section("=",1))
                      .arg(lActionInfo["Status"])
                        .arg(lActionInfo["Mutex"])
                        .toLatin1().constData());
                return true;
            }

            // GCORE-2202
            // If the file was inserted the last time by the same task
            // Ignore it
            if(TaskId == lActionInfo["TaskId"].toInt())
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Add new action: File already inserted by this task. FileName[%1], Status[%2]...")
                      .arg(Command.section("=",1))
                      .arg(lActionInfo["Status"])
                        .toLatin1().constData());
                return true;
            }

            strQuery = "SELECT name FROM ym_tasks ";
            strQuery+= "WHERE task_id="+lActionInfo["TaskId"];
            clQuery.exec(strQuery);
            clQuery.first();

            Error = "Action aborted:";
            Error+= " File already targeted by this Task:\n";
            Error+= " Task="+clQuery.value(0).toString()+",\n ";
            Error+= Command;
            Error+= ". \nCHECK YOUR TASKS:\n - equivalent (sub-)directory.\n - equivalent (sub-)extensions.";
            Error+= " \nPlease contact Quantix support at ";
            Error+=  QString(GEX_EMAIL_SUPPORT);
            return false;
        }

        // Before inserting a new incremental_update
        // Check if it is not already inserted
        else if(Type=="INCREMENTAL_UPDATE")
        {

          // The IncrementalUpdate still processed by another node
          // Ignore this IncrementalUpdate
          if(lActionInfo["Status"] != "READY")
          {
              GSLOG(SYSLOG_SEV_WARNING, QString("Add new action: IncrementalUpdate still processed by another node. Command[%1], Status[%2], Mutex[%3]...")
                    .arg(Command)
                    .arg(lActionInfo["Status"])
                      .arg(lActionInfo["Mutex"])
                      .toLatin1().constData());
          }
          else
          {
              GSLOG(SYSLOG_SEV_WARNING, QString("Add new action: IncrementalUpdate already inserted with this key. Command[%1], DatabaseId[%2]...")
                    .arg(Command)
                    .arg(DatabaseId)
                      .toLatin1().constData());

          }
          return true;
        }

        // Avoid inserting twice a parameterless task
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Add new action: Task already inserted with this ID: [%1]...")
                  .arg(TaskId).toLatin1().constData());
            return true;
        }
    }

    // This action can be inserted
    mActionsInfoList[lKey]["Category"]=Category;
    mActionsInfoList[lKey]["Type"]=Type;
    mActionsInfoList[lKey]["TaskId"]=(TaskId<=0?"null":QString::number(TaskId));
    mActionsInfoList[lKey]["DatabaseId"]=(DatabaseId<=0?"null":QString::number(DatabaseId));
    mActionsInfoList[lKey]["Command"]=Command;
    mActionsInfoList[lKey]["Mutex"]="";
    mActionsInfoList[lKey]["Status"]="READY";

    // Insert this action into the sorted list
    mActionsList.append(lKey);
    return true;
}

bool    AdminEngine::GetNextAction(QString Category, QStringList Type, int &ActionId)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Get new action for Category [%1], Type [%2]...")
          .arg(Category)
          .arg(Type.join(","))
          .toLatin1().constData());

    QString        strQuery;
    QString        strQueryClause;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Wake Up !
    ConnectToAdminServer();

    ActionId = -1;
    mEventsExecutionStack.clear();

    // Select only one action from the list
    strQueryClause= " WHERE status='READY' ";
    // If this node already tried this action
    // strQueryClause+= "AND node_list NOT LIKE '%|"+QString::number(m_nNodeId)+"|%' ";
    strQueryClause+= "AND mutex IS NULL ";
    strQueryClause+= "AND category='"+Category+"' ";
    strQueryClause+= "AND type IN ('"+Type.join("','")+"') ";
    strQueryClause+= " %1 ";
    strQueryClause+= "LIMIT 1";

    strQuery = "SELECT action_id, now() FROM ym_actions ";
    // +strQueryClause.arg("ORDER BY action_id");
    // Remove the ORDER BY that can fail with some server
    // and that is not useful here
    strQuery+= strQueryClause.arg(" ");
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }

    // At least one action is READY
    if(clQuery.first())
    {
        QString strDate = "now()";
        // Because some ghost actions can stay into the ym_actions
        // We need to update and select a new action
        // according to a specific date
        // MySql format 2015-09-02 04:52:22
        strDate = NormalizeDateToSql(clQuery.value(1).toDateTime());

        // Have one or more action READY
        // Try to lock one of them
        // Not especially the same as selected before
        strQuery = "UPDATE ym_actions SET ";
        strQuery+= "start_time="+strDate+", ";
        strQuery+= "node_list=concat(node_list,concat('"+QString::number(m_nNodeId)+"','|')), ";
        strQuery+= "mutex="+NormalizeStringToSql(mMutex)+", ";
        strQuery+= "status='RUNNING' ";
        // Remove the ORDER BY that can fail with some server
        strQuery+= strQueryClause.arg(" ");
        if(!clQuery.exec(strQuery))
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Node[%1][%2] was not able to lock a new action. Will try later...")
                  .arg(m_nNodeId).arg(mMutex)
                  .toLatin1().constData());
            SetLastError(clQuery);
            return false;
        }
        clQuery.exec("COMMIT");

        // Check if the action was locked
        strQuery = "SELECT action_id FROM ym_actions ";
        strQuery+= "WHERE mutex="+NormalizeStringToSql(mMutex)+" ";
        //strQuery+= "AND node_list LIKE '%"+QString::number(m_nNodeId)+"%' ";
        strQuery+= "AND status='RUNNING' ";
        // GCORE-4202
        // Add the date to select the good action
        // if some unexpected ghost actions
        strQuery+= "AND start_time="+strDate;
        if(!clQuery.exec(strQuery))
            SetLastError(clQuery, true);

        if(clQuery.first())
        {
            // Action locked
            ActionId = clQuery.value(0).toInt();

            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("Node[%1][%2] locked action[%3]")
                  .arg(m_nNodeId).arg(mMutex).arg(ActionId)
                  .toLatin1().constData());

            // GCORE-4202
            // Each time we lock an action with a valid mutex
            // Clean the ym_actions if there is some old lines issues
            // ie an action line that must be removed in the past
            // but for unexpected reason which still present
            // Check if there are some old actions to remove
            // and trace the query
            strQuery = "SELECT count(*) FROM ym_actions ";
            strQuery+= "WHERE mutex="+NormalizeStringToSql(mMutex)+" ";
            strQuery+= "AND action_id<>"+QString::number(ActionId);
            if(!clQuery.exec(strQuery))
                SetLastError(clQuery);
            else if(clQuery.value(0).toInt() > 0)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Remove %1 Ghost actions found for Node[%2]Mutex[%3]")
                      .arg(clQuery.value(0).toInt())
                      .arg(m_nNodeId).arg(mMutex).toLatin1().constData());
                strQuery = "DELETE FROM ym_actions ";
                strQuery+= "WHERE mutex="+NormalizeStringToSql(mMutex)+" ";
                strQuery+= "AND action_id<>"+QString::number(ActionId);
                if(!clQuery.exec(strQuery))
                    SetLastError(clQuery);
            }
            clQuery.exec("COMMIT");
            return true;
        }
        // Action not locked
        // Try again
        return GetNextAction(Category,Type, ActionId);
    }

    return false;
}

bool    AdminEngine::GetActionInfo(int ActionId, QString &ActionType,int &TaskId, int &DatabaseId, QString& Command)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Get new action info for ActionId [%1]...")
          .arg(ActionId)
          .toLatin1().constData());

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    TaskId = DatabaseId = -1;
    Command = ActionType = "";
    strQuery = "SELECT task_id, type, database_id, command FROM ym_actions ";
    strQuery+= "WHERE action_id="+QString::number(ActionId);
    if(!clQuery.exec(strQuery))
        SetLastError(clQuery, true);
    if(!clQuery.first())
        return false;

    TaskId          = clQuery.value(0).toInt();
    ActionType      = clQuery.value(1).toString();
    DatabaseId      = clQuery.value(2).toInt();
    Command         = clQuery.value(3).toString();

    return true;
}

bool    AdminEngine::UpdateAction(int ActionId, QString Command)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Update action info for ActionId [%1]...")
          .arg(ActionId)
          .toLatin1().constData());

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Wake Up !
    ConnectToAdminServer();

    strQuery = "UPDATE ym_actions ";
    strQuery+= "SET command="+NormalizeStringToSqlLob(Command);
    strQuery+= "WHERE action_id="+QString::number(ActionId);
    if(!clQuery.exec(strQuery))
        SetLastError(clQuery, true);
    clQuery.exec("COMMIT");

    return true;
}

bool    AdminEngine::CloseAction(int ActionId, int /*Size*/,
                                 QString Status, QString Summary,
                                 QString Comment)
{

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Close action with ActionId[%1], Status[%2], Summary[%3], Comment[%4]...")
          .arg(ActionId)
          .arg(Status)
          .arg(Summary)
          .arg(Comment)
          .toLatin1().constData());

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Wake Up !
    ConnectToAdminServer();

    // First start to remove the action
    // Instead to try to update it before
    // If the update failed, the delete wasn't executed
    // Go to Event log
    strQuery = "DELETE FROM ym_actions ";
    strQuery+= "WHERE action_id="+QString::number(ActionId);
    if(!clQuery.exec(strQuery))
        SetLastError(clQuery, true);
    clQuery.exec("COMMIT");
    return true;
}


bool    AdminEngine::GetMasterRole()
{
    if(!ConnectToAdminServer())
        return false;

    // The 2 first lines are for MASTER_YM and MASTER_PAT
    //     Clean role for old mutex
    //     Delete Disabled tasks

    QStringList Types = CGexMoTaskItem::GetNameListFor(GS::LPPlugin::ProductInfo::getInstance()->isPATMan());
    QString     strQuery;
    QSqlQuery   clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Check if have some actions to do
    strQuery = "SELECT action_id FROM ym_actions ";
    strQuery+= "WHERE status='READY' ";
    strQuery+= "AND mutex IS NULL ";
    strQuery+= "AND category='EXECUTION' ";
    strQuery+= "AND type IN ('"+Types.join("','")+"')";
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if(clQuery.first())
    {
        // Already have some actions to do
        // Don't need to take the master role
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Node[%1][%2] try to get the MASTER role but there are already some actions to do...")
              .arg(m_nNodeId).arg(mMutex)
              .toLatin1().constData());
        return false;
    }

    QString lApplicationName = "YIELD-MAN";;
    QString lMasterRole = "MASTER_YM";;
    int lMasterActionId = 1;
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
    {
        lApplicationName = "PAT-MAN";
        lMasterRole = "MASTER_PAT";
        lMasterActionId = 2;
    }

    // Check if the 2 first line is for MASTER role
    // Check if have the MASTER role
    // Don't use the node_list for the MASTER role (to preserve the DUPLICATED check on '1,|' from old GEX with the new PK(node_list,action_id))
    // Then it is not possible to check which node took the MASTER role
    strQuery = "SELECT mutex, start_time, DATE_ADD( now(), INTERVAL -1 MINUTE ) as last_min, DATE_ADD( now(), INTERVAL -1 HOUR ) as last_hour ";
    //strQuery+= ", REPLACE(node_list,'|','') as last_node ";
    strQuery+= "FROM ym_actions ";
    strQuery+= "WHERE action_id="+QString::number(lMasterActionId);
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if(!clQuery.first())
    {
        // No line found for master role
        // Insert a new one
        // If the key is already into the table (inserted by another process), nothing inserted
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Node[%1][%2] insert the missing line for MASTER role.")
              .arg(m_nNodeId).arg(mMutex)
              .toLatin1().constData());
        strQuery = "INSERT INTO ym_actions VALUES(now(),"+QString::number(lMasterActionId)+",'ADMINISTRATION','"+lMasterRole+"',now(),'|',null,null,null,'READY','MASTER ROLE FOR "+lApplicationName+"')";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        clQuery.exec("COMMIT");
    }
    else
    {
        // SCHEDULER_INTERVAL_DEFINITION (use this tag to retrieve all linked codes)
        // THE GetMasterRole USES 60s BETWEEN EACH MASTER ACTIVITIES DEFINED INTO timerScheduler
        // Have a line for the MASTER role
        // Check if the MASTER role was given less than 1mn ago
        // In this case, nothing to do
        // The last node_id is not saved anymore into the ym_actions for the MASTER role
        // Don't check the last execution
        // This test is only effective if there is nthing to do ...
        //if(clQuery.value(1).toDateTime()>clQuery.value(2).toDateTime())
        //{
        //    // The last time a YM took the MASTER role was less than 1mn
        //    // But if it was me, I want to ignore the time check
        //    // and force the execution
        //    if(m_nNodeId != clQuery.value(4).toInt())
        //    {
        //        // Don't need to take the master role
        //        GSLOG(SYSLOG_SEV_DEBUG,
        //              QString("Node[%1][%2] try to get the MASTER role but another node took the role less than 1 min ago...")
        //              .arg(m_nNodeId).arg(mMutex)
        //              .toLatin1().constData());
        //        return false;
        //    }
        //}

        if(!clQuery.value(0).toString().isEmpty())
        {
            // If the line is already locked
            // we need to check the validity of the lock
            // Release the master role for all if the start_time is > 1h
            // or release the master role if the mutex is obsolete
            if((clQuery.value(1).toDateTime()>clQuery.value(3).toDateTime())
                    && IsMutexActive(clQuery.value(0).toString()))
            {
                // The line is already locked
                // The mutex is valid
                // The lock was done less than 1h ago
                // Another node is preparing the actions to do
                // Don't need to take the master role
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Node[%1][%2] try to get the MASTER role but another node already has it...")
                      .arg(m_nNodeId).arg(mMutex)
                      .toLatin1().constData());
                return false;
            }

            // Find old lock (> 1h), just auto release
            // Find invalid mutex, just auto release

            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Found Ghost MASTER role on mutex[%1] or start_time[%2]")
                  .arg(clQuery.value(0).toString()).arg(clQuery.value(1).toDateTime().toString())
                  .toLatin1().constData());

            strQuery = "UPDATE ym_actions ";
            strQuery+= "SET mutex=NULL,";
            strQuery+= "status='READY' ";
            strQuery+= "WHERE ((mutex='";
            strQuery+= clQuery.value(0).toString();
            strQuery+= "') OR (start_time<DATE_ADD( now(), INTERVAL -1 HOUR ))) ";
            strQuery+= "AND action_id="+QString::number(lMasterActionId);
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
            clQuery.exec("COMMIT");
        }
    }

    // Then try to update and get the master role
    strQuery = "UPDATE ym_actions ";
    strQuery+= "SET mutex='"+mMutex+"', ";
    // Don't use the node_list for the MASTER role (to preserve the DUPLICATED check on '1,|' from old GEX with the new PK(node_list,action_id))
    strQuery+= "node_list='|', ";
    strQuery+= "status='RUNNING', ";
    strQuery+= "start_time=now() ";
    strQuery+= "WHERE mutex IS NULL ";
    strQuery+= "AND status='READY' ";
    strQuery+= "AND category='ADMINISTRATION' ";
    strQuery+= "AND type='"+lMasterRole+"' ";
    strQuery+= "AND action_id="+QString::number(lMasterActionId);
    if(!clQuery.exec(strQuery))
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Node[%1][%2] was not able to get the MASTER role. Will try later...")
              .arg(m_nNodeId).arg(mMutex)
              .toLatin1().constData());
        SetLastError(clQuery);
        return false;
    }
    clQuery.exec("COMMIT");

    // Check if have the MASTER role
    strQuery = "SELECT action_id FROM ym_actions ";
    strQuery+= "WHERE mutex='"+mMutex+"' ";
    strQuery+= "AND status='RUNNING' ";
    strQuery+= "AND category='ADMINISTRATION' ";
    strQuery+= "AND type='"+lMasterRole+"' ";
    strQuery+= "AND action_id="+QString::number(lMasterActionId);
    if(!clQuery.exec(strQuery))
        SetLastError(clQuery, true);
    if(clQuery.first())
    {
        QString lKey, lCategory, lType, lTaskId, lDatabaseId, lStatus, lCommand, lMutex;
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Node[%1][%2] has the MASTER role")
              .arg(m_nNodeId).arg(mMutex)
              .toLatin1().constData());

        // First check if have some task to do
        //   Clean the Action table
        //     Delete old mutex
        //     Delete Disabled tasks
        GS::Gex::Engine::GetInstance().GetAdminEngine().CleanActions();

        // When the actions table was correctly clean
        // Take a list of all actions running into the actions table
        // Then we will be able to reject some action insertion because of duplication
        mActionsInfoList.clear();
        mActionsList.clear();
        // Check if have some actions to do
        strQuery = "SELECT category, type, task_id, database_id, status, command, mutex FROM ym_actions ";
        strQuery+= "WHERE category='EXECUTION' ";
        strQuery+= "AND type IN ('"+Types.join("','")+"')";
        if(!clQuery.exec(strQuery))
            SetLastError(clQuery,true);

        while(clQuery.next())
        {
            lCategory = clQuery.value(0).toString();
            lType = clQuery.value(1).toString();
            lTaskId = clQuery.value(2).toString();
            lDatabaseId = clQuery.value(3).toString();
            lStatus = clQuery.value(4).toString();
            lCommand = clQuery.value(5).toString();
            lMutex = clQuery.value(6).toString();
            lKey = lCommand;
            // For Incremental Update, use the key db_update_name|target
            // Do not use the database_id into because of SPIDER or MASTER/SLAVE which can duplicate the actions todo
            //if(lType=="INCREMENTAL_UPDATE")
            //    lKey = lCommand+"|database_id="+lDatabaseId;
            //else
            if(lCommand.indexOf("FileName=") > 0)
                lKey = "FileName="+lCommand.section("FileName=",1).section("|",0,0);
            mActionsInfoList[lKey]["Category"] = lCategory;
            mActionsInfoList[lKey]["Type"] = lType;
            mActionsInfoList[lKey]["TaskId"] = lTaskId;
            mActionsInfoList[lKey]["DatabaseId"] = lDatabaseId;
            mActionsInfoList[lKey]["Status"] = lStatus;
            mActionsInfoList[lKey]["Command"] = lCommand;
            mActionsInfoList[lKey]["Mutex"] = lMutex;

            // Old action
            // Don't insert it into the mActionsList
        }
        return true;
    }
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Node[%1][%2] try to get the MASTER role but another node just had it...")
          .arg(m_nNodeId).arg(mMutex)
          .toLatin1().constData());

    return false;
}

bool    AdminEngine::ReleaseMasterRole()
{
    // The 2 first lines are for MASTER_YM and MASTER_PAT
    QString        lQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    QString strMasterRole = "MASTER_YM";;
    int lMasterActionId = 1;
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
    {
        strMasterRole = "MASTER_PAT";
        lMasterActionId = 2;
    }

    // Insert all the actions now
    if(!mActionsList.isEmpty())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Node[%1][%2] inserts %3 new actions")
              .arg(m_nNodeId)
              .arg(mMutex)
              .arg(mActionsList.count())
              .toLatin1().constData());

        if(!mAttributes.contains("max_allowed_packet"))
        {
            lQuery = "SELECT @@max_allowed_packet";
            if(!clQuery.exec(lQuery))
                SetLastError(clQuery);
            if(clQuery.first())
                mAttributes["max_allowed_packet"] = clQuery.value(0).toInt();
            else
                mAttributes["max_allowed_packet"] = 4194304; // default value
        }

        QString lKey;
        QString lHeadQuery,lNewInsert;
        lQuery = "";
        lHeadQuery = "INSERT INTO ym_actions(creation_time,";
        lHeadQuery+= "category,type,start_time,node_list,mutex,task_id,database_id,status,command)";
        lHeadQuery+= " VALUES";
        while(!mActionsList.isEmpty())
        {
            lKey = mActionsList.takeFirst();

            // Create the bulck insert
            // but not too big to be under the max_allowed_packet
            if(!lQuery.isEmpty())
                lQuery+= ",";
            lQuery += lNewInsert;

            lNewInsert= "(now(),";
            lNewInsert+= NormalizeStringToSql(mActionsInfoList[lKey]["Category"])+",";
            lNewInsert+= NormalizeStringToSql(mActionsInfoList[lKey]["Type"])+",";
            lNewInsert+= "null,";
            lNewInsert+= NormalizeStringToSql("|")+",";
            lNewInsert+= "null, ";
            lNewInsert+= mActionsInfoList[lKey]["TaskId"]+",";
            lNewInsert+= mActionsInfoList[lKey]["DatabaseId"]+",";
            lNewInsert+= NormalizeStringToSql("READY")+",";
            lNewInsert+= NormalizeStringToSqlLob(mActionsInfoList[lKey]["Command"])+")";

            if((lHeadQuery.length() + lQuery.length() + lNewInsert.length()) > mAttributes["max_allowed_packet"].toInt())
            {
                // Execute the query
                if(!clQuery.exec(lHeadQuery+lQuery))
                    SetLastError(clQuery);
                lQuery = "";
            }
        }
        if((lQuery.length() + lNewInsert.length()) > 0)
        {
            // Execute the query
            if(!lQuery.isEmpty())
                lQuery += ",";
            if(!clQuery.exec(lHeadQuery + lQuery + lNewInsert))
                SetLastError(clQuery);
        }
    }
    mActionsInfoList.clear();
    mActionsList.clear();

    GSLOG(SYSLOG_SEV_WARNING,
          QString("Node[%1][%2] releases the MASTER role")
          .arg(m_nNodeId)
          .arg(mMutex).toLatin1().constData());
    // Release the master role
    lQuery = "UPDATE ym_actions ";
    lQuery+= "SET mutex=null,";
    lQuery+= "status='READY',";
    lQuery+= "creation_time=now() ";
    lQuery+= "WHERE mutex='"+mMutex+"' ";
    lQuery+= "AND action_id="+QString::number(lMasterActionId);
    if(!clQuery.exec(lQuery))
        SetLastError(clQuery, true);
    clQuery.exec("COMMIT");

    return true;
}

}
}
