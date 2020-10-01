#include "olddatapump_task.h"
#include "datapump_taskdata.h"
#include "gexmo_constants.h"

#include "engine.h"
#include "scheduler_engine.h"

CGexMoTaskOldDataPump::CGexMoTaskOldDataPump(QObject *parent)
    : CGexMoTaskDataPump(parent)
{
    mProperties = NULL;
}

CGexMoTaskOldDataPump::CGexMoTaskOldDataPump(const CGexMoTaskOldDataPump* orig, QString copyName)
    : CGexMoTaskDataPump(orig, copyName)
{
}

CGexMoTaskItem* CGexMoTaskOldDataPump::Duplicate(QString copyName)
{
    return new CGexMoTaskOldDataPump(this, copyName);
}

CGexMoTaskOldDataPump::~CGexMoTaskOldDataPump()
{
}

void CGexMoTaskOldDataPump::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;
    m_iStatus = MO_TASK_STATUS_ERROR;
    m_strLastInfoMsg =  "Old task format. You must convert this task to specify a new format...";
}

int CGexMoTaskOldDataPump::GetTaskType()
{
    return GEXMO_TASK_OLD_DATAPUMP;
}

QString CGexMoTaskOldDataPump::GetTypeName()
{
    return "OLD_DATAPUMP";
}

bool CGexMoTaskOldDataPump::IsExecutable()
{
    return false;
}
