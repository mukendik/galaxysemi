/******************************************************************************!
 * \file gexdb_plugin_galaxy_insert_sqlitefile.cpp
 ******************************************************************************/
#include <QSqlError>
#include <QFileInfo>
#include "gqtl_log.h"
#include "gexdb_plugin_galaxy.h"
#include "gs_gtl_traceability.h"
#include "gs_json.h"
#include "gs_buffer.h"

#define GSET_LAST_ERROR(q) \
GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, q.toUtf8().constData(),\
            dbQuery.lastError().text().toUtf8().constData());\
*m_pbDelayInsertion = true

// Macro query without result
#define QUERY_EXEC_OR_RETURN(q) \
{\
    QString lMacroQuery = q;\
    if (! dbQuery.Execute(lMacroQuery))\
    {\
        GSET_LAST_ERROR(lMacroQuery);\
        return -1;\
    }\
}

// Macro query with result
#define QUERY_FIRST_OR_RETURN(q) \
{\
    QString lMacroQuery = q;\
    if (! dbQuery.Execute(lMacroQuery) || ! dbQuery.first())\
    {\
        GSET_LAST_ERROR(lMacroQuery);\
        return -1;\
    }\
}

/******************************************************************************!
 * \fn InsertDataFromJson
 * \brief Create and execute mysql query from a json structure
 *        splitlot_id and gtl_splitlot_id are used if not nul
 ******************************************************************************/
bool GexDbPlugin_Galaxy::InsertDataFromJson(
    GexDbPlugin_Query& dbQuery,
    const char* json,
    const char* table,
    unsigned int tdrSplitlotId,
    unsigned int gtlSplitlotId)
{
    GsJsonIterator iter;
    const char* label;
    char* value;
    QString firstLabel;
    int count;
    int pass;
    int len;
    QString lQuery = QString("INSERT INTO ") + table + "(";

    count = 0;
    for (GsJsonIteratorBegin(&iter, json);
         GsJsonIteratorEnd(&iter) == 0;
         GsJsonIteratorNext(&iter))
    {
        label = GsJsonIteratorLabel(&iter);
        // GCORE-10060 - REMOVE BINARY flags
        // ignore old flags
        if(label == QString("flags"))
            continue;
        if (! count)
        {
            firstLabel = label;
            if (tdrSplitlotId)
            {
                lQuery += "splitlot_id,";
            }
            if (gtlSplitlotId)
            {
                lQuery += "gtl_splitlot_id,";
            }
        }
        else if (label == firstLabel)
        {
            break;
        }
        if (count)
        {
            lQuery += ',';
        }
        lQuery += label;
        ++count;
    }
    if (count == 0)
    {
        return true;
    }
    lQuery += ")VALUES(";
    pass = 0;
    for (GsJsonIteratorBegin(&iter, json);
         GsJsonIteratorEnd(&iter) == 0;
         GsJsonIteratorNext(&iter))
    {
        label = GsJsonIteratorLabel(&iter);
        // GCORE-10060 - REMOVE BINARY flags
        // ignore old flags
        if(label == QString("flags"))
            continue;
        if (! pass)
        {
            if (tdrSplitlotId)
            {
                lQuery += QString("%1,").arg(tdrSplitlotId);
            }
            if (gtlSplitlotId)
            {
                lQuery += QString("%1,").arg(gtlSplitlotId);
            }
        }
        else if (pass == count)
        {
            lQuery += "),(";
            if (tdrSplitlotId)
            {
                lQuery += QString("%1,").arg(tdrSplitlotId);
            }
            if (gtlSplitlotId)
            {
                lQuery += QString("%1,").arg(gtlSplitlotId);
            }
            pass = 0;
        }
        else
        {
            lQuery += ',';
        }
        value = const_cast<char*>(GsJsonIteratorValue(&iter));
        if (value[0] == '"')
        {
            value[0] = '\'';
            len = strlen(value);
            if (value[len - 1] == '"')
            {
                value[len - 1] = '\'';
            }
        }
        if (::strcmp(value, "'(null)'") == 0)
        {
            lQuery += "NULL";
        }
        else
        {
            lQuery += value;
        }
        ++pass;
    }
    lQuery += ")";
    if (! dbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Error executing query. Query = %1").arg(lQuery).
              toUtf8().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Json = %1").arg(json).toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return false;
    }

    return true;
}

/******************************************************************************!
 * \fn CheckFTRunFromJson
 * \brief Check if (part_id,site_no) exists
 ******************************************************************************/
