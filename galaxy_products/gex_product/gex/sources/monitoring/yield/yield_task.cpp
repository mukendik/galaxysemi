#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"

#include "gexmo_constants.h"

#include "engine.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"

CGexMoTaskYieldMonitor::CGexMoTaskYieldMonitor(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskYieldMonitor::CGexMoTaskYieldMonitor(CGexMoTaskYieldMonitor *orig, QString copyName)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoYieldMonitoringTaskData(this));
        *mProperties = *(orig->mProperties);
        mProperties->strTitle = copyName;
    }
}

CGexMoTaskItem* CGexMoTaskYieldMonitor::Duplicate(QString copyName)
{
    return new CGexMoTaskYieldMonitor(this, copyName);
}

CGexMoTaskYieldMonitor::~CGexMoTaskYieldMonitor()
{
}

int CGexMoTaskYieldMonitor::GetTaskType()
{
    return GEXMO_TASK_YIELDMONITOR;
}

QString CGexMoTaskYieldMonitor::GetTypeName()
{
    return "YIELDMONITOR";
}

void CGexMoTaskYieldMonitor::SetProperties(GexMoYieldMonitoringTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoYieldMonitoringTaskData* CGexMoTaskYieldMonitor::GetProperties()
{
    return mProperties;
}

QString CGexMoTaskYieldMonitor::GetDatabaseName()
{
    QString DatabaseName = CGexMoTaskItem::GetDatabaseName();
    if(DatabaseName.isEmpty() && mProperties)
    {
        DatabaseName = mProperties->strDatabase;
    }
    return DatabaseName;
}

bool CGexMoTaskYieldMonitor::SetDatabaseName(QString DatabaseName)
{
    CGexMoTaskItem::SetDatabaseName(DatabaseName);
    if(mProperties)
    {
        mProperties->strDatabase = DatabaseName;
    }
    return true;
}

void CGexMoTaskYieldMonitor::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionYieldMonitoring(this);
}

bool CGexMoTaskYieldMonitor::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionYieldMonitoring_OneRule(this);
}

void CGexMoTaskYieldMonitor::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;

    this->CheckDatabase(checkDatabaseOption);
    if(this->m_iStatus != MO_TASK_STATUS_NOERROR)
    {
        return;
    }

    QString checkType=GetAttribute("CheckType").toString();
    if (checkType!="FixedYieldTreshold" && checkType!="BinsPerSite")
    {
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg =    QString("Alarm check type '%s' not supported. Select a new option...").arg(checkType);
    }
}

CGexMoTaskItem* CGexMoTaskYieldMonitor::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskYieldMonitoring(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskYieldMonitor::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewYieldItem(this, nCurrentRow, allowEdit);
    }
}
