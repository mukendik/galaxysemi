///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#include <QDesktopWidget>

#include <gqtl_log.h>

#include "engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "gexmo_constants.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_taskdata.h"
#include "sya_task.h"
#include "reporting/reporting_taskdata.h"
#include "mo_email.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_taskdata.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "mo_task.h"
#include "mo_task_create_gui.h"
#include "product_info.h"
#include "admin_engine.h"
#include "admin_gui.h"
#include "message.h"
#include "spm_dialog.h"
#include "datapump/datapump_task.h"
#include "pat/pat_task.h"
#include "trigger/trigger_task.h"
#include "yield/yield_task.h"
#include "spm/spm_task.h"
#include "reporting/reporting_task.h"
#include "status/status_task.h"
#include "converter/converter_task.h"
#include "outlierremoval/outlierremoval_task.h"
#include "autoadmin/autoadmin_task.h"
#include "spm_engine.h"
#include "sya_engine.h"
#include "sya_dialog.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include "autoadmin/mo_create_task_auto_admin.h"

extern GexMainwindow *	pGexMainWindow;

CGexMoTaskItem* SchedulerGui::CreateTaskDataPump(int TaskType, CGexMoTaskDataPump *ptTaskItem, bool bReadOnly)
{
    int nDatabaseType = 0;
    if((TaskType == GEXMO_TASK_DATAPUMP) || (TaskType == GEXMO_TASK_OLD_DATAPUMP))
    {
        nDatabaseType = DB_SELECT_FOR_INSERTION | DB_TYPE_BLACKHOLE;
        // If user connected from Examinator-PRO
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            nDatabaseType |= DB_STATUS_UPLOADED;
    }

    GexMoCreateTaskDataPump cDataPumpTask(nDatabaseType);
    bool	bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        // ALLOW ALL TYPE OF DATABASES
        if(ptTaskItem->IsUploaded())
            nDatabaseType |= DB_STATUS_UPLOADED;

        if((TaskType == GEXMO_TASK_DATAPUMP) || (TaskType == GEXMO_TASK_OLD_DATAPUMP))
            pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cDataPumpTask.comboBoxDatabaseTarget,
                                                                  nDatabaseType, DB_TDR_YM_PROD);

        cDataPumpTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cDataPumpTask.children(),false);
        }
        else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            AdminUserLogin::EnabledFieldItem(cDataPumpTask.children(),false);
            // task is editable
            // database and folder cannot be updated
            cDataPumpTask.lineEditInputFileExtensions->setEnabled(true);
            cDataPumpTask.comboBoxTaskPriority->setEnabled(true);
            cDataPumpTask.checkBoxExecutionWindow->setEnabled(true);
            cDataPumpTask.timeEditStart->setEnabled(true);
            cDataPumpTask.timeEditStop->setEnabled(true);
            cDataPumpTask.groupBoxYieldMonitoring->setEnabled(true);
            cDataPumpTask.lineEditYieldBins->setEnabled(true);
            cDataPumpTask.comboBoxYieldLevel->setEnabled(true);
            cDataPumpTask.lineEditYieldMinimumParts->setEnabled(true);
            cDataPumpTask.lineEditEmailFrom->setEnabled(true);
            cDataPumpTask.lineEditEmailList->setEnabled(true);
            cDataPumpTask.comboBoxMailFormat->setEnabled(true);
            cDataPumpTask.groupBoxRejectSmallSplitlots->setEnabled(true);
            cDataPumpTask.checkBoxRejectSmallSplitlots_NbParts->setEnabled(true);
            cDataPumpTask.spinBoxRejectSmallSplitlots_NbParts->setEnabled(true);
            cDataPumpTask.checkBoxRejectSmallSplitlots_GdpwPercent->setEnabled(true);
            cDataPumpTask.comboBoxRejectSmallSplitlots_GdpwPercent->setEnabled(true);
            cDataPumpTask.spinBoxMaxPartForTestResultInsertion->setEnabled(true);
            cDataPumpTask.checkBoxRejectFilesOnPassBinlist->setEnabled(true);
            cDataPumpTask.lineEditPassBinlistForRejectTest->setEnabled(true);
        }
    }
    else
    {
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            return NULL;

        // ALLOW ALL TYPE OF DATABASES
        if((TaskType == GEXMO_TASK_DATAPUMP) || (TaskType == GEXMO_TASK_OLD_DATAPUMP))
            pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cDataPumpTask.comboBoxDatabaseTarget,
                                                                  nDatabaseType,DB_TDR_YM_PROD);
    }

    // No database link if trigger datapump
    if((TaskType == GEXMO_TASK_PATPUMP) || (TaskType == GEXMO_TASK_TRIGGERPUMP))
    {
        if(bCreateTask)
        {
            if(TaskType == GEXMO_TASK_PATPUMP)
                cDataPumpTask.LineEditTitle->setText("PAT Trigger DataPump");
            else
                cDataPumpTask.LineEditTitle->setText("Trigger Pump");
        }
    }

    if(bCreateTask)
    {
        QString strEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() &&
                GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            strEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;

        cDataPumpTask.lineEditEmailList->setText(strEmail);
    }

    // Display Dialog box.
    if(cDataPumpTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid DataPump task entered, save data into internal structure list.
    GexMoDataPumpTaskData *ptTaskData;

    if(bCreateTask == true)
    {
        // Creating a new task.
        switch(TaskType)
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
        ptTaskData = new GexMoDataPumpTaskData(ptTaskItem);
        // Copy all data entered
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cDataPumpTask.cDataPumpData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}


///////////////////////////////////////////////////////////
// Create task: Yield monitoring (flat files only)
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskYieldMonitoring(CGexMoTaskYieldMonitor *ptTaskItem, bool bReadOnly)
{
    int nDatabaseType = DB_SELECT_FOR_INSERTION;

    // If user connected from Examinator-PRO
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        nDatabaseType |= DB_STATUS_UPLOADED;

    GexMoCreateTaskYieldCheck cYieldCheckTask(nDatabaseType);
    bool	bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        // ALLOW ALL TYPE OF DATABASES
        if(ptTaskItem->IsUploaded())
            nDatabaseType |= DB_STATUS_UPLOADED;

        pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cYieldCheckTask.comboBoxDatabase,
                                                              nDatabaseType,
                                                              DB_TDR_YM_PROD);

        cYieldCheckTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cYieldCheckTask.children(),false);
        }
    }
    else
    {
        // ALLOW ALL TYPE OF DATABASES
        pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(cYieldCheckTask.comboBoxDatabase,
                                                              nDatabaseType,
                                                              DB_TDR_YM_PROD);
    }

    if(bCreateTask)
    {
        QString strEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            strEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        cYieldCheckTask.lineEditEmailList->setText(strEmail);
    }

    //cYieldCheckTask.OnDatabaseChanged();

    // Display Dialog box.
    if(cYieldCheckTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid DataPump task entered, save data into internal structure list.
    GexMoYieldMonitoringTaskData *ptTaskData;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskYieldMonitor;
        ptTaskData = new GexMoYieldMonitoringTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cYieldCheckTask.cYieldData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Create task: SYA
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskSYA(CGexMoTaskSYA *SourceTask, bool ReadOnly)
{
    CGexMoTaskSYA *lTask = SourceTask;

    if(!lTask)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating a new SYA task...");
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Editing an existing SYA task...");
    }

    int lDatabaseType = DB_TYPE_SQL|DB_SUPPORT_INSERTION;

    // If user connected from Examinator-PRO
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        lDatabaseType |= DB_STATUS_UPLOADED;

    bool lIsNewTask = ( lTask == NULL );

    // Create task item, unless this is an edit
    if(lIsNewTask)
    {
        // Eventually retrieve user's email
        QString lEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
        {
            lEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        }
        // Create the new task
        lTask = GS::Gex::Engine::GetInstance().GetSYAEngine().CreateSYATask(lEmail);
    }

    // Make sure we have an item now
    if(!lTask)
    {
        QString lError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(lError);
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed to create a new SYA task: %1").arg(lError).toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could not create new SYA task!\n\n%1").arg(lError));
        return 0;
    }

    // SPM task dialog
    GS::Gex::SYADialog lSyaDialog(lTask);

    // Initialize SYA task dialog GUI
    if(!lSyaDialog.InitializeUI(lDatabaseType, lIsNewTask))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed initializing SYA task GUI").toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could not initializing SYA task GUI!"));
        return 0;
    }

    // Initialize SPM task GUI
    if(!lSyaDialog.LoadFields())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed loading fields into SPM task GUI").toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could load fields into SPM task GUI!"));
        return 0;
    }

    // In read only mode, grey all UI controls
    if (lTask->m_iDatabaseId)
    {
        GexDatabaseEntry* pDatabaseEntry =
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lTask->m_iDatabaseId);
        if (! (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
        {
            bool lReadWriteMode;
            GS::Gex::Message::request("",
                "Database is not connected.\n"
                "Database access is required for computing limits,"
                " using the pickers to set the task parameters, simulating a check, etc.\n\n"
                "Continue in read/write mode ?\n"
                "Answer no for read-only mode.", lReadWriteMode);
            ReadOnly = ! lReadWriteMode;
        }
    }
    if(ReadOnly)
        AdminUserLogin::EnabledFieldItem(lSyaDialog.children(),false);

    pGexMainWindow->centralWidget()->setEnabled(false);

    // Display Dialog box
    //-- Do not remove the setVisible, it enables to not have the modality set
    //-- by the exec call.....
    lSyaDialog.setVisible(true);
    lSyaDialog.raise();
    // Display Dialog box
    if(lSyaDialog.exec() != 1)
    {
        pGexMainWindow->centralWidget()->setEnabled(true);
        // Delete the created task unless it has been saved otherwise
        if(lTask->GetID() == -1)
        {
            GS::Gex::Engine::GetInstance().GetSPMEngine().DeleteTask(lTask);
            return NULL;
        }
    }

    pGexMainWindow->centralWidget()->setEnabled(true);
    // If in ReadOnly mode, we're done
    if(ReadOnly)
        return NULL;

    return lTask;
}


