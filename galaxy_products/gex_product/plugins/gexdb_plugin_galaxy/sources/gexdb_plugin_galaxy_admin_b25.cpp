// gexdb_plugin_galaxy_admin_b25.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B24->B25 upgrade
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

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B24_to_B25()
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

    //GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, "VERSION NOT READY !!!");
    //goto updatedb_error_noset;

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateDb|eUpdateIndexes)) goto updatedb_error_noset;

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(6);

    // ***********************************************************************************************///
    // *******************************START OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///
    unsigned int uiUpdateFlags;
    GetDbUpdateSteps(uiUpdateFlags);
    if(uiUpdateFlags&eUpdateConsOld)
    {
        if (!AddDbUpdateSteps(eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures)) goto updatedb_error_noset;
        if (!RemoveDbUpdateSteps(eUpdateConsOld)) goto updatedb_error_noset;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // CREATE PIN MAPS TABLES
    ///////////////////////////////////////////////////////////////////////////////
    // FT table
    if(!UpdateDb_B24_to_B25_CreatePinMapsTables(eFinalTest))
        goto updatedb_error_noset;
    // WT table
    if(!UpdateDb_B24_to_B25_CreatePinMapsTables(eWaferTest))
        goto updatedb_error_noset;
    IncrementProgress();

    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_error;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET ";
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B25)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B25);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B25);
    if(!clQuery.Execute(strQuery)) goto updatedb_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    ResetProgress(true);

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_writelog;

updatedb_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_writelog:

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
// Creates global_files table
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B24_to_B25_CreatePinMapsTables(TestingStage testingStage)
{
    if (!m_pclDatabaseConnector)
        return false;

    if ((testingStage != eFinalTest) && (testingStage != eWaferTest))
    {
        InsertIntoUpdateLog("Error unknown or not allowed testing stage.");
        return false;
    }

    QString             strQuery;
    QString             strLogMessage, strTableName, strTableSpace, strLoggingMode;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MySQL: get storage engine
    QString	strEngine,strFormat;
    if(m_pclDatabaseConnector->IsMySqlDB())
        GetStorageEngineName(strEngine,strFormat);

    SetTestingStage(testingStage);

    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE tables _pin_map
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set table name
    strTableName = NormalizeTableName("_pin_map");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);
    IncrementProgress();

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TAB_PARTITIONS WHERE TABLE_NAME=UPPER('"
                +m_strPrefixTable.toUpper()+"_MPTEST_INFO')";
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

        strQuery = "CREATE TABLE " + strTableName + " (\n"
        " SPLITLOT_ID NUMBER(10,0) NOT NULL,\n"
        " TPIN_PMRINDEX NUMBER(6,0) NOT NULL,\n"
        " CHAN_TYP NUMBER(5) DEFAULT 0,\n"
        " CHAN_NAM VARCHAR2(255) DEFAULT '',\n"
        " PHY_NAM VARCHAR2(255) DEFAULT '',\n"
        " LOG_NAM VARCHAR2(255) DEFAULT '',\n"
        " HEAD_NUM NUMBER(4,0) DEFAULT 1,\n"
        " SITE_NUM NUMBER(4,0) DEFAULT 1\n"
        ") PARTITION BY RANGE (SPLITLOT_ID) \n"
        "  (PARTITION FIRSTPART  VALUES LESS THAN (MAXVALUE) \n"
        "   PCTFREE 5 PCTUSED 80  "+strLoggingMode+" TABLESPACE " + strTableSpace + " COMPRESS ) \n";

    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery = "CREATE TABLE " + strTableName + " (\n"
        "  splitlot_id int(10) unsigned NOT NULL,\n"
        "  tpin_pmrindex smallint(6) NOT NULL,\n"
        "  chan_typ smallint(5) unsigned DEFAULT '0',\n"
        "  chan_nam varchar(255) DEFAULT '',\n"
        "  phy_nam varchar(255) DEFAULT '',\n"
        "  log_nam varchar(255) DEFAULT '',\n"
        "  head_num tinyint(4) unsigned DEFAULT '1',\n"
        "  site_num tinyint(4) unsigned DEFAULT '1'\n"
        ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    IncrementProgress();

    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}



