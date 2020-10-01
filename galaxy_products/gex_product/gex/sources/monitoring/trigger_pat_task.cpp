#include "trigger_pat_task.h"
#include "datapump/datapump_taskdata.h"

#include "engine.h"
#include "scheduler_engine.h"

CGexMoTaskTriggerPatPump::CGexMoTaskTriggerPatPump(QObject *parent)
    : CGexMoTaskDataPump(parent)
{
}

CGexMoTaskTriggerPatPump::CGexMoTaskTriggerPatPump(CGexMoTaskTriggerPatPump* orig, QString copyName)
    : CGexMoTaskDataPump(orig, copyName)
{
}

CGexMoTaskTriggerPatPump::~CGexMoTaskTriggerPatPump()
{
}

QString CGexMoTaskTriggerPatPump::Execute(QString DataFile)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteTriggerPumpTask(this, DataFile);
}

void CGexMoTaskTriggerPatPump::CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption)
{
    m_strName = mProperties->strTitle;
    m_strDatabaseName = mProperties->strDatabaseTarget = "";
    m_iDatabaseId = 0;

    int oldStatus = m_iStatus;
    QString oldInfoMsg = m_strLastInfoMsg;

    // Force the user to select new option
    if(mProperties->bExecuteBatchAfterInsertion)
    {
        m_iStatus = MO_TASK_STATUS_WARNING;
        m_strLastInfoMsg = "Option 'Execute Batch on insertion' is not allowed. Option will be disabled...";
        mProperties->bExecuteBatchAfterInsertion = false;
    }

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
        this->CheckFolder(folderList);
    }
}
