#ifdef _WIN32
    #include <windows.h>
#endif

#include <QtGui>
#include <QtNetwork>

#include <gqtl_sysutils.h>
#include <QMessageBox>

#include "gex-ls.h"
#include "gex_shared.h"
#include "licensestatus.h"
#include "cryptofile.h"
#include "htmlreport.h"


// Used for encrypting data.
#define GEX_CRYPTO_KEY		"(gex@galaxysemi.com)"
#define GEX_KEY_BYTES		12

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
LicenseStatusDialog::LicenseStatusDialog(const QString & strUserHome, const QString & strAppName, const QString & strOutputStatusFile, QWidget* parent)
        : QDialog( parent)
{
	// Setup UI
	setupUi(this);

        // Setup the application buttons
        setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

	// set the Gex default palette
	m_gexSkin.applyPalette(this);

	// Initializes variables...
	m_strUserHome = strUserHome;
	m_strAppName = strAppName;
	m_strOutputStatusFile = strOutputStatusFile;
	socket = NULL;
	iSocketStatus = GEX_CLIENT_SOCKET_IDLE;

	// Make sure status tab widget is on 'Live' tab
	tabWidgetStatus->setCurrentIndex(0);
	
	// Initialize the 2 tree views
	QStringList headers;
	
	TreeWidgetClientsList->setColumnCount(3);
	headers << tr("User") << tr("Computer") << tr("Session time (h:m:s)") << tr("Version") << tr("Session Id");
	TreeWidgetClientsList->setHeaderLabels(headers);
	TreeWidgetClientsList->setSortingEnabled(false);
	TreeWidgetClientsList->setDragEnabled(false);
	TreeWidgetClientsList->setRootIsDecorated(false);
	TreeWidgetClientsList->setItemsExpandable(false);

	headers.clear();
	TreeWidgetLicenseDetails->setColumnCount(2);
	headers << tr("Field") << tr("Value");
	TreeWidgetLicenseDetails->setHeaderLabels(headers);
	TreeWidgetLicenseDetails->setSortingEnabled(false);
	TreeWidgetLicenseDetails->setDragEnabled(false);
	TreeWidgetLicenseDetails->setRootIsDecorated(false);
	TreeWidgetLicenseDetails->setItemsExpandable(false);

	headers.clear();
	TreeWidgetHistory->setColumnCount(4);
	headers << tr("Event") << tr("Date") << tr("Time") << tr("Computer") << tr("User") << tr("Total in use") << tr("Total allowed") << tr("Message");
	TreeWidgetHistory->setHeaderLabels(headers);
	TreeWidgetHistory->setSortingEnabled(false);
	TreeWidgetHistory->setDragEnabled(false);
	TreeWidgetHistory->setRootIsDecorated(false);
	TreeWidgetHistory->setItemsExpandable(false);

	// Initialize the DateTime control in history tab
	dateEditBegin->setDate(QDate(2007, 12, 31));
	dateEditEnd->setDate(QDate::currentDate());

	// Timer running to check completion of Scoket based tasks...
	connect( &timerSocket, SIGNAL(timeout()),this, SLOT(OnTimerEvent(void)) );

	// Button used to request history connection to the server
	connect(buttonFindHistory, SIGNAL(clicked()), this, SLOT(RequestLicenseManagerHistory(void)));
	
	// catch the signal when source changes in the browser
	connect(textBrowserStatistic, SIGNAL(sourceChanged(const QUrl&)), this, SLOT(onSourceChanged(const QUrl&)));

	// Updates Windows title to current software version string
	setWindowTitle(m_strAppName);

	// Set the default page to the text browser 
	QString strAppDirectory;
	CGexSystemUtils::GetApplicationDirectory(strAppDirectory);
	m_strDefaultReport = strAppDirectory + "\\html\\pages\\_gexls_report_default.htm";
	CGexSystemUtils::NormalizePath(m_strDefaultReport);

	textBrowserStatistic->setSource(QUrl::fromLocalFile(m_strDefaultReport));
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

		if(strString.startsWith("FilterDateFrom=") == true)
			dateEditBegin->setDate(QDate::fromString(strString.section('=',1), Qt::ISODate));

		if(strString.startsWith("FilterDateTo=") == true)
			dateEditEnd->setDate(QDate::fromString(strString.section('=',1), Qt::ISODate));
	}
	while(hFile.atEnd() == false);
	file.close();

	// Update GUI menus/Objects
	UpdateGuiButtons(false);

	// Create the report directory and add the .png file useful for html report
	createReportDir();

	// initialize the event manager with the statistic manager
	m_EventManager.setStatisticManager(&m_statisticManager);

	// If option to output license status to file, trigger server connection
	if(m_strOutputStatusFile.isEmpty() == false)
		on_buttonConnect_clicked();
}

