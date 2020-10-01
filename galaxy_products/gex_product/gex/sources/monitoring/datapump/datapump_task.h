#ifndef MO_TASK_DATAPUMP_H
#define MO_TASK_DATAPUMP_H

#include <QString>
#include "mo_task.h"

class GexMoDataPumpTaskData;

class CGexMoTaskDataPump : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskDataPump)

public:
    CGexMoTaskDataPump(QObject* parent=NULL);
protected:
    CGexMoTaskDataPump(const CGexMoTaskDataPump* orig, QString copyName);
public:
    virtual ~CGexMoTaskDataPump();

    // CGexMoTaskItem virtual methods overrides:

    virtual CGexMoTaskItem* Duplicate(QString copyName);
    QDir::SortFlags GetDataFileSort();
    QString Execute(QString DataFile);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);
    GexMoDataPumpTaskData* GetDataPumpData();

public slots:
    void SetProperties(GexMoDataPumpTaskData* properties);
    GexMoDataPumpTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    bool IsExecutable();
    QString GetDatabaseName();
    bool SetDatabaseName(QString DatabaseName);
    QString GetFrequencyLabel();
    QString GetDataFilePath();
    QString GetDataFileExtension();
    bool IsDataFileScanSubFolder();

protected:
    GexMoDataPumpTaskData* mProperties;

    //overrides
    bool CheckExecutionWindow();
    bool CheckFrequency();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // MO_TASK_DATAPUMP_H
