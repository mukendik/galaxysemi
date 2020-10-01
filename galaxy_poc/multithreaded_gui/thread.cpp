#include "thread.h"

void Thread::run()
{
    qDebug("Start of run thread...");
    bool lOK=false;
    while (mCounter.toLongLong()<9999999)
    {
        qlonglong lCV=mCounter.toLongLong(&lOK);
        if (!lOK)
            break;
        //mCounter=QString::number(++lCV); // crash...
        SetCurrentvalue(QString::number(++lCV));
        emit sNewValue();
        QApplication::processEvents();
        // asking the thread to sleep a little bit prevent the GUI to freeze. But GalaxySemi is asking for another solution.
        //usleep(1);
    }
    qDebug("End of run thread...");
}

QString Thread::GetCurrentvalue() const
{
    QMutexLocker lML(&mMutex); // ?
    return mCounter;
}

void Thread::SetCurrentvalue(const QString& lNV)
{
    QMutexLocker lML(&mMutex); // not needed ?
    mCounter=lNV;
}
