#include "task.h"

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////
Task::Task()
{
    mId = -1;
}

///////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////
Task::~Task()
{

}

///////////////////////////////////////////////////////////////////
// Return task name, link to the task type
////////////////////////////////////////////////////////////////////
QString Task::Name()
{
    QString strName;
    switch (mType)
    {
    case DataPump :
        strName = "DataPump";
        break;
    case DataPumpInsertion :
        strName = "DataPump";
        break;
    case DataPumpTrigger :
        strName = "TriggerPump";
        break;
    case DataPumpPat :
        strName = "PatPump";
        break;
    case YieldMonitor :
        strName = "YieldMonitor";
        break;
    case Reporting :
        strName = "Reporting";
        break;
    case Status :
        strName = "Status";
        break;
    case Converter :
        strName = "Converter";
        break;
    case OutlierRemoval :
        strName = "OutlierRemoval";
        break;
    case AutoAdmin :
        strName = "AutoAdmin";
        break;
    case Spm :
        strName = "Spm";
        break;
    case Sya :
        strName = "Sya";
        break;
    default: break;
    }
    return strName;
}

///////////////////////////////////////////////////////////////////
// Set the Task type
////////////////////////////////////////////////////////////////////
void Task::SetType(taskType eTaskType)
{
    mType = eTaskType;
}

///////////////////////////////////////////////////////////////////
// Set the task Id
////////////////////////////////////////////////////////////////////
void Task::SetId(int iId)
{
    mId = iId;
}

///////////////////////////////////////////////////////////////////
// Set the task start time
////////////////////////////////////////////////////////////////////
void Task::SetStartTime(QDateTime startTime)
{
    mStartTime = startTime;
}


} // END Gex
} // END GS

