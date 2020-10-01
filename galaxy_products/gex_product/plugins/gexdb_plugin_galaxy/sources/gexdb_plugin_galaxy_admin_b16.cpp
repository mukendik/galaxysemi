// gexdb_plugin_galaxy_admin_b16.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B15->B16 upgrade
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
#include "consolidation_tree.h"
#include "consolidation_tree_replies.h"
#include <gqtl_log.h>

// Qt includes
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update DB: B15 -> B16
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Major/Minor version : change db_version_nb(unsigned int) to float
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B15_to_B16()
{
    if (!m_pclDatabaseConnector)
        return false;

    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        InsertIntoUpdateLog("NOT SUPPORTED ...");
        return false;
    }

    // IGNORE THIS VERSION
    // CALL DIRECTLY THE GOOD ONE
    return UpdateDb_B16_to_B17();
#if 0

    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage, strRootUser, strRootPassword;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nIncrementalUpdates = 0;

    /// ***********************************************************************************************///
    /// *******************************START OF NEW UPDATE HERE*************************************///
    /// ***********************************************************************************************///

    // Init progress bar
    if(pProgressBar)
    {
        pProgressBar->reset();
        /// TODO DEFINE NB STEPS
        pProgressBar->setMaximum(17);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop consolidated triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!DropConsolidatedTriggers(pTextEdit_Log, pProgressBar, eUnknownStage))
    {
        InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, m_pMainWindow);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
            goto updatedb_b15_to_b16_error_noset;

        // Retrieve values from dialog
        strRootUser = clGexdbGetrootDialog.GetRootUsername();
        strRootPassword = clGexdbGetrootDialog.GetRootPassword();

        if(!DropConsolidatedTriggers(pTextEdit_Log, pProgressBar, eUnknownStage, &strRootUser, &strRootPassword))
            goto updatedb_b15_to_b16_error_noset;
    }
    IncrementProgress();

    IncrementProgress();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Create new consolidation rules tables
    /// TODO Populate with default rules
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Creating global_files table.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B15_to_B16_CreateGlobalFilesTable(pTextEdit_Log, pProgressBar))
        goto updatedb_b15_to_b16_error_noset;


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Add fields to existing tables.
    // Populate fields with null or default values
    // Populate wafer_info fields with default consolidation :
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating wt_wafer_info table.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B15_to_B16_UpdateWtWaferInfoTable(pTextEdit_Log, pProgressBar))
        goto updatedb_b15_to_b16_error_noset;


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Create new wafer, sublot intermediate tables
    // Empty tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Creating WT intermediate tables.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B15_to_B16_CreateWtIntermediateTables(pTextEdit_Log, pProgressBar))
        goto updatedb_b15_to_b16_error_noset;


    UpdateConsolidationProcess(pTextEdit_Log, pProgressBar, eUnknownStage);

    IncrementProgress();


    /// ***********************************************************************************************///
    /// ************************************END OF NEW UPDATE HERE*************************************///
    /// ***********************************************************************************************///

    // Make sure progress bar is set to max
    if(pProgressBar)
    {
        int nPorgressMax = pProgressBar->maximum();
        pProgressBar->setValue(nPorgressMax);
        m_pqApplication->processEvents();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Check and Update indexes only if this is the last build
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //	if(GEXDB_DB_VERSION_BUILD == GEXDB_DB_VERSION_BUILD_B16)
    //	{
    //		if(!UpdateDb_UpdateIndexes(pTextEdit_Log, pProgressBar))
    //			goto updatedb_b15_to_b16_error_noset;
    //	}

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
    if(!clQuery.Execute(strQuery)) goto updatedb_b15_to_b16_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b15_to_b16_error;

    strQuery = "INSERT INTO global_info VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B16);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B16);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B16);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b15_to_b16_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b15_to_b16_writelog;

updatedb_b15_to_b16_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
updatedb_b15_to_b16_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b15_to_b16_writelog:

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Updates wt_wafer_info_table
// Add: consolidation_status, consolidated_data_type, consolidation_name, consolidation_prod_flow
// and consolidation_ref_date fields
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B15_to_B16_UpdateWtWaferInfoTable()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strTable, strQuery, strLogMessage;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QSqlRecord			clRecords;


    SetTestingStage(eWaferTest);
    strTable = NormalizeTableName("_wafer_info");
    IncrementProgress();

    // ADD consolidation_status column if it does not already exist
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("consolidation_status"))
    {
        // Add columns
        strLogMessage = "o Updating wt_wafer_info table.";
        InsertIntoUpdateLog(strLogMessage);
        strLogMessage = "Adding column consolidation_status to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN consolidation_status VARCHAR(255) DEFAULT NULL";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                // No Default value:  unsupported add column (if default value) operation on compressed table
                strQuery = "ALTER TABLE " + strTable + " ADD ( consolidation_status VARCHAR2(255))";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    // Switch to nullable
    // Switch to nullable
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT NULLABLE, DATA_DEFAULT FROM user_tab_columns ";
        strQuery+= " WHERE UPPER(table_name) = '"+strTable.toUpper()+"' AND UPPER(column_name) = 'CONSOLIDATION_STATUS'";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(clQuery.First()
                &&((clQuery.value(0).toString() == "N") || (clQuery.value(1).toString() != "")))
        {
            strLogMessage = "Switch " + strTable + ".consolidation_status to nullable ...";
            InsertIntoUpdateLog(strLogMessage);
            // Ran in a second time because
            // Oracle 10 reject add column with default value in the same query
            strQuery = "ALTER TABLE " + strTable + " MODIFY ( consolidation_status DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

    }
    else
    {
        strQuery = "SELECT IS_NULLABLE, COLUMN_DEFAULT FROm information_schema.columns ";
        strQuery+= " WHERE table_schema='"+m_pclDatabaseConnector->m_strSchemaName+"' ";
        strQuery+= " AND table_name='"+strTable.toLower()+"' and column_name='consolidation_status'";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(clQuery.First()
                &&((clQuery.value(0).toString() == "NO") || (clQuery.value(1).toString() != "")))
        {
            strLogMessage = "Switch " + strTable + ".consolidation_status to nullable ...";
            InsertIntoUpdateLog(strLogMessage);
            strQuery = "ALTER TABLE " + strTable + " MODIFY consolidation_status VARCHAR(255) DEFAULT NULL";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
    }

    // ADD consolidation_ref_date column if it does not already exist
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("consolidation_ref_date"))
    {
        // Add columns
        if(strLogMessage.isEmpty())
        {
            strLogMessage = "o Updating wt_wafer_info table.";
            InsertIntoUpdateLog(strLogMessage);
        }
        strLogMessage = "Adding column consolidation_ref_date to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN consolidation_ref_date DATETIME";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                // No Default value:  unsupported add column (if default value) operation on compressed table
                strQuery = "ALTER TABLE " + strTable + " ADD ( consolidation_ref_date DATE)";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    // UPDATE consolidation_status, consolidated_data_type, consolidation_name and consolidation_prod_flow fields

    // consolidation_status:	o P if wt_wafer_info.nb_parts>0
    //							o F if wt_wafer_info.nb_parts=0

    // Check if consolidation_status was already updated
    strQuery = "SELECT COUNT(*) FROM "+strTable;
    strQuery+= " WHERE ((consolidation_status IS NULL) OR (consolidation_status = ''))";
    if(!clQuery.Execute(strQuery) || !clQuery.First())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(clQuery.value(0).toInt() > 0)
    {
        if(strLogMessage.isEmpty())
        {
            strLogMessage = "o Updating wt_wafer_info table.";
            InsertIntoUpdateLog(strLogMessage);
        }
        strLogMessage = "Updating consolidation_status values...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery = "UPDATE " + strTable + "\n";
        strQuery += "SET consolidation_status=\n";
        strQuery += "CASE ";
        strQuery += "WHEN (nb_parts > 0) THEN 'P'\n";
        strQuery += "ELSE 'F'\n";
        strQuery += "END";

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE/POPULATE table wt_wafer_consolidation
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Set table name
    strTable = NormalizeTableName("_wafer_consolidation");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTable);

    strLogMessage = "o Create and porpulate wt_wafer_consolidation table.";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "Creating table "+strTable+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        QString strTableSpace, strLoggingMode;
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='GLOBAL_INFO'";
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

        strQuery = "CREATE TABLE "+strTable+" (\n";
        strQuery += "lot_id						VARCHAR2(255)	DEFAULT '',\n";
        strQuery += "wafer_id					VARCHAR2(255)	DEFAULT NULL,\n";
        strQuery += "nb_parts					NUMBER(8)		DEFAULT 0,\n";
        strQuery += "nb_parts_good				NUMBER(8)		DEFAULT 0,\n";
        strQuery += "consolidated_data_type		VARCHAR2(255)	NOT NULL,\n";
        strQuery += "consolidation_name			VARCHAR2(255)	NOT NULL,\n";
        strQuery += "consolidation_prod_flow	VARCHAR2(255)	NOT NULL\n";
        strQuery += ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        QString	strEngine,strFormat;
        GetStorageEngineName(strEngine,strFormat);

        strQuery =  "CREATE TABLE " + strTable + "(\n";
        strQuery += "lot_id						varchar(255)	NOT NULL DEFAULT '',\n";
        strQuery += "wafer_id					varchar(255)	DEFAULT NULL,\n";
        strQuery += "nb_parts					mediumint(8)	NOT NULL DEFAULT '0',\n";
        strQuery += "nb_parts_good				mediumint(8)	NOT NULL DEFAULT '0',\n";
        strQuery += "consolidated_data_type		VARCHAR(255)	NOT NULL,\n";
        strQuery += "consolidation_name			VARCHAR(255)	NOT NULL,\n";
        strQuery += "consolidation_prod_flow	VARCHAR(255)	NOT NULL\n";
        strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    strLogMessage = "Populating table "+strTable+"...";
    InsertIntoUpdateLog(strLogMessage);

    strQuery =  "INSERT INTO "+strTable;
    strQuery += "( ";
    strQuery += "SELECT wi.lot_id, wi.wafer_id, wi.nb_parts, wi.nb_parts_good, 'physical', 'final', 'Y' ";
    strQuery += "FROM "+NormalizeTableName("_wafer_info")+" wi ";
    strQuery += "WHERE wi.consolidation_status='P' ";
    strQuery += ")";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    IncrementProgress();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creates global_files table
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B15_to_B16_CreateGlobalFilesTable()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    QString				strLogMessage, strTableName, strTableSpace, strLoggingMode;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MySQL: get storage engine
    QString	strEngine,strFormat;
    if(m_pclDatabaseConnector->IsMySqlDB())
        GetStorageEngineName(strEngine,strFormat);

    /////////////////////////////////////////
    // DROP/CREATE tables
    /////////////////////////////////////////

    // Set table name
    strTableName = "global_files";

    strLogMessage = "o Creating global_files table.";
    InsertIntoUpdateLog(strLogMessage);

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);
    IncrementProgress();

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='GLOBAL_INFO'";
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

        strQuery = "CREATE TABLE "+strTableName+" (\n";
        strQuery += "file_id			NUMBER			NOT NULL ,\n";
        strQuery += "file_name			VARCHAR2(255)	NOT NULL,\n";
        strQuery += "file_type			VARCHAR2(255)	NOT NULL,\n";
        strQuery += "file_format		VARCHAR2(255)	NOT NULL,\n";
        strQuery += "file_content		CLOB			NOT NULL,\n";
        strQuery += "file_checksum		NUMBER			NOT NULL,\n";
        strQuery += "file_last_update	DATE			NOT NULL,\n";
        strQuery+=	"PRIMARY KEY  (file_id)\n";
        strQuery += ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery =  "CREATE TABLE " + strTableName + "(\n";
        strQuery += "file_id			int unsigned	NOT NULL	AUTO_INCREMENT,\n";
        strQuery += "file_name			VARCHAR(255)	NOT NULL,\n";
        strQuery += "file_type			VARCHAR(255)	NOT NULL,\n";
        strQuery += "file_format		VARCHAR(255)	NOT NULL,\n";
        strQuery += "file_content		MEDIUMBLOB		NOT NULL,\n";
        strQuery += "file_checksum		int unsigned	NOT NULL,\n";
        strQuery += "file_last_update	DATETIME		NOT NULL,\n";
        strQuery+=	"PRIMARY KEY  (file_id)\n";
        strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
    }

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
// Creates WT intermadiate tables:
//									o wt_wafer_hbin_inter
//									o wt_wafer_sbin_inter
//									o wt_wafer_consolidation
//									o wt_wafer_consolidation_inter
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B15_to_B16_CreateWtIntermediateTables()
{

    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    QString				strLogMessage, strTableName, strTableSpace, strLoggingMode;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MySQL: get storage engine
    QString	strEngine,strFormat;
    if(m_pclDatabaseConnector->IsMySqlDB())
        GetStorageEngineName(strEngine,strFormat);

    SetTestingStage(eWaferTest);

    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE tables wt_wafer_hbin_inter: Same as wt_wafer_hbin + consolidation_name
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set table name
    strTableName = NormalizeTableName("_wafer_hbin_inter");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);
    IncrementProgress();

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='WT_LOT'";
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

        strQuery = "CREATE TABLE "+strTableName+" (\n";
        strQuery += "lot_id				VARCHAR2(255)			NOT NULL,\n";
        strQuery += "wafer_id			VARCHAR2(255)			DEFAULT NULL,\n";
        strQuery += "hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,\n";
        strQuery += "hbin_name 			VARCHAR2(255)		DEFAULT '',\n";
        strQuery += "hbin_cat 			CHAR(1)					DEFAULT NULL,\n";
        strQuery += "nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, \n";
        strQuery += "consolidation_name VARCHAR2(255)			NOT NULL\n";
        strQuery += ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery =  "CREATE TABLE " + strTableName + "(\n";
        strQuery += "lot_id				varchar(255)		NOT NULL,\n";
        strQuery += "wafer_id			varchar(255)		DEFAULT NULL,\n";
        strQuery += "hbin_no			smallint(5)			unsigned NOT NULL DEFAULT '0',\n";
        strQuery += "hbin_name			varchar(255)	DEFAULT '',\n";
        strQuery += "hbin_cat			char(1)				DEFAULT NULL,\n";
        strQuery += "nb_parts			mediumint(8)		unsigned NOT NULL DEFAULT '0',\n";
        strQuery += "consolidation_name varchar(255)		NOT NULL,\n";
        strQuery += "KEY wtwaferhbininter_lot_wafer (lot_id,wafer_id)\n";
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

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE tables wt_wafer_sbin_inter: Same as wt_wafer_sbin  + consolidation_name
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Set table name
    strTableName = NormalizeTableName("_wafer_sbin_inter");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);
    IncrementProgress();

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "CREATE TABLE "+strTableName+" (\n";
        strQuery += "lot_id				VARCHAR2(255)		NOT NULL,\n";
        strQuery += "wafer_id			VARCHAR2(255)	DEFAULT NULL,\n";
        strQuery += "sbin_no			NUMBER(5)		DEFAULT '0',\n";
        strQuery += "sbin_name			VARCHAR2(255)	DEFAULT '',\n";
        strQuery += "sbin_cat			CHAR(1)			DEFAULT NULL,\n";
        strQuery += "nb_parts			NUMBER(8)		DEFAULT '0',\n";
        strQuery += "consolidation_name VARCHAR2(255)	NOT NULL\n";
        strQuery += ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery =  "CREATE TABLE " + strTableName + "(\n";
        strQuery += "lot_id				varchar(255)	NOT NULL,\n";
        strQuery += "wafer_id			varchar(255)	DEFAULT NULL,\n";
        strQuery += "sbin_no			smallint(5)		unsigned NOT NULL DEFAULT '0',\n";
        strQuery += "sbin_name			varchar(255)	DEFAULT '',\n";
        strQuery += "sbin_cat			char(1)			DEFAULT NULL,\n";
        strQuery += "nb_parts			mediumint(8)	unsigned NOT NULL DEFAULT '0',\n";
        strQuery += "consolidation_name	varchar(255)	NOT NULL,\n";
        strQuery += "KEY wtwafersbininter_lot_wafer (lot_id,wafer_id)\n";
        strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // DROP/CREATE table wt_wafer_consolidation_inter
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Set table name
    strTableName = NormalizeTableName("_wafer_consolidation_inter");

    // DROP TABLE IF EXISTS
    UpdateDb_DropTableIfExists(strTableName);

    strLogMessage = "Creating table "+strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // ORACLE
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "CREATE TABLE "+strTableName+" (\n";
        strQuery += "lot_id						VARCHAR2(255)	DEFAULT '',\n";
        strQuery += "wafer_id					VARCHAR2(255)	DEFAULT NULL,\n";
        strQuery += "nb_parts					NUMBER(8)		DEFAULT 0,\n";
        strQuery += "nb_parts_good				NUMBER(8)		DEFAULT 0,\n";
        strQuery += "consolidated_data_type		VARCHAR2(255)	NOT NULL,\n";
        strQuery += "consolidation_name			VARCHAR2(255)	NOT NULL,\n";
        strQuery += "consolidation_prod_flow	VARCHAR2(255)	NOT NULL\n";
        strQuery += ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS\n";
    }
    // MYSQL
    else if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery =  "CREATE TABLE " + strTableName + "(\n";
        strQuery += "lot_id						varchar(255)	NOT NULL DEFAULT '',\n";
        strQuery += "wafer_id					varchar(255)	DEFAULT NULL,\n";
        strQuery += "nb_parts					mediumint(8)	NOT NULL DEFAULT '0',\n";
        strQuery += "nb_parts_good				mediumint(8)	NOT NULL DEFAULT '0',\n";
        strQuery += "consolidated_data_type		VARCHAR(255)	NOT NULL,\n";
        strQuery += "consolidation_name			VARCHAR(255)	NOT NULL,\n";
        strQuery += "consolidation_prod_flow	VARCHAR(255)	NOT NULL\n";
        strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    IncrementProgress();

    InsertIntoUpdateLog("DONE.", false);

    // Create index
    //	QStringList lstIndexesToCheck;
    //	lstIndexesToCheck.append("wafer_id|wt_wafer_info%");
    //	lstIndexesToCheck.append("lot_id,wafer_id|%_wafer_%");
    //	UpdateDb_UpdateIndexes(pTextEdit_Log, pProgressBar, lstIndexesToCheck);

    IncrementProgress();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Updates global_info_table
