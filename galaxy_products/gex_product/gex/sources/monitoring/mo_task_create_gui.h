///////////////////////////////////////////////////////////
// Examinator Monitoring: Yield Check dialog box
///////////////////////////////////////////////////////////

#ifndef GEXMO_TASKSGUI_H
#define GEXMO_TASKSGUI_H

#include "ui_mo_datapump_dialog.h"
#include "datapump/datapump_taskdata.h"

#include "ui_mo_file_converter_dialog.h"
#include "converter/converter_taskdata.h"

#include "ui_mo_outlier_removal_dialog.h"
#include "outlierremoval/outlierremoval_taskdata.h"

#include "ui_mo_reporting_dialog.h"
#include "reporting/reporting_taskdata.h"

#include "ui_mo_status_dialog.h"
#include "status/status_taskdata.h"

#include "ui_mo_yieldcheck_dialog.h"
#include "yield/yield_taskdata.h"

#include <qfiledialog.h>
#include <qscriptsyntaxhighlighter_p.h>
#include <qmessagebox.h>
#include <time.h>

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;

class CGexMoTaskAutoAdmin;


//! \class Pump task dialog box.
class CGexMoTaskDataPump;
class GexMoDataPumpTaskData;
class GexMoCreateTaskDataPump: public QDialog, public Ui::monitor_datapump_basedialog
{
    Q_OBJECT

