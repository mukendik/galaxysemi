///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

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
#include <gstdl_utils_c.h>
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
#include "statisticalMonitoring/statistical_monitoring_task.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include "mo_task.h"
#include "mo_task_create_gui.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "db_datakeys_dialog.h"
#include "gqtl_datakeys.h"
#include "command_line_options.h"
#include "db_external_database.h"
#include "yield/yield_task.h"
#include "gex_shared.h"
#include "mo_email.h"
#include "message.h"
#include "admin_engine.h"
#include "admin_gui.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *  pGexMainWindow;
extern void             WriteDebugMessageFile(const QString & strMessage);

///////////////////////////////////////////////////////////
// Task Scheduler window
///////////////////////////////////////////////////////////
SchedulerGui::SchedulerGui(QWidget *parent, bool /*modal*/, Qt::WindowFlags fl)
    :QWidget(parent, fl)
{
    setupUi(this);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    move(0,0);

    QObject::connect(buttonNewTask,         SIGNAL(clicked()), this, SLOT(OnNewTask()));
    QObject::connect(buttonTaskProperties,  SIGNAL(clicked()), this, SLOT(OnTaskProperties()));
    QObject::connect(buttonDuplicateTask,   SIGNAL(clicked()), this, SLOT(OnDuplicateTask()));
    QObject::connect(buttonRunTask,         SIGNAL(clicked()), this, SLOT(OnRunAllTasks()));
    QObject::connect(buttonDeleteTask,      SIGNAL(clicked()), this, SLOT(OnDeleteTasks()));
    QObject::connect(buttonPauseScheduler,  SIGNAL(clicked()), this, SLOT(OnPauseScheduler()));
    QObject::connect(buttonRunScheduler,    SIGNAL(clicked()), this, SLOT(OnRunScheduler()));

    // Change the tip of the Run button that run all tasks now
    buttonNewTask->setToolTip("Create a new Task");
    buttonTaskProperties->setToolTip("Edit Task settings");
    buttonDuplicateTask->setToolTip("Duplicate a Task");
    buttonRunTask->setToolTip("Execute all Tasks now");
    buttonDeleteTask->setToolTip("Delete all selected Tasks");

    // GCORE-17451 - hide SPM management GUI
    tabWidget->removeTab(8);

    // New GUI
    QObject::connect(tabWidget ,            SIGNAL(currentChanged (int)), this, SLOT(OnSelectTab(int)));

    QObject::connect(tableWidgetTasksListDataPump,          SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListTriggerSya,        SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListTriggerPat,        SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListReporting,         SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListConverter,         SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListOutlierRemoval,    SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListYieldLimits ,      SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListStatus ,           SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListAutoAdmin ,        SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListSPM ,              SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
    QObject::connect(tableWidgetTasksListSYA_2 ,            SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));

    QObject::connect(tableWidgetTasksListDataPump,          SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListTriggerSya,        SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListTriggerPat,        SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListReporting,         SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListConverter,         SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListOutlierRemoval,    SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListYieldLimits ,      SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListStatus ,           SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListAutoAdmin ,        SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListSPM ,              SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));
    QObject::connect(tableWidgetTasksListSYA_2 ,            SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectItem()));


    QObject::connect((QObject*) tableWidgetTasksListDataPump->horizontalHeader() ,          SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListTriggerSya->horizontalHeader() ,        SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListTriggerPat->horizontalHeader() ,        SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListReporting->horizontalHeader() ,         SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListConverter->horizontalHeader() ,         SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListOutlierRemoval->horizontalHeader() ,    SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListYieldLimits->horizontalHeader() ,       SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListStatus->horizontalHeader() ,            SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListAutoAdmin->horizontalHeader() ,         SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListSPM->horizontalHeader() ,               SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
    QObject::connect((QObject*) tableWidgetTasksListSYA_2->horizontalHeader() ,             SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));

    // Connect signals: Contextual menu
    tableWidgetTasksListDataPump->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListTriggerSya->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListTriggerPat->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListReporting->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListConverter->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListOutlierRemoval->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListYieldLimits->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListStatus->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListAutoAdmin->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListSPM->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidgetTasksListSYA_2->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect((QObject*) tableWidgetTasksListDataPump,       SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListTriggerSya,     SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListTriggerPat,     SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListReporting,      SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListConverter,      SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListOutlierRemoval, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListYieldLimits,    SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListStatus,         SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListAutoAdmin,      SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListSPM,            SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));
    QObject::connect((QObject*) tableWidgetTasksListSYA_2,          SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnContextMenu(const QPoint &)));

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

    QString     TypeId;
    QStringList TaskTypeList = CGexMoTaskItem::GetTypeListFor(GS::LPPlugin::ProductInfo::getInstance()->isPATMan());
    tableWidgetTasksListDataPump->hide();
    tableWidgetTasksListTriggerSya->hide();
    tableWidgetTasksListTriggerPat->hide();
    tableWidgetTasksListReporting->hide();
    tableWidgetTasksListConverter->hide();
    tableWidgetTasksListYieldLimits->hide();
    tableWidgetTasksListOutlierRemoval->hide();
    tableWidgetTasksListStatus->hide();
    tableWidgetTasksListAutoAdmin->hide();
    tableWidgetTasksListSPM->hide();
    tableWidgetTasksListSYA_2->hide();

    foreach(TypeId, TaskTypeList)
    {
        switch(TypeId.toInt())
        {
        case GEXMO_TASK_DATAPUMP:
            tableWidgetTasksListDataPump->show();
            break;
        case GEXMO_TASK_TRIGGERPUMP:
            tableWidgetTasksListTriggerSya->show();
            break;
        case GEXMO_TASK_PATPUMP:
            tableWidgetTasksListTriggerPat->show();
            break;
        case GEXMO_TASK_YIELDMONITOR:
            tableWidgetTasksListYieldLimits->show();
            break;
        case GEXMO_TASK_REPORTING:
            tableWidgetTasksListReporting->show();
            break;
        case GEXMO_TASK_STATUS:
            tableWidgetTasksListStatus->show();
            break;
        case GEXMO_TASK_CONVERTER:
            tableWidgetTasksListConverter->show();
            break;
        case GEXMO_TASK_OUTLIER_REMOVAL:
            tableWidgetTasksListOutlierRemoval->show();
            break;
        case GEXMO_TASK_AUTOADMIN:
            tableWidgetTasksListAutoAdmin->show();
            break;
        case GEXMO_TASK_SPM:
            tableWidgetTasksListSPM->show();
            break;
        case GEXMO_TASK_SYA:
            tableWidgetTasksListSYA_2->show();
            break;
        }
    }


    if(tableWidgetTasksListDataPump->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksDataPump));
    if(tableWidgetTasksListTriggerSya->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksTriggerSya));
    if(tableWidgetTasksListTriggerPat->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksTriggerPat));
    if(tableWidgetTasksListReporting->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksReporting));

    if(tableWidgetTasksListConverter->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksConverter));
    if(tableWidgetTasksListYieldLimits->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksYield));
    if(tableWidgetTasksListOutlierRemoval->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksOutlierRemoval));
    if(tableWidgetTasksListStatus->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksStatus));
    if(tableWidgetTasksListAutoAdmin->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksAutoAdmin));
    if(tableWidgetTasksListSPM->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksSpm));
    if(tableWidgetTasksListSYA_2->isHidden())
        tabWidget->removeTab(tabWidget->indexOf(tabTasksSya));

    tabWidget->setCurrentIndex(0);


    connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
            SIGNAL(sUpdateListView(QStringList)),  this, SLOT(UpdateListView(QStringList)));
    connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
            SIGNAL(sUpdateListViewItem(CGexMoTaskItem *,QString)),  this, SLOT(UpdateListViewItem(CGexMoTaskItem *,QString)));
    connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
            SIGNAL(sDisplayStatusMessage(QString)),  this, SLOT(DisplayStatusMessage(QString)));
    connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
            SIGNAL(sPopupMessage(QString )),  this, SLOT(PopupMessage(QString)));
    connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
            SIGNAL(sGetCurrentTableType( )),  this, SLOT(GetCurrentTableType()));
    connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
            SIGNAL(sPopupMessage(QString )),  this, SLOT(PopupMessage(QString)));

    UpdateListView(QStringList()<<"Clear"<<"Wait");
    UpdateListView(QStringList()<<"Load"<<"Force");
}

