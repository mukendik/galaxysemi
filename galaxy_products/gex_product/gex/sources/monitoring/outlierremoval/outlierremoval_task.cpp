#include "outlierremoval_task.h"
#include "outlierremoval_taskdata.h"
#include "gexmo_constants.h"

#include "scheduler_engine.h"
#include "engine.h"
#include "mo_scheduler_gui.h"
#include "db_engine.h"
#include "gex_database_entry.h"

CGexMoTaskOutlierRemoval::CGexMoTaskOutlierRemoval(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskOutlierRemoval::CGexMoTaskOutlierRemoval(CGexMoTaskOutlierRemoval *orig, QString copyName)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoOutlierRemovalTaskData(this));
        *mProperties = *(orig->mProperties);
        mProperties->strTitle = copyName;
    }
}

CGexMoTaskItem* CGexMoTaskOutlierRemoval::Duplicate(QString copyName)
{
    return new CGexMoTaskOutlierRemoval(this, copyName);
}

CGexMoTaskOutlierRemoval::~CGexMoTaskOutlierRemoval()
{
}

int CGexMoTaskOutlierRemoval::GetTaskType()
{
    return GEXMO_TASK_OUTLIER_REMOVAL;
}

QString CGexMoTaskOutlierRemoval::GetTypeName()
{
    return "OUTLIER_REMOVAL";
}

void CGexMoTaskOutlierRemoval::SetProperties(GexMoOutlierRemovalTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoOutlierRemovalTaskData* CGexMoTaskOutlierRemoval::GetProperties()
{
    return mProperties;
}

QString CGexMoTaskOutlierRemoval::GetDatabaseName()
{
    QString DatabaseName = CGexMoTaskItem::GetDatabaseName();
    if(DatabaseName.isEmpty() && mProperties)
    {
        DatabaseName = mProperties->strDatabase;
    }
    return DatabaseName;
}

bool CGexMoTaskOutlierRemoval::SetDatabaseName(QString DatabaseName)
{
    CGexMoTaskItem::SetDatabaseName(DatabaseName);
    if(mProperties)
    {
        mProperties->strDatabase = DatabaseName;
    }
    return true;
}

void CGexMoTaskOutlierRemoval::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionOutlierRemoval(this);
}

bool CGexMoTaskOutlierRemoval::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionOutlierRemoval(this);
}

void CGexMoTaskOutlierRemoval::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;

    GexDatabaseEntry *pDatabaseEntry=NULL;
    QString strReferencedDatabase;
    if(this->m_iDatabaseId != 0)
    {
        // Check if database is referenced
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(this->m_iDatabaseId);
        if(pDatabaseEntry)
        {
            strReferencedDatabase = pDatabaseEntry->LogicalName();
        }
    }
    if(!strReferencedDatabase.isEmpty() && strReferencedDatabase != this->GetDatabaseName())
    {
        SetDatabaseName(strReferencedDatabase);
    }
}

CGexMoTaskItem* CGexMoTaskOutlierRemoval::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskOutlierRemoval(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskOutlierRemoval::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewOutlierItem(this, nCurrentRow, allowEdit);
    }
}