    QScriptSyntaxHighlighter mPreScriptHighlighter;
    QScriptSyntaxHighlighter mPostScriptHighlighter;


public:
    GexMoCreateTaskDataPump(int nDatabaseType, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    //! \brief Load dialog box fields with specified data structure.
    Q_INVOKABLE void LoadFields(CGexMoTaskDataPump *ptTaskItem);
    GexMoDataPumpTaskData cDataPumpData;

public slots:
    //! \brief Select folder from which data files will be read.
    void OnBrowse(void);
    //! \brief Show/Hide Execution window calendar
    void OnExecutionWindow(void);
    //! \brief Hide/Show Move/FTP destination folder
    void OnPostImportMove(void);
    //! \brief Browse disk to pick Move/FTP destination folder (for file successfully inserted)
    void OnBrowsePostImportFolder(void);
    //! \brief Browse disk to pick Move/FTP destination folder (for file FAILING insertion)
    void OnBrowsePostImportFailureFolder(void);
    //! \brief Browse disk to pick Move/FTP destination folder (for file DELAY insertion)
    void OnBrowsePostImportDelayFolder(void);
    //! \brief Hide/Show Yield monitoring
    void OnYieldMonitoring(void);
    //! \brief Enable/disable PreInsertion script
    void OnPreInsertion(void);
    //! \brief Enable/disable PostInsertion script
    void OnPostInsertion(void);
    //! \brief Task frequency changed...
    void OnTaskFrequency(void);
    //! \brief Select mailing list for email notification
    void OnMailingList(void);
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief User cancelled.
    void OnCancel(void);
    //! \brief Database target changed, update GUI.
    void OnDatabaseTarget();
    //! \brief Data type changed, update GUI.
    void OnDataType();
    //! \brief Browse disk + select shell program: for Yield alarm
    void OnSelectInsertionBatch(void);
    //! \brief Hide/Show Yield monitoring
    void OnRejectFilesOnPassBinList(void);
};

class CGexMoTaskConverter;
class GexMoFileConverterTaskData;
class GexMoCreateTaskFileConverter: public QDialog, public Ui::monitor_file_converter_basedialog
{
    Q_OBJECT

public:
    GexMoCreateTaskFileConverter( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    Q_INVOKABLE void	LoadFields(CGexMoTaskConverter *ptTaskItem);
    GexMoFileConverterTaskData cConverterData;

private:

public slots:
    void	OnBrowseInputFolder(void);
    void	OnBrowseOutputFolder(void);
    void	OnBrowsePostConversion(void);
    void	OnBrowseFailedConversion(void);
    void    OnBrowseScriptToRun(void);
    void	OnPostConversionMode(void);
    void	OnOk(void);
    void    OnCancel(void);
    void    runScript(int);

};

class GexDatabaseEntry;
class CGexMoTaskOutlierRemoval;
class GexMoOutlierRemovalTaskData;
class GexMoCreateTaskOutlierRemoval: public QDialog, public Ui::mo_outlier_removal_basedialog
{
    Q_OBJECT

public:
    GexMoCreateTaskOutlierRemoval( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    //! \brief Load dialog box fields with specified data structure.
    Q_INVOKABLE void	LoadFields(CGexMoTaskOutlierRemoval *ptTaskItem, QStringList *plistTestingStages=NULL);

    GexMoOutlierRemovalTaskData cOutlierRemovalData;

private:
    GexDatabaseEntry	*m_pDatabaseEntry;

public slots:
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief Cancelling
    void OnCancel(void);
    //! \brief Show list of ProductIDs, let user pick one!
    void OnPickProductID(void);
    //! \brief Products string list changed...check if valid.
    void OnProductsChange(const QString &strString);
    //! \brief Select mailing list for email notification
    void OnMailingList(void);
    //! \brief Select type of report to email on Yield exception
    void OnMessageContents(void);
    //! \brief RDB: Database selection changed
    void OnDatabaseChanged();
    //! \brief ?
    void onGenerationModeChange();
};

//! \class Reporting task dialog box.
class CGexMoTaskReporting;
class GexMoReportingTaskData;
class GexMoCreateTaskReporting: public QDialog, public Ui::monitor_reporting_basedialog
{
    Q_OBJECT

public:
    GexMoCreateTaskReporting( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    GexMoReportingTaskData cReportingData;

public slots:
    //! \brief Load dialog box fields with specified data structure.
    void LoadFields(CGexMoTaskReporting *ptTaskItem);
    //! \brief Select script file to execute.
    void OnBrowse(void);
    //! \brief Show/Hide Execution window calendar
    void OnExecutionWindow(void);
    //! \brief Select mailing list for email notification
    void OnMailingList(void);
    //! \brief Task frequency changed...
    void OnTaskFrequency(void);
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief Cancelling
    void OnCancel();
};

class CGexMoTaskStatus;
class GexMoStatusTaskData;
class GexMoCreateTaskStatus: public QDialog, public Ui::monitor_status_basedialog
{
    Q_OBJECT

public:
    GexMoCreateTaskStatus( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    //! \brief Load dialog box fields with specified data structure.
    Q_INVOKABLE void LoadFields(CGexMoTaskStatus *ptTaskItem);

    GexMoStatusTaskData	m_cStatusData;

public slots:
    //! \brief Select Intranet folder
    void OnBrowse(void);
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief Cancelling...
    void OnCancel(void);
};

class CGexMoTaskYieldMonitor;
class GexMoYieldMonitoringTaskData;
//! \class Yield Monitoring task dialog box.
class GexMoCreateTaskYieldCheck: public QDialog, public Ui::monitor_yield_basedialog
{
    Q_OBJECT

public:
    GexMoCreateTaskYieldCheck(int nDatabaseType, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );

    // Load dialog box fields with specified data structure.
    Q_INVOKABLE void LoadFields(CGexMoTaskYieldMonitor *ptTaskItem, QStringList *plistTestingStages=NULL);
    // Load dialog box fields with specified data structure.
    Q_INVOKABLE void LoadFields(GexMoYieldMonitoringTaskData *ptYieldData, QStringList *plistTestingStages=NULL);

    GexMoYieldMonitoringTaskData cYieldData;

private:
    GexDatabaseEntry *m_pDatabaseEntry;

public slots:
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief Cancelling
    void OnCancel(void);

    //! \brief Show list of ProductIDs, let user pick one!
    void OnPickProductID(void);
    //! \brief Show list of binning, let user pick one!
    void OnPickBinningList(void);
    //! \brief Products string list changed...check if valid.
    void OnProductsChange(const QString &strString);
    // Select Yield check type:
    // - Fixed threshold
    // - SBL/YBL data file (Statistical Yield Limit, Yield Bin limit)
    // - Bins % per site
    void OnYieldCheckType(void);
    //void	OnLoadSblFile(void);

    //! \brief Select mailing list for email notification
    void OnMailingList(void);
    //! \brief Select type of report to email on Yield exception
    void OnMessageContents(void);
    //! \brief RDB: Database selection changed
    void OnDatabaseChanged();
};

#endif
