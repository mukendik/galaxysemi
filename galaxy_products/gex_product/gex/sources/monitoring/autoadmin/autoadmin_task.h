#ifndef AUTOADMIN_TASK_H
#define AUTOADMIN_TASK_H

#include <QString>
#include "mo_task.h"

class GexMoAutoAdminTaskData;

class CGexMoTaskAutoAdmin : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskAutoAdmin)

public:
    CGexMoTaskAutoAdmin(QObject* parent=NULL);
    virtual ~CGexMoTaskAutoAdmin();

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName) {return NULL;}
    QString Execute(QString DataFile);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);
    QString GetPropertiesTitle();

public slots:
    void SetProperties(GexMoAutoAdminTaskData* properties);
    GexMoAutoAdminTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    bool IsExecutable();

protected:
    GexMoAutoAdminTaskData* mProperties;

    // overrides
    bool CheckExecutionWindow();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // AUTOADMIN_TASK_H
