// gexdb_plugin_galaxy_admin_b16.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B16->B17 upgrade
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
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QProgressBar>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update DB: B16->B17
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Add Status to Global_info
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B16_to_B17()
{
    if (!m_pclDatabaseConnector)
        return false;

    QString             lQuery;
    bool                lStatus = false;
    QString             lLogMessage, lErrorMessage, lRootUser, lRootPassword;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    int lIncrementalSplitlots;

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
    SetMaxProgress(17);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update Global Info
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lLogMessage = "o Updating GLOBAL_INFO table.";
    InsertIntoUpdateLog(lLogMessage);

    if(!UpdateDb_B15_to_B16_UpdateGlobalInfoTable())
        goto updatedb_b16_to_b17_error_noset;
    IncrementProgress();

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateDb
                          |eUpdateConsTree
                          |eUpdateConsTriggers
                          |eUpdateConsTables
                          |eUpdateConsProcedures
                          |eUpdateIndexes)) goto updatedb_b16_to_b17_error_noset;

    // ***********************************************************************************************///
    // *******************************START OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lQuery = "UPDATE global_info SET ";
    lQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B17)+"'";
    lQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B17);
    lQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B17);

    if(!clQuery.Execute(lQuery)) goto updatedb_b16_to_b17_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop consolidated triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!DropConsolidationTriggers(eUnknownStage))
    {
        InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

        if(m_pclDatabaseConnector->IsOracleDB())
            goto updatedb_b16_to_b17_error_noset;

        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
            goto updatedb_b16_to_b17_error_noset;

        // Retrieve values from dialog
        lRootUser = clGexdbGetrootDialog.GetRootUsername();
        lRootPassword = clGexdbGetrootDialog.GetRootPassword();

        if(!DropConsolidationTriggers(eUnknownStage, &lRootUser, &lRootPassword))
            goto updatedb_b16_to_b17_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Create and Populate consolidation rules tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!UpdateDb_B15_to_B16_CreateGlobalFilesTable())
        goto updatedb_b16_to_b17_error_noset;


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Add fields to existing tables.
    // Populate fields with null or default values
    // Populate wafer_info fields with default consolidation :
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!UpdateDb_B15_to_B16_UpdateWtWaferInfoTable())
        goto updatedb_b16_to_b17_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Create new wafer, sublot intermediate tables
    // Empty tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lLogMessage = "o Creating WT intermediate tables.";
    InsertIntoUpdateLog(lLogMessage);
    if(!UpdateDb_B15_to_B16_CreateWtIntermediateTables())
        goto updatedb_b16_to_b17_error_noset;

    // ***********************************************************************************************///
    // *******************************END OF NEW UPDATE HERE*************************************///
    // ***********************************************************************************************///

    /////////////////////////////////////////
    // Check incremental update
    // and update global_info.incremental_splitlots
    GetIncrementalUpdatesCount(true, lIncrementalSplitlots);
    IncrementProgress();

    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_b16_to_b17_error;

    ResetProgress(true);

    // Success
    lStatus = true;
    InsertIntoUpdateLog(" ");
    lLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(lLogMessage);
    goto updatedb_b16_to_b17_writelog;

updatedb_b16_to_b17_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_b16_to_b17_error_noset:
    lErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    lLogMessage = "Status = ERROR (";
    lLogMessage+= lErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    lLogMessage+= ").";
    InsertIntoUpdateLog(lLogMessage);

updatedb_b16_to_b17_writelog:

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    InsertIntoUpdateLog(" ");
    lLogMessage = "Update history saved to log file ";
    lLogMessage+= m_strUpdateDbLogFile;
    lLogMessage+= ".";
    InsertIntoUpdateLog(lLogMessage);

    return lStatus;
}

