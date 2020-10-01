#ifndef GEXMO_SCHEDULER_H
#define GEXMO_SCHEDULER_H

#include <QObject>
#include <QVariant>
#include <QTimer>
#include <QDateTime>
#include <QTextStream>
#include <QScriptValue>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QNetworkReply>
//#include "engine.h"

/*
 1 : move some methods : from GUI object "GexMoScheduler" "SchedulerEngine" :
        GS::Gex::Engine::GetInstance.GetSchedulerEngine().DoThat()
        GS::Gex::SchedulerEngine::GetInstance().DoThat()
 2 : move members
 3 : create slots & signals :
        in SchedulerEngine: emits NewAlarm;
        in SchedulerGUI : connect(SchedulerEngine, SIGNAL(NewAlarm), this, SLOT(NewAlarm))
*/


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
class CGexMoTaskItem;
class CGexMoTaskDataPump;
class CGexMoTaskTriggerPatPump;
class CGexMoTaskPatPump;
class CGexMoTaskReporting;
class CGexMoTaskConverter;
class CGexMoTaskYieldMonitor;
class CGexMoTaskStatus;
class CGexMoTaskAutoAdmin;
class CGexMoTaskStatisticalMonitoring;
class CGexMoTaskSPM;
class CGexMoTaskSYA;
class CGexMoTaskOutlierRemoval;
//class GS::Gex::PATProcessing;
//#include "gex_pat_processing.h"
class CGexCompositePatProcessing;
class GexDatabaseEntry;
class GexDbPlugin_SYA_Item;
class GexDbPlugin_SYA_Limitset;
// in report_build.h
class CGexFileInGroup;
class CTest;
// in patman_lib.h
class GexTbPatSinf;
#ifdef GCORE15334
class GexTbPatDialog;
#endif
class CBinning;

// Qt forward declarations
class QEventLoop;

// in classes.h
namespace GS
{
namespace Gex
{
class PATProcessing;

namespace PAT
{
class ExternalMapDetails;
}
}
namespace QtLib
{
class Range;
class DatakeysContent;
}
}

namespace GS
{
namespace Gex
{
class SchedulerEngine : public QObject
{
    friend class EnginePrivate;

    Q_OBJECT

    // WRITE stopScheduler(bool) ???
    //! \brief Gives if the scheduler .... Sandrine: please write comments....
    //Q_PROPERTY(bool mSchedulerStopped READ isSchedulerStopped)
    //! \brief gives current state whatever the request // WRITE stopScheduler
    //Q_PROPERTY(bool mStopped READ isStopped WRITE Stop)
    //! \brief gives current state whatever the request // WRITE stopScheduler
    //Q_PROPERTY(bool mStarted READ isStarted WRITE Start)
    //! \brief About requesting
    //Q_PROPERTY(bool mStopRequested READ isStopRequested WRITE stopScheduler)
    //! \brief Has tasks running
    Q_PROPERTY(bool mHasTasksRunning READ HasTasksRunning)
    //! \brief Taks loaded ...
    Q_PROPERTY(bool mTasksLoaded READ isTasksLoaded)
    //! \brief Start requested or not...
    //Q_PROPERTY(bool mStartRequested READ isStartRequested() WRITE Start)
    //! \brief Status 'STARTED', 'RUNNING', 'STOP_REQUESTED', 'STOPPED'...
    Q_PROPERTY(QString mStatus READ getStatus())

    SchedulerEngine(QObject* parent);
    ~SchedulerEngine();

public:
    // Returns an html or raw text string with bins inside GexDbPlugin_SYA_Item
    enum eSummaryFormat { HTML_FORMAT, TSV_FORMAT, CSV_FORMAT } ;

    //! \brief Key to retrieve the monitoring logs folder
    static const QString sMonitoringLogsFolderKey;

    enum ShellMsgType
    {
        ShellYield,        // Yield alarm
        ShellSpm,          // SPM alarm
        ShellSya,          // SYA alarm
        ShellPat           // PAT alarm
    };

