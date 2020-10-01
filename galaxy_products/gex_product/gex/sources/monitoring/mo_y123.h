///////////////////////////////////////////////////////////
// Class implementing features enabling Yield-Man to be 
// integrated in the Y123-Web process loop.
// * read settings to connect to Y123-Client
// * connect to Y123 client, and retrieve settings to connect to y123db
// * update Y123db with appropriate status
///////////////////////////////////////////////////////////

#ifndef GEXMO_Y123_H
#define GEXMO_Y123_H

// Standard includes

// QT includes 
#include <qobject.h>
#include <qdatetime.h>
#include <QSqlDatabase>
#include <QAbstractSocket>

// Galaxy modules includes
#include <gstdl_errormgr.h>

// Local includes

// Forward declarations

///////////////////////////////////////////////////////////
// CGexMoY123 class: interaction with Y123-Web applications...
///////////////////////////////////////////////////////////

class QTcpSocket;
class QSqlDatabase;

class CGexMoY123 : public QObject
{
    Q_OBJECT
	
public:
	GDECLARE_ERROR_MAP(CGexMoY123)
	{
		eReadSettings,						// Error reading settings
		eClientConnect,						// Error connecting to Y123-client
		eDbConnect,							// Error connecting to Y123Db
		eDbNotConnected,					// Not connected to Y123Db
		eOpenFile,							// Error file manipulation
		eInvalidTrackingKey,				// Invalid TrackingKey (no entry from user_events)
		eDbQuery							// Error executing query Y123DB
	}
	GDECLARE_END_ERROR_MAP(CGexMoY123)

	enum Y123Status
	{
		eOk,
		eWait,
		eError
	};

	enum YieldManStatus
	{
		eSuccess,
		eUserError,
		eInternalError
	};
		
	// Constructor / destructor functions
	CGexMoY123();			// Consturctor
	~CGexMoY123();			// Destructor

	// Functions
	int		Init();
	bool	InitError(const QString& strErrorMessage);
	int		Start(const QString & strTrackingKey, const QStringList &lstDataFiles);
	int		Stop(int eStatus, const QString & strMessage="");
	int		Stop(int eStatus, const QString & strMessage, const QStringList &strlWarnings);
	QString	GetSocketStatus();

	// Variables

private:
	// Functions
	bool	LoadSettings(QString strSettingsFile);
	bool	ReadDbSettings();
	bool	GenerateTriggerFile(const QStringList &strlWarnings);

	void	OpenSocket(void);
	void	CloseSocket(void);

	bool	DbConnect();
	bool	DbDisconnect();
	bool	DbBeginTrackingKey();
	bool	DbProcessingTrackingKey(int nStep);
	bool	DbEndTrackingKey(int eStatus, const QString & strMessage);


	// Variables
	int				m_iSocketStatus;				// Keeps track of Client/server pending events.
	QString			m_strTrackingKey;
	int				m_nTimeOut;

	QString			m_strComputerName;			// Computer used
	QString			m_strComputerIp;			// Computer used

	// for client connection
	QString			m_strServerName;
	unsigned int	m_uiServerSocket;
	QTcpSocket*		m_pSocket;

	// for SerberDb connection
	unsigned int	m_uiPort;
	QString			m_strHostName;
	QString			m_strDatabaseName;
	QString			m_strUserName;
	QString			m_strPassword;
	
	QSqlDatabase	m_sqlDatabase;
	QString			m_strConnectionName;

	// for file manipulation
	QString			m_strClientDirectory;
	QString			m_strDataminingDirectory;
	QString			m_strTriggerFormat;

	// for update Y123 DB with cuurent ProgressBar Step
	QTime			m_tLastUpdate;
	int				m_nCurrentStep;

// Overrides
protected:
    bool	eventFilter(QObject *obj, QEvent *ev);

protected slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();
    void socketReadyRead();
	void socketClosed();
};

#endif
