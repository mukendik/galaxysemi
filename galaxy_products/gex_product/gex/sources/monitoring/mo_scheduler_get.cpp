///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "gexmo_constants.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "browser_dialog.h"
#include "pickproduct_id_dialog.h"
#include "cbinning.h"
#include "gqtl_datakeys.h"
#include "db_transactions.h"
#include "tb_toolbox.h"
#include "gex_group_of_files.h"
#include "product_info.h"
#include "engine.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "reporting/reporting_taskdata.h"
#include "spm/spm_task.h"
#include "sya/sya_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include "mo_email.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_taskdata.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "status/status_task.h"
#include "autoadmin/autoadmin_task.h"
#include "outlierremoval/outlierremoval_task.h"
#include "admin_engine.h"

extern QString workingFolder();
extern GexScriptEngine *pGexScriptEngine;

const QString GS::Gex::SchedulerEngine::sMonitoringLogsFolderKey="MonitoringLogsFolder";

QVariant GS::Gex::SchedulerEngine::Get(const QString &key)
{
    if (key==sMonitoringLogsFolderKey)
    {
        if (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        {
            // ToDo: create a const QString for "MONITORING_LOGS_FOLDER"
            QVariant lMLF=GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSetting("MONITORING_LOGS_FOLDER");
            if (!lMLF.isNull() && lMLF.isValid())
            {
                if (pGexScriptEngine)
                {
                    QVariant lVar=pGexScriptEngine->evaluate(lMLF.toString()).toVariant();
                    if (lVar.isValid() && !pGexScriptEngine->hasUncaughtException())
                    {
                        if (!lVar.toString().endsWith('/') || !lVar.toString().endsWith('\\'))
                            lVar=lVar.toString()+"/";
                        return lVar;
                    }
                }
            }
        }
        return GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+"/GalaxySemi/logs/";
    }
    //if (key=="zozo")
      //  return QVariant();
    return QVariant();
}


CGexMoTaskItem *GS::Gex::SchedulerEngine::GetTaskHandle(int iHandle)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem * ptTask = NULL;

    lstIteratorTask.toFront();

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        // Task can be in disable mode
        // Then ignore it
        if(!ptTask->IsUsable())
            continue;

        if(ptTask->GetTaskType() == iHandle)
            return ptTask;	// found it!
    };

    // Task not found!
    return NULL;
}
CGexMoTaskStatus *GS::Gex::SchedulerEngine::GetStatusTask(void)
{
    return (CGexMoTaskStatus*)GetTaskHandle(GEXMO_TASK_STATUS);
}

QString GS::Gex::SchedulerEngine::GetStatusTaskReportsDir()
{
    CGexMoTaskStatus *t = GetStatusTask();
    if (!t)
    {
        GSLOG(SYSLOG_SEV_ERROR, "GetStatusTaskReportsDir impossible because no Status Task found");
        return "";
    }
    QString d=t->GetProperties()->reportURL();

    if (d.isEmpty() || d == "default" || d == "(default)")
        d = workingFolder() + QDir::separator() + "reports";

    return d;
}

CGexMoTaskAutoAdmin *GS::Gex::SchedulerEngine::GetAutoAdminTask(void)
{
    return (CGexMoTaskAutoAdmin*)GetTaskHandle(GEXMO_TASK_AUTOADMIN);
}

bool GS::Gex::SchedulerEngine::LogTaskDetails()
{
    // Check if have to log all details!
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        CGexMoTaskAutoAdmin *ptTask = GetAutoAdminTask();
        if(ptTask
                && (ptTask->GetProperties()->mLogContents == GEXMO_AUTOADMIN_LOG_DETAILS))
            return true;
    }
    return false;
}

CGexMoTaskYieldMonitor *GS::Gex::SchedulerEngine::GetProductYieldInfo(
        GS::QtLib::DatakeysContent &dbKeysContent, bool bFindGoodBinList, CGexMoTaskItem *ptFromTask)
{
    return GetProductYieldInfo(dbKeysContent.Get("Product").toString(),
                               dbKeysContent.Get("DatabaseName").toString(),
                               dbKeysContent.Get("TestingStage").toString(),
                               bFindGoodBinList,
                               ptFromTask);
}

