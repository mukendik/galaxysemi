#include <QSqlRecord>
#include <QHostInfo>
#include <QDir>
#include <QSqlError>
#include <QSqlDriver>
#include <QApplication>
#include <time.h>
#include "gexdb_plugin_base.h"
#include <gqtl_log.h>

#define GEXDB_ORACLE8_FRIENDLYNAME	"Oracle (QOCI8)"
#define GEXDB_ORACLE_FRIENDLYNAME	"Oracle (QOCI)"
#define GEXDB_MYSQL3_FRIENDLYNAME	"MySQL (QMYSQL3)"
#define GEXDB_SQLITE_FRIENDLYNAME	"SQLite (QSQLITE)"

int GexDbPlugin_Connector::n_instances=0;

///////////////////////////////////////////////////////////
// GexDbPlugin_Connector class: database connection...
///////////////////////////////////////////////////////////

// Error map
GBEGIN_ERROR_MAP(GexDbPlugin_Connector)
// General
GMAP_ERROR(eMarkerNotFound,"Marker not found: %s.")
// Database errors
GMAP_ERROR(eDB_Connect,"Error connecting to database %s on host (%s, %d):\n%s.")
GMAP_ERROR(eDB_CreateConnector,"Error creating database connector for driver %s, using connection name %s.")
GMAP_ERROR(eDB_NoConnection,"Lost connection to database %s.")
GMAP_ERROR(eDB_NoFields,"Table %s has no fields.\n(host=%s, port=%d).")
GMAP_ERROR(eDB_InvalidTable,"Table %s does not exist.\n(host=%s, port=%d).")
GMAP_ERROR(eDB_InvalidField,"Field %s does not exist in table %s.\n(host=%s, port=%d).")
GMAP_ERROR(eDB_Query,"Error executing SQL query.\nQUERY=%s\nERROR=%s")
GEND_ERROR_MAP(GexDbPlugin_Connector)

///////////////////////////////////////////////////////////
// Constructor / Destructor / Operators
///////////////////////////////////////////////////////////
// Constructor 1
GexDbPlugin_Connector::GexDbPlugin_Connector(const QString & strPluginName, GexDbPlugin_Base *pPluginBase/*=NULL*/): m_pPluginBase(pPluginBase)
{
    n_instances++;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" now %1 instances").
          arg(n_instances).toLatin1().constData());
    m_strPluginName = strPluginName;
    m_uiPort = 0;
    m_bUseQuotesInSqlQueries = false;
    m_bAdminUser = false;
    m_bConnectedAsAdmin = false;
    m_bCustomerDebugMode = false;
}

// Constructor 2
GexDbPlugin_Connector::GexDbPlugin_Connector(const QString & strPluginName, bool bCustomerDebugMode, const QString & strUserProfile): m_pPluginBase(NULL)
{
    n_instances++;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" now %1 instances").
          arg(n_instances).toLatin1().constData());
    m_strPluginName = strPluginName;
    m_uiPort = 0;
    m_bUseQuotesInSqlQueries = false;
    m_bAdminUser = false;
    m_bConnectedAsAdmin = false;
    m_nConnectionSID = 0;
    m_bCustomerDebugMode = bCustomerDebugMode;
    m_strUserProfile = strUserProfile;
}

// Copy constructor
GexDbPlugin_Connector::GexDbPlugin_Connector(const GexDbPlugin_Connector& source)
{
    n_instances++;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" now %1 instances").
          arg(n_instances).toLatin1().constData());
    *this = source;
}

// Destructor
GexDbPlugin_Connector::~GexDbPlugin_Connector()
{
    n_instances--;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" still %1 instances").
          arg(n_instances).toLatin1().constData());

    OpenSqlConnectionThread *pThread;
    while(!m_lstOpenSqlThread.isEmpty())
    {
        pThread = m_lstOpenSqlThread.takeLast();
        if(pThread->isRunning())
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString(" OpenSqlConnectionThread for %1 still alive").
                  arg(this->m_strConnectionName).toLatin1().constData());
            pThread->WaitForFinished();
        }
        delete pThread;
    }

    Disconnect();
    if(!m_strConnectionName.simplified().isEmpty() && QSqlDatabase::contains(m_strConnectionName))
    {
        // Remove the ConnectionName
        // And also all derived
        QString lConnectionName;
        QStringList lConnectionList = QSqlDatabase::connectionNames();
        while(lConnectionList.count() > 0)
        {
            lConnectionName = lConnectionList.takeFirst();
            if(lConnectionName.startsWith(m_strConnectionName))
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Closing database connection[%1].").arg(lConnectionName).toLatin1().data());
                QSqlDatabase::removeDatabase(lConnectionName);
            }
        }
    }
}

// Operator =
GexDbPlugin_Connector& GexDbPlugin_Connector::operator=(const GexDbPlugin_Connector& source)
{
    m_strPluginName = source.m_strPluginName;
    m_strHost_Unresolved = source.m_strHost_Unresolved;
    m_strHost_Name = source.m_strHost_Name;
    m_strHost_IP = source.m_strHost_IP;
    m_strHost_LastUsed = source.m_strHost_LastUsed;
    m_strDriver = source.m_strDriver;
    m_strDatabaseName = source.m_strDatabaseName;
    m_strUserName = source.m_strUserName;
    m_strPassword = source.m_strPassword;
    m_strUserName_Admin = source.m_strUserName_Admin;
    m_strPassword_Admin = source.m_strPassword_Admin;
    m_strSchemaName = source.m_strSchemaName;
    m_uiPort = source.m_uiPort;
    m_bUseQuotesInSqlQueries = source.m_bUseQuotesInSqlQueries;
    m_bAdminUser = source.m_bAdminUser;
    m_bConnectedAsAdmin = source.m_bConnectedAsAdmin;
    m_bCustomerDebugMode = source.m_bCustomerDebugMode;
    m_strUserProfile = source.m_strUserProfile;
    // Added recently. Since having some crashes, try to remove to check if could be the cause of the crashes
    m_pPluginBase = source.m_pPluginBase;

    return *this;
}

///////////////////////////////////////////////////////////
// Disconnect connection to database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::Disconnect(void)
{
    // Closes link to database
    if(!m_strConnectionName.simplified().isEmpty() && QSqlDatabase::contains(m_strConnectionName))
    {
        // Following line has been removed, because it is causing unpredictable crashes in QT
        if(QSqlDatabase::database(m_strConnectionName,false).isOpen())
        {
            QSqlDatabase::database(m_strConnectionName,false).close();
            GSLOG(SYSLOG_SEV_WARNING, QString("Closing database connection[%1].").arg(m_strConnectionName).toLatin1().data());
            m_nConnectionSID = 0;
        }
    }

    // Success
    return true;
}