    // Return true if all tasks were loaded (from local file, from DB)
    Q_INVOKABLE bool    isTasksLoaded()                     {return mAllTasksLoaded;}
    // Return true if a task is being edited into the GUI
    bool    isTaskBeingEdited()                                  {return mTaskBeingEdited;}
    // For indicate that a task is edited
    void    setTaskBeingEdited(bool Edited)                      {mTaskBeingEdited = Edited;}
    // Force the Scheduler to be active
    Q_INVOKABLE bool stopScheduler(bool stop);
    // Return true if the Scheduler is started
    Q_INVOKABLE bool isSchedulerStopped()                   {return mSchedulerStopped;}
    //! \brief ...
    Q_INVOKABLE bool isStopRequested()                      {return (mSchedulerStopped && isSchedulerRunning()); }
    //! \brief Start the scheduler
    Q_INVOKABLE bool Start()                                { return stopScheduler(false); }
    //! \brief Stop the scheduler
    Q_INVOKABLE bool Stop()                                 { return stopScheduler(true); }
    //! \brief
    Q_INVOKABLE bool isStopped()                            {return !isSchedulerRunning(); }
    //! \brief
    Q_INVOKABLE bool isStarted()                            {return isSchedulerRunning(); }
    //!
    Q_INVOKABLE bool HasTasksRunning();
    //! \brief Return the Scheduler's Status 'STARTED', 'RUNNING', 'STOP_REQUESTED', 'STOPPED'...
    Q_INVOKABLE QString getStatus();

    // Tasks List Management
    QList<CGexMoTaskItem*> getTasksList()                   {return mTasksList;}
    void    appendToTasksList(CGexMoTaskItem* pTask)        {mTasksList.append(pTask);}
    int     getNextLocalTasksIndex()                        {return mLocalTasksIndex--;}

    const QMap<QString, QString>& GetSYALightWeightOptions(int taskId) {return mSYALightWeightOptions[taskId];}
    const QMap<QString, QString>& GetSPMLightWeightOptions(int taskId) {return mSPMLightWeightOptions[taskId];}

private:
    // scheduler is allowed
    bool            mAllowed;

    // For monitoring Status
    time_t          mLastDataReceived;

    QList<CGexMoTaskItem*> mTasksList;      // List of Task items.
    bool            mAllTasksLoaded;        // True if tasks was loaded
    bool            mTaskBeingEdited;            // True if a task is actualy edited then ignore the reload file tasks
    QTime           mLastTimeTasksLoaded;
    // Case 7200: use CHECKSUM on last_update rather than MAX
    qlonglong       mLastDbTasksUpdateCheckSum;// Last CHECKSUM(Date&Time) from ym_tasks...
    qlonglong       mLastDbTasksIdChecksum;   // Last CHECKSUM(task_id) from ym_tasks...
    QTextStream     mLocalTasksFile;
    int             mLocalTasksIndex;
    QDateTime       mLastLocalTasksLoaded;  // Last Date&Time task list was reloaded from XML file...

    // SCHEDULER_INTERVAL_DEFINITION (use this tag to retrieve all linked codes)
    // CHECK THE IMPACT INTO THE GetMasterRole THAT USES 60s BETWEEN EACH MASTER ACTIVITIES
    QTimer          timerScheduler;
    bool            mSchedulerStopped;      // Scheduler mode false=Acvite & running, true= paused
    bool            mSchedulerProcessing;   // Flag to avoid recursive calls into 'Task Processing' functions.
    bool            mSchedulerTimerStarted; // Set to true once the monitoring timer has been started.
    QDate           mLastCheckScheduler;    // Date on last OnCheckScheduler() run

    bool            mTaskIdReceived;        // indicate if new task id has been received or not
    int             mLastTaskIdReceived;    // Last received task id

    QTimer          timerRenameFiles;
    QStringList     mFilesToRename;         // List of temporary files to rename

    QSqlQuery       m_QueryTaskOptions;
    bool            GetNextTaskOptionValue(int TaskId, QString& Field, QString& Value);

    QMap<int, QMap<QString, QString> > mSYALightWeightOptions;
    QMap<int, QMap<QString, QString> > mSPMLightWeightOptions;

