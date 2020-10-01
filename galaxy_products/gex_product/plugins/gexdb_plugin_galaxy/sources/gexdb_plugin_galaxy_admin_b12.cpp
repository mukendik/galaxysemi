// gexdb_plugin_galaxy_admin_b12.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B11->B12 upgrade
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
// Update DB: B11 -> B12
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B11_to_B12()
{
    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nIncrementalUpdates = 0;
    QString				strRootUser, strRootPassword;

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
    SetProgress(44);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Update xx_product_sbin, and xx_product_hbin
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Checking if xx_product_sbin and xx_product_hbin table have to be updated.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B11_to_B12_UpdateProductBinTables())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Update xx_metadata tables...
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Update xx_metadata tables.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B11_to_B12_UpdateMapping())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Reload Metadata (fake B12, because we already created the az_field column in UpdateDb_B11_to_B12_UpdateMapping())
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!LoadMetaData(12))
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We re-create all B10 consolidated tables because loading of metadata changed:
    // meta-data with same GEX name, same Meta name, same DB field are replaced, so there should not be
    // any duplicate (_1) field in the consolidated table
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 5) Drop consolidated triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Update consolidated tables.";
    InsertIntoUpdateLog(strLogMessage);
    if(!DropConsolidationTriggers(eUnknownStage))
    {
        InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
            goto updatedb_b11_to_b12_error_noset;

        // Retrieve values from dialog
        strRootUser = clGexdbGetrootDialog.GetRootUsername();
        strRootPassword = clGexdbGetrootDialog.GetRootPassword();

        if(!DropConsolidationTriggers(eUnknownStage, &strRootUser, &strRootPassword))
            goto updatedb_b11_to_b12_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 6) Re-Create consolidated tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidatedTables())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 7) Re-Create consolidated update procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Update consolidated update procedures.";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidatedUpdateProcedures())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 8) Create TL tables (TrackingLot level consolidated tables)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Create TL tables (TrackingLot level consolidated tables).";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidatedTables_TL())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 9) Create TL tables update stored procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Create TL tables update stored procedures.";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidatedUpdateProcedures_TL())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 10) Create AZ tables (AZ level consolidated tables)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Create AZ tables (AZ level consolidated tables).";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidatedTables_AZ())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 11) Create AZ tables update stored procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Create AZ tables update stored procedures.";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidatedUpdateProcedure_AZ())
        goto updatedb_b11_to_b12_error_noset;
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 12) Re-Create consolidated triggers
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Update consolidated triggers.";
    InsertIntoUpdateLog(strLogMessage);
    if(!CreateConsolidationTriggers())
    {
        InsertIntoUpdateLog("TRIGGER generation FAILED, trying with root access.");

        // Get root user+password if not already done for the drop
        if(strRootUser.isEmpty())
        {
            GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

            if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
                goto updatedb_b11_to_b12_error_noset;

            // Retrieve values from dialog
            strRootUser = clGexdbGetrootDialog.GetRootUsername();
            strRootPassword = clGexdbGetrootDialog.GetRootPassword();
        }

        if(!CreateConsolidationTriggers(&strRootUser, &strRootPassword))
            goto updatedb_b11_to_b12_error_noset;
    }
    IncrementProgress();

    // Add GEXDB_MYSQL_ENGINE global option, if not already set
    InsertIntoUpdateLog("o Adding/updating GEXDB_MYSQL_ENGINE global option...");
    strQuery = "select * from global_options where option_name='GEXDB_MYSQL_ENGINE'";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(!clQuery.Next())
    {
        // Option not set, add it
        strQuery = "insert into global_options values('GEXDB_MYSQL_ENGINE', 'MyISAM')";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }
    else
    {
        // Option already set, leave it
    }

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
    if(!clQuery.Execute(strQuery)) goto updatedb_b11_to_b12_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b11_to_b12_error;

    strQuery = "INSERT INTO global_info VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B12);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B12);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B12);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b11_to_b12_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b11_to_b12_writelog;

