/****************************************************************************
** Deriven from gexftp_page_widget_base.cpp
****************************************************************************/
#include "gexftp_page_widget.h"
#include "gexftp_settings_widget.h"
#include "gexftp_servertransfer_widget.h"
#include "gexftp_download.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpPage_widget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpPage_widget::CGexFtpPage_widget(CGexFtpSettings * pDataSettings, QWidget* parent, Qt::WindowFlags fl) : QWidget(parent, fl)
{
	// Setup UI
	setupUi(this);

	// Create FTP settings page
	m_pFtpSettingsWidget = new CGexFtpSettings_widget(pDataSettings, tabWidget);
	tabWidget->insertTab(0, m_pFtpSettingsWidget, QString("Settings"));
	tabWidget->setCurrentIndex(0);

	m_pDownloadWidget = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpPage_widget::~CGexFtpPage_widget()
{
    // no need to delete child widgets, Qt does it all for us
}

/////////////////////////////////////////////////////////////////////////////////////
// Enable FTP settings widget
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpPage_widget::setPageEnabled(bool bEnable)
{
	if(bEnable == true)
		m_pFtpSettingsWidget->setEnabled(true);
	else
		m_pFtpSettingsWidget->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////
// Close FTP transfers
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpPage_widget::CloseTransfers()
{
	if(m_pDownloadWidget != NULL)
	{
		stackedWidget->removeWidget(m_pDownloadWidget);
		delete m_pDownloadWidget;
		m_pDownloadWidget = NULL;
	}

	stackedWidget->setCurrentWidget(pageIdle);
}

/////////////////////////////////////////////////////////////////////////////////////
// Start FTP transfer for specified server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpPage_widget::StartTransfer(QApplication* qApplication, const QString & strApplicationPath, const CGexFtpDownload * pDownload, const QString& strSettingsName)
{
	if(m_pDownloadWidget != NULL)
	{
		stackedWidget->removeWidget(m_pDownloadWidget);
		delete m_pDownloadWidget;
		m_pDownloadWidget = NULL;
	}

	m_pDownloadWidget = new CGexFtpServerTransfer_widget(qApplication, strApplicationPath, strSettingsName, stackedWidget);

	connect( pDownload,	SIGNAL( sChangeTimeout(int)),							m_pDownloadWidget,	SLOT( onChangeTimeout(int)));
	connect( pDownload,	SIGNAL( sDownloadProgress(int, int)),					m_pDownloadWidget,	SLOT( onDownloadProgress(int, int)));
	connect( pDownload,	SIGNAL( sListFileInfo(int, int)),						m_pDownloadWidget,	SLOT( onListFileInfo(int, int)));
	connect( pDownload,	SIGNAL( sDownloadInfo(int, int, int)),					m_pDownloadWidget,	SLOT( onDownloadInfo(int, int, int)));
	connect( pDownload,	SIGNAL( sTransferStatus(int, const QString&, bool)),	m_pDownloadWidget,	SLOT( onTransferStatus(int, const QString&, bool)));
	connect( pDownload,	SIGNAL( sStartFileTransfer(const QString&, int, int)),	m_pDownloadWidget,	SLOT( onStartFileTransfer(const QString&, int, int)));
	connect( pDownload,	SIGNAL( sCleanup()),									m_pDownloadWidget,	SLOT( onCleanup()));
	connect( pDownload,	SIGNAL( sLogMessage(int, const QString&)),				m_pDownloadWidget,	SLOT( onLogMessage(int, const QString&)));
	connect( pDownload, SIGNAL( sFileInfo(const QUrlInfo&)),					m_pDownloadWidget,	SLOT( onFileInfo(const QUrlInfo&)));
	connect( pDownload, SIGNAL( sMaxFileToDownLoadInfo(int)),					m_pDownloadWidget, SLOT( onMaxFileToDownLoadInfo(int)));

	connect( m_pDownloadWidget, SIGNAL(sUserAbort()),							pDownload,			SLOT( onUserAbort()));
	stackedWidget->addWidget(m_pDownloadWidget);
	stackedWidget->setCurrentWidget(m_pDownloadWidget);
}

