/****************************************************************************
** Deriven from gex_email_mainwindow_base.cpp
****************************************************************************/

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextBrowser>
#include <QUrl>
#include <QDesktopWidget>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <gqtl_sysutils.h>
#include <gqtl_skin.h>
#include <gex_constants.h>

#include "gex_email_mainwindow.h"
#include "gex_email_send.h"
#include "gex_email_smtp.h"
#include "gex_email_sendwidget.h"


// Some constants
#define GEX_EMAIL_BUTTON_START_TEXT		"&Start"
#define GEX_EMAIL_BUTTON_STOP_TEXT		"&Stop"

/////////////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////////////
CGexEmailMainwindow::CGexEmailMainwindow(QApplication* /*qApplication*/,
										 const QString& strUserHome,
										 const QString& strApplicationDir,
										 const QString& strLocalFolder,
										 const QString& strApplicationName,
										 QWidget* parent,
										 Qt::WindowFlags fl)
    : QMainWindow(parent, fl), CGexEmailMainBase(strUserHome, strApplicationDir, strLocalFolder, strApplicationName)
{
	QString strPixMapSource;

	// Setup UI
	setupUi(this);

	// Set the default palette 
	CGexSkin gexSkin;
	gexSkin.applyPalette(this);
		
    // Signals / Slots connections
    connect( buttonBrowseEmailDirectory, SIGNAL( clicked() ), this, SLOT( OnButtonBrowseEmailDirectory() ) );
     connect( buttonExit, SIGNAL( clicked() ), this, SLOT( OnCloseWindow() ) );
    connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( OnButtonHelp() ) );
    connect( buttonSave, SIGNAL( clicked() ), this, SLOT( OnButtonSave() ) );
    connect( buttonStartStop, SIGNAL( clicked() ), this, SLOT( OnButtonStartStop() ) );
    connect( this, SIGNAL( sMainSelectionChanged(int) ), this, SLOT( OnMainSelectionChanged(int) ) );
    connect( listMainSelection, SIGNAL( currentRowChanged(int) ), this, SLOT( OnMainSelectionChanged(int) ) );
	connect( buttonClearLog, SIGNAL( clicked() ), this, SLOT( OnButtonClearLog() ) );
   		
	connect( m_pCore, SIGNAL(sStatusChanged(CGexEmailCore::Status, const QString&, const QString&)), this, SLOT(OnStatusChanged(CGexEmailCore::Status, const QString&, const QString&)));
	connect( m_pCore, SIGNAL(sStartSpooling()), this, SLOT(OnStartSpooling()));
    connect( m_pCore, SIGNAL(sStopSpooling()), this, SLOT(OnStopSpooling()));
	connect( m_pCore, SIGNAL(sLogInfo(int, const QString&, const QString&, const QString&)), this, SLOT(OnLog(int, const QString&, const QString&, const QString&)));
	connect( m_pCore, SIGNAL(sSpoolingTimer(bool)), this, SLOT(OnSpoolingTimer(bool)));
	connect( m_pCore, SIGNAL(sSettingsChanged() ), this, SLOT(OnSettingsChanged()));
	connect( m_pCore, SIGNAL(sEmailDirectoryChanged(const QString &) ), editEmailDirectory, SLOT( setText(const QString&) ) );
	connect( m_pCore, SIGNAL(sCriticalMessage(const QString&)), this, SLOT(onCriticalMessage(const QString&)));

	// Initialize data
	setWindowTitle(strApplicationName);
	m_strImagesDirectory = strApplicationDir + "/images";
	CGexSystemUtils::NormalizePath(m_strImagesDirectory);
	strPixMapSource = strApplicationDir + "/images/gexemail_info.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxInfo = QPixmap(strPixMapSource);
	strPixMapSource = strApplicationDir + "/images/gexemail_sent.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxInfoSent = QPixmap(strPixMapSource);
	strPixMapSource = strApplicationDir + "/images/gexemail_error.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxError = QPixmap(strPixMapSource);
	strPixMapSource = strApplicationDir + "/images/gexemail_warning.png";
	CGexSystemUtils::NormalizePath(strPixMapSource);
	m_pxWarning = QPixmap(strPixMapSource);

	m_strHtmlSource = strApplicationDir + "/help/pages/gexemail_help.htm";
	CGexSystemUtils::NormalizePath(m_strHtmlSource);
	
	m_bServicePaused	= true;

	// Enable start operation, Start/Stop button
	m_bStartEnabled		= false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexEmailMainwindow::~CGexEmailMainwindow()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Adds a page
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::AddPage(const QString& strSelectionText, const QString& strSelectionPixmap, QWidget* pPage, bool bDisableWhenRunning)
{
	GexEmail_Page	EmailPage;
	QString			strPixmap;

	// Add to list of pages
	EmailPage.pPage = pPage;
	EmailPage.bDisableWhenRunning = bDisableWhenRunning;
	m_listPages.append(EmailPage);
	
	// Insert item into selection listbox
	strPixmap = m_strImagesDirectory + "/" + strSelectionPixmap;
	CGexSystemUtils::NormalizePath(strPixmap);
	new QListWidgetItem(QIcon(strPixmap), strSelectionText, listMainSelection);
}

