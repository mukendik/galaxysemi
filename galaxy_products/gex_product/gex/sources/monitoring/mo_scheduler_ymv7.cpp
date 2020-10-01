///////////////////////////////////////////////////////////
// Examinator Monitoring: Scheduler page
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef unix
#include <unistd.h>
#include <errno.h>
#elif defined(WIN32)
  #include <io.h>
#endif

#include <qradiobutton.h>
#include <qregexp.h>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include "message.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "mo_datapump.h"
#include "mo_yieldcheck.h"
#include "mo_rdb_yieldcheck.h"
#include "mo_speccheck.h"
#include "mo_reporting.h"
#include "mo_status.h"
#include "mo_picktask.h"
#include "mo_outlier_removal.h"
#include "mo_autoadmin.h"
#include "mo_file_converter.h"
#include "mo_scheduler.h"
#include "gex_constants.h"
#include "gexmo_constants.h"

// YIELDMANDB
#include "yieldmandb_login.h"
#include "db_admin.h"
#include <QSqlError>

#include <QListIterator>


// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"


// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern CGexSkin			gGexSkin;			// holds the skin settings

// in browser_dialog.cpp
extern void	gexEmptyTemporaryFile(bool bFullErase);

// in main.cpp
extern void				WriteDebugMessageFile(QString strMessage);
extern char	*szAppFullName;
extern GexMainwindow	*pGexMainWindow;

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
	AddNewUrl(strString,false);
}

///////////////////////////////////////////////////////////
// Small dialog box to select the Task type to create...
///////////////////////////////////////////////////////////
GexMoPickTask::GexMoPickTask( QWidget* parent, bool modal, Qt::WFlags fl) : QDialog(parent, fl)
{
	setupUi(this);
	setModal(modal);

	// Set Examinator skin
	gGexSkin.applyPalette(this);

	QObject::connect(pushButtonDataPump,				SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonYieldMonitoring,		SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonYieldMonitoring_RDB,		SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonParameterMonitoring,	SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonReporting,				SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonStatus,					SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonFileConverter,			SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonOutlier,				SIGNAL(pressed()), this, SLOT(onTaskSelected()));
    QObject::connect(pushButtonAutoAdmin,				SIGNAL(pressed()), this, SLOT(onTaskSelected()));

	if((ReportOptions.uOptionalModules & GEX_OPTIONAL_MODULE_PATMAN) || (ReportOptions.lProductID == GEX_DATATYPE_GEX_PATMAN))
	{
		// Enable PAT-Man support
		pushButtonOutlier->show();
	}
	else
	{
		// Disable PAT-Man support
		pushButtonOutlier->hide();
	}

	// If NOT RDB, hide RDB button
	if(ReportOptions.uOptionalModules & GEX_OPTIONAL_MODULE_PLUGIN)
	{
		// Enable RDB support
		pushButtonParameterMonitoring->show();
	}
	else
	{
		// Disable RDB support
		pushButtonParameterMonitoring->hide();
    }

	// Clear variables
	iTaskSelected = -1;
}

// Task selected, close Dialog box!
void GexMoPickTask::onTaskSelected(void)
{
	// Check which button is selected
	if(pushButtonDataPump->isDown())
		iTaskSelected = GEXMO_TASK_DATAPUMP;
	if(pushButtonYieldMonitoring->isDown())
		iTaskSelected = GEXMO_TASK_YIELDMONITOR;
	if(pushButtonYieldMonitoring_RDB->isDown())
		iTaskSelected = GEXMO_TASK_YIELDMONITOR_RDB;
	if(pushButtonParameterMonitoring->isDown())
		iTaskSelected = GEXMO_TASK_SPECMONITOR;
	if(pushButtonReporting->isDown())
		iTaskSelected = GEXMO_TASK_REPORTING;
	if(pushButtonStatus->isDown())
		iTaskSelected = GEXMO_TASK_STATUS;
	if(pushButtonFileConverter->isDown())
		iTaskSelected = GEXMO_TASK_CONVERTER;
	if(pushButtonOutlier->isDown())
		iTaskSelected = GEXMO_TASK_OUTLIER_REMOVAL;
	if(pushButtonAutoAdmin->isDown())
		iTaskSelected = GEXMO_TASK_AUTOADMIN;

	done(1);
}

///////////////////////////////////////////////////////////
// Task Item initialization
///////////////////////////////////////////////////////////
CGexMoTaskItem::CGexMoTaskItem()
{
	iTaskId		= -1;
	iUserId		= -1;
	iGroupId	= -1;
	iDatabaseId = -1;
	iPermissions = 0;
	iStatus		= -1;

	bEnabled	= true;

	ptDataPump		= NULL;
	ptYield			= NULL;
	ptYield_RDB		= NULL;
	ptSpec			= NULL;
	ptReporting		= NULL;
	ptStatus		= NULL;
	ptConverter		= NULL;
	ptOutlier		= NULL;
	ptAutoAdmin		= NULL;
}

///////////////////////////////////////////////////////////
// Task Scheduler window
///////////////////////////////////////////////////////////
GexMoScheduler::GexMoScheduler( QWidget* parent, bool modal, Qt::WFlags fl) : QDialog(parent, fl)
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

    move(0,0);

	QObject::connect(buttonNewTask,			SIGNAL(clicked()), this, SLOT(OnNewTask()));
    QObject::connect(buttonTaskProperties,	SIGNAL(clicked()), this, SLOT(OnTaskProperties()));
    QObject::connect(buttonDuplicateTask,	SIGNAL(clicked()), this, SLOT(OnDuplicateTask()));
    QObject::connect(buttonRunTask,			SIGNAL(clicked()), this, SLOT(OnRunTask()));
    QObject::connect(buttonDeleteTask,		SIGNAL(clicked()), this, SLOT(OnDeleteTask()));
    QObject::connect(buttonPauseScheduler,	SIGNAL(clicked()), this, SLOT(OnPauseScheduler()));
    QObject::connect(buttonRunScheduler,	SIGNAL(clicked()), this, SLOT(OnRunScheduler()));


	// YIELDMANDB
	// New GUI
	QObject::connect(tableWidgetTasksListDataPump,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListReporting,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListParameterSpecs,	SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListConverter,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListOutlierRemoval,	SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListYield ,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListYieldRdb ,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));
	QObject::connect(tableWidgetTasksListStatus ,			SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnTaskProperties()));

	QObject::connect((QObject*) tableWidgetTasksListDataPump->horizontalHeader() ,			SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListReporting->horizontalHeader() ,			SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListParameterSpecs->horizontalHeader() ,	SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListConverter->horizontalHeader() ,			SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListOutlierRemoval->horizontalHeader() ,	SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListYield->horizontalHeader() ,				SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListYieldRdb->horizontalHeader() ,			SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));
	QObject::connect((QObject*) tableWidgetTasksListStatus->horizontalHeader() ,			SIGNAL(sectionClicked ( int)), this, SLOT(OnSelectHeader(int)));

	// Sorting: forced Ascending on 1st column (Task type)
	/*ListView->setSorting(0);	// YIELDMANDB No Qt3ListView

	// Set minimum size for the columns
	ListView->setColumnWidth (0,100);	// Task type
	ListView->setColumnWidth (1,150);	// Title
	ListView->setColumnWidth (2,80);	// Frequency
	ListView->setColumnWidth (3,80);	// Alarm condiction
	ListView->setColumnWidth (4,200);	// Emails
	*/

	// Load tasks from disk
	LoadDbTasks();

	// Clear status message window.
	DisplayStatusMessage();

	// Timer used to check for "Rename of temporary files" (resolution: .5 sec)
	connect( &timerRenameFiles, SIGNAL(timeout()),this, SLOT(OnCheckRenameFiles(void)) );

	// Write into Log file
	pGexMainWindow->OnAppendMoHistoryLog("","Launching",szAppFullName);

	// Flag set to 'true' when currenty processing tasks.
	bProcessingTaskList=false;

	// Flag set to 'true' when the monitoring timer has been started
	m_bMonitoringTimerStarted = false;

	// Each time the solution is launched, update the Intranet web site so to always be up-to-date.
	ExecuteStatusTask();

	// Start scheduler timer (resolution: 1 minute, unless we are in Y123 mode)
	connect( &timerMonitoring, SIGNAL(timeout()),this, SLOT(OnCheckScheduler(void)) );

	// Force Running status at startup: so no task is taking place when launched in GUI mode.
    RunTaskScheduler(true);
}

///////////////////////////////////////////////////////////
// Task Scheduler Destructor function.
///////////////////////////////////////////////////////////
GexMoScheduler::~GexMoScheduler()
{
	// Write into Log file
	pGexMainWindow->OnAppendMoHistoryLog("","Closing",szAppFullName);
}

///////////////////////////////////////////////////////////
// Start/Stop Task scheduler
///////////////////////////////////////////////////////////
void GexMoScheduler::RunTaskScheduler(bool bRun)
{
	// Keep track of scheduler status.
	m_ActiveScheduler = bRun;

	// YIELDMANDB
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
	{
		// Do nothing
	}
	else
	// Check if a user is connected
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
	&& !pGexMainWindow->m_pYieldManDb->IsConnected())
	{
		// Do nothing
	}
	else
	{
		// Keep track of scheduler status.
		m_ActiveScheduler = bRun;
	}


	// Update GUI accordingly
	if(m_ActiveScheduler)
	{
		// RUN task scheduler (Green text)
		DisplayStatusMessage("<b><font color=\"#009933\">* Scheduler is Running *</font></b>");

		// Update GUI
		buttonRunScheduler->setChecked(true);
		buttonPauseScheduler->setChecked(false);
	}
	else
	{
		// Requesting PAUSE task scheduler (Orange text)
		DisplayStatusMessage("<b><font color=\"#FF6600\">* PAUSE requested...</font></b>");

		// Update GUI
		buttonRunScheduler->setChecked(false);
		buttonPauseScheduler->setChecked(true);
	}

	// Call Scheduler loop now just to have GUI fully updated!
    QTimer::singleShot(0, this, SLOT(OnCheckScheduler(void)));
}

///////////////////////////////////////////////////////////
// GUI button for Starting Task scheduler
///////////////////////////////////////////////////////////
void GexMoScheduler::OnRunScheduler(void)
{
	RunTaskScheduler(true);
}

///////////////////////////////////////////////////////////
// GUI button for Stopping Task scheduler
///////////////////////////////////////////////////////////
void GexMoScheduler::OnPauseScheduler(void)
{
	RunTaskScheduler(false);
}

///////////////////////////////////////////////////////////
// Timer based: check if temporary files have to be renamed...
///////////////////////////////////////////////////////////
void GexMoScheduler::OnCheckRenameFiles(void)
{
    // If none,
	if(strFilesToRename.count() <= 0)
		return;

	// Try to delete destination file + rename temporary file
	QStringList	strFailures;
	QString strSource,strDest;
	QDir	cDir;
	int	iIndex;
	bool	bSuccess;
    for ( QStringList::Iterator it = strFilesToRename.begin(); it != strFilesToRename.end(); ++it )
	{
        strSource = strDest = *it;
		bSuccess = false;
		if(strDest.endsWith(GEX_TEMPORARY_HTML))
		{
			// This is a valid temporary file name.
			iIndex = strDest.lastIndexOf(GEX_TEMPORARY_HTML);
			strDest = strDest.left(iIndex);

			// Make sure destination can be erased
			#ifdef unix
					chmod(strDest.toLatin1().constData(),0777);
			#else
					_chmod(strDest.toLatin1().constData(),_S_IREAD | _S_IWRITE);
			#endif

			if(QFile::exists(strSource))
			{
				// Erase destination
				cDir.remove(strDest);

				// Rename source to destination
				if(cDir.rename(strSource,strDest) == true)
					bSuccess = true;	// Success: remove entry from the list
			}
			else
				bSuccess = true;	// Source doesn't exist!...maybe we already processed it.
        }

		// Check if successful rename...if not, add failing file to the new list.
		if(bSuccess == false)
			strFailures += strSource;
    }

	// If some files couldn't be renamed, update the new list of files to rename.
	strFilesToRename = strFailures;

    // If need to reshedule a 'rename' session, trigger timer,
	if(strFilesToRename.count() > 0)
		timerRenameFiles.start(500,true);
}

///////////////////////////////////////////////////////////
// Timer based: check for tasks to execute...called every minute
///////////////////////////////////////////////////////////
void GexMoScheduler::OnCheckScheduler(void)
{
	QDateTime	cCurrentDateTime = QDateTime::currentDateTime();

	// Debug message
	QString strString;
	strString = "---- GexMoScheduler::OnCheckScheduler(";
	strString += cCurrentDateTime.toString("hh:mm:ss")+"): ";
	WriteDebugMessageFile(strString);

	CGexMoTaskItem *ptTask;
	QString	strMessage;
	QString strErrorTitle, strErrorTask, strError;
	QTime cTime=QTime::currentTime();
	QDateTime cExecutionDateTime;

	// Check if node fully ready. If not, give it some time to get ready.
	if(pGexMainWindow->eClientState != GexMainwindow::eState_NodeReady)
	{
	    QTimer::singleShot(1000, this, SLOT(OnCheckScheduler(void)));
		return;
	}

	// Now that the client node is fully ready, make sure the monitoring timer has been started, else start it
	if(!m_bMonitoringTimerStarted)
	{
		if(ReportOptions.bY123WebMode)
			timerMonitoring.start(1000);	// 1 second (1,000 Msec.)
		else
			timerMonitoring.start(60000);	// 1 minute (60,000 Msec.)
		m_bMonitoringTimerStarted = true;
	}

	if(bProcessingTaskList == true)
		return;	// ignore timer calls until tasks under execution not completely finished!

	// Reload tasks if file timestamp has changed and is old enough( keep 5 secs. delay to void sharing issues)!
	//LoadTasks(false);

	// YIELDMANDB
	// Check if the tasks list is loaded
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT))
	{
		if(pGexMoTasksList.count() == 0)
			LoadDbTasks();
	}
	else
		LoadDbTasks();
	// If User PAUSE, do not process next task until PAUSE released!
	if(m_ActiveScheduler == false)
	{
		// Ensure user knows scheduler is paused (Red text)
		DisplayStatusMessage("<b><font color=\"#FF0000\">* SCHEDULER IS PAUSED *</font></b>");

		return;
	}

    // Enter into 'task processing' section
	bProcessingTaskList = true;

	// If we start a new day, make sure HTML intranet web page is created, even if no files processed yet.
	static QDate cLastIntranetUpdate(1900,1,1);
	QDate cCurrentDate;
	cCurrentDate = QDate::currentDate();
	if((cLastIntranetUpdate.daysTo(cCurrentDate) > 1) || (cLastIntranetUpdate.dayOfYear() != cCurrentDate.dayOfYear()))
	{
		// Force to create the Web pages of the day.
		ExecuteStatusTask();
		cLastIntranetUpdate = cCurrentDate;
	}

	unsigned int uiIndex;
	for(uiIndex=0; uiIndex<pGexMoTasksList.count(); uiIndex++)
	{
		ptTask = pGexMoTasksList.at(uiIndex);
		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_DATAPUMP:
				// YIELDMANDB
				// Check if yieldman is connected
				if( pGexMainWindow->m_pYieldManDb
				&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
				&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
				{
					// Output message to the console
					strError = "*INFO* YieldMan lost the acreditation for this task";
					strErrorTitle = ptTask->ptDataPump->strTitle;
					strErrorTask = "DataPump";
					break;
				}

				// Check if load task from YieldManDb
				if( pGexMainWindow->m_pYieldManDb
				&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT))
				{
					QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
					QString strQuery;

					strQuery = "SELECT * FROM ym_databases WHERE database_id="+QString::number(ptTask->iDatabaseId);
					strQuery+= " WHERE node_id="+QString::number(pGexMainWindow->m_pYieldManDb->m_nNodeId);
					if(clQuery.exec(strQuery))
					{
						if(!clQuery.first())
						{
							// This YieldMan lost the acreditation for this task
							// Ignore

							// Output message to the console
							strError = "*INFO* YieldMan lost the acreditation for this task";
							strErrorTitle = ptTask->ptDataPump->strTitle;
							strErrorTask = "DataPump";

							break;
						}
					}
				}
				// If a Time window is defined (AND task not Forced to be executed), check if we fall into it!
				if((ptTask->ptDataPump->bExecutionWindow) &&
					(ptTask->ptDataPump->tLastExecuted != 1))
				{
					if((cTime < ptTask->ptDataPump->cStartTime) ||
						(cTime > ptTask->ptDataPump->cStopTime))
						break;	// NOT in window, then quiet exit!
				}

				// If Manual execution...tell user that file created less than 1 minute ago won't be processed
				// Unless we are in Y123 mode
				if((ptTask->ptDataPump->tLastExecuted == 1) && !ReportOptions.bY123WebMode)
				{
					// Display warning.
                    GS::Gex::Message::information(
                        szAppFullName,
                        "*INFO* Files received less than "
                        "1 minute ago won't be processed\n"
                        "(This is to avoid reading incomplete files)");
				}

				// If we are in the time window (or it's not activated), check frequency.
				cExecutionDateTime.setTime_t(ptTask->ptDataPump->tLastExecuted);

				// Compute the new date adding the frequency period
				if((AddDateFrequency(&cExecutionDateTime,ptTask->ptDataPump->iFrequency,ptTask->ptDataPump->iDayOfWeek) == false) && (ptTask->ptDataPump->tLastExecuted != 1))
					break;	// Day of week mismatch...so not the right day to execute this task!...Ignored if Execution forced

				// Check if we have not reached the execution time yet...
				if(cCurrentDateTime < cExecutionDateTime)
                    break;

				// Execute Data pump task
				ExecuteDataPumpTask(ptTask);

				// Update the 'Last executed' timestamp
				ptTask->ptDataPump->tLastExecuted = time(NULL);
				break;

			case GEXMO_TASK_REPORTING:
				// If a Time window is defined (AND task not Forced to be executed), check if we fall into it!
				if((ptTask->ptReporting->bExecutionWindow) &&
					(ptTask->ptReporting->tLastExecuted != 1))
				{
					if((cTime < ptTask->ptReporting->cStartTime) ||
						(cTime > ptTask->ptReporting->cStopTime))
						break;	// NOT in window, then quiet exit!
				}

				// If we are in the time window (or it's not activated), check frequency.
				cExecutionDateTime.setTime_t(ptTask->ptReporting->tLastExecuted);

				// Compute the new date adding the frequency period
				if((AddDateFrequency(&cExecutionDateTime,ptTask->ptReporting->iFrequency,ptTask->ptReporting->iDayOfWeek) == false) && (ptTask->ptReporting->tLastExecuted != 1))
					break;	// Day of week mismatch...so not the right day to execute this task!...Ignored if Execution forced

				// Check if we have not reached the execution time yet...
				if(cCurrentDateTime < cExecutionDateTime)
                    break;

				// Execute Reporting task
				strError = ExecuteReportingTask(ptTask);

				// Output message to the console (if error occured)
				if(strError.isEmpty() == false)
				{
					strErrorTitle = ptTask->ptReporting->strTitle;
					strErrorTask = "Reporting";
				}

				// Update the 'Last executed' timestamp
				ptTask->ptReporting->tLastExecuted = time(NULL);
				break;

			case GEXMO_TASK_CONVERTER:
				// If we are in the time window (or it's not activated), check frequency.
				cExecutionDateTime.setTime_t(ptTask->ptConverter->tLastExecuted);

				// Compute the new date adding the frequency period
				if((AddDateFrequency(&cExecutionDateTime,ptTask->ptConverter->iFrequency,ptTask->ptConverter->iDayOfWeek) == false) && (ptTask->ptConverter->tLastExecuted != 1))
					break;	// Day of week mismatch...so not the right day to execute this task!...Ignored if Execution forced

				// Check if we have not reached the execution time yet...
				if(cCurrentDateTime < cExecutionDateTime)
                    break;

				// Execute File Converter task
				strError = ExecuteFileConverterTask(ptTask);

				// Output message to the console (if error occured)
				if(strError.isEmpty() == false)
				{
					strErrorTitle = ptTask->ptConverter->strTitle;
					strErrorTask = "File Conversion";
				}

				// Update the 'Last executed' timestamp
				ptTask->ptConverter->tLastExecuted = time(NULL);
				break;

			case GEXMO_TASK_AUTOADMIN:
				// If task not Forced to be executed, check if we have past the starting time!
				if(ptTask->ptAutoAdmin->tLastExecuted != 1)
				{
					if(cTime < ptTask->ptAutoAdmin->cStartTime)
						break;	// NOT yet reached the starting time, then quiet exit!

					// If task already executed once today (same day of year), then ignore.
					cExecutionDateTime.setTime_t(ptTask->ptAutoAdmin->tLastExecuted);
					QDate cCurrentDate = cCurrentDateTime.date();
					QDate cLastExecutionDate = cExecutionDateTime.date();
					if(cCurrentDate.dayOfYear()  == cLastExecutionDate.dayOfYear())
						break;
				}

				// Execute autoAdmin task
				strError = ExecuteAutoAdminTask(ptTask);

				// Output message to the console (if error occured)
				if(strError.isEmpty() == false)
				{
					strErrorTitle = ptTask->ptAutoAdmin->strTitle;
					strErrorTask = "AutoAdmin";
				}

				// Update the 'Last executed' timestamp
				ptTask->ptAutoAdmin->tLastExecuted = time(NULL);
				break;
		}

		// If User PAUSE, do not process next task until PAUSE released!
		if(m_ActiveScheduler == false)
		{
			// Ensure user knows scheduler is paused (Red text)
			DisplayStatusMessage("<b><font color=\"#FF0000\">* SCHEDULER IS PAUSED *</font></b>");
			break;
		}

		// Output message to the console (if error occured)
		if(strError.isEmpty() == false)
		{
			pGexMainWindow->OnAppendMoHistoryLog(strMessage,strErrorTask,strErrorTitle);
			pGexMainWindow->OnAppendMoHistoryLog(strError);

			// Clear error.
			strError = "";
		}
	};

	// If we have one or more files to rename, do it!
	if(strFilesToRename.count() > 0)
		OnCheckRenameFiles();

	// If temporary files to erase, do it now (could have been intermediate STDF or while unziping files for .CSL scripts executions...)
	gexEmptyTemporaryFile(true);

	// Allow next timer tick to enter in this function again.
	bProcessingTaskList=false;
}

///////////////////////////////////////////////////////////
// Return handle to a task structure (name specified by caller).
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::FindTaskInList(int iTaskId)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);
	CGexMoTaskItem * ptTask = NULL;

	lstIteratorTask.toFront();

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task identified by iTaskId number
		if(ptTask->iTaskId == iTaskId)
		{
			return ptTask;
		}
	};

	// Task title not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to a task structure for monitoring yield
