/****************************************************************************
** Deriven from gexftp_mainwindow_base.cpp
****************************************************************************/
#include "gexftp_mainwindow.h"
#include "gexftp_client.h"
#include "gexftp_service_widget.h"
#include "gexftp_page_widget.h"

#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>
#include <gqtl_skin.h>
#include <gex_constants.h>

#include <QTimer>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QDesktopWidget>
#include <QCloseEvent>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


// Some constants
#define GEX_FTP_BUTTON_START_TEXT		"&Start"
#define GEX_FTP_BUTTON_STOP_TEXT		"&Stop"

/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtp_mainwindow as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtp_mainwindow::CGexFtp_mainwindow(QApplication* qApplication, 
										const QString & strUserHome, 
										const QString & strApplicationPath,  
										const QString& strLocalFolder, 
										const QString & strApplicationName, 
										QWidget* parent, 
										Qt::WindowFlags fl)
		: QMainWindow(parent, fl ), CGexFtpMainBase(strUserHome, strApplicationPath, strLocalFolder, strApplicationName)
{
	// Setup UI
	setupUi(this);
	
	// Apply default palette 
	CGexSkin gexSkin;
	gexSkin.applyPalette(this);
		
	QString strPixMapSource;
	
	// Initialize data
	m_qApplication		= qApplication;

	setWindowTitle(strApplicationName);
	m_strImagesDirectory	= strApplicationPath + "/images";
	CGexSystemUtils::NormalizePath(m_strImagesDirectory);
	strPixMapSource			= strApplicationPath + "/images/gexftp_info.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxInfo				= QPixmap(strPixMapSource);
	strPixMapSource			= strApplicationPath + "/images/gexftp_transferred.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxInfoTransfer		= QPixmap(strPixMapSource);
	strPixMapSource			= strApplicationPath + "/images/gexftp_error.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxError				= QPixmap(strPixMapSource);
	strPixMapSource			= strApplicationPath + "/images/gexftp_warning.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxWarning				= QPixmap(strPixMapSource);
	m_bServicePaused		= false;

	// Service page
	m_pServicePage = NULL;
	
	// Disable sorting on log listview
	QStringList strHeader;
	strHeader << "Date" << "Ftp server" << "Message";
	treeWidgetLog->setHeaderLabels(strHeader);
	
	// At startup, spooling is off, and validated only if Init() successful
	m_bStartEnabled = false;

	// Set the source of html help viewer
	QString strHtmlSource = strApplicationPath + "/help/pages/gexftp_help.htm";
	CGexSystemUtils::NormalizePath(strHtmlSource);
	textBrowserHelp->setSource(QUrl::fromLocalFile(strHtmlSource));

	// Init page
	m_nPageNb		= 0;
	m_nPage_Help	= -1;
	m_nPage_General	= -1;
	m_nPage_Ftp		= -1;
	m_nPage_Log		= -1;
	m_nPage_Service	= -1;
	
	// signals and slots connections
    connect( this,		SIGNAL( sMainSelectionChanged() ),			this, SLOT( OnMainSelectionChanged() ) );
    connect( m_pCore,	SIGNAL( sStartSpooling() ),					this, SLOT( OnStartSpooling() ) );
    connect( m_pCore,	SIGNAL( sStopSpooling() ),					this, SLOT( OnStopSpooling() ) );
	connect( m_pCore,	SIGNAL( sSpoolingTimer(bool)),				this, SLOT( OnSpoolingTimer(bool)));
	
	connect( m_pCore,	SIGNAL( sStartDownload(const QString&)),	this, SLOT( onStartDownload(const QString&)));
	connect( m_pCore,	SIGNAL( sStopDownload()),					this, SLOT( onStopDownload()));

	connect( m_pCore,	SIGNAL( sStatusChanged(CGexFtpCore::Status, const QString&, const QString&)),	this, SLOT(OnStatusChanged(CGexFtpCore::Status, const QString&, const QString&)));
	connect( m_pCore,	SIGNAL( sLogInfo(int, const QString&, const QString&, const QString&)),			this, SLOT(OnLog(int, const QString&, const QString&, const QString&)));

	connect( m_pCore->settings(), SIGNAL(sSettingsLoaded(bool, const QString&)),	this, SLOT(onSettingsLoaded(bool, const QString&)));
	connect( m_pCore->settings(), SIGNAL(sSettingsSaved(bool, const QString&)),		this, SLOT(onSettingsSaved(bool, const QString&)));

	connect( buttonHelp,				SIGNAL( clicked() ),				this,					SLOT( OnButtonHelp() ) );
	connect( listWidgetMainSelection,	SIGNAL(itemSelectionChanged() ),	this,					SLOT( OnMainSelectionChanged() ) );
	connect( buttonExit,				SIGNAL( clicked() ),				this,					SLOT( OnCloseWindow() ) );
	connect( buttonStartStop,			SIGNAL( clicked() ),				this,					SLOT( OnButtonStartStop() ) );
	connect( buttonSave,				SIGNAL( clicked() ),				m_pCore->settings(),	SLOT( save()) );
	
	connect( m_pCore->settings(),		SIGNAL(sProfileInChange(bool)),		buttonStartStop,		SLOT( setDisabled(bool)) );
}