bool GexDbPlugin_Galaxy::CheckFTRunFromJson(
    GexDbPlugin_Query& dbQuery,
    const char* json,
    unsigned int tdrSplitlotId)
{
    QString lSiteNo;
    QString lPartId;
    QString lHbinNo;
    QString lSbinNo;
    QString lQuery;

    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, json);
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        char* label = GsJsonIteratorLabel(&jsonIter);
        if (strcmp(label, "site_no") == 0)
        {
            if (! lSiteNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("site_no set multiple times in json: %1").
                      arg(json).toUtf8().constData());
                return false;
            }
            lSiteNo = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label, "part_id") == 0)
        {
            if (! lPartId.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("part_id set multiple times in json: %1").
                      arg(json).toUtf8().constData());
                return false;
            }
            lPartId = GsJsonIteratorValue(&jsonIter);
            lPartId.replace('"', '\'');
        }
        else if (strcmp(label, "hbin_no") == 0)
        {
            if (! lHbinNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("hbin_no set multiple times in json: %1").
                      arg(json).toUtf8().constData());
                return false;
            }
            lHbinNo = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label, "sbin_no") == 0)
        {
            if (! lSbinNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("sbin_no set multiple times in json: %1").
                      arg(json).toUtf8().constData());
                return false;
            }
            lSbinNo = GsJsonIteratorValue(&jsonIter);

            if (lSiteNo.isEmpty() ||
                lPartId.isEmpty() ||
                lHbinNo.isEmpty() ||
                lSbinNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Some required fields are missing or "
                              "empty in json: %1").arg(json).
                      toUtf8().constData());
                return false;
            }
            lQuery = QString("SELECT hbin_no,sbin_no FROM ft_run "
                             "WHERE splitlot_id=%1 "
                             "AND part_id=%2 "
                             "AND site_no=%3").
                arg(tdrSplitlotId).
                arg(lPartId).
                arg(lSiteNo);
            if (! dbQuery.Execute(lQuery) || ! dbQuery.First())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Could not find matching TDR run for "
                              "splitlot_id=%1, part_id=%2, site_no=%3").
                      arg(tdrSplitlotId).arg(lPartId).arg(lSiteNo).
                      toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Json = %1").arg(json).toUtf8().constData());
                GSET_LAST_ERROR(lQuery);
                return false;
            }
            if (dbQuery.value(0).toString() != lHbinNo)
            {
                QString lMessage =
                    QString("hbin mismatch between GTL SQLite and TDR (%1) != "
                            "(%2) for splitlot_id=%3, part_id=%4, site_no=%5").
                    arg(dbQuery.value(0).toString()).
                    arg(lHbinNo).
                    arg(tdrSplitlotId).
                    arg(lPartId).
                    arg(lSiteNo);
                GSLOG(SYSLOG_SEV_WARNING, lMessage.toUtf8().constData());
                GSET_LAST_ERROR(lMessage);
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Json = %1").arg(json).toUtf8().constData());
                //FIXME: uncomment this when validated
                //return false;
            }
            if (dbQuery.value(1).toString() != lSbinNo)
            {
                QString lMessage =
                    QString("sbin mismatch between GTL SQLite and TDR (%1) != "
                            "(%2) for splitlot_id=%3, part_id=%4, site_no=%5").
                    arg(dbQuery.value(1).toString()).
                    arg(lSbinNo).
                    arg(tdrSplitlotId).
                    arg(lPartId).
                    arg(lSiteNo);
                GSLOG(SYSLOG_SEV_WARNING, lMessage.toUtf8().constData());
                GSET_LAST_ERROR(lMessage);
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Json = %1").arg(json).toUtf8().constData());
                //FIXME: uncomment this when validated
                //return false;
            }
            lSiteNo = "";
            lPartId = "";
            lHbinNo = "";
            lSbinNo = "";
        }
    }

    return true;
}

/******************************************************************************!
 * \fn CheckFtTestInfoFromJson
 * \brief Check if (tnum,tname) exists and is unique
 ******************************************************************************/
QMap<int,int>*
GexDbPlugin_Galaxy::CheckFtTestInfoFromJson(GexDbPlugin_Query& dbQuery,
                                            const char* json,
                                            const char* tablePrefix,
                                            unsigned int tdrSplitlotId)
{
    QString tNum;
    QString tName;
    QString lQuery;
    int tdrTestId;
    int gtlTestId = -1;
    QMap<int,int>* testId = new QMap<int,int>;

    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, json);
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        char* label = GsJsonIteratorLabel(&jsonIter);
        if (strcmp(label + strlen(tablePrefix), "test_info_id") == 0)
        {
            if (gtlTestId != -1)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("test_info_id set multiple times in json: %1").
                      arg(json).toUtf8().constData());
                delete testId;
                return NULL;
            }
            gtlTestId = atoi(GsJsonIteratorValue(&jsonIter));
        }
        else if (strcmp(label, "tnum") == 0)
        {
            if (! tNum.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("tnum set multiple times in json: %1").arg(json).
                      toUtf8().constData());
                delete testId;
                return NULL;
            }
            tNum = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label, "tname") == 0)
        {
            if (tNum.isEmpty() || gtlTestId == -1)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("tname set multiple times in json: %1").arg(json).
                      toUtf8().constData());
                delete testId;
                return NULL;
            }
            tName = GsJsonIteratorValue(&jsonIter);
            tName.replace('"', '\'');
            lQuery = QString("SELECT %1test_info_id FROM ft_%1test_info"
                             " WHERE splitlot_id=%2 AND tnum=%3 AND tname=%4").
                arg(tablePrefix).
                arg(tdrSplitlotId).
                arg(tNum).
                arg(tName);
            if (! dbQuery.Execute(lQuery) || ! dbQuery.first() ||
                dbQuery.size() != 1)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Could not find matching TDR test for "
                              "splitlot_id=%1, tnum=%2, tname=%3").
                      arg(tdrSplitlotId).
                      arg(tNum).
                      arg(tName).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Json = %1").arg(json).toUtf8().constData());
                if (dbQuery.size() != 1)
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                                lQuery.toUtf8().constData(),
                                "No result");
                }
                else
                {
                    GSET_LAST_ERROR(lQuery);
                }
                delete testId;
                return NULL;
            }
            tdrTestId = dbQuery.value(0).toInt();
            testId->insert(gtlTestId, tdrTestId);
            tNum = "";
            gtlTestId = -1;
        }
    }

    return testId;
}

