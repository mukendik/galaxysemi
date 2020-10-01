///////////////////////////////////////////////////////////
// Class implementing features enabling Yield-Man to be
// integrated in the Y123-Web process loop.
// * read settings to connect to Y123-Client
// * connect to Y123 client, and retrieve settings to connect to y123db
// * update Y123db with appropriate status
///////////////////////////////////////////////////////////

// Standard includes

// Qt includes
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QProgressBar>

#ifdef _WIN32
#include <winsock.h>
#endif

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>

// Local includes
#include "mo_y123.h"
#include "engine.h"

// Gex UI
extern QProgressBar	*	GexProgressBar;	// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// CGexMoY123
//////////////////////////////////////////////////////////////////////
// Error map
GBEGIN_ERROR_MAP(CGexMoY123)
    GMAP_ERROR(eReadSettings, "Error reading settings %s")
    GMAP_ERROR(eClientConnect, "Error connecting to client %s")
    GMAP_ERROR(eDbConnect, "Error connecting to Y123DB %s")
    GMAP_ERROR(eDbNotConnected, "Not connected to Y123Db")
    GMAP_ERROR(eOpenFile, "Error opening file %s")
    GMAP_ERROR(eInvalidTrackingKey, "Invalid TrackingKey (%s)")
    GMAP_ERROR(eDbQuery, "Error executing query (%s)")
GEND_ERROR_MAP(CGexMoY123)

// Y123_CLIENT status monitored on socket
#define	GEXMO_Y123_CLIENT_SOCKET_IDLE			1		// client not currently in talks with server
#define	GEXMO_Y123_CLIENT_TRY_CONNECT			2		// trying to connect to server
#define	GEXMO_Y123_CLIENT_CONNECTED				3		// Connection to Y123-Client established
#define	GEXMO_Y123_CLIENT_WAIT_DBCONNECTION		4		// trying to connect to DbServer
#define	GEXMO_Y123_CLIENT_READY					5		// Connection to Y123-Client established, connection to Y123db succeeded: we are Ready

#define	GEXMO_Y123_CLIENT_TRY_CONNECT_TIMEOUT	2		// Connection timeout after 15secs.

///////////////////////////////////////////////////////////
// Constructor/Destructor
///////////////////////////////////////////////////////////
CGexMoY123::CGexMoY123()
{
    m_pSocket = NULL;
    m_iSocketStatus = GEXMO_Y123_CLIENT_SOCKET_IDLE;

    m_strClientDirectory = getenv("Y123_CLIENT_PATH");
}

CGexMoY123::~CGexMoY123()
{
}

///////////////////////////////////////////////////////////
// Get socket status information
QString CGexMoY123::GetSocketStatus()
{
    QString strStatus;
    switch(m_iSocketStatus)
    {
    case GEXMO_Y123_CLIENT_SOCKET_IDLE :
        strStatus = "client not currently in talks with server";
        break;
    case GEXMO_Y123_CLIENT_TRY_CONNECT :
        strStatus = "trying to connect to server";
        break;
    case GEXMO_Y123_CLIENT_CONNECTED :
        strStatus = "Connection to Y123-Client established";
        break;
    case GEXMO_Y123_CLIENT_WAIT_DBCONNECTION :
        strStatus = "trying to connect to DbServer";
        break;
    case GEXMO_Y123_CLIENT_READY :
        strStatus = "Connection to Y123-Client established, connection to Y123db succeeded";
        break;
    }
    return strStatus;
}

