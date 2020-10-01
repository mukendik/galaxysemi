#include "reporting_task.h"
#include "reporting_taskdata.h"
#include "gexmo_constants.h"

#include "engine.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"

CGexMoTaskReporting::CGexMoTaskReporting(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskReporting::CGexMoTaskReporting(CGexMoTaskReporting *orig, QString copyName)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoReportingTaskData(this));
        *mProperties = *(orig->mProperties);
        mProperties->strTitle = copyName;
    }
}

CGexMoTaskItem* CGexMoTaskReporting::Duplicate(QString copyName)
{
    return new CGexMoTaskReporting(this, copyName);
}

CGexMoTaskReporting::~CGexMoTaskReporting()
{
}

int CGexMoTaskReporting::GetTaskType()
{
    return GEXMO_TASK_REPORTING;
}

QString CGexMoTaskReporting::GetTypeName()
{
    return "REPORTING";
}

void CGexMoTaskReporting::SetProperties(GexMoReportingTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoReportingTaskData* CGexMoTaskReporting::GetProperties()
{
    return mProperties;
}

bool CGexMoTaskReporting::IsExecutable()
{
    return true;
}

QString CGexMoTaskReporting::GetFrequencyLabel()
{
    QString FrequencyLabel;
    int Frequency=-1;
    int DayOfWeek=-1;

    if(mProperties)
    {
        Frequency = mProperties->iFrequency;
        DayOfWeek = mProperties->iDayOfWeek;
    }

    if(Frequency>=0)
    {
        FrequencyLabel = gexMoLabelTaskFrequency[Frequency];
        if(Frequency >= GEXMO_TASKFREQ_1WEEK)
        {
            // Frequency is over 1 week...then specify the day of the week!
            FrequencyLabel += " (";
            FrequencyLabel += gexMoLabelTaskFrequencyDayOfWeek[DayOfWeek];
            FrequencyLabel += ") ";
        }
    }

    return FrequencyLabel;
}

bool CGexMoTaskReporting::CheckExecutionWindow()
{
    if(mProperties)
    {
        // If a Time window is defined (AND task not Forced to be executed), check if we fall into it!
        if( (mProperties->bExecutionWindow) &&
            (m_tLastExecuted != 1))
        {
            QTime cTime=QTime::currentTime();
            if((cTime < mProperties->cStartTime) || (cTime > mProperties->cStopTime))
                return false;
        }
    }

    return true;
}

bool CGexMoTaskReporting::CheckFrequency()
{
    QDateTime cCurrentDateTime = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QDateTime cExecutionDateTime;

    // Check if Task has an execution windows and frequency
    // Check frequency only for local task (else load-balancing and priority)
    if(mProperties)
    {
        // Check frequency
        // If we are in the time window (or it's not activated), check frequency.
        cExecutionDateTime.setTime_t(m_tLastExecuted);

        // Compute the new date adding the frequency period
        if((AddDateFrequency(&cExecutionDateTime,mProperties->iFrequency,mProperties->iDayOfWeek) == false) && (m_tLastExecuted != 1))
            return false;  // Day of week mismatch...so not the right day to execute this task!...Ignored if Execution forced

        // Check if we have not reached the execution time yet...
        if(cCurrentDateTime < cExecutionDateTime)
            return false;
    }

    return true;
}

QString CGexMoTaskReporting::Execute(QString /*DataFile*/)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteReportingTask(this);
}

QString CGexMoTaskReporting::GetPropertiesTitle()
{
    if(mProperties)
    {
        return mProperties->strTitle;
    }
    return "";
}

void CGexMoTaskReporting::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionReporting(this);
}

bool CGexMoTaskReporting::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionReporting(this);
}

void CGexMoTaskReporting::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;
    m_iDatabaseId = 0;

    if(checkFolderOption)
    {
        QStringList folderList;
        folderList << mProperties->strScriptPath;
        this->CheckFolder(folderList);
    }
}

CGexMoTaskItem* CGexMoTaskReporting::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskReporting(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskReporting::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewReportingItem(this, nCurrentRow, allowEdit);
    }
}
