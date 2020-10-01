#include "converter_task.h"
#include "converter_taskdata.h"
#include "gexmo_constants.h"

#include "scheduler_engine.h"
#include "engine.h"
#include "mo_scheduler_gui.h"

CGexMoTaskConverter::CGexMoTaskConverter(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskConverter::CGexMoTaskConverter(CGexMoTaskConverter *orig, QString copyName)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoFileConverterTaskData(this));
        *mProperties = *(orig->mProperties);
        mProperties->strTitle = copyName;
    }
}

CGexMoTaskItem* CGexMoTaskConverter::Duplicate(QString copyName)
{
    return new CGexMoTaskConverter(this, copyName);
}

CGexMoTaskConverter::~CGexMoTaskConverter()
{
}

int CGexMoTaskConverter::GetTaskType()
{
    return GEXMO_TASK_CONVERTER;
}

QString CGexMoTaskConverter::GetTypeName()
{
    return "CONVERTER";
}

void CGexMoTaskConverter::SetProperties(GexMoFileConverterTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoFileConverterTaskData* CGexMoTaskConverter::GetProperties()
{
    return mProperties;
}

bool CGexMoTaskConverter::IsExecutable()
{
    return true;
}

QString CGexMoTaskConverter::GetFrequencyLabel()
{
    QString FrequencyLabel;
    int Frequency=-1;
    int DayOfWeek=-1;

    if(IsLocal() && mProperties)
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

QString CGexMoTaskConverter::GetDataFilePath()
{
    QString Path;

    if(mProperties)
    {
        Path = mProperties->strInputFolder;
    }

    return Path;
}

QString CGexMoTaskConverter::GetDataFileExtension()
{
    QString Ext;

    if(mProperties)
    {
        Ext = mProperties->strFileExtensions;
    }

    return Ext;
}

bool CGexMoTaskConverter::CheckFrequency()
{
    QDateTime cCurrentDateTime = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QDateTime cExecutionDateTime;

    // Check if Task has an execution windows and frequency
    // Check frequency only for local task (else load-balancing and priority)
    if(mProperties && IsLocal())
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

QString CGexMoTaskConverter::Execute(QString DataFile)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteFileConverterTask(this, DataFile);
}

QString CGexMoTaskConverter::GetPropertiesTitle()
{
    if(mProperties)
    {
        return mProperties->strTitle;
    }
    return "";
}

GexMoFileConverterTaskData* CGexMoTaskConverter::GetConverterData()
{
    return mProperties;
}

void CGexMoTaskConverter::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionFileConverter(this);
}

bool CGexMoTaskConverter::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionFileConverter(this);
}

void CGexMoTaskConverter::CheckTaskStatusInternal(bool /*checkDatabaseOption*/, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;
    m_iDatabaseId = 0;

    if(checkFolderOption)
    {
        QStringList folderList;
        folderList << mProperties->strInputFolder;
        if(mProperties->iOnSuccess == 2)
        {
            folderList << mProperties->strOutputSuccess;
        }
        folderList << mProperties->strOutputFolder;
        if(mProperties->iOnError == 2)
        {
            folderList << mProperties->strOutputError;
        }
        this->CheckFolder(folderList);
    }
}

CGexMoTaskItem* CGexMoTaskConverter::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskFileConverter(this, readOnly);
    }
    return NULL;
}

void CGexMoTaskConverter::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewConverterItem(this, nCurrentRow, allowEdit);
    }
}
