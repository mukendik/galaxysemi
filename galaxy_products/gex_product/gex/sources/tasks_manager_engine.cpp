#include <gqtl_log.h>

#include "tasks_manager_engine.h"
#include "task.h"

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////
TasksManagerEngine::TasksManagerEngine()
{
    setObjectName("GSTasksManagerEngine");
    mTaskCounted = 0;
    mIsCloseRequired = false;
}

///////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////
TasksManagerEngine::~TasksManagerEngine()
{
}

///////////////////////////////////////////////////////////////////
// Send a signal to stop all running task
////////////////////////////////////////////////////////////////////
void TasksManagerEngine::StopAllTask()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Stop all task...");
    emit sStopAllRunningTask();
}

///////////////////////////////////////////////////////////////////
// Add a task in the task list and set the task Id
////////////////////////////////////////////////////////////////////
int TasksManagerEngine::AddTask(Task *pTask)
{
    // increment Task Id
    mTaskCounted++;
    pTask->SetId(mTaskCounted);
    pTask->SetStartTime(QDateTime::currentDateTime());
    // Insert pTask in the task list
    mTaskList << pTask;

    // Refresh the dialog
    TaskListUpdated();

    // Return the new task Id (use when the task will be stop to remove it from the task list)
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("TaskManager addTask : %1 - %2")
          .arg( pTask->Id())
          .arg( pTask->Name()).toLatin1().constData());
    return pTask->Id();
}

///////////////////////////////////////////////////////////////////
// Add a task in the task list
////////////////////////////////////////////////////////////////////
int TasksManagerEngine::AddTask(int iType)
{
    Task *pTask = new Task();
    pTask->SetType((Task::taskType) iType);
    // Return the new task Id (use when the task will be stop to remove it from the task list)
    return AddTask(pTask);
}

///////////////////////////////////////////////////////////////////
// On click on button cancel
////////////////////////////////////////////////////////////////////
void TasksManagerEngine::OnCancelRequested()
{
    mIsCloseRequired = false;
    emit sRestartAllPendingTask();
}

///////////////////////////////////////////////////////////////////
// Show the gui and launch signal to stop all running tasks
////////////////////////////////////////////////////////////////////
bool TasksManagerEngine::OnStopAllTaskRequired()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " TaskManager onStopAllTaskRequired");
    // Call stop of all running tasks
    StopAllTask();

    // If no more running task
    if (mTaskList.count() == 0)
        return true;

    mIsCloseRequired = true;

    return false;
}

///////////////////////////////////////////////////////////////////
// Launch when a task has been stopped, remove the task link to the
// task id from the task list
////////////////////////////////////////////////////////////////////
void TasksManagerEngine::OnStoppedTask(int iId)
{
    bool bIsRemoved = false;
    QList<Task*>::iterator it = mTaskList.begin();
    // While the task has not be removed
    while (!bIsRemoved && (it != mTaskList.end()))
    {
        if ((*it)->Id() == iId)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("TaskManager onStoppedTask : %1 - %2").arg( iId).arg( (*it)->Name())
                  .toLatin1().constData());
            delete *it;
            mTaskList.erase(it);
            bIsRemoved = true;
        }
        else
            ++it;
    }
    // Refresh view of tasks after removal
    TaskListUpdated();
    // If no more task running -> emit signal
    if ((mTaskList.count() == 0) && mIsCloseRequired)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "TaskManager onStoppedTask : Close Application Required");
        emit sCloseApplication();
    }
}

///////////////////////////////////////////////////////////////////
// false If no more task running
///////////////////////////////////////////////////////////////////
int TasksManagerEngine::RunningTasksCount()
{
    return mTaskList.count();
}


///////////////////////////////////////////////////////////////////
// Refresh the task list
////////////////////////////////////////////////////////////////////
void TasksManagerEngine::TaskListUpdated()
{
    // Clear the view
    QList<Task*>::iterator it = mTaskList.begin();
    QStringList lTaskList;
    while (it != mTaskList.end())
    {
        // Add a task to the list view
        lTaskList.append(QString(QString::number((*it)->Id())).append(";").
                         append((*it)->Name()).append(";").
                         append((*it)->StartTime().toString("hh:mm:ss MM-dd-yyyy")));
        ++it;
    }
    emit sTaskListUpdated(lTaskList);
}


} // END Gex
} // END GS

