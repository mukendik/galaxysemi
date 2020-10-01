/******************************************************************************!
 * \file admin_engine_updata.cpp
 ******************************************************************************/
#include <QSqlRecord>
#include <QSqlError>
#include "gqtl_log.h"
#include "engine.h"
#include "gqtl_sysutils.h"
#include "dir_access_base.h"
#include "admin_engine.h"
#include "product_info.h"

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn AdminServerUpdate
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate()
{
    bool bStatus;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return false;

    // Connect the Node to have the node_id for trace
    ConnectNode();

    // Kill all connection
    AdminServerUpdate_CloseAllSqlConnection();

    // Start the update
    AddNewEvent("UPDATE",
                "ADMINISTRATION",
                0,
                0,
                0,
                "Execute=YieldManDbUpdate()",
                "START",
                "Update ym_admin_db",
                GetServerVersionName(true));

    GSLOG(SYSLOG_SEV_DEBUG, "Start Updating YieldMan Server");

    GS::Gex::Engine::GetInstance().
    UpdateLabelStatus("Updating YieldMan Server...");

    m_strUpdateDbLogFile =
            GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();
    m_strUpdateDbLogFile += QDir::separator();
    m_strUpdateDbLogFile += "logs";
    m_strUpdateDbLogFile += QDir::separator();
    m_strUpdateDbLogFile += QDate::currentDate().toString(Qt::ISODate);
    m_strUpdateDbLogFile += QDir::separator();

    // Create the folder if not exists
    QDir dir;
    dir.mkpath(m_strUpdateDbLogFile);

    m_strUpdateDbLogFile += "yieldmandb_update_b" +
            QString::number(m_nServerBuild) + "_to_b" +
            QString::number(YIELDMANDB_BUILD_NB) + "_" +
            GS::Gex::Engine::GetInstance().GetClientDateTime().toString("yyyyMMdd-hhmm") + ".log";

    CGexSystemUtils clSysUtils;
    clSysUtils.NormalizePath(m_strUpdateDbLogFile);
    m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
    m_hUpdateDbLogFile.open(QIODevice::QIODevice::WriteOnly |
                            QIODevice::Append |
                            QIODevice::Text);


    bStatus = AdminServerUpdate_loop();

    if(bStatus)
    {
        // First update of the events flow
        AddNewEvent("UPDATE", "ADMINISTRATION", "PASS",
                    m_UpdateDbLogContents.join("\n"),
                    GetServerVersionName(true));

        GSLOG(SYSLOG_SEV_DEBUG, "End Updating YieldMan Server");
    }
    else
    {
        QString strError;
        GetLastError(strError);
        InsertIntoUpdateLog(strError);

        // First update of the events flow
        AddNewEvent("UPDATE", "ADMINISTRATION", "FAIL",
                    m_UpdateDbLogContents.join("\n"),
                    GetServerVersionName(true));


        GSLOG(SYSLOG_SEV_DEBUG, "Fail to Update YieldMan Server");
    }
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("");

    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.close();
    }
    m_UpdateDbLogContents.clear();

    return bStatus;
}

/******************************************************************************!
 * \fn AdminServerUpdate_loop
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_loop()
{
    bool bStatus;

    // Make sure current DB has to be updated
    GetServerVersion();

    // The IsUptoDate check the Major and Minor version
    // The build could be updated without Major or Minor update
    // New YieldMan can update some table (change nullable field for example)
    // that have no impact on any other process (Old YieldMan/Examinator)

    // To know if have an update to do
    // check also the build
    if (m_nServerBuild > YIELDMANDB_BUILD_NB)
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Server Version '%1' - Plugin Version '%2'").
              arg(GetServerVersionName(true)).
              arg(GetCurrentVersionName(true)).toLatin1().constData());
        return false;
    }

    if (((int) YIELDMANDB_VERSION_MAJOR == m_nServerMajorVersion) &&
        ((int) YIELDMANDB_VERSION_MINOR == m_nServerMinorVersion) &&
        ((int) YIELDMANDB_BUILD_NB == m_nServerBuild) &&
            m_strServerStatus.isEmpty())
    {
        // If no update, update indexes
        AdminServerUpdate_UpdateIndexes();
        AdminServerUpdate_UpdateEvents();
        return true;
    }

    int lServerBuild = m_nServerBuild;
    // if an error has occurred during last DB re-apply the last update
    // i.e. current version = B17 + UPDATING_DATABASE re-aplly B16 to B17
    if (!m_strServerStatus.isEmpty())
        lServerBuild--;


    // Incremantally update DB
    bStatus = false;
    switch (lServerBuild)
    {
    case 1:
        InsertIntoUpdateLog("YieldManDb V1.0 B1 -> YieldManDb V1.0 B2");
        bStatus = AdminServerUpdate_B1_to_B2();
        break;
    case 2:
        InsertIntoUpdateLog("YieldManDb V1.0 B2 -> YieldManDb V1.0 B3");
        bStatus = AdminServerUpdate_B2_to_B3();
        break;
    case 3:
        InsertIntoUpdateLog("YieldManDb V1.0 B3 -> YieldManDb V2.0 B4");
        bStatus = AdminServerUpdate_B3_to_B4();
        break;
    case 4:
        InsertIntoUpdateLog("YieldManDb V2.0 B4 -> YieldManDb V2.0 B5");
        bStatus = AdminServerUpdate_B4_to_B5();
        break;
    case 5:
        InsertIntoUpdateLog("YieldManDb V2.0 B5 -> YieldManDb V2.0 B6");
        bStatus = AdminServerUpdate_B5_to_B6();
        break;
    case 6:
        InsertIntoUpdateLog("YieldManDb V2.0 B6 -> YieldManDb V2.0 B7");
        bStatus = AdminServerUpdate_B6_to_B7();
        break;
    case 7:
        bStatus = AdminServerUpdate_B7_to_B8();
        break;
    case 8:
        bStatus = AdminServerUpdate_B8_to_B9();
        break;
    case 9:
        bStatus = AdminServerUpdate_B9_to_B10();
        break;
    case 10:
        bStatus = AdminServerUpdate_B10_to_B11();
        break;
    case 11:
        bStatus = AdminServerUpdate_B11_to_B12();
        break;
    case 12:
        bStatus = AdminServerUpdate_B12_to_B13();
        break;
    default:
        if (lServerBuild < YIELDMANDB_BUILD_NB)
        {
            bStatus =
                AdminServerUpdate_FromSqlScript(lServerBuild,
                                                YIELDMANDB_BUILD_NB);
        }
        break;
    }

    if (! bStatus)
        return false;

    return AdminServerUpdate_loop();
}

/******************************************************************************!
 * \fn AdminServerUpdate_CloseAllSqlConnection
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_CloseAllSqlConnection()
{
    // Check if a user is connected
    if(m_pCurrentUser == NULL)
        return false;

    // Kill all ym_admin_db SQL connection except me
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));
    QString lQuery;
    QStringList lSessionIdToKill;
    QStringList lCurrentSessionId;
    lCurrentSessionId << mMutex.section(":", 0, 0);
    lCurrentSessionId << mDirAccessPlugin->GetConnector()->GetSessionId().section(":", 1, 1);

    lQuery = "SELECT ID FROM information_schema.processlist";
    lQuery += " WHERE USER='" + m_pDatabaseConnector->m_strUserName_Admin + "'";
    if (! clQuery.exec(lQuery))
    {
        return false;
    }
    while (clQuery.next())
    {
        // Ignore my SID
        if (lCurrentSessionId.contains(clQuery.value(0).toString()))
        {
            continue;
        }
        lSessionIdToKill << clQuery.value(0).toString();
    }
    while (! lSessionIdToKill.isEmpty())
    {
        lQuery = "KILL " + lSessionIdToKill.takeFirst();
        clQuery.exec(lQuery);
    }

    lQuery = "UPDATE ym_users SET mutex=null WHERE user_id!="+QString::number(m_pCurrentUser->m_nUserId);
    clQuery.exec(lQuery);

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B1_to_B2
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_B1_to_B2()
{
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Clean ym_users_settings TABLE
        strQuery = "DELETE FROM ym_users_profiles WHERE script_content IS NULL";
        clQuery.exec(strQuery);

        // Check if already updated
        strQuery =
            "SELECT COLUMN_TYPE FROM information_schema.columns "
            "WHERE TABLE_SCHEMA='" +
            m_pDatabaseConnector->m_strSchemaName + "'";
        strQuery +=
            " AND TABLE_NAME='ym_users_profiles' "
            "AND COLUMN_NAME='script_content'";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if ((clQuery.value(0).toString().toUpper() != "MEDIUMTEXT") ||
            (clQuery.value(0).toString().toLower() != "LONGTEXT"))
        {
            // Change TEXT field to MEDIUMTEXT
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.write("* UPDATE ym_users_profiles TABLE:\n");
            }
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.write("         MODIFY script_content "
                                         "type to MEDIUMTEXT\n");
            }

            strQuery =
                "ALTER TABLE ym_users_profiles MODIFY "
                "script_content MEDIUMTEXT";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery =
        "INSERT INTO ym_settings VALUES"
        "('DB_VERSION_NAME','Yield-Man Admin Server V1.0 B2')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','2')";
    clQuery.exec(strQuery);

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B2_to_B3
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_B2_to_B3()
{
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Check if already updated
        strQuery =
            "SELECT COLUMN_TYPE FROM information_schema.columns "
            "WHERE TABLE_SCHEMA='" +
            m_pDatabaseConnector->m_strSchemaName + "'" +
            " AND TABLE_NAME='ym_tasks_options' AND COLUMN_NAME='value'";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().toUpper() != "VARCHAR(4096)")
        {
            // Change VARCHAR(1024) field to VARCHAR(4096)
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.write("* UPDATE ym_tasks_options TABLE:\n");
            }
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.
                    write("         MODIFY value type to VARCHAR(4096)\n");
            }

            strQuery =
                "ALTER TABLE ym_tasks_options MODIFY "
                "ym_tasks_options.value VARCHAR(4096)";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }
    else if (m_pDatabaseConnector->IsOracleDB())
    {

        // Change VARCHAR(1024) field to VARCHAR(4096)
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.write("* UPDATE ym_tasks_options TABLE:\n");
        }
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.
                write("         MODIFY value type to VARCHAR(4096)\n");
        }

        strQuery = "ALTER TABLE ym_tasks_options MODIFY (value VARCHAR2(4096))";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery =
        "INSERT INTO ym_settings VALUES"
        "('DB_VERSION_NAME','Yield-Man Admin Server V1.0 B3')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','3')";
    clQuery.exec(strQuery);

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B3_to_B4
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_B3_to_B4()
{
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    // For all users defined in ym_users
    // Drop user<user_id>
    // Grant ym_admin_user_<user_id>
    QString strUserNodeId;
    QStringList lstUserNodeId;
    strQuery = "SELECT user_id from ym_users";
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    while (clQuery.next())
    {
        lstUserNodeId.append(clQuery.value(0).toString());
    }

    while (! lstUserNodeId.isEmpty())
    {
        strUserNodeId = lstUserNodeId.takeFirst();
        // Drop user
        if (m_pDatabaseConnector->IsOracleDB())
        {
            strQuery = "DROP USER user" + strUserNodeId + " CASCADE";
        }
        else
        {
            strQuery = "DROP USER 'user" + strUserNodeId + "'@'%'";
        }

        clQuery.exec(strQuery);
    }
    strQuery = "SELECT node_id from ym_nodes";
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    while (clQuery.next())
    {
        lstUserNodeId.append(clQuery.value(0).toString());
    }

    while (! lstUserNodeId.isEmpty())
    {
        strUserNodeId = lstUserNodeId.takeFirst();
        // Drop user
        if (m_pDatabaseConnector->IsOracleDB())
        {
            strQuery = "DROP USER node" + strUserNodeId + " CASCADE";
        }
        else
        {
            strQuery = "DROP USER 'node" + strUserNodeId + "'@'%'";
        }

        clQuery.exec(strQuery);
    }

    // Reset the last_access to show Summary dialog
    strQuery = "UPDATE ym_users SET last_access=null WHERE lower(login)='root'";
    clQuery.exec(strQuery);

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        QString(YIELDMANDB_VERSION_NAME) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','2.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','4')";
    clQuery.exec(strQuery);

    clQuery.exec("COMMIT");  // To unlock any tables

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B4_to_B5
 * \return false on error
 ******************************************************************************/
