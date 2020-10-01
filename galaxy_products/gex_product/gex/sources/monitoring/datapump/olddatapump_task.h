#ifndef OLDDATAPUMP_TASK_H
#define OLDDATAPUMP_TASK_H

#include <QString>
#include "datapump_task.h"

class GexMoDataPumpTaskData;

class CGexMoTaskOldDataPump : public CGexMoTaskDataPump
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskOldDataPump)

public:
    CGexMoTaskOldDataPump(QObject* parent=NULL);
protected:
    CGexMoTaskOldDataPump(const CGexMoTaskOldDataPump* orig, QString copyName);
public:
    CGexMoTaskItem* Duplicate(QString copyName);
    virtual ~CGexMoTaskOldDataPump();

public slots:
    // CGexMoTaskDataPump virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    bool IsExecutable();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // OLDDATAPUMP_TASK_H
