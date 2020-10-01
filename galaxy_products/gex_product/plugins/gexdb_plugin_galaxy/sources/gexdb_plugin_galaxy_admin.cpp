// gexdb_plugin_galaxy_admin.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
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
#include "gexdb_plugin_option.h"
#include "import_constants.h"
#include "consolidation_tree.h"
#include "gqtl_datakeys_content.h"
#include "gqtl_datakeys_engine.h"
#include "test_filter.h"
#include <fstream>

// Standard includes
#include <math.h>
#include <sstream>

// Qt includes
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QRegExp>
#include <QDir>
#include <QApplication>
#include <QProgressBar>
#include <QProgressDialog>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTimer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>
#include <gqtl_log.h>
#include "gexdbthreadquery.h"

#include "JobDefinitions.h"


////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// implementation of nested slot functors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy::FunctorSlotForNetworkSignalsBase::FunctorSlotForNetworkSignalsBase(bool &status,
                                                                                       QEventLoop &local_loop ) :
    m_status(status),
    m_local_loop(local_loop) {}

void GexDbPlugin_Galaxy::FunctorSlotForNetworkSignalsBase::terminate_local_loop()
{
    // disconnect all signals from the local loop
    m_local_loop.disconnect();

    // finishing the local loop, resuming processing in caller
    m_local_loop.quit();
}

void GexDbPlugin_Galaxy::FunctorSlotForNetworkSignalsBase::something_went_wrong()
{
    // modify the status, indicating something went wrong
    m_status = false;

    terminate_local_loop();
}

void GexDbPlugin_Galaxy::FunctorSlotForNetworkSignalsBase::everything_is_alright()
{
    // modify the status, indicating everything was alright
    m_status = true;

    terminate_local_loop();
}

GexDbPlugin_Galaxy::ConnectionErrorSlot::ConnectionErrorSlot( bool &status, QEventLoop &local_loop ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ) {}

void GexDbPlugin_Galaxy::ConnectionErrorSlot::operator ()( QTcpSocket::SocketError )
{
    // well if this signal is emitted, something went wrong...
    something_went_wrong();
}

GexDbPlugin_Galaxy::PostErrorSlot::PostErrorSlot( bool &status, QEventLoop &local_loop ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ) {}

void GexDbPlugin_Galaxy::PostErrorSlot::operator ()( QNetworkReply::NetworkError )
{
    // well if this signal is emitted, something went wrong...
    something_went_wrong();
}

GexDbPlugin_Galaxy::PostFinishedSlot::PostFinishedSlot( bool &status, QEventLoop &local_loop , QByteArray &response ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ),
    m_response( response ) {}

void GexDbPlugin_Galaxy::PostFinishedSlot::operator ()( QByteArray response )
{
    m_response.clear();
    m_response = response;

    // OK! Cool stuff is gonna take place!
    everything_is_alright();
}

GexDbPlugin_Galaxy::GetErrorSlot::GetErrorSlot( bool &status, QEventLoop &local_loop ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ) {}

void GexDbPlugin_Galaxy::GetErrorSlot::operator ()( QNetworkReply::NetworkError )
{
    // well if this signal is emitted, something went wrong...
    something_went_wrong();
}

GexDbPlugin_Galaxy::GetFinishedSlot::GetFinishedSlot(bool &status, QByteArray& response, QEventLoop &local_loop ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ), m_response(response) {}

void GexDbPlugin_Galaxy::GetFinishedSlot::operator ()(QByteArray response)
{
    m_response = response;

    // OK! Cool stuff is gonna take place!
    everything_is_alright();
}

GexDbPlugin_Galaxy::ConnectionTimeoutSlot::ConnectionTimeoutSlot( bool &status, QEventLoop &local_loop ) :
    FunctorSlotForNetworkSignalsBase( status, local_loop ) {}

void GexDbPlugin_Galaxy::ConnectionTimeoutSlot::operator ()()
{
    // well if this signal is emitted, something went wrong...
    something_went_wrong();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// end of implementation of nested slot functors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////
// Set tesing stage functions
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::SetTestingStage(const QString & strTestingStage)
{
    // Check testingstage to query
    if(strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        return SetTestingStage(eElectTest);
    if(strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        return SetTestingStage(eWaferTest);
    if(strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        return SetTestingStage(eFinalTest);
    if(strTestingStage == GEXDB_PLUGIN_GALAXY_AZ)
        return true;

    return false;
}

void GexDbPlugin_Galaxy::SetTestingStage()
{
    // If testing stage is already set, just quit
    if(m_eTestingStage != eUnknownStage)
        return;

    if (!mpDbKeysEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Set Testing Stage : mpDbKeysEngine NULL !");
        return;
    }

    // Check if the testing stage is forced by config.gexdbkeys file
    if(!mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().isEmpty())
    {
        if((mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "e")
                && !mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty())
        {
            SetTestingStage(eElectTest);
            return;
        }
        else if (mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "e")
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Cannot force to ETest because WaferId not empty: %1").arg(
                      mpDbKeysEngine->dbKeysContent().Get("Wafer").toString()).toLatin1().data() );

        if((mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "w")
                && !mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty())
        {
            SetTestingStage(eWaferTest);
            return;
        }
        if(mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "f")
        {
            SetTestingStage(eFinalTest);
            return;
        }
    }

    // If we have no wafer records, the testing stage is FINAL TEST
    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WCR]==0))
    {
        SetTestingStage(eFinalTest);
        return;
    }

    // This is a wafer, check if we have the marker specifying the original
    // file is an E-Test format converted by Examinator.
    QString strUserTxt = mpDbKeysEngine->dbKeysContent().Get("UserText").toString();
    if((strUserTxt.startsWith(GEX_IMPORT_DATAORIGIN_LABEL))
            && (strUserTxt.section(":",1,1) == GEX_IMPORT_DATAORIGIN_ETEST))
    {
        SetTestingStage(eElectTest);
        return;
    }

    // Wafer with no E-Test marker: testing stage is WAFER-SORT
    SetTestingStage(eWaferTest);
}

bool GexDbPlugin_Galaxy::SetTestingStage(int eTestingStage)
{
    // Reset testing stage variables
    m_eTestingStage = eUnknownStage;
    m_strTablePrefix = "";
    m_strPrefixTable = "";
    m_strTestingStage = "Unknown";

    // Set testing stage variables
    switch(eTestingStage)
    {
    case eFinalTest:
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        m_strTablePrefix = GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX;
        m_strPrefixTable = "ft";
        m_strTestingStage = GEXDB_PLUGIN_GALAXY_FTEST;
        break;
    case eWaferTest:
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = GEXDB_PLUGIN_GALAXY_WTEST_TABLE_PREFIX;
        m_strPrefixTable = "wt";
        m_strTestingStage = GEXDB_PLUGIN_GALAXY_WTEST;
        break;
    case eElectTest:
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = GEXDB_PLUGIN_GALAXY_ETEST_TABLE_PREFIX;
        m_strPrefixTable = "et";
        m_strTestingStage = GEXDB_PLUGIN_GALAXY_ETEST;
        break;
    default:
        return false;
    }

    m_eTestingStage = eTestingStage;
    return true;
}

///////////////////////////////////////////////////////////
// Set tesing stage functions
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetTestingStageName(int eTestingStage, QString & strTestingStageName)
{
    strTestingStageName="Unknown";
    switch(eTestingStage)
    {
    case eFinalTest:
        strTestingStageName=GEXDB_PLUGIN_GALAXY_FTEST;
        break;
    case eWaferTest:
        strTestingStageName=GEXDB_PLUGIN_GALAXY_WTEST;
        break;
    case eElectTest:
        strTestingStageName=GEXDB_PLUGIN_GALAXY_ETEST;
        break;
    }
}

int GexDbPlugin_Galaxy::GetTestingStageEnum(const QString &strTestingStage)
{
    int eTestingStage = eUnknownStage;

    if(strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        eTestingStage = GexDbPlugin_Galaxy::eElectTest;
    else if(strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        eTestingStage = GexDbPlugin_Galaxy::eWaferTest;
    else if(strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        eTestingStage = GexDbPlugin_Galaxy::eFinalTest;

    return eTestingStage;
}

void GexDbPlugin_Galaxy::GetCurrentTestingStageName(QString & strTestingStageName)
{
    strTestingStageName = m_strTestingStage;
}

///////////////////////////////////////////////////////////
// Add DB type (MySQL, Oracle...) to base DB name
///////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::GetDbName(const QString & strDbName_Base)
{
    if (!m_pclDatabaseConnector)
        return "error : m_pclDatabaseConnector NULL !";

    QString strDbName = strDbName_Base;
    if(strDbName.isEmpty())
    {
        strDbName = m_strDbVersionName.section("V",0,0).simplified() + " V" + QString::number(m_uiDbVersionMajor) + "." + QString::number(100+m_uiDbVersionMinor).right(2) + " B" + QString::number(m_uiDbVersionBuild);
    }
    else
        strDbName = strDbName_Base;

    // GCORE-1151 : Remove Oracle traces from V7.3 package
    //    if(m_pclDatabaseConnector->IsOracleDB())
    //        return(strDbName + " (Oracle)");
    /*else*/ if(m_pclDatabaseConnector->IsMySqlDB())
        return(strDbName + " (MySQL)");
    else if(m_pclDatabaseConnector->IsSQLiteDB())
        return(strDbName + " (SQLite)");

    return strDbName;
}

///////////////////////////////////////////////////////////
// Get the status of the database
///////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::GetDbStatus()
{
    QString strDbStatus;
    if(!m_strDbStatus.isEmpty())
    {
        QString strFirstDbStatus = m_strDbStatus.section("|", 0, 0);
        if(m_mapDbStatusMessage.contains(strFirstDbStatus))
        {
            strDbStatus = "Updating process was aborted";
            foreach(const QString &DbStatus, m_strDbStatus.split("|"))
            {
                if(m_mapDbStatusMessage.contains(DbStatus))
                    strDbStatus+=" - "+m_mapDbStatusMessage[DbStatus];
            }
        }
        else
            strDbStatus = "Database was disabled - "+m_strDbStatus;
    }

    return strDbStatus;
}

///////////////////////////////////////////////////////////
// Check if DB is up-to-date
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsDbUpToDate(bool *pbDbIsUpToDate,
                                      QString & strCurrentDbVersion_Name,
                                      unsigned int *puiCurrentDbVersion_Build,
                                      QString & strLatestSupportedDbVersion_Name,
                                      unsigned int *puiLatestSupportedDbVersion_Build)
{
    if(IsAdr() || IsLocalAdr())
    {
        if (pbDbIsUpToDate)
            *pbDbIsUpToDate = true;
        // Init latest supported version
        strLatestSupportedDbVersion_Name      = GetDbName(GEXDB_DB_VERSION_NAME).replace("TDR","ADR");
        *puiLatestSupportedDbVersion_Build    = GEXDB_DB_VERSION_BUILD;
        strCurrentDbVersion_Name              = m_strDbVersionName;
        *(puiCurrentDbVersion_Build)          = (*puiLatestSupportedDbVersion_Build);

        return true;
    }

    if (pbDbIsUpToDate)
        *pbDbIsUpToDate = false;

    // Init latest supported version
    strLatestSupportedDbVersion_Name      = GetDbName(GEXDB_DB_VERSION_NAME);
    *puiLatestSupportedDbVersion_Build    = GEXDB_DB_VERSION_BUILD;

    bool cnxOk = ConnectToCorporateDb();
    strCurrentDbVersion_Name              = GetDbName();
    *(puiCurrentDbVersion_Build)          = m_uiDbVersionBuild;

    // Check database connection
    if(!cnxOk)
    {
        GSLOG(SYSLOG_SEV_ERROR, "error : unable to connect to DB");
        QString strError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbStatus, NULL, strError.toLatin1().constData());
        return false;
    }

    // Check if DB is up-to-date
    if(*puiCurrentDbVersion_Build == GEXDB_DB_VERSION_BUILD)
        *pbDbIsUpToDate = true;

    m_uiCurrentGexdbBuild = *puiCurrentDbVersion_Build;

    /////////////////////////
    // CHECK IF ALL UPDATES ARE DONE
    // If uptodate or compatible
    if(( ((m_uiDbVersionMajor*100+m_uiDbVersionMinor) == GEXDB_DB_VERSION_NB)   // COMPATIBLE
         || *pbDbIsUpToDate)                                                    // UPTODATE
            && !m_strDbStatus.isEmpty())    // need some pending update
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, GetDbStatus().toLatin1().constData());
        *pbDbIsUpToDate = false;
        return true;
    }

    if(!*pbDbIsUpToDate)
        GSET_ERROR2(GexDbPlugin_Base, eDB_NotUptoDate, NULL, strCurrentDbVersion_Name.toLatin1().constData(), strLatestSupportedDbVersion_Name.toLatin1().constData());

    return true;
}

///////////////////////////////////////////////////////////
// Check if DB is up-to-date
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsDbUpToDateForInsertion(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                  unsigned int *puiCurrentDbVersion_Build,
                                                  QString & strLatestSupportedDbVersion_Name,
                                                  unsigned int *puiLatestSupportedDbVersion_Build)
{
    if(!GexDbPlugin_Galaxy::IsDbUpToDate(pbDbIsUpToDate, strCurrentDbVersion_Name,puiCurrentDbVersion_Build,
                                         strLatestSupportedDbVersion_Name,puiLatestSupportedDbVersion_Build))
    {
        QString strError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strError.toLatin1().constData());
        return false;
    }

    if(IsAdr() || IsLocalAdr())
    {
        // For Adr
        // No consolidationTree
        // No SecuredMode
        // No Architecture(QueryEngine) but Metadata

        /////////////////////////
        // FORCE THE DB STATUS TO NEED UPDATE GEX
        if(m_strDbStatus.isEmpty())
        {
            bool cnxOk = ConnectToCorporateDb();
            if(cnxOk && m_strDbStatus.isEmpty() && m_pclDatabaseConnector)
            {
                GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
                QString             strQuery;

                strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET db_status='INCOMPATIBLE_GEX_VERSION'";
                clQuery.Execute(strQuery);
            }
        }
    }
    else
    {
        /////////////////////////
        // BEGIN FLEXIBLE CONSOLIDATION
        if(*pbDbIsUpToDate && (*puiCurrentDbVersion_Build>=GEXDB_DB_VERSION_BUILD_B16) && (m_pConsolidationTree == NULL))
        {
            QString strError;
            *pbDbIsUpToDate = false;
            strError = "Consolidation Rules not loaded";
            GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, strError.toLatin1().constData());
            return true;
        }


        if (m_pConsolidationTree->isUpToDate() == false)
            m_pConsolidationTree->loadFromDB();

        if(*pbDbIsUpToDate &&
                (*puiCurrentDbVersion_Build>=GEXDB_DB_VERSION_BUILD_B16) &&
                (!m_pConsolidationTree->isValid()))
        {
            QString strError;
            //*pbDbIsUpToDate = false;
            strError = "Consolidation Rules are invalid";
            GSET_ERROR1(GexDbPlugin_Base,
                        eDB_Consolidation,
                        NULL,
                        strError.toLatin1().constData());
            return false;
        }

        // END FLEXIBLE CONSOLIDATION
        /////////////////////////
    }

    // Check build version
    if(*pbDbIsUpToDate)
        return true;

    /////////////////////////
    // CHECK IF ALL UPDATES ARE DONE
    if(!m_strDbStatus.isEmpty())
        return true;

    // Not compatible with futur update
    if(*puiCurrentDbVersion_Build > *puiLatestSupportedDbVersion_Build)
        return true;


    // Check minor version (for insertion)
    int nDbMajorMinorVersion = m_uiDbVersionMajor*100 + m_uiDbVersionMinor;
    int nDbSupportedMajorMinor = GEXDB_DB_VERSION_NB;

    // Check retro compatibility
    // Check with min Major.Minor supported
    if((nDbMajorMinorVersion != nDbSupportedMajorMinor)
            && (nDbMajorMinorVersion >= GEXDB_DB_VERSION_MIN_NB)
            && (nDbMajorMinorVersion <= GEXDB_DB_VERSION_NB))
    {
        *pbDbIsUpToDate = true;
        return true;
    }

    *pbDbIsUpToDate = (nDbMajorMinorVersion == nDbSupportedMajorMinor);

    return true;
}

///////////////////////////////////////////////////////////
// Check if DB is up-to-date
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsDbUpToDateForExtraction(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                   unsigned int *puiCurrentDbVersion_Build,
                                                   QString & strLatestSupportedDbVersion_Name,
                                                   unsigned int *puiLatestSupportedDbVersion_Build)
{

    if(!GexDbPlugin_Galaxy::IsDbUpToDate(pbDbIsUpToDate, strCurrentDbVersion_Name,puiCurrentDbVersion_Build,
                                         strLatestSupportedDbVersion_Name,puiLatestSupportedDbVersion_Build))
    {
        QString strError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strError.toLatin1().constData());
        return false;
    }

    // Check build version
    if(*pbDbIsUpToDate)
        return true;

    /////////////////////////
    // CHECK IF ALL UPDATES ARE DONE
    if(!m_strDbStatus.isEmpty())
        return true;

    // Check major version (for extraction)
    int nDbMajorVersion = m_uiDbVersionMajor;
    int nDbSupportedMajor = QString::number(GEXDB_DB_VERSION_NB/100).toInt();

    // Check retro compatibility
    // Check with min Major.Minor supported
    if((nDbMajorVersion != nDbSupportedMajor)
            && (nDbMajorVersion == QString::number(GEXDB_DB_VERSION_MIN_NB/100).toInt()))
    {
        *pbDbIsUpToDate = true;
        return true;
    }

    *pbDbIsUpToDate = (nDbMajorVersion == nDbSupportedMajor);

    return true;
}

///////////////////////////////////////////////////////////
// Check if DB is compatible for standard extraction
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsCompatibleForStdExtraction(unsigned int uiGexDbBuild, unsigned int uiBuildSupportedByPlugin)
{
    GSLOG(SYSLOG_SEV_DEBUG, (QString(" GexDbBuild=%1 BuildSupportedByPlugin=%2").arg(
                                 uiGexDbBuild).arg( uiBuildSupportedByPlugin)).toLatin1().constData());
    // If same version: OK
    if(uiGexDbBuild == uiBuildSupportedByPlugin)
        return true;

    return false;
}

///////////////////////////////////////////////////////////
// Update DB
///////////////////////////////////////////////////////////
// Empty command: apply UpdateDb from current version to supported version
// 'InnoDB' command: apply UpdateDb_To_InnoDB for conversion
// 'Barracuda' command: apply UpdateDb_To_InnoDBBarracuda for conversion
// 'UpdateIndexes'
// 'UpdateIncremental'
// 'UpdateOnce' command: apply just one update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb(QString command)
{
    if (mGexScriptEngine->property("GS_DAEMON").toBool())
{
        GSLOG(SYSLOG_SEV_WARNING, "Update Database not allowed in daemon mode");
        return false;
    }

    bool          bStatus = true;
    QString       lCurrentDbVersion_Name, lLatestDbVersion_Name, lMessage;
    QString       lCommand = command;
    unsigned int  uiCurrentDbVersion_Build, uiLatestSupportedDbVersion_Build;

    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::UpdateDb: command='%1'").arg(command));


    // Make sure current DB has to be updated
    SetAdminLogin(true);
    bool bDbIsUpToDate;
    if(!IsDbUpToDate(&bDbIsUpToDate, lCurrentDbVersion_Name, &uiCurrentDbVersion_Build, lLatestDbVersion_Name, &uiLatestSupportedDbVersion_Build))
        return false;

    if (!m_pclDatabaseConnector)
        return false;

    if(lCommand=="InnoDB")
    {
        if (!m_pclDatabaseConnector)
            return false;

        if(!m_pclDatabaseConnector->IsMySqlDB())
            return false;

        // MyISAM -> InnoDB
        SetUpdateDbLogFile("gexdb_update_"+m_pclDatabaseConnector->m_strSchemaName+"_toInnoDB_"
                           +QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")+".log");

        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName(GEXDB_DB_VERSION_NAME) + ": -> InnoDB.</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        bStatus = UpdateDb_To_InnoDB();
        if(m_hUpdateDbLogFile.isOpen())
            m_hUpdateDbLogFile.close();
        return bStatus;
    }
    if(lCommand=="Barracuda")
    {
        if (!m_pclDatabaseConnector)
            return false;

        if(!m_pclDatabaseConnector->IsMySqlDB())
            return false;

        // MyISAM -> InnoDB
        SetUpdateDbLogFile("gexdb_update_"+m_pclDatabaseConnector->m_strSchemaName+"_toInnoDBBarracuda_"
                           +QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")+".log");

        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName(GEXDB_DB_VERSION_NAME) + " -> InnoDB Barracuda.</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        bStatus = UpdateDb_To_InnoDB(true);
        if(m_hUpdateDbLogFile.isOpen())
            m_hUpdateDbLogFile.close();
        return bStatus;
    }
    if(lCommand=="UpdateIndexes")
    {
        if (!m_pclDatabaseConnector)
            return false;

        if(m_strUpdateDbLogFile.isEmpty())
        {
            // Update Indexes
            SetUpdateDbLogFile("gexdb_update_"+m_pclDatabaseConnector->m_strSchemaName+"_UpdateIndexes_"
                               +QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")+".log");

            InsertIntoUpdateLog(" ");
            lMessage = "<b><u>" + GetDbName(GEXDB_DB_VERSION_NAME) + ": Update Indexes.</u></b>";
            InsertIntoUpdateLog(lMessage);
            InsertIntoUpdateLog(" ");
        }

        bStatus = UpdateDb_UpdateIndexes();
        if(m_hUpdateDbLogFile.isOpen())
            m_hUpdateDbLogFile.close();
        return bStatus;
    }

    if(bDbIsUpToDate)
    {
        // Nothing to do
        return true;
    }

    unsigned int uiUpdateFlags;
    GetDbUpdateSteps(uiUpdateFlags);

    m_mapTablesDesc.clear();

    if(uiUpdateFlags & eUpdateUnknown)
    {
        lMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        InsertIntoUpdateLog("<b>o Invalid Database Status:</b> ");
        InsertIntoUpdateLog(" - "+lMessage.section(":",1));
        InsertIntoUpdateLog("");
        InsertIntoUpdateLog("The current Database Status is invalid.");
        InsertIntoUpdateLog("Your Galaxy Application need to be updated.");
        InsertIntoUpdateLog("");
        InsertIntoUpdateLog("Update CANCELLED");
        return false;
    }

    // if an error has occurred during last DB re-apply the last update
    // i.e. current version = B17 + UPDATING_DATABASE re-aplly B15 to B17
    if ((uiCurrentDbVersion_Build > 15) && (uiUpdateFlags & eUpdateDb))
        uiCurrentDbVersion_Build--;

    if(m_strUpdateDbLogFile.isEmpty())
    {
        QString LogFile = "galaxy_update_"+m_pclDatabaseConnector->m_strSchemaName+"_b"+QString::number(uiCurrentDbVersion_Build);

        if(uiCurrentDbVersion_Build != GEXDB_DB_VERSION_BUILD)
        {
            if(lCommand == "UpdateOnce")
                LogFile+= "_to_b"+QString::number(uiCurrentDbVersion_Build+1);
            else
                LogFile+= "_to_b"+QString::number(GEXDB_DB_VERSION_BUILD);
        }

        LogFile+="_"+QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")+".log";
        SetUpdateDbLogFile(LogFile);
    }

    // Incremantally update DB
    switch(uiCurrentDbVersion_Build)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        bStatus = false; // Not supported
        break;

    case 11:
        // GEXDB B11 -> GEXDB B12
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B12);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B11_to_B12();
        break;

    case 12:
        // GEXDB B12 -> GEXDB B13
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B13);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B12_to_B13();
        break;

    case 13:
        // GEXDB B13 -> GEXDB B14
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B14);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B13_to_B14();
        break;
    case 14:
        // GEXDB B14 -> GEXDB B15
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B15);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B14_to_B15();
        break;

    case 15:
        // UPDATE WAS CANCELED
        // DATABASE IS IN LAST VERSION BUILD
        // BUT STATUS IS INVALID
        // RESTART THE UPDATE FROM THE PREVIOUS VERSION
    case 16:
        // GEXDB B15 OR B16 OR B17[INSTABLE] -> GEXDB B17
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B17);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B16_to_B17();
        break;

    case 17:
        // GEXDB B17 -> GEXDB B18
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B18);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B17_to_B18();
        break;
    case 18:
        // GEXDB B18 -> GEXDB B19
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B19);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B18_to_B19();
        break;
    case 19:
        // GEXDB B19 -> GEXDB B20
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B20);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B19_to_B20();
        break;
    case 20:
        // GEXDB B20 -> GEXDB B21
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B21);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B20_to_B21();
        break;
    case 21:
        // GEXDB B21 -> GEXDB B22
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B22);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B21_to_B22();
        break;
    case 22:
        // GEXDB B22 -> GEXDB B23
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B23);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B22_to_B23();
        break;
    case 23:
        // GEXDB B23 -> GEXDB B24
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B24);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B23_to_B24();
        break;
    case 24:
        // GEXDB B24 -> GEXDB B25
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>" + GetDbName() + " -> ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME_B25);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        // Performing update
        bStatus = UpdateDb_B24_to_B25();
        break;
    default:
        if(uiCurrentDbVersion_Build < GEXDB_DB_VERSION_BUILD)
        {
            bStatus = UpdateDb_FromSqlScript(uiCurrentDbVersion_Build,GEXDB_DB_VERSION_BUILD, *m_pclDatabaseConnector);
            GetDbUpdateSteps(uiUpdateFlags);
        }
        // If consolidation tree has to be updated
        ConnectToCorporateDb();
        InsertIntoUpdateLog(" ");
        lMessage = "<b><u>Update ";
        lMessage += GetDbName(GEXDB_DB_VERSION_NAME);
        lMessage += "</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        if (bStatus
                &&
                (uiUpdateFlags & eUpdateConsTree))
            bStatus = InsertDefaultConsolidationTree();
        // If consolidation process is required
        if (bStatus
                &&
                ((uiUpdateFlags & eUpdateConsTriggers)||(uiUpdateFlags & eUpdateConsTables)||(uiUpdateFlags & eUpdateConsProcedures)))
            bStatus = UpdateConsolidationProcess(eUnknownStage);

        // Then update indexes is required after the final update
        if (bStatus)
            bStatus = UpdateDb_UpdateIndexes();

        InsertIntoUpdateLog(" ");
        lMessage = "Update history saved to log file ";
        lMessage+= m_strUpdateDbLogFile;
        lMessage+= ".";
        InsertIntoUpdateLog(lMessage);
        break;
    }

    if(m_hUpdateDbLogFile.isOpen())
        m_hUpdateDbLogFile.close();
    //m_strUpdateDbLogFile = "";

    if(!bStatus)
        return false;

    if(lCommand == "UpdateOnce")
    {
        if((uiUpdateFlags & eUpdateConsTree)
                || (uiUpdateFlags & eUpdateConsTriggers)
                || (uiUpdateFlags & eUpdateConsTables)
                || (uiUpdateFlags & eUpdateConsProcedures)
                || (uiUpdateFlags & eUpdateIndexes))
        {

            // Just do one update and finalize with consolidation/indexes
            // If consolidation tree has to be updated
            ConnectToCorporateDb();
            InsertIntoUpdateLog(" ");
            lMessage = "<b><u>Update ";
            lMessage += GetDbName();
            lMessage += "</u></b>";
            InsertIntoUpdateLog(lMessage);
            InsertIntoUpdateLog(" ");

            if (bStatus
                    &&
                    (uiUpdateFlags & eUpdateConsTree))
                bStatus = InsertDefaultConsolidationTree();
            // If consolidation process is required
            if (bStatus
                    &&
                    ((uiUpdateFlags & eUpdateConsTriggers)||(uiUpdateFlags & eUpdateConsTables)||(uiUpdateFlags & eUpdateConsProcedures)))
                bStatus = UpdateConsolidationProcess(eUnknownStage);
            // If update indexes is required
            if (bStatus
                    &&
                    (uiUpdateFlags & eUpdateIndexes))
                bStatus = UpdateDb_UpdateIndexes();

            InsertIntoUpdateLog(" ");
            lMessage = "Update history saved to log file ";
            lMessage+= m_strUpdateDbLogFile;
            lMessage+= ".";
            InsertIntoUpdateLog(lMessage);
        }
        m_strUpdateDbLogFile = "";
        return bStatus;
    }

    return UpdateDb();
}