///////////////////////////////////////////////////////////
// Init: notify that Yield-Man started
//
// * Read settings
// * Connect to Client
// * Check connection to y123db
//
///////////////////////////////////////////////////////////
int CGexMoY123::Init()
{
    QString			strSettingsFile;
    CGexSystemUtils clUtils;

    // Check socket action status (if any).
    if(m_iSocketStatus == GEXMO_Y123_CLIENT_CONNECTED)
        return eOk;

    if(m_pSocket == NULL)
    {
        m_pSocket = new QTcpSocket();
        connect ( m_pSocket, SIGNAL( connectionClosed () ), this, SLOT( socketClosed() ) );
        connect ( m_pSocket, SIGNAL( connected() ), this, SLOT( socketConnected() ) );
        connect ( m_pSocket, SIGNAL( error(QAbstractSocket::SocketError) ), this, SLOT( socketError(QAbstractSocket::SocketError) ) );
        connect ( m_pSocket, SIGNAL( readyRead() ), this, SLOT( socketReadyRead() ) );
    }


    // first have to load client settings
    if(m_strServerName.isEmpty())
    {
        strSettingsFile = m_strClientDirectory+"\\.y123_client.conf";

        // Build settings file name
        clUtils.NormalizePath(strSettingsFile);
        // Read config file
        if(!LoadSettings(strSettingsFile))
        {
            // fatal error
            return eError;
        }
    }

    // Check socket action status (if any).
    switch(m_iSocketStatus)
    {
        case GEXMO_Y123_CLIENT_TRY_CONNECT: // We are waiting for connection to server to complete...
            m_nTimeOut++;
            if(m_nTimeOut > GEXMO_Y123_CLIENT_TRY_CONNECT_TIMEOUT)
            {
                // Abort this socket!.
                CloseSocket();
                GSET_ERROR1(CGexMoY123, eClientConnect, NULL, QString("[time out]").toLatin1().constData());
                return eError;
            }
            return eWait;

        case GEXMO_Y123_CLIENT_CONNECTED:	// Socket connected, refersh every Xs econds the clients list.
            // Db Connection OK
            return eOk;

        case GEXMO_Y123_CLIENT_WAIT_DBCONNECTION: // We are waiting for connection to server to complete...
            m_nTimeOut++;
            if(m_nTimeOut > GEXMO_Y123_CLIENT_TRY_CONNECT_TIMEOUT)
            {
                // Abort this socket!.
                CloseSocket();
                GSET_ERROR1(CGexMoY123, eDbConnect, NULL, QString("[time out]").toLatin1().constData());
                return eError;
            }
            return eWait;

        case GEXMO_Y123_CLIENT_SOCKET_IDLE:	// Not waiting for any client/server exchange.
            // Connect to Client server
            OpenSocket();
            return eWait;

        default:
            break;
    }

    return eError;
}

///////////////////////////////////////////////////////////
// InitError: notify that Yield-Man cannot started
//
// * Try to update Y123DB
//
///////////////////////////////////////////////////////////
bool CGexMoY123::InitError(const QString& strErrorMessage)
{
    if(!DbConnect())
        return false;

    // The TrackingKey is unknown
    // So, have to find in the Y123DB the current entry to update it

    QString		strQuery;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    QDateTime	clDateTime = QDateTime::currentDateTime();
    QString		strTimestamp = "FROM_UNIXTIME(" + QString::number(clDateTime.toTime_t()) +")";
    QString		strLastStatus;

    strQuery =  "UPDATE events_user SET ";
    strQuery += " Step='4_CLIENT_INSERTION',";
    strQuery += " ClientName='"+m_strComputerName+"',";
    strQuery += " Status='INTERNAL_ERROR',";
    strQuery += " ProgressInStep='STOPPED',";
    strQuery += " ErrorCode='INTERNAL',";
    if(!strErrorMessage.isEmpty())
        strQuery += " ErrorMessage='" + QString(strErrorMessage).replace("\\","\\\\") +"',";
    strQuery += "LastEventTime="+strTimestamp;
    strQuery += " WHERE ";
    strQuery += " Step='3_CLIENT_INIT' AND";
    strQuery += " ProgressInStep='STOPPED' AND";
    strQuery += " ClientName='"+m_strComputerName+"'";
    if(!clQuery.exec(strQuery))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery,NULL, clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clQuery.exec("commit");

    return true;
}