/******************************************************************************!
 * \fn CheckFtBinFromJson
 * \brief Check if (xbin_no,xbin_cat) exists
 ******************************************************************************/
bool
GexDbPlugin_Galaxy::CheckFtBinFromJson(GexDbPlugin_Query& dbQuery,
                                       const char* json,
                                       const char* tablePrefix,
                                       unsigned int tdrSplitlotId)
{
    QString binNo;
    QString binName;
    QString binCat;
    QString binFamily;
    QString binSubFamily;
    QString lQuery;

    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, json);
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        char* label = GsJsonIteratorLabel(&jsonIter);
        if (strcmp(label + 1, "bin_no") == 0)
        {
            binNo = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label + 1, "bin_name") == 0)
        {
            binName = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label + 1, "bin_cat") == 0)
        {
            binCat = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label + 1, "bin_family") == 0)
        {
            binFamily = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label + 1, "bin_subfamily") == 0)
        {
            binSubFamily = GsJsonIteratorValue(&jsonIter);
            if (binNo.isEmpty() || binCat.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Some required fields are missing or "
                              "empty in json: %1").arg(json).
                      toUtf8().constData());
                return false;
            }
            lQuery = QString("SELECT %1bin_cat FROM ft_%1bin"
                             " WHERE splitlot_id=%2 AND %1bin_no=%3").
                arg(tablePrefix).
                arg(tdrSplitlotId).
                arg(binNo);
            if (! dbQuery.Execute(lQuery) || ! dbQuery.first())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Could not find matching TDR bin for "
                              "splitlot_id=%1, bin_no=%2").
                      arg(tdrSplitlotId).arg(binNo).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Json = %1").arg(json).toUtf8().constData());
                GSET_LAST_ERROR(lQuery);
                return false;
            }
            if (dbQuery.size() == 1)
            {
                QString cat = dbQuery.value(0).toString();
                if (cat != binCat)
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                          QString("Bin category mismatch between "
                                  "GTL SQLite and TDR (%1) != (%2) for "
                                  "splitlot_id=%3, bin_no=%4").
                          arg(cat).
                          arg(binCat).
                          arg(tdrSplitlotId).
                          arg(binNo).toUtf8().constData());
                    GSLOG(SYSLOG_SEV_DEBUG,
                          QString("Query = %1").arg(lQuery).
                          toUtf8().constData());
                    GSLOG(SYSLOG_SEV_DEBUG,
                          QString("Json = %1").arg(json).toUtf8().constData());
                    GSET_LAST_ERROR(lQuery);
                    return false;
                }
            }
            /*
            else
            {
                lQuery = QString("INSERT INTO ft_%1bin "
                                 "(splitlot_id,%1bin_no,%1bin_name,"
                                 "%1bin_cat,%1bin_family,%1bin_subfamily)"
                                 "VALUES(%2,%3,%4,%5,%6,%7)").
                    arg(tablePrefix).
                    arg(tdrSplitlotId).
                    arg(binNo).
                    arg(binName).
                    arg(binCat).
                    arg(binFamily).
                    arg(binSubFamily);
                if (! dbQuery.Execute(lQuery))
                {
                    GSLOG(SYSLOG_SEV_DEBUG,
                          QString("query = %1").arg(lQuery).
                          toUtf8().constData());
                    GSLOG(SYSLOG_SEV_DEBUG,
                          QString("json = %1").arg(json).toUtf8().constData());
                    GSET_LAST_ERROR(lQuery);
                    return false;
                }
            }
            */
            binNo = "";
            binCat = "";
        }
    }

    return true;
}

/******************************************************************************!
 * \fn InsertGlobalFileFromJson
 ******************************************************************************/
