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

#include "message.h"
#include "engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "datapump/datapump_task.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "sya_task.h"
#include "reporting/reporting_task.h"
#include "reporting/reporting_taskdata.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_task.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_task.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_task.h"
#include "converter/converter_taskdata.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "mo_task.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "mo_email.h"
#include "spm_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"

#include "admin_engine.h"
#include "admin_gui.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// report_build.cpp
extern CReportOptions   ReportOptions;      // Holds options (report_build.h)

// in main.cpp
extern void             WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *  pGexMainWindow;
extern GexScriptEngine* pGexScriptEngine;

#define TABLE_WIDGET_SHOW_ADMIN_OFFSET  false
#define TABLE_WIDGET_ADMIN_OFFSET 10

//////////////////////////////////////////////////////////////////////
// Make sure the qApplication->processEvent function is not called too often
//////////////////////////////////////////////////////////////////////
QTime   g_clLastProcessEventsTime;       // Time used to store last time ProcessEvents was executed
#define PROCESS_EVENTS_INTERVAL   500    // Min interval between processEvents calls (milliseconds)
void ProcessEvents(bool bForce)
{
    // Check interval
    if(bForce || (g_clLastProcessEventsTime.elapsed() > PROCESS_EVENTS_INTERVAL))
    {
        QCoreApplication::processEvents();
        g_clLastProcessEventsTime.start();
    }
}

// for optimization
// store the line where the task is stored in the view
// do not delete any line in the view ! (hide it)
QMap<int,int> gTasksRowInView;


///////////////////////////////////////////////////////////
// WIZARD: Scheduler tasks
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexMo_Tasks(void)
{
    // Show wizard page : select file.
    ShowWizardDialog(GEXMO_TASKS);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXMO_BROWSER_TASKS_LINK;
    AddNewUrl(strString);
}

///////////////////////////////////////////////////////////
// Shows message in message box...or print message to 'stderr' if in 'hide' running mode.
///////////////////////////////////////////////////////////
void SchedulerGui::PopupMessage(QString strMessage)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());

    if(pGexMainWindow == NULL)
        return;

    QMessageBox::information(pGexMainWindow,
                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             strMessage);
}

void SchedulerGui::UpdateListView(QStringList Options)
{
    bool EnableTable = true;
    if(Options.contains("Wait"))
        EnableTable = false;

    tableWidgetTasksListDataPump->setEnabled(EnableTable);
    tableWidgetTasksListTriggerSya->setEnabled(EnableTable);
    tableWidgetTasksListTriggerPat->setEnabled(EnableTable);
    tableWidgetTasksListReporting->setEnabled(EnableTable);
    tableWidgetTasksListConverter->setEnabled(EnableTable);
    tableWidgetTasksListOutlierRemoval->setEnabled(EnableTable);
    tableWidgetTasksListYieldLimits->setEnabled(EnableTable);
    tableWidgetTasksListStatus->setEnabled(EnableTable);
    tableWidgetTasksListAutoAdmin->setEnabled(EnableTable);
    tableWidgetTasksListSPM->setEnabled(EnableTable);
    tableWidgetTasksListSYA_2->setEnabled(EnableTable);

    if(Options.contains("Clear"))
    {
        UpdateListViewHeader(tableWidgetTasksListDataPump);
        UpdateListViewHeader(tableWidgetTasksListTriggerSya);
        UpdateListViewHeader(tableWidgetTasksListTriggerPat);
        UpdateListViewHeader(tableWidgetTasksListReporting);
        UpdateListViewHeader(tableWidgetTasksListConverter);
        UpdateListViewHeader(tableWidgetTasksListOutlierRemoval);
        UpdateListViewHeader(tableWidgetTasksListYieldLimits);
        UpdateListViewHeader(tableWidgetTasksListStatus);
        UpdateListViewHeader(tableWidgetTasksListAutoAdmin);
        UpdateListViewHeader(tableWidgetTasksListSPM);
        UpdateListViewHeader(tableWidgetTasksListSYA_2);
    }

    ProcessEvents(true);

    if(Options.contains("Load"))
    {
        int TotalTasks = GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList().count();

        if(TotalTasks > 0)
        {
            textLabelScheduler->setText(QString::number(TotalTasks)+" tasks loaded");

            CGexMoTaskItem* ptTaskItem = NULL;
            QListIterator<CGexMoTaskItem*> lstIteratorTask(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList());
            QString lOption;
            if(Options.contains("Force"))
                lOption = "Force";
            lstIteratorTask.toFront();
            int TotalInfo = 0;
            int DebugInfo = 0;
            QTime timer;
            timer.start();
            while(lstIteratorTask.hasNext())
            {
                ptTaskItem = lstIteratorTask.next();

                if(ptTaskItem == NULL)
                    continue;

                // Add task to list and force to Update all tables with the first 15 rows
                UpdateListViewItem(ptTaskItem, lOption);
                TotalInfo++;
                ++DebugInfo;
                if(((DebugInfo>77) || (timer.elapsed()>500))
                        && (DebugInfo*2 < TotalTasks))
                {
                    DebugInfo = 0;
                    timer.start();
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(
                                "Updating view... "+QString::number(TotalInfo)+" tasks");
                }
            }
        }

        if(TotalTasks == 0)
            textLabelScheduler->setText("");
        else
            textLabelScheduler->setText(QString::number(TotalTasks)+" tasks loaded");
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("");
    }
}