// Do not test connection each time
// Only test connection every 1s
#define TEST_CONNECTION_INTERVAL   1000    // Min interval between processEvents calls (milliseconds)
///////////////////////////////////////////////////////////
// Check if connected to database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::IsConnected(void)
{
    if(!m_strConnectionName.simplified().isEmpty() && QSqlDatabase::contains(m_strConnectionName))
    {
        if(QSqlDatabase::database(m_strConnectionName,false).isOpen())
        {
            if(m_bAdminUser == m_bConnectedAsAdmin)
            {
                // Check interval
                if(m_LastTestConnection.elapsed() > TEST_CONNECTION_INTERVAL)
                {
                    m_LastTestConnection.start();
                    QSqlQuery clQuery(QSqlDatabase::database(m_strConnectionName));
                    clQuery.setForwardOnly(true);

                    // test the connection
                    if(clQuery.exec("SELECT 1 FROM dual"))
                        return true;
                }
                else
                    return true;
            }
            Disconnect();
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// Unique number from SQL(Oracle/MySql) connection
///////////////////////////////////////////////////////////
int GexDbPlugin_Connector::GetConnectionSID()
{
    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(m_pPluginBase, QSqlDatabase::database(m_strConnectionName));

    int	nConnectionSID = 0;
    if(IsOracleDB())
    {
        // Get the current SID connection given by Oracle
        //strQuery = "SELECT sid FROM v$session WHERE username = '"+m_strUserName_Admin.toUpper()+"' AND status='ACTIVE'";
        lQuery = "SELECT sid FROM v$session s, v$process p WHERE s.paddr = p.addr AND s.audsid = userenv('sessionid') AND s.status='ACTIVE'";
        if(!clGexDbQuery.Execute(lQuery) || !clGexDbQuery.First())
        {
            GSLOG(SYSLOG_SEV_ERROR, "GexDbPlugin_Connector::GetConnectionSID(): no SID found!");
            return nConnectionSID;
        }

        nConnectionSID = clGexDbQuery.value(0).toInt();
    }
    else if(IsMySqlDB())
    {
        // Get the current SID connection given by MySQL
        lQuery = "SELECT CONNECTION_ID()";
        if(!clGexDbQuery.Execute(lQuery) || !clGexDbQuery.First())
        {
            GSLOG(SYSLOG_SEV_ERROR, "GexDbPlugin_Connector::GetConnectionSID(): no SID found!");
            return nConnectionSID;
        }

        nConnectionSID = clGexDbQuery.value(0).toInt();
    }

    m_nConnectionSID = nConnectionSID;

    return nConnectionSID;
}

///////////////////////////////////////////////////////////
// HostName fron SQL connection
///////////////////////////////////////////////////////////
QString GexDbPlugin_Connector::GetConnectionHostName()
{
    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(m_pPluginBase, QSqlDatabase::database(m_strConnectionName));

    QString	lConnectionHostName;

    // Get the current SID connection given by MySQL
    lQuery = "SELECT @@hostname";
    if(!clGexDbQuery.Execute(lQuery) || !clGexDbQuery.First())
    {
        GSLOG(SYSLOG_SEV_ERROR, "GexDbPlugin_Connector::GetConnectionHostName(): no HostName found!");
        return lConnectionHostName;
    }

    lConnectionHostName = clGexDbQuery.value(0).toString();

    m_strConnectionHostName = lConnectionHostName;

    return lConnectionHostName;
}

///////////////////////////////////////////////////////////
// Retrieves the list of tables available in database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::EnumTables(QStringList &strTables)
{
    // Check database connection
    if(!IsConnected())
    {
        GSET_ERROR1(GexDbPlugin_Connector, eDB_NoConnection, NULL, m_strDatabaseName.toLatin1().constData());
        return false;
    }

    // Get list of valid tables in external database
    if(IsOracleDB())
    {
        QString strQuery = "SELECT TABLE_NAME FROM sys.USER_TABLES";
        GexDbPlugin_Query	clGexDbQuery(m_pPluginBase, QSqlDatabase::database(m_strConnectionName));
        if(!clGexDbQuery.Execute(strQuery))
        {
            // Set error
            GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clGexDbQuery.Next())
        {
            // table can be stored as 'table_name' or 'owner.table_name'
            // Add the 2 syntaxes
            strTables.append(clGexDbQuery.value(0).toString());
            strTables.append(m_strUserName_Admin+"."+clGexDbQuery.value(0).toString());
        }
    }
    else
    {
        strTables = QSqlDatabase::database(m_strConnectionName).tables();
        if(strTables.isEmpty() && IsMySqlDB())
        {
            // Found a BUG in linux
            // Check directly with the query
            QString strQuery = "select table_name from information_schema.tables ";
            strQuery+= " where table_schema = '"+m_strSchemaName+"' ";
            strQuery+= " and table_type = 'BASE TABLE'";
            GexDbPlugin_Query	clGexDbQuery(m_pPluginBase, QSqlDatabase::database(m_strConnectionName));
            if(!clGexDbQuery.Execute(strQuery))
            {
                // Set error
                GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            while(clGexDbQuery.Next())
                strTables.append(clGexDbQuery.value(0).toString());
        }
    }

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Retrieves the list of fields in a given table
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::EnumFields(QString strTable,QStringList &strlFields)
{
    // Check database connection
    if(!IsConnected())
    {
        GSET_ERROR1(GexDbPlugin_Connector, eDB_NoConnection, NULL, m_strDatabaseName.toLatin1().constData());
        return false;
    }

    // Check if table exists
    QStringList strlTables;
    EnumTables(strlTables);
    if(strlTables.contains(strTable, Qt::CaseInsensitive) == 0)
    {
        QString strFullTable = m_strSchemaName + ".";
        QString	strHost = m_strHost_Name + "[" + m_strHost_IP + "]";
        strFullTable += strTable;
        GSET_ERROR3(GexDbPlugin_Connector, eDB_InvalidTable, NULL, strFullTable.toLatin1().constData(), strHost.toLatin1().constData(), m_uiPort);
        return false;
    }

    // Get list of fields in a given table of the extrenal database
    QSqlRecord clRecord = QSqlDatabase::database(m_strConnectionName).record(strTable);
    if(clRecord.isEmpty())
    {
        QString strFullTable = m_strSchemaName + ".";
        QString	strHost = m_strHost_Name + "[" + m_strHost_IP + "]";
        strFullTable += strTable;
        GSET_ERROR3(GexDbPlugin_Connector, eDB_NoFields, NULL, strFullTable.toLatin1().constData(), strHost.toLatin1().constData(), m_uiPort);
        return false;
    }

    // Populate string list with results
    for(int i = 0; i < clRecord.count(); i++)
        strlFields.append(clRecord.fieldName(i));

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Load settings from DomElt and init the calls variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::LoadSettingsFromDom(QDomElement const &node)
{
    QDomElement nodeConnection = node.firstChildElement("Connection");
    if (!nodeConnection.isNull())
    {
        if (!LoadHostFromDom(nodeConnection))
            return false;
        if (!LoadUnresolvedHostNameFromDom(nodeConnection))
            return false;
        if (!LoadHostNameFromDom(nodeConnection))
            return false;
        if (!LoadHostIPFromDom(nodeConnection))
            return false;
        if (!LoadDriverFromDom(nodeConnection))
            return false;
        if (!LoadDatabaseNameFromDom(nodeConnection))
            return false;
        if (!LoadUserNameFromDom(nodeConnection))
            return false;
        if (!LoadUserPasswordFromDom(nodeConnection))
            return false;
        if (!LoadUserName_AdminFromDom(nodeConnection))
            return false;
        if (!LoadUserPassword_AdminFromDom(nodeConnection))
            return false;
        if (!LoadSchemaNameFromDom(nodeConnection))
            return false;
        if (!LoadPortFromDom(nodeConnection))
            return false;
    }
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<Connection>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadHostFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("Host");
    if (!elt.isNull())
        m_strHost_Unresolved = m_strHost_Name = m_strHost_IP = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<Host>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadUnresolvedHostNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("UnresolvedHostName");
    if (!elt.isNull())
        m_strHost_Unresolved = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<UnresolvedHostName>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadHostNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("HostName");
    if (!elt.isNull())
        m_strHost_Name = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<HostName>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadHostIPFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("HostIP");
    if (!elt.isNull())
        m_strHost_IP = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<HostIP>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadDriverFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("Driver");
    if (!elt.isNull())
        m_strDriver = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<Driver>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadDatabaseNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("DatabaseName");
    if (!elt.isNull())
        m_strDatabaseName = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<DatabaseName>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadUserNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("UserName");
    if (!elt.isNull())
    {
        m_strUserName = elt.text();
        // In case the plugin has no Admin user defined, populate admin user as standard user
        if(m_strUserName_Admin.isEmpty())
            m_strUserName_Admin = m_strUserName;
    }
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<UserName>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadUserPasswordFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("UserPassword");
    if (!elt.isNull())
    {
        // Decrypt password
#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        //GSLOG(SYSLOG_SEV_EMERGENCY, QString("Decrypting password from string: %1").arg(strParameter));
#endif
        GexDbPlugin_Base::DecryptPassword(elt.text(), m_strPassword);
        // In case the plugin has no Admin user defined, populate admin user as standard user
        if(m_strPassword_Admin.isEmpty())
            m_strPassword_Admin = m_strPassword;
    }
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<UserPassword>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadUserName_AdminFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("UserName_Admin");
    if (!elt.isNull())
    {
        // In case the plugin has no Admin user defined, populate admin user as standard user
        if(!elt.text().isEmpty())
            m_strUserName_Admin = elt.text();
    }
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<UserName_Admin>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadUserPassword_AdminFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("UserPassword_Admin");
    if (!elt.isNull())
    {
        // In case the plugin has no Admin user defined, populate admin user as standard user
        if(!elt.text().isEmpty())
        {
            // Decrypt password
#ifdef QT_DEBUG
#ifdef CASE4882
            // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
            // DO NOT ADD THIS GSLOG in RELEASE VERSION
            GSLOG(SYSLOG_SEV_DEBUG, QString("Decrypting password from string: %1").arg(elt.text()));
#endif
#endif
            GexDbPlugin_Base::DecryptPassword(elt.text(), m_strPassword_Admin);
        }
    }
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<UserPassword_Admin>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadSchemaNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("SchemaName");
    if (!elt.isNull())
        m_strSchemaName = elt.text();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<SchemaName>");
        return false;
    }

    return true;
}

bool GexDbPlugin_Connector::LoadPortFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("Port");
    if (!elt.isNull())
        m_uiPort = elt.text().toInt();
    else
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<Port>");
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////
// Load settings from file and init the calls variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::LoadSettings(QFile *pSettingsFile)
{
    // Assign file I/O stream
    QString		strString;
    QString		strKeyword;
    QString		strParameter;
    QTextStream hFile(pSettingsFile);

    GSLOG(SYSLOG_SEV_DEBUG, pSettingsFile->fileName().toLatin1().data() );

    m_strSettingsFile = pSettingsFile->fileName();

    // Rewind file first
    pSettingsFile->reset();

    // Search for marker used by this object's settings
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(strString.toLower() == "<connection>")
            break;
    }
    if(hFile.atEnd())
    {
        GSET_ERROR1(GexDbPlugin_Connector, eMarkerNotFound, NULL, "<Connection>");
        return false;	// Failed reading file.
    }

    while(!hFile.atEnd())
    {
        // Read line.
        strString = hFile.readLine().trimmed();
        strKeyword = strString.section('=',0,0);
        strParameter = strString.section('=',1);

        if(strString.toLower() == "</connection>")
            break;

        if(strKeyword.toLower() == "host")
        {
            m_strHost_Unresolved = m_strHost_Name = m_strHost_IP = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "unresolvedhostname")
        {
            m_strHost_Unresolved = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "hostname")
        {
            m_strHost_Name = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "hostip")
        {
            m_strHost_IP = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "driver")
        {
            m_strDriver = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "databasename")
        {
            m_strDatabaseName = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "username")
        {
            m_strUserName = strParameter;
            // In case the plugin has no Admin user defined, populate admin user as standard user
            if(m_strUserName_Admin.isEmpty())
                m_strUserName_Admin=m_strUserName;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "userpassword")
        {
            // Decrypt password
#ifdef QT_DEBUG
            // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
            // DO NOT ADD THIS GSLOG in RELEASE VERSION
            //GSLOG(SYSLOG_SEV_EMERGENCY, QString("Decrypting password from string: %1").arg(strParameter));
#endif
            GexDbPlugin_Base::DecryptPassword(strParameter, m_strPassword);
            // In case the plugin has no Admin user defined, populate admin user as standard user
            if(m_strPassword_Admin.isEmpty())
                m_strPassword_Admin=m_strPassword;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "username_admin")
        {
            // In case the plugin has no Admin user defined, populate admin user as standard user
            if(!strParameter.isEmpty())
                m_strUserName_Admin = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "userpassword_admin")
        {
            // In case the plugin has no Admin user defined, populate admin user as standard user
            if(!strParameter.isEmpty())
            {
                // Decrypt password
#ifdef QT_DEBUG
                // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
                // DO NOT ADD THIS GSLOG in RELEASE VERSION
#ifdef CASE4882
                GSLOG(SYSLOG_SEV_DEBUG, QString("Decrypting password from string: %1").arg(strParameter));
#endif
#endif
                GexDbPlugin_Base::DecryptPassword(strParameter, m_strPassword_Admin);
            }
            goto next_line_1;
        }

        if(strKeyword.toLower() == "schemaname")
        {
            m_strSchemaName = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "port")
        {
            m_uiPort = strParameter.toInt();
            goto next_line_1;
        }

next_line_1:
        strString = "";
    }

    // Success
    return true;
}

QDomElement GexDbPlugin_Connector::GetSettingsDom(QDomDocument &doc)
{
    QDomElement domConnection = doc.createElement("Connection");

    domConnection.appendChild(GetHostDom(doc));
    domConnection.appendChild(GetUnresolvedHostNameDom(doc));
    domConnection.appendChild(GetHostNameDom(doc));
    domConnection.appendChild(GetHostIPDom(doc));
    domConnection.appendChild(GetDriverDom(doc));
    domConnection.appendChild(GetDatabaseNameDom(doc));
    domConnection.appendChild(GetUserNameDom(doc));
    domConnection.appendChild(GetUserPasswordDom(doc));
    domConnection.appendChild(GetUserName_AdminDom(doc));
    domConnection.appendChild(GetUserPassword_AdminDom(doc));
    domConnection.appendChild(GetSchemaNameDom(doc));
    domConnection.appendChild(GetPortDom(doc));

    return domConnection;
}

QDomElement   GexDbPlugin_Connector::GetHostDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Host");
    // For compatibility with B157 (not supporting HostName/HostIP, only Host)
    if(!m_strHost_Name.isEmpty())
        domText = doc.createTextNode(m_strHost_Name);
    else
        domText = doc.createTextNode(m_strHost_IP);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetUnresolvedHostNameDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("UnresolvedHostName");
    domText = doc.createTextNode(m_strHost_Unresolved);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetHostNameDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("HostName");
    domText = doc.createTextNode(m_strHost_Name);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetHostIPDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("HostIP");
    domText = doc.createTextNode(m_strHost_IP);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetDriverDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Driver");
    domText = doc.createTextNode(m_strDriver);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetDatabaseNameDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("DatabaseName");
    domText = doc.createTextNode(m_strDatabaseName);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetUserNameDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("UserName");
    domText = doc.createTextNode(m_strUserName);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetUserPasswordDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("UserPassword");

    // Encrypt passwords
    QString strCryptedPassword = "";
    if(!m_strPassword.isEmpty())
    {
#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        // Please do not rape the logs levels : if you have special logs :
        //  1 add a DEFINES += BERNARD_GARROS in your user_custom.pri
        //  2 hide your logs in an ifdef
#ifdef BERNARD_GARROS
        GSLOG(SYSLOG_SEV_EMERGENCY,
              QString("Crypting password %1 for user %2").arg(m_strPassword).arg(m_strUserName));
#endif
#endif
        GexDbPlugin_Base::CryptPassword(m_strPassword, strCryptedPassword);
    }

    domText = doc.createTextNode(strCryptedPassword);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetUserName_AdminDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("UserName_Admin");
    domText = doc.createTextNode(m_strUserName_Admin);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetUserPassword_AdminDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("UserPassword_Admin");

    QString strCryptedPassword_Admin = "";
    if(!m_strPassword_Admin.isEmpty())
    {
#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
#ifdef CASE4882
        GSLOG(SYSLOG_SEV_EMERGENCY, QString("Crypting password %1 for user %2").arg(m_strPassword_Admin).arg(m_strUserName_Admin));
#endif
#endif
        GexDbPlugin_Base::CryptPassword(m_strPassword_Admin, strCryptedPassword_Admin);
    }
    domText = doc.createTextNode(strCryptedPassword_Admin);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetSchemaNameDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("SchemaName");
    domText = doc.createTextNode(m_strSchemaName);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement   GexDbPlugin_Connector::GetPortDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Port");
    domText = doc.createTextNode(QString::number(m_uiPort));
    domElt.appendChild(domText);

    return domElt;
}


///////////////////////////////////////////////////////////
// Write settings to file using informations loaded in the class variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::WriteSettings(QTextStream *phFile)
{
    // Encrypt passwords
    QString strCryptedPassword = "";
    if(!m_strPassword.isEmpty())
    {
#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        // Please do not rape the logs levels : if you have special logs :
        //  1 add a DEFINES += BERNARD_GARROS in your user_custom.pri
        //  2 hide your logs in an ifdef
#ifdef BERNARD_GARROS
        GSLOG(SYSLOG_SEV_EMERGENCY,
              QString("Crypting password %1 for user %2").arg(m_strPassword).arg(m_strUserName));
#endif
#endif
        GexDbPlugin_Base::CryptPassword(m_strPassword, strCryptedPassword);
    }
    QString strCryptedPassword_Admin = "";
    if(!m_strPassword_Admin.isEmpty())
    {
#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
#ifdef CASE4882
        GSLOG(SYSLOG_SEV_EMERGENCY, QString("Crypting password %1 for user %2").arg(m_strPassword_Admin).arg(m_strUserName_Admin));
#endif
#endif
        GexDbPlugin_Base::CryptPassword(m_strPassword_Admin, strCryptedPassword_Admin);
    }

    // Fill file with Database details...
    *phFile << endl;
    *phFile << "<Connection>" << endl;
    // For compatibility with B157 (not supporting HostName/HostIP, only Host)
    if(!m_strHost_Name.isEmpty())
        *phFile << "Host=" << m_strHost_Name << endl;
    else
        *phFile << "Host=" << m_strHost_IP << endl;
    *phFile << "UnresolvedHostName=" << m_strHost_Unresolved << endl;
    *phFile << "HostName=" << m_strHost_Name << endl;
    *phFile << "HostIP=" << m_strHost_IP << endl;
    *phFile << "Driver=" << m_strDriver << endl;
    *phFile << "DatabaseName=" << m_strDatabaseName << endl;
    *phFile << "UserName=" << m_strUserName << endl;
    *phFile << "UserPassword=" << strCryptedPassword << endl;
    *phFile << "UserName_Admin=" << m_strUserName_Admin << endl;
    *phFile << "UserPassword_Admin=" << strCryptedPassword_Admin << endl;
    if (!IsSQLiteDB())
        *phFile << "SchemaName=" << m_strSchemaName << endl;
    *phFile << "Port=" << m_uiPort << endl;
    *phFile << "</Connection>" << endl;

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Updates field mapping
// o every field in the map must exist in the database (otherwise it is removed)
// o every field in the map must exist in the list of Examinator fields (otherwise it is removed)
// o fields from the database that are not in the map are added
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::UpdateMapping(int *pnItemsRemoved, int *pnItemsAdded, const QString & strTableName, const QStringList & strlGexFields, QMap<QString, QString> & cFieldMapping)
{
    // Init counters
    *pnItemsRemoved = *pnItemsAdded = 0;

    // Get list of fields
    QStringList strlFields;
    if(!EnumFields(strTableName, strlFields))
        return false;

    // Check if items in map exist in database and in list of Examinator fields
    QMap<QString,QString>::Iterator itMap = cFieldMapping.begin();
    QMap<QString,QString>::Iterator itMapTemp;
    QString							strKey, strData;
    while(itMap != cFieldMapping.end())
    {
        strKey = itMap.key();
        strData = itMap.value();
        if(strKey.isEmpty() || (strlFields.contains(strKey) == 0))
        {
            // Store iterator in temp variable and increment it
            itMapTemp = itMap++;

            // Remove item from map
            cFieldMapping.erase(itMapTemp);
            (*pnItemsRemoved)++;
        }
        else if(!strData.isEmpty() && (strlGexFields.contains(strData) == 0))
        {
            // Store iterator in temp variable and increment it
            itMapTemp = itMap++;

            // Remove item from map
            cFieldMapping.erase(itMapTemp);
            (*pnItemsRemoved)++;
        }
        else
            itMap++;
    }

    // Add fields from database that are not in the map
    for (QStringList::Iterator itDbFields = strlFields.begin(); itDbFields != strlFields.end(); ++itDbFields)
    {
        if(!cFieldMapping.contains(*itDbFields))
        {
            cFieldMapping[*itDbFields] = "";
            (*pnItemsAdded)++;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Set the NLS parameters for this session to be compatible with the OS regional settings
// to ensure that conversion on fields retrieved through SQL queries work properly
// (ie double lfValue = clQuery..value(5).toDouble())
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::SetNlsParameters()
{
    // Check OS regional settings and set NLS parameters (country, territory) depending on OS regional settings
    QLocale				clLocal = QLocale::system();
    QLocale::Language	eLanguage = clLocal.language();
    QLocale::Country	eCountry = clLocal.country();
    QString				strNLS_Language, strNLS_Territory;

    switch(eLanguage)
    {
    case QLocale::French:
        strNLS_Language = "FRENCH";
        break;

    case QLocale::English:
        if(eCountry == QLocale::UnitedStates)
            strNLS_Language = "AMERICAN";
        else
            strNLS_Language = "ENGLISH";
        break;

    default:
        strNLS_Language = "ENGLISH";
        break;
    }
    switch(eCountry)
    {
    case QLocale::France:
        strNLS_Territory = "FRANCE";
        break;

    case QLocale::UnitedStates:
        strNLS_Territory = "AMERICA";
        break;

    case QLocale::UnitedKingdom:
        strNLS_Territory = "UNITED KINGDOM";
        break;

    default:
        strNLS_Territory = "AMERICA";
        break;
    }

    // Set NLS parameters (country, territory) depending on OS regional settings
    QString		strQuery;
    GexDbPlugin_Query	clGexDbQuery(m_pPluginBase, QSqlDatabase::database(m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    // NLS Language
    strQuery = "alter session set NLS_LANGUAGE='";
    strQuery += strNLS_Language;
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Set error
        GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    // Dump query performance in debug mode
    clGexDbQuery.DumpPerformance();

    // NLS Territory
    strQuery = "alter session set NLS_TERRITORY='";
    strQuery += strNLS_Territory;
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Set error
        GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    // Dump query performance in debug mode
    clGexDbQuery.DumpPerformance();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Returns true if connected to an Oracle DB
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::IsOracleDB() const
{
    return ((m_strDriver == "QOCI8") || (m_strDriver == "QOCI"));
}

//////////////////////////////////////////////////////////////////////
// Returns true if connected to a MySQL DB
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::IsMySqlDB() const
{
    return (m_strDriver == "QMYSQL3");
}

QString	GexDbPlugin_Connector::GetSQLiteDriverVersion()
{
    // retrieve engine version
    QVariant v = QSqlDatabase::database(m_strConnectionName).driver()->handle();
    if ( !v.isValid() )
    {
        GSLOG(SYSLOG_SEV_ERROR, " sqlite handle qvariant not valid ");
        return QString("?");
    }

    if ( strcmp(v.typeName(), "sqlite3*") != 0 )
    {
        GSLOG(SYSLOG_SEV_ERROR, QString(" wrong handle type name = %1").arg(v.typeName()).toLatin1().data() ) ;
        return QString("?");
    }

    // v.data() returns a pointer to the handle
    sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
    if (handle == 0)
    {
        GSLOG(SYSLOG_SEV_ERROR, "cant get sqlite3 handle");
        return QString("?");
    }

    const char* v2 = sqlite3_libversion();
    if (!v2)
    {
        GSLOG(SYSLOG_SEV_ERROR, "failed to get SQLite version ");
        return QString("?");
    }

    //qDebug( QString("GexDbPlugin_Galaxy::InsertDataFile: SQLite version: %1").arg( v2 ));
    return QString(v2);
}

bool	GexDbPlugin_Connector::IsSQLiteDB()
{
    return (m_strDriver == "QSQLITE");
}

bool GexDbPlugin_Connector::IsSQLiteDB(QString & strDatabaseDriver)
{
    return (strDatabaseDriver == "QSQLITE");
}

//////////////////////////////////////////////////////////////////////
// Returns true if connected to an Oracle DB
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::IsOracleDB(QString & strDatabaseDriver)
{
    return ((strDatabaseDriver == "QOCI8") || (strDatabaseDriver == "QOCI"));
}


///////////////////////////////////////////////////////////
// Sets ptr to plugin base (useful if connector not the private plugin base connector)
///////////////////////////////////////////////////////////
void GexDbPlugin_Connector::SetPluginBasePtr(GexDbPlugin_Base *pPluginBase)
{
    m_pPluginBase = pPluginBase;
}

///////////////////////////////////////////////////////////
// Set db login mode (std user/admin user)
///////////////////////////////////////////////////////////
void GexDbPlugin_Connector::SetAdminLogin(bool bAdminLogin)
{
    m_bAdminUser = bAdminLogin;
}

///////////////////////////////////////////////////////////
// Return list of valid database drivers on current computer
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::EnumDrivers(QStringList & strlDrivers, QStringList & strlFriendlyNames)
{
    strlDrivers.clear();
    strlFriendlyNames.clear();

    // Fill list of drivers available...
    strlDrivers = QSqlDatabase::drivers();

    QString dnames;
    dnames.append(" drivers found: ");
    int i=0;
    for (i=0; i<strlDrivers.count(); i++)
        dnames.append(QString(", ").append(strlDrivers.at(i)));
    GSLOG(SYSLOG_SEV_DEBUG, dnames.toLatin1().data() );

    // Generate friendly names
    QString strDriver;
    for (QStringList::Iterator it = strlDrivers.begin(); it != strlDrivers.end(); it++, i++)
    {
        strDriver = *it;
        if (strDriver == "QOCI8")
            strlFriendlyNames += GEXDB_ORACLE8_FRIENDLYNAME;
        else if (strDriver == "QOCI")
            strlFriendlyNames += GEXDB_ORACLE_FRIENDLYNAME;
        else if (strDriver == "QMYSQL3")
            strlFriendlyNames += GEXDB_MYSQL3_FRIENDLYNAME;
#ifdef GEX_SQLITE_SUPPORT
        else if(strDriver == "QSQLITE")
            strlFriendlyNames += GEXDB_SQLITE_FRIENDLYNAME;
#endif
    }

    dnames.clear();
    dnames.append("GexDbPlugin_Connector:: supported drivers : ");
    for (i=0; i<strlFriendlyNames.count(); i++)
        dnames.append(QString(", ").append(strlFriendlyNames.at(i)));
    GSLOG(SYSLOG_SEV_INFORMATIONAL, dnames.toLatin1().data());
    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Return driver name corresponding to specified friendly name
///////////////////////////////////////////////////////////
QString GexDbPlugin_Connector::GetDriverName(const QString& strFriendlyName)
{
    QString strDriverName = "";

    if(strFriendlyName == GEXDB_ORACLE8_FRIENDLYNAME)
        strDriverName = "QOCI8";
    else if(strFriendlyName == GEXDB_ORACLE_FRIENDLYNAME)
        strDriverName = "QOCI";
    else if(strFriendlyName == GEXDB_MYSQL3_FRIENDLYNAME )
        strDriverName = "QMYSQL3";
#ifdef GEX_SQLITE_SUPPORT
    else if(strFriendlyName == GEXDB_SQLITE_FRIENDLYNAME )
        strDriverName = "QSQLITE";
#endif

    return strDriverName;
}

///////////////////////////////////////////////////////////
// Return friendly name corresponding to specified driver name
///////////////////////////////////////////////////////////
QString GexDbPlugin_Connector::GetFriendlyName(QString & strDriverName)
{
    QString strFriendlyName = "";

    if(strDriverName == "QOCI8")
        strFriendlyName = GEXDB_ORACLE8_FRIENDLYNAME;
    if(strDriverName == "QOCI")
        strFriendlyName = GEXDB_ORACLE_FRIENDLYNAME;
    else if(strDriverName == "QMYSQL3")
        strFriendlyName = GEXDB_MYSQL3_FRIENDLYNAME;
#ifdef GEX_SQLITE_SUPPORT
    else if(strDriverName == "QSQLITE")
        strFriendlyName = GEXDB_SQLITE_FRIENDLYNAME;
#endif

    return strFriendlyName;
}

///////////////////////////////////////////////////////////
// Resolve host name
// Make sure host name/IP are set correctly
// Resolved the UserHostName with result in hostName and hostIP
// Return the list of IP found
///////////////////////////////////////////////////////////
QStringList GexDbPlugin_Connector::ResolveHostName(QString UserHostName,
                               QString& hostName,
                               QString& hostIP,
                               bool *pbIsLocalHost)
{
    QStringList lIPs;
    QString lOriginalHostName = UserHostName.simplified();
    hostName = hostIP = "";

    // Consider host is not local, will be updated at the end of resolve
    *pbIsLocalHost = false;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Resolving host name for host %1...").arg(lOriginalHostName).toLatin1().data());

    // Check if HostName is in fact an IP address
    QHostAddress clHostAddress(lOriginalHostName);
    if(clHostAddress.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
    {
        // IP address: save IP address, and resolved host name
        hostIP = hostName = lOriginalHostName;
        lIPs << hostIP;
        QHostInfo clHostInfo = QHostInfo::fromName(lOriginalHostName);
        if(clHostInfo.error() == QHostInfo::NoError)
            hostName = clHostInfo.hostName();
    }
    else
    {
        // Host Name: try to resolve it to get IP
        QHostInfo clHostInfo = QHostInfo::fromName(lOriginalHostName);

        // succeeded in getting FQDN infos
        if(clHostInfo.error() == QHostInfo::NoError)
        {
            hostName = clHostInfo.hostName();

            // In case HostName still "localhost"
            if(hostName.toLower() == "localhost")
            {
                // GCORE-10191 : attempt to get the hostname of the machine
                hostName = QHostInfo::localHostName();

                // and associated IPs
                clHostInfo = QHostInfo::fromName(hostName);

                // attempt failed, back to previous FQDN name found
                if(clHostInfo.error() != QHostInfo::NoError)
                {
                    clHostInfo = QHostInfo::fromName(lOriginalHostName);
                }

                // Get first IPV4 address
                for(int i = 0; i < clHostInfo.addresses().size(); ++i)
                {
                    clHostAddress = clHostInfo.addresses().at(i);
                    if(clHostAddress.protocol() == QAbstractSocket::IPv4Protocol)
                    {
                        if(hostIP.isEmpty())
                            hostIP = clHostAddress.toString();
                        lIPs << clHostAddress.toString();
                    }
                }
            }
        }
    }

    // Check if localhost
    if(hostName.toLower() == "localhost" || hostName.startsWith("127.") || hostName.toLower() == QHostInfo::localHostName().toLower())
        *pbIsLocalHost = true;
    if(hostIP.toLower() == "localhost" || hostIP.startsWith("127.") || hostIP.toLower() == QHostInfo::localHostName().toLower())
        *pbIsLocalHost = true;

    if(*pbIsLocalHost==false)
    {
        // Get local host info
        QHostInfo clLocalHostInfo = QHostInfo::fromName(QHostInfo::localHostName());
        if(clLocalHostInfo.error() == QHostInfo::NoError)
        {
            QHostAddress clHostAddress(QHostInfo::localHostName());
            // Get first IPV4 address
            for(int i = 0; i < clLocalHostInfo.addresses().size(); ++i)
            {
                clHostAddress = clLocalHostInfo.addresses().at(i);
                if(clHostAddress.protocol() == QAbstractSocket::IPv4Protocol)
                {
                    if(clHostAddress.toString()==hostIP)
                        *pbIsLocalHost = true;
                }
            }
        }
        if(*pbIsLocalHost)
            hostName = QHostInfo::localHostName();
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Original HostName  = %1...").arg(UserHostName).toLatin1().data() );
    GSLOG(SYSLOG_SEV_DEBUG, QString("Resolved HostName  = %1...").arg(hostName).toLatin1().data() );
    GSLOG(SYSLOG_SEV_DEBUG, QString("Resolved HostIP    = %1...").arg(hostIP).toLatin1().data() );
    GSLOG(SYSLOG_SEV_DEBUG, *pbIsLocalHost ? QString("LocalHost = YES").toLatin1().data() : QString("LocalHost = NO").toLatin1().data() );

    return lIPs;
}

///////////////////////////////////////////////////////////
// Open Database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::OpenDatabase()
{
    OpenSqlConnectionThread* pThread;
    QString         strHostName, strDbError;
    QStringList lHostList;

    {// { } to keep QSQLDatabase out of scope
        QSqlDatabase clSqlDatabase = QSqlDatabase::database(m_strConnectionName,false);

        // First
        // If already resolved and connected with success,  try with m_strHost_LastUsed
        if(!m_strHost_LastUsed.isEmpty())
        {
            // Try with last Host used
            strHostName = m_strHost_LastUsed;
            GSLOG(SYSLOG_SEV_DEBUG, QString("Trying with hostname '%1'...").arg(strHostName).toLatin1().data() );
            clSqlDatabase.setHostName(strHostName);
            if(OpenConnection(m_strConnectionName))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("SUCCESS").toLatin1().data() );
                return true;
            }
            else
                GSLOG(SYSLOG_SEV_DEBUG, QString("Failed using '%1'").arg(strHostName).toLatin1().data() );
        }
        m_strHost_LastUsed = "";
        m_bIsLocalHost = false;

        // Then
        // Try to resolve HostName
        // Stored config = SC_HOST[192.168.1.11]localhost => 192.168.1.11 Ethernet card
        // Resolved under linux => SC_HOST[192.168.1.47] => 192.168.1.47 Wifi card
        // Try first with the original resolved IP
        // If error try with the new IP

        // Make sure host name/IP are set correctly
        // First resolve the HostName
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // ResolveHostName doesn't update m_strHost_Name and m_strHost_IP any more
        // The cfgWizard must Resolve the UseHost before calling the connect()
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        QString lResolvedHostName, lResolvedHostIP;
        QStringList lIPs = ResolveHostName(m_strHost_Name,lResolvedHostName,lResolvedHostIP, &m_bIsLocalHost);
        if(lResolvedHostName.isEmpty() && lResolvedHostIP.isEmpty())
        {
            // This is not an issue !
            GSLOG(SYSLOG_SEV_WARNING, QString("Failed resolving Host name '%1'").arg(m_strHost_Name).toLatin1().data() );
        }

        if(m_pPluginBase)
        {
            // Check if have some Open threads running
            for(int ii=m_lstOpenSqlThread.count(); ii>0; ii--)
            {
                pThread = m_lstOpenSqlThread.at(ii-1);
                if(pThread == NULL)
                    continue;

                // If the thread running until a long time
                // accept to start a new thread
                // else reject
                if(pThread->isRunning() || !pThread->isFinished())
                {
                    if(pThread->GetTimerElapsed() > 30000)
                        continue;

                    return false;
                }

                // Thread terminated
                // Do some cleanup
                delete m_lstOpenSqlThread.takeLast();
            }
        }

        if(IsMySqlDB())
        {
            //  void QSqlDatabase::setConnectOptions ( const QString & options)
            //  options available to MySql are:
            //  CLIENT_COMPRESS
            //  CLIENT_FOUND_ROWS
            //  CLIENT_IGNORE_SPACE
            //  CLIENT_SSL
            //  CLIENT_ODBC
            //  CLIENT_NO_SCHEMA
            //  CLIENT_INTERACTIVE
            //  UNIX_SOCKET
            //  MYSQL_OPT_RECONNECT
            //  QString lConnectOptions = "CLIENT_REMEMBER_OPTIONS=1;CLIENT_LOCAL_FILES=1";
            // case 4965
            // Setup TCP Packet Compression in MySQL
            // when connection is not on localhost
            if(!m_bIsLocalHost)
            {
              QString lConnectOptions = "CLIENT_COMPRESS=1";
              clSqlDatabase.setConnectOptions(lConnectOptions);
            }
        }

        // Try connecting in this order:
        // 1. localhost (if on localhost)
        // 2. 127.0.0.1 (if on localhost)
        // 3. Host Name
        // 4. Host IP
        // 5. Unresolved host name
        QStringList lHostList;
        if(m_bIsLocalHost)
            lHostList << "localhost" << "127.0.0.1";
        if(!lHostList.contains(m_strHost_Name))
            lHostList << m_strHost_Name;
        if(!lHostList.contains(m_strHost_IP))
            lHostList << m_strHost_IP;
        if(!lHostList.contains(m_strHost_Unresolved))
            lHostList << m_strHost_Unresolved;
        if(!lHostList.contains(lResolvedHostName))
            lHostList << lResolvedHostName;
        if(!lHostList.contains(lResolvedHostIP))
            lHostList << lResolvedHostIP;
        // Then try with all IPs found
        foreach(const QString &lHostIP, lIPs)
        {
            if(!lHostList.contains(lHostIP))
                lHostList << lHostIP;
        }


        foreach(const QString &lHostName, lHostList)
        {
            // Try with HostIP
            strHostName = lHostName;
            GSLOG(SYSLOG_SEV_DEBUG, QString("Trying with hostname '%1'...").arg(strHostName).toLatin1().data() );
            clSqlDatabase.setHostName(strHostName);
            if(OpenConnection(m_strConnectionName))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("SUCCESS").toLatin1().data() );
                m_strHost_LastUsed = strHostName;
                return true;
            }
            else
                GSLOG(SYSLOG_SEV_DEBUG, QString("Failed using '%1'").arg(strHostName).toLatin1().data() );
        }

        // Error: set error and return false
        QSqlError clSqlError = clSqlDatabase.lastError();
        strDbError = clSqlError.text();
    }

    if(m_lstOpenSqlThread.isEmpty())
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Open DB : remove Database %1...").arg(m_strConnectionName).toLatin1().constData());
        QSqlDatabase::removeDatabase(m_strConnectionName);
    }
    else
        strDbError = "Connection timeout";

    GSLOG(SYSLOG_SEV_WARNING, QString("Open DB failed for %1").arg(m_strDatabaseName).toLatin1().data());
    GSET_ERROR4(GexDbPlugin_Connector, eDB_Connect, NULL, m_strDatabaseName.toLatin1().constData(), lHostList.join(" | ").toLatin1().constData(), m_uiPort, strDbError.toLatin1().constData());

    return false;
}

///////////////////////////////////////////////////////////
// Open database connection into a thread
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::OpenConnection(QString &strConnectionName)
{
    /*int nConnectionTimeOut = 0;
  QApplication *pApplication = NULL;
  if(m_pPluginBase)
  {
    pApplication = m_pPluginBase->m_pqApplication;
    nConnectionTimeOut = m_pPluginBase->m_nConnectionTimeOut;
  }*/

#if 1
    QSqlDatabase clDatabase = QSqlDatabase::database(strConnectionName,false);
    if(!clDatabase.isValid())
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid connection to %1: %2").
              arg(strConnectionName).
              arg(clDatabase.lastError().text()).
              toLatin1().data());

    // Execute the connection in another thread
    // Check if already connected
    bool lStatus = clDatabase.open();
    if (! lStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to open connection to %1: %2").
              arg(strConnectionName).
              arg(clDatabase.lastError().text()).
              toLatin1().data());
    }
    return lStatus;
#else

    OpenSqlConnectionThread* pThread;

    // Open the database into a Thread
    pThread = new OpenSqlConnectionThread();
    m_lstOpenSqlThread.append(pThread);
    if(pThread->exec(strConnectionName,pApplication,nConnectionTimeOut))
    {
        delete m_lstOpenSqlThread.takeLast();
        return true;
    }

    // Check if Thread finished
    if(!(pThread->isRunning() || !pThread->isFinished()))
        delete m_lstOpenSqlThread.takeLast();

    return false;
#endif
}
///////////////////////////////////////////////////////////
// Connect to dabase
///////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::Connect()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" Connecting to %1 with %2...").arg(m_strDatabaseName).arg(m_strDriver).toLatin1().data());

#ifdef unix
    // FIX QT BUG: crash when opening new connection after having removal all existing connection
    // using QSqlDatabase::removeDatabase(...)
    // With this fix we always keep at least one connection
    if (!QSqlDatabase::contains("ghost_conn") && QSqlDatabase::isDriverAvailable("QMYSQL"))
        QSqlDatabase::addDatabase("QMYSQL", "ghost_conn");
#endif

    QString		strMessage;

    if(m_strConnectionName.isEmpty())
    {
        // Generate connection ID
        m_strConnectionName = "plugin_conn:"+m_strPluginName+":"+m_strDriver+":"+QString::number(m_uiPort)+"["+m_strUserName_Admin + "@"+m_strHost_Name+"]";

        // Buils connection name
        m_strConnectionName = m_strConnectionName;

#ifdef GEX_SQLITE_SUPPORT
        //m_strConnectionName = m_strDatabaseName;
#endif
    }


    // Add database if not already done
    if(!QSqlDatabase::contains(m_strConnectionName))
    {
        // Add database link: make sure we use a unique name for the database connection
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" adding %1 %2").arg(m_strDriver, m_strConnectionName).toLatin1().constData() );
        QSqlDatabase::addDatabase(m_strDriver, m_strConnectionName);
        if(!QSqlDatabase::contains(m_strConnectionName))
        {
            // Trace message
            strMessage = "cant create database connection - %1 - %2";
            GSLOG(SYSLOG_SEV_ERROR, strMessage.arg(m_strConnectionName).arg(m_strDriver).toLatin1().data() );

            // Error: set error and return false
            GSET_ERROR2(GexDbPlugin_Connector, eDB_CreateConnector, NULL, m_strDriver.toLatin1().constData(), m_strConnectionName.toLatin1().constData());
            return false;
        }
    }

    QSqlDatabase clSqlDatabase = QSqlDatabase::database(m_strConnectionName,false);
    if(!clSqlDatabase.isValid())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid connection to %1: %2").
              arg(m_strConnectionName).
              arg(clSqlDatabase.lastError().text()).
              toLatin1().data());

        return false;
    }

    // Trace message
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        strMessage = "Connecting to database " + m_strDatabaseName;
        strMessage += " on host " + m_strHost_Name + " (" + m_strHost_IP + ")";
        strMessage += "  (port=" + QString::number(m_uiPort);
        if(m_bAdminUser)
            strMessage += ", user=" + m_strUserName_Admin;
        else
            strMessage += ", user=" + m_strUserName;
        strMessage += ", connection name=";
        strMessage += m_strConnectionName;
        strMessage += ", driver=";
        strMessage += m_strDriver;
        strMessage += ")...";
        GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data() );
    }

    // Closes connection if opened
    if(clSqlDatabase.isOpen())
        clSqlDatabase.close();

    GSLOG(SYSLOG_SEV_DEBUG, "Resetting some flags");

    // Reset some flags
    m_bUseQuotesInSqlQueries = false;
    m_strNlsNumericCharacters = "";
    m_bPartitioningDb = false;

    // Set connection paramaters
    GSLOG(SYSLOG_SEV_DEBUG, "Set connection parameter...");
    clSqlDatabase.setDatabaseName(m_strDatabaseName);
    if(m_bAdminUser)
    {
        clSqlDatabase.setUserName(m_strUserName_Admin);
        clSqlDatabase.setPassword(m_strPassword_Admin);
    }
    else
    {
        clSqlDatabase.setUserName(m_strUserName);
        clSqlDatabase.setPassword(m_strPassword);
    }

    clSqlDatabase.setPort(m_uiPort);

    if (this->IsSQLiteDB())
    {
        if(m_pPluginBase)
        {
            // With SQLite, the database to be opened must be in the current dir. Let s change it temporaly.
            //qDebug("GexDbPlugin_Connector::Connect: SQLite : app path=%s", m_pPluginBase->m_strApplicationPath.toLatin1().data() );
            //qDebug("GexDbPlugin_Connector::Connect: SQLite : LocalFolder=%s", m_pPluginBase->m_strLocalFolder.toLatin1().data() );
            qDebug("GexDbPlugin_Connector::Connect: SQLite : DBFolder = %s", m_pPluginBase->m_strDBFolder.toLatin1().data());
            if (!QDir::setCurrent( m_pPluginBase->m_strDBFolder ))
            {
                qDebug(" failed to change the current working dir !!!");
                return false;
            }
        }
        else
        {
            qDebug(" failed to change the current working dir !!!");
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, "Connecting...");



    // Open Database
    if(!OpenDatabase())
    {
        if (this->IsSQLiteDB() && m_pPluginBase)
            QDir::setCurrent( m_pPluginBase->m_strApplicationPath );

        // Fill list of drivers available...
        QStringList strlDrivers;
        strlDrivers = QSqlDatabase::drivers();
        GSLOG(SYSLOG_SEV_NOTICE, QString("Drivers found: ").append(strlDrivers.join(", ")).toLatin1().data() );

        return false;
    }

    if (this->IsSQLiteDB() && m_pPluginBase)
        QDir::setCurrent(m_pPluginBase->m_strApplicationPath );

    // Trace message
    strMessage = "...connected.";
    GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());

    GexDbPlugin_Query	clGexDbQuery(m_pPluginBase, clSqlDatabase);
    QString		strQuery;
    clGexDbQuery.setForwardOnly(true);

    // Check database type and version, and set some flags
    if(IsOracleDB())
    {
#if 0
        // Execute query to get Oracle version
        strQuery = "SELECT * from v$version where BANNER like '%Oracle%'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            // Display error
            GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());

            m_clSqlDatabase.close();
            QSqlDatabase::removeDatabase(m_clSqlDatabase);
            return false;
        }

        // Check version
        if(clGexDbQuery.Next())
        {
            QString strVersion = clGexDbQuery.value(0).toString();
            if(strVersion.contains("Oracle9i"))
                m_bUseQuotesInSqlQueries = true;

            // Dump query performance in debug mode
            clGexDbQuery.DumpPerformance();
        }