    // Avoid to recreate severaltime an empty QList in GetProductStatMonTask when error
    QList<CGexMoTaskStatisticalMonitoring*> mEmptyTaskList;

    QMap<QString,int> mTimersTrace;
    // Time used to measure execution time
    QTime   mTime;

    bool TimersTraceReset(QString TimerName="");
    bool TimersTraceStart(QString TimerName="");
    bool TimersTraceStop(QString TimerName="");
    bool TimersTraceDump(QString TimerName="");

public slots:
    // Init method
    void Activate();
    bool isActivated()  {return mAllowed;}

    // Get an attribute/member...
    QVariant Get(const QString &key);
    // Convert a string into a value. eg: "1.05mv" -> 0.0015
    void ExtractStringValue(QString strValue, double &lfValue, double lfInvalid);
    // Return handle to a task structure for monitoring yield
    // of a given ProductID/TestingStage/Database.
    CGexMoTaskYieldMonitor *GetProductYieldInfo(GS::QtLib::DatakeysContent &dbKeysContent,
                                                bool bFindGoodBinList=false,
                                                CGexMoTaskItem *ptFromTask=NULL);
    CGexMoTaskYieldMonitor *GetProductYieldInfo(QString ProductID, QString DatabaseName,
                                                QString TestingStage,
                                                bool bFindGoodBinList=false,
                                                CGexMoTaskItem *ptFromTask=NULL);

    // Return handle to a task structure for monitoring SPM
    // of a given ProductID/TestingStage/Database. RDB version.
    QList<CGexMoTaskStatisticalMonitoring*> GetProductStatMonTask(GS::QtLib::DatakeysContent &dbKeysContent,
                                                           CGexMoTaskItem *ptFromTask=NULL);

    // Return handle to a task structure for PAT-Man / Outlier identification
    // of a given ProductID.
    CGexMoTaskOutlierRemoval *GetProductOutlierInfo(QString szProductID,
                                          CGexMoTaskItem *ptFromTask=NULL);

    // Return handle to a given task structure or NULL
    CGexMoTaskItem *GetTaskHandle(int iHandle);

    // Return handle to the Status task structure
    CGexMoTaskStatus *GetStatusTask(void);

    // Return handle to the Auto Admin task structure
    // Can return NULL if no AutoAdmin task defined !
    CGexMoTaskAutoAdmin *GetAutoAdminTask(void);

    // return true if the Log Task Details is activated in the AutoAdmin task
    bool LogTaskDetails();

    // Execute Yield monitoring task...
    // Deprecated : to be removed
    bool    CheckSblYield(QString &strSblFile,
                          GS::QtLib::Range *pBinCheck, CBinning *ptBinList,
                          QString &strYieldAlarmMessage, QString &strErrorMessage);

    // Execute Script to analyze data file...
    QString ExecuteDataFileAnalysis(GS::QtLib::DatakeysContent &dbKeysContent,
                                    QList <int> &cSitesList);

    // Get Bin Summary...
    QString GetBinSummaryString(CGexGroupOfFiles *pGroup,
                                long lTotalBins,
                                bool bHtmlFormat=false,
                                bool bMultiSiteFile=false);

    // Monitoring
    void    AppendMoHistoryLog(const QString &lLines,
                               const QString &lTaskType="",
                               const QString &lTitle="");

    // Append lines to Monitoring Report file in widget.
    void    AppendMoReportLog(const QString &lStatus,
                              const QString &lFileName,
                              const QString &lCause,
                              const QString &lDataPump,
                              const QString &lDirectory,
                              unsigned int lFileSize,
                              unsigned int lOriginalSize,
                              unsigned int lTotalInsertionTime,
                              unsigned int lUncompressTime,
                              unsigned int lConvertTime);

    /////////////////////
    // LoadBalacing
    // Insert new actions for given database now
    QString InsertNewActions(GexDatabaseEntry *pDatabaseEntry);

    // Execute given action now
    QString ExecuteOneAction(int ActionId);
    // Insert new actions for given task now
    QString InsertNewActions(CGexMoTaskItem *ptTask);