// of a given ProductID.
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetProductYieldInfo(QString szProductID,bool bFindGoodBinList, CGexMoTaskItem *ptFromTask)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

	CGexMoTaskItem *	ptTask = NULL;
	QString				strPattern;
	QRegExp				rx("",FALSE);	// NOT case sensitive.

	// If looking from 1st matching task
	if(ptFromTask == NULL)
		lstIteratorTask.toFront();
	else
		// Looking for next matching task
		lstIteratorTask.findNext(ptFromTask);

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task can be in disable mode
		// Then ignore it
		if(!ptTask->bEnabled)
			continue;

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_YIELDMONITOR:
				// If looking for the Good Bins list only, check this first
				if(bFindGoodBinList && (ptTask->ptYield->iBiningType != 0))
					break;	// This Yield definition is NOT for Good bins...

				strPattern = ptTask->ptYield->strProductID;
				// Replace ',' or ';' with '|' pipe OR character.
				strPattern.replace( ",", "|" );
				strPattern.replace( ";", "|" );

				// remove any space (as the '|' doesn't work it is has leading spaces!).
				strPattern.replace( " ", "" );

				rx.setPattern(strPattern);
				// If wildcar used, set its support.
				if(strPattern.find("*") >= 0 || strPattern.find("?") >= 0)
					rx.setWildcard(true);
				else
					rx.setWildcard(false);

				if(rx.search(szProductID) >= 0)
					return ptTask;	// Found Yield task  for this ProductID
				break;
		}
	};
	// Task title not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to a task structure for monitoring yield
// of a given ProductID. RDB version.
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetProductYieldInfo_Rdb(QString szProductID,CGexMoTaskItem *ptFromTask)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

	CGexMoTaskItem *	ptTask = NULL;

	// If looking from 1st matching task
	if(ptFromTask == NULL)
		lstIteratorTask.toFront();
	else
		// Looking for next matching task
		lstIteratorTask.findNext(ptFromTask);

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task can be in disable mode
		// Then ignore it
		if(!ptTask->bEnabled)
			continue;

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_YIELDMONITOR_RDB:
				QList <GexMoYieldMonitoringTaskData *>::iterator itRules;
				GexMoYieldMonitoringTaskData *ptYield;
				if(ptTask->ptYield_RDB->cYield_SYA_Rules.count() > 0)
				{
					for (itRules = ptTask->ptYield_RDB->cYield_SYA_Rules.begin(); itRules != ptTask->ptYield_RDB->cYield_SYA_Rules.end(); ++itRules)
					{
						// Handle to Rule
						ptYield = *itRules;

						// Check if right Product
						if(ptYield->strProductID == szProductID)
							return ptTask;	// Found Yield task  for this ProductID
					}
				}
				break;
		}
	};
	// Task title not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to a task structure for PAT-Man / Outlier identification
// of a given ProductID.
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetProductOutlierInfo(QString szProductID, CGexMoTaskItem *ptFromTask)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

	CGexMoTaskItem *	ptTask	= NULL;
	QString				strPattern;
	QRegExp				rx("",FALSE);	// NOT case sensitive.

	// If no Product defined, force it t 'default'
	if(szProductID.isEmpty())
		szProductID = "default";

	// If looking from 1st matching task
	if(ptFromTask == NULL)
		lstIteratorTask.toFront();
	else
		// Looking for next matching task
		lstIteratorTask.findNext(ptFromTask);

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task can be in disable mode
		// Then ignore it
		if(!ptTask->bEnabled)
			continue;

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_OUTLIER_REMOVAL:
				strPattern = ptTask->ptOutlier->strProductID;
				// Replace ',' or ';' with '|' pipe OR character.
				strPattern.replace( ",", "|" );
				strPattern.replace( ";", "|" );

				// remove any space (as the '|' doesn't work it is has leading spaces!).
				strPattern.replace( " ", "" );

				rx.setPattern(strPattern);
				// If wildcar used, set its support.
				if(strPattern.find("*") >= 0 || strPattern.find("?") >= 0)
					rx.setWildcard(true);
				else
					rx.setWildcard(false);

				if(rx.search(szProductID) >= 0)
					return ptTask;	// Found Yield task  for this ProductID
				break;
		}
	};
	// Task title not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to a task structure for monitoring Specs
// of a given ProductID.
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetProductSpecsInfo(QString szProductID, CGexMoTaskItem *ptFromTask)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

	CGexMoTaskItem *	ptTask = NULL;
	QString				strPattern;
	QRegExp				rx("",FALSE);	// NOT case sensitive.

	// If looking from 1st matching task
	if(ptFromTask == NULL)
		lstIteratorTask.toFront();
	else
		// Looking for next matching task
		lstIteratorTask.findNext(ptFromTask);

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task can be in disable mode
		// Then ignore it
		if(!ptTask->bEnabled)
			continue;

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_SPECMONITOR:
				strPattern = ptTask->ptSpec->strProductID;
				// Replace ',' or ';' with '|' pipe OR character.
				strPattern.replace( ",", "|" );
				strPattern.replace( ";", "|" );

				// remove any space (as the '|' doesn't work it is has leading spaces!).
				strPattern.replace( " ", "" );

				rx.setPattern(strPattern);
				// If wildcar used, set its support.
				if(strPattern.find("*") >= 0 || strPattern.find("?") >= 0)
					rx.setWildcard(true);
				else
					rx.setWildcard(false);

				if(rx.search(szProductID) >= 0)
					return ptTask;	// Found task for this ProductID
				break;
		}
	};
	// Task title not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to a given task structure
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetTaskHandle(int iHandle)
{
	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);
	CGexMoTaskItem * ptTask = NULL;

	lstIteratorTask.toFront();

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		// YIELDMANDB
		// Task can be in disable mode
		// Then ignore it
		if(!ptTask->bEnabled)
			continue;

		if(ptTask->iTaskType == iHandle)
			return ptTask;	// found it!
	};

	// Task not found!
	return NULL;
}

///////////////////////////////////////////////////////////
// Return handle to the Status task structure
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetStatusTask(void)
{
	return GetTaskHandle(GEXMO_TASK_STATUS);
}

///////////////////////////////////////////////////////////
// Return handle to the Auto Admin task structure
///////////////////////////////////////////////////////////
CGexMoTaskItem *GexMoScheduler::GetAutoAdminTask(void)
{
	return GetTaskHandle(GEXMO_TASK_AUTOADMIN);
}



///////////////////////////////////////////////////////////
// Delete given task structure from list (name specified by caller).
///////////////////////////////////////////////////////////
void	GexMoScheduler::DeleteTaskInList(int iTaskId)
{
    // Find
	CGexMoTaskItem *ptTask = FindTaskInList(iTaskId);
	if(ptTask == NULL)
		return;

	switch(ptTask->iTaskType)
	{
		case GEXMO_TASK_DATAPUMP:
			delete ptTask->ptDataPump;
			break;
		case GEXMO_TASK_YIELDMONITOR:
			delete ptTask->ptYield;
			break;
		case GEXMO_TASK_YIELDMONITOR_RDB:
			delete ptTask->ptYield_RDB;
			break;
		case GEXMO_TASK_SPECMONITOR:
			delete ptTask->ptSpec;
			break;
		case GEXMO_TASK_REPORTING:
			delete ptTask->ptReporting;
			break;
		case GEXMO_TASK_STATUS:
			delete ptTask->ptStatus;
			break;
		case GEXMO_TASK_CONVERTER:
			delete ptTask->ptConverter;
			break;
		case GEXMO_TASK_OUTLIER_REMOVAL:
			delete ptTask->ptOutlier;
			break;
		case GEXMO_TASK_AUTOADMIN:
			delete ptTask->ptAutoAdmin;
			break;
	}

	pGexMoTasksList.remove(ptTask);
	// Destroy it from memory.
	delete ptTask;
}

///////////////////////////////////////////////////////////
// Duplicate the given task structure from list (name specified by caller).
///////////////////////////////////////////////////////////
void	GexMoScheduler::DuplicateTaskInList(int iTaskId,QString strCopyName)
{
    // Find
	CGexMoTaskItem *ptOriginalTask = FindTaskInList(iTaskId);
	if(ptOriginalTask == NULL)
		return;

	// Refuse to duplicate 'Status' task, there is only one per list!
	if(ptOriginalTask->iTaskType == GEXMO_TASK_STATUS)
	{
        GS::Gex::Message::information(
            szAppFullName,
            "*Error* Only one 'Status' task is allowed, can't duplicate it!");
		return;
	}

	// Refuse to duplicate 'AutoAdmin' task, there is only one per list!
	if(ptOriginalTask->iTaskType == GEXMO_TASK_AUTOADMIN)
	{
        GS::Gex::Message::information(
            szAppFullName,
            "*Error* Only one 'Auto Admin' task is allowed, "
            "can't duplicate it!");
		return;
	}

	// Refuse to duplicate 'Yield SYL/SBL' task, there is only one per list!
	if(ptOriginalTask->iTaskType == GEXMO_TASK_YIELDMONITOR_RDB)
	{
        GS::Gex::Message::information(
            szAppFullName,
            "*Error* Only one 'Yield SYL/SBL' task table is allowed, "
            "can't duplicate it!");
		return;
	}


	// Creating a new task.
	CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
	// Copy task type.
	ptTaskItem->iTaskType = ptOriginalTask->iTaskType;
	ptTaskItem->iUserId = ptOriginalTask->iUserId;
	ptTaskItem->iGroupId = ptOriginalTask->iGroupId;
	ptTaskItem->iPermissions = ptOriginalTask->iPermissions;

	GexMoDataPumpTaskData *ptTaskDataPump;
	GexMoYieldMonitoringTaskData *ptTaskYield;
	GexMoYieldMonitoringTaskData_RDB *ptTaskYield_RDB;
	GexMoSpecMonitoringTaskData *ptTaskSpec;
	GexMoReportingTaskData *ptTaskDataReporting;
	GexMoFileConverterTaskData *ptTaskDataConvert;
	GexMoOutlierRemovalTaskData *ptTaskOutlierRemoval;

	switch(ptOriginalTask->iTaskType)
	{
		case GEXMO_TASK_DATAPUMP:
			ptTaskDataPump = new GexMoDataPumpTaskData;
			// Copy field structure form original
			*ptTaskDataPump = *(ptOriginalTask->ptDataPump);
			ptTaskDataPump->strTitle = strCopyName;
			ptTaskItem->ptDataPump = ptTaskDataPump;
			break;
		case GEXMO_TASK_YIELDMONITOR:
			ptTaskYield = new GexMoYieldMonitoringTaskData;
			// Copy field structure form original
			*ptTaskYield = *(ptOriginalTask->ptYield);
			ptTaskYield->strTitle = strCopyName;
			ptTaskItem->ptYield = ptTaskYield;
			break;
		case GEXMO_TASK_YIELDMONITOR_RDB:
			ptTaskYield = new GexMoYieldMonitoringTaskData;
			// Copy field structure form original
			*ptTaskYield_RDB = *(ptOriginalTask->ptYield_RDB);
			ptTaskYield_RDB->strTitle = strCopyName;
			ptTaskItem->ptYield_RDB = ptTaskYield_RDB;
			break;
		case GEXMO_TASK_SPECMONITOR:
			ptTaskSpec = new GexMoSpecMonitoringTaskData;
			// Copy field structure form original
			*ptTaskSpec = *(ptOriginalTask->ptSpec);
			ptTaskSpec->strTitle = strCopyName;
			ptTaskItem->ptSpec = ptTaskSpec;
			break;
		case GEXMO_TASK_REPORTING:
			ptTaskDataReporting = new GexMoReportingTaskData;
			// Copy field structure form original
			*ptTaskDataReporting = *(ptOriginalTask->ptReporting);
			ptTaskDataReporting->strTitle = strCopyName;
			ptTaskItem->ptReporting = ptTaskDataReporting;
			break;
		case GEXMO_TASK_CONVERTER:
			ptTaskDataConvert = new GexMoFileConverterTaskData;
			// Copy field structure form original
			*ptTaskDataConvert = *(ptOriginalTask->ptConverter);
			ptTaskDataConvert->strTitle = strCopyName;
			ptTaskItem->ptConverter = ptTaskDataConvert;
			break;

		case GEXMO_TASK_OUTLIER_REMOVAL:
			ptTaskOutlierRemoval = new GexMoOutlierRemovalTaskData;
			// Copy field structure form original
			*ptTaskOutlierRemoval = *(ptOriginalTask->ptOutlier);
			ptTaskOutlierRemoval->strTitle = strCopyName;
			ptTaskItem->ptOutlier = ptTaskOutlierRemoval;
			break;
	}

	// YieldManDb
	ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
	ptTaskItem->strName		= strCopyName;

	pGexMoTasksList.append(ptTaskItem);
}

///////////////////////////////////////////////////////////
// Load Tasks from disk
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadTasks(bool bForceReload/*=true*/)
{
	// Load Tasks from Local File

	QString			strString;
	QFileInfo		cFileInfo(pGexMainWindow->strMoTasksFile);	// Task file.
	CGexMoTaskItem* ptTaskItem;

	unsigned int		uiNbInitialTasksEntries = pGexMoTasksList.count();

	m_nIndexInvalidTask = -1;

	// If Reload check enabled, check if new task file available...
	if(bForceReload == false)
	{
		// Check if Task file more recent than when last loaded...
		if(m_dtTaskLoadTimestamp.secsTo(cFileInfo.lastModified()) < 5)
			return;	// File not old enough!
	}

	// Build path to the 'Tasks' list.
    QFile file( pGexMainWindow->strMoTasksFile ); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
		return;	// Failed opening Tasks file.

	// Read Tasks definition File
	hTasks.setDevice(&file);	// Assign file handle to data stream

	// Check if valid header...or empty!
	strString = hTasks.readLine();
	if(strString != "<tasks>")
		return;

	// Empty existing Task list if any...
	DeleteTasksList();

	QStringList lstWarning;

	// Load task list (read XML file)
	do
	{
		// Read one line from file
		QString strString = hTasks.readLine();

		ptTaskItem = NULL;

		// Task type: DATA PUMP
		if(strString.startsWith("<task_data_pump>") == true)
			ptTaskItem = LoadTaskSectionDataPump();
		else
		// Task type: YIELD MONITORING
		if(strString.startsWith("<task_yield_monitoring>") == true)
			ptTaskItem = LoadTaskSectionYieldMonitoring();
		else
		// Task type: YIELD MONITORING (RDB: SYL/SBL)
		if(strString.startsWith("<task_yield_monitoring_rdb>") == true)
			ptTaskItem = LoadTaskSectionYieldMonitoring_RDB();
		else
		// Task type: SPEC MONITORING
		if(strString.startsWith("<task_spec_monitoring>") == true)
			ptTaskItem = LoadTaskSectionSpecMonitoring();
		else
		// Task type: REPORTING
		if(strString.startsWith("<task_reporting>") == true)
			ptTaskItem = LoadTaskSectionReporting();
		else
		// Task type: STATUS
		if(strString.startsWith("<task_status>") == true)
			ptTaskItem = LoadTaskSectionStatus();
		else
		// Task type: FILE CONVERTER
		if(strString.startsWith("<task_file_converter>") == true)
			ptTaskItem = LoadTaskSectionFileConverter();
		else
		// Task type: OUTLIER REMOVAL
		if(strString.startsWith("<task_outlier_monitoring>") == true)
			ptTaskItem = LoadTaskSectionOutlierRemoval();
		else
		// Task type: AUTO ADMIN
		if(strString.startsWith("<task_autoadmin>") == true)
			ptTaskItem = LoadTaskSectionAutoAdmin();
		else
			continue;

		// YIELDMANDB
		if(ptTaskItem == NULL)
			continue;

		// For each task loaded
		// linked with External Database
		//		- check if database is exist then update the database id else ignore this task (WARNING)
		// save this task
		//		- update the task id

		bool	bCheckDatabase = false;
		QString strTaskName;
		QString strDataBaseName;

		strTaskName		= ptTaskItem->strName;
		strDataBaseName = ptTaskItem->strDatabaseName;

		switch(ptTaskItem->iTaskType)
		{
			case GEXMO_TASK_DATAPUMP:
				bCheckDatabase	= true;
				break;
			case GEXMO_TASK_YIELDMONITOR:
				bCheckDatabase = true;
				break;
			case GEXMO_TASK_YIELDMONITOR_RDB:
				bCheckDatabase = true;
				break;
			case GEXMO_TASK_SPECMONITOR:
				bCheckDatabase = false;
				break;
			case GEXMO_TASK_REPORTING:
				bCheckDatabase = false;
				break;
			case GEXMO_TASK_STATUS:
				bCheckDatabase = false;
				break;
			case GEXMO_TASK_CONVERTER:
				bCheckDatabase = false;
				break;
			case GEXMO_TASK_OUTLIER_REMOVAL:
				bCheckDatabase = false;
				break;
			case GEXMO_TASK_AUTOADMIN:
				bCheckDatabase = false;
				break;
		}

		// Then Check if the Database is referenced
		if(bCheckDatabase)
		{
			GexDatabaseEntry* pDatabaseEntry = NULL;

			if(!strDataBaseName.isEmpty())
				pDatabaseEntry = pGexMainWindow->pDatabaseCenter->FindDatabaseEntry(strDataBaseName);


			if(pDatabaseEntry)
			{
				ptTaskItem->iStatus = 1;
				ptTaskItem->iDatabaseId = pDatabaseEntry->m_nDatabaseId;
			}
			else
			{
				QString strError;
				QString strMessage;

				ptTaskItem->iStatus = -1;
				ptTaskItem->bEnabled = false;
				ptTaskItem->iDatabaseId = -1;

				strMessage = "Task '"+strTaskName+"' must have a valid Database referenced.";
				if(strDataBaseName.isEmpty())
					strMessage += " [Empty database name]";
				else
					strMessage += " [" + strDataBaseName + ": connection error]";
				pGexMainWindow->pDatabaseCenter->UpdateLogError(strError, strMessage);

				lstWarning.append(strMessage);

				// On error
				// Show History file windows
				// Only the first time
				if(uiNbInitialTasksEntries == 0)
				{
					//pGexMainWindow->ShowWizardDialog(GEXMO_HISTORY);
				}
				uiNbInitialTasksEntries++;
			}

		}
		else
		{
			ptTaskItem->iDatabaseId = -1;
			ptTaskItem->iStatus = 1;
		}

		// Add task to list
		UpdateListViewItem(ptTaskItem,NULL,true);

	}
	while(hTasks.atEnd() == false);
	file.close();

	// YIELDMANDB
	// Only display this warning for the first importation
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT)
	&& !lstWarning.isEmpty())
	{
		// Display msgBox
        GS::Gex::Message::information("YieldManDb tasks importation" ,
                                      lstWarning.join("\n"));
	}

	// Save timestamp of Task file loaded
	m_dtTaskLoadTimestamp = cFileInfo.lastModified();
}

