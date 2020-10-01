///////////////////////////////////////////////////////////
// GEX-LM server header file
///////////////////////////////////////////////////////////
#ifndef GEXLM_SERVER_H
#define GEXLM_SERVER_H

#include <QDateTime>
#include <QTcpSocket>
#include <QTcpServer>

class ClientSocket;	// Declaration follows!

///////////////////////////////////////////////////////////
//  The SimpleServer class handles new connections to the server. For every
//  client that connects, it creates a new ClientSocket -- that instance is now
//  responsible for the communication with that client.
///////////////////////////////////////////////////////////
class SimpleServer : public QTcpServer
{
	Q_OBJECT

public:
    SimpleServer(QObject* parent=0, int nPortNumber=4242);

private slots:
	void			OnNewConnection();
};

class ClientNode : public QObject
{
	Q_OBJECT

public:

	ClientNode();
	~ClientNode();

	QString					strComputer;		// Client computer name
	QString					strUser;			// Client user name
	QString					m_strClientId;		// Client identifier: concatenation of client informations
	QDateTime				LastHandshake;		// Time of last Client/Server handshake
	QList<ClientSocket *>	m_lstClientSocket;// Handle to list of socket instances created.
	QString					BuildLogMessage(ClientSocket * pClientSocket);	// Build the message to log when client start or stop a connection

	void					OnDisconnectClient();	// Client closed connection...
};

///////////////////////////////////////////////////////////
//  The ClientSocket class provides a socket that is connected with a client.
//  For every client that connects to the server, the server creates a new
//  instance of this class.
///////////////////////////////////////////////////////////
class ClientSocket : public QObject
{
	Q_OBJECT

public:
	ClientSocket(QTcpSocket *pTcpSocket);
    ~ClientSocket();

	quint16			tcpSocketPeerPort() const				{ return m_pTcpSocket->peerPort(); }
	QString			clientAppName()							{ return m_strClientAppName; }
	QDateTime		sessionStartDate()						{ return m_dtSessionStart;}

private:

	QTcpSocket *	m_pTcpSocket;							// Pointer on the client socket
	QString			m_strClientAppName;						// Client full GEX application name (with version...)
	QDateTime		m_dtSessionStart;						// Session date (license request date)

	void			WriteLineToSocket(QString strMessage);	// Send string to client (encrypt string)
	QString			ReadLineFromSocket(void);				// Read + decrypt string received from client
	void			ReadHistoryDataFromLogFile(QStringList& strHistoryList, const QDate& dateBegin, const QDate& dateEnd);	// Read log files and extract datas that match with filters

	// YIELDMANDB
	QString			LoadYieldManDbSettings();
	void			WriteYieldManDbSettings(QString strConnection);
	// YIELDMANDB

public slots:

	void			OnReadyRead();							// Receiving query from client...
	void			OnDisconnectSocket();					// Socket closed connection...
};

#endif // ifdef GEXLM_SERVER_H
