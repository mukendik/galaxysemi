#include "incrementalupdate_task.h"
#include "gexmo_constants.h"

#include "engine.h"

CGexMoTaskIncrementalUpdate::CGexMoTaskIncrementalUpdate(QObject *parent)
    : CGexMoTaskItem(parent)
{
}

CGexMoTaskIncrementalUpdate::~CGexMoTaskIncrementalUpdate()
{
}

int CGexMoTaskIncrementalUpdate::GetTaskType()
{
    return GEXMO_TASK_INCREMENTAL_UPDATE;
}

QString CGexMoTaskIncrementalUpdate::GetTypeName()
{
    return "INCREMENTAL_UPDATE";
}

