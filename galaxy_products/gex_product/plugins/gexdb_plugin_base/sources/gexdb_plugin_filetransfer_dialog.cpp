/****************************************************************************
** Deriven from ftp_transfer_dialogbase.cpp
****************************************************************************/

// Local includes
#include "gexdb_plugin_filetransfer_dialog.h"
#include "gexdb_plugin_datafile.h"

// QT includes
#include "qftp.h"
#include <qhostaddress.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qdir.h>
#include <qtimer.h>

// Windows includes 
#ifdef _WIN32
#include <winsock.h>
#endif

// Standard includes
#if defined unix || __MACH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

// Galaxy includes
#include <gqtl_sysutils.h>

/////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_SEQUENCE_CONNECT					1
#define GEXFTP_SEQUENCE_LOGIN					2
#define GEXFTP_SEQUENCE_CHDIR					3
#define GEXFTP_SEQUENCE_GET						4
#define GEXFTP_SEQUENCE_CLOSE					5
#define GEXFTP_SEQUENCE_NEXTFILE				6

///////////////////////////////////////////////////////////
// Class for ftp transfer operations
///////////////////////////////////////////////////////////
// Error map
GBEGIN_ERROR_MAP(GexDbFtpTransferDialog)
	GMAP_ERROR(eFtpError,"FTP error: %s.")
	GMAP_ERROR(eCreateLocalFile,"Error creating local file: %s.")
	GMAP_ERROR(eRemoveLocalFile,"Error removing local file: %s.")
	GMAP_ERROR(eOperationAborted,"Operation aborted.")
	GMAP_ERROR(eFtpConnectionBroken,"Ftp connection lost.")
GEND_ERROR_MAP(GexDbFtpTransferDialog)