// MINOR UPDATE
// will be compatible with previous version
// Allow null value for node_id and user_id into
// ym_nodes_options and ym_users_options
// This null value is ised for general options and
// will be applied for all node/user that have not this options explicitly set
bool AdminEngine::AdminServerUpdate_B4_to_B5()
{
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(
                          m_pDatabaseConnector->m_strConnectionName));

    // Allow null value for node_id and user_id into
    // ym_nodes_options and ym_users_options
    QString strValue;
    QString strTableName;
    QString strFieldName;
    QStringList lstTableFieldToUpdate;
    lstTableFieldToUpdate.append("ym_nodes_options.node_id");
    lstTableFieldToUpdate.append("ym_users_options.user_id");
    while (! lstTableFieldToUpdate.isEmpty())
    {
        strValue = lstTableFieldToUpdate.takeFirst();
        strTableName = strValue.section(".", 0, 0);
        strFieldName = strValue.section(".", 1);

        // Check if already updated
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery =
                "SELECT UPPER(IS_NULLABLE),COLUMN_TYPE "
                "FROM information_schema.columns WHERE TABLE_SCHEMA='" +
                m_pDatabaseConnector->m_strSchemaName +
                "' AND TABLE_NAME='" + strTableName +
                "' AND COLUMN_NAME='" + strFieldName + "'";
        }
        else
        {
            strQuery =
                "SELECT UPPER(NULLABLE), "
                "concat(DATA_TYPE,concat('(',concat(DATA_PRECISION,')'))) "
                "FROM sys.USER_TAB_COLUMNS "
                "WHERE upper(TABLE_NAME)=upper('" + strTableName + "') "
                "AND (upper(COLUMN_NAME)=upper('" + strFieldName + "'))";
        }
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().left(1) == "N")
        {
            // Change to nullable
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.
                    write(QString("* UPDATE " + strTableName +
                                  " TABLE:\n").toLatin1().constData());
            }
            if (m_hUpdateDbLogFile.isOpen())
            {
                m_hUpdateDbLogFile.
                    write("         MODIFY value type to nullable\n");
            }

            if (m_pDatabaseConnector->IsMySqlDB())
            {
                strQuery =
                    "ALTER TABLE " + strTableName +
                    " MODIFY " + strFieldName + " " +
                    clQuery.value(1).toString() + " NULL";
            }
            else
            {
                strQuery =
                    "ALTER TABLE " + strTableName +
                    " MODIFY ( " + strFieldName + " " +
                    clQuery.value(1).toString() + " NULL)";
            }
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','2.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','5')";
    clQuery.exec(strQuery);

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B5_to_B6
 * \return false on error
 ******************************************************************************/
// MINOR UPDATE
// will be compatible with previous version
// Add ym_databases_options for more options
bool AdminEngine::AdminServerUpdate_B5_to_B6()
{
    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));
    // Add ym_databases_options for more options
    // Check if already updated
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strQuery =
            "SELECT TABLE_NAME FROM information_schema.tables "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "' AND TABLE_NAME='ym_databases_options'";
    }
    else
    {
        strQuery =
            "SELECT TABLE_NAME FROM sys.USER_TABLES "
            "WHERE upper(TABLE_NAME)=upper('ym_databases_options')";
    }
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if (! clQuery.first())
    {
        // Have to create
        // Change to nullable
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.write("* CREATE ym_databases_options TABLE:\n");
        }
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.write("         to store new options field\n");
        }

        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "CREATE TABLE `ym_databases_options` (";
            strQuery += "        `database_id` int(11) NOT NULL DEFAULT '0',";
            strQuery += "      `field` varchar(256) NOT NULL DEFAULT '',";
            strQuery += "      `value` varchar(1024) NOT NULL DEFAULT ''";
            strQuery += "    ) ENGINE=InnoDB DEFAULT CHARSET=latin1";
        }
        else
        {
            strQuery = "CREATE TABLE ym_databases_options (";
            strQuery += "      database_id NUMBER(11) NOT NULL,";
            strQuery += "      field VARCHAR2(256) NOT NULL,";
            strQuery += "      value VARCHAR2(1024)";
            strQuery +=
                "    ) TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS";
        }
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','2.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','6')";
    clQuery.exec(strQuery);

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B6_to_B7
 * \return false on error
 ******************************************************************************/