CGexMoTaskYieldMonitor *GS::Gex::SchedulerEngine::GetProductYieldInfo(
        QString ProductID,QString DatabaseName, QString TestingStage, bool bFindGoodBinList, CGexMoTaskItem *ptFromTask)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem *        ptTask = NULL;
    CGexMoTaskYieldMonitor* ptYieldTask = NULL;
    QString                 strPattern;
    QStringList             patternsList;
    QRegExp                 rx("", Qt::CaseInsensitive);	// NOT case sensitive.
    rx.setPatternSyntax(QRegExp::Wildcard);

    // If looking from 1st matching task
    if(ptFromTask == NULL)
        lstIteratorTask.toFront();
    else
        // Looking for next matching task
        lstIteratorTask.findNext(ptFromTask);

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        if(ptTask->GetTaskType() != GEXMO_TASK_YIELDMONITOR)
            continue;

        ptYieldTask = (CGexMoTaskYieldMonitor*)ptTask;

        // Task can be in disable mode
        // Then ignore it
        if(!ptYieldTask->IsUsable())
            continue;

        // Check concordance produit/testing_stage/database
        if(ptYieldTask->GetDatabaseName() != DatabaseName)
            continue;
        if(!ptYieldTask->GetProperties()->strTestingStage.isEmpty() // can be empty for FILE-BASED
                && (ptYieldTask->GetProperties()->strTestingStage != TestingStage))
            continue;

        // If looking for the Good Bins list only, check this first
        if(bFindGoodBinList && (ptYieldTask->GetProperties()->iBiningType != 0))
            continue;	// This Yield definition is NOT for Good bins...

        strPattern = ptYieldTask->GetProperties()->strProductID;

        // Replace ',' or ';' with '|' pipe OR character.
        strPattern.replace( ",", "|" );
        strPattern.replace( ";", "|" );
        patternsList = strPattern.split("|");
        for (int i = 0; i < patternsList.size(); ++i)
        {
            QString pattern = patternsList.at(i).simplified();
            if (!pattern.isEmpty())
            {
                rx.setPattern(pattern);
                if(rx.exactMatch(ProductID.simplified()))
                    return ptYieldTask;	// Found Yield task  for this ProductID
            }
        }
    };
    // Task not found!
    return NULL;
}