#endif

        // Execute query to get value of Oracle NLS_NUMERIC_CHARACTERS parameter
        // !!!!!!!!!! This has to be executed before function SetNlsParameters() is called !!!!!!!!!!!!!!!!!!!
        strQuery = "select VALUE from nls_session_parameters where PARAMETER='NLS_NUMERIC_CHARACTERS'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            // Set error
            GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());

            clSqlDatabase.close();
            GSLOG(SYSLOG_SEV_DEBUG, QString("QSqlDatabase::removeDatabase(%1)").arg(m_strConnectionName).toLatin1().constData());
            QSqlDatabase::removeDatabase(m_strConnectionName);

            return false;
        }

        // Save value
        if(clGexDbQuery.Next())
        {
            m_strNlsNumericCharacters = clGexDbQuery.value(0).toString();

        }

        // Set the NLS parameters for this session to be compatible with the OS regional settings
        // to ensure that conversion on fields retrieved through SQL queries work properly
        // (ie double lfValue = clGexDbQuery..value(5).toDouble())
        if(!SetNlsParameters())
        {
            clSqlDatabase.close();
            GSLOG(SYSLOG_SEV_DEBUG, QString("QSqlDatabase::removeDatabase(%1)").arg(m_strConnectionName).toLatin1().constData() );
            QSqlDatabase::removeDatabase(m_strConnectionName);

            return false;
        }

        // initialization for Database partitioning
        strQuery = "SELECT value FROM v$option WHERE parameter='Partitioning'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            // Set error
            GSET_ERROR2(GexDbPlugin_Connector, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());

            clSqlDatabase.close();
            GSLOG(SYSLOG_SEV_DEBUG, QString("QSqlDatabase::removeDatabase(%1)").arg(m_strConnectionName).toLatin1().constData() );
            QSqlDatabase::removeDatabase(m_strConnectionName);

            return false;
        }
        clGexDbQuery.First();
        m_bPartitioningDb = clGexDbQuery.value(0).toString().startsWith("true",Qt::CaseInsensitive);
        m_bTransactionDb = true;
        m_bCompressionProtocol = false;
        GetConnectionSID();
        GetConnectionHostName();

        // Set scan off to unvalidate & for define parameter
        strQuery = "SET SCAN OFF";
        if(!clGexDbQuery.Execute(strQuery))
            strMessage = clGexDbQuery.lastError().text();

        strQuery = "SET DEFINE OFF";
        if(!clGexDbQuery.Execute(strQuery))
            strMessage = clGexDbQuery.lastError().text();

        strQuery = "SET ESCAPE OFF";
        if(!clGexDbQuery.Execute(strQuery))
            strMessage = clGexDbQuery.lastError().text();

        // Dump query performance in debug mode
        clGexDbQuery.DumpPerformance();
    }
    else if(IsMySqlDB())
    {
        // initialization for Database partitioning
        //strQuery = "SELECT @@have_partitioning";
        strQuery = "SELECT plugin_status FROM information_schema.plugins";
        strQuery+= "   WHERE UPPER(plugin_name) = 'PARTITION'";
        if(clGexDbQuery.Execute(strQuery))
        {
            clGexDbQuery.First();
            m_bPartitioningDb =
                    (clGexDbQuery.value(0).toString().startsWith("YES",Qt::CaseInsensitive)
                     || clGexDbQuery.value(0).toString().startsWith("ACTIVE",Qt::CaseInsensitive));

            // Dump query performance in debug mode
            clGexDbQuery.DumpPerformance();
        }

        // initialization for Database transaction
        if(clGexDbQuery.Execute("SELECT @@AUTOCOMMIT"))
        {
            clGexDbQuery.First();
            m_bTransactionDb = (clGexDbQuery.value(0).toInt() == 0);

            // Dump query performance in debug mode
            clGexDbQuery.DumpPerformance();
        }

        // initialization for Database transaction level
        if(clGexDbQuery.Execute("SELECT @@SESSION.TX_ISOLATION"))
        {
            clGexDbQuery.First();
            if(clGexDbQuery.value(0).toString().toUpper() != "READ-COMMITTED")
            {
                clGexDbQuery.Execute("SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED");
            }
            // Dump query performance in debug mode
            clGexDbQuery.DumpPerformance();
        }

        // initialization for Database compression
        if(clGexDbQuery.Execute("SHOW STATUS like 'Compression'"))
        {
            clGexDbQuery.First();
            m_bCompressionProtocol = (clGexDbQuery.value(1).toString().toUpper() == "ON");

            // Dump query performance in debug mode
            clGexDbQuery.DumpPerformance();
            if(m_bCompressionProtocol)
                GSLOG(SYSLOG_SEV_DEBUG, "CLIENT COMPRESSION PROTOCOL activated!");
        }
        GetConnectionSID();
        GetConnectionHostName();

        // Check user privileges
        if(clGexDbQuery.Execute("SHOW GRANTS"))
        {
            while(clGexDbQuery.Next())
            {
                if(clGexDbQuery.value(0).toString().startsWith("GRANT USAGE",Qt::CaseInsensitive))
                    continue;
                GSLOG(SYSLOG_SEV_DEBUG, clGexDbQuery.value(0).
                      toString().toLatin1().constData());
            }
        }
    }

    // Update SqlReservedWord
    m_lstSqlReservedWords = SqlReservedWordList();
    m_bConnectedAsAdmin = m_bAdminUser;
    m_LastTestConnection.start();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Returns true if connected to a MySQL DB
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Connector::IsMySqlDB(QString & strDatabaseDriver)
{
    return (strDatabaseDriver == "QMYSQL3");
}

