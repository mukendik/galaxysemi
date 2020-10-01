#ifdef _WIN32
    #include <windows.h>
#endif

#include <QtGui>
#include <QtNetwork>

#include <gqtl_sysutils.h>

#include "vfei_simulator.h"
#include "licensestatus.h"


///////////////////////////////////////////////////////////
// Constructor.
///////////////////////////////////////////////////////////
LicenseStatusDialog::LicenseStatusDialog(const QString & strUserHome, const QString & strAppName, const QString & strOutputStatusFile, QWidget* parent, Qt::WFlags f) 
	: QDialog( parent, f )
{
	// Setup UI
	setupUi(this);
	
	// Support for Drag&Drop
	setAcceptDrops(true);

	// set the Gex default palette
	m_gexSkin.applyPalette(this);

	// Initializes variables...
	m_strUserHome = strUserHome;
	m_strAppName = strAppName;
	m_strOutputStatusFile = strOutputStatusFile;
	socket = NULL;
	iSocketStatus = GEX_CLIENT_SOCKET_IDLE;

	
	// Timer running to check completion of Scoket based tasks...
	connect( &timerSocket, SIGNAL(timeout()),this, SLOT(OnTimerEvent(void)) );

	// Updates Windows title to current software version string
	setWindowTitle(m_strAppName);

	// Set the default page to the text browser 
	QString strAppDirectory;
	CGexSystemUtils::GetApplicationDirectory(strAppDirectory);
	m_strDefaultReport = strAppDirectory + "\\html\\pages\\_gexls_report_default.htm";
	CGexSystemUtils::NormalizePath(m_strDefaultReport);

	// Read valid Socket# and server name from configuration file!
	m_strConfigFile = m_strUserHome + GEXLS_CONFIG_FILE;
	CGexSystemUtils::NormalizePath(m_strConfigFile);

	// Build path to configuration file
    QFile file(m_strConfigFile); // Read the config file
    if (file.open(QIODevice::ReadOnly) == false)
		return;

	QTextStream hFile;
	hFile.setDevice(&file);	// Assign file handle to data stream

	// Check if valid header...or empty!
	QString strString = hFile.readLine();
	if(strString != "<gexls_config>")
		return;

	do
	{
		// Read one line from file
		strString = hFile.readLine();

		if(strString.startsWith("server=") == true)
		{
			m_strServerName = strString.section('=',1);
			LineEditServerName->setText(m_strServerName);
		}

		if(strString.startsWith("socket=") == true)
		{
			m_strSocketPort = strString.section('=',1);
			LineEditSocketPort->setText(m_strSocketPort);
		}

	}
	while(hFile.atEnd() == false);
	file.close();

	// Update GUI menus/Objects
	UpdateGuiButtons(FALSE);

	// If option to output license status to file, trigger server connection
	if(m_strOutputStatusFile.isEmpty() == false)
		on_buttonConnect_clicked();
}