QList<CGexMoTaskStatisticalMonitoring*> GS::Gex::SchedulerEngine::GetProductStatMonTask(QtLib::DatakeysContent &dbKeysContent, CGexMoTaskItem *ptFromTask)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    QString lProductID      = dbKeysContent.Get("Product").toString();
    QString lDatabaseName   = dbKeysContent.Get("DatabaseName").toString();
    QString lTestingStage   = dbKeysContent.Get("TestingStage").toString();
    // During the insertion process, tasks are executed on insertion success
    // The InsertionStatus is set and always = "PASS"
    bool    lOnInsertion    = (dbKeysContent.Get("InsertionStatus").toString()=="PASS");
    // During a trigger process, a specific task can be required
    QString lTaskName       = dbKeysContent.Get("TaskName").toString();
    QString lTaskType       = dbKeysContent.Get("TaskType").toString();
    int lTaskTypeEnum;
    if(lTaskType == "SPM")
    {
        lTaskTypeEnum = GEXMO_TASK_SPM;
    }
    else if(lTaskType == "SYA")
    {
        lTaskTypeEnum = GEXMO_TASK_SYA;
    }
    else
    {
        return mEmptyTaskList;
    }

    QRegExp lRegExp("",Qt::CaseInsensitive,QRegExp::Wildcard);	// NOT case sensitive.
    CGexMoTaskItem *        ptTask = NULL;
    QList<CGexMoTaskStatisticalMonitoring*> lTasks;
    // If looking from 1st matching task
    if(ptFromTask == NULL)
    {
        lstIteratorTask.toFront();
    }
    else
    {
        // Looking for next matching task
        lstIteratorTask.findNext(ptFromTask);
    }

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        if(ptTask->GetTaskType() != lTaskTypeEnum)
        {
            continue;
        }

        // Task can be in disable mode
        // Then ignore it
        if(!ptTask->IsUsable())
        {
            continue;
        }

        if(!lTaskName.isEmpty() && (ptTask->GetName().toUpper()!=lTaskName.toUpper()))
        {
            continue;
        }

        CGexMoTaskStatisticalMonitoring* statMonTask = (CGexMoTaskStatisticalMonitoring*)ptTask;

        // Check concordance database
        if(statMonTask->GetDatabaseName() != lDatabaseName)
        {
            continue;
        }

        // Check concordance testing_stage
        if(statMonTask->GetProperties()->GetAttribute(C_TESTINGSTAGE).toString() != lTestingStage)
        {
            continue;
        }

        // Check if the task can be executed on Insertion
        if(lOnInsertion && !statMonTask->GetProperties()->GetAttribute(C_ACTIVEONINSERT).toBool())
        {
            continue;
        }

        // Or on Trigger
        if(!lOnInsertion && !statMonTask->GetProperties()->GetAttribute(C_ACTIVEONTRIGGER).toBool())
        {
            continue;
        }

        // Check if the product matchs
        QString lProductRegexp = statMonTask->GetProperties()->GetAttribute(C_PRODUCTREGEXP).toString();
        // Replace ',' or ';' with '|' pipe OR character.
        lProductRegexp.replace( ",", "|" ).replace( ";", "|" );

        // Return the first task that matchs the RegExp
        QStringList patternsList = lProductRegexp.split("|",QString::SkipEmptyParts);
        while(!patternsList.isEmpty())
        {
            lRegExp.setPattern(patternsList.takeFirst().simplified());
            if(lRegExp.exactMatch(lProductID))
            {
                lTasks << statMonTask; // Found task for this ProductID
            }
        }
    };
    // Task not found!
    if(lTasks.isEmpty())
    {
        return mEmptyTaskList;
    }

    QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIter(lTasks.begin());
    QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIterEnd(lTasks.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        (*lIter)->LoadTaskDetails();
    }

   return lTasks;
}

CGexMoTaskOutlierRemoval *GS::Gex::SchedulerEngine::GetProductOutlierInfo(QString szProductID, CGexMoTaskItem *ptFromTask)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);

    CGexMoTaskItem *	ptTask	= NULL;
    QString				strPattern;
    QStringList         patternsList;
    QRegExp				rx("", Qt::CaseInsensitive);	// NOT case sensitive.
    rx.setPatternSyntax(QRegExp::Wildcard);
    szProductID = szProductID.simplified();

    // If no Product defined, force it t '*'
    if(szProductID.isEmpty())
    {
        szProductID = "default";
    }

    // If looking from 1st matching task
    if(ptFromTask == NULL)
    {
        lstIteratorTask.toFront();
    }
    else
    {
        // Looking for next matching task
        lstIteratorTask.findNext(ptFromTask);
    }

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        // Task can be in disable mode
        // Then ignore it
        if(!ptTask->IsUsable())
            continue;

        switch(ptTask->GetTaskType())
        {
        case GEXMO_TASK_OUTLIER_REMOVAL:
            strPattern = ((CGexMoTaskOutlierRemoval*)ptTask)->GetProperties()->strProductID;
            // Replace ',' or ';' with '|' pipe OR character.
            strPattern.replace( ",", "|" );
            strPattern.replace( ";", "|" );

            patternsList = strPattern.split("|");
            for (int i = 0; i < patternsList.size(); ++i)
            {
                QString pattern = patternsList.at(i).simplified();
                if (!pattern.isEmpty())
                {
                    rx.setPattern(pattern);
                    if(rx.exactMatch(szProductID))
                        return (CGexMoTaskOutlierRemoval*)ptTask;	// Found task  for this ProductID
                }
            }
            break;
        default:
            break;
        }
    };
    // Task not found!
    return NULL;
}