///////////////////////////////////////////////////////////
// Check if a specific SID is active
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsSessionIdActive(QString &SessionId)
{
    // If we are not able to know if the Id is active
    // We must return true
    QString lConnection;
    QString lHostName = m_strInsertionHostName;
    QString lSessionId = SessionId.section(":",0,0);
    QString lOtherHostName = SessionId.section(":",1);

    lConnection = m_pclDatabaseConnector->m_strConnectionName;
    if(lHostName != lOtherHostName)
    {
        // Connect to this IP and check
        lConnection = m_pclDatabaseConnector->m_strConnectionName;
        lConnection+= " on ";
        lConnection+= lOtherHostName;

        // Add database link: make sure we use a unique name for the database connection
        // On some MySql server with AUTO connection configuration
        // The addDatabase can directly open a default connection (see at ASE, not reproducible ?)
        QSqlDatabase clSqlDatabase;
        if(!QSqlDatabase::contains(lConnection))
        {
            clSqlDatabase = QSqlDatabase::addDatabase(m_pclDatabaseConnector->m_strDriver, lConnection);
            // Force to close the connection if AUTO connection
            if(clSqlDatabase.isOpen())
                clSqlDatabase.close();
            clSqlDatabase.setHostName(lOtherHostName);
            clSqlDatabase.setDatabaseName(m_pclDatabaseConnector->m_strDatabaseName);
            clSqlDatabase.setUserName(m_pclDatabaseConnector->m_strUserName_Admin);
            clSqlDatabase.setPassword(m_pclDatabaseConnector->m_strPassword_Admin);
            clSqlDatabase.setPort(m_pclDatabaseConnector->m_uiPort);
            GSLOG(SYSLOG_SEV_WARNING, QString("Create a new connection to check EXPIRED session_id on %1 for %2: db[%3]user[%4]port[%5]")
                  .arg(lHostName).arg(lOtherHostName)
                  .arg(m_pclDatabaseConnector->m_strDatabaseName)
                  .arg(m_pclDatabaseConnector->m_strUserName_Admin)
                  .arg(m_pclDatabaseConnector->m_uiPort).toLatin1().constData());
        }
        else
            clSqlDatabase = QSqlDatabase::database(lConnection);
        if(!clSqlDatabase.isOpen())
        {
            clSqlDatabase.open();
            GSLOG(SYSLOG_SEV_WARNING, QString("Open the connection to check EXPIRED session_id on %1 for %2: db[%3]user[%4]port[%5]")
                  .arg(lHostName).arg(lOtherHostName)
                  .arg(m_pclDatabaseConnector->m_strDatabaseName)
                  .arg(m_pclDatabaseConnector->m_strUserName_Admin)
                  .arg(m_pclDatabaseConnector->m_uiPort).toLatin1().constData());
        }
    }
    QString    lQuery;
    QSqlQuery  lSessionQuery(QSqlDatabase::database(lConnection));
    lQuery = "SELECT ID FROM information_schema.PROCESSLIST WHERE ID="+lSessionId;
    if(!lSessionQuery.exec(lQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg( lSessionQuery.lastError().text()).toLatin1().constData());
        // Clean the connection if unable to open it
        QSqlDatabase::removeDatabase(lConnection);
        return true;
    }
    if(!lSessionQuery.first())
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("EXPIRED session_id %1 for %2")
              .arg(lSessionId).arg(lOtherHostName).toLatin1().constData());
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Get incremental updates
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetIncrementalUpdatesCount(bool checkDatabase, int &incrementalSplitlots)
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;

    incrementalSplitlots = 0;

    // Get incremental updates
    QString   lQuery;
    QString   lString;
    QSqlQuery clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    lQuery = "SELECT INCREMENTAL_SPLITLOTS FROM "+NormalizeTableName("global_info",false);
    if(!clQuery.exec(lQuery) || !clQuery.first())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    incrementalSplitlots = clQuery.value(0).toInt();

    if((m_uiDbVersionBuild>=GEXDB_DB_VERSION_BUILD_B37)
            && checkDatabase)
    {
        // if not up to date
        // reject
        QMap<QString,int> mapUpdateNumber;

        int nTestingStage;
        int iIncrementalUpdates = 0;
        QString lTableName;
        QString lUpdateName;
        QString lMessage;
        QStringList lstValues;

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
            lTableName = NormalizeTableName("_SPLITLOT");
            // GCORE-4820
            // with WHERE clause and a count(*) = 12s
            // without  < 1s
            // The goal is to split this query into 2
            // One for the list of the values
            // One for the count(*)
            // By removing the WHERE, we can have some incorrect result
            // if the TDR is not clean (valid_splitlot='N' + BINNING_CONSOLIDATION)
            // Need to clean the TDR regularely
            // Get all distinct value
            lQuery = "SELECT DISTINCT INCREMENTAL_UPDATE FROM "+lTableName;
            if(!clQuery.exec(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            // For each distinct value, check the count
            while(clQuery.next())
                if(!clQuery.value(0).toString().remove("|").remove(" ").isEmpty())
                    lstValues.append(clQuery.value(0).toString());
            while(!lstValues.isEmpty())
            {
                // with the WHERE clause = 2s
                // without < 1s
                lQuery = "SELECT INCREMENTAL_UPDATE, count(*) FROM "+lTableName;
                lQuery+= " WHERE INCREMENTAL_UPDATE='"+lstValues.takeFirst()+"'";
                if(!clQuery.exec(lQuery) || !clQuery.first())
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }
                lString = clQuery.value(0).toString()+"|";
                iIncrementalUpdates = clQuery.value(1).toInt();
                while(lString.contains("|"))
                {
                    lUpdateName = lString.section("|",0,0).simplified();
                    lString = lString.section("|",1);
                    if(lUpdateName.isEmpty())
                        continue;
                    if(!mapUpdateNumber.contains(lUpdateName))
                        mapUpdateNumber[lUpdateName] = 0;
                    mapUpdateNumber[lUpdateName] += iIncrementalUpdates;
                }
            }
        }

        lTableName = NormalizeTableName("INCREMENTAL_UPDATE",false);
        lQuery  = " SELECT DB_UPDATE_NAME FROM "+lTableName;
        if(!clQuery.exec(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clQuery.next())
        {
            lUpdateName = clQuery.value(0).toString();
            if(!mapUpdateNumber.contains(lUpdateName))
                mapUpdateNumber[lUpdateName] = 0;
        }

        // This lines will automatically insert OUR INCREMENTAL KEY into the tdr.incremental_update table
        // Default value for status is DISABLED
        // Check if BINNING_CONSOLIDATION, AGENT_WORKFLOW and FT_CONSOLIDATE_SOFTBIN exists
        foreach(QString lKey, mIncrementalUpdateName.keys())
        {
            if(!mapUpdateNumber.contains(lKey))
                mapUpdateNumber[lKey] = 0;
        }

        while(!mapUpdateNumber.isEmpty())
        {
            lUpdateName = mapUpdateNumber.begin().key();
            iIncrementalUpdates = mapUpdateNumber.take(lUpdateName);

            lQuery  = " SELECT DB_UPDATE_NAME FROM "+lTableName;
            lQuery += " WHERE DB_UPDATE_NAME='"+lUpdateName+"'";
            if(!clQuery.exec(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(!clQuery.first())
            {
                // For OUR INCREMENTAL KEY BINNING_CONSOLIDATION, BINNING_CONSOLIDATION_DELAY, AGENT_WOKFLOW
                // default status is ENABLED
                if(
                    (lUpdateName == "BINNING_CONSOLIDATION")
                    || (lUpdateName == "BINNING_CONSOLIDATION_DELAY")
                    || (lUpdateName == "AGENT_WORKFLOW")
                )
                {
                    lQuery  = " INSERT INTO "+lTableName;
                    lQuery += " (db_update_name,remaining_splitlots,status) ";
                    lQuery += " VALUES('"+lUpdateName+"',"+QString::number(iIncrementalUpdates)+",'ENABLED')";
                }
                else
                {
                    lQuery  = " INSERT INTO "+lTableName;
                    lQuery += " (db_update_name,remaining_splitlots) ";
                    lQuery += " VALUES('"+lUpdateName+"',"+QString::number(iIncrementalUpdates)+")";
                }
                if(!clQuery.exec(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }
            }
            else
            {
                lQuery  = " UPDATE "+lTableName;
                lQuery += " SET REMAINING_SPLITLOTS="+QString::number(iIncrementalUpdates);
                lQuery += " WHERE DB_UPDATE_NAME='"+lUpdateName+"'";
                if(!clQuery.exec(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }
            }
        }

        clQuery.exec("COMMIT");

        /////////////////////////////////////////
        // GLOBAL_INFO TABLE
        /////////////////////////////////////////
        // Check if some incremental updates pending
        iIncrementalUpdates = 0;
        lQuery = "SELECT SUM(REMAINING_SPLITLOTS) FROM "+lTableName;
        lQuery+= " WHERE status='ENABLED'";
        if(!clQuery.exec(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if(clQuery.first())
            iIncrementalUpdates = clQuery.value(0).toInt();

        lQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET INCREMENTAL_SPLITLOTS=" + QString::number(iIncrementalUpdates);
        if(!clQuery.exec(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        clQuery.exec("COMMIT");

        InsertIntoUpdateLog(" ");
        lMessage = "o Check Incremental updates";
        InsertIntoUpdateLog(lMessage);

        if(incrementalSplitlots == iIncrementalUpdates)
            InsertIntoUpdateLog(" No new incremental updates found.");

        lMessage = " Total of ";
        lMessage += QString::number(iIncrementalUpdates);
        lMessage += " splitlots to be updated</u></b>";
        InsertIntoUpdateLog(lMessage);
        InsertIntoUpdateLog(" ");

        incrementalSplitlots = iIncrementalUpdates;
    }
    lQuery = "SELECT * ";
    lQuery+= " FROM "+NormalizeTableName("incremental_update",false);
    lQuery+= " WHERE status='ENABLED'";
    m_bEnableAutomaticIncrementalUpdates = (clQuery.exec(lQuery) && clQuery.first());

    return true;
}

///////////////////////////////////////////////////////////
// Get incremental updates of incremental To Do according to the settings
// Map of DB_UPDATE_NAME
//      Map per TestingStage
//          QStringList per target (can be LOT_ID or SPLITLOT_ID)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & IncrementalUpdatesList)
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;

    // Update and check if there is some incremental update to do
    int lCount;
    // Update the new count of remaining_splitlots and if have enabled update now
    GetIncrementalUpdatesCount(true,lCount);
    if(lCount == 0)
        return true;

    // First get the list of all incremental update settings
    QMap< QString,QMap< QString,QString > > lIncrementalUpdates;
    if(!GetIncrementalUpdatesSettings(lIncrementalUpdates))
        return false;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;

    QString lFrequency;
    QString lValue;
    QDateTime lLastExecution;
    QDateTime lNextExecution;
    int lMin = -1;
    int lHour = -1;
    int lDay = -1;
    int lMonth = -1;
    int lDow = -1;
    // For each update, check the DB
    foreach(const QString &lKey, lIncrementalUpdates.keys())
    {
        // Update default value
        if(lIncrementalUpdates[lKey]["status"].isEmpty())
            lIncrementalUpdates[lKey]["status"] = lIncrementalUpdates[lKey]["status_default"];
        if(lIncrementalUpdates[lKey]["frequency"].isEmpty())
            lIncrementalUpdates[lKey]["frequency"] = lIncrementalUpdates[lKey]["frequency_default"];
        if(lIncrementalUpdates[lKey]["max_items"].isEmpty())
            lIncrementalUpdates[lKey]["max_items"] = lIncrementalUpdates[lKey]["max_items_default"];

        // First check if it is ENABLED
        if(lIncrementalUpdates[lKey]["status"] != "ENABLED")
            continue;

        // And check if there is some remaining stuff
        // If no remaining splitlots, do not execute the followwed queries
        // that can be time consumming for Step 2
        if(lIncrementalUpdates[lKey]["remaining_splitlots"].toInt() == 0)
            continue;

        // Check the frequency and decide according to the last_schedule
        if(lIncrementalUpdates[lKey]["last_schedule"].isEmpty())
            lLastExecution = QDateTime::currentDateTime().addYears(-10);
        else
            lLastExecution = QDateTime::fromString(lIncrementalUpdates[lKey]["last_schedule"]);
        lNextExecution = lLastExecution;
        lFrequency = lIncrementalUpdates[lKey]["frequency"];
        // min
        lValue = lFrequency.section(" ",0,0);
        if(lValue != "*")
        {
            if(lValue.startsWith("*/"))
                // every
                lNextExecution = lLastExecution.addSecs(lValue.remove("*/").toInt()*60);
            else
                lMin = lValue.toInt();
            if(lMin>=0
                    && QDateTime::currentDateTime().time().minute()!=lMin)
                continue;
        }
        // hour
        lValue = lFrequency.section(" ",1,1);
        if(lValue != "*")
        {
            if(lValue.startsWith("*/"))
                // every
                lNextExecution = lLastExecution.addSecs(lValue.remove("*/").toInt()*60*60);
            else
                lHour = lValue.toInt();
            if(lHour>=0
                    && QDateTime::currentDateTime().time().hour()!=lHour)
                continue;
        }
        // day
        lValue = lFrequency.section(" ",1,1);
        if(lValue != "*")
        {
            if(lValue.startsWith("*/"))
                // every
                lNextExecution = lLastExecution.addSecs(lValue.remove("*/").toInt()*60*60*24);
            else
                lDay = lValue.toInt();
            if(lDay>=0
                    && QDateTime::currentDateTime().date().day()!=lDay)
                continue;
        }
        // month
        lValue = lFrequency.section(" ",1,1);
        if(lValue != "*")
        {
            if(lValue.startsWith("*/"))
                // every
                lNextExecution = lLastExecution.addDays((int) 30/lValue.remove("*/").toInt());
            else
                lMonth = lValue.toInt();
            if(lMonth>=0
                    && QDateTime::currentDateTime().date().month()!=lMonth)
                continue;
        }
        // dow
        lValue = lFrequency.section(" ",1,1);
        if(lValue != "*")
        {
            if(!lValue.startsWith("*/"))
                lDow = lValue.toInt();
            if(lDow>=0
                    && QDateTime::currentDateTime().date().dayOfWeek()!=lDow)
                continue;
        }
        if(lNextExecution>=QDateTime::currentDateTime())
            continue;

        // This incremental update can be run now
        // Check the TDR to retrieve per TestingStage the list of LOT/SPLITLOT to do
        // FOR WaferSort AND FinalTest TESTINGSTAGES, check if have some update
        // Step 1: Check if the INCREMENTAL_UPDATE starts with the key
        // This is mandatory to use the INDEX and optimize the execution
        for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
        {
            switch(nTestingStage)
            {
            case 0 :
                // FOR WAFER TEST
                SetTestingStage(eWaferTest);
                break;
            case 1 :
                // FOR FINAL TEST
                SetTestingStage(eFinalTest);
                break;
            case 2 :
                // FOR E TEST
                SetTestingStage(eElectTest);
                break;
            }

            // Check if this incremental update can be used on this TestingStage
            if(!lIncrementalUpdates[lKey]["testing_stage"].contains(m_strTestingStage,Qt::CaseInsensitive))
                continue;

            int lMaxItems = lIncrementalUpdates[lKey]["max_items"].toInt();
            int lNbItems = 0;
            if(IncrementalUpdatesList.contains(lKey)
                    && IncrementalUpdatesList[lKey].contains(m_strTestingStage))
                lNbItems = IncrementalUpdatesList[lKey][m_strTestingStage].count();
            if((lMaxItems >= 0)
                    && (lNbItems >= lMaxItems))
                continue;

            QString lTableName = NormalizeTableName("_SPLITLOT");
            QString lClauseWhere;

            if(mBackgroundTransferActivated
                    && (mBackgroundTransferInProgress > 0))
            {
                // We need to ignore splitlots not already TRANSFERRED
                // First check if have a BACKGROUND TRANSFER IN PROGRESS
                BackgroundTransferExcludeForIncremental(m_strPrefixTable,lIncrementalUpdates[lKey]["target"],lClauseWhere);
            }

            //lQuery = " SELECT "+lIncrementalUpdates[lKey]["target"]+", MAX(insertion_time) AS last_insertion FROM "+lTableName;
            //lQuery+= " WHERE ";
            //lQuery+= " INCREMENTAL_UPDATE like '"+lKey+"%' ";
            //lQuery+= " AND VALID_SPLITLOT<>'N'";
            //lQuery+= " GROUP BY "+lIncrementalUpdates[lKey]["target"];
            //lQuery+= " ORDER BY last_insertion ";
            // It is not possible to use this query that can take more than 10s
            // Do not agregate or select distinct
            // Most of the time, this will be one of the oldest
            // Can be aleatory
            lQuery = " SELECT "+lIncrementalUpdates[lKey]["target"]+" FROM "+lTableName;
            lQuery+= " WHERE ";
            lQuery+= " (INCREMENTAL_UPDATE = '"+lKey+"' "; // Match with the key
            lQuery+= " OR INCREMENTAL_UPDATE like '"+lKey+"|%') "; // OR Starts with the key
            lQuery+= " AND VALID_SPLITLOT<>'N'";
            lQuery+= lClauseWhere;
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            while(clGexDbQuery.Next())
            {
                if((lMaxItems >= 0)
                        && (lNbItems >= lMaxItems))
                        break;
                if(IncrementalUpdatesList[lKey][m_strTestingStage].contains(clGexDbQuery.value(0).toString()))
                    continue;

                IncrementalUpdatesList[lKey][m_strTestingStage] << (clGexDbQuery.value(0).toString());
                // Count the number of items targetted
                ++lNbItems;
            }
        }

        // Step 2: If the Step 1 doesn't return a result
        // Check if the INCREMENTAL_UPDATE contains the key
        // This will not use the INDEX
        for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
        {
            switch(nTestingStage)
            {
            case 0 :
                // FOR WAFER TEST
                SetTestingStage(eWaferTest);
                break;
            case 1 :
                // FOR FINAL TEST
                SetTestingStage(eFinalTest);
                break;
            case 2 :
                // FOR E TEST
                SetTestingStage(eElectTest);
                break;
            }

            // Check if this incremental update can be used on this TestingStage
            if(!lIncrementalUpdates[lKey]["testing_stage"].contains(m_strTestingStage,Qt::CaseInsensitive))
                continue;

            int lMaxItems = lIncrementalUpdates[lKey]["max_items"].toInt();
            int lNbItems = 0;
            if(IncrementalUpdatesList.contains(lKey)
                    && IncrementalUpdatesList[lKey].contains(m_strTestingStage))
                lNbItems = IncrementalUpdatesList[lKey][m_strTestingStage].count();
            if((lMaxItems >= 0)
                    && (lNbItems >= lMaxItems))
                continue;

            QString lTableName = NormalizeTableName("_SPLITLOT");
            // It is not possible to use this query that can take more than 10s
            // Do not agregate or select distinct
            // Most of the time, this will be one of the oldest
            // Can be aleatory
            lQuery = " SELECT "+lIncrementalUpdates[lKey]["target"]+" FROM "+lTableName;
            lQuery+= " WHERE ";
            lQuery+= " (INCREMENTAL_UPDATE like '%|"+lKey+"|%' ";   // Contains the key is between a |
            lQuery+= " OR INCREMENTAL_UPDATE like '%|"+lKey+"') ";  // OR Ends with the key is after a |
            lQuery+= " AND VALID_SPLITLOT<>'N'";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            while(clGexDbQuery.Next())
            {
                if((lMaxItems >= 0)
                        && (lNbItems >= lMaxItems))
                    break;
                if(IncrementalUpdatesList[lKey][m_strTestingStage].contains(clGexDbQuery.value(0).toString()))
                    continue;

                IncrementalUpdatesList[lKey][m_strTestingStage] << (clGexDbQuery.value(0).toString());
                // Count the number of items targetted
                ++lNbItems;
            }
        }

    }

    // Check if found something to do
    if(IncrementalUpdatesList.isEmpty())
    {
        if(lCount > 0)
        {
            QString lMsg = QString("There are no valid incremental updates");
            GSLOG(SYSLOG_SEV_ALERT, lMsg.toLatin1().data());
            InsertIntoUpdateLog(lMsg);
            for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
            {
                switch(nTestingStage)
                {
                case 0 :
                    // FOR WAFER TEST
                    SetTestingStage(eWaferTest);
                    break;
                case 1 :
                    // FOR FINAL TEST
                    SetTestingStage(eFinalTest);
                    break;
                case 2 :
                    // FOR E TEST
                    SetTestingStage(eElectTest);
                    break;
                }

                QString lTableName = NormalizeTableName("_SPLITLOT");
                lQuery = " UPDATE "+lTableName;
                lQuery+= " SET INCREMENTAL_UPDATE=null";
                lQuery+= " WHERE VALID_SPLITLOT='N'";
                clGexDbQuery.Execute(lQuery);
            }
        }
        clGexDbQuery.Execute("COMMIT");
        return true;
    }

    // For each db_update_name, update the 'last_schedule'
    lQuery = "UPDATE incremental_update SET last_schedule=now() ";
    lQuery+= " WHERE db_update_name IN ('"+QStringList(IncrementalUpdatesList.keys()).join("','")+"')";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return true;
    }
    clGexDbQuery.Execute("COMMIT");
    return true;
}

///////////////////////////////////////////////////////////
// Get incremental updates of incremental To Do for manual update
// Map of DB_UPDATE_NAME
//      Map per TestingStage
//          QStringList per target (can be LOT_ID or SPLITLOT_ID)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetFirstIncrementalUpdatesList(QString incrementalName, QMap< QString,QMap< QString,QStringList > > & IncrementalUpdatesList)
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;

    // Update and check if there is some incremental update to do
    int lCount;
    // Update the new count of remaining_splitlots and if have enabled update now
    GetIncrementalUpdatesCount(true,lCount);
    if(lCount == 0)
        return true;

    // First get the list of all incremental update settings
    QMap< QString,QMap< QString,QString > > lIncrementalUpdates;
    if(!GetIncrementalUpdatesSettings(lIncrementalUpdates))
        return false;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;

    // For each update, check the DB
    foreach(const QString &lKey, lIncrementalUpdates.keys())
    {
        if(!incrementalName.isEmpty()
                && incrementalName!=lKey)
            continue;

        // Update default value
        if(lIncrementalUpdates[lKey]["status"].isEmpty())
            lIncrementalUpdates[lKey]["status"] = lIncrementalUpdates[lKey]["status_default"];
        if(lIncrementalUpdates[lKey]["frequency"].isEmpty())
            lIncrementalUpdates[lKey]["frequency"] = lIncrementalUpdates[lKey]["frequency_default"];
        if(lIncrementalUpdates[lKey]["max_items"].isEmpty())
            lIncrementalUpdates[lKey]["max_items"] = lIncrementalUpdates[lKey]["max_items_default"];

        // First check if it is ENABLED
        if(lIncrementalUpdates[lKey]["status"] != "ENABLED")
            continue;

        // And check if there is some remaining stuff
        // If no remaining splitlots, do not execute the followwed queries
        // that can be time consumming for Step 2
        if(lIncrementalUpdates[lKey]["remaining_splitlots"].toInt() == 0)
            continue;

        // Check the TDR to retrieve per TestingStage the list of LOT/SPLITLOT to do
        // FOR WaferSort AND FinalTest TESTINGSTAGES, check if have some update
        // Step 1: Check if the INCREMENTAL_UPDATE starts with the key
        // This is mandatory to use the INDEX and optimize the execution
        for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
        {
            switch(nTestingStage)
            {
            case 0 :
                // FOR WAFER TEST
                SetTestingStage(eWaferTest);
                break;
            case 1 :
                // FOR FINAL TEST
                SetTestingStage(eFinalTest);
                break;
            case 2 :
                // FOR E TEST
                SetTestingStage(eElectTest);
                break;
            }

            // Check if this incremental update can be used on this TestingStage
            if(!lIncrementalUpdates[lKey]["testing_stage"].contains(m_strTestingStage,Qt::CaseInsensitive))
                continue;

            QString lTableName = NormalizeTableName("_SPLITLOT");
            QString lClauseWhere;

            if(mBackgroundTransferActivated
                    && (mBackgroundTransferInProgress > 0))
            {
                // We need to ignore splitlots not already TRANSFERRED
                // First check if have a BACKGROUND TRANSFER IN PROGRESS
                BackgroundTransferExcludeForIncremental(m_strPrefixTable,lIncrementalUpdates[lKey]["target"],lClauseWhere);
            }

            // Extract the oldest target for update
            // It is not possible to use this query that can take more than 10s
            // Just take the first
            // Most of the time, this will be one of the oldest
            // Can be aleatory
            lQuery = " SELECT "+lIncrementalUpdates[lKey]["target"]+" FROM "+lTableName;
            lQuery+= " WHERE ";
            lQuery+= " (INCREMENTAL_UPDATE = '"+lKey+"' "; // Match with the key
            lQuery+= " OR INCREMENTAL_UPDATE like '"+lKey+"|%') "; // Starts with the key
            lQuery+= " AND VALID_SPLITLOT<>'N'";
            lQuery+= lClauseWhere;
            lQuery+= " LIMIT 1";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            // Take the first;
            if(clGexDbQuery.Next())
                IncrementalUpdatesList[lKey][m_strTestingStage] << (clGexDbQuery.value(0).toString());
        }

        // Step 2: If the Step 1 doesn't return a result
        // Check if the INCREMENTAL_UPDATE contains the key
        // This will not use the INDEX
        for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
        {
            switch(nTestingStage)
            {
            case 0 :
                // FOR WAFER TEST
                SetTestingStage(eWaferTest);
                break;
            case 1 :
                // FOR FINAL TEST
                SetTestingStage(eFinalTest);
                break;
            case 2 :
                // FOR E TEST
                SetTestingStage(eElectTest);
                break;
            }

            // Check if this incremental update can be used on this TestingStage
            if(!lIncrementalUpdates[lKey]["testing_stage"].contains(m_strTestingStage,Qt::CaseInsensitive))
                continue;

            // Check if the first step have a result
            if(IncrementalUpdatesList.contains(lKey)
                    && IncrementalUpdatesList[lKey].contains(m_strTestingStage)
                    && !IncrementalUpdatesList[lKey][m_strTestingStage].isEmpty())
                continue;

            QString lTableName = NormalizeTableName("_SPLITLOT");
            QString lClauseWhere;

            if(mBackgroundTransferActivated
                    && (mBackgroundTransferInProgress > 0))
            {
                // We need to ignore splitlots not already TRANSFERRED
                // First check if have a BACKGROUND TRANSFER IN PROGRESS
                BackgroundTransferExcludeForIncremental(m_strPrefixTable,lIncrementalUpdates[lKey]["target"],lClauseWhere);
            }

            // Just take the first
            // Most of the time, this will be one of the oldest
            // Can be aleatory
            lQuery = " SELECT "+lIncrementalUpdates[lKey]["target"]+" FROM "+lTableName;
            lQuery+= " WHERE ";
            lQuery+= " (INCREMENTAL_UPDATE like '%|"+lKey+"|%' "; // Contains the key is after a |
            lQuery+= " OR INCREMENTAL_UPDATE like '%|"+lKey+"') "; // Ends with the key is after a |
            lQuery+= " AND VALID_SPLITLOT<>'N'";
            lQuery+= lClauseWhere;
            lQuery+= " LIMIT 1";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            // Take the first;
            if(clGexDbQuery.Next())
                IncrementalUpdatesList[lKey][m_strTestingStage] << (clGexDbQuery.value(0).toString());
        }

        if(!IncrementalUpdatesList.contains(lKey))
        {
            QString lMsg = QString("There are no valid incremental updates for %1").arg(lKey);
            GSLOG(SYSLOG_SEV_ALERT, lMsg.toLatin1().data());
            InsertIntoUpdateLog(lMsg);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Get incremental updates
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > & incrementalUpdates)
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;

    incrementalUpdates.clear();

    // Get incremental updates
    QString   strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    Query_Empty();

    if(m_uiDbVersionBuild < 37)
    {
        m_strlQuery_Fields.append("Field|incremental_update.db_update_name");
        m_strlQuery_Fields.append("Field|incremental_update.initial_splitlots");
        m_strlQuery_Fields.append("Field|incremental_update.remaining_splitlots");
    }
    else
    {
        m_strlQuery_Fields.append("Field|incremental_update.db_update_name");
        m_strlQuery_Fields.append("Field|incremental_update.processed_splitlots");
        m_strlQuery_Fields.append("Field|incremental_update.remaining_splitlots");
        m_strlQuery_Fields.append("Field|incremental_update.status");
        m_strlQuery_Fields.append("Field|incremental_update.frequency");
        m_strlQuery_Fields.append("Field|incremental_update.max_items");
        m_strlQuery_Fields.append("Field|incremental_update.last_schedule");
        m_strlQuery_Fields.append("Field|incremental_update.last_execution");
    }
    Query_BuildSqlString(strQuery, true);
    if(!clQuery.exec(strQuery))
        return false;

    int lIndex;
    QString lIncrementalName;
    while(clQuery.next())
    {
        lIndex = 0;
        lIncrementalName = clQuery.value(lIndex++).toString().toUpper();
        incrementalUpdates[lIncrementalName]["db_update_name"]=lIncrementalName;
        incrementalUpdates[lIncrementalName]["processed_splitlots"]=QString::number(clQuery.value(lIndex++).toUInt());
        incrementalUpdates[lIncrementalName]["remaining_splitlots"]=QString::number(clQuery.value(lIndex++).toUInt());
        if(m_uiDbVersionBuild < 37)
        {
            incrementalUpdates[lIncrementalName]["status"]="";
            incrementalUpdates[lIncrementalName]["frequency"]="";
            incrementalUpdates[lIncrementalName]["max_items"]="";
            incrementalUpdates[lIncrementalName]["last_schedule"]="";
            incrementalUpdates[lIncrementalName]["last_execution"]="";
        }
        else
        {
            incrementalUpdates[lIncrementalName]["status"]=
                    clQuery.value(lIndex++).toString().toUpper();
            incrementalUpdates[lIncrementalName]["status_type"]=
                    "DISABLED|ENABLED";
            incrementalUpdates[lIncrementalName]["frequency"]=
                    clQuery.value(lIndex++).toString();
            incrementalUpdates[lIncrementalName]["frequency_type"]=
                    "CRON (min hour day month dow)";
            incrementalUpdates[lIncrementalName]["max_items"]=
                    clQuery.value(lIndex++).toString();
            incrementalUpdates[lIncrementalName]["max_items_type"]=
                    "NUMBER";
            incrementalUpdates[lIncrementalName]["last_schedule"]=
                    clQuery.value(lIndex++).toString();
            incrementalUpdates[lIncrementalName]["last_execution"]=
                    clQuery.value(lIndex++).toString();
        }
        // Update definition
        incrementalUpdates[lIncrementalName]["db_update_name_description"]=
                "Custom Incremental Update KeyWord.";
        incrementalUpdates[lIncrementalName]["testing_stage"]=
                QString(GEXDB_PLUGIN_GALAXY_ETEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_WTEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_FTEST);
        incrementalUpdates[lIncrementalName]["target"]=
                "splitlot_id";
        if(mIncrementalUpdateName.contains(lIncrementalName))
        {
            incrementalUpdates[lIncrementalName]["db_update_name_description"]=
                    mIncrementalUpdateName[lIncrementalName]["description"];
            incrementalUpdates[lIncrementalName]["testing_stage"]=
                    mIncrementalUpdateName[lIncrementalName]["testing_stage"];
            incrementalUpdates[lIncrementalName]["target"]=
                    mIncrementalUpdateName[lIncrementalName]["target"];
        }

        incrementalUpdates[lIncrementalName]["processed_splitlots_description"]=
                "Total of splitlots processed by this update.";
        incrementalUpdates[lIncrementalName]["remaining_splitlots_description"]=
                "Remaining splitlots to process with this update.";
        incrementalUpdates[lIncrementalName]["status_default"]=
                "ENABLED";
        incrementalUpdates[lIncrementalName]["status_description"]=
                "Indicates if the update can be schedule\n"\
                "* ENABLED: the update will be checked during the Scheduler"\
                "* DISABLED: the update will be ignored";
        incrementalUpdates[lIncrementalName]["frequency_default"]=
                "*/10 * * * *";
        incrementalUpdates[lIncrementalName]["frequency_description"]=
                "A CRON expression is a string comprising 5 fields separated by white space that represents a set of times.\n"\
                "* '*'=> all unit\n"\
                "* 'int'=> at a specific unit,\n"\
                "* '*/int'=> each 'int' unit";
        incrementalUpdates[lIncrementalName]["max_items_default"]=
                "10";
        incrementalUpdates[lIncrementalName]["max_items_description"]=
                "Represents the number of items to process for each iteration of the update.\n"\
                "* -1: indicates NO LIMIT";
        incrementalUpdates[lIncrementalName]["last_schedule_description"]=
                "Indicates when this update was scheduled by the YieldMan Scheduler.";
        incrementalUpdates[lIncrementalName]["last_execution_description"]=
                "Indicates when this update was executed.";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Set incremental updates
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates)
{
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;

    // Get incremental updates
    QSqlQuery clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString   lQuery;
    QString   lValue;

    QString   lLog;
    lLog = "************************************************************************* ";
    InsertIntoUpdateLog(lLog);
    lLog = "o Save Incremental Update settings";
    InsertIntoUpdateLog(lLog);

    // Remove deleted entries
    lQuery = "DELETE FROM incremental_update WHERE db_update_name NOT IN ('"+QStringList(incrementalUpdates.keys()).join("','")+"')";
    if(!clQuery.exec(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        lLog =GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        InsertIntoUpdateLog(lLog);
        return false;
    }

    // Add the new
    // Update the existing
    lQuery = "INSERT INTO incremental_update(db_update_name,status,frequency,max_items) VALUES";
    lValue = "";
    foreach(const QString &lKey, incrementalUpdates.keys())
    {
        lLog = "* Save "+lKey;
        InsertIntoUpdateLog(lLog);

        // Check if can be enabled for Custom Incremental Updates
        if(!mIncrementalUpdateName.contains(lKey)
                && (incrementalUpdates[lKey]["status"]=="ENABLED"))
        {
            if(clQuery.exec("SELECT routine_name FROM information_schema.ROUTINES WHERE routine_schema='"+
                            m_pclDatabaseConnector->m_strDatabaseName+
                            "' AND routine_name LIKE '%custom_incremental_update%' AND routine_definition LIKE '%"+
                            lKey+"%'")
                    && !clQuery.first())
            {
                lLog = "<font color=RED>You need to update the stored procedures xx_custom_incremental_update() \n"\
                       " before to manage this IncrementalKeyword</font>";
                InsertIntoUpdateLog(lLog);
                continue;
            }
        }
        lValue="(";
        lValue+=TranslateStringToSqlVarChar(incrementalUpdates[lKey]["db_update_name"])+",";
        lValue+=TranslateStringToSqlVarChar(incrementalUpdates[lKey]["status"])+",";
        lValue+=TranslateStringToSqlVarChar(incrementalUpdates[lKey]["frequency"])+",";
        lValue+=TranslateStringToSqlVarChar(incrementalUpdates[lKey]["max_items"])+")";
        lValue+=" ON DUPLICATE KEY UPDATE status="+TranslateStringToSqlVarChar(incrementalUpdates[lKey]["status"])+",";
        lValue+="frequency="+TranslateStringToSqlVarChar(incrementalUpdates[lKey]["frequency"])+",";
        lValue+="max_items="+TranslateStringToSqlVarChar(incrementalUpdates[lKey]["max_items"]);
        if(!clQuery.exec(lQuery + lValue))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            lLog =GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            InsertIntoUpdateLog(lLog);
            return false;
        }
    }

    int lCount;
    // Update the new count of remaining_splitlots and if have enabled update now
    GetIncrementalUpdatesCount(true,lCount);

    lLog = "--> "+QString::number(incrementalUpdates.count())+" Incremental Updates updated";
    InsertIntoUpdateLog(lLog);

    return true;
}

///////////////////////////////////////////////////////////
// Check incremental updates property
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsIncrementalUpdatesSettingsValidValue(QString &name, QString &value)
{

    bool    lStatus = true;
    QString lOptionName = name.toLower().simplified();
    QString lOptionValue = value.toUpper().simplified();
    if(lOptionName.toLower() == "status")
    {
        if(lOptionValue.startsWith("E"))
            lOptionValue = "ENABLED";
        if(lOptionValue.startsWith("D"))
            lOptionValue = "DISABLED";

        if(lOptionValue != "ENABLED" && lOptionValue != "DISABLED")
        {
            // ENABLED or DISABLED status
            lOptionValue = "ENABLED";
            lStatus = false;
        }
    }
    if(lOptionName.toLower() == "frequency")
    {
        // CRON definition
        QMap<QString,QString> lValues;
        QMap<QString,QString> lRegExps;
        lRegExps["min"]="\\*(/\\d*)?|[0-5]?\\d";
        lRegExps["hour"]="\\*(/\\d*)?|[01]?\\d|2[0-3]";
        lRegExps["day"]="\\*(/\\d*)?|0?[1-9]|[12]\\d|3[01]";
        lRegExps["month"]="\\*(/\\d*)?|[1-9]|1[012]";
        lRegExps["dow"]="\\*|[0-7]";

        if(lOptionValue.split(" ").count() != 5)
        {
            lOptionValue = "*/10 * * * *";
            lStatus = false;
        }
        lValues["min"]=lOptionValue.section(" ",0,0);
        lValues["hour"]=lOptionValue.section(" ",1,1);
        lValues["day"]=lOptionValue.section(" ",2,2);
        lValues["month"]=lOptionValue.section(" ",3,3);
        lValues["dow"]=lOptionValue.section(" ",4,4);

        foreach(const QString &lKey , lValues.keys())
        {
            QRegExp lRegExp(lRegExps[lKey]);
            if (!lRegExp.exactMatch(lValues[lKey]))
            {
                lOptionValue = "*/10 * * * *";
                lStatus = false;
                break;
            }
        }
    }
    if(lOptionName.toLower() == "max_items")
    {
        // NUMBER
        lOptionValue.toInt(&lStatus);
        if(!lStatus)
            lOptionValue = "10";
    }

    value = lOptionValue;
    return lStatus;
}

bool GexDbPlugin_Galaxy::FlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey)
{
    if(splitlots.isEmpty()) return true;
    if(incrementalKey.isEmpty()) return true;

    /////////////////////////////////////////////
    /// the incremental_update is used for sevral kind of update
    /// * can be NULL
    /// * can already contains the new key
    /// * can contains a list of other keys separated with a |
    /// the new key need to be concatened to the existing one

    QString lKey = incrementalKey.toUpper().simplified();
    QString lQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Starts with NULL
    lQuery = "UPDATE " + NormalizeTableName( "_splitlot" );
    lQuery += " SET incremental_update='"+lKey+"'";
    lQuery+= " WHERE valid_splitlot<>'N' ";
    lQuery+= " AND splitlot_id IN (" + splitlots.join( ",") + ")";
    lQuery+= " AND (incremental_update IS NULL";
    lQuery+= "   OR incremental_update = '')";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    lQuery = "UPDATE " + NormalizeTableName( "_splitlot" );
    lQuery += " SET incremental_update=CONCAT(incremental_update,'|','"+lKey+"')";
    lQuery+= " WHERE valid_splitlot<>'N' ";
    lQuery+= " AND splitlot_id IN (" + splitlots.join( ",") + ")";
    lQuery+= " AND (incremental_update <> '"+lKey+"'";              // Extact match
    lQuery+= "   AND incremental_update NOT LIKE '"+lKey+"|%'";     // Starts with
    lQuery+= "   AND incremental_update NOT LIKE '%|"+lKey+"|%'";   // Contains
    lQuery+= "   AND incremental_update NOT LIKE '%|"+lKey+"')";    // Ends with
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clGexDbQuery.Execute("COMMIT");
    return true;
}

bool GexDbPlugin_Galaxy::UnFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey)
{
    if(splitlots.isEmpty()) return true;
    if(incrementalKey.isEmpty()) return true;

    /////////////////////////////////////////////
    /// the incremental_update is used for several kind of update
    /// * can be NULL
    /// * can already contains the new key
    /// * can contains a list of other keys separated with a |
    /// the new key need to be concatened to the existing one

    QString lKey = incrementalKey.toUpper().simplified();
    QString lQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Extract each  list of incremental_update keys with the associated splitlots list
    QMap<QString,QString> lKeysSplitlots;
    lQuery = "SELECT incremental_update, GROUP_CONCAT(splitlot_id) AS splitlots";
    lQuery+= " FROM "+NormalizeTableName("_splitlot");
    lQuery+= " WHERE valid_splitlot<>'N' ";
    lQuery+= " AND splitlot_id IN (" + splitlots.join( ",") + ")";
    lQuery+= " AND (incremental_update = '"+lKey+"'";              // Extact match
    lQuery+= "   OR incremental_update LIKE '"+lKey+"|%'";     // Starts with
    lQuery+= "   OR incremental_update LIKE '%|"+lKey+"|%'";   // Contains
    lQuery+= "   OR incremental_update LIKE '%|"+lKey+"')";    // Ends with
    lQuery+= " GROUP BY incremental_update";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    while(clGexDbQuery.Next())
    {
        if(clGexDbQuery.value("incremental_update").toString().isEmpty())
            continue;
        lKeysSplitlots[clGexDbQuery.value("incremental_update").toString().simplified().toUpper()] = clGexDbQuery.value("splitlots").toString();
    }

    // Update all splitlots with the new keys list
    foreach(const QString &lKeys, lKeysSplitlots.keys())
    {
        // List of existing keys
        QStringList lNewKeys = QString(lKeys).remove(" ").split("|",QString::SkipEmptyParts);

        // Remove the incrementalKey
        lNewKeys.removeOne(lKey);

        // Update the splitlot table
        lQuery = "UPDATE "+NormalizeTableName("_splitlot");
        lQuery+= " SET incremental_update=";
        if(lNewKeys.isEmpty())
            lQuery+="null";
        else
            lQuery+=TranslateStringToSqlVarChar(lNewKeys.join("|"));
        lQuery+= " WHERE splitlot_id IN ("+lKeysSplitlots[lKeys]+")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }
    clGexDbQuery.exec("COMMIT");
    return true;
}

bool GexDbPlugin_Galaxy::SwitchFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalOldKey, const QString &incrementalNewKey)
{
    if(splitlots.isEmpty()) return true;
    if(incrementalOldKey.isEmpty()) return true;
    if(incrementalNewKey.isEmpty()) return true;

    /////////////////////////////////////////////
    /// the incremental_update is used for several kind of update
    /// * can be NULL
    /// * can already contains the new key
    /// * can contains a list of other keys separated with a |
    /// the new key need to be concatened to the existing one

    QString lKey = incrementalOldKey.toUpper().simplified();
    QString lQuery;
    QStringList additionalNewKeys;
    additionalNewKeys.append(incrementalNewKey.toUpper().simplified());

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Extract each  list of incremental_update keys with the associated splitlots list
    QMap<QString,QString> lKeysSplitlots;
    lQuery = "SELECT incremental_update, GROUP_CONCAT(splitlot_id) AS splitlots";
    lQuery+= " FROM "+NormalizeTableName("_splitlot");
    lQuery+= " WHERE splitlot_id IN ("+splitlots.join(",")+")";
    lQuery+= " AND (incremental_update = '"+lKey+"'";          // Extact match
    lQuery+= "   OR incremental_update LIKE '"+lKey+"|%'";     // Starts with
    lQuery+= "   OR incremental_update LIKE '%|"+lKey+"|%'";   // Contains
    lQuery+= "   OR incremental_update LIKE '%|"+lKey+"')";    // Ends with
    lQuery+= " GROUP BY incremental_update";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    while(clGexDbQuery.Next())
    {
        if(clGexDbQuery.value("incremental_update").toString().isEmpty())
            continue;
        lKeysSplitlots[clGexDbQuery.value("incremental_update").toString().simplified().toUpper()] = clGexDbQuery.value("splitlots").toString();
    }

    // Update all splitlots with the new keys list
    foreach(const QString &lKeys, lKeysSplitlots.keys())
    {
        // List of existing keys
        QStringList lNewKeys = QString(lKeys).remove(" ").split("|",QString::SkipEmptyParts);
        lNewKeys.append(additionalNewKeys);

        // Remove the incrementalKey
        lNewKeys.removeOne(lKey);

        // Update the splitlot table
        lQuery = "UPDATE "+NormalizeTableName("_splitlot");
        lQuery+= " SET incremental_update=";
        if(lNewKeys.isEmpty())
            lQuery+="null";
        else
            lQuery+=TranslateStringToSqlVarChar(lNewKeys.join("|"));
        lQuery+= " WHERE splitlot_id IN ("+lKeysSplitlots[lKeys]+")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update DB: incremental update
// incrementalName : the name of incremental update to apply
// testingStage : Testing Stage to check
// TESTING_STAGE=FT
// target
// LOT_ID=ABC
// SPLITLOT_ID=1502300000
// summary
//  QMap[incrementalName]
//      testing_stage
//      target
//      targeted_splitlots
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString > &summary)
{
    summary.clear();
    if (!m_pclDatabaseConnector)
        return false;
    if(!m_pclDatabaseConnector->IsConnected())
        return false;
    if(incrementalName.isEmpty()
            || testingStage.isEmpty()
            || target.isEmpty())
        return false;

    if(testingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        // FOR WAFER TEST
        SetTestingStage(eWaferTest);
    else if(testingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        // FOR ELECT TEST
        SetTestingStage(eElectTest);
    else if(testingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        // FOR FINAL TEST
        SetTestingStage(eFinalTest);
    else
        return false;

    // Incremantally update DB
    QString   lQuery;
    QString   lLogMessage;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    bool              lStatus = false;
    int               lIncrementalUpdates = 0;
    QString           lUpdateName = incrementalName;
    QMap<QString,QString> lUpdateOptions;

    // Update the "target" to "splitlot_id" or "lot_id"
    QString lTarget = "splitlot_id";
    if(mIncrementalUpdateName.contains(lUpdateName))
        lTarget = mIncrementalUpdateName[lUpdateName]["target"];
    lUpdateOptions[lTarget] = target;

    summary["incrementalupdate"] = incrementalName;
    summary["testingstage"] = m_strTestingStage;
    summary["databasename"] = m_pclDatabaseConnector->m_strDatabaseName;

    if(lTarget=="lot_id") summary["lot"]=target;
    else summary["splitlotid"]=target;

    summary["status"] = "DELAY";

    // Get more info
    QString                     lTableName, lLotId, lSubLotWaferId;
    QStringList                 lstFlaggedSplitLots;
    QStringList                 lstSubLotWafers;
    QMap<QString,QStringList>   lstSubLotWafersSplitlots;
    QStringList                 lIncrementalWarnings;

    int   lNbCompletedSplitLots = 1;
    int   nNbStep = 0;
    int   nProgress = 0;

    SetProgress(0);
    // For all incremental update
    lTableName = NormalizeTableName("_SPLITLOT");
    lQuery = "SELECT group_concat(SPLITLOT_ID ORDER BY SPLITLOT_ID SEPARATOR ',') as SPLITLOT_LIST, ";
    lQuery+= " LOT_ID, ";
    if(lUpdateOptions.contains("lot_id"))
    {
        if(m_eTestingStage == eFinalTest)
        {
            lQuery+= " SUBLOT_ID AS SUB_LIST, ";
        }
        else
        {
            lQuery+= " WAFER_ID AS SUB_LIST, ";
        }
    }
    else
    {
        lQuery+= " SPLITLOT_ID AS SUB_LIST, ";
    }
    lQuery+= " max(SPLITLOT_ID) as MAX_INSERTION";
    lQuery+= " FROM "+lTableName;
    lQuery+= " WHERE ";
    if(lUpdateOptions.contains("lot_id"))
        lQuery+= " LOT_ID="+TranslateStringToSqlVarChar(target);
    else
        lQuery+= " SPLITLOT_ID="+target;
    lQuery+= " AND VALID_SPLITLOT<>'N'";
    lQuery+= " AND ((INCREMENTAL_UPDATE = '"+incrementalName+"' )";         // Extact match
    lQuery+= "   OR (INCREMENTAL_UPDATE like '"+incrementalName+"|%' )";    // Starts with
    lQuery+= "   OR (INCREMENTAL_UPDATE like '%|"+incrementalName+"|%' )";  // Contains
    lQuery+= "   OR (INCREMENTAL_UPDATE like '%|"+incrementalName+"' ))";   // Ends with
    if(lUpdateOptions.contains("lot_id"))
    {
        lQuery+= " GROUP BY LOT_ID";
        if(m_eTestingStage == eFinalTest)
        {
            lQuery+= ",SUBLOT_ID ";
        }
        else
        {
            lQuery+= ",WAFER_ID ";
        }
    }
    else
    {
        lQuery+= " GROUP BY SPLITLOT_ID";
    }
    lQuery+= " ORDER BY MAX_INSERTION ";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto updatedb_incrementalupdatecommand_error;
    }
    while(clGexDbQuery.Next())
    {
        lstFlaggedSplitLots += clGexDbQuery.value("SPLITLOT_LIST").toString().split(",");
        lLotId = clGexDbQuery.value("LOT_ID").toString();
        lstSubLotWafers += clGexDbQuery.value("SUB_LIST").toString();
        lstSubLotWafersSplitlots[clGexDbQuery.value("SUB_LIST").toString()] = clGexDbQuery.value("SPLITLOT_LIST").toString().split(",");
    }
    nNbStep = lstSubLotWafers.count();
    SetMaxProgress(nNbStep);

    if(lstFlaggedSplitLots.isEmpty())
    {
        // Nothing to do
        summary["status"] = "PASS";
        summary["info"] = "Nothing to do";

        // If Nothing To Do, this means:
        // * an incremental update was processed and the flags was removed
        // * some incrental update splitlots are flagged as INVALID
        return true;
    }

    /////////////////////////////////////////
    lLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(lLogMessage);
    lLogMessage = "o Incremental update "+lUpdateName;
    if(mIncrementalUpdateName.contains(lUpdateName))
        lLogMessage += ": "+mIncrementalUpdateName[lUpdateName]["description"];
    InsertIntoUpdateLog(lLogMessage);
    foreach(const QString &lKey,summary.keys())
    {
        if(lKey.startsWith("status",Qt::CaseInsensitive))
            continue;
        InsertIntoUpdateLog("* "+lKey+": "+summary[lKey]);
    }
    InsertIntoUpdateLog("* Splitlots targeted by this update: "+QString::number(lstFlaggedSplitLots.count()));

    if(incrementalName == "BINNING_CONSOLIDATION_DELAY")
    {
        // To allow CONSOLIDATION in the process
        // Switch to BINNING_CONSOLIDATION
        // If the CONSOLIDATION fails again
        // * move to BINNING_CONSOLIDATION_DELAY to reschedule a consolidation
        // * or move to BINNING_CONSOLIDATION_DELAY to stop
        lUpdateName = "BINNING_CONSOLIDATION";
        SwitchFlagSplitlotsForIncrementalUpdate(lstFlaggedSplitLots, incrementalName, lUpdateName);
    }

    if(lUpdateName == "BINNING_CONSOLIDATION")
    {
        QStringList lConsolidationWarnings;

        ////////////////////////////////////////////////////////////////
        // BINNING CONSOLIDATION incremental update
        // * Perform binning consolidation (wafer and lot level)
        ////////////////////////////////////////////////////////////////
        summary["status"] = "DELAY";
        lNbCompletedSplitLots = 0;

        // FOR ETEST AND WAFER
        if((m_eTestingStage == eElectTest)
                || (m_eTestingStage == eWaferTest))
        {

            /////////////////////////////////////////
            // WAFER CONSOLIDATION
            // for each WAFER_ID, call the consolidation function

            /////////////////////////////////////////
            // Update progress
            lLogMessage = "... Lot " + lLotId;
            lLogMessage += ": computing yield statistics consolidation on ";
            lLogMessage += NormalizeTableName("_WAFER_INFO");
            lLogMessage += " table.";
            InsertIntoUpdateLog(lLogMessage);

            // Do a commit before call new query
            clGexDbQuery.Execute("COMMIT");

            // CONSOLIDATION PHASE
            InsertIntoUpdateLog(QString(" - Wafer "));
            while(!lstSubLotWafers.isEmpty())
            {
                lSubLotWaferId = lstSubLotWafers.takeFirst();

                // Update progress for each wafer
                SetProgress(++nProgress);
                InsertIntoUpdateLog(QString(" '%1'").arg(lSubLotWaferId),false);
                QCoreApplication::processEvents();

                lStatus = UpdateWaferConsolidation(lLotId,lSubLotWaferId);

                GetWarnings(lConsolidationWarnings);
                if(!lConsolidationWarnings.isEmpty())
                {
                    QString lWarning = QString(" - Wafer '%1'").arg(lSubLotWaferId);
                    lWarning += " - "+lConsolidationWarnings.takeLast();
                    InsertIntoUpdateLog(lWarning);
                    if(!lstSubLotWafers.isEmpty())
                        InsertIntoUpdateLog(QString(" - Wafer "));
                    lIncrementalWarnings += lWarning;
                }

                if(lStatus)
                {
                    lNbCompletedSplitLots += lstSubLotWafersSplitlots[lSubLotWaferId].count();
                }
                else
                {
                    summary["status"] = "FAIL";
                    // GCORE-14433 - Consolidation hangs.
                    QString lMoveToUpdate = "BINNING_CONSOLIDATION_DELAY";
                    if(incrementalName == "BINNING_CONSOLIDATION_DELAY")
                    {
                        // If we want to block the CONSOLIDATION
                        // lMoveToUpdate = "BINNING_CONSOLIDATION_FAIL";
                    }
                    SwitchFlagSplitlotsForIncrementalUpdate(lstSubLotWafersSplitlots[lSubLotWaferId], lUpdateName, lMoveToUpdate);
                    // Ignore the DELAY
                    // Continue on other SubList
                    // Process a LotConsolidation if at least one SubList was consolidated
                    // goto updatedb_incrementalupdatecommand_error;
                }
            }
        }

        // FOR FINAL TEST
        if(m_eTestingStage == eFinalTest)
        {
            /////////////////////////////////////////
            // SUBLOT CONSOLIDATION
            // for each SUBLOT_ID, call the consolidation function

            /////////////////////////////////////////
            // Update progress
            lLogMessage = "... Lot " + lLotId;
            lLogMessage += ": computing yield statistics consolidation on ";
            lLogMessage += NormalizeTableName("_SUBLOT_INFO");
            lLogMessage += " table.";
            InsertIntoUpdateLog(lLogMessage);

            // Do a commit before call new query
            clGexDbQuery.Execute("COMMIT");

            // CONSOLIDATION PHASE
            InsertIntoUpdateLog(QString(" - Sublot "));
            while(!lstSubLotWafers.isEmpty())
            {
                lSubLotWaferId = lstSubLotWafers.takeFirst();

                // Udate progress for each sublot
                SetProgress(++nProgress);
                InsertIntoUpdateLog(QString(" '%1'").arg(lSubLotWaferId),false);
                QCoreApplication::processEvents();

                lStatus = UpdateSubLotConsolidation(lLotId,lSubLotWaferId);
                GetWarnings(lConsolidationWarnings);
                if(!lConsolidationWarnings.isEmpty())
                {
                    QString lWarning = QString(" - Sublot '%1'").arg(lSubLotWaferId);
                    lWarning += " - "+lConsolidationWarnings.takeLast();
                    InsertIntoUpdateLog(lWarning);
                    if(!lstSubLotWafers.isEmpty())
                        InsertIntoUpdateLog(QString(" - Sublot "));
                    lIncrementalWarnings += lWarning;
                }

                if(lStatus)
                {
                    lNbCompletedSplitLots += lstSubLotWafersSplitlots[lSubLotWaferId].count();
                }
                else
                {
                    summary["status"] = "FAIL";
                    // GCORE-14433 - Consolidation hangs.
                    QString lMoveToUpdate = "BINNING_CONSOLIDATION_DELAY";
                    if(incrementalName == "BINNING_CONSOLIDATION_DELAY")
                    {
                        // If we want to block the CONSOLIDATION
                        // lMoveToUpdate = "BINNING_CONSOLIDATION_FAIL";
                    }
                    SwitchFlagSplitlotsForIncrementalUpdate(lstSubLotWafersSplitlots[lSubLotWaferId], lUpdateName, lMoveToUpdate);
                    // Ignore the DELAY
                    // Continue on other SubList
                    // Process a LotConsolidation if at least one SubList was consolidated
                    // goto updatedb_incrementalupdatecommand_error;
                }
            }
        }

        if(lNbCompletedSplitLots == 0)
        {
            goto updatedb_incrementalupdatecommand_error;
        }

        /////////////////////////////////////////
        // LOT CONSOLIDATION

        /////////////////////////////////////////
        // Update progress
        lLogMessage = "... Lot " + lLotId;
        lLogMessage += ": computing yield statistics consolidation on ";
        lLogMessage += NormalizeTableName("_LOT");
        lLogMessage += " table.";
        InsertIntoUpdateLog(lLogMessage);
        SetProgress(++nProgress);

        /////////////////////////////////////////
        // CONSOLIDATION PHASE
        lStatus = UpdateLotConsolidation(lLotId);
        GetWarnings(lConsolidationWarnings);
        if(!lConsolidationWarnings.isEmpty())
        {
            QString lWarning = QString("Consolidation for Lot '%1'").arg(lSubLotWaferId);
            lWarning += " - "+lConsolidationWarnings.takeLast();
            InsertIntoUpdateLog(lWarning);
            lIncrementalWarnings += lWarning;
        }

        // unlock table
        clGexDbQuery.Execute("COMMIT"); // the only way for unlock table

        if(!lStatus)
        {
            summary["status"] = "FAIL";
            goto updatedb_incrementalupdatecommand_error;
        }

        /////////////////////////////////////////
        // CONSOLIDATION ENDING FOR LOT_ID

        SetProgress(++nProgress);

        ////////////////////////////////////////////////////////////////
        // END BINNING_CONSOLIDATION incremental update
        ////////////////////////////////////////////////////////////////
    }

    if(lUpdateName == "FT_CONSOLIDATE_SOFTBIN")
    {
        ////////////////////////////////////////////////////////////////
        // FT_CONSOLIDATE_SOFTBIN incremental update
        // * Perform binning consolidation (ft lot level)
        ////////////////////////////////////////////////////////////////


        /////////////////////////////////////////
        // LOT CONSOLIDATION

        /////////////////////////////////////////
        // Update progress
        lLogMessage = "... Lot " + lLotId;
        lLogMessage += ": computing yield statistics consolidation on ";
        lLogMessage += NormalizeTableName("_LOT_SBIN");
        lLogMessage += " table.";
        InsertIntoUpdateLog(lLogMessage);
        SetProgress(++nProgress);

        /////////////////////////////////////////
        // CONSOLIDATION PHASE
        bool bDelay;
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]=1;
        m_pbDelayInsertion  = &bDelay;
        mpDbKeysEngine      = NULL;

        /////////////////////////////////////////
        // HBIN and SBIN updated during the call of functions UpdateWaferInfoTable and UpdateLotTable
        m_mapHBinInfo.clear();
        m_mapSBinInfo.clear();
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]=1;

        m_strLotId = lLotId;
        // GCORE-187 - SubLot consolidation
        if(!ConsolidateLot(lLotId,true,false))
        {
            summary["status"] = "FAIL";
            goto updatedb_incrementalupdatecommand_error;
        }


        /////////////////////////////////////////
        // CONSOLIDATION ENDING FOR LOT_ID

        /////////////////////////////////////////
        // Update INCREMENTAL_UPDATE column for all SPLITLOT_ID with current LOT_ID
        /////////////////////////////////////////
        QCoreApplication::processEvents();
        SetProgress(++nProgress);

        ////////////////////////////////////////////////////////////////
        // END FT_CONSOLIDATE_SOFTBIN incremental update
        ////////////////////////////////////////////////////////////////
    }

    if(
        lUpdateName == "AGENT_WORKFLOW"
    )
    {
        // get agent global options
        QString agentEnabledString;
        QString agentAddress;
        QString agentPortString;
        QString agentRoute;
        bool agentSettingsRetrieved = true;

        agentSettingsRetrieved &= GetGlobalOptionValue(eExternalServicesAgentWorkflowEnabled, agentEnabledString);
        agentSettingsRetrieved &= GetGlobalOptionValue(eExternalServicesJobQueueAddress, agentAddress);
        agentSettingsRetrieved &= GetGlobalOptionValue(eExternalServicesJobQueuePort, agentPortString);
        agentSettingsRetrieved &= GetGlobalOptionValue(eExternalServicesJobQueueRoute, agentRoute);
        if(!agentSettingsRetrieved)
        {
            summary["status"] = "DELAY";
            summary["info"] = "Errors when getting database options for statistical agents: settings not defined.";
            goto updatedb_incrementalupdatecommand_error;
        }

        bool agentEnabled = (agentEnabledString == "TRUE");
        unsigned short agentPort = agentPortString.toInt(&agentSettingsRetrieved);
        if(!agentSettingsRetrieved)
        {
            summary["status"] = "DELAY";
            summary["info"] = "Errors when getting database options for statistical agents: invalid port value.";
            goto updatedb_incrementalupdatecommand_error;
        }

        // if agent infrastructure interaction isn't enabled, stop here, but it is not an error
        if(!agentEnabled)
        {
            summary["status"] = "PASS";
            summary["info"] = "Agent is disabled.";
            lStatus = true;
            InsertIntoUpdateLog("INFO: " + summary["info"]);
            // And unflag the targeted splitlots ...
        }
        else
        {
            // Extract the result of the consolidation for the affected splitlots
            // Through the consolidation and consolidation_inter tables

            // Extract only PROD_DATA flow = 'Y' from consolidation_prod_flow for the first version
            // For the second version, the consolidation_flow will replace the consolidation_prod_flow
            // And the extraction will be on all

            QString lSplitlotTable = NormalizeTableName("_SPLITLOT");
            QString waferField = "wafer_id, ";
            if(m_eTestingStage == eFinalTest)
            {
                waferField = "null AS wafer_id, ";
            }

            // Extract the list of jobs that need to be posted
            lQuery = "SELECT DISTINCT lot_id, sublot_id, " + waferField + "test_flow, test_insertion FROM " + lSplitlotTable + " WHERE splitlot_id IN (" + lstFlaggedSplitLots.join(",") + ");";
            if(!clGexDbQuery.Execute(lQuery))
            {
                // If we are not able to execute this process
                // We can't finalize the JobPost request
                // We need to restart
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                summary["status"] = "FAIL";
                goto updatedb_incrementalupdatecommand_error;
            }

            QList<JobDefinition> jobDefs;
            JobDefinition jobDef;
            QStringList jobDetails;
            while(clGexDbQuery.next())
            {
                jobDef.lotId = clGexDbQuery.value("lot_id").toString();
                jobDef.sublotId = clGexDbQuery.value("sublot_id").toString();
                jobDef.waferId = clGexDbQuery.value("wafer_id").toString();
                jobDef.testFlow = clGexDbQuery.value("test_flow").toString();
                jobDef.testInsertion = clGexDbQuery.value("test_insertion").toString();
                jobDefs.append(jobDef);
            }

            if(jobDefs.isEmpty())
            {
                summary["status"] = "PASS";
                summary["info"] = QString("Nothing to post.");
            }
            else
            {
                // create the http channel to send further defined job data
                GS::Gex::HttpChannel *http = CreateHttpChannel(agentAddress.toStdString(),
                                                               agentPort,
                                                               agentRoute.toStdString() );

                // local event loop, synchronizing work for socket and http signal handling
                QEventLoop localLoop;

                // initiates a timer for the connection, once timeout is reached, local_loop is terminated, leading to an error
                QTimer timer;
                timer.setSingleShot( true );
                timer.start( 60000 );

                // status updated after socket and http request signal handling, true if all is alright, false if an error occurs
                // during a connection or a HTTP post
                bool status = false;

                // the future response of the server
                QByteArray postResponse;

                // create instances of slot functors that will be connected to HttpChannel signals in the local event loop, making
                // the processing of these signal synchronous
                ConnectionTimeoutSlot onConnectionTimeout( status, localLoop );
                ConnectionErrorSlot onConnectionError( status, localLoop );
                PostErrorSlot onPostError( status, localLoop );
                PostFinishedSlot onPostFinished( status, localLoop, postResponse );

                // connecting various signals from http to the local event loop
                connect( http, &GS::Gex::HttpChannel::connectionError,
                         &localLoop, onConnectionError,
                         Qt::AutoConnection );

                connect( http, &GS::Gex::HttpChannel::postError,
                         &localLoop, onPostError,
                         Qt::AutoConnection );

                connect( http, &GS::Gex::HttpChannel::postFinished,
                         &localLoop, onPostFinished,
                         Qt::AutoConnection );

                // connecting the timer on the local loop, implementing a timeout check on the connection
                connect( &timer, &QTimer::timeout,
                         &localLoop, onConnectionTimeout,
                         Qt::AutoConnection );

                // create some job's properties from key_content
                std::stringstream ss;
                QStringList job_id_sequence;
                // test_flow.-1.FINAL.P,
                // test_insertion.0.final.P
                foreach(jobDef, jobDefs)
                {
                    ss.str("");
                    ss.clear();

                    ss << testingStage.toStdString()
                       << " -- "
                       << QString("%1/%2/%3").arg(jobDef.lotId).arg(jobDef.sublotId).arg(jobDef.waferId).toStdString()
                       << " -- "
                       << jobDef.testFlow.toStdString()
                       << "."
                       << jobDef.testInsertion.toStdString();

                    const std::string jobTitle( ss.str() );

                    // create job data
                    GS::Gex::StatisticalAgentJobData jobdata(jobTitle,
                        GS::Gex::StatisticalAgentJobData::EvaluateTestingStage(testingStage.toStdString()),
                        jobDef.lotId.toStdString(),
                        jobDef.sublotId.toStdString(),
                        jobDef.waferId.toStdString(),
                        "test_insertion",
                        jobDef.testFlow.toStdString(),
                        jobDef.testInsertion.toStdString(),
                        m_pclDatabaseConnector->mAdminDbHostName.toStdString(),
                        m_pclDatabaseConnector->mAdminDbPort,
                        m_pclDatabaseConnector->mAdminDbDatabaseName.toStdString(),
                        m_pclDatabaseConnector->mAdminDbUser.toStdString(),
                        m_pclDatabaseConnector->mAdminDbPwd.toStdString(),
                        m_pclDatabaseConnector->m_strHost_Name.toStdString(),
                        m_pclDatabaseConnector->m_uiPort,
                        m_pclDatabaseConnector->m_strDatabaseName.toStdString(),
                        m_pclDatabaseConnector->m_strUserName.toStdString(),
                        m_pclDatabaseConnector->m_strPassword.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrHostName.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrPort,
                        m_pclDatabaseConnector->m_linkedAdrDatabaseName.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrUser.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrPwd.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrHostName.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrPort,
                        m_pclDatabaseConnector->m_linkedAdrDatabaseName.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrUser.toStdString(),
                        m_pclDatabaseConnector->m_linkedAdrPwd.toStdString()
                    );

                    // Post all Job for Binning stats
                    GS::Gex::StatisticalAgentJob job(GS::Gex::StatisticalAgentJob::AgentWorkflow, jobdata );

                    // posting now
                    http->PostData(job);

                    // if failed, stop here and notify caller by returning false
                    localLoop.exec();

                    if(!status)
                    {
                        summary["status"] = "DELAY";
                        summary["info"] = "An error occurs during the connection or the HTTP post";
                        goto updatedb_incrementalupdatecommand_error;
                    }

                    job_id_sequence.push_back(postResponse);
                }

                summary["created_job_id_sequence"] = job_id_sequence.join( ',' );
                summary["info"] = QString("%1 Jobs posted").arg( job_id_sequence.size() );
            }
        }

    }

    // Customer incremental update
    if(!mIncrementalUpdateName.contains(lUpdateName)
            && (lstFlaggedSplitLots.count()==1))
    {
        ////////////////////////////////////////////////////////////////
        // CUSTOM_INCREMENTAL_UPDATE incremental update
        // * call xx_custom_incremental_update(splitlot_id,incremental_keyword).
        ////////////////////////////////////////////////////////////////
        QString     strProcedureName;
        QString strMessage;
        int       nStatus;

        strProcedureName = NormalizeTableName("_CUSTOM_INCREMENTAL_UPDATE");
        summary["procedure"] = strProcedureName;

        lQuery = "CALL " + strProcedureName;
        lQuery += "(" + lstFlaggedSplitLots.first() + ",";
        lQuery += TranslateStringToSqlVarChar(lUpdateName) + ",";
        lQuery += "@strMessage,@nStatus)";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            goto updatedb_incrementalupdatecommand_error;
        }
        lQuery = "select @strMessage, @nStatus";
        if(!clGexDbQuery.exec(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            goto updatedb_incrementalupdatecommand_error;
        }
        clGexDbQuery.first();
        // Retrieve parameter values
        strMessage = clGexDbQuery.value(0).toString();      // the returned message
        nStatus = clGexDbQuery.value(1).toInt();            // nStatus is the return status: 0 is NOK, 1 is OK, 2 delay insertion

        if(!strMessage.startsWith("Empty custom") && !strMessage.isEmpty())
            summary["info"] = strMessage;
        if(nStatus == 1)
            summary["status"] = "PASS";
        else if(nStatus == 2)
            summary["status"] = "DELAY";
        else
            summary["status"] = "FAIL";


        if(nStatus != 1)
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_CustomIncrementalProcedure, NULL, strProcedureName.toLatin1().constData(),
                        QString("%1: %2").arg(summary["status"]).arg(summary["info"]).toLatin1().constData());
            goto updatedb_incrementalupdatecommand_error;
        }

        // unlock table
        clGexDbQuery.Execute("COMMIT"); // the only way for unlock table
    }

    // Update the summary
    lStatus = true;
    summary["status"] = "PASS";
    summary["nb_splitlots"] = QString::number(lstFlaggedSplitLots.count());

    /////////////////////////////////////////
    lLogMessage = "--> "+QString::number(lNbCompletedSplitLots)+" splitlots updated";
    InsertIntoUpdateLog(lLogMessage);

    /////////////////////////////////////////
    // Update the incremental_update field
    // Remove the IncrementalKeyword from incremental_update field
    UnFlagSplitlotsForIncrementalUpdate(lstFlaggedSplitLots, lUpdateName);

    QCoreApplication::processEvents();
    // Update the incremental_update from all splitlot table
    GetIncrementalUpdatesCount(true,lIncrementalUpdates);

    goto updatedb_incrementalupdatecommand_writelog;

updatedb_incrementalupdatecommand_error:
    // Write error message
    if(lIncrementalWarnings.isEmpty())
    {
        lIncrementalWarnings += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        if(lIncrementalWarnings.isEmpty())
            lIncrementalWarnings += "Unknown error ...";
        InsertIntoUpdateLog(lIncrementalWarnings.takeLast());
    }

    summary["error"] = lIncrementalWarnings.join("\n ");

updatedb_incrementalupdatecommand_writelog:

    // Update the incremental update table
    lQuery = "UPDATE incremental_update ";
    lQuery+= " SET processed_splitlots=processed_splitlots+"+QString::number(lNbCompletedSplitLots);
    lQuery+= " , last_execution=now()";
    lQuery+= " WHERE db_update_name='"+incrementalName+"'";
    clGexDbQuery.exec(lQuery);
    if(incrementalName != lUpdateName)
    {
        lQuery = "UPDATE incremental_update ";
        lQuery+= " SET processed_splitlots=processed_splitlots+"+QString::number(lNbCompletedSplitLots);
        lQuery+= " , last_execution=now()";
        lQuery+= " WHERE db_update_name='"+lUpdateName+"'";
        clGexDbQuery.exec(lQuery);
    }
    clGexDbQuery.exec("COMMIT");

        if(!lIncrementalWarnings.isEmpty())
        {
            summary["info"] = lIncrementalWarnings.join("\n ");
        }

    if(!m_strUpdateDbLogFile.isEmpty())
    {
        lLogMessage = "Incremental update history saved to log file ";
        lLogMessage+= m_strUpdateDbLogFile;
        lLogMessage+= ".";
        InsertIntoUpdateLog(" ");
        InsertIntoUpdateLog(lLogMessage);
    }

    return lStatus;
}

///////////////////////////////////////////////////////////
// \brief GexDbPlugin_Galaxy::BackgroundTransferLazyMode
// \param aTableName : table that need to be query
// \param aSplitlotID : splitlot_id that need to be extracted
// \return : false if query issue
bool GexDbPlugin_Galaxy::BackgroundTransferLazyMode(QString aTableName, long aSplitlotID)
{
    if(mBackgroundTransferActivated)
    {
    QString             lQuery;
    GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if(mBackgroundTransferInProgress > 0)
    {
            // Check if always running
            lQuery = "SELECT MIN(update_id) AS update_id FROM background_transfer_tables WHERE STATUS <> 'DONE'";
        if(!lGexDbQuery.exec(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if(lGexDbQuery.first())
            mBackgroundTransferInProgress = lGexDbQuery.value("update_id").toInt();
        else
            mBackgroundTransferInProgress = 0;
        }
        if(mBackgroundTransferInProgress > 0)
    {
            // Check if the table was already focused
            // And if the splitlot was already transfered or not
            if(!mBackgroundTransferCurrentIndex.contains(aTableName)
                    || (mBackgroundTransferCurrentIndex[aTableName]>aSplitlotID))
    {
                // Need to update the CurrentIndex
                // Case 1
                // The table was not a part of the BACKGROUND TRANSFER flow or is DONE
                // Then update the CurrentIndex with 0
                // Case 2
                // The table is IN PROGRESS
                // Then update the CurrentIndex with the good value
                lQuery = "SELECT MAX(current_index) AS current_index FROM background_transfer_partitions WHERE STATUS <> 'DONE' ";
    lQuery += " AND table_name = '" + aTableName + "'";
    lQuery += " AND update_id >= " + QString::number(mBackgroundTransferInProgress);
    if(!lGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(lGexDbQuery.First())
    {
        mBackgroundTransferCurrentIndex[aTableName] = lGexDbQuery.value("current_index").toLongLong();
    }
    else
    {
        mBackgroundTransferCurrentIndex[aTableName] = 0;
    }
            }
    if(aSplitlotID <= mBackgroundTransferCurrentIndex[aTableName])
    {
                // It seems that the splitlot wasn't already transfered
                // But because of the MULTI-THREAD, may be a thread already start the transfer
                // Before to force the transfer
                // Check if the splitlot is in progress
                lQuery = "SELECT current_index FROM background_transfer_partitions WHERE STATUS <> 'DONE' ";
                lQuery+= " AND table_name='"+aTableName+"'";
                lQuery+= " AND min_index<="+QString::number(aSplitlotID);
                lQuery+= " AND "+QString::number(aSplitlotID)+"<=current_index";
                lQuery+= " AND update_id >= "+QString::number(mBackgroundTransferInProgress);
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
                if(lGexDbQuery.First())
                {
                    // The splitlot is always targeted for a next transfer
                    // Force a transfer
                    lQuery = "CALL background_transfer_force_splitlot('"+aTableName+"',"+
                            QString::number(aSplitlotID)+",@Msg,@Status)";
                    if(!lGexDbQuery.Execute(lQuery))
        {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
                        return false;
                    }
                    lQuery = "SELECT @Msg,@Status";
                    if(!lGexDbQuery.Execute(lQuery) || !lGexDbQuery.First())
            {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
                        return false;
            }
                    // 0: Error
                    // 1: Success
                    if(lGexDbQuery.value(1).toInt())
            {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "CALL background_transfer_force_splitlot FAIL", lGexDbQuery.value(0).toString().toLatin1().constData());
                return false;
            }
                }
            }
        }
    }
    return true;
}


///////////////////////////////////////////////////////////
// \brief GexDbPlugin_Galaxy::BackgroundTransferLazyMode
// \param aPrefixTable : stage that need to be query
// \param aTraget : target granularity that need to be updated
// \param aExcludeClause : SQL Clause (splitlot_id or target that need to be excluded)
// \return : false if query issue
bool GexDbPlugin_Galaxy::BackgroundTransferExcludeForIncremental(QString aPrefixTable, QString aTarget, QString &aExcludeClause)
{
    aExcludeClause = "";
    if(mBackgroundTransferActivated)
    {
        QString             lQuery;
        GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        if(mBackgroundTransferInProgress > 0)
        {
            // Check if always running
            lQuery = "SELECT MIN(update_id) AS update_id FROM background_transfer_tables WHERE STATUS <> 'DONE'";
            if(!lGexDbQuery.exec(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(lGexDbQuery.first())
                mBackgroundTransferInProgress = lGexDbQuery.value("update_id").toInt();
            else
                mBackgroundTransferInProgress = 0;
            }
        if(mBackgroundTransferInProgress > 0)
        {
            QString lBackgroundTable;
            QString lBackgroundCurrentIndex;

            // Check if the table was already focused
            if(!mBackgroundTransferCurrentIndex.contains(aPrefixTable)
                    || mBackgroundTransferCurrentIndex[aPrefixTable] > 0)
            {
                mBackgroundTransferCurrentIndex[aPrefixTable] = 0;

                // Get the splitlot_id in progress
                lQuery = "SELECT table_name,  ";
                lQuery+= " MAX(current_index) AS current_index  ";
                lQuery+= " FROM background_transfer_partitions  ";
                lQuery+= " WHERE STATUS <> 'DONE' ";
                lQuery+= " AND current_index > 0 ";
                lQuery+= " AND table_name LIKE '"+aPrefixTable+"%'";
                lQuery+= " AND update_id >= "+QString::number(mBackgroundTransferInProgress);
                lQuery+= " GROUP BY table_name";
                if(!lGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }
                // We need to get the table which have the biggest current_index
                // This table/current_index will be the first targeted by the BACKGROUND TRANSFER
                while(lGexDbQuery.Next())
                {
                    if(mBackgroundTransferCurrentIndex[aPrefixTable] <= lGexDbQuery.value("current_index").toLongLong())
                    {
                        mBackgroundTransferCurrentIndex[aPrefixTable] = lGexDbQuery.value("current_index").toLongLong();
                        lBackgroundCurrentIndex = lGexDbQuery.value("current_index").toString();
                        lBackgroundTable = lGexDbQuery.value("table_name").toString();
                    }
                }
            }
            if(mBackgroundTransferCurrentIndex[aPrefixTable] > 0)
            {
                // All splitlot_id < current_index must be ignored
                aExcludeClause = " AND splitlot_id > "+lBackgroundCurrentIndex;

                if((aTarget != "splitlot_id")
                        && !lBackgroundTable.isEmpty())
                {
                    QString lLotsQuery;
                    QStringList lTargets;
                    // If the target is not a splitlot_id
                    // We need to eliminate potential 'lot_id' where some splitlots are not transfered
                    // First get the list of target not completed
                    // Select target where splitlot_id is <= current_index
                    // This list is the old insertion and the not transferred data
                    // This list will be smaller and smaller
                    lLotsQuery = "SELECT DISTINCT S."+aTarget;
                    lLotsQuery+= " FROM "+aPrefixTable+"_splitlot S";
                    lLotsQuery+= " WHERE S.VALID_SPLITLOT<>'N' ";
                    // Get lots even if not flagged for incremental_update
                    // This Lots was probably consolidated and dont't have the flag
                    // but the data must be ready for a new consolidation
                    // lLotsQuery+= " AND S.INCREMENTAL_UPDATE IS NOT NULL ";
                    lLotsQuery+= " AND S.SPLITLOT_ID <= "+lBackgroundCurrentIndex;

                    // Then eliminate the new target not completed
                    lQuery = "SELECT DISTINCT "+aTarget;
                    lQuery+= " FROM "+aPrefixTable+"_splitlot ";
                    lQuery+= " WHERE VALID_SPLITLOT<>'N' ";
                    lQuery+= " AND LOT_ID IN ("+lLotsQuery+")";
                    lQuery+= " AND (INCREMENTAL_UPDATE IS NOT NULL AND INCREMENTAL_UPDATE <> '') ";
                    lQuery+= aExcludeClause;
                    if(!lGexDbQuery.exec(lQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lGexDbQuery.lastError().text().toLatin1().constData());
                        return false;
                    }
                    lTargets.clear();
                    while(lGexDbQuery.Next())
                    {
                        lTargets << lGexDbQuery.value(0).toString();
                    }
                    if(!lTargets.isEmpty())
                    {
                        aExcludeClause += " AND "+aTarget;
                        aExcludeClause += " NOT IN ('"+lTargets.join("','")+"') ";
                    }
                }
            }
        }
    }
    return true;
}


GS::Gex::HttpChannel* GexDbPlugin_Galaxy::CreateHttpChannel(const std::string &address,
                                                            unsigned short port,
                                                            const std::string &route,
                                                            QObject *parent)
{
    if( mHttpChannel == NULL )
    {
        mHttpChannel = new GS::Gex::HttpChannel( address, port, route, parent );
    }
    else
    {
        mHttpChannel->disconnect();
        mHttpChannel->SetAddress( address );
        mHttpChannel->SetPort( port );
        mHttpChannel->SetRoute( route );
        mHttpChannel->setParent( parent );
    }

    return mHttpChannel;
}

int GexDbPlugin_Galaxy::IsConsolidationInProgress(QString /*testingStage*/, QString /*lot*/, QString /*sublots*/, QString /*wafers*/, QString /*consoType*/, QString /*testFlow*/, QString /*consoLevel*/, QString /*testInsertion*/)
{
    return 0;
}

bool GexDbPlugin_Galaxy::CheckJob(const QString &jobTitle, const QString &testingStage, const QString &lot, const QStringList &sublots, const QStringList &wafers, const QString &consoType, const QString &testFlow, const QString &consoLevel, const QString &testInsertion)
{
    QStringList splitTitle = jobTitle.split("--");
    if(splitTitle.size() != 3)
    {
        return false;
    }
    QString jobTestingStage = splitTitle[0].trimmed();
    if(jobTestingStage != testingStage)
    {
        return false;
    }
    QStringList jobLotAndWafer = splitTitle[1].split('[');
    QString jobLot = jobLotAndWafer[0].trimmed();
    if(jobLot != lot)
    {
        return false;
    }
    QString jobWafer = jobLotAndWafer[1].trimmed().remove(']');
    if(testingStage == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        if(!sublots.isEmpty() && !sublots.contains(jobWafer))
        {
            return false;
        }
    }
    else
    {
        if(!wafers.isEmpty() && !wafers.contains(jobWafer))
        {
            return false;
        }
    }
    QStringList jobDetails = splitTitle[2].trimmed().split('.');
    if(jobDetails.size() != 5)
    {
        return false;
    }
    if(!consoLevel.isEmpty() && (jobDetails[0] != consoLevel))
    {
        return false;
    }
    if(!consoType.isEmpty() && (jobDetails[1] != consoType))
    {
        return false;
    }
    if(!testInsertion.isEmpty() && (jobDetails[3] != testInsertion))
    {
        return false;
    }
    if(!testFlow.isEmpty() && (jobDetails[4] != testFlow))
    {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update DB:
// Check and update indexes for Oracle and MySql
// If lstIndexesToCheck is not empty, check indexes from it
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_UpdateIndexes(QStringList lstIndexesToCheck)
{
    QString       strQuery;
    QString       strLogMessage, strErrorMessage;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    unsigned int      uiNbIndexesCreated=0;

    // Only check indexes when recent version
    if(m_uiDbVersionBuild < GEXDB_DB_VERSION_BUILD_B17)
        return true;

    // TD-78

    /////////////////////////////////////////
    // Check if all tables with splitlot_id column have an index
    /////////////////////////////////////////
    InsertIntoUpdateLog("o Checking indexes...");


    // MySql-specific
    QString       strTable;
    QString       strIndexes;
    QString       strIndexName;
    QString       strValue;
    QStringList   lstTablesForIndexes;
    QStringList   lstTableIndexesToCheck;

    // Update db_status to record the start
    if (!AddDbUpdateSteps(eUpdateIndexes)) goto updatedb_updateindexes_error;

    // Check if have indexes to do
    // for all tables with splitlot_id column
    // Select tables with splitlot_id
    // Select indexes with splitlot_id
    // Then extract info and check needed indexes

    if(lstIndexesToCheck.isEmpty())
    {
        // Add the list of indexes for consolidated tables
        lstTableIndexesToCheck = GetIndexForConsolidatedTables();

        // For Consolidation tree
        lstTableIndexesToCheck.append("file_name,file_type,file_format|global_files");

        lstTableIndexesToCheck.append("start_t|%_splitlot");
        lstTableIndexesToCheck.append("lot_id|%_splitlot");
        lstTableIndexesToCheck.append("sublot_id|%_splitlot");
        lstTableIndexesToCheck.append("wafer_id|wt_splitlot");
        lstTableIndexesToCheck.append("wafer_id|et_splitlot");
        lstTableIndexesToCheck.append("product_name|%_splitlot");
        lstTableIndexesToCheck.append("part_typ|%_splitlot");
        lstTableIndexesToCheck.append("test_insertion_index|%_splitlot");
        lstTableIndexesToCheck.append("test_insertion|%_splitlot");
        lstTableIndexesToCheck.append("test_flow|%_splitlot");
        lstTableIndexesToCheck.append("proc_id|%_splitlot");
        lstTableIndexesToCheck.append("file_name|%_splitlot");
        lstTableIndexesToCheck.append("prod_data|%_splitlot");
        lstTableIndexesToCheck.append("incremental_update|%_splitlot");
        lstTableIndexesToCheck.append("valid_splitlot|%_splitlot");

        lstTableIndexesToCheck.append("wafer_id|%_wafer_info");
        lstTableIndexesToCheck.append("sublot_id|ft_sublot_info");

        // Add index for all tables that contains the product_name
        // except if already in the PK
        // splitlot, lot, sublot
        // wafer
        // wafer_xbin, sublot_xbin
        lstTableIndexesToCheck.append("product_name|%lot%");
        lstTableIndexesToCheck.append("product_name|%consolidation%");
        lstTableIndexesToCheck.append("final|%consolidation%");
        lstTableIndexesToCheck.append("consolidation_name|%consolidation%");
        lstTableIndexesToCheck.append("lot_id,wafer_id,sbin_cat|%wafer_sbin%");
        lstTableIndexesToCheck.append("lot_id,wafer_id,hbin_cat|%wafer_hbin%");
        lstTableIndexesToCheck.append("lot_id,sublot_id,sbin_cat|ft_sublot_sbin%");
        lstTableIndexesToCheck.append("lot_id,sublot_id,hbin_cat|ft_sublot_hbin%");

        lstTableIndexesToCheck.append("product_id,creation_date|%_sya_set");

        // For die traceability
        lstTableIndexesToCheck.append("wt_product_id|ft_die_tracking");
        lstTableIndexesToCheck.append("part_id|ft_run_dietrace");
        lstTableIndexesToCheck.append("product|ft_dietrace_config");
        lstTableIndexesToCheck.append("lot_id|ft_dietrace_config");
        lstTableIndexesToCheck.append("wafer_id|ft_dietrace_config");
        lstTableIndexesToCheck.append("die_index|ft_dietrace_config");

        // For gtl
        lstTableIndexesToCheck.append("run_id|ft_%test_rollinglimits");
        lstTableIndexesToCheck.append("run_id|ft_ptest_rollingstats");
        lstTableIndexesToCheck.append("run_id|ft_event");
        lstTableIndexesToCheck.append("run_id|ft_%test_outliers");
        lstTableIndexesToCheck.append("limits_run_id|ft_%test_outliers");

        // For run table
        // DB-37
        // Ignore useless INDEX on RUN table for HIAD
    }
    else
        lstTableIndexesToCheck = lstIndexesToCheck;

    SetMaxProgress(mProgress + lstTableIndexesToCheck.count());

    while(!lstTableIndexesToCheck.isEmpty())
    {
        IncrementProgress();
        if(m_pclDatabaseConnector->IsMySqlDB())
        {
            // LINUX
            // INDEX_NAME, COLMUN_NAME not case sensitive
            // TABLE_SCHEMA, TABLE_NAME case sensitive
            strQuery = "SELECT T0.TABLE_NAME, T0.INDEX_NAME, T0.MAX_SEQ, T0.INDEXES FROM ";
            strQuery+= " ( ";
            strQuery+= "    SELECT T.TABLE_NAME, S.INDEX_NAME, MAX(S.SEQ_IN_INDEX) AS MAX_SEQ, ";
            strQuery+= "    GROUP_CONCAT(S.COLUMN_NAME ORDER BY S.SEQ_IN_INDEX SEPARATOR ',') AS INDEXES ";
            strQuery+= "    FROM ";
            strQuery+= "         (SELECT DISTINCT COLUMNS.TABLE_SCHEMA, COLUMNS.TABLE_NAME ";
            strQuery+= "          FROM information_schema.COLUMNS ";
            strQuery+= "          INNER JOIN information_schema.TABLES ";
            strQuery+= "          ON TABLES.TABLE_SCHEMA=COLUMNS.TABLE_SCHEMA";
            strQuery+= "          AND TABLES.TABLE_NAME=COLUMNS.TABLE_NAME   ";
            strQuery+= "          WHERE COLUMNS.TABLE_SCHEMA='" + m_pclDatabaseConnector->m_strSchemaName + "' ";
            strQuery+= "          AND (COLUMNS.TABLE_NAME) LIKE '%2'";
            strQuery+= "          AND (COLUMNS.COLUMN_NAME)='%1' ";
            strQuery+= "          AND TABLES.TABLE_TYPE='BASE TABLE'";
            strQuery+= "         ) T ";
            strQuery+= "    LEFT OUTER JOIN information_schema.STATISTICS S0 ";
            strQuery+= "  ON T.TABLE_SCHEMA=S0.TABLE_SCHEMA ";
            strQuery+= "  AND T.TABLE_NAME=S0.TABLE_NAME ";
            strQuery+= "  AND (S0.COLUMN_NAME)='%1' ";
            strQuery+= "  LEFT OUTER JOIN information_schema.STATISTICS S ";
            strQuery+= "  ON T.TABLE_SCHEMA=S.TABLE_SCHEMA ";
            strQuery+= "  AND T.TABLE_NAME=S.TABLE_NAME ";
            strQuery+= "  AND S0.INDEX_NAME=S.INDEX_NAME ";
            strQuery+= "  GROUP BY T.TABLE_NAME, S.INDEX_NAME ";
            strQuery+= "  ) T0 ";
            strQuery+= " ORDER BY T0.TABLE_NAME , T0.MAX_SEQ DESC";
        }
        else if(m_pclDatabaseConnector->IsSQLiteDB())
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

        // "tracking_lot_id|wt_consolidated_wafer"
        strIndexName = lstTableIndexesToCheck.takeFirst().toLower().simplified().remove(" ");
        // wt_consolidated_wafer
        strTable = strIndexName.section("|",1,1);
        // tracking_lot_id
        strIndexName = strIndexName.section("|",0,0);
        strQuery = strQuery.arg(strIndexName.section(",",0,0),strTable);
        if(!clQuery.Execute(strQuery)) goto updatedb_updateindexes_error;
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
            if(strIndexName == "lot_id")
            {
                strIndexes = "lot_id";
            }
            else if(strIndexName == "start_t")
            {
                strIndexes = "start_t";
            }
            else if(strIndexName == "splitlot_id")
            {
                strIndexes = "splitlot_id";
                // Add ptest_info_id for test tables
                if(strTable.contains("test_",Qt::CaseInsensitive))
                    strIndexes += "," + strTable.section("_",1,1) + "_info_id";
            }
            else
                strIndexes = strIndexName;

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

            // Create new index
            // Create index TABLE_NAME on TABLE_NAME(INDEX1,INDEX2)
            strValue = strTable + "(" + strIndexes + ")";

            // Before adding this index for creation
            // Check if have table in the next result
            if(clQuery.Next())
            {
                QString strNextValue = clQuery.value(0).toString();
                clQuery.previous();
                if(strTable  == strNextValue)
                {
                    strTable = "";
                    continue;
                }
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
            strIndexName = strIndexName.replace("_INTERMEDIATE","_I")
                    .replace("_INTER","_I")
                    .replace("STATS_SUMMARY","SUM")
                    .replace("STATS_SAMPLES","SAMP")
                    .replace("TRACKING_LOT_ID","TL")
                    .replace("INSERTION_INDEX","INSERTIONINDEX")
                    .remove("_INFO_ID")
                    .remove("_INDEX")
                    .remove("_ID")
                    .remove("_NAME")
                    .remove("_NO")
                    .remove("_").replace(",","_").replace("(","_");
            strIndexName = strIndexName.toLower();

            strLogMessage = "Adding index to " + strIndexes + " ... ";
            InsertIntoUpdateLog(strLogMessage);
            // Drop index if exist
            strQuery = "DROP INDEX " + strIndexName;
            if(m_pclDatabaseConnector->IsMySqlDB())
                strQuery+= " ON " + strTable;
            clQuery.Execute(strQuery);

            // Then create a new one
            strQuery = "CREATE INDEX " + strIndexName;
            strQuery+= " ON " + strIndexes;
            if(!clQuery.Execute(strQuery)) goto updatedb_updateindexes_error;


            InsertIntoUpdateLog("DONE.",false);
            uiNbIndexesCreated++;
        }

    }

    strQuery = "";

    // Log status
    InsertIntoUpdateLog(QString(" %1 indexes updated").arg(uiNbIndexesCreated));
    InsertIntoUpdateLog(" ");

    // Update db_status to record the end
    if(!RemoveDbUpdateSteps(eUpdateIndexes)) goto updatedb_updateindexes_error;

    return true;

updatedb_updateindexes_error:
    // Write error message
    if(!strQuery.isEmpty())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clQuery.lastError().text().toLatin1().constData());
    }
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(" ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage;
    strLogMessage+= ").";
    InsertIntoUpdateLog(strLogMessage);

    return false;
}


///////////////////////////////////////////////////////////
// Insert into update log
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::InsertIntoUpdateLog(const QString & strMessage, bool bAddCR/*=true*/, bool bPlainText/*=false*/)
{
    QString strText = strMessage;
    if(!bPlainText && strMessage.startsWith("o "))
        strText = "<b>"+strMessage+"</b>";
    if(bAddCR)
    {
        if(bPlainText)
            strText = strText + "\n";
        else
            strText = "<br/>\n" + strText;
    }

    emit sLogRichMessage(strText, bPlainText);

    if(!m_strUpdateDbLogFile.isEmpty())
    {
        // Check if the log file is open
        if(!m_hUpdateDbLogFile.isOpen())
        {
            CGexSystemUtils   clSysUtils;
            clSysUtils.NormalizePath(m_strUpdateDbLogFile);
            m_hUpdateDbLogFile.setFileName(m_strUpdateDbLogFile);
            m_hUpdateDbLogFile.open(QIODevice::QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        }

        if(m_hUpdateDbLogFile.isOpen())
        {
            // Check if it is HTML mode
            if(!bPlainText)
            {
                // convert HTML to text
                strText = strText.replace( QRegExp("<[^>]*>"), "" );
                strText = strText.replace("&amp;","&").replace("&lt;","<").replace("&gt;",">");
            }

            // Add DateTime info for each new row
            if(strText.contains("\n"))
                strText = strText.replace("\n","\n["+QDateTime::currentDateTime().toString()+"] ");

            m_hUpdateDbLogFile.write(strText.toLatin1());

            m_hUpdateDbLogFile.close();
        }
    }
    QCoreApplication::processEvents();
}

///////////////////////////////////////////////////////////
// Update DB: purge selected splitlots
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::PurgeSplitlots(QStringList & strlSplitlots, QString & strTestingStage, QString & strCaseTitle, QString *pstrLog/*=NULL*/)
{
    // Check if some splitlots to be processed
    int nSplitlots = strlSplitlots.size();
    if(nSplitlots == 0)
        return true;

    SetUpdateDbLogFile("gexdb_purge_corrupted_binnings_"+QDate::currentDate().toString(Qt::ISODate)+".log");

    // Purge/consolidation on specified splitlots
    QString           strQuery, strLogMessage, strErrorMessage;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    bool              bStatus = false, bOK = false;
    QStringList       strlWafers;

    // Set Admin login
    if (!m_pclDatabaseConnector)
        return false;
    m_pclDatabaseConnector->SetAdminLogin(true);
    m_pclDatabaseConnector->Connect();

    // Write Start TimeStamp
    QTime clTime;
    clTime.start();
    InsertIntoUpdateLog(" ");
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " CORRUPTED DATABASE PURGE: " + strCaseTitle;
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);

    // Progress
    int nProcessedSplitlots=0;
    SetMaxProgress(nSplitlots);
    SetProgress(0);

    // Set testing stage
    SetTestingStage(strTestingStage);

    ////////////////////////////////////////////////////////////////
    // FOR EACH SPLITLOT:
    // 1. Save Lot_ID, Wafer_ID
    // 2. Remove splitlot from all tables
    // 3. Check if still data for this Lot/Wafer: if not, delete Lot/Wafer from consolidated wafer tables
    // 4. Check if still data for this Lot: if not, delete Lot from consolidated lot tables
    //
    // FOR each lot (for which we processed at least 1 splitlot):
    // 1. Check if still data for this Lot/Wafer:
    //        o if not, delete Lot/Wafer from consolidated wafer tables
    //        o else, call Wafer consolidation for Lot/Wafer
    // 2. Check if still data for this Lot:
    //        o if not, delete Lot from consolidated lot tables
    //        o else, call Lot consolidation for Lot
    ////////////////////////////////////////////////////////////////
    QString strSplitlotID, strLotID, strWaferID, strFilePath, strFileName, strLotWafer;
    //FIXME: not used ?
    //nSplitlotId = -1;
    clGexDbQuery.setForwardOnly(true);
    while(!strlSplitlots.isEmpty())
    {
        strSplitlotID = strlSplitlots.takeFirst();
        //FIXME: not used ?
        //nSplitlotId =
        strSplitlotID.toInt(&bOK);
        if(bOK)
        {
            // TD-78

            // 1. Save Lot_ID, Wafer_ID
            strQuery = "select lot_id, wafer_id, file_path, file_name from " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
            strQuery += " where splitlot_id=" + strSplitlotID;
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto purge_error;
            }
            clGexDbQuery.First();
            strLotID = clGexDbQuery.value(0).toString();
            strWaferID = clGexDbQuery.value(1).toString();
            strFilePath = clGexDbQuery.value(2).toString();
            strFileName = clGexDbQuery.value(3).toString();

            // Log purge info
            strLogMessage = "Purging splitlot " + strSplitlotID;
            strLogMessage += " (LotID=" + strLotID;
            strLogMessage += ", WaferID=" + strWaferID;
            strLogMessage += ", FilePath=" + strFilePath;
            strLogMessage += ", FileName=" + strFileName;
            strLogMessage += ")";
            InsertIntoUpdateLog(strLogMessage);

            // 2. Remove splitlot from all tables
            strQuery = "update " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
            strQuery += " set valid_splitlot='N' where splitlot_id=" + strSplitlotID;
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto purge_error;
            }
            strQuery = "update " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
            strQuery += " set insertion_time=(insertion_time-200) where splitlot_id=" + strSplitlotID;
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto purge_error;
            }

            // Call PURGE_INVALID_SPLITLOTS procedure
            QString   strStatus;
            strQuery = "CALL " + NormalizeTableName("PURGE_INVALID_SPLITLOTS",false) + "()";
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());

                // Restore
                strQuery = "update " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
                strQuery += " set valid_splitlot='Y' where splitlot_id=" + strSplitlotID;
                clGexDbQuery.Execute(strQuery);
                goto purge_error;
            }

            // 3 & 4: remove from consolidated tables if no more splitlots
            ConsolidateWafer(strLotID, strWaferID, false);

            strLogMessage = "DONE.";
            InsertIntoUpdateLog(strLogMessage);

            // Save Lot/Wafer into lot list
            strLotWafer = strLotID + "|" + strWaferID;
            if(!strlWafers.contains(strLotWafer))
                strlWafers.append(strLotWafer);
        }

        // Increment processed splitlots, and update progress bar
        SetProgress(++nProcessedSplitlots);
        QCoreApplication::processEvents();
    }

    // For each LOT:
    // 1. Check if still data for this Lot/Wafer:
    //        o if not, delete Lot/Wafer from consolidated wafer tables
    //        o else, call Wafer consolidation for Lot/Wafer
    // 2. Check if still data for this Lot:
    //        o if not, delete Lot from consolidated lot tables
    //        o else, call Lot consolidation for Lot
    strLogMessage = "Executing LOT consolidation...";
    InsertIntoUpdateLog(strLogMessage);

    int nIndex;
    for(nIndex=0; nIndex<strlWafers.size(); nIndex++)
    {
        strLotWafer = strlWafers.at(nIndex);
        strLotID = strLotWafer.section('|', 0, 0);
        strWaferID = strLotWafer.section('|', 1, 1);
        if(!ConsolidateWafer(strLotID, strWaferID, strCaseTitle, pstrLog))
            goto purge_error;
    }

    strLogMessage = "DONE.";
    InsertIntoUpdateLog(strLogMessage);

    // unlock table
    clGexDbQuery.Execute("COMMIT"); // the only way for unlock table

    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("");

    // Sucess
    bStatus = true;
    strLogMessage = "Status = SUCCESS: " + QString::number(nProcessedSplitlots);
    strLogMessage += " splitlots purged!";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    goto purge_writelog;

purge_error:
    // Write error message
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

purge_writelog:
    // Set progress bar to 100%
    SetProgress(nSplitlots);
    QCoreApplication::processEvents();

    // Set user login
    m_pclDatabaseConnector->SetAdminLogin(false);
    m_pclDatabaseConnector->Connect();

    // Write Stop TimeStamp
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " => STOPPING PURGE (";
    strLogMessage += QString::number((double)clTime.elapsed()/60000.0, 'f', 1) + " min).";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);


    strLogMessage = "Corrupted binnings purge saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

// Get MySQL storage engine name
bool GexDbPlugin_Galaxy::GetStorageEngineName(QString & strStorageEngine, QString & strStorageFormat)
{
    QString   strQuery;

    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Clear storage engine string
    strStorageEngine = "";
    strStorageFormat = "";

    // Check DB type
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        // Default is MyISAM
        strStorageEngine = "MyISAM";
        strQuery = "select option_value from "+NormalizeTableName("global_options",false)+" where option_name='GEXDB_MYSQL_ENGINE'";
        if(!clQuery.Execute(strQuery))
            return false;

        if(clQuery.First())
        {
            strStorageEngine = clQuery.value(0).toString();

            // Default is empty
            strStorageFormat = "";
            strQuery = "select option_value from "+NormalizeTableName("global_options",false)+" where option_name='GEXDB_MYSQL_ROWFORMAT'";
            if(!clQuery.Execute(strQuery))
                return false;

            if(clQuery.First() && !clQuery.value(0).toString().isEmpty())
                strStorageFormat = "ROW_FORMAT="+clQuery.value(0).toString();

        }
        else
        {
            // case 4663: option can be delete after a HouseKeeping update
            // Check if GexDb B12
            // Then this option must exist
            // Get the engine for ft_ptest_results
            // Insert the result for GEXDB_MYSQL_ENGINE
            strQuery = "select db_version_build from "+NormalizeTableName("global_info",false)+"";
            if(!clQuery.Execute(strQuery) || !clQuery.First())
                return false;
            if(clQuery.value(0).toInt() < 12)
                return true;

            strQuery = "select ENGINE, ROW_FORMAT from information_schema.tables where table_name='ft_ptest_results'";
            if(!clQuery.Execute(strQuery))
                return false;

            if(clQuery.First())
            {
                if(clQuery.value(0).toString().toLower() == "innodb")
                    strStorageEngine = "InnoDB";
                QString strCompressed;
                if((strStorageEngine == "InnoDB") && (clQuery.value(1).toString().toLower() == "compressed"))
                    strCompressed = " Compressed";

                // Update global_options
                strQuery = "insert into "+NormalizeTableName("global_options",false)+" values('GEXDB_MYSQL_ENGINE','"+strStorageEngine+"')";
                clQuery.Execute(strQuery);

                if(!strCompressed.isEmpty())
                {
                    strQuery = "insert into "+NormalizeTableName("global_options",false)+" values('GEXDB_MYSQL_ROWFORMAT','"+strCompressed+"')";
                    clQuery.Execute(strQuery);
                }

            }
        }
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Database restriction security
//////////////////////////////////////////////////////////////////////
bool  GexDbPlugin_Galaxy::GetSecuredMode(QString &strSecuredMode)
{
    strSecuredMode = "PUBLIC";

    if (!m_pclDatabaseConnector)
        return false;

    //if(m_strSecuredMode.isEmpty())
    GetGlobalOptionValue(eDatabaseRestrictionMode,m_strSecuredMode);

    strSecuredMode = m_strSecuredMode;
    return true;
}

bool  GexDbPlugin_Galaxy::UpdateSecuredMode(QString strSecuredMode)
{
    m_strSecuredMode = "";
    return SetGlobalOptionValue(eDatabaseRestrictionMode,strSecuredMode);
}


//////////////////////////////////////////////////////////////////////
// Consolidate all wafers for specified lot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConsolidateWafers(QString & strLotID, QString & strCaseTitle, QString *pstrLog/*=NULL*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString           strQuery, strWaferID;

    // Set Admin login
    m_pclDatabaseConnector->SetAdminLogin(true);
    m_pclDatabaseConnector->Connect();

    // Set testing stage
    SetTestingStage(eWaferTest);

    // TD-78

    strQuery = "select distinct wafer_id from " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
    strQuery += " where lot_id='" + strLotID;
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    bool bStatus = true;
    while(clGexDbQuery.Next() && bStatus)
    {
        strWaferID = clGexDbQuery.value(0).toString();
        bStatus = ConsolidateWafer(strLotID, strWaferID, strCaseTitle, pstrLog);
    }

    // Set User login
    m_pclDatabaseConnector->SetAdminLogin(false);
    m_pclDatabaseConnector->Connect();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConsolidateWafer(QString& strLotID,
                                          QString& strWaferID,
                                          QString& strCaseTitle,
                                          QString* /*pstrLog = NULL*/)
{
    // Purge/consolidation on specified splitlots
    QString       strLogMessage, strErrorMessage;
    if (!m_pclDatabaseConnector)
        return false;

    SetUpdateDbLogFile("gexdb_wafer_consolidation_"+QDate::currentDate().toString(Qt::ISODate)+".log");

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    bool              bStatus = false;

    // Set Admin login
    m_pclDatabaseConnector->SetAdminLogin(true);
    m_pclDatabaseConnector->Connect();

    // Write Start TimeStamp
    QTime clTime;
    clTime.start();
    InsertIntoUpdateLog(" ");
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " WAFER CONSOLIDATION: " + strCaseTitle;
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "Consolidating wafer";
    strLogMessage += " (LotID=" + strLotID;
    strLogMessage += ", WaferID=" + strWaferID;
    strLogMessage += ")";
    InsertIntoUpdateLog(strLogMessage);

    // Set testing stage
    SetTestingStage(eWaferTest);

    ////////////////////////////////////////////////////////////////
    // FOR EACH SPLITLOT:
    // 1. Check if still data for this Lot/Wafer:
    //        o if not, delete Lot/Wafer from consolidated wafer tables
    //        o else, call Wafer consolidation for Lot/Wafer
    // 2. Check if still data for this Lot:
    //        o if not, delete Lot from consolidated lot tables
    //        o else, call Lot consolidation for Lot
    ////////////////////////////////////////////////////////////////
    if(!ConsolidateWafer(strLotID, strWaferID))
        goto consolidation_error;

    // unlock table
    clGexDbQuery.Execute("COMMIT"); // the only way for unlock table

    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("");

    // Sucess
    bStatus = true;
    strLogMessage = "Status = SUCCESS!" ;
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    goto consolidation_writelog;

consolidation_error:
    // Write error message
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

consolidation_writelog:
    // Write Stop TimeStamp
    strLogMessage = QDateTime::currentDateTime().toString(Qt::ISODate);
    strLogMessage += " => STOPPING CONSOLIDATION (";
    strLogMessage += QString::number((double)clTime.elapsed()/60000.0, 'f', 1) + " min).";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    // Set User login
    m_pclDatabaseConnector->SetAdminLogin(false);
    m_pclDatabaseConnector->Connect();

    strLogMessage = "Wafer consolidation saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(strLogMessage);

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConsolidateWafer(QString & strLotID, QString & strWaferID, bool bCallConsolidationFunction/*=true*/)
{
    Q_UNUSED(strLotID)
    Q_UNUSED(strWaferID)
    Q_UNUSED(bCallConsolidationFunction)

    /*
    QString strQuery;

    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // TD-78

    // 1. Check if still data for this Lot/Wafer:
    //        o if not, delete Lot/Wafer from consolidated wafer tables
    //        o else, call Wafer consolidation for Lot/Wafer
    strQuery = "select count(splitlot_id) from " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
    strQuery += " where lot_id='" + strLotID;
    strQuery += "' and wafer_id='" + strWaferID;
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clGexDbQuery.First();
    if(clGexDbQuery.value(0).toInt() > 0)
    {
        if(bCallConsolidationFunction)
        {
            /////////////////////////////////////////
            if(!UpdateWaferConsolidation(strLotID,strWaferID))
                return false;
        }
    }
    else
    {

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"consolidated_wafer",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "' and wafer_id='" + strWaferID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"wafer_info",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "' and wafer_id='" + strWaferID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"wafer_sbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "' and wafer_id='" + strWaferID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"wafer_hbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "' and wafer_id='" + strWaferID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }

    // 2. Check if still data for this Lot:
    //        o if not, delete Lot from consolidated lot tables
    //        o else, call Lot consolidation for Lot
    if(!ConsolidateLot(strLotID, false,bCallConsolidationFunction))
        return false;
    */
    return true;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified lot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConsolidateLot(QString & strProductName, QString & strTrackingLotID, QString & strLotID, bool bCallConsolidationFunction/*=true*/)
{
    m_strProductName = strProductName;
    m_strTrackingLotId = strTrackingLotID;
    return ConsolidateLot(strLotID, false,bCallConsolidationFunction);
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified lot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConsolidateLot(QString & strLotID, bool bConsolidateOnlySBinTable/*=false*/, bool bCallConsolidationFunction/*=true*/)
{
    QString strQuery;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Init variables
    if(m_eTestingStage == eUnknownStage)
        SetTestingStage(eFinalTest);

    // TD-78

    // 1. Check if still data for this Lot:
    //        o if not, delete Lot from consolidated lot tables
    //        o else, call Lot consolidation for Lot
    strQuery = "select count(splitlot_id) from " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
    strQuery += " where lot_id='" + strLotID;
    strQuery += "'";
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    clGexDbQuery.First();
    if(clGexDbQuery.value(0).toInt() > 0)
    {
        // TD-78

        QString strSubLot;
        strQuery = "select sublot_id from " + NormalizeTableName(m_strTablePrefix+"splitlot",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

//        if(bConsolidateOnlySBinTable)
//        {
//            while(clGexDbQuery.next())
//            {
//                strSubLot = clGexDbQuery.value(0).toString();
//                if(!UpdateFtSubLotSBinTable(strLotID,strSubLot))
//                    return false;
//            }
//            // Consolidate SBin
//            if(!UpdateFtLotSBinTable(strLotID))
//                return false;
//        }
//        else
        if(bCallConsolidationFunction || bConsolidateOnlySBinTable)
        {
            while(clGexDbQuery.next())
            {
                strSubLot = clGexDbQuery.value(0).toString();
                if(!UpdateSubLotConsolidation(strLotID,strSubLot))
                    return false;
            }
            /////////////////////////////////////////
            if(!UpdateLotConsolidation(strLotID))
                return false;
        }
    }
    else
    {
        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"lot",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"lot_sbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"lot_hbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "update " + NormalizeTableName(m_strTablePrefix+"sublot_info",false);
        strQuery+= " set nb_parts=0, nb_parts_good=0";
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"sublot_sbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";

        // TD-78

        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // TD-78

        strQuery = "delete from " + NormalizeTableName(m_strTablePrefix+"sublot_hbin",false);
        strQuery += " where lot_id='" + strLotID;
        strQuery += "'";
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Drop table if exist
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_DropTableIfExists(const QString & strTableName)
{
    if (!m_pclDatabaseConnector)
        return false;

    QString           strLogMessage, strQuery;
    QStringList       strlTables;
    GexDbPlugin_Query clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    //strLogMessage = "Checking if " + strTableName + " table exists.";
    //InsertIntoUpdateLog(strLogMessage);
    m_pclDatabaseConnector->EnumTables(strlTables);
    if(strlTables.contains(strTableName, Qt::CaseInsensitive))
    {
        strLogMessage = "Dropping table "+strTableName+"...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TABLE "+strTableName;
        clQuery.Execute(strQuery);
        InsertIntoUpdateLog("DONE.", false);
    }
    else
    {
        //strLogMessage = strTableName + " table doesn't exists.";
        //InsertIntoUpdateLog(strLogMessage);
    }

    return true;
}

bool GexDbPlugin_Galaxy::GetDbUpdateSteps(unsigned int & uiFlags)
{
    uiFlags = 0;
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString             strQuery, strDbUpdateSteps,strMessage;

    strMessage = "Retrieve remaining database update steps...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());

    strQuery = "SELECT db_status FROM "+NormalizeTableName("global_info",false)+"";
    if (clQuery.Execute(strQuery))
    {
        if (clQuery.First())
        {
            strMessage = "";
            strDbUpdateSteps = clQuery.value(0).toString();
            foreach(const QString &strDbUpdateStep, strDbUpdateSteps.split("|"))
            {
                if(strDbUpdateStep.simplified().isEmpty())
                    continue;

                if (strDbUpdateStep == GEXDB_DB_UPDATE_DB)
                    uiFlags |= eUpdateDb;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_CONS_TREE)
                    uiFlags |= eUpdateConsTree;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_CONS_OLD)
                    uiFlags |= eUpdateConsOld;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_CONS_TRIGGERS)
                    uiFlags |= eUpdateConsTriggers;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_CONS_TABLES)
                    uiFlags |= eUpdateConsTables;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_CONS_PROCEDURES)
                    uiFlags |= eUpdateConsProcedures;
                else if (strDbUpdateStep == GEXDB_DB_UPDATE_INDEXES)
                    uiFlags |= eUpdateIndexes;
                else
                {
                    uiFlags |= eUpdateUnknown;
                    GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, strDbUpdateStep.toLatin1().constData());
                }
            }
        }
        else
        {
            // the table is empty error
            GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbStatus, NULL, QString("Global_info table is empty...").toLatin1().constData());
            return false;
        }
    }
    else
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    strMessage += ((uiFlags & eUpdateDb) ? "Update DB" : "");
    strMessage += ((uiFlags & eUpdateConsOld) ? " Old Update Consolidation Process" : "");
    strMessage += ((uiFlags & eUpdateConsTree) ? " Update Consolidation Tree" : "");
    strMessage += ((uiFlags & eUpdateConsTriggers) ? " Update Consolidation Triggers" : "");
    strMessage += ((uiFlags & eUpdateConsTables) ? " Update Consolidation Tables" : "");
    strMessage += ((uiFlags & eUpdateConsProcedures) ? " Update Consolidation Procedures" : "");
    strMessage += ((uiFlags & eUpdateIndexes) ? " Update Indexes" : "");
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());
    return true;
}

bool GexDbPlugin_Galaxy::AddDbUpdateSteps(unsigned int uiFlags)
{
    if (!m_pclDatabaseConnector || (uiFlags == 0))
        return false;

    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString             strQuery, strDbUpdateSteps;
    unsigned int        uiCurrentUpdateSteps = 0;

    // Get current update db steps
    if (!GetDbUpdateSteps(uiCurrentUpdateSteps))
        return false;

    // Set old flag to new values
    if ((uiFlags & eUpdateConsOld) || (uiCurrentUpdateSteps & eUpdateConsOld))
        uiCurrentUpdateSteps |= eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures;

    // Set or keep if already set
    if ((uiFlags & eUpdateDb) || (uiCurrentUpdateSteps & eUpdateDb))
        strDbUpdateSteps = QString(GEXDB_DB_UPDATE_DB);
    // Set or keep if already set
    if ((uiFlags & eUpdateConsTree) || (uiCurrentUpdateSteps & eUpdateConsTree))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TREE);
    }
    // Set or keep if already set
    if ((uiFlags & eUpdateConsTriggers) || (uiCurrentUpdateSteps & eUpdateConsTriggers))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TRIGGERS);
    }
    // Set or keep if already set
    if ((uiFlags & eUpdateConsTables) || (uiCurrentUpdateSteps & eUpdateConsTables))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TABLES);
    }
    // Set or keep if already set
    if ((uiFlags & eUpdateConsProcedures) || (uiCurrentUpdateSteps & eUpdateConsProcedures))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_PROCEDURES);
    }
    // Set or keep if already set
    if ((uiFlags & eUpdateIndexes) || (uiCurrentUpdateSteps & eUpdateIndexes))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_INDEXES);
    }

    // Update DB
    strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET db_status=" + TranslateStringToSqlVarChar(strDbUpdateSteps);
    if (!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool GexDbPlugin_Galaxy::RemoveDbUpdateSteps(unsigned int uiFlags)
{
    if (!m_pclDatabaseConnector || (uiFlags == 0))
        return false;

    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString             strQuery, strDbUpdateSteps;
    unsigned int        uiCurrentUpdateSteps = 0;

    // Get current update db steps
    if (!GetDbUpdateSteps(uiCurrentUpdateSteps))
        return false;

    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateDb) && (uiCurrentUpdateSteps & eUpdateDb))
        strDbUpdateSteps = QString(GEXDB_DB_UPDATE_DB);
    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateConsOld) && (uiCurrentUpdateSteps & eUpdateConsOld))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : ""); //separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_OLD);
    }
    if (!(uiFlags & eUpdateConsTree) && (uiCurrentUpdateSteps & eUpdateConsTree))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : ""); //separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TREE);
    }
    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateConsTriggers) && (uiCurrentUpdateSteps & eUpdateConsTriggers))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : ""); //separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TRIGGERS);
    }
    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateConsTables) && (uiCurrentUpdateSteps & eUpdateConsTables))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : ""); //separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_TABLES);
    }
    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateConsProcedures) && (uiCurrentUpdateSteps & eUpdateConsProcedures))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : ""); //separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_CONS_PROCEDURES);
    }
    // keep if have not to unset and was already set
    if (!(uiFlags & eUpdateIndexes) && (uiCurrentUpdateSteps & eUpdateIndexes))
    {
        strDbUpdateSteps.append(!strDbUpdateSteps.isEmpty() ? "|" : "");//separator
        strDbUpdateSteps.append(GEXDB_DB_UPDATE_INDEXES);
    }
    // Update DB
    strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET db_status=" + TranslateStringToSqlVarChar(strDbUpdateSteps);
    if (!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool GexDbPlugin_Galaxy::ExecuteSqlScript(const QString & /*fileName*/)
{
    return false;
}

////////////////////////////////////////////////////////////////////////
/// \brief GexDbPlugin_Galaxy::UpdateDb_CheckServerVersion
/// \param databaseType
/// \return true if the Sql server is configured for the databaseType
/// For TDR (Manual, Prod, Charac)
/// * Partition (MANDATORY)
/// * Spider (INFO)
/// * TokuDB (INFO)
/// For ADR and Prod TDR:
/// * Dynamic column (MariaDB 10.0.1)
////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_CheckServerVersion(DataBaseType databaseType, QString lSqlConnection)
{
    QString lConnectionName = lSqlConnection;
    if(lConnectionName.isEmpty())
    {
        if(m_pclDatabaseConnector == NULL)
            return false;
        lConnectionName = m_pclDatabaseConnector->m_strConnectionName;
    }

    GexDbPlugin_Query   lSqlQuery(this, QSqlDatabase::database(lConnectionName));
    QString             lQuery;

    QString lMessage, lValue;

    QString lServerType = "MYSQL";
    QString lServerVersion = "5.5.0";
    bool    lCheckDynamicColumn = false;

    switch (databaseType)
    {
    case GexDbPlugin_Galaxy::eProdTdrDb :
    case GexDbPlugin_Galaxy::eAdrDb:
        lServerType = "MARIADB";
        lServerVersion = "10.0.1";
        lCheckDynamicColumn = true;
        break;
    default:
        lServerType = "MYSQL";
        lServerVersion = "5.5.0";
        break;
    }

    if(!m_pclDatabaseConnector->IsMySqlDB())
    {
        InsertIntoUpdateLog(" - MYSQL VERSION = ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED SERVER \n<br>";
        lMessage+= "o "+m_pclDatabaseConnector->GetFriendlyName(m_pclDatabaseConnector->m_strDriver)+" \n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - NEED "+lServerType+" "+lServerVersion+" OR after";
        InsertIntoUpdateLog(lMessage);
        // Fatal error
        return false;
    }



    InsertIntoUpdateLog("o Check the MySql Server configuration");
    // Check if have MySql 5.5 or after
    lQuery = "SELECT @@version";
    if(!lSqlQuery.exec(lQuery) || !lSqlQuery.first())
    {
        lMessage = "Error executing SQL query.\n<br>";
        lMessage+= "QUERY=" + lQuery + "\n<br>";
        lMessage+= "ERROR=" + lSqlQuery.lastError().text();
        InsertIntoUpdateLog(lMessage);
        return false;
    }

    // 5.5.28-community
    lValue = lSqlQuery.value(0).toString().section("-",0,0).simplified();
    bool lNeedNewestVersion = false;
    // Check the MAJOR
    if(lValue.section(".",0,0).toInt() < lServerVersion.section(".",0,0).toInt())
        lNeedNewestVersion = true;
    // Check the MINOR if same MAJOR
    else if((lValue.section(".",0,0).toInt() == lServerVersion.section(".",0,0).toInt())
            && (lValue.section(".",1,1).toInt() < lServerVersion.section(".",1,1).toInt()))
        lNeedNewestVersion = true;
    // Check the BUILD if same MAJOR.MINOR
    else if((lValue.section(".",0,0).toInt() == lServerVersion.section(".",0,0).toInt())
            && (lValue.section(".",1,1).toInt() == lServerVersion.section(".",1,1).toInt())
            && (lValue.section(".",2,2).toInt() < lServerVersion.section(".",2,2).toInt()))
        lNeedNewestVersion = true;
    if(lNeedNewestVersion)
    {
        InsertIntoUpdateLog(" - MYSQL VERSION = "+lSqlQuery.value(0).toString()+" ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED MYSQL VERSION\n<br>";
        lMessage+= "o MYSQL VERSION = "+lSqlQuery.value(0).toString()+ "\n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - NEED "+lServerType+" "+lServerVersion+" OR after";
        InsertIntoUpdateLog(lMessage);
        // Fatal error
        return false;
    }
    else
        InsertIntoUpdateLog(" - MYSQL VERSION = "+lSqlQuery.value(0).toString()+" ... PASS");

    if(lCheckDynamicColumn)
    {
        // Check if have DYNAMIC_COLUMN features
        lQuery = "SELECT COLUMN_CHECK(null)";
        if(!lSqlQuery.exec(lQuery))
        {
            InsertIntoUpdateLog(" - MYSQL DYNAMIC_COLUMN FEATURE ... FAIL");
            lMessage = "*************************\n<br>";
            lMessage+= "o UNSUPPORTED MYSQL VERSION\n<br>";
            lMessage+= "o MYSQL DYNAMIC_COLUMN FEATURE disabled\n<br>";
            lMessage+= "*************************\n<br>";
            lMessage+= " - NEED "+lServerType+" "+lServerVersion+" OR after";
            InsertIntoUpdateLog(lMessage);
            // Fatal error
            return false;
        }
        else
            InsertIntoUpdateLog(" - MYSQL DYNAMIC_COLUMN FEATURE enabled ... PASS");
    }

    QStringList lLstUpdate;
    QString lMySqlIniFile;
    QString lMySqlLogErrorFile;
    // Find the MySql my.ini config file
    lQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    lQuery+= "   WHERE VARIABLE_NAME = 'BASEDIR'";
    if(lSqlQuery.exec(lQuery) && lSqlQuery.first())
    {
        if(QFileInfo(lSqlQuery.value(0).toString()+"my.ini").exists())
            lMySqlIniFile = lSqlQuery.value(0).toString()+"my.ini";
        else if (QFileInfo(lSqlQuery.value(0).toString()+"my.cnf").exists())
            lMySqlIniFile = lSqlQuery.value(0).toString()+"my.cnf";
    }
    if(lMySqlIniFile.isEmpty())
    {
        lQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        lQuery+= "   WHERE VARIABLE_NAME = 'DATADIR'";
        if(lSqlQuery.exec(lQuery) && lSqlQuery.first())
        {
            if(QFileInfo(lSqlQuery.value(0).toString()+"my.ini").exists())
                lMySqlIniFile = lSqlQuery.value(0).toString()+"my.ini";
            else if (QFileInfo(lSqlQuery.value(0).toString()+"my.cnf").exists())
                lMySqlIniFile = lSqlQuery.value(0).toString()+"my.cnf";
        }
    }
    if(lMySqlIniFile.isEmpty())
        lMySqlIniFile = "BASEDIR/my.ini or BASEDIR/my.cnf";

    // Get the MySql LogError
    lQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    lQuery+= "   WHERE VARIABLE_NAME = 'LOG_ERROR'";
    if(lSqlQuery.exec(lQuery) && lSqlQuery.first())
        lMySqlLogErrorFile = lSqlQuery.value(0).toString();

    // Check if have INNODB engine
    lQuery = "SELECT support FROM information_schema.engines";
    lQuery+= "   WHERE UPPER(engine) = 'INNODB'";
    if(!lSqlQuery.exec(lQuery) || !lSqlQuery.first())
    {
        lMessage = "Error executing SQL query.\n<br>";
        lMessage+= "QUERY=" + lQuery + "\n<br>";
        lMessage+= "ERROR=" + lSqlQuery.lastError().text();
        InsertIntoUpdateLog(lMessage);
        return false;
    }
    // YES
    lValue = lSqlQuery.value(0).toString();
    if(!lValue.startsWith("YES",Qt::CaseInsensitive)
            && !lValue.startsWith("DEFAULT",Qt::CaseInsensitive))
    {
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'HAVE_INNODB' = "+lSqlQuery.value(0).toString()+" ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED MYSQL INNODB ENGINE\n<br>";
        lMessage+= "o MYSQL PLUGIN = 'INNODB' disabled\n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - NEED MYSQL INNODB ENGINE\n<br>";
        lMessage+= "o Check your MySql config file:\n<br>";
        lMessage+= " - "+lMySqlIniFile+"\n<br>";
        lMessage+= " - If skip-innodb is uncommented, try commenting it out and restarting mysqld.\n<br>";
        lMessage+= "o Check your MySql error log file:\n<br>";
        lMessage+= " - "+lMySqlLogErrorFile+"\n<br>";
        lMessage+= " - InnoDB would be disabled when it fail to start, please check your error log.";

        lLstUpdate.append(lMessage);
    }
    else
        InsertIntoUpdateLog(" - MYSQL PLUGIN 'INNODB' = "+lSqlQuery.value(0).toString()+" ... PASS");

    // Check if have INNODB_FILE_PER_TABLE
    lQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    lQuery+= "   WHERE VARIABLE_NAME = 'INNODB_FILE_PER_TABLE'";
    if(!lSqlQuery.exec(lQuery) || !lSqlQuery.first())
    {
        lMessage = "Error executing SQL query.\n<br>";
        lMessage+= "QUERY=" + lQuery + "\n<br>";
        lMessage+= "ERROR=" + lSqlQuery.lastError().text();
        InsertIntoUpdateLog(lMessage);
        return false;
    }
    // ON
    lValue = lSqlQuery.value(0).toString();
    if(!lValue.startsWith("ON",Qt::CaseInsensitive))
    {
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'INNODB_FILE_PER_TABLE' = "+lSqlQuery.value(0).toString()+" ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED MYSQL DATA FILE MANAGMENT\n<br>";
        lMessage+= "o MYSQL GLOBAL_VARIABLES = 'INNODB_FILE_PER_TABLE' disabled\n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - This option must be enabled in order to create the Quantix DB\n<br>";
        lMessage+= "o Check your MySql config file:\n<br>";
        lMessage+= " - "+lMySqlIniFile+"\n<br>";
        lMessage+= " - add a line to the [mysqld] section.\n<br>";
        lMessage+= "   [mysqld]\n<br>";
        lMessage+= "   innodb_file_per_table\n<br>";
        lMessage+= "o Check your MySql error log file:\n<br>";
        lMessage+= " - "+lMySqlLogErrorFile+"\n<br>";
        lMessage+= " - InnoDB would be disabled when it fail to start, please check your error log.\n<br>";

        lLstUpdate.append(lMessage);
    }
    else
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'INNODB_FILE_PER_TABLE' = "+lSqlQuery.value(0).toString()+" ... PASS");

    // Check if have partitioning
    lQuery = "SELECT plugin_status FROM information_schema.plugins";
    lQuery+= "   WHERE UPPER(plugin_name) = 'PARTITION'";
    if(!lSqlQuery.exec(lQuery) || !lSqlQuery.first())
    {
        lMessage = "Error executing SQL query.\n<br>";
        lMessage+= "QUERY=" + lQuery + "\n<br>";
        lMessage+= "ERROR=" + lSqlQuery.lastError().text();
        InsertIntoUpdateLog(lMessage);
        return false;
    }
    // YES
    lValue = lSqlQuery.value(0).toString();
    if(!lValue.startsWith("YES",Qt::CaseInsensitive)
            && !lValue.startsWith("ACTIVE",Qt::CaseInsensitive))
    {
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'HAVE_PARTITIONING' = "+lSqlQuery.value(0).toString()+" ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED MYSQL PARTITION MANAGMENT\n<br>";
        lMessage+= "o MYSQL PLUGIN = 'PARTITION' disabled\n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - This option must be enabled in order to create the Quantix DB\n<br>";
        lMessage+= "o Check your MySql error log file:\n<br>";
        lMessage+= " - "+lMySqlLogErrorFile+"\n<br>";
        lMessage+= " - your version of MySQL was not built with partitioning support.";

        lLstUpdate.append(lMessage);
    }
    else
        InsertIntoUpdateLog(" - MYSQL PLUGIN = 'PARTITION' = "+lSqlQuery.value(0).toString()+" ... PASS");

    // Check if have EVENT_SCHEDULER option
    // min sql version is v5.1.6 for this feature
    lQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    lQuery+= "   WHERE VARIABLE_NAME = 'EVENT_SCHEDULER'";
    if(!lSqlQuery.exec(lQuery) || !lSqlQuery.first())
    {
        lMessage = "Error executing SQL query.\n<br>";
        lMessage+= "QUERY=" + lQuery + "\n<br>";
        lMessage+= "ERROR=" + lSqlQuery.lastError().text();
        InsertIntoUpdateLog(lMessage);
        return false;
    }

    // ON
    lValue = lSqlQuery.value(0).toString();
    if((!lValue.startsWith("ON",Qt::CaseInsensitive))
            && (!lValue.startsWith("YES",Qt::CaseInsensitive))
            && (!lValue.startsWith("1",Qt::CaseInsensitive))
            && (!lValue.startsWith("ENAB",Qt::CaseInsensitive)))
    {
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'EVENT_SCHEDULER' = "+lSqlQuery.value(0).toString()+" ... FAIL");
        lMessage = "*************************\n<br>";
        lMessage+= "o UNSUPPORTED MYSQL EVENT SCHEDULER OPTION\n<br>";
        lMessage+= "o MYSQL GLOBAL_VARIABLES = 'EVENT_SCHEDULER' disabled\n<br>";
        lMessage+= "*************************\n<br>";
        lMessage+= " - NEED MYSQL EVENT_SCHEDULER OPTION\n<br>";
        lMessage+= "o Check your MySql config file:\n<br>";
        lMessage+= " - "+lMySqlIniFile+"\n<br>";
        lMessage+= " - add a line to the [mysqld] section.\n<br>";
        lMessage+= "   [mysqld]\n<br>";
        lMessage+= "   event_scheduler = 1\n<br>";
        lMessage+= "o Check your MySql error log file:\n<br>";
        lMessage+= " - "+lMySqlLogErrorFile+"\n<br>";
        lMessage+= " - EVENT_SCHEDULER would be disabled when it fail to start, please check your error log.\n";

        lLstUpdate.append(lMessage);
    }
    else
        InsertIntoUpdateLog(" - MYSQL GLOBAL_VARIABLES 'EVENT_SCHEDULER' = "+lSqlQuery.value(0).toString()+" ... PASS");

    if(!lLstUpdate.isEmpty())
    {
        InsertIntoUpdateLog(" ");
        InsertIntoUpdateLog("o Your MySql Server need to be updated o");
        InsertIntoUpdateLog(lLstUpdate.join("\n<br>"));
        return false;
    }
    InsertIntoUpdateLog("\n");

    return true;
}

bool GexDbPlugin_Galaxy::UpdateDb_FromSqlScript(UINT fromBuild, UINT toBuild, const GexDbPlugin_Connector& rootConnector)
{
    if(fromBuild < 25)
        return false;
    if(toBuild > GEXDB_DB_VERSION_BUILD)
        return false;

    if(!UpdateDb_CheckServerVersion(m_eDbType))
        return false;

    if(!rootConnector.IsMySqlDB())
        return false;

    QDir lDir;
    QString lFileTemplate, lMessage, lValue;
    QStringList lUpdateFiles, lSqlFilesToProcess, lSqlUpdateFiles;
    QFile       lFileToSource;
    QTextStream lSqlStream(&lFileToSource);
    QMap<UINT, QString> lUpdatesMap;
    UINT lVersion;

    // Get the list of update files
    QString lUpdateFilesPath = m_strApplicationPath + QDir::separator();
    lUpdateFilesPath += "install/mysql/";
    lDir.setPath(lUpdateFilesPath);

    lFileTemplate = GetUpdateScriptName(m_eDbType);

    lUpdateFiles = lDir.entryList( QStringList(lFileTemplate), QDir::Files, QDir::Name);

    // record update versions retrieved
    while(!lUpdateFiles.isEmpty())
    {
        lValue = lUpdateFiles.takeFirst();
        lVersion = lValue.toLower().section("_b",1).section("_to",0,0).toUInt();
        lUpdatesMap[lVersion] = lUpdateFilesPath + lValue;
    }

    // check if have all versions
    for(lVersion = fromBuild; lVersion < toBuild; ++lVersion)
    {
        if(!lUpdatesMap.contains(lVersion))
        {
            // error
            QString lMissingFile = lFileTemplate;
            lMissingFile.replace('*', "b" + QString::number(lVersion) + "_to_b" + QString::number(lVersion + 1));
            InsertIntoUpdateLog("<b> ***********************************************************</b>",true);
            InsertIntoUpdateLog("<b> -----------------------   WARNING   -----------------------</b>",true);
            InsertIntoUpdateLog("<b> Sql update script not found (" + lUpdateFilesPath + lMissingFile + ") !</b>",true);
            InsertIntoUpdateLog("<b> ***********************************************************</b>",true);
            return false;
        }
    }

    QString		lQueryContent, lQueryDisplay, lDelimiter, lLine;
    QSqlQuery	lQuery = QSqlQuery(QSqlDatabase::database(rootConnector.m_strConnectionName));

    // Ready to start the update
    // GCORE-11149 Background Transfer issue
    // Check if the user have the PROCESS privilege for BACKGROUND TRANSFER
    if((fromBuild<=GEXDB_DB_VERSION_BUILD_B75)
            && (m_eDbType != eAdrDb) && (m_eDbType != eAdrLocalDb))
    {
        bool lHaveProcessPrivilege = false;
        // Do not check the PROCESS privilege for ADR
        lQueryContent = "SHOW GRANTS";
        // Execute the query
        if(!lQuery.exec(lQueryContent))
        {
            lMessage = "Error executing SQL query.\n";
            lMessage+= "QUERY=" + lQueryContent + "\n";
            lMessage+= "ERROR=" + lQuery.lastError().text();
            InsertIntoUpdateLog(lMessage,true);
            return false;
        }
        while(lQuery.next())
        {
            // GRANT PROCESS, FILE ON *.* TO ...
            if(lQuery.value(0).toString().contains("PROCESS",Qt::CaseInsensitive))
                lHaveProcessPrivilege = true;
            // GRANT ALL PRIVILEGES ON *.* TO ...
            if(lQuery.value(0).toString().contains("GRANT ALL PRIVILEGES ON *.*",Qt::CaseInsensitive))
                lHaveProcessPrivilege = true;
        }
        if(!lHaveProcessPrivilege)
        {
            QString lRootConnection;
            QStringList lstUninstall;
            InsertIntoUpdateLog("o Need an update of MySql users");
            if(!CreateUpdateDatabaseUsers(rootConnector, lRootConnection, m_eDbType, lstUninstall, lMessage))
            {
                InsertIntoUpdateLog(lMessage,true);
                return false;
            }
        }
    }

    // File containing common STORED PROCEDURES/FUCTIONS required for an update
    lSqlFilesToProcess << lUpdateFilesPath + "common_update_initialize.sql";
    lSqlFilesToProcess << lUpdateFilesPath + "tdr_update_initialize.sql";
    // Updates sql scripts
    for(lVersion = fromBuild; lVersion < toBuild; ++lVersion)
    {
        lSqlFilesToProcess << lUpdatesMap[lVersion];
    }
    // File to drop common STORED PROCEDURES/FUCTIONS required for an update
    lSqlFilesToProcess << lUpdateFilesPath + "tdr_update_finalize.sql";
    lSqlFilesToProcess << lUpdateFilesPath + "common_update_finalize.sql";

    // This loop is dedicated to extract from all the script
    // the question which may need to stop and update the process
    // Before to start in the TDR update
    int lLinesCount = 0;
    QString lBuildTo;
    QStringList lUpdateInfo;
    lSqlUpdateFiles << lSqlFilesToProcess;

    QString lTopMessage, lHtmlText, lBottomMessage;

    while (!lSqlUpdateFiles.isEmpty())
    {
        lFileToSource.setFileName(lSqlUpdateFiles.takeFirst());
        if(!lFileToSource.open(QIODevice::ReadOnly))
        {
            lMessage = lFileToSource.errorString();
            InsertIntoUpdateLog(lMessage,true);
            return false;
        }

        lBuildTo = lFileToSource.fileName().section("update_",1).section(".sql",0,0);
        lUpdateInfo.clear();
        while(! lSqlStream.atEnd())
        {
            lLine = lSqlStream.readLine();
            ++lLinesCount;
            if(lLine.simplified().isEmpty())
                continue;
            if(lLine.simplified().startsWith("INFO",Qt::CaseInsensitive))
            {
                lUpdateInfo << lLine.remove("INFO:", Qt::CaseSensitive);
                continue;
            }
            if(lLine.simplified().startsWith("QUESTION",Qt::CaseInsensitive))
            {
                lBottomMessage = lLine.remove("QUESTION:", Qt::CaseSensitive);
                lUpdateInfo << "";

                if(!lBuildTo.isEmpty())
                {
                    InsertIntoUpdateLog("o Update version " + lBuildTo.replace("_"," "),true);
                    lHtmlText += "<h4>o Update version " + lBuildTo.replace("_"," ") + "</h4>\n";
                }
                InsertIntoUpdateLog(lUpdateInfo.join("<BR>\n"),true);
                lHtmlText += lUpdateInfo.join("<BR>\n");

                lBuildTo = "";
                lUpdateInfo.clear();
            }
            if(lLine.simplified().startsWith("WARNING",Qt::CaseInsensitive))
            {
                lUpdateInfo << lLine.section("WARNING:",1);

                if(!lBuildTo.isEmpty())
                {
                    InsertIntoUpdateLog("o Update version " + lBuildTo.replace("_"," "),true);
                    lHtmlText += "<h4>o Update version " + lBuildTo.replace("_"," ") + "</h4>\n";
                }
                InsertIntoUpdateLog(lUpdateInfo.join("<BR>\n"),true);
                lHtmlText += "<b><font color=\"red\">"+lUpdateInfo.join("<BR>\n")+"</font></b>";

                lBuildTo = "";
                lUpdateInfo.clear();
            }
        }
    }

    if(!lHtmlText.isEmpty())
    {
        // Generate the HTML Dialog Box
        // to display all the Message/WarningInRed
        // in once
        QDialog clDialogBox;
        QVBoxLayout clVBoxLayout;
        QHBoxLayout clHBoxLayout;
        QButtonGroup clButtonGroup;

        QLabel clTopLabel;
        QTextBrowser clHtmlText;
        QLabel clBottomLabel;
        QPushButton clButtonOne;
        QPushButton clButtonThree;

        lTopMessage = "<center><font size=\"6\" face=\"arial\" color=\"#858585\">Information about Quantix database update</font>\n";
        lTopMessage+= "<hr color=\"#afc22a\"></center>";

        lTopMessage+= "Information about this update";
        lTopMessage+= ".<br>\nClick the 'Next' button to continue:\n";

        if(lBottomMessage.isEmpty())
        {
            lBottomMessage+= "<h4>Continue ?</h4>";
        }

        clHtmlText.setReadOnly(true);
        clHtmlText.setAcceptDrops(false);
        clHtmlText.setProperty("textInteractionFlags",Qt::NoTextInteraction);
        clHtmlText.setLineWrapMode(QTextEdit::NoWrap);
        clDialogBox.setLayout(&clVBoxLayout);
        clDialogBox.setWindowTitle("Quantix Database Update");
        clTopLabel.setText(lTopMessage);
        clVBoxLayout.addWidget(&clTopLabel);
        clHtmlText.setText(lHtmlText);
        clVBoxLayout.addWidget(&clHtmlText);
        clBottomLabel.setText(lBottomMessage);
        clVBoxLayout.addWidget(&clBottomLabel);

        clButtonOne.setText("Next");
        clButtonThree.setText("Cancel");

        clButtonGroup.addButton(&clButtonOne, QMessageBox::YesToAll);
        clButtonGroup.addButton(&clButtonThree,QMessageBox::Cancel);

        clHBoxLayout.addWidget(&clButtonOne);
        clHBoxLayout.addWidget(&clButtonThree);

        clVBoxLayout.addLayout(&clHBoxLayout);

        QObject::connect(&clButtonGroup, SIGNAL(buttonClicked(int)), &clDialogBox, SLOT(done(int)));

        clDialogBox.setModal(true);
        clDialogBox.exec();
        int nResult = clDialogBox.result();
        if((nResult != QMessageBox::Cancel) && (nResult != QMessageBox::Yes) && (nResult != QMessageBox::YesToAll))
            nResult = QMessageBox::Cancel;

        QObject::disconnect(&clButtonGroup, SIGNAL(buttonClicked(int)), &clDialogBox, SLOT(done(int)));

        if(nResult == QMessageBox::Cancel)
        {
            InsertIntoUpdateLog("o Update was canceled ",true);
            return false;
        }

    }

    // Take the first script to execute
     // and update nw the TDR
     // Init for progress bar
     int lProgress = 0;
     ResetProgress(false);
     SetMaxProgress(lLinesCount);
     while (!lSqlFilesToProcess.isEmpty())
     {
         lFileToSource.setFileName(lSqlFilesToProcess.takeFirst());
         InsertIntoUpdateLog("o Execute " + lFileToSource.fileName() + "...");
         if(!lFileToSource.open(QIODevice::ReadOnly))
         {
             lMessage = lFileToSource.errorString();
             InsertIntoUpdateLog(lMessage,true);
             return false;
         }

         lQueryContent = "";
         lDelimiter = ";";
         while(! lSqlStream.atEnd())
         {
             lLine = lSqlStream.readLine();

             ++lProgress;

             if(lLine.simplified().isEmpty())
                 continue;
             if(lLine.simplified().startsWith("--") && (lDelimiter==";"))
                 continue;
             if(lLine.simplified().startsWith("exit;"))
                 break;
             if(lLine.simplified().startsWith("DEFINE",Qt::CaseInsensitive))
                 continue;
             if(lLine.simplified().startsWith("CAUTION",Qt::CaseInsensitive))
                 continue;
             if(lLine.simplified().startsWith("INFO",Qt::CaseInsensitive))
                 continue;
             if(lLine.simplified().startsWith("QUESTION",Qt::CaseInsensitive))
                 continue;
             if(lLine.simplified().startsWith("WARNING",Qt::CaseInsensitive))
                 continue;

             if(lLine.simplified().startsWith("DELIMITER",Qt::CaseInsensitive))
             {
                 lDelimiter = lLine.simplified().section(" ",1,1);
                 continue;
             }

             lQueryContent += lLine;
             if(lLine.indexOf(lDelimiter) >= 0)
             {
                 // Remove delimiter at the end of the line
                 // Do not remove delimter into a string
                 if(lQueryContent.count(lDelimiter)>1)
                 {
                     int lLastIndex = lQueryContent.lastIndexOf(lDelimiter);
                     lQueryContent = lQueryContent.left(lLastIndex);
                 }
                 else
                     lQueryContent.remove(lDelimiter);
                 lQueryDisplay = lQueryContent.simplified();
                 if(lQueryDisplay.startsWith("CREATE ",Qt::CaseInsensitive))
                 {
                     lQueryDisplay = lQueryDisplay.remove("IF NOT EXISTS").simplified();
                     lQueryDisplay = lQueryDisplay.remove("/* */").remove("/**/");
                     if(lQueryDisplay.count("(") > 0)
                         lQueryDisplay = lQueryDisplay.section("(",0,0);

                     // Do not display for internal creation
                     if(lFileToSource.fileName().endsWith("alize.sql",Qt::CaseInsensitive))
                         lQueryDisplay = "";
                 }
                 else if(lQueryDisplay.startsWith("DROP ",Qt::CaseInsensitive))
                 {
                     lQueryDisplay = lQueryDisplay.remove("IF EXISTS").simplified();
                     if(lQueryDisplay.count("(") > 0)
                         lQueryDisplay = lQueryDisplay.section("(",0,0);

                     // Do not display for internal creation
                     if(lFileToSource.fileName().endsWith("alize.sql",Qt::CaseInsensitive))
                         lQueryDisplay = "";
                 }

                 if(!lQueryDisplay.isEmpty())
                     InsertIntoUpdateLog(lQueryDisplay.left(100)+" ... ");

                 // Execute the query
                 if(!lQuery.exec(lQueryContent))
                 {
                     // If the error is ALREADY EXISTS and the query contains the comment /*IF NOT EXISTS*/
                     // then ignore this error
                     if(lQueryContent.section("(",0,0).contains("IF NOT EXISTS",Qt::CaseInsensitive)
                             && (lQuery.lastError().number() == 1304))
                     {
                         // Ignore this error
                     }
                     else
                     {
                         lMessage = "Error executing SQL query.\n";
                         lMessage+= "QUERY=" + lQueryContent + "\n";
                         lMessage+= "ERROR=" + lQuery.lastError().text();
                         InsertIntoUpdateLog(lMessage,true);
                         return false;
                     }
                 }

                 if (lQueryContent.startsWith("CALL ",Qt::CaseInsensitive)
                         && (!lQueryContent.contains("log_message",Qt::CaseInsensitive)))
                 {
                     // check CALL status
                     // Check if have some Errors before to execute another query
                     QString lSqlError;
                     lQuery.exec("SHOW ERRORS");
                     if(lQuery.first())
                     {
                         lSqlError = "Level="+lQuery.value("Level").toString()+
                                 ", Code="+lQuery.value("Code").toString()+
                                 ", Message="+lQuery.value("Message").toString();
                     }

                     QSqlQuery lCheckStatus = QSqlQuery(QSqlDatabase::database(rootConnector.m_strConnectionName));
                     QString lCheckContent = "select @status, @message";
                     if (!lCheckStatus.exec(lCheckContent))
                     {
                         lMessage = "Error executing SQL query.\n";
                         lMessage+= "QUERY=" + lCheckContent + "\n";
                         lMessage+= "ERROR=" + lCheckStatus.lastError().text();
                         InsertIntoUpdateLog(lMessage);
                         return false;
                     }
                     int lStatusNo = lCheckStatus.record().indexOf("@status");
                     int lMessageNo = lCheckStatus.record().indexOf("@message");
                     if (!lCheckStatus.first() || (lStatusNo < 0))
                     {
                         lMessage = "Error no status retrieved for " + lQueryContent.section(";",0,0) + "\n";
                         lMessage += "check stored procedure from scripts *_update_initialize.sql to see the template for error management";
                         InsertIntoUpdateLog(lMessage);
                         return false;
                     }
                     else
                     {
                         int lStatus = lCheckStatus.value(lStatusNo).toInt();
                         if (lStatus == 0)
                         {
                             lMessage = "Error for " + lQueryContent.section(";",0,0);
                             InsertIntoUpdateLog(lMessage);
                             if(!lCheckStatus.value(lMessageNo).toString().isEmpty())
                                 InsertIntoUpdateLog(lCheckStatus.value(lMessageNo).toString());
                             if(!lSqlError.isEmpty())
                                 InsertIntoUpdateLog(lSqlError);
                             else if(lCheckStatus.value(lMessageNo).toString() == "Error SQL Exception")
                                 InsertIntoUpdateLog(QString("To get the MySql error, execute the CALL to the stored procedure with a MySql browser."));
                             return false;
                         }
                     }
                 }
                 if(!lQueryDisplay.isEmpty())
                     InsertIntoUpdateLog(" DONE",false);

                 lQueryDisplay = lQueryContent = "";
                 SetProgress(lProgress);
             }
             else
                 lQueryContent += "\n";
         }
     }
     // move progress bar to max
     ResetProgress(true);

    InsertIntoUpdateLog("o Success...");

    return true;
}

bool GexDbPlugin_Galaxy::InsertDefaultConsolidationTree()
{
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog("o Insert default consolidation tree...");

    QString logMessage,defaultFile;

    defaultFile = m_strApplicationPath + "/samples/consolidation/consolidation_tree_schema.xml";

    AddDbUpdateSteps(eUpdateConsTree);

    if (!m_pConsolidationTree || !m_pConsolidationTree->importFromFile(defaultFile))
    {
        logMessage = "Unable to import default Consolidation Tree in the database.";
        GSLOG(SYSLOG_SEV_ERROR, logMessage.toLatin1().constData());

        if (m_pConsolidationTree)
            logMessage += "Invalid consolidation tree from file: " + defaultFile;
        GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, logMessage.toLatin1().constData());
        InsertIntoUpdateLog("ERROR: Unable to import default Consolidation Tree in the database");
        return false;
    }

    InsertIntoUpdateLog("DONE.", false);

    RemoveDbUpdateSteps(eUpdateConsTree);

    // No consolidation allowed for characterization DB
    if (IsCharacTdr())
    {
        InsertIntoUpdateLog("TDR Characterization DB does not use the consolidation process.");
        RemoveDbUpdateSteps(eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures);
    }

    return true;
}

bool GexDbPlugin_Galaxy::LoadDatabaseArchitecture()
{
    QFile file(":/resources/galaxy_tables_dependencies.xml");

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString lErrorMsg;
    int lErrorLine, lErrorColumn;
    QDomDocument lDomDocument;
    bool lStatus = false;

    if (!lDomDocument.setContent(&file, &lErrorMsg, &lErrorLine, &lErrorColumn))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error while loading tables dependencies: "
                      "%1 at line %2, column %3.").
              arg(lErrorMsg).
              arg(QString::number(lErrorLine)).
              arg(QString::number(lErrorColumn)).toLatin1().constData());
    }
    else
        lStatus = GexDbPlugin_Base::LoadDatabaseArchitecture(lDomDocument);

    file.close();

    return lStatus;
}