///////////////////////////////////////////////////////////
// Update List View item: Data Pump
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewHeader(QTableWidget *ptTable)
{
    if(m_lstTableGenericLabels.isEmpty())
    {
        // Add Admin field
        m_lstTableGenericLabels << "TaskId" << "RowUpdate" << "StatusUpdate" << "LastUpdate" << "UserId" << "GroupId" << "DatabaseId" << "iPermissions" << "CreationDate" << "ExpirationDate" << "Enable";
        // Add Status icon field
        m_lstTableGenericLabels << "Owner" << "   " << "   " << "   " << "Info";
    }

    QStringList lstTableLabels = m_lstTableGenericLabels;

    if(ptTable == tableWidgetTasksListDataPump)
    {
        lstTableLabels << "Title" << "Database" << "Data Source" << "Data Type" << "Priority" << "Yield Check" << "Alarm condition" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListTriggerSya)
    {
        lstTableLabels << "Title" << "Data Source" << "Data Type" << "Priority" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListTriggerPat)
    {
        lstTableLabels << "Title" << "Data Source" << "Data Type" << "Priority" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListReporting)
    {
        lstTableLabels << "Title" << "Script" << "Frequency" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListConverter)
    {
        lstTableLabels << "Title" << "Data Source" << "Format" << "Priority";
    }
    else if(ptTable == tableWidgetTasksListOutlierRemoval)
    {
        lstTableLabels << "Title" << "Database" << "Data Type" << "Product" << "Alarm condition" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListYieldLimits)
    {
        lstTableLabels << "Title" << "Database" << "Data Type" << "Product" << "MinimumParts" << "Alarm condition" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListStatus)
    {
        lstTableLabels << "Title" << "Web site organization" << "Intranet web file" << "Reports path" << "URL";
    }
    else if(ptTable == tableWidgetTasksListAutoAdmin)
    {
        lstTableLabels << "Title" << "Frequency" << "Alarm condition" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListSPM)
    {
        lstTableLabels << "Title" << "ExpirationDate" << "Database" << "Data Type" << "Product" << "Stats" << "MinimumLots" << "Filtering Algorithm" << "N1(Standard)" << "N2(Critical)" << "Emails";
    }
    else if(ptTable == tableWidgetTasksListSYA_2)
    {
        lstTableLabels << "Title" << "ExpirationDate" << "Database" << "Data Type" << "Product" << "MinimumLots" << "Filtering Algorithm" << "N1(Standard)" << "N2(Critical)" << "Emails";
    }

    ptTable->clear();

    ptTable->setRowCount(0);

    ptTable->setColumnCount(lstTableLabels.count());

    ptTable->setHorizontalHeaderLabels(lstTableLabels);
    ptTable->verticalHeader()->setVisible(false);
    ptTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ptTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    int nIndex = 0;
    // adjust original size
    for(nIndex=TABLE_WIDGET_ADMIN_OFFSET ; nIndex<lstTableLabels.count() ; nIndex++)
    {
        ptTable->setColumnWidth(nIndex,QString(lstTableLabels.at(nIndex)).length()*7);
    }

    // Hide admin column
    if(TABLE_WIDGET_SHOW_ADMIN_OFFSET)
    {
        for(nIndex = 0;nIndex<=TABLE_WIDGET_ADMIN_OFFSET;nIndex++)
            ptTable->showColumn(nIndex);
    }
    else
    {
        for(nIndex = 0;nIndex<=TABLE_WIDGET_ADMIN_OFFSET;nIndex++)
            ptTable->hideColumn(nIndex);
    }
    nIndex = TABLE_WIDGET_ADMIN_OFFSET;

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        ptTable->hideColumn(nIndex+1);  // Owner
        ptTable->hideColumn(nIndex+3);  // Locked
    }

    // Always hide lock column
    ptTable->hideColumn(nIndex+3);
    gTasksRowInView.clear();
}

