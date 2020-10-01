#ifndef REPORTING_TASK_H
#define REPORTING_TASK_H

#include <QString>
#include "mo_task.h"

class GexMoReportingTaskData;

class CGexMoTaskReporting : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskReporting)

public:
    CGexMoTaskReporting(QObject* parent=NULL);
protected:
    CGexMoTaskReporting(CGexMoTaskReporting* orig, QString copyName);
public:
    virtual ~CGexMoTaskReporting();

    // CGexMoTaskItem virtual methods overrides:

    QString Execute(QString DataFile);
    CGexMoTaskItem* Duplicate(QString copyName);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);
    QString GetPropertiesTitle();

public slots:
    void SetProperties(GexMoReportingTaskData* properties);
    GexMoReportingTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    bool IsExecutable();
    QString GetFrequencyLabel();

protected:
    GexMoReportingTaskData* mProperties;

    // overrides
    bool CheckExecutionWindow();
    bool CheckFrequency();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // REPORTING_TASK_H
