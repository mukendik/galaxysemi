#include "datapump_task.h"
#include "datapump_taskdata.h"
#include "gexmo_constants.h"

#include "engine.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "admin_engine.h"
#include "gex_database_entry.h"
#include "db_engine.h"

CGexMoTaskDataPump::CGexMoTaskDataPump(QObject *parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
}

CGexMoTaskDataPump::CGexMoTaskDataPump(const CGexMoTaskDataPump* orig, QString copyName)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoDataPumpTaskData(this));
        *mProperties = *(orig->mProperties);
        mProperties->strTitle = copyName;
    }
}

CGexMoTaskItem* CGexMoTaskDataPump::Duplicate(QString copyName)
{
    return new CGexMoTaskDataPump(this, copyName);
}

CGexMoTaskDataPump::~CGexMoTaskDataPump()
{
}

void CGexMoTaskDataPump::SetProperties(GexMoDataPumpTaskData *properties)
{
    mProperties = properties;
    mTaskProperties = properties;
}

GexMoDataPumpTaskData* CGexMoTaskDataPump::GetProperties()
{
    return mProperties;
}

int CGexMoTaskDataPump::GetTaskType()
{
    return GEXMO_TASK_DATAPUMP;
}

QString CGexMoTaskDataPump::GetTypeName()
{
    return "DATAPUMP";
}

bool CGexMoTaskDataPump::IsExecutable()
{
    return true;
}

QString CGexMoTaskDataPump::GetDatabaseName()
{
    QString DatabaseName = CGexMoTaskItem::GetDatabaseName();
    if(DatabaseName.isEmpty() && mProperties)
    {
        DatabaseName = mProperties->strDatabaseTarget;
    }
    return DatabaseName;
}

bool CGexMoTaskDataPump::SetDatabaseName(QString DatabaseName)
{
    CGexMoTaskItem::SetDatabaseName(DatabaseName);
    if(mProperties)
    {
        mProperties->strDatabaseTarget = DatabaseName;
    }
    return true;
}

QString CGexMoTaskDataPump::GetFrequencyLabel()
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

QString CGexMoTaskDataPump::GetDataFilePath()
{
    QString Path;

    if(mProperties)
    {
        Path = mProperties->strDataPath;
    }

    return Path;
}

QString CGexMoTaskDataPump::GetDataFileExtension()
{
    QString Ext;

    if(mProperties)
    {
        Ext = mProperties->strImportFileExtensions;
    }

    return Ext;
}

bool CGexMoTaskDataPump::IsDataFileScanSubFolder()
{
    bool ScanSubFolder = false;

    if(mProperties)
    {
        ScanSubFolder = mProperties->bScanSubFolders;
    }

    return ScanSubFolder;
}

QDir::SortFlags CGexMoTaskDataPump::GetDataFileSort()
{
    QDir::SortFlags SortFile = CGexMoTaskItem::GetDataFileSort();

    if(mProperties)
    {
        SortFile = mProperties->eSortFlags;
    }

    return SortFile;
}

bool CGexMoTaskDataPump::CheckExecutionWindow()
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

bool CGexMoTaskDataPump::CheckFrequency()
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

QString CGexMoTaskDataPump::Execute(QString DataFile)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteDataPumpTask(this, DataFile);
}

GexMoDataPumpTaskData* CGexMoTaskDataPump::GetDataPumpData()
{
    return mProperties;
}

void CGexMoTaskDataPump::SaveTaskToXML(QTextStream& XMLStream)
{
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveTaskSectionDataPump(this);
}

bool CGexMoTaskDataPump::LoadTaskDataFromDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadDbTaskSectionDataPump(this);
}

void CGexMoTaskDataPump::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;

    int oldStatus = m_iStatus;
    QString oldInfoMsg = m_strLastInfoMsg;

    // Leave File on disk is obsolete
    // Force the user to select new option
    if(mProperties->iPostImport == GEXMO_POSTIMPORT_LEAVE)
    {
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg = "Option 'Leave files on server' is obsolete. Select a new option...";
        return;
    }

    // Datapump must be uploaded

    this->CheckDatabase(checkDatabaseOption);
    if(this->m_iStatus != MO_TASK_STATUS_NOERROR)
    {
        return;
    }

    // For LOAD-BALANCING

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().IsLoadBalancingMode())
    {
        if(m_iDatabaseId < 0)
        {
            // Database not referenced into ym_admin_db
            m_iStatus = MO_TASK_STATUS_WARNING;
            m_strLastInfoMsg = "Load-balancing OFF. Database not uploaded. ";
            GexDatabaseEntry *dbEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(this->GetDatabaseName(),false);
            if(dbEntry && dbEntry->IsBlackHole())
            {
                m_strLastInfoMsg+= "Select "
                                    +GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseGalaxyBlackHoleName()
                                    +"...";
            }
            else
            {
                m_strLastInfoMsg+= "Select a new database...";
            }
            return;
        }

        if(IsLocal() && (GetID() != 0))// new task ignored
        {
            // Task must be uploaded
            m_iStatus = MO_TASK_STATUS_WARNING;
            m_strLastInfoMsg = "Load-balancing OFF. Task must be uploaded";
            return;
        }
    }

    // Check Path access

    if((oldStatus != m_iStatus) || (oldInfoMsg != m_strLastInfoMsg))
    {
        checkFolderOption = true;
    }

    if(checkFolderOption)
    {
        QStringList folderList;
        folderList << mProperties->strDataPath;
        if(mProperties->iPostImportDelay > 1)
            folderList << mProperties->strPostImportDelayMoveFolder;
        if(mProperties->iPostImportFailure > 1)
            folderList << mProperties->strPostImportFailureMoveFolder;
        if(mProperties->iPostImport > 1)
            folderList << mProperties->strPostImportMoveFolder;
        if(mProperties->bExecuteBatchAfterInsertion)
            folderList << mProperties->strBatchToExecuteAfterInsertion;

        this->CheckFolder(folderList);
    }
}

CGexMoTaskItem* CGexMoTaskDataPump::CreateTask(SchedulerGui *gui, bool readOnly)
{
    if(gui)
    {
        return gui->CreateTaskDataPump(this->GetTaskType(), this, readOnly);
    }
    return NULL;
}

void CGexMoTaskDataPump::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewDataPumpItem(this, nCurrentRow, allowEdit);
    }
}
