#ifndef MY_CORE_TASK_H
#define MY_CORE_TASK_H

#include <QObject>
#include <QElapsedTimer>

class MyCoreTask
        : public QObject
{
    Q_OBJECT

public:

    MyCoreTask(QObject * parent = NULL);
    ~MyCoreTask();

    bool    IsRunning() const;
    bool    ExecuteSynchronousTask(int lDuration);
    bool    ExecuteAsynchronousTask(int lDuration);

    void    customEvent(QEvent * e);
    void    timerEvent(QTimerEvent * event);

private:

    bool            mRunning;
    int             mTimerID;
    qint64          mTaskDuration;
    qint64          mTimerTick;
};

#endif // MY_CORE_TASK_H