bool GexDbPlugin_Galaxy::InsertGlobalFileFromJson(
    GexDbPlugin_Query& dbQuery,
    std::string& fileInfos,
    const char* json,
    unsigned int tdrSplitlotId)
{
    QString name;
    QString format;
    QString date;
    QString lQuery;
    int checksum;
    int fileId;

    // Values from json
    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, fileInfos.c_str());
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        char* label = GsJsonIteratorLabel(&jsonIter);
        if (strcmp(label, "file_name") == 0)
        {
            name = GsJsonIteratorValue(&jsonIter);
            name.replace('"', '\'');
            name.replace('\\', "\\\\");
        }
        else if (strcmp(label, "file_format") == 0)
        {
            format = GsJsonIteratorValue(&jsonIter);
            format.replace('"', '\'');
        }
        else if (strcmp(label, "file_last_update") == 0)
        {
            date = GsJsonIteratorValue(&jsonIter);
            date.replace('"', '\'');
        }
    }
    if (name.isEmpty())
    {
        return true;
    }
    if (date == "'(null)'")
    {
        date = QString("'") + QDateTime(QDate::currentDate()).
            toString("yyyy-MM-dd hh:mm:ss") + QString("'");
    }

    // Checksum
    checksum = qChecksum(json, strlen(json));
    if (checksum == 0)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("checksum = 0, json = %1").arg(json).
              toUtf8().constData());
    }

    // Insert into global_files
    lQuery = QString("INSERT INTO global_files ("
                     "file_name,file_format,file_content,file_checksum,"
                     "file_type,file_last_update)VALUES("
                     "%1,%2,'%3',%4,'Recipe',%5)").
        arg(name).arg(format).arg(json).arg(checksum).arg(date);
    if (! dbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error inserting data into global_files. Query = %1").
              arg(lQuery).toUtf8().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Json = %1").arg(fileInfos.c_str()).toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return false;
    }

    // Last insert id ==> ft_splitlot.recipe_id
    lQuery = QString("SELECT LAST_INSERT_ID()");
    if  (! dbQuery.Execute(lQuery) || ! dbQuery.first())
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error getting last file_id when inserting into "
                      "global_files. Query = %1").arg(lQuery).
              toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return false;
    }
    fileId = dbQuery.value(0).toInt();

    // Update recipe_id for ft_splitlot
    lQuery = QString("UPDATE ft_splitlot "
                     "SET recipe_id=%1 WHERE splitlot_id=%2").
        arg(fileId).
        arg(tdrSplitlotId);
    if  (! dbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error upading ft_splitlot.recipe_id. Query = %1").
              arg(lQuery).toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return false;
    }

    // Update recipe_id for ft_gtl_splitlot
    lQuery = QString("UPDATE ft_gtl_splitlot "
                     "SET recipe_id=%1 WHERE splitlot_id=%2").
        arg(fileId).
        arg(tdrSplitlotId);
    if  (! dbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error upading ft_gtl_splitlot.recipe_id. Query = %1").
              arg(lQuery).toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return false;
    }

    return true;
}

/******************************************************************************!
 * \fn InsertDataFileSqliteReturn
 ******************************************************************************/
static bool
InsertDataFileSqliteReturn(struct GsGtlTraceability* tr, bool status)
{
    if (gs_gtl_traceability_getLastError(tr))
    {
        GSLOG(SYSLOG_SEV_ERROR, gs_gtl_traceability_getLastStrError(tr));
    }
    /* Do this when gs_gtl_traceability_init() is called
       instead of gs_gtl_traceability_init_with_gsdata()
       gs_gtl_traceability_unloadData(tr);
       gs_gtl_traceability_quit(tr);
    */
    free(tr);
    return status;
}

/******************************************************************************!
 * \fn GetTDRSplitlotIdFromSP
 ******************************************************************************/
int
GexDbPlugin_Galaxy::GetTDRSplitlotFromSP(GexDbPlugin_Query& dbQuery,
                                         unsigned int sqliteSplitlotId,
                                         const QString& strDataFileName)
{
    QString procedureName("ft_gtl_insertion_validation");

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Calling GTL traceability to TDR matching stored procedure "
                  "(%1).").arg(procedureName).toUtf8().constData());

    // Call
    QString lQuery = QString("CALL %1(%2,'%3',"
                             "@outSplitlot,@outMessage,@outStatus)").
        arg(procedureName).
        arg(sqliteSplitlotId).
        arg(QFileInfo(strDataFileName).fileName());
    GSLOG(SYSLOG_SEV_DEBUG, lQuery.toUtf8().constData());
    QUERY_EXEC_OR_RETURN(lQuery)

    // Retrieve parameter values
    QUERY_FIRST_OR_RETURN("SELECT @outSplitlot,@outMessage,@outStatus")

    // Return
    int status = dbQuery.value(2).toInt();  // 0 NOK, 1 OK, 2 delay
    if (status == 0)
    {
        QString message = dbQuery.value(1).toString();
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    procedureName.toUtf8().constData(),
                    message.toUtf8().constData());
        return -1;
    }
    else if (status == 2)
    {
        QString message = dbQuery.value(1).toString();
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    procedureName.toUtf8().constData(),
                    message.toUtf8().constData());
        *m_pbDelayInsertion = true;
        return -1;
    }

    return dbQuery.value(0).toUInt();
}

/******************************************************************************!
 * \fn DeleteGtlSplitlot
 ******************************************************************************/
