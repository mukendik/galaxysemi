// gexdb_plugin_galaxy_admin_b13.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B12->B13 upgrade
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
#include <QSqlRecord>
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
// Update DB: B12 -> B13
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage;
    QString strRootUser, strRootPassword;
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
    SetProgress(28);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Update SYA tables:
    //    xx_sbl: add rule_type, n1_parameter, n2_parameter (set these fields to null for existing data)
    //    xx_sya_set: add rule_name (set this field to 'unknown' for existing data)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating SYA tables.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B12_to_B13_UpdateSyaTables())
        goto updatedb_b12_to_b13_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Update tables for genealogy
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating tables for genealogy.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B12_to_B13_UpdateTablesForGenealogy())
        goto updatedb_b12_to_b13_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Update tables with nullable fields
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating product binning tables with nullable bin_name fields.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B12_to_B13_UpdateTablesForNullableValue())
        goto updatedb_b12_to_b13_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Drop consolidated triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Updating consolidated triggers.";
    InsertIntoUpdateLog(strLogMessage);
    if(!DropConsolidationTriggers(eUnknownStage))
    {
        InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
            goto updatedb_b12_to_b13_error_noset;

        // Retrieve values from dialog
        strRootUser = clGexdbGetrootDialog.GetRootUsername();
        strRootPassword = clGexdbGetrootDialog.GetRootPassword();

        if(!DropConsolidationTriggers(eUnknownStage, &strRootUser, &strRootPassword))
            goto updatedb_b12_to_b13_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 5) Re-Create consolidated triggers
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidationTriggers())
    {
        InsertIntoUpdateLog("TRIGGER generation FAILED, trying with root access.");

        // Get root user+password if not already done for the drop
        if(strRootUser.isEmpty())
        {
            GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

            if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
                goto updatedb_b12_to_b13_error_noset;

            // Retrieve values from dialog
            strRootUser = clGexdbGetrootDialog.GetRootUsername();
            strRootPassword = clGexdbGetrootDialog.GetRootPassword();
        }

        if(!CreateConsolidationTriggers(&strRootUser, &strRootPassword))
            goto updatedb_b12_to_b13_error_noset;
    }
    IncrementProgress();

    // Make sure progress bar is set to max
    ResetProgress(true);
    QCoreApplication::processEvents();

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
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;

    strQuery = "INSERT INTO global_info VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B13);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B13);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B13);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b12_to_b13_writelog;