///////////////////////////////////////////////////////////
// Task Scheduler Destructor function.
///////////////////////////////////////////////////////////
SchedulerGui::~SchedulerGui()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Write into Log file
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog("", "Closing",
                                                                               GS::Gex::Engine::GetInstance().Get("AppFullName").toString() );
    }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include "license_provider_common.h"

void SchedulerGui::OnRunScheduler(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "OnRunScheduler clicked");
    // for debug
    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "RunScheduler not allowed");
        return;
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().RunTaskScheduler(true);
    GS::Gex::Engine::GetInstance().GetCommandLineOptions().SetWelcomeBoxEnabled(false);
}

void SchedulerGui::OnPauseScheduler(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "OnPauseScheduler clicked");
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().RunTaskScheduler(false);
}
///////////////////////////////////////////////////////////
// Enabled or Disabled task
///////////////////////////////////////////////////////////
// nAction = -1 : tasks enabled are disabled and tasks disables are enables
// nAction = 0 : all tasks are disabled
// nAction = 1 : all tasks are enabled
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnEnabledTasks(QList<CGexMoTaskItem*> lstTasks, int nAction)
{
    if(pGexMainWindow == NULL)
        return;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    // Check if a user is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        return;

    CGexMoTaskItem      *   ptTaskItem;

    if(lstTasks.isEmpty())
        return; // Task not found in list...

    bool bEnabledState;

    foreach(ptTaskItem, lstTasks)
    {
        bEnabledState = ptTaskItem->GetEnabledState();
        if(nAction == -1)
            bEnabledState = !bEnabledState;
        if(nAction == 0)
            bEnabledState = false;
        if(nAction == 1)
            bEnabledState = true;

        if(bEnabledState == ptTaskItem->GetEnabledState())
            continue; // EnableStatus not change

        //if(ptTaskItem->m_iStatus != MO_TASK_STATUS_ERROR)
        {
            ptTaskItem->SetEnabledState(!ptTaskItem->GetEnabledState());

            // Save new task list to disk
            if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(ptTaskItem))
            {
                QString strError;
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
                    GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(strError);
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && strError.isEmpty())
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
                GSLOG(SYSLOG_SEV_ERROR, QString("SaveDbTasks return error %1").arg( strError).toLatin1().constData() );
            }
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTaskStatus(ptTaskItem,"Folder|DatabaseStatus");
        }


        // Update ListView
        UpdateListViewItem(ptTaskItem);
    }
}

///////////////////////////////////////////////////////////
// Create a new task
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnNewTask(void)
{
    OnTaskProperties(NULL, true);
}