///////////////////////////////////////////////////////////
// Update List View item: Data Pump
// options :
//      "force": use to load the first 15 tasks in each table
//      "delete": use to delete a specific line referenced by the TaskID
//      "<int>": use to update a specific line referenced by <int> when TaskId was changed
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewItem(CGexMoTaskItem *ptTaskData,QString options)
{
    if(ptTaskData == NULL)
        return;

    bool bForceView = false;
    bool bDeleteTask = false;
    bool bReplaceTask = false;
    int  TaskId = 0;

    bForceView = (options.toLower() == "force");
    bDeleteTask = (options.toLower() == "delete");
    TaskId = options.toInt(&bReplaceTask);

    if(!bReplaceTask)
        TaskId = ptTaskData->GetID();

    bool bIsEditabled = false;
    QTableWidget *ptTable = NULL;
    QWidget * pCurrentTab = tabWidget->currentWidget();
    QWidget * pWidget = NULL;


    ptTable = GetTableWidgetForTaskType(ptTaskData->GetTaskType());
    pWidget = GetTabWidgetForTaskType(ptTaskData->GetTaskType());

    if((ptTable == NULL) || (pWidget == NULL))
        return;

    ptTable->setEnabled(true);

    bool bVisible = (pWidget == pCurrentTab);

    // Only update the GUI-tab when it is visible
    // The widget is refresh after each tab selection
    // if bForceView => only force for the 10 first rows
    if(!bDeleteTask
            && !bReplaceTask
            && GS::Gex::Engine::GetInstance().GetSchedulerEngine().isTasksLoaded()
            && !bVisible
            && (!bForceView || (bForceView && (ptTable->rowCount() > 15))))
        return;

    int nCurrentRow = -1;
    bool bUpdateColumnSize = (ptTable->rowCount() == 0);

    bool bCreateTask = false;
    QTableWidgetItem *ptItem = NULL;

    // Find the good row
    // check if the task already referenced
    if(gTasksRowInView.contains(TaskId))
    {
        nCurrentRow = gTasksRowInView[TaskId];
        ptItem = ptTable->item(nCurrentRow,0);
        if(ptItem && (ptItem->text().toInt() == TaskId))
        {
            // found
        }
        else
        {
            // User can have reorganize the order by view
            // Find the good line
            for(nCurrentRow=0; nCurrentRow<ptTable->rowCount(); nCurrentRow++)
            {
                if(ptTable->isRowHidden(nCurrentRow))
                    continue;

                ptItem = ptTable->item(nCurrentRow,0);
                if(ptItem && (ptItem->text().toInt() == TaskId))
                    break;
                ptItem = NULL;
            }
        }
    }

    if(ptItem == NULL)
    {
        // New line
        nCurrentRow = -1;
    }


    if(nCurrentRow == ptTable->rowCount())
        nCurrentRow = -1;

    bCreateTask = (nCurrentRow < 0);
    // If task not exist and have to delete it
    // nothing to do
    if(bCreateTask && bDeleteTask)
        return;

    // If task not exist and have to replace it
    // nothing to do
    if(bCreateTask && bReplaceTask)
        return;

    //  If have to delete this task
    if(bDeleteTask)
    {
        // hide line
        ptTable->hideRow(nCurrentRow);
        gTasksRowInView.remove(TaskId);
        return;
    }

    if(bReplaceTask)
        gTasksRowInView.remove(TaskId);

    if(bCreateTask)
    {

        // Insert new item into ListView
        nCurrentRow = ptTable->rowCount();
        ptTable->setRowCount(ptTable->rowCount()+1);
        ptTable->setRowHeight(nCurrentRow, 20);

        // Fix freeze
        if(nCurrentRow < 10)
            ProcessEvents(true);
    }

    if(nCurrentRow < 0)
        return;

    gTasksRowInView[ptTaskData->GetID()] = nCurrentRow;

    // Fix freeze
    ProcessEvents(false);

    int nIndex = 0;
    // Update Admin fields if needed
    ptItem = ptTable->item(nCurrentRow,1);
    if(ptItem)
    {
        QString strValue = ptItem->text();
        QDateTime LastRowUpdate = QDateTime::fromString(strValue,Qt::ISODate);

        // hide lines where the user is not the owner
        // TASKS NOT LOADING - NOTHING TO DO
        if((LastRowUpdate >= ptTaskData->m_LastStatusUpdate)
                && (LastRowUpdate >= ptTaskData->m_clLastUpdate))
            return;

        ptTable->showRow(nCurrentRow);
    }

    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetID());
    fillCellData(ptTable,nCurrentRow,nIndex++,GS::Gex::Engine::GetInstance().GetServerDateTime());
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_LastStatusUpdate);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_clLastUpdate);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_iUserId);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_iGroupId);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_iDatabaseId);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_iPermissions);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_clCreationDate);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_clExpirationDate);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetEnabledState());

    QString strOwner;
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(ptTaskData->m_iUserId))
        strOwner = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[ptTaskData->m_iUserId]->m_strLogin;
    fillCellData(ptTable,nCurrentRow,nIndex++,strOwner);

    // Update Location icon
    QIcon   clIconLocation(QPixmap(QString::fromUtf8(":/gex/icons/task.png")));
    QString strInfoMsg = "Local tasks";
    if(ptTaskData->IsUploaded())
    {
        clIconLocation.addPixmap(QString::fromUtf8(":/gex/icons/task-share.png"));
        strInfoMsg = "Task referenced in YieldManDb";
    }
    fillCellData(ptTable,nCurrentRow,nIndex++,clIconLocation,strInfoMsg);

    // Update Locked icon
    QIcon   clIconLock;
    strInfoMsg = "Task not locked";
    if((ptTaskData->IsUploaded()) && (ptTaskData->m_iPermissions == 0))
    {
        clIconLock.addPixmap(QString::fromUtf8(":/gex/icons/lock.png"));
        strInfoMsg = "Task locked in YieldManDb";
    }
    fillCellData(ptTable,nCurrentRow,nIndex++,clIconLock,strInfoMsg);

    // Update Status icon
    // Update Info column
    QString strTaskInfo;
    QIcon   clIconEnabled(QPixmap(QString::fromUtf8(":/gex/icons/enable.png")));
    strInfoMsg = "Ready";
    strTaskInfo = "";
    if(!ptTaskData->GetEnabledState())
    {
        clIconEnabled.addPixmap(QString::fromUtf8(":/gex/icons/disable.png"));
        strInfoMsg = strTaskInfo = "Disabled";
    }

    if(ptTaskData->m_iStatus == MO_TASK_STATUS_ERROR)
        clIconEnabled.addPixmap(QString::fromUtf8(":/gex/icons/error.png"));
    if(ptTaskData->m_iStatus == MO_TASK_STATUS_WARNING)
        clIconEnabled.addPixmap(QString::fromUtf8(":/gex/icons/warning.png"));

    strInfoMsg = ptTaskData->m_strLastInfoMsg;
    if(strInfoMsg.contains("["))
        strTaskInfo = strInfoMsg.section("[",1).section("]",0,0);
    else
        strTaskInfo = strInfoMsg.section(".",0,0).remove("This ");

    fillCellData(ptTable,nCurrentRow,nIndex++,clIconEnabled,strInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strTaskInfo,strInfoMsg);

    ptTaskData->UpdateListView(this, nCurrentRow, bIsEditabled);

    // Adjust the size of the column
    if(bUpdateColumnSize)
    {
        for(int iCol=TABLE_WIDGET_ADMIN_OFFSET; iCol<ptTable->columnCount(); iCol++)
        {
            ptTable->resizeColumnToContents(iCol);
            if(ptTable->columnWidth(iCol) > 350)
                ptTable->setColumnWidth(iCol,350);
        }
    }

    // Set font color
    QColor rgbColor = QColor(0,0,0);
    if(!ptTaskData->GetEnabledState() && (ptTaskData->m_iStatus != MO_TASK_STATUS_ERROR))
        rgbColor = QColor(180,180,180);

    int lColumns = ptTable->columnCount();
    for(int iCol=TABLE_WIDGET_ADMIN_OFFSET; iCol<lColumns; iCol++)
    {
        ptItem = ptTable->item(nCurrentRow,iCol);
        // Do not update color if already at the good one
        if(ptItem)
        {
            if(ptItem->textColor() == rgbColor)
                break;
            ptItem->setTextColor(rgbColor);
        }
    }

    // Select empty entry in the list.
    ptTable->setCurrentCell(ptTable->rowCount(),0);
    ptTable->showRow(nCurrentRow);
}

