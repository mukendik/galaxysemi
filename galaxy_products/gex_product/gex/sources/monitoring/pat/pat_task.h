#ifndef PAT_TASK_H
#define PAT_TASK_H

#include <QString>
#include "trigger_pat_task.h"

class CGexMoTaskPatPump : public CGexMoTaskTriggerPatPump
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskPatPump)

public:
    CGexMoTaskPatPump(QObject* parent=NULL);
protected:
    CGexMoTaskPatPump(CGexMoTaskPatPump* orig, QString copyName);
public:
    virtual ~CGexMoTaskPatPump();

public:
    // CGexMoTaskTriggerPatPump virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);

public slots:
    // CGexMoTaskTriggerPatPump virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
};

#endif // PAT_TASK_H
