/******************************************************************************!
 * \file db_import_file_sqlite.cpp
 *
 * DatabaseEngine::ImportFile
 *   DatabaseEngine::ImportFileSqlite
 *     DatabaseEngine::ImportFileSqliteSplitlot
 *       DatakeysLoader::Load
 *       DatabaseEngine::LoadConfigKeys
 *       DatabaseEngine::DatabaseInsertFile
 *         GexRemoteDatabase::InsertDataFile
 *           GexDbPlugin_Galaxy::InsertDataFile
 *             GexDbPlugin_Galaxy::InsertDataFileSqlite
 *
 ******************************************************************************/
#include "db_engine.h"
#include "classes.h"  // For Range
#include "gqtl_log.h"
#include "engine.h"
#include "gex_database_entry.h"
#include "gs_data.h"
#include "gs_json.h"

namespace GS
{

namespace Gex
{

/******************************************************************************!
 * \fn GsDataDeleter
 ******************************************************************************/
static void GsDataDeleter(struct GsData* d)
{
    if (d == NULL)
    {
        return;
    }
    gs_data_unloadData(d);
    gs_data_quit(d);
    free(d);
}

/******************************************************************************!
 * \fn ImportFileSqliteSplitlot
 ******************************************************************************/
int DatabaseEngine::ImportFileSqliteSplitlot(
    struct GsData* lGsData,
    int lSqliteSplitlotId,
    GexDatabaseEntry* pDatabaseEntry,
    QString lFileToInsert,
    QString& strErrorMessage,
    bool& lEditHeaderInfo,
    GS::QtLib::DatakeysEngine& dbKeysEngine,
    CGexMoTaskItem* ptTask,
    bool bImportFromRemoteDB,
    QString& lErrorMessage)
{
    int lStatus;

    GSLOG(SYSLOG_SEV_DEBUG, "ImportFileSqliteSplitlot begin");

    // Load Datakeys
    if (! GS::QtLib::DatakeysLoader::Load(lGsData,
                                          lSqliteSplitlotId,
                                          lFileToInsert,
                                          dbKeysEngine.dbKeysContent(),
                                          lErrorMessage,
                                          true))
    {
        return Failed;
    }

    // Load ConfigKeys
    bool bFailedValidationStep = false;
    lEditHeaderInfo = ! bImportFromRemoteDB;
    long t_SetupTime =
        dbKeysEngine.dbKeysContent().Get("SetupTime").toLongLong();
    long t_StartTime =
        dbKeysEngine.dbKeysContent().Get("StartTime").toLongLong();
    long t_FinishTime =
        dbKeysEngine.dbKeysContent().Get("FinishTime").toLongLong();
    if (! this->LoadConfigKeys(dbKeysEngine,
                               ptTask,
                               lEditHeaderInfo,
                               bFailedValidationStep,
                               lErrorMessage,
                               m_strInsertionShortErrorMessage,
                               lFileToInsert))
    {
        return Failed;
    }
    {
        QString strError;
        this->CheckOverloadedKeyWords(t_SetupTime, t_StartTime, t_FinishTime,
                                      dbKeysEngine, strError);
        if (! strError.isEmpty())
        {
            lErrorMessage = strError;
            return false;
        }
    }

    // Pre insertion script
    if (pDatabaseEntry->m_pExternalDatabase == NULL)
    {
        return Passed;
    }
    if (bImportFromRemoteDB)
    {
        return Passed;
    }
    lStatus =
        this->RunPreInsertionScript(strErrorMessage,
                                    dbKeysEngine,
                                    ptTask,
                                    bImportFromRemoteDB,
                                    lErrorMessage);
    if (lStatus != Passed)
    {
        return lStatus;
    }

    // Insert
    lStatus =
        this->DatabaseInsertFile(lGsData,
                                 lSqliteSplitlotId,
                                 pDatabaseEntry,
                                 lFileToInsert,
                                 strErrorMessage,
                                 dbKeysEngine,
                                 ptTask,
                                 bImportFromRemoteDB);

    GSLOG(SYSLOG_SEV_DEBUG, "ImportFileSqliteSplitlot end");

    return lStatus;
}

/******************************************************************************!
 * \fn ImportFileSqlite
 ******************************************************************************/
int
DatabaseEngine::ImportFileSqlite(GexDatabaseEntry* pDatabaseEntry,
                                 QString strSourceArchive,
                                 QString lFileToInsert,
                                 QString& strErrorMessage,
                                 bool& lEditHeaderInfo,
                                 GS::QtLib::DatakeysEngine& dbKeysEngine,
                                 CGexMoTaskItem* ptTask,
                                 bool bImportFromRemoteDB)
{
    DbKeyContentSetFileInfo(dbKeysEngine, lFileToInsert, "", strSourceArchive);

    QString lErrorMessage;
    QString lLocalErrorMessage;
    int lStatus;

    GSLOG(SYSLOG_SEV_DEBUG, "ImportFileSqlite begin");

    QSharedPointer<struct GsData> lGsData(gs_data_new(), GsDataDeleter);
    gs_data_init(lGsData.data(), GS_DATA_SQLITE);
    if (gs_data_getLastError(lGsData.data()))
    {
        lErrorMessage = gs_data_getLastStrError(lGsData.data());
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("failed to load  %1 with error : %2").
              arg(lFileToInsert).arg(lErrorMessage).
              toLatin1().constData());
        GSET_ERROR2(DatabaseEngine, eFailedToLoadDataKeys, NULL,
                    lFileToInsert.toLatin1().constData(),
                    lErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        return Failed;
    }
    gs_data_loadData(lGsData.data(), lFileToInsert.toUtf8().constData());
    if (gs_data_getLastError(lGsData.data()))
    {
        lErrorMessage = gs_data_getLastStrError(lGsData.data());
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("failed to load  %1 with error : %2").
              arg(lFileToInsert).arg(lErrorMessage).
              toLatin1().constData());
        GSET_ERROR2(DatabaseEngine, eFailedToLoadDataKeys, NULL,
                    lFileToInsert.toLatin1().constData(),
                    lErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        return Failed;
    }

    // splitlot_id list
    const char* json = gs_data_getSplitlotIdList(lGsData.data());
    if (gs_data_getLastError(lGsData.data()))
    {
        lErrorMessage = gs_data_getLastStrError(lGsData.data());
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("failed to load  %1 with error : %2").
              arg(lFileToInsert).arg(lErrorMessage).
              toLatin1().constData());
        GSET_ERROR2(DatabaseEngine, eFailedToLoadDataKeys, NULL,
                    lFileToInsert.toLatin1().constData(),
                    lErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        return Failed;
    }
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("splitlot_id list = %1").arg(json).toUtf8().constData());
    std::string splitlotIdList(json);

    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, splitlotIdList.c_str());
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        int lSqliteSplitlotId = GsJsonIteratorInt(&jsonIter);

        lStatus =
            this->ImportFileSqliteSplitlot(lGsData.data(),
                                           lSqliteSplitlotId,
                                           pDatabaseEntry,
                                           lFileToInsert,
                                           strErrorMessage,
                                           lEditHeaderInfo,
                                           dbKeysEngine,
                                           ptTask,
                                           bImportFromRemoteDB,
                                           lErrorMessage);
        if (lStatus != Passed)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("failed to load  %1 with error : %2").
                  arg(lFileToInsert).arg(lErrorMessage).
                  toLatin1().constData());
            GSET_ERROR2(DatabaseEngine, eFailedToLoadDataKeys, NULL,
                        lFileToInsert.toLatin1().constData(),
                        lErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, lStatus);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return lStatus;
        }
    }

    return Passed;
}

}

}
