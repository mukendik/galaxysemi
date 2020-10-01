///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////

#ifndef GEXMO_SCHEDULER_H
#define GEXMO_SCHEDULER_H

#include "ui_mo_scheduler_dialog.h"

// New GUI
// ui dialog box was modified and doesn't include now Q3Header and Q3ListView
// For compatibilité, need to include Qt3Support here because some class use
// this declaration
#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
// YIELDMANDB
#include "gexdb_plugin_base.h"

#include <time.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <QTextStream>
#include <QList>
#include <QDateTime>


// Forward declarations
class GexMoDataPumpTaskData;
class GexMoYieldMonitoringTaskData;
class GexMoYieldMonitoringTaskData_RDB;
class GexMoSpecMonitoringTaskData;
class GexMoReportingTaskData;
class GexMoStatusTaskData;
class GexMoFileConverterTaskData;
class GexMoPickTask;
class GexMoOutlierRemovalTaskData;
class GexMoAutoAdminTaskData;
class CGexGroupOfFiles;
class CGexPatProcessing;
class CGexCompositePatProcessing;
class GexDatabaseEntry;
class GexDbPlugin_SYA_Item;
class GexDbPlugin_SYA_Limitset;

// in classes.h
class CGexRange;

// in db_datakeys.h
class GexDatabaseKeysContent;

// in report_build.h
class CGexFileInGroup;
class CTest;

// in patman_lib.h
class GexTbPatSinf;
class GexTbPatDialog;
class CBinning;

// Hold all data for a task
class CGexMoTaskItem
{
public:
	CGexMoTaskItem();

	// YieldManDb
	int				iTaskId;
	int				iUserId;
	int				iGroupId;
	int				iDatabaseId;
	QString			strDatabaseName;
	int				iPermissions;
	QString			strName;
	int				iTaskType;
	int				iStatus;
	QDateTime		clCreationDate;
	QDateTime		clExpirationDate;
	QDateTime		clLastUpdate;

	bool			bEnabled;

	// Task details
	GexMoDataPumpTaskData *ptDataPump;
	GexMoYieldMonitoringTaskData *ptYield;
	GexMoYieldMonitoringTaskData_RDB *ptYield_RDB;
	GexMoSpecMonitoringTaskData *ptSpec;
	GexMoReportingTaskData *ptReporting;
	GexMoStatusTaskData *ptStatus;
	GexMoFileConverterTaskData *ptConverter;
	GexMoOutlierRemovalTaskData *ptOutlier;
	GexMoAutoAdminTaskData *ptAutoAdmin;
};

class GexMoScheduler: public QDialog, public Ui::monitor_scheduler_basedialog
{
	Q_OBJECT

public:
	GexMoScheduler( QWidget* parent = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
	~GexMoScheduler();

	CGexMoTaskItem *GetProductYieldInfo(QString szProductID,bool bFindGoodBinList=false,CGexMoTaskItem *ptFromTask=NULL);
	CGexMoTaskItem *GetProductYieldInfo_Rdb(QString szProductID,CGexMoTaskItem *ptFromTask=NULL);
	CGexMoTaskItem *GetProductSpecsInfo(QString szProductID,CGexMoTaskItem *ptFromTask=NULL);
	CGexMoTaskItem *GetProductOutlierInfo(QString szProductID,CGexMoTaskItem *ptFromTask=NULL);
	CGexMoTaskItem *GetTaskHandle(int iHandle);
	CGexMoTaskItem *GetStatusTask(void);
	CGexMoTaskItem *GetAutoAdminTask(void);
	void	ExtractStringValue(QString strValue,double &lfValue,double lfInvalid);
	bool	CheckSblYield(QString &strSblFile,CGexRange *pBinCheck,CBinning	*ptBinList,QString &strYieldAlarmMessage,QString &strErrorMessage);
	QString ExecuteDataFileAnalysis(GexDatabaseKeysContent *pKeyContent,QList <int> &cSitesList);
	QString GetBinSummaryString(CGexGroupOfFiles *pGroup,long lTotalBins,bool bHtmlFormat=false,bool bMultiSiteFile=false);
	QString GetBinSummaryString(GexDbPlugin_SYA_Item *pSyaItem, GexDbPlugin_SYA_Limitset * pSya_Limitset, bool bHtmlFormat);
	QString ExecuteYieldTask(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent);
	QString ExecuteYieldTask_Rdb(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent);
	QString ExecuteYieldTask_Rdb_StdYield(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent);
	QString ExecuteYieldTask_Rdb_SYA(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent);
	QString ExecuteSpecsTask(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent);
	QString ExecuteOutlierRemovalTask(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent,bool bTriggerFile,CGexPatProcessing	&cTriggerFields);
	QString ExecutePatProcessing(GexDatabaseEntry *pDatabaseEntry,CGexPatProcessing &cFields,QString &strErrorMessage);
	QString ExecuteWaferExportTask(GexDatabaseEntry *pDatabaseEntry, CGexMoTaskItem *ptTask,GexDatabaseKeysContent *pKeyContent,bool bTriggerFile,CGexPatProcessing	&cTriggerFields);
	QString ExecuteWaferExport(GexDatabaseEntry *pDatabaseEntry,CGexPatProcessing &cFields,QString &strErrorMessage);

	QString ExecuteCompositePatProcessing(GexDatabaseEntry *pDatabaseEntry,CGexCompositePatProcessing &cFields,QString &strErrorMessage);
	bool	CompositePat_MergeDataFiles(CGexCompositePatProcessing &cFields,QString &strErrorMessage);
	void	DisplayStatusMessage(QString strText="");
	bool	ReadConfigFile(void);
	bool	WriteConfigFile(void);

	// New GUI
	void	fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText, QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);

private:
	void	RunTaskScheduler(bool bRun);
	// YIELMANDB
	// All tasks were identified by iTaskId
	CGexMoTaskItem *FindTaskInList(int iTaskId);
	void	DeleteTaskInList(int iTaskId);
	void	DuplicateTaskInList(int iTaskId,QString strCopyName);

