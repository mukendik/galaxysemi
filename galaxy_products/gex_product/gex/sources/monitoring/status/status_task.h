#ifndef STATUS_TASK_H
#define STATUS_TASK_H

#include <QString>
#include "mo_task.h"

class GexMoStatusTaskData;

class CGexMoTaskStatus : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskStatus)

public:
    CGexMoTaskStatus(QObject* parent=NULL);
    virtual ~CGexMoTaskStatus();

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName) {return NULL;}
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);

public slots:
    void SetProperties(GexMoStatusTaskData* properties);
    GexMoStatusTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();

protected:
    GexMoStatusTaskData* mProperties;

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // STATUS_TASK_H