///////////////////////////////////////////////////////////
// Load from disk...section: Data Pump
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionDataPump()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoDataPumpTaskData *ptTaskDataPump = new GexMoDataPumpTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskDataPump->strTitle = strString.section('=',1);

		// Read data path
		if(strString.startsWith("DataPath=") == true)
			ptTaskDataPump->strDataPath = strString.section('=',1);

		// Scan sub-folder flag
		if(strString.startsWith("ScanSubFolders=") == true)
		{
			if(strString.section('=',1) == "YES")
				ptTaskDataPump->bScanSubFolders = true;
			else
				ptTaskDataPump->bScanSubFolders = false;
		}

		// List of files extensions to import
		if(strString.startsWith("ImportFileExtensions=") == true)
				ptTaskDataPump->strImportFileExtensions = strString.section('=',1);

		// Read Database targetted
		if(strString.startsWith("Database=") == true)
			ptTaskDataPump->strDatabaseTarget = strString.section('=',1);

		// Read Data Type targetted
		if(strString.startsWith("DataType=") == true)
			ptTaskDataPump->uiDataType = strString.section('=',1).toUInt();

		// Read Testing stage (if WYR data type)
		if(strString.startsWith("TestingStage=") == true)
			ptTaskDataPump->strTestingStage = strString.section('=',1);

		// Read Task frequency
		if(strString.startsWith("Frequency=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iFrequency = strString.toLong();
		}

		// Read Task Day of Week execution
		if(strString.startsWith("DayOfWeek=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iDayOfWeek = strString.toLong();
		}

		// Execution window flag
		if(strString.startsWith("ExecWindow=") == true)
		{
			if(strString.section('=',1) == "YES")
				ptTaskDataPump->bExecutionWindow = true;
			else
				ptTaskDataPump->bExecutionWindow = false;
		}

		// Read Start-time
		if(strString.startsWith("StartTime=") == true)
		{
			strString = strString.section('=',1);
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTaskDataPump->cStartTime = tStartTime;
		}

		// Read Stop-time
		if(strString.startsWith("StopTime=") == true)
		{
			strString = strString.section('=',1);
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStopTime(iHour,iMinute);
			ptTaskDataPump->cStopTime = tStopTime;
		}

		// Read PostImport task: Rename, Move or Delete files.
		if(strString.startsWith("PostImport=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iPostImport= strString.toLong();
		}

		// Read Move/FTP folder
		if(strString.startsWith("PostImportFolder=") == true)
			ptTaskDataPump->strPostImportMoveFolder = strString.section('=',1);

		// Read PostImport task (failed files): Rename, Move or Delete files.
		if(strString.startsWith("PostImportFailure=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iPostImportFailure= strString.toLong();
		}
		// Read Move/FTP folder (failed files)
		if(strString.startsWith("PostImportFailureFolder=") == true)
			ptTaskDataPump->strPostImportFailureMoveFolder = strString.section('=',1);

		// Read PostImport task (delayed files): Rename, Move or Delete files.
		if(strString.startsWith("PostImportDelay=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iPostImportDelay= strString.toLong();
		}
		// Read Move/FTP folder (delayed files)
		if(strString.startsWith("PostImportDelayFolder=") == true)
			ptTaskDataPump->strPostImportDelayMoveFolder = strString.section('=',1);

		// check Yield window flag
		if(strString.startsWith("CheckYield=") == true)
		{
			if(strString.section('=',1) == "YES")
				ptTaskDataPump->bCheckYield = true;
			else
				ptTaskDataPump->bCheckYield = false;
		}

		// Read Good bin list
		if(strString.startsWith("YieldBins=") == true)
			ptTaskDataPump->strYieldBinsList = strString.section('=',1);

		// Read Alarm level (0-100%)
		if(strString.startsWith("AlarmLevel=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->iAlarmLevel = strString.toInt();
		}

		// Read Minimum parts to have a valid file
		if(strString.startsWith("MinimumParts=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->lMinimumyieldParts = strString.toLong();
		}

		// Read Email notification list
		if(strString.startsWith("Emails=") == true)
			ptTaskDataPump->strEmailNotify = strString.section('=',1);

		// Read Email format: HTML or TXT
		if(strString.startsWith("EmailFormat=") == true)
		{
			if(strString.section('=',1) == "HTML")
				ptTaskDataPump->bHtmlEmail = true;
			else
				ptTaskDataPump->bHtmlEmail = false;
		}

		// Read Last time task was executed...
		if(strString.startsWith("LastExecuted=") == true)
		{
			strString = strString.section('=',1);
			ptTaskDataPump->tLastExecuted = strString.toLong();
		}

		if(strString.startsWith("</task_data_pump>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_DATAPUMP;
			ptTaskItem->ptDataPump = ptTaskDataPump;

			// YieldManDb
			ptTaskItem->iTaskId			= m_nIndexInvalidTask--;
			ptTaskItem->strName			= ptTaskDataPump->strTitle;
			ptTaskItem->strDatabaseName = ptTaskDataPump->strDatabaseTarget;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			// YIELDMANDB
			//UpdateListViewDataPumpItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: YIELD MONITORING
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionYieldMonitoring()
{
	QString strString;
	QString strSection;

	// YieldManDb
	// All YieldMonitoring and YieldLimit rules are loaded in ptYield

	// Allocate buffer to store information read from disk.
	GexMoYieldMonitoringTaskData* ptTaskYield = NULL;

	ptTaskYield = LoadTaskSectionYieldMonitoring_RDB_OneRule();

	if(ptTaskYield == NULL)
		return NULL;

	// End of section, add Entry to the task list

	// Creating a new task.
	CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
	ptTaskItem->iTaskType = GEXMO_TASK_YIELDMONITOR;
	ptTaskItem->ptYield = ptTaskYield;


	// YieldManDb
	ptTaskItem->iTaskId			= m_nIndexInvalidTask--;
	ptTaskItem->strName			= ptTaskYield->strTitle;
	ptTaskItem->strDatabaseName = ptTaskYield->strDatabase;

	// Add task to internal structure
	pGexMoTasksList.append(ptTaskItem);

	// Add task to list
	//UpdateListViewYieldItem(ptTaskItem,NULL,true);
	return ptTaskItem;
}

///////////////////////////////////////////////////////////
// Load from disk...section: YIELD MONITORING RDB  (SYL/SBL)
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionYieldMonitoring_RDB()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store ALL RDB-Yield-Management rules
	GexMoYieldMonitoringTaskData_RDB *ptTaskYield_RDB = NULL;
	GexMoYieldMonitoringTaskData *ptTaskYield = NULL;

	// Creating a new task.
	CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
	ptTaskItem->iTaskType = GEXMO_TASK_YIELDMONITOR_RDB;
	ptTaskItem->ptYield_RDB = new GexMoYieldMonitoringTaskData_RDB;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Task Title
		if(strString.startsWith("TaskTitle=") == true)
			ptTaskItem->strName = ptTaskItem->ptYield_RDB->strTitle = strSection;

		// Extract Yield rule type
		if(strString.startsWith("<task_yield_monitoring_rdb_yl>") == true)
		{
			ptTaskYield = LoadTaskSectionYieldMonitoring_RDB_OneRule();

			if(ptTaskYield == NULL)
				return NULL;

			ptTaskItem->ptYield_RDB->cYieldRules.append(ptTaskYield);
		}
		if(strString.startsWith("<task_yield_monitoring_rdb_sya>") == true)
		{
			ptTaskYield = LoadTaskSectionYieldMonitoring_RDB_OneRule();

			if(ptTaskYield == NULL)
				return NULL;

			ptTaskItem->ptYield_RDB->cYield_SYA_Rules.append(ptTaskYield);
		}

		// End of rule
		if(strString.startsWith("</task_yield_monitoring_rdb>") == true)
		{
			// End of section, add Entry to the task list

			// YieldManDb
			ptTaskItem->iTaskId			= m_nIndexInvalidTask--;
			ptTaskItem->strDatabaseName = "";

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewYieldItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}

	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}


///////////////////////////////////////////////////////////
// Load from disk...section: YIELD MONITORING RDB  (SYL/SBL)
///////////////////////////////////////////////////////////
GexMoYieldMonitoringTaskData * GexMoScheduler::LoadTaskSectionYieldMonitoring_RDB_OneRule()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store ALL RDB-Yield-Management rules
	GexMoYieldMonitoringTaskData *ptTaskYield = new GexMoYieldMonitoringTaskData;
	QString strTitle;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskYield->strTitle = strSection;

		if(strString.startsWith("State=") == true)
			ptTaskYield->bEnabled = strSection == "1";

		// Read ProductID
		if(strString.startsWith("ProductID=") == true)
			ptTaskYield->strProductID = strSection;

		// Read Yield Bin list
		if(strString.startsWith("YieldBins=") == true)
			ptTaskYield->strYieldBinsList = strSection;

		// Bin list type: 0=Good bins, 1=Failing bins.
		if(strString.startsWith("BiningType=") == true)
			ptTaskYield->iBiningType = strSection.toInt();

		// Read Alarm level (0-100%)
		if(strString.startsWith("AlarmLevel=") == true)
			ptTaskYield->iAlarmLevel = strSection.toInt();

		// Read Flag: Check if Yield OVER or UNDER the limit.
		if(strString.startsWith("AlarmDirection=") == true)
			ptTaskYield->iAlarmIfOverLimit = strSection.toInt();


		// Read Minimum parts to have a valid file
		if(strString.startsWith("MinimumParts=") == true)
			ptTaskYield->lMinimumyieldParts = strSection.toLong();

		// Read SBL/YBL data file (if exists)
		if(strString.startsWith("SblFile=") && QFile::exists(strSection))
			ptTaskYield->strSblFile = strSection;

		// Read Email 'From'
		if(strString.startsWith("EmailFrom=") == true)
			ptTaskYield->strEmailFrom = strSection;

		// Read Email notification list
		if(strString.startsWith("Emails=") == true)
			ptTaskYield->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString.startsWith("EmailFormat=") == true)
		{
			if(strString.section('=',1) == "HTML")
				ptTaskYield->bHtmlEmail = true;
			else
				ptTaskYield->bHtmlEmail = false;
		}

		// Read Email message contents type to send
		if(strString.startsWith("EmailReportType=") == true)
			ptTaskYield->iEmailReportType = strSection.toInt();

		// Read Email report notification type: send as attachment or leave on server
		if(strString.startsWith("NotificationType=") == true)
			ptTaskYield->iNotificationType = strSection.toInt();

		// Read Alarm type: Standard, Critical...
		if(strString.startsWith("ExceptionLevel=") == true)
			ptTaskYield->iExceptionLevel = strSection.toInt();

		/////////////////////////////////////////////////////////////////////////////
		// SYL-SBL specifics
		/////////////////////////////////////////////////////////////////////////////
		if(strString.startsWith("Database=") == true)
			ptTaskYield->strDatabase = strSection;

		if(strString.startsWith("TestingStage=") == true)
			ptTaskYield->strTestingStage = strSection;

		if(strString.startsWith("RuleType=") == true)
			ptTaskYield->iSYA_Rule = strSection.toInt();		// Rule: 0=N*Sigma, 1=N*IQR

		if(strString.startsWith("RuleTypeString=") == true)		// Rule string: N*Sigma, N*IQR
			ptTaskYield->strSYA_Rule = strSection;

		if(strString.startsWith("N_Parameter=") == true)
			ptTaskYield->fSYA_N1_value = strSection.toFloat();	// N parameter (compatibility, new fields are N1, N2)

		if(strString.startsWith("N1_Parameter=") == true)
			ptTaskYield->fSYA_N1_value = strSection.toFloat();	// N1 parameter

		if(strString.startsWith("N2_Parameter=") == true)
			ptTaskYield->fSYA_N2_value = strSection.toFloat();	// N2 parameter

		if(strString.startsWith("MinimumLotsRequired=") == true)
			ptTaskYield->iSYA_LotsRequired = strSection.toInt();// Minimum Total lots required for computing new SYL-SBL

		if(strString.startsWith("ValidityPeriod=") == true)
			ptTaskYield->iSYA_Period = strSection.toInt();// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...

		if(strString.startsWith("ExpirationDate=") == true)
		{
			int iDay,iMonth,iYear;
			QString strString;
			iDay = strSection.section(' ',0,0).trimmed().toInt();
			iMonth = strSection.section(' ',1,1).trimmed().toInt();
			iYear = strSection.section(' ',2,2).trimmed().toInt();
			ptTaskYield->cExpiration.setDate(iYear,iMonth,iDay);
		}

		if(strString.startsWith("SBL_LL_Disabled=") == true)
		{
			ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;
			ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;
		}

		if(strString.startsWith("SBL1_LL_Disabled=") == true)
			ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;

		if(strString.startsWith("SBL2_LL_Disabled=") == true)
			ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;

		if(strString.startsWith("SBL_HL_Disabled=") == true)
		{
			ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;
			ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;
		}

		if(strString.startsWith("SBL1_HL_Disabled=") == true)
			ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;

		if(strString.startsWith("SBL2_HL_Disabled=") == true)
			ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;

		if(strString.startsWith("SYL_LL_Disabled=") == true)
		{
			ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";
			ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";
		}

		if(strString.startsWith("SYL1_LL_Disabled=") == true)
			ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";

		if(strString.startsWith("SYL2_LL_Disabled=") == true)
			ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";

		if(strString.startsWith("SYL_HL_Disabled=") == true)
		{
			ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";
			ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";
		}

		if(strString.startsWith("SYL1_HL_Disabled=") == true)
			ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";

		if(strString.startsWith("SYL2_HL_Disabled=") == true)
			ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";

		if(strString.startsWith("IgnoreDataPointsWithNullSigma=") == true)
			ptTaskYield->bSYA_IgnoreDataPointsWithNullSigma = strSection == "1";

		if(strString.startsWith("IgnoreOutliers=") == true)
			ptTaskYield->bSYA_IgnoreOutliers = strSection == "1";

		if(strString.startsWith("MinDataPoints=") == true)
			ptTaskYield->iSYA_MinDataPoints = strSection.toInt();


		// End of rule
		if(strString.startsWith("</task_yield_monitoring") == true)
		{
			// End of section, add Entry to the task list

			return ptTaskYield;
		}
next_line:;
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: SPEC MONITORING
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionSpecMonitoring()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoSpecMonitoringTaskData *ptTaskSpec= new GexMoSpecMonitoringTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskSpec->strTitle = strSection;

		// Read ProductID
		if(strString.startsWith("ProductID=") == true)
			ptTaskSpec->strProductID = strSection;

		// Read Parameter/Test#
		if(strString.startsWith("Parameter=") == true)
			ptTaskSpec->strParameter = strSection;

		// Read Parameter inof to monitor
		if(strString.startsWith("MonitorInfo=") == true)
			ptTaskSpec->iMonitorInfo = strSection.toInt();

		// Read Low spec limit (if any)
		if(strString.startsWith("LowSpec=") == true)
			ptTaskSpec->strLowSpec = strSection;

		// Read High spec limit (if any)
		if(strString.startsWith("HighSpec=") == true)
			ptTaskSpec->strHighSpec = strSection;

		// Emails list...
		if(strString.startsWith("Emails=") == true)
			ptTaskSpec->strEmailNotify = strSection;

		// Emails from...
		if(strString.startsWith("EmailFrom=") == true)
			ptTaskSpec->strEmailFrom = strSection;

		// Read Email format: HTML or TXT
		if(strString.startsWith("EmailFormat=") == true)
		{
			if(strString.section('=',1) == "HTML")
				ptTaskSpec->bHtmlEmail = true;
			else
				ptTaskSpec->bHtmlEmail = false;
		}

		// Read Alarm type: Standard, Critical...
		if(strString.startsWith("ExceptionLevel=") == true)
			ptTaskSpec->iExceptionLevel = strSection.toInt();

		if(strString.startsWith("</task_spec_monitoring>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_SPECMONITOR;
			ptTaskItem->ptSpec = ptTaskSpec;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskSpec->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewSpecItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: REPORTING
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionReporting()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoReportingTaskData *ptTaskReporting = new GexMoReportingTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskReporting->strTitle = strString.section('=',1);

		// Read Script path
		if(strString.startsWith("ScriptPath=") == true)
			ptTaskReporting->strScriptPath = strString.section('=',1);

		// Read Task frequency
		if(strString.startsWith("Frequency=") == true)
		{
			strString = strString.section('=',1);
			ptTaskReporting->iFrequency = strString.toLong();
		}

		// Read Task Day of Week execution
		if(strString.startsWith("DayOfWeek=") == true)
		{
			strString = strString.section('=',1);
			ptTaskReporting->iDayOfWeek = strString.toLong();
		}

		// Execution window flag
		if(strString.startsWith("ExecWindow=") == true)
		{
			if(strString.section('=',1) == "YES")
				ptTaskReporting->bExecutionWindow = true;
			else
				ptTaskReporting->bExecutionWindow = false;
		}

		// Read Start-time
		if(strString.startsWith("StartTime=") == true)
		{
			strString = strString.section('=',1);
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTaskReporting->cStartTime = tStartTime;
		}

		// Read Stop-time
		if(strString.startsWith("StopTime=") == true)
		{
			strString = strString.section('=',1);
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStopTime(iHour,iMinute);
			ptTaskReporting->cStopTime = tStopTime;
		}

		// Read Email Notification type
		if(strString.startsWith("NotificationType=") == true)
		{
			strString = strString.section('=',1);
			ptTaskReporting->iNotificationType = strString.toInt();
		}

		// Read Email notification list
		if(strString.startsWith("Emails=") == true)
			ptTaskReporting->strEmailNotify = strString.section('=',1);

		// Read Email format: HTML or TXT
		if(strString.startsWith("EmailFormat=") == true)
		{
			if(strString.section('=',1) == "HTML")
				ptTaskReporting->bHtmlEmail = true;
			else
				ptTaskReporting->bHtmlEmail = false;
		}

		// Read Last time task was executed...
		if(strString.startsWith("LastExecuted=") == true)
		{
			strString = strString.section('=',1);
			ptTaskReporting->tLastExecuted = strString.toLong();
		}

		if(strString.startsWith("</task_reporting>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_REPORTING;
			ptTaskItem->ptReporting = ptTaskReporting;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskReporting->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewReportingItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: STATUS
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionStatus()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoStatusTaskData *ptTaskStatus = new GexMoStatusTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskStatus->strTitle = strSection;

		// Read Web organization type
		if(strString.startsWith("WebOrganization=") == true)
		{
			int iOneWebPerDatabase;
			iOneWebPerDatabase = strSection.toLong();
            if (iOneWebPerDatabase)
				ptTaskStatus->bOneWebPerDatabase = true;
			else
				ptTaskStatus->bOneWebPerDatabase = false;
		}

		// Read Intranet path
		if(strString.startsWith("IntranetPath=") == true)
			ptTaskStatus->strIntranetPath = strSection;

		// Read Home page name
		if(strString.startsWith("HomePage=") == true)
			ptTaskStatus->strHomePage = strSection;

		// Report's URL name to display in Emails (hyperlink)
		if(strString.startsWith("ReportURL=") == true)
			ptTaskStatus->strReportURL = strSection;

		if(strString.startsWith("</task_status>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_STATUS;
			ptTaskItem->ptStatus = ptTaskStatus;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskStatus->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewStatusItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: FileConverter
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionFileConverter()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoFileConverterTaskData *ptTaskDataConvert = new GexMoFileConverterTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskDataConvert->strTitle = strSection;

		// Input Folder
		if(strString.startsWith("InputFolder=") == true)
			ptTaskDataConvert->strInputFolder = strSection;

		// Input file extensions
		if(strString.startsWith("ImportFileExtensions=") == true)
			ptTaskDataConvert->strFileExtensions = strSection;

		// Execution frequency
		if(strString.startsWith("Frequency=") == true)
			ptTaskDataConvert->iFrequency = strSection.toInt();

		// Execution Day of week (if frequency is week, or month,...)
		if(strString.startsWith("DayOfWeek=") == true)
			ptTaskDataConvert->iDayOfWeek = strSection.toInt();

		// Output Folder
		if(strString.startsWith("OutputFolder=") == true)
			ptTaskDataConvert->strOutputFolder = strSection;

		// Output Format: STDF (0) or CSV (1)
		if(strString.startsWith("OutputFormat=") == true)
			ptTaskDataConvert->iFormat = strSection.toInt();

		// Include Timestamp info in file name to create?
		if(strString.startsWith("TimeStampFile=") == true)
			ptTaskDataConvert->bTimeStampName = (strSection.toInt() != 0) ? true : false;

		// What to to file file successfuly converted
		if(strString.startsWith("SuccessMode=") == true)
			ptTaskDataConvert->iOnSuccess = strSection.toInt();

		// Folder where to move source files (if successfuly converted)
		if(strString.startsWith("SuccessFolder=") == true)
			ptTaskDataConvert->strOutputSuccess = strSection;

		// What to to file file that failed conversion
		if(strString.startsWith("FailMode=") == true)
			ptTaskDataConvert->iOnError = strSection.toInt();

		// Folder where to move source files (if failed conversion)
		if(strString.startsWith("FailFolder=") == true)
			ptTaskDataConvert->strOutputError = strSection;

		if(strString.startsWith("</task_file_converter>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_CONVERTER;
			ptTaskItem->ptConverter = ptTaskDataConvert;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskDataConvert->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewConverterItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: OUTLIER REMOVAL
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionOutlierRemoval()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoOutlierRemovalTaskData *ptTaskOutlier= new GexMoOutlierRemovalTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);


		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskOutlier->strTitle = strSection;

		// Read ProductID
		if(strString.startsWith("ProductID=") == true)
			ptTaskOutlier->strProductID = strSection;

		// Read Alarm level
		if(strString.startsWith("AlarmLevel=") == true)
			ptTaskOutlier->lfAlarmLevel = strSection.toDouble();

		// Read Alarm Type: % (0) or #parts (1)
		if(strString.startsWith("AlarmType=") == true)
			ptTaskOutlier->iAlarmType = strSection.toInt();

		// Read Minimum parts to have a valid file
		if(strString.startsWith("MinimumParts=") == true)
			ptTaskOutlier->lMinimumyieldParts = strSection.toLong();

		// Notify if distribution shape changes compared to historical data
		if(strString.startsWith("NotifyShapeChange=") == true)
			ptTaskOutlier->bNotifyShapeChange = (bool) strSection.toLong();

		// Read Maximum number of Die mismatch between E-Test & STDF wafermaps
		if(strString.startsWith("CompositeEtestAlarm=") == true)
			ptTaskOutlier->lCompositeEtestAlarm = strSection.toLong();

		// Read Maximum number of Die to reject on the exclusion zone stacked wafer.
		if(strString.startsWith("CompositeExclusionZoneAlarm=") == true)
			ptTaskOutlier->lCompositeExclusionZoneAlarm = strSection.toLong();

		// Read Email notification list
		if(strString.startsWith("Emails=") == true)
			ptTaskOutlier->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString.startsWith("EmailFormat=") == true)
		{
			if(strString.section('=',1) == "HTML")
				ptTaskOutlier->bHtmlEmail = true;
			else
				ptTaskOutlier->bHtmlEmail = false;
		}

		// Read Email message contents type to send
		if(strString.startsWith("EmailReportType=") == true)
			ptTaskOutlier->iEmailReportType = strSection.toInt();

		// Read Email report notification type: send as attachment or leave on server
		if(strString.startsWith("NotificationType=") == true)
			ptTaskOutlier->iNotificationType = strSection.toInt();

		// Read Alarm type: Standard, Critical...
		if(strString.startsWith("ExceptionLevel=") == true)
			ptTaskOutlier->iExceptionLevel = strSection.toInt();

		if(strString.startsWith("</task_outlier_monitoring>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_OUTLIER_REMOVAL;
			ptTaskItem->ptOutlier = ptTaskOutlier;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskOutlier->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewOutlierItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Load from disk...section: Auto Admin
///////////////////////////////////////////////////////////
CGexMoTaskItem* GexMoScheduler::LoadTaskSectionAutoAdmin()
{
	QString strString;
	QString strSection;

	// Allocate buffer to store information read from disk.
	GexMoAutoAdminTaskData *ptTaskAutoAdmin = new GexMoAutoAdminTaskData;

	do
	{
		// Read one line from file
		strString = hTasks.readLine();
		strSection = strString.section('=',1);

		// Read Title
		if(strString.startsWith("Title=") == true)
			ptTaskAutoAdmin->strTitle = strSection;

		// Time of day to start auto-admin
		if(strString.startsWith("StartTime=") == true)
		{
			strString = strString.section('=',1);
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTaskAutoAdmin->cStartTime = tStartTime;
		}

		// Read Web organization type
		if(strString.startsWith("KeepReportDuration=") == true)
			ptTaskAutoAdmin->iKeepReportDuration = strSection.toLong();

		// Read log file contents type
		if(strString.startsWith("LogContents=") == true)
			ptTaskAutoAdmin->iLogContents = strSection.toLong();

		// Read Email notification list
		if(strString.startsWith("Emails=") == true)
			ptTaskAutoAdmin->strEmailNotify = strString.section('=',1);

		// Read Last time task was executed...
		if(strString.startsWith("LastExecuted=") == true)
		{
			strString = strString.section('=',1);
			ptTaskAutoAdmin->tLastExecuted = strString.toLong();
		}

		// Read Shell to launch if: Yield Alarm
		if(strString.startsWith("ShellYieldAlarm=") == true)
			ptTaskAutoAdmin->strShellYieldAlarm_Std = strString.section('=',1);
		if(strString.startsWith("ShellYieldAlarmCritical=") == true)
			ptTaskAutoAdmin->strShellYieldAlarm_Critical = strString.section('=',1);

		// Read Shell to launch if: Parameter Alarm
		if(strString.startsWith("ShellParameterAlarm=") == true)
			ptTaskAutoAdmin->strShellParameterAlarm_Std = strString.section('=',1);
		if(strString.startsWith("ShellParameterAlarmCritical=") == true)
			ptTaskAutoAdmin->strShellParameterAlarm_Critical = strString.section('=',1);

		// Read Shell to launch if: Pat Alarm
		if(strString.startsWith("ShellPatAlarm=") == true)
			ptTaskAutoAdmin->strShellPatAlarm_Std = strString.section('=',1);
		if(strString.startsWith("ShellPatAlarmCritical=") == true)
			ptTaskAutoAdmin->strShellPatAlarm_Critical = strString.section('=',1);

		if(strString.startsWith("</task_autoadmin>") == true)
		{
			// End of section, add Entry to the task list

			// Creating a new task.
			CGexMoTaskItem *ptTaskItem = new CGexMoTaskItem;
			ptTaskItem->iTaskType = GEXMO_TASK_AUTOADMIN;
			ptTaskItem->ptAutoAdmin = ptTaskAutoAdmin;

			// YieldManDb
			ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
			ptTaskItem->strName		= ptTaskAutoAdmin->strTitle;

			// Add task to internal structure
			pGexMoTasksList.append(ptTaskItem);

			// Add task to list
			//UpdateListViewAutoAdminItem(ptTaskItem,NULL,true);

			// Stop reading this section!
			return ptTaskItem;
		}
	}
	while(hTasks.atEnd() == false);

	// Unexpected end of section...
	// quietly return.
	return NULL;
}

///////////////////////////////////////////////////////////
// Delete Task list in memory, empty list view.
///////////////////////////////////////////////////////////
void GexMoScheduler::DeleteTasksList(void)
{
	CGexMoTaskItem * ptTask = NULL;

	while(!pGexMoTasksList.isEmpty())
	{
		ptTask = pGexMoTasksList.takeFirst();

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_DATAPUMP:
				delete ptTask->ptDataPump;
				break;
			case GEXMO_TASK_YIELDMONITOR:
				delete ptTask->ptYield;
				break;
			case GEXMO_TASK_YIELDMONITOR_RDB:
				delete ptTask->ptYield_RDB;
				break;
			case GEXMO_TASK_SPECMONITOR:
				delete ptTask->ptSpec;
				break;
			case GEXMO_TASK_REPORTING:
				delete ptTask->ptReporting;
				break;
			case GEXMO_TASK_STATUS:
				delete ptTask->ptStatus;
				break;
			case GEXMO_TASK_CONVERTER:
				delete ptTask->ptConverter;
				break;
			case GEXMO_TASK_OUTLIER_REMOVAL:
				delete ptTask->ptOutlier;
				break;
			case GEXMO_TASK_AUTOADMIN:
				delete ptTask->ptAutoAdmin;
				break;
		}

		// Delete node
		delete ptTask;
	};

	// Clear list view.
	//ListView->clear();
	tableWidgetTasksListDataPump->clear();
	tableWidgetTasksListReporting->clear();
	tableWidgetTasksListParameterSpecs->clear();
	tableWidgetTasksListConverter->clear();
	tableWidgetTasksListOutlierRemoval->clear();
	tableWidgetTasksListYield->clear();
	tableWidgetTasksListYieldRdb->clear();
	tableWidgetTasksListStatus->clear();
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section DATA PUMP
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionDataPump(GexMoDataPumpTaskData *ptDataPump)
{
	hTasks << "<task_data_pump>" << endl;
	hTasks << "Title=" << ptDataPump->strTitle << endl;
	hTasks << "DataPath=" << ptDataPump->strDataPath << endl;

	if(ptDataPump->bScanSubFolders)
		hTasks << "ScanSubFolders=YES" << endl;
	else
		hTasks << "ScanSubFolders=NO" << endl;

	hTasks << "ImportFileExtensions=" << ptDataPump->strImportFileExtensions << endl;
	hTasks << "Database=" << ptDataPump->strDatabaseTarget << endl;
	hTasks << "DataType=" << QString::number(ptDataPump->uiDataType) << endl;
	hTasks << "TestingStage=" << ptDataPump->strTestingStage << endl;
	hTasks << "Frequency=" << ptDataPump->iFrequency << endl;
	hTasks << "DayOfWeek=" << ptDataPump->iDayOfWeek << endl;

	if(ptDataPump->bExecutionWindow)
		hTasks << "ExecWindow=YES" << endl;
	else
		hTasks << "ExecWindow=NO" << endl;

	hTasks << "StartTime=" << ptDataPump->cStartTime.hour() << ",";
	hTasks << ptDataPump->cStartTime.minute() << endl;
	hTasks << "StopTime=" << ptDataPump->cStopTime.hour() << ",";
	hTasks << ptDataPump->cStopTime.minute() << endl;
	hTasks << "PostImport=" << ptDataPump->iPostImport<< endl;
	hTasks << "PostImportFolder=" << ptDataPump->strPostImportMoveFolder << endl;
	hTasks << "PostImportFailure=" << ptDataPump->iPostImportFailure<< endl;
	hTasks << "PostImportFailureFolder=" << ptDataPump->strPostImportFailureMoveFolder << endl;
	hTasks << "PostImportDelay=" << ptDataPump->iPostImportDelay<< endl;
	hTasks << "PostImportDelayFolder=" << ptDataPump->strPostImportDelayMoveFolder << endl;

	if(ptDataPump->bCheckYield)
		hTasks << "CheckYield=YES" << endl;
	else
		hTasks << "CheckYield=NO" << endl;

	hTasks << "YieldBins=" << ptDataPump->strYieldBinsList << endl;
	hTasks << "AlarmLevel=" << ptDataPump->iAlarmLevel << endl;
	hTasks << "MinimumParts=" << ptDataPump->lMinimumyieldParts << endl;
	hTasks << "Emails=" << ptDataPump->strEmailNotify << endl;

	if(ptDataPump->bHtmlEmail)
		hTasks << "EmailFormat=HTML" << endl;
	else
		hTasks << "EmailFormat=TEXT" << endl;

	hTasks << "LastExecuted=" << ptDataPump->tLastExecuted << endl;
	hTasks << "</task_data_pump>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section YIELD MONITORING
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionYieldMonitoring(GexMoYieldMonitoringTaskData *ptYield)
{
	// Write rule to disk
	hTasks << "<task_yield_monitoring>" << endl;

	// Write rule to disk
	SaveTaskSectionYieldMonitoring_RDB_OneRule(ptYield);

	hTasks << "</task_yield_monitoring>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: One rule from Section YIELD MONITORING RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionYieldMonitoring_RDB_OneRule(GexMoYieldMonitoringTaskData *ptYield)
{
	hTasks << "Title=" << ptYield->strTitle << endl;
	hTasks << "State=" << (ptYield->bEnabled ? "1" : "0") << endl;
	hTasks << "ProductID=" << ptYield->strProductID << endl;
	hTasks << "YieldBins=" << ptYield->strYieldBinsList << endl;
	hTasks << "BiningType=" << ptYield->iBiningType << endl;
	hTasks << "AlarmLevel=" << ptYield->iAlarmLevel << endl;
	hTasks << "AlarmDirection=" << ptYield->iAlarmIfOverLimit << endl;
	hTasks << "MinimumParts=" << ptYield->lMinimumyieldParts << endl;
	hTasks << "EmailFrom=" << ptYield->strEmailFrom << endl;
	hTasks << "Emails=" << ptYield->strEmailNotify << endl;

	if(ptYield->bHtmlEmail)
		hTasks << "EmailFormat=HTML" << endl;
	else
		hTasks << "EmailFormat=TEXT" << endl;

	hTasks << "EmailReportType=" << ptYield->iEmailReportType << endl;
	hTasks << "NotificationType=" << ptYield->iNotificationType << endl;
	hTasks << "ExceptionLevel=" << ptYield->iExceptionLevel << endl;							// Alarm type: Standard, Critical...

	/////////////////////////////////////////////////////////////////////////////
	// SYL-SBL specifics
	/////////////////////////////////////////////////////////////////////////////
	hTasks << "Database=" << ptYield->strDatabase << endl;
	hTasks << "TestingStage=" << ptYield->strTestingStage << endl;
	hTasks << "RuleType=" << QString::number(ptYield->iSYA_Rule) << endl;						// Rule: 0=N*Sigma, 1=N*IQR
	hTasks << "RuleTypeString=" << ptYield->strSYA_Rule << endl;								// Rule string: N*Sigma, N*IQR
	hTasks << "N1_Parameter=" << QString::number(ptYield->fSYA_N1_value) << endl;				// N1 parameter
	hTasks << "N2_Parameter=" << QString::number(ptYield->fSYA_N2_value) << endl;				// N1 parameter
	hTasks << "MinimumLotsRequired=" << QString::number(ptYield->iSYA_LotsRequired) << endl;	// Minimum Total lots required for computing new SYL-SBL
	hTasks << "ValidityPeriod=" << QString::number(ptYield->iSYA_Period) << endl;				// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...
    hTasks << "ExpirationDate=" <<
        QString::number(ptYield->cExpiration.day()) << " "
		<< QString::number(ptYield->cExpiration.month()) << " "
		<< QString::number(ptYield->cExpiration.year()) << endl;							// Period for reprocessing SYL/SBL: 0=1week,1=1Month,2...
	hTasks << "SBL1_LL_Disabled=" << ptYield->strSYA_SBL1_LL_Disabled << endl;				// List of Binnings for which the SBL1 LL should be disabled
	hTasks << "SBL1_HL_Disabled=" << ptYield->strSYA_SBL1_HL_Disabled << endl;				// List of Binnings for which the SBL1 HL should be disabled
	hTasks << "SBL2_LL_Disabled=" << ptYield->strSYA_SBL2_LL_Disabled << endl;				// List of Binnings for which the SBL2 LL should be disabled
	hTasks << "SBL2_HL_Disabled=" << ptYield->strSYA_SBL2_HL_Disabled << endl;				// List of Binnings for which the SBL2 HL should be disabled
	hTasks << "SYL1_LL_Disabled=" << (ptYield->bSYA_SYL1_LL_Disabled ? "1" : "0") << endl;		// True if SYL1 LL should be disabled
	hTasks << "SYL1_HL_Disabled=" << (ptYield->bSYA_SYL1_HL_Disabled ? "1" : "0") << endl;		// True if SYL1 HL should be disabled
	hTasks << "SYL2_LL_Disabled=" << (ptYield->bSYA_SYL2_LL_Disabled ? "1" : "0") << endl;		// True if SYL2 LL should be disabled
	hTasks << "SYL2_HL_Disabled=" << (ptYield->bSYA_SYL2_HL_Disabled ? "1" : "0") << endl;		// True if SYL2 HL should be disabled
	hTasks << "IgnoreDataPointsWithNullSigma=" << (ptYield->bSYA_IgnoreDataPointsWithNullSigma ? "1" : "0") << endl;	// Set to true if datapoints with null sigma should be ignored
	hTasks << "IgnoreOutliers=" << (ptYield->bSYA_IgnoreOutliers ? "1" : "0") << endl;			// Set to true if outliers should be ignored
	hTasks << "MinDataPoints=" << QString::number(ptYield->iSYA_MinDataPoints) << endl;			// Minimum datapoints (wafers, lots if FT) to compute SYL/SBL
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section YIELD MONITORING RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionYieldMonitoring_RDB(GexMoYieldMonitoringTaskData_RDB *ptYield_RDB)
{
	hTasks << "<task_yield_monitoring_rdb>" << endl;
	hTasks << "TaskTitle=" << ptYield_RDB->strTitle << endl;

	// Write all Standard Yield Rules
	QList <GexMoYieldMonitoringTaskData *>::iterator itRules;
	GexMoYieldMonitoringTaskData *ptYield;
	if(ptYield_RDB->cYieldRules.count() > 0)
	{

		for (itRules = ptYield_RDB->cYieldRules.begin(); itRules != ptYield_RDB->cYieldRules.end(); ++itRules)
		{
			// Handle to Rule
			ptYield = *itRules;

			// Write rule to disk
			hTasks << "<task_yield_monitoring_rdb_YL>" << endl;

			// Write rule to disk
			SaveTaskSectionYieldMonitoring_RDB_OneRule(ptYield);

			hTasks << "</task_yield_monitoring_rdb_YL>" << endl;

		}
	}

	// Write all SYA rule
	if(ptYield_RDB->cYield_SYA_Rules.count() > 0)
	{
		for (itRules = ptYield_RDB->cYield_SYA_Rules.begin(); itRules != ptYield_RDB->cYield_SYA_Rules.end(); ++itRules)
		{
			// Handle to Rule
			ptYield = *itRules;

			// Write rule to disk
			hTasks << "<task_yield_monitoring_rdb_SYA>" << endl;

			// Write rule to disk
			SaveTaskSectionYieldMonitoring_RDB_OneRule(ptYield);

			hTasks << "</task_yield_monitoring_rdb_SYA>" << endl;
		}
	}

	hTasks << "</task_yield_monitoring_rdb>" << endl;
}


///////////////////////////////////////////////////////////
// Save Tasks to disk: Section SPEC MONITORING
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionSpecMonitoring(GexMoSpecMonitoringTaskData *ptSpec)
{
	hTasks << "<task_spec_monitoring>" << endl;
	hTasks << "Title=" << ptSpec->strTitle << endl;
	hTasks << "ProductID=" << ptSpec->strProductID << endl;
	hTasks << "Parameter=" << ptSpec->strParameter << endl;
	hTasks << "MonitorInfo=" << ptSpec->iMonitorInfo << endl;
	hTasks << "LowSpec=" << ptSpec->strLowSpec << endl;
	hTasks << "HighSpec=" << ptSpec->strHighSpec << endl;
	hTasks << "EmailFrom=" << ptSpec->strEmailFrom << endl;
	hTasks << "Emails=" << ptSpec->strEmailNotify << endl;

	if(ptSpec->bHtmlEmail)
		hTasks << "EmailFormat=HTML" << endl;
	else
		hTasks << "EmailFormat=TEXT" << endl;

	hTasks << "ExceptionLevel=" << ptSpec->iExceptionLevel << endl;	// Alarm type: Standard, Critical...

	hTasks << "</task_spec_monitoring>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section REPORTING
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionReporting(GexMoReportingTaskData *ptReporting)
{
	QString	strEmailNotify;		// Email addresses to notify.

	hTasks << "<task_reporting>" << endl;
	hTasks << "Title=" << ptReporting->strTitle << endl;
	hTasks << "ScriptPath=" << ptReporting->strScriptPath << endl;
	hTasks << "Frequency=" << ptReporting->iFrequency << endl;
	hTasks << "DayOfWeek=" << ptReporting->iDayOfWeek << endl;

	if(ptReporting->bExecutionWindow)
		hTasks << "ExecWindow=YES" << endl;
	else
		hTasks << "ExecWindow=NO" << endl;

	hTasks << "StartTime=" << ptReporting->cStartTime.hour() << ",";
	hTasks << ptReporting->cStartTime.minute() << endl;
	hTasks << "StopTime=" << ptReporting->cStopTime.hour() << ",";
	hTasks << ptReporting->cStopTime.minute() << endl;

	hTasks << "NotificationType=" << ptReporting->iNotificationType << endl;
	hTasks << "Emails=" << ptReporting->strEmailNotify << endl;

	if(ptReporting->bHtmlEmail)
		hTasks << "EmailFormat=HTML" << endl;
	else
		hTasks << "EmailFormat=TEXT" << endl;

	hTasks << "LastExecuted=" << ptReporting->tLastExecuted << endl;
	hTasks << "</task_reporting>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section STATUS
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionStatus(GexMoStatusTaskData *ptStatus)
{
	int iOneWebPerDatabase;
	QString	strEmailNotify;		// Email addresses to notify.

	hTasks << "<task_status>" << endl;
	hTasks << "Title=" << ptStatus->strTitle << endl;

	if(ptStatus->bOneWebPerDatabase == true)
		iOneWebPerDatabase = 1;
	else
		iOneWebPerDatabase = 0;
	hTasks << "WebOrganization=" << QString::number(iOneWebPerDatabase) << endl;

	hTasks << "IntranetPath=" << ptStatus->strIntranetPath << endl;
	hTasks << "HomePage=" << ptStatus->strHomePage << endl;
	hTasks << "ReportURL=" << ptStatus->strReportURL << endl;
	hTasks << "</task_status>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section FILE CONVERTER
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionFileConverter(GexMoFileConverterTaskData *ptConverter)
{
	hTasks << "<task_file_converter>" << endl;
	hTasks << "Title=" << ptConverter->strTitle << endl;
	hTasks << "InputFolder=" << ptConverter->strInputFolder << endl;
	hTasks << "ImportFileExtensions=" << ptConverter->strFileExtensions << endl;
	hTasks << "Frequency=" << ptConverter->iFrequency << endl;
	hTasks << "DayOfWeek=" << ptConverter->iDayOfWeek << endl;
	hTasks << "OutputFolder=" << ptConverter->strOutputFolder << endl;
	hTasks << "OutputFormat=" << ptConverter->iFormat << endl;
	hTasks << "TimeStampFile=" << (int) ptConverter->bTimeStampName << endl;
	hTasks << "SuccessMode=" << ptConverter->iOnSuccess << endl;
	hTasks << "SuccessFolder=" << ptConverter->strOutputSuccess << endl;
	hTasks << "FailMode=" << ptConverter->iOnError << endl;
	hTasks << "FailFolder=" << ptConverter->strOutputError << endl;
	hTasks << "</task_file_converter>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section OUTLIER REMOVAL
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionOutlierRemoval(GexMoOutlierRemovalTaskData *ptOutlier)
{
	hTasks << "<task_outlier_monitoring>" << endl;
	hTasks << "Title=" << ptOutlier->strTitle << endl;
	hTasks << "ProductID=" << ptOutlier->strProductID << endl;
	hTasks << "AlarmLevel=" << ptOutlier->lfAlarmLevel << endl;
	hTasks << "AlarmType=" << ptOutlier->iAlarmType << endl;
	hTasks << "MinimumParts=" << ptOutlier->lMinimumyieldParts << endl;
	int	iValue;
	if(ptOutlier->bNotifyShapeChange)
		iValue = 1;
	else
		iValue = 0;
	hTasks << "NotifyShapeChange=" << iValue << endl;

	hTasks << "CompositeEtestAlarm=" << ptOutlier->lCompositeEtestAlarm << endl;
	hTasks << "CompositeExclusionZoneAlarm=" << ptOutlier->lCompositeExclusionZoneAlarm << endl;
	hTasks << "Emails=" << ptOutlier->strEmailNotify << endl;

	if(ptOutlier->bHtmlEmail)
		hTasks << "EmailFormat=HTML" << endl;
	else
		hTasks << "EmailFormat=TEXT" << endl;

	hTasks << "EmailReportType=" << ptOutlier->iEmailReportType << endl;
	hTasks << "NotificationType=" << ptOutlier->iNotificationType << endl;
	hTasks << "ExceptionLevel=" << ptOutlier->iExceptionLevel << endl;	// Alarm type: Standard, Critical...

	hTasks << "</task_outlier_monitoring>" << endl;
}

///////////////////////////////////////////////////////////
// Save Tasks to disk: Section Auto Admin
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTaskSectionAutoAdmin(GexMoAutoAdminTaskData *ptAutoAdmin)
{
	hTasks << "<task_autoadmin>" << endl;
	hTasks << "Title=" << ptAutoAdmin->strTitle << endl;

	hTasks << "StartTime=" << ptAutoAdmin->cStartTime.hour() << ",";
	hTasks << ptAutoAdmin->cStartTime.minute() << endl;

	hTasks << "Emails=" << ptAutoAdmin->strEmailNotify << endl;
	hTasks << "KeepReportDuration=" << ptAutoAdmin->iKeepReportDuration << endl;
	hTasks << "LogContents=" << ptAutoAdmin->iLogContents << endl;
	hTasks << "LastExecuted=" << ptAutoAdmin->tLastExecuted << endl;

	// Shell on alarms
	hTasks << "ShellYieldAlarm=" << ptAutoAdmin->strShellYieldAlarm_Std << endl;
	hTasks << "ShellParameterAlarm=" << ptAutoAdmin->strShellParameterAlarm_Std << endl;
	hTasks << "ShellPatAlarm=" << ptAutoAdmin->strShellPatAlarm_Std << endl;
	hTasks << "ShellYieldAlarmCritical=" << ptAutoAdmin->strShellYieldAlarm_Critical << endl;
	hTasks << "ShellParameterAlarmCritical=" << ptAutoAdmin->strShellParameterAlarm_Critical << endl;
	hTasks << "ShellPatAlarmCritical=" << ptAutoAdmin->strShellPatAlarm_Critical << endl;

	hTasks << "</task_autoadmin>" << endl;
}

///////////////////////////////////////////////////////////
// Create a new task
///////////////////////////////////////////////////////////
void GexMoScheduler::OnNewTask(void)
{
	OnTaskProperties(true);
}

///////////////////////////////////////////////////////////
// Edit task properties
///////////////////////////////////////////////////////////
void GexMoScheduler::OnTaskProperties(bool bCreateNewTask)
{

	// YIELDMANDB
	// Check if yieldman is connected
	if( bCreateNewTask
	&&	pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
		return;

	// Check if a user is connected
	if( bCreateNewTask
	&&	pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
	&& !pGexMainWindow->m_pYieldManDb->IsConnected())
	{
		return;
	}

	QTableWidget *ptTable = NULL;
	QWidget * pCurrentTab = tabWidget->currentWidget();

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	int iTaskType;

	if(pCurrentTab == tabTasksDataPump)
	{
		iTaskType = GEXMO_TASK_DATAPUMP;
		ptTable = tableWidgetTasksListDataPump;
	}
	else
	if(pCurrentTab == tabTasksReporting)
	{
		iTaskType = GEXMO_TASK_REPORTING;
		ptTable = tableWidgetTasksListReporting;
	}
	else
	if(pCurrentTab == tabTasksParameterSpecs)
	{
		iTaskType = GEXMO_TASK_SPECMONITOR;
		ptTable = tableWidgetTasksListParameterSpecs;
	}
	else
	if(pCurrentTab == tabTasksConverter)
	{
		iTaskType = GEXMO_TASK_CONVERTER;
		ptTable = tableWidgetTasksListConverter;
	}
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
	{
		iTaskType = GEXMO_TASK_OUTLIER_REMOVAL;
		ptTable = tableWidgetTasksListOutlierRemoval;
	}
	else
	if(pCurrentTab == tabTasksYield)
	{
		iTaskType = GEXMO_TASK_YIELDMONITOR;
		ptTable = tableWidgetTasksListYield;
	}
	else
	if(pCurrentTab == tabTasksYieldRdb)
	{
		iTaskType = GEXMO_TASK_YIELDMONITOR_RDB;
		ptTable = tableWidgetTasksListYieldRdb;
	}
	else
	if(pCurrentTab == tabTasksStatus)
	{
		iTaskType = GEXMO_TASK_STATUS;
		ptTable = tableWidgetTasksListStatus;
	}

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	CGexMoTaskItem *ptTaskItem = NULL;

	if(!bCreateNewTask)
	{
		// If not for creation
		// Task must exist

		ptItem = ptTable->item(ptTable->currentRow(),0);
		// Check if found a selection (the 1st one)
		if(ptItem == NULL)
			return;

		int iTaskId = ptItem->text().toInt();
		ptTaskItem = FindTaskInList(iTaskId);
		if(ptTaskItem == NULL)
			return;	// Task not found in list...
	}



	bool	bCheckDatabase = false;
	QString strTaskName;
	QString strDataBaseName;
	// Call the relevant dialog box (& fill it with relevant data)
	switch(iTaskType)
	{
		case GEXMO_TASK_DATAPUMP:
			CreateTaskDataPump(ptTaskItem,ptItem);
			bCheckDatabase	= true;
			strTaskName		= ptTaskItem->ptDataPump->strTitle;
			strDataBaseName = ptTaskItem->ptDataPump->strDatabaseTarget;
			break;
		case GEXMO_TASK_YIELDMONITOR:
			CreateTaskYieldMonitoring(ptTaskItem,ptItem);
			bCheckDatabase = true;
			strTaskName		= ptTaskItem->ptYield->strTitle;
			strDataBaseName = ptTaskItem->ptYield->strDatabase;
			break;
		case GEXMO_TASK_YIELDMONITOR_RDB:
			CreateTaskYieldMonitoring_RDB(ptTaskItem,ptItem);
			bCheckDatabase = true;
			strTaskName		= ptTaskItem->ptYield_RDB->strTitle;
			strDataBaseName = ptTaskItem->strDatabaseName;
			break;
		case GEXMO_TASK_SPECMONITOR:
			CreateTaskSpecMonitoring(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptSpec->strTitle;
			strDataBaseName = "";
			break;
		case GEXMO_TASK_REPORTING:
			CreateTaskReporting(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptReporting->strTitle;
			strDataBaseName = "";
			break;
		case GEXMO_TASK_STATUS:
			CreateTaskStatus(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptStatus->strTitle;
			strDataBaseName = "";
			break;
		case GEXMO_TASK_CONVERTER:
			CreateTaskFileConverter(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptConverter->strTitle;
			strDataBaseName = "";
			break;
		case GEXMO_TASK_OUTLIER_REMOVAL:
			CreateTaskOutlierRemoval(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptOutlier->strTitle;
			strDataBaseName = "";
			break;
		case GEXMO_TASK_AUTOADMIN:
			CreateTaskAutoAdmin(ptTaskItem,ptItem);
			bCheckDatabase = false;
			strTaskName		= ptTaskItem->ptAutoAdmin->strTitle;
			strDataBaseName = "";
			break;
	}


	// Then Check if the Database is referenced
	if(bCheckDatabase)
	{
		GexDatabaseEntry* pDatabaseEntry = NULL;

		if(!strDataBaseName.isEmpty())
			pDatabaseEntry = pGexMainWindow->pDatabaseCenter->FindDatabaseEntry(strDataBaseName);


		if(pDatabaseEntry)
		{
			ptTaskItem->iStatus = 1;
			ptTaskItem->iDatabaseId = pDatabaseEntry->m_nDatabaseId;

			// And save this task in YieldManDb only if no problem
			SaveDbTasks(ptTaskItem);
		}
		else
		{
			ptTaskItem->iStatus = -1;
			ptTaskItem->bEnabled = false;
		}

	}
	else
	{
		ptTaskItem->iDatabaseId = -1;
		ptTaskItem->iStatus = 1;

		// And save this task in YieldManDb only if no problem
		SaveDbTasks(ptTaskItem);
	}
}

///////////////////////////////////////////////////////////
// Duplicate selected task
///////////////////////////////////////////////////////////
void GexMoScheduler::OnDuplicateTask(void)
{
	// YIELDMANDB
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
		return;

	// Check if a user is connected
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
	&& !pGexMainWindow->m_pYieldManDb->IsConnected())
	{
		return;
	}

	QWidget * pCurrentTab = tabWidget->currentWidget();

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	QTableWidget *ptTable = NULL;

	if(pCurrentTab == tabTasksDataPump)
		ptTable = tableWidgetTasksListDataPump;
	else
	if(pCurrentTab == tabTasksReporting)
		ptTable = tableWidgetTasksListReporting;
	else
	if(pCurrentTab == tabTasksParameterSpecs)
		ptTable = tableWidgetTasksListParameterSpecs;
	else
	if(pCurrentTab == tabTasksConverter)
		ptTable = tableWidgetTasksListConverter;
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
		ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nRowDuplicated = ptTable->currentRow();

	ptItem = ptTable->item(nRowDuplicated,0);
	// Check if found a selection (the 1st one)
	if(ptItem == NULL)
		return;

	// Duplicate objects selected
	int	nIndex=0;
	QString strTitle = "Copy of ";
	strTitle += ptTable->item(nRowDuplicated,4)->text();

	int iTaskId = ptTable->item(nRowDuplicated,0)->text().toInt();
	// Duplicate internal structures
	DuplicateTaskInList(iTaskId,strTitle);

	// Find
	CGexMoTaskItem *ptTask = FindTaskInList(pGexMoTasksList.count());
	if(ptTask == NULL)
		return;

	// Update ListView
	UpdateListViewItem(ptTask,NULL,true);

	SaveDbTasks(ptTask);

}

///////////////////////////////////////////////////////////
// Execute given task
///////////////////////////////////////////////////////////
void GexMoScheduler::OnRunTask(void)
{
	// YIELDMANDB
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
		return;

	// Check if a user is connected
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
	&& !pGexMainWindow->m_pYieldManDb->IsConnected())
	{
		return;
	}

	QWidget * pCurrentTab = tabWidget->currentWidget();

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	QTableWidget *ptTable = NULL;

	if(pCurrentTab == tabTasksDataPump)
		ptTable = tableWidgetTasksListDataPump;
	else
	if(pCurrentTab == tabTasksReporting)
		ptTable = tableWidgetTasksListReporting;
	else
	if(pCurrentTab == tabTasksParameterSpecs)
		ptTable = tableWidgetTasksListParameterSpecs;
	else
	if(pCurrentTab == tabTasksConverter)
		ptTable = tableWidgetTasksListConverter;
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
		ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	ptItem = ptTable->item(ptTable->currentRow(),0);

	// Check if found a selection (the 1st one)
	if(ptItem == NULL)
		return;

	// Find selected items

	// Reset timestamp for selected tasks. 0 is a reset, 1 means reset+ignore execution window so task is always executed on request!
	int iTaskId = ptItem->text().toInt();

	CGexMoTaskItem *ptTask = FindTaskInList(iTaskId);
	switch(ptTask->iTaskType)
	{
		case GEXMO_TASK_DATAPUMP:
			ptTask->ptDataPump->tLastExecuted = 1;
			break;
		case GEXMO_TASK_REPORTING:
			ptTask->ptReporting->tLastExecuted = 1;
			break;
		case GEXMO_TASK_AUTOADMIN:
			ptTask->ptAutoAdmin->tLastExecuted = 1;
			break;
		case GEXMO_TASK_CONVERTER:
			ptTask->ptConverter->tLastExecuted = 1;
			break;
	}

	// Execute tasks.
	OnCheckScheduler();

	// Save new task list to disk
	SaveDbTasks(ptTask);
}

///////////////////////////////////////////////////////////
// Delete given task(s)
///////////////////////////////////////////////////////////
void GexMoScheduler::OnDeleteTask(void)
{
	// YIELDMANDB
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
		return;

	// Check if a user is connected
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
	&& !pGexMainWindow->m_pYieldManDb->IsConnected())
	{
		return;
	}

	QWidget * pCurrentTab = tabWidget->currentWidget();
	bool	bNeedConfirmation=true;

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	QTableWidget *ptTable = NULL;

	if(pCurrentTab == tabTasksDataPump)
		ptTable = tableWidgetTasksListDataPump;
	else
	if(pCurrentTab == tabTasksReporting)
		ptTable = tableWidgetTasksListReporting;
	else
	if(pCurrentTab == tabTasksParameterSpecs)
		ptTable = tableWidgetTasksListParameterSpecs;
	else
	if(pCurrentTab == tabTasksConverter)
		ptTable = tableWidgetTasksListConverter;
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
		ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	ptItem = ptTable->item(ptTable->currentRow(),0);

	// Check if found a selection (the 1st one)
	if(ptItem == NULL)
		return;

	// Remove object if it is selected
	// Ask confirmation first
	if(bNeedConfirmation == true)
	{
        bool lOk;
        GS::Gex::Message::request("Delete Tasks", "Confirm to permanently remove\nthe selected task(s) ?", lOk);
        if (! lOk)
        {
            return;
        }

		// Only ask once!
		bNeedConfirmation=false;
	}

	// Delete Task entry from internal list.
	DeleteTaskInList(ptItem->text().toInt());

	// Remove selected item from list
	ptTable->removeRow(ptTable->currentRow());

	// Save new task list ot disk
	SaveDbTasks();
}

///////////////////////////////////////////////////////////
// Update List View item: Data Pump
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{

	switch(ptTaskData->iTaskType)
	{
		case GEXMO_TASK_DATAPUMP:
			UpdateListViewDataPumpItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_YIELDMONITOR:
			UpdateListViewYieldItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_YIELDMONITOR_RDB:
			UpdateListViewYieldItem_RDB(ptTaskData,bCreateTask);
			break;
		case GEXMO_TASK_SPECMONITOR:
			UpdateListViewSpecItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_REPORTING:
			UpdateListViewReportingItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_STATUS:
			UpdateListViewStatusItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_CONVERTER:
			UpdateListViewConverterItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_OUTLIER_REMOVAL:
			UpdateListViewOutlierItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
		case GEXMO_TASK_AUTOADMIN:
			UpdateListViewAutoAdminItem(ptTaskData,ptListViewItem,bCreateTask);
			break;
	}

}

///////////////////////////////////////////////////////////
// Update List View item: Data Pump
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewDataPumpItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListDataPump;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	int nCurrentRow = ptTable->currentRow();
	if(ptListViewItem)
		nCurrentRow = ptListViewItem->row();

	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strAlarmCondition;
	QString strEmails;
	QString strFrequency = gexMoLabelTaskFrequency[ptTaskData->ptDataPump->iFrequency];
	if(ptTaskData->ptDataPump->iFrequency >= GEXMO_TASKFREQ_1WEEK)
	{
		// Frequency is over 1 week...then specify the day of the week!
		strFrequency += " (";
		strFrequency += gexMoLabelTaskFrequencyDayOfWeek[ptTaskData->ptDataPump->iDayOfWeek];
		strFrequency += ") ";
	}

	if(ptTaskData->ptDataPump->bCheckYield == false)
	{
		strAlarmCondition = strEmails = " -";
	}
	else
	{
		strAlarmCondition = "Yield < ";
		strAlarmCondition += QString::number(ptTaskData->ptDataPump->iAlarmLevel);
		strAlarmCondition += "%";
		strEmails = ptTaskData->ptDataPump->strEmailNotify;
	}

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}

		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptDataPump->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptDataPump->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: Yield Monitoring
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewYieldItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{

	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListYield;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	int nCurrentRow = ptTable->currentRow();
	if(ptListViewItem)
		nCurrentRow = ptListViewItem->row();

	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = " - ";
	QString strAlarmCondition;
	QString strEmails;

	strAlarmCondition = "Yield ";
	if(ptTaskData->ptYield->iAlarmIfOverLimit == 0)
		strAlarmCondition += "< ";	// Alarm if Yield UNDER Limit
	else
		strAlarmCondition += "> ";	// Alarm if Yield OVER Limit
	strAlarmCondition += QString::number(ptTaskData->ptYield->iAlarmLevel);
	strAlarmCondition += "%";
	strEmails = ptTaskData->ptYield->strEmailNotify;

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails

		}

		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptYield->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptYield->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}

}

///////////////////////////////////////////////////////////
// Update List View item: Yield Monitoring RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewYieldItem_RDB(CGexMoTaskItem *ptTaskData,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListYieldRdb;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strTaskLabel = " - Yield Monitoring SYL/SBL";
    QString strRDB_YieldTitle = "Yield Management Rules";
	QString strFrequency = " - ";
	QString strAlarmCondition = " - ";
	QString strEmails = " - ";

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskLabel);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskLabel);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: Spec Monitoring
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewSpecItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListParameterSpecs;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = " - ";
	QString strAlarmCondition;
	QString strEmails;

	if(ptTaskData->ptSpec->strLowSpec.isEmpty() == false)
	{
		strAlarmCondition += gexMoLabelSpecInfo[ptTaskData->ptSpec->iMonitorInfo];
		strAlarmCondition += " < " + ptTaskData->ptSpec->strLowSpec;
		if(ptTaskData->ptSpec->strHighSpec.isEmpty() == false)
			strAlarmCondition += ", or ";
	}

	if(ptTaskData->ptSpec->strHighSpec.isEmpty() == false)
	{
		strAlarmCondition += gexMoLabelSpecInfo[ptTaskData->ptSpec->iMonitorInfo];
		strAlarmCondition += " > " + ptTaskData->ptSpec->strHighSpec;
	}

	strEmails = ptTaskData->ptSpec->strEmailNotify;

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptSpec->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptSpec->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: Reporting
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewReportingItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListReporting;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strAlarmCondition = " -";
	QString strEmails = ptTaskData->ptReporting->strEmailNotify;
	QString strFrequency = gexMoLabelTaskFrequency[ptTaskData->ptReporting->iFrequency];
	if(ptTaskData->ptReporting->iFrequency >= GEXMO_TASKFREQ_1WEEK)
	{
		// Frequency is over 1 week...then specify the day of the week!
		strFrequency += " (";
		strFrequency += gexMoLabelTaskFrequencyDayOfWeek[ptTaskData->ptReporting->iDayOfWeek];
		strFrequency += ") ";
	}

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptReporting->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptReporting->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: Status
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewStatusItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListStatus;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = " -";
	QString strAlarmCondition = " -";
	QString strEmails = " -";

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptStatus->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptStatus->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: File Converter
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewConverterItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListConverter;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = gexMoLabelTaskFrequency[ptTaskData->ptConverter->iFrequency];
	if(ptTaskData->ptConverter->iFrequency >= GEXMO_TASKFREQ_1WEEK)
	{
		// Frequency is over 1 week...then specify the day of the week!
		strFrequency += " (";
		strFrequency += gexMoLabelTaskFrequencyDayOfWeek[ptTaskData->ptConverter->iDayOfWeek];
		strFrequency += ") ";
	}
	QString strAlarmCondition = " -";
	QString strEmails = " -";

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptConverter->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptConverter->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: PAT-Man (Outlier Removal)
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewOutlierItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = " - ";
	QString strAlarmCondition;
	QString strEmails;

	strAlarmCondition = "Yield loss ";
	strAlarmCondition += "> ";	// Alarm if Yield loss OVER limit
	strAlarmCondition += QString::number(ptTaskData->ptOutlier->lfAlarmLevel);
	if(ptTaskData->ptOutlier->iAlarmType == 0)
		strAlarmCondition += "%";
	else
		strAlarmCondition += " parts";
	strEmails = ptTaskData->ptOutlier->strEmailNotify;

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptOutlier->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptOutlier->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Update List View item: AutoAdmin
///////////////////////////////////////////////////////////
void GexMoScheduler::UpdateListViewAutoAdminItem(CGexMoTaskItem *ptTaskData,QTableWidgetItem *ptListViewItem,bool bCreateTask)
{
	QTableWidget *ptTable = NULL;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	int nCurrentRow = ptTable->currentRow();
	if(nCurrentRow < 0)
		ptTable->setRowCount(0);	// Reset raw count

	QString strTaskId = QString::number(ptTaskData->iTaskId);
	QString strUserId = QString::number(ptTaskData->iUserId);
	QString	strGroupId = QString::number(ptTaskData->iGroupId);
	QString	strPermissions = QString::number(ptTaskData->iPermissions);
	QString strFrequency = " Daily";
	QString strAlarmCondition = " -";
	QString strEmails = ptTaskData->ptAutoAdmin->strEmailNotify;

	if(bCreateTask == true)
	{
		// For the first load
		// Check if all columns are created
		if(ptTable->columnCount() < 8)
		{
			// Have to create all needed columns
			ptTable->setColumnCount(8);
			QStringList lstLabels;
			lstLabels << "TaskId" << "UserId" << "GroupId" << "Permissions" << "Title" << "Frequency" << "AlarmCondition" << "Emails";
			ptTable->setHorizontalHeaderLabels(lstLabels);

			// Set minimum size for the columns
			int nIndex = 0;
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,50);
			ptTable->setColumnWidth (nIndex++,150);	// Title
			ptTable->setColumnWidth (nIndex++,80);	// Frequency
			ptTable->setColumnWidth (nIndex++,80);	// Alarm condiction
			ptTable->setColumnWidth (nIndex++,200);	// Emails
		}
		// Insert item into ListView
		nCurrentRow = ptTable->rowCount();
		ptTable->setRowCount(1+nCurrentRow);
		ptTable->setRowHeight(nCurrentRow, 20);

		// Add entry: title, task type, Frequency, Alarm condition, email addresses
		int nIndex = 0;
		fillCellData(ptTable,nCurrentRow,nIndex++,strTaskId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strUserId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strGroupId);
		fillCellData(ptTable,nCurrentRow,nIndex++,strPermissions);
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptAutoAdmin->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
	else
	{
		// Overwrite existing list view item.
		int nIndex = 4;
		fillCellData(ptTable,nCurrentRow,nIndex++,ptTaskData->ptAutoAdmin->strTitle);
		fillCellData(ptTable,nCurrentRow,nIndex++,strFrequency);
		fillCellData(ptTable,nCurrentRow,nIndex++,strAlarmCondition);
		fillCellData(ptTable,nCurrentRow,nIndex++,strEmails);

		// Select new database entry in the list.
		ptTable->selectRow(nCurrentRow);
	}
}

///////////////////////////////////////////////////////////
// Create task: Data Pump
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskDataPump(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskDataPump cDataPumpTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cDataPumpTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cDataPumpTask.children(),false);
		}
	}

	// Display Dialog box.
datapump_dlg:
	if(cDataPumpTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cDataPumpTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cDataPumpTask.LineEditTitle->setFocus();
		goto datapump_dlg;
	}
*/

	// If valid DataPump task entered, save data into internal structure list.
	GexMoDataPumpTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoDataPumpTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_DATAPUMP;
	}
	else
		ptTaskData = ptTaskItem->ptDataPump;

	// Copy Dialog box fields into structure
	*ptTaskData = cDataPumpTask.cDataPumpData;
	ptTaskItem->ptDataPump = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Yield monitoring (flat files only)
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskYieldMonitoring(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskYieldCheck cYieldCheckTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cYieldCheckTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cYieldCheckTask.children(),false);

		}
	}

	// Display Dialog box.
yieldcheck_dlg:
	if(cYieldCheckTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cYieldCheckTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cYieldCheckTask.LineEditTitle->setFocus();
		goto yieldcheck_dlg;
	}
*/

	// If valid DataPump task entered, save data into internal structure list.
	GexMoYieldMonitoringTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoYieldMonitoringTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_YIELDMONITOR;
	}
	else
		ptTaskData = ptTaskItem->ptYield;

	// Copy Dialog box fields into structure
	*ptTaskData = cYieldCheckTask.cYieldData;
	ptTaskItem->ptYield = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Yield monitoring
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskYieldMonitoring_RDB(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	// We do have a Galaxy RDB database, then display RDB-Yield managment GUI
	QString strTaskName = "Yield Management Rules";
	GexMoCreateTaskYieldCheck_RDB cYieldCheckTask;

	// Chekc if RDV Yield Management task already exists
	//CGexMoTaskItem *ptTaskItem = FindTaskInList(strTaskName);


	// Copy into GUI all Yield rules
	if(ptTaskItem)
		cYieldCheckTask.loadYieldRules(ptTaskItem->ptYield_RDB);

	// Display Rule editor
	if(cYieldCheckTask.exec() != 1)
		return;

	// If valid DataPump task entered, save data into internal structure list.
	GexMoYieldMonitoringTaskData_RDB *ptTaskData;

	bool bCreateTask=false;
	if(ptTaskItem == NULL)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoYieldMonitoringTaskData_RDB;
		ptTaskData->strTitle = strTaskName;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_YIELDMONITOR_RDB;

		// Copy Dialog box fields into structure
		ptTaskItem->ptYield_RDB = ptTaskData;

		// If Task not already in list, add it.
		pGexMoTasksList.append(ptTaskItem);
		bCreateTask = true;
	}
	else
		ptTaskData = ptTaskItem->ptYield_RDB;

	// Retrieve all rules from GUI
	cYieldCheckTask.getYieldRules(ptTaskItem->ptYield_RDB);


	// Update ListView
	UpdateListViewYieldItem_RDB(ptTaskItem,bCreateTask);

	// Save new task list to disk
	SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Parameter Specs. monitoring
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskSpecMonitoring(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskSpecCheck cSpecCheckTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cSpecCheckTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cSpecCheckTask.children(),false);
		}
	}

	// Display Dialog box.
speccheck_dlg:
	if(cSpecCheckTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cSpecCheckTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cSpecCheckTask.LineEditTitle->setFocus();
		goto speccheck_dlg;
	}
*/
	// If valid Spec Check task entered, save data into internal structure list.
	GexMoSpecMonitoringTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoSpecMonitoringTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_SPECMONITOR;
	}
	else
		ptTaskData = ptTaskItem->ptSpec;

	// Copy Dialog box fields into structure
	*ptTaskData = cSpecCheckTask.cSpecData;
	ptTaskItem->ptSpec = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Reporting
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskReporting(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskReporting cReportingTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cReportingTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cReportingTask.children(),false);
		}
	}

	// Display Dialog box.
reporting_dlg:
	if(cReportingTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cReportingTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cReportingTask.LineEditTitle->setFocus();
		goto reporting_dlg;
	}
*/
	// If valid DataPump task entered, save data into internal structure list.
	GexMoReportingTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoReportingTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_REPORTING;
	}
	else
		ptTaskData = ptTaskItem->ptReporting;

	// Copy Dialog box fields into structure
	*ptTaskData = cReportingTask.cReportingData;
	ptTaskItem->ptReporting = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Status
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskStatus(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskStatus	cStatusTask;
	bool					bCreateTask = true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cStatusTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cStatusTask.children(),false);
		}
	}
	else
	{
		QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

		// Reject creation if 'Status' task already exists!
		lstIteratorTask.toFront();

		while(lstIteratorTask.hasNext())
		{
			ptTaskItem = lstIteratorTask.next();

			if(ptTaskItem->iTaskType == GEXMO_TASK_STATUS)
			{
                GS::Gex::Message::information(
                    szAppFullName,
                    "The 'Status' task already exists, and only one can be "
                    "created.\nYou are now going to edit the existing one!");

				// Select the 'Status' item to Edit it.
				/*
				QString strTaskName;
				Q3ListViewItem *ptItem = ListView->firstChild();
				while(ptItem != NULL)
				{
					if(ptTaskItem->ptStatus->strTitle == ptItem->text(1))
					{
						ListView->clearSelection();
						ListView->setSelected(ptItem,TRUE);
						OnTaskProperties();
						return;
					}

					// Move to next item
					ptItem = ptItem->itemBelow();
				};
				*/
				return;
			}
        };
		// Reset pointer as it was when entering!
		ptTaskItem = NULL;
	}

	// Display Dialog box.
status_dlg:
	if(cStatusTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cStatusTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cStatusTask.LineEditTitle->setFocus();
		goto status_dlg;
	}
*/
	// If valid Status task entered, save data into internal structure list.
	GexMoStatusTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoStatusTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_STATUS;
	}
	else
		ptTaskData = ptTaskItem->ptStatus;

	// Copy Dialog box fields into structure
	*ptTaskData = cStatusTask.cStatusData;
	ptTaskItem->ptStatus = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: File Converter (Batch convert to STDF or CSV)
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskFileConverter(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
    GexMoCreateTaskFileConverter cFileConverterTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cFileConverterTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;

		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cFileConverterTask.children(),false);
		}
	}

	// Display Dialog box.
file_convert_dlg:
	if(cFileConverterTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cFileConverterTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cFileConverterTask.LineEditTitle->setFocus();
		goto file_convert_dlg;
	}
*/
	// If valid task entered, save data into internal structure list.
	GexMoFileConverterTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoFileConverterTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_CONVERTER;
	}
	else
		ptTaskData = ptTaskItem->ptConverter;

	// Copy Dialog box fields into structure
	*ptTaskData = cFileConverterTask.cConverterData;
	ptTaskItem->ptConverter = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView: FILE CONVERTER
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Outlier Removal
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskOutlierRemoval(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
	GexMoCreateTaskOutlierRemoval cOutlierRemovalTask;
	bool	bCreateTask=true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cOutlierRemovalTask.LoadFields(ptTaskItem);
		bCreateTask = false;

		// Check if the user is allowed to modified this task
		bool	bReadOnlyTask=false;
		// Check if yieldman is connected
		if( pGexMainWindow->m_pYieldManDb
		&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
		&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
			bReadOnlyTask=true;

		// Check if a user is connected
		if( pGexMainWindow->m_pYieldManDb
		&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_ADMIN_LOGIN)
		&&  pGexMainWindow->m_pYieldManDb->IsConnected(true))
			bReadOnlyTask=false;
		else if(!pGexMainWindow->m_pYieldManDb->IsAllowedToModify(ptTaskItem->iUserId, ptTaskItem->iPermissions))
			bReadOnlyTask=true;

		if(bReadOnlyTask)
		{
			GexYieldManDbLogin::EnabledFieldItem(cOutlierRemovalTask.children(),false);
		}
	}

	// Display Dialog box.
outlier_removal_dlg:
	if(cOutlierRemovalTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	// Check on YieldManId and Folder
/*
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cOutlierRemovalTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cOutlierRemovalTask.LineEditTitle->setFocus();
		goto outlier_removal_dlg;
	}
*/
	// If valid DataPump task entered, save data into internal structure list.
	GexMoOutlierRemovalTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoOutlierRemovalTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_OUTLIER_REMOVAL;
	}
	else
		ptTaskData = ptTaskItem->ptOutlier;

	// Copy Dialog box fields into structure
	*ptTaskData = cOutlierRemovalTask.cOutlierRemovalData;
	ptTaskItem->ptOutlier = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
	{
		// YieldManDb
		ptTaskItem->iTaskId		= m_nIndexInvalidTask--;
		ptTaskItem->strName		= ptTaskData->strTitle;

		pGexMoTasksList.append(ptTaskItem);
	}

	// Update ListView
	UpdateListViewItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	//SaveTasks();
}

///////////////////////////////////////////////////////////
// Create task: Auto Admin
///////////////////////////////////////////////////////////
void GexMoScheduler::CreateTaskAutoAdmin(CGexMoTaskItem *ptTaskItem,QTableWidgetItem *ptListViewItem)
{
/*	GexMoCreateTaskAutoAdmin	cAutoAdminTask;
	bool						bCreateTask = true;

	// If Task already exists in the list, fills its fields
	if(ptTaskItem != NULL)
	{
		cAutoAdminTask.LoadFields(ptTaskItem);
		bCreateTask = false;
	}
	else
	{
		// Reject creation if 'Auto Admin' task already exists!
		QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

		lstIteratorTask.toFront();
		while(lstIteratorTask.hasNext())
		{
			ptTaskItem = lstIteratorTask.next();

			if(ptTaskItem->iTaskType == GEXMO_TASK_AUTOADMIN)
			{
                Message::information(pGexMainWindow, szAppFullName,
                "The 'Auto Admin' task already exists, and only one can be "
                "created.\nYou are now going to edit the existing one!");

				// Select the 'Auto Admin' item to Edit it.
				QString strTaskName;
				Q3ListViewItem *ptItem = ListView->firstChild();
				while(ptItem != NULL)
				{
					if(ptTaskItem->ptAutoAdmin->strTitle == ptItem->text(1))
					{
						ListView->clearSelection();
						ListView->setSelected(ptItem,TRUE);
						OnTaskProperties();						return;
					}

					// Move to next item
					ptItem = ptItem->itemBelow();
				};
				return;
			}
        };
		// Reset pointer as it was when entering!
		ptTaskItem = NULL;
	}

	// Display Dialog box.
autoadmin_dlg:
	if(cAutoAdminTask.exec() != 1)
		return;

	// Check if Title already exists (do not allow it!)
	CGexMoTaskItem *ptTaskItemDuplicate = FindTaskInList(cAutoAdminTask.LineEditTitle->text());
	if((bCreateTask == true) && (ptTaskItemDuplicate != NULL) ||
	( (bCreateTask == false) && (ptTaskItemDuplicate != NULL) && (ptTaskItemDuplicate != ptTaskItem) ))
	{
		QString strErrorMessage = "*Error* Task title already in use\nPlease enter a different title!";
        Message::information(pGexMainWindow, szAppFullName,strErrorMessage);
		cAutoAdminTask.LineEditTitle->setFocus();
		goto autoadmin_dlg;
	}

	// If valid Auto Admin task entered, save data into internal structure list.
	GexMoAutoAdminTaskData *ptTaskData;

	if(bCreateTask == true)
	{
		// Creating a new task.
		ptTaskItem = new CGexMoTaskItem;
		ptTaskData = new GexMoAutoAdminTaskData;
		// Copy all data entered
		ptTaskItem->iTaskType = GEXMO_TASK_AUTOADMIN;
	}
	else
		ptTaskData = ptTaskItem->ptAutoAdmin;

	// Copy Dialog box fields into structure
	*ptTaskData = cAutoAdminTask.cAutoAdminData;
	ptTaskItem->ptAutoAdmin = ptTaskData;

	// If Task not already in list, add it.
	if(bCreateTask == true)
		pGexMoTasksList.append(ptTaskItem);

	// Update ListView
	UpdateListViewAutoAdminItem(ptTaskItem,ptListViewItem,bCreateTask);

	// Save new task list to disk
	SaveTasks();
*/
}

///////////////////////////////////////////////////////////
// Timer based: check for tasks to execute...
// return false if day of week is not matching!
///////////////////////////////////////////////////////////
bool GexMoScheduler::AddDateFrequency(QDateTime *pDateTime,int iFrequency,int iDayOfWeek/*=-1*/)
{
	bool	bValidDayOfWeek=true;
	QDate	cCurrentDate = QDate::currentDate();
	int		iToday = cCurrentDate.dayOfWeek()-1;	// 0=Monday,...6=Sunday

	// If in Y123 mode, set frequency to every second
	if(ReportOptions.bY123WebMode)
	{
		*pDateTime = pDateTime->addSecs(1);
		return true;
	}

	switch(iFrequency)
	{
		case	GEXMO_TASKFREQ_1MIN:				// Task every: 1 minute
			*pDateTime = pDateTime->addSecs(60);
			break;

		case	GEXMO_TASKFREQ_2MIN:				// Task every: 2 minutes
			*pDateTime = pDateTime->addSecs(2*60);
			break;

		case	GEXMO_TASKFREQ_3MIN:				// Task every: 3 minutes
			*pDateTime = pDateTime->addSecs(3*60);
			break;

		case	GEXMO_TASKFREQ_4MIN:				// Task every: 4 minutes
			*pDateTime = pDateTime->addSecs(4*60);
			break;

		case	GEXMO_TASKFREQ_5MIN:				// Task every: 5 minutes
			*pDateTime = pDateTime->addSecs(5*60);
			break;

		case	GEXMO_TASKFREQ_10MIN:			// Task every: 10 minutes
			*pDateTime = pDateTime->addSecs(10*60);
			break;

		case	GEXMO_TASKFREQ_15MIN:			// Task every: 15 minutes
			*pDateTime = pDateTime->addSecs(15*60);
			break;

		case	GEXMO_TASKFREQ_30MIN:			// Task every: 30 minutes
			*pDateTime = pDateTime->addSecs(30*60);
			break;

		case	GEXMO_TASKFREQ_1HOUR:			// Task every: 1 hour
			*pDateTime = pDateTime->addSecs(60*60);
			break;

		case	GEXMO_TASKFREQ_2HOUR:			// Task every: 2 hours
			*pDateTime = pDateTime->addSecs(2*60*60);
			break;

		case	GEXMO_TASKFREQ_3HOUR:			// Task every: 3 hours
            *pDateTime = pDateTime->addSecs(3*60*60);
			break;

		case	GEXMO_TASKFREQ_4HOUR:			// Task every: 4 hours
            *pDateTime = pDateTime->addSecs(4*60*60);
			break;

		case	GEXMO_TASKFREQ_5HOUR:			// Task every: 5 hours
            *pDateTime = pDateTime->addSecs(5*60*60);
			break;

		case	GEXMO_TASKFREQ_6HOUR:			// Task every: 6 hours
            *pDateTime = pDateTime->addSecs(6*60*60);
			break;

		case	GEXMO_TASKFREQ_12HOUR:			// Task every: 12 hours
            *pDateTime = pDateTime->addSecs(12*60*60);
			break;

		case	GEXMO_TASKFREQ_1DAY:				// Task every: 1 day
            *pDateTime = pDateTime->addDays(1);
			break;

		case	GEXMO_TASKFREQ_2DAY:				// Task every: 2 days
            *pDateTime = pDateTime->addDays(2);
			break;

		case	GEXMO_TASKFREQ_3DAY:				// Task every: 3 days
            *pDateTime = pDateTime->addDays(3);
			break;

		case	GEXMO_TASKFREQ_4DAY:				// Task every: 4 days
            *pDateTime = pDateTime->addDays(4);
			break;

		case	GEXMO_TASKFREQ_5DAY:				// Task every: 5 days
            *pDateTime = pDateTime->addDays(5);
			break;

		case	GEXMO_TASKFREQ_6DAY:				// Task every: 6 days
            *pDateTime = pDateTime->addDays(6);
			break;

		case	GEXMO_TASKFREQ_1WEEK:			// Task every: 1 week
            *pDateTime = pDateTime->addDays(7);
			if(iToday != iDayOfWeek) bValidDayOfWeek = false;
			break;

		case	GEXMO_TASKFREQ_2WEEK:			// Task every: 2 weeks
            *pDateTime = pDateTime->addDays(14);
			if(iToday != iDayOfWeek) bValidDayOfWeek = false;
			break;

		case	GEXMO_TASKFREQ_3WEEK:			// Task every: 3 weeks
            *pDateTime = pDateTime->addDays(21);
			if(iToday != iDayOfWeek) bValidDayOfWeek = false;
			break;

		case	GEXMO_TASKFREQ_1MONTH:			// Task every: 1 month
            *pDateTime = pDateTime->addMonths(1);
			if(iToday != iDayOfWeek) bValidDayOfWeek = false;
			break;
	}

	// return 'true' if this task can be executed today!
	if(iDayOfWeek >= 0)
		return bValidDayOfWeek;
	return true;
}

///////////////////////////////////////////////////////////
// Display a message reviewing current task status
// return false if day of week is not matching!
///////////////////////////////////////////////////////////
void GexMoScheduler::DisplayStatusMessage(QString strText/*=""*/)
{
	TextLabelTaskStatus->setText(strText);
}

///////////////////////////////////////////////////////////
// Execute Alarm Shell...
// return: true on success (no error)
///////////////////////////////////////////////////////////
bool GexMoScheduler::LaunchAlarmShell(int iAlarmType,int iSeverity,int iAlarmCount,GexDatabaseKeysContent *pKeyContent)
{
	// Get handle to Auto Admin task.
	CGexMoTaskItem *ptAutoAdminTask = GetAutoAdminTask();

	// If no such task, then no shell enabled...quietly return
    if (ptAutoAdminTask == NULL)
		return true;

	QString	strShellFile;
	QString strArgumentsLine;
	switch(iAlarmType)
	{
			case ShellYieldAlarm:		// Yield alarm
				strShellFile = (iSeverity) ? ptAutoAdminTask->ptAutoAdmin->strShellYieldAlarm_Critical : ptAutoAdminTask->ptAutoAdmin->strShellYieldAlarm_Std;
			// Arg1: Alarm type
			strArgumentsLine = "YIELD_ALARM ";
			break;

		case ShellParameterAlarm:	// Parametric test alarm
			strShellFile = (iSeverity) ? ptAutoAdminTask->ptAutoAdmin->strShellParameterAlarm_Critical : ptAutoAdminTask->ptAutoAdmin->strShellParameterAlarm_Std;
			// Arg1: Alarm type
			strArgumentsLine = "PARAM_ALARM ";
			break;

		case ShellPatAlarm:			// PAT alarm
			strShellFile = (iSeverity) ? ptAutoAdminTask->ptAutoAdmin->strShellPatAlarm_Critical : ptAutoAdminTask->ptAutoAdmin->strShellPatAlarm_Std;
			// Arg1: Alarm type
			strArgumentsLine = "PAT_ALARM ";
			break;
	}

	// If no shell (or shell doesn't exist), simply quietly return.
	if(QFile::exists(strShellFile) == false)
		return true;

	// Arg2: Date & Time argument...
	strArgumentsLine += QDateTime::currentDateTime().toString(Qt::ISODate);

	// Arg3: Total alarms...
	strArgumentsLine += " " + QString::number(iAlarmCount);

	// Arg4: ProductID...
	if(pKeyContent->strProductID.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strProductID;

	// Arg5: LotID...
	if(pKeyContent->strLot.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strLot;

	// Arg6: SubLotID...
	if(pKeyContent->strSubLot.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strSubLot;

	// Arg7: WaferID...
	if(pKeyContent->strWaferID.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strWaferID;

	// Arg8: Tester name...
	if(pKeyContent->strTesterName.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strTesterName;

	// Arg9: Operator...
	if(pKeyContent->strOperator.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strOperator;

	// Arg10: Data file name...
	if(pKeyContent->strFileName.isEmpty())
		strArgumentsLine += " ?";
	else
		strArgumentsLine += " " + pKeyContent->strFileName;

	// Launch Shell command (minimized)
	#ifdef _WIN32
		// Replace '/' to '\' to avoid MS-DOS compatibility issues
		strArgumentsLine = strArgumentsLine.replace('/','\\');

		ShellExecuteA(NULL,
			   "open",
			   strShellFile.toLatin1().constData(),
			   strArgumentsLine.toLatin1().constData(),
			   NULL,
			   SW_SHOWMINIMIZED);
	#else
		strShellFile = strShellFile + " " + strArgumentsLine;
		system(strShellFile.toLatin1().constData());
	#endif

	// Success
	return true;
}

///////////////////////////////////////////////////////////
// Load Tasks from YieldManDb
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTasks(bool bForceReload/*=true*/)
{

	if((pGexMainWindow->eClientState != GexMainwindow::eState_NodeReady)
	&& (pGexMainWindow->eClientState != GexMainwindow::eState_LicenseGranted))
		return;

	// Before loading all tasks, have to load databases information
	pGexMainWindow->pDatabaseCenter->LoadDatabasesListIfEmpty();

	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
		return;

	// Check if load tasks from YieldManDb
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT))
	{

		// Load Tasks from YieldManDb

		// For the first run (YieldMan node must be updated and referenced in the ym_nodes table)
		if((ReportOptions.lProductID != GEX_DATATYPE_GEX_MONITORING)
		&& (pGexMainWindow->m_pYieldManDb->m_nNodeId < 0))
			return;

		QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

		QString strQuery;
		strQuery = "SELECT task_id,user_id,group_id,database_id,name,type,enabled,creation_date,expiration_date,last_update FROM ym_tasks";
		strQuery+= " WHERE node_id="+QString::number(pGexMainWindow->m_pYieldManDb->m_nNodeId);
		if(clQuery.exec(strQuery))
		{
			if(clQuery.first())
			{
				// Load Tasks from YieldManDb
				QString strString;
				int		iIndex;

				// Load task list
				do
				{
					iIndex = 0;

					CGexMoTaskItem *ptTask = new CGexMoTaskItem;

					ptTask->iTaskId = clQuery.value(iIndex++).toInt();
					ptTask->iUserId = clQuery.value(iIndex++).toInt();
					ptTask->iGroupId = clQuery.value(iIndex++).toInt();
					ptTask->iDatabaseId = clQuery.value(iIndex++).toInt();
					ptTask->strName = clQuery.value(iIndex++).toString();
					ptTask->iTaskType = clQuery.value(iIndex++).toInt();
					ptTask->bEnabled = (clQuery.value(iIndex++).toInt() == 1);
					ptTask->clCreationDate = clQuery.value(iIndex++).toDateTime();
					ptTask->clExpirationDate = clQuery.value(iIndex++).toDateTime();
					ptTask->clLastUpdate = clQuery.value(iIndex++).toDateTime();


					switch(ptTask->iTaskType)
					{
						case GEXMO_TASK_DATAPUMP:
							LoadDbTaskSectionDataPump(ptTask);
							break;
						case GEXMO_TASK_YIELDMONITOR:
							LoadDbTaskSectionYieldMonitoring(ptTask);
							break;
						case GEXMO_TASK_YIELDMONITOR_RDB:
							LoadDbTaskSectionYieldMonitoring_RDB(ptTask);
							break;
						case GEXMO_TASK_SPECMONITOR:
							LoadDbTaskSectionSpecMonitoring(ptTask);
							break;
						case GEXMO_TASK_REPORTING:
							LoadDbTaskSectionReporting(ptTask);
							break;
						case GEXMO_TASK_STATUS:
							LoadDbTaskSectionStatus(ptTask);
							break;
						case GEXMO_TASK_CONVERTER:
							LoadDbTaskSectionFileConverter(ptTask);
							break;
						case GEXMO_TASK_OUTLIER_REMOVAL:
							LoadDbTaskSectionOutlierRemoval(ptTask);
							break;
						case GEXMO_TASK_AUTOADMIN:
							LoadDbTaskSectionAutoAdmin(ptTask);
							break;
					}

					// Check if have this database referenced in the databse YieldManDb
					if(ptTask->iDatabaseId > 0)
					{
						if(pGexMainWindow->pDatabaseCenter->FindDatabaseEntry(ptTask->iDatabaseId) == NULL)
						{
							// Disable this task
							ptTask->bEnabled = false;
						}
					}

					// Add task to internal structure
					pGexMoTasksList.append(ptTask);

					// Add task to list
					UpdateListViewItem(ptTask,NULL,true);

				}
				while(clQuery.next());

				// Save timestamp of Task file loaded
				m_dtTaskLoadTimestamp = QDateTime::currentDateTime();
				return;
			}

			// No Task saved for this node
			// Check if have some tasks from the local file
		}
		else
		{
			QString strError = clQuery.lastError().text();
		}
	}

	// Else
	// Load Tasks from Local File
	LoadTasks(bForceReload);

	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel >= YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT))
		SaveDbTasks();
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: Data Pump
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionDataPump(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if( pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptDataPump)
		delete ptTask->ptDataPump;

	ptTask->ptDataPump = new GexMoDataPumpTaskData;

	while(clQuery.next())
	{
		// Read one line
		strString = clQuery.value(0).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptDataPump->strTitle = clQuery.value(1).toString();

		// Read data path
		if(strString == "DataPath")
			ptTask->ptDataPump->strDataPath = clQuery.value(1).toString();

		// Scan sub-folder flag
		if(strString == "ScanSubFolders")
		{
			if(clQuery.value(1).toString() == "YES")
				ptTask->ptDataPump->bScanSubFolders = true;
			else
				ptTask->ptDataPump->bScanSubFolders = false;
		}

		// List of files extensions to import
		if(strString == "ImportFileExtensions")
				ptTask->ptDataPump->strImportFileExtensions = clQuery.value(1).toString();

		// Read Database targetted
		if(strString == "Database")
			ptTask->ptDataPump->strDatabaseTarget = clQuery.value(1).toString();

		// Read Data Type targetted
		if(strString == "DataType")
			ptTask->ptDataPump->uiDataType = clQuery.value(1).toUInt();

		// Read Testing stage (if WYR data type)
		if(strString == "TestingStage")
			ptTask->ptDataPump->strTestingStage = clQuery.value(1).toString();

		// Read Task frequency
		if(strString == "Frequency")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->iFrequency = strString.toLong();
		}

		// Read Task Day of Week execution
		if(strString == "DayOfWeek")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->iDayOfWeek = strString.toLong();
		}

		// Execution window flag
		if(strString == "ExecWindow")
		{
			if(clQuery.value(1).toString() == "YES")
				ptTask->ptDataPump->bExecutionWindow = true;
			else
				ptTask->ptDataPump->bExecutionWindow = false;
		}

		// Read Start-time
		if(strString == "StartTime")
		{
			strString = clQuery.value(1).toString();
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTask->ptDataPump->cStartTime = tStartTime;
		}

		// Read Stop-time
		if(strString == "StopTime")
		{
			strString = clQuery.value(1).toString();
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStopTime(iHour,iMinute);
			ptTask->ptDataPump->cStopTime = tStopTime;
		}

		// Read PostImport task: Rename, Move or Delete files.
		if(strString == "PostImport")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->iPostImport= strString.toLong();
		}

		// Read Move/FTP folder
		if(strString == "PostImportFolder")
			ptTask->ptDataPump->strPostImportMoveFolder = clQuery.value(1).toString();

		// Read PostImport task: Rename, Move or Delete files.
		if(strString == "PostImportFailure")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->iPostImportFailure= strString.toLong();
		}

		// Read Move/FTP folder
		if(strString == "PostImportFailureFolder")
			ptTask->ptDataPump->strPostImportFailureMoveFolder = clQuery.value(1).toString();

		// check Yield window flag
		if(strString == "CheckYield")
		{
			if(clQuery.value(1).toString() == "YES")
				ptTask->ptDataPump->bCheckYield = true;
			else
				ptTask->ptDataPump->bCheckYield = false;
		}

		// Read Good bin list
		if(strString == "YieldBins")
			ptTask->ptDataPump->strYieldBinsList = clQuery.value(1).toString();

		// Read Alarm level (0-100%)
		if(strString == "AlarmLevel")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->iAlarmLevel = strString.toInt();
		}

		// Read Minimum parts to have a valid file
		if(strString == "MinimumParts")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->lMinimumyieldParts = strString.toLong();
		}

		// Read Email notification list
		if(strString == "Emails")
			ptTask->ptDataPump->strEmailNotify = clQuery.value(1).toString();

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(clQuery.value(1).toString() == "HTML")
				ptTask->ptDataPump->bHtmlEmail = true;
			else
				ptTask->ptDataPump->bHtmlEmail = false;
		}

		// Read Last time task was executed...
		if(strString == "LastExecuted")
		{
			strString = clQuery.value(1).toString();
			ptTask->ptDataPump->tLastExecuted = strString.toLong();
		}
	}
}