QString SchedulerGui::OnTaskProperties(CGexMoTaskItem *ptTask, bool bCreateNewTask)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "On task properties...");
    if(pGexMainWindow == NULL)
        return "error: GexMainWindow NULL";

    bool bReadOnly = !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped();

    // Check if yieldman is connected
    if( bCreateNewTask &&  GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return "error: Admin Engine not connected";

    // Check if the user is allowed to modify this task
    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        bReadOnly=true;

    // Check if a user is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        bReadOnly = true;

    // If Scheduler is active (and user not connected) and CreateNewTask is true
    // then ignore
    if(bReadOnly && bCreateNewTask)
    {
        CheckButtonsStatus();
        return "error: illegal action";
    }

    if(ptTask && bCreateNewTask)
    {
        // incompatible option
        return "error: incompatible option CreateNew and given task";
    }

    int iTaskType = 0;
    CGexMoTaskItem *ptTaskItem = ptTask;
    if(ptTaskItem == NULL)
    {
        QList<CGexMoTaskItem*> lstTasksSelected = GetSelectedTasks();

        if(!bCreateNewTask)
        {
            // take the first selected
            if(lstTasksSelected.isEmpty() || (lstTasksSelected.first() == NULL))
                return "error: Given task not found in list"; // Task not found in list...

            ptTaskItem = lstTasksSelected.first();
        }
    }

    // Update the Events flow
    CGexMoTaskItem *ptTaskTrace=ptTaskItem;

    if(ptTaskItem)
    {
        // If Task exist and is uploaded
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().ReloadDbTaskIfUpdated(ptTaskItem);

        iTaskType = ptTaskItem->GetTaskType();

        if(iTaskType == GEXMO_TASK_OLD_DATAPUMP)
            return "error: old datapump";

        // If the task is local
        //  if scheduler is paused then allow to modify
        // If the task is uploaded
        //  if scheduler is paused AND user is connected AND user is allowed to modify then allow to modify

        if(ptTaskItem->IsUploaded())
        {
            // Check if a user is connected
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                    && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(ptTaskItem))
                bReadOnly=true;

            if(!bReadOnly
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                bReadOnly = !GS::Gex::Engine::GetInstance().GetAdminEngine().Lock(ptTaskTrace);
                if(bReadOnly)
                {
                    QString msg;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(msg);
                    QMessageBox::information(pGexMainWindow,
                                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                             msg);
                }
            }
        }
    }
    else
    {
        iTaskType = GetCurrentTableType();
        // If no item in list, just return!
        if(iTaskType == 0)
            return "error: no item in list";
    }

    if(!bReadOnly)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(true);

    if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isTaskBeingEdited() && ptTaskTrace && ptTaskTrace->IsUploaded())
        ptTaskTrace->TraceUpdate("EDIT","START","Task edited");

    // Call the relevant dialog box (& fill it with relevant data)
    if(ptTaskItem)
    {
        ptTaskItem = ptTaskItem->CreateTask(this, bReadOnly);
    }
    else
    {
        switch(iTaskType)
        {
        case GEXMO_TASK_DATAPUMP:
        case GEXMO_TASK_TRIGGERPUMP:
        case GEXMO_TASK_PATPUMP:
            ptTaskItem = CreateTaskDataPump(iTaskType,NULL,bReadOnly);
            break;
        case GEXMO_TASK_YIELDMONITOR:
            ptTaskItem = CreateTaskYieldMonitoring(NULL,bReadOnly);
            break;
        case GEXMO_TASK_SPM:
            ptTaskItem = CreateTaskSPM(NULL,bReadOnly);
            break;
        case GEXMO_TASK_SYA:
            ptTaskItem = CreateTaskSYA(NULL,bReadOnly);
            break;
        case GEXMO_TASK_REPORTING:
            ptTaskItem = CreateTaskReporting(NULL,bReadOnly);
            break;
        case GEXMO_TASK_STATUS:
            ptTaskItem = CreateTaskStatus(NULL,bReadOnly);
            break;
        case GEXMO_TASK_CONVERTER:
            ptTaskItem = CreateTaskFileConverter(NULL,bReadOnly);
            break;
        case GEXMO_TASK_OUTLIER_REMOVAL:
            ptTaskItem = CreateTaskOutlierRemoval(NULL,bReadOnly);
            break;
        case GEXMO_TASK_AUTOADMIN:
            ptTaskItem = CreateTaskAutoAdmin(NULL,bReadOnly);
            break;
        default:
            ptTaskItem = NULL;
        }
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(false);

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(ptTaskTrace);

    if(bReadOnly)
        return "error: read only";

    // ptTaskItem is NULL if Properties canceled
    if(ptTaskItem == 0)
    {
        if(ptTaskTrace && ptTaskTrace->IsUploaded())
            // Update the Events flow
            ptTaskTrace->TraceUpdate("EDIT","FAIL","Edition canceled");
        return "error: cancelled";
    }

    // Then Check if the Database is referenced
    ptTaskItem->m_strDatabaseName = "";
    ptTaskItem->m_iDatabaseId = 0;
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTaskStatus(ptTaskItem,"All");

    // If Task not already in list, add it.
    if(bCreateNewTask)
    {
        if(ptTaskItem->IsLocal())
            ptTaskItem->SetID( GS::Gex::Engine::GetInstance().GetSchedulerEngine().getNextLocalTasksIndex());

        //WriteDebugMessageFile("Insert new task "+ptTaskItem->m_strName+"["+QString::number(ptTaskItem->GetID())+"]");
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().appendToTasksList(ptTaskItem);
    }

    bool bStatus = GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(ptTaskItem);

    // Update the Events flow
    if(ptTaskItem && ptTaskItem->IsUploaded())
    {
        QString strError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
        if(bCreateNewTask)
        {
            ptTaskItem->TraceUpdate("CREATE","START","Task creation");
            ptTaskItem->TraceUpdate("CREATE",(bStatus?"PASS":"FAIL"),(bStatus?"Created":strError));
        }
        else
            ptTaskItem->TraceUpdate("EDIT",(bStatus?"PASS":"FAIL"),(bStatus?"Saved":strError));
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTaskStatus(ptTaskItem,"Folder");

    // Update ListView
    UpdateListViewItem(ptTaskItem);

    return "ok";
}

///////////////////////////////////////////////////////////
// Duplicate selected task
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnDuplicateTask(void)
{
    if(pGexMainWindow == NULL)
        return;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    QWidget * pCurrentTab = tabWidget->currentWidget();

    // If no item in list, just return!
    if(pCurrentTab == NULL)
        return;

    QList<CGexMoTaskItem*> lstTasksSelected = GetSelectedTasks();

    if(lstTasksSelected.isEmpty())
        return;

    // Take the first item selected
    CGexMoTaskItem *ptTask = lstTasksSelected.first();

    if(ptTask == NULL)
        return;

    QString strTitle = "Copy of "+ptTask->m_strName;

    // Duplicate internal structures
    DuplicateTaskInList(ptTask,strTitle);
}

///////////////////////////////////////////////////////////
// Execute all tasks
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnRunAllTasks()
{
    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        return;


    QListIterator<CGexMoTaskItem*> lstIteratorTask(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList());
    CGexMoTaskItem* ptTask = NULL;

    lstIteratorTask.toFront();
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if(!ptTask)
            continue;

        switch(ptTask->GetTaskType())
        {
        case GEXMO_TASK_DATAPUMP:
        case GEXMO_TASK_TRIGGERPUMP:
        case GEXMO_TASK_PATPUMP:
        case GEXMO_TASK_REPORTING:
        case GEXMO_TASK_AUTOADMIN:
        case GEXMO_TASK_CONVERTER:
            ptTask->m_tLastExecuted = 1;
            break;
        }
    }

    // Execute tasks.
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().OnCheckScheduler();
}