bool GexDbPlugin_Galaxy::LoadIncrementalUpdateNames()
{
    mIncrementalUpdateName.clear();

    // Enter here all Incremental Update Description
    mIncrementalUpdateName["FT_CONSOLIDATE_SOFTBIN"]["description"]="Perform SOFT binning consolidation.";
    mIncrementalUpdateName["FT_CONSOLIDATE_SOFTBIN"]["testing_stage"]=QString(GEXDB_PLUGIN_GALAXY_FTEST);
    mIncrementalUpdateName["FT_CONSOLIDATE_SOFTBIN"]["target"]="lot_id";
    mIncrementalUpdateName["FT_CONSOLIDATE_SOFTBIN"]["customer"]="TDR Consolidation";

    mIncrementalUpdateName["BINNING_CONSOLIDATION"]["description"]="Perform binning consolidation.";
    mIncrementalUpdateName["BINNING_CONSOLIDATION"]["testing_stage"]=QString(GEXDB_PLUGIN_GALAXY_ETEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_WTEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_FTEST);
    mIncrementalUpdateName["BINNING_CONSOLIDATION"]["target"]="lot_id";
    mIncrementalUpdateName["BINNING_CONSOLIDATION"]["customer"]="TDR Consolidation";

    mIncrementalUpdateName["BINNING_CONSOLIDATION_DELAY"]["description"]="Reschedule binning consolidation after SQL issue.";
    mIncrementalUpdateName["BINNING_CONSOLIDATION_DELAY"]["testing_stage"]=QString(GEXDB_PLUGIN_GALAXY_ETEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_WTEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_FTEST);
    mIncrementalUpdateName["BINNING_CONSOLIDATION_DELAY"]["target"]="lot_id";
    mIncrementalUpdateName["BINNING_CONSOLIDATION_DELAY"]["customer"]="TDR Consolidation";

    mIncrementalUpdateName["AGENT_WORKFLOW"]["description"] = "Post agent consolidation job.";
    mIncrementalUpdateName["AGENT_WORKFLOW"]["testing_stage"] = QString(GEXDB_PLUGIN_GALAXY_ETEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_WTEST)+"|"+QString(GEXDB_PLUGIN_GALAXY_FTEST);
    mIncrementalUpdateName["AGENT_WORKFLOW"]["target"] = "lot_id";
    mIncrementalUpdateName["AGENT_WORKFLOW"]["customer"] = "ADR Consolidation";

    return true;
}