///////////////////////////////////////////////////////////
// Load from YieldManDb...section: YIELD MONITORING
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionYieldMonitoring(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;

	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptYield)
		delete ptTask->ptYield;

	GexMoYieldMonitoringTaskData *ptTaskYield = NULL;
	ptTaskYield = new GexMoYieldMonitoringTaskData;
	ptTaskYield->clear();
	ptTask->ptYield = ptTaskYield;


	while(clQuery.next())
	{

		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();


		// Read Title
		if(strString == "Title")
			ptTaskYield->strTitle = strSection;

		if(strString == "State")
			ptTaskYield->bEnabled = strSection == "1";

		// Read ProductID
		if(strString == "ProductID")
			ptTaskYield->strProductID = strSection;

		// Read Yield Bin list
		if(strString == "YieldBins")
			ptTaskYield->strYieldBinsList = strSection;

		// Bin list type: 0=Good bins, 1=Failing bins.
		if(strString == "BiningType")
			ptTaskYield->iBiningType = strSection.toInt();

		// Read Alarm level (0-100%)
		if(strString == "AlarmLevel")
			ptTaskYield->iAlarmLevel = strSection.toInt();

		// Read Flag: Check if Yield OVER or UNDER the limit.
		if(strString == "AlarmDirection")
			ptTaskYield->iAlarmIfOverLimit = strSection.toInt();


		// Read Minimum parts to have a valid file
		if(strString == "MinimumParts")
			ptTaskYield->lMinimumyieldParts = strSection.toLong();

		// Read SBL/YBL data file (if exists)
		if((strString == "SblFile=") && QFile::exists(strSection))
			ptTaskYield->strSblFile = strSection;

		// Read Email 'From'
		if(strString == "EmailFrom")
			ptTaskYield->strEmailFrom = strSection;

		// Read Email notification list
		if(strString == "Emails")
			ptTaskYield->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(strSection == "HTML")
				ptTaskYield->bHtmlEmail = true;
			else
				ptTaskYield->bHtmlEmail = false;
		}

		// Read Email message contents type to send
		if(strString == "EmailReportType")
			ptTaskYield->iEmailReportType = strSection.toInt();

		// Read Email report notification type: send as attachment or leave on server
		if(strString == "NotificationType")
			ptTaskYield->iNotificationType = strSection.toInt();

		// Read Alarm type: Standard, Critical...
		if(strString == "ExceptionLevel")
			ptTaskYield->iExceptionLevel = strSection.toInt();
	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: YIELD MONITORING RDB  (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionYieldMonitoring_RDB(CGexMoTaskItem *	ptTask)
{

	// TO DO

	return;

	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;

	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptYield)
		delete ptTask->ptYield;
	if(ptTask->ptYield_RDB)
		delete ptTask->ptYield_RDB;

	GexMoYieldMonitoringTaskData *ptTaskYield = NULL;
	ptTaskYield = new GexMoYieldMonitoringTaskData;
	ptTaskYield->clear();

	if(ptTask->iTaskType == GEXMO_TASK_YIELDMONITOR)
		ptTask->ptYield = ptTaskYield;
	else
	{
		ptTask->ptYield_RDB = new GexMoYieldMonitoringTaskData_RDB;
		ptTask->ptYield_RDB->cYield_SYA_Rules.append(ptTaskYield);
	}

	while(clQuery.next())
	{

		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();


		// Read Title
		if(strString == "Title")
			ptTaskYield->strTitle = strSection;

		if(strString == "State")
			ptTaskYield->bEnabled = strSection == "1";

		// Read ProductID
		if(strString == "ProductID")
			ptTaskYield->strProductID = strSection;

		// Read Yield Bin list
		if(strString == "YieldBins")
			ptTaskYield->strYieldBinsList = strSection;

		// Bin list type: 0=Good bins, 1=Failing bins.
		if(strString == "BiningType")
			ptTaskYield->iBiningType = strSection.toInt();

		// Read Alarm level (0-100%)
		if(strString == "AlarmLevel")
			ptTaskYield->iAlarmLevel = strSection.toInt();

		// Read Flag: Check if Yield OVER or UNDER the limit.
		if(strString == "AlarmDirection")
			ptTaskYield->iAlarmIfOverLimit = strSection.toInt();


		// Read Minimum parts to have a valid file
		if(strString == "MinimumParts")
			ptTaskYield->lMinimumyieldParts = strSection.toLong();

		// Read SBL/YBL data file (if exists)
		if((strString == "SblFile=") && QFile::exists(strSection))
			ptTaskYield->strSblFile = strSection;

		// Read Email 'From'
		if(strString == "EmailFrom")
			ptTaskYield->strEmailFrom = strSection;

		// Read Email notification list
		if(strString == "Emails")
			ptTaskYield->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(strSection == "HTML")
				ptTaskYield->bHtmlEmail = true;
			else
				ptTaskYield->bHtmlEmail = false;
		}

		// Read Email message contents type to send
		if(strString == "EmailReportType")
			ptTaskYield->iEmailReportType = strSection.toInt();

		// Read Email report notification type: send as attachment or leave on server
		if(strString == "NotificationType")
			ptTaskYield->iNotificationType = strSection.toInt();

		// Read Alarm type: Standard, Critical...
		if(strString == "ExceptionLevel")
			ptTaskYield->iExceptionLevel = strSection.toInt();

		/////////////////////////////////////////////////////////////////////////////
		// SYL-SBL specifics
		/////////////////////////////////////////////////////////////////////////////
		if(strString == "Database")
			ptTaskYield->strDatabase = strSection;

		if(strString == "TestingStage")
			ptTaskYield->strTestingStage = strSection;

		if(strString == "RuleType")
			ptTaskYield->iSYA_Rule = strSection.toInt();		// Rule: 0=N*Sigma, 1=N*IQR

		if(strString == "RuleTypeString")		// Rule string: N*Sigma, N*IQR
			ptTaskYield->strSYA_Rule = strSection;

		if(strString == "N_Parameter")
			ptTaskYield->fSYA_N1_value = strSection.toFloat();	// N parameter (compatibility, new fields are N1, N2)

		if(strString == "N1_Parameter")
			ptTaskYield->fSYA_N1_value = strSection.toFloat();	// N1 parameter

		if(strString == "N2_Parameter")
			ptTaskYield->fSYA_N2_value = strSection.toFloat();	// N2 parameter

		if(strString == "MinimumLotsRequired")
			ptTaskYield->iSYA_LotsRequired = strSection.toInt();// Minimum Total lots required for computing new SYL-SBL

		if(strString == "ValidityPeriod")
			ptTaskYield->iSYA_Period = strSection.toInt();// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...

		if(strString == "ExpirationDate")
		{
			int iDay,iMonth,iYear;
			QString strString;
			iDay = strSection.section(' ',0,0).stripWhiteSpace().toInt();
			iMonth = strSection.section(' ',1,1).stripWhiteSpace().toInt();
			iYear = strSection.section(' ',2,2).stripWhiteSpace().toInt();
			ptTaskYield->cExpiration.setDate(iYear,iMonth,iDay);
		}

		if(strString == "SBL_LL_Disabled")
		{
			ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;
			ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;
		}

		if(strString == "SBL1_LL_Disabled")
			ptTaskYield->strSYA_SBL1_LL_Disabled = strSection;

		if(strString == "SBL2_LL_Disabled")
			ptTaskYield->strSYA_SBL2_LL_Disabled = strSection;

		if(strString == "SBL_HL_Disabled")
		{
			ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;
			ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;
		}

		if(strString == "SBL1_HL_Disabled")
			ptTaskYield->strSYA_SBL1_HL_Disabled = strSection;

		if(strString == "SBL2_HL_Disabled")
			ptTaskYield->strSYA_SBL2_HL_Disabled = strSection;

		if(strString == "SYL_LL_Disabled")
		{
			ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";
			ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";
		}

		if(strString == "SYL1_LL_Disabled")
			ptTaskYield->bSYA_SYL1_LL_Disabled = strSection == "1";

		if(strString == "SYL2_LL_Disabled")
			ptTaskYield->bSYA_SYL2_LL_Disabled = strSection == "1";

		if(strString == "SYL_HL_Disabled")
		{
			ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";
			ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";
		}

		if(strString == "SYL1_HL_Disabled")
			ptTaskYield->bSYA_SYL1_HL_Disabled = strSection == "1";

		if(strString == "SYL2_HL_Disabled")
			ptTaskYield->bSYA_SYL2_HL_Disabled = strSection == "1";

		if(strString == "IgnoreDataPointsWithNullSigma")
			ptTaskYield->bSYA_IgnoreDataPointsWithNullSigma = strSection == "1";

		if(strString == "IgnoreOutliers")
			ptTaskYield->bSYA_IgnoreOutliers = (strSection == "1");

		if(strString == "MinDataPoints")
			ptTaskYield->iSYA_MinDataPoints = strSection.toInt();
	}

}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: SPEC MONITORING
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionSpecMonitoring(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptSpec)
		delete ptTask->ptSpec;

	ptTask->ptSpec = new GexMoSpecMonitoringTaskData;

	// Allocate buffer to store information read from disk.
	GexMoSpecMonitoringTaskData *ptTaskSpec= new GexMoSpecMonitoringTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptSpec->strTitle = strSection;

		// Read ProductID
		if(strString == "ProductID")
			ptTask->ptSpec->strProductID = strSection;

		// Read Parameter/Test#
		if(strString == "Parameter")
			ptTask->ptSpec->strParameter = strSection;

		// Read Parameter inof to monitor
		if(strString == "MonitorInfo")
			ptTask->ptSpec->iMonitorInfo = strSection.toInt();

		// Read Low spec limit (if any)
		if(strString == "LowSpec")
			ptTask->ptSpec->strLowSpec = strSection;

		// Read High spec limit (if any)
		if(strString == "HighSpec")
			ptTask->ptSpec->strHighSpec = strSection;

		// Emails list...
		if(strString == "Emails")
			ptTask->ptSpec->strEmailNotify = strSection;

		// Emails from...
		if(strString == "EmailFrom")
			ptTask->ptSpec->strEmailFrom = strSection;

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(strSection == "HTML")
				ptTask->ptSpec->bHtmlEmail = true;
			else
				ptTask->ptSpec->bHtmlEmail = false;
		}

		// Read Alarm type: Standard, Critical...
		if(strString == "ExceptionLevel")
			ptTask->ptSpec->iExceptionLevel = strSection.toInt();

	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: REPORTING
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionReporting(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptReporting)
		delete ptTask->ptReporting;

	ptTask->ptReporting = new GexMoReportingTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptReporting->strTitle = strSection;

		// Read Script path
		if(strString == "ScriptPath")
			ptTask->ptReporting->strScriptPath = strSection;

		// Read Task frequency
		if(strString == "Frequency")
		{
			strString = strSection;
			ptTask->ptReporting->iFrequency = strString.toLong();
		}

		// Read Task Day of Week execution
		if(strString == "DayOfWeek")
		{
			strString = strSection;
			ptTask->ptReporting->iDayOfWeek = strString.toLong();
		}

		// Execution window flag
		if(strString == "ExecWindow")
		{
			if(strSection == "YES")
				ptTask->ptReporting->bExecutionWindow = true;
			else
				ptTask->ptReporting->bExecutionWindow = false;
		}

		// Read Start-time
		if(strString == "StartTime")
		{
			strString = strSection;
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTask->ptReporting->cStartTime = tStartTime;
		}

		// Read Stop-time
		if(strString == "StopTime")
		{
			strString = strSection;
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStopTime(iHour,iMinute);
			ptTask->ptReporting->cStopTime = tStopTime;
		}

		// Read Email Notification type
		if(strString == "NotificationType")
		{
			strString = strSection;
			ptTask->ptReporting->iNotificationType = strString.toInt();
		}

		// Read Email notification list
		if(strString == "Emails")
			ptTask->ptReporting->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(strSection == "HTML")
				ptTask->ptReporting->bHtmlEmail = true;
			else
				ptTask->ptReporting->bHtmlEmail = false;
		}

		// Read Last time task was executed...
		if(strString == "LastExecuted")
		{
			strString = strSection;
			ptTask->ptReporting->tLastExecuted = strString.toLong();
		}

	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: STATUS
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionStatus(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptStatus)
		delete ptTask->ptStatus;

	ptTask->ptStatus = new GexMoStatusTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptStatus->strTitle = strSection;

		// Read Web organization type
		if(strString == "WebOrganization")
		{
			int iOneWebPerDatabase;
			iOneWebPerDatabase = strSection.toLong();
			if(iOneWebPerDatabase)
				ptTask->ptStatus->bOneWebPerDatabase = true;
			else
				ptTask->ptStatus->bOneWebPerDatabase = false;
		}

		// Read Intranet path
		if(strString == "IntranetPath")
			ptTask->ptStatus->strIntranetPath = strSection;

		// Read Home page name
		if(strString == "HomePage")
			ptTask->ptStatus->strHomePage = strSection;

		// Report's URL name to display in Emails (hyperlink)
		if(strString == "ReportURL")
			ptTask->ptStatus->strReportURL = strSection;

	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: FileConverter
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionFileConverter(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptConverter)
		delete ptTask->ptConverter;

	ptTask->ptConverter = new GexMoFileConverterTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptConverter->strTitle = strSection;

		// Input Folder
		if(strString == "InputFolder")
			ptTask->ptConverter->strInputFolder = strSection;

		// Input file extensions
		if(strString == "ImportFileExtensions")
			ptTask->ptConverter->strFileExtensions = strSection;

		// Execution frequency
		if(strString == "Frequency")
			ptTask->ptConverter->iFrequency = strSection.toInt();

		// Execution Day of week (if frequency is week, or month,...)
		if(strString == "DayOfWeek")
			ptTask->ptConverter->iDayOfWeek = strSection.toInt();

		// Output Folder
		if(strString == "OutputFolder")
			ptTask->ptConverter->strOutputFolder = strSection;

		// Output Format: STDF (0) or CSV (1)
		if(strString == "OutputFormat")
			ptTask->ptConverter->iFormat = strSection.toInt();

		// Include Timestamp info in file name to create?
		if(strString == "TimeStampFile")
			ptTask->ptConverter->bTimeStampName = (strSection.toInt() != 0) ? true : false;

		// What to to file file successfuly converted
		if(strString == "SuccessMode")
			ptTask->ptConverter->iOnSuccess = strSection.toInt();

		// Folder where to move source files (if successfuly converted)
		if(strString == "SuccessFolder")
			ptTask->ptConverter->strOutputSuccess = strSection;

		// What to to file file that failed conversion
		if(strString == "FailMode")
			ptTask->ptConverter->iOnError = strSection.toInt();

		// Folder where to move source files (if failed conversion)
		if(strString == "FailFolder")
			ptTask->ptConverter->strOutputError = strSection;

	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: OUTLIER REMOVAL
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionOutlierRemoval(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptOutlier)
		delete ptTask->ptOutlier;

	ptTask->ptOutlier = new GexMoOutlierRemovalTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptOutlier->strTitle = strSection;

		// Read ProductID
		if(strString == "ProductID")
			ptTask->ptOutlier->strProductID = strSection;

		// Read Alarm level
		if(strString == "AlarmLevel")
			ptTask->ptOutlier->lfAlarmLevel = strSection.toDouble();

		// Read Alarm Type: % (0) or #parts (1)
		if(strString == "AlarmType")
			ptTask->ptOutlier->iAlarmType = strSection.toInt();

		// Read Minimum parts to have a valid file
		if(strString == "MinimumParts")
			ptTask->ptOutlier->lMinimumyieldParts = strSection.toLong();

		// Notify if distribution shape changes compared to historical data
		if(strString == "NotifyShapeChange")
			ptTask->ptOutlier->bNotifyShapeChange = (bool) strSection.toLong();

		// Read Maximum number of Die mismatch between E-Test & STDF wafermaps
		if(strString == "CompositeEtestAlarm")
			ptTask->ptOutlier->lCompositeEtestAlarm = strSection.toLong();

		// Read Maximum number of Die to reject on the exclusion zone stacked wafer.
		if(strString == "CompositeExclusionZoneAlarm")
			ptTask->ptOutlier->lCompositeExclusionZoneAlarm = strSection.toLong();

		// Read Email notification list
		if(strString == "Emails")
			ptTask->ptOutlier->strEmailNotify = strSection;

		// Read Email format: HTML or TXT
		if(strString == "EmailFormat")
		{
			if(strSection == "HTML")
				ptTask->ptOutlier->bHtmlEmail = true;
			else
				ptTask->ptOutlier->bHtmlEmail = false;
		}

		// Read Email message contents type to send
		if(strString == "EmailReportType")
			ptTask->ptOutlier->iEmailReportType = strSection.toInt();

		// Read Email report notification type: send as attachment or leave on server
		if(strString == "NotificationType")
			ptTask->ptOutlier->iNotificationType = strSection.toInt();

		// Read Alarm type: Standard, Critical...
		if(strString == "ExceptionLevel")
			ptTask->ptOutlier->iExceptionLevel = strSection.toInt();

	}
}