///////////////////////////////////////////////////////////
// Delete given task(s)
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnDeleteTasks(QList<CGexMoTaskItem*> lstTasks)
{
    if(pGexMainWindow == NULL)
        return;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    QList<CGexMoTaskItem*> lstTasksItems = lstTasks;
    CGexMoTaskItem *ptTaskItem;

    if(lstTasksItems.isEmpty())
    {
        lstTasksItems = GetSelectedTasks();
    }

    if(lstTasksItems.isEmpty())
        return;

    // Remove object if it is selected
    // Ask to confirm delete task(s)
    QString strMessage = "<br>Confirm to permanently remove the ";
    if(lstTasksItems.count() > 1)
        strMessage += QString::number(lstTasksItems.count())+" selected tasks";
    else
        strMessage += "selected task";
    strMessage += "?\n<br>";
    int lCount = 0;
    foreach(ptTaskItem, lstTasksItems)
    {
        ++lCount;
        if(ptTaskItem->IsUploaded())
            strMessage += "  * <img src=\""+QString::fromUtf8(":/gex/icons/task-share.png")+"\"> ";
        else
            strMessage += "  * <img src=\""+QString::fromUtf8(":/gex/icons/task.png")+"\"> ";
        strMessage += ptTaskItem->m_strName + "\n<br>";
        if (lCount>6)
        {
            strMessage += "  ...\n<br>";
            break;
        }
    }

    bool lDelete;
    GS::Gex::Message::request("", "Delete Tasks", lDelete);
    if (! lDelete)
    {
        return;
    }

    foreach(ptTaskItem, lstTasksItems)
    {

        // Save new task list ot disk
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().DeleteDbTask(ptTaskItem);
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadTasksList(false);
}


///////////////////////////////////////////////////////////
// Lock/Unlock given task in YieldManDb
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnLockTask()
{
    if(pGexMainWindow == NULL)
        return;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    CGexMoTaskItem      *ptTaskItem = NULL;
    QList<CGexMoTaskItem*> lstTasksSelected = GetSelectedTasks();

    if(!lstTasksSelected.isEmpty())
        ptTaskItem = lstTasksSelected.first();

    if(ptTaskItem)
    {
        if(ptTaskItem->IsLocal())
            return;

        // First version
        // Lock/Unlock for Other and Group
        if(ptTaskItem->m_iPermissions == 0)
            ptTaskItem->m_iPermissions = 111111;
        else
            ptTaskItem->m_iPermissions = 0;

        // Save new task list ot disk
        if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(ptTaskItem))
        {
            QString strError;
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
                GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(strError);
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && strError.isEmpty())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
            WriteDebugMessageFile("SaveDbTasks() return an error ["+strError+"]");
        }
    }
}

///////////////////////////////////////////////////////////
// Change owner for the given task in YieldManDb
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnChangeTasksOwner(QList<CGexMoTaskItem*> lstTasks)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return;

    // Check if yieldman is connected
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    QList<CGexMoTaskItem*> lstTasksItems = lstTasks;
    CGexMoTaskItem      *ptTaskItem;

    if(lstTasksItems.isEmpty())
    {
        lstTasksItems = GetSelectedTasks();
    }


    if(lstTasksItems.isEmpty())
        return;

    // Take the first task for user reference
    ptTaskItem = lstTasksItems.first();

    QDialog wSelectOwner;
    wSelectOwner.setModal(true);
    wSelectOwner.setLayout(new QVBoxLayout());
    wSelectOwner.setWindowFlags(wSelectOwner.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    wSelectOwner.setWindowTitle("Quantix Yield-Man");
    QLabel clText;
    clText.setText("<img src=\""+QString::fromUtf8(":/gex/icons/yieldmandb_users.png")+"\"> Please select the new owner:");
    wSelectOwner.layout()->addWidget(&clText);

    QComboBox comboUserList;

    AdminUser*  pUser = NULL;
    QMap<int, AdminUser*>::iterator itUser;
    for(itUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.begin(); itUser != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.end(); itUser++)
    {
        pUser = itUser.value();
        // if MasterAdmin => all users
        // if GroupAdmin => only users from the group
        // if User => not allowed
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin()
                || (GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId == pUser->m_nUserId))
            comboUserList.insertItem(pUser->m_nUserId,pUser->m_strLogin,pUser->m_nUserId);
    }

    if(comboUserList.count() == 0)
        return;

    comboUserList.setCurrentIndex(comboUserList.findData(ptTaskItem->m_iUserId));

    wSelectOwner.layout()->addWidget(&comboUserList);

    QPushButton ok("OK", &wSelectOwner);
    wSelectOwner.layout()->addWidget(&ok);
    QObject::connect(&ok, SIGNAL(released()), &wSelectOwner, SLOT(accept()));
    wSelectOwner.show();

    if (wSelectOwner.exec()==QDialog::Rejected)
        return;

    if(comboUserList.currentIndex() == comboUserList.findData(ptTaskItem->m_iUserId))
        return;

    int nNewUserId = comboUserList.itemData(comboUserList.currentIndex()).toInt();
    foreach(ptTaskItem, lstTasksItems)
    {
        if(ptTaskItem->IsLocal())
            return;

        ptTaskItem->m_iUserId = nNewUserId;

        // Save new task list ot disk
        if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(ptTaskItem))
        {
            QString strError;
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
                GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(strError);
            if(strError.isEmpty())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
            WriteDebugMessageFile("SaveDbTasks() return an error ["+strError+"]");
        }

        // Update ListView
        UpdateListViewItem(ptTaskItem);
    }
}

