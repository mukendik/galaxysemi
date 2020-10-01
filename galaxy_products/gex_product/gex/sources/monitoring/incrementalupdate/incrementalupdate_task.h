#ifndef INCREMENTALUPDATE_TASK_H
#define INCREMENTALUPDATE_TASK_H

#include <QString>
#include "mo_task.h"

class CGexMoTaskIncrementalUpdate : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskIncrementalUpdate)

public:
    CGexMoTaskIncrementalUpdate(QObject* parent=NULL);
    virtual ~CGexMoTaskIncrementalUpdate();

public slots:
    int GetTaskType();
    QString GetTypeName();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption) {}
};

#endif // INCREMENTALUPDATE_TASK_H