    // Execute Yield monitoring task...
    QString ExecuteYieldTask(GexDatabaseEntry *pDatabaseEntry,
                             CGexMoTaskYieldMonitor *ptTask,
                             GS::QtLib::DatakeysContent& dbKeysContent);

    // Execute Stat Parameter monitoring task
    QString ExecuteStatMonTask(CGexMoTaskStatisticalMonitoring *ptTask,
                               GS::QtLib::DatakeysContent& dbKeysContent);
#ifdef GCORE15334

    //! \brief Execute Outlier Removal task... PATPump can be null
    //! \return A string empty, starting with "ok" or starting with "error..." or anything else. TODO: Clean me
    //! \returns It seems an empty string means success, a non empty one an error: to be confirmed.
    QString ExecuteOutlierRemovalTask(CGexMoTaskOutlierRemoval *ptOutlierRemovalTask,
                                      CGexMoTaskPatPump *lPATPumpTask,
                                      GS::QtLib::DatakeysContent &dbKeysContent,
                                      // bool bTriggerFile, // PAT-17 : unusefull argument
                                      GS::Gex::PATProcessing  &cTriggerFields,
                                      const QString &lDataFile = "");
#endif

    // Execute Wafer Export task...
    // In case a 'Export WaferMap' task is implemented (from YM GUI),
    // refer to function ExecuteOutlierRemovalTask() to check how the
    // task is implemented.
    QString ExecuteWaferExportTask(CGexMoTaskTriggerPatPump *ptTask,
                                   GS::QtLib::DatakeysContent &dbKeysContent,
                                   GS::Gex::PATProcessing &cTriggerFields);

    // Export wafermap from STDF file...
    // In case a 'Export WaferMap' task is implemented (from YM GUI),
    // refer to function ExecutePatProcessing() to check how the task
    // should be loded.
    QString ExecuteWaferExport(CGexMoTaskTriggerPatPump *ptTask,
                               GS::Gex::PATProcessing &cFields,
                               QString &strErrorMessage);

    //! \brief Execute Composite PAT processing on a file...
    //! \return Returns "ok" or "error...."
    QString ExecuteCompositePatProcessing(CGexCompositePatProcessing &cFields,
                                          QString &strErrorMessage);
    //! \brief Exec composite PAT using a ScriptValue typed array
    QString ExecuteCompositePat(QScriptValue lFields);

    //! \brief Execute PAT processing on a file... ? Hervé ?
    bool    CompositePatMergeDataFiles(CGexCompositePatProcessing &cFields,
                                       QString &strErrorMessage);

    // Read Monitoring config file
    bool    ReadConfigFile(void);
    bool    WriteConfigFile(void);
    // Get task id for the last launched task ?????????????????????????????????
    void    setLastTaskId(int iId);
    // Start/Stop Task scheduler
    void    RunTaskScheduler(bool bRun);
    // Will search for all tasks of given type and add a pointer for each to output
    // Find all tasks with given name and push it into the given list
    QString FindTasksByName(const QString &name, QList<CGexMoTaskItem*> &list);

    // Shell call over alarm: Function definition + List of Alarm types that can call Shell script
    // unknown params can be '?'
    //
    bool LaunchAlarmShell(int iAlarmType,int iSeverity,int iAlarmCount,
                          QString product, QString lot,
                          QString sublot="?", QString wafer="?",
                          QString testername="?", QString operatorname="?",
                          QString filename="?",
                          QString logfilepath="?"
            );
    // YIELMANDB

    void            LoadLocalTasks(bool bForceReload=true);

    CGexMoTaskItem* LoadTaskSectionDataPump();
    CGexMoTaskItem* LoadTaskSectionYieldMonitoring_OneRule();
    CGexMoTaskItem* LoadTaskSectionReporting();
    CGexMoTaskItem* LoadTaskSectionStatus();
    CGexMoTaskItem* LoadTaskSectionFileConverter();
    CGexMoTaskItem* LoadTaskSectionOutlierRemoval();
    CGexMoTaskItem* LoadTaskSectionAutoAdmin();

    void    SaveLocalTasks(void);