int GexDbPlugin_Galaxy::DeleteGtlSplitlot(GexDbPlugin_Query& dbQuery,
                                          unsigned int /*tdrSplitlotId*/)
{
    QString lQuery;
    int gtlSplitlotId;

    // Delete obsolete gtl splitlots
    QUERY_EXEC_OR_RETURN(QString("SELECT gtl_splitlot_id "
                                 "FROM ft_gtl_splitlot "
                                 "WHERE session_id NOT IN "
                                 "(SELECT p.id "
                                 "FROM information_schema.processlist AS p "
                                 "WHERE p.user='%1') "
                                 "AND valid_splitlot='N'").
                         arg(m_pclDatabaseConnector->m_strUserName_Admin))
    if (dbQuery.first())
    {
        while (dbQuery.next()) {
            gtlSplitlotId = dbQuery.value(0).toInt();
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("delete gtl_splitlot_id=%1").arg(gtlSplitlotId).
                  toUtf8().constData());
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_ptest_rollinglimits "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_ptest_rollingstats "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_mptest_rollinglimits "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_mptest_rollingstats "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_event "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_ptest_outliers "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
            QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_mptest_outliers "
                                         "WHERE gtl_splitlot_id=%1").
                                 arg(gtlSplitlotId))
        }

        // Delete recipe in global_files
        /* The recipe should not be deleted here, but rather just
           overwritten when the GTL splitlot gets inserted, otherwise,
           the risk is to delete a recipe that will not get inserted again

        lQuery = QString("SELECT recipe_id FROM ft_splitlot "
                         "WHERE splitlot_id=%1").arg(tdrSplitlotId);
        if (! dbQuery.Execute(lQuery) || ! dbQuery.first())
        {
            GSET_LAST_ERROR(lQuery);
            return -1;
        }
        int fileId = dbQuery.value(0).toInt();
        QUERY_EXEC_OR_RETURN(QString("DELETE FROM global_files "
                                     "WHERE file_id=%1").arg(fileId))
        QUERY_EXEC_OR_RETURN(QString("UPDATE ft_splitlot "
                                     "SET recipe_id='0' "
                                     "WHERE splitlot_id=%1").
                             arg(tdrSplitlotId))

        // Final
        QUERY_EXEC_OR_RETURN(QString("DELETE FROM ft_gtl_splitlot "
                                     "WHERE gtl_splitlot_id=%1").
                             arg(gtlSplitlotId))
        */
    }

    return 0;
}

/******************************************************************************!
 * \fn InsertGtlSplitlotId
 ******************************************************************************/
int
GexDbPlugin_Galaxy::InsertGtlSplitlot(GexDbPlugin_Query& dbQuery,
                                      struct GsGtlTraceability* tr,
                                      unsigned int sqliteSplitlotId,
                                      unsigned int tdrSplitlotId)
{
    if (this->DeleteGtlSplitlot(dbQuery, tdrSplitlotId) == -1)
    {
        return -1;
    }

    // Set valid_splitlot = N
    char* c = gs_buffer_get(tr->buffer);
    if (c == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot get GTL traceability buffer");
        return -1;
    }
    while (*c != '\0')
    {
        if (::strncmp(c, "\"valid_splitlot\":", 17) == 0)
        {
            c += 18;
            *c = 'N';
        }
        ++c;
    }

    QUERY_FIRST_OR_RETURN(QString("SELECT CONNECTION_ID()"))
    int sessionId = dbQuery.value(0).toInt();
    // UGLY MODE BEGIN.
    //TODO: replace this ugly code once we use a more powerful JSON library
    //TODO: we should probably add an object to the JSON,
    //TODO: and create one JSON encapsulating the 2 objects
    QString extraJson;
    extraJson = QString(",\"sqlite_splitlot_id\":%1,\"session_id\":%2}]}").
        arg(sqliteSplitlotId).
        arg(sessionId);
    if (tr->buffer->ptr[tr->buffer->size - 3] != '}')
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error processing GTL traceability json. Json = %1").
              arg(gs_buffer_get(tr->buffer)).toUtf8().constData());
        return -1;
    }
    tr->buffer->ptr[tr->buffer->size - 3] = '\0';
    tr->buffer->size -= 3;
    gs_buffer_add(tr->buffer, extraJson.toUtf8().constData());
    const char* json = gs_buffer_get(tr->buffer);
    // UGLY MODE END
    if (this->InsertDataFromJson(dbQuery, json, "ft_gtl_splitlot",
                                 tdrSplitlotId, 0) == false)
    {
        return -1;
    }

    QUERY_FIRST_OR_RETURN(QString("SELECT LAST_INSERT_ID()"))
    return dbQuery.value(0).toUInt();
}

/******************************************************************************!
 * \fn ValidGtlSplitlot
 ******************************************************************************/
int GexDbPlugin_Galaxy::ValidGtlSplitlot(GexDbPlugin_Query& dbQuery,
                                         unsigned int tdrSplitlotId,
                                         unsigned int sqliteSplitlotId)
{
    QUERY_EXEC_OR_RETURN(QString("UPDATE ft_gtl_splitlot "
                                 "SET valid_splitlot='Y' "
                                 "WHERE sqlite_splitlot_id=%1 "
                                 "AND splitlot_id=%2").
                         arg(sqliteSplitlotId).
                         arg(tdrSplitlotId))

    return 0;
}

