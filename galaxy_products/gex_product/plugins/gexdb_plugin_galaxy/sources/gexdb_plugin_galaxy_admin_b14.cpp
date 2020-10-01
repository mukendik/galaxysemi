// gexdb_plugin_galaxy_admin_b14.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B13->B14 upgrade
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
// Update DB: B13 -> B14
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B13_to_B14()
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
    SetMaxProgress(4);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Checking indexes for consolidated tables...
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!UpdateDb_UpdateIndexes(GetIndexForConsolidatedTables()))
        goto updatedb_b13_to_b14_error_noset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Drop consolidated triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!DropConsolidationTriggers(eUnknownStage))
    {
        InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
            goto updatedb_b13_to_b14_error_noset;

        // Retrieve values from dialog
        strRootUser = clGexdbGetrootDialog.GetRootUsername();
        strRootPassword = clGexdbGetrootDialog.GetRootPassword();

        if(!DropConsolidationTriggers(eUnknownStage, &strRootUser, &strRootPassword))
            goto updatedb_b13_to_b14_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Re-Create consolidated triggers
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidationTriggers())
    {
        InsertIntoUpdateLog("TRIGGER generation FAILED, trying with root access.");

        // Get root user+password if not already done for the drop
        if(strRootUser.isEmpty())
        {
            GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

            if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
                goto updatedb_b13_to_b14_error_noset;

            // Retrieve values from dialog
            strRootUser = clGexdbGetrootDialog.GetRootUsername();
            strRootPassword = clGexdbGetrootDialog.GetRootPassword();
        }

        if(!CreateConsolidationTriggers(&strRootUser, &strRootPassword))
            goto updatedb_b13_to_b14_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Create xx_custom_incremental_update stored procedure
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "o Create xx_custom_incremental_update stored procedure.";
    InsertIntoUpdateLog(strLogMessage);
    if(!UpdateDb_B13_to_B14_CreateCustomIncrementalUpdateProcedure())
        goto updatedb_b13_to_b14_error_noset;

    IncrementProgress();

    // Make sure progress bar is set to max
    ResetProgress(false);
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
    if(!clQuery.Execute(strQuery)) goto updatedb_b13_to_b14_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b13_to_b14_error;

    strQuery = "INSERT INTO global_info VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B14);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B14);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B14);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b13_to_b14_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(strLogMessage);
    goto updatedb_b13_to_b14_writelog;

updatedb_b13_to_b14_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                clQuery.lastError().text().toLatin1().constData());
updatedb_b13_to_b14_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

updatedb_b13_to_b14_writelog:

    InsertIntoUpdateLog(" ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update DB: B13 -> B14: custom update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B13_to_B14_CreateCustomIncrementalUpdateProcedure()
{
    int					nTestingStage;
    QString				strProcedureName;
    QString				strLogMessage;
    QString				strQuery;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Create xx_custom_incremental_update stored procedure
    ////////////////////////////////////////////////////////////////////////////////////
    // FOR EACH TESTING STAGE DO THE SAME ACTION
    for(nTestingStage=1; nTestingStage<4; nTestingStage++ )
    {
        SetTestingStage(nTestingStage);
        strProcedureName = NormalizeTableName("_custom_incremental_update");

        // Drop procedure first
        clQuery.Execute("DROP PROCEDURE " + strProcedureName);

        // Create procedure
        strLogMessage = "Creating stored procedure " + strProcedureName+"...";
        InsertIntoUpdateLog(strLogMessage);

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 Splitlot			IN	number,\n";
            strQuery += "	 IncrementalKeyword	IN	varchar2,\n";
            strQuery += "	 Message OUT VARCHAR2,\n";
            strQuery += "	 Status OUT NUMBER\n";
            strQuery += "	 )\n";
            strQuery += "IS\n";
            strQuery += "BEGIN\n";
            strQuery += "	Message := 'Empty custom incremental update stored procedure';\n";
            strQuery += "	Status := 0;\n";
            strQuery += "RETURN;\n";
            strQuery += "END;\n";

        }
        else
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 IN Splitlot INT,\n";
            strQuery += "	 IN IncrementalKeyword VARCHAR(1024),\n";
            strQuery += "	 OUT Message VARCHAR(1024),\n";
            strQuery += "	 OUT Status INT)\n";
            strQuery += "BEGIN\n";
            strQuery += "	SELECT 'Empty custom incremental update stored procedure' INTO Message From dual;\n";
            strQuery += "	SELECT 0 INTO Status FROM dual;\n";
            strQuery += "END\n";

        }

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(),
                        clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    return true;
}


