// gexdb_plugin_galaxy_admin_b16.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B19->B20 upgrade
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
#include <QCryptographicHash>
#include <QProgressBar>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B19_to_B20()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString             strQuery;
    bool                bStatus = false;
    QString             strLogMessage, strErrorMessage;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QMap<QString,QString> lTransfertOptions;
    lTransfertOptions["GEXDB_INSERTION_ALLOW_DYNAMIC_LIMITS"]       ="TEST_ALLOW_DYNAMIC_LIMITS";
    lTransfertOptions["GEXDB_ORACLE_INDEX_INSERTMODE"]              ="ORACLE_INDEX_INSERTMODE";
    lTransfertOptions["GEXDB_ORACLE_SPLITLOTS_PER_PARTITION"]       ="ORACLE_SPLIT_PARTITION_BY";
    lTransfertOptions["GEXDB_ORACLE_FT_INITIAL_EXTENT_SIZE_KB"]     ="ORACLE_FT_INITIAL_EXTENT_SIZE_KB";
    lTransfertOptions["GEXDB_ORACLE_FT_NEXT_EXTENT_SIZE_MB"]        ="ORACLE_FT_NEXT_EXTENT_SIZE_MB";
    lTransfertOptions["GEXDB_ORACLE_FT_MAX_DATAFILE_SIZE_MB"]       ="ORACLE_FT_MAX_DATAFILE_SIZE_MB";
    lTransfertOptions["GEXDB_ORACLE_WT_INITIAL_EXTENT_SIZE_KB"]     ="ORACLE_WT_INITIAL_EXTENT_SIZE_KB";
    lTransfertOptions["GEXDB_ORACLE_WT_NEXT_EXTENT_SIZE_MB"]        ="ORACLE_WT_NEXT_EXTENT_SIZE_MB";
    lTransfertOptions["GEXDB_ORACLE_WT_MAX_DATAFILE_SIZE_MB"]       ="ORACLE_WT_MAX_DATAFILE_SIZE_MB";
    lTransfertOptions["GEXDB_ORACLE_ET_INITIAL_EXTENT_SIZE_KB"]     ="ORACLE_ET_INITIAL_EXTENT_SIZE_KB";
    lTransfertOptions["GEXDB_ORACLE_ET_NEXT_EXTENT_SIZE_MB"]        ="ORACLE_ET_NEXT_EXTENT_SIZE_MB";
    lTransfertOptions["GEXDB_ORACLE_ET_MAX_DATAFILE_SIZE_MB"]       ="ORACLE_ET_MAX_DATAFILE_SIZE_MB";
    lTransfertOptions["GEX_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN"]  ="BINNING_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN";
    lTransfertOptions["GEXDB_FT_CONSOLIDATE_SBIN"]                  ="BINNING_FT_CONSOLIDATE_SBIN";
    lTransfertOptions["GEXDB_INSERTION_ALLOW_MISSING_WAFERNB"]      ="INSERTION_ALLOW_MISSING_WAFERNB";
    lTransfertOptions["GEXDB_FT_MAP_INVALID_HBIN_WITH_SBIN"]        ="BINNING_FT_MAP_INVALID_HBIN_WITH_SBIN";
    lTransfertOptions["GEXDB_WT_MAP_INVALID_HBIN_WITH_SBIN"]        ="BINNING_WT_MAP_INVALID_HBIN_WITH_SBIN";
    lTransfertOptions["GEXDB_TESTNAME_REMOVE_SEQUENCER_NAME"]       ="TEST_REMOVE_SEQUENCER_NAME";
    lTransfertOptions["GEXDB_TESTNAME_ALLOW_CUSTOM_STDF_RECORDS"]   ="INSERTION_ALLOW_CUSTOM_STDF_RECORDS";
    lTransfertOptions["GEXDB_INSERTION_TIMER_INTERVAL"]             ="INSERTION_TIMER_INTERVAL";
    lTransfertOptions["GEXDB_INSERTION_FLUSH_SQLBUFFER_AFTER"]      ="INSERTION_FLUSH_SQLBUFFER_AFTER";
    lTransfertOptions["GEXDB_INSERTION_ALLOW_DUPLICATE_TESTNUMBERS"]="TEST_ALLOW_DUPLICATE_TESTNUMBERS";
    lTransfertOptions["GEXDB_ORACLE_SPLIT_PARTITION_BY"]            ="ORACLE_SPLIT_PARTITION_BY";
    lTransfertOptions["GEXDB_MYSQL_SPLIT_PARTITION_BY"]             ="MYSQL_SPLIT_PARTITION_BY";
    lTransfertOptions["GEXDB_MYSQL_SPLITLOTS_PER_PARTITION"]        ="MYSQL_SPLIT_PARTITION_BY";
    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateDb)) goto updatedb_B19_to_B20_error_noset;

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(lTransfertOptions.count()+2);

    // ***********************************************************************************************///
    // *******************************START OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET ";
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B20)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B20);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B20);
    if(!clQuery.Execute(strQuery)) goto updatedb_B19_to_B20_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //    Transfert option name
    //    => GEX_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN to GEXDB_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Add new options for Partitioning Granularity.";
    InsertIntoUpdateLog(strLogMessage);

    foreach(const QString &Option, lTransfertOptions.keys())
    {
        strQuery = "UPDATE "+NormalizeTableName("global_options",false)+" SET option_name='"+lTransfertOptions[Option]+"' ";
        strQuery+= " WHERE option_name='"+Option+"' ";
        if(!clQuery.Execute(strQuery)) goto updatedb_B19_to_B20_error;
        IncrementProgress();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //    => GEXDB_ORACLE_SPLITLOTS_PER_PARTITION=<number> to GEXDB_ORACLE_SPLIT_PARTITION_BY=SPLITLOT|<number>
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "SELECT count(*) FROM "+NormalizeTableName("global_options",false)+" ";
    strQuery+= " WHERE option_name LIKE '%_SPLIT_PARTITION_BY' ";
    if(m_pclDatabaseConnector->IsOracleDB())
        strQuery+= " AND (LENGTH(TRIM(TRANSLATE(option_value, '0123456789',' '))) IS NULL) ";
    else
        strQuery+= " AND (option_value+0) > 0 ";
    if(!clQuery.Execute(strQuery)) goto updatedb_B19_to_B20_error;
    if(clQuery.First() && (clQuery.value(0).toInt() > 0))
    {

        strQuery = "UPDATE "+NormalizeTableName("global_options",false)+" SET option_value=concat('SPLITLOT|', ";
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery+= "TO_CHAR(option_value)";
        else
            strQuery+= "option_value";
        strQuery+= ") ";
        strQuery+= " WHERE option_name LIKE '%_SPLIT_PARTITION_BY' ";
        if(!clQuery.Execute(strQuery)) goto updatedb_B19_to_B20_error;
    }
    IncrementProgress();

    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_B19_to_B20_error;

    ResetProgress(true);

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_B19_to_B20_writelog;

updatedb_B19_to_B20_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_B19_to_B20_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_B19_to_B20_writelog:

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