updatedb_b12_to_b13_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
updatedb_b12_to_b13_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b12_to_b13_writelog:

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update SYA tables:
//    xx_sbl: add rule_type, n1_parameter, n2_parameter (set these fields to null for existing data)
//    xx_sya_set: add rule_name (set this field to 'unknown' for existing data)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13_UpdateSyaTables()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    QString				strLogMessage, strTable;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nTestingStage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Update xx_sbl tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FOR EACH TESTING STAGE DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++ )
    {
        SetTestingStage(nTestingStage);

        strTable = NormalizeTableName("_sbl");

        // ADD rule_type column if it does not already exist
        strLogMessage = "Checking if rule_type has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        QSqlRecord clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("rule_type"))
        {
            // Add columns
            strLogMessage = "Adding column rule_type to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN rule_type varchar(255) DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( rule_type VARCHAR2(255) DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();

        // ADD n1_parameter column if it does not already exist
        strLogMessage = "Checking if n1_parameter has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("n1_parameter"))
        {
            // Add columns
            strLogMessage = "Adding column n1_parameter to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN n1_parameter float DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( n1_parameter NUMBER    DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();

        // ADD n2_parameter column if it does not already exist
        strLogMessage = "Checking if n2_parameter has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("n2_parameter"))
        {
            // Add columns
            strLogMessage = "Adding column n2_parameter to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN n2_parameter float DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( n2_parameter NUMBER    DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Update xx_sya_set tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FOR EACH TESTING STAGE DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++ )
    {
        SetTestingStage(nTestingStage);

        strTable = NormalizeTableName("_sya_set");

        // ADD rule_name column if it does not already exist
        strLogMessage = "Checking if rule_name has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        QSqlRecord clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("rule_name"))
        {
            // Add columns
            strLogMessage = "Adding column rule_name to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN rule_name varchar(255) DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( rule_name VARCHAR2(255) DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update tables for Genealogy:
//    xx_wafer_info: add wafer_nb
//    xx_splitlot: add wafer_nb
//    et_wafer_info: add site_config
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13_UpdateTablesForGenealogy()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    QString				strLogMessage, strTable;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nTestingStage;
    QSqlRecord			clRecords;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) et_wafer_info/wt_wafer_info/et_splitlot/wt_splitlot: add wafer_nb
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FOR ET, WT DO THE SAME ACTION
    for(nTestingStage=0; nTestingStage<2; nTestingStage++)
    {
        switch(nTestingStage)
        {
        case 0 :
            // FOR WAFER TEST
            SetTestingStage(eWaferTest);
            break;
        case 1 :
            // FOR ELECT TEST
            SetTestingStage(eElectTest);
            break;
        default:
            continue;
        }

        // ADD xx_wafer_info.wafer_nb column if it does not already exist
        strTable = NormalizeTableName("_wafer_info");
        strLogMessage = "Checking if wafer_nb has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("wafer_nb"))
        {
            // Add columns
            strLogMessage = "Adding column wafer_nb to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN wafer_nb tinyint(3) unsigned DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( wafer_nb   NUMBER(3)  DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();

        // ADD xx_splitlot.wafer_nb column if it does not already exist
        strTable = NormalizeTableName("_splitlot");
        strLogMessage = "Checking if wafer_nb has to be added to table "+strTable;
        InsertIntoUpdateLog(strLogMessage);
        clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
        if(!clRecords.contains("wafer_nb"))
        {
            // Add columns
            strLogMessage = "Adding column wafer_nb to table " + strTable + "...";
            InsertIntoUpdateLog(strLogMessage);
            if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
                strQuery = "ALTER TABLE " + strTable + " ADD COLUMN wafer_nb tinyint(3) unsigned DEFAULT NULL";
            else
                if(m_pclDatabaseConnector->IsOracleDB())
                    strQuery = "ALTER TABLE " + strTable + " ADD ( wafer_nb   NUMBER(3)  DEFAULT NULL)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            InsertIntoUpdateLog("DONE.", false);
        }
        else
            InsertIntoUpdateLog("Column already present.");
        IncrementProgress();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) et_wafer_info/et_splitlot: add site_config
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eElectTest);

    // ADD et_wafer_info.site_config column if it does not already exist
    strTable = NormalizeTableName("_wafer_info");
    strLogMessage = "Checking if site_config has to be added to table "+strTable;
    InsertIntoUpdateLog(strLogMessage);
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("site_config"))
    {
        // Add columns
        strLogMessage = "Adding column site_config to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN site_config varchar(255) DEFAULT NULL";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "ALTER TABLE " + strTable + " ADD ( site_config VARCHAR2(255) DEFAULT NULL)";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    else
        InsertIntoUpdateLog("Column already present.");
    IncrementProgress();

    // ADD et_splitlot.site_config column if it does not already exist
    strTable = NormalizeTableName("_splitlot");
    strLogMessage = "Checking if site_config has to be added to table "+strTable;
    InsertIntoUpdateLog(strLogMessage);
    clRecords = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("site_config"))
    {
        // Add columns
        strLogMessage = "Adding column site_config to table " + strTable + "...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable + " ADD COLUMN site_config varchar(255) DEFAULT NULL";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "ALTER TABLE " + strTable + " ADD ( site_config VARCHAR2(255) DEFAULT NULL)";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    else
        InsertIntoUpdateLog("Column already present.");
    IncrementProgress();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update product tables for Oracle:
//    must accept null or empty values
// Update mptest_results tables:
//    must accept null or empty values
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13_UpdateTablesForNullableValue()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strQuery;
    QString				strLogMessage, strTable;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nTestingStage;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) et_wafer_info/wt_wafer_info/et_splitlot/wt_splitlot: add wafer_nb
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FOR ET, WT DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++)
    {
        SetTestingStage(nTestingStage);

        // Change product_hbin.bin_name field nullable
        strTable = NormalizeTableName("_product_hbin");
        strLogMessage = "Changing type of bin_name field in " + strTable + " (switch to nullable)...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN bin_name varchar(255) NULL";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "ALTER TABLE " + strTable + " MODIFY ( bin_name   varchar2(255) NULL)";
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
        IncrementProgress();

        // Change product_sbin.bin_name field nullable
        strTable = NormalizeTableName("_product_sbin");
        strLogMessage = "Changing type of bin_name field in " + strTable + " (switch to nullable)...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN bin_name varchar(255) NULL ";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "ALTER TABLE " + strTable + " MODIFY ( bin_name   varchar2(255)  NULL)";
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
        IncrementProgress();

        if(nTestingStage == eElectTest) // Elest test
            continue;

#if 0
        // Do not update the field for mptest_results.value nullable
        // Alter table could take a very long time for some big DB
        // Still use the INVALID_RESULT flag

        // Change mptest_results.value field nullable
        strTable = NormalizeTableName("_mptest_results");
        strLogMessage = "Changing type of value field in " + strTable + " (switch to nullable)...";
        InsertIntoUpdateLog(strLogMessage);
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN value float NULL ";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "ALTER TABLE " + strTable + " MODIFY ( value NUMBER NULL)";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
#endif
        IncrementProgress();

    }

    return true;
}