///////////////////////////////////////////////////////////
// Update List View item: Data Pump
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewDataPumpItem(CGexMoTaskDataPump* ptTaskData,
                                              int nCurrentRow,
                                              bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    if(ptTaskData->GetTaskType() == GEXMO_TASK_DATAPUMP)
        ptTable = tableWidgetTasksListDataPump;
    else if(ptTaskData->GetTaskType() == GEXMO_TASK_TRIGGERPUMP)
        ptTable = tableWidgetTasksListTriggerSya;
    else if(ptTaskData->GetTaskType() == GEXMO_TASK_PATPUMP)
        ptTable = tableWidgetTasksListTriggerPat;
    else if(ptTaskData->GetTaskType() == GEXMO_TASK_OLD_DATAPUMP)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            ptTable = tableWidgetTasksListTriggerPat;
        else
            ptTable = tableWidgetTasksListDataPump;
    }


    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strDataType = "Test Data";
    QString strAlarmCondition;
    QString strEmails;
    QString strPriority;
    strPriority = ptTaskData->GetPriorityLabel();
    if(strPriority.isEmpty())
        strPriority = ptTaskData->GetFrequencyLabel();

    if(ptTaskData->GetProperties()->bCheckYield == false)
    {
        strAlarmCondition = strEmails = " -";
    }
    else
    {
        strAlarmCondition = "Yield < ";
        strAlarmCondition += QString::number(ptTaskData->GetProperties()->iAlarmLevel);
        strAlarmCondition += "%";
        strEmails = ptTaskData->GetProperties()->strEmailNotify;
    }
    if(ptTaskData->GetProperties()->uiDataType == GEX_DATAPUMP_DATATYPE_WYR)
        strDataType = "Weekly Yield Report on "+ ptTaskData->GetProperties()->strTestingStage;
    if(ptTaskData->GetTaskType() != GEXMO_TASK_DATAPUMP)
        strDataType = "Trigger file";

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    // For DataPump
    // "Title" << "Database" << "Data Source" << "Data Type" << "Frequency" << "Yield Check" << "Alarm condiction" << "Emails";
    // Else
    // "Title" << "Data Source" << "Data Type" << "Frequency" << "Emails";
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strTitle,ptTaskData->m_strLastInfoMsg);
    if(ptTable == tableWidgetTasksListDataPump)
        fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_strDatabaseName,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strDataPath,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strDataType,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strPriority,ptTaskData->m_strLastInfoMsg);
    if(ptTable == tableWidgetTasksListDataPump)
        fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->bCheckYield,ptTaskData->m_strLastInfoMsg);
    if(ptTable == tableWidgetTasksListDataPump)
        fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strEmails,ptTaskData->m_strLastInfoMsg);

}