///////////////////////////////////////////////////////////
// Drag & Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void LicenseStatusDialog::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/plain"))
		event->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void LicenseStatusDialog::dropEvent(QDropEvent *e)
{
	if(!e->provides("text/uri-list"))
	{
		// No files dropped...ignore drag&drop.
		e->ignore();
		return;
	}

	QString		strFileName;
	QStringList strFileList;
	QList<QUrl> lstUrls = e->mimeData()->urls();

	for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
	{
		strFileName = lstUrls.at(nUrl).toLocalFile();

		if (!strFileName.isEmpty())
			strFileList << strFileName;
	}

	if(strFileList.count() <= 0)
	{
		// Items dropped are not regular files...ignore.
		e->ignore();
		return;
	}

	// Get first file dropped
	QString strFile = strFileList.first();
	StartRunScript(strFile);


	e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Timer-based event.
///////////////////////////////////////////////////////////
void LicenseStatusDialog::OnTimerEvent()
{
	// If VFEI Script to run, check if right-time to do so
	if(m_bIdle && m_hScriptFile.device() && (m_hScriptFile.status() == QTextStream::Ok))
	{
		// Get one line from VFEI script, then run it
		QString strMessage = m_hScriptFile.readLine();
		strMessage = strMessage.trimmed();	// Reove all begin/ending whitespaces

		QTextStream os(socket);

		// Send command line server
		if(strMessage.startsWith("CMD/A=",Qt::CaseInsensitive))
		{
			// Server has sent a message...process it.
			// Read line, then parse it.
			QString strString = "[";
			strString += QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
			strString += "]:\n" + strMessage + "\n";

			textEdit->append(strString);


			os << strMessage << "\n";
			m_bIdle = false;	// Do not send next line until reply received!
		}
		else
		if(strMessage.startsWith("WAIT_END",Qt::CaseInsensitive))
			m_bIdle = false;	// Force to wait unril next string result received from PAT-Man!
		else
		if(strMessage.startsWith("REWIND",Qt::CaseInsensitive))
			m_hScriptFile.seek(0);	// Force to Rewind
	}
	else
	if((m_hScriptFile.status() == QTextStream::ReadPastEnd) && QFile::exists(m_fScriptFile.fileName()) )
	{
		// If end of script reached, notify user!
		LineEditScriptFile->setText("* Script Finished *");
		m_fScriptFile.setFileName("");
	}
}

///////////////////////////////////////////////////////////
// Update GUI menus/Objects
///////////////////////////////////////////////////////////
void LicenseStatusDialog::UpdateGuiButtons(bool bConnected)
{
	// Enable/Disable 'Connect' button + Server info on GUI
	buttonConnect->setEnabled(!bConnected);
	buttonDisconnect->setEnabled(bConnected);
	LineEditServerName->setEnabled(!bConnected);
	LineEditSocketPort->setEnabled(!bConnected);

	buttonSendCMD->setEnabled(bConnected);
	buttonClearServer->setEnabled(bConnected);
	textEdit->setEnabled(bConnected);
	GroupBoxRunScript->setEnabled(bConnected);

	// Clear GUI lists if just connecting
	if(bConnected)
	{
	    textEdit->clear();
	    textEditServer->clear();
	}
}

///////////////////////////////////////////////////////////
// Client/Server running mode: Server is present!
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketConnected()
{
	// Success: connected to GEX-LM server!
	iSocketStatus = GEX_CLIENT_SOCKET_IDLE;


	// Now starts refresh polling loop
	dtSocketEvent = QDateTime::currentDateTime();
	dtSocketEvent = dtSocketEvent.addDays(-1);	// Will force an immediate update of the display.
	iSocketStatus = GEX_CLIENT_CONNECTED;

	// Save valid Socket# and server name to configuration file!
    QFile file(m_strConfigFile); // Write the text to the file
    if (file.open(QIODevice::WriteOnly) == true)
	{
		QTextStream hFile;

		hFile.setDevice(&file);	// Assign file handle to data stream

		hFile << "<gexls_config>" << endl;
		hFile << "server=" << m_strServerName << endl;
		hFile << "socket=" << m_strSocketPort << endl;
		file.close();
	}

	// Update GUI menus/Objects
	UpdateGuiButtons(TRUE);
}

///////////////////////////////////////////////////////////
// Close socket connection + reset GUI
///////////////////////////////////////////////////////////
void LicenseStatusDialog::OnCloseConnection()
{
	// Stop timer.
	timerSocket.stop();

	// Disconnect socket signale, and delete socket
	disconnect((QObject *)socket, 0, 0, 0);
	if(socket != NULL)
	{
		socket->deleteLater();			// Rather than 'delete socket': avoids crash in QT dll
		socket = NULL;
	}

	// Update GUI menus/Objects
	UpdateGuiButtons(FALSE);

	// If output license status to file, exit on error.
	if(m_strOutputStatusFile.isEmpty() == false)
		exit(EXIT_FAILURE);

	// Stop running Script if any!
	m_hScriptFile.setStatus(QTextStream::ReadPastEnd);
}

///////////////////////////////////////////////////////////
// Client/Server running mode: Server has closed the connection
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketConnectionClosed()
{
	QMessageBox::information(this,m_strAppName,"Server closed the connection!\nEither application on server was closed\nor network connection failure.\n");
	OnCloseConnection();
}

///////////////////////////////////////////////////////////
// Client/Server running mode: 
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketReadyRead()
{
	QString			strString1,strString2,strString3;
	QString			strServerMessage;

	// Buffer used to collect status lines (in case output to file is enabled)
	QString strOutputStatusBuffer;
	if(m_strOutputStatusFile.isEmpty() == false)
		strOutputStatusBuffer = m_strAppName + QString("\nUser,Computer,Session time (h:m:s)\n----,--------,---------------------------\n");
	
	// Server has sent a message...process it.
	// Read line, then parse it.
	strServerMessage = "[";
	strServerMessage += QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
	strServerMessage += "]:\n" + ReadLineFromSocket();

	textEditServer->append(strServerMessage);

	// Add CR+LF
	textEditServer->append(QString("\n"));

	// If script to run, then can send next line.
	m_bIdle = true;
}
 
///////////////////////////////////////////////////////////
// Client/Server running mode: 
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketError(QAbstractSocket::SocketError socketError)
{
	// Error code may be one of:
	// QSocket::ErrConnectionRefused - if the connection was refused 
	// QSocket::ErrHostNotFound - if the host was not found 
	// QSocket::ErrSocketRead - if a read from the socket failed 
	// QMessageBox::information(this,m_strAppName,"socket error!");
	// OnCloseConnection();

	int nTest = 0;
	nTest++;
}

///////////////////////////////////////////////////////////
// Read + decrypt string received from client
///////////////////////////////////////////////////////////
QString LicenseStatusDialog::ReadLineFromSocket(void)
{
	QString strMessage = socket->readLine();
	strMessage = strMessage.trimmed();		// Remove leading '\n'

	return strMessage;
}

///////////////////////////////////////////////////////////
// Send CMD line
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonSendCMD_clicked()
{
	QString strMessage = textEdit->toPlainText();
	QTextStream os(socket);

	// Send command line server
	os << strMessage << "\n";
}


///////////////////////////////////////////////////////////
// Clear reception text GUI
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonClearServer_clicked()
{
	textEditServer->clear();
}

///////////////////////////////////////////////////////////
// User wants to connect to a given server...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonConnect_clicked()
{
	// create the socket and connect various of its signals
	if(socket != NULL)
	{
		socket->deleteLater();
		socket = NULL;
	}

	// Status: // GEX trying to connect to server
	dtSocketEvent = QDateTime::currentDateTime();
	iSocketStatus = GEX_CLIENT_TRY_CONNECT;

	socket = new QTcpSocket(this);
	connect( (QObject *)socket, SIGNAL(connected()),this,
		SLOT(socketConnected()) );
	connect( (QObject *)socket, SIGNAL(connectionClosed()),this,
		SLOT(socketConnectionClosed()) );
	connect( (QObject *)socket, SIGNAL(readyRead()),this,
		SLOT(socketReadyRead()) );
	connect( (QObject *)socket, SIGNAL(error(QAbstractSocket::SocketError)),this,
		SLOT(socketError( QAbstractSocket::SocketError)) );

	// connect to the server
	QString strString;
	int		iSocketPort;			// Socket port # (if running in client/server mode)

	m_strSocketPort = LineEditSocketPort->text();	// Socket port#
	iSocketPort = m_strSocketPort.toInt();
	m_strServerName = LineEditServerName->text();	// Server name

	// Socket connection request.
	socket->connectToHost(m_strServerName, iSocketPort);

	// Start timer: 300ms sec.
	timerSocket.start(300);

	// System is Ready (eg: ready to run Script file)
	m_bIdle = true;
}

///////////////////////////////////////////////////////////
// User wants to disconnect from a given server...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonDisconnect_clicked()
{
	OnCloseConnection();
}

///////////////////////////////////////////////////////////
// Select script to RUN
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonRunScript_clicked()
{
	// Browse disk to find FVEI Script file
	QString strScriptFile;
	strScriptFile = QFileDialog::getOpenFileName(this,"Select VFEI script","","*.txt");
	if(QFile::exists(strScriptFile) == false)
		return;

	// Run script file
	StartRunScript(strScriptFile);
}

///////////////////////////////////////////////////////////
// Run Script file.
///////////////////////////////////////////////////////////
void LicenseStatusDialog::StartRunScript(QString strScriptFile)
{
	// Update file name on GUI
	LineEditScriptFile->setText(strScriptFile);

	// Open file
	m_fScriptFile.setFileName(strScriptFile);
	if(!m_fScriptFile.open( IO_ReadOnly ))
	{
		// Failed Opening Script file
		m_fScriptFile.setFileName("");
		QMessageBox::information(this,"Error","Failed opening VFEI Script file");
		return;
	}

	// Assign file I/O stream
	m_hScriptFile.setDevice(&m_fScriptFile);
}