QStringList GexDbPlugin_Galaxy::FieldToDelete(const QString &strField, const QString &strTable,
                                              const QString &strConnectionName){

    QStringList oQueriesList;
    QList<GexDbThreadQuery *> oThreadDone;
    GexDbThreadQuery *poThread = 0;
    bool bEnable = true;

    QString strQuery = QString("SELECT DISTINCT \n"
                               "%1, COUNT(%1)\n"
                               "FROM %2\n"
                               "WHERE valid_splitlot = 'N' \n"
                               "Group BY %1\n").arg(strField).arg(strTable);
    oThreadDone.clear();
    oQueriesList.clear();
    poThread = 0;
    oQueriesList << strQuery;
    oThreadDone = GexDbThreadQuery::executeQueries(0, this, strConnectionName, oQueriesList);
    if(oThreadDone.isEmpty()){
        return QStringList();
    }

    poThread = oThreadDone.first();
    if(bEnable && !poThread->getQueryStatus())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;
        return QStringList();
    }

    QMap<QString, int> oInvalidFieldCount;
    QString strClause;

    if(poThread->getQueryResult()->First()){
        QString strFieldValue = poThread->getQueryResult()->value(0).toString();
        int     iFieldCount = poThread->getQueryResult()->value(1).toInt();
        oInvalidFieldCount.insert(strFieldValue, iFieldCount);
        strClause = QString ("%1 = '%2'").arg(strField).arg(strFieldValue);

        while(poThread->getQueryResult()->Next()){
            strFieldValue = poThread->getQueryResult()->value(0).toString();
            iFieldCount = poThread->getQueryResult()->value(1).toInt();
            oInvalidFieldCount.insert(strFieldValue, iFieldCount);
            strClause += QString (" OR %1 = '%2'").arg(strField).arg(strFieldValue);
        }
    }

    qDeleteAll(oThreadDone);
    oThreadDone.clear();
    poThread = 0;

    QStringList oFieldToDelete;
    if(!strClause.isEmpty()){
        strQuery = QString("SELECT DISTINCT \n"
                           "%1, COUNT(%1)\n"
                           "FROM %2\n"
                           "WHERE %3\n"
                           "Group BY %1\n").arg(strField).arg(strTable).arg(strClause);

        oQueriesList.clear();
        oThreadDone.clear();
        poThread = 0;
        oQueriesList << strQuery;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return QStringList();
        }
        poThread = oThreadDone.first();

        if(bEnable && !poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return QStringList();
        }

        if(poThread->getQueryResult()->First()){
            QString strFieldValue = poThread->getQueryResult()->value(0).toString();
            int     iFieldCount = poThread->getQueryResult()->value(1).toInt();
            if(oInvalidFieldCount.contains(strFieldValue)){
                if(oInvalidFieldCount[strFieldValue] == iFieldCount)
                    oFieldToDelete.append(strFieldValue);
            }

            while(poThread->getQueryResult()->Next()){
                strFieldValue = poThread->getQueryResult()->value(0).toString();
                iFieldCount = poThread->getQueryResult()->value(1).toInt();
                if(oInvalidFieldCount.contains(strFieldValue)){
                    if(oInvalidFieldCount[strFieldValue] == iFieldCount)
                        oFieldToDelete.append(strFieldValue);
                }
            }
        }
    }

    qDeleteAll(oThreadDone);
    oThreadDone.clear();
    poThread = 0;
    return oFieldToDelete;
}

