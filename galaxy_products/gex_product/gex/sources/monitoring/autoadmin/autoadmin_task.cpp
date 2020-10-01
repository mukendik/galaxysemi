#include "autoadmin_task.h"
#include "autoadmin_taskdata.h"
#include "gexmo_constants.h"

#include "scheduler_engine.h"
#include "engine.h"
#include "mo_scheduler_gui.h"

CGexMoTaskAutoAdmin::CGexMoTaskAutoAdmin(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskAutoAdmin::~CGexMoTaskAutoAdmin()
{
}

int CGexMoTaskAutoAdmin::GetTaskType()
{
    return GEXMO_TASK_AUTOADMIN;
}

QString CGexMoTaskAutoAdmin::GetTypeName()
{
    return "AUTOADMIN";
}

void CGexMoTaskAutoAdmin::SetProperties(GexMoAutoAdminTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoAutoAdminTaskData* CGexMoTaskAutoAdmin::GetProperties()
{
    return mProperties;
}

bool CGexMoTaskAutoAdmin::IsExecutable()
{
    return true;
}

bool CGexMoTaskAutoAdmin::CheckExecutionWindow()
{
    QDateTime cCurrentDateTime = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QDateTime cExecutionDateTime;

    // If task not Forced to be executed, check if we have past the starting time!
    if(m_tLastExecuted != 1)
    {
        QTime cTime=QTime::currentTime();
        if(cTime < mProperties->mStartTime)
            return false;  // NOT yet reached the starting time, then quiet exit!

        // If task already executed once today (same day of year), then ignore.
        cExecutionDateTime.setTime_t(m_tLastExecuted);
        QDate cCurrentDate = cCurrentDateTime.date();
        QDate cLastExecutionDate = cExecutionDateTime.date();
        if(cCurrentDate.dayOfYear()  == cLastExecutionDate.dayOfYear())
            return false;
    }

    return true;
}

QString CGexMoTaskAutoAdmin::Execute(QString /*DataFile*/)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteAutoAdminTask(this);
}

QString CGexMoTaskAutoAdmin::GetPropertiesTitle()
{
    if(mProperties)
    {
        return mProperties->mTitle;
    }
    return "";
}

void CGexMoTaskAutoAdmin::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionAutoAdmin(this);
}

bool CGexMoTaskAutoAdmin::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionAutoAdmin(this);
}

void CGexMoTaskAutoAdmin::CheckTaskStatusInternal(bool /*checkDatabaseOption*/, bool checkFolderOption)
{
    m_strName = mProperties->mTitle;
    m_iDatabaseId = 0;

    if(checkFolderOption)
    {
        QStringList lFolderList;
        lFolderList << mProperties->GetAttribute(C_ShellPatAlarmCritical).toString();
        lFolderList << mProperties->GetAttribute(C_ShellPatAlarm).toString();
        lFolderList << mProperties->GetAttribute(C_ShellYieldAlarmCritical).toString();
        lFolderList << mProperties->GetAttribute(C_ShellYieldAlarm).toString();
        lFolderList << mProperties->GetAttribute(C_ShellSpmAlarmCritical).toString();
        lFolderList << mProperties->GetAttribute(C_ShellSpmAlarm).toString();
        lFolderList << mProperties->GetAttribute(C_ShellSyaAlarmCritical).toString();
        lFolderList << mProperties->GetAttribute(C_ShellSyaAlarm).toString();

        this->CheckFolder(lFolderList);
    }
}

CGexMoTaskItem* CGexMoTaskAutoAdmin::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskAutoAdmin(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskAutoAdmin::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewAutoAdminItem(this, nCurrentRow, allowEdit);
    }
}