///////////////////////////////////////////////////////////
// Start: notify that Yield-Man step started
//
// 1) If not connected to Y123-Client:
// * Read settings
// * Connect to Client
// * Check connection to y123db
//
// 2) Update Y123DB
///////////////////////////////////////////////////////////
int CGexMoY123::Start(const QString & strTrackingKey, const QStringList &lstDataFiles)
{
    // Save tracking key
    m_strTrackingKey = strTrackingKey;

    // Check socket action status (if any).
    if(m_iSocketStatus != GEXMO_Y123_CLIENT_CONNECTED)
    {
        // Connection lost, try to re-connect
        int eStatus = Init();
        if(eStatus != eOk)
            return eStatus;
    }

    if(!DbConnect())
        return eError;

    // Check some validation before start
    // 1 - Is a valid TrackingKey ?
    QString		strBuffer;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    strBuffer =  "SELECT Status, Step, ProgressInStep, ClientName FROM events_user ";
    strBuffer += " WHERE TrackingKey=" + m_strTrackingKey;
    if(!clQuery.exec(strBuffer))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery,NULL, clQuery.lastError().text().toLatin1().constData());
        return eError;
    }
    if(!clQuery.first())
    {
        GSET_ERROR1(CGexMoY123, eInvalidTrackingKey,NULL, "Invalid entry for user_events table");
        return eError;
    }
    // Verify if this TrackingKey is always valid to yieldman (after an internal error ...)
    QString strStatus, strStep, strProgress, strClientName;
    strStatus = clQuery.value(0).toString();
    strStep = clQuery.value(1).toString();
    strProgress = clQuery.value(2).toString();
    strClientName = clQuery.value(3).toString();
    if((strStatus != "PROGRESS")
    || (strStep != "3_CLIENT_INIT")
    || (strProgress != "STOPPED")
    || (strClientName != m_strComputerName))
    {
        // Then ignore this trigger
        // Probably an old trigger
        // Have to delete it
        GSET_ERROR1(CGexMoY123, eOpenFile,NULL, "Old trigger file : have to ignore it");
        return eError;
    }

    // 2 - Update Y123db to specify that the Yield-Man step started
    if(!DbBeginTrackingKey())
    {
        DbEndTrackingKey(eInternalError, GGET_LASTERRORMSG(CGexMoY123,this));
        return eError;
    }

    if((m_strTriggerFormat.isEmpty() || m_strDataminingDirectory.isEmpty()))
    {
        if(!ReadDbSettings())
        {
            DbEndTrackingKey(eInternalError, GGET_LASTERRORMSG(CGexMoY123,this));
            return eError;
        }
    }

    // 3 - DataFiles are accessible ?
    QFile	clFile;
    QStringList::const_iterator it;
    for(it = lstDataFiles.begin(); it != lstDataFiles.end(); it++)
    {
        clFile.setFileName(*it);
        if (clFile.open(QIODevice::ReadOnly) == false)
        {
            strBuffer =  *it + " - "+ clFile.errorString();
            GSET_ERROR1(CGexMoY123, eOpenFile,NULL, strBuffer.toLatin1().constData());
            DbEndTrackingKey(eInternalError, GGET_LASTERRORMSG(CGexMoY123,this));
            return eError;
        }
        clFile.close();
    }

    // Connect the ProgressBar to the function EventFilter
    if(GexProgressBar)
        GexProgressBar->installEventFilter(this);
    m_tLastUpdate = QTime::currentTime().addSecs(-2);
    m_nCurrentStep = -1;

    // Success
    return eOk;
}

///////////////////////////////////////////////////////////
// Stop: notify that Yield-Man step finished
///////////////////////////////////////////////////////////
int CGexMoY123::Stop(int eStatus, const QString &strMessage)
{
    QStringList strlWarnings;
    // Disconnect the ProgressBar to the function EventFilter
    if(GexProgressBar)
        GexProgressBar->removeEventFilter(this);

    return Stop(eStatus, strMessage, strlWarnings);
}

int CGexMoY123::Stop(int eStatus, const QString &strMessage, const QStringList &strlWarnings)
{
    if(eStatus == eSuccess)
    {
        // Generate the trigger file for Yield123
        if(!GenerateTriggerFile(strlWarnings))
        {
            DbEndTrackingKey(eInternalError, GGET_LASTERRORMSG(CGexMoY123,this));
            return eError;
        }
    }


    if(!DbEndTrackingKey(eStatus, strMessage))
        return eError;

    return eOk;
}


