#ifndef MO_SCHEDULER_GUI_H
#define MO_SCHEDULER_GUI_H

#include "ui_mo_scheduler_dialog.h"

// New GUI
#include "gexdb_plugin_base.h"

#include <time.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <QTextStream>
#include <QList>
#include <QDateTime>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>

// Forward declarations
class GexMoDataPumpTaskData;
class GexMoYieldMonitoringTaskData;
class GexMoSpecMonitoringTaskData;
class GexMoReportingTaskData;
class GexMoStatusTaskData;
class GexMoFileConverterTaskData;
class GexMoOutlierRemovalTaskData;
class GexMoAutoAdminTaskData;
class CGexGroupOfFiles;
//class GS::Gex::PATProcessing;
//#include "gex_pat_processing.h"
class CGexCompositePatProcessing;
class GexDatabaseEntry;
class GexDbPlugin_SYA_Item;
class GexDbPlugin_SYA_Limitset;

// in classes.h
namespace GS
{
    namespace Gex
    {
#ifdef GCORE15334

        class PATProcessing;
#endif
    }
    namespace QtLib
    {
        class Range;
    }
}



// in report_build.h
class CGexFileInGroup;
class CTest;

// in patman_lib.h
class CBinning;
class QTableComboBox;
class QTablePushButton;
class CGexMoTaskItem;
class CGexMoTaskDataPump;
class CGexMoTaskYieldMonitor;
class CGexMoTaskSPM;
class CGexMoTaskSYA;
class CGexMoTaskReporting;
class CGexMoTaskStatus;
class CGexMoTaskConverter;
class CGexMoTaskOutlierRemoval;
class CGexMoTaskAutoAdmin;

class SchedulerGui: public QWidget, public Ui::monitor_scheduler_basedialog
{
    Q_OBJECT

public:
    SchedulerGui( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~SchedulerGui();

    void    UpdateListViewHeader(QTableWidget *ptTable);

    //! \brief Create task: Data Pump
    Q_INVOKABLE CGexMoTaskItem* CreateTaskDataPump(int TaskType, CGexMoTaskDataPump *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskYieldMonitoring(CGexMoTaskYieldMonitor *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskSPM(CGexMoTaskSPM *SourceTask=NULL, bool ReadOnly=false);
    CGexMoTaskItem* CreateTaskSYA(CGexMoTaskSYA *SourceTask=NULL, bool ReadOnly=false);
    CGexMoTaskItem* CreateTaskReporting(CGexMoTaskReporting *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskStatus(CGexMoTaskStatus *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskFileConverter(CGexMoTaskConverter *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskOutlierRemoval(CGexMoTaskOutlierRemoval *ptTaskItem=NULL, bool bReadOnly=false);
    CGexMoTaskItem* CreateTaskAutoAdmin(CGexMoTaskAutoAdmin *ptTaskItem=NULL, bool bReadOnly=false);

    void UpdateListViewDataPumpItem(CGexMoTaskDataPump *ptTaskItem,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewYieldItem(CGexMoTaskYieldMonitor *ptTaskItem,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewReportingItem(CGexMoTaskReporting *ptTaskItem,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewStatusItem(CGexMoTaskStatus *ptTaskItem,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewConverterItem(CGexMoTaskConverter *ptTaskData,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewOutlierItem(CGexMoTaskOutlierRemoval *ptTaskData,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewAutoAdminItem(CGexMoTaskAutoAdmin *ptTaskData,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewSPM(CGexMoTaskSPM *ptTaskItem,int nCurrentRow, bool bIsEditabled=false);
    void UpdateListViewSYA(CGexMoTaskSYA *ptTaskItem, int nCurrentRow, bool bIsEditabled=false);
private:

    // Display a message reviewing current task status
    // return false if day of week is not matching!
    void    DisplayTipMessage(QString strText="");
    void    DuplicateTaskInList(CGexMoTaskItem *ptTask,QString strCopyName);


    QTableWidget*   GetTableWidgetForTaskType(int TaskTypeQWidget);
    QWidget*        GetTabWidgetForTaskType(int TaskType);
    QTableWidget*   GetCurrentTableWidget();

    // New GUI
    QStringList m_lstTableGenericLabels;
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, int nInt, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, bool bBool, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, float fFloat, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, QDateTime clDateTime, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void    fillCellData(QTableWidget *ptTable, int iRow, int iCol, QIcon clIcon, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);

    // New GUI (sortable)
    QMap<QString,int>       m_mapSortable;

public slots :
    // Do NOT move these functions to a non public slots part as it is necessary for JavaScript access
    // Gui slot
    void    UpdateListView(QStringList Options);
    void    UpdateListViewItem(CGexMoTaskItem *ptTaskItem, QString Options=QString());
    // Display a message reviewing current task status
    // return false if day of week is not matching!
    void    DisplayStatusMessage(QString strText="");
    void    PopupMessage(QString strMessage);

    // Give the list of tasks selected in the GUI
    QList<CGexMoTaskItem*> GetSelectedTasks();

    void    OnChangeTasksOwner(QList<CGexMoTaskItem*> lstTasks = QList<CGexMoTaskItem*>());
    void    OnEnabledTasks(QList<CGexMoTaskItem*> lstTasks = QList<CGexMoTaskItem*>(), int nAction=-1);
    void    OnDeleteTasks(QList<CGexMoTaskItem*> lstTasks = QList<CGexMoTaskItem*>());

    void    OnNewTask(void);

    void    OnLockTask(void);
    //! \brief Edit task properties ?
    QString OnTaskProperties(CGexMoTaskItem *ptTask = NULL, bool bCreateNewTask=false);
    void    OnDuplicateTask(void);

    // Execute all tasks
    void    OnRunAllTasks();
    void    OnRunScheduler();
    void    OnPauseScheduler();

    // New GUI (sortable)
    void    OnSelectItem(void);
    void    OnSelectHeader(int nColumn);
    void    OnSelectTab(int nIndex);

    int     GetCurrentTableType();

    void    OnContextMenu(const QPoint &pPoint);
    /*!
     * \fn OnRenew_SYA
     */
    void OnRenew_SYA();

    //! \brief Check button status when scheduler is actived/desactived : Check button status when a task is selected
    void    CheckButtonsStatus();
};

#endif // MO_SCHEDULER_GUI_H
