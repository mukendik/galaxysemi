#ifndef TESTABLE_G_TRIGGER_ENGINE_H
#define TESTABLE_G_TRIGGER_ENGINE_H

#include "g-trigger_engine.h"

#include <QString>

class TestableGTriggerEngine : public GS::GTrigger::GTriggerEngine
{
    Q_OBJECT

public :
    TestableGTriggerEngine( const QString &aAppName, const QString &aAppPath ) :
        GS::GTrigger::GTriggerEngine( aAppName, aAppPath ) {}

signals :
    void TestFinished();

public slots :
    void OnTimerEvent()
    {
        GS::GTrigger::GTriggerEngine::OnTimerEvent();

        emit TestFinished();
    }
};

#endif // TESTABLE_G_TRIGGER_ENGINE_H