/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GexDbFtpTransferDialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GexDbFtpTransferDialog::GexDbFtpTransferDialog(const QString & strApplicationPath,
                                               const QString & strLocalFtpDirectory,
                                               tdGexDbPluginDataFileList *pFilesToTransfer,
                                               CGexSkin * pGexSkin,
                                               QWidget* parent,
                                               Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
	setupUi(this);

	// Set Examinator skin
	pGexSkin->applyPalette(this);
	
	QObject::connect(buttonAbort,	SIGNAL(clicked()), this, SLOT(OnButtonAbort()));
    QObject::connect(buttonDetails, SIGNAL(clicked()), this, SLOT(OnButtonDetails()));

	CGexSystemUtils	clSysUtils;
	QString			strPixMapSource;

	QWidget::hide();
	
	// Init private members
	m_pCurrentFile = NULL;
	m_strLocalFtpDirectory = strLocalFtpDirectory;
	m_plistFilesToTransfer = pFilesToTransfer;
	strPixMapSource.sprintf("%s/images/ftp_transferred.png", strApplicationPath.toLatin1().constData());
	clSysUtils.NormalizePath(strPixMapSource);
	m_pxTransferred = QPixmap(strPixMapSource);
	strPixMapSource.sprintf("%s/images/ftp_nottransferred.png", strApplicationPath.toLatin1().constData());
	clSysUtils.NormalizePath(strPixMapSource);
	m_pxNotTransferred = QPixmap(strPixMapSource);
	strPixMapSource.sprintf("%s/images/ftp_transfererror.png", strApplicationPath.toLatin1().constData());
	clSysUtils.NormalizePath(strPixMapSource);
	m_pxTransferError = QPixmap(strPixMapSource);
	strPixMapSource.sprintf("%s/images/ftp_transferring.png", strApplicationPath.toLatin1().constData());
	clSysUtils.NormalizePath(strPixMapSource);
	m_pxTransferring = QPixmap(strPixMapSource);
	m_bTransferDone = false;
	m_pclFtp = NULL;

	// Set dialog box with minimum details
	m_bDetailsVisible = false;
	buttonDetails->setText("&Details >>");
	checkBoxAutoClose->hide();
	treeWidgetFileTransfer->hide();
	spacerDetails->changeSize(16, 16, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	adjustSize();

	// Create timer
    m_pTimer = new QTimer(this);
    m_pTimer->setObjectName("transferTimer");
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    m_pTimer->setSingleShot(false);
    m_pTimer->start(100);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GexDbFtpTransferDialog::~GexDbFtpTransferDialog()
{
	DeleteFtpObject();
}

/////////////////////////////////////////////////////////////////////////////////////
// Start the ftp sequence
/////////////////////////////////////////////////////////////////////////////////////
bool GexDbFtpTransferDialog::Start()
{
	// Check if there are files to be transferred
	if(m_plistFilesToTransfer->count() == 0)
		return false;
	
	// Fill list view with files to transfer
	tdGexDbPluginDataFileListIterator lstIteratorDataFile(*m_plistFilesToTransfer);

	while(lstIteratorDataFile.hasNext())
	{
		m_pCurrentFile = lstIteratorDataFile.next();

		// Only consider remote files
		if (m_pCurrentFile && m_pCurrentFile->m_bRemoteFile)
		{
			QStringList lstRowData;
			lstRowData << m_pCurrentFile->m_strFileName << m_pCurrentFile->m_strFilePath << "Not transferred";
			m_pCurrentItemInTreeWidget = new QTreeWidgetItem(treeWidgetFileTransfer, lstRowData);
			m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxNotTransferred));
		}
	}

	// First file to transfer
	lstIteratorDataFile.toFront();
	m_pCurrentFile = NULL;

	GexDbPlugin_DataFile * pTmpDataFile	= NULL;

	while(lstIteratorDataFile.hasNext())
	{
		pTmpDataFile = lstIteratorDataFile.next();

		if (pTmpDataFile && !pTmpDataFile->m_bRemoteFile)
			m_pCurrentFile = pTmpDataFile;
	}

	if(!m_pCurrentFile)
		return false;

/* QT3 clean-up HTH case 3363
	m_pCurrentFile = m_plistFilesToTransfer->first();
	while(m_pCurrentFile)
	{
		// Only consider remote files
		if(m_pCurrentFile->m_bRemoteFile)
		{
			QStringList lstRowData;
			lstRowData << m_pCurrentFile->m_strFileName << m_pCurrentFile->m_strFilePath << "Not transferred";
			m_pCurrentItemInTreeWidget = new QTreeWidgetItem(treeWidgetFileTransfer, lstRowData);
			m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxNotTransferred));
		}

		m_pCurrentFile = m_plistFilesToTransfer->next();
	}

	// First file to transfer
	m_pCurrentFile = m_plistFilesToTransfer->first();
	while(m_pCurrentFile && !m_pCurrentFile->m_bRemoteFile)
		m_pCurrentFile = m_plistFilesToTransfer->next();
	if(!m_pCurrentFile)
		return false;
*/

	m_pCurrentItemInTreeWidget = treeWidgetFileTransfer->findItems(m_pCurrentFile->m_strFileName, Qt::MatchExactly | Qt::MatchCaseSensitive,0).first();
	m_pCurrentItemInTreeWidget->setText(2, "Transferring...");
	m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferring));
	textLabelFileInProgress->setText(m_pCurrentFile->m_strFileName);
	
	// Create FTP object and connect slots
	CreateFtpObject();

	// Create a timer to launch the first FTP sequence asap
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_connect()));
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));

	// Success
	return true;
}

/****************************************************************************
**
** Slots connected to signals of the QFtp class
**
*****************************************************************************/
void GexDbFtpTransferDialog::ftp_stateChanged(int state)
{
	QString strState;

	switch(state)
	{
        case QFtp::Unconnected:
			strState = "Unconnected";
			break;

        case QFtp::HostLookup:
			strState = "HostLookup";
			break;

        case QFtp::Connecting:
			strState = "Connecting";
			break;

		case QFtp::Connected:
			strState = "Connected";
			break;

		case QFtp::LoggedIn:
			strState = "LoggedIn";
			break;

		case QFtp::Closing:
			strState = "Closing";
			break;
	}
}

