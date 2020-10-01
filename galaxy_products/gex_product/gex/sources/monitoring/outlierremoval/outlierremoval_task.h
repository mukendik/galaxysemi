#ifndef OUTLIERREMOVAL_TASK_H
#define OUTLIERREMOVAL_TASK_H

#include <QString>
#include "mo_task.h"

class GexMoOutlierRemovalTaskData;

class CGexMoTaskOutlierRemoval : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskOutlierRemoval)

public:
    CGexMoTaskOutlierRemoval(QObject* parent=NULL);
protected:
    CGexMoTaskOutlierRemoval(CGexMoTaskOutlierRemoval* orig, QString copyName);
public:
    virtual ~CGexMoTaskOutlierRemoval();

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);

public slots:
    void SetProperties(GexMoOutlierRemovalTaskData* properties);
    GexMoOutlierRemovalTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    QString GetDatabaseName();
    bool SetDatabaseName(QString DatabaseName);

protected:
    GexMoOutlierRemovalTaskData* mProperties;

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // OUTLIERREMOVAL_TASK_H