///////////////////////////////////////////////////////////
// Updates progress bar status...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::UpdateProcessBar(QString strMessage,int iTotalSteps,int iStep)
{
	TextLabelStatus->setText(strMessage);
	if(iStep < 0)
		ProgressBar->hide();
	else
	{
		ProgressBar->show();
		ProgressBar->setMinimum(0);
		ProgressBar->setMaximum(iTotalSteps);
		ProgressBar->setValue(iStep);
	}
}

///////////////////////////////////////////////////////////
// Timer callback: used to check Socket tasks completion...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::OnTimerEvent()
{
	// Check if Client/Server currently waiting for a connection to complete...
	QString		strString;
	QDateTime	dtNow;
	int			iSec;

	// Check socket action status (if any). 
	switch(iSocketStatus)
	{
		case GEX_CLIENT_TRY_CONNECT: // We are waiting for connection to server to complete...
			dtNow = QDateTime::currentDateTime();
			iSec = dtSocketEvent.secsTo(dtNow);
			UpdateProcessBar("Connecting to server...",GEX_CLIENT_TRY_CONNECT_TIMEOUT,iSec);	// Show process bar...step 1 of 30 
			if(iSec > GEX_CLIENT_TRY_CONNECT_TIMEOUT)
			{
				// Reset status...avoids multiple callbacks to this function until user clicks 'OK'!
				iSocketStatus = GEX_CLIENT_SOCKET_IDLE;

				// Connection failed as no reply from server arrived...abort!
				QMessageBox::information(this,m_strAppName,"Timeout: failed connecting to server.\nServer name/IP is probably incorrect\n");

				// Update status
				UpdateProcessBar("Failed connecting to server",GEX_CLIENT_TRY_CONNECT_TIMEOUT,-1);

				// Abort this socket!.
				OnCloseConnection();
			}
			break;
		case GEX_CLIENT_CONNECTED:	// Socket connected, refersh every Xs econds the clients list.
			dtNow = QDateTime::currentDateTime();
			iSec = dtSocketEvent.secsTo(dtNow);
			strString = "Next update in: ";
			strString += QString::number(GEX_CLIENT_REFRESH-iSec);
			strString += " sec...";
			if(iSec > GEX_CLIENT_REFRESH)
			{
				// Reset event timer.
				dtSocketEvent = QDateTime::currentDateTime();

				// Request GEX-LM status
				RequestLicenseManagerStatus();
			}
			else
				UpdateProcessBar(strString,GEX_CLIENT_REFRESH,-1);	// Tells when is next Refresh...
			break;

		case GEX_CLIENT_SOCKET_IDLE:	// Not waiting for any client/server exchange.
		default:
			break;
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
	groupBoxHistoryParameter->setEnabled(bConnected);
	tabWidgetHistory->setEnabled(bConnected);
		
	// Update status line
	UpdateProcessBar("Ready", 0, -1);
}

///////////////////////////////////////////////////////////
// Client/Server running mode: Server is present!
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketConnected()
{
	// Success: connected to GEX-LM server!
	iSocketStatus = GEX_CLIENT_SOCKET_IDLE;

	// Update status
	UpdateProcessBar("Success connecting to server",GEX_CLIENT_TRY_CONNECT_TIMEOUT,-1);

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
		hFile << "FilterDateFrom=" << dateEditBegin->date().toString(Qt::ISODate) << endl;
		hFile << "FilterDateTo=" << dateEditEnd->date().toString(Qt::ISODate);
		file.close();
	}

	// Update GUI menus/Objects
	UpdateGuiButtons(true);
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

	// Emtpy the list boxes
	TreeWidgetLicenseDetails->clear();
	TreeWidgetClientsList->clear();
	TreeWidgetHistory->clear();

	m_EventManager.clean();
	m_statisticManager.clear();

	// reload the default html pages in the browser
	textBrowserStatistic->setSource(QUrl::fromLocalFile(m_strDefaultReport));

	// Update GUI menus/Objects
	UpdateGuiButtons(false);

	// If output license status to file, exit on error.
	if(m_strOutputStatusFile.isEmpty() == false)
		exit(-1);
}