    void    SaveGexEmailSectionDirectory(CGexMoTaskStatus* ptTask);
    void    SaveTaskSectionDataPump(CGexMoTaskDataPump* ptTask);
    void    SaveTaskSectionYieldMonitoring(CGexMoTaskYieldMonitor* ptTask);
    void    SaveTaskSectionYieldMonitoring_OneRule(CGexMoTaskYieldMonitor* ptTask);
    void    SaveTaskSectionReporting(CGexMoTaskReporting* ptTask);
    void    SaveTaskSectionStatus(CGexMoTaskStatus* ptTask);
    void    SaveTaskSectionFileConverter(CGexMoTaskConverter* ptTask);
    void    SaveTaskSectionOutlierRemoval(CGexMoTaskOutlierRemoval* ptTask);
    void    SaveTaskSectionAutoAdmin(CGexMoTaskAutoAdmin* ptTask);

    // YIELMANDB
    // Database update
    bool    LoadUploadedTasks(CGexMoTaskItem* ptTask = NULL);

    bool    LoadDbTaskSectionDataPump(CGexMoTaskDataPump* ptTask);
    bool    LoadDbTaskSectionYieldMonitoring_OneRule(CGexMoTaskYieldMonitor* ptTask);
    bool    LoadDbTaskSectionReporting(CGexMoTaskReporting* ptTask);
    bool    LoadDbTaskSectionStatus(CGexMoTaskStatus* ptTask);
    bool    LoadDbTaskSectionFileConverter(CGexMoTaskConverter* ptTask);
    bool    LoadDbTaskSectionOutlierRemoval(CGexMoTaskOutlierRemoval * ptTask);
    bool    LoadDbTaskSectionAutoAdmin(CGexMoTaskAutoAdmin * ptTask);
    bool    DumpTaskOptions(int taskId, QMap<QString, QString>& taskOptions);

    bool    SaveDbTasks(CGexMoTaskItem* ptTask=NULL);
    bool    SaveDbTaskLastExecution(CGexMoTaskItem* ptTask);
    bool    SaveDbTaskAttributes(CGexMoTaskItem* ptTask=NULL);

    // Check Task and update Status
    void    CheckTaskStatus(CGexMoTaskItem *ptTask, QString Options=QString());
    void    ReloadDbTaskIfUpdated(CGexMoTaskItem *ptTask);

    bool    DeleteDbTask(CGexMoTaskItem *ptTask);
    void    DeleteTaskInList(CGexMoTaskItem *ptTask);
    void    DeleteTasksList();

    void    InsertDataFileLoop(CGexMoTaskDataPump *ptTask, QString DataFile);
    void    ExecuteTriggerPumpLoop(CGexMoTaskTriggerPatPump *ptTask, QString DataFile);
    void    ConvertDataFileLoop(CGexMoTaskConverter *ptTask, QString DataFile);
    QString ConvertOneFile(CGexMoTaskConverter *ptTask, QString strFullFileName);

    // After file processed (datapump/converter), move/rename it
    // return error message if any
    QString   ProcessFileAfterExecution(int TaskStatus, int TaskId, QString FileName, QString &FileDest);
    QString   ProcessFileAfterExecution(int TaskStatus, CGexMoTaskItem *Task, QString FullFileName, QString &FileDest);
    QString   ProcessFileAfterExecution(int TaskStatus, CGexMoTaskItem *Task, GS::QtLib::DatakeysContent &KeysContent);

#ifdef GCORE15334

    /*!
     * \fn CreatePatLogFile
     * \brief Create PAT Log file (if enabled)
     */
    void SetProperties(GS::Gex::PATProcessing& lPP,
                       const QString& lErrorMessage,
                       const GS::Gex::PAT::ExternalMapDetails& lExternalMapDetails);
    /*!
     * \fn CreatePatLogFile
     * \brief Create PAT Log file (if enabled)
     */
    void    CreatePatLogFile(GS::Gex::PATProcessing &lPATProcessing,
                             const PAT::ExternalMapDetails& lExternalMapDetails);
#endif
    void    CreateWaferExportLogFile(GS::Gex::PATProcessing &cFields, QString & strDestFile, QString &strErrorMessage);
    void    CreateCompositePatLogFile(CGexCompositePatProcessing &lCompositePatProcess);
    void    CloseCompositePatLogFile(CGexCompositePatProcessing &cFields);
    void    CreateCompositePatReportFile(CGexCompositePatProcessing &cFields);
    void    CloseCompositePatReportFile(CGexCompositePatProcessing &cFields);
    bool    CreateCompositePatResultFile(CGexCompositePatProcessing &cFields,QString &strErrorMessage);