///////////////////////////////////////////////////////////
// Table mouse Right-click
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnContextMenu(const QPoint& /*pPoint*/)
{
    if(pGexMainWindow == NULL)
        return;

    bool bPauseEnabled = true;
    bool bTaskEnabled = true;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    // Check if a user is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
    {
        bPauseEnabled = false;
        bTaskEnabled = false;
    }

    if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        bTaskEnabled = false;

    QMenu   *pMenu = new QMenu(this);
    QAction *pAction_PauseScheduler = NULL;
    QAction *pAction_ChangeOwner = NULL;
    QAction *pAction_NewEntry = NULL;
    QAction *pAction_EditEntry = NULL;
    QAction *pAction_DuplicateEntry = NULL;
    QAction *pAction_DeleteEntry = NULL;
    QAction *pAction_EnabledEntry = NULL;
    QAction *pAction_DisabledEntry = NULL;
    QAction *pAction_ExecuteEntry = NULL;
    QAction *pAction_SYA_Manage = NULL;
    QAction *pAction_SYA_Renew = NULL;
    /*QAction *pAction_ConvertToInsertion = NULL;
    QAction *pAction_ConvertToSya = NULL;
    QAction *pAction_ConvertToPat = NULL;*/

    QWidget *pCurrentTab = tabWidget->currentWidget();

    bool    bTaskExecutable = false;


    // If no item in list, just return!
    if(pCurrentTab == NULL)
        return;

    int iTaskType = GetCurrentTableType();
    bool bChangeOwnerDisable = false;

    if((iTaskType == GEXMO_TASK_DATAPUMP) ||
            (iTaskType == GEXMO_TASK_TRIGGERPUMP) ||
            (iTaskType == GEXMO_TASK_PATPUMP) ||
            (iTaskType == GEXMO_TASK_CONVERTER) ||
            (iTaskType == GEXMO_TASK_AUTOADMIN))
    {
        bChangeOwnerDisable = true;
        bTaskExecutable = true;
    }
    else if((iTaskType == GEXMO_TASK_REPORTING) ||
            (iTaskType == GEXMO_TASK_STATUS))
    {
        bTaskExecutable = true;
    }

    CGexMoTaskItem          *ptTaskItem = NULL;
    QList<CGexMoTaskItem*> lstTaskItems = GetSelectedTasks();

    // Create menu
    if(buttonRunScheduler->isVisible())
    {
        if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
            pAction_PauseScheduler = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/stop_service.png")),"Stop scheduler..." );
        else
            pAction_PauseScheduler = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/start_service.png")),"Run scheduler..." );
        pMenu->addSeparator();
    }

    pAction_NewEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/script_new.png")),"Create a new task" );
    pAction_NewEntry->setEnabled(false);
    if(buttonNewTask->isEnabled())
        pAction_NewEntry->setEnabled(true);

    if(lstTaskItems.count() == 1)
    {
        ptTaskItem = lstTaskItems.first();

        // Check if found a selection
        if(ptTaskItem)
        {

            // Check if a user is connected for Examinator-PRO
            if(ptTaskItem && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                    && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(ptTaskItem))
                bTaskEnabled=false;

            pMenu->addSeparator();

            if(ptTaskItem->IsUploaded())
            {
                pAction_ChangeOwner = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_users.png")),"Change owner for this task" );
                pAction_ChangeOwner->setEnabled(bTaskEnabled && !bChangeOwnerDisable);
            }
            if(ptTaskItem->GetEnabledState())
                pAction_EnabledEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/disable.png")),"Disable this task" );
            else
                pAction_EnabledEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/enable.png")),"Enable this task" );
            pAction_EnabledEntry->setEnabled(bTaskEnabled);// & (ptTaskItem->m_iStatus != MO_TASK_STATUS_ERROR));
            if(bTaskExecutable)
            {
                pAction_ExecuteEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/script_run.png")),"Execute this task now" );
                pAction_ExecuteEntry->setEnabled(buttonRunScheduler->isVisible()
                                                 && GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()
                                                 && (ptTaskItem->m_iStatus != MO_TASK_STATUS_ERROR));
            }
            if((iTaskType == GEXMO_TASK_SYA)
                    && (ptTaskItem!=NULL) && (ptTaskItem->HasProperties()==true))
            {
                pAction_SYA_Renew = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/sya_task.png")),"Renew limits with last computation settings" );
                pAction_SYA_Renew->setEnabled(bTaskEnabled);
            }

            pMenu->addSeparator();
            pAction_EditEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/properties.png")),"Edit this task" );

            pAction_DuplicateEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/copy.png")),"Duplicate this task" );
            pAction_DuplicateEntry->setEnabled(false);
            if(buttonDuplicateTask->isEnabled())
                pAction_DuplicateEntry->setEnabled(true);
            pAction_DeleteEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/file_remove.png")),"Delete this task" );
            pAction_DeleteEntry->setEnabled(false);
            if(buttonDeleteTask->isEnabled())
                pAction_DeleteEntry->setEnabled(bTaskEnabled);
        }
    }
    else if(!lstTaskItems.isEmpty())
    {
        bool bAddAction_ChangeOwner = false;
        bool bAddAction_EnabledEntry = false;
        bool bAddAction_DisabledEntry = false;
        bool bAddAction_SYA = false;

        // Multi tasks selected
        foreach (ptTaskItem, lstTaskItems)
        {
            if(ptTaskItem)
            {
                // Check if a user is connected for Examinator-PRO
                if(ptTaskItem && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                        && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(ptTaskItem))
                    bTaskEnabled=false;
                if(ptTaskItem->IsUploaded())
                    bAddAction_ChangeOwner = true;
                if(ptTaskItem->m_iStatus != MO_TASK_STATUS_ERROR)
                {
                    if(ptTaskItem->GetEnabledState())
                        bAddAction_DisabledEntry = true;
                    else
                        bAddAction_EnabledEntry = true;
                }
                if (iTaskType == GEXMO_TASK_SYA &&
                    ptTaskItem &&
                    ptTaskItem->HasProperties())
                {
                    bAddAction_SYA = true;
                }
            }
        }

        // Add mutli menu
        pMenu->addSeparator();

        pAction_ChangeOwner = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_users.png")),"Change owner for all selected tasks" );
        pAction_ChangeOwner->setEnabled(bAddAction_ChangeOwner && bTaskEnabled && !bChangeOwnerDisable);

        pAction_DisabledEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/disable.png")),"Disable all selected tasks" );
        pAction_DisabledEntry->setEnabled(bAddAction_DisabledEntry && bTaskEnabled);
        pAction_EnabledEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/enable.png")),"Enable all selected tasks" );
        pAction_EnabledEntry->setEnabled(bAddAction_EnabledEntry && bTaskEnabled);

        if (bAddAction_SYA)
        {
            pMenu->addSeparator();
            pAction_SYA_Renew = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/sya_task.png")),"Renew limits with last computation settings");
            pAction_SYA_Renew->setEnabled(bTaskEnabled);
        }

        pMenu->addSeparator();
        pAction_DeleteEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/file_remove.png")),"Delete all selected tasks" );
        pAction_DeleteEntry->setEnabled(false);
        if(buttonDeleteTask->isEnabled())
            pAction_DeleteEntry->setEnabled(bTaskEnabled);
    }

    if(pAction_PauseScheduler)
        pAction_PauseScheduler->setEnabled(bPauseEnabled);

    // Display menu...
    pMenu->setMouseTracking(true);
    QAction *pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    // Check menu selection activated
    if(pActionResult == NULL)
        return;

    if(pActionResult == pAction_PauseScheduler)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().RunTaskScheduler(
                    GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped());
    else if(pActionResult == pAction_EnabledEntry)
    {
        if(lstTaskItems.count() < 2)
            OnEnabledTasks(lstTaskItems);
        else
            OnEnabledTasks(lstTaskItems,1);
    }
    else if(pActionResult == pAction_DisabledEntry)
    {
        OnEnabledTasks(lstTaskItems,0);
    }
    else if(pActionResult == pAction_ExecuteEntry)
    {
        // Force execution even if the Scheduler is paused
        ptTaskItem->m_tLastExecuted = 1;

        // If pump
        if((iTaskType == GEXMO_TASK_DATAPUMP) ||
                (iTaskType == GEXMO_TASK_TRIGGERPUMP) ||
                (iTaskType == GEXMO_TASK_PATPUMP) ||
                (iTaskType == GEXMO_TASK_CONVERTER))
        {
            // If Manual execution...tell user that file created less than 15 secondes ago won't be processed
            // Display warning.
            QMessageBox::information(pGexMainWindow,
                                     GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                     "*INFO* Files received less than 15 secondes ago won't be processed\n(This is to avoid reading incomplete files)");
        }

        GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteTask(ptTaskItem);
    }
    else if(pActionResult == pAction_NewEntry)
        OnNewTask();
    else if(pActionResult == pAction_EditEntry)
        OnTaskProperties(ptTaskItem);
    else if(pActionResult == pAction_ChangeOwner)
    {
        OnChangeTasksOwner(lstTaskItems);
    }
    else if(pActionResult == pAction_DuplicateEntry)
        OnDuplicateTask();
    else if(pActionResult == pAction_DeleteEntry)
    {
        OnDeleteTasks(lstTaskItems);
    }
    else if (pActionResult == pAction_SYA_Renew)
    {
        if (iTaskType == GEXMO_TASK_SYA)
        {
            OnRenew_SYA();
        }
    }
}