void GS::Gex::SchedulerEngine::AppendMoHistoryLog(const QString &lLines,
                                                  const QString &lTaskType /*= ""*/,
                                                  const QString &lTitle /*= ""*/)
{
    QString			lCurrentLogFile;
    QDir			lDir;

    // Update Yield-Man history log
    lCurrentLogFile = Get(sMonitoringLogsFolderKey).toString();
    lDir.mkdir(lCurrentLogFile);
    lCurrentLogFile += GEXMO_LOG_FILE_ROOT;
    lCurrentLogFile += "_";
    lCurrentLogFile += QDate::currentDate().toString(Qt::ISODate);
    lCurrentLogFile += ".log";
    CGexSystemUtils::NormalizePath(lCurrentLogFile);

    QString		lMessage;

    // Add task type to message
    if(lTaskType.isEmpty() == false)
    {
        QDateTime cCurrentDateTime=GS::Gex::Engine::GetInstance().GetClientDateTime();
        lMessage += cCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
        lMessage += "Task: " + lTaskType + " - ";
        lMessage += lTitle ;

        if(!lMessage.endsWith("\n"))
            lMessage += "\n";
    }

    // Add lines to message
    if(lLines.isEmpty() == false)
    {
        lMessage += lLines;
        if(!lMessage.endsWith("\n"))
            lMessage += "\n";
    }

    // Maks sure message is not empty
    if(lMessage.isEmpty())
        return;

    // Make sure each message is separated by an empty line
    lMessage += "\n";

    QFile file(lCurrentLogFile); // Append text to the log file
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == false)
        return;	// Failed opening Log file.

    QTextStream stream(&file);
    // Write message to file
    stream << lMessage;

    // Close Log file.
    file.close();

    // Emit signal so that history gets reloaded into Admin tab
    emit sMoHistoryLogUpdated(lCurrentLogFile, lMessage);
}

void GS::Gex::SchedulerEngine::AppendMoReportLog(const QString &lStatus,
                                                 const QString &lFileName,
                                                 const QString &lCause,
                                                 const QString &lDataPump,
                                                 const QString &lDirectory,
                                                 unsigned int lFileSize,
                                                 unsigned int /*lOriginalSize*/,
                                                 unsigned int lTotalInsertionTime,
                                                 unsigned int /*lUncompressTime*/,
                                                 unsigned int /*lConvertTime*/)
{
    QString	lCurrentLogFile;

    // Update Yield-Man (not PAT-Man ?) report file and view
    lCurrentLogFile = Get(sMonitoringLogsFolderKey).toString();
    lCurrentLogFile += GEXMO_REPORT_FILE_ROOT;
    lCurrentLogFile += "_";
    lCurrentLogFile += QDate::currentDate().toString(Qt::ISODate);
    lCurrentLogFile += ".log";
    CGexSystemUtils::NormalizePath(lCurrentLogFile);

    QFile file(lCurrentLogFile); // Append text to the report file
    bool bWriteHeader=false;

    if(file.exists() == false)
        bWriteHeader = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == false)
        return;	// Failed opening Report file.

    QTextStream stream(&file);
    QString	lMessage;

    // Wtrite header?
    if(bWriteHeader)
        lMessage += "Date,Time,Status,FileName,Cause,DataPump,Directory,Size (Mb),Duration (s),Speed (Mb/s)\n";

    QString lFileSizeTxt = "n/a";
    QString lTotalInsertionTimeTxt = "n/a";
    QString lSpeedTxt = "n/a";
    if(lFileSize > 0)
        lFileSizeTxt = QString::number(float(lFileSize)/(1024.0F*1024.0F), 'f', 1);
    if(lTotalInsertionTime > 0)
        lTotalInsertionTimeTxt = QString::number(lTotalInsertionTime);
    if(lFileSize > 0 && lTotalInsertionTime > 0)
        lSpeedTxt = QString::number((float(lFileSize)/(1024.0F*1024.0F))/(float)lTotalInsertionTime);


    // Write new line
    lMessage += QDate::currentDate().toString(Qt::ISODate);
    lMessage += "," + QTime::currentTime().toString(Qt::ISODate);
    lMessage += "," + lStatus;
    lMessage += "," + lFileName;
    lMessage += "," + lCause;
    lMessage += "," + lDataPump;
    lMessage += "," + lDirectory;
    lMessage += "," + lFileSizeTxt;
    lMessage += "," + lTotalInsertionTimeTxt;
    lMessage += "," + lSpeedTxt;

    // Append message to file
    stream << lMessage << endl;

    // Close Log file.
    file.close();

    // Emit signal so that report gets reloaded into Admin tab
    emit sMoReportLogUpdated(lCurrentLogFile, lMessage);
}