///////////////////////////////////////////////////////////
// Update List View item: Yield Monitoring
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewYieldItem(CGexMoTaskYieldMonitor* ptTaskData,
                                           int nCurrentRow,
                                           bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListYieldLimits;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strAlarmCondition;
    QString strEmails;

    QString checkType=ptTaskData->GetAttribute("CheckType").toString();

    if (checkType=="FixedYieldTreshold")
    {
        strAlarmCondition = "Yield ";
        if(ptTaskData->GetProperties()->iAlarmIfOverLimit == 0)
            strAlarmCondition += "< ";  // Alarm if Yield UNDER Limit
        else
            strAlarmCondition += "> ";  // Alarm if Yield OVER Limit
        strAlarmCondition += QString::number(ptTaskData->GetProperties()->iAlarmLevel);
        strAlarmCondition += "%";
    }
    else if (checkType=="BinsPerSite")
    {
        strAlarmCondition = "Bins percent delta per site";
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Update list view yield item : BinsPerSite");
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "Update list view yield item : unknown check type");
    }

    strEmails = ptTaskData->GetProperties()->strEmailNotify;

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    //"Title" << "Database" << "Data Type" << "Product" << "MinimumParts" << "Alarm condiction" << "Emails"
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->strTitle,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->m_strDatabaseName,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->strTestingStage,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->strProductID, ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 (int)ptTaskData->GetProperties()->lMinimumyieldParts,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 strAlarmCondition,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strEmails,ptTaskData->m_strLastInfoMsg);
}

///////////////////////////////////////////////////////////
// Update List View item: SYA
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewSYA(CGexMoTaskSYA* ptTaskData,
                                               int nCurrentRow,
                                               bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListSYA_2;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    int nIndex = m_lstTableGenericLabels.count();

    // "Title" << "ExpirationDate" << "TDR" << "Data Type" << "Product" << "Stats" << "MinimumLots" << "Filtering Algorithm" << "N1(Standard)" << "N2(Critical)" << "Emails";
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_TITLE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 QDateTime(ptTaskData->GetProperties()->GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE).toDate()),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_DATABASE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_TESTINGSTAGE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_PRODUCTREGEXP).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_MINLOTS).toInt(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_DEFAULTALGO).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_N1).toFloat(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_N2).toFloat(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_EMAILTO).toString(),ptTaskData->m_strLastInfoMsg);
}


///////////////////////////////////////////////////////////
// Update List View item: SPM
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewSPM(CGexMoTaskSPM* ptTaskData,
                                               int nCurrentRow,
                                               bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListSPM;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    int nIndex = m_lstTableGenericLabels.count();

    // "Title" << "ExpirationDate" << "Database" << "Data Type" << "Product" << "Stats" << "MinimumLots" << "Filtering Algorithm" << "N1(Standard)" << "N2(Critical)" << "Emails";
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_TITLE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 QDateTime(ptTaskData->GetProperties()->GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE).toDate()),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_DATABASE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_TESTINGSTAGE).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_PRODUCTREGEXP).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_STATSTOMONITOR).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_MINLOTS).toInt(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_DEFAULTALGO).toString(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_N1).toFloat(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_N2).toFloat(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,
                 ptTaskData->GetProperties()->GetAttribute(C_EMAILTO).toString(),ptTaskData->m_strLastInfoMsg);
}

///////////////////////////////////////////////////////////
// Update List View item: Reporting
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewReportingItem(CGexMoTaskReporting* ptTaskData,
                                               int nCurrentRow,
                                               bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListReporting;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strEmails = ptTaskData->GetProperties()->strEmailNotify;
    QString strFrequency = ptTaskData->GetFrequencyLabel();

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    //"Title" << "Script" << "Frequency" << "Emails";
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strTitle,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strScriptPath,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strEmails,ptTaskData->m_strLastInfoMsg);
}