bool GexDbPlugin_Galaxy::purgeDataBase(GexDbPlugin_Filter & roFilters, GexDbPlugin_SplitlotList &oSplitLotList)
{

    if (!m_pclDatabaseConnector)
        return false;

    bool bIsYmProdTdr = IsYmProdTdr();
    bool bIsManualProdTdr = IsManualProdTdr();
    //bool bIsCharacTdr = IsCharacTdr();

    if(oSplitLotList.isEmpty() && !bIsYmProdTdr)
        return false;

    QSqlDatabase oSqlDataBase = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName);
    bool bEnable = true;

    // Check testingstage to query
    if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        SetTestingStage(eElectTest);
    else if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        SetTestingStage(eWaferTest);
    else if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        SetTestingStage(eFinalTest);
    else
        return false;

    //Invalidate row with slected split_lot_id
    QString strSplitLotTable = NormalizeTableName("_SPLITLOT");
    QString strSplitLotFieldToUpdate ="valid_splitlot";
    QString strSplitLotIdFieldName = "splitlot_id";
    QString strClause;
    QString strQuery;

    QStringList oQueriesList;
    QList<GexDbThreadQuery *> oThreadDone;
    GexDbThreadQuery *poThread = 0;

    if(bIsYmProdTdr){
        strQuery = QString("SELECT DISTINCT \n %1 \n").arg(strSplitLotIdFieldName);
        strQuery += QString("FROM %1\n").arg(strSplitLotTable);
        strQuery += QString("WHERE %1 = 'N'\n").arg(strSplitLotFieldToUpdate);

        oThreadDone.clear();
        oQueriesList.clear();
        oQueriesList << strQuery;
        if(poThread) delete poThread;
        poThread = 0;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }
        poThread = oThreadDone.first();
        if(!poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }

        if(poThread->getQueryResult()->First()){
            QString strSplitLot = poThread->getQueryResult()->value(0).toString();
            strClause = QString("(%1 = '%2')").arg(strSplitLotIdFieldName).arg(strSplitLot);
            while(poThread->getQueryResult()->Next()){
                strSplitLot = poThread->getQueryResult()->value(0).toString();
                strClause += QString(" OR (%1 = '%2') ").arg(strSplitLotIdFieldName).arg(strSplitLot);
            }
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;
        if(strClause.isEmpty())
        {
            // No result
            // Nothing to do
            return true;
        }
    } else {
        strClause = QString("(%1 = '%2')").arg(strSplitLotIdFieldName).arg(oSplitLotList.at(0)->m_lSplitlotID);
        for (int iIdx=1; iIdx<oSplitLotList.count(); ++iIdx){
            strClause += QString(" OR (%1 = '%2') ").arg(strSplitLotIdFieldName).arg(oSplitLotList.at(iIdx)->m_lSplitlotID);
        }
    }



    QString strSplitLotIdClause = strClause;
    if(!bIsYmProdTdr){
        strQuery = QString("UPDATE %1\n").arg(strSplitLotTable);
        strQuery += QString("SET %1 = 'N'\n").arg(strSplitLotFieldToUpdate);
        strQuery += QString("WHERE %1\n").arg(strSplitLotIdClause);
        oThreadDone.clear();
        oQueriesList.clear();
        oQueriesList << strQuery;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }
        poThread = oThreadDone.first();
        if(bEnable && !poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;
    }

    SetProgress(2* (100/11));


    //Retrive invalid lot_id
    QStringList oLotIdToDelete = FieldToDelete("lot_id", strSplitLotTable, m_pclDatabaseConnector->m_strConnectionName);
    SetProgress(3* (100/11));

    //Retrive invalid wafer_id
    QStringList oWaferToDelete = FieldToDelete("wafer_id", strSplitLotTable, m_pclDatabaseConnector->m_strConnectionName);
    SetProgress(4* (100/11));

    //Find table containg splitlot_id field
    QStringList oDBTablesList;
    if(!m_pclDatabaseConnector->IsOracleDB())
        oDBTablesList = oSqlDataBase.tables();
    else{
        QString strUserTablesQuery = "select table_name from USER_TABLES";
        oThreadDone.clear();
        oQueriesList.clear();
        oQueriesList << strUserTablesQuery;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }
        poThread = oThreadDone.first();
        if(bEnable && !poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }
        if(poThread->getQueryResult()->First()){
            QString strTableName = poThread->getQueryResult()->value(0).toString().toLower();
            oDBTablesList.append(strTableName);
            while(poThread->getQueryResult()->Next()){
                strTableName = poThread->getQueryResult()->value(0).toString().toLower();
                oDBTablesList.append(strTableName);
            }
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;

    }
    SetProgress(5* (100/11));

    //filter only on current Testing stage tables
    QStringList oTestingStageTables = oDBTablesList.filter(m_strPrefixTable+"_");
    SetProgress(6* (100/11));

    QStringList oSplitlotIdTables;
    QStringList oLotIDTables;
    QStringList oWaferIDTables;
    foreach(const QString &strTableName, oTestingStageTables)
    {
        QSqlRecord oSqlRecord = oSqlDataBase.record(strTableName);
        for(int iField=0; iField<oSqlRecord.count(); ++iField)
        {
            if(oSqlRecord.fieldName(iField).toLower() == "splitlot_id")
            {
                oSplitlotIdTables.append(strTableName);
                break;
            }
        }
        if(!strTableName.toLower().contains("consolidat"))
        {
            if(strTableName.toLower().contains("_lot"))
                oLotIDTables.append(strTableName);
            else if(strTableName.toLower().contains("_wafer"))
                oWaferIDTables.append(strTableName);
        }
    }

    SetProgress(7* (100/11));
    //Remove table entry which contains splitlot_id
    QStringList oConsilatedTables;
    int iSProgress = 7* (100/11);
    int iEProgress = 8* (100/11);
    int iTableIdx =0;
    double dStep = double(iEProgress - iSProgress)/oSplitlotIdTables.count();

    oThreadDone.clear();
    oQueriesList.clear();
    poThread = 0;
    foreach(const QString &strTable, oSplitlotIdTables)
    {
        if(strTable == strSplitLotTable)
            continue;
        if(strTable.toLower().contains("consolidat")&& !bIsManualProdTdr )
        {
            oConsilatedTables.append(strTable);
            continue;
        }

        strQuery = QString("DELETE FROM %1\n").arg(strTable);
        strQuery += QString("WHERE %1 \n").arg(strSplitLotIdClause);
        oQueriesList.append(strQuery);
    }
    //---Execute Queries;

    oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList, 10,dStep);
    if(oThreadDone.isEmpty()){
        return false;
    }
    bool bError = false;
    foreach(poThread, oThreadDone)
    {
        if(bEnable && !poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            bError = true;
        }
    }
    if(bError){
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;
        return false;
    }
    qDeleteAll(oThreadDone);
    oThreadDone.clear();
    poThread = 0;
    oQueriesList.clear();
    //----
    SetProgress(8* (100/11));

    if(!oLotIdToDelete.isEmpty()){

        strClause = QString("(lot_id = '%1')").arg(oLotIdToDelete[0]);
        for (int iIdx=1; iIdx<oLotIdToDelete.count(); ++iIdx){
            strClause += QString(" OR (lot_id = '%2') ").arg(oLotIdToDelete[iIdx]);
        }

        // GCORE-1200: Checked [SC]
        strQuery = QString("SELECT DISTINCT \n"
                           "product_name \n"
                           "FROM %1 \n"
                           "WHERE %2 \n").arg(NormalizeTableName("_lot")).arg(strClause);
        oThreadDone.clear();
        oQueriesList.clear();
        poThread = 0;
        oQueriesList << strQuery;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }

        poThread = oThreadDone.first();
        if(bEnable && !poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }

        QStringList oProdNameToCheck;
        if(poThread->getQueryResult()->First()){
            QString strProdName = poThread->getQueryResult()->value(0).toString();
            oProdNameToCheck.append(strProdName);
            while(poThread->getQueryResult()->Next()){
                strProdName = poThread->getQueryResult()->value(0).toString();
                oProdNameToCheck.append(strProdName);
            }
        }

        iSProgress = 8* (100/11);
        iEProgress = 9* (100/11);
        iTableIdx =0;
        dStep = double(iEProgress - iSProgress)/(oLotIDTables.count()+2);
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        oQueriesList.clear();
        poThread = 0;

        foreach(const QString &strTable, oLotIDTables)
        {
            strQuery = QString("DELETE FROM %1\n").arg(strTable);
            strQuery += QString("WHERE %1 \n").arg(strClause);
            oQueriesList << strQuery;
        }
        //---retrive info
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList, 10,dStep);
        if(oThreadDone.isEmpty()){
            return false;
        }
        bool bError = false;
        foreach(poThread, oThreadDone)
        {
            if(bEnable && !poThread->getQueryStatus())
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
                bError = true;
            }
        }
        if(bError){
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        oQueriesList.clear();
        poThread = 0;

        ///

        //Retrive invalid product_name
        bool bFirst = true;
        foreach(const QString &strProduct, oProdNameToCheck)
        {
            if(bFirst)
            {
                strClause = QString("product_name = '%2'").arg(strProduct);
                bFirst = false;
            }
            else
                strClause += QString("OR product_name = '%2'").arg(strProduct);
        }

        // GCORE-1200: Checked [SC]
        strQuery = QString("SELECT \n"
                           "product_name, Count(product_name)\n"
                           "FROM %1 \n"
                           "WHERE %2 \n"
                           "Group BY product_name\n").arg(NormalizeTableName("_lot")).arg(strClause);
        oQueriesList<<strQuery;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }


        poThread = oThreadDone.first();
        if(bEnable && !poThread->getQueryStatus())
        {
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            return false;
        }

        if(poThread->getQueryResult()->First()){

            QString strProdFound = poThread->getQueryResult()->value(0).toString();
            if(oProdNameToCheck.contains(strProdFound))
                oProdNameToCheck.removeAll(strProdFound);
            while(poThread->getQueryResult()->Next()){
                strProdFound = poThread->getQueryResult()->value(0).toString();
                if(oProdNameToCheck.contains(strProdFound))
                    oProdNameToCheck.removeAll(strProdFound);
            }
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        oQueriesList.clear();

        if(!oProdNameToCheck.isEmpty())
        {
            bool bFirst = true;
            foreach(const QString &strProduct, oProdNameToCheck)
            {
                if(bFirst)
                {
                    strClause = QString("product_name = '%2'").arg(strProduct);
                    bFirst = false;
                }
                else
                    strClause += QString("OR product_name = '%2'").arg(strProduct);
            }

            // GCORE-1200: Checked [SC]
            strQuery = QString("DELETE FROM %1\n").arg(NormalizeTableName("_product_hbin"));
            strQuery += QString("WHERE %1 \n").arg(strClause);
            SetProgress(iSProgress + int((++iTableIdx)*dStep));
            oThreadDone.clear();
            oQueriesList.clear();
            poThread = 0;
            oQueriesList << strQuery;
            oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
            SetProgress(iSProgress + int((++iTableIdx)*dStep));
            if(oThreadDone.isEmpty())
                return false;

            poThread = oThreadDone.first();
            if(bEnable && !poThread->getQueryStatus())
            {
                qDeleteAll(oThreadDone);
                oThreadDone.clear();
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            poThread->getQueryResult()->lastError().text().toLatin1().constData());
                return false;
            }


            strQuery = QString("DELETE FROM %1\n").arg(NormalizeTableName("_product_sbin"));
            strQuery += QString("WHERE %1 \n").arg(strClause);
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            oQueriesList.clear();
            poThread = 0;
            oQueriesList << strQuery;
            oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
            SetProgress(iSProgress + int((++iTableIdx)*dStep));
            if(oThreadDone.isEmpty())
            {
                return false;
            }
            poThread = oThreadDone.first();
            if(bEnable && !poThread->getQueryStatus())
            {
                qDeleteAll(oThreadDone);
                oThreadDone.clear();
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
                return false;
            }
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            oQueriesList.clear();
            poThread = 0;

        }
    }
    SetProgress(9* (100/11));

    iSProgress = 9* (100/11);
    iEProgress = 10* (100/11);
    iTableIdx =0;
    dStep = double(iEProgress - iSProgress)/oLotIDTables.count();


    if(!oWaferToDelete.isEmpty()){
        strClause = QString("(wafer_id = '%1')").arg(oWaferToDelete[0]);
        for (int iIdx=1; iIdx<oWaferToDelete.count(); ++iIdx){
            strClause += QString(" OR (wafer_id = '%1') ").arg(oWaferToDelete[iIdx]);
        }
        qDeleteAll(oThreadDone);
        poThread = 0;
        oThreadDone.clear();
        oQueriesList.clear();

        foreach(const QString &strTable, oWaferIDTables)
        {
            strQuery = QString("DELETE FROM %1\n").arg(strTable);
            strQuery += QString("WHERE %1 \n").arg(strClause);
            oQueriesList.append(strQuery);
        }
        //Execute Query
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList, 10, dStep);
        if(oThreadDone.isEmpty())
            return false;

        bool bError = false;
        foreach(poThread, oThreadDone)
        {
            if(bEnable && !poThread->getQueryStatus())
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
                bError = true;
            }
        }
        if(bError)
        {
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        oQueriesList.clear();
        poThread = 0;
        //Execute Query

    }
    SetProgress(10* (100/11));
    //Delete invalid row from split lot_id
    strQuery = QString("DELETE FROM %1\n").arg(strSplitLotTable);
    strQuery += QString("WHERE %1 = 'N'\n").arg(strSplitLotFieldToUpdate);
    oThreadDone.clear();
    oQueriesList.clear();
    qDeleteAll(oThreadDone);
    poThread = 0;
    oQueriesList << strQuery;
    oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
    if(oThreadDone.isEmpty()){
        return false;
    }
    poThread = oThreadDone.first();
    if(bEnable && !poThread->getQueryStatus())
    {
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
        return false;
    }
    qDeleteAll(oThreadDone);
    oThreadDone.clear();
    oQueriesList.clear();
    poThread = 0;
    SetProgress(11* (100/11));

    return true;
}