/////////////////////////////////////////////////////////////////////////////////////
// Custom event received
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::eventFilter(QObject* obj, QEvent* /*ev*/)
{
    // Each time the ProgressDialog is updated
    // Update the Yield123 status
    // Check last update
    if(m_tLastUpdate < (QTime::currentTime().addSecs(-2)))
    {
        // Update each 2 s
        m_tLastUpdate = QTime::currentTime();

        // this is GexProgressBar
        QProgressBar	*pclProgress = (QProgressBar*)obj;
        if(pclProgress)
        {
            int nValue = pclProgress->value();
            int nMax = pclProgress->maximum();
            int nStep = (int)((100.0*(double)nValue)/(double)nMax);

            if(m_nCurrentStep != nStep)
            {
                m_nCurrentStep = nStep;

                // Update Y123Db on Server
                DbProcessingTrackingKey(nStep);
            }
        }
    }
    // standard event processing
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////
// Read application settings
// Return bool
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::LoadSettings(QString strSettingsFile)
{
    FILE*		hSettingsFile;
    int			iBufferSize = 512;
    char		szString[iBufferSize];
    int			iLastCheckedLine = 0;
    QByteArray	pString;

    // Open settings file
    hSettingsFile = fopen(strSettingsFile.toLatin1().constData(), "rt");
    if(!hSettingsFile)
    {
        GSET_ERROR1(CGexMoY123, eReadSettings, NULL, QString("from file " + strSettingsFile + "[cannot open file]").toLatin1().constData());
        return false;
    }

    // Y123 Server Socket  section
    if(ut_ReadFromIniFile(hSettingsFile,"Y123LocalServerSocket","LocalServerHost",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
        m_strServerName = szString;
    else
    {
        GSET_ERROR1(CGexMoY123, eReadSettings, NULL, QString("from file " + strSettingsFile + "[cannot read LocalServerHost in Y123LocalServerSocket section]").toLatin1().constData());
        return false;
    }
    if(ut_ReadFromIniFile(hSettingsFile,"Y123LocalServerSocket","LocalServerSocket",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
        m_uiServerSocket = QString(szString).toInt();
    else
    {
        GSET_ERROR1(CGexMoY123, eReadSettings, NULL, QString("from file " + strSettingsFile+"[cannot read LocalServerSocket in Y123LocalServerSocket section]").toLatin1().constData());
        return false;
    }

    // Close settings file
    fclose(hSettingsFile);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to connect to Client
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::OpenSocket(void)
{
    // Connect to Client server
    m_pSocket->connectToHost(m_strServerName, m_uiServerSocket);
    m_iSocketStatus = GEXMO_Y123_CLIENT_TRY_CONNECT;
    m_nTimeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// Close Socket
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::CloseSocket(void)
{
    // Disconnect
    m_pSocket->close();
    m_iSocketStatus = GEXMO_Y123_CLIENT_SOCKET_IDLE;
    m_nTimeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::socketError(QAbstractSocket::SocketError nError)
{
    QString strError;

    switch(nError)
    {
        case QTcpSocket::ConnectionRefusedError:
            strError.sprintf("Error: connection refused!");
            break;
        case QTcpSocket::HostNotFoundError:
            strError.sprintf("Error: host not found!");
            break;
        case QTcpSocket::UnknownSocketError:
        default:
            strError.sprintf("Error: unknown error!");
            break;
    }
    CloseSocket();
    GSET_ERROR1(CGexMoY123, eClientConnect, NULL, QString("["+strError+"]").toLatin1().constData());
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::socketConnected()
{
    m_iSocketStatus = GEXMO_Y123_CLIENT_WAIT_DBCONNECTION;
    m_nTimeOut = 0;

    QTextStream os(m_pSocket);

    // Command string to send to server

    QString		strQueryServer = "[Y123_SEND_COMPUTERINFORMATION]";
    // Send command line server
    os << strQueryServer << "\n";
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::socketClosed()
{
    m_iSocketStatus = GEXMO_Y123_CLIENT_SOCKET_IDLE;
    m_nTimeOut = 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
void CGexMoY123::socketReadyRead()
{
    QString			strString1,strString2,strString3;
    QString			strServerMessage;

    // Server has sent a message...process it.
    while(m_pSocket->canReadLine())
    {
        // Read line, then parse it.
        strServerMessage = m_pSocket->readLine();
        strServerMessage = strServerMessage.trimmed();

        if(strServerMessage.startsWith("[Y123_READ_COMPUTERINFORMATION]") == true)
        {
            // This line includes a node status string...parse it.
            int iPos = 0;
            strServerMessage = strServerMessage.section("[Y123_READ_COMPUTERINFORMATION]",1);

            m_strComputerName = strServerMessage.section(';',iPos,iPos);		// ComputerName
            iPos++;
            m_strComputerIp = strServerMessage.section(';',iPos,iPos);			// ComputerIp

            // And immediatly send request for ask ServerDb connection
            QTextStream os(m_pSocket);

            // Command string to send to server

            QString		strQueryServer = "[Y123_SEND_SERVERDBCONNECTION]";
            strQueryServer += "YieldMan;";
            strQueryServer += m_strComputerName +";";
            strQueryServer += m_strComputerIp;

            // Send command line server
            os << strQueryServer << "\n";
        }
        else 		if(strServerMessage.startsWith("[Y123_READ_SERVERDBCONNECTION]") == true)
        {
            // This line includes a node status string...parse it.
            int iPos = 0;
            strServerMessage = strServerMessage.section("[Y123_READ_SERVERDBCONNECTION]",1);

            m_uiPort = strServerMessage.section(';',iPos,iPos).toInt();		// Port
            iPos++;
            m_strHostName = strServerMessage.section(';',iPos,iPos);		// Host
            iPos++;
            m_strDatabaseName = strServerMessage.section(';',iPos,iPos);	// DBName
            iPos++;
            m_strUserName = strServerMessage.section(';',iPos,iPos);		// User
            iPos++;
            m_strPassword = strServerMessage.section(';',iPos,iPos);		// Pwd

            // Try to connect
            // else Wait
            DbDisconnect();
            if(DbConnect())
            {
                // Read Database Settings
                m_iSocketStatus = GEXMO_Y123_CLIENT_CONNECTED;
                m_nTimeOut = 0;
            }
        }
    };
}


/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::DbConnect()
{
    // Closes link to database if opened
    if(m_sqlDatabase.isValid() && m_sqlDatabase.isOpen())
    {
        QSqlQuery clQuery(QString::null, m_sqlDatabase);

        // test the connection
        if(clQuery.exec("SELECT 1 FROM dual"))
            return true;
    }

    // Closes link to database if opened
    DbDisconnect();

    if(!m_sqlDatabase.isValid())
    {
        // Connect to database
        QDateTime	clDateTime = QDateTime::currentDateTime();
        QString		strTimestamp = clDateTime.toString("ddMMyyyy_hhmmss");
        m_strConnectionName = strTimestamp;
        m_sqlDatabase = QSqlDatabase::addDatabase("QMYSQL3", m_strConnectionName);
    }

    if(!m_sqlDatabase.isValid())
    {
        // Error: set error and return false
        QString		strDbError = "Add Database object";
        GSET_ERROR1(CGexMoY123, eDbConnect, NULL, QString("(host="+m_strHostName+", port="+QString::number(m_uiPort)+", db="+m_strDatabaseName+", user="+m_strUserName+", pwd="+m_strPassword+") ["+strDbError+"]").toLatin1().constData());
        return false;
    }

    // Connect to DB
    m_sqlDatabase.setHostName(m_strHostName);
    m_sqlDatabase.setDatabaseName(m_strDatabaseName);
    m_sqlDatabase.setUserName(m_strUserName);
    m_sqlDatabase.setPassword(m_strPassword);
    m_sqlDatabase.setPort(m_uiPort);
    if(!m_sqlDatabase.open())
    {
        // Error: set error and return false
        QSqlError	clSqlError = m_sqlDatabase.lastError();
        QString		strDbError = clSqlError.text();
        GSET_ERROR1(CGexMoY123, eDbConnect, NULL, QString("(host="+m_strHostName+", port="+QString::number(m_uiPort)+", db="+m_strDatabaseName+", user="+m_strUserName+", pwd="+m_strPassword+") ["+strDbError+"]").toLatin1().constData());
        return false;
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::DbDisconnect()
{
    // Closes link to database if opened
    if(m_sqlDatabase.isValid() && m_sqlDatabase.isOpen())
        m_sqlDatabase.close();

    QSqlDatabase::removeDatabase(m_strConnectionName);

    m_sqlDatabase = QSqlDatabase();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Update Y123db to specify that the Yield-Man step started
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::DbBeginTrackingKey()
{

    if(!DbConnect())
        return false;

    QString		strQuery;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    QDateTime	clDateTime = QDateTime::currentDateTime();
    QString		strTimestamp = "FROM_UNIXTIME(" + QString::number(clDateTime.toTime_t()) +")";
    QString		strLastStatus;

    strQuery =  "UPDATE events_user SET ";
    strQuery += " Step='4_CLIENT_INSERTION',";
    strQuery += " ProgressInStep='STARTED',";
    strQuery += " ClientName='"+m_strComputerName+"',";
    strQuery += " LastEventTime="+strTimestamp;
    strQuery += " WHERE TrackingKey=" + m_strTrackingKey;
    strQuery += " AND ";
    strQuery += " Step='3_CLIENT_INIT' ";
    strQuery += " AND ";
    strQuery += " ProgressInStep='STOPPED' ";
    strQuery += " AND ";
    strQuery += " ClientName='"+m_strComputerName+"'";
    if(!clQuery.exec(strQuery))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery,NULL, clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clQuery.exec("commit");

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Update Y123db to specify that the Yield-Man step started
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::DbProcessingTrackingKey(int nStep)
{

    if(!DbConnect())
        return false;

    QString		strQuery;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    QDateTime	clDateTime = QDateTime::currentDateTime();
    QString		strTimestamp = "FROM_UNIXTIME(" + QString::number(clDateTime.toTime_t()) +")";
    QString		strLastStatus;

    strQuery =  "UPDATE events_user SET ";
    strQuery += " Step='4_CLIENT_INSERTION',";
    strQuery += " ProgressInStep='STEP["+QString::number(nStep)+"%]',";
    strQuery += " ClientName='"+m_strComputerName+"',";
    strQuery += " LastEventTime="+strTimestamp;
    strQuery += " WHERE TrackingKey=" + m_strTrackingKey;
    strQuery += " AND ";
    strQuery += " Step='4_CLIENT_INSERTION' ";
    strQuery += " AND ";
    strQuery += " NOT ProgressInStep='STOPPED' ";
    strQuery += " AND ";
    strQuery += " ClientName='"+m_strComputerName+"'";
    if(!clQuery.exec(strQuery))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery,NULL, clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clQuery.exec("commit");

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::DbEndTrackingKey(int eStatus, const QString & strMessage)
{

    if(!DbConnect())
        return false;

    QString		strQuery;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    QDateTime	clDateTime = QDateTime::currentDateTime();
    QString		strTimestamp = "FROM_UNIXTIME(" + QString::number(clDateTime.toTime_t()) +")";
    QString		strLastStatus;

    // if no error only update ProgressInStep
    // else update ErrorMessage
    strQuery =  "UPDATE events_user SET ";
    strQuery += " Step='4_CLIENT_INSERTION',";
    strQuery += " ClientName='"+m_strComputerName+"',";

    if(eStatus == eInternalError)
        strQuery += " Status='INTERNAL_ERROR',";
    if(eStatus == eUserError)
        strQuery += " Status='USER_ERROR',";

    strQuery += " ProgressInStep='STOPPED',";

    if(eStatus == eInternalError)
        strQuery += " ErrorCode='INTERNAL',";
    if(eStatus == eUserError)
        strQuery += " ErrorCode='CORRUPTED',";

    if(!strMessage.isEmpty())
        strQuery += " ErrorMessage='" + QString(strMessage).replace("\\","\\\\") +"',";

    strQuery += " LastEventTime="+strTimestamp;
    strQuery += " WHERE TrackingKey=" + m_strTrackingKey;
    strQuery += " AND ";
    strQuery += " Step='4_CLIENT_INSERTION' ";
    strQuery += " AND ";
    strQuery += " ClientName='"+m_strComputerName+"'";
    if(!clQuery.exec(strQuery))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery,NULL, clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clQuery.exec("commit");

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::ReadDbSettings()
{
    if(!DbConnect())
        return false;

    QString		strQuery, strSection;
    QSqlQuery	clQuery(QString::null,m_sqlDatabase);

    if(!clQuery.exec("SELECT * FROM settings"))
    {
        GSET_ERROR1(CGexMoY123, eDbQuery, NULL, QString("["+clQuery.lastError().text()+"]").toLatin1().constData());
        return false;
    }
    while(clQuery.next())
    {
        strSection = clQuery.value(0).toString();
        if(strSection == "Y123_PATH_DATAMINING_TRIGGER")
        {
            m_strDataminingDirectory= clQuery.value(1).toString();
        }
        else if(strSection == "Y123_FORMAT_TRIGGER")
        {
            m_strTriggerFormat = clQuery.value(1).toString();
        }
    }

    if(m_strDataminingDirectory.isEmpty())
    {
        GSET_ERROR1(CGexMoY123, eReadSettings, NULL, QString("from Y123DB [Cannot find Y123_PATH_DATAMINING_TRIGGER entry in settings table]").toLatin1().constData());
        return false;
    }
    if(m_strTriggerFormat.isEmpty())
    {
        GSET_ERROR1(CGexMoY123, eReadSettings, NULL, QString("from Y123DB [Cannot find Y123_FORMAT_TRIGGER entry in settings table]").toLatin1().constData());
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Generate Trigger file
/////////////////////////////////////////////////////////////////////////////////////
bool CGexMoY123::GenerateTriggerFile(const QStringList &strlWarnings)
{

    QString strDirectory;
    QString strFullFileName;
    QString strTemp;
    CGexSystemUtils clUtils;

    strDirectory = m_strClientDirectory + "/" + m_strDataminingDirectory;

    strTemp = m_strTriggerFormat;
    strTemp.replace("$(file)",m_strTrackingKey, Qt::CaseInsensitive);
    strFullFileName = strDirectory + strTemp;
    clUtils.NormalizePath(strFullFileName);


    QFile file(strFullFileName + "_tmp");

    // Create trigger file.
    if (file.open(QIODevice::WriteOnly) == false)
    {
        GSET_ERROR1(CGexMoY123, eOpenFile, NULL, QString("["+file.errorString()+"]").toLatin1().constData());
        return false;	// Failed writing to trigger file.
    }

    // Write Trigger File
    QTextStream hTrigger(&file);	// Assign file handle to data stream
    QDateTime cTime = QDateTime::currentDateTime();

    // Write header
    hTrigger << "######################################################################" << endl;
    hTrigger << "# YIELD123 Trigger file" << endl;
    hTrigger << "# Created with : "
      << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
    hTrigger << "# Creation Date: " << cTime.toString("d MMMM yyyy h:mm:ss") << endl;
    hTrigger << "######################################################################" << endl;
    hTrigger << "<GalaxyTrigger>" << endl;
    hTrigger << "Action=YIELD123REPORT" << endl;
    hTrigger << "TrackingKey=" << m_strTrackingKey << endl;
    if(!strlWarnings.isEmpty())
        hTrigger << "Warning=" << strlWarnings.join("\nWarning=") << endl;
    // End of trigger file marker
    hTrigger << "</GalaxyTrigger>" << endl;

    // close file
    file.close();

    QDir	clDir;
    clDir.remove(strFullFileName);
    if(!clDir.rename(strFullFileName + "_tmp",strFullFileName))
    {
        GSET_ERROR1(CGexMoY123, eOpenFile, NULL, QString("[Cannot create trigger file "+strFullFileName+"]").toLatin1().constData());
        return false;	// Failed writing to trigger file.
    }

    return true;

}