///////////////////////////////////////////////////////////
// Create task: SPM
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskSPM(CGexMoTaskSPM *SourceTask, bool ReadOnly)
{
    CGexMoTaskSPM *lTask = SourceTask;

    if(!lTask)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating a new SPM task...");
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Editing an existing SPM task...");
    }

    // GCORE-14130: allows non connected DB to be specified in DB selection list
    int lDatabaseType = DB_TYPE_SQL|DB_SUPPORT_INSERTION|DB_STATUS_ADR_LINK;

    // If user connected from Examinator-PRO
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        lDatabaseType |= DB_STATUS_UPLOADED;

    bool lIsNewTask = ( lTask == NULL );

    // Create task item, unless this is an edit
    if(lIsNewTask)
    {
        // Eventually retrieve user's email
        QString lEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
        {
            lEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        }
        // Create the new task
        lTask = GS::Gex::Engine::GetInstance().GetSPMEngine().CreateSPMTask(lEmail);
    }

    // Make sure we have an item now
    if(!lTask)
    {
        QString lError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(lError);
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed to create a new SPM task: %1").arg(lError).toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could not create new SPM task!\n\n%1").arg(lError));
        return 0;
    }

    // SPM task dialog
    GS::Gex::SPMDialog lSpmDialog(lTask);

    // Initialize SPM task dialog GUI
    if(!lSpmDialog.InitializeUI(lDatabaseType, lIsNewTask))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed initializing SPM task GUI").toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could not initializing SPM task GUI!"));
        return 0;
    }

    // Initialize SPM task GUI
    if(!lSpmDialog.LoadFields())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed loading fields into SPM task GUI").toLatin1().constData());
        QMessageBox::warning(pGexMainWindow, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             QString("Could load fields into SPM task GUI!"));
        return 0;
    }

    // In read only mode, grey all UI controls
    if(ReadOnly)
        AdminUserLogin::EnabledFieldItem(lSpmDialog.children(),false);

    pGexMainWindow->centralWidget()->setEnabled(false);

    // Display Dialog box
    //-- Do not remove the setVisible, it enables to not have the modality set
    //-- by the exec call.....
    lSpmDialog.setVisible(true);
    lSpmDialog.raise();
    // Display Dialog box
    if(lSpmDialog.exec() != 1)
    {
        pGexMainWindow->centralWidget()->setEnabled(true);
        // Delete the created task unless it has been saved otherwise
        if(lTask->GetID() == -1)
        {
            GS::Gex::Engine::GetInstance().GetSPMEngine().DeleteTask(lTask);
            return NULL;
        }
    }

    pGexMainWindow->centralWidget()->setEnabled(true);
    // If in ReadOnly mode, we're done
    if(ReadOnly)
        return NULL;

    return lTask;
}