QString GS::Gex::SchedulerEngine::GetBinSummaryString(
        CGexGroupOfFiles *pGroup,
        long lTotalBins,
        bool bHtmlFormat,
        bool bMultiSiteFile)
{
    GexMoBuildEmailString cEmailString;
    QString strBinSummary;
    CBinning *ptBinCell;
    QString		strTitle,strBin,strName,strPass,strTotal,strPercent;

    ptBinCell= pGroup->cMergedData.ptMergedSoftBinList;

    if(bHtmlFormat)
    {
        // Write table title.
        if(bMultiSiteFile)
            strTitle = "Software Bins Summary: " + pGroup->strGroupName;
        else
            strTitle = "Software Bins Summary";
        cEmailString.WriteTitle(strTitle);

        // Table with Lot Info
        cEmailString.WriteHtmlOpenTable();

        // Write table
        QStringList strList;
        strList << "Bin#" << "Bin Name" << "Pass/Fail" << "Total count" << "Percentage";
        cEmailString.WriteLabelLineList(strList);
    }
    else
    {
        if(bMultiSiteFile)
            strTitle = "\nSoftware Bins Summary: " + pGroup->strGroupName + "\n";
        else
            strTitle = "\nSoftware Bins Summary:\n";
        strBinSummary = strTitle;
        strBinSummary += "-----------------------------------------------------------------\n";
        strBinSummary += "Bin#    Bin Name            Pass/Fail   Total count    Percentage\n";
        strBinSummary += "------- ------------------- ----------- -------------- ----------\n";
    }

    while(ptBinCell != NULL)
    {
        // Label

        strBin.sprintf("%-7d ",ptBinCell->iBinValue);

        if(ptBinCell->strBinName.isEmpty())
            strName.sprintf("%-19s","-");
        else
            strName.sprintf("%-19s",ptBinCell->strBinName.toLatin1().constData());
        strName.truncate(19);

        strPass.sprintf("     %c       ",ptBinCell->cPassFail);
        strTotal.sprintf("%10d     ",ptBinCell->ldTotalCount);
        strPercent.sprintf("%.2f%%",(double)(100.0*ptBinCell->ldTotalCount)/(double)lTotalBins);

        if(ptBinCell->ldTotalCount > 0)
        {
            if(bHtmlFormat)
            {
                QStringList strList(strBin);
                strList << strName << strPass << strTotal << strPercent;
                cEmailString.WriteInfoLineList(strList);
            }
            else
                strBinSummary += strBin + strName + strPass + strTotal + strPercent + "\n";
        }

        // Go to next Bin entry
        ptBinCell = ptBinCell->ptNextBin;
    };

    if(bHtmlFormat)
    {
        // Close HTML Table
        cEmailString.WriteHtmlCloseTable();

        // Return full HTML string built.
        return cEmailString.ClosePage();
    }
    else
        return strBinSummary;
}