/////////////////////////////////////////////////////////////////////////////////////
// Adds a page
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::AddPage(const QString& strSelectionText, const QString& strSelectionPixmap, QWidget* pPage, bool bDisableWhenRunning)
{
	GexFtp_Page		Page;
	QString			strPixmap;

	// Add to list of pages
	Page.pPage = pPage;
	Page.bDisableWhenRunning = bDisableWhenRunning;
	m_listPages.append(Page);
	
	// Insert item into selection listbox
	strPixmap = m_strImagesDirectory + "/" + strSelectionPixmap;
	CGexSystemUtils::NormalizePath(strPixmap);

	new QListWidgetItem(QIcon(strPixmap), strSelectionText, listWidgetMainSelection);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtp_mainwindow::~CGexFtp_mainwindow()
{
   	
}

///////////////////////////////////////////////////////////
// Resize GEX-FTP application to fit in screen...
///////////////////////////////////////////////////////////
void CGexFtp_mainwindow::ResizeApplication(void)
{
	int	iSizeX,iSizeY;

    QDesktopWidget *d = QApplication::desktop();
	int iWidth = d->width();     // returns desktop width

	if(iWidth <= 640)
	{
		move(0,0);
		iSizeX = 350;
		iSizeY = 350;
	}
	else
	if(iWidth <= 800)
	{
		move(0,0);
		iSizeX = 350;
		iSizeY = 350;
	}
	else
	{
		move(20,20);
		iSizeX = 350;
		iSizeY = 350;
	}

	// Resize application
	resize( iSizeX,iSizeY );
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialize data
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtp_mainwindow::init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication, QtServiceController * pServiceController)
{ 
	bool bInit = m_pCore->init(bStartFtpSpoolerAtLaunch, bRunAsService, pbExitApplication);
	
	if (*pbExitApplication == false)
	{
		// Resize the application window
		ResizeApplication();

		if(m_pCore->runAsService() == true)
			// Servive mode: disable 'Exit' button
			buttonExit->setEnabled(false);

		// Initialize widget
		initGui(pServiceController);
	}

	// Display the window if core initialization is not correct or application is not runnng as a service
	if (bInit == false || bRunAsService == false)
		show();
	
	return bInit;
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialize gui widget
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::initGui(QtServiceController * pServiceController)
{
	radioButtonEvery->setChecked(m_pCore->settings()->dataGeneral().every());
	radioButtonDaily->setChecked(m_pCore->settings()->dataGeneral().daily());
	radioButtonWeekly->setChecked(m_pCore->settings()->dataGeneral().weekly());
	
	spinBoxHours->setValue(m_pCore->settings()->dataGeneral().everyHour());
	spinBoxMinutes->setValue(m_pCore->settings()->dataGeneral().everyMinute());
	spinBoxSeconds->setValue(m_pCore->settings()->dataGeneral().everySecond());

	timeEditDaily->setTime(m_pCore->settings()->dataGeneral().dailyAt());
	spinBoxDays->setValue(m_pCore->settings()->dataGeneral().dailyEveryNDays());

	timeEditWeekly->setTime(m_pCore->settings()->dataGeneral().weeklyAt());
	comboWeekdays->setCurrentIndex(m_pCore->settings()->dataGeneral().weeklyOn());

	spinDaysToKeepLogFiles->setValue(m_pCore->settings()->dataGeneral().nbDaysToKeepLog());

	checkBoxFullLog->setChecked(m_pCore->settings()->dataGeneral().fullLog());
	checkBoxEnableFtpTimeout->setChecked(m_pCore->settings()->dataGeneral().ftpTimeout());
	spinBoxFtpTimeoutSeconds->setValue(m_pCore->settings()->dataGeneral().ftpTimeoutAfter());
	spinBoxFileAgeBeforeTransfer_Min->setValue(m_pCore->settings()->dataGeneral().fileAgeBeforeTransfer_Min());
	spinBoxMaxReconnectionAttempts->setValue(m_pCore->settings()->dataGeneral().maxReconnectionAttempts());
	checkBoxEnableMaxFilePerProfile->setChecked(m_pCore->settings()->dataGeneral().enableMaxFilePerProfile());
	spinBoxMaxFilePerProfile->setValue(m_pCore->settings()->dataGeneral().maxFilePerProfile());
        m_poCBCaseInsensitive->setChecked(m_pCore->settings()->dataGeneral().caseInsensitive());
	// Create FTP settings page
	m_pFtpPage = new CGexFtpPage_widget(m_pCore->settings(), stackedWidget);
	connect(m_pFtpPage->GetSettingsWidget(), SIGNAL(sDisplayHelp()), this, SLOT(DisplayFtpSettingsHelp()));
	stackedWidget->addWidget(m_pFtpPage);
	
	// Add page selections
	m_nPageNb = 0;
	AddPage(tr( "Help" ), "gexftp_help.png", pageHelp, false);
	m_nPage_Help = m_nPageNb++;
	AddPage(tr( "General" ), "gexftp_general.png", pageGeneral, true);
	m_nPage_General = m_nPageNb++;
	AddPage(tr( "Ftp" ), "gexftp_ftpsettings.png", m_pFtpPage, true);
	m_nPage_Ftp = m_nPageNb++;
	AddPage(tr( "Activity log" ), "gexftp_log.png", pageLog, false);
	m_nPage_Log = m_nPageNb++;

	// If service installed, add service page
	if(pServiceController && pServiceController->isInstalled())
	{
		// Create service page widget and ass it to the stack
		m_pServicePage = new CGexFtpService_widget(&m_pxInfo, &m_pxInfoTransfer, &m_pxError, &m_pxWarning, pServiceController, m_pCore->settings(), stackedWidget);
		stackedWidget->addWidget(m_pServicePage);

		// Add page to list
		AddPage(tr( "Service" ), "gexftp_service.png", (QWidget *)m_pServicePage, true);
		m_nPage_Service = m_nPageNb++;
	}

	// Display 'Ftp' page on startup
	listWidgetMainSelection->setCurrentRow(m_nPage_Ftp);
	emit sMainSelectionChanged();

	OnStateChanged_FtpSpoolingSettings();

	connect( buttonClearLog,					SIGNAL( clicked() ),			this,										SLOT( OnButtonClearLog() ) );

	connect( radioButtonDaily,					SIGNAL( toggled(bool) ),		this,										SLOT( OnStateChanged_FtpSpoolingSettings() ) );
	connect( radioButtonEvery,					SIGNAL( toggled(bool) ),		this,										SLOT( OnStateChanged_FtpSpoolingSettings() ) );
	connect( radioButtonWeekly,					SIGNAL( toggled(bool) ),		this,										SLOT( OnStateChanged_FtpSpoolingSettings() ) );
	connect( checkBoxEnableFtpTimeout,			SIGNAL( stateChanged(int) ),	this,										SLOT( OnStateChanged_FtpSpoolingSettings() ) );
	connect( checkBoxEnableMaxFilePerProfile,	SIGNAL( stateChanged(int) ),	this,										SLOT( OnStateChanged_FtpSpoolingSettings() ) );

	connect( radioButtonDaily,					SIGNAL( toggled(bool) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setDaily(bool) ) );
	connect( radioButtonEvery,					SIGNAL( toggled(bool) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setEvery(bool) ) );
	connect( radioButtonWeekly,					SIGNAL( toggled(bool) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setWeekly(bool) ) );
	connect( checkBoxEnableFtpTimeout,			SIGNAL( stateChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setFtpTimeout(int) ) );
	connect( spinBoxHours,						SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setEveryHour(int) ) );
	connect( spinBoxMinutes,					SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setEveryMinute(int) ) );
	connect( spinBoxSeconds,					SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setEverySecond(int) ) );
	connect( timeEditDaily,						SIGNAL( timeChanged(const QTime&) ),&m_pCore->settings()->dataGeneral(),	SLOT( setDailyAt(const QTime&) ) );
	connect( spinBoxDays,						SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setDailyEveryNDays(int) ) );
	connect( timeEditWeekly,					SIGNAL( timeChanged(const QTime&) ),&m_pCore->settings()->dataGeneral(),	SLOT( setWeeklyAt(const QTime&) ) );
	connect( comboWeekdays,						SIGNAL( activated(int) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setWeeklyOn(int) ) );
	connect( spinDaysToKeepLogFiles,			SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setNbDaysToKeepLog(int) ) );
	connect( checkBoxFullLog,					SIGNAL( toggled(bool) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setFullLog(bool) ) );
        connect( m_poCBCaseInsensitive,					SIGNAL( toggled(bool) ),		&m_pCore->settings()->dataGeneral(),		SLOT( setCaseInsensitive(bool) ) );
	connect( spinBoxFtpTimeoutSeconds,			SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setFtpTimeoutAfter(int) ) );
	connect( spinBoxFileAgeBeforeTransfer_Min,	SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setFileAgeBeforeTransfer_Min(int) ) );

	connect( spinBoxMaxReconnectionAttempts,	SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setMaxReconnectionAttempts(int)) );
	connect( checkBoxEnableMaxFilePerProfile,	SIGNAL( toggled(bool)),			&m_pCore->settings()->dataGeneral(),		SLOT( setEnableMaxFilePerProfile(bool)) );
	connect( spinBoxMaxFilePerProfile,			SIGNAL( valueChanged(int) ),	&m_pCore->settings()->dataGeneral(),		SLOT( setMaxFilePerProfile(int)) );

	connect(m_pCore->settings(),				SIGNAL(sSettingsChanged()),		this,										SLOT(OnSettingsChanged()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Pause execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::pause(void)
{ 
	m_bServicePaused = true;

	m_pCore->pause();
}

/////////////////////////////////////////////////////////////////////////////////////
// Resume execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::resume(void)
{ 
	m_bServicePaused = false;

	if (m_bStartEnabled == true)
		m_pCore->resume();
}

/////////////////////////////////////////////////////////////////////////////////////
// Stopexecution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::stop(void)
{ 
	m_pCore->stop();
}

/////////////////////////////////////////////////////////////////////////////////////
// Main selection changed, switch to correct widget in the stack
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnMainSelectionChanged()
{
	// Get selected item and activate corresponding page
	stackedWidget->setCurrentWidget(m_listPages[listWidgetMainSelection->currentRow()].pPage);
}

/////////////////////////////////////////////////////////////////////////////////////
// Clear Log
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnButtonHelp()	
{
	// Display 'Help' page
	listWidgetMainSelection->setCurrentRow(m_nPage_Help);
	emit sMainSelectionChanged();	
}

/////////////////////////////////////////////////////////////////////////////////////
// Display help
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnButtonClearLog()
{
	treeWidgetLog->clear();

	m_pCore->cleanLogFiles();
}

/////////////////////////////////////////////////////////////////////////////////////
// Start spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnStartSpooling()
{
#ifdef _WIN32
	// Hide application if running as a Windows service
	if(m_pCore->runAsService() && !m_bServicePaused)
		hide();
#endif // _WIN32

	// Disable "Save settings" button
	buttonSave->setEnabled(false);

	// change button text
	buttonStartStop->setText(GEX_FTP_BUTTON_STOP_TEXT);
	
	// Disable pages
	for(QList<GexFtp_Page>::iterator it = m_listPages.begin(); it != m_listPages.end(); ++it)
	{
		if((*it).bDisableWhenRunning == true)
		{
			CGexFtpPage_widget * pPage = qobject_cast<CGexFtpPage_widget *>((*it).pPage);
	
			if (pPage)
				pPage->setPageEnabled(false);
			else
				(*it).pPage->setEnabled(false);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnStopSpooling()
{
	// Enable "Save settings" button if settings changed
	buttonSave->setEnabled(m_pCore->settings()->dataChanged());

	// We need to stop the timers and change button text
	buttonStartStop->setText(GEX_FTP_BUTTON_START_TEXT);
		
	// Enable all pages
	for(QList<GexFtp_Page>::iterator it = m_listPages.begin(); it != m_listPages.end(); ++it)
	{
		CGexFtpPage_widget * pPage = qobject_cast<CGexFtpPage_widget *>((*it).pPage);
	
		if (pPage)
			pPage->setPageEnabled(true);
		else
			(*it).pPage->setEnabled(true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Start/Stop button was pressed: start or stop the timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnButtonStartStop()
{
	if(m_pCore->running())
		m_pCore->stop();
	else
		m_pCore->start();
}

/////////////////////////////////////////////////////////////////////////////////////
// Set text in status bar
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnStatusChanged(CGexFtpCore::Status eStatus,
										 const QString& strArg1 /*= ""*/,
										 const QString& /*strArg2 = ""*/)
{
	QString strString = "Unknown";
	int		nNextSpooling_Min, nNextSpooling_Sec;

	switch(eStatus)
	{
		case CGexFtpCore::StatusReady:
			strString = tr("Ready");
			break;
		case CGexFtpCore::StatusCountDown:
			if(m_pCore->nextSpooling() == -1)
			{
				strString = "Next Ftp server check: ";
				strString += m_pCore->nextCheck().toString();
			}
			else
			{
				nNextSpooling_Min = m_pCore->nextSpooling() / 60;
				nNextSpooling_Sec = m_pCore->nextSpooling() - (nNextSpooling_Min * 60);
				if(nNextSpooling_Min == 0)
					strString.sprintf("Next Ftp server check in %d seconds...", m_pCore->nextSpooling());
				else if(nNextSpooling_Min == 1)
					strString.sprintf("Next Ftp server check in 1 minute %d seconds...", nNextSpooling_Sec);
				else
					strString.sprintf("Next Ftp server check in %d minutes %d seconds...", nNextSpooling_Min, nNextSpooling_Sec);
			}
			break;
		case CGexFtpCore::StatusCheckingFtpServers:
			strString = tr("Checking for files to download on FTP servers...");
			break;
		case CGexFtpCore::StatusCheckingFtpServer:
			strString = tr("Checking for files to download on FTP server ");
			strString += strArg1;
			strString += tr("...");
			break;
	}

	labelStatus->setText(strString);
	m_qApplication->processEvents();
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that some Ftp settings have changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnSettingsChanged()
{
	// Enable "Save settings" button
	buttonSave->setEnabled(m_pCore->settings()->dataChanged());
}

/////////////////////////////////////////////////////////////////////////////////////
// Close event received, check if settings have been changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::closeEvent(QCloseEvent * e)
{
	// Need to save changes ?
	if(m_pCore->settings()->dataChanged())
	{
		int nStatus = QMessageBox::warning(this, "Ftp Settings", "Some settings or Ftp profiles have changed.\nDo you want to save them?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
			
		if (nStatus ==QMessageBox::Cancel)
		{
			e->ignore();
			return;
		}
		else if(nStatus == QMessageBox::Yes)
		{
			m_pCore->settings()->save();
		}
	}

	e->accept();
}

/////////////////////////////////////////////////////////////////////////////////////
// Custom event received
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::customEvent(QEvent * e)
{
	if(e->type() == QEvent::User+10)
	{
		pause();
		showNormal();
		raise();
	}
	else if(e->type() == QEvent::User+20)
	{
		resume();
		
		if (m_bStartEnabled)
			hide();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// The application is being closed: send the close signal
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnCloseWindow()
{
	// Close Window
	close();
}


/////////////////////////////////////////////////////////////////////////////////////
// State of 1 radio button changed in Ftp server spooling settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnStateChanged_FtpSpoolingSettings()
{
	// Radio button 'Every...'
	spinBoxHours->setEnabled(radioButtonEvery->isChecked());
	spinBoxMinutes->setEnabled(radioButtonEvery->isChecked());
	spinBoxSeconds->setEnabled(radioButtonEvery->isChecked());

	// Radio button 'Daily...'
	timeEditDaily->setEnabled(radioButtonDaily->isChecked());
	spinBoxDays->setEnabled(radioButtonDaily->isChecked());

	// Radio button 'Weekly...'
	timeEditWeekly->setEnabled(radioButtonWeekly->isChecked());
	comboWeekdays->setEnabled(radioButtonWeekly->isChecked());

	// Check box 'Enable FTP timeout'
	spinBoxFtpTimeoutSeconds->setEnabled(checkBoxEnableFtpTimeout->isChecked());

	// Check box “Limit to <user input> files for download per ftp profile.”
	spinBoxMaxFilePerProfile->setEnabled(checkBoxEnableMaxFilePerProfile->isChecked());
}

/////////////////////////////////////////////////////////////////////////////////////
// Display Ftp settings help section
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::DisplayFtpSettingsHelp()
{
	// Display 'Help' page on startup
	listWidgetMainSelection->setCurrentRow(m_nPage_Help);
	emit sMainSelectionChanged();
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that settings have been loaded
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::onSettingsLoaded(bool bResult,
										  const QString& /*strSettingsFile*/)
{
	// Error loading settings, disable start operation, Start/Stop button
	m_bStartEnabled = bResult;
	buttonStartStop->setEnabled(bResult);

	// Disable "Save settings" button, so user cannot save settings loaded from service to local settings file
	buttonSave->setEnabled(m_pCore->settings()->dataChanged());

	// Make sure relevant page is displayed on startup
	if (m_nPageNb != 0)
	{
		listWidgetMainSelection->setCurrentRow(m_nPage_Ftp);
		emit sMainSelectionChanged();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that settings have been loaded
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::onSettingsSaved(bool bResult,
										 const QString& /*strSettingsFile*/)
{
	if (!bResult)
	{
		QString strMessage;
		strMessage = "Couldn't save Ftp settings.\nSee log file for further information.";
		QMessageBox::warning(this, "SaveSettings", strMessage, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}

	// Enable start operation, Start/Stop button
	m_bStartEnabled = bResult;
	buttonStartStop->setEnabled(bResult);

	buttonSave->setEnabled(m_pCore->settings()->dataChanged());
}


/////////////////////////////////////////////////////////////////////////////////////
// Add to log file and log page
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnLog(int nMsgType, const QString& strFtpServer, const QString& strMsg, const QString& strDateTime)
{
	QString strMsgType;

	// Add to list view
	QTreeWidgetItem *pItem = new QTreeWidgetItem(treeWidgetLog);
	pItem->setText(0, strDateTime);
	pItem->setText(1, strFtpServer);
	pItem->setText(2, strMsg);

	switch(nMsgType)
	{
		case GEXFTP_MSG_INFO:
			strMsgType = "Info";
			pItem->setIcon(0, m_pxInfo);
			break;
		case GEXFTP_MSG_INFO_XFER:
			strMsgType = "InfoTransfer";
			pItem->setIcon(0, m_pxInfoTransfer);
			break;
		case GEXFTP_MSG_WARNING:
			strMsgType = "Warning";
			pItem->setIcon(0, m_pxWarning);
			break;
		case GEXFTP_MSG_ERROR:
			strMsgType = "Error";
			pItem->setIcon(0, m_pxError);
			break;
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that spooling timer is active or not
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtp_mainwindow::OnSpoolingTimer(bool bActive)
{
	buttonStartStop->setEnabled(bActive);
}

void CGexFtp_mainwindow::onStartDownload(const QString& strFtpServer)
{
	// Add download widget
	m_pFtpPage->StartTransfer(m_qApplication, m_pCore->applicationDir(), m_pCore->ftpClient()->downloader(), strFtpServer);
}

void CGexFtp_mainwindow::onStopDownload()
{
	// Add download widget
	m_pFtpPage->CloseTransfers();
}

void CGexFtp_mainwindow::onProfileChanging(bool bIsChangeApplied)
{
	buttonStartStop->setEnabled(bIsChangeApplied);
}