///////////////////////////////////////////////////////////
// Create task: Reporting
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskReporting(CGexMoTaskReporting *ptTaskItem, bool bReadOnly)
{
    GexMoCreateTaskReporting cReportingTask;
    bool	bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        cReportingTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cReportingTask.children(),false);
        }
    }

    if(bCreateTask)
    {
        QString strEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            strEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        cReportingTask.lineEditEmailList->setText(strEmail);
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Allow creation/Edit but with message
        QString strMessage;
        QString strApplicationName =
                GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
        // Get the short name
        if(strApplicationName.count("-") > 1)
            strApplicationName = strApplicationName.section("-",0,1);
        else
            strApplicationName = strApplicationName.section("-",0,0);
        strMessage = "<b>You want to create a 'Reporting' task from '";
        strMessage+= strApplicationName;
        strMessage+= "'.</b>\n<br>\n<br>The path of the CSL, and the paths used inside the CSL have to be accessible from Yield-Man.\n<br>";
        strMessage+= "If a path is specified that is not accessible from Yield-Man, it will generate an error.";

        QMessageBox::information(pGexMainWindow,
                                 GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                 strMessage);
    }

    // Display Dialog box.
    if(cReportingTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid DataPump task entered, save data into internal structure list.
    GexMoReportingTaskData *ptTaskData=0;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskReporting;
        ptTaskData = new GexMoReportingTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cReportingTask.cReportingData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Create task: Status
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskStatus(CGexMoTaskStatus *ptTaskItem, bool bReadOnly)
{
    GexMoCreateTaskStatus	cStatusTask;
    bool	bCreateTask = true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        cStatusTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cStatusTask.children(),false);
        }
        else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            AdminUserLogin::EnabledFieldItem(cStatusTask.children(),false);
            cStatusTask.comboBoxWebStructure->setEnabled(true);
            cStatusTask.LineEditHomePage->setEnabled(true);
            cStatusTask.LineEditHomeReportURL->setEnabled(true);
            cStatusTask.LineEditHomeReportHttpURL->setEnabled(true);
        }
    }
    else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return NULL;
    else
    {
        QListIterator<CGexMoTaskItem*> lstIteratorTask(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList());

        // Reject creation if 'Status' task already exists!
        lstIteratorTask.toFront();

        while(lstIteratorTask.hasNext())
        {
            if(lstIteratorTask.next()->GetTaskType() == GEXMO_TASK_STATUS)
            {
                QMessageBox::information(pGexMainWindow,
                                         GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                         "The 'Status' task already exists, and only one can "
                                         "be created.\nYou are now going to edit the existing one!");
                return NULL;
            }
        };
    }

    // Display Dialog box.
    if(cStatusTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid Status task entered, save data into internal structure list.
    GexMoStatusTaskData *ptTaskData;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskStatus;
        ptTaskData = new GexMoStatusTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cStatusTask.m_cStatusData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Create task: File Converter (Batch convert to STDF or CSV)
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskFileConverter(CGexMoTaskConverter *ptTaskItem, bool bReadOnly)
{
    GexMoCreateTaskFileConverter cFileConverterTask;
    bool	bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        cFileConverterTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cFileConverterTask.children(),false);
        }
        else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            AdminUserLogin::EnabledFieldItem(cFileConverterTask.children(),false);
            cFileConverterTask.lineEditInputFileExtensions->setEnabled(true);
            cFileConverterTask.comboBoxFormat->setEnabled(true);
            cFileConverterTask.comboBoxTaskPriority->setEnabled(true);
            cFileConverterTask.comboBoxTaskFileOrder->setEnabled(true);
            cFileConverterTask.checkBoxTimeStampName->setEnabled(true);
        }
    }
    else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return NULL;

    // Display Dialog box.
    if(cFileConverterTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid task entered, save data into internal structure list.
    GexMoFileConverterTaskData *ptTaskData;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskConverter;
        ptTaskData = new GexMoFileConverterTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cFileConverterTask.cConverterData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Create task: Outlier Removal
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskOutlierRemoval(CGexMoTaskOutlierRemoval *ptTaskItem, bool bReadOnly)
{
    GexMoCreateTaskOutlierRemoval cOutlierRemovalTask;
    bool	bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        cOutlierRemovalTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cOutlierRemovalTask.children(),false);
        }
    }

    if(bCreateTask)
    {
        QString strEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            strEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        cOutlierRemovalTask.lineEditEmailList->setText(strEmail);
    }

    // Display Dialog box.
    if(cOutlierRemovalTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid DataPump task entered, save data into internal structure list.
    GexMoOutlierRemovalTaskData *ptTaskData;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskOutlierRemoval;
        ptTaskData = new GexMoOutlierRemovalTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cOutlierRemovalTask.cOutlierRemovalData;
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Create task: Auto Admin
///////////////////////////////////////////////////////////
// GUI DEPENDENT
CGexMoTaskItem* SchedulerGui::CreateTaskAutoAdmin(CGexMoTaskAutoAdmin *ptTaskItem, bool bReadOnly)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create Task AutoAdmin...");
    GexMoCreateTaskAutoAdmin cAutoAdminTask;
    bool bCreateTask=true;

    // If Task already exists in the list, fills its fields
    if(ptTaskItem != NULL)
    {
        cAutoAdminTask.LoadFields(ptTaskItem);
        bCreateTask = false;

        if(bReadOnly)
        {
            AdminUserLogin::EnabledFieldItem(cAutoAdminTask.children(),false);
        }
        else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            AdminUserLogin::EnabledFieldItem(cAutoAdminTask.children(),false);
            cAutoAdminTask.comboBoxLogContent->setEnabled(true);
            cAutoAdminTask.timeEditStart->setEnabled(true);
            cAutoAdminTask.comboBoxEraseReports->setEnabled(true);
            cAutoAdminTask.lineEditEmailFrom->setEnabled(true);
            cAutoAdminTask.lineEditEmailList->setEnabled(true);
            cAutoAdminTask.checkBoxSendLogs->setEnabled(true);
        }
    }
    else if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return NULL;

    if(bCreateTask)
    {
        QString strEmail;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            strEmail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strEmail;
        cAutoAdminTask.lineEditEmailList->setText(strEmail);
    }

    // Display Dialog box.
    if(cAutoAdminTask.exec() != 1)
        return NULL;

    if(bReadOnly)
        return NULL;

    // If valid DataPump task entered, save data into internal structure list.
    GexMoAutoAdminTaskData *ptTaskData=0;

    if(bCreateTask == true)
    {
        // Creating a new task.
        ptTaskItem = new CGexMoTaskAutoAdmin;
        ptTaskData = new GexMoAutoAdminTaskData(ptTaskItem);
    }
    else
        ptTaskData = ptTaskItem->GetProperties();

    // Copy Dialog box fields into structure
    *ptTaskData = cAutoAdminTask.GetData();
    ptTaskItem->SetProperties(ptTaskData);

    return ptTaskItem;
}
