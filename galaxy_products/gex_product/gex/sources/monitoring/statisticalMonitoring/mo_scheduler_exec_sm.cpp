#include "statistical_monitoring_task.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_alarm_struct.h"

#include "message.h"
#include "engine.h"
#include "db_engine.h"
#include "scheduler_engine.h"
#include "db_external_database.h"
#include "gex_database_entry.h"
#include "gexdb_plugin_base.h"
#include "gqtl_datakeys.h"
#include <gqtl_log.h>
#include "mo_email.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "gex_constants.h"
#include "gqtl_sysutils.h"
#include "gex_database_filter.h"
#include "product_info.h"
#include "gexmo_constants.h"

// \brief GS::Gex::SchedulerEngine::ExecuteStatMonTask
// \param ptTask
// \param dbKeysContent
// \return
QString GS::Gex::SchedulerEngine::ExecuteStatMonTask(
        CGexMoTaskStatisticalMonitoring *ptTask,
        QtLib::DatakeysContent &dbKeysContent)
{
    // Check if rule disabled
    if(!ptTask->IsUsable())
    {
        return "delay: Task disabled because it is not usable";
    }

    QString lTaskType = ptTask->GetTypeName();
    // Add some additional info on the execution of this task
    // into the ym_events
    dbKeysContent.Set(lTaskType + "Task",ptTask->GetName());
    dbKeysContent.Set(lTaskType + "Status","FAIL");
    dbKeysContent.Set(lTaskType + "Info","Execution failed");

    QString lMessageStatus;

    QString strString = "Execute " + lTaskType + " Task '";
    strString += ptTask->GetProperties()->GetAttribute(C_TITLE).toString();
    strString += "' ";
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    QString lProductName = dbKeysContent.Get("Product").toString();
    QString lLot = dbKeysContent.Get("Lot").toString();
    QString lSubLotId = dbKeysContent.Get("SubLot").toString();
    QString lWaferId = dbKeysContent.Get("Wafer").toString();
    QChar lExecutionType = QChar('T'); // for Trigger execution
    if(dbKeysContent.Get("InsertionStatus").toString() == "PASS")
    {
        lExecutionType = QChar('I'); // for Datapump Insertion execution
    }

    // On insertion, do the check on the current splitlot_id
    long lSplitlotId = -1;
    if(lExecutionType == QChar('I'))
    {
        lSplitlotId = dbKeysContent.Get("SplitlotId").toLongLong();
    }

    QString lExecution;
    lExecution = "TaskName="+ptTask->GetName();
    lExecution+= "|Database="+dbKeysContent.Get("DatabaseName").toString();
    lExecution+= "|TestingStage="+dbKeysContent.Get("TestingStage").toString();
    lExecution+= "|Product="+lProductName;
    lExecution+= "|Lot="+lLot;
    lExecution+= "|SubLot="+lSubLotId;
    lExecution+= "|Wafer="+lWaferId;
    if(lExecutionType == QChar('I'))
    {
        lExecution+= "|SplitlotId="+QString::number(lSplitlotId);
        lExecution+= "|ExecutionType=OnInsertion";
    }
    else
        lExecution+= "|ExecutionType=OnTrigger";

    // Get Email spooling folder.
    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        lMessageStatus = "delay: Reporting: failed to get Email spooling folder";
        return lMessageStatus;
    }

    // We have a spooling folder: create email file in it!
    QString lMailFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    lMailFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    lMailFilePath += GEXMO_AUTOREPORT_EMAILS;
    onStartTask(ptTask,lExecution);

    lMessageStatus = ptTask->ExecuteTask(dbKeysContent, lMailFilePath);

    onStopTask(ptTask,lMessageStatus);
    return lMessageStatus;
}