/******************************************************************************!
 * \fn GetTdrRunId
 * \brief Get the TDR run_id for (sqliteRunId, part_id, site_no)
 ******************************************************************************/
int GexDbPlugin_Galaxy::GetTdrRunId(unsigned int tdrSplitlotId,
                                    int sqliteRunId,
                                    const std::string& lFtRunJson)
{
    QString lRunId;
    QString lSiteNo;
    QString lPartId;
    QString lQuery;
    GexDbPlugin_Query
        dbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->
                                             m_strConnectionName));

    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, lFtRunJson.c_str());
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        char* label = GsJsonIteratorLabel(&jsonIter);
        if (strcmp(label, "run_id") == 0)
        {
            if (! lRunId.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("run_id set multiple times in json: %1").
                      arg(lFtRunJson.c_str()).toUtf8().constData());
                return -1;
            }
            lRunId = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label, "site_no") == 0)
        {
            if (! lSiteNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("site_no set multiple times in json: %1").
                      arg(lFtRunJson.c_str()).toUtf8().constData());
                return -1;
            }
            lSiteNo = GsJsonIteratorValue(&jsonIter);
        }
        else if (strcmp(label, "part_id") == 0)
        {
            if (! lPartId.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("part_id set multiple times in json: %1").
                      arg(lFtRunJson.c_str()).toUtf8().constData());
                return -1;
            }
            lPartId = GsJsonIteratorValue(&jsonIter);

            if (lRunId.isEmpty() ||
                lSiteNo.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Some required fields are missing or empty in "
                              "json: %1").arg(lFtRunJson.c_str()).
                      toUtf8().constData());
                return -1;
            }

            if (lRunId.toInt() != sqliteRunId)
            {
                lRunId = "";
                lSiteNo = "";
                lPartId = "";
                continue;
            }

            lQuery = QString("SELECT run_id FROM ft_run "
                             "WHERE splitlot_id=%1 "
                             "AND site_no=%2 "
                             "AND part_id=%3").
                arg(tdrSplitlotId).
                arg(lSiteNo).
                arg(lPartId);
            if (! dbQuery.Execute(lQuery) || ! dbQuery.First())
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Could not find matching TDR run for "
                              "splitlot_id=%1, part_id=%2, site_no=%3").
                      arg(tdrSplitlotId).
                      arg(lPartId).
                      arg(lSiteNo).toUtf8().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Query = %1").arg(lQuery).toUtf8().constData());
                GSET_LAST_ERROR(lQuery);
                return -1;
            }

            return dbQuery.value(0).toInt();
        }
    }

    return -1;
}

/******************************************************************************!
 * \fn UpdateEventIdAndRunId
 * \brief Update json with the TDR run_id and a new event_id, for each event
 ******************************************************************************/
struct GsBuffer*
GexDbPlugin_Galaxy::UpdateEventIdAndRunId(struct GsBuffer* src,
                                          unsigned int tdrSplitlotId,
                                          const std::string& lFtRunJson)
{
    QString lQuery;
    GexDbPlugin_Query
        dbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->
                                             m_strConnectionName));

    // Get the offset for event_id
    // (We cannot use start_t because it can be the same with two sqlite files)
    lQuery = QString("SELECT MAX(event_id) AS event_id "
                     "FROM ft_event WHERE splitlot_id=%1").
        arg(tdrSplitlotId);
    if (! dbQuery.Execute(lQuery) || ! dbQuery.First())
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error retrieving max event_id from "
                      "ft_event for splitlot %1").arg(tdrSplitlotId).
              toUtf8().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Query = %1").arg(lQuery).toUtf8().constData());
        GSET_LAST_ERROR(lQuery);
        return NULL;
    }
    int offset = dbQuery.value(0).toUInt() + 1;

    char* c = gs_buffer_get(src);
    if (c == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot get buffer");
        return NULL;
    }

    struct GsBuffer* buff;
    buff = gs_buffer_new();
    buff = gs_buffer_init(buff);
    if (buff == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot initialize buffer");
        return NULL;
    }

    while (*c != '\0')
    {
        if (::strncmp(c, "\"event_id\":", 11) == 0)
        {
            c += 11;
            gs_buffer_add(buff, "\"event_id\":%d", atoi(c) + offset);
            while (*c >= '0' &&
                   *c <= '9')
            {
                ++c;
            }
        }
        else if (::strncmp(c, "\"run_id\":", 9) == 0)
        {
            c += 9;
            gs_buffer_add(buff, "\"run_id\":%d",
                          this->GetTdrRunId(tdrSplitlotId,
                                            atoi(c),
                                            lFtRunJson));
            while (*c >= '0' &&
                   *c <= '9')
            {
                ++c;
            }
        }
        else
        {
            gs_buffer_add(buff, "%c", *c);
            ++c;
        }
    }

    return buff;
}