///////////////////////////////////////////////////////////
// Load from YieldManDb...section: Auto Admin
///////////////////////////////////////////////////////////
void GexMoScheduler::LoadDbTaskSectionAutoAdmin(CGexMoTaskItem *	ptTask)
{
	if(ptTask == NULL)
		return;

	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString strString;
	QString strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);

	QString strQuery;
	strQuery = "SELECT field,value FROM ym_tasks_options WHERE task_id=" + QString::number(ptTask->iTaskId);

	if(!clQuery.exec(strQuery))
		return;

	// Allocate buffer to store information read from disk.
	if(ptTask->ptAutoAdmin)
		delete ptTask->ptAutoAdmin;

	ptTask->ptAutoAdmin = new GexMoAutoAdminTaskData;

	while(clQuery.next())
	{
		// Read one line from file
		strString = clQuery.value(0).toString();
		strSection = clQuery.value(1).toString();

		// Read Title
		if(strString == "Title")
			ptTask->ptAutoAdmin->strTitle = strSection;

		// Time of day to start auto-admin
		if(strString == "StartTime")
		{
			strString = strSection;
			strSection = strString.section(',',0,0);
			int iHour = strSection.toInt();
			strSection = strString.section(',',1,1);
			int iMinute = strSection.toInt();
			QTime tStartTime(iHour,iMinute);
			ptTask->ptAutoAdmin->cStartTime = tStartTime;
		}

		// Read Web organization type
		if(strString == "KeepReportDuration")
			ptTask->ptAutoAdmin->iKeepReportDuration = strSection.toLong();

		// Read log file contents type
		if(strString == "LogContents")
			ptTask->ptAutoAdmin->iLogContents = strSection.toLong();

		// Read Email notification list
		if(strString == "Emails")
			ptTask->ptAutoAdmin->strEmailNotify = strSection;

		// Read Last time task was executed...
		if(strString == "LastExecuted")
		{
			strString = strSection;
			ptTask->ptAutoAdmin->tLastExecuted = strString.toLong();
		}

		// Read Shell to launch if: Yield Alarm
		if(strString == "ShellYieldAlarm")
			ptTask->ptAutoAdmin->strShellYieldAlarm_Std = strSection;
		if(strString == "ShellYieldAlarmCritical")
			ptTask->ptAutoAdmin->strShellYieldAlarm_Critical = strSection;

		// Read Shell to launch if: Parameter Alarm
		if(strString == "ShellParameterAlarm")
			ptTask->ptAutoAdmin->strShellParameterAlarm_Std = strSection;
		if(strString == "ShellParameterAlarmCritical")
			ptTask->ptAutoAdmin->strShellParameterAlarm_Critical = strSection;

		// Read Shell to launch if: Pat Alarm
		if(strString == "ShellPatAlarm")
			ptTask->ptAutoAdmin->strShellPatAlarm_Std = strSection;
		if(strString == "ShellPatAlarmCritical")
			ptTask->ptAutoAdmin->strShellPatAlarm_Critical = strSection;

	}
}