void GexDbFtpTransferDialog::ftp_commandStarted(int /*id*/)
{
	UpdateTransferStatus(iCommandStarted);
}

void GexDbFtpTransferDialog::ftp_commandFinished(int /*id*/, bool error)
{
	// Check for errors
    if(error) 
		UpdateTransferStatus(iCommandFinishedNOK);
	else
		UpdateTransferStatus(iCommandFinishedOK);
}

void GexDbFtpTransferDialog::ftp_done(bool error)
{
	QString strError;
	QRegExp	clRegExp;

	// Error during last sequence?
	if(error)
	{
		if(m_nCurrentSequence == GEXFTP_SEQUENCE_CLOSE)
		{
			// Not really a problem, file has already been transferred, process next file
			// Disconnect from current Ftp sequence
			disconnect(this, SIGNAL(sNextSequence()), 0, 0);
			// Connect next Ftp sequence
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_nextfile()));
		}
		else
		{
			// Update error code
			clRegExp.setPattern("[\n\r\t]");
			strError = m_pclFtp->errorString().replace(clRegExp, " ");
			GSET_ERROR1(GexDbFtpTransferDialog, eFtpError, NULL, strError.toLatin1().constData());

			// Update transfer status
			m_pCurrentFile->m_bTransferOK = false;
			m_pCurrentFile->m_strTransferError = strError;
			m_pCurrentItemInTreeWidget->setText(2, strError);
			m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferError));

			// Remove local file
			m_clCurrentFileHandle.close();
			m_clCurrentFileHandle.remove(m_strFileInTransferLocalName);

			// Close connection and process next file
			// Disconnect from current Ftp sequence
			disconnect(this, SIGNAL(sNextSequence()), 0, 0);
			// Connect next Ftp sequence
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_close()));
		}
	}
	else if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
	{
		// Close and rename local file
		m_clCurrentFileHandle.close();
		QDir clDir;
		clDir.rename(m_strFileInTransferLocalName, m_strFinalLocalName);

		// Update text of 'File in progress' label
		textLabelFileInProgress->setText("Done");

		// Update transfer status
		m_pCurrentFile->m_bTransferOK = true;
		m_pCurrentItemInTreeWidget->setText(2, "Transferred");
		m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferred));
	}

	// Reset progress bar
	progressTransfer->reset();
    QCoreApplication::processEvents();

	// Create a timer to launch the next FTP sequence asap
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Update transfer status
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::UpdateTransferStatus(int nInfoCode)
{
	QString strInfoMsg;

	switch(nInfoCode)
	{
		case iCommandStarted:
			switch(m_pclFtp->currentCommand())
			{
				case QFtp::ConnectToHost:
					strInfoMsg = "COMMAND connectToHost...";
					break;
				case QFtp::Login:
					strInfoMsg = "COMMAND login...";
					break;
				case QFtp::Close:
					strInfoMsg = "COMMAND close...";
					break;
				case QFtp::List:
					strInfoMsg = "COMMAND list...";
					break;
				case QFtp::Cd:
					strInfoMsg = "COMMAND cd...";
					break;
				case QFtp::Get:
					strInfoMsg.sprintf("COMMAND get (%s <- %s)...", m_strFileInTransferLocalName.toLatin1().constData(), m_strFileInTransferRemoteName.toLatin1().constData());
					break;
				case QFtp::Remove:
					strInfoMsg = "COMMAND remove...";
					break;
				case QFtp::Mkdir:
					strInfoMsg = "COMMAND mkdir...";
					break;
				case QFtp::Rmdir:
					strInfoMsg = "COMMAND rmdir...";
					break;
				case QFtp::Rename:
					strInfoMsg = "COMMAND rename...";
					break;
				case QFtp::RawCommand:
					strInfoMsg = "COMMAND rawCommand...";
					break;
				case QFtp::None:
				default:
					break;
			}
			break;
		case iCommandFinishedOK:
			strInfoMsg = m_pCurrentItemInTreeWidget->text(3) + " OK";
			break;
		case iCommandFinishedNOK:
			strInfoMsg = m_pCurrentItemInTreeWidget->text(3) + " NOK";
			break;
		default:
			return;
	}

	m_pCurrentItemInTreeWidget->setText(3, strInfoMsg);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CONNECT
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequence_connect()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_chdir()));

	// Connect to Ftp server
	m_nCurrentSequence = GEXFTP_SEQUENCE_CONNECT;
	ResolveHost();
	m_pclFtp->connectToHost(m_strFtpServer, m_pCurrentFile->m_uiPort);
	m_pclFtp->login(m_pCurrentFile->m_strUserName, m_pCurrentFile->m_strPassword);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CHDIR
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequence_chdir()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));

	// Change directory on server
	m_nCurrentSequence = GEXFTP_SEQUENCE_CHDIR;
	if(m_pCurrentFile->m_strFilePath.isEmpty())
		m_pclFtp->cd("/");
	else
		m_pclFtp->cd(m_pCurrentFile->m_strFilePath);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: GET
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequence_get()
{
	QString			strError;
	CGexSystemUtils	clSysUtils;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_close()));

	// Get file
	m_nCurrentSequence = GEXFTP_SEQUENCE_GET;

	m_strFinalLocalName = m_strLocalFtpDirectory;
	m_strFinalLocalName += "/";
	m_strFinalLocalName += m_pCurrentFile->m_strFileName;
	clSysUtils.NormalizePath(m_strFinalLocalName);
	m_strFileInTransferLocalName = m_strFinalLocalName + ".__tmp__";
	m_strFileInTransferRemoteName = m_pCurrentFile->m_strFileName.trimmed();

	// Try to remove local file if it exists
	if((QFile::exists(m_strFinalLocalName)) && (QFile::remove(m_strFinalLocalName) == false))
	{
		// Update error status
		GSET_ERROR1(GexDbFtpTransferDialog, eRemoveLocalFile, NULL, m_strFinalLocalName.toLatin1().constData());
		GetLastError(strError);
		m_pCurrentFile->m_bTransferOK = false;
		m_pCurrentFile->m_strTransferError = strError;
		m_pCurrentItemInTreeWidget->setText(2, strError);
		m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferError));
		
		// If logged in, create a timer to launch the next FTP sequence asap
		if(m_pclFtp->state() == QFtp::LoggedIn)
			QTimer::singleShot(0, this, SLOT(ftp_sequencer()));

		return;
	}

	// Open local file
    m_clCurrentFileHandle.setFileName(m_strFileInTransferLocalName);
    if(m_clCurrentFileHandle.open(QIODevice::WriteOnly) == false)
	{
		// Update error status
		GSET_ERROR1(GexDbFtpTransferDialog, eCreateLocalFile, NULL, m_strFinalLocalName.toLatin1().constData());
		GetLastError(strError);
		m_pCurrentFile->m_bTransferOK = false;
		m_pCurrentFile->m_strTransferError = strError;
		m_pCurrentItemInTreeWidget->setText(2, strError);
		m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferError));
		
		// If logged in, create a timer to launch the next FTP sequence asap
		if(m_pclFtp->state() == QFtp::LoggedIn)
			QTimer::singleShot(0, this, SLOT(ftp_sequencer()));

		return;
	}

	// Schedule FTP Get and command
	m_pclFtp->get(m_strFileInTransferRemoteName, &m_clCurrentFileHandle);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CLOSE
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequence_close()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_nextfile()));

	// Delete Ftp object (using close is an issue because sometimes the commandFinished signal is not emitted)
	DeleteFtpObject();
	m_nCurrentSequence = GEXFTP_SEQUENCE_CLOSE;

	// Create a timer to launch the next FTP sequence asap
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: NEXT
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequence_nextfile()
{
	// Create Ftp object
	CreateFtpObject();

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_connect()));

	// Next file to transfer
	tdGexDbPluginDataFileListIterator lstIteratorDataFile(*m_plistFilesToTransfer);
	GexDbPlugin_DataFile * pTmpDataFile	= NULL;

	m_nCurrentSequence	= GEXFTP_SEQUENCE_NEXTFILE;
	m_pCurrentFile		= NULL;

	while(lstIteratorDataFile.hasNext())
	{
		pTmpDataFile = lstIteratorDataFile.next();

		if (pTmpDataFile && !pTmpDataFile->m_bRemoteFile)
			m_pCurrentFile = pTmpDataFile;
	}

