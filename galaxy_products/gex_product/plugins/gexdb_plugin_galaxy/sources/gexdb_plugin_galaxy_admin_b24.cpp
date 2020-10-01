// gexdb_plugin_galaxy_admin_b24.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B23->B24 upgrade
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
bool GexDbPlugin_Galaxy::UpdateDb_B23_to_B24()
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
    strQuery += " DB_VERSION_NAME='"+GetDbName(GEXDB_DB_VERSION_NAME_B24)+"'";
    strQuery += ",DB_VERSION_NB=" + QString::number(GEXDB_DB_VERSION_NB_B24);
    strQuery += ",DB_VERSION_BUILD=" + QString::number(GEXDB_DB_VERSION_BUILD_B24);
    if(!clQuery.Execute(strQuery)) goto updatedb_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO
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