//////////////////////////////////////////////////////////////////////
// Returns details about last error
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Connector::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(GexDbPlugin_Connector,this);
}


//////////////////////////////////////////////////////////////////////////////
// build the list of Mysql/Oracle reserved word
// last update : PYC, 22/09/2011
//////////////////////////////////////////////////////////////////////////////

QStringList   GexDbPlugin_Connector::SqlReservedWordList()        // build the list of Mysql/Oracle reserved word
{
    QStringList qlWordList;
    if(IsMySqlDB())
    {
        qlWordList << "ADD" << "ALL" << "ALTER" << "ANALYZE" << "AND" << "AS" << "ASC" << "ASENSITIVE" << "BEFORE";
        qlWordList << "BETWEEN" << "BIGINT" << "BINARY" << "BLOB" << "BOTH" << "BY" << "CALL" << "CASCADE" << "CASE";
        qlWordList << "CHANGE" << "CHAR" << "CHARACTER" << "CHECK" << "COLLATE" << "COLUMN" << "CONDITION" << "CONSTRAINT";
        qlWordList << "CONTINUE" << "CONVERT" << "CREATE" << "CROSS" << "CURRENT_DATE" << "CURRENT_TIME" << "CURRENT_TIMESTAMP";
        qlWordList << "CURRENT_USER" << "CURSOR" << "DATABASE" << "DATABASES" << "DAY_HOUR" << "DAY_MICROSECOND";
        qlWordList << "DAY_MINUTE" << "DAY_SECOND" << "DEC" << "DECIMAL" << "DECLARE" << "DEFAULT" << "DELAYED" << "DELETE";
        qlWordList << "DESC" << "DESCRIBE" << "DETERMINISTIC" << "DISTINCT" << "DISTINCTROW" << "DIV" << "DOUBLE" << "DROP";
        qlWordList << "DUAL" << "EACH" << "ELSE" << "ELSEIF" << "ENCLOSED" << "ESCAPED" << "EXISTS" << "EXIT" << "EXPLAIN";
        qlWordList << "false" << "FETCH" << "FLOAT" << "FLOAT4" << "FLOAT8" << "FOR" << "FORCE" << "FOREIGN" << "FROM";
        qlWordList << "FULLTEXT" << "GRANT" << "GROUP" << "HAVING" << "HIGH_PRIORITY" << "HOUR_MICROSECOND" << "HOUR_MINUTE";
        qlWordList << "HOUR_SECOND" << "IF" << "IGNORE" << "IN" << "INDEX" << "INFILE" << "INNER" << "INOUT" << "INSENSITIVE";
        qlWordList << "INSERT" << "INT" << "INT1" << "INT2" << "INT3" << "INT4" << "INT8" << "INTEGER" << "INTERVAL";
        qlWordList << "INTO" << "IS" << "ITERATE" << "JOIN" << "KEY" << "KEYS" << "KILL" << "LEADING" << "LEAVE" << "LEFT";
        qlWordList << "LIKE" << "LIMIT" << "LINES" << "LOAD" << "LOCALTIME" << "LOCALTIMESTAMP" << "LOCK" << "LONG" << "LONGBLOB";
        qlWordList << "LONGTEXT" << "LOOP" << "LOW_PRIORITY" << "MATCH" << "MEDIUMBLOB" << "MEDIUMINT" << "MEDIUMTEXT";
        qlWordList << "MIDDLEINT" << "MINUTE_MICROSECOND" << "MINUTE_SECOND" << "MOD" << "MODIFIES" << "NATURAL" << "NOT";
        qlWordList << "NO_WRITE_TO_BINLOG" << "NULL" << "NUMERIC" << "ON" << "OPTIMIZE" << "OPTION" << "OPTIONALLY" << "OR";
        qlWordList << "ORDER" << "OUT" << "OUTER" << "OUTFILE" << "PRECISION" << "PRIMARY" << "PROCEDURE" << "PURGE";
        qlWordList << "READ" << "READS" << "REAL" << "REFERENCES" << "REGEXP" << "RELEASE" << "RENAME" << "REPEAT" << "REPLACE";
        qlWordList << "REQUIRE" << "RESTRICT" << "RETURN" << "REVOKE" << "RIGHT" << "RLIKE" << "SCHEMA" << "SCHEMAS";
        qlWordList << "SECOND_MICROSECOND" << "SELECT" << "SENSITIVE" << "SEPARATOR" << "SET" << "SHOW" << "SMALLINT" << "SONAME";
        qlWordList << "SPATIAL" << "SPECIFIC" << "SQL" << "SQLEXCEPTION" << "SQLSTATE" << "SQLWARNING" << "SQL_BIG_RESULT";
        qlWordList << "SQL_CALC_FOUND_ROWS" << "SQL_SMALL_RESULT" << "SSL" << "STARTING" << "STRAIGHT_JOIN" << "TABLE";
        qlWordList << "TERMINATED" << "THEN" << "TINYBLOB" << "TINYINT" << "TINYTEXT" << "TO" << "TRAILING" << "TRIGGER";
        qlWordList << "TRUE" << "UNDO" << "UNION" << "UNIQUE" << "UNLOCK" << "UNSIGNED" << "UPDATE" << "USAGE" << "USE";
        qlWordList << "USING" << "UTC_DATE" << "UTC_TIME" << "UTC_TIMESTAMP" << "VALUES" << "VARBINARY" << "VARCHAR";
        qlWordList << "VARCHARACTER" << "VARYING" << "WHEN" << "WHERE" << "WHILE" << "WITH" << "WRITE" << "XOR" << "YEAR_MONTH" << "ZEROFILL";

        // MySQL : 5.0:
        qlWordList << "ASENSITIVE" << "CALL" << "CONDITION" << "CONTINUE" << "CURSOR" << "DECLARE" << "DETERMINISTIC";
        qlWordList << "EACH" << "ELSEIF" << "EXIT" << "FETCH" << "INOUT" << "INSENSITIVE" << "ITERATE" << "LEAVE" << "LOOP";
        qlWordList << "MODIFIES" << "OUT" << "READS" << "RELEASE" << "REPEAT" << "RETURN" << "SCHEMA" << "SCHEMAS" << "SENSITIVE";
        qlWordList << "SPECIFIC" << "SQL" << "SQLEXCEPTION" << "SQLSTATE" << "SQLWARNING" << "TRIGGER" << "UNDO" << "WHILE";

        // Comment Forbidden by ansi sql but allowed by mysql :
        // ACTION; BIT; DATE; ENUM; NO; TEXT; TIME; TIMESTAMP
    }
    if(IsOracleDB())
    {
        qlWordList << "ACCESS" << "ACCOUNT" << "ACTIVATE" << "ADD" << "ADMIN" << "ADVISE" << "AFTER" << "ALL" << "ALL_ROWS";
        qlWordList << "ALLOCATE" << "ALTER" << "ANALYZE" << "AND" << "ANY" << "ARCHIVE" << "ARCHIVELOG" << "ARRAY" << "AS";
        qlWordList << "ASC" << "AT" << "AUDIT" << "AUTHENTICATED" << "AUTHORIZATION" << "AUTOEXTEND" << "AUTOMATIC";
        qlWordList << "BACKUP" << "BECOME" << "BEFORE" << "BEGIN" << "BETWEEN" << "BFILE" << "BITMAP" << "BLOB" << "BLOCK";
        qlWordList << "BODY" << "BY" << "CACHE" << "CACHE_INSTANCES" << "CANCEL" << "CASCADE" << "CAST" << "CFILE" << "CHAINED";
        qlWordList << "CHANGE" << "CHAR" << "CHAR_CS" << "CHARACTER" << "CHECK" << "CHECKPOINT" << "CHOOSE" << "CHUNK" << "CLEAR";
        qlWordList << "CLOB" << "CLONE" << "CLOSE" << "CLOSE_CACHED_OPEN_CURSORS" << "CLUSTER" << "COALESCE" << "COLUMN";
        qlWordList << "COLUMNS" << "COMMENT" << "COMMIT" << "COMMITTED" << "COMPATIBILITY" << "COMPILE" << "COMPLETE";
        qlWordList << "COMPOSITE_LIMIT" << "COMPRESS" << "COMPUTE" << "CONNECT" << "CONNECT_TIME" << "CONSTRAINT" << "CONSTRAINTS";
        qlWordList << "CONTENTS" << "CONTINUE" << "CONTROLFILE" << "CONVERT" << "COST" << "CPU_PER_CALL" << "CPU_PER_SESSION";
        qlWordList << "CREATE" << "CURRENT" << "CURRENT_SCHEMA" << "CURREN_USER" << "CURSOR" << "CYCLE" << "DANGLING";
        qlWordList << "DATABASE" << "DATAFILE" << "DATAFILES" << "DATAOBJNO" << "DATE" << "DBA" << "DBHIGH" << "DBLOW";
        qlWordList << "DBMAC" << "DEALLOCATE" << "DEBUG" << "DEC" << "DECIMAL" << "DECLARE" << "DEFAULT" << "DEFERRABLE";
        qlWordList << "DEFERRED" << "DEGREE" << "DELETE" << "DEREF" << "DESC" << "DIRECTORY" << "DISABLE" << "DISCONNECT";
        qlWordList << "DISMOUNT" << "DISTINCT" << "DISTRIBUTED" << "DML" << "DOUBLE" << "DROP" << "DUMP" << "EACH";
        qlWordList << "ELSE" << "ENABLE" << "END" << "ENFORCE" << "ENTRY" << "ESCAPE" << "EXCEPT" << "EXCEPTIONS";
        qlWordList << "EXCHANGE" << "EXCLUDING" << "EXCLUSIVE" << "EXECUTE" << "EXISTS" << "EXPIRE" << "EXPLAIN";
        qlWordList << "EXTENT" << "EXTENTS" << "EXTERNALLY" << "FAILED_LOGIN_ATTEMPTS" << "false" << "FAST" << "FILE";
        qlWordList << "FIRST_ROWS" << "FLAGGER" << "FLOAT" << "FLOB" << "FLUSH" << "FOR" << "FORCE" << "FOREIGN";
        qlWordList << "FREELIST" << "FREELISTS" << "FROM" << "FULL" << "FUNCTION" << "GLOBAL" << "GLOBALLY" << "GLOBAL_NAME";
        qlWordList << "GRANT" << "GROUP" << "GROUPS" << "HASH" << "HASHKEYS" << "HAVING" << "HEADER" << "HEAP" << "IDENTIFIED";
        qlWordList << "IDGENERATORS" << "IDLE_TIME" << "IF" << "IMMEDIATE" << "IN" << "INCLUDING" << "INCREMENT" << "INDEX";
        qlWordList << "INDEXED" << "INDEXES" << "INDICATOR" << "IND_PARTITION" << "INITIAL" << "INITIALLY" << "INITRANS";
        qlWordList << "INSERT" << "INSTANCE" << "INSTANCES" << "INSTEAD" << "INT" << "INTEGER" << "INTERMEDIATE" << "INTERSECT";
        qlWordList << "INTO" << "IS" << "ISOLATION" << "ISOLATION_LEVEL" << "KEEP" << "KEY" << "KILL" << "LABEL" << "LAYER";
        qlWordList << "LESS" << "LEVEL" << "LIBRARY" << "LIKE" << "LIMIT" << "LINK" << "LIST" << "LOB" << "LOCAL" << "LOCK";
        qlWordList << "LOCKED" << "LOG" << "LOGFILE" << "LOGGING" << "LOGICAL_READS_PER_CALL" << "LOGICAL_READS_PER_SESSION";
        qlWordList << "LONG" << "MANAGE" << "MASTER" << "MAX" << "MAXARCHLOGS" << "MAXDATAFILES" << "MAXEXTENTS" << "MAXINSTANCES";
        qlWordList << "MAXLOGFILES" << "MAXLOGHISTORY" << "MAXLOGMEMBERS" << "MAXSIZE" << "MAXTRANS" << "MAXVALUE" << "MIN";
        qlWordList << "MEMBER" << "MINIMUM" << "MINEXTENTS" << "MINUS" << "MINVALUE" << "MLSLABEL" << "MLS_LABEL_FORMAT";
        qlWordList << "MODE" << "MODIFY" << "MOUNT" << "MOVE" << "MTS_DISPATCHERS" << "MULTISET" << "NATIONAL" << "NCHAR";
        qlWordList << "NCHAR_CS" << "NCLOB" << "NEEDED" << "NESTED" << "NETWORK" << "NEW" << "NEXT" << "NOARCHIVELOG";
        qlWordList << "NOAUDIT" << "NOCACHE" << "NOCOMPRESS" << "NOCYCLE" << "NOFORCE" << "NOLOGGING" << "NOMAXVALUE";
        qlWordList << "NOMINVALUE" << "NONE" << "NOORDER" << "NOOVERRIDE" << "NOPARALLEL" << "NOPARALLEL" << "NOREVERSE";
        qlWordList << "NORMAL" << "NOSORT" << "NOT" << "NOTHING" << "NOWAIT" << "NULL" << "NUMBER" << "NUMERIC" << "NVARCHAR2";
        qlWordList << "OBJECT" << "OBJNO" << "OBJNO_REUSE" << "OF" << "OFF" << "OFFLINE" << "OID" << "OIDINDEX" << "OLD";
        qlWordList << "ON" << "ONLINE" << "ONLY" << "OPCODE" << "OPEN" << "OPTIMAL" << "OPTIMIZER_GOAL" << "OPTION";
        qlWordList << "OR" << "ORDER" << "ORGANIZATION" << "OSLABEL" << "OVERFLOW" << "OWN" << "PACKAGE" << "PARALLEL";
        qlWordList << "PARTITION" << "PASSWORD" << "PASSWORD_GRACE_TIME" << "PASSWORD_LIFE_TIME" << "PASSWORD_LOCK_TIME";
        qlWordList << "PASSWORD_REUSE_MAX" << "PASSWORD_REUSE_TIME" << "PASSWORD_VERIFY_FUNCTION" << "PCTFREE";
        qlWordList << "PCTINCREASE" << "PCTTHRESHOLD" << "PCTUSED" << "PCTVERSION" << "PERCENT" << "PERMANENT";
        qlWordList << "PLAN" << "PLSQL_DEBUG" << "POST_TRANSACTION" << "PRECISION" << "PRESERVE" << "PRIMARY";
        qlWordList << "PRIOR" << "PRIVATE" << "PRIVATE_SGA" << "PRIVILEGE" << "PRIVILEGES" << "PROCEDURE";
        qlWordList << "PROFILE" << "PUBLIC" << "PURGE" << "QUEUE" << "QUOTA" << "RANGE" << "RAW" << "RBA" << "READ";
        qlWordList << "READUP" << "REAL" << "REBUILD" << "RECOVER" << "RECOVERABLE" << "RECOVERY" << "REF" << "REFERENCES";
        qlWordList << "REFERENCING" << "REFRESH" << "RENAME" << "REPLACE" << "RESET" << "RESETLOGS" << "RESIZE" << "RESOURCE";
        qlWordList << "RESTRICTED" << "RETURN" << "RETURNING" << "REUSE" << "REVERSE" << "REVOKE" << "ROLE" << "ROLES";
        qlWordList << "ROLLBACK" << "ROW" << "ROWID" << "ROWNUM" << "ROWS" << "RULE" << "SAMPLE" << "SAVEPOINT" << "SB4";
        qlWordList << "SCAN_INSTANCES" << "SCHEMA" << "SCN" << "SCOPE" << "SD_ALL" << "SD_INHIBIT" << "SD_SHOW" << "SEGMENT";
        qlWordList << "SEG_BLOCK" << "SEG_FILE" << "SELECT" << "SEQUENCE" << "SERIALIZABLE" << "SESSION" << "SESSION_CACHED_CURSORS";
        qlWordList << "SESSIONS_PER_USER" << "SET" << "SHARE" << "SHARED" << "SHARED_POOL" << "SHRINK" << "SIZE" << "SKIP";
        qlWordList << "SKIP_UNUSABLE_INDEXES" << "SMALLINT" << "SNAPSHOT" << "SOME" << "SORT" << "SPECIFICATION" << "SPLIT";
        qlWordList << "SQL_TRACE" << "STANDBY" << "START" << "STATEMENT_ID" << "STATISTICS" << "STOP" << "STORAGE" << "STORE";
        qlWordList << "STRUCTURE" << "SUCCESSFUL" << "SWITCH" << "SYS_OP_ENFORCE_NOT_NULL$" << "SYS_OP_NTCIMG$" << "SYNONYM";
        qlWordList << "SYSDATE" << "SYSDBA" << "SYSOPER" << "SYSTEM" << "TABLE" << "TABLES" << "TABLESPACE" << "TABLESPACE_NO";
        qlWordList << "TABNO" << "TEMPORARY" << "THAN" << "THE" << "THEN" << "THREAD" << "TIMESTAMP" << "TIME" << "TO";
        qlWordList << "TOPLEVEL" << "TRACE" << "TRACING" << "TRANSACTION" << "TRANSITIONAL" << "TRIGGER" << "TRIGGERS" << "TRUE";
        qlWordList << "TRUNCATE" << "TX" << "TYPE" << "UB2" << "UBA" << "UID" << "UNARCHIVED" << "UNDO" << "UNION" << "UNIQUE";
        qlWordList << "UNLIMITED" << "UNLOCK" << "UNRECOVERABLE" << "UNTIL" << "UNUSABLE" << "UNUSED" << "UPDATABLE" << "UPDATE";
        qlWordList << "USAGE" << "USE" << "USER" << "USING" << "VALIDATE" << "VALIDATION" << "VALUE" << "VALUES" << "VARCHAR" << "VARCHAR2";
        qlWordList << "VARYING" << "VIEW" << "WHEN" << "WHENEVER" << "WHERE" << "WITH" << "WITHOUT" << "WORK" << "WRITE" << "WRITEDOWN";
        qlWordList << "WRITEUP" << "XID" << "YEAR" << "ZONE";

    }

    return qlWordList;
}


