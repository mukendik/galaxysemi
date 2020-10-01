/****************************************************************************
** Deriven from gex_connectiontest_dialog_base.cpp
****************************************************************************/

#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>
#include <QHostAddress>

#ifdef _WIN32
#include <winsock.h>
#endif

#if defined __unix__ || __MACH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "gex_connectiontest_dialog.h"

#include <gqtl_skin.h>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexConnectionTestDialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
//
// The dialog will by default be modeless, unless you set 'modal' to
// true to construct a modal dialog.
/////////////////////////////////////////////////////////////////////////////////////
CGexConnectionTestDialog::CGexConnectionTestDialog(QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
	// Setup UI
	setupUi(this);

	// Set the default palette 
	CGexSkin gexSkin;
	gexSkin.applyPalette(this);

	m_pSocket = new QTcpSocket(this);
    connect(m_pSocket, SIGNAL(hostFound()), this, SLOT(socketHostFound()));
    connect(m_pSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
	m_textStream = new QTextStream(m_pSocket);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexConnectionTestDialog::~CGexConnectionTestDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/////////////////////////////////////////////////////////////////////////////////////
// Set Smtp server settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexConnectionTestDialog::Init(const QString & strSmtpServer, uint uiSmtpPort)
{
	m_strSmtpServer = strSmtpServer;
	m_uiSmtpPort = uiSmtpPort;

	editSmtpServer->setText(m_strSmtpServer);
	editSmtpPort->setText(QString::number(m_uiSmtpPort));

	// Message
	listWidgetMessages->clear();
	listWidgetMessages->addItem(tr("Retrieving server information"));

	// Launch the test procedure asap
	QTimer::singleShot(0, this, SLOT(StartTest()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Start test
/////////////////////////////////////////////////////////////////////////////////////
void CGexConnectionTestDialog::StartTest()
{
	QString strServerName = "???";
	QString	strServerIP = "???";
	QString strSmtpServer = m_strSmtpServer;

	// Make sure server name is in IP format
	QHostAddress	clHostAddr;
	QByteArray		pString = m_strSmtpServer.toLatin1();
	if(clHostAddr.setAddress(m_strSmtpServer) == false)
	{
		// The server name is in domain name format. Try to retrieve the IP address.
		m_bUsingServerName = true;
		strServerName = m_strSmtpServer;
		hostent* host = 0;
		host = gethostbyname(pString.constData());
		if(host)
		{
			unsigned long ul_HostAddress;
			memcpy(&(ul_HostAddress), host->h_addr, host->h_length);
			ul_HostAddress = htonl(ul_HostAddress);
			clHostAddr.setAddress((quint32)ul_HostAddress);
			strServerIP = clHostAddr.toString();
			strSmtpServer = strServerIP;
		}
	}
	else
	{
		// The server name is in IP address format. Try to retrieve the server name.
		m_bUsingServerName = false;
		strServerIP = m_strSmtpServer;
		unsigned long ul_HostAddress = (unsigned long)clHostAddr.toIPv4Address();
		ul_HostAddress = ntohl(ul_HostAddress);
		char *pHostAddress = (char *)&ul_HostAddress;
		struct hostent *host = 0;
		host = gethostbyaddr(pHostAddress, 4, AF_INET);
		if(host)
			strServerName = host->h_name;
	}
		
	// Message 
	QString strMessage = tr("Connecting to server (Name=");
	strMessage += strServerName;
	strMessage += ", IP=";
	strMessage += strServerIP;
	strMessage += ", Port=";
	strMessage += QString::number(m_uiSmtpPort);
	strMessage += ")";
	listWidgetMessages->addItem(strMessage);

	// Connect
	m_pSocket->connectToHost(strSmtpServer, m_uiSmtpPort);
	buttonOk->setEnabled(false);
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop test
/////////////////////////////////////////////////////////////////////////////////////
void CGexConnectionTestDialog::StopTest()
{
	CloseSocket();
}

void CGexConnectionTestDialog::CloseSocket(void)
{
	m_pSocket->close();
}

void CGexConnectionTestDialog::socketError(QAbstractSocket::SocketError Error)
{
    QString lErrorString;

    switch(Error)
	{
		case QAbstractSocket::ConnectionRefusedError:
            lErrorString = "Error: connection refused!";
			break;
		case QAbstractSocket::RemoteHostClosedError:
            lErrorString = "Error: remote host closed connection!";
			break;
		case QAbstractSocket::HostNotFoundError:
			if(m_bUsingServerName)
                lErrorString = "Error: host not found!";
			else
                lErrorString = "Error: host name not found! Try to use the host's IP address instead of its name.";
			break;
		case QAbstractSocket::SocketAccessError:
            lErrorString = "Error: access refused!";
			break;
		case QAbstractSocket::SocketResourceError:
            lErrorString = "Error: resource!";
			break;
		case QAbstractSocket::SocketTimeoutError:
            lErrorString = "Error: timeout!";
			break;
		case QAbstractSocket::DatagramTooLargeError:
            lErrorString = "Error: datagram too large!";
			break;
		case QAbstractSocket::NetworkError:
            lErrorString = "Error: network!";
			break;
		case QAbstractSocket::AddressInUseError:
            lErrorString = "Error: address in use!";
			break;
		case QAbstractSocket::SocketAddressNotAvailableError:
            lErrorString = "Error: address not available!";
			break;
		case QAbstractSocket::UnsupportedSocketOperationError:
            lErrorString = "Error: unsupported socket operation!";
			break;
		case QAbstractSocket::UnknownSocketError:
		default:
            lErrorString.sprintf("Error: unknown error!");
			break;
	}

    listWidgetMessages->addItem(lErrorString);
	StopTest();
}

void CGexConnectionTestDialog::socketHostFound()
{
	listWidgetMessages->addItem(tr("Host found"));
}

void CGexConnectionTestDialog::socketConnected()
{
	listWidgetMessages->addItem(tr("Connection successful"));
}

void CGexConnectionTestDialog::socketReadyRead()
{
    // SMTP is line-oriented
    if ( !m_pSocket->canReadLine() )
        return;

	// The first message received after connection must be the initial response string
	// starting with 220
    QString strResponseLine, strResponse;
    do {
        strResponseLine = m_pSocket->readLine();
        strResponse += strResponseLine;
    } while( m_pSocket->canReadLine() && strResponseLine[3] != ' ' );
    strResponseLine.truncate( 3 );

	QString strString;
	if(strResponseLine.toUInt() != 220)
	{
		strString = "Error: unexpected initial response from server " + strResponse;
		strString += "!";
		listWidgetMessages->addItem(strString);
		strString = "Test FAILED";
		listWidgetMessages->addItem(strString);
		buttonOk->setEnabled(false);
	}
	else
	{
		strString = "Initial response received from server " + strResponse;
		listWidgetMessages->addItem(strString);
		strString = "Test SUCCESSFUL";
		listWidgetMessages->addItem(strString);
		buttonOk->setEnabled(true);
	}

	StopTest();
}