///////////////////////////////////////////////////////////
// Client/Server running mode: Server has closed the connection
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketDisconnected()
{
	QMessageBox::information(this,m_strAppName,"Server closed the connection!\nEither 'gexlm' application on server was closed\nor network connection failure.\n");
	OnCloseConnection();
}

///////////////////////////////////////////////////////////
// Client/Server running mode: 
///////////////////////////////////////////////////////////
void LicenseStatusDialog::socketReadyRead()
{
	QString			strString1,strString2,strString3,strString4,strString5;
	QTreeWidgetItem *pItem;
	QString			strServerMessage;

	// Buffer used to collect status lines (in case output to file is enabled)
	QString strOutputStatusBuffer;
	if(m_strOutputStatusFile.isEmpty() == false)
		strOutputStatusBuffer = m_strAppName + QString("\nUser,Computer,Session time (h:m:s),Version\n----,--------,---------------------------,--------------------------------------------------\n");
	
	// Server has sent a message...process it.
	while(socket->canReadLine()) 
	{
		// Read line, then parse it.
		strServerMessage = ReadLineFromSocket();

		if(strServerMessage.startsWith("GEX_NODESTATUS") == true)
		{
			// This line includes a node status string...parse it.
			strString1 = strServerMessage.section(';',1,1);		// User name
			strString2 = strServerMessage.section(';',2,2);		// Computer name
			strString3	= strServerMessage.section(';',3,3);	// Session duration
			strString4	= strServerMessage.section(';',4,4);	// GEX version (not provided with older GEX-LM)
			strString5	= strServerMessage.section(';',5,5);	// Node Id
			if(strString4.isEmpty())
				strString4 = "??";

			// Insert node description in the list...
			pItem = new QTreeWidgetItem(TreeWidgetClientsList);
			pItem->setText(0, strString1);
			pItem->setText(1, strString2);
			pItem->setText(2, strString3);
			pItem->setText(3, strString4);
			pItem->setText(4, strString5);

			// Check if output info to file enabled...
			if(m_strOutputStatusFile.isEmpty() == false)
				strOutputStatusBuffer += strString1 + "," + strString2 + "," + strString3 + "," + strString4 + "\n";
		}
		if(strServerMessage.startsWith("GEX_NETWORKSTATUS") == true)
		{
			// This line includes a network status summary string...
			strString1 = strServerMessage.section(';',1,1);		// Licenses used
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Total in use");
			pItem->setText(1, strString1);
			
			strString1 = strServerMessage.section(';',2,2);		// Max. Licenses allowed.
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Total allowed");
			pItem->setText(1, strString1);
			
			strString1 = strServerMessage.section(';',3,3);		// Running Mode.0=Examinator,4=ExaminatorDB,...
			if(strString1.isEmpty() == true)
				strString1 = QString::number(GEX_DATATYPE_ALLOWED_ANY);	// Examinator
			switch(strString1.toInt())
			{
				case GEX_DATATYPE_ALLOWED_ANY:		// Examinator: Any tester type is supported
					strString1 = "Examinator";
					break;
				case GEX_DATATYPE_ALLOWED_CREDENCE:	// Examinator: Only Credence data files allowed
					strString1 = "Examinator for Credence";
					break;
				case GEX_DATATYPE_ALLOWED_SZ:		// Examinator: Only SZ data files allowed
					strString1 = "Examinator for SZ";
					break;
				case GEX_DATATYPE_ALLOWED_TERADYNE:	// Examinator: Only Teradyne data files allowed
					strString1 = "Examinator for Teradyne";
					break;
				case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
					strString1 = "Examinator Pro";
					break;
			}
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Product name");
			pItem->setText(1, strString1);
		}
		if(strServerMessage.startsWith("GEX_LICENSEINFO") == true)
		{
			// This line includes the license info string...
			strString1 = strServerMessage.section(';',1,1);		// GEX-LM Name+Version#
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Server running");
			pItem->setText(1, strString1);
			
			strString1 = strServerMessage.section(';',2,2);		// Expiration date
			strString1 += " (Year month day)";
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Expiration date");
			pItem->setText(1, strString1);

			strString1 = strServerMessage.section(';',3,3);		// License KeyID
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "Product keyID");
			pItem->setText(1, strString1);

			// List email/web links to Galaxy
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "> Galaxy support");
			pItem->setText(1, GEX_EMAIL_SUPPORT);
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "> Sales");
			pItem->setText(1, GEX_EMAIL_SALES);
			pItem = new QTreeWidgetItem(TreeWidgetLicenseDetails);
			pItem->setText(0, "> Web");
			pItem->setText(1, "www.mentor.com");
		}

		if(strServerMessage.startsWith("GEX_HISTORYDATA") == true)
		{
			QStringList	strFieldList;

			// Parse the history 
			strString1 = strServerMessage.section(';',1);
			const CEventLM * pEventLM = m_EventManager.pushEvent(strString1);
			
			if (pEventLM)
				pEventLM->rawData(strFieldList);
			else
				strFieldList << "g_error";

			pItem = new QTreeWidgetItem(TreeWidgetHistory, strFieldList);
		}

		if(strServerMessage.startsWith("GEX_HISTORYSTART") == true)
		{
			// Get the total number of event that GEX-LM will send us
			strString1 = strServerMessage.section(';',1);
		}

		if(strServerMessage.startsWith("GEX_HISTORYSTOP") == true)
		{
			m_EventManager.pushEndEvent();

			// Create the report file
			CHtmlReport htmlReport(&m_gexSkin);
			if (htmlReport.createReport(m_statisticManager))
			{
				textBrowserStatistic->setSource(QUrl::fromLocalFile(htmlReport.normalizedFilePath()));
				textBrowserStatistic->reload();
			}
		}
		
		TreeWidgetClientsList->sortItems(0, Qt::AscendingOrder);
		TreeWidgetLicenseDetails->sortItems(0, Qt::AscendingOrder);
	};

	// Check if info to dump to status file
	if(strOutputStatusBuffer.isEmpty() == false)
	{
		QFile cFile(m_strOutputStatusFile);
		if(!cFile.open(QIODevice::WriteOnly | QIODevice::Text))
			return;	// Failed to create file.
		QTextStream hStatusFile(&cFile);

		hStatusFile << strOutputStatusBuffer << endl;
		cFile.close();

		// Status file created, exit.
		exit(1);
	}
}
 
