
#include <QSqlDatabase>
#include <QStringList>
#include <QSqlError>
#include <QVariant>


#include "sql_connector.h"
#include "gqtl_log.h"

namespace GS
{
namespace DAPlugin
{
SqlConnector::SqlConnector()
{
    mHost = "";
    mPort = 0;
    mUser = "";
    mPass = "";
    mConnectionID = "";
    mDatabaseSID = "";
    mSchema = "";
    mDriver = "";
    mLastError = "";
    mPrivateConnectionID = true;
}

SqlConnector::~SqlConnector()
{
    if(mPrivateConnectionID)
    {
        // YM have access to all connection open through QSqlDatabase
        // and can share a Connection with the plugin
        // YM will close all connectionNames
        // If it is a Private connection
        // Remove the ConnectionName
        // And also all derived
        QString lConnectionName;
        QStringList lConnectionList = QSqlDatabase::connectionNames();
        while(lConnectionList.count() > 0)
        {
            lConnectionName = lConnectionList.takeFirst();
            if(lConnectionName.startsWith(mConnectionID))
                QSqlDatabase::removeDatabase(lConnectionName);
        }
    }
}

QSqlQuery SqlConnector::ExecQuery(const QString& query, bool &ok)
{
    if (query.isEmpty())
    {
        ok = false;
        mLastError = "Empty query";
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return (QSqlQuery)"";
    }

    if (!IsDatabaseConnected())
    {
        ok = false;
        mLastError = "Connection lost";
        GSLOG(SYSLOG_SEV_ERROR, "Connection lost");
        return (QSqlQuery)"";
    }

    QSqlQuery lQuery(QSqlDatabase::database(mConnectionID));
    if (!lQuery.exec(query))
    {
        mLastError = "Unable to exec query: " + query + " - Error: " + lQuery.lastError().text();
        ok = false;
        // Auto-Commit on error
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        lQuery.exec("COMMIT");
        return lQuery;
    }

    ok = true;
    return lQuery;
}

void SqlConnector::SetHost(const QString& value)
{
    mHost = value;
}

void SqlConnector::SetPort(const int value)
{
    mPort = value;
}

void SqlConnector::SetUser(const QString& value)
{
    mUser = value;
}

void SqlConnector::SetPass(const QString& value)
{
    mPass = value;
}

void SqlConnector::SetDriver(const QString& value)
{
    mDriver = value;
}

void SqlConnector::SetDatabaseSID(const QString& value)
{
    mDatabaseSID = value;
}

void SqlConnector::SetSchemaName(const QString& value)
{
    mSchema = value;
}

void SqlConnector::SetConnectionID(const QString& value)
{
    if(!mConnectionID.isEmpty())
        Disconnect();
    mConnectionID = value;
    mPrivateConnectionID = false;
}

QString SqlConnector::ConnectionID() const
{
    return mConnectionID;
}

QString SqlConnector::Schema() const
{
    return mSchema;
}

bool SqlConnector::Connect(const QString& dirUserId)
{
    if (!Connect())
        return false;

    mSessionId = RetrieveSqlSessionId();

    if (mSessionId.isEmpty())
    {
        mLastError = "empty session ID ";
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }

    mSessionId.prepend(dirUserId + ":");

    return true;
}

QString SqlConnector::GetSessionId()
{
    return mSessionId;
}

bool SqlConnector::IsSessionActive(const QString &sessionId)
{
    // If we are not able to know if the Id is active
    // We must return true
    QString lQueryContent;
    // SessionId = dirUserId:SqlSessionId
    // SqlSessionId = CONNECTION_ID():HOST:HOST_CONNECTOR
    // SessionId = login:CONNECTION_ID():HOST:HOST_CONNECTOR
    QString lSqlSessionId = sessionId.section(":", 1);
    QString lUserSID = lSqlSessionId.section(":",0,0);
    // QString lUserHostName = lSqlSessionId.section(":",1,1);
    QString lUserConnectorHostName = lSqlSessionId.section(":",2,2);
    QString lCurrentHostName = mSessionId.section(":",2,2);
    // Same server
    // User laptop = XX:192.168.1.55:head
    // you XX:localhost:head
    if(lUserConnectorHostName.toUpper() == lCurrentHostName.toUpper())
    {
        lQueryContent = "SELECT * FROM information_schema.PROCESSLIST ";
        lQueryContent+= " WHERE ID="+lUserSID;
        bool lOk = true;
        QSqlQuery lQuery = ExecQuery(lQueryContent, lOk);
        if (!lOk)
        {
            mLastError = "Unable to exec query: " + lQueryContent + " - Error: " + lQuery.lastError().text();
            GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
            return true;
        }

        if(!lQuery.first())
        {
            GSLOG(SYSLOG_SEV_CRITICAL, QString("EXPIRED da_session_id %1 for %2")
                  .arg(lUserSID).arg(lUserConnectorHostName).toLatin1().constData());
            return false;
        }
    }
    else
    {
        // For spider environement
        // The connection can be force the local HostName
        // for Node connector
        // we need to connect to the user connector host name
        // and check if the ID comes from this IP
        // User laptop = XX:192.168.1.55:head
        // you XX:localhost:comp1

        // Connect to this IP and check
        QString lConnectionID = mConnectionID;
        lConnectionID+= " on ";
        lConnectionID+= lUserConnectorHostName;

        // Add database link: make sure we use a unique name for the database connection
        // On some MySql server with AUTO connection configuration
        // The addDatabase can directly open a default connection (see at ASE, not reproducible ?)
        {
            QSqlDatabase clSqlDatabase;
        if(!QSqlDatabase::contains(lConnectionID))
        {
                clSqlDatabase = QSqlDatabase::addDatabase(mDriver, lConnectionID);
                // Force to close the connection if AUTO connection
                if(clSqlDatabase.isOpen())
                    clSqlDatabase.close();
            clSqlDatabase.setHostName(lUserConnectorHostName);
            clSqlDatabase.setDatabaseName(mSchema);
            clSqlDatabase.setUserName(mUser);
            clSqlDatabase.setPassword(mPass);
            clSqlDatabase.setPort(mPort);
                GSLOG(SYSLOG_SEV_WARNING, QString("Create a new connection to check EXPIRED mutex on %1 for %2: db[%3]user[%4]port[%5]")
                      .arg(lCurrentHostName).arg(lUserConnectorHostName)
                      .arg(mSchema)
                      .arg(mUser)
                      .arg(mPort).toLatin1().constData());
            }
            else
                clSqlDatabase = QSqlDatabase::database(lConnectionID);

            if(!clSqlDatabase.isOpen())
            {
            clSqlDatabase.open();
                GSLOG(SYSLOG_SEV_WARNING, QString("Open the connection to check EXPIRED mutex on %1 for %2: db[%3]user[%4]port[%5]")
                  .arg(lCurrentHostName).arg(lUserConnectorHostName)
                  .arg(mSchema)
                  .arg(mUser)
                  .arg(mPort).toLatin1().constData());
        }
        }

        QSqlQuery  lQuery(QSqlDatabase::database(lConnectionID));
            lQueryContent = "SELECT * FROM information_schema.PROCESSLIST ";
            lQueryContent+= " WHERE ID='"+lUserSID+"'";
            if(!lQuery.exec(lQueryContent))
            {
                mLastError = "Unable to exec query: " + lQueryContent + " - Error: " + lQuery.lastError().text();
                GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
            // Clean the connection if unable to open it
            QSqlDatabase::removeDatabase(lConnectionID);
                return true;
            }

            if(!lQuery.first())
            {
                GSLOG(SYSLOG_SEV_CRITICAL, QString("EXPIRED da_session_id %1 for %2")
                      .arg(lUserSID).arg(lUserConnectorHostName).toLatin1().constData());
                return false;
            }
        }
    return true;
}

bool SqlConnector::Disconnect()
{
    if (mConnectionID.isEmpty())
    {
        mLastError = "Unable to disconnect, no connection ID";
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }
    if(!QSqlDatabase::contains(mConnectionID))
    {
        mLastError = QString("Unable to disconnect, connection %1 not found").arg(mConnectionID);
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }
    // Check validity
    QSqlDatabase lDatabase = QSqlDatabase::database(mConnectionID,false);
    if(!lDatabase.isValid())
    {
        mLastError = "Unable to disconnect, Invalid connection: " + lDatabase.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }
    // Close connection if opened
    if(lDatabase.isOpen())
        lDatabase.close();
    // If still opened
    if (lDatabase.isOpen())
    {
        mLastError = "Unable to disconnect: " + lDatabase.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }

    QSqlDatabase::removeDatabase(mConnectionID);

    mSessionId = "";

    return true;
}

QString SqlConnector::GetLastError() const
{
    return mLastError;
}

bool SqlConnector::IsOracleServer()
{
    return ((mDriver == "QOCI8") || (mDriver == "QOCI"));
}

bool SqlConnector::IsMySqlServer()
{
    return (mDriver == "QMYSQL3");
}

// SqlSessionId = CONNECTION_ID():HOST:HOST_CONNECTOR
// when open a local connection on 'host1' = XX:localhost:host1
// when open a remote connection on 'host1' from 'host2' = XX:host2_IP:host1
// YM on comp1, user = XX:localhost:comp1
// GEX on laptop(192.168.1.55), user = XX:localhost:head
QString SqlConnector::RetrieveSqlSessionId()
{
    QString lQueryContent;
    QString lSessionId;

    // SHOW PROCESSLIST SHOWS ONLY USER'S PROCESSES
    // TO SEE WE HAVE TO BE CONNECTED AS SUPER  ie ROOT
    lQueryContent = "SELECT CONCAT(ID,':',HOST), @@hostname FROM information_schema.PROCESSLIST ";
    lQueryContent+= " WHERE ID=connection_id()";
    bool lOk = true;
    QSqlQuery lQuery = ExecQuery(lQueryContent, lOk);
    if (!lOk)
    {
        mLastError = "Unable to exec query: " + lQueryContent + "- Error: " + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return QString();
    }

    lQuery.first();
    lSessionId = lQuery.value(0).toString().section(":",0,1);
    lSessionId+=":"+lQuery.value(1).toString();

    return lSessionId;
}

bool SqlConnector::Connect()
{
    // Build connectionId if empty
    if (mConnectionID.isEmpty())
        mConnectionID = QString(SQL_DIR_TABLE)+":"+mDriver+":"+QString::number(mPort)+"["+mUser + "@" + mHost+"]";
    // Add database if not already done
    if(!QSqlDatabase::contains(mConnectionID))
    {
        QSqlDatabase::addDatabase(mDriver, mConnectionID);
        if(!QSqlDatabase::contains(mConnectionID))
        {
            mLastError = "Unable to add new connection to plugin.";
            GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
            return false;
        }
    }
    // Check validity
    QSqlDatabase lDatabase = QSqlDatabase::database(mConnectionID,false);
    if(!lDatabase.isValid())
    {
        mLastError = "Invalid connection: " +
                lDatabase.lastError().text() + "\n " +
                "Available drivers: " +
                QSqlDatabase::drivers().join(",");
                GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }

    // Force a new connection if the settings was changed
    bool lForceReconnect =
            (lDatabase.hostName() != mHost)
            || (lDatabase.port() != mPort)
            || (lDatabase.userName() != mUser)
            || (lDatabase.password() != mPass)
            || (lDatabase.databaseName() != mDatabaseSID)
            //|| (lDatabase.driver() != mDriver)
            ;

    // Close connection if opened
    if(lDatabase.isOpen())
    {
        if(!lForceReconnect)
            return true;
        GSLOG(SYSLOG_SEV_INFORMATIONAL,"Force to reconnect");
        lDatabase.close();
    }
    // Set parameters
    lDatabase.setHostName(mHost);
    lDatabase.setPort(mPort);
    lDatabase.setUserName(mUser);
    lDatabase.setPassword(mPass);
    lDatabase.setDatabaseName(mDatabaseSID);
    // Try to open connection
    if (!lDatabase.open())
    {
        mLastError = "Unable to open connection: " +
                lDatabase.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,QString("Connect to host[%1] port[%2] user[%3] db[%4] connID[%5]")
          .arg(mHost)
          .arg(mPort)
          .arg(mUser)
          .arg(mDatabaseSID)
          .arg(mConnectionID)
          .toLatin1().data());

    return true;
}


QDateTime SqlConnector::GetServerDateTime()
{
    QDateTime lServerDate = QDateTime::currentDateTime();
    QString   lQueryContent;

    if(IsOracleServer())
        lQueryContent = "SELECT SYSDATE FROM DUAL";
    else
        lQueryContent = "SELECT now()";


    bool lOk = true;
    QSqlQuery lQuery = ExecQuery(lQueryContent, lOk);
    if(lOk && lQuery.first())
        lServerDate = lQuery.value(0).toDateTime();
    else
    {
        mLastError = "Unable to exec query: " + lQueryContent + " - Error: " + GetLastError();
        GSLOG(SYSLOG_SEV_WARNING, mLastError.toLatin1().data());
    }

    return lServerDate;
}

bool SqlConnector::IsDatabaseConnected()
{
    // Check validity
    QSqlDatabase lDatabase = QSqlDatabase::database(mConnectionID,false);
    if(!lDatabase.isValid())
    {
        mLastError = "Invalid connection: " + lDatabase.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }
    // Check if opened
    if (!lDatabase.isOpen())
    {
        mLastError = "Connection closed: " + lDatabase.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }
    // Check connection with simple query
    QSqlQuery lQuery(lDatabase);
    lQuery.setForwardOnly(true);
    if(!lQuery.exec("SELECT 1 FROM dual"))
    {
        mLastError = "Connection lost: " + lQuery.lastError().text();
        GSLOG(SYSLOG_SEV_ERROR, mLastError.toLatin1().data());
        return false;
    }

    return true;
}

QString SqlConnector::TranslateStringToSqlVarChar(const QString& value)
{
    QString lNewValue;
    if(value.isNull() || value.isEmpty() || (value[0].toLatin1() == 0))
        lNewValue = "";
    else
        lNewValue = value;
    // Special Character Escape Sequences
    lNewValue.replace("'","''");

    // escape char
    if(!IsOracleServer())
        lNewValue.replace("\\","\\\\");

    return lNewValue;
}

QString SqlConnector::TranslateStringToSqlLob(const QString& value)
{
    QString lNewValue = TranslateStringToSqlVarChar(value);

    if(IsOracleServer() && (lNewValue.length()>4000))
    {
        // Split string into c_lob(4000 size)||c_lob(4000 size)
        for(int i = lNewValue.length()-4000; i>0; i-=4000)
            lNewValue = lNewValue.insert(i,"')||to_clob('");
        lNewValue = "to_clob('"+lNewValue+"')";
    }
    else if(IsMySqlServer())
    {
        lNewValue = "CONVERT('" + lNewValue.toLatin1() + "' USING latin1)";
    }
    else
        lNewValue = "'"+lNewValue+"'";

    return lNewValue;
}
} // END DAPlugin
} // END GS

