#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QString>
#include <QApplication>
#include <QWeakPointer>
#include <QMutex>

class Thread: public QThread
{
    Q_OBJECT

    public:
        Thread():QThread(), mCounter("0") //, mMutex() //, mSP(sp)
        {
            qDebug("New thread...");
        }
        ~Thread() { qDebug("Thread destructor..."); }
        void run();
        QString GetCurrentvalue() const;
        void SetCurrentvalue(const QString&);
        QWeakPointer<Thread> mWP;

    private:
        QString mCounter;
        mutable QMutex mMutex;

    signals:
        void sNewValue();
};

#endif // THREAD_H