///////////////////////////////////////////////////////////
// Client/Server running mode: 
///////////////////////////////////////////////////////////
void
LicenseStatusDialog::socketError(QAbstractSocket::SocketError /*socketError*/)
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
// Request server to sent its current status + nodes in use
///////////////////////////////////////////////////////////
void LicenseStatusDialog::RequestLicenseManagerStatus()
{
	CCryptoFile ascii;	// Buffer used for crypting data!
	QTextStream os(socket);
	char*		szCryptedString;

	// Command string to send to server : GEX_GetUsers
	QString		strQueryServer = "GEX_GetUsers";
	QByteArray	pString = strQueryServer.toLatin1();

	ascii.SetBufferFromBuffer(pString.data(), (strQueryServer.length()+1)*sizeof(char));
	ascii.Encrypt_Buffer((BYTE*)GEX_CRYPTO_KEY, GEX_KEY_BYTES);
	ascii.GetBufferToHexa(&szCryptedString);

	// Send command line server
	os << szCryptedString << "\n";
    free(szCryptedString);
	// Emtpy the list boxes
	TreeWidgetLicenseDetails->clear();
	TreeWidgetClientsList->clear();
}

///////////////////////////////////////////////////////////
// Request server to sent its history
///////////////////////////////////////////////////////////
void LicenseStatusDialog::RequestLicenseManagerHistory()
{
	CCryptoFile ascii;	// Buffer used for crypting data!
	QTextStream os(socket);
	char*		szCryptedString;

	// Command string to send to server : GEX_GetUsers + filters
	QString		strQueryServer = "GEX_GetHistory;";
	strQueryServer += dateEditBegin->date().toString(Qt::ISODate);
	strQueryServer += ";";
	strQueryServer += dateEditEnd->date().toString(Qt::ISODate);
	strQueryServer += ";";

	QByteArray	pString = strQueryServer.toLatin1();

	ascii.SetBufferFromBuffer(pString.data(), (strQueryServer.length()+1)*sizeof(char));
	ascii.Encrypt_Buffer((BYTE*)GEX_CRYPTO_KEY, GEX_KEY_BYTES);
	ascii.GetBufferToHexa(&szCryptedString);

	// Send command line server
	os << szCryptedString << "\n";
    free(szCryptedString);

	// Clean the history list and clear the widget displaying raw datas
	m_EventManager.clean();
	TreeWidgetHistory->clear();
	
	// set the datetime in the statistic manager
	m_statisticManager.init(dateEditBegin->date(), dateEditEnd->date());
}