///////////////////////////////////////////////////////////
// Save Tasks to disk
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveTasks(void)
{
	// Load Tasks from Local File

	QListIterator<CGexMoTaskItem*> lstIteratorTask(pGexMoTasksList);

	QString				strString,strSection;
	CGexMoTaskItem *	ptTask = NULL;

	// Build path to the 'Tasks' list.
	QFile file( pGexMainWindow->strMoTasksFile ); // Write the text to the file
	if (file.open(IO_WriteOnly) == false)
		return;	// Failed writing to tasks file.

	// Write Tasks definition File
	hTasks.setDevice(&file);	// Assign file handle to data stream

	// Check if valid header...or empty!
	hTasks << "<tasks>" << endl;

	lstIteratorTask.toFront();

	while(lstIteratorTask.hasNext())
	{
		ptTask = lstIteratorTask.next();

		switch(ptTask->iTaskType)
		{
			case GEXMO_TASK_DATAPUMP:
				SaveTaskSectionDataPump(ptTask->ptDataPump);
				break;
			case GEXMO_TASK_YIELDMONITOR:
				SaveTaskSectionYieldMonitoring(ptTask->ptYield);
				break;
			case GEXMO_TASK_YIELDMONITOR_RDB:
				SaveTaskSectionYieldMonitoring_RDB(ptTask->ptYield_RDB);
				break;
			case GEXMO_TASK_SPECMONITOR:
				SaveTaskSectionSpecMonitoring(ptTask->ptSpec);
				break;
			case GEXMO_TASK_REPORTING:
				SaveTaskSectionReporting(ptTask->ptReporting);
				break;
			case GEXMO_TASK_STATUS:
				SaveTaskSectionStatus(ptTask->ptStatus);
				break;
			case GEXMO_TASK_CONVERTER:
				SaveTaskSectionFileConverter(ptTask->ptConverter);
				break;
			case GEXMO_TASK_OUTLIER_REMOVAL:
				SaveTaskSectionOutlierRemoval(ptTask->ptOutlier);
				break;
			case GEXMO_TASK_AUTOADMIN:
				SaveTaskSectionAutoAdmin(ptTask->ptAutoAdmin);
				break;
		}
	};

	hTasks << "</tasks>" << endl;

	file.close();
}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTasks(CGexMoTaskItem *	ptTask)
{
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb
	&&  pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector
	&& !pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->Connect())
	{
		SaveTasks();
		return;
	}

	// Check if load tasks from YieldManDb
	if( pGexMainWindow->m_pYieldManDb
	&& (pGexMainWindow->m_pYieldManDb->m_nOptionLevel < YIELDMANDB_OPTIONLEVEL_TASK_MANAGMENT))
	{
		SaveTasks();
		return;
	}

	QList<CGexMoTaskItem*> lstTasks;
	if(ptTask)
		lstTasks.append(ptTask);
	else
		lstTasks = pGexMoTasksList;

	QListIterator<CGexMoTaskItem*> lstIteratorTask(lstTasks);

	QString				strString,strSection;
	CGexMoTaskItem *	ptTaskItem = NULL;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;

	lstIteratorTask.toFront();

	while(lstIteratorTask.hasNext())
	{
		ptTaskItem = lstIteratorTask.next();

		strQuery = "SELECT * FROM ym_tasks WHERE task_id="+QString::number(ptTaskItem->iTaskId);
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
		if(!clQuery.first())
		{
			// Insert
			QTime clTime = QTime::currentTime();
			strQuery = "INSERT INTO ym_tasks(name, creation_date)";
			strQuery+= "VALUES('" + clTime.toString("hhmmsszz") + "', NOW())";
			if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

			// Retrieve the task_id
			strQuery = "SELECT task_id FROM ym_tasks WHERE name='"+clTime.toString("hhmmsszz")+"'";
			if(!clQuery.exec(strQuery))
				return;

			clQuery.first();
			ptTaskItem->iTaskId = clQuery.value(0).toInt();
		}

		// Update
		strQuery = "UPDATE ym_tasks SET ";
		strQuery+= "  user_id=" + QString::number(ptTaskItem->iUserId);
		if(pGexMainWindow->m_pYieldManDb->m_nNodeId > 0)
			strQuery+= "  ,node_id=" + QString::number(pGexMainWindow->m_pYieldManDb->m_nNodeId);
		strQuery+= ", group_id=" + QString::number(ptTaskItem->iGroupId);
		strQuery+= ", database_id=" + QString::number(ptTaskItem->iDatabaseId);
		strQuery+= ", name='" + ptTaskItem->strName +"'";
		strQuery+= ", type=" + QString::number(ptTaskItem->iTaskType);
		strQuery+= ", enabled=" + QString(ptTaskItem->bEnabled ? "1" : "0");
		strQuery+= ", permisions=" + QString::number(ptTaskItem->iPermissions);
		strQuery+= ", last_update='" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"'";
		strQuery+= " WHERE task_id="+QString::number(ptTaskItem->iTaskId);
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

		switch(ptTaskItem->iTaskType)
		{
			case GEXMO_TASK_DATAPUMP:
				SaveDbTaskSectionDataPump(ptTaskItem);
				break;
			case GEXMO_TASK_YIELDMONITOR:
				SaveDbTaskSectionYieldMonitoring_RDB_OneRule(ptTaskItem);
				break;
			case GEXMO_TASK_YIELDMONITOR_RDB:
				SaveDbTaskSectionYieldMonitoring_RDB(ptTaskItem);
				break;
			case GEXMO_TASK_SPECMONITOR:
				SaveDbTaskSectionSpecMonitoring(ptTaskItem);
				break;
			case GEXMO_TASK_REPORTING:
				SaveDbTaskSectionReporting(ptTaskItem);
				break;
			case GEXMO_TASK_STATUS:
				SaveDbTaskSectionStatus(ptTaskItem);
				break;
			case GEXMO_TASK_CONVERTER:
				SaveDbTaskSectionFileConverter(ptTaskItem);
				break;
			case GEXMO_TASK_OUTLIER_REMOVAL:
				SaveDbTaskSectionOutlierRemoval(ptTaskItem);
				break;
			case GEXMO_TASK_AUTOADMIN:
				SaveDbTaskSectionAutoAdmin(ptTaskItem);
				break;
		}
	}
}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section DATA PUMP
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionDataPump(CGexMoTaskItem *	ptTask)
{
	// Check if yieldman is connected
	if( pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptDataPump->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'DataPath','" + ptTask->ptDataPump->strDataPath + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ScanSubFolders','" + (ptTask->ptDataPump->bScanSubFolders ? "YES" : "NO") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ImportFileExtensions','" + ptTask->ptDataPump->strImportFileExtensions + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Database','" + ptTask->ptDataPump->strDatabaseTarget + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'DataType','" + QString::number(ptTask->ptDataPump->uiDataType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'TestingStage','" + ptTask->ptDataPump->strTestingStage + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Frequency','" + QString::number(ptTask->ptDataPump->iFrequency) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'DayOfWeek','" + QString::number(ptTask->ptDataPump->iDayOfWeek) + "')";


	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExecWindow','" + (ptTask->ptDataPump->bExecutionWindow ? "YES" : "NO") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'StartTime','" + QString::number(ptTask->ptDataPump->cStartTime.hour()) + "," + QString::number(ptTask->ptDataPump->cStartTime.minute()) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'StopTime','" + QString::number(ptTask->ptDataPump->cStopTime.hour()) + "," + QString::number(ptTask->ptDataPump->cStopTime.minute()) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'PostImport','" + QString::number(ptTask->ptDataPump->iPostImport) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'PostImportFolder','" + ptTask->ptDataPump->strPostImportMoveFolder + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'PostImportFailure','" + QString::number(ptTask->ptDataPump->iPostImportFailure) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'PostImportFailureFolder','" + ptTask->ptDataPump->strPostImportFailureMoveFolder + "')";


	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'CheckYield','" + (ptTask->ptDataPump->bCheckYield ? "YES" : "NO") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'YieldBins','" + ptTask->ptDataPump->strYieldBinsList + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmLevel','" + QString::number(ptTask->ptDataPump->iAlarmLevel) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinimumParts','" + QString::number(ptTask->ptDataPump->lMinimumyieldParts) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTask->ptDataPump->strEmailNotify + "')";


	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTask->ptDataPump->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LastExecuted','" + QDateTime::fromTime_t(ptTask->ptDataPump->tLastExecuted).toString("yyyy-MM-dd hh:mm:ss") + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section YIELD MONITORING
///////////////////////////////////////////////////////////
/*
void GexMoScheduler::SaveDbTaskSectionYieldMonitoring(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	GexMoYieldMonitoringTaskData *ptTaskYield = NULL;
	ptTaskYield = ptTask->ptYield;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTaskYield->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ProductID','" + ptTaskYield->strProductID + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'YieldBins','" + ptTaskYield->strYieldBinsList + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'BiningType','" + QString::number(ptTaskYield->iBiningType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmLevel','" + QString::number(ptTaskYield->iAlarmLevel) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmDirection','" + QString::number(ptTaskYield->iAlarmIfOverLimit) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinimumParts','" + QString::number(ptTaskYield->lMinimumyieldParts) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SblFile','" + ptTaskYield->strSblFile + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFrom','" + ptTaskYield->strEmailFrom + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTaskYield->strEmailNotify + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTaskYield->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailReportType','" + QString::number(ptTaskYield->iEmailReportType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'NotificationType','" + QString::number(ptTaskYield->iNotificationType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExceptionLevel','" + QString::number(ptTaskYield->iExceptionLevel) + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}
*/

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: One rule from Section YIELD MONITORING RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionYieldMonitoring_RDB_OneRule(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString		strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	GexMoYieldMonitoringTaskData *ptTaskYield = NULL;
	if(ptTask->iTaskType == GEXMO_TASK_YIELDMONITOR)
		ptTaskYield = ptTask->ptYield;
	else
		ptTaskYield = ptTask->ptYield_RDB->cYield_SYA_Rules.first();

	// Insert new values
	strQuery += "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTaskYield->strTitle + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'State','" + (ptTaskYield->bEnabled ? "1" : "0") + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ProductID','" + ptTaskYield->strProductID + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'YieldBins','" + ptTaskYield->strYieldBinsList + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'BiningType','" + QString::number(ptTaskYield->iBiningType) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmLevel','" + QString::number(ptTaskYield->iAlarmLevel) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmDirection','" + QString::number(ptTaskYield->iAlarmIfOverLimit) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinimumParts','" + QString::number(ptTaskYield->lMinimumyieldParts) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFrom','" + ptTaskYield->strEmailFrom + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTaskYield->strEmailNotify + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTaskYield->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailReportType','" + QString::number(ptTaskYield->iEmailReportType) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'NotificationType','" + QString::number(ptTaskYield->iNotificationType) + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExceptionLevel','" + QString::number(ptTaskYield->iExceptionLevel) + "')";


	/////////////////////////////////////////////////////////////////////////////
	// SYL-SBL specifics
	/////////////////////////////////////////////////////////////////////////////
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Database','" + ptTaskYield->strDatabase + "')";

	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'TestingStage','" + ptTaskYield->strTestingStage + "')";
	// Rule: 0=N*Sigma, 1=N*IQR
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'RuleType','" + QString::number(ptTaskYield->iSYA_Rule) + "')";
	// Rule string: N*Sigma, N*IQR
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'RuleTypeString','" + ptTaskYield->strSYA_Rule + "')";
	// N1 parameter
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'N1_Parameter','" + QString::number(ptTaskYield->fSYA_N1_value) + "')";
	// N1 parameter
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'N2_Parameter','" + QString::number(ptTaskYield->fSYA_N2_value) + "')";
	// Minimum Total lots required for computing new SYL-SBL
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinimumLotsRequired','" + QString::number(ptTaskYield->iSYA_LotsRequired) + "')";
	// Period for reprocessing SYL-SBL: 0=1week,1=1Month,2...
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ValidityPeriod','" + QString::number(ptTaskYield->iSYA_Period) + "')";
	// Period for reprocessing SYL/SBL: 0=1week,1=1Month,2...
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExpirationDate','" + QString::number(ptTaskYield->cExpiration.day()) + " " + QString::number(ptTaskYield->cExpiration.month()) + " " + QString::number(ptTaskYield->cExpiration.year()) + "')";
	// List of Binnings for which the SBL1 LL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SBL1_LL_Disabled','" + ptTaskYield->strSYA_SBL1_LL_Disabled + "')";
	// List of Binnings for which the SBL1 HL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SBL1_HL_Disabled','" + ptTaskYield->strSYA_SBL1_HL_Disabled + "')";
	// List of Binnings for which the SBL2 LL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SBL2_LL_Disabled','" + ptTaskYield->strSYA_SBL2_LL_Disabled + "')";
	// List of Binnings for which the SBL2 HL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SBL2_HL_Disabled','" + ptTaskYield->strSYA_SBL2_HL_Disabled + "')";
	// True if SYL1 LL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SYL1_LL_Disabled','" + (ptTaskYield->bSYA_SYL1_LL_Disabled ? "1" : "0") + "')";
	// True if SYL1 HL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SYL1_HL_Disabled','" + (ptTaskYield->bSYA_SYL1_HL_Disabled ? "1" : "0") + "')";
	// True if SYL2 LL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SYL2_LL_Disabled','" + (ptTaskYield->bSYA_SYL2_LL_Disabled ? "1" : "0") + "')";
	// True if SYL2 HL should be disabled
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SYL2_HL_Disabled','" + (ptTaskYield->bSYA_SYL2_HL_Disabled ? "1" : "0") + "')";
	// Set to true if datapoints with null sigma should be ignored
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'IgnoreDataPointsWithNullSigma','" + (ptTaskYield->bSYA_IgnoreDataPointsWithNullSigma ? "1" : "0") + "')";
	// Set to true if outliers should be ignored
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'IgnoreOutliers','" + (ptTaskYield->bSYA_IgnoreOutliers ? "1" : "0") + "')";
	// Minimum datapoints (wafers, lots if FT) to compute SYL/SBL
	strQuery += strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinDataPoints','" + QString::number(ptTaskYield->iSYA_MinDataPoints) + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section YIELD MONITORING RDB (SYL/SBL)
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionYieldMonitoring_RDB(CGexMoTaskItem *	ptTask)
{
	/*
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QString				strString,strSection;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;

	int iIndex = 100;

	// Write all Standard Yield Rules
	QList <GexMoYieldMonitoringTaskData *>::iterator itRules;
	GexMoYieldMonitoringTaskData *ptYield;
	if(ptTask->ptYield_RDB->cYieldRules.count() > 0)
	{

		for (itRules = ptTask->ptYield_RDB->cYieldRules.begin(); itRules != ptTask->ptYield_RDB->cYieldRules.end(); ++itRules)
		{

			// Handle to Rule
			ptYield = *itRules;

			strQuery = "SELECT * FROM ym_tasks WHERE task_id="+QString::number(iIndex);
			if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
			if(!clQuery.first())
			{
				// Insert
				strQuery = "INSERT INTO ym_tasks(task_id, user_id, group_id, database_id, name, type, status, permisions, creation_date)";
				strQuery+= "VALUES(" + QString::number(iIndex) + ",0,0,null    ,''   ,0    ,0      ,0          ,'" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "')";
				if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
			}

			// Update
			strQuery = "UPDATE ym_tasks SET ";
			strQuery+= "user_id=" + QString::number(ptTask->iUserId);
			strQuery+= ", group_id=" + QString::number(ptTask->iGroupId);
			strQuery+= ", database_id=" + QString::number(ptTask->iDatabaseId);
			strQuery+= ", name='" + ptYield->strTitle +"'";
			strQuery+= ", type=11";
			strQuery+= ", enabled=" + (ptTask->bEnabled ? "1" : "0");
			strQuery+= ", permisions=" + QString::number(ptTask->iPermissions);
			strQuery+= ", last_execution='" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"'";
			strQuery+= " WHERE task_id="+QString::number(iIndex);
			if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();


			// Write rule to disk
			SaveDbTaskSectionYieldMonitoring_RDB_OneRule(iIndex,ptYield);

			iIndex++;
		}
	}

	// Write all SYA rule
	if(ptTask->ptYield_RDB->cYield_SYA_Rules.count() > 0)
	{
		for (itRules = ptTask->ptYield_RDB->cYield_SYA_Rules.begin(); itRules != ptTask->ptYield_RDB->cYield_SYA_Rules.end(); ++itRules)
		{
			// Handle to Rule
			ptYield = *itRules;

			strQuery = "SELECT * FROM ym_tasks WHERE task_id="+QString::number(iIndex);
			if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
			if(!clQuery.first())
			{
				// Insert
				strQuery = "INSERT INTO ym_tasks(task_id, user_id, group_id, database_id, name, type, status, permisions, creation_date)";
				strQuery+= "VALUES(" + QString::number(iIndex) + ",0,0,null    ,''   ,0    ,0      ,0          ,'" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "')";
				if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
			}

			// Update
			strQuery = "UPDATE ym_tasks SET ";
			strQuery+= "user_id=" + QString::number(ptTask->iUserId);
			strQuery+= ", group_id=" + QString::number(ptTask->iGroupId);
			strQuery+= ", database_id=" + QString::number(ptTask->iDatabaseId);
			strQuery+= ", name='" + ptYield->strTitle +"'";
			strQuery+= ", type=12";
			strQuery+= ", enabled=" + (ptTask->bEnabled ? "1" : "0");
			strQuery+= ", permisions=" + QString::number(ptTask->iPermissions);
			strQuery+= ", last_execution='" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"'";
			strQuery+= " WHERE task_id="+QString::number(iIndex);
			if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();


			// Write rule to disk
			ptTask->iTaskId = iIndex;
			SaveDbTaskSectionYieldMonitoring_RDB_OneRule(iIndex,ptYield);
		}
	}
*/
}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section SPEC MONITORING
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionSpecMonitoring(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptSpec->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ProductID','" + ptTask->ptSpec->strProductID + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Parameter','" + ptTask->ptSpec->strParameter + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MonitorInfo','" + QString::number(ptTask->ptSpec->iMonitorInfo) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LowSpec','" + ptTask->ptSpec->strLowSpec + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'HighSpec','" + ptTask->ptSpec->strHighSpec + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFrom','" + ptTask->ptSpec->strEmailFrom + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTask->ptSpec->strEmailNotify + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTask->ptSpec->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExceptionLevel','" + QString::number(ptTask->ptSpec->iExceptionLevel) + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section REPORTING
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionReporting(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;

	QString	strEmailNotify;		// Email addresses to notify.

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptReporting->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ScriptPath','" + ptTask->ptReporting->strScriptPath + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Frequency','" + QString::number(ptTask->ptReporting->iFrequency) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'DayOfWeek','" + QString::number(ptTask->ptReporting->iDayOfWeek) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExecWindow','" + (ptTask->ptReporting->bExecutionWindow ? "YES" : "NO") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'StartTime','" + QString::number(ptTask->ptReporting->cStartTime.hour()) + "," + QString::number(ptTask->ptReporting->cStartTime.minute()) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'StopTime','" + QString::number(ptTask->ptReporting->cStopTime.hour()) + "," + QString::number(ptTask->ptReporting->cStopTime.minute()) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'NotificationType','" + QString::number(ptTask->ptReporting->iNotificationType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTask->ptReporting->strEmailNotify + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTask->ptReporting->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LastExecuted','" + QDateTime::fromTime_t(ptTask->ptReporting->tLastExecuted).toString("yyyy-MM-dd hh:mm:ss") + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section STATUS
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionStatus(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptStatus->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'WebOrganization','" + (ptTask->ptStatus->bOneWebPerDatabase ? "1" : "0") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'IntranetPath','" + ptTask->ptStatus->strIntranetPath + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'HomePage','" + ptTask->ptStatus->strHomePage + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ReportURL','" + ptTask->ptStatus->strReportURL + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section FILE CONVERTER
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionFileConverter(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;


	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptConverter->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'InputFolder','" + ptTask->ptConverter->strInputFolder + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ImportFileExtensions','" + ptTask->ptConverter->strFileExtensions + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Frequency','" + QString::number(ptTask->ptConverter->iFrequency) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'DayOfWeek','" + QString::number(ptTask->ptConverter->iDayOfWeek) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'OutputFolder','" + ptTask->ptConverter->strOutputFolder + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'OutputFormat','" + QString::number(ptTask->ptConverter->iFormat) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'TimeStampFile','" + QString::number(ptTask->ptConverter->bTimeStampName) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SuccessMode','" + QString::number(ptTask->ptConverter->iOnSuccess) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'SuccessFolder','" + ptTask->ptConverter->strOutputSuccess + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'FailMode','" + QString::number(ptTask->ptConverter->iOnError) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'FailFolder','" + ptTask->ptConverter->strOutputError + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LastExecuted','" + QDateTime::fromTime_t(ptTask->ptConverter->tLastExecuted).toString("yyyy-MM-dd hh:mm:ss") + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section OUTLIER REMOVAL
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionOutlierRemoval(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptOutlier->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ProductID','" + ptTask->ptOutlier->strProductID + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmLevel','" + QString::number(ptTask->ptOutlier->lfAlarmLevel) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'AlarmType','" + QString::number(ptTask->ptOutlier->iAlarmType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'MinimumParts','" + QString::number(ptTask->ptOutlier->lMinimumyieldParts) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'NotifyShapeChange','" + (ptTask->ptOutlier->bNotifyShapeChange ? "1" : "0") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'CompositeEtestAlarm','" + QString::number(ptTask->ptOutlier->lCompositeEtestAlarm) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'CompositeExclusionZoneAlarm','" + QString::number(ptTask->ptOutlier->lCompositeExclusionZoneAlarm) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTask->ptOutlier->strEmailNotify + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailFormat','" + (ptTask->ptOutlier->bHtmlEmail ? "HTML" : "TEXT") + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'EmailReportType','" + QString::number(ptTask->ptOutlier->iEmailReportType) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'NotificationType','" + QString::number(ptTask->ptOutlier->iNotificationType) + "')";

	// Alarm type: Standard, Critical...
	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ExceptionLevel','" + QString::number(ptTask->ptOutlier->iExceptionLevel) + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// Save Tasks to YieldManDb: Section Auto Admin
///////////////////////////////////////////////////////////
void GexMoScheduler::SaveDbTaskSectionAutoAdmin(CGexMoTaskItem *	ptTask)
{
	if(pGexMainWindow->m_pYieldManDb)
		return;

	QSqlQuery	clQuery(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->m_sqlDatabase);
	QString strQuery;


	// Delete all entry from this task
	strQuery = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(ptTask->iTaskId);
	if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();

	QString		strAnd;
	strQuery = "INSERT INTO ym_tasks_options VALUES";
	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
		strAnd = ",";
	else
		strAnd = ";\n" + strQuery;

	// Insert new values
	strQuery+= "(" + QString::number(ptTask->iTaskId) + ",'Title','" + ptTask->ptAutoAdmin->strTitle + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'StartTime','" +  QString::number(ptTask->ptAutoAdmin->cStartTime.hour()) + "," + QString::number(ptTask->ptAutoAdmin->cStartTime.minute()) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'Emails','" + ptTask->ptAutoAdmin->strEmailNotify + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'KeepReportDuration','" + QString::number(ptTask->ptAutoAdmin->iKeepReportDuration) + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LogContents','" + QString::number(ptTask->ptAutoAdmin->iLogContents) + "')";

	// Shell on alarms
	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellYieldAlarm','" + ptTask->ptAutoAdmin->strShellYieldAlarm_Std + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellParameterAlarm','" + ptTask->ptAutoAdmin->strShellParameterAlarm_Std + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellPatAlarm','" + ptTask->ptAutoAdmin->strShellPatAlarm_Std + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellYieldAlarmCritical','" + ptTask->ptAutoAdmin->strShellYieldAlarm_Critical + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellParameterAlarmCritical','" + ptTask->ptAutoAdmin->strShellParameterAlarm_Critical + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'ShellPatAlarmCritical','" + ptTask->ptAutoAdmin->strShellPatAlarm_Critical + "')";

	strQuery+= strAnd + "(" + QString::number(ptTask->iTaskId) + ",'LastExecuted','" + QDateTime::fromTime_t(ptTask->ptAutoAdmin->tLastExecuted).toString("yyyy-MM-dd hh:mm:ss") + "')";

	if(pGexMainWindow->m_pYieldManDb->m_pDatabaseConnector->IsMySqlDB())
	{
		if(!clQuery.exec(strQuery)) strQuery = clQuery.lastError().text();
	}
	else
	{
		if(!clQuery.exec("BEGIN\n"+strQuery+"END;\n")) strQuery = clQuery.lastError().text();
	}

}

///////////////////////////////////////////////////////////
// signal select item
///////////////////////////////////////////////////////////
void GexMoScheduler::OnSelectItem()
{

	QTableWidget *ptTable = NULL;
	QWidget * pCurrentTab = tabWidget->currentWidget();

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	pGexMainWindow->OnActiveUser();

	if(pCurrentTab == tabTasksDataPump)
		ptTable = tableWidgetTasksListDataPump;
	else
	if(pCurrentTab == tabTasksReporting)
		ptTable = tableWidgetTasksListReporting;
	else
	if(pCurrentTab == tabTasksParameterSpecs)
		ptTable = tableWidgetTasksListParameterSpecs;
	else
	if(pCurrentTab == tabTasksConverter)
		ptTable = tableWidgetTasksListConverter;
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
		ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

	QTableWidgetItem *ptItem = NULL;
	ptTable->selectRow(ptTable->currentRow());

}

///////////////////////////////////////////////////////////
// signal select header
///////////////////////////////////////////////////////////
void GexMoScheduler::OnSelectHeader(int nColumn)
{
	QWidget * pCurrentTab = tabWidget->currentWidget();

	// If no item in list, just return!
	if(pCurrentTab == NULL)
		return;

	QTableWidget *ptTable = NULL;

	if(pCurrentTab == tabTasksDataPump)
		ptTable = tableWidgetTasksListDataPump;
	else
	if(pCurrentTab == tabTasksReporting)
		ptTable = tableWidgetTasksListReporting;
	else
	if(pCurrentTab == tabTasksParameterSpecs)
		ptTable = tableWidgetTasksListParameterSpecs;
	else
	if(pCurrentTab == tabTasksConverter)
		ptTable = tableWidgetTasksListConverter;
	else
	if(pCurrentTab == tabTasksOutlierRemoval)
		ptTable = tableWidgetTasksListOutlierRemoval;

	// If no item in list, just return!
	if(ptTable == NULL)
		return;

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
// set Table cell contents + tootip
///////////////////////////////////////////////////////////
void GexMoScheduler::fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText, QString strTooltip/*=""*/, bool bReadOnly/*=false*/, bool bAlarm/*=false*/)
{
	QTableWidgetItem *ptItem = ptTable->item(iRow,iCol);

	if(ptItem == NULL)
	{
		ptItem = new QTableWidgetItem(strText);
		// Add item to table cell
		ptTable->setItem(iRow,iCol,ptItem);
	}

	ptItem->setText(strText);

	// Add tooltip
	ptItem->setToolTip(strTooltip);

	// Check if Read-Only mode
	if(bReadOnly)
		ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);

	// Check if Alarm color (orange) to use as background color
	if(bAlarm)
		ptItem->setBackground(QBrush(QColor(255,146,100)));	// Orange background

}