updatedb_b11_to_b12_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
updatedb_b11_to_b12_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b11_to_b12_writelog:

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}


bool GexDbPlugin_Galaxy::UpdateDb_B11_to_B12_UpdateMapping()
{
    QString				strQuery;
    QString				strLogMessage, strTable1, strField;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_Query	clQuery2(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    unsigned int		uiRowsRemoved=0;
    int					nItems=0, nTestingStage;

    // FOR EACH TESTING STAGE DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++ )
    {
        SetTestingStage(nTestingStage);

        // Remove duplicates from xx_metadata_mapping
        uiRowsRemoved = 0;
        strTable1 = NormalizeTableName("_metadata_mapping");
        strLogMessage = "Removing duplicate entries in " + strTable1 + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "SELECT DISTINCT meta_name FROM " + strTable1;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clQuery.Next())
        {
            // For each distinct meta_name, remove duplicates
            strField = clQuery.value(0).toString();
            do
            {
                strQuery = "SELECT meta_name FROM " + strTable1 + " WHERE LOWER(meta_name)='" + strField.toLower() + "'";
                if(!clQuery2.Execute(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery2.lastError().text().toLatin1().constData());
                    return false;
                }
                nItems = clQuery2.size();
                if(nItems > 1)
                {
                    strQuery = "DELETE FROM " + strTable1 + " WHERE LOWER(meta_name)='" + strField.toLower() + "' limit 1";
                    if(!clQuery2.Execute(strQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery2.lastError().text().toLatin1().constData());
                        return false;
                    }
                    uiRowsRemoved++;
                }
            }
            while(nItems > 1);
        }
        strLogMessage = "DONE (" + QString::number(uiRowsRemoved) + " removed).";
        InsertIntoUpdateLog(strLogMessage, false);
        IncrementProgress();

        // Remove duplicates from xx_metadata_link
        uiRowsRemoved = 0;
        strTable1 = NormalizeTableName("_metadata_link");
        strLogMessage = "Removing duplicate entries in " + strTable1 + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "SELECT DISTINCT link_name FROM " + strTable1;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clQuery.Next())
        {
            // For each distinct meta_name, remove duplicates
            strField = clQuery.value(0).toString();
            do
            {
                strQuery = "SELECT link_name FROM " + strTable1 + " WHERE LOWER(link_name)='" + strField.toLower() + "'";
                if(!clQuery2.Execute(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery2.lastError().text().toLatin1().constData());
                    return false;
                }
                nItems = clQuery2.size();
                if(nItems > 1)
                {
                    strQuery = "DELETE FROM " + strTable1 + " WHERE LOWER(link_name)='" + strField.toLower() + "' limit 1";
                    if(!clQuery2.Execute(strQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery2.lastError().text().toLatin1().constData());
                        return false;
                    }
                    uiRowsRemoved++;
                }
            }
            while(nItems > 1);
        }
        strLogMessage = "DONE (" + QString::number(uiRowsRemoved) + " removed).";
        InsertIntoUpdateLog(strLogMessage, false);
        IncrementProgress();

        strTable1 = NormalizeTableName("_metadata_mapping");

        // ADD az_field column
        // Drop columns first
        if(m_pclDatabaseConnector->IsSQLiteDB())
        {
            // DROP COLUMN NOT SUPPORTED
            strQuery = "CREATE TABLE " + strTable1 + "_new AS ";
            strQuery+= "SELECT meta_name, gex_name, gexdb_table_name, gexdb_field_fullname, gexdb_link_name, gex_display_in_gui, bintype_field, time_field, custom_field, numeric_field, fact_field, consolidated_field, er_display_in_gui ";
            strQuery+= "FROM " + strTable1;
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strQuery = "DROP TABLE " + strTable1;
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strQuery = "ALTER TABLE " + strTable1 + "_new RENAME TO " + strTable1;
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
        else
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "ALTER TABLE " + strTable1 + " DROP COLUMN az_field";
            clQuery.Execute(strQuery);
        }
        else
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "ALTER TABLE " + strTable1 + " SET UNUSED COLUMN az_field";
            clQuery.Execute(strQuery);
            strQuery = "ALTER TABLE " + strTable1 + " DROP UNUSED COLUMNS";
            clQuery.Execute(strQuery);
        }

        // Add columns
        strLogMessage = "Adding columns to " + strTable1;
        InsertIntoUpdateLog(strLogMessage);
        InsertIntoUpdateLog("Adding column az_field...");
        if(m_pclDatabaseConnector->IsMySqlDB() || m_pclDatabaseConnector->IsSQLiteDB())
            strQuery = "ALTER TABLE " + strTable1 + " ADD COLUMN az_field char(1) NOT NULL DEFAULT 'N'";
        else
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery = "ALTER TABLE " + strTable1 + " ADD ( az_field char(1))";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
        IncrementProgress();
        strQuery = "UPDATE " + strTable1 + " SET az_field=consolidated_field";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }

    return true;
}

