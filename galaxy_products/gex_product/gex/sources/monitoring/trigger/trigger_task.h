#ifndef TRIGGER_TASK_H
#define TRIGGER_TASK_H

#include <QString>
#include "trigger_pat_task.h"

class CGexMoTaskTriggerPump : public CGexMoTaskTriggerPatPump
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskTriggerPump)

public:
    CGexMoTaskTriggerPump(QObject* parent=NULL);
protected:
    CGexMoTaskTriggerPump(CGexMoTaskTriggerPump* orig, QString copyName);
public:
    virtual ~CGexMoTaskTriggerPump();

    // CGexMoTaskTriggerPatPump virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);

public slots:
    // CGexMoTaskTriggerPatPump virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
};

#endif // TRIGGER_TASK_H