/* QT3 clean-up HTH case 3363
	// Next file to transfer
	m_nCurrentSequence = GEXFTP_SEQUENCE_NEXTFILE;
	m_pCurrentFile = m_plistFilesToTransfer->next();
	while(m_pCurrentFile && !m_pCurrentFile->m_bRemoteFile)
		m_pCurrentFile = m_plistFilesToTransfer->next();
*/

	if(!m_pCurrentFile)
	{
		// All transfer finished
		Cleanup();
		return;
	}
	
	m_pCurrentItemInTreeWidget = treeWidgetFileTransfer->findItems(m_pCurrentFile->m_strFileName, Qt::MatchExactly | Qt::MatchCaseSensitive, 0).first();
	m_pCurrentItemInTreeWidget->setText(2, "Transferring...");
	m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferring));
	textLabelFileInProgress->setText(m_pCurrentFile->m_strFileName);

	// Create a timer to launch the next FTP sequence asap
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Create Ftp object
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::CreateFtpObject()
{
	// Make sure Ftp object is deleted
	DeleteFtpObject();

	// Create FTP object and connect slots
	m_pclFtp = new QFtp(this);
	connect(m_pclFtp, SIGNAL(commandStarted(int)),
		SLOT(ftp_commandStarted(int)));
	connect(m_pclFtp, SIGNAL(commandFinished(int,bool)),
		SLOT(ftp_commandFinished(int,bool)));
	connect(m_pclFtp, SIGNAL(done(bool)),
		SLOT(ftp_done(bool)));
	connect(m_pclFtp, SIGNAL(dataTransferProgress(qint64,qint64)),
		this, SLOT(OnTransferProgress(qint64,qint64)) );
	connect(m_pclFtp, SIGNAL(stateChanged(int)),
		SLOT(ftp_stateChanged(int)));
}