///////////////////////////////////////////////////////////
// Update List View item: Status
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewStatusItem(CGexMoTaskStatus* ptTaskData,
                                            int nCurrentRow,
                                            bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListStatus;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strOrganization = " -";
    strOrganization = "One Web site from all databases";
    if(!ptTaskData->GetProperties()->isOneWebPerDatabase())
        strOrganization = "One child Web site per database";

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    //"Title" << "Web site organization" << "Intranet web file" << "Reports path" << "URL"
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->title(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strOrganization,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->homePage(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->reportURL(),ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->reportHttpURL(),ptTaskData->m_strLastInfoMsg);

}

///////////////////////////////////////////////////////////
// Update List View item: File Converter
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewConverterItem(CGexMoTaskConverter* ptTaskData,
                                               int nCurrentRow,
                                               bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListConverter;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strFormat = "CSV";
    QString strPriority;
    strPriority = ptTaskData->GetPriorityLabel();
    if(strPriority.isEmpty())
        strPriority = ptTaskData->GetFrequencyLabel();

    if(ptTaskData->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_STDF)
        strFormat = "STDF";
    if(ptTaskData->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_ATDF)
        strFormat = "ATDF";

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    // "Title" << "Data Source" << "Format" << "Frequency"
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strTitle,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strInputFolder,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strFormat,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strPriority,ptTaskData->m_strLastInfoMsg);
}

///////////////////////////////////////////////////////////
// Update List View item: PAT-Man (Outlier Removal)
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewOutlierItem(CGexMoTaskOutlierRemoval* ptTaskData,
                                             int nCurrentRow,
                                             bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListOutlierRemoval;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strAlarmCondition;
    QString strEmails;

    strAlarmCondition = "Yield loss ";
    strAlarmCondition += "> ";  // Alarm if Yield loss OVER limit
    strAlarmCondition += QString::number(ptTaskData->GetProperties()->lfAlarmLevel);
    if(ptTaskData->GetProperties()->iAlarmType == 0)
        strAlarmCondition += "%";
    else
        strAlarmCondition += " parts";
    strEmails = ptTaskData->GetProperties()->strEmailNotify;

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses
    //"Title" << "Database" << "Data Type"<< "Product" << "Alarm condiction" << "Emails"
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strTitle,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->m_strDatabaseName,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strTestingStage,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->strProductID,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strEmails,ptTaskData->m_strLastInfoMsg);
}

///////////////////////////////////////////////////////////
// Update List View item: AutoAdmin
///////////////////////////////////////////////////////////
void SchedulerGui::UpdateListViewAutoAdminItem(CGexMoTaskAutoAdmin* ptTaskData,
                                               int nCurrentRow,
                                               bool /*bIsEditabled*/)
{
    QTableWidget *ptTable = NULL;

    ptTable = tableWidgetTasksListAutoAdmin;

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    if(nCurrentRow < 0)
        return;

    if(ptTaskData->GetProperties() == NULL)
        return;

    QString strFrequency = "Daily - ";
    QString strAlarmCondition = "Exceptions only";
    QString strEmails = ptTaskData->GetProperties()->mEmailNotify;

    strFrequency += ptTaskData->GetProperties()->mStartTime.toString();
    if(ptTaskData->GetProperties()->mLogContents == GEXMO_AUTOADMIN_LOG_DETAILS)
        strAlarmCondition = "All tasks details";

    int nIndex = m_lstTableGenericLabels.count();
    // Add entry: title, task type, Frequency, Alarm condition, email addresses

    fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->GetProperties()->mTitle,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition,ptTaskData->m_strLastInfoMsg);
    fillCellData(ptTable,nCurrentRow,nIndex++,strEmails,ptTaskData->m_strLastInfoMsg);
}



///////////////////////////////////////////////////////////
// Get from List View item
///////////////////////////////////////////////////////////
// GUI DEPENDENT
QTableWidget * SchedulerGui::GetTableWidgetForTaskType(int TaskType)
{
    QTableWidget *ptTable;
    ptTable = NULL;

    switch(TaskType)
    {
    case GEXMO_TASK_OLD_DATAPUMP:
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        {
            ptTable = tableWidgetTasksListTriggerPat;
        }
        else
        {
            ptTable = tableWidgetTasksListDataPump;
        }
        break;
    case GEXMO_TASK_DATAPUMP:
        ptTable = tableWidgetTasksListDataPump;
        break;
    case GEXMO_TASK_TRIGGERPUMP:
        ptTable = tableWidgetTasksListTriggerSya;
        break;
    case GEXMO_TASK_PATPUMP:
        ptTable = tableWidgetTasksListTriggerPat;
        break;
    case GEXMO_TASK_YIELDMONITOR:
        ptTable = tableWidgetTasksListYieldLimits;
        break;
    case GEXMO_TASK_SPM:
        ptTable = tableWidgetTasksListSPM;
        break;
    case GEXMO_TASK_SYA:
        ptTable = tableWidgetTasksListSYA_2;
        break;
    case GEXMO_TASK_REPORTING:
        ptTable = tableWidgetTasksListReporting;
        break;
    case GEXMO_TASK_STATUS:
        ptTable = tableWidgetTasksListStatus;
        break;
    case GEXMO_TASK_CONVERTER:
        ptTable = tableWidgetTasksListConverter;
        break;
    case GEXMO_TASK_OUTLIER_REMOVAL:
        ptTable = tableWidgetTasksListOutlierRemoval;
        break;
    case GEXMO_TASK_AUTOADMIN:
        ptTable = tableWidgetTasksListAutoAdmin;
        break;
    }

    return ptTable;
}

