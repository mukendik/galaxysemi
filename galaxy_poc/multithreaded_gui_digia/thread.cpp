#include "thread.h"

void Thread::run()
{
    qDebug("Start of run thread...");

    QObject::connect(this, SIGNAL(sNewValue()), this, SLOT(createNewValue()), Qt::QueuedConnection);
    emit sNewValue();
    exec();
    qDebug("End of run thread...");
}

void  Thread::createNewValue()
{
//    while (mCounter.toLongLong()<9999999)
    {
        bool lOK=false;
        qlonglong lCV=mCounter.toLongLong(&lOK);
        if (!lOK)
            terminate();
//             break;
        //mCounter=QString::number(++lCV); // crash...
        SetCurrentvalue(QString::number(++lCV));
        emit sNewValue();
    //    QApplication::processEvents();
        // asking the thread to sleep a little bit prevent the GUI to freeze. But GalaxySemi is asking for another solution.
        //usleep(1);
    }
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
