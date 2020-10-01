/****************************************************************************
** Deriven from gexftp_service_widget_base.cpp
****************************************************************************/
#ifdef _WIN32
#include <windows.h>
#endif

#include "gexftp_service_widget.h"
#include "gexftp_settings.h"

#include <gex_constants.h>
#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>

#include <QFileDialog>
#include <QTimer>
#include <QDate>
#include <QTextStream>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpService_widget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpService_widget::CGexFtpService_widget( const QPixmap *ppxInfo, const QPixmap *ppxInfoTransfer, const QPixmap *ppxError, const QPixmap *ppxWarning, QtServiceController * pServiceController, CGexFtpSettings * pDataSettings, QWidget* parent, Qt::WindowFlags fl )
						: QWidget(parent, fl) 
{
	// Setup UI
	setupUi(this);

	// Init some variables
	m_ppxInfo					= ppxInfo;
	m_ppxInfoTransfer			= ppxInfoTransfer;
	m_ppxError					= ppxError;
	m_ppxWarning				= ppxWarning;
	m_uiServiceLogFileLinesRead = 0;
	m_uiServiceLogFileSize		= 0;
	m_bSettingsOK				= true;
	m_pServiceController		= pServiceController;
	m_pDataSettings				= pDataSettings;

	// Configure log widget
	QStringList strHeader;
	strHeader << "Date" << "Ftp server" << "Message";
	treeWidgetLog->setHeaderLabels(strHeader);

	// Update GUI
	UpdateGUI();

	// signals and slots connections
    connect( buttonBrowseServiceHomeDir,	SIGNAL( clicked() ),							this,				SLOT( OnBrowseServiceHomeDir() ) );
    connect( buttonStartService,			SIGNAL( clicked() ),							this,				SLOT( OnButtonStartService() ) );
    connect( buttonStopService,				SIGNAL( clicked() ),							this,				SLOT( OnButtonStopService() ) );
    connect( buttonLoadServiceSettings,		SIGNAL( clicked() ),							m_pDataSettings,	SLOT( loadService() ) );
    connect( buttonSaveServiceSettings,		SIGNAL( clicked() ),							m_pDataSettings,	SLOT( saveService() ) );
	connect( m_pDataSettings,				SIGNAL( sSettingsLoaded(bool, const QString&)),	this,				SLOT( OnSettingsLoaded(bool)));

	// Create a timer to refresh widget GUI, depending on service status
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(UpdateGUI()) );
	timer->start( 1000 ); // 1 second timer
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpService_widget::~CGexFtpService_widget()
{
    // no need to delete child widgets, Qt does it all for us
}

/////////////////////////////////////////////////////////////////////////////////////
// Browse for service home directory
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpService_widget::OnBrowseServiceHomeDir(void)
{
	QString strDirectory = QFileDialog::getExistingDirectory(this, "Select the home directory used by the gex-ftp service", m_pDataSettings->serviceHomeDir());	

	// Check if different directory selected
	if(strDirectory != m_pDataSettings->serviceHomeDir())
	{
		m_pDataSettings->setServiceHomeDir(strDirectory);
		
		// Update GUI
		UpdateGUI();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Update GUI
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpService_widget::UpdateGUI(void)
{
	bool bServiceIsRunning, bSettingsFileExists;

	// Service home directory
	editServiceHomeDir->setText(m_pDataSettings->serviceHomeDir());

	// Build name of current service log file
	QString strServiceLogFile;
	CGexSystemUtils clSysUtils;
	strServiceLogFile = m_pDataSettings->serviceHomeDir() + "/gex_ftp_";
	strServiceLogFile += QDate::currentDate().toString(Qt::ISODate);
	strServiceLogFile += ".log";
	CGexSystemUtils::NormalizePath(strServiceLogFile);

	// Enable/Disable buttons
	bServiceIsRunning = (m_pServiceController && m_pServiceController->isRunning());
	bSettingsFileExists = QFile::exists(m_pDataSettings->serviceSettingsFile());
	
	// Start/Stop buttons
	buttonStartService->setEnabled(!bServiceIsRunning);
	buttonStopService->setEnabled(bServiceIsRunning);
	// Load button: enabled if service is not running and file exists
	// Save button: enabled if service is not running and settings OK and file exists
	buttonLoadServiceSettings->setEnabled(!bServiceIsRunning && bSettingsFileExists);
	buttonSaveServiceSettings->setEnabled(!bServiceIsRunning && m_bSettingsOK && bSettingsFileExists);

	// Update service log
	if(!bSettingsFileExists)
	{
		labelLog->setText("Service log: ??");
		m_uiServiceLogFileLinesRead = 0;
		m_uiServiceLogFileSize = 0;
		treeWidgetLog->clear();
	}
	else if(QFile::exists(strServiceLogFile))
	{
		// If first time or new log file, clear log
		if(m_strServiceLogFile != strServiceLogFile)
		{
			QString strLabel = "Servive log: " + strServiceLogFile;
			m_strServiceLogFile = strServiceLogFile;
			labelLog->setText(strLabel);
			m_uiServiceLogFileLinesRead = 0;
			m_uiServiceLogFileSize = 0;
			treeWidgetLog->clear();
		}

		// Read log file and display in log view
		QFile fileLog(m_strServiceLogFile);
		if(fileLog.size() != m_uiServiceLogFileSize)
		{
			if(fileLog.open(QIODevice::ReadOnly) == true)
			{
				QTextStream streamLog(&fileLog);
				for(unsigned int uiLine=0; uiLine<m_uiServiceLogFileLinesRead; uiLine++)
					streamLog.readLine();

				QString strLine = streamLog.readLine();
				QString strMsgType;
				while(!strLine.isNull())
				{
					if(strLine != "Date,Type,Ftp server,Message")
					{
						QTreeWidgetItem * pItem = new QTreeWidgetItem(treeWidgetLog);
						pItem->setText(0, strLine.section(',', 0, 0));
						pItem->setText(1, strLine.section(',', 2, 2));
						pItem->setText(2, strLine.section(',', 3, 3));
						strMsgType = strLine.section(',', 1, 1);
						
						if(strMsgType == "Info")
							pItem->setIcon(0, *m_ppxInfo);
						if(strMsgType == "InfoTransfer")
							pItem->setIcon(0, *m_ppxInfoTransfer);
						if(strMsgType == "Warning")
							pItem->setIcon(0, *m_ppxWarning);
						if(strMsgType == "Error")
							pItem->setIcon(0, *m_ppxError);
					}
					m_uiServiceLogFileLinesRead++;
					strLine = streamLog.readLine();
				}
				m_uiServiceLogFileSize = fileLog.size();
				fileLog.close();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Start service button clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpService_widget::OnButtonStartService(void)
{
	if (m_pServiceController && !m_pServiceController->isRunning())
		m_pServiceController->start();
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop service button clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpService_widget::OnButtonStopService(void)
{
	if (m_pServiceController && m_pServiceController->isRunning())
		m_pServiceController->stop();
}

/////////////////////////////////////////////////////////////////////////////////////
// Settings loaded signal received
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpService_widget::OnSettingsLoaded(bool bOK)
{
	m_bSettingsOK = bOK;

	// Update path in GUI
	UpdateGUI();
}