    // Return Format string associated with GUI selection: 'word', 'pdf', etc...
    QString getOutputReportFormat(int iFormat);
//    int     CheckPatYieldLoss(GS::Gex::PATProcessing &cFields, double lfYieldLoss,
//                              long lPatFailingParts, long lTotalParts,
//                              int lDistributionMismatch,
//                              GexDatabaseKeysContent &dbKeysContent,
//                              QString &strErrorMessage, QString &lLogMessage);
#ifdef GCORE15334

    bool    CheckForPATNotification(GS::Gex::PATProcessing &cFields, long lTotalParts,
                                    int lDistributionMismatch);
    bool    SendPATNotification(GS::Gex::PATProcessing &cFields, int lDistributionMismatch,
                                long lTotalParts, GS::QtLib::DatakeysContent &dbKeysContent,
                                QString &strErrorMessage, QString &lLogMessage);

    // Execute PAT processing on a file...
    void    BuildRecipeFileFullName(GS::Gex::PATProcessing &cFields,
                                    GS::QtLib::DatakeysContent &dbKeysContent);
#endif
    bool    MergeInputFiles(GS::Gex::PATProcessing &cFields,
                            QString &strTestDataFile,
                            QString &strErrorMessage);
    void    sendInsertionLogsMail(GexMoAutoAdminTaskData* ptAutoAdminTask, QDate dLastLog);

    // Check if Scheduler is active or with task running
    bool    isSchedulerRunning();
    // When a task start : send a signal to get a task id from the task manager
    int     onStartTask(CGexMoTaskItem *ptTask, QString Command=QString());
    // When a task stop : send a signal to get a task id from the task manager
    int     onStopTask(CGexMoTaskItem *ptTask, QString Error=QString());
    // When a task start : send a signal to get a task id from the task manager
    int     onStartTask(int iType);
    // Execute given task now
    QString ExecuteTask(CGexMoTaskItem *ptTask, QString DataFile=QString());
    // Execute Status task...caller is scheduler.
    void    ExecuteStatusTask(void);
    // Execute DataPump task...
    QString ExecuteDataPumpTask(CGexMoTaskDataPump *ptTask, QString DataFile);
    // Execute TriggerPump task...
    QString ExecuteTriggerPumpTask(CGexMoTaskTriggerPatPump *ptTask, QString DataFile);
    // Execute Reporting task...
    QString ExecuteReportingTask(CGexMoTaskReporting *ptTask);
    // Execute Converter task...
    QString ExecuteFileConverterTask(CGexMoTaskConverter *ptTask, QString DataFile);
    // Execute AutoAdmin task...
    QString ExecuteAutoAdminTask(CGexMoTaskAutoAdmin *ptTask);
    // Execute Yield monitoring task...

    // Simply send an email
    // needs the status yask to be created
    // needs gex_email daemon for the emails to be send
    // strAttachments are files separated by ; char
    QString SendEmail(QString strFrom, QString strTo,
                      QString strSubject, QString strBody,
                      bool bHtmlFormat, QString strAttachments);

    // All tasks were identified by iTaskId
    // Return handle to a task structure (name specified by caller).
    // Iterate on pGexMoTasksList
    // Could return NULL if no task with given ID found
    CGexMoTaskItem *FindTaskInList(int iTaskId);

    // if type -1, give ALL taks
    bool    FindAllTasksOfType(int type, QVector<CGexMoTaskItem*>& output);

    // if type "", give ALL taks
    bool    FindAllTasksOfType(const QString & type, QVector<CGexMoTaskItem*>& output);

