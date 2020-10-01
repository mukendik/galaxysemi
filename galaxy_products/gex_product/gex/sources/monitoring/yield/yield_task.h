#ifndef YIELD_TASK_H
#define YIELD_TASK_H

#include "mo_task.h"

class GexMoYieldMonitoringTaskData;

class CGexMoTaskYieldMonitor : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskYieldMonitor)

public:
    CGexMoTaskYieldMonitor(QObject* parent=NULL);
protected:
    CGexMoTaskYieldMonitor(CGexMoTaskYieldMonitor* orig, QString copyName);
public:
    virtual ~CGexMoTaskYieldMonitor();

    // CGexMoTaskItem virtual methods overrides:

    virtual CGexMoTaskItem* Duplicate(QString copyName);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    virtual CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    virtual void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);

public slots:
    void SetProperties(GexMoYieldMonitoringTaskData* properties);
    GexMoYieldMonitoringTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    QString GetDatabaseName();
    bool SetDatabaseName(QString DatabaseName);

protected:
    GexMoYieldMonitoringTaskData* mProperties;

private:
    virtual void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // YIELD_TASK_H
