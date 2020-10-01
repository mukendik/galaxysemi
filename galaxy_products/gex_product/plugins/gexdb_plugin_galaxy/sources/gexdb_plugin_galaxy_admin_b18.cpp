// gexdb_plugin_galaxy_admin_b16.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B17->B18 upgrade
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
#include "gex_shared.h"

// Qt includes
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QCryptographicHash>
#include <QProgressBar>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B17_to_B18()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        InsertIntoUpdateLog("NOT SUPPORTED ...");
        return false;
    }

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateDb|eUpdateIndexes)) goto updatedb_b17_to_b18_error_noset;

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(60);

    // ***********************************************************************************************///
    // *******************************START OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "UPDATE global_info SET ";
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B18)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B18);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B18);
    if(!clQuery.Execute(strQuery)) goto updatedb_b17_to_b18_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) UPDATE GLOBAL_INFO TABLE: add DB_Type field
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating GLOBAL_INFO table.";
    InsertIntoUpdateLog(strLogMessage);

    if(!UpdateDb_B17_to_B18_UpdateGlobalInfoTable())
        goto updatedb_b17_to_b18_error_noset;
    IncrementProgress();


    ///////////////////////////////////////////////////////////////////////////////
    // 3) CREATE CHARACTERIZATION TABLES
    ///////////////////////////////////////////////////////////////////////////////
    // FT table
    if(!UpdateDb_B17_to_B18_CreateTestConditionsTable(eFinalTest))
        goto updatedb_b17_to_b18_error_noset;
    // WT table
    if(!UpdateDb_B17_to_B18_CreateTestConditionsTable(eWaferTest))
        goto updatedb_b17_to_b18_error_noset;
    // ET table
    if(!UpdateDb_B17_to_B18_CreateTestConditionsTable(eElectTest))
        goto updatedb_b17_to_b18_error_noset;
    IncrementProgress();


    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_b17_to_b18_error;

    ResetProgress(true);

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b17_to_b18_writelog;

updatedb_b17_to_b18_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_b17_to_b18_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b17_to_b18_writelog:

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
// Updates global_info_table
// Add: db_type
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B17_to_B18_UpdateGlobalInfoTable()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strTable, strQuery, strLogMessage;
    GexDbPlugin_Query     clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QSqlRecord			clRecords;

    strTable = NormalizeTableName("global_info",false);

    // ADD db_type column if it does not already exist
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("db_type"))
    {
        // Add columns
        strLogMessage = "Adding column db_type to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN db_type VARCHAR(255)";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                // No Default value:  unsupported add column (if default value) operation on compressed table
                strQuery = "ALTER TABLE " + strTable + " ADD ( db_type VARCHAR2(255))";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();


    // Update B17 to b18 so the DB is supposed to be a YM TDR DB
    strLogMessage = "Updating database type...";
    InsertIntoUpdateLog(strLogMessage);
    QString dbType;
    dbType = m_pclDatabaseConnector->m_strSchemaName;
    dbType += GEXDB_YM_PROD_TDR_KEY;
    QByteArray hash = QCryptographicHash::hash(dbType.toLatin1(),QCryptographicHash::Md5);
    strQuery = "UPDATE " + strTable + " SET db_type='"+ QString(hash.toHex()) +"'";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);

    IncrementProgress();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create [et_] OR [ft_] OR [wt_] test_conditions tables
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B17_to_B18_CreateTestConditionsTable(TestingStage testingStage)
{
    if (!m_pclDatabaseConnector)
        return false;

    if ((testingStage != eFinalTest) && (testingStage != eWaferTest) && (testingStage != eElectTest))
    {
        InsertIntoUpdateLog("Error unknown testing stage.");
        return false;
    }

    QString				strQuery;
    QString				strLogMessage, strTableName, strTableSpace, strLoggingMode;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int                 condition_number = GEXDB_PLUGIN_GALAXY_TEST_CONDITIONS_COLUMNS;

    // MySQL: get storage engine
    QString	strEngine,strFormat;
    if(m_pclDatabaseConnector->IsMySqlDB())
        GetStorageEngineName(strEngine,strFormat);

    SetTestingStage(testingStage);

    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE tables _test_conditions
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set table name
    strTableName = NormalizeTableName("_test_conditions");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);
    IncrementProgress();

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TAB_PARTITIONS WHERE TABLE_NAME=UPPER('"
                +m_strPrefixTable.toUpper()+"_PTEST_INFO')";
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
                "test_info_id     NUMBER(5,0)     NOT NULL,\n"
                "test_type        CHAR(1)         NOT NULL,\n";
        // Add 100 conditions columns
        for (int i = 1; i <= condition_number; ++i)
            strQuery += "condition_" + QString::number(i) + " VARCHAR2(255) DEFAULT NULL,\n";
        strQuery += "CONSTRAINT PK_" + m_strPrefixTable.toUpper() + "_TEST_CONDITIONS PRIMARY KEY(splitlot_id, test_info_id, test_type)\n"
                ") TABLESPACE " + strTableSpace + " PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery =  "CREATE TABLE " + strTableName + "(\n"
                "splitlot_id  int(10)     unsigned    NOT NULL,\n"
                "test_info_id smallint(5) unsigned    NOT NULL,\n"
                "test_type    char(1)                 NOT NULL,\n";
        // Add 100 conditions columns
        for (int i = 1; i <= condition_number; ++i)
            strQuery += "condition_" + QString::number(i) + " varchar(255) DEFAULT NULL,\n";

        strQuery += "  PRIMARY KEY (splitlot_id,test_info_id,test_type)\n";
        strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
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

