#include "pat_task.h"
#include "gexmo_constants.h"

CGexMoTaskPatPump::CGexMoTaskPatPump(QObject *parent)
    : CGexMoTaskTriggerPatPump(parent)
{
}

CGexMoTaskPatPump::CGexMoTaskPatPump(CGexMoTaskPatPump* orig, QString copyName)
    : CGexMoTaskTriggerPatPump(orig, copyName)
{
}

CGexMoTaskItem* CGexMoTaskPatPump::Duplicate(QString copyName)
{
    return new CGexMoTaskPatPump(this, copyName);
}

CGexMoTaskPatPump::~CGexMoTaskPatPump()
{
}

int CGexMoTaskPatPump::GetTaskType()
{
    return GEXMO_TASK_PATPUMP;
}

QString CGexMoTaskPatPump::GetTypeName()
{
    return "PATPUMP";
}