bool GexDbPlugin_Galaxy::exportCSVCondition(const QString &strCSVFileName, const QMap<QString, QString>& oExportConditions, GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList, QProgressDialog *poProgress){

    QMap<QString, QString> oConditions = oExportConditions;
    CSVConditionDumper oDumper;
    if(!oDumper.create(strCSVFileName)){
        return false;
    }
    oDumper.setProgress(poProgress);

    QStringList oTestWhereClause;

    if(roFilters.strTestList != "*" &&  !roFilters.strTestList.isEmpty()){
        GexDbPlugin_Galaxy_TestFilter oTestFilter(roFilters.strTestList);
        QStringList oRawTestsList = oTestFilter.getCleanTestNblist().split(",");
        foreach(const QString &strRawTest, oRawTestsList)
        {
            if(!strRawTest.contains("-"))
                oTestWhereClause.append(QString("'%1'").arg(strRawTest));
            else
            {
                int iStart = strRawTest.section("-",0,0).toInt();
                int iEnd = strRawTest.section("-",1,1).toInt();
                for(int iIdx=iStart; iIdx<=iEnd; iIdx++)
                    oTestWhereClause.append(QString("'%1'").arg(iIdx));
            }
        }
    }

    // Check testing stage to query
    if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        SetTestingStage(eElectTest);
    else if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        SetTestingStage(eWaferTest);
    else if(roFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        SetTestingStage(eFinalTest);
    else
        return false;

    QStringList oTestConditionsFriendlyName;
    QStringList oValuesOrder;
    if(oConditions.isEmpty())
        QueryTestConditionsList(roFilters, oTestConditionsFriendlyName);
    else{
        oTestConditionsFriendlyName = oConditions["gui_order"].split("|");
        oConditions.remove("gui_order");

        oValuesOrder =  oConditions["values_order"].split("#");
        oConditions.remove("values_order");
    }

    oDumper.addConditionHeader("Part_Id");

    QStringList oTestConditionsTableName;
    foreach(const QString &strCondition, oTestConditionsFriendlyName)
    {
        oTestConditionsTableName.append(getTestConditionColumn(strCondition));
        oDumper.addConditionHeader(strCondition);
    }

    QStringList oTestConditionTableFields;
    oTestConditionTableFields = QStringList() << "test_type" << "splitlot_id" << oTestConditionsTableName << "test_info_id" ;

    //Invalidate row with slected split_lot_id
    QString strTestConditionsTable = NormalizeTableName("_TEST_CONDITIONS");

    QStringList oSplitLotClause;
    foreach(GexDbPlugin_SplitlotInfo *poSplitLotId, oSplitLotList){
        oSplitLotClause.append(QString("%1.splitlot_id = %2").arg(strTestConditionsTable).arg(poSplitLotId->m_lSplitlotID));
    }
    QString strWhereClause = oSplitLotClause.join(" OR ");

    QStringList oConditionClause;
    if(!oConditions.isEmpty()){
        foreach(const QString &strCondition, oConditions.keys())
        {
            if(!oConditions[strCondition].isEmpty())
            {
                QStringList oValues = oConditions[strCondition].split("|");
                foreach(const QString &strValue, oValues)
                {
                    oConditionClause.append(QString(" %1.%2 = '%3'").arg(strTestConditionsTable).arg(getTestConditionColumn(strCondition)).arg(strValue));
                }
            }
        }
        if(!oConditionClause.isEmpty())
            strWhereClause = QString(" (%1) AND (%2) ").arg(strWhereClause).arg(oConditionClause.join(" OR "));
    }

    QStringList oOrderByClause = oTestConditionTableFields;

    //Loop over the different test type to Join tables and get Test Result
    QStringList oTestType = QStringList () << "P" /*<< "F"*/ << "M";
    QStringList oTableType = QStringList () << "p" /*<< "f"*/ << "mp";
    QString strConditionTableFields = strTestConditionsTable + "." + oTestConditionTableFields.join(QString(", %1.").arg(strTestConditionsTable));

    QString strRunsTable = NormalizeTableName("_RUN");
    QString strRunsTableFields = strRunsTable + ".part_id";

    if(poProgress){
        poProgress->setLabelText("Querying");
        poProgress->setValue(1);
    }
    QCoreApplication::processEvents();

    for(int iTestType=0; iTestType<oTestType.count(); iTestType++){
        if(poProgress){
            poProgress->setValue((int)(10 + 0.35 * (100.0 * (double)iTestType/(double)oTestType.count())));
            if(poProgress->wasCanceled())
                return false;
        }
        QCoreApplication::processEvents();

        QString strTestType = oTestType[iTestType];
        QString strTablePrefix = oTableType[iTestType];

        QString strTestTypeFilter =  QString("%1.test_type = '%2'").arg(strTestConditionsTable).arg(strTestType);
        QString strTestInfoTable = NormalizeTableName(QString("_%1test_info"   ).arg(strTablePrefix.toLower()));
        QString strTestResultTable = NormalizeTableName(QString("_%1test_results").arg(strTablePrefix.toLower()));
        //test_info fields
        // Fix inconsistant TNum/TName
        // Add the TestNum: from the TestName
        QString strTestInfoTableFields = QString("%1.splitlot_id, %1.%2test_info_id, %1.tnum, CONCAT(%1.tnum,':',%1.tname)")
                .arg(strTestInfoTable)
                .arg(strTablePrefix.toLower()) ;

        QString strTestResultTableFields = QString("%1.splitlot_id, %1.%2test_info_id, %1.run_id")
                .arg(strTestResultTable)
                .arg(strTablePrefix.toLower()) ;

        if(strTestType == "F"){
            strTestInfoTableFields += QString(", %1.testseq").arg(strTestInfoTable);
            strTestResultTableFields += QString(", %1.result_flags").arg(strTestResultTable);
        }else {
            strTestInfoTableFields += QString(", %1.units, %1.ll, %1.hl").arg(strTestInfoTable);
            strTestResultTableFields += QString(", %1.value").arg(strTestResultTable);
        }

        QString strJoinClauseInfo = QString(" ( %1 = %2 ) ").arg(strTestConditionsTable + QString(".test_info_id")).arg(strTestInfoTable +"." + strTablePrefix.toLower() + "test_info_id")
                + " AND " +
                QString(" ( %1 = %2 ) ").arg(strTestConditionsTable + QString(".splitlot_id")).arg(strTestInfoTable +".splitlot_id");

        QString strJoinClauseResult = QString(" ( %1 = %2 ) ").arg(strTestConditionsTable + QString(".test_info_id")).arg(strTestResultTable +"." + strTablePrefix.toLower() + "test_info_id")
                + " AND " +
                QString(" ( %1 = %2 ) ").arg(strTestConditionsTable + QString(".splitlot_id")).arg(strTestResultTable +".splitlot_id");

        QString strJoinClause = "( " + strJoinClauseInfo + ") AND (" + strJoinClauseResult  + ")";

        QString strJPartIdClause =  QString(" ( %1 = %2 ) ").arg(strTestConditionsTable + QString(".splitlot_id")).arg(strRunsTable +".splitlot_id") +
                " AND " + QString(" ( %1 = %2 ) ").arg(strTestResultTable + QString(".run_id")).arg(strRunsTable +".run_id");


        QString strLimitTestNumber;
        if(!oTestWhereClause.isEmpty()){
            QString strNumField = QString("( %1.tnum = " ).arg(strTestInfoTable);
            strLimitTestNumber =strNumField +  oTestWhereClause.join(" ) OR " + strNumField) + " ) ";
            strLimitTestNumber = " AND (" + strLimitTestNumber + " ) ";
        }

        int iTestTypeEntriesCount = oTestConditionTableFields.count() + strTestInfoTableFields.split(", ").count() + strTestResultTableFields.split(",").count();


        QString strQuery = QString("SELECT %1 \n FROM %2 \n WHERE %3 \n ORDER BY %4 \n")
                .arg( strConditionTableFields + " , " + strTestInfoTableFields + " , " + strTestResultTableFields + " , " + strRunsTableFields)//1
                .arg(strRunsTable + " , " + strTestConditionsTable + " , " + strTestInfoTable + " , " + strTestResultTable)//2
                .arg( "(" +strJPartIdClause +") AND ( " + strTestTypeFilter + ") AND ("+ strWhereClause + ") AND (" + strJoinClause + " ) "+  strLimitTestNumber)//3
                .arg(strTestConditionsTable + "." + oOrderByClause.join(QString(", %1.").arg(strTestConditionsTable))
                     + ", " + strRunsTable + ".run_id");//4


        oDumper.dumpSQL(QString("TstType : %1 \n %2 \n")
                        .arg(strTestType).arg(strQuery));


        QStringList oQueriesList;
        QList<GexDbThreadQuery *> oThreadDone;
        GexDbThreadQuery *poThread = 0;
        oThreadDone.clear();
        oQueriesList.clear();
        oQueriesList << strQuery;
        if(poThread) delete poThread;
        poThread = 0;
        oThreadDone = GexDbThreadQuery::executeQueries(0, this, m_pclDatabaseConnector->m_strConnectionName, oQueriesList);
        if(oThreadDone.isEmpty()){
            return false;
        }
        if(poProgress && poProgress->wasCanceled())
            return false;

        poThread = oThreadDone.first();
        if(!poThread->getQueryStatus())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), poThread->getQueryResult()->lastError().text().toLatin1().constData());
            qDeleteAll(oThreadDone);
            oThreadDone.clear();
            poThread = 0;
            return false;
        }
        bool bNext = poThread->getQueryResult()->Next();

        while(bNext){
            if(poProgress && poProgress->wasCanceled())
                return false;

            QStringList oTestEntries;
            CSVConditionDumper::entryToStringList(poThread->getQueryResult(), oTestEntries);

            if(oTestEntries.isEmpty() || oTestEntries.count() < iTestTypeEntriesCount){
                bNext = poThread->getQueryResult()->Next();
                continue;
            }

            QString strTestName = oTestEntries[oTestConditionTableFields.count()+3];
            if(strTestName.isEmpty())
                strTestName = oTestEntries[oTestConditionTableFields.count()+ 2];
            int iIdx = oDumper.addTestHeader(strTestName);
            QString strCurrentGroup = oDumper.addGroup(QStringList(oTestEntries.mid(0, oTestConditionTableFields.count()-1)).join("*"));

            //Get all Result for this test
            QString strCurrentTest = QStringList(oTestEntries.mid(0, oTestConditionTableFields.count())).join("*");
            bool bFirstItem = true;
            QMap<QString, QString> oTestData;

            oTestData["type"] = strTestType;
            oTestData["idx"] = QString::number(iIdx);
            oTestData["name"] = strTestName;
            if(strTestType != "F"){
                oTestData["tnum"] = "";
                oTestData["units"] = "";
                oTestData["ll"] = "";
                oTestData["hl"] = "";
            }else
                oTestData["tnum"] = "";

            while(strCurrentTest == QStringList(oTestEntries.mid(0, oTestConditionTableFields.count())).join("*"))
            {
                //Processing
                if(bFirstItem){
                    if(strTestType != "F"){
                        if(oTestData["tnum"].isEmpty())
                            oTestData["tnum"] = oTestEntries[oTestConditionTableFields.count()+ 2];

                        if(oTestData["units"].isEmpty())
                            oTestData["units"] = oTestEntries[oTestConditionTableFields.count()+ 4];

                        if(oTestData["ll"].isEmpty())
                            oTestData["ll"] = oTestEntries[oTestConditionTableFields.count()+ 5];

                        if(oTestData["hl"].isEmpty())
                            oTestData["hl"] = oTestEntries[oTestConditionTableFields.count()+ 6];

                        if(!oTestData["tnum"].isEmpty() && !oTestData["units"].isEmpty() && !oTestData["ll"].isEmpty() && !oTestData["hl"].isEmpty())
                            bFirstItem = false;
                    }else {
                        if(oTestData["tnum"].isEmpty())
                            oTestData["tnum"] = oTestEntries[oTestConditionTableFields.count()+ 2];
                        if(!oTestData["tnum"].isEmpty())
                            bFirstItem = false;
                    }
                }
                if(strTestType != "F"){
                    oTestData["run_id"].append(QString(",%1").arg(oTestEntries[oTestConditionTableFields.count()+ 9]));
                    oTestData["value"].append( QString(",%1").arg(oTestEntries[oTestConditionTableFields.count()+ 10]));
                    oTestData["part_id"].append( QString(",%1").arg(oTestEntries[oTestConditionTableFields.count() + 11]));
                }else {
                    oTestData["run_id"].append(QString(",%1").arg(oTestEntries[oTestConditionTableFields.count()+ 7]));
                    oTestData["value"].append(QString(",%1").arg(oTestEntries[oTestConditionTableFields.count()+ 7]));
                }

                bNext = poThread->getQueryResult()->Next();
                if(!bNext){
                    break;
                } else
                    CSVConditionDumper::entryToStringList(poThread->getQueryResult(), oTestEntries);
            }

            //Dump the test data in the group file
            if(!oDumper.createTempGroupFile(strCurrentGroup, oTestData))
                return false;
        }
        qDeleteAll(oThreadDone);
        oThreadDone.clear();
        poThread = 0;
    }


    if(oDumper.groupCount() > 0){
        if(poProgress){
            if(poProgress->wasCanceled())
                return false;
            poProgress->setValue(55);
            poProgress->setLabelText("Generating csv file");
        }
        QCoreApplication::processEvents();
        oDumper.dumpTempGroupFile(oValuesOrder);
    }

    if(poProgress){
        poProgress->setValue(100);
    }
    QCoreApplication::processEvents();

    return true;
}

