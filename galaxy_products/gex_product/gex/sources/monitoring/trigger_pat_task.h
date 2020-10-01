#ifndef TRIGGER_PAT_TASK_H
#define TRIGGER_PAT_TASK_H

#include <QString>
#include "datapump/datapump_task.h"

class CGexMoTaskTriggerPatPump : public CGexMoTaskDataPump
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskTriggerPatPump)

public:
    CGexMoTaskTriggerPatPump(QObject* parent=NULL);
protected:
    CGexMoTaskTriggerPatPump(CGexMoTaskTriggerPatPump* orig, QString copyName);
public:
    CGexMoTaskItem* Duplicate(QString copyName) = 0;
    virtual ~CGexMoTaskTriggerPatPump();

    // CGexMoTaskItem virtual methods overrides:

    QString Execute(QString DataFile);

public slots:
    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType() = 0;

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // TRIGGER_PAT_TASK_H