	void	LoadTasks(bool bForceReload=true);
	CGexMoTaskItem*	LoadTaskSectionDataPump();
	CGexMoTaskItem*	LoadTaskSectionYieldMonitoring();
	GexMoYieldMonitoringTaskData *	LoadTaskSectionYieldMonitoring_RDB_OneRule();
	CGexMoTaskItem*	LoadTaskSectionYieldMonitoring_RDB();
	CGexMoTaskItem*	LoadTaskSectionSpecMonitoring();
	CGexMoTaskItem*	LoadTaskSectionReporting();
	CGexMoTaskItem*	LoadTaskSectionStatus();
	CGexMoTaskItem*	LoadTaskSectionFileConverter();
	CGexMoTaskItem*	LoadTaskSectionOutlierRemoval();
	CGexMoTaskItem*	LoadTaskSectionAutoAdmin();
	void	SaveTasks(void);
	void	SaveTaskSectionDataPump(GexMoDataPumpTaskData *ptDataPump);
	void	SaveTaskSectionYieldMonitoring(GexMoYieldMonitoringTaskData *ptYield);
	void	SaveTaskSectionYieldMonitoring_RDB_OneRule(GexMoYieldMonitoringTaskData *ptYield);
	void	SaveTaskSectionYieldMonitoring_RDB(GexMoYieldMonitoringTaskData_RDB *ptYield);
	void	SaveTaskSectionSpecMonitoring(GexMoSpecMonitoringTaskData *ptSpec);
	void	SaveTaskSectionReporting(GexMoReportingTaskData *ptReporting);
	void	SaveTaskSectionStatus(GexMoStatusTaskData *ptStatus);
	void	SaveTaskSectionFileConverter(GexMoFileConverterTaskData *ptConverter);
	void	SaveTaskSectionOutlierRemoval(GexMoOutlierRemovalTaskData *ptAutoAdmin);
	void	SaveTaskSectionAutoAdmin(GexMoAutoAdminTaskData *ptAutoAdmin);