// GUI DEPENDENT
QWidget * SchedulerGui::GetTabWidgetForTaskType(int TaskType)
{
    QWidget *pWidget = NULL;

    switch(TaskType)
    {
    case GEXMO_TASK_OLD_DATAPUMP:
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        {
            pWidget = tabTasksTriggerPat;
        }
        else
        {
            pWidget = tabTasksDataPump;
        }
        break;
    case GEXMO_TASK_DATAPUMP:
        pWidget = tabTasksDataPump;
        break;
    case GEXMO_TASK_TRIGGERPUMP:
        pWidget = tabTasksTriggerSya;
        break;
    case GEXMO_TASK_PATPUMP:
        pWidget = tabTasksTriggerPat;
        break;
    case GEXMO_TASK_YIELDMONITOR:
        pWidget = tabTasksYield;
        break;
    case GEXMO_TASK_SPM:
        pWidget = tabTasksSpm;
        break;
    case GEXMO_TASK_SYA:
        pWidget = tabTasksSya;
        break;
    case GEXMO_TASK_REPORTING:
        pWidget = tabTasksReporting;
        break;
    case GEXMO_TASK_STATUS:
        pWidget = tabTasksStatus;
        break;
    case GEXMO_TASK_CONVERTER:
        pWidget = tabTasksConverter;
        break;
    case GEXMO_TASK_OUTLIER_REMOVAL:
        pWidget = tabTasksOutlierRemoval;
        break;
    case GEXMO_TASK_AUTOADMIN:
        pWidget = tabTasksAutoAdmin;
        break;
    }

    return pWidget;
}


// GUI DEPENDENT
QTableWidget * SchedulerGui::GetCurrentTableWidget()
{
    QWidget     *pCurrentTab = tabWidget->currentWidget();
    QTableWidget*ptTable = NULL;

    // If no item in list, just return!
    if(pCurrentTab == NULL)
        return NULL;

    if(pCurrentTab == tabTasksDataPump)
        ptTable = tableWidgetTasksListDataPump;
    else if(pCurrentTab == tabTasksTriggerSya)
        ptTable = tableWidgetTasksListTriggerSya;
    else if(pCurrentTab == tabTasksTriggerPat)
        ptTable = tableWidgetTasksListTriggerPat;
    else if(pCurrentTab == tabTasksReporting)
        ptTable = tableWidgetTasksListReporting;
    else if(pCurrentTab == tabTasksConverter)
        ptTable = tableWidgetTasksListConverter;
    else if(pCurrentTab == tabTasksOutlierRemoval)
        ptTable = tableWidgetTasksListOutlierRemoval;
    else if(pCurrentTab == tabTasksYield)
        ptTable = tableWidgetTasksListYieldLimits;
    else if(pCurrentTab == tabTasksSpm)
        ptTable = tableWidgetTasksListSPM;
    else if(pCurrentTab == tabTasksSya)
        ptTable = tableWidgetTasksListSYA_2;
    else if(pCurrentTab == tabTasksStatus)
        ptTable = tableWidgetTasksListStatus;
    else if(pCurrentTab == tabTasksAutoAdmin)
        ptTable = tableWidgetTasksListAutoAdmin;

    return ptTable;
}

// GUI DEPENDENT
int SchedulerGui::GetCurrentTableType()
{
    QWidget     *pCurrentTab = tabWidget->currentWidget();

    // If no item in list, just return!
    if(pCurrentTab == NULL)
        return 0;

    if(pCurrentTab == tabTasksDataPump)
        return GEXMO_TASK_DATAPUMP;
    else if(pCurrentTab == tabTasksTriggerSya)
        return GEXMO_TASK_TRIGGERPUMP;
    else if(pCurrentTab == tabTasksTriggerPat)
        return GEXMO_TASK_PATPUMP;
    else if(pCurrentTab == tabTasksReporting)
        return GEXMO_TASK_REPORTING;
    else if(pCurrentTab == tabTasksConverter)
        return GEXMO_TASK_CONVERTER;
    else if(pCurrentTab == tabTasksOutlierRemoval)
        return GEXMO_TASK_OUTLIER_REMOVAL;
    else if(pCurrentTab == tabTasksYield)
        return GEXMO_TASK_YIELDMONITOR;
    else if(pCurrentTab == tabTasksSpm)
        return GEXMO_TASK_SPM;
    else if(pCurrentTab == tabTasksSya)
        return GEXMO_TASK_SYA;
    else if(pCurrentTab == tabTasksStatus)
        return GEXMO_TASK_STATUS;
    else if(pCurrentTab == tabTasksAutoAdmin)
        return GEXMO_TASK_AUTOADMIN;

    return 0;
}

