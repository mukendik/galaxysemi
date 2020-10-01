#ifndef TESTABLE_G_TRIGGER_ENGINE_WORKER_H
#define TESTABLE_G_TRIGGER_ENGINE_WORKER_H

#include "qx_worker.h"

class TestableGTriggerEngineWorker : public Qx::QxWorker
{
public :
    TestableGTriggerEngineWorker( const QString &aAppName, const QString &aAppDirPath ) :
        mAppName{ aAppName }, mAppDirPath{ aAppDirPath } {}

private :
    void Process()
    {
        mEngine.reset( new TestableGTriggerEngine{ mAppName, mAppDirPath } );

        connect( mEngine.data(), &TestableGTriggerEngine::TestFinished, [ & ]()
        {
            mEngine.reset( nullptr );
            emit Done();
        } );

        mEngine->OnStart();
    }

private :
    QString mAppName;
    QString mAppDirPath;
    QScopedPointer< TestableGTriggerEngine > mEngine;
};

#endif // TESTABLE_G_TRIGGER_ENGINE_WORKER_H