	// YIELMANDB
	// Database update
	void	LoadDbTasks(bool bForceReload=true);
	void	LoadDbTaskSectionDataPump(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionYieldMonitoring(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionYieldMonitoring_RDB(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionSpecMonitoring(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionReporting(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionStatus(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionFileConverter(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionOutlierRemoval(CGexMoTaskItem *	ptTask);
	void	LoadDbTaskSectionAutoAdmin(CGexMoTaskItem *	ptTask);
	void	SaveDbTasks(CGexMoTaskItem *	ptTask=NULL);
	void	SaveDbTaskSectionDataPump(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionYieldMonitoring_RDB_OneRule(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionYieldMonitoring_RDB(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionSpecMonitoring(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionReporting(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionStatus(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionFileConverter(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionOutlierRemoval(CGexMoTaskItem *	ptTask);
	void	SaveDbTaskSectionAutoAdmin(CGexMoTaskItem *	ptTask);


	void	DeleteTasksList(void);
	void	CreateTaskDataPump(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskYieldMonitoring(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskYieldMonitoring_RDB(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskSpecMonitoring(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskReporting(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskStatus(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskFileConverter(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskOutlierRemoval(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	CreateTaskAutoAdmin(CGexMoTaskItem *ptTaskItem=NULL,QTableWidgetItem *ptListViewItem=NULL);
	void	UpdateListViewItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem,bool bCreateTask=true);
	void	UpdateListViewDataPumpItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem,bool bCreateTask=true);
	void	UpdateListViewYieldItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem=NULL,bool bCreateTask=true);
	void	UpdateListViewYieldItem_RDB(CGexMoTaskItem *ptTaskItem,bool bCreateTask=true);
	void	UpdateListViewSpecItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem=NULL,bool bCreateTask=true);
	void	UpdateListViewReportingItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem=NULL,bool bCreateTask=true);
	void	UpdateListViewStatusItem(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem=NULL,bool bCreateTask=true);
	void	UpdateListViewConverterItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask);
	void	UpdateListViewOutlierItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask);
	void	UpdateListViewAutoAdminItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask=true);
	bool	AddDateFrequency(QDateTime *pDateTime,int iFrequency,int iDayOfWeek=-1);
	QString ExecuteDataPumpTask(CGexMoTaskItem *ptTask);
	QString ExecuteReportingTask(CGexMoTaskItem *ptTask);
	void	ParameterFailingSpecs(GexMoSpecMonitoringTaskData *ptSpecTask,CGexFileInGroup *pFile,CTest *ptTestCell,GexDatabaseKeysContent *pKeyContent,QString &strEmailBody);
	QString ExecuteFileConverterTask(CGexMoTaskItem *ptTask);
	QString ExecuteAutoAdminTask(CGexMoTaskItem *ptTask);
	int		CheckPatYieldLoss(CGexPatProcessing &cFields,double lfYieldLoss,long lPatFailingParts,long lTotalParts,int lDistributionMismatch,GexDatabaseKeysContent *pKeyContent,QString &strErrorMessage);
	void	CreatePatLogFile(CGexPatProcessing &cFields,GexTbPatDialog *cPat,QString &strErrorMessage);
	void	CreateWaferExportLogFile(CGexPatProcessing &cFields,GexTbPatDialog *cPat, QString & strDestFile, QString &strErrorMessage);
	void	CreateCompositePatLogFile(CGexCompositePatProcessing &cFields);
	void	CloseCompositePatLogFile(CGexCompositePatProcessing &cFields);
	void	CreateCompositePatReportFile(CGexCompositePatProcessing &cFields);
	void	CloseCompositePatReportFile(CGexCompositePatProcessing &cFields);
	QString getOutputReportFormat(int iFormat);
	bool	Create_CompositePat_ResultFile(CGexCompositePatProcessing &cFields,QString &strErrorMessage);
	void	InsertDataFileLoop(CGexMoTaskItem *ptTask);
	void	ConvertDataFileLoop(CGexMoTaskItem *ptTask);
	void	GetListOfFiles(const QString & strRootPath, QString strSubPath, const QString & strExtensions, QStringList & strlDataFiles, bool bScanSubFolders, bool bClearList=true);

	void	BuildRecipeFileFullName(CGexPatProcessing &cFields,GexDatabaseKeysContent *pKeyContent);
	bool	MergeInputFiles(GexDatabaseEntry *pDatabaseEntry,CGexPatProcessing &cFields,QString &strTestDataFile,QString &strErrorMessage);

	void	ExecuteStatusTask(void);
	time_t	iLastDataReceived;
	bool	bProcessingTaskList;	// Flag to avoid recursive calls into 'Task Processing' functions.

	QDateTime m_dtTaskLoadTimestamp;	// Last Date&Time task list was reloaded...
	QTextStream hTasks;
	QTimer	timerMonitoring;
	QTimer	timerRenameFiles;
	bool	m_ActiveScheduler;			// Scheduler mode true=Acvite & running, false= paused
	bool	m_bRetrieveListOfDataFiles;	// Used to force reading list of files to insert in database (DataPump)
	bool	m_bMonitoringTimerStarted;	// Set to TRUE once the monitoring timer has been started.

	// List of Task items.
	QList<CGexMoTaskItem*> pGexMoTasksList;
	// New GUI (sortable)
	QMap<QString,int>		m_mapSortable;
	int						m_nIndexInvalidTask;

	// List of temporary files to rename
	QStringList	strFilesToRename;

	// Shell call over alarm: Function definition + List of Alarm types that can call Shell script
	bool LaunchAlarmShell(int iAlarmType,int iSeverity,int iAlarmCount,GexDatabaseKeysContent *pKeyContent);
	enum ShellAlarmType
	{
		ShellYieldAlarm,		// Yield alarm
		ShellParameterAlarm,	// Parametric test alarm
		ShellPatAlarm			// PAT alarm
	};
	
public slots:
	void	OnRunScheduler(void);
	void	OnPauseScheduler(void);

	void	OnNewTask(void);
	void	OnTaskProperties(bool bCreateNewTask=false);
	void	OnDuplicateTask(void);
	void	OnRunTask(void);
	void	OnDeleteTask(void);
	void	OnCheckRenameFiles(void);
	void	OnCheckScheduler(void);

	// New GUI (sortable)
	void	OnSelectItem(void);
	void	OnSelectHeader(int nColumn);

};
#endif
