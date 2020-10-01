#include "trigger_task.h"
#include "gexmo_constants.h"

CGexMoTaskTriggerPump::CGexMoTaskTriggerPump(QObject *parent)
    : CGexMoTaskTriggerPatPump(parent)
{
}

CGexMoTaskTriggerPump::CGexMoTaskTriggerPump(CGexMoTaskTriggerPump *orig, QString copyName)
    : CGexMoTaskTriggerPatPump(orig, copyName)
{
}

CGexMoTaskItem* CGexMoTaskTriggerPump::Duplicate(QString copyName)
{
    return new CGexMoTaskTriggerPump(this, copyName);
}

CGexMoTaskTriggerPump::~CGexMoTaskTriggerPump()
{
}

int CGexMoTaskTriggerPump::GetTaskType()
{
    return GEXMO_TASK_TRIGGERPUMP;
}

QString CGexMoTaskTriggerPump::GetTypeName()
{
    return "TRIGGERPUMP";
}