    // if type -1, give ALL taks.
    // the returned QScriptValue is an array of CGexMoTaskItem* accessible through array[i]
    // type is GEXMO_TASK_... as defined in header
    QScriptValue    FindAllTasksOfType(int type);

    // Load Tasks : start load tasks in AdminDB and then Load filebased Tasks
    // return false on error
    bool    LoadTasksList(bool bForceReload=false);
    // JS Action compatibility ???
    bool    LoadDbTasks() {return LoadTasksList(false);}
    // Check Tasks List - status
    void    CheckTasksList(bool bForceReload=false);
    // Save Tasks : if TaskId = 0, save all updated DB tasks else TaskId if found and updated
    // return false on error
    bool    SaveDbTasksById(int TaskId=0);

    // Enable all task with this given type or all if no type :
    // Types are : DATAPUMP, YIELDMONITOR, YIELDMONITOR_RDB, SPECMONITOR, REPORTING, STATUS, CONVERTER, OUTLIER_REMOVAL, AUTOADMIN
    bool    EnableAllTasks(QString type="");
    bool    DisableAllTasks(QString type="");
#ifdef GCORE15334
    //! \brief Execute PAT processing on a file. The PATPump arg can be NULL !
    QString ExecutePatProcessing(GS::Gex::PATProcessing &cFields, QString &strErrorMessage, CGexMoTaskPatPump *lPATPump);
    // dirty ?
    QString ExecutePatProcessing(GS::Gex::PATProcessing *cFields);
#endif
    // Enable task with given name : exemple : "datapump*"
    // If several tasks with same name, enable them all
    bool    EnableTask(QString TaskName);
    // Enable task with given name : exemple : "datapump*"
    // If several tasks with same name, enable them all
    bool    DisableTask(QString TaskName);

    void    OnUploadTask(CGexMoTaskItem *ptTask = NULL);

    // Timer based: check if temporary files have to be renamed...
    void    OnCheckRenameFiles(void);

    // Timer based: check for tasks to execute...called every minute
    void    OnCheckScheduler();
    void    OnStopAllRunningTask();     // Require all running task to stop
    void    OnRestartAllPendingTask();  // Require all pending task to restart

    // Retrieve the dir where YM will generate the reports, default is probably UserFolder/GalaxySemi/reports
    QString GetStatusTaskReportsDir();

    // Try to really stop all current process(es) for given DB unsing the StopProcess() signal
    QString StopProcess(const QString &DBname);

private:
    /*!
     * \fn ExecRecurrentScripts
     */
    void ExecRecurrentScripts(const QString &lFolder) const;

    /**
     * @brief find all wafer or sublots having passed with success a consolidation
     * @param all_wafers_or_sublots all wafers or sublots flagged for consolidation
     * @param[output] pass_wafers_or_sublots all wafers or sublots having passed the consolidation
     * @return true if all is done correctly, false otherwise
     */
    bool FindPassWafersOrSublots( const QStringList &all_wafers_or_sublots,
                                  QStringList &pass_wafers_or_sublots ) const;

    /**
     * @brief GetPassingValuesRelatedTo extrct passing values in a collection that are related to element in another
     * collection
     * @param all_values all value, even not passing ones
     * @param collection_of_related related values
     * @param[output] passing_values destination that will contain passing values
     * @return true if everything is allright during extraction, otherwise false
     */
    bool GetPassingValuesRelatedTo( const QStringList &all_values,
                                    const QStringList &collection_of_related,
                                    QStringList &passing_values ) const;

signals:
    void    sStartTask(int iType);
    void    sStoppedTask(int iId);
    void    sMoHistoryLogUpdated(const QString&,const QString&);
    void    sMoReportLogUpdated(const QString&,const QString&);

    // GUI update
    void    sUpdateListView(QStringList Options);
    void    sUpdateListViewItem(CGexMoTaskItem *,QString);
    void    sDisplayStatusMessage(QString strText);
    void    sPopupMessage(QString strMessage);
    int     sGetCurrentTableType();
    void    sEnableGexAccess();
};
}
}

#endif