///////////////////////////////////////////////////////////
// Read + decrypt string received from client
///////////////////////////////////////////////////////////
QString LicenseStatusDialog::ReadLineFromSocket(void)
{
	CCryptoFile ascii;			// Buffer used for crypting data!
	char*		szAsciiKey;
	QString		strMessage;
	QByteArray	pString;

	strMessage = socket->readLine();
	strMessage = strMessage.trimmed();		// Remove leading '\n'
	pString = strMessage.toLatin1();
	ascii.SetBufferFromHexa(pString.data());
	ascii.Decrypt_Buffer((BYTE*)GEX_CRYPTO_KEY,GEX_KEY_BYTES);
	ascii.GetBuffer(&szAsciiKey);
    QString strAsciiKey = szAsciiKey;
    free(szAsciiKey);
    return strAsciiKey;
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
	connect( (QObject *)socket, SIGNAL(disconnected()),this,
		SLOT(socketDisconnected()) );
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
}

///////////////////////////////////////////////////////////
// User wants to disconnect from a given server...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::on_buttonDisconnect_clicked()
{
	OnCloseConnection();
}

///////////////////////////////////////////////////////////
// Create report directory...
///////////////////////////////////////////////////////////
void LicenseStatusDialog::createReportDir()
{
	QStringList strDataFiles;
	QStringList::Iterator it;
	QString strSource,strDestination,strSrcImageFile,strDestImageFile;
	QDir	cDir;

	// Source (<gex_application>/images/)
	CGexSystemUtils::GetApplicationDirectory(strSource);
	strSource += "/images/";

	// Create report folder directory in case it doesn't exit yet !
	CGexSystemUtils::GetUserHomeDirectory(strDestination);
	strDestination += GEXLS_REPORT_DIR;
	cDir.mkdir(strDestination);

	// Create directory <stdf-file-path>/<report>/images folder
	strDestination += "/";

	strSrcImageFile		= strSource + "ruler.png";
	strDestImageFile	= strDestination + "ruler.png";

	if (cDir.exists(strSrcImageFile))
	{
		QFile::remove(strDestImageFile);
		QFile::copy(strSrcImageFile, strDestImageFile);
	}

	strSrcImageFile		= strSource + "bar1.png";
	strDestImageFile	= strDestination + "bar1.png";

	if (cDir.exists(strSrcImageFile))
	{
		QFile::remove(strDestImageFile);
		QFile::copy(strSrcImageFile, strDestImageFile);
	}
}

///////////////////////////////////////////////////////////
// Source has changed, update the label above the browser
///////////////////////////////////////////////////////////
void LicenseStatusDialog::onSourceChanged(const QUrl& urlName)
{
	lineEditReportLocation->setText(urlName.toLocalFile());
}
