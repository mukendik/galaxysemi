// gexdb_plugin_galaxy_admin_b16.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B18->B19 upgrade
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_getroot_dialog.h"
#include "import_constants.h"
#include <gqtl_log.h>

// Qt includes
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QProgressBar>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B18_to_B19()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString             strQuery;
    bool                bStatus = false;
    QString             strLogMessage, strErrorMessage;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        InsertIntoUpdateLog("NOT SUPPORTED ...");
        return false;
    }

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateDb)) goto updatedb_B18_to_B19_error_noset;

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(12);

    // ***********************************************************************************************///
    // *******************************START OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "UPDATE global_info SET ";
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B19)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B19);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B19);
    if(!clQuery.Execute(strQuery)) goto updatedb_B18_to_B19_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) SCHEDULE SIZE ANALYZE FOR MYSQL
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    if(!UpdateDb_B18_to_B19_UpdateSizeScheduler())
//    {
//        // The scheduler size analyze is OPTIONAL
//        // The user can skip this step
//        QString strMessage;
//        strMessage = "Error when creating the Gex Event Scheduler \n";
//        strMessage+= "to compute the database size in a SQL background process.\n";
//        strMessage+= "This new feature can be update.\n";
//        strMessage+= "\n";
//        strMessage+= "Do you want to ignore this new feature?";
//        int iStatus = QMessageBox::question(pGexMainWindow,
//            "Event Scheduler error",
//            "You've edited the Bin colors.\nDo you want the report pages to be rebuilt ?",
//            QMessageBox::Yes  | QMessageBox::Default,
//            QMessageBox::No  | QMessageBox::Escape);

//        // check if user confirms that pages have to be rebuilt.
//        if(iStatus != QMessageBox::Yes)
//            return;


//        goto updatedb_B18_to_B19_error_noset;
//    }
    IncrementProgress();


    ///////////////////////////////////////////////////////////////////////////////
    // 3) CREATE DIE TRACEABILITY TABLES
    ///////////////////////////////////////////////////////////////////////////////
    // FT table
    if(!UpdateDb_B18_to_B19_CreateDieTraceabilityTable())
        goto updatedb_B18_to_B19_error_noset;
    IncrementProgress();


    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_B18_to_B19_error;

    ResetProgress(true);

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_B18_to_B19_writelog;

updatedb_B18_to_B19_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_B18_to_B19_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_B18_to_B19_writelog:

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FOR MYSQL
// create a scheduler job
// update size into global_options
// Check if the scheduler is active
//select @@global.event_scheduler;
//If result is 0 or NOT:
//1. Add a new line in the my.ini file
//event_scheduler=ON
//2. Restart the MySql server (or not, item 3)
//3. with the root connection, activate the scheduler
//set @@global.event_scheduler=1;
// Create the event schedule
//delimiter |
//CREATE DEFINER='gexdb' EVENT database_size
//ON SCHEDULE EVERY 1 DAY
//COMMENT 'Compute the database size.'
//DO
//BEGIN
//DECLARE size BIGINT;
//SELECT count(*) FROM global_options WHERE option_name='GEXDB_MYSQL_SIZE' INTO size;
//IF (size = 0) THEN
//INSERT INTO global_options VALUES('GEXDB_MYSQL_SIZE','0');
//END IF;

//UPDATE global_options
//SET option_value=
//(SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024)
//FROM information_schema.TABLES
//WHERE TABLE_SCHEMA='gexdb')
//WHERE option_name='GEXDB_MYSQL_SIZE';
//END|
//delimiter ;
//*** SQL QUERY
//GRANT EVENT ON gexdb.* TO gexdb;
//*** syntax for event scheduler:
//event-scheduler = ON
//*** SQL version
//min sql version is v5.1.6 for this feature
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B18_to_B19_UpdateSizeScheduler()
{
    if (!m_pclDatabaseConnector)
        return false;

    // Only for MySql server
    if(!m_pclDatabaseConnector->IsMySqlDB())
        return true;

    QString               strMessage, strQuery, strLogMessage;
    GexDbPlugin_Query     clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));


    QString   strValue;
    int       nValue;