///////////////////////////////////////////////////////////
// signal select item
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnSelectItem()
{
    if(pGexMainWindow == NULL)
        return;

    // Check if can be deleted all tasks
    CheckButtonsStatus();
}

///////////////////////////////////////////////////////////
// signal select header
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnSelectHeader(int nColumn)
{
    QTableWidget *ptTable = GetCurrentTableWidget();

    // If no item in list, just return!
    if(ptTable == NULL)
        return;

    // Check if some tasks updated then reload them
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadTasksList(false);

    if(ptTable->rowCount() <= 1)
        return;

    if(nColumn < 0)
        return;

    Qt::SortOrder order = Qt::AscendingOrder;
    int iColumnSorted = -1;
    if(m_mapSortable.contains(ptTable->objectName()))
        iColumnSorted = m_mapSortable[ptTable->objectName()];

    if(iColumnSorted == nColumn)
    {
        order = Qt::DescendingOrder;
        m_mapSortable.remove(ptTable->objectName());
    }
    else
        m_mapSortable[ptTable->objectName()] = nColumn;


    ptTable->sortItems(nColumn,order);

}

///////////////////////////////////////////////////////////
// signal select tab
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::OnSelectTab(int /*nIndex*/)
{
    if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isTaskBeingEdited())
        return;

    if(isHidden())
        return;

    // Check if some tasks updated then reload them
    if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerRunning())
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().LoadTasksList(false);

    // Have to enable/disable New/Edit/... button if Examinator-PRO
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return;

    CheckButtonsStatus();
}

/******************************************************************************!
 * \fn OnRenew_SYA
 ******************************************************************************/