/******************************************************************************!
 * \fn UpdateWithTdrRunId
 * \brief Update json with the TDR run_id
 ******************************************************************************/
struct GsBuffer*
GexDbPlugin_Galaxy::UpdateWithTdrRunId(struct GsBuffer* src,
                                       unsigned int tdrSplitlotId,
                                       const std::string& lFtRunJson,
                                       const std::string& fieldName)
{
    char* c = gs_buffer_get(src);
    if (c == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot get buffer");
        return NULL;
    }

    struct GsBuffer* buff;
    buff = gs_buffer_new();
    buff = gs_buffer_init(buff);
    if (buff == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot initialize buffer");
        return NULL;
    }

    std::string lField = "\"" + fieldName + "\":";
    while (*c != '\0')
    {
        if (::strncmp(c, lField.c_str(), ::strlen(lField.c_str())) == 0)
        {
            c += ::strlen(lField.c_str());
            gs_buffer_add(buff, "%s%d",
                          lField.c_str(),
                          this->GetTdrRunId(tdrSplitlotId,
                                            atoi(c),
                                            lFtRunJson));
            while (*c >= '0' &&
                   *c <= '9')
            {
                ++c;
            }
        }
        else
        {
            gs_buffer_add(buff, "%c", *c);
            ++c;
        }
    }

    return buff;
}

/******************************************************************************!
 * \fn UpdateTestId
 * \brief Update json by translate test_info_id with values in the map
 ******************************************************************************/
struct GsBuffer*
GexDbPlugin_Galaxy::UpdateTestId(struct GsBuffer* src,
                                 QMap<int,int>* testId)
{
    char* c = gs_buffer_get(src);
    if (c == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot get buffer");
        return NULL;
    }

    struct GsBuffer* buff;
    buff = gs_buffer_new();
    buff = gs_buffer_init(buff);
    if (buff == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot initialize buffer");
        return NULL;
    }

    while (*c != '\0')
    {
        if (::strncmp(c, "test_info_id\":", 14) == 0)
        {
            c += 14;
            int i = atoi(c);
            if (! testId->contains(i))
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Cannot find test_info_id key in "
                              "map for id = %1").arg(i).toUtf8().constData());
                gs_buffer_quit(buff);
                delete buff;
                return NULL;
            }
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("test_info_id %1 ==> %2").
                  arg(i).
                  arg(testId->value(i)).toUtf8().constData());
            gs_buffer_add(buff, "test_info_id\":%d", testId->value(i));
            while (*c >= '0' &&
                   *c <= '9')
            {
                ++c;
            }
        }
        else
        {
            gs_buffer_add(buff, "%c", *c);
            ++c;
        }
    }

    return buff;
}

/******************************************************************************!
 * \fn DeleteGsBuffer
 * \brief Called at the end of scope of the shared pointer
 ******************************************************************************/
static void DeleteGsBuffer(struct GsBuffer* buff)
{
    gs_buffer_quit(buff);
    delete buff;
}

/******************************************************************************!
 * \fn InsertDataFileSqlite
 * \brief Main function of this file
 ******************************************************************************/
