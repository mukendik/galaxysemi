#ifndef CONVERTER_TASK_H
#define CONVERTER_TASK_H

#include <QString>
#include "mo_task.h"

class GexMoFileConverterTaskData;

class CGexMoTaskConverter : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskConverter)

public:
    CGexMoTaskConverter(QObject* parent=NULL);
protected:
    CGexMoTaskConverter(CGexMoTaskConverter* orig, QString copyName);
public:
    virtual ~CGexMoTaskConverter();

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);
    QString Execute(QString DataFile);
    void SaveTaskToXML(QTextStream& XMLStream);
    bool LoadTaskDataFromDb();
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);
    QString GetPropertiesTitle();
    GexMoFileConverterTaskData* GetConverterData();

public slots:
    void SetProperties(GexMoFileConverterTaskData* properties);
    GexMoFileConverterTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();
    QString GetFrequencyLabel();
    QString GetDataFilePath();
    QString GetDataFileExtension();

protected:
    GexMoFileConverterTaskData* mProperties;

    // overrides
    bool IsExecutable();
    bool CheckFrequency();

private:
    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);
};

#endif // CONVERTER_TASK_H
