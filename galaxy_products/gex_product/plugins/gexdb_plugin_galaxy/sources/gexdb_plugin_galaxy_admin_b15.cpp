// gexdb_plugin_galaxy_admin_b15.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B14->B15 upgrade
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

// Standard includes
#include <math.h>

// Qt includes
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QDir>
#include <QProgressBar>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Update DB: B14 -> B15
///////////////////////////////////////////////////////////
// Major/Minor version : change db_version_nb(unsigned int) to float
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B14_to_B15()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nIncrementalUpdates = 0;

    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        InsertIntoUpdateLog("NOT SUPPORTED ...");
        return false;
    }

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(2);

    IncrementProgress();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Update incremental_update table
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!UpdateDb_B14_to_B15_UpdateIncrementalUpdateTable())
        goto updatedb_b14_to_b15_error_noset;

    IncrementProgress();

    // Make sure progress bar is set to max
    ResetProgress(true);
    QCoreApplication::processEvents();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Check and Update indexes only if this is the last build
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //	if(!UpdateDb_UpdateIndexes(pTextEdit_Log, pProgressBar))
    //		goto updatedb_b14_to_b15_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update Global Info
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating GLOBAL_INFO table.";
    InsertIntoUpdateLog(strLogMessage);

    /////////////////////////////////////////
    // GLOBAL_INFO TABLE
    /////////////////////////////////////////

    // Add INCREMENTAL_SPLITLOTS
    // Check if some incremental updates pending
    strQuery = "SELECT SUM(remaining_splitlots) from incremental_update";
    if(!clQuery.Execute(strQuery)) goto updatedb_b14_to_b15_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b14_to_b15_error;

    strQuery = "INSERT INTO global_info (db_version_name,db_version_nb,db_version_build,incremental_splitlots) VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B15);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B15);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B15);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b14_to_b15_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b14_to_b15_writelog;

updatedb_b14_to_b15_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_b14_to_b15_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b14_to_b15_writelog:

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update DB: B14 -> B15: global_info update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B14_to_B15_UpdateGlobalInfoTable()
{
    QString				strTable;
    QString				strLogMessage;
    QString				strQuery;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Update global_info table
    ////////////////////////////////////////////////////////////////////////////////////
    // Change product_hbin.bin_name field nullable
    strTable = NormalizeTableName("global_info",false);
    strLogMessage = "o Changing type of db_version_nb field in " + strTable + " (switch to float)...";
    InsertIntoUpdateLog(strLogMessage);
    if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
        strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN db_version_nb FLOAT NOT NULL";
    else
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery = "ALTER TABLE " + strTable + " MODIFY ( bin_name   NUMBER NOT NULL)";
    if(!clQuery.Execute(strQuery))
    {
        if(m_pclDatabaseConnector->IsOracleDB() && clQuery.lastError().text().contains("ORA-01451",Qt::CaseInsensitive))
        {
            // Column already nullable
            // ignore this error
        }
        else
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }
    InsertIntoUpdateLog("DONE.", false);

    return true;
}

///////////////////////////////////////////////////////////
// Update DB: B14 -> B15: incremental_update update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B14_to_B15_UpdateIncrementalUpdateTable()
{
    QString				strTable;
    QString				strLogMessage;
    QString				strQuery;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Update incremental_update table
    ////////////////////////////////////////////////////////////////////////////////////
    // Change incremental_update.db_version_build field nullable
    strTable = NormalizeTableName("incremental_update",false);
    strLogMessage = "o Update " + strTable + " table (switch to nullable)";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "Changing type of db_version_build field...";
    InsertIntoUpdateLog(strLogMessage);
    if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
        strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN db_version_build smallint(5) NULL";
    else
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery = "ALTER TABLE " + strTable + " MODIFY ( db_version_build  NUMBER(6) NULL)";
    if(!clQuery.Execute(strQuery))
    {
        if(m_pclDatabaseConnector->IsOracleDB() && clQuery.lastError().text().contains("ORA-01451",Qt::CaseInsensitive))
        {
            // Column already nullable
            // ignore this error
        }
        else
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }
    InsertIntoUpdateLog("DONE.", false);

    return true;
}

