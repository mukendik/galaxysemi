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

#include "admin_engine.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "mo_task.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_constants.h"
#include "gex_database_entry.h"
#include "engine.h"
#include <gqtl_log.h>
#include "libgexoc.h"
#include "product_info.h"
#include "scheduler_engine.h"

// in main.cpp
//extern void                 WriteDebugMessageFile(const QString & strMessage);
extern GexScriptEngine*     pGexScriptEngine;

// report_build.cpp
extern CReportOptions       ReportOptions;    // Holds options (report_build.h)

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn Log
 * \brief Log to admin log. Status = "WARNING", "INFO", or "UNEXPECTED"
 ******************************************************************************/
void AdminEngine::Log(QString Status, QString Summary, QString Comment)
{
    QString lStr;

    this->AddNewEvent("", "", Status, Summary, Comment);

    QDateTime lCurrentDateTime =
        GS::Gex::Engine::GetInstance().GetClientDateTime();
    lStr = lCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
    lStr += QString("%1: %2: %3\n").arg(Status).arg(Summary).arg(Comment);
    GS::Gex::Engine::GetInstance().
        GetSchedulerEngine().AppendMoHistoryLog(lStr);
}

bool    AdminEngine::AddNewEvent(QString Category,QString Type,
                                 QString Status, QString Summary, QString Comment)
{
    QDateTime Date = GetServerDateTime();
    return AddNewEvent(Category,Type,Date,Status,Summary,Comment);
}

bool    AdminEngine::AddNewEvent(QString Category,QString Type,QDateTime Date,
                                 QString Status, QString Summary, QString Comment)
{
    int         ParentTaskId = 0;
    QString     ParentCategory = Category;

    // retrieve the task of the parent event
    if(((Category == "EXECUTION") || Category.isEmpty())
            && (!mEventsExecutionStack.isEmpty()))
    {
        ParentCategory = "EXECUTION";
        ParentTaskId = SplitCommand(mEventsExecutionStack.top())["TaskId"].toInt();
    }

    return AddNewEvent(ParentCategory,Type,Date,0,ParentTaskId,0,"",Status,Summary,Comment);
}

bool    AdminEngine::AddNewEvent(QString Category,QString Type,
                                 int Size, int TaskId, int DatabaseId, QString Command,
                                 QString Status, QString Summary, QString Comment)
{
    QDateTime Date = GetServerDateTime();
    return AddNewEvent(Category,Type,Date,Size,TaskId,DatabaseId,Command,Status,Summary,Comment);

}