///////////////////////////////////////////////////////////
// Resize GEX-EMAIL application to fit in screen...
///////////////////////////////////////////////////////////
void CGexEmailMainwindow::ResizeApplication(void)
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
// Initialize window
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailMainwindow::init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication)
{ 
	bool bInit = m_pCore->init(bStartEmailSpoolerAtLaunch, bAllowUserToSelectEmailSpoolingDir, bRunAsService, pbExitApplication);

	if (*pbExitApplication == false)
	{
		// Resize the application window
		ResizeApplication();

		// Hide 'Browse' button?
		if(!bAllowUserToSelectEmailSpoolingDir)
			buttonBrowseEmailDirectory->hide();

		// Disable 'Exit' button in service mode
		if(m_pCore->runAsService() == true)
			buttonExit->setEnabled(false);

		// Disable "Save settings" button, because it may been enabled when loading some settings
		buttonSave->setEnabled(m_pCore->needSaveSettings());

		// Initialize widget
		initGui();

		uint uiPageToDisplay = m_uiPage_General;

		if (bInit)
		{
			// Enable start operation, Start/Stop button
			m_bStartEnabled = true;
			buttonStartStop->setEnabled(true);
		}
		else
			uiPageToDisplay = m_uiPage_Smtp;
		
		// Make sure relevant page is displayed on startup
		emit sMainSelectionChanged(uiPageToDisplay);
	}
	
	// Display the window if core initialization is not correct or application is not runnng as a service
	if (bInit == false || bRunAsService == false)
		show();

	return bInit;
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialize gui widget
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::initGui()
{
	// Add page selections
	unsigned int uiPageNb = 0;
	AddPage(tr( "Help" ), "gexemail_help.png", pageHelp, false);
	m_uiPage_Help		= uiPageNb++;

	AddPage(tr( "General" ), "gexemail_general.png", pageGeneral, true);
	m_uiPage_General	= uiPageNb++;

	const CGexEmailSend * pMailSend = m_pCore->mailSystem();

	if (pMailSend && pMailSend->type() == CGexEmailSend::sendType_smtp)
	{
		CGexEmailSendWidgetSmtp * pWidgetSmtp = new CGexEmailSendWidgetSmtp((CGexEmailSmtp*)pMailSend, widgetStack, 0);
		widgetStack->addWidget(pWidgetSmtp);
		
        AddPage(tr(pWidgetSmtp->name().toLatin1().constData()),
                pWidgetSmtp->icon(), pWidgetSmtp, false);
		m_uiPage_Smtp		= uiPageNb++;
	}
	
	AddPage(tr( "Activity log" ), "gexemail_log.png", pageLog, false);
	m_uiPage_Log		= uiPageNb++;

	// Setup log treeWidget
	QStringList headers;
	treeWidgetLog->setColumnCount(3);
	headers << tr("Date") << tr("Mail file") << tr("Message");
	treeWidgetLog->setHeaderLabels(headers);
	treeWidgetLog->setSortingEnabled(false);
	treeWidgetLog->setDragEnabled(false);
	treeWidgetLog->setRootIsDecorated(false);
	treeWidgetLog->setItemsExpandable(false);

	// Set the source of html help viewer
	textBrowserHelp->setSource(QUrl::fromLocalFile(m_strHtmlSource));	

	// Set the value for general page widget
	spinBoxHours->setValue(m_pCore->hours());
	spinBoxMinutes->setValue(m_pCore->minutes());
	spinBoxSeconds->setValue(m_pCore->seconds());
	
	checkBoxFullLog->setChecked(m_pCore->fullLog());
	spinDaysToKeepLogFiles->setValue(m_pCore->daysToKeepLogFiles());

	connect( spinBoxHours, SIGNAL( valueChanged(int) ), m_pCore, SLOT( setHours(int) ) );
    connect( spinBoxMinutes, SIGNAL( valueChanged(int) ), m_pCore, SLOT( setMinutes(int) ) );
    connect( spinBoxSeconds, SIGNAL( valueChanged(int) ), m_pCore, SLOT( setSeconds(int) ) );
    connect( spinDaysToKeepLogFiles, SIGNAL(valueChanged(int)), m_pCore, SLOT( setDaysToKeepFiles(int)));
	connect( checkBoxFullLog, SIGNAL(stateChanged(int)), m_pCore, SLOT( setFullLog(int)));
}

/////////////////////////////////////////////////////////////////////////////////////
// Pause execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::pause(void)
{ 
	m_bServicePaused = true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Resume execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::resume(void)
{ 
	m_bServicePaused = false;

	if (m_bStartEnabled)
		m_pCore->resume();
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::stop()
{
	m_pCore->stop();
}

/////////////////////////////////////////////////////////////////////////////////////
// Main selection changed, switch to correct widget in the stack
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnMainSelectionChanged(int nCurrntRow)
{
	// Get selected item and activate corresponding page
	listMainSelection->setCurrentRow(nCurrntRow);
	widgetStack->setCurrentWidget(m_listPages[nCurrntRow].pPage);
}

/////////////////////////////////////////////////////////////////////////////////////
// Clear Log
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnButtonHelp()
{
	// Display 'Help' page
	emit sMainSelectionChanged(m_uiPage_Help);
}

/////////////////////////////////////////////////////////////////////////////////////
// Display help
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnButtonClearLog()
{
	treeWidgetLog->clear();

	m_pCore->cleanLogFiles();
}

/////////////////////////////////////////////////////////////////////////////////////
// Save Settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnButtonSave()
{
	if(m_pCore->SaveSettings() == false)
	{
		QString strMessage;
		strMessage = "Couldn't save settings.\nSee log file for further information.";
		QMessageBox::warning(this, "SaveSettings", strMessage, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		m_bStartEnabled = false;
		buttonStartStop->setEnabled(false);
		return;
	}

	// Disable "Save settings" button
	buttonSave->setEnabled(m_pCore->needSaveSettings());

	// Enable start operation, Start/Stop button
	m_bStartEnabled = true;
	buttonStartStop->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Start spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnStartSpooling()
{
#ifdef _WIN32
	// Hide application if running as a Windows service
	if(m_pCore->runAsService() && !m_bServicePaused)
		hide();
#endif // _WIN32
		
	// Disable the Save button
	buttonSave->setEnabled(false);

	// We need to start the timer and change button text
	buttonStartStop->setText(GEX_EMAIL_BUTTON_STOP_TEXT);

	// Disable pages
	for(QList<GexEmail_Page>::iterator it = m_listPages.begin(); it != m_listPages.end(); ++it)
	{
		if((*it).bDisableWhenRunning == true)
			(*it).pPage->setEnabled(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnStopSpooling()
{
	// Enable "Save settings" button if settings changed
	buttonSave->setEnabled(m_pCore->needSaveSettings());

	// We need to stop the timers and change button text
	buttonStartStop->setText(GEX_EMAIL_BUTTON_START_TEXT);

	// Enable all pages
	for(QList<GexEmail_Page>::iterator it = m_listPages.begin(); it != m_listPages.end(); ++it)
		(*it).pPage->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Start/Stop button was pressed: start or stop the timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnButtonStartStop()
{
	if(m_pCore->running())
		m_pCore->stop();
	else
		m_pCore->start();
}

/////////////////////////////////////////////////////////////////////////////////////
// Set text in status bar
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnStatusChanged(CGexEmailCore::Status eStatus, const QString& strArg1 /*= ""*/, const QString& strArg2 /*= ""*/)
{
	QString strString = "Unknown";
	int		nNextSpooling_Min, nNextSpooling_Sec;

	switch(eStatus)
	{
		case CGexEmailCore::StatusReady:
			strString = tr("Ready");
			break;
		case CGexEmailCore::StatusCountDown:
			if(m_pCore->nextSpooling() == -1)
			{
				strString = "Next Mail check: ";
				strString += m_pCore->nextCheck().toString();
			}
			else
			{
				nNextSpooling_Min = m_pCore->nextSpooling()/60;
				nNextSpooling_Sec = m_pCore->nextSpooling() - (nNextSpooling_Min*60);
				if(nNextSpooling_Min == 0)
					strString.sprintf("Next Mail check in %d seconds...", m_pCore->nextSpooling());
				else if(nNextSpooling_Min == 1)
					strString.sprintf("Next Mail check in 1 minute %d seconds...", nNextSpooling_Sec);
				else
					strString.sprintf("Next Mail check in %d minutes %d seconds...", nNextSpooling_Min, nNextSpooling_Sec);
			}
			break;
		case CGexEmailCore::StatusCheckingMails:
			strString = tr("Checking for emails to send...");
			break;
		case CGexEmailCore::StatusProcessingMailFile:
			strString = "Processing email file " + strArg1;
			strString += " ...";
			break;
		case CGexEmailCore::StatusSendingMail:
			strString = "Sending file " + strArg1;
			strString += " to " + strArg2;
			strString += " ...";
			break;
	}

	labelStatus->setText(strString);
	m_qApplication->processEvents();
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to log file and log page
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnLog(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime)
{
	QString strMsgType;

	// Add to list view
	QTreeWidgetItem *pItem = new QTreeWidgetItem(treeWidgetLog);
	pItem->setText(0, strDateTime);
	pItem->setText(1, strMailFile);
	pItem->setText(2, strMsg);

	switch(nMsgType)
	{
		case GEXEMAIL_MSG_INFO:
			strMsgType = "Info";
			pItem->setIcon(0, m_pxInfo);
			break;
		case GEXEMAIL_MSG_INFO_SENT:
			strMsgType = "InfoSent";
			pItem->setIcon(0, m_pxInfoSent);
			break;
		case GEXEMAIL_MSG_WARNING:
			strMsgType = "Warning";
			pItem->setIcon(0, m_pxWarning);
			break;
		case GEXEMAIL_MSG_ERROR:
			strMsgType = "Error";
			pItem->setIcon(0, m_pxError);
			break;
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Close event received, check if settings have been changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::closeEvent(QCloseEvent * e)
{
	// Need to save changes ?
	if(m_pCore->needSaveSettings())
	{
		int nStatus = QMessageBox::warning(this, "Email Settings", "Some settings have changed.\nDo you want to save them?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
		if(nStatus == QMessageBox::Cancel)
			return;
		if(nStatus == QMessageBox::Yes)
		{
			if(m_pCore->SaveSettings() == false)
			{
				QString strMessage;
				strMessage = "Couldn't save Email settings.\nSee log file for further information.";
				QMessageBox::warning(this, "SaveSettings", strMessage, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			}
		}
	}

	e->accept();
}

/////////////////////////////////////////////////////////////////////////////////////
// Custom event received
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::customEvent(QEvent * e)
{
	if(e->type() == QEvent::User+10)
	{
		pause();
		showNormal();
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
void CGexEmailMainwindow::OnCloseWindow()
{
	// Close Window
	close();
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that some Email settings have changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnSettingsChanged()
{
	// Enable "Save settings" button
	buttonSave->setEnabled(m_pCore->needSaveSettings());
}

/////////////////////////////////////////////////////////////////////////////////////
// Browse for email direcory
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnButtonBrowseEmailDirectory()
{
	// Display Open file dialog so the user can select a file
	QString strFileName = QFileDialog::getExistingDirectory(this,"Select the directory where GEX mail files are stored", m_pCore->emailDirectory());

	// Check if new file selected
	if((strFileName.isEmpty() == true) || (m_pCore->emailDirectory() == strFileName))
		return;

	emit sSettingsChanged();
	m_pCore->SetEmailDirectory(strFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that spooling timer is active or not
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::OnSpoolingTimer(bool bActive)
{
	buttonStartStop->setEnabled(bActive);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that a error has been raised
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailMainwindow::onCriticalMessage(const QString& strMessage)
{
	QMessageBox::critical(NULL, m_pCore->applicationName(), strMessage);
}