// Add: db_status
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B15_to_B16_UpdateGlobalInfoTable()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strTable, strQuery, strLogMessage;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QSqlRecord			clRecords;

    strTable = NormalizeTableName("global_info",false);

    // ADD db_status column if it does not already exist
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("db_status"))
    {
        // Add columns
        strLogMessage = "Adding column db_status to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN db_status VARCHAR(255) DEFAULT NULL";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                // No Default value:  unsupported add column (if default value) operation on compressed table
                strQuery = "ALTER TABLE " + strTable + " ADD ( db_status VARCHAR2(255))";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    // Switch to nullable
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT NULLABLE, DATA_DEFAULT FROM user_tab_columns ";
        strQuery+= " WHERE UPPER(table_name) = 'GLOBAL_INFO' AND UPPER(column_name) = 'DB_STATUS'";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(clQuery.First()
                &&((clQuery.value(0).toString() == "N") || (clQuery.value(1).toString() != "")))
        {
            strLogMessage = "Switch " + strTable + ".db_status to nullable ...";
            InsertIntoUpdateLog(strLogMessage);
            // Ran in a second time because
            // Oracle 10 reject add column with default value in the same query
            strQuery = "ALTER TABLE " + strTable + " MODIFY ( db_status DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

    }
    else
    {
        strQuery = "SELECT IS_NULLABLE, COLUMN_DEFAULT FROm information_schema.columns ";
        strQuery+= " WHERE table_schema='"+m_pclDatabaseConnector->m_strSchemaName+"' ";
        strQuery+= " AND table_name='global_info' and column_name='db_status'";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(clQuery.First()
                &&((clQuery.value(0).toString() == "NO") || (clQuery.value(1).toString() != "")))
        {
            strLogMessage = "Switch " + strTable + ".db_status to nullable ...";
            InsertIntoUpdateLog(strLogMessage);
            strQuery = "ALTER TABLE " + strTable + " MODIFY ( db_status DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

    }

    return true;
}

