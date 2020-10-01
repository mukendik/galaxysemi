#include "status_task.h"
#include "status_taskdata.h"
#include "gexmo_constants.h"

#include "scheduler_engine.h"
#include "engine.h"
#include "mo_scheduler_gui.h"
#include "report_options.h"

extern CReportOptions ReportOptions;

CGexMoTaskStatus::CGexMoTaskStatus(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskStatus::~CGexMoTaskStatus()
{
}

int CGexMoTaskStatus::GetTaskType()
{
    return GEXMO_TASK_STATUS;
}

QString CGexMoTaskStatus::GetTypeName()
{
    return "STATUS";
}

void CGexMoTaskStatus::SetProperties(GexMoStatusTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoStatusTaskData* CGexMoTaskStatus::GetProperties()
{
    return mProperties;
}

void CGexMoTaskStatus::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionStatus(this);
}

bool CGexMoTaskStatus::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionStatus(this);
}

void CGexMoTaskStatus::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->title();
    m_iDatabaseId = 0;

    if(checkFolderOption)
    {
        QStringList folderList;
        folderList << mProperties->intranetPath();
        QString strValue = mProperties->reportURL();
        if(strValue.isEmpty() || strValue.toLower() == "default" || strValue.toLower() == "(default)")
        {
            // No Web task defined or no custom report folder defined : use default report folder
            strValue = ReportOptions.GetServerDatabaseFolder(true);
        }
        folderList << strValue;
        this->CheckFolder(folderList);
    }
}

CGexMoTaskItem* CGexMoTaskStatus::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskStatus(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskStatus::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewStatusItem(this, nCurrentRow, allowEdit);
    }
}
