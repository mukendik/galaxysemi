// gexdb_plugin_galaxy_admin_b23.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B21->B22 upgrade
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
bool GexDbPlugin_Galaxy::UpdateDb_B22_to_B23()
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
    if (!AddDbUpdateSteps(eUpdateDb)) goto updatedb_error_noset;

    // Init progress bar
    ResetProgress(false);
    SetMaxProgress(2);

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update global_info DB_VERSION AND STATUS
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET ";
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B23)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B23);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B23);
    if(!clQuery.Execute(strQuery)) goto updatedb_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MySql Linux configuration
    // Case sensitive Schema name
    // Have to update indexes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        // Check Upper case in Shema name
        if(m_pclDatabaseConnector->m_strSchemaName.toLower()
                != m_pclDatabaseConnector->m_strSchemaName)
        {
            // Check Linux version
            strQuery = "SHOW VARIABLES LIKE 'VERSION_COMPILE_OS'";
            if(!clQuery.Execute(strQuery)) goto updatedb_error;
            if(clQuery.First() && clQuery.value(1).toString().startsWith("Linux",Qt::CaseInsensitive))
            {
                strLogMessage = "o Add a step to update indexes.";
                InsertIntoUpdateLog(strLogMessage);
                if (!AddDbUpdateSteps(eUpdateIndexes)) goto updatedb_error_noset;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    IncrementProgress();

    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateDb)) goto updatedb_error;

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