//////////////////////////////////////////////////////////////////////
// Apply some modification to be compatible with VarChar SQL insertion
// strValue is the string to translate
// bAddQuote is true to add 'strValue'
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Connector::TranslateStringToSqlVarChar(const QString &strValue, bool bAddQuote/*=true*/)
{
    QString strString;
    if(strValue.isNull() || strValue.isEmpty() || (strValue[0].toLatin1() == 0))
        strString = "";
    else
        // replace non latin1 characters by "?"
        strString = QString::fromLatin1(strValue.toLatin1());

    // Special Character Escape Sequences
    strString.replace("'","''");

    // New Oracle definition:
    // SET SCAN OFF => & is not used for input
    // SET ESCAPE OFF => for escape char

    // escape char
    if(!IsOracleDB())
        strString.replace("\\","\\\\");

    if(bAddQuote)
        strString.insert(0,"'").append("'");
    return strString;
}

QString GexDbPlugin_Connector::TranslateStringToSqlLob(const QString& value)
{
    QString lNewValue = TranslateStringToSqlVarChar(value,false);

    if(IsOracleDB() && (lNewValue.length()>4000))
    {
        // Split string into c_lob(4000 size)||c_lob(4000 size)
        for(int i = lNewValue.length()-4000; i>0; i-=4000)
            lNewValue = lNewValue.insert(i,"')||TO_CLOB('");
        lNewValue = "TO_CLOB('"+lNewValue+"')";
    }
    else if(IsMySqlDB())
    {
        lNewValue = "CONVERT('" + lNewValue + "' USING latin1)";
    }
    else
        lNewValue = "'"+lNewValue+"'";

    return lNewValue;
}

