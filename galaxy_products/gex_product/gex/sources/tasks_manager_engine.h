#ifndef TASKS_MANAGER_ENGINE_H
#define TASKS_MANAGER_ENGINE_H

#include <QObject>
#include <QStringList>

namespace GS
{
namespace Gex
{

class Task;

class TasksManagerEngine: public QObject
{
    Q_OBJECT
public :
    TasksManagerEngine();
    virtual ~TasksManagerEngine();
    /// \brief Add a task in the task list and set the task Id / return task id
    int		AddTask(int iType);
    /// \brief Add a task in the task list / return task id
    int		AddTask(Task *pTask);
    /// \brief Send a signal to stop all running task
    Q_INVOKABLE void StopAllTask();
    /// \brief Show the gui and launch signal to stop all running tasks
    bool	OnStopAllTaskRequired();
    /// \return number of running tasks
    Q_INVOKABLE int	RunningTasksCount();

signals :
    /// \brief signal to stop all running task
    void	sStopAllRunningTask();
    /// \brief signal to restart all pending task
    void	sRestartAllPendingTask();
    /// \brief signal to confirm that all task are stopped
    void	sCloseApplication();
    /// \brief signal that task list has been updated
    void    sTaskListUpdated(QStringList taskList);

public slots :
    /// \brief Launch when a task has been stopped, remove the task link to the task id from the task list
    void	OnStoppedTask(int iId);
    /// \brief On click on button cancel
    void	OnCancelRequested();

private :
    Q_DISABLE_COPY(TasksManagerEngine);
    /// \brief build qstringlist of tasks and emit a signal with the list
    void            TaskListUpdated();

    int				mTaskCounted;       ///< Counted task since the object creation
    bool			mIsCloseRequired;   ///< True if closed has been required
    QList<Task*>	mTaskList;          ///< list of running tasks
};

} // END Gex
} // END GS

#endif // TASKS_MANAGER_ENGINE_H