///////////////////////////////////////////////////////////
// Update DB: incremental B14
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B14_Incremental(QString & /*strLog*/)
{

    // Incremantally update DB
    QString				strQuery;
    QString				strLogMessage, strErrorMessage;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    bool				bStatus = false;
    unsigned int		uiIncrementalUpdates = 0, uiIncrementalUpdates_B14 = 0;
    QString				strUpdateName;
    QStringList			lstUpdateName;

    int				nTestingStage, nSplitlotId = -1;
    QString			strTableName;
    QStringList		lstSplitLots;

    int	nNbStep, nProgress;

    // Write Start TimeStamp
    QTime clTime;
    clTime.start();
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " => STARTING UPDATE.";
    InsertIntoUpdateLog(strLogMessage);
    InsertIntoUpdateLog(" ");

    QStringList lstReservedKeyword;
    lstReservedKeyword.append("FT_CONSOLIDATE_SOFTBIN");
    lstReservedKeyword.append("BINNING_CONSOLIDATION");

    // If some standard incremental update has to be performed, do it here!
    nProgress = 0;
    nNbStep = 2;
    SetMaxProgress(nNbStep);
    SetProgress(0);
    if(m_pclDatabaseConnector->IsOracleDB() || m_pclDatabaseConnector->IsMySqlDB())
    {

        ////////////////////////////////////////////////////////////////
        // START CUSTOM_INCREMENTAL_UPDATE incremental update
        ////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////
        // CUSTOM_INCREMENTAL_UPDATE incremental update
        // * call xx_custom_incremental_update(splitlot_id,incremental_keyword).
        ////////////////////////////////////////////////////////////////

        nSplitlotId = -1;
        lstSplitLots.clear();
        /////////////////////////////////////////
        // Get the list of incremental keyword in the incremental_update table
        QString		strProcedureName;
        QString		strIncrementalKeyword;
        QStringList lstCustomKeyword;

        strQuery = "SELECT DISTINCT db_update_name FROM incremental_update ";
        strQuery+= " WHERE UPPER(db_update_name) NOT IN ('"+lstReservedKeyword.join("','")+"')";
        strQuery+= " AND remaining_splitlots>0";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            goto updatedb_b14_incremental_error;
        }
        while(clGexDbQuery.Next())
            lstCustomKeyword.append(clGexDbQuery.value(0).toString());

        nNbStep += lstCustomKeyword.count();
        SetMaxProgress(nNbStep);


        // Then find the first SPLITLOT_ID for CUSTOM_INCREMENTAL_UPDATE
        while(!lstCustomKeyword.isEmpty())
        {
            strIncrementalKeyword = lstCustomKeyword.takeFirst();
            SetProgress(++nProgress);


            // FOR ALL TESTINGSTAGES, check if have some consolidation
            for(nTestingStage=0; nTestingStage<3; nTestingStage++)
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
                case 2 :
                    // FOR FINAL TEST
                    SetTestingStage(eFinalTest);
                    break;
                }

                strTableName = NormalizeTableName("_SPLITLOT");
                strQuery = " SELECT SPLITLOT_ID FROM "+strTableName;
                strQuery+= " WHERE ";
                strQuery+= " UPPER(INCREMENTAL_UPDATE) like '%"+strIncrementalKeyword.toUpper()+"%' ";
                strQuery+= " ORDER BY SPLITLOT_ID ";
                if(!clGexDbQuery.Execute(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    goto updatedb_b14_incremental_error;
                }
                if(clGexDbQuery.First())
                {
                    nSplitlotId = clGexDbQuery.value(0).toInt();
                    break;
                }
            }

            if(nSplitlotId > 0)
            {
                QString strMessage;
                int		nStatus;

                /////////////////////////////////////////
                strLogMessage = "************************************************************************* ";
                InsertIntoUpdateLog(strLogMessage);
                strLogMessage = "Incremental update "+strIncrementalKeyword+" : custom incremental update. ";
                InsertIntoUpdateLog(strLogMessage);

                /////////////////////////////////////////
                // Testing stage is updated
                // Have one splitlot for update phase
                // Get the corresponding LOT_ID
                strProcedureName = NormalizeTableName("_CUSTOM_INCREMENTAL_UPDATE");
                if(m_pclDatabaseConnector->IsOracleDB())
                {
                    strMessage = "";

                    strQuery = "CALL " + strProcedureName;
                    strQuery += "(" + QString::number(nSplitlotId) + ",";
                    strQuery += TranslateStringToSqlVarChar(strIncrementalKeyword) + ",";
                    strQuery += "?,?)";
                    for(int i=0 ; i<1024; i++)
                        strMessage += " ";

                    clGexDbQuery.prepare(strQuery);
                    clGexDbQuery.bindValue(0,strMessage,QSql::Out);
                    clGexDbQuery.bindValue(1, 0, QSql::Out);
                    if(!clGexDbQuery.exec())
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                    clGexDbQuery.lastError().text().toLatin1().constData());
                        goto updatedb_b14_incremental_error;
                    }

                    // Retrieve parameter values
                    strMessage = clGexDbQuery.boundValue(0).toString().simplified();		// the returned message
                    nStatus = clGexDbQuery.boundValue(1).toInt();								// nStatus is the return status: 0 is NOK, 1 is OK

                }
                else
                {
                    strQuery = "CALL " + strProcedureName;
                    strQuery += "(" + QString::number(nSplitlotId) + ",";
                    strQuery += TranslateStringToSqlVarChar(strIncrementalKeyword) + ",";
                    strQuery += "@strMessage,@nStatus)";
                    if(!clGexDbQuery.Execute(strQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                    clGexDbQuery.lastError().text().toLatin1().constData());
                        goto updatedb_b14_incremental_error;
                    }
                    strQuery = "select @strMessage, @nStatus";
                    if(!clGexDbQuery.exec(strQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                    clGexDbQuery.lastError().text().toLatin1().constData());
                        goto updatedb_b14_incremental_error;
                    }
                    clGexDbQuery.first();
                    // Retrieve parameter values
                    strMessage = clGexDbQuery.value(0).toString();		// the returned message
                    nStatus = clGexDbQuery.value(1).toInt();			// nStatus is the return status: 0 is NOK, 1 is OK, 2 delay insertion
                }


                if(nStatus == 0)
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strProcedureName.toLatin1().constData(),
                                strMessage.toLatin1().constData());
                    goto updatedb_b14_incremental_error;
                }

                /////////////////////////////////////////
                // Update the incremental_update field
                // Remove the IncrementalKeyword from incremental_update field
                strQuery = "UPDATE "+NormalizeTableName("_splitlot")+" S ";
                strQuery+= " SET incremental_update=REPLACE(UPPER(S.incremental_update),'"+strIncrementalKeyword.toUpper()+"','') ";
                strQuery+= " WHERE ";
                strQuery+= " S.splitlot_id="+QString::number(nSplitlotId);
                clGexDbQuery.Execute(strQuery);

                /////////////////////////////////////////
                strLogMessage = "--> 1 splitlot updated.\n";
                InsertIntoUpdateLog(strLogMessage);

                // unlock table
                clGexDbQuery.Execute("COMMIT"); // the only way for unlock table
            }
        }

        ////////////////////////////////////////////////////////////////
        // END BINNING_CONSOLIDATION incremental update
        ////////////////////////////////////////////////////////////////
        clGexDbQuery.Execute("COMMIT");

    }
    SetProgress(++nProgress);

    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("");
    /////////////////////////////////////////
    // INCREMENTAL_UPDATE TABLE
    /////////////////////////////////////////
    /////////////////////////////////////////
    // Update REMAINING_SPLITLOTS column in INCREMENTAL_UPDATE for current DB_UPDATE_NAME
    /////////////////////////////////////////
    // Update progress
    strLogMessage = "======================================================== ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "Updating REMAINING_SPLITLOTS on ";
    strLogMessage += NormalizeTableName("INCREMENTAL_UPDATE",false);
    strLogMessage += " table.";
    InsertIntoUpdateLog(strLogMessage);

    // For each DB_UPDATE_NAME, collect all splitlots for incremental update
    strQuery = "SELECT UPPER(DB_UPDATE_NAME) FROM incremental_update";
    strQuery+= " WHERE UPPER(db_update_name) NOT IN ('"+lstReservedKeyword.join("','").toUpper()+"')";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto updatedb_b14_incremental_error;
    }
    while(clGexDbQuery.Next())
        lstUpdateName.append(clGexDbQuery.value(0).toString());

    while(!lstUpdateName.isEmpty())
    {
        QCoreApplication::processEvents();

        uiIncrementalUpdates = 0;
        strUpdateName = lstUpdateName.takeFirst();
        for(nTestingStage=0; nTestingStage<3; nTestingStage++)
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
            case 2 :
                // FOR FINAL TEST
                SetTestingStage(eFinalTest);
                break;
            }
            strTableName = NormalizeTableName("_SPLITLOT");
            strQuery  = "SELECT COUNT(*) FROM "+strTableName;
            strQuery += " WHERE UPPER(INCREMENTAL_UPDATE) like '%"+strUpdateName+"%'";
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                goto updatedb_b14_incremental_error;
            }
            clGexDbQuery.First();
            uiIncrementalUpdates += clGexDbQuery.value(0).toUInt();
        }
        strLogMessage = "* Splitlots marked for "+strUpdateName+": "+QString::number(uiIncrementalUpdates);
        InsertIntoUpdateLog(strLogMessage);

        strTableName = NormalizeTableName("INCREMENTAL_UPDATE",false);
        strQuery  = " UPDATE "+strTableName;
        strQuery += " SET REMAINING_SPLITLOTS="+QString::number(uiIncrementalUpdates);
        strQuery += " WHERE UPPER(DB_UPDATE_NAME)='"+strUpdateName+"'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            goto updatedb_b14_incremental_error;
        }
    }
    /////////////////////////////////////////
    // Update progress
    strLogMessage = "======================================================== ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "Updating INCREMENTAL_SPLITLOTS on ";
    strLogMessage += NormalizeTableName("GLOBAL_INFO",false);
    strLogMessage += " table.";
    InsertIntoUpdateLog(strLogMessage);

    // Check if some incremental updates pending
    strQuery = "SELECT SUM(REMAINING_SPLITLOTS) from incremental_update";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto updatedb_b14_incremental_error;
    }
    if(clGexDbQuery.First())
        uiIncrementalUpdates = clGexDbQuery.value(0).toUInt();

    /////////////////////////////////////////
    // GLOBAL_INFO TABLE
    /////////////////////////////////////////
    strQuery = "SELECT SUM(REMAINING_SPLITLOTS) from incremental_update where DB_VERSION_BUILD=14";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto updatedb_b14_incremental_error;
    }
    if(clGexDbQuery.First())
        uiIncrementalUpdates_B14 = clGexDbQuery.value(0).toUInt();

    strQuery = "UPDATE global_info SET INCREMENTAL_SPLITLOTS=" + QString::number(uiIncrementalUpdates);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto updatedb_b14_incremental_error;
    }

    // Sucess
    bStatus = true;
    strLogMessage = "Status = SUCCESS (total of ";
    strLogMessage += QString::number(uiIncrementalUpdates_B14);
    strLogMessage += " still to be updated).";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    ResetProgress(true);

    goto updatedb_b14_incremental_writelog;

updatedb_b14_incremental_error:
    // Write error message
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

updatedb_b14_incremental_writelog:
    // Write Stop TimeStamp
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " => STOPPING UPDATE (";
    strLogMessage += QString::number((double)clTime.elapsed()/60000.0, 'f', 1) + " min).";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    strLogMessage = "Incremental update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