//////////////////////////////////////////////////////////////////////
// Translate TimeStamp to Formated DataTime to be compatible with DateTime SQL insertion
// strUnixTimeStamp is the timestamp or a field
// eFormat
// eConcatFormat
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Connector::TranslateUnixTimeStampToSqlDateTime(const QString strUnixTimeStamp, enum SqlUnixDateFormat eFormat, enum SqlUnixDateFormat eConcatFormat)
{
    QString strDate;

    QString strFormatToQuarter;
    QString strFormatToYear;
    QString strFormatToWYear;
    QString strFormatToWeek;
    QString strFormatToMonth;
    QString strFormatToDate;

    QString strConcat;

    if(IsSQLiteDB())
    {
        strFormatToQuarter	= "(CASE WHEN strftime('%m',%1, 'unixepoch') IN ('01','02','03') THEN '1' ELSE (CASE WHEN strftime('%m',%1, 'unixepoch') IN ('04','05','06') THEN '2' ELSE (CASE WHEN strftime('%m',%1, 'unixepoch') IN ('07','08','09') THEN '3' ELSE '4' END) END) END)";
        strFormatToYear		= "strftime('%Y',%1, 'unixepoch','localtime')";
        strFormatToWYear	= "strftime('%Y',%1, 'unixepoch','localtime','weekday 0','-6 days')";
        strFormatToWeek		= "strftime('%W',%1, 'unixepoch','localtime')";
        strFormatToMonth	= "strftime('%M',%1, 'unixepoch','localtime')";
        strFormatToDate		= "strftime('%Y-%m-%d',%1, 'unixepoch','localtime')";

        strConcat = "(%1||'-'||%2)";
    }
    else
        if(IsMySqlDB())
        {
            strFormatToQuarter	= "cast(quarter(from_unixtime(%1)) as char(1)) ";
            strFormatToYear		= "date_format(from_unixtime(%1),'%Y')";
            strFormatToWYear	= "date_format(from_unixtime(%1),'%x')";
            strFormatToWeek		= "date_format(from_unixtime(%1),'%v')";
            strFormatToMonth	= "date_format(from_unixtime(%1),'%m')";
            strFormatToDate		= "date_format(from_unixtime(%1),'%Y-%m-%d')";
            strConcat = "concat(%1,concat('-',%2))";
        }
        else
            if(IsOracleDB())
            {
                // Oracle
                strFormatToQuarter	= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'Q')";
                strFormatToYear		= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'YYYY')";
                strFormatToWYear	= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'IYYY')";
                strFormatToWeek		= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'IW')";
                strFormatToMonth	= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'MM')";
                strFormatToDate		= "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+%1/(24*3600),'YYYY-MM-DD')";
                strConcat = "concat(%1,concat('-',%2))";
            }
            else
                return "";

    strDate = strFormatToDate.arg(strUnixTimeStamp);

    switch(eFormat)
    {
    case eYear:
        strDate = strFormatToYear.arg(strUnixTimeStamp);
        break;
    case eQuarter:
        strDate = strFormatToQuarter.arg(strUnixTimeStamp);
        break;
    case eWeek:
        strDate = strFormatToWeek.arg(strUnixTimeStamp);
        break;
    case eWYear:
        strDate = strFormatToWYear.arg(strUnixTimeStamp);
        break;
    case eMonth:
        strDate = strFormatToMonth.arg(strUnixTimeStamp);
        break;
    case eDate:
        strDate = strFormatToDate.arg(strUnixTimeStamp);
        break;
    case eEmpty:
        break;
    }
    if(eConcatFormat != eEmpty)
    {
        QString strValue;

        switch(eConcatFormat)
        {
        case eYear:
            strValue = strFormatToYear.arg(strUnixTimeStamp);
            break;
        case eQuarter:
            strValue = strFormatToQuarter.arg(strUnixTimeStamp);
            break;
        case eWeek:
            strValue = strFormatToWeek.arg(strUnixTimeStamp);
            break;
        case eWYear:
            strValue = strFormatToWYear.arg(strUnixTimeStamp);
            break;
        case eMonth:
            strValue = strFormatToMonth.arg(strUnixTimeStamp);
            break;
        case eDate:
            strValue = strFormatToDate.arg(strUnixTimeStamp);
            break;
        case eEmpty:
            break;
        }
        strDate = strConcat.arg(strDate,strValue);
    }
    return strDate;
}