////////////////////////////////////////////////
bool    AdminEngine::AddNewEvent(QString Category,QString Type,QDateTime Date,
                                 int Size, int TaskId, int DatabaseId, QString Command,
                                 QString Status, QString Summary, QString Comment)
{
    if(this->IsActivated())
    {
        QString     ParentEvent;
        int         ParentEventId = 0;
        QString     ParentCategory = Category;
        QString     ParentType = Type;
        QDateTime   ParentDate = Date;
        int         ParentSize = Size;
        int         ParentDatabaseId = DatabaseId;
        QString     ParentCommand = Command;
        QString     ParentSummary = Summary;
        QString     ParentStatus = Status;
        QString     strQuery;
        QSqlQuery   clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
        bool        MonitoringMode = false;
        MonitoringMode = GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();

        // Do not trace INFO when not MonitoringMode
        if(!MonitoringMode && (Status=="INFO"))
            return true;

        // For category "EXECUTION"
        // Stack the Event when started
        // Pop at the end and trace in the ym_event
        if((Category == "EXECUTION") || Category.isEmpty())
        {
            if(!mEventsExecutionStack.isEmpty())
            {
                // retrieve the date of the parent event
                ParentEventId = SplitCommand(mEventsExecutionStack.top())["EventId"].toInt();
                // to update the duration
                if(SplitCommand(mEventsExecutionStack.top())["TaskId"].toInt() == TaskId)
                {
                    ParentDate = QDateTime::fromString(SplitCommand(mEventsExecutionStack.top())["Date"],Qt::ISODate);
                    ParentSize = SplitCommand(mEventsExecutionStack.top())["Size"].toInt();
                    ParentCommand = SplitCommand(mEventsExecutionStack.top())["Command"].replace("&#124;","|").replace("&#61;","=");
                    ParentCategory = SplitCommand(mEventsExecutionStack.top())["Category"];
                    ParentType = SplitCommand(mEventsExecutionStack.top())["Type"];
                    ParentDatabaseId = SplitCommand(mEventsExecutionStack.top())["DatabaseId"].toInt();
                }

                // Change INFO Status to WARNING (only for EXECUTION process)
                if((Status == "INFO") && (ParentSummary.contains("warning(s)",Qt::CaseInsensitive)
                                          || ParentSummary.contains("alarm(s)",Qt::CaseInsensitive)
                                          || ParentSummary.contains("Failed",Qt::CaseInsensitive)))
                    ParentStatus = "WARNING";

            }
            else if(Status == "UNEXPECTED")
            {
                // Try to retrieve the original event_id
                strQuery = "SELECT event_id, start_time, size, command, category, type, database_id FROM ym_events \n";
                // Instead to use start_time (not indexed), use creation_time
                // Reduce the execution by 2 and more
                strQuery+= " WHERE creation_time>="+NormalizeDateToSql(Date);
                strQuery+= " AND category='"+Category+"'";
                strQuery+= " AND type='"+Type+"' ";
                strQuery+= " AND task_id="+(TaskId<=0?"null":QString::number(TaskId));
                strQuery+= " AND database_id="+QString::number(DatabaseId);
                strQuery+= " AND link IS NULL";
                strQuery+= " AND command LIKE '%"+Command.replace("\\","_").replace("/","_")+"%'";
                strQuery+= " ORDER BY event_id";
                if(clQuery.exec(strQuery) && clQuery.first())
                {
                    ParentEventId = clQuery.value(0).toInt();
                    ParentDate = clQuery.value(1).toDateTime();
                    ParentSize = clQuery.value(2).toInt();
                    ParentCommand = clQuery.value(3).toString();
                    ParentCategory = clQuery.value(4).toString();
                    ParentType = clQuery.value(5).toString();
                    ParentDatabaseId = clQuery.value(6).toInt();
                }
                ParentSummary = "";
                strQuery = "SELECT name FROM ym_databases WHERE database_id="+QString::number(DatabaseId);
                if(clQuery.exec(strQuery) && clQuery.first())
                    ParentSummary += "databasename="+clQuery.value(0).toString()+"|";
                strQuery = "SELECT name FROM ym_tasks WHERE task_id="+QString::number(TaskId);
                if(clQuery.exec(strQuery) && clQuery.first())
                    ParentSummary += ParentType.toLower()+"name="+clQuery.value(0).toString()+"|";
                ParentSummary += Summary;
            }
            // Get the last size after uncompress
            if(Size > ParentSize)
                ParentSize = Size;
        }
        // For category "UPDATE"
        // Stack the Event weh started
        // Pop at the end and trace int the ym_event
        if(Category == "UPDATE")
        {
            if(!mEventsStack.isEmpty())
            {
                // retrieve the date of the parent event
                ParentEventId = SplitCommand(mEventsStack.top())["EventId"].toInt();
                // to update the duration
                if(SplitCommand(mEventsStack.top())["TaskId"].toInt() == TaskId)
                {
                    ParentDate = QDateTime::fromString(SplitCommand(mEventsStack.top())["Date"],Qt::ISODate);
                    ParentSize = SplitCommand(mEventsStack.top())["Size"].toInt();
                    ParentCommand = SplitCommand(mEventsStack.top())["Command"].replace("&#124;","|").replace("&#61;","=");
                    ParentCategory = SplitCommand(mEventsStack.top())["Category"];
                    ParentType = SplitCommand(mEventsStack.top())["Type"];
                    ParentDatabaseId = SplitCommand(mEventsStack.top())["DatabaseId"].toInt();
                }
            }
        }

        if(ParentCategory.isEmpty())
            ParentCategory = "APPLICATION";

        if(ParentType.isEmpty())
        {
            ParentType = "GEX";
            if(MonitoringMode)
            {
                if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
                    ParentType = "PAT";
                else
                    ParentType = "YM";
            }
        }

        strQuery = "INSERT INTO ym_events(creation_time,";
        strQuery+= "link,category,type,start_time,duration,size,node_id,";
        strQuery+= "user_id,task_id,database_id,command,status,summary,comment)";
        strQuery+= " VALUES(NOW(),";

        strQuery+= (ParentEventId>0?QString::number(ParentEventId):"null")+",";
        strQuery+= NormalizeStringToSql(ParentCategory)+",";
        strQuery+= NormalizeStringToSql(ParentType)+",";
        strQuery+= NormalizeDateToSql(ParentDate)+",";
        strQuery+= QString::number(ParentDate.secsTo(GetServerDateTime()))+",";
        strQuery+= QString::number(ParentSize)+",";
        strQuery+= (m_nNodeId<=0?"null":QString::number(m_nNodeId))+",";
        strQuery+= (IsUserConnected()?QString::number(m_pCurrentUser->m_nUserId):"null")+",";
        strQuery+= (TaskId<=0?"null":QString::number(TaskId))+",";
        strQuery+= (ParentDatabaseId<=0?"null":QString::number(ParentDatabaseId))+",";
        strQuery+= NormalizeStringToSql(ParentCommand)+",";
        strQuery+= NormalizeStringToSql(ParentStatus)+",";
        if(ParentSummary.isEmpty() && ((Status=="PASS"||(Status=="FAIL"))))
            strQuery+= NormalizeStringToSql((Status=="PASS"?"Pass":"Error"))+",";
        else
            strQuery+= NormalizeStringToSqlLob(ParentSummary)+",";
        strQuery+= NormalizeStringToSql(Comment)+")";
        if(!clQuery.exec(strQuery))
            SetLastError(clQuery, true);

        clQuery.exec("COMMIT"); // To unlock any tables

        // For category "EXECUTION"
        if((Category == "EXECUTION") || Category.isEmpty())
        {
            if(Status == "START")
            {
                // Retrieve the current AUTO_INCREMENT
                strQuery = "SELECT LAST_INSERT_ID()";
                if(!clQuery.exec(strQuery))
                {
                    SetLastError(clQuery);
                    return false;
                }
                clQuery.first();
                ParentEventId = clQuery.value(0).toLongLong();

                // Stack all needed info
                ParentEvent="TaskId="+QString::number(TaskId);
                ParentEvent+="|Category="+ParentCategory;
                ParentEvent+="|Type="+ParentType;
                ParentEvent+="|DatabaseId="+QString::number(ParentDatabaseId);
                ParentEvent+="|EventId="+QString::number(ParentEventId);
                ParentEvent+="|Date="+ParentDate.toString(Qt::ISODate);
                ParentEvent+="|Size="+QString::number(ParentSize);
                ParentEvent+="|Command="+ParentCommand.replace("|","&#124;").replace("=","&#61;");
                mEventsExecutionStack.push(ParentEvent);
            }
            else if(!mEventsExecutionStack.isEmpty() && ((Status == "PASS")||(Status == "FAIL")||(Status == "DELAY")))
                mEventsExecutionStack.pop();
        }

        // For category "UPDATE"
        if(Category == "UPDATE")
        {
            if(Status == "START")
            {
                // Retrieve the current AUTO_INCREMENT
                strQuery = "SELECT LAST_INSERT_ID()";
                if(!clQuery.exec(strQuery))
                {
                    SetLastError(clQuery);
                    return false;
                }
                clQuery.first();
                ParentEventId = clQuery.value(0).toLongLong();

                // Stack all needed info
                ParentEvent="TaskId="+QString::number(TaskId);
                ParentEvent+="|Category="+ParentCategory;
                ParentEvent+="|Type="+ParentType;
                ParentEvent+="|DatabaseId="+QString::number(ParentDatabaseId);
                ParentEvent+="|EventId="+QString::number(ParentEventId);
                ParentEvent+="|Date="+ParentDate.toString(Qt::ISODate);
                ParentEvent+="|Size="+QString::number(ParentSize);
                ParentEvent+="|Command="+ParentCommand.replace("|","&#124;").replace("=","&#61;");
                mEventsStack.push(ParentEvent);
            }
            else if(!mEventsStack.isEmpty() && ((Status == "PASS")||(Status == "FAIL")||(Status == "DELAY")))
                mEventsStack.pop();
        }

        return true;
    }
    return false;
}

}
}
