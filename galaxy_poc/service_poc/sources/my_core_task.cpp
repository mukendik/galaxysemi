#include "my_core_task.h"
#include "my_event.h"
#include <gqtl_log.h>

#define TIMER_TICK    500

MyCoreTask::MyCoreTask(QObject *parent)
    : QObject(parent), mRunning(false), mTimerID(0), mTaskDuration(0), mTimerTick(0)
{
}

MyCoreTask::~MyCoreTask()
{

}

bool MyCoreTask::IsRunning() const
{
    return mRunning;
}

bool MyCoreTask::ExecuteSynchronousTask(int lDuration)
{
    if (IsRunning() == false)
    {
        mRunning        = true;
        mTaskDuration   = lDuration;
        mTimerTick      = 0;

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Synchronous Task started with duration of %1 ms.")
              .arg(mTaskDuration).toLatin1().constData());

        QElapsedTimer   lElapsedTimer;

        lElapsedTimer.start();

        while (lElapsedTimer.hasExpired(mTaskDuration) == false)
        {
            if ((lElapsedTimer.elapsed() - mTimerTick) > TIMER_TICK)
            {
                mTimerTick += TIMER_TICK;

                GSLOG(SYSLOG_SEV_NOTICE,
                      QString("Timer tick reached (%1 ms)").arg(mTimerTick).toLatin1().constData());
            }
        }

        mRunning    = false;
        mTimerTick  = 0;

        GSLOG(SYSLOG_SEV_NOTICE, "Synchronous task finished");

        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "A Task is already running.");
        return false;
    }
}

bool MyCoreTask::ExecuteAsynchronousTask(int lDuration)
{
    if (IsRunning() == false)
    {
        mRunning = true;

        mRunning        = true;
        mTaskDuration   = lDuration;
        mTimerTick      = 0;

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Asynchronous Task started with duration of %1 ms.")
              .arg(mTaskDuration).toLatin1().constData());

        // Start a timer event that will occuer every TIMER_TICK ms
        mTimerID = startTimer(TIMER_TICK);

        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "A Task is already running.");
        return false;
    }
}

void MyCoreTask::customEvent(QEvent *e)
{
    GSLOG(SYSLOG_SEV_NOTICE, "Processing custom event.");

    if(e->type() == QEvent::Type(MyEvent::UserSyncTask))
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Processing custom event for Synchronous Task.");

        MyEvent * lMyEvent = dynamic_cast<MyEvent *> (e);

        if (lMyEvent)
            ExecuteSynchronousTask(lMyEvent->GetDuration());
    }
    else if(e->type() == QEvent::Type(MyEvent::UserAsyncTask))
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Processing custom event for Asynchronous Task.");

        MyEvent * lMyEvent = dynamic_cast<MyEvent *> (e);

        if (lMyEvent)
            ExecuteAsynchronousTask(lMyEvent->GetDuration());
    }
}

void MyCoreTask::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mTimerID)
    {
        mTimerTick += TIMER_TICK;

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Timer tick reached (%1 ms)").arg(mTimerTick).toLatin1().constData());

        if (mTimerTick >= mTaskDuration)
        {
            killTimer(mTimerID);

            mTimerID    = 0;
            mTimerTick  = 0;
            mRunning    = false;

            GSLOG(SYSLOG_SEV_NOTICE, "Asynchronous task finished");
        }
    }
}