QTime gTimer;
QDateTime gCurrentDateTime;

QDateTime GexDbPlugin_Connector::ServerDateTime()
{
    // Initialize with the currentDateTime
    QDateTime DateTime = QDateTime::currentDateTime();
    QString   strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_strConnectionName));

    // Recheck the Server DateTime every 1mn
    if(!gTimer.isValid() || gTimer.elapsed() > 60000)
    {
        if(IsOracleDB())
            strQuery = "SELECT SYSDATE FROM DUAL";
        else
            strQuery = "SELECT now()";

        if(clQuery.exec(strQuery) && clQuery.first())
            gCurrentDateTime = clQuery.value(0).toDateTime();
        else
            gCurrentDateTime = QDateTime::currentDateTime();
        gTimer.start();
    }
    DateTime = gCurrentDateTime.addMSecs(gTimer.elapsed());

    return DateTime;
}

QString GexDbPlugin_Connector::TranslateDateToSqlDateTime(QDateTime Date)
{
    QString strValue;
    if (IsMySqlDB())
    {
        strValue = "'" + Date.toString("yyyy-MM-dd hh:mm:ss") + "'";
    }
    else
    {
        strValue = "TO_DATE('" + Date.toString("yyyy-MM-dd hh:mm:ss") +
                "','YYYY-MM-DD HH24:MI:SS')";
    }
    return strValue;

}