/////////////////////////////////////////////////////////////////////////////////////
// Delete Ftp object
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::DeleteFtpObject()
{
	if(m_pclFtp)
	{
		disconnect(m_pclFtp, 0, 0, 0);
		if(m_pclFtp->state() != QFtp::Unconnected)
		{
			m_pclFtp->abort();
			m_pclFtp->close();
		}
		m_pclFtp->deleteLater();
		m_pclFtp = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence control
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ftp_sequencer()
{
	emit sNextSequence();
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Details' button has been pressed to expand/reduce the list of fields
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::OnButtonDetails()
{
	if(m_bDetailsVisible == true)
	{
		// Hide widgets with details
		m_bDetailsVisible = false;
		checkBoxAutoClose->hide();
		treeWidgetFileTransfer->hide();
		spacerDetails->changeSize(16, 16, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		adjustSize();
		
		// Change button label
		buttonDetails->setText("&Details >>");
	}
	else
	{
		// Show widgets with details
		m_bDetailsVisible = true;
		checkBoxAutoClose->show();
		treeWidgetFileTransfer->show();
		spacerDetails->changeSize(16, 16, QSizePolicy::Fixed, QSizePolicy::Fixed);
		adjustSize();

		// Change button label
		buttonDetails->setText("<< &Details");
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Abort' button has been pressed
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::OnButtonAbort()
{
	// If transfer completed, just close the dialog
	if(m_bTransferDone)
		accept();

	// Disconnect all signals of the Ftp object, abort operations, and delete object
	DeleteFtpObject();
	
	// Update error status
	QString strError;
	GSET_ERROR0(GexDbFtpTransferDialog, eOperationAborted, NULL);

	// Check if a transfer was in progress
	if(m_pCurrentFile)
	{
		// Update transfer status
		GetLastError(strError);
		m_pCurrentFile->m_bTransferOK = false;
		m_pCurrentFile->m_strTransferError = strError;
		m_pCurrentItemInTreeWidget->setText(2, strError);
		m_pCurrentItemInTreeWidget->setIcon(2, QIcon(m_pxTransferError));

		// Remove local file
		m_clCurrentFileHandle.close();
		QFile::remove(m_strFileInTransferLocalName);
	}

	// Call cleanup
	Cleanup();
}

/////////////////////////////////////////////////////////////////////////////////////
// Cleanup
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::Cleanup()
{
	// Reset current pointers
	m_pCurrentFile = NULL;
	m_pCurrentItemInTreeWidget = NULL;

	// Disconnect all signals of the Ftp object!!!!!!!
	disconnect(m_pclFtp, 0, 0, 0);

	// Transfer is completed, exit dialog on next timer tick
    m_bTransferDone = true;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Reset progress bar
	progressTransfer->reset();
    QCoreApplication::processEvents();
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to resolve host name domain<->IP
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::ResolveHost()
{
	// Make sure server name is in IP format
	m_strFtpSiteName = "???";
	m_strFtpSiteIP = "???";
	m_strFtpServer = m_pCurrentFile->m_strHostName;

	QHostAddress clHostAddr;
	if(clHostAddr.setAddress(m_strFtpServer) == false)
	{
		// The server name is in domain name format. Try to retrieve the IP address.
		m_strFtpSiteName = m_strFtpServer;
		hostent* host = 0;
		host = gethostbyname(m_strFtpServer.toLatin1().constData());
		if(host)
		{
			unsigned long ul_HostAddress;
			memcpy(&(ul_HostAddress), host->h_addr, host->h_length);
			ul_HostAddress = htonl(ul_HostAddress);
            clHostAddr.setAddress((quint32)ul_HostAddress);
			m_strFtpSiteIP = clHostAddr.toString();
			m_strFtpServer = m_strFtpSiteIP;
		}
	}
	else
	{
		// The server name is in IP address format. Try to retrieve the server name.
		m_strFtpSiteIP = m_strFtpServer;
		unsigned long ul_HostAddress = (unsigned long)clHostAddr.toIPv4Address();
		ul_HostAddress = ntohl(ul_HostAddress);
		char *pHostAddress = (char *)&ul_HostAddress;
		struct hostent *host = 0;
		host = gethostbyaddr(pHostAddress, 4, AF_INET);
		if(host)
			m_strFtpSiteName = host->h_name;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Called during Ftp transfer to notify transfer progress
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::OnTransferProgress(qint64 nProgress, qint64 nTotalSteps)
{
	// Update progress bar
	if ((int)nTotalSteps != progressTransfer->maximum())
		progressTransfer->setMaximum((int)nTotalSteps);
   
	progressTransfer->setValue((int)nProgress);

	// Process events
    QCoreApplication::processEvents();
}

/////////////////////////////////////////////////////////////////////////////////////
// Timer function
/////////////////////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::OnTimer()
{
	// Check if the transfer is completed
	if(m_bTransferDone)
	{
		// Stop this timer
		disconnect(m_pTimer, 0, 0, 0);
		m_pTimer->deleteLater();
			
		// Check if dialog box should be closed automatically
		if(checkBoxAutoClose->isChecked())
			accept();

        // Change 'Abort' button to 'OK' button
        buttonAbort->setText("OK");
	}
}

//////////////////////////////////////////////////////////////////////
// Returns details about last error
//////////////////////////////////////////////////////////////////////
void GexDbFtpTransferDialog::GetLastError(QString & strError)
{
	strError = GGET_LASTERRORMSG(GexDbFtpTransferDialog,this);
}