bool
GexDbPlugin_Galaxy::
UpdateDb_B11_to_B12_UpdateProductBinTables()
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsMySqlDB())
        return true;

    QString				strQuery;
    QString				strLogMessage, strTable, strTableSpace, strLoggingMode;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nTestingStage;

    // MySQL: get storage engine
    QString	strEngine,strFormat;
    if(m_pclDatabaseConnector->IsMySqlDB())
        GetStorageEngineName(strEngine,strFormat);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Create ft_product_sbin table if not exist
    //    Database creation script for GEXDB B10 didn't create this table in B157
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eFinalTest);
    strTable = NormalizeTableName("_product_sbin");
    strLogMessage = "Checking if "+strTable+" table has to be created.";
    InsertIntoUpdateLog(strLogMessage);

    // Create if not exist
    //strQuery = "SELECT * FROM "+strTable;
    //if(clQuery.Execute(strQuery))
    QStringList strlTables;
    m_pclDatabaseConnector->EnumTables(strlTables);
    if(strlTables.contains(strTable, Qt::CaseInsensitive))
        InsertIntoUpdateLog("Table already exists.");
    else
    {
        strLogMessage = "Creating table "+strTable+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Create table
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            // MySQL
            strQuery =  "CREATE TABLE "+strTable+" (";
            strQuery += "product_name	varchar(255)			NOT NULL,\n";
            strQuery += "bin_no			smallint(5) unsigned	NOT NULL,\n";
            strQuery += "bin_name		varchar(255)			NOT NULL DEFAULT '',\n";
            strQuery += "bin_cat		char(1)					DEFAULT NULL,\n";
            strQuery += "nb_parts		bigint(20)				NOT NULL DEFAULT 0,\n";
            strQuery += "KEY ft_product_sbin_bin (product_name,bin_no)\n";
            strQuery += ") ENGINE=" + strEngine + " DEFAULT CHARSET=latin1 PACK_KEYS=1 "+strFormat+"\n";
        }
        else
        if(m_pclDatabaseConnector->IsSQLiteDB())
        {
            // SQLite
            strQuery =  "CREATE TABLE "+strTable+" (";
            strQuery += "product_name	varchar(255)			NOT NULL,\n";
            strQuery += "bin_no			smallint(5)				NOT NULL,\n";
            strQuery += "bin_name		varchar(255)			NOT NULL DEFAULT '',\n";
            strQuery += "bin_cat		char(1)					DEFAULT NULL,\n";
            strQuery += "nb_parts		bigint(20)				NOT NULL DEFAULT 0,\n";
            strQuery += "KEY ft_product_sbin_bin (product_name,bin_no)\n";
            strQuery += ")";
        }
        else
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            // Oracle
            // Found some specification
            // Get some information like for the PRODUCT table
            strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='FT_LOT'";
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

            strQuery =  "CREATE TABLE "+strTable+" (";
            strQuery += "product_name		VARCHAR2(255)	NOT NULL,\n";
            strQuery += "bin_no				NUMBER			NOT NULL,\n";
            strQuery += "bin_name			VARCHAR2(255)	DEFAULT '' NOT NULL,\n";
            strQuery += "bin_cat			CHAR(1)			DEFAULT NULL,\n";
            strQuery += "nb_parts			NUMBER			DEFAULT 0 NOT NULL\n";
            strQuery+= ") TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 "+strLoggingMode+" COMPRESS";
        }
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = "CREATE INDEX " + strTable + "_bin ON " + strTable + " (product_name,bin_no)";
            if(!clQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }

        }

        InsertIntoUpdateLog("DONE.", false);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Update xx_product_xbin tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FOR EACH TESTING STAGE DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++ )
    {
        SetTestingStage(nTestingStage);

        // Change bin_no field type (adding unsigned)
        strTable = NormalizeTableName("_product_hbin");
        strLogMessage = "Changing type of bin_no field in " + strTable + " (switch to unsigned)...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN bin_no smallint(5) unsigned NOT NULL";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        strTable = NormalizeTableName("_product_sbin");
        strLogMessage = "Changing type of bin_no field in " + strTable + " (switch to unsigned)...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "ALTER TABLE " + strTable + " MODIFY COLUMN bin_no smallint(5) unsigned NOT NULL";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update DB: B11 -> B12
// Check and update indexes for Oracle and MySql
///////////////////////////////////////////////////////////
bool
GexDbPlugin_Galaxy::
UpdateDb_B11_to_B12_UpdateIndexes()
{
#if 0
    QString				strQuery;
    QString				strLogMessage, strErrorMessage;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    unsigned int		uiNbIndexesCreated=0;

    /////////////////////////////////////////
    // Check if all tables with splitlot_id column have an index
    /////////////////////////////////////////
    InsertIntoUpdateLog("Updating indexes...");

    // MySql-specific
    QString strTable;
    QString	strIndexes;
    QString strIndexName;
    QString strValue;
    QStringList lstTablesForIndexes;
    QStringList lstIndexToCheck;

    // Check if have indexes to do
    // for all tables with splitlot_id column
    // Select tables with splitlot_id
    // Select indexes with splitlot_id
    // Then extract info and check needed indexes

    lstIndexToCheck.append("splitlot_id|ft_%");
    lstIndexToCheck.append("splitlot_id|et_%");
    lstIndexToCheck.append("splitlot_id|wt_%");
    lstIndexToCheck.append("lot_id|ft_lot%");
    lstIndexToCheck.append("lot_id|et_lot%");
    lstIndexToCheck.append("lot_id|wt_lot%");
    lstIndexToCheck.append("start_t|ft_splitlot");
    lstIndexToCheck.append("start_t|et_splitlot");
    lstIndexToCheck.append("start_t|wt_splitlot");

    while(!lstIndexToCheck.isEmpty())
    {
        IncrementProgress();
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "SELECT T0.TABLE_NAME, T0.INDEX_NAME, T0.MAX_SEQ, T0.INDEXES FROM ";
            strQuery+= " ( ";
            strQuery+= "    SELECT T.TABLE_NAME, S.INDEX_NAME, MAX(S.SEQ_IN_INDEX) AS MAX_SEQ, ";
            strQuery+= "    GROUP_CONCAT(S.COLUMN_NAME ORDER BY S.SEQ_IN_INDEX SEPARATOR ',') AS INDEXES ";
            strQuery+= "    FROM ";
            strQuery+= "         (SELECT DISTINCT TABLE_SCHEMA, TABLE_NAME ";
            strQuery+= "          FROM information_schema.COLUMNS ";
            strQuery+= "          WHERE TABLE_SCHEMA='" + m_pclDatabaseConnector->m_strSchemaName + "' ";
            strQuery+= "          AND (TABLE_NAME) LIKE '%2'";
            strQuery+= "          AND (COLUMN_NAME)='%1' ";
            strQuery+= "         ) T ";
            strQuery+= "    LEFT OUTER JOIN information_schema.STATISTICS S0 ";
            strQuery+= " 	ON T.TABLE_SCHEMA=S0.TABLE_SCHEMA ";
            strQuery+= " 	AND T.TABLE_NAME=S0.TABLE_NAME ";
            strQuery+= " 	AND (S0.COLUMN_NAME)='%1' ";
            strQuery+= " 	LEFT OUTER JOIN information_schema.STATISTICS S ";
            strQuery+= " 	ON T.TABLE_SCHEMA=S.TABLE_SCHEMA ";
            strQuery+= " 	AND T.TABLE_NAME=S.TABLE_NAME ";
            strQuery+= " 	AND S0.INDEX_NAME=S.INDEX_NAME ";
            strQuery+= " 	GROUP BY T.TABLE_NAME, S.INDEX_NAME ";
            strQuery+= "  ) T0 ";
            strQuery+= " ORDER BY T0.TABLE_NAME , T0.MAX_SEQ DESC";
        }
        else
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery = " SELECT T2.TABLE_NAME, T2.INDEX_NAME, MAX(T2.COLUMN_POSITION) AS MAX_SEQ, ";
            strQuery+= " wm_concat(T2.COLUMN_NAME) AS INDEXES , MAX(T2.COMPRESSION) ";
            strQuery+= " FROM ";
            strQuery+= " ( ";
            strQuery+= " SELECT T1.TABLE_NAME, C1.INDEX_NAME, C1.COLUMN_POSITION, C1.COLUMN_NAME, P.COMPRESSION ";
            strQuery+= " FROM ";
            strQuery+= " ( ";
            strQuery+= " 	SELECT DISTINCT T.TABLE_NAME, C.INDEX_NAME ";
            strQuery+= " 	FROM (SELECT DISTINCT TABLE_NAME FROM USER_TAB_COLUMNS WHERE LOWER(COLUMN_NAME)='%1'";
            strQuery+= "          AND lower(TABLE_NAME) LIKE '%2'";
            strQuery+= "         ) T  ";
            strQuery+= "         LEFT OUTER JOIN USER_IND_COLUMNS C ";
            strQuery+= " 	ON T.TABLE_NAME=C.TABLE_NAME ";
            strQuery+= " 	AND lower(C.COLUMN_NAME)='%1' ";
            strQuery+= " ) T1 ";
            strQuery+= " LEFT OUTER JOIN ";
            strQuery+= " USER_IND_COLUMNS C1 ";
            strQuery+= " ON T1.TABLE_NAME=C1.TABLE_NAME ";
            strQuery+= " AND T1.INDEX_NAME=C1.INDEX_NAME ";
            strQuery+= " LEFT OUTER JOIN USER_TAB_PARTITIONS P ";
            strQuery+= " ON T1.TABLE_NAME=P.TABLE_NAME ";
            strQuery+= " AND P.PARTITION_POSITION=1 ";
            strQuery+= " ORDER BY T1.TABLE_NAME, C1.INDEX_NAME, C1.COLUMN_POSITION ";
            strQuery+= " )T2 ";
            strQuery+= " GROUP BY T2.TABLE_NAME, T2.INDEX_NAME";
        }
        else
        if(m_pclDatabaseConnector->IsSQLiteDB())
        {
            strQuery = " SELECT T2.tbl_name, T2.name, 1, T2.sql FROM ";
            strQuery+= "( SELECT tbl_name FROM sqlite_master WHERE type='table'  AND lower(sql) LIKE '% %1 %' AND lower(NAME) LIKE '%2'";
            strQuery+= ") AS T1 ";
            strQuery+= "LEFT OUTER JOIN ";
            strQuery+= "sqlite_master T2 ";
            strQuery+= "ON ";
            strQuery+= "T1.tbl_name=T2.tbl_name ";
            strQuery+= "WHERE T2.type='index' ";
            strQuery+= "ORDER BY T2.tbl_name, T2.name ";
        }
        else
            return false;

        strIndexName = lstIndexToCheck.takeFirst();
        strQuery = strQuery.arg(strIndexName.section("|",0,0),strIndexName.section("|",1,1));
        if(!clQuery.Execute(strQuery)) goto updatedb_b9_to_b10_error;
        strTable = "";


        // For each table
        // Check indexes
        while(clQuery.Next())
        {
            if(strTable == clQuery.value(0).toString())
                continue;

            // Table Name
            strTable = clQuery.value(0).toString();
            // Index defined (empty if null)
            strValue = clQuery.value(3).toString();

            // Construct the needed index
            if(strIndexName.section("|",0,0) == "lot_id")
            {
                strIndexes = "lot_id";
                if(strTable.endsWith("_lot",Qt::CaseInsensitive))
                    strIndexes = "product_name," + strIndexes;
            }
            else
            if(strIndexName.section("|",0,0) == "start_t")
            {
                strIndexes = "start_t";
            }
            else
            {
                strIndexes = "splitlot_id";
                if(strTable.toLower().indexOf("test_") == 4)
                    strIndexes += "," + strTable.section("_",1,1) + "_info_id";
            }

            if(!strValue.isEmpty())
            {
                // Have an index defined
                if(m_pclDatabaseConnector->IsSQLiteDB())
                    strValue = strValue.section("(",1);
                // Check if it is the good one or more
                // Check with the existing index
                if(strValue.startsWith(strIndexes,Qt::CaseInsensitive))
                    continue;
            }

            // Check if have table in the next result
            clQuery.Next();
            strValue = clQuery.value(0).toString();
            clQuery.previous();
            if(strTable  == strValue)
            {
                strTable = "";
                continue;
            }
            // Create new index
            // Create index TABLE_NAME on TABLE_NAME(INDEX1,INDEX2)
            strValue = strTable + "(" + strIndexes + ")";

            if(m_pclDatabaseConnector->IsOracleDB())
            {
                // For Oracle
                // Specific creation when table partitionned and compressed
                if(!clQuery.value(4).toString().isEmpty())
                    strValue += " LOCAL";
                if(clQuery.value(4).toString() == "ENABLED")
                    strValue += " COMPRESS";

                strValue = strValue.toUpper();
            }
            lstTablesForIndexes.append(strValue);
        }

        // Have the list of all indexes to create
        // Start the process
        while(!lstTablesForIndexes.isEmpty())
        {
            strIndexes = lstTablesForIndexes.takeFirst();
            strTable = strIndexes.section("(",0,0);
            // construct a unique name not too long (oracle restriction)
            strIndexName = strIndexes.section(")",0,0).toUpper();
            strIndexName = strIndexName.remove("_INFO").replace("STATS_SUMMARY","SUM").replace("STATS_SAMPLES","SAMP").remove("_ID").remove("_").replace(",","_").replace("(","_");
            if(m_pclDatabaseConnector->IsOracleDB())
                strIndexName = strIndexName.left(30);
            else
                strIndexName = strIndexName.toLower();

            strLogMessage = "Adding index " + strIndexes + " ... ";
            InsertIntoUpdateLog(strLogMessage);
            // Drop index if exist
            strQuery = "DROP INDEX " + strIndexName;
            if(m_pclDatabaseConnector->IsMySqlDB())
                strQuery+= " ON " + strTable;
            clQuery.Execute(strQuery);

            // Then create a new one
            strQuery = "CREATE INDEX " + strIndexName;
            strQuery+= " ON " + strIndexes;
            if(!clQuery.Execute(strQuery)) goto updatedb_b9_to_b10_error;


            InsertIntoUpdateLog("DONE.",false);
            uiNbIndexesCreated++;
        }
    }

    // Log status
    InsertIntoUpdateLog(QString(" %1 indexes updated").arg(uiNbIndexesCreated));
    InsertIntoUpdateLog(" ");

    return true;

updatedb_b11_to_b12_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage;
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);
#endif

    return false;
}