//    <li><b>Database Size Scheduler</b>
//    <i>( MySql only )</i>
//    <BR> <u>Create an Event Scheduler</u>
//    <BR>Compute the database size in a SQL background process
//    <BR> <u>Add the Event privilege to Gex user</u>
//    <BR>- NEED MYSQL 5.1.12 OR after
//    <BR>- need the SQL root access
//    (if the Gex user doesn't have the Event privilege)</i>

    strLogMessage = "o Add a Database Size Event Scheduler.";
    InsertIntoUpdateLog(strLogMessage);

    ///////////////////////////////////////////
    // * Check the MySql Server configuration
    ///////////////////////////////////////////
    // Check if have MySql 5.1.12 or after
    // to have the EVENT SCHEDULER
    strQuery = "SELECT @@version";
    if(!clQuery.exec(strQuery) || !clQuery.first())
    {
        strMessage = "Error executing SQL query.\n";
        strMessage+= "QUERY=" + strQuery + "\n";
        strMessage+= "ERROR=" + clQuery.lastError().text();
        InsertIntoUpdateLog(strMessage);
        return false;
    }
    // 5.1.12-community
    strValue = clQuery.value(0).toString();
    nValue = strValue.section(".",0,1).remove('.').toInt();
    strValue = strValue.section(".",2).section("-",0,0).simplified();
    if((nValue < 51) || ((nValue == 51) && (strValue.toInt() < 12)))
    {
        strMessage = "UNSUPPORTED MYSQL VERSION\n";
        strMessage+= " - MYSQL VERSION = "+clQuery.value(0).toString()+ "\n";
        strMessage = "*************************\n";
        strMessage+= " - NEED MYSQL 5.1.12 OR after";
        InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
        return false;
    }

    // Check if have EVENT_SCHEDULER option
    // min sql version is v5.1.6 for this feature
    strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    strQuery+= "   WHERE VARIABLE_NAME = 'EVENT_SCHEDULER'";
    if(!clQuery.exec(strQuery) || !clQuery.first())
    {
        strMessage = "Error executing SQL query.\n";
        strMessage+= "QUERY=" + strQuery + "\n";
        strMessage+= "ERROR=" + clQuery.lastError().text();
        InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
        return false;
    }
    // ON
    strValue = clQuery.value(0).toString();
    if((!strValue.startsWith("ON",Qt::CaseInsensitive))
            && (!strValue.startsWith("YES",Qt::CaseInsensitive))
            && (!strValue.startsWith("ENAB",Qt::CaseInsensitive)))
    {
        QString strMySqlIniFile;
        QString strMySqlLogErrorFile;
        // Find the MySql my.ini config file
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'BASEDIR'";
        if(clQuery.exec(strQuery) && clQuery.first())
        {
            if(QFileInfo(clQuery.value(0).toString()+"my.ini").exists())
                strMySqlIniFile = clQuery.value(0).toString()+"my.ini";
            else if (QFileInfo(clQuery.value(0).toString()+"my.cnf").exists())
                strMySqlIniFile = clQuery.value(0).toString()+"my.cnf";
        }
        if(strMySqlIniFile.isEmpty())
        {
            strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
            strQuery+= "   WHERE VARIABLE_NAME = 'DATADIR'";
            if(clQuery.exec(strQuery) && clQuery.first())
            {
                if(QFileInfo(clQuery.value(0).toString()+"my.ini").exists())
                    strMySqlIniFile = clQuery.value(0).toString()+"my.ini";
                else if (QFileInfo(clQuery.value(0).toString()+"my.cnf").exists())
                    strMySqlIniFile = clQuery.value(0).toString()+"my.cnf";
            }
        }
        if(strMySqlIniFile.isEmpty())
            strMySqlIniFile = "BASEDIR/my.ini or BASEDIR/my.cnf";

        // Get the MySql LogError
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'LOG_ERROR'";
        if(clQuery.exec(strQuery) && clQuery.first())
            strMySqlLogErrorFile = clQuery.value(0).toString();

        strMessage = "UNSUPPORTED MYSQL EVENT SCHEDULER OPTION\n";
        strMessage+= " - MYSQL GLOBAL_VARIABLES = 'EVENT_SCHEDULER' disabled\n";
        strMessage+= "*************************\n";
        strMessage+= " - NEED MYSQL EVENT_SCHEDULER OPTION\n";
        strMessage+= "\n";
        strMessage+= "o Check your MySql config file:\n";
        strMessage+= " - "+strMySqlIniFile+"\n";
        strMessage+= " - add a line to the [mysqld] section.\n";
        strMessage+= "   [mysqld]\n";
        strMessage+= "   event-scheduler = ON\n";
        strMessage+= "o Check your MySql error log file:\n";
        strMessage+= " - "+strMySqlLogErrorFile+"\n";
        strMessage+= " - EVENT_SCHEDULER would be disabled when it fail to start, please check your error log.\n";
        InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
        return false;
    }

    ///////////////////////////////////////////
    // Check if the user have the EVENT privilege
    ///////////////////////////////////////////
    bool          bAddEventPrivilege = false;
    QStringList   lstValue;
    strQuery = "SHOW GRANTS for '"+m_pclDatabaseConnector->m_strUserName_Admin+"'@'%'";
    if(!clQuery.exec(strQuery))
    {
        strMessage = "Error executing SQL query.\n";
        strMessage+= "QUERY=" + strQuery + "\n";
        strMessage+= "ERROR=" + clQuery.lastError().text();
        InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
        return false;
    }
    while(clQuery.Next())
        lstValue.append(clQuery.value(0).toString());
    strQuery = "SHOW GRANTS for '"+m_pclDatabaseConnector->m_strUserName_Admin+"'@'localhost'";
    if(clQuery.exec(strQuery))
    {
        while(clQuery.Next())
            lstValue.append(clQuery.value(0).toString());
    }
    while(!lstValue.isEmpty())
    {
        strValue = lstValue.takeFirst().toUpper().section(" ON ",0,0);
        if(strValue.contains("USAGE",Qt::CaseInsensitive))
            continue;
        if(!strValue.contains("EVENT",Qt::CaseInsensitive)
        && !strValue.contains("ALL PRIVILEGES"))
        {
            bAddEventPrivilege = true;
            break;
        }
    }

    if(bAddEventPrivilege)
    {
        // Add EVENT PRIVILEGE to user
        strLogMessage = "Adding the EVENT privilege to "+m_pclDatabaseConnector->m_strUserName_Admin+"...";
        InsertIntoUpdateLog(strLogMessage);
        // Connect as root
        // Get root user+password if not already done for the drop
        QString     strRootUser, strRootPassword;
        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
          return false;

        // Retrieve values from dialog
        strRootUser = clGexdbGetrootDialog.GetRootUsername();
        strRootPassword = clGexdbGetrootDialog.GetRootPassword();

        GexDbPlugin_Connector       pclDatabaseConnector(m_strPluginName, this);
        pclDatabaseConnector = *m_pclDatabaseConnector;
        pclDatabaseConnector.m_strUserName_Admin = strRootUser;
        pclDatabaseConnector.m_strPassword_Admin = strRootPassword;
        pclDatabaseConnector.m_strDatabaseName = "mysql";
        pclDatabaseConnector.m_strSchemaName = "mysql";
        pclDatabaseConnector.m_strHost_Name = "localhost";
        pclDatabaseConnector.m_strHost_Name = "127.0.0.1";
        pclDatabaseConnector.SetAdminLogin(true);
        if(!pclDatabaseConnector.Connect())
          return false;
        QSqlDatabase        clSqlDatabase = QSqlDatabase::database(pclDatabaseConnector.m_strConnectionName);
        GexDbPlugin_Query   clQuery(this, clSqlDatabase);

        strQuery = "GRANT ALL PRIVILEGES ON `"+m_pclDatabaseConnector->m_strSchemaName+"`.*"
                " TO '"+m_pclDatabaseConnector->m_strUserName_Admin+"'@'localhost'";
        if(!clQuery.exec(strQuery))
        {
            strMessage = "Error executing SQL query.\n";
            strMessage+= "QUERY=" + strQuery + "\n";
            strMessage+= "ERROR=" + clQuery.lastError().text();
            InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
            return false;
        }
        strQuery = "GRANT ALL PRIVILEGES ON `"+m_pclDatabaseConnector->m_strSchemaName+"`.*"
                " TO '"+m_pclDatabaseConnector->m_strUserName_Admin+"'@'%'";
        if(!clQuery.exec(strQuery))
        {
            strMessage = "Error executing SQL query.\n";
            strMessage+= "QUERY=" + strQuery + "\n";
            strMessage+= "ERROR=" + clQuery.lastError().text();
            InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
            return false;
        }
        clQuery.exec("FLUSH PRIVILEGES");
        pclDatabaseConnector.Disconnect();
        // Reconnect
        m_pclDatabaseConnector->Disconnect();
        m_pclDatabaseConnector->Connect();
        InsertIntoUpdateLog("DONE.", false);
    }

    ///////////////////////////////////////////
    // Create the event schedule
    ///////////////////////////////////////////
    // Drop the event if exists
    // Create the event
    // Try with the gex admin user
    // if error, try with the root user
    strQuery = "DROP EVENT database_size";
    clQuery.exec(strQuery);

    strQuery = "CREATE EVENT database_size\n"
               " ON SCHEDULE EVERY 1 DAY\n"
               " COMMENT 'Compute the database size.'\n"
               " DO\n"
               " BEGIN\n"
               "   DECLARE size BIGINT;\n"
               "   SELECT count(*) FROM global_options WHERE option_name='GEXDB_MYSQL_SIZE' INTO size;\n"
               "   IF (size = 0) THEN\n"
               "     INSERT INTO global_options VALUES('GEXDB_MYSQL_SIZE','0');\n"
               "   END IF;\n"
               "   \n"
               "   UPDATE global_options\n"
               "     SET option_value=\n"
               "       (SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024)\n"
               "        FROM information_schema.TABLES\n"
               "        WHERE TABLE_SCHEMA='"+m_pclDatabaseConnector->m_strSchemaName+"')\n"
               "   WHERE option_name='GEXDB_MYSQL_SIZE';\n"
               " END";
    if(!clQuery.exec(strQuery))
    {
        strMessage = "Error executing SQL query.\n";
        strMessage+= "QUERY=" + strQuery + "\n";
        strMessage+= "ERROR=" + clQuery.lastError().text();
        InsertIntoUpdateLog(strMessage.replace("\n","<BR>\n"));
        return false;
    }

    IncrementProgress();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create die traceability tables
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B18_to_B19_CreateDieTraceabilityTable()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString             strQuery;
    QString             strLogMessage, strTableName, strTableSpace, strLoggingMode;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QStringList         lstValues;

    // MySQL: get storage engine
    QString             strEngine,strFormat;


    SetTestingStage(eFinalTest);

    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // CREATE tables _run_dietrace
    // Do not drop table if already exists
    // Kate have already create this 2 tables
    // Just check some type ?
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set table name
    strTableName = NormalizeTableName("_run_dietrace");

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // Check if exists
    // If not create
    // Check if some field have to be updated
    // If yes update

    // Check if table not already exists
    lstValues = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).tables();
    if(lstValues.contains(strTableName,Qt::CaseInsensitive))
    {
        // Check some types
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            // For Kate
            strQuery = "SELECT COLUMN_TYPE FROM information_schema.columns "
                    " WHERE table_schema='" + m_pclDatabaseConnector->m_strSchemaName + "' "
                    " AND table_name='" + strTableName + "' and (column_name='splitlot_id' || column_name='run_id' || column_name='die_config_id') "
                    " AND column_type NOT LIKE '%unsigned%'";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(clQuery.First())
            {
                strLogMessage = "Switch " + strTableName + ".splitlot_id to unsigned ...";
                InsertIntoUpdateLog(strLogMessage);
                strQuery = "ALTER TABLE " + strTableName + " MODIFY ( "
                        " splitlot_id   int(10) unsigned        NOT NULL,"
                        " run_id        mediumint(8) unsigned   NOT NULL,"
                        " die_config_id smallint(5) unsigned    NOT NULL)";
                if(!clQuery.Execute(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }
            }
        }
        IncrementProgress();
    }
    else
    {

        // ORACLE
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TAB_PARTITIONS WHERE TABLE_NAME='"
                    +m_strPrefixTable.toUpper()+"_RUN'";
            if(!clQuery.exec(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }

            clQuery.first();
            strTableSpace = clQuery.value(0).toString();
            if(clQuery.value(1).toString().compare("NO"))
                strLoggingMode = "NOLOGGING";
            else
                strLoggingMode = "LOGGING";

            strQuery = "CREATE TABLE "+strTableName+" (\n"
                    "splitlot_id      NUMBER(10,0)    NOT NULL,\n"
                    "run_id           NUMBER(8,0)     NOT NULL,\n"
                    "die_config_id    NUMBER(5,0)     NOT NULL,\n"
                    "part_id          VARCHAR2(255)   DEFAULT NULL,\n"
                    "part_x           NUMBER(6,0)     DEFAULT NULL,\n"
                    "part_y           NUMBER(6,0)     DEFAULT NULL,\n"
                    "CONSTRAINT PK_ftrundietrace PRIMARY KEY(splitlot_id, run_id, die_config_id)\n"
                    ") PARTITION BY RANGE (SPLITLOT_ID) \n"
                    "(PARTITION FIRSTPART  VALUES LESS THAN (MAXVALUE)) \n"
                    " TABLESPACE " + strTableSpace + " PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
        }
        // MYSQL
        else if(m_pclDatabaseConnector->IsMySqlDB())
        {
            GetStorageEngineName(strEngine,strFormat);

            strQuery =  "CREATE TABLE " + strTableName + "(\n"
                    "splitlot_id    int(10) unsigned        NOT NULL,\n"
                    "run_id         mediumint(8) unsigned   NOT NULL,\n"
                    "die_config_id  smallint(5) unsigned    NOT NULL,\n"
                    "part_id        varchar(255)            DEFAULT NULL,\n"
                    "part_x         smallint(6)             DEFAULT NULL,\n"
                    "part_y         smallint(6)             DEFAULT NULL,\n"
                    "PRIMARY KEY ftrundietrace (splitlot_id,run_id,die_config_id),\n"
                    "KEY ftrundietrace_partid  (part_id)\n"
                    ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
        }

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        // For ORACLE, add index
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "CREATE INDEX PK_ftrundietrace_partid ON " + strTableName + " (part_id)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

        InsertIntoUpdateLog("DONE.", false);
        IncrementProgress();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // CREATE tables _run_dietrace
    // Do not drop table if already exists
    // Kate have already create this 2 tables
    // Just check some type ?
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set table name
    strTableName = NormalizeTableName("_dietrace_config");

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // Check if exists
    // If not create
    // Check if some field have to be updated
    // If yes update

    // Check if table not already exists
    lstValues = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).tables();
    if(lstValues.contains(strTableName,Qt::CaseInsensitive))
    {
        // Check some types
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            // For Kate
            strQuery = "SELECT COLUMN_TYPE FROm information_schema.columns "
                    " WHERE table_schema='" + m_pclDatabaseConnector->m_strDatabaseName + "' "
                    " AND table_name='" + strTableName + "' and (column_name='splitlot_id' || column_name='die_index' || column_name='die_config_id') "
                    " AND column_type NOT LIKE '%unsigned%'";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(clQuery.First())
            {
                strLogMessage = "Switch " + strTableName + ".splitlot_id to unsigned ...";
                InsertIntoUpdateLog(strLogMessage);
                strQuery = "ALTER TABLE " + strTableName + " MODIFY ( "
                        "splitlot_id    int(10) unsigned        NOT NULL,\n"
                        "die_config_id  smallint(5) unsigned    NOT NULL,\n"
                        "die_index      smallint(5) unsigned    NOT NULL)";
                if(!clQuery.Execute(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }
            }
        }
        IncrementProgress();
    }
    else
    {

        // ORACLE
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TAB_PARTITIONS WHERE TABLE_NAME='"
                    +m_strPrefixTable.toUpper()+"_RUN'";
            if(!clQuery.exec(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }

            clQuery.first();
            strTableSpace = clQuery.value(0).toString();
            if(clQuery.value(1).toString().compare("NO"))
                strLoggingMode = "NOLOGGING";
            else
                strLoggingMode = "LOGGING";

            strQuery = "CREATE TABLE "+strTableName+" (\n"
                    "splitlot_id    NUMBER(10,0)    NOT NULL,\n"
                    "die_config_id  NUMBER(5,0)     NOT NULL,\n"
                    "die_index      NUMBER(5,0)     NOT NULL,\n"
                    "product        VARCHAR2(255)   DEFAULT NULL,\n"
                    "lot_id         VARCHAR2(255)   DEFAULT NULL,\n"
                    "wafer_id       VARCHAR2(255)   DEFAULT NULL,\n"
                    "CONSTRAINT PK_ftdietraceconfig PRIMARY   KEY(splitlot_id, die_config_id)\n"
                    ") PARTITION BY RANGE (SPLITLOT_ID) \n"
                    "(PARTITION FIRSTPART  VALUES LESS THAN (MAXVALUE)) \n"
                    " TABLESPACE " + strTableSpace + " PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
        }
        // MYSQL
        else if(m_pclDatabaseConnector->IsMySqlDB())
        {
            GetStorageEngineName(strEngine,strFormat);

            strQuery =  "CREATE TABLE " + strTableName + "(\n"
                    "splitlot_id    int(10) unsigned        NOT NULL,\n"
                    "die_config_id  smallint(5) unsigned    NOT NULL,\n"
                    "die_index      smallint(5) unsigned    NOT NULL,\n"
                    "product        varchar(255)            DEFAULT NULL,\n"
                    "lot_id         varchar(255)            DEFAULT NULL,\n"
                    "wafer_id       varchar(255)            DEFAULT NULL,\n"
                    "PRIMARY KEY ftdietraceconfig   (splitlot_id,die_config_id),\n"
                    "KEY ftdietraceconfig_product   (product),\n"
                    "KEY ftdietraceconfig_lot       (lot_id),\n"
                    "KEY ftdietraceconfig_wafer     (wafer_id),\n"
                    "KEY ftdietraceconfig_die       (die_index)\n"
                    ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
        }

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        // For ORACLE, add indexes
        // For ORACLE, add index
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "CREATE INDEX PK_ftdietraceconfig_product ON " + strTableName + " (product)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strQuery = "CREATE INDEX PK_ftdietraceconfig_lot ON " + strTableName + " (lot_id)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strQuery = "CREATE INDEX PK_ftdietraceconfig_wafer ON " + strTableName + " (wafer_id)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strQuery = "CREATE INDEX PK_ftdietraceconfig_die ON " + strTableName + " (die_index)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

        InsertIntoUpdateLog("DONE.", false);
        IncrementProgress();
    }

    return true;
}