// GUI DEPENDENT
QList<CGexMoTaskItem*> SchedulerGui::GetSelectedTasks()
{
    QList<CGexMoTaskItem*> lstTasksSelected;

    QTableWidget		*ptTable = GetCurrentTableWidget();
    QTableWidgetItem	*ptItem = NULL;

    if(ptTable == NULL)
        return lstTasksSelected;

    CGexMoTaskItem			*ptTaskItem = NULL;
    // If at least one item in table
    if(ptTable->rowCount() > 0)
    {
        int iTaskId;
        for(int ii=0; ii<ptTable->rowCount(); ii++)
        {
            if(ptTable->isRowHidden(ii))
                continue;

            // Check a visible item
            ptItem = ptTable->item(ii,m_lstTableGenericLabels.count());
            if(ptItem && ptItem->isSelected())
            {
                // Row selected
                // Take the first column for TaskId
                ptItem = ptTable->item(ii,0);
                if(ptItem)
                {
                    iTaskId = ptItem->text().toInt();
                    ptTaskItem = GS::Gex::Engine::GetInstance().GetSchedulerEngine().FindTaskInList(iTaskId);
                    if(ptTaskItem)
                        lstTasksSelected.append(ptTaskItem);
                }
            }
        }
    }

    return lstTasksSelected;
}


///////////////////////////////////////////////////////////
// set Table cell contents + tootip
///////////////////////////////////////////////////////////
void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, int nInt, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    fillCellData(ptTable,iRow,iCol,QString::number(nInt),strTooltip,bReadOnly,bAlarm);
}

void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, bool bBool, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    QString strValue = "NO";
    if(bBool)
        strValue = "YES";
    fillCellData(ptTable,iRow,iCol,strValue,strTooltip,bReadOnly,bAlarm);
}

void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, float fFloat, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    fillCellData(ptTable,iRow,iCol,QString::number(fFloat),strTooltip,bReadOnly,bAlarm);
}

void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, QDateTime clDateTime, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    QString strValue = clDateTime.toString(Qt::ISODate);
    // Remove time info if not set
    if(clDateTime.time() == QTime(0,0))
        strValue = clDateTime.date().toString(Qt::ISODate);

    fillCellData(ptTable,iRow,iCol,strValue,strTooltip,bReadOnly,bAlarm);
}

void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    QTableWidgetItem *ptItem = ptTable->item(iRow,iCol);

    if(ptItem == NULL)
    {
        ptItem = new QTableWidgetItem(strText);
        // Add item to table cell
        ptTable->setItem(iRow,iCol,ptItem);
    }
    else
    {
        // Check if need update
        if(ptItem->text() == strText)
        {
            if((strTooltip.isEmpty() && ptItem->toolTip() == strText)
                    || (!strTooltip.isEmpty() && ptItem->toolTip() == strTooltip))
                return;
        }
    }

    ptItem->setText(strText);

    // Add tooltip
    QString strTip = strTooltip;
    if(strTip.isEmpty())
        strTip = strText;
    ptItem->setToolTip(strTip);

    // Check if Read-Only mode
    if(bReadOnly)
        ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
    else
        ptItem->setFlags(ptItem->flags() | Qt::ItemIsEditable);

    // Check if Alarm color (orange) to use as background color
    if(bAlarm)
        ptItem->setBackground(QBrush(QColor(255,146,100))); // Orange background
}

void SchedulerGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, QIcon clIcon, QString strTooltip/*=""*/, bool bReadOnly/*=true*/, bool bAlarm/*=false*/)
{
    if(ptTable == NULL)
        return;

    QTableWidgetItem *ptItem = ptTable->item(iRow,iCol);

    if(ptItem == NULL)
    {
        ptItem = new QTableWidgetItem(clIcon,"");
        // Add item to table cell
        ptTable->setItem(iRow,iCol,ptItem);
    }
    else
    {
        // Check if need update
        //        int nItemIcon = ptItem->icon().cacheKey();
        //        int nNewIcon = clIcon.cacheKey();
        //        if(nItemIcon == nNewIcon)
        //        {
        //            if(ptItem->toolTip() != strTooltip)
        //                ptItem->setToolTip(strTooltip);
        //            return;
        //        }
        //        else
        ptItem->setIcon(clIcon);

        ptItem->setText("");
    }

    ptItem->setTextAlignment(Qt::AlignHCenter);

    // Add tooltip
    ptItem->setToolTip(strTooltip);

    // Check if Read-Only mode
    if(bReadOnly)
        ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
    else
        ptItem->setFlags(ptItem->flags() | Qt::ItemIsEditable);

    // Check if Alarm color (orange) to use as background color
    if(bAlarm)
        ptItem->setBackground(QBrush(QColor(255,146,100))); // Orange background

}
