///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined unix || __MACH__
#include <unistd.h>
#include <errno.h>
#elif defined(WIN32)
#include <io.h>
#endif

#include <QRadioButton>
#include <QRegExp>
#include <QApplication>
#include <QDesktopWidget>
#include <QSqlError>
#include <QListIterator>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_taskdata.h"
#include "reporting/reporting_taskdata.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_taskdata.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "datapump/datapump_task.h"
#include "pat/pat_task.h"
#include "trigger/trigger_task.h"
#include "yield/yield_task.h"
#include "reporting/reporting_task.h"
#include "status/status_task.h"
#include "converter/converter_task.h"
#include "outlierremoval/outlierremoval_task.h"
#include "autoadmin/autoadmin_task.h"
#include "spm/spm_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"

#include "mo_email.h"
#include "admin_engine.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// report_build.cpp
extern CReportOptions   ReportOptions;      // Holds options (report_build.h)

// in main.cpp
extern void             WriteDebugMessageFile(const QString & strMessage);
extern GexScriptEngine* pGexScriptEngine;

///////////////////////////////////////////////////////////
// Load Tasks from disk
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::LoadLocalTasks(bool bForceReload/*=true*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadLocalTasks - bForceReload = %1").arg(bForceReload?"true":"false")
          .toLatin1().constData());

    // Load Tasks from Local File
    QString         strString;
    QFileInfo       cFileInfo(GS::Gex::Engine::GetInstance().GetTasksXmlFileName());   // Task file.
    CGexMoTaskItem* ptTaskItem;

    // Check if new Tasks Xml file exist
    // Else use the old version
    if(!cFileInfo.exists())
        CGexSystemUtils::CopyFile(GS::Gex::Engine::GetInstance().GetOldTasksFileName(),
                                  GS::Gex::Engine::GetInstance().GetTasksXmlFileName());

    // If Reload check enabled, check if new task file available...
    if(bForceReload == false)
    {
        QDateTime clLastFileUpdate = cFileInfo.lastModified();
        // Check if Task file more recent than when last loaded...
        GSLOG(SYSLOG_SEV_DEBUG, QString("LoadLocalTasks - cFileInfo = %1")
              .arg(clLastFileUpdate.toString()).toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG, QString("LoadLocalTasks - m_dtTaskLoadTimestamp = %1")
              .arg(mLastLocalTasksLoaded.toString()).toLatin1().constData());
        if(mLastLocalTasksLoaded.secsTo(clLastFileUpdate) < 5)
        {
            // File not old enough!
            // Nothing todo
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "LoadLocalTasks - Load Tasks DONE");
            return;
        }
    }

    // Reset all local tasks
    foreach(ptTaskItem, mTasksList)
    {
        if(ptTaskItem && ptTaskItem->IsLocal())
        {
            // Remove this task from the task list
            DeleteTaskInList(ptTaskItem);
        }
    }

    mLocalTasksIndex = -1;

    // Build path to the 'Tasks' list.
    QFile file( cFileInfo.absoluteFilePath() ); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
        return; // Failed opening Tasks file.


    // Read Tasks definition File
    mLocalTasksFile.setDevice(&file);    // Assign file handle to data stream

    // Goto <tasks> header
    while(mLocalTasksFile.atEnd() == false)
    {
        strString = mLocalTasksFile.readLine();
        if(strString == "<tasks>")
            break;
    }

    // Check if valid header...or empty!
    if(strString != "<tasks>")
    {
        file.close();
        return;
    }

    // Load task list (read XML file)
    do
    {
        // Read one line from file
        QString strString = mLocalTasksFile.readLine();

        ptTaskItem = NULL;

        // Task type: DATA PUMP
        if(strString.startsWith("<task_data_pump>") == true)
            ptTaskItem = LoadTaskSectionDataPump();

        // Task type: YIELD MONITORING
        else if(strString.startsWith("<task_yield_monitoring>") == true)
            ptTaskItem = LoadTaskSectionYieldMonitoring_OneRule();

        // Task type: REPORTING
        else if(strString.startsWith("<task_reporting>") == true)
            ptTaskItem = LoadTaskSectionReporting();

        // Task type: STATUS
        else if(strString.startsWith("<task_status>") == true)
            ptTaskItem = LoadTaskSectionStatus();

        // Task type: FILE CONVERTER
        else if(strString.startsWith("<task_file_converter>") == true)
            ptTaskItem = LoadTaskSectionFileConverter();

        // Task type: OUTLIER REMOVAL
        else if(strString.startsWith("<task_outlier_monitoring>") == true)
            ptTaskItem = LoadTaskSectionOutlierRemoval();

        // Task type: AUTO ADMIN
        else if(strString.startsWith("<task_autoadmin>") == true)
            ptTaskItem = LoadTaskSectionAutoAdmin();

        else
            continue;

        if(ptTaskItem == NULL)
            continue;

        ptTaskItem->SetID(mLocalTasksIndex--);

        // Add task to internal structure
        mTasksList.append(ptTaskItem);

        // Update the current NodeId
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            ptTaskItem->m_iNodeId = GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId();
    } while(mLocalTasksFile.atEnd() == false);

    file.close();

    // Save timestamp of Task file loaded
    mLastLocalTasksLoaded = QFileInfo(GS::Gex::Engine::GetInstance().GetTasksXmlFileName()).lastModified();


    GSLOG(SYSLOG_SEV_INFORMATIONAL, "LoadLocalTasks - Load all Tasks DONE");
}

///////////////////////////////////////////////////////////
// Load from disk...section: Data Pump
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionDataPump()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    int     nValue;
    double  lfValue;
    int     Type=GEXMO_TASK_DATAPUMP;

    // Allocate buffer to store information read from disk.
    GexMoDataPumpTaskData *ptTaskDataPump = new GexMoDataPumpTaskData(this);
    bool bHavePriority = false;
    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskDataPump->strTitle = strSection;

        else if(strString.startsWith("Type=") == true)
            Type = strSection.toInt();

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        // Read data path
        else if(strString.startsWith("DataPath=") == true)
            ptTaskDataPump->strDataPath = strSection;

        // Scan sub-folder flag
        else if(strString.startsWith("ScanSubFolders=") == true)
        {
            if(strSection == "YES")
                ptTaskDataPump->bScanSubFolders = true;
            else
                ptTaskDataPump->bScanSubFolders = false;
        }

        // Sort By flag
        else if(strString.startsWith("SortBy=") == true)
        {
            ptTaskDataPump->eSortFlags = QDir::Name;
            if(strSection.startsWith("Time"))
            {
                ptTaskDataPump->eSortFlags = QDir::Time;
                if(strSection.endsWith("Desc"))
                    ptTaskDataPump->eSortFlags |= QDir::Reversed;
            }
        }

        // List of files extensions to import
        else if(strString.startsWith("ImportFileExtensions=") == true)
        {       ptTaskDataPump->strImportFileExtensions = strSection;  }

        // Read Database targetted
        else if(strString.startsWith("Database=") == true)
        {   ptTaskDataPump->strDatabaseTarget = strSection;  }

        // Read Data Type targetted
        else if(strString.startsWith("DataType=") == true)
        {   ptTaskDataPump->uiDataType = strSection.toUInt(); }

        // Read Testing stage (if WYR data type)
        else if(strString.startsWith("TestingStage=") == true)
        {   ptTaskDataPump->strTestingStage = strSection; }

        // Read Task Priority
        else if(strString.startsWith("Priority=") == true)
        {
            bHavePriority = true;
            ptTaskDataPump->mPriority = strSection.toInt();
        }

        // Read Task frequency
        else if(strString.startsWith("Frequency=") == true)
            ptTaskDataPump->iFrequency = strSection.toLong();

        // Read Task Day of Week execution
        else if(strString.startsWith("DayOfWeek=") == true)
        {
            strString = strSection;
            ptTaskDataPump->iDayOfWeek = strString.toLong();
        }

        // Execution window flag
        else if(strString.startsWith("ExecWindow=") == true)
        {
            if(strSection == "YES")
                ptTaskDataPump->bExecutionWindow = true;
            else
                ptTaskDataPump->bExecutionWindow = false;
        }

        // Read Start-time
        else if(strString.startsWith("StartTime=") == true)
        {
            strString = strSection;
            strSection = strString.section(',',0,0);
            int iHour = strSection.toInt();
            strSection = strString.section(',',1,1);
            int iMinute = strSection.toInt();
            QTime tStartTime(iHour,iMinute);
            ptTaskDataPump->cStartTime = tStartTime;
        }

        // Read Stop-time
        else if(strString.startsWith("StopTime=") == true)
        {
            strString = strSection;
            strSection = strString.section(',',0,0);
            int iHour = strSection.toInt();
            strSection = strString.section(',',1,1);
            int iMinute = strSection.toInt();
            QTime tStopTime(iHour,iMinute);
            ptTaskDataPump->cStopTime = tStopTime;
        }

        // Read PostImport task: Rename, Move or Delete files.
        else if(strString.startsWith("PostImport=") == true)
        {
            strString = strSection;
            ptTaskDataPump->iPostImport= strString.toLong();
        }

        // Read Move/FTP folder
        else if(strString.startsWith("PostImportFolder=") == true)
        {   ptTaskDataPump->strPostImportMoveFolder = strSection;
        }

        // Read PostImport task (failed files): Rename, Move or Delete files.
        else if(strString.startsWith("PostImportFailure=") == true)
        {
            strString = strSection;
            ptTaskDataPump->iPostImportFailure= strString.toLong();
        }
        // Read Move/FTP folder (failed files)
        else if(strString.startsWith("PostImportFailureFolder=") == true)
        {   ptTaskDataPump->strPostImportFailureMoveFolder = strSection; }

        // Read PostImport task (delayed files): Rename, Move or Delete files.
        else if(strString.startsWith("PostImportDelay=") == true)
        {
            strString = strSection;
            ptTaskDataPump->iPostImportDelay= strString.toLong();
        }
        // Read Move/FTP folder (delayed files)
        else if(strString.startsWith("PostImportDelayFolder=") == true)
        {   ptTaskDataPump->strPostImportDelayMoveFolder = strSection; }

        // check Yield window flag
        else if(strString.startsWith("CheckYield=") == true)
        {
            if(strSection == "YES")
                ptTaskDataPump->bCheckYield = true;
            else
                ptTaskDataPump->bCheckYield = false;
        }

        // Read Good bin list
        else if(strString.startsWith("YieldBins=") == true)
        {   ptTaskDataPump->strYieldBinsList = strSection; }

        // Read Alarm level (0-100%)
        else if(strString.startsWith("AlarmLevel=") == true)
        {
            strString = strSection;
            ptTaskDataPump->iAlarmLevel = strString.toInt();
        }

        // Read Minimum parts to have a valid file
        else if(strString.startsWith("MinimumParts=") == true)
        {
            strString = strSection;
            ptTaskDataPump->lMinimumyieldParts = strString.toLong();
        }

        // Read Email 'From'
        else if(strString.startsWith("EmailFrom=") == true)
        {
            ptTaskDataPump->strEmailFrom = strSection;
        }

        // Read Email notification list
        else if(strString.startsWith("Emails=") == true)
        {
            ptTaskDataPump->strEmailNotify = strSection;
        }

        // Read Email format: HTML or TXT
        else if(strString.startsWith("EmailFormat=") == true)
        {
            if(strSection == "HTML")
                ptTaskDataPump->bHtmlEmail = true;
            else
                ptTaskDataPump->bHtmlEmail = false;
        }

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }

        else if(strString.startsWith("</task_data_pump>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskDataPump *ptTaskItem = NULL;
            switch(Type)
            {
            case GEXMO_TASK_PATPUMP:
                ptTaskItem = new CGexMoTaskPatPump;
                break;
            case GEXMO_TASK_TRIGGERPUMP:
                ptTaskItem = new CGexMoTaskTriggerPump;
                break;
            default:
                ptTaskItem = new CGexMoTaskDataPump;
                break;
            }

            // For DataPump,TriggerPump,PatPump

            ptTaskItem->SetProperties(ptTaskDataPump);
            ptTaskItem->m_strName           = ptTaskDataPump->strTitle;
            ptTaskItem->m_strDatabaseName   = ptTaskDataPump->strDatabaseTarget;
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted     = tLastExecuted;

            // Old frequency
            // < 30mn [7] => High
            // < 1h   [8] => Medium
            // else       => Low
            if(!bHavePriority)
            {
                if(ptTaskDataPump->iFrequency <= 7)
                    ptTaskDataPump->mPriority = 2;
                else if(ptTaskDataPump->iFrequency <= 8)
                    ptTaskDataPump->mPriority = 1;
                else
                    ptTaskDataPump->mPriority = 0;
            }

            ptTaskDataPump->SetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName, GexMoDataPumpTaskData::sIllegalScript);
            ptTaskDataPump->SetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName, false);
            ptTaskDataPump->SetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName, GexMoDataPumpTaskData::sIllegalScript);
            ptTaskDataPump->SetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName, false);

            // Stop reading this section!
            return ptTaskItem;
        }

        // Other options
        else if(strString.startsWith("RejectFilesWithFewerParts=") == true)
        {
            // Old option where only 1 mode was supported (abs nb of parts, -1 for disabled)
            nValue = strSection.toInt();
            if(nValue > 0)
            {
                ptTaskDataPump->bRejectSmallSplitlots_NbParts = true;
                ptTaskDataPump->uiRejectSmallSplitlots_NbParts = (unsigned int)nValue;
            }
        }
        else if(strString.startsWith("RejectSmallSplitlots_NbParts=") == true)
        {
            // New option, nb_parts mode (abs nb of parts, -1 for disabled)
            nValue = strSection.toInt();
            if(nValue > 0)
            {
                ptTaskDataPump->bRejectSmallSplitlots_NbParts = true;
                ptTaskDataPump->uiRejectSmallSplitlots_NbParts = (unsigned int)nValue;
            }
        }
        else if(strString.startsWith("RejectSmallSplitlots_Gdpw_Percent=") == true)
        {
            // New option, gdpw_percent mode (percentage of GDPW, -1 for disabled)
            lfValue = strSection.toDouble();
            if(lfValue > 0.0)
            {
                ptTaskDataPump->bRejectSmallSplitlots_GdpwPercent = true;
                ptTaskDataPump->lfRejectSmallSplitlots_GdpwPercent = lfValue;
            }
        }

        else if(strString.startsWith("ExecuteBatchAfterInsertion=") == true)
        {
            if(strSection == "YES")
                ptTaskDataPump->bExecuteBatchAfterInsertion = true;
            else
                ptTaskDataPump->bExecuteBatchAfterInsertion = false;
        }
        else if(strString.startsWith("BatchToExecuteAfterInsertion=") == true)
            ptTaskDataPump->strBatchToExecuteAfterInsertion = strSection;
        else if(strString.startsWith("MaxPartsForTestResultInsertion=") == true)
        {
            strString = strSection;
            ptTaskDataPump->nMaxPartsForTestResultInsertion = strString.toInt();
        }
        else if(strString.startsWith("RejectFilesOnPassBinlist=") == true)
        {
            if(strSection == "YES")
                ptTaskDataPump->bRejectFilesOnPassBinlist = true;
            else
                ptTaskDataPump->bRejectFilesOnPassBinlist = false;
        }
        else if(strString.startsWith("PassBinlistForRejectTest=") == true)
            ptTaskDataPump->strPassBinListForRejectTest = strSection;
        else
        {
            // Unknown option !!!
            if(!QVariant(strSection).isNull())
                ptTaskDataPump->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskDataPump)
        delete ptTaskDataPump;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: YIELD MONITORING RDB  (SYL/SBL)
///////////////////////////////////////////////////////////
CGexMoTaskItem*  GS::Gex::SchedulerEngine::LoadTaskSectionYieldMonitoring_OneRule(void)
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer for one Rule.
    GexMoYieldMonitoringTaskData *ptTaskYield = NULL;
    ptTaskYield = new GexMoYieldMonitoringTaskData(this);
    ptTaskYield->clear();

    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
        {
            ptTaskYield->strTitle = strSection;
            continue;
        }

        if(strString.startsWith("State=") == true)
        {   bEnabledState = (strSection == "1"); continue; }

        // Read ProductID
        if(strString.startsWith("ProductID=") == true)
        {
            ptTaskYield->strProductID = strSection;
            continue;
        }

        // Read Yield Bin list
        if(strString.startsWith("YieldBins=") == true)
        {   ptTaskYield->strYieldBinsList = strSection; continue; }

        // Bin list type: 0=Good bins, 1=Failing bins.
        if(strString.startsWith("BiningType=") == true)
        {   ptTaskYield->iBiningType = strSection.toInt(); continue; }

        // Read Alarm level (0-100%)
        if(strString.startsWith("AlarmLevel=") == true)
        {   ptTaskYield->iAlarmLevel = strSection.toInt(); continue; }

        // Read Flag: Check if Yield OVER or UNDER the limit.
        if(strString.startsWith("AlarmDirection=") == true)
        {   ptTaskYield->iAlarmIfOverLimit = strSection.toInt(); continue; }

        // Read Minimum parts to have a valid file
        if(strString.startsWith("MinimumParts=") == true)
        {   ptTaskYield->lMinimumyieldParts = strSection.toLong(); continue; }

        // Read SBL/YBL data file (if exists)
        if(strString.startsWith("SblFile=") && QFile::exists(strSection))
        {
            GSLOG(SYSLOG_SEV_WARNING, "SYASBL through file no more supported in this version");
            //ptTaskYield->strSblFile = strSection;
            continue;
        }

        // Read Email 'From'
        if(strString.startsWith("EmailFrom=") == true)
        {
            ptTaskYield->strEmailFrom = strSection;
            continue;
        }

        // Read Email notification list
        if(strString.startsWith("Emails=") == true)
        {   ptTaskYield->strEmailNotify = strSection; continue;
        }

        // Read Email format: HTML or TXT
        if(strString.startsWith("EmailFormat=") == true)
        {
            if(strSection == "HTML")
                ptTaskYield->bHtmlEmail = true;
            else
                ptTaskYield->bHtmlEmail = false;
            continue;
        }

        // Read Email message contents type to send
        if(strString.startsWith("EmailReportType=") == true)
        {   ptTaskYield->iEmailReportType = strSection.toInt(); continue; }

        // Read Email report notification type: send as attachment or leave on server
        if(strString.startsWith("NotificationType=") == true)
        {   ptTaskYield->iNotificationType = strSection.toInt(); continue; }

        // Read Alarm type: Standard, Critical...
        if(strString.startsWith("ExceptionLevel=") == true)
        {   ptTaskYield->iExceptionLevel = strSection.toInt(); continue; }

        /////////////////////////////////////////////////////////////////////////////
        // SYL-SBL specifics
        /////////////////////////////////////////////////////////////////////////////
        if(strString.startsWith("ActiveOnDatafileInsertion=") == true)
        {   ptTaskYield->bSYA_active_on_datafile_insertion = strSection == "1"; continue; }

        if(strString.startsWith("ActiveOnTriggerFile=") == true)
        {   ptTaskYield->bSYA_active_on_trigger_file = strSection == "1"; continue; }

        if(strString.startsWith("BinRule=") == true)
        {
            int     nBinNo;
            int     nRuleType;
            bool    bIsNumber;

            nBinNo = strSection.section("|",0,0).toInt(&bIsNumber);

            // If no error
            if(bIsNumber)
            {
                nRuleType = strSection.section("|",1,1).toInt(&bIsNumber);

                // Add the rule bin
                ptTaskYield->mapBins_rules[nBinNo]["RuleType"] = nRuleType;
                if(nRuleType == eManual)
                {
                    ptTaskYield->mapBins_rules[nBinNo]["LL1"] = strSection.section("|",2,2).toFloat();
                    ptTaskYield->mapBins_rules[nBinNo]["HL1"] = strSection.section("|",3,3).toFloat();
                    ptTaskYield->mapBins_rules[nBinNo]["LL2"] = strSection.section("|",4,4).toFloat();
                    ptTaskYield->mapBins_rules[nBinNo]["HL2"] = strSection.section("|",5,5).toFloat();
                }
                else
                {
                    ptTaskYield->mapBins_rules[nBinNo]["N1"] = strSection.section("|",2,2).toFloat();
                    ptTaskYield->mapBins_rules[nBinNo]["N2"] = strSection.section("|",3,3).toFloat();
                }
            }
            continue;
        }

        if(strString.startsWith("Database=") == true)
        {   ptTaskYield->strDatabase = strSection; continue; }

        if(strString.startsWith("TestingStage=") == true)
        {   ptTaskYield->strTestingStage = strSection; continue; }

        if(strString.startsWith("RuleType=") == true)
        {
            bool ok=false;
            ptTaskYield->eSYA_Rule = (OutlierRule) strSection.toInt(&ok);  // Rule: 0=N*Sigma, 1=....
            if (!ok)
                GSLOG(SYSLOG_SEV_ERROR, QString("Undefined RuleType %1").arg( strSection).toLatin1().constData() );
            continue;
        }

        if(strString.startsWith("RuleTypeString=") == true)     // Rule string: N*Sigma, N*IQR
        {   ptTaskYield->strSYA_Rule = strSection; continue; }

        if(strString.startsWith("N_Parameter=") == true)
        {   ptTaskYield->fSYA_N1_value = strSection.toFloat();  // N parameter (compatibility, new fields are N1, N2)
            continue;
        }

        if(strString.startsWith("N1_Parameter=") == true)
        {   ptTaskYield->fSYA_N1_value = strSection.toFloat();  // N1 parameter
            continue;
        }

        if(strString.startsWith("N2_Parameter=") == true)
        {   ptTaskYield->fSYA_N2_value = strSection.toFloat();  // N2 parameter
            continue;
        }
        if(strString.startsWith("MinimumLotsRequired=") == true)
        {   ptTaskYield->iSYA_LotsRequired = strSection.toInt();// Minimum Total lots required for computing new SYL-SBL
            continue;
        }
        if(strString.startsWith("ValidityPeriod=") == true)
        {   ptTaskYield->iSYA_Period = strSection.toInt();// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...
            continue;
        }
        if(strString.startsWith("ExpirationDate=") == true)
        {
            int iDay,iMonth,iYear;
            iDay = strSection.section(' ',0,0).trimmed().toInt();
            iMonth = strSection.section(' ',1,1).trimmed().toInt();
            iYear = strSection.section(' ',2,2).trimmed().toInt();
            QDate d = QDate(iYear,iMonth,iDay);
            ptTaskYield->SetAttribute("ExpirationDate",d);
            continue;
        }

        if(strString.startsWith("SBL_LL_Disabled=") == true)
        {
            ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;
            ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;
            continue;
        }

        if(strString.startsWith("SBL1_LL_Disabled=") == true)
        {   ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;
            continue;
        }
        if(strString.startsWith("SBL2_LL_Disabled=") == true)
        {   ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;
            continue;
        }

        if(strString.startsWith("SBL_HL_Disabled=") == true)
        {
            ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;
            ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;
            continue;
        }

        if(strString.startsWith("SBL1_HL_Disabled=") == true)
        {   ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;
            continue;
        }

        if(strString.startsWith("SBL2_HL_Disabled=") == true)
        {   ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;
            continue;
        }

        if(strString.startsWith("SYL_LL_Disabled=") == true)
        {
            ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";
            ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("SYL1_LL_Disabled=") == true)
        {   ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("SYL2_LL_Disabled=") == true)
        {   ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("SYL_HL_Disabled=") == true)
        {
            ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";
            ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("SYL1_HL_Disabled=") == true)
        {   ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("SYL2_HL_Disabled=") == true)
        {   ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";
            continue;
        }

        if(strString.startsWith("IgnoreDataPointsWithNullSigma=") == true)
        {   ptTaskYield->bSYA_IgnoreDataPointsWithNullSigma = strSection == "1";
            continue;
        }

        if(strString.startsWith("IgnoreOutliers=") == true)
        {   ptTaskYield->bSYA_IgnoreOutliers = strSection == "1";
            continue;
        }

        if(strString.startsWith("UseGrossDie=") == true)
        {   ptTaskYield->bSYA_UseGrossDie = strSection == "1";
            continue;
        }

        if(strString.startsWith("MinDataPoints=") == true)
        {   ptTaskYield->iSYA_MinDataPoints = strSection.toInt(); continue; }


        // Read Last time task was executed...
        if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
            continue;
        }

        if(strString.startsWith("</task_yield_monitoring_rdb_YL>")
                || strString.startsWith("</task_yield_monitoring>"))
        {
            // End of section, add Entry to the task list
            // Creating a new task.
            CGexMoTaskYieldMonitor *ptTaskItem = new CGexMoTaskYieldMonitor;
            ptTaskItem->SetProperties(ptTaskYield);
            ptTaskItem->m_strName           = ptTaskYield->strTitle;
            ptTaskItem->m_strDatabaseName   = ptTaskYield->strDatabase;
            ptTaskItem->m_tLastExecuted     = tLastExecuted;
            ptTaskItem->SetEnabledState(bEnabledState);

            if((ptTaskItem->GetAttribute("CheckType").isNull())
                    || (ptTaskItem->GetAttribute("CheckType").toString() == "Unknown"))
            {
                // Check if the CheckType is setted
                ptTaskItem->SetAttribute("CheckType", QVariant("FixedYieldTreshold"));
            }

            return ptTaskItem;
        }

        if(strString.startsWith("</task_yield_monitoring_rdb>") == true)
        {
            // Stop reading this section!
            if(ptTaskYield)
                delete ptTaskYield;
            return NULL;
        }

        // if not hard coded in ptTaskYield, let s add it to the map
        ptTaskYield->SetAttribute(strString.section('=',0,0), QVariant(strSection));
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskYield)
        delete ptTaskYield;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: REPORTING
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionReporting()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer to store information read from disk.
    GexMoReportingTaskData *ptTaskReporting = new GexMoReportingTaskData(this);
    bool bHavePriority = false;
    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskReporting->strTitle = strSection;

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        // Read Script path
        else if(strString.startsWith("ScriptPath=") == true)
            ptTaskReporting->strScriptPath = strSection;

        // Read Task Priority
        else if(strString.startsWith("Priority=") == true)
        {
            bHavePriority = true;
            ptTaskReporting->mPriority = strSection.toInt();
        }


        // Read Task frequency
        else if(strString.startsWith("Frequency=") == true)
        {
            strString = strSection;
            ptTaskReporting->iFrequency = strString.toLong();
        }

        // Read Task Day of Week execution
        else if(strString.startsWith("DayOfWeek=") == true)
        {
            strString = strSection;
            ptTaskReporting->iDayOfWeek = strString.toLong();
        }

        // Execution window flag
        else if(strString.startsWith("ExecWindow=") == true)
        {
            if(strSection == "YES")
                ptTaskReporting->bExecutionWindow = true;
            else
                ptTaskReporting->bExecutionWindow = false;
        }

        // Read Start-time
        else if(strString.startsWith("StartTime=") == true)
        {
            strString = strSection;
            strSection = strString.section(',',0,0);
            int iHour = strSection.toInt();
            strSection = strString.section(',',1,1);
            int iMinute = strSection.toInt();
            QTime tStartTime(iHour,iMinute);
            ptTaskReporting->cStartTime = tStartTime;
        }

        // Read Stop-time
        else if(strString.startsWith("StopTime=") == true)
        {
            strString = strSection;
            strSection = strString.section(',',0,0);
            int iHour = strSection.toInt();
            strSection = strString.section(',',1,1);
            int iMinute = strSection.toInt();
            QTime tStopTime(iHour,iMinute);
            ptTaskReporting->cStopTime = tStopTime;
        }

        // Read Email Notification type
        else if(strString.startsWith("NotificationType=") == true)
        {
            strString = strSection;
            ptTaskReporting->iNotificationType = strString.toInt();
        }

        // Read Email From
        else if(strString.startsWith("EmailFrom=") == true)
            ptTaskReporting->strEmailFrom = strSection;

        // Read Email notification list
        else if(strString.startsWith("Emails=") == true)
            ptTaskReporting->strEmailNotify = strSection;

        // Read Email format: HTML or TXT
        else if(strString.startsWith("EmailFormat=") == true)
        {
            if(strSection == "HTML")
                ptTaskReporting->bHtmlEmail = true;
            else
                ptTaskReporting->bHtmlEmail = false;
        }

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }

        else if(strString.startsWith("</task_reporting>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskReporting *ptTaskItem = new CGexMoTaskReporting;
            ptTaskItem->SetProperties(ptTaskReporting);

            ptTaskItem->m_strName       = ptTaskReporting->strTitle;
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted = tLastExecuted;

            // Old frequency
            // < 30mn [7] => High
            // < 1h   [8] => Medium
            // else       => Low
            if(!bHavePriority)
            {
                if(ptTaskReporting->iFrequency <= 7)
                    ptTaskReporting->mPriority = 2;
                else if(ptTaskReporting->iFrequency <= 8)
                    ptTaskReporting->mPriority = 1;
                else
                    ptTaskReporting->mPriority = 0;
            }

            // Stop reading this section!
            return ptTaskItem;
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strSection).isNull())
                ptTaskReporting->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskReporting)
        delete ptTaskReporting;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: STATUS
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionStatus()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer to store information read from disk.
    GexMoStatusTaskData *ptTaskStatus = new GexMoStatusTaskData(this);

    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskStatus->setTitle(strSection);

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        // Read Web organization type
        else if(strString.startsWith("WebOrganization=") == true)
        {
            int iOneWebPerDatabase;
            iOneWebPerDatabase = strSection.toLong();
            if(iOneWebPerDatabase)
                ptTaskStatus->setOneWebPerDatabase(true);
            else
                ptTaskStatus->setOneWebPerDatabase(false);
        }

        // Read Intranet path
        else if(strString.startsWith("IntranetPath=") == true)
            ptTaskStatus->setIntranetPath(strSection);

        // Read Home page name
        else if(strString.startsWith("HomePage=") == true)
            ptTaskStatus->setHomePage(strSection);

        // Report's URL name, use to display in Emails if http URL is empty(hyperlink)
        else if(strString.startsWith("ReportURL=") == true)
            ptTaskStatus->setReportURL(strSection);

        // Report's http URL name to display in Emails (hyperlink)
        else if(strString.startsWith("ReportHttpURL=") == true)
            ptTaskStatus->setReportHttpURL(strSection);

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }
        else if(strString.startsWith("</task_status>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskStatus *ptTaskItem = new CGexMoTaskStatus;
            ptTaskItem->SetProperties(ptTaskStatus);

            ptTaskItem->m_strName       = ptTaskStatus->title();
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted     = tLastExecuted;

            // Stop reading this section!
            return ptTaskItem;
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strSection).isNull())
                ptTaskStatus->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskStatus)
        delete ptTaskStatus;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: FileConverter
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionFileConverter()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer to store information read from disk.
    GexMoFileConverterTaskData *ptTaskDataConvert = new GexMoFileConverterTaskData(this);
    bool bHavePriority = false;
    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskDataConvert->strTitle = strSection;

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        // Input Folder
        else if(strString.startsWith("InputFolder=") == true)
            ptTaskDataConvert->strInputFolder = strSection;

        // Input file extensions
        else if(strString.startsWith("ImportFileExtensions=") == true)
            ptTaskDataConvert->strFileExtensions = strSection;

        // Read Priority
        else if(strString == "Priority")
        {
            ptTaskDataConvert->mPriority = strSection.toInt();
            bHavePriority = true;
        }

        // Execution frequency
        else if(strString.startsWith("Frequency=") == true)
            ptTaskDataConvert->iFrequency = strSection.toLong();

        // Execution Day of week (if frequency is week, or month,...)
        else if(strString.startsWith("DayOfWeek=") == true)
        {
            strString = strSection;
            ptTaskDataConvert->iDayOfWeek = strString.toLong();
        }
        // Output Folder
        else if(strString.startsWith("OutputFolder=") == true)
            ptTaskDataConvert->strOutputFolder = strSection;

        // Output Format: STDF (0) or CSV (1)
        else if(strString.startsWith("OutputFormat=") == true)
            ptTaskDataConvert->iFormat = strSection.toInt();

        // Include Timestamp info in file name to create?
        else if(strString.startsWith("TimeStampFile=") == true)
            ptTaskDataConvert->bTimeStampName = (strSection.toInt() != 0) ? true : false;

        // What to to file file successfuly converted
        else if(strString.startsWith("SuccessMode=") == true)
            ptTaskDataConvert->iOnSuccess = strSection.toInt();

        // Folder where to move source files (if successfuly converted)
        else if(strString.startsWith("SuccessFolder=") == true)
            ptTaskDataConvert->strOutputSuccess = strSection;

        // What to to file file that failed conversion
        else if(strString.startsWith("FailMode=") == true)
            ptTaskDataConvert->iOnError = strSection.toInt();

        // Folder where to move source files (if failed conversion)
        else if(strString.startsWith("FailFolder=") == true)
            ptTaskDataConvert->strOutputError = strSection;

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }
        else if(strString.startsWith("</task_file_converter>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskConverter *ptTaskItem = new CGexMoTaskConverter;
            ptTaskItem->SetProperties(ptTaskDataConvert);

            ptTaskItem->m_strName       = ptTaskDataConvert->strTitle;
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted     = tLastExecuted;

            // Old frequency
            // < 30mn [7] => High
            // < 1h   [8] => Medium
            // else       => Low
            if(!bHavePriority)
            {
                if(ptTaskDataConvert->iFrequency <= 7)
                    ptTaskDataConvert->mPriority = 2;
                else if(ptTaskDataConvert->iFrequency <= 8)
                    ptTaskDataConvert->mPriority = 1;
                else
                    ptTaskDataConvert->mPriority = 0;
            }

            // Stop reading this section!
            return ptTaskItem;
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strSection).isNull())
                ptTaskDataConvert->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskDataConvert)
        delete ptTaskDataConvert;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: OUTLIER REMOVAL
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionOutlierRemoval()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer to store information read from disk.
    GexMoOutlierRemovalTaskData *ptTaskOutlier= new GexMoOutlierRemovalTaskData(this);

    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();


        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskOutlier->strTitle = strSection;

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        else if(strString.startsWith("Database=") == true)
            ptTaskOutlier->strDatabase = strSection;

        else if(strString.startsWith("TestingStage=") == true)
            ptTaskOutlier->strTestingStage = strSection;

        // Read ProductID
        else if(strString.startsWith("ProductID=") == true)
            ptTaskOutlier->strProductID = strSection;

        // Read Alarm level
        else if(strString.startsWith("AlarmLevel=") == true)
            ptTaskOutlier->lfAlarmLevel = strSection.toDouble();

        // Read Alarm Type: % (0) or #parts (1)
        else if(strString.startsWith("AlarmType=") == true)
            ptTaskOutlier->iAlarmType = strSection.toInt();

        // Read Minimum parts to have a valid file
        else if(strString.startsWith("MinimumParts=") == true)
            ptTaskOutlier->lMinimumyieldParts = strSection.toLong();

        // Notify if distribution shape changes compared to historical data
        else if(strString.startsWith("NotifyShapeChange=") == true)
            ptTaskOutlier->bNotifyShapeChange = (bool) strSection.toLong();

        // Read Maximum number of Die mismatch between E-Test & STDF wafermaps
        else if(strString.startsWith("CompositeEtestAlarm=") == true)
            ptTaskOutlier->lCompositeEtestAlarm = strSection.toLong();

        // Read Maximum number of Die to reject on the exclusion zone stacked wafer.
        else if(strString.startsWith("CompositeExclusionZoneAlarm=") == true)
            ptTaskOutlier->lCompositeExclusionZoneAlarm = strSection.toLong();

        // Read Email 'From'
        else if(strString.startsWith("EmailFrom=") == true)
            ptTaskOutlier->strEmailFrom = strSection;

        // Read Email notification list
        else if(strString.startsWith("Emails=") == true)
            ptTaskOutlier->strEmailNotify = strSection;

        // Read Email format: HTML or TXT
        else if(strString.startsWith("EmailFormat=") == true)
        {
            if(strSection == "HTML")
                ptTaskOutlier->bHtmlEmail = true;
            else
                ptTaskOutlier->bHtmlEmail = false;
        }

        // Read Email message contents type to send
        else if(strString.startsWith("EmailReportType=") == true)
            ptTaskOutlier->iEmailReportType = strSection.toInt();

        // Read Email report notification type: send as attachment or leave on server
        else if(strString.startsWith("NotificationType=") == true)
            ptTaskOutlier->iNotificationType = strSection.toInt();

        // Read Alarm type: Standard, Critical...
        else if(strString.startsWith("ExceptionLevel=") == true)
            ptTaskOutlier->iExceptionLevel = strSection.toInt();

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }
        else if(strString.startsWith("</task_outlier_monitoring>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskOutlierRemoval *ptTaskItem = new CGexMoTaskOutlierRemoval;
            ptTaskItem->SetProperties(ptTaskOutlier);

            ptTaskItem->m_strName       = ptTaskOutlier->strTitle;
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted     = tLastExecuted;

            // Stop reading this section!
            return ptTaskItem;
        }
        else
        {
            // Unknown option !!!
            if(!QVariant(strSection).isNull())
                ptTaskOutlier->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskOutlier)
        delete ptTaskOutlier;
    return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: Auto Admin
///////////////////////////////////////////////////////////
CGexMoTaskItem* GS::Gex::SchedulerEngine::LoadTaskSectionAutoAdmin()
{
    QString strString;
    QString strSection;
    bool    bEnabledState = true;
    long    tLastExecuted = 0;

    // Allocate buffer to store information read from disk.
    GexMoAutoAdminTaskData *ptTaskAutoAdmin = new GexMoAutoAdminTaskData(this);

    do
    {
        // Read one line from file
        strString = mLocalTasksFile.readLine();
        strSection = strString.section('=',1).trimmed();

        // Read Title
        if(strString.startsWith("Title=") == true)
            ptTaskAutoAdmin->mTitle = strSection;

        else if(strString.startsWith("State=") == true)
            bEnabledState = (strSection == "1");

        // Time of day to start auto-admin
        else if(strString.startsWith("StartTime=") == true)
        {
            strString = strSection;
            strSection = strString.section(',',0,0);
            int iHour = strSection.toInt();
            strSection = strString.section(',',1,1);
            int iMinute = strSection.toInt();
            QTime tStartTime(iHour,iMinute);
            ptTaskAutoAdmin->mStartTime = tStartTime;
        }

        // Read Web organization type
        else if(strString.startsWith("KeepReportDuration=") == true)
            ptTaskAutoAdmin->mKeepReportDuration = strSection.toLong();

        // Read log file contents type
        else if(strString.startsWith("LogContents=") == true)
            ptTaskAutoAdmin->mLogContents = strSection.toLong();

        // Read Email From
        else if(strString.startsWith("EmailFrom=") == true)
            ptTaskAutoAdmin->mEmailFrom = strSection;

        // Read Email notification list
        else if(strString.startsWith("Emails=") == true)
            ptTaskAutoAdmin->mEmailNotify = strSection;

        else if (strString.startsWith("SendInsertionLogs="))
            ptTaskAutoAdmin->mSendInsertionLogs = strSection.toInt();

        else if (strString.startsWith("LastInsertionLogsSent="))
            ptTaskAutoAdmin->mLastInsertionLogsSent = QDateTime::fromTime_t(strSection.toInt()).date();

        // Read Last time task was executed...
        else if(strString.startsWith("LastExecuted=") == true)
        {
            strString = strSection;
            tLastExecuted = strString.toLong();
        }

        else if(strString.startsWith("</task_autoadmin>") == true)
        {
            // End of section, add Entry to the task list

            // Creating a new task.
            CGexMoTaskAutoAdmin *ptTaskItem = new CGexMoTaskAutoAdmin;
            ptTaskItem->SetProperties(ptTaskAutoAdmin);

            ptTaskItem->m_strName       = ptTaskAutoAdmin->mTitle;
            ptTaskItem->SetEnabledState(bEnabledState);
            ptTaskItem->m_tLastExecuted = tLastExecuted;

            // Stop reading this section!
            return ptTaskItem;
        }
        else
        {
            // any non hard coded option
            if(!QVariant(strSection).isNull())
                ptTaskAutoAdmin->SetAttribute(strString.section('=',0,0), QVariant(strSection));
        }
    }
    while(mLocalTasksFile.atEnd() == false);

    // Unexpected end of section...
    // quietly return.
    if(ptTaskAutoAdmin)
        delete ptTaskAutoAdmin;
    return NULL;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveLocalTasks(void)
{
    // Save Tasks from Local File
    // Do not save if a task is still edited
    // Force to Save if system loading the tasks
    if(mAllTasksLoaded && mTaskBeingEdited)
        return;

    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);

    CGexMoTaskItem *    ptTask = NULL;

    // Before to save
    // check if some tasks was modified
    // if not, this is not necessary to reload all
    QDateTime clLastFileUpdate = QFileInfo(GS::Gex::Engine::GetInstance().GetTasksXmlFileName()).lastModified();

    // Build path to the 'Tasks' list.
    QFile file(GS::Gex::Engine::GetInstance().GetTasksXmlFileName());

    // Write the text to the file
    if (file.open(QIODevice::WriteOnly) == false) return; // Failed writing to tasks file.

    // Write Tasks definition File
    mLocalTasksFile.setDevice(&file);    // Assign file handle to data stream

    // For GexEmail.exe configuration
    // Save gex_email section if TaskStatus is uploaded and not present in Local tasks
    CGexMoTaskStatus* ptTaskStatus = GetStatusTask();
    if(ptTaskStatus != NULL)
    {
        // If already uploaded
        if(ptTaskStatus->IsUploaded())
        {
            mLocalTasksFile << "<gex_email>" << endl;
            SaveGexEmailSectionDirectory(ptTaskStatus);
            mLocalTasksFile << "</gex_email>" << endl;
        }
    }

    // Save tasks sections
    mLocalTasksFile << "<tasks>" << endl;

    lstIteratorTask.toFront();

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        // Save only local tasks
        if(ptTask->IsUploaded())
            continue;

        ptTask->m_clLastUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();

        ptTask->SaveTaskToXML(mLocalTasksFile);
    };

    mLocalTasksFile << "</tasks>" << endl;

    file.close();

    // Force to reload all tasks
    if(clLastFileUpdate.isNull() || mLastLocalTasksLoaded.secsTo(clLastFileUpdate) == 0)
        mLastLocalTasksLoaded = QFileInfo(GS::Gex::Engine::GetInstance().GetTasksXmlFileName()).lastModified();
    else
    {
        mLastLocalTasksLoaded = GS::Gex::Engine::GetInstance().GetClientDateTime().addYears(-1);
        mLastDbTasksUpdateCheckSum = 0;
    }

}

///////////////////////////////////////////////////////////
// Save Email directory to disk: Section Gex_email
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveGexEmailSectionDirectory(CGexMoTaskStatus *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoStatusTaskData *ptStatus = ptTask->GetProperties();

    QString strEmailDirectory = ptStatus->intranetPath();
    strEmailDirectory += "/";
    strEmailDirectory += GEXMO_AUTOREPORT_FOLDER;
    strEmailDirectory += "/";
    strEmailDirectory += GEXMO_AUTOREPORT_EMAILS;
    CGexSystemUtils::NormalizePath(strEmailDirectory);

    mLocalTasksFile << "EmailDirectory=" << strEmailDirectory << endl;
}


///////////////////////////////////////////////////////////
// Save Tasks to disk: Section DATA PUMP
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionDataPump(CGexMoTaskDataPump *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoDataPumpTaskData *ptDataPump = ptTask->GetProperties();

    mLocalTasksFile << "<task_data_pump>" << endl;
    mLocalTasksFile << "Title=" << ptDataPump->strTitle << endl;
    mLocalTasksFile << "Type=" << ptTask->GetTaskType() << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "DataPath=" << ptDataPump->strDataPath << endl;

    if(ptDataPump->bScanSubFolders)
        mLocalTasksFile << "ScanSubFolders=YES" << endl;
    else
        mLocalTasksFile << "ScanSubFolders=NO" << endl;

    mLocalTasksFile << "SortBy=" << ((ptDataPump->eSortFlags & QDir::Time) ? "Time" : "Name") << ((ptDataPump->eSortFlags & QDir::Reversed) ? "|Desc" : "") << endl;
    mLocalTasksFile << "ImportFileExtensions=" << ptDataPump->strImportFileExtensions << endl;
    if(!ptDataPump->strDatabaseTarget.isEmpty())
    {

        mLocalTasksFile << "Database=" << ptDataPump->strDatabaseTarget << endl;
        mLocalTasksFile << "DataType=" << QString::number(ptDataPump->uiDataType) << endl;
        mLocalTasksFile << "TestingStage=" << ptDataPump->strTestingStage << endl;
    }
    mLocalTasksFile << "Frequency=" << ptDataPump->iFrequency << endl;
    mLocalTasksFile << "DayOfWeek=" << ptDataPump->iDayOfWeek << endl;
    if(ptDataPump->bExecutionWindow)
        mLocalTasksFile << "ExecWindow=YES" << endl;
    else
        mLocalTasksFile << "ExecWindow=NO" << endl;

    mLocalTasksFile << "StartTime=" << ptDataPump->cStartTime.hour() << ",";
    mLocalTasksFile << ptDataPump->cStartTime.minute() << endl;
    mLocalTasksFile << "StopTime=" << ptDataPump->cStopTime.hour() << ",";
    mLocalTasksFile << ptDataPump->cStopTime.minute() << endl;
    mLocalTasksFile << "PostImport=" << ptDataPump->iPostImport<< endl;
    mLocalTasksFile << "PostImportFolder=" << ptDataPump->strPostImportMoveFolder << endl;
    mLocalTasksFile << "PostImportFailure=" << ptDataPump->iPostImportFailure<< endl;
    mLocalTasksFile << "PostImportFailureFolder=" << ptDataPump->strPostImportFailureMoveFolder << endl;
    mLocalTasksFile << "PostImportDelay=" << ptDataPump->iPostImportDelay<< endl;
    mLocalTasksFile << "PostImportDelayFolder=" << ptDataPump->strPostImportDelayMoveFolder << endl;

    if(ptDataPump->bCheckYield)
        mLocalTasksFile << "CheckYield=YES" << endl;
    else
        mLocalTasksFile << "CheckYield=NO" << endl;

    mLocalTasksFile << "YieldBins=" << ptDataPump->strYieldBinsList << endl;
    mLocalTasksFile << "AlarmLevel=" << ptDataPump->iAlarmLevel << endl;
    mLocalTasksFile << "MinimumParts=" << ptDataPump->lMinimumyieldParts << endl;
    mLocalTasksFile << "EmailFrom=" << ptDataPump->strEmailFrom << endl;
    mLocalTasksFile << "Emails=" << ptDataPump->strEmailNotify << endl;

    if(ptDataPump->bHtmlEmail)
        mLocalTasksFile << "EmailFormat=HTML" << endl;
    else
        mLocalTasksFile << "EmailFormat=TEXT" << endl;

    // Other options
    if(ptDataPump->bRejectSmallSplitlots_NbParts)
        mLocalTasksFile << "RejectSmallSplitlots_NbParts=" << ptDataPump->uiRejectSmallSplitlots_NbParts << endl;
    else
        mLocalTasksFile << "RejectSmallSplitlots_NbParts=-1" << endl;
    if(ptDataPump->bRejectSmallSplitlots_GdpwPercent)
        mLocalTasksFile << "RejectSmallSplitlots_Gdpw_Percent=" << QString::number(ptDataPump->lfRejectSmallSplitlots_GdpwPercent, 'f', 2) << endl;
    else
        mLocalTasksFile << "RejectSmallSplitlots_Gdpw_Percent=-1" << endl;
    if(ptDataPump->bExecuteBatchAfterInsertion)
        mLocalTasksFile << "ExecuteBatchAfterInsertion=YES" << endl;
    else
        mLocalTasksFile << "ExecuteBatchAfterInsertion=NO" << endl;
    mLocalTasksFile << "BatchToExecuteAfterInsertion=" << ptDataPump->strBatchToExecuteAfterInsertion << endl;
    mLocalTasksFile << "MaxPartsForTestResultInsertion=" << ptDataPump->nMaxPartsForTestResultInsertion << endl;
    if(ptDataPump->bRejectFilesOnPassBinlist)
        mLocalTasksFile << "RejectFilesOnPassBinlist=YES" << endl;
    else
        mLocalTasksFile << "RejectFilesOnPassBinlist=NO" << endl;
    mLocalTasksFile << "PassBinlistForRejectTest=" << ptDataPump->strPassBinListForRejectTest << endl;

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    // Let s invalid some attr that we dont want to save at all in xml
    ptTask->SetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName, QVariant());
    ptTask->SetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName, QVariant());
    ptTask->SetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName, QVariant());
    ptTask->SetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName, QVariant());

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_data_pump>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section YIELD MONITORING
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionYieldMonitoring(CGexMoTaskYieldMonitor *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    if(ptTask->GetProperties()->eSYA_Rule == eNone)
        mLocalTasksFile << "<task_yield_monitoring_rdb_YL>" << endl;
    else
        mLocalTasksFile << "<task_yield_monitoring_rdb_SYA>" << endl;

    SaveTaskSectionYieldMonitoring_OneRule(ptTask);

    if(ptTask->GetProperties()->eSYA_Rule == eNone)
        mLocalTasksFile << "</task_yield_monitoring_rdb_YL>" << endl;
    else
        mLocalTasksFile << "</task_yield_monitoring_rdb_SYA>" << endl;
}


///////////////////////////////////////////////////////////
// Save Tasks to disk: One rule from Section YIELD MONITORING RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionYieldMonitoring_OneRule(CGexMoTaskYieldMonitor *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoYieldMonitoringTaskData *ptYield = ptTask->GetProperties();

    mLocalTasksFile << "Title=" << ptYield->strTitle << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "ProductID=" << ptYield->strProductID << endl;
    mLocalTasksFile << "YieldBins=" << ptYield->strYieldBinsList << endl;
    mLocalTasksFile << "BiningType=" << ptYield->iBiningType << endl;
    mLocalTasksFile << "AlarmLevel=" << ptYield->iAlarmLevel << endl;
    mLocalTasksFile << "AlarmDirection=" << ptYield->iAlarmIfOverLimit << endl;
    mLocalTasksFile << "MinimumParts=" << ptYield->lMinimumyieldParts << endl;
    mLocalTasksFile << "EmailFrom=" << ptYield->strEmailFrom << endl;
    mLocalTasksFile << "Emails=" << ptYield->strEmailNotify << endl;

    if(ptYield->bHtmlEmail)
        mLocalTasksFile << "EmailFormat=HTML" << endl;
    else
        mLocalTasksFile << "EmailFormat=TEXT" << endl;

    mLocalTasksFile << "EmailReportType=" << ptYield->iEmailReportType << endl;
    mLocalTasksFile << "NotificationType=" << ptYield->iNotificationType << endl;
    mLocalTasksFile << "ExceptionLevel=" << ptYield->iExceptionLevel << endl;                            // Alarm type: Standard, Critical...

    /////////////////////////////////////////////////////////////////////////////
    // SYL-SBL specifics
    /////////////////////////////////////////////////////////////////////////////

    mLocalTasksFile << "ActiveOnDatafileInsertion=" << (ptYield->bSYA_active_on_datafile_insertion ? "1" : "0") << endl;
    mLocalTasksFile << "ActiveOnTriggerFile=" << (ptYield->bSYA_active_on_trigger_file ? "1" : "0") << endl;

    QMapIterator<int, QMap<QString, QVariant> > itBinRule(ptYield->mapBins_rules);
    int     nBinNo;
    int     nRuleType;
    QString strValues;
    while (itBinRule.hasNext())
    {
        itBinRule.next();
        nBinNo = itBinRule.key();
        nRuleType = itBinRule.value()["RuleType"].toInt();
        if(nRuleType == eManual)
        {
            strValues = itBinRule.value()["LL1"].toString()+"|";
            strValues+= itBinRule.value()["HL1"].toString()+"|";
            strValues+= itBinRule.value()["LL2"].toString()+"|";
            strValues+= itBinRule.value()["HL2"].toString();
        }
        else
        {
            strValues = itBinRule.value()["N1"].toString()+"|";
            strValues+= itBinRule.value()["N2"].toString();
        }
        mLocalTasksFile << "BinRule=" << QString("%1|%2|%3").arg(QString::number(nBinNo),QString::number(nRuleType),strValues) << endl;
    }

    mLocalTasksFile << "Database=" << ptYield->strDatabase << endl;
    mLocalTasksFile << "TestingStage=" << ptYield->strTestingStage << endl;
    mLocalTasksFile << "RuleType=" << QString::number(ptYield->eSYA_Rule) << endl;                       // Rule: 0=N*Sigma, 1=...
    mLocalTasksFile << "RuleTypeString=" << ptYield->strSYA_Rule << endl;                                // Rule string: Mean+/-N*Sigma, ...
    mLocalTasksFile << "N1_Parameter=" << QString::number(ptYield->fSYA_N1_value) << endl;               // N1 parameter
    mLocalTasksFile << "N2_Parameter=" << QString::number(ptYield->fSYA_N2_value) << endl;               // N1 parameter
    mLocalTasksFile << "MinimumLotsRequired=" << QString::number(ptYield->iSYA_LotsRequired) << endl;    // Minimum Total lots required for computing new SYL-SBL
    mLocalTasksFile << "ValidityPeriod=" << QString::number(ptYield->iSYA_Period) << endl;               // Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...
    mLocalTasksFile << "SBL1_LL_Disabled=" << ptYield->strSYA_SBL1_LL_Disabled << endl;                  // List of Binnings for which the SBL1 LL should be disabled
    mLocalTasksFile << "SBL1_HL_Disabled=" << ptYield->strSYA_SBL1_HL_Disabled << endl;                  // List of Binnings for which the SBL1 HL should be disabled
    mLocalTasksFile << "SBL2_LL_Disabled=" << ptYield->strSYA_SBL2_LL_Disabled << endl;                  // List of Binnings for which the SBL2 LL should be disabled
    mLocalTasksFile << "SBL2_HL_Disabled=" << ptYield->strSYA_SBL2_HL_Disabled << endl;                  // List of Binnings for which the SBL2 HL should be disabled
    mLocalTasksFile << "SYL1_LL_Disabled=" << (ptYield->bSYA_SYL1_LL_Disabled ? "1" : "0") << endl;      // True if SYL1 LL should be disabled
    mLocalTasksFile << "SYL1_HL_Disabled=" << (ptYield->bSYA_SYL1_HL_Disabled ? "1" : "0") << endl;      // True if SYL1 HL should be disabled
    mLocalTasksFile << "SYL2_LL_Disabled=" << (ptYield->bSYA_SYL2_LL_Disabled ? "1" : "0") << endl;      // True if SYL2 LL should be disabled
    mLocalTasksFile << "SYL2_HL_Disabled=" << (ptYield->bSYA_SYL2_HL_Disabled ? "1" : "0") << endl;      // True if SYL2 HL should be disabled
    mLocalTasksFile << "IgnoreDataPointsWithNullSigma=" << (ptYield->bSYA_IgnoreDataPointsWithNullSigma ? "1" : "0") << endl;    // Set to true if datapoints with null sigma should be ignored
    mLocalTasksFile << "IgnoreOutliers=" << (ptYield->bSYA_IgnoreOutliers ? "1" : "0") << endl;          // Set to true if outliers should be ignored
    mLocalTasksFile << "UseGrossDie=" << (ptYield->bSYA_UseGrossDie ? "1" : "0") << endl;                // Set to true if Gross Die should be used
    mLocalTasksFile << "MinDataPoints=" << QString::number(ptYield->iSYA_MinDataPoints) << endl;         // Minimum datapoints (wafers, lots if FT) to compute SYL/SBL

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section REPORTING
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionReporting(CGexMoTaskReporting *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoReportingTaskData *ptReporting = ptTask->GetProperties();

    mLocalTasksFile << "<task_reporting>" << endl;
    mLocalTasksFile << "Title=" << ptReporting->strTitle << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "ScriptPath=" << ptReporting->strScriptPath << endl;
    mLocalTasksFile << "Frequency=" << ptReporting->iFrequency << endl;
    mLocalTasksFile << "DayOfWeek=" << ptReporting->iDayOfWeek << endl;

    if(ptReporting->bExecutionWindow)
        mLocalTasksFile << "ExecWindow=YES" << endl;
    else
        mLocalTasksFile << "ExecWindow=NO" << endl;

    mLocalTasksFile << "StartTime=" << ptReporting->cStartTime.hour() << ",";
    mLocalTasksFile << ptReporting->cStartTime.minute() << endl;
    mLocalTasksFile << "StopTime=" << ptReporting->cStopTime.hour() << ",";
    mLocalTasksFile << ptReporting->cStopTime.minute() << endl;

    mLocalTasksFile << "NotificationType=" << ptReporting->iNotificationType << endl;
    mLocalTasksFile << "EmailFrom=" << ptReporting->strEmailFrom << endl;
    mLocalTasksFile << "Emails=" << ptReporting->strEmailNotify << endl;

    if(ptReporting->bHtmlEmail)
        mLocalTasksFile << "EmailFormat=HTML" << endl;
    else
        mLocalTasksFile << "EmailFormat=TEXT" << endl;

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_reporting>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section STATUS
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionStatus(CGexMoTaskStatus *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoStatusTaskData *ptStatus = ptTask->GetProperties();

    int iOneWebPerDatabase;

    mLocalTasksFile << "<task_status>" << endl;
    mLocalTasksFile << "Title=" << ptStatus->title() << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;

    if(ptStatus->isOneWebPerDatabase() == true)
        iOneWebPerDatabase = 1;
    else
        iOneWebPerDatabase = 0;
    mLocalTasksFile << "WebOrganization=" << QString::number(iOneWebPerDatabase) << endl;

    mLocalTasksFile << "IntranetPath=" << ptStatus->intranetPath() << endl;
    mLocalTasksFile << "HomePage=" << ptStatus->homePage() << endl;
    mLocalTasksFile << "ReportURL=" << ptStatus->reportURL() << endl;
    mLocalTasksFile << "ReportHttpURL=" << ptStatus->reportHttpURL() << endl;

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_status>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section FILE CONVERTER
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionFileConverter(CGexMoTaskConverter *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoFileConverterTaskData *ptConverter = ptTask->GetProperties();

    mLocalTasksFile << "<task_file_converter>" << endl;
    mLocalTasksFile << "Title=" << ptConverter->strTitle << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "InputFolder=" << ptConverter->strInputFolder << endl;
    mLocalTasksFile << "ImportFileExtensions=" << ptConverter->strFileExtensions << endl;
    mLocalTasksFile << "OutputFolder=" << ptConverter->strOutputFolder << endl;
    mLocalTasksFile << "OutputFormat=" << ptConverter->iFormat << endl;
    mLocalTasksFile << "TimeStampFile=" << (int) ptConverter->bTimeStampName << endl;
    mLocalTasksFile << "SuccessMode=" << ptConverter->iOnSuccess << endl;
    mLocalTasksFile << "SuccessFolder=" << ptConverter->strOutputSuccess << endl;
    mLocalTasksFile << "FailMode=" << ptConverter->iOnError << endl;
    mLocalTasksFile << "FailFolder=" << ptConverter->strOutputError << endl;
    mLocalTasksFile << "Frequency=" << ptConverter->iFrequency << endl;
    mLocalTasksFile << "DayOfWeek=" << ptConverter->iDayOfWeek << endl;

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_file_converter>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section OUTLIER REMOVAL
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionOutlierRemoval(CGexMoTaskOutlierRemoval *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoOutlierRemovalTaskData *ptOutlier = ptTask->GetProperties();

    mLocalTasksFile << "<task_outlier_monitoring>" << endl;
    mLocalTasksFile << "Title=" << ptOutlier->strTitle << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "Database=" << ptOutlier->strDatabase << endl;
    mLocalTasksFile << "TestingStage=" << ptOutlier->strTestingStage << endl;
    mLocalTasksFile << "ProductID=" << ptOutlier->strProductID << endl;
    mLocalTasksFile << "AlarmLevel=" << ptOutlier->lfAlarmLevel << endl;
    mLocalTasksFile << "AlarmType=" << ptOutlier->iAlarmType << endl;
    mLocalTasksFile << "MinimumParts=" << ptOutlier->lMinimumyieldParts << endl;
    int iValue;
    if(ptOutlier->bNotifyShapeChange)
        iValue = 1;
    else
        iValue = 0;
    mLocalTasksFile << "NotifyShapeChange=" << iValue << endl;

    mLocalTasksFile << "CompositeEtestAlarm=" << ptOutlier->lCompositeEtestAlarm << endl;
    mLocalTasksFile << "CompositeExclusionZoneAlarm=" << ptOutlier->lCompositeExclusionZoneAlarm << endl;
    mLocalTasksFile << "EmailFrom=" << ptOutlier->strEmailFrom << endl;
    mLocalTasksFile << "Emails=" << ptOutlier->strEmailNotify << endl;

    if(ptOutlier->bHtmlEmail)
        mLocalTasksFile << "EmailFormat=HTML" << endl;
    else
        mLocalTasksFile << "EmailFormat=TEXT" << endl;

    mLocalTasksFile << "EmailReportType=" << ptOutlier->iEmailReportType << endl;
    mLocalTasksFile << "NotificationType=" << ptOutlier->iNotificationType << endl;
    mLocalTasksFile << "ExceptionLevel=" << ptOutlier->iExceptionLevel << endl;  // Alarm type: Standard, Critical...

    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_outlier_monitoring>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section Auto Admin
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::SaveTaskSectionAutoAdmin(CGexMoTaskAutoAdmin *ptTask)
{
    if(ptTask == NULL)
        return;

    if(ptTask->GetProperties() == NULL)
        return;

    GexMoAutoAdminTaskData *ptAutoAdmin = ptTask->GetProperties();

    mLocalTasksFile << "<task_autoadmin>" << endl;
    mLocalTasksFile << "Title=" << ptAutoAdmin->mTitle << endl;
    mLocalTasksFile << "State=" << (ptTask->GetEnabledState() ? "1" : "0") << endl;
    mLocalTasksFile << "StartTime=" << ptAutoAdmin->mStartTime.hour() << ",";
    mLocalTasksFile << ptAutoAdmin->mStartTime.minute() << endl;
    mLocalTasksFile << "EmailFrom=" << ptAutoAdmin->mEmailFrom << endl;
    mLocalTasksFile << "Emails=" << ptAutoAdmin->mEmailNotify << endl;
    mLocalTasksFile << "SendInsertionLogs=" << ptAutoAdmin->mSendInsertionLogs << endl;
    mLocalTasksFile << "LastInsertionLogsSent=" << QDateTime(ptAutoAdmin->mLastInsertionLogsSent).toTime_t() << endl;
    mLocalTasksFile << "KeepReportDuration=" << ptAutoAdmin->mKeepReportDuration << endl;
    mLocalTasksFile << "LogContents=" << ptAutoAdmin->mLogContents << endl;
    mLocalTasksFile << "LastExecuted=" << ptTask->m_tLastExecuted << endl;

    ptTask->saveAttributesToTextStream(mLocalTasksFile);

    mLocalTasksFile << "</task_autoadmin>" << endl;
}

