#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include "gexmo_constants.h"

namespace GS
{
namespace Gex
{

class Task
{
public:
    enum taskType
    {
        DataPump            = GEXMO_TASK_OLD_DATAPUMP,
        DataPumpInsertion   = GEXMO_TASK_DATAPUMP,
        DataPumpTrigger     = GEXMO_TASK_TRIGGERPUMP,
        DataPumpPat         = GEXMO_TASK_PATPUMP,
        YieldMonitor        = GEXMO_TASK_YIELDMONITOR,
        Spm                 = GEXMO_TASK_SPM,
        Sya                 = GEXMO_TASK_SYA,
        Reporting           = GEXMO_TASK_REPORTING,
        Status              = GEXMO_TASK_STATUS,
        Converter           = GEXMO_TASK_CONVERTER,
        OutlierRemoval      = GEXMO_TASK_OUTLIER_REMOVAL,
        AutoAdmin           = GEXMO_TASK_AUTOADMIN
    };

    enum status
    {
        Started,
        Stopped,
        Paused
    };

    Task();
    virtual ~Task();
    /// \brief Set the task Id
    void		SetId(int iId);
    /// \brief Set the Task type
    void		SetType(taskType eTaskType);
    /// \brief Set the task name
    void		SetName(QString strName);
    /// \brief Set the task start time
    void		SetStartTime(QDateTime StartTime);
    /// \return task id
    int			Id()        {return mId;}
    /// \return task type
    taskType	Type()      {return mType;}
    /// \return task name, linked to the task type
    QString		Name();
    /// \return task start time
    QDateTime	StartTime() {return mStartTime;}

private:
    Q_DISABLE_COPY(Task);
    int			mId;            ///< Task id
    taskType	mType;          ///< Task type
    QString		mName;          ///< Task name
    QDateTime	mStartTime;     ///< Task start date
};

} // END Gex
} // END GS

#endif // TASK_H