// MINOR UPDATE
// will be compatible with previous version
// Update indexes
bool AdminEngine::AdminServerUpdate_B6_to_B7()
{
    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    // Update indexes
    if (! AdminServerUpdate_UpdateIndexes())
    {
        return false;
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','2.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','7')";
    clQuery.exec(strQuery);

    // Success
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("Status = SUCCESS.\n");
    }

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B7_to_B8
 * \return false on error
 ******************************************************************************/
// MAJOR UPDATE
// not compatible with previous version
// Update ym_nodes table - Add new columns
bool AdminEngine::AdminServerUpdate_B7_to_B8()
{

    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V2.0 B7 -> YieldManDb V3.0 B8");

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));
    // Update ym_nodes table
    QString strTableName;
    QSqlRecord clRecords;

    // Dump all the ym_admin_db
    QString BackupFile;
    AdminServerBackup(BackupFile);

    // CREATE ym_events table
    strTableName = "ym_events";
    // Check if already updated
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strQuery =
            "SELECT TABLE_NAME FROM information_schema.tables "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "' AND TABLE_NAME='" + strTableName + "'";
    }
    else
    {
        strQuery =
            "SELECT TABLE_NAME FROM sys.USER_TABLES "
            "WHERE upper(TABLE_NAME)=upper('" + strTableName + "')";
    }
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if (! clQuery.first())
    {
        // Have to create
        // Change to nullable
        InsertIntoUpdateLog("* CREATE " + strTableName + " TABLE:");
        InsertIntoUpdateLog("         to store new events flow");

        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "CREATE TABLE " + strTableName + " (";
            strQuery += "      creation_time datetime NOT NULL,";
            strQuery +=
                "      event_id bigint unsigned NOT NULL AUTO_INCREMENT,";
            strQuery += "      link bigint unsigned DEFAULT NULL,";
            strQuery += "      category varchar(256) NOT NULL,";
            strQuery += "      type varchar(256) NOT NULL,";
            strQuery += "      start_time datetime,";
            strQuery += "      duration int(10) unsigned DEFAULT 0,";
            strQuery += "      size int(10) unsigned DEFAULT 0,";
            strQuery += "      node_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      user_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      task_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      database_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      command varchar(1024) DEFAULT NULL,";
            strQuery += "      status varchar(256) DEFAULT NULL,";
            strQuery += "      summary MEDIUMTEXT DEFAULT NULL,";
            strQuery += "      comment varchar(1024) DEFAULT NULL,";
            strQuery += "      PRIMARY KEY (event_id)";
            strQuery += "    ) ENGINE=InnoDB DEFAULT CHARSET=latin1";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            strQuery = "DROP EVENT IF EXISTS event_add_partition";
            clQuery.exec(strQuery);
            strQuery =
                "CREATE EVENT event_add_partition ON SCHEDULE EVERY 1 DAY DO \n";
            strQuery += " BEGIN \n";
            strQuery +=
                "                 DECLARE new_partition CHAR(32) DEFAULT ";
            strQuery += "                     CONCAT( 'p', ";
            strQuery +=
                "                         DATE_FORMAT( DATE_ADD( CURDATE(), INTERVAL 1 DAY ), '%Y%m%d' ) ";
            strQuery += "                     );\n";
            strQuery +=
                "                 DECLARE max_day INTEGER DEFAULT TO_DAYS( CURDATE ) + 1;\n";
            strQuery +=
                "                 SET @stmt = CONCAT(\"ALTER TABLE log ADD PARTITION (PARTITION \", ";
            strQuery +=
                "                             new_partition, \" VALUES LESS THAN (\", max_day, \"))\");\n";
            strQuery += "                 PREPARE stmt FROM @stmt; ";
            strQuery += " \n";
            strQuery += "                 EXECUTE stmt; \n";
            strQuery += "             END";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            strQuery = "CREATE TABLE " + strTableName + " (";
            strQuery += "      creation_time DATE NOT NULL,";
            strQuery += "      event_id NUMBER(20) NOT NULL,";
            strQuery += "      link NUMBER(20) DEFAULT NULL,";
            strQuery += "      category VARCHAR2(256) NOT NULL,";
            strQuery += "      type VARCHAR2(256) NOT NULL,";
            strQuery += "      start_time DATE,";
            strQuery += "      duration NUMBER(11) DEFAULT 0,";
            strQuery += "      \"size\" NUMBER(11) DEFAULT 0,";
            strQuery += "      node_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      user_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      task_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      database_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      command VARCHAR2(1024) DEFAULT NULL,";
            strQuery += "      status VARCHAR2(256) DEFAULT NULL,";
            strQuery += "      summary CLOB DEFAULT NULL,";
            strQuery += "      \"comment\" VARCHAR2(1024) DEFAULT NULL,";
            strQuery += "      PRIMARY KEY (event_id)";
            strQuery += "    ) PARTITION BY RANGE (creation_time)";
            strQuery += "    INTERVAL (NUMTODSINTERVAL(1,'day'))";
            strQuery +=
                "    (PARTITION FIRSTPART  VALUES LESS THAN "
                "(TO_DATE('01-01-2012','DD-MM-YYYY')) ";
            strQuery +=
                "     TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS)";

            /*
               CREATE TABLE TEST
               (TIME_ID NUMBER,
               REGION_ID NUMBER,
               ORDER_ID NUMBER,
               ORDER_DATE DATE
               )
               PARTITION BY RANGE (ORDER_DATE)
               INTERVAL (NUMTODSINTERVAL(7,'day'))
               (PARTITION p_first VALUES LESS THAN ('01-JAN-2006'));
               // The additional partitions and local indexes are
               // automatically created.
             */

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            // Create sequence for oracle
            strQuery = "CREATE SEQUENCE " + strTableName + "_sequence";
            strQuery += " START WITH 1 INCREMENT BY 1 NOCACHE";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    // First update of the events flow
    AddNewEvent("UPDATE",
                "ADMINISTRATION",
                "START",
                "Update ym_admin_db for LoadBalancing process",
                GetServerVersionName(true));

    // Clean databases
    // Delete some tables
    QStringList lTableToRemove;
    lTableToRemove << "ym_databases_permisions";
    lTableToRemove << "ym_logs";
    lTableToRemove << "ym_logs_options";
    lTableToRemove << "ym_patmans";
    lTableToRemove << "ym_patmans_options";
    lTableToRemove << "ym_users_templates";
    lTableToRemove << "ym_users_templates_sections";
    lTableToRemove << "ym_users_templates_sections_options";

    // Check if already updated
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strQuery =
            "SELECT TABLE_NAME FROM information_schema.tables "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "' AND ";
    }
    else
    {
        strQuery = "SELECT TABLE_NAME FROM sys.USER_TABLES WHERE ";
    }
    strQuery +=
        "TABLE_NAME IN ('" + lTableToRemove.join("','").toUpper() + "')";
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    lTableToRemove.clear();
    while (clQuery.next())
    {
        lTableToRemove << clQuery.value(0).toString();
    }
    while (! lTableToRemove.isEmpty())
    {
        strTableName = lTableToRemove.takeFirst();
        InsertIntoUpdateLog("* DROP " + strTableName + " TABLE");

        // Drop foreign key
        strQuery = "DROP TABLE " + strTableName;
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "SET FOREIGN_KEY_CHECKS=0;\n" + strQuery +
                ";\nSET FOREIGN_KEY_CHECKS=1";
        }
        clQuery.exec(strQuery);
        // Delete sequence
        if (m_pDatabaseConnector->IsOracleDB())
        {
            strQuery = "DROP SEQUENCE " + strTableName + "_sequence";
            clQuery.exec(strQuery);
        }
    }
    // UPDATE ym_nodes table
    // ADD name,status,last_update,mutex columns if it do not already exist
    strTableName = "ym_nodes";
    clRecords =
        QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName).
        record(strTableName);
    if (! clRecords.contains("name"))
    {
        // Add columns
        InsertIntoUpdateLog("* UPDATE " + strTableName + " TABLE:");
        InsertIntoUpdateLog("         ADD new colmuns");
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "ALTER TABLE " + strTableName + " ADD COLUMN ";
            strQuery += "( type VARCHAR(256) DEFAULT NULL";
            strQuery += ", name VARCHAR(256) DEFAULT NULL";
            strQuery += ", status VARCHAR(256) DEFAULT NULL";
            strQuery += ", last_update DATETIME DEFAULT NULL";
            strQuery += ", mutex VARCHAR(256) DEFAULT NULL";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            // No Default value: unsupported add column
            // (if default value) operation on compressed table
            strQuery = "ALTER TABLE " + strTableName + " ADD ";
            strQuery += "( type VARCHAR2(255)";
            strQuery += ", name VARCHAR2(255)";
            strQuery += ", status VARCHAR2(256)";
            strQuery += ", last_update DATE";
            strQuery += ", mutex VARCHAR2(256)";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            strQuery = "ALTER TABLE " + strTableName + " MODIFY";
            strQuery += "( type DEFAULT NULL";
            strQuery += ", name DEFAULT NULL";
            strQuery += ", status DEFAULT NULL";
            strQuery += ", last_update DEFAULT NULL";
            strQuery += ", mutex DEFAULT NULL";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        // Increase the cpu field size
        InsertIntoUpdateLog("         Changing type of cpu colmuns");
        if (m_pDatabaseConnector->IsMySqlDB() ||
            m_pDatabaseConnector->IsSQLiteDB())
        {
            strQuery =
                "ALTER TABLE " + strTableName +
                " MODIFY COLUMN cpu VARCHAR(1024)";
        }
        else if (m_pDatabaseConnector->IsOracleDB())
        {
            strQuery =
                "ALTER TABLE " + strTableName +
                " MODIFY ( cpu VARCHAR2(1024))";
        }
        if (! clQuery.exec(strQuery))
        {
            if (m_pDatabaseConnector->IsOracleDB() &&
                clQuery.lastError().text().contains("ORA-01451",
                                                    Qt::CaseInsensitive))
            {
                // Column already on VARCHAR2(1024)
                // ignore this error
            }
            else
            {
                SetLastError(clQuery);
                return false;
            }
        }

        // Populate name
        strQuery = "UPDATE " + strTableName;
        strQuery +=
            " SET name="
            "CONCAT(host_name,CONCAT('[',CONCAT(gex_server_profile,']')))";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    // CREATE ym_actions table
    strTableName = "ym_actions";
    // Check if already updated
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strQuery =
            "SELECT TABLE_NAME FROM information_schema.tables "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "' AND TABLE_NAME='" + strTableName + "'";
    }
    else
    {
        strQuery =
            "SELECT TABLE_NAME FROM sys.USER_TABLES "
            "WHERE upper(TABLE_NAME)=upper('" + strTableName + "')";
    }
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if (! clQuery.first())
    {
        // Have to create
        // Change to nullable
        InsertIntoUpdateLog("* CREATE " + strTableName + " TABLE:");
        InsertIntoUpdateLog("         to store new actions flow");

        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "CREATE TABLE " + strTableName + " (";
            strQuery += "      creation_time datetime NOT NULL,";
            strQuery +=
                "      action_id int(10) unsigned NOT NULL AUTO_INCREMENT,";
            strQuery += "      category varchar(256) NOT NULL,";
            strQuery += "      type varchar(256) NOT NULL,";
            strQuery += "      start_time datetime DEFAULT NULL,";
            strQuery += "      node_list varchar(256) DEFAULT '|',";
            strQuery += "      mutex varchar(256) DEFAULT NULL,";
            strQuery += "      task_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      database_id int(10) unsigned DEFAULT NULL,";
            strQuery += "      status varchar(256) DEFAULT NULL,";
            strQuery += "      command varchar(256) DEFAULT NULL,";
            strQuery += "      PRIMARY KEY (action_id)";
            strQuery += "    ) ENGINE=InnoDB DEFAULT CHARSET=latin1";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            strQuery = "CREATE TABLE " + strTableName + " (";
            strQuery += "      creation_time DATE NOT NULL,";
            strQuery += "      action_id NUMBER(11) NOT NULL,";
            strQuery += "      category VARCHAR2(256) NOT NULL,";
            strQuery += "      type VARCHAR2(256) NOT NULL,";
            strQuery += "      start_time DATE DEFAULT NULL,";
            strQuery += "      node_list VARCHAR2(256) DEFAULT '|',";
            strQuery += "      mutex VARCHAR2(256) DEFAULT NULL,";
            strQuery += "      task_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      database_id NUMBER(11) DEFAULT NULL,";
            strQuery += "      status VARCHAR2(256) DEFAULT NULL,";
            strQuery += "      command VARCHAR2(256) DEFAULT NULL,";
            strQuery += "      PRIMARY KEY (action_id)";
            strQuery +=
                "    ) TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            // Create sequence for oracle
            strQuery = "CREATE SEQUENCE " + strTableName + "_sequence";
            strQuery += " START WITH 1 INCREMENT BY 1 NOCACHE";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    // UPDATE ym_databases table
    // ADD last_update,mutex columns if it do not already exist
    strTableName = "ym_databases";
    clRecords =
        QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName).
        record(strTableName);
    if (! clRecords.contains("last_update"))
    {
        // Add columns
        InsertIntoUpdateLog("* UPDATE " + strTableName + " TABLE:");
        InsertIntoUpdateLog("         ADD new colmuns");
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "ALTER TABLE " + strTableName + " ADD COLUMN ";
            strQuery += "( type VARCHAR(30) DEFAULT NULL";
            strQuery += ", last_update DATETIME DEFAULT NULL";
            strQuery += ", mutex VARCHAR(256) DEFAULT NULL";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            // No Default value:  unsupported add column
            // (if default value) operation on compressed table
            strQuery = "ALTER TABLE " + strTableName + " ADD ";
            strQuery += "( type VARCHAR2(30)";
            strQuery += ", last_update DATE";
            strQuery += ", mutex VARCHAR2(256)";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            strQuery = "ALTER TABLE " + strTableName + " MODIFY ";
            strQuery += "( type DEFAULT NULL";
            strQuery += ", last_update DEFAULT NULL";
            strQuery += ", mutex DEFAULT NULL";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    // UPDATE ym_users table
    // ADD last_update,mutex columns if it do not already exist
    strTableName = "ym_users";
    clRecords =
        QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName).
        record(strTableName);
    if (! clRecords.contains("last_update"))
    {
        // Add columns
        InsertIntoUpdateLog("* UPDATE " + strTableName + " TABLE:");
        InsertIntoUpdateLog("         ADD new colmuns");
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "ALTER TABLE " + strTableName + " ADD COLUMN ";
            strQuery += "( last_update DATETIME DEFAULT NULL";
            strQuery += ", mutex VARCHAR(256) DEFAULT NULL";
            strQuery += ")";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            // No Default value:  unsupported add column
            // (if default value) operation on compressed table
            strQuery = "ALTER TABLE " + strTableName + " ADD ";
            strQuery += "( last_update DATE";
            strQuery += ", mutex VARCHAR2(256)";
            strQuery += ")";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            strQuery = "ALTER TABLE " + strTableName + " MODIFY ";
            strQuery += "( last_update DEFAULT NULL";
            strQuery += ", mutex DEFAULT NULL";
            strQuery += ")";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    // Update ym_tasks => convert DATAPUMP to OLD_DATAPUMP
    // ignore tasks without *.gtf and *.js and *.* in the extension
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strQuery = "UPDATE ym_tasks T INNER JOIN ym_tasks_options O ";
        strQuery += " ON T.task_id=O.task_id ";
        strQuery += " AND T.type=1 AND O.field='ImportFileExtensions' ";
        strQuery +=
            " AND (O.value LIKE '%*.gtf%' OR "
            "O.value LIKE '%*.js%' OR O.value LIKE '%*.*%') ";
        strQuery += " SET T.type=99 ";
    }
    else
    {
        strQuery =
            "UPDATE ym_tasks SET type=99 "
            "WHERE task_id IN (SELECT O.task_id FROM ";
        strQuery +=
            " ym_tasks T INNER JOIN ym_tasks_options O ON T.task_id=O.task_id ";
        strQuery += " AND T.type=1 AND O.field='ImportFileExtensions' ";
        strQuery +=
            " AND (O.value LIKE '%*.gtf%' "
            "OR O.value LIKE '%*.js%' OR O.value LIKE '%*.*%'))";
    }
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }


    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','8')";
    clQuery.exec(strQuery);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    m_bFirstActivation = true;

    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B8_to_B9
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
// Update ym_events table - Add partitions
bool AdminEngine::AdminServerUpdate_B8_to_B9()
{

    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.0 B8 -> YieldManDb V3.0 B9");

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Update ym_nodes table
        QString strTableName;

        // CREATE ym_events table
        strTableName = "ym_events";
        // Check if already updated
        strQuery =
            "SELECT PARTITION_NAME FROM information_schema.PARTITIONS "
            "WHERE TABLE_NAME='" + strTableName +
            "' AND TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "'";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.first() && clQuery.value(0).toString().isEmpty())
        {
            // Have to create
            // Modify the primary key for partitioning
            InsertIntoUpdateLog("* UPDATE primary keys for " + strTableName +
                                " TABLE:");

            strQuery = "ALTER TABLE ym_events ";
            strQuery += "DROP PRIMARY KEY ";
            strQuery += ", ADD PRIMARY KEY (event_id,creation_time)";

            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            // Create partition for existing data
            InsertIntoUpdateLog("* ADD partitions for " + strTableName +
                                " TABLE:");

            strQuery =
                "SELECT DISTINCT DATE(creation_time), "
                "TO_DAYS(creation_time)+1 ";
            strQuery += " FROM " + strTableName + " ORDER BY creation_time";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
            strQuery = "ALTER TABLE " + strTableName +
                " PARTITION BY RANGE(TO_DAYS(creation_time)) (";
            while (clQuery.next())
            {
                strQuery += "PARTITION p" +
                    clQuery.value(0).toString().remove("-") +
                    " VALUES LESS THAN (" +
                    QString::number(clQuery.value(1).toInt()) + "),";
            }
            // Add the LastPart
            strQuery += "PARTITION LastPart VALUES LESS THAN (MAXVALUE))";
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            // Update the event for partitioning
            // To do in B10
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','9')";
    clQuery.exec(strQuery);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B9_to_B10
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
// Update ym_events table - partitions and index
bool AdminEngine::AdminServerUpdate_B9_to_B10()
{

    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.0 B9 -> YieldManDb V3.0 B10");

    QString strTableName, strColumn;
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Update ym_nodes table
        // UPDATE ym_events table
        strTableName = "ym_events";

        InsertIntoUpdateLog("* Update " + strTableName + " EVENT:");
        InsertIntoUpdateLog("         to store new events flow");

        // Update the event for partitioning
        // To do in B11
    }

    QString strGenericQuery;
    QStringList lstNullableField;
    lstNullableField << "ym_settings|value";
    lstNullableField << "ym_nodes_options|node_id";
    lstNullableField << "ym_nodes_options|value";
    lstNullableField << "ym_users_options|user_id";
    lstNullableField << "ym_users_options|value";
    // Check if already updated
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        strGenericQuery =
            "SELECT UPPER(IS_NULLABLE),COLUMN_TYPE "
            "FROM information_schema.columns WHERE TABLE_SCHEMA='" +
            m_pDatabaseConnector->m_strSchemaName +
            "' AND TABLE_NAME='%1' AND COLUMN_NAME='%2'";
    }
    else
    {
        strGenericQuery =
            "SELECT UPPER(NULLABLE), "
            "concat(DATA_TYPE,concat('(',concat(DATA_PRECISION,')'))) "
            "FROM sys.USER_TAB_COLUMNS WHERE TABLE_NAME='%1' "
            "AND COLUMN_NAME='%2'";
    }
    while (! lstNullableField.isEmpty())
    {
        QString Value = lstNullableField.takeFirst();
        if (m_pDatabaseConnector->IsOracleDB())
        {
            Value = Value.toUpper();
        }
        strTableName = Value.section("|", 0, 0);
        strColumn = Value.section("|", 1, 1);
        strQuery = strGenericQuery.arg(strTableName).arg(strColumn);
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().left(1) == "N")
        {
            // Change to nullable
            InsertIntoUpdateLog("* UPDATE " + strTableName + " TABLE:");
            InsertIntoUpdateLog("         MODIFY " + strColumn +
                                " type to nullable");

            if (m_pDatabaseConnector->IsMySqlDB())
            {
                strQuery =
                    "ALTER TABLE " + strTableName +
                    " MODIFY value " + clQuery.value(1).toString() + " NULL";
            }
            else
            {
                strQuery =
                    "ALTER TABLE " + strTableName +
                    " MODIFY ( value " + clQuery.value(1).toString() + " NULL)";
            }
            if (! clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    InsertIntoUpdateLog(
        "* Add predefined options to " + strTableName + " table :");
    InsertIntoUpdateLog("BI_SERVER option");
    strQuery = "DELETE FROM ym_settings WHERE field='BI_SERVER'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('BI_SERVER','')";
    clQuery.exec(strQuery);
    InsertIntoUpdateLog("LDAP_SERVER option");
    strQuery = "DELETE FROM ym_settings WHERE field='LDAP_SERVER'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('LDAP_SERVER','')";
    clQuery.exec(strQuery);


    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','10')";
    clQuery.exec(strQuery);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B10_to_B11
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
// Update ym_events table for events flow
// Update old events when CRASH -> UNEXPECTED
bool AdminEngine::AdminServerUpdate_B10_to_B11()
{
    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.0 B10 -> YieldManDb V3.0 B11");

    QString strTableName = "ym_events";
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Update ym_nodes table
        // UPDATE ym_events table
        InsertIntoUpdateLog("* Update " + strTableName + " EVENT:");
        InsertIntoUpdateLog("         to store new events flow");

        // Update the event for partitioning
        strQuery = "DROP EVENT IF EXISTS event_add_partition";
        clQuery.exec(strQuery);
        strQuery =
            "CREATE EVENT event_add_partition ON SCHEDULE EVERY 1 DAY STARTS '"
            +
            QDate::currentDate().addDays(1).toString("yyyy-MM-dd") +
            " 00:00:00' DO \n";
        strQuery += "BEGIN \n";
        strQuery += "  -- New partition name (DayAfter)\n";
        strQuery += "  DECLARE new_partition CHAR(32) DEFAULT     \n";
        strQuery +=
            "    CONCAT( 'p',         DATE_FORMAT( DATE_ADD( CURDATE(), INTERVAL 1 DAY ), '%Y%m%d' )         );\n";
        strQuery += "  -- New max value (DayAfter+1) \n";
        strQuery +=
            "  DECLARE max_day INTEGER DEFAULT TO_DAYS( CURDATE() ) + 2;\n";
        strQuery += "  DECLARE status_msg VARCHAR(1024);\n";
        strQuery += "  DECLARE status_value VARCHAR(10);\n";
        strQuery += "  DECLARE CONTINUE HANDLER FOR 1517 \n";
        strQuery += "    BEGIN\n";
        strQuery +=
            "      set @status_msg = CONCAT('Duplicate partition name ',new_partition);\n";
        strQuery += "      set @status_value = 'FAIL';\n";
        strQuery += "    END;\n";
        strQuery += "  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION \n";
        strQuery += "    BEGIN\n";
        strQuery += "      set @status_msg = 'Error occurred';\n";
        strQuery += "      set @status_value = 'FAIL';\n";
        strQuery += "    END;\n";
        strQuery += "\n";
        strQuery += "   -- Update the Events table\n";
        strQuery += "  SET @StartTime = now(); \n";
        strQuery += "  SET @status_value = 'PASS';\n";
        strQuery +=
            "  SET @status_msg = CONCAT('New partition name ',new_partition);\n";
        strQuery += "  INSERT INTO ym_events \n";
        strQuery +=
            "    VALUES(now(),null,null,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,\n";
        strQuery +=
            "    'Execute=event_add_partition','START','Event scheduler running',''); \n";
        strQuery += "  SET @event_id = LAST_INSERT_ID(); \n";
        strQuery += "\n";
        strQuery += "  -- Prepare split LastPartition \n";
        strQuery +=
            "  SET @stmt = CONCAT('ALTER TABLE ym_events REORGANIZE PARTITION LastPart INTO (PARTITION ',\n";
        strQuery +=
            "    new_partition,         ' VALUES LESS THAN (',          max_day,          '),\n";
        strQuery += "    PARTITION LastPart VALUES LESS THAN MAXVALUE)');\n";
        strQuery +=
            "  PREPARE stmt FROM @stmt;  -- Execute and split the Last partition\n";
        strQuery += "  EXECUTE stmt; \n";
        strQuery += "\n";
        strQuery += "  -- Update the Events table\n";
        strQuery += "  INSERT INTO ym_events \n";
        strQuery +=
            "    VALUES(now(),null,@event_id,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,\n";
        strQuery +=
            "    'Execute=event_add_partition',@status_value,@status_msg,''); \n";
        strQuery += "END";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    InsertIntoUpdateLog("* Update " + strTableName + " EVENT:");
    InsertIntoUpdateLog("         modify CRASH events flow");

    strQuery = "UPDATE " + strTableName;
    strQuery +=
        " SET status='UNEXPECTED', summary='error=Action ended unexpectedly'";
    strQuery += " WHERE status='CRASH'";
    if (! clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','11')";
    clQuery.exec(strQuery);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B11_to_B12
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
// Check if the ym_databases_options exists (fix creation script)
bool AdminEngine::AdminServerUpdate_B11_to_B12()
{
    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.0 B11 -> YieldManDb V3.0 B12");

    QString strTableName;
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    // Check if ym_databases_options table exists
    strTableName = "ym_databases_options";

    QStringList Tables;
    m_pDatabaseConnector->EnumTables(Tables);
    if (! Tables.contains(strTableName, Qt::CaseInsensitive))
    {
        // Table not exists
        // Have to create
        // Change to nullable
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.write("* CREATE ym_databases_options TABLE:\n");
        }
        if (m_hUpdateDbLogFile.isOpen())
        {
            m_hUpdateDbLogFile.write("         to store new options field\n");
        }

        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery = "CREATE TABLE `ym_databases_options` (";
            strQuery += "      `database_id` int(11) NOT NULL DEFAULT '0',";
            strQuery += "      `field` varchar(256) NOT NULL DEFAULT '',";
            strQuery += "      `value` varchar(1024) NOT NULL DEFAULT ''";
            strQuery += "    ) ENGINE=InnoDB DEFAULT CHARSET=latin1";
        }
        else
        {
            strQuery = "CREATE TABLE ym_databases_options (";
            strQuery += "      database_id NUMBER(11) NOT NULL,";
            strQuery += "      field VARCHAR2(256) NOT NULL,";
            strQuery += "      value VARCHAR2(1024)";
            strQuery +=
                "    ) TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS";
        }
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(strQuery);
    strQuery = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.0')";
    clQuery.exec(strQuery);
    strQuery = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','12')";
    clQuery.exec(strQuery);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_B12_to_B13
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
bool AdminEngine::AdminServerUpdate_B12_to_B13()
{

    QString lMsg = "Updating YieldMan Server: %1...";

    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly |
            QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.0 B12 -> YieldManDb V3.1 B13");

    QString lTableName;
    QString lQueryContent;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(lMsg.arg("Schema modification"));
    QCoreApplication::processEvents();
    // Update ym_actions table
    lTableName = "ym_actions";
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Check if already updated
        lQueryContent =
            "SELECT COLUMN_TYPE FROM information_schema.columns "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "'";
        lQueryContent +=
            " AND TABLE_NAME='" + lTableName +
            "' AND COLUMN_NAME='command'";
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().toUpper() != "TEXT")
        {
            // Change VARCHAR(256) field to TEXT
            InsertIntoUpdateLog("* UPDATE " + lTableName + " TABLE:");
            InsertIntoUpdateLog("         MODIFY command type to TEXT");

            lQueryContent =
                "ALTER TABLE " + lTableName +
                " MODIFY " + lTableName + ".command TEXT";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }
    else if (m_pDatabaseConnector->IsOracleDB())
    {

        // Change VARCHAR(256) field to BLOB
        InsertIntoUpdateLog("* UPDATE " + lTableName + " TABLE:");
        InsertIntoUpdateLog("         MODIFY command type to CLOB");

        lQueryContent = "DROP SEQUENCE " + lTableName + "_sequence";
        clQuery.exec(lQueryContent);
        lQueryContent = "DROP TABLE " + lTableName;
        clQuery.exec(lQueryContent);

        lQueryContent = "CREATE TABLE " + lTableName + " (";
        lQueryContent += "      creation_time DATE NOT NULL,";
        lQueryContent += "      action_id NUMBER(11) NOT NULL,";
        lQueryContent += "      category VARCHAR2(256) NOT NULL,";
        lQueryContent += "      type VARCHAR2(256) NOT NULL,";
        lQueryContent += "      start_time DATE DEFAULT NULL,";
        lQueryContent += "      node_list VARCHAR2(256) DEFAULT '|',";
        lQueryContent += "      mutex VARCHAR2(256) DEFAULT NULL,";
        lQueryContent += "      task_id NUMBER(11) DEFAULT NULL,";
        lQueryContent += "      database_id NUMBER(11) DEFAULT NULL,";
        lQueryContent += "      status VARCHAR2(256) DEFAULT NULL,";
        lQueryContent += "      command CLOB DEFAULT NULL,";
        lQueryContent += "      PRIMARY KEY (action_id)";
        lQueryContent +=
            "    ) TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS";
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }

        // Create sequence for oracle
        lQueryContent = "CREATE SEQUENCE " + lTableName + "_sequence";
        lQueryContent += " START WITH 1 INCREMENT BY 1 NOCACHE";
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    // Check if ym_databases_options table exists
    lTableName = "da_galaxy";

    QStringList lTablesList;
    m_pDatabaseConnector->EnumTables(lTablesList);
    if (lTablesList.contains(lTableName, Qt::CaseInsensitive))
    {
        // Drop table if exists
        lQueryContent = "DROP TABLE " + lTableName;
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    InsertIntoUpdateLog("* CREATE " + lTableName + " TABLE:");
    InsertIntoUpdateLog("         to store Users/Groups management system");

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        lQueryContent = "CREATE TABLE `" + lTableName + "` (\n"
            "`idda_galaxy` int(11) NOT NULL,\n"
            "`session_id` varchar(255) DEFAULT NULL,\n"
            "`directory` mediumTEXT,\n"
            "`checksum` int(10) DEFAULT NULL,\n"
            "`last_update` datetime DEFAULT NULL,\n"
            "PRIMARY KEY (`idda_galaxy`)\n"
            ") ENGINE=InnoDB DEFAULT CHARSET=latin1\n";
    }
    else
    {
        lQueryContent = "DROP TABLE " + lTableName;
        clQuery.exec(lQueryContent);
        lQueryContent = "CREATE TABLE " + lTableName + " (\n"
            "idda_galaxy NUMBER(11) NOT NULL,\n"
            "session_id VARCHAR2(256) DEFAULT NULL,\n"
            "directory CLOB,\n"
            "checksum NUMBER(10) DEFAULT NULL,\n"
            "last_update DATE DEFAULT NULL,\n"
            "PRIMARY KEY (idda_galaxy)\n"
            ") TABLESPACE ym_admin_db PCTFREE 5 PCTUSED 80 COMPRESS";
    }
    if (! clQuery.exec(lQueryContent))
    {
        SetLastError(clQuery);
        return false;
    }

    // Update users list
    InsertIntoUpdateLog("* POPULATE " + lTableName + " TABLE:");
    InsertIntoUpdateLog("         to store Users/Groups management system");

    if (m_mapUsers.isEmpty())
    {
        LoadUsersList();
    }

    GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(lMsg.arg("Loading Directory Access Plugin"));
    QCoreApplication::processEvents();
    if (! mDirAccessPlugin)
    {
        LoadDirectoryAccessPlugin();
    }

    if (! mDirAccessPlugin || ! mDirAccessPlugin->GetConnector())
    {
        return false;
    }

    ///////////////////////////////
    // TODO
    // Keep the B12 install version
    // for update the PWD admin into DA
    // TODO
    // Duplicate this code for new ym_admin_db creation
    ///////////////////////////////

    // Connect 'admin'
    // 'ymadmin'
    QMap<QString, QString> lConParam;
    lConParam.insert("dir_user", "admin");
    lConParam.insert("dir_pass", "ymadmin");

    if (! mDirAccessPlugin->GetConnector()->Connect(lConParam))
    {
        QString lMsg =
            "Directory Access error: " + mDirAccessPlugin->GetLastError();
        SetLastError(eDB_InvalidEntry, lMsg);
        return false;
    }

    // Update/create public groups
    // Add all users into PUBLIC
    // Update users list
    GS::DAPlugin::UsersBase* users = mDirAccessPlugin->GetUsers();
    GS::DAPlugin::GroupsBase* groups = mDirAccessPlugin->GetGroups();
    GS::DAPlugin::AppEntriesBase* applications =
        mDirAccessPlugin->GetAppEntries();
    if (! users || ! applications || ! groups)
    {
        QString lMsg =
            "Directory Access error: " + mDirAccessPlugin->GetLastError();
        SetLastError(eDB_InvalidEntry, lMsg);
        return false;
    }

    QTime timer;
    timer.start();
    int nUsers = 0;
    QString lUsersMsg = lMsg.arg("Importing %1 users");
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(lUsersMsg.arg(""));
    QCoreApplication::processEvents();
    // Set application privileges for administrators group
    foreach(AdminUser * lUser, m_mapUsers.values())
    {
        ++nUsers;
        if (timer.elapsed() > 200)
        {
            timer.start();
            GS::Gex::Engine::GetInstance().
                UpdateLabelStatus(lUsersMsg.arg(nUsers));
            QCoreApplication::processEvents();
        }

        // Update users
        // If customer created 'admin' or 'anonymous'!
        if (lUser->m_strLogin.toLower() == "anonymous")
        {
            lUser->m_strLogin = "user_anonymous";
        }
        // Fix the issue when creating ym_admin_db and user_sequence start to 2
        if ((lUser->m_strLogin.toLower() == "admin")
            && (m_mapUsers.count() > 1))
        {
            lUser->m_strLogin = "user_admin";
        }

        // REPLACE 'root' to 'admin'
        if (lUser->m_strLogin.toLower() == "root")
        {
            lUser->m_strLogin = "admin";
        }

        if (lUser->m_strPwd.isEmpty())
        {
            lUser->m_strPwd = "1234";
        }

        if (! users->Add(lUser->m_strLogin, lUser->m_strPwd))
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  users->GetLastError().toLatin1().constData());
        }


        // Update pwd if user already exists
        users->UpdateUserAttribute(lUser->m_strLogin,
                                   "password",
                                   lUser->m_strPwd);
        users->UpdateUserAttribute(lUser->m_strLogin, "email",
                                   lUser->m_strEmail);
        users->UpdateUserAttribute(lUser->m_strLogin, "name", lUser->m_strName);
        users->UpdateUserAttribute(lUser->m_strLogin,
                                   "creation_date",
                                   lUser->m_clCreationDate.toString(
                                       "yyyy-MM-dd HH:mm:ss"));

        // Add users to group
        groups->AddUser("public", lUser->m_strLogin);
        if (lUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        {
            applications->AddUserPrivilege("galaxy",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:users_groups_administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator:databases",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:yieldman",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:yieldman:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:patman",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:patman:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
        }
        else
        {
            // true is the default value for SHOW_ALL_USERS_RULES
            // (if not exists)
            // SHOW_ALL_USERS_RULES can be defined for all user
            // (user_id=NULL OR user_id=0)
            // and can be overwrite by each user (all false + user=2 true)
            // Option 1: add privileges for all user in the public group
            // Option 2: no privileges in the public group but for each users
            // if (lUser->GetAttribute("SHOW_ALL_USERS_RULES").toString() !=
            //     "true")
            //    applications->
            //        AddUserPrivilege("galaxy:examinator:administrator",
            //                         lUser->m_strLogin,0);
            // else
            //    Add this privileges for all existing users
            applications->AddUserPrivilege("galaxy:examinator:administrator",
                                           lUser->m_strLogin,
                                           1);
        }
    }

    GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(lMsg.arg("Save Directory Access Plugin"));
    QCoreApplication::processEvents();
    if (! mDirAccessPlugin->SaveChanges())
    {
        QString lMsg = "Directory Access error: " +
            mDirAccessPlugin->GetLastError();
        SetLastError(eDB_InvalidEntry, lMsg);
        return false;
    }

    // disconnect
    // and reconnect as anonymous
    if (m_pCurrentUser)
    {
        lConParam.insert("dir_user", m_pCurrentUser->m_strLogin);
        lConParam.insert("dir_pass", m_pCurrentUser->m_strPwd);
    }
    else
    {
        lConParam.insert("dir_user", "anonymous");
        lConParam.insert("dir_pass", "");
    }

    if (! mDirAccessPlugin->GetConnector()->ChangeUser(lConParam))
    {
        QString lMsg =
            "Directory Access error: " + mDirAccessPlugin->GetLastError();
        SetLastError(eDB_InvalidEntry, lMsg);
        return false;
    }

    // If no error, remove admin privileges from ym_users table
    // Update default pwd
    foreach(AdminUser* lUser, m_mapUsers.values())
    {

        if (lUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        {
            // DISABLE MASTER PRIVILEGES FOR V7.0
            lUser->m_nType = YIELDMANDB_USERTYPE_USER;
        }
        SaveUser(lUser);
    }

    // Reload users list if updated
    LoadUsersList();

    GS::Gex::Engine::GetInstance().UpdateLabelStatus();
    QCoreApplication::processEvents();

    lQueryContent =
        "DELETE FROM ym_settings WHERE field LIKE 'DB_VERSION_NAME%'";
    clQuery.exec(lQueryContent);
    lQueryContent = "DELETE FROM ym_settings WHERE field LIKE 'DB_VERSION_NB%'";
    clQuery.exec(lQueryContent);
    lQueryContent = "DELETE FROM ym_settings WHERE field LIKE 'DB_BUILD_NB%'";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','" +
        NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.1')";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','13')";
    clQuery.exec(lQueryContent);

#if 0
    // For QA - RETRO COMPATIBILITY FOR MONITORING
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME_FOR_QA','"
        + NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + " for QA')";
    lQuery.exec(lQueryContent);
    lQueryContent =
        "INSERT INTO ym_settings VALUES('DB_VERSION_NB_FOR_QA','3.0')";
    lQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_BUILD_NB_FOR_QA','12')";
    lQuery.exec(lQueryContent);
#endif

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    m_bFirstActivation = true;
    return true;
}

#ifdef DA_GALAXY_REGENERATE

bool AdminEngine::AdminServerUpdate_dagalaxy()
{

    QString lMsg = "Updating YieldMan Server: %1...";

    // Force to use TEST
    m_pDatabaseConnector->m_strDatabaseName = "test";
    m_pDatabaseConnector->m_strSchemaName = "test";
    QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName).setDatabaseName("test");

    QString lTableName;
    QString lQueryContent;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    int             iIndex;
    AdminUser       *pUser=0;

    // Check if ym_databases_options table exists
    lTableName = "da_galaxy";

    // Drop table if exists
    lQueryContent = "DROP TABLE IF EXISTS test." + lTableName;
    if (! clQuery.exec(lQueryContent))
    {
        SetLastError(clQuery);
        return false;
    }

    lQueryContent = "CREATE TABLE test." + lTableName + " (\n"
        "`idda_galaxy` int(11) NOT NULL,\n"
        "`session_id` varchar(255) DEFAULT NULL,\n"
        "`directory` mediumTEXT,\n"
        "`checksum` int(10) DEFAULT NULL,\n"
        "`last_update` datetime DEFAULT NULL,\n"
        "PRIMARY KEY (`idda_galaxy`)\n"
        ") ENGINE=InnoDB DEFAULT CHARSET=latin1\n";

    if (! clQuery.exec(lQueryContent))
    {
        SetLastError(clQuery);
        return false;
    }

    // Update users list
    InsertIntoUpdateLog("* POPULATE " + lTableName + " TABLE:");
    InsertIntoUpdateLog("         to store Users/Groups management system");

    // VISHAY HACK
    lQueryContent = "SELECT user_id, group_id, login, pwd, name, email, type, os_login, profile_id, creation_date, expiration_date, last_access, last_update "
               "FROM test.ym_users";
    if(!clQuery.exec(lQueryContent))
    {
        SetLastError(clQuery);
        return false;
    }

    // Load users
    QMap<int,AdminUser*> lmapUsers;
    while(clQuery.next())
    {
        iIndex = 0;

        // Check if already in the list
        if(lmapUsers.contains(clQuery.value(0).toInt()))
            pUser = lmapUsers[clQuery.value(0).toInt()];
        else
            pUser = new AdminUser;


        pUser->m_nUserId = clQuery.value(iIndex++).toInt();
        pUser->m_nGroupId = clQuery.value(iIndex++).toInt();
        pUser->m_strLogin = clQuery.value(iIndex++).toString();

        GexDbPlugin_Base::DecryptPassword(clQuery.value(iIndex++).toString(),pUser->m_strPwd);
        pUser->m_strName = clQuery.value(iIndex++).toString();
        pUser->m_strEmail = clQuery.value(iIndex++).toString();
        pUser->m_nType = clQuery.value(iIndex++).toInt();
        pUser->m_strOsLogin = clQuery.value(iIndex++).toString();
        pUser->m_nProfileId = clQuery.value(iIndex++).toInt();
        pUser->m_clCreationDate = clQuery.value(iIndex++).toDateTime();
        pUser->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();
        pUser->m_clAccessDate = clQuery.value(iIndex++).toDateTime();
        pUser->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();

        lmapUsers[pUser->m_nUserId] = pUser;
    }

    if (mDirAccessPlugin)
    {
        if(mDirAccessPlugin->GetConnector()
                && mDirAccessPlugin->GetConnector()->IsConnected())
            mDirAccessPlugin->GetConnector()->Disconnect();

        delete mDirAccessPlugin;
        mDirAccessPlugin = NULL;
    }

    if (! mDirAccessPlugin)
    {
        LoadDirectoryAccessPlugin();
    }

    if (! mDirAccessPlugin || ! mDirAccessPlugin->GetConnector())
    {
        return false;
    }

    // Connect 'admin'
    // 'ymadmin'
    QMap<QString, QString> lConParam;
    lConParam.insert("dir_user", "admin");
    lConParam.insert("dir_pass", "ymadmin");
    lConParam.insert("sql_shema", "test");

    if (! mDirAccessPlugin->GetConnector()->Connect(lConParam))
    {
        return false;
    }

    // Update/create public groups
    // Add all users into PUBLIC
    // Update users list
    GS::DAPlugin::UsersBase* users = mDirAccessPlugin->GetUsers();
    GS::DAPlugin::GroupsBase* groups = mDirAccessPlugin->GetGroups();
    GS::DAPlugin::AppEntriesBase* applications =
        mDirAccessPlugin->GetAppEntries();
    if (! users || ! applications || ! groups)
    {
        return false;
    }

    QTime timer;
    timer.start();
    int nUsers = 0;
    QString lUserLogin;
    QString lUsersMsg = lMsg.arg("Importing %1 users");
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(lUsersMsg.arg(""));
    QCoreApplication::processEvents();
    // Set application privileges for administrators group
    foreach(AdminUser * lUser, lmapUsers.values())
    {
        ++nUsers;

        lUserLogin = lUser->m_strLogin;

        // Update users
        if (lUser->m_strPwd.isEmpty())
        {
            lUser->m_strPwd = "1234";
        }

        if (! users->Add(lUser->m_strLogin, lUser->m_strPwd))
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  users->GetLastError().toLatin1().constData());
        }


        // Update pwd if user already exists
        users->UpdateUserAttribute(lUser->m_strLogin,
                                   "password",
                                   lUser->m_strPwd);
        users->UpdateUserAttribute(lUser->m_strLogin, "email",
                                   lUser->m_strEmail);
        users->UpdateUserAttribute(lUser->m_strLogin, "name", lUser->m_strName);
        users->UpdateUserAttribute(lUser->m_strLogin,
                                   "creation_date",
                                   lUser->m_clCreationDate.toString(
                                       "yyyy-MM-dd HH:mm:ss"));

        // Add users to group
        groups->AddUser("public", lUser->m_strLogin);
        if (lUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        {
            applications->AddUserPrivilege("galaxy",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:users_groups_administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:examinator:databases",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:yieldman",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:yieldman:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:patman",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
            applications->AddUserPrivilege("galaxy:patman:administrator",
                                           lUser->m_strLogin,
                                           GS::DAPlugin::READACCESS |
                                           GS::DAPlugin::WRITEACCESS);
        }
        else
        {
            // true is the default value for SHOW_ALL_USERS_RULES
            // (if not exists)
            // SHOW_ALL_USERS_RULES can be defined for all user
            // (user_id=NULL OR user_id=0)
            // and can be overwrite by each user (all false + user=2 true)
            // Option 1: add privileges for all user in the public group
            // Option 2: no privileges in the public group but for each users
            // if (lUser->GetAttribute("SHOW_ALL_USERS_RULES").toString() !=
            //     "true")
            //    applications->
            //        AddUserPrivilege("galaxy:examinator:administrator",
            //                         lUser->m_strLogin,0);
            // else
            //    Add this privileges for all existing users
            applications->AddUserPrivilege("galaxy:examinator:administrator",
                                           lUser->m_strLogin,
                                           1);
        }
    }
    if (! mDirAccessPlugin->SaveChanges())
    {
        return false;
    }

    return true;
}
#endif
/******************************************************************************!
 * \fn AdminServerUpdate_B13_to_B14
 * \return false on error
 ******************************************************************************/
// BUILD UPDATE
// compatible with previous version
bool AdminEngine::AdminServerUpdate_B13_to_B14()
{

    QString lMsg = "Updating YieldMan Server: %1...";

    if (! m_hUpdateDbLogFile.isOpen())
    {
        CGexSystemUtils clSysUtils;
        clSysUtils.NormalizePath(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
        m_hUpdateDbLogFile.open(
            QIODevice::QIODevice::WriteOnly | QIODevice::Append |
            QIODevice::Text);
    }

    InsertIntoUpdateLog("YieldManDb V3.1 B13 -> YieldManDb V3.1 B14");

    QString lTableName;
    QString lQueryContent;
    QSqlQuery clQuery(QSqlDatabase::database(
                          m_pDatabaseConnector->m_strConnectionName));

    GS::Gex::Engine::GetInstance().
        UpdateLabelStatus(lMsg.arg("Schema modification"));
    QCoreApplication::processEvents();
    // Update ym_actions table
    lTableName = "ym_tasks_options";
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Check if already updated
        lQueryContent =
            "SELECT COLUMN_TYPE FROM information_schema.columns "
            "WHERE TABLE_SCHEMA='" + m_pDatabaseConnector->m_strSchemaName +
            "'";
        lQueryContent +=
            " AND TABLE_NAME='" + lTableName +
            "' AND COLUMN_NAME='value'";
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().toUpper() != "TEXT")
        {
            // Change VARCHAR(4096) field to TEXT
            InsertIntoUpdateLog("* UPDATE " + lTableName + " TABLE:");
            InsertIntoUpdateLog("         MODIFY value type to TEXT");

            lQueryContent =
                "ALTER TABLE " + lTableName +
                " MODIFY " + lTableName + ".value TEXT";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }
    else if (m_pDatabaseConnector->IsOracleDB())
    {

        // Change VARCHAR(4096) field to BLOB
        InsertIntoUpdateLog("* UPDATE " + lTableName + " TABLE:");
        InsertIntoUpdateLog("         MODIFY `value` type to CLOB");

        // Check
        lQueryContent =
            "SELECT data_type FROM user_tab_columns "
            "WHERE table_name='YM_TASKS_OPTIONS' AND column_name='VALUE'";
        if (! clQuery.exec(lQueryContent))
        {
            SetLastError(clQuery);
            return false;
        }
        if (! clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        if (clQuery.value(0).toString().toUpper() != "CLOB")
        {
            // Change VARCHAR(4096) field to CLOB
            lQueryContent = "ALTER TABLE " + lTableName +
                " ADD value_tmp CLOB NULL";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
            lQueryContent = "UPDATE " + lTableName + " SET value_tmp = value";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
            lQueryContent = "ALTER TABLE " + lTableName +
                " SET UNUSED COLUMN value";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
            lQueryContent = "ALTER TABLE " + lTableName + " DROP UNUSED COLUMN";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
            lQueryContent = "ALTER TABLE " + lTableName +
                " RENAME COLUMN value_tmp TO value";
            if (! clQuery.exec(lQueryContent))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    lQueryContent = "DELETE FROM ym_settings WHERE field='DB_VERSION_NAME'";
    clQuery.exec(lQueryContent);
    lQueryContent = "DELETE FROM ym_settings WHERE field='DB_VERSION_NB'";
    clQuery.exec(lQueryContent);
    lQueryContent = "DELETE FROM ym_settings WHERE field='DB_BUILD_NB'";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_VERSION_NAME','"
        + NormalizeStringToSql(YIELDMANDB_VERSION_NAME, false) + "')";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_VERSION_NB','3.1')";
    clQuery.exec(lQueryContent);
    lQueryContent = "INSERT INTO ym_settings VALUES('DB_BUILD_NB','14')";
    clQuery.exec(lQueryContent);

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}

/******************************************************************************!
 * \fn UpdateDb_FromSqlScript
 * \return false on error
 ******************************************************************************/
bool AdminEngine::AdminServerUpdate_FromSqlScript(UINT fromBuild, UINT toBuild)
{
    if (fromBuild < 13)
    {
        SetLastError(eDB_InvalidEntry, QString("Invalid fromBuild[%1] for AdminServerUpdate_FromSqlScript").arg(fromBuild));
        return false;
    }
    if (toBuild > YIELDMANDB_BUILD_NB)
    {
        SetLastError(eDB_InvalidEntry, QString("Invalid toBuild[%1] for AdminServerUpdate_FromSqlScript").arg(toBuild));
        return false;
    }

    QDir lDir;
    QString lFileTemplate, lMessage, lValue;
    QStringList lUpdateFiles, lSqlFilesToProcess;
    QFile lFileToSource;
    QTextStream lSqlStream(&lFileToSource);
    QMap<UINT, QString> lUpdatesMap;
    UINT lVersion;

    GS::Gex::Engine::GetInstance().UpdateLabelStatus("");
    QCoreApplication::processEvents();
    // Get the list of update files
    QString lUpdateFilesPath = GS::Gex::Engine::GetInstance().Get(
            "ApplicationDir").toString() + QDir::separator();
    lUpdateFilesPath += "install/mysql/";
    lDir.setPath(lUpdateFilesPath);

    lFileTemplate = "ym_admin_db_mysql_update_*.sql";

    lUpdateFiles = lDir.entryList(QStringList(lFileTemplate),
                                  QDir::Files, QDir::Name);

    // record update versions retrieved
    while (! lUpdateFiles.isEmpty())
    {
        lValue = lUpdateFiles.takeFirst();
        lVersion =
            lValue.toLower().section("_b", 1).section("_to", 0, 0).toUInt();
        lUpdatesMap[lVersion] = lUpdateFilesPath + lValue;
    }

    // check if have all versions
    for (lVersion = fromBuild; lVersion < toBuild; ++lVersion)
    {
        if (! lUpdatesMap.contains(lVersion))
        {
            // error
            QString lMissingFile = lFileTemplate;
            lMissingFile.replace('*', "b" + QString::number(
                                     lVersion) + "_to_b" +
                                 QString::number(lVersion + 1));
            InsertIntoUpdateLog(
                "<b> ***************************************************</b>");
            InsertIntoUpdateLog(
                "<b> -------------------   WARNING   -------------------</b>");
            InsertIntoUpdateLog(
                "<b> Sql update script not found (" +
                lUpdateFilesPath + lMissingFile + ") !</b>");
            InsertIntoUpdateLog(
                "<b> ***************************************************</b>");
            SetLastError(eDB_InvalidEntry, QString("Sql update script not found (%1)").arg(lUpdateFilesPath + lMissingFile));
            return false;
        }
    }

    // File containing common STORED PROCEDURES/FUCTIONS required for an update
    lSqlFilesToProcess << lUpdateFilesPath + "common_update_initialize.sql";
    lSqlFilesToProcess << lUpdateFilesPath + "admin_update_initialize.sql";
    // Updates sql scripts
    for (lVersion = fromBuild; lVersion < toBuild; ++lVersion)
    {
        lSqlFilesToProcess << lUpdatesMap[lVersion];
    }
    // File to drop common STORED PROCEDURES/FUCTIONS required for an update
    lSqlFilesToProcess << lUpdateFilesPath + "admin_update_finalize.sql";
    lSqlFilesToProcess << lUpdateFilesPath + "common_update_finalize.sql";

    // Take the first script to execute
    while (! lSqlFilesToProcess.isEmpty())
    {
        lFileToSource.setFileName(lSqlFilesToProcess.takeFirst());
        InsertIntoUpdateLog("o Execute " + lFileToSource.fileName() + "...");
        if (! lFileToSource.open(QIODevice::ReadOnly))
        {
            lMessage = lFileToSource.errorString();
            SetLastError(eDB_InvalidEntry, lMessage);
            InsertIntoUpdateLog(lMessage);
            return false;
        }

        // count line for progress bar
        int lLinesCount = 0;
        while (! lSqlStream.atEnd())
        {
            lSqlStream.readLine();
            ++lLinesCount;
        }
        // Init for progress bar
        //        int lProgress = 0;
        //        ResetProgress(false);
        //        SetMaxProgress(lLinesCount);
        // Reset stream
        lSqlStream.seek(0);

        QString lQueryContent, lDelimiter, lLine;
        QSqlQuery lQuery =
            QSqlQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

        lQueryContent = "";
        lDelimiter = ";";
        while (! lSqlStream.atEnd())
        {
            lLine = lSqlStream.readLine();
            if (lLine.simplified().isEmpty())
            {
                continue;
            }
            if (lLine.simplified().startsWith("--") && (lDelimiter == ";"))
            {
                continue;
            }
            if (lLine.simplified().startsWith("exit;"))
            {
                break;
            }
            if (lLine.simplified().startsWith("DEFINE", Qt::CaseInsensitive))
            {
                continue;
            }
            if (lLine.simplified().startsWith("CAUTION", Qt::CaseInsensitive))
            {
                continue;
            }
            if (lLine.simplified().startsWith("INFO", Qt::CaseInsensitive))
            {
                continue;
            }
            if (lLine.simplified().startsWith("QUESTION", Qt::CaseInsensitive))
            {
                continue;
            }
            if (lLine.simplified().startsWith("DELIMITER", Qt::CaseInsensitive))
            {
                lDelimiter = lLine.simplified().section(" ", 1, 1);
                continue;
            }

            lQueryContent += lLine;
            if (lLine.indexOf(lDelimiter) >= 0)
            {
                lQueryContent = lQueryContent.remove(lDelimiter);
                // Execute the query
                /// TODO find a way to detect error on exec when the query is
                /// a "CALL " to a stored procedure
                if (! lQuery.exec(lQueryContent))
                {
                    // If the error is ALREADY EXISTS and
                    // the query contains the comment /* IF NOT EXISTS */
                    // then ignore this error
                    if (lQueryContent.section("(", 0, 0).
                        contains("IF NOT EXISTS", Qt::CaseInsensitive) &&
                        (lQuery.lastError().number() == 1304))
                    {
                        // Ignore this error
                    }
                    else
                    {
                        SetLastError(eDB_Query,
                                     lQuery.lastQuery().left(1024),
                                     QString::number(lQuery.lastError().number()),
                                     lQuery.lastError().text());
                        GetLastError(lMessage);
                        InsertIntoUpdateLog(lMessage);
                        return false;
                    }
                }

                lQueryContent = lQueryContent.simplified();
                if (lQueryContent.startsWith("CREATE ", Qt::CaseInsensitive))
                {
                    lQueryContent =
                        lQueryContent.remove("IF NOT EXISTS").simplified();
                    lQueryContent =
                        lQueryContent.remove("/* */").remove("/**/");
                    if (lQueryContent.count("(") > 0)
                    {
                        lQueryContent = lQueryContent.section("(", 0, 0);
                    }
                    InsertIntoUpdateLog(lQueryContent.section(" ",
                                                              0,
                                                              2) + "... DONE");
                }
                else if (lQueryContent.startsWith("DROP ", Qt::CaseInsensitive))
                {
                    lQueryContent =
                        lQueryContent.remove("IF EXISTS").simplified();
                    if (lQueryContent.count("(") > 0)
                    {
                        lQueryContent = lQueryContent.section("(", 0, 0);
                    }
                    InsertIntoUpdateLog(lQueryContent.section(" ",
                                                              0,
                                                              2) + "... DONE");
                }
                else if (lQueryContent.startsWith("CALL ", Qt::CaseInsensitive)
                         && (! lQueryContent.contains("log_message",
                                                      Qt::CaseInsensitive)))
                {
                    InsertIntoUpdateLog(lQueryContent.section(";",
                                                              0,
                                                              0) + "... DONE");
                    // check CALL status
                    QSqlQuery lCheckStatus =
                        QSqlQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                                         m_strConnectionName));
                    QString lCheckContent = "select @status,@message";
                    if (! lCheckStatus.exec(lCheckContent))
                    {
                        SetLastError(eDB_Query,
                                     lCheckStatus.lastQuery().left(1024),
                                     QString::number(lCheckStatus.lastError().number()),
                                     lCheckStatus.lastError().text());
                        GetLastError(lMessage);
                        InsertIntoUpdateLog(lMessage);
                        return false;
                    }
                    if (! lCheckStatus.first())
                    {
                        lMessage = "Error no status retrieved for " +
                            lQueryContent.section(";", 0, 0);
                        InsertIntoUpdateLog(lMessage);
                        SetLastError(eDB_InvalidEntry, lMessage);
                        return false;
                    }
                    else
                    {
                        int lStatus = lCheckStatus.value(0).toInt();
                        if (lStatus == 0)
                        {
                            SetLastError(eDB_Query,
                                         lQueryContent.section(";",0,0).left(1024),
                                         QString::number(lStatus),
                                         lCheckStatus.value(1).toString());
                            GetLastError(lMessage);
                            InsertIntoUpdateLog(lMessage);
                            return false;
                        }
                    }
                }
                lQueryContent = "";
            }
            else
            {
                lQueryContent += "\n";
            }
            //            ++lProgress;
            //            SetProgress(lProgress);
        }
        // move progress bar to max
        //        ResetProgress(true);
    }

    InsertIntoUpdateLog("o Success...");

    return true;
}

/******************************************************************************!
 * \fn InsertIntoUpdateLog
 * \return false on error
 ******************************************************************************/
bool AdminEngine::InsertIntoUpdateLog(QString Log)
{
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write(Log.toLatin1().constData());
    }
    if (m_hUpdateDbLogFile.isOpen())
    {
        m_hUpdateDbLogFile.write("\n");
    }
    m_UpdateDbLogContents.append(Log);
    return true;
}

/******************************************************************************!
 * \fn AdminServerUpdate_UpdateIndexes
 * \return false on error
 ******************************************************************************/
// Update DB:
// Check and update indexes for Oracle and MySql
// If lstIndexesToCheck is not empty, check indexes from it
bool AdminEngine::AdminServerUpdate_UpdateIndexes(QStringList lstIndexesToCheck)
{
    QString strQuery;
    QString strLogMessage;
    if (! m_pDatabaseConnector)
    {
        return false;
    }
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));
    unsigned int uiNbIndexesCreated = 0;

    /////////////////////////////////////////
    // Check if all tables with splitlot_id column have an index
    /////////////////////////////////////////

    InsertIntoUpdateLog("* Checking indexes");

    // MySql-specific
    QString strTable;
    QString strIndexes;
    QString strIndexName;
    QString strValue;
    QStringList lstTablesForIndexes;
    QStringList lstTableIndexesToCheck;

    // Check if have indexes to do
    // for all tables with splitlot_id column
    // Select tables with splitlot_id
    // Select indexes with splitlot_id
    // Then extract info and check needed indexes

    if (lstIndexesToCheck.isEmpty())
    {
        lstTableIndexesToCheck.append("database_id|%");
        lstTableIndexesToCheck.append("node_id|%");
        lstTableIndexesToCheck.append("group_id|%");
        lstTableIndexesToCheck.append("user_id|%");
        lstTableIndexesToCheck.append("task_id|%");
        lstTableIndexesToCheck.append("profile_id|%");
        lstTableIndexesToCheck.append("action_id|%");
        lstTableIndexesToCheck.append("log_id|%");
        lstTableIndexesToCheck.append("last_update|%");
        lstTableIndexesToCheck.append("status|%");
        lstTableIndexesToCheck.append("field|%");
        lstTableIndexesToCheck.append("mutex|%");
        lstTableIndexesToCheck.append("cpu|%");
        lstTableIndexesToCheck.append("login|%");
        lstTableIndexesToCheck.append("event_id|%");
        lstTableIndexesToCheck.append("link|%");
        lstTableIndexesToCheck.append("category|%");
        lstTableIndexesToCheck.append("type|%");
        lstTableIndexesToCheck.append("start_time|%");
        lstTableIndexesToCheck.append("status|%");
    }
    else
    {
        lstTableIndexesToCheck = lstIndexesToCheck;
    }

    while (! lstTableIndexesToCheck.isEmpty())
    {
        if (m_pDatabaseConnector->IsMySqlDB())
        {
            strQuery =
                "SELECT T0.TABLE_NAME, T0.INDEX_NAME, T0.MAX_SEQ, T0.INDEXES FROM ";
            strQuery += " ( ";
            strQuery +=
                "    SELECT T.TABLE_NAME, S.INDEX_NAME, MAX(S.SEQ_IN_INDEX) AS MAX_SEQ, ";
            strQuery +=
                "    GROUP_CONCAT(S.COLUMN_NAME ORDER BY S.SEQ_IN_INDEX SEPARATOR ',') AS INDEXES ";
            strQuery += "    FROM ";
            strQuery +=
                "         (SELECT DISTINCT COLUMNS.TABLE_SCHEMA, COLUMNS.TABLE_NAME ";
            strQuery += "          FROM information_schema.COLUMNS ";
            strQuery += "          INNER JOIN information_schema.TABLES ";
            strQuery += "          ON TABLES.TABLE_SCHEMA=COLUMNS.TABLE_SCHEMA";
            strQuery += "          AND TABLES.TABLE_NAME=COLUMNS.TABLE_NAME   ";
            strQuery += "          WHERE COLUMNS.TABLE_SCHEMA='" +
                m_pDatabaseConnector->m_strSchemaName + "' ";
            strQuery += "          AND COLUMNS.TABLE_NAME LIKE '%2'";
            strQuery += "          AND COLUMNS.COLUMN_NAME='%1' ";
            strQuery += "          AND TABLES.TABLE_TYPE='BASE TABLE'";
            strQuery += "         ) T ";
            strQuery += "    LEFT OUTER JOIN information_schema.STATISTICS S0 ";
            strQuery += "    ON T.TABLE_SCHEMA=S0.TABLE_SCHEMA ";
            strQuery += "    AND T.TABLE_NAME=S0.TABLE_NAME ";
            strQuery += "    AND S0.COLUMN_NAME='%1' ";
            strQuery += "    LEFT OUTER JOIN information_schema.STATISTICS S ";
            strQuery += "    ON T.TABLE_SCHEMA=S.TABLE_SCHEMA ";
            strQuery += "    AND T.TABLE_NAME=S.TABLE_NAME ";
            strQuery += "    AND S0.INDEX_NAME=S.INDEX_NAME ";
            strQuery += "    GROUP BY T.TABLE_NAME, S.INDEX_NAME ";
            strQuery += "  ) T0 ";
            strQuery += " ORDER BY T0.TABLE_NAME , T0.MAX_SEQ DESC";
        }
        else if (m_pDatabaseConnector->IsOracleDB())
        {
            strQuery =
                " SELECT T2.TABLE_NAME, T2.INDEX_NAME, MAX(T2.COLUMN_POSITION) AS MAX_SEQ, ";
            strQuery += " (T2.COLUMN_NAME) AS INDEXES , MAX(T2.COMPRESSION) ";
            strQuery += " FROM ";
            strQuery += " ( ";
            strQuery +=
                " SELECT T1.TABLE_NAME, C1.INDEX_NAME, C1.COLUMN_POSITION, C1.COLUMN_NAME, P.COMPRESSION ";
            strQuery += " FROM ";
            strQuery += " ( ";
            strQuery += "    SELECT DISTINCT T.TABLE_NAME, C.INDEX_NAME ";
            strQuery +=
                "    FROM (SELECT DISTINCT TABLE_NAME FROM USER_TAB_COLUMNS WHERE COLUMN_NAME='%1'";
            strQuery += "          AND TABLE_NAME LIKE '%2'";
            strQuery += "         ) T  ";
            strQuery += "         LEFT OUTER JOIN USER_IND_COLUMNS C ";
            strQuery += "    ON T.TABLE_NAME=C.TABLE_NAME ";
            strQuery += "    AND C.COLUMN_NAME='%1' ";
            strQuery += " ) T1 ";
            strQuery += " LEFT OUTER JOIN ";
            strQuery += " USER_IND_COLUMNS C1 ";
            strQuery += " ON T1.TABLE_NAME=C1.TABLE_NAME ";
            strQuery += " AND T1.INDEX_NAME=C1.INDEX_NAME ";
            strQuery += " LEFT OUTER JOIN USER_TAB_PARTITIONS P ";
            strQuery += " ON T1.TABLE_NAME=P.TABLE_NAME ";
            strQuery += " AND P.PARTITION_POSITION=1 ";
            strQuery +=
                " ORDER BY T1.TABLE_NAME, C1.INDEX_NAME, C1.COLUMN_POSITION ";
            strQuery += " )T2 ";
            strQuery +=
                " GROUP BY T2.TABLE_NAME, T2.INDEX_NAME, T2.COLUMN_POSITION, T2.COLUMN_NAME ";
            strQuery +=
                " ORDER BY T2.TABLE_NAME, T2.INDEX_NAME, T2.COLUMN_POSITION DESC";
        }
        else if (m_pDatabaseConnector->IsSQLiteDB())
        {
            strQuery = " SELECT T2.tbl_name, T2.name, 1, T2.sql FROM ";
            strQuery +=
                "( SELECT tbl_name FROM sqlite_master WHERE type='table'  AND lower(sql) LIKE '% %1 %' AND lower(NAME) LIKE '%2'";
            strQuery += ") AS T1 ";
            strQuery += "LEFT OUTER JOIN ";
            strQuery += "sqlite_master T2 ";
            strQuery += "ON ";
            strQuery += "T1.tbl_name=T2.tbl_name ";
            strQuery += "WHERE T2.type='index' ";
            strQuery += "ORDER BY T2.tbl_name, T2.name ";
        }
        else
        {
            return false;
        }

        strIndexName = lstTableIndexesToCheck.takeFirst();
        if (m_pDatabaseConnector->IsOracleDB())
        {
            strIndexName = strIndexName.toUpper();
        }

        strTable = strIndexName.section("|", 1, 1);
        strIndexName = strIndexName.section("|", 0, 0);
        strQuery = strQuery.arg(strIndexName.section(",", 0, 0), strTable);
        if (! clQuery.exec(strQuery))
        {
            goto updatedb_updateindexes_error;
        }
        strTable = "";

        // For each table
        // Check indexes
        while (clQuery.next())
        {
            if (strTable == clQuery.value(0).toString())
            {
                continue;
            }

            // Table Name
            strTable = clQuery.value(0).toString();
            // Index defined (empty if null)
            strValue = clQuery.value(3).toString();

            // Construct the needed index
            strIndexes = strIndexName;

            if (! strValue.isEmpty())
            {
                // Have an index defined
                if (m_pDatabaseConnector->IsSQLiteDB())
                {
                    strValue = strValue.section("(", 1);
                }
                // Check if it is the good one or more
                // Check with the existing index

                // For ORACLE
                if (m_pDatabaseConnector->IsOracleDB() &&
                    (clQuery.value(2).toInt() > 1))
                {
                    while (clQuery.value(2).toInt() > 1)
                    {
                        if (! clQuery.next())
                        {
                            break;
                        }
                        strValue.prepend(clQuery.value(3).toString() + ",");
                    }
                }
                if (strValue.startsWith(strIndexes, Qt::CaseInsensitive))
                {
                    continue;
                }
            }

            // Create new index
            // Create index TABLE_NAME on TABLE_NAME(INDEX1,INDEX2)
            strValue = strTable + "(" + strIndexes + ")";

            if (m_pDatabaseConnector->IsOracleDB())
            {
                // For Oracle
                // Specific creation when table partitionned and compressed
                if (! clQuery.value(4).toString().isEmpty())
                {
                    strValue += " LOCAL";
                }
                if (clQuery.value(4).toString() == "ENABLED")
                {
                    strValue += " COMPRESS";
                }

                strValue = strValue.toUpper();
            }

            // Before adding this index for creation
            // Check if have table in the next result
            if (clQuery.next())
            {
                QString strNextValue = clQuery.value(0).toString();
                clQuery.previous();
                if (strTable == strNextValue)
                {
                    strTable = "";
                    continue;
                }
            }

            lstTablesForIndexes.append(strValue);
        }

        // Have the list of all indexes to create
        // Start the process
        while (! lstTablesForIndexes.isEmpty())
        {
            strIndexes = lstTablesForIndexes.takeFirst();
            strTable = strIndexes.section("(", 0, 0);
            // construct a unique name not too long (oracle restriction)
            strIndexName = strIndexes.section(")", 0, 0).toUpper();
            strIndexName = strIndexName.
                remove("_ID").remove("_NAME").remove("_NO").remove("_").
                replace(",", "_").replace("(", "_");
            if (m_pDatabaseConnector->IsOracleDB())
            {
                strIndexName = strIndexName.left(30);
            }
            else
            {
                strIndexName = strIndexName.toLower();
            }

            strLogMessage = "Adding index to " + strIndexes;
            InsertIntoUpdateLog(strLogMessage);
            // Drop index if exist
            strQuery = "DROP INDEX " + strIndexName;
            if (m_pDatabaseConnector->IsMySqlDB())
            {
                strQuery += " ON " + strTable;
            }
            clQuery.exec(strQuery);

            // Then create a new one
            strQuery = "CREATE INDEX " + strIndexName;
            strQuery += " ON " + strIndexes;
            if (! clQuery.exec(strQuery))
            {
                goto updatedb_updateindexes_error;
            }

            uiNbIndexesCreated++;
        }
    }

    // Log status
    if (uiNbIndexesCreated == 0)
    {
        InsertIntoUpdateLog(" No new index");
    }
    else
    {
        InsertIntoUpdateLog(QString(" %1 indexes updated").
                            arg(uiNbIndexesCreated));
    }

    return true;

updatedb_updateindexes_error:
    // Write error message
    InsertIntoUpdateLog("");
    strLogMessage = "Status = ERROR (";
    strLogMessage += clQuery.lastError().text();
    strLogMessage += ").";
    InsertIntoUpdateLog(strLogMessage);

    return false;
}

bool AdminEngine::AdminServerUpdate_UpdateEvents()
{
    QString strTableName = "ym_events";
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    if (m_pDatabaseConnector->IsMySqlDB())
    {
        // Update ym_nodes table
        // UPDATE ym_events table
        InsertIntoUpdateLog("* Update " + strTableName + " EVENT:");
        InsertIntoUpdateLog("         to store new events flow");

        // Update the event for partitioning
        strQuery = "DROP EVENT IF EXISTS event_add_partition";
        clQuery.exec(strQuery);
        strQuery =
                "CREATE EVENT event_add_partition ON SCHEDULE EVERY 1 DAY STARTS '"
                +
                QDate::currentDate().addDays(1).toString("yyyy-MM-dd") +
                " 00:00:00' DO \n";
        strQuery += "BEGIN \n";
        strQuery += "  -- New partition name (DayAfter)\n";
        strQuery += "  DECLARE new_partition CHAR(32) DEFAULT     \n";
        strQuery +=
                "    CONCAT( 'p',         DATE_FORMAT( DATE_ADD( CURDATE(), INTERVAL 1 DAY ), '%Y%m%d' )         );\n";
        strQuery += "  -- New max value (DayAfter+1) \n";
        strQuery +=
                "  DECLARE max_day INTEGER DEFAULT TO_DAYS( CURDATE() ) + 2;\n";
        strQuery += "  DECLARE status_msg VARCHAR(1024);\n";
        strQuery += "  DECLARE status_value VARCHAR(10);\n";
        strQuery += "  DECLARE CONTINUE HANDLER FOR 1517 \n";
        strQuery += "    BEGIN\n";
        strQuery +=
                "      set @status_msg = CONCAT('Duplicate partition name ',new_partition);\n";
        strQuery += "      set @status_value = 'FAIL';\n";
        strQuery += "    END;\n";
        strQuery += "  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION \n";
        strQuery += "    BEGIN\n";
        strQuery += "      set @status_msg = 'Error occurred';\n";
        strQuery += "      set @status_value = 'FAIL';\n";
        strQuery += "    END;\n";
        strQuery += "\n";
        strQuery += "   -- Update the Events table\n";
        strQuery += "  SET @StartTime = now(); \n";
        strQuery += "  SET @status_value = 'PASS';\n";
        strQuery +=
                "  SET @status_msg = CONCAT('New partition name ',new_partition);\n";
        strQuery += "  INSERT INTO ym_events \n";
        strQuery +=
                "    VALUES(now(),null,null,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,\n";
        strQuery +=
                "    'Execute=event_add_partition','START','Event scheduler running',''); \n";
        strQuery += "  SET @event_id = LAST_INSERT_ID(); \n";
        strQuery += "\n";
        strQuery += "  -- Prepare split LastPartition \n";
        strQuery +=
                "  SET @stmt = CONCAT('ALTER TABLE ym_events REORGANIZE PARTITION LastPart INTO (PARTITION ',\n";
        strQuery +=
                "    new_partition,         ' VALUES LESS THAN (',          max_day,          '),\n";
        strQuery += "    PARTITION LastPart VALUES LESS THAN MAXVALUE)');\n";
        strQuery +=
                "  PREPARE stmt FROM @stmt;  -- Execute and split the Last partition\n";
        strQuery += "  EXECUTE stmt; \n";
        strQuery += "\n";
        strQuery += "  -- Update the Events table\n";
        strQuery += "  INSERT INTO ym_events \n";
        strQuery +=
                "    VALUES(now(),null,@event_id,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,\n";
        strQuery +=
                "    'Execute=event_add_partition',@status_value,@status_msg,''); \n";
        strQuery += "END";
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
    }

    // Success
    InsertIntoUpdateLog("Status = SUCCESS.");
    return true;
}
}
}