void SchedulerGui::OnRenew_SYA()
{
    QList<QTableWidgetItem*> lList;
    QList<QTableWidgetItem*>::const_iterator lIter;
    QTableWidgetItem* lItem;
    CGexMoTaskItem* lTaskItem;
    QString ret;
    int lSelectedRow;
    int lSelectedCol;
    int lTaskId;

    lList = tableWidgetTasksListSYA_2->selectedItems();
    lSelectedCol = 0;
    for (lIter = lList.begin();
         lIter != lList.end(); ++lIter)
    {
        lSelectedRow = (*lIter)->row();
        if (lSelectedCol == 0)
        {
            lSelectedCol = (*lIter)->column();
        }
        if ((*lIter)->column() != lSelectedCol)
        {
            continue;
        }
        lItem = tableWidgetTasksListSYA_2->item(lSelectedRow,0);
        lTaskId = lItem->text().toInt();
        lTaskItem = GS::Gex::Engine::GetInstance().GetSchedulerEngine().FindTaskInList(lTaskId);
        if (lTaskItem == NULL)
        {
            continue;
        }
        if(lTaskItem->GetTaskType() == GEXMO_TASK_SYA)
        {
            CGexMoTaskStatisticalMonitoring* statMonTask = (CGexMoTaskStatisticalMonitoring*) lTaskItem;
            if(statMonTask->RenewActiveVersion() == RenewalFailed)
            {
                QString error = GGET_LASTERRORMSG(CGexMoTaskStatisticalMonitoring,statMonTask);
                QMessageBox::warning(pGexMainWindow,GS::Gex::Engine::GetInstance().Get("AppFullName").toString(), error);
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Duplicate the given task structure from list (name specified by caller).
///////////////////////////////////////////////////////////
// GUI DEPENDENT
void SchedulerGui::DuplicateTaskInList(CGexMoTaskItem *ptTask,QString strCopyName)
{
    if(ptTask == NULL)
        return;

    // Refuse to duplicate 'Status' task, there is only one per list!
    if(ptTask->GetTaskType() == GEXMO_TASK_STATUS)
    {
        PopupMessage("*Error* Only one 'Status' task is allowed, can't duplicate it!");
        return;
    }

    // Refuse to duplicate 'AutoAdmin' task, there is only one per list!
    if(ptTask->GetTaskType() == GEXMO_TASK_AUTOADMIN)
    {
        PopupMessage("*Error* Only one 'Auto Admin' task is allowed, can't duplicate it!");
        return;
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(true);

    // Creating a new task.
    CGexMoTaskItem *ptTaskItem = ptTask->Duplicate(strCopyName);
    ptTaskItem->SetID(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getNextLocalTasksIndex());

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        ptTaskItem->m_iNodeId = GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeId();

    CGexMoTaskItem *ptTaskAccepted = ptTaskItem->CreateTask(this, false);

    if(ptTaskAccepted == NULL)
    {
        // Action was canceled
        return;
    }

    // YieldManDb
    ptTaskItem->m_strName = strCopyName;

    // Then Check if the Database is referenced
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTaskStatus(ptTaskItem, "Folder");

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Insert new task %1 [%2]").arg(ptTaskItem->m_strName)
          .arg(QString::number(ptTaskItem->GetID())).toLatin1().data() );
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().appendToTasksList(ptTaskItem);

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTaskStatus(ptTaskItem,"Folder");

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(false);
    // Automatically upload all tasks
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(ptTaskItem);

    // Update ListView
    UpdateListViewItem(ptTaskItem);

}

void SchedulerGui::DisplayStatusMessage(QString strText/*=""*/)
{
    if((pGexMainWindow == NULL) || (pGexMainWindow->pWizardAdminGui == NULL))
        return;

    if(pGexMainWindow->pWizardAdminGui->TextLabelStatus == NULL)
        return;

    bool bPauseRequested = false;
    if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()
            && GS::Gex::Engine::GetInstance().HasTasksRunning())
        bPauseRequested = true;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        pGexMainWindow->pWizardAdminGui->TextLabelStatus->show();

    QString strMsg = strText.replace("\n"," ").replace("<br>"," ");
    if(strMsg.isEmpty())
    {
        if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        {
            // RUN task scheduler (Green text)
            strMsg="<b><font color=\"#009933\">* Scheduler is Running *</font></b>";
        }

        else if(bPauseRequested)
        {
            // Requesting PAUSE task scheduler (Orange text)
            strMsg="<b><font color=\"#FF6600\">* STOP requested...</font></b>";
            bPauseRequested = true;
        }
        else
        {
            // Ensure user knows scheduler is paused (Red text)
            strMsg="<b><font color=\"#FF0000\">* SCHEDULER IS STOPPED *</font></b>";
            if(GS::Gex::Engine::GetInstance().GetLicensePassive())
                strMsg="<b><font color=\"#FF0000\">Waiting for client to be ready (Passive License)</font></b>";
        }

        // Update GUI only if status is changed
        if(!bPauseRequested && (pGexMainWindow->pWizardAdminGui->TextLabelStatus->text() != strMsg))
            pGexMainWindow->pWizardAdminGui->TextLabelStatus->setText(strMsg);
    }
    else
    {
        if((strMsg.length()*7) > pGexMainWindow->pWizardAdminGui->TextLabelStatus->width())
        {
            // Then truncate message if have a fileName
            if(strMsg.count("\\") > 4)
            {
                strMsg = strMsg.section("\\",0,2) + " ... " +strMsg.section("\\",strMsg.count("\\")-2,strMsg.count("\\"));
            }
        }
    }

    // Update button status
    CheckButtonsStatus();

    pGexMainWindow->pWizardAdminGui->TextLabelStatus->setText(strMsg);

    // Update Tip message
    DisplayTipMessage(strMsg);
}

///////////////////////////////////////////////////////////
// Display a message reviewing current task status
// return false if day of week is not matching!
///////////////////////////////////////////////////////////
void SchedulerGui::DisplayTipMessage(QString strText/*=""*/)
{
    if((pGexMainWindow == NULL) || (pGexMainWindow->pWizardAdminGui == NULL))
        return;

    if(pGexMainWindow->pWizardAdminGui->TextLabelTip == NULL)
        return;

    QString strMsg = strText;
    // Display good Tip info
    // If GexMo and not connected : Tip: Connect as YieldMan Admin to manage Tasks Scheduler
    // If GexMo and connected and running : Tip: Pause Scheduler to edit tasks
    // If GexMo and connected and pause : Tip: Double click any task listed to review it.
    // If not GexMo : Tip: Double click any task listed to review it (ReadOnly).
    strMsg = "Tip: Double click any item listed to review it.";
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected(true))
            strMsg = "Tip: <font color=\"#FF0000\">Log in as Yield-Man Admin to edit items</font>";

        else if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
            strMsg = "Tip: <font color=\"#FF0000\">Stop Scheduler to edit items</font>";
        else
            strMsg = "Tip: Double click any item listed to review it.";
    }
    else
    {
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
            strMsg = "Tip: <font color=\"#FF0000\">Log in to edit items</font>";
        else
            strMsg = "Tip: Double click any item listed to review it.";
    }

    if(strMsg.isEmpty())
        strMsg = " ";

    pGexMainWindow->pWizardAdminGui->TextLabelTip->setText(strMsg);
}

void SchedulerGui::CheckButtonsStatus()
{
    if((pGexMainWindow == NULL) || (pGexMainWindow->pWizardAdminGui == NULL))
        return;

    bool ActiveScheduler = !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped();
    bool bPauseRequested = false;
    if(!ActiveScheduler && GS::Gex::Engine::GetInstance().HasTasksRunning())
        bPauseRequested = true;

    buttonRunScheduler->setChecked(ActiveScheduler);
    buttonPauseScheduler->setChecked(!ActiveScheduler);


    // Update button for Databases Admin
    if(!bPauseRequested )
        pGexMainWindow->pWizardAdminGui->ButtonReloadListEnabled(!ActiveScheduler);

    bool bUserEnabled = true;

    // Initialization
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {

        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // User admin must be connected
            bUserEnabled = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();

            // if user Admin is disconnected
            if(bUserEnabled)
            {
                buttonRunScheduler->show();
                buttonPauseScheduler->show();
            }
            else
            {
                buttonRunScheduler->hide();
                buttonPauseScheduler->hide();
            }
        }
        else
        {
            // Examinator
            // User must be connected if OptionLevel >= USER_LOGIN
            bUserEnabled = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();

            buttonRunScheduler->hide();
            buttonPauseScheduler->hide();
            buttonRunTask->hide();

        }
    }

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        buttonRunScheduler->setEnabled(bUserEnabled);
        buttonPauseScheduler->setEnabled(bUserEnabled);

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        {
            buttonRunTask->setEnabled(!bPauseRequested && ActiveScheduler);
            buttonNewTask->setEnabled(!bPauseRequested && !ActiveScheduler);
            buttonDuplicateTask->setEnabled(!bPauseRequested && !ActiveScheduler);
            buttonDeleteTask->setEnabled(!bPauseRequested && !ActiveScheduler);
        }
        else
        {
            buttonRunTask->setEnabled(ActiveScheduler);
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
            buttonDeleteTask->setEnabled(false);
        }
    }
    else
    {
        buttonRunTask->setEnabled(!bPauseRequested && ActiveScheduler);
        buttonNewTask->setEnabled(!bPauseRequested && !ActiveScheduler);
        buttonDuplicateTask->setEnabled(!bPauseRequested && !ActiveScheduler);
        buttonDeleteTask->setEnabled(!bPauseRequested && !ActiveScheduler);
    }

    QList<CGexMoTaskItem*> lstTasksSelected = GetSelectedTasks();

    bool bHaveOneItemSelected = !lstTasksSelected.isEmpty() && (lstTasksSelected.count() == 1);
    buttonTaskProperties->setEnabled(bHaveOneItemSelected);
    buttonDuplicateTask->setEnabled(buttonDuplicateTask->isEnabled() && bHaveOneItemSelected);

    if(buttonDeleteTask->isEnabled())
    {
        CGexMoTaskItem* ptTaskItem;
        int iTaskId;

        bool bEnableDeleteButton = true;
        bool bEnableEditButton =  buttonDuplicateTask->isEnabled();

        foreach(ptTaskItem, lstTasksSelected)
        {
            iTaskId = ptTaskItem->GetID();
            // If Task exist and is uploaded
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().ReloadDbTaskIfUpdated(ptTaskItem);

            // Check if always exists
            ptTaskItem = GS::Gex::Engine::GetInstance().GetSchedulerEngine().FindTaskInList(iTaskId);

            if(ptTaskItem == NULL)
                bEnableDeleteButton = false;    // Task not found in list...
            else
            {
                if(ptTaskItem->GetTaskType() == GEXMO_TASK_OLD_DATAPUMP)
                    bEnableEditButton = false;
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(ptTaskItem))
                    bEnableDeleteButton = false;
            }
        }

        buttonDuplicateTask->setEnabled(bEnableEditButton);
        buttonTaskProperties->setEnabled(bEnableEditButton);

        buttonDeleteTask->setEnabled(bEnableDeleteButton);
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        QWidget * pCurrentTab = tabWidget->currentWidget();

        // If no item in list, just return!
        if(pCurrentTab == NULL)
            return;

        if(pCurrentTab == tabTasksDataPump)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
        else if(pCurrentTab == tabTasksTriggerSya)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
        else if(pCurrentTab == tabTasksTriggerPat)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
        else if(pCurrentTab == tabTasksConverter)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
        else if(pCurrentTab == tabTasksStatus)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
        else if(pCurrentTab == tabTasksAutoAdmin)
        {
            // Only in ReadOnly
            buttonNewTask->setEnabled(false);
            buttonDuplicateTask->setEnabled(false);
        }
    }
}