bool GexDbPlugin_Galaxy::InsertDataFileSqlite(struct GsData* lGsData,
                                              int lSqliteSplitlotId,
                                              const QString& strDataFileName)
{
    QSharedPointer<struct GsBuffer> buff;
    GexDbPlugin_Query
        dbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->
                                             m_strConnectionName));
    int tdrSplitlotId;
    int gtlSplitlotId;
    const char* json;

    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_new");
    struct GsGtlTraceability* tr = gs_gtl_traceability_new();
    tr = gs_gtl_traceability_init_with_gsdata(tr, lGsData);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // sqlite ft_splitlot
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtSplitlot");
    json = gs_gtl_traceability_getFtSplitlot(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    // TDR splitlot_id
    GSLOG(SYSLOG_SEV_DEBUG, "GetTDRSplitlotFromSP");
    tdrSplitlotId =
        this->GetTDRSplitlotFromSP(dbQuery, lSqliteSplitlotId, strDataFileName);
    if (tdrSplitlotId < 0)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              "Failed to retrieve TDR splitlot from stored procedure");
        QString lQuery = "SELECT KEY_NAME, KEY_VALUE FROM keyscontent";
        if(! dbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                        lQuery.toLatin1().constData(),
                        dbQuery.lastError().text().toLatin1().constData());
        }
        else
        {
            while (dbQuery.Next())
            {
                QString lKey = dbQuery.value(0).toString().toLower();
                QString lContent = dbQuery.value(1).toString();
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Key=%1 Content=%2;").
                      arg(lKey).
                      arg(lContent).toUtf8().constData());
            }
        }

        return InsertDataFileSqliteReturn(tr, false);
    }
    // InsertGtlSplitlot
    GSLOG(SYSLOG_SEV_DEBUG, "InsertGtlSplitlot");
    gtlSplitlotId =
        this->InsertGtlSplitlot(dbQuery, tr,
                                lSqliteSplitlotId, tdrSplitlotId);
    if (gtlSplitlotId == -1)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_run
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtRun");
    json = gs_gtl_traceability_getFtRun(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->CheckFTRunFromJson(dbQuery, json,
                                 tdrSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    std::string lFtRunJson(json);  // To update run_id in ft_event

    // ft_ptest_info
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtPtestInfo");
    json = gs_gtl_traceability_getFtPtestInfo(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    QSharedPointer<QMap<int,int> >
        ptestId(this->CheckFtTestInfoFromJson(dbQuery, json, "p",
                                              tdrSplitlotId));
    if (ptestId.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_mptest_info
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtMPtestInfo");
    json = gs_gtl_traceability_getFtMPtestInfo(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    QSharedPointer<QMap<int,int> >
        mptestId(this->CheckFtTestInfoFromJson(dbQuery, json, "mp",
                                               tdrSplitlotId));
    if (mptestId.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_event
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtEvent");
    json = gs_gtl_traceability_getFtEvent(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateEventIdAndRunId(tr->buffer,
                                    tdrSplitlotId,
                                    lFtRunJson), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_event",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_hbin
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtHBin");
    json = gs_gtl_traceability_getFtHBin(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->CheckFtBinFromJson(dbQuery, json, "h",
                                 tdrSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_ptest_outliers
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtPtestOutliers");
    json = gs_gtl_traceability_getFtPtestOutliers(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "limits_run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_ptest_outliers",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_ptest_rollinglimits
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtPtestRollingLimits");
    json =
        gs_gtl_traceability_getFtPtestRollingLimits(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_ptest_rollinglimits",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_ptest_rollingstats
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtPtestRollingStats");
    json = gs_gtl_traceability_getFtPtestRollingStats(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_ptest_rollingstats",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_mptest_outliers
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtMPtestOutliers");
    json = gs_gtl_traceability_getFtMPtestOutliers(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "limits_run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_mptest_outliers",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_mptest_rollinglimits
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtMPtestRollingLimits");
    json =
        gs_gtl_traceability_getFtMPtestRollingLimits(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_mptest_rollinglimits",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_mptest_rollingstats
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtMPtestRollingStats");
    json =
        gs_gtl_traceability_getFtMPtestRollingStats(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateTestId(tr->buffer, ptestId.data()), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    buff = QSharedPointer<struct GsBuffer>(
        this->UpdateWithTdrRunId(buff.data(), tdrSplitlotId, lFtRunJson,
                                 "run_id"), DeleteGsBuffer);
    if (buff.isNull())
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertDataFromJson(dbQuery, gs_buffer_get(buff.data()),
                                 "ft_mptest_rollingstats",
                                 tdrSplitlotId, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_sbin
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtSBin");
    json = gs_gtl_traceability_getFtSBin(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->CheckFtBinFromJson(dbQuery, json, "s",
                                 tdrSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // ft_gtl_info
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getFtGtlInfo");
    json = gs_gtl_traceability_getFtGtlInfo(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("json = %1").
          arg(json).toUtf8().constData());
    if (this->InsertDataFromJson(dbQuery, json, "ft_gtl_info",
                                 0, gtlSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // global_files
    GSLOG(SYSLOG_SEV_DEBUG, "gs_gtl_traceability_getGlobalFileInfos");
    json = gs_gtl_traceability_getGlobalFileInfos(tr, lSqliteSplitlotId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    std::string fileInfos(json);
    int fileId = -1;
    GsJsonIterator jsonIter;
    for (GsJsonIteratorBegin(&jsonIter, fileInfos.c_str());
         GsJsonIteratorEnd(&jsonIter) == 0;
         GsJsonIteratorNext(&jsonIter))
    {
        if (strcmp(GsJsonIteratorLabel(&jsonIter), "file_id") == 0)
        {
            const char* value = GsJsonIteratorValue(&jsonIter);
            fileId = atoi(value);
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("Retrieving data from global_files: "
                          "file_id = %1, value = %2").
                  arg(fileId).arg(value).toUtf8().constData());
        }
    }
    if (fileId < 0)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              "Cannot find file_id when attempting to "
              "retrieve data from global_files");
        GSLOG(SYSLOG_SEV_ERROR, QString("json = %1").arg(json).
              toUtf8().constData());
        return InsertDataFileSqliteReturn(tr, false);
    }

    json = gs_gtl_traceability_getGlobalFileContent(tr, fileId);
    if (gs_gtl_traceability_getLastError(tr))
    {
        return InsertDataFileSqliteReturn(tr, false);
    }
    if (this->InsertGlobalFileFromJson(dbQuery, fileInfos, json,
                                       tdrSplitlotId) == false)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    // Valid gtl splitlot
    GSLOG(SYSLOG_SEV_DEBUG, "ValidGtlSplitlot");
    if (this->ValidGtlSplitlot(dbQuery,
                               tdrSplitlotId, lSqliteSplitlotId) == -1)
    {
        return InsertDataFileSqliteReturn(tr, false);
    }

    return InsertDataFileSqliteReturn(tr, true);
}