///////////////////////////////////////////////////////////
// For debug purpose, write message to debug trace file
///////////////////////////////////////////////////////////
void GexDbPlugin_Connector::WriteDebugMessageFile(const QString & strMessage, bool /*bUpdateGlobalTrace=false*/)
{
    QString strLogMessage;

    if(m_nConnectionSID>0)
        strLogMessage = "[SID:"+QString::number(m_nConnectionSID)+"] ";
    strLogMessage += strMessage;
    // Write Debug message
    GSLOG(SYSLOG_SEV_DEBUG, strLogMessage.left(65280).toLatin1().constData() );

    /*
    if(m_bCustomerDebugMode)
    {
        FILE			*hFile;
        CGexSystemUtils	clSysUtils;
        QString			strTraceFile;

        // Construct file name
        strTraceFile = m_strUserProfile + "/GalaxySemi/logs/";
        strTraceFile += QDate::currentDate().toString(Qt::ISODate);
        strTraceFile +=  "/GSLOG_";
        strTraceFile += QDate::currentDate().toString(Qt::ISODate);
        strTraceFile += "_sql.txt";
        clSysUtils.NormalizePath(strTraceFile);

        // Write message to local trace file
        hFile = fopen(strTraceFile.toLatin1().constData(),"a");
        if(hFile == NULL)
            return;
        fprintf(hFile,"[%s %s] %s\n",QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(), QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(), strLogMessage.toLatin1().constData());
        fclose(hFile);

        if(bUpdateGlobalTrace)
        {
            // Construct file name
            strTraceFile = m_strUserProfile + "/GalaxySemi/logs/";
            strTraceFile += QDate::currentDate().toString(Qt::ISODate);
            strTraceFile += "/GSLOG_";
            strTraceFile += QDate::currentDate().toString(Qt::ISODate);
            strTraceFile += ".txt";
            clSysUtils.NormalizePath(strTraceFile);

            // Write message to local trace file
            hFile = fopen(strTraceFile.toLatin1().constData(),"a");
            if(hFile == NULL)
                return;
            fprintf(hFile,"[%s %s] %s\n",QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(), QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(), strLogMessage.toLatin1().constData());
            fclose(hFile);
        }
    }
    */
}

///////////////////////////////////////////////////////////
// OpenThread
// Create a thread to execute GexDbPlugin_Galaxy::connectToCoorporate
// Emit processEvents events
///////////////////////////////////////////////////////////
bool OpenSqlConnectionThread::exec(QString strConnectionName, int nConnectionTimeOut)
{
    QTime	clLastRefresh;

    m_bConnected = false;
    m_bConnectionTimeOut = false;
    m_strConnectionName = strConnectionName;

    clLastRefresh.start();
    m_clTimer.start();

    // Launch the thread and open the connection
    start();

    // Wait the end of the thread
    while(isRunning() || !isFinished())
    {

        // Each 1 secondes
        if((nConnectionTimeOut>0) && (m_clTimer.elapsed() > (1000*nConnectionTimeOut)))
        {
            m_bConnectionTimeOut = true;
            return false;
        }

        if(clLastRefresh.elapsed()>1000)
        {
            QCoreApplication::processEvents();
            clLastRefresh.start();
        }

        this->msleep(200);
    }

    // End of the thread
    // Return the result
    return m_bConnected;
}

void OpenSqlConnectionThread::WaitForFinished()
{
    QTime	clLastRefresh;

    clLastRefresh.start();
    // Wait the end of the thread
    while(isRunning() || !isFinished())
    {

        // Each 1 secondes
        if(clLastRefresh.elapsed() >= 1000)
        {
            QCoreApplication::processEvents();
            clLastRefresh.start();
        }

        this->msleep(200);
    }
}

///////////////////////////////////////////////////////////
// Run the thread
///////////////////////////////////////////////////////////
void OpenSqlConnectionThread::run()
{
    m_bConnected = false;
    QSqlDatabase clDatabase = QSqlDatabase::database(m_strConnectionName,false);

    // Execute the connection in another thread
    // Check if already connected
    m_bConnected=clDatabase.open();
}
