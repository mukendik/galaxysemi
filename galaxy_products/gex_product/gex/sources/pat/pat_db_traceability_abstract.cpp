#include "pat_db_traceability_abstract.h"
#include "pat_db_traceability_v1.h"
#include "pat_db_traceability_v2.h"
#include "pat_db_traceability_v3.h"
#include "cpart_info.h"
#include "gex_report.h"
#include "pat_outliers.h"
#include "gqtl_log.h"
#include "gs_types.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QVariant>

QString TestNumberLabel(int lTestNumber, int lPinIndex)
{
    QString lLabel = QString::number(lTestNumber);

    if (lPinIndex >= 0)
        lLabel += "." + QString::number(lPinIndex);

    return lLabel;
}

namespace GS
{
namespace Gex
{

PATDbTraceabilityAbstract::PATDbTraceabilityAbstract(CGexReport *lReportContext,
                                                     const QString &lDbFilename,
                                                     const QString &lConnectionName)
    : mReportContext(lReportContext), mDbFilename(lDbFilename), mConnectionName(lConnectionName)
{
}

PATDbTraceabilityAbstract::~PATDbTraceabilityAbstract()
{
    if (QSqlDatabase::contains(mConnectionName))
        QSqlDatabase::removeDatabase(mConnectionName);
}

const QString &PATDbTraceabilityAbstract::GetDbFilename() const
{
    return mDbFilename;
}

const QString &PATDbTraceabilityAbstract::GetErrorMessage() const
{
    return mErrorMessage;
}

bool PATDbTraceabilityAbstract::QueryRecipeID(const QString &lProductId, const QString &lLotId,
                                              const QString &lSublotId, int &lRecipeID)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(mConnectionName));
    QString     lQuery;
    int         lCount = 0;

    lQuery = "select recipe_id, count(*) from ft_splitlot ";
    lQuery += "where part_typ='" + lProductId + "' ";
    lQuery += "and lot_id='" + lLotId + "' ";

    if (lSublotId.isEmpty() == false)
        lQuery += "and sublot_id='" + lSublotId + "' ";

    lQuery += "group by recipe_id";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(mDbFilename).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    while(lDbQuery.next())
    {
        lRecipeID       = lDbQuery.value(0).toInt();

        // Count number of rows returned
        ++lCount;
    }

    if (lCount == 0)
    {
        mErrorMessage = QString("No data found for product [%1], lot [%2]")
                        .arg(lProductId).arg(lLotId);

        if (lSublotId.isEmpty() == false)
            mErrorMessage += QString(" and sublot [%1]").arg(lSublotId);

        mErrorMessage += " in PAT traceability DB " + mDbFilename;

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }
    else if (lCount > 1)
    {
        mErrorMessage = QString("Unable to find recipe content. Several recipe correspond to the " \
                                " product [%1], lot [%2]").arg(lProductId).arg(lLotId);

        if (lSublotId.isEmpty() == false)
            mErrorMessage += QString(" and sublot [%1]").arg(lSublotId);

        mErrorMessage += " in PAT traceability DB " + mDbFilename;

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool PATDbTraceabilityAbstract::QueryRecipeContent(int lRecipeID,
                                                QString& lRecipeName, QString &lRecipeContent)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(mConnectionName));
    QString     lQuery;

    lQuery = "select global_files.file_name, global_files.file_content from global_files ";

    // If recipe ID specified apply a filter
    if (lRecipeID >= 0)
        lQuery += "where file_id=" + QString::number(lRecipeID);
    else
        lQuery += "order by file_id";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Always return the first recipe content returned by the query
    if (lDbQuery.next())
    {
        lRecipeName     = lDbQuery.value(0).toString();
        lRecipeContent  = lDbQuery.value(1).toString();
    }
    else
    {
        mErrorMessage = QString("No recipe file found for recipe_id %1 in PAT traceability DB %2")
                        .arg(lRecipeID).arg(GetDbFilename());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool PATDbTraceabilityAbstract::QuerySplitLotCount(int &lSplitlotCount,
                                                   const QString &lProductId,
                                                   const QString &lLotId,
                                                   const QString &lSublotId,
                                                   int lRetestIndex /*= -1*/)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(mConnectionName));
    QString     lQuery;

    lQuery = "select count(*) from ft_splitlot ";
    lQuery += "where part_typ='" + lProductId + "' ";
    lQuery += "and lot_id='" + lLotId + "' ";

    if (lSublotId.isEmpty() == false)
        lQuery += "and sublot_id='" + lSublotId + "' ";

    if (lRetestIndex != -1)
        lQuery += "and retest_index=" + QString::number(lRetestIndex)+ " ";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (lDbQuery.next())
        lSplitlotCount = lDbQuery.value(0).toInt();
    else
        lSplitlotCount = 0;

    return true;
}

bool PATDbTraceabilityAbstract::QueryTotalParts(const QString &lProductId,
                                                const QString &lLotId,
                                                const QString &lSublotId,
                                                int &lTested,
                                                int &lRetested)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(mConnectionName));
    QString     lQuery;

    // Retriev the total parts tested (Initial test phase)
    lQuery = "select sum(nb_parts) from ft_splitlot ";
    lQuery += "where retest_index = 0 and part_typ='" + lProductId + "' ";
    lQuery += "and lot_id='" + lLotId + "' ";

    if (lSublotId.isEmpty() == false)
        lQuery += "and sublot_id='" + lSublotId + "' ";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(mDbFilename).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (lDbQuery.next())
        lTested     = lDbQuery.value(0).toInt();
    else
    {
        mErrorMessage = QString("Unable to find the total parts tested in PAT traceability DB %2")
                        .arg(GetDbFilename());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Retrieve the total parts retested (All retest phase)
    lQuery = "select sum(nb_parts) from ft_splitlot ";
    lQuery += "where retest_index != 0 and part_typ='" + lProductId + "' ";
    lQuery += "and lot_id='" + lLotId + "' ";

    if (lSublotId.isEmpty() == false)
        lQuery += "and sublot_id='" + lSublotId + "' ";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(mDbFilename).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    if (lDbQuery.next())
        lRetested     = lDbQuery.value(0).toInt();
    else
    {
        mErrorMessage = QString("Unable to find the total parts retested in PAT traceability DB %2")
                        .arg(GetDbFilename());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool PATDbTraceabilityAbstract::QueryOutliers(QList<CPatOutlierPart *> &lOutlierList,
                                              QMap<QString, int> &lOutliersCount,
                                              QStringList &lWarnings)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(GetConnectionName()));
    QString     lQuery;

    // Build query in order to get outliers list
    lQuery = BuildQueryOutliers();

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2; %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return ParseQueryOutliers(lDbQuery, lOutlierList, lOutliersCount, lWarnings);
}

bool PATDbTraceabilityAbstract::QueryDPATTestLimits(QList<PATTestLimits> &lTestLimits)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(GetConnectionName()));
    QString     lQuery;

    // Build query to get DPAT Test limits
    lQuery = BuildQueryDPATTestLimits();

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2; %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return ParseQueryPATTestLimits(lDbQuery, lTestLimits);
}

bool PATDbTraceabilityAbstract::QuerySPATTestLimits(QList<PATTestLimits> &lTestLimits)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(GetConnectionName()));
    QString     lQuery;

    // Build query to get DPAT Test limits
    lQuery = BuildQuerySPATTestLimits();

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return ParseQueryPATTestLimits(lDbQuery, lTestLimits);
}

bool PATDbTraceabilityAbstract::QueryRollingLimits(QStringList &lWarnings)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(GetConnectionName()));
    QString     lQuery;

    // Buildquery to get Rolling limits
    lQuery = BuildQueryRollingLimits();

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2; %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    return ParseQueryRollingLimits(lDbQuery, lWarnings);
}

PATDbTraceabilityAbstract *PATDbTraceabilityAbstract::CreateDbTraceability(CGexReport * lReportContext,
                                                                           const QString &lDbFilename,
                                                                           QString& lErrorMessage)
{
    PATDbTraceabilityAbstract * lDbTraceability = NULL;

    if (lDbFilename.isEmpty())
    {
        lErrorMessage = QString("PAT Treaceability file name is empty.");
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return NULL;
    }

    if (lReportContext == NULL)
    {
        lErrorMessage = QString("No report context associated with the PAT traceability file.");
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return NULL;
    }

    QString         lConnectionName = QUuid::createUuid().toString();
    QSqlDatabase    lDbSqlite       = QSqlDatabase::addDatabase("QSQLITE", lConnectionName);

    lDbSqlite.setDatabaseName(lDbFilename);

    // Open database
    if (lDbSqlite.open())
    {
        int         lDBVersion      = -1;
        QSqlQuery   lDbQuery(lDbSqlite);
        QString     lQuery;
        gsint8      lRowCount = 0;

        // Query the shema to find the right table name where the GTL infos are kept
        lQuery = "SELECT * FROM sqlite_master WHERE type='table' and name='global_settings'";

        if (lDbQuery.exec(lQuery) == false)
        {
            lErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2")
                            .arg(lQuery).arg(lDbFilename);

            GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
            return NULL;
        }
        else
        {
            if (lDbQuery.next() == false)
            {
                lQuery = "SELECT * FROM sqlite_master WHERE type='table' and name='ft_gtl_info'";

                if (lDbQuery.exec(lQuery) == false)
                {
                    lErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2")
                                    .arg(lQuery).arg(lDbFilename);

                    GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
                    return NULL;
                }

                // Check if table name has been found
                if (lDbQuery.next())
                {
                    lQuery = "select gtl_value from ft_gtl_info where gtl_key='db_version_nb' group by gtl_value";
                }
                else
                {
                    lErrorMessage = QString("Failed to retrieve global_settings or ft_gtl_info tables from PAT traceability DB %2")
                                    .arg(lDbFilename);

                    GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
                    return NULL;
                }
            }
            else
            {
                lQuery = "select value from global_settings where key='db_version_nb' group by value";
            }
        }

        // Query the global_settings or ft_gtl_info table to find the schema version
        if (lDbQuery.exec(lQuery) == false)
        {
            lErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2")
                            .arg(lQuery).arg(lDbFilename);

            GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
            return NULL;
        }

        while (lDbQuery.next())
        {
            lDBVersion = lDbQuery.value(0).toInt();

            // Nb returned rows found
            ++lRowCount;
        }

        // Check how many DB schema version found.
        // If not 1, returns an error
        if (lRowCount == 0)
        {
            lErrorMessage = QString("DB schema version not found in PAT traceability DB %1")
                            .arg(lDbFilename);

            GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
            return NULL;
        }
        else if(lRowCount > 1)
        {
            lErrorMessage = QString("Multiple DB schema versions found in PAT traceability DB %1")
                            .arg(lDbFilename);

            GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
            return NULL;
        }

        // Create the DB Traceability accessor according to the DB version
        switch(lDBVersion)
        {
            case DBVersion1:
                // Instanciate DBTraceability for Version 1
                lDbTraceability = new PATDbTraceabilityV1(lReportContext, lDbFilename, lConnectionName);
                break;

            case DBVersion2:
                // Instanciate DBTraceability for Version 2
                lDbTraceability = new PATDbTraceabilityV2(lReportContext, lDbFilename, lConnectionName);;
                break;

            case DBVersion3:
                // Instanciate DBTraceability for Version 3
                lDbTraceability = new PATDbTraceabilityV3(lReportContext, lDbFilename, lConnectionName);;
                break;

            default:
                lErrorMessage = QString("DB schema version %1 is not supported")
                                .arg(lDBVersion);

                GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
                break;
        }
    }
    else
    {
        lErrorMessage = QString("Unable open the PAT traceability DB %1: %2").arg(lDbFilename)
                            .arg(lDbSqlite.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
    }

    return lDbTraceability;
}

const QString &PATDbTraceabilityAbstract::GetConnectionName() const
{
    return mConnectionName;
}

CGexReport *PATDbTraceabilityAbstract::GetReportContext() const
{
    return mReportContext;
}

QString PATDbTraceabilityAbstract::BuildQueryRollingLimits() const
{
    QString lQuery;

    lQuery = "select * from (";
    lQuery += "select 'P' as ttype, ft_ptest_info.tnum as tnum, '-1' as tpinindex, ";
    lQuery += "ft_ptest_info.tname as tname, ft_run.site_no as siteno, ft_run.part_id as partid, ";
    lQuery += "ft_ptest_rollinglimits.LL as llimit, ft_ptest_rollinglimits.HL as hlimit, ";
    lQuery += "ft_run.run_id as runid, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_run.splitlot_id= ft_ptest_info.splitlot_id and ";
    lQuery += "ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_ptest_info.splitlot_id ";
    lQuery += "union ";
    lQuery += "select 'M' as ttype, ft_mptest_info.tnum as tnum, ";
    lQuery += "ft_mptest_info.tpin_arrayindex as tpinindex, ft_mptest_info.tname as tname, ";
    lQuery += "ft_run.site_no as siteno, ft_run.part_id as partid, ";
    lQuery += "ft_mptest_rollinglimits.LL as llimit, ft_mptest_rollinglimits.HL as hlimit, ";
    lQuery += "ft_run.run_id as runid, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_mptest_info ";
    lQuery += "inner join ft_mptest_rollinglimits ";
    lQuery += "on ft_mptest_info.splitlot_id=ft_mptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_mptest_info.mptest_info_id=ft_mptest_rollinglimits.mptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_run.splitlot_id= ft_mptest_info.splitlot_id and ";
    lQuery += "ft_mptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_mptest_info.splitlot_id ) ";
    lQuery += "where retest_index == 0 ";
    lQuery += "order by runid, siteno, partid;";

    return lQuery;
}

QString PATDbTraceabilityAbstract::BuildQuerySPATTestLimits() const
{
    QString lQuery;

    lQuery =  "select * from (";
    lQuery += "select 'P' as ttype, ft_ptest_info.tnum as tnum, '-1' as tpinindex, ";
    lQuery += "ft_ptest_info.tname as tname, ft_run.site_no as siteno, ft_run.run_id as runid, ";
    lQuery += "ft_ptest_rollinglimits.limit_mode as limitmode, ft_ptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_ptest_rollinglimits.HL as hlimit, ft_ptest_rollingstats.distribution_shape as shape, ";
    lQuery += "ft_ptest_rollingstats.n_factor as nfactor, ft_ptest_rollingstats.t_factor as tfactor, ";
    lQuery += "ft_ptest_rollingstats.mean as tmean, ft_ptest_rollingstats.sigma as tsigma, ";
    lQuery += "ft_ptest_rollingstats.q1 as tq1, ft_ptest_rollingstats.median as tmedian, ";
    lQuery += "ft_ptest_rollingstats.q3 as tq3, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_ptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_ptest_rollingstats ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollingstats.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollingstats.ptest_info_id and ft_ptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_ptest_info.splitlot_id ";
    lQuery += "where ft_ptest_rollinglimits.limit_type='S' ";
    lQuery += "union ";
    lQuery += "select 'M' as ttype, ft_mptest_info.tnum as tnum, ft_mptest_info.tpin_arrayindex as tpinindex, ";
    lQuery += "ft_mptest_info.tname as tname, ft_run.site_no as siteno, ft_run.run_id as runid, ";
    lQuery += "ft_mptest_rollinglimits.limit_mode as limitmode, ft_mptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_mptest_rollinglimits.HL as hlimit, ft_mptest_rollingstats.distribution_shape as shape, ";
    lQuery += "ft_mptest_rollingstats.n_factor as nfactor, ft_mptest_rollingstats.t_factor as tfactor, ";
    lQuery += "ft_mptest_rollingstats.mean as tmean, ft_mptest_rollingstats.sigma as tsigma, ";
    lQuery += "ft_mptest_rollingstats.q1 as tq1, ft_mptest_rollingstats.median as tmedian, ";
    lQuery += "ft_mptest_rollingstats.q3 as tq3, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_mptest_info ";
    lQuery += "inner join ft_mptest_rollinglimits ";
    lQuery += "on ft_mptest_info.splitlot_id=ft_mptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_mptest_info.mptest_info_id=ft_mptest_rollinglimits.mptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_mptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_mptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_mptest_rollingstats ";
    lQuery += "on ft_mptest_info.splitlot_id=ft_mptest_rollingstats.splitlot_id and ";
    lQuery += "ft_mptest_info.mptest_info_id=ft_mptest_rollingstats.mptest_info_id and ft_mptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_mptest_info.splitlot_id ";
    lQuery += "where ft_mptest_rollinglimits.limit_type='S') ";
    lQuery += "order by tnum, tpinindex, tname, siteno, retest_index, runid";

    return lQuery;
}

QString PATDbTraceabilityAbstract::BuildQueryDPATTestLimits() const
{
    QString lQuery;

    lQuery =  "select * from (";
    lQuery += "select 'P' as ttype, ft_ptest_info.tnum as tnum, '-1' as tpinindex, ";
    lQuery += "ft_ptest_info.tname as tname, ft_run.site_no as siteno, ft_run.run_id as runid, ";
    lQuery += "ft_ptest_rollinglimits.limit_mode as limitmode, ft_ptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_ptest_rollinglimits.HL as hlimit, ft_ptest_rollingstats.distribution_shape as shape, ";
    lQuery += "ft_ptest_rollingstats.n_factor as nfactor, ft_ptest_rollingstats.t_factor as tfactor, ";
    lQuery += "ft_ptest_rollingstats.mean as tmean, ft_ptest_rollingstats.sigma as tsigma, ";
    lQuery += "ft_ptest_rollingstats.q1 as tq1, ft_ptest_rollingstats.median as tmedian, ";
    lQuery += "ft_ptest_rollingstats.q3 as tq3, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_ptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_ptest_rollingstats ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollingstats.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollingstats.ptest_info_id and ft_ptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_ptest_info.splitlot_id ";
    lQuery += "where ft_ptest_rollinglimits.limit_type='D' ";
    lQuery += "union ";
    lQuery += "select 'M' as ttype, ft_mptest_info.tnum as tnum, ft_mptest_info.tpin_arrayindex as tpinindex, ";
    lQuery += "ft_mptest_info.tname as tname, ft_run.site_no as siteno, ft_run.run_id as runid, ";
    lQuery += "ft_mptest_rollinglimits.limit_mode as limitmode, ft_mptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_mptest_rollinglimits.HL as hlimit, ft_mptest_rollingstats.distribution_shape as shape, ";
    lQuery += "ft_mptest_rollingstats.n_factor as nfactor, ft_mptest_rollingstats.t_factor as tfactor, ";
    lQuery += "ft_mptest_rollingstats.mean as tmean, ft_mptest_rollingstats.sigma as tsigma, ";
    lQuery += "ft_mptest_rollingstats.q1 as tq1, ft_mptest_rollingstats.median as tmedian, ";
    lQuery += "ft_mptest_rollingstats.q3 as tq3, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_mptest_info ";
    lQuery += "inner join ft_mptest_rollinglimits ";
    lQuery += "on ft_mptest_info.splitlot_id=ft_mptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_mptest_info.mptest_info_id=ft_mptest_rollinglimits.mptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_mptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_mptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_mptest_rollingstats ";
    lQuery += "on ft_mptest_info.splitlot_id=ft_mptest_rollingstats.splitlot_id and ";
    lQuery += "ft_mptest_info.mptest_info_id=ft_mptest_rollingstats.mptest_info_id and ft_mptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "inner join ft_splitlot ";
    lQuery += "on ft_splitlot.splitlot_id=ft_mptest_info.splitlot_id ";
    lQuery += "where ft_mptest_rollinglimits.limit_type='D') ";
    lQuery += "order by tnum, tpinindex, tname, siteno, retest_index, runid";

    return lQuery;
}

QString PATDbTraceabilityAbstract::BuildQueryOutliers() const
{
    // This query is used for both V2 and V3 schema.
    // MODIFYING THIS QUERY MIGHT BREAK REPORTING FEATURES FOR V2 OR V3.
    // IF THIS QUERY HAS TO BE MODIFIED FOR ANY REASONS, YOU MIGHT HAVE TO REIMPLEMENT IT IN THE INHERITED CLASSES
    QString lQuery;

    lQuery =  "select * from (";
    lQuery += "select ft_run.run_id as runid, ft_splitlot.lot_id as lotid, ft_hbin.hbin_no as hbinno, ";
    lQuery += "ft_hbin.bin_subfamily as binsubfamily, ft_run.site_no as site_no, ft_run.part_id as partid, ";
    lQuery += "ft_ptest_outliers.value as tvalue, 'P' as ttype, ft_ptest_info.tnum as tnum, ";
    lQuery += "'-1' as tpinindex, ft_ptest_info.tname as tname, ft_ptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_ptest_rollinglimits.HL as hlimit, ft_ptest_rollinglimits.limit_type as limittype, ";
    lQuery += "ft_ptest_outliers.limits_run_id as limit_runid, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_splitlot ";
    lQuery += "inner join ft_event ";
    lQuery += "on ft_splitlot.splitlot_id=ft_event.splitlot_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_event.run_id=ft_run.run_id and ft_event.splitlot_id=ft_run.splitlot_id ";
    lQuery += "inner join ft_hbin ";
    lQuery += "on ft_run.hbin_no=ft_hbin.hbin_no and ft_run.splitlot_id=ft_hbin.splitlot_id ";
    lQuery += "inner join ft_ptest_outliers ";
    lQuery += "on ft_ptest_outliers.run_id=ft_run.run_id  and ";
    lQuery += "ft_ptest_outliers.splitlot_id=ft_run.splitlot_id ";
    lQuery += "inner join ft_ptest_info ";
    lQuery += "on ft_ptest_info.ptest_info_id=ft_ptest_outliers.ptest_info_id ";
    lQuery += "and ft_ptest_outliers.splitlot_id=ft_ptest_info.splitlot_id ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_rollinglimits.splitlot_id= ft_ptest_outliers.splitlot_id and ";
    lQuery += "ft_ptest_rollinglimits.ptest_info_id= ft_ptest_outliers.ptest_info_id and ";
    lQuery += "ft_ptest_rollinglimits.run_id=ft_ptest_outliers.limits_run_id ";
    lQuery += "where ft_hbin.bin_family='PAT' ";
    lQuery += "union ";
    lQuery += "select ft_run.run_id as runid, ft_splitlot.lot_id as lotid, ft_hbin.hbin_no as hbinno, ";
    lQuery += "ft_hbin.bin_subfamily as binsubfamily, ft_run.site_no as site_no, ft_run.part_id as partid, ";
    lQuery += "ft_mptest_outliers.value as tvalue, 'M' as ttype, ft_mptest_info.tnum as tnum, ";
    lQuery += "ft_mptest_info.tpin_arrayindex as tpinindex, ft_mptest_info.tname as tname, ";
    lQuery += "ft_mptest_rollinglimits.LL as llimit, ";
    lQuery += "ft_mptest_rollinglimits.HL as hlimit, ft_mptest_rollinglimits.limit_type as limittype, ";
    lQuery += "ft_mptest_outliers.limits_run_id as limit_runid, ft_splitlot.retest_index as retest_index ";
    lQuery += "from ft_splitlot ";
    lQuery += "inner join ft_event ";
    lQuery += "on ft_splitlot.splitlot_id=ft_event.splitlot_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_event.run_id=ft_run.run_id and ft_event.splitlot_id=ft_run.splitlot_id ";
    lQuery += "inner join ft_hbin ";
    lQuery += "on ft_run.hbin_no=ft_hbin.hbin_no and ft_run.splitlot_id=ft_hbin.splitlot_id ";
    lQuery += "inner join ft_mptest_outliers ";
    lQuery += "on ft_mptest_outliers.run_id=ft_run.run_id  and ";
    lQuery += "ft_mptest_outliers.splitlot_id=ft_run.splitlot_id ";
    lQuery += "inner join ft_mptest_info ";
    lQuery += "on ft_mptest_info.mptest_info_id=ft_mptest_outliers.mptest_info_id ";
    lQuery += "and ft_mptest_outliers.splitlot_id=ft_mptest_info.splitlot_id ";
    lQuery += "inner join ft_mptest_rollinglimits ";
    lQuery += "on ft_mptest_rollinglimits.splitlot_id= ft_mptest_outliers.splitlot_id and ";
    lQuery += "ft_mptest_rollinglimits.mptest_info_id= ft_mptest_outliers.mptest_info_id and ";
    lQuery += "ft_mptest_rollinglimits.run_id=ft_mptest_outliers.limits_run_id ";
    lQuery += "where ft_hbin.bin_family='PAT') ";
    lQuery += "order by retest_index, runid";

    return lQuery;
}

bool PATDbTraceabilityAbstract::ParseQueryOutliers(QSqlQuery& lDbQuery,
                                                   QList<CPatOutlierPart*>& lOutlierList,
                                                   QMap<QString, int> &lOutliersCount,
                                                   QStringList &lWarnings)
{
    QString               lLimitType;
    bool                lFindRun            = false;
    int                 lPreviousRunID      = -1;
    int                 lRunID              = -1;
    int                 lPreviousTestNumber = -1;
    int                 lTestNumber         = -1;
    int                 lLimitsRunID        = 0;
    CPatOutlierPart *   lOutlier            = NULL;
    CPartInfo *         lPartInfo           = NULL;
    CGexGroupOfFiles *  lGroup              = NULL;
    CGexFileInGroup *   lFile               = NULL;
    CPatFailingTest     lFailingTest;
    QMap<int, QPair<int, QString> > lSitesInfo;
    QPair<int, QString>             lSiteInfo;

    while (lDbQuery.next())
    {
        lRunID      = lDbQuery.value(0).toInt();
        lTestNumber = lDbQuery.value(8).toInt();

        if (lRunID != lPreviousRunID)
        {
            // Insert failing test to current outlier
            if (lPreviousTestNumber != -1)
            {
                QString lKey = QString("%1:%2:%3")
                               .arg(TestNumberLabel(lFailingTest.mTestNumber, lFailingTest.mPinIndex))
                               .arg(lFailingTest.mSite).arg(lLimitsRunID);

                if (lOutliersCount.contains(lKey))
                    lOutliersCount[lKey]++;
                else
                    lOutliersCount.insert(lKey, 1);

                lOutlier->cOutlierList.append(lFailingTest);
            }

            lOutlier            = new CPatOutlierPart;
            lPreviousRunID      = lRunID;
            lPreviousTestNumber = -1;

            lLimitsRunID        = lDbQuery.value(14).toInt();
            lOutlier->iPatHBin  = lDbQuery.value(2).toInt();
            lOutlier->iSite     = lDbQuery.value(4).toInt();
            lOutlier->strPartID = lDbQuery.value(5).toString();
            lOutlier->iDieX     = GEX_WAFMAP_INVALID_COORD;
            lOutlier->iDieY     = GEX_WAFMAP_INVALID_COORD;
            lOutlier->lRunID    = -1;
            lOutlier->mRetestIndex  = lDbQuery.value(15).toInt();

            if (lSitesInfo.contains(lOutlier->iSite))
                lSiteInfo   = lSitesInfo.value(lOutlier->iSite);
            else
            {
                lSiteInfo.first     = 0;
                lSiteInfo.second    = "";

                lSitesInfo.insert(lOutlier->iSite, lSiteInfo);
            }

            int lGroupId = GetReportContext()->getGroupForSite(lOutlier->iSite);

            if (lGroupId != -1)
                lGroup = GetReportContext()->getGroupsList().at(lGroupId);

            if (lGroup)
                lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
            else
                lFile = NULL;

            // Looking for the run id
            lFindRun = false;

            if (lFile)
            {
                int lIdx;
                for(lIdx = lSiteInfo.first; lIdx < lFile->pPartInfoList.count(); ++lIdx)
                {
                    lPartInfo = lFile->pPartInfoList.at(lIdx);

                    if (lOutlier->iSite == lPartInfo->m_site &&
                        lOutlier->strPartID == lPartInfo->getPartID())
                    {
                        lOutlier->iDieX     = lPartInfo->iDieX;
                        lOutlier->iDieY     = lPartInfo->iDieY;
                        lOutlier->lRunID    = lIdx+1;

                        lFindRun = true;
                        break;
                    }
                }

                // Update site info
                lSiteInfo.first     = lIdx;
            }

            // Update last part id for that site
            lSiteInfo.second    = lOutlier->strPartID;

            lSitesInfo.insert(lOutlier->iSite, lSiteInfo);

            if (lFindRun == false)
            {
                QString lMessage;

                lMessage = "Unable to find any matching run for outlier on site ";
                lMessage += QString("%1 and part_id %2.").arg(lOutlier->iSite).arg(lOutlier->strPartID);

                lWarnings.append(lMessage);

                GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().constData());
            }

            lOutlierList.append(lOutlier);
        }

        // New test number found, create a new failing test
        if (lPreviousTestNumber != lTestNumber)
        {
            if (lPreviousTestNumber != -1)
            {
                QString lKey = QString("%1:%2:%3")
                               .arg(TestNumberLabel(lFailingTest.mTestNumber, lFailingTest.mPinIndex))
                               .arg(lFailingTest.mSite).arg(lLimitsRunID);

                if (lOutliersCount.contains(lKey))
                    lOutliersCount[lKey]++;
                else
                    lOutliersCount.insert(lKey, 1);

                lOutlier->cOutlierList.append(lFailingTest);
            }

            lFailingTest.mSite          = lDbQuery.value(4).toInt();
            lFailingTest.mValue         = lDbQuery.value(6).toDouble();
            lFailingTest.mTestNumber    = lDbQuery.value(8).toInt();
            lFailingTest.mPinIndex      = lDbQuery.value(9).toInt();;
            lFailingTest.mTestName      = lDbQuery.value(10).toString();
            lFailingTest.mLowLimit      = lDbQuery.value(11).toDouble();
            lFailingTest.mHighLimit     = lDbQuery.value(12).toDouble();

            if (lFailingTest.mValue < lFailingTest.mLowLimit)
                lFailingTest.mFailureDirection = -1;
            else if (lFailingTest.mValue > lFailingTest.mHighLimit)
                lFailingTest.mFailureDirection = +1;

            lLimitType = lDbQuery.value(13).toString();

            if (lLimitType == "S")
                lFailingTest.mFailureMode = 0;
            else if (lLimitType == "D")
                lFailingTest.mFailureMode = 1;
            else
                lFailingTest.mFailureMode = 1;
        }
        else
        {
            double lLL  = lDbQuery.value(11).toDouble();
            double lHL  = lDbQuery.value(12).toDouble();

            if (lFailingTest.mValue < lFailingTest.mLowLimit || lFailingTest.mValue < lLL)
                lFailingTest.mFailureDirection = -1;
            else if (lFailingTest.mValue > lFailingTest.mHighLimit || lFailingTest.mValue > lHL)
                lFailingTest.mFailureDirection = +1;

            lFailingTest.mLowLimit     = qMin(lFailingTest.mLowLimit, lLL);
            lFailingTest.mHighLimit    = qMax(lFailingTest.mHighLimit, lHL);
        }

        lPreviousTestNumber = lTestNumber;
    }

    if (lPreviousTestNumber != -1)
    {
        QString lKey = QString("%1:%2:%3")
                       .arg(TestNumberLabel(lFailingTest.mTestNumber, lFailingTest.mPinIndex))
                       .arg(lFailingTest.mSite).arg(lLimitsRunID);

        if (lOutliersCount.contains(lKey))
            lOutliersCount[lKey]++;
        else
            lOutliersCount.insert(lKey, 1);

        lOutlier->cOutlierList.append(lFailingTest);
    }

    return true;
}

bool PATDbTraceabilityAbstract::ParseQueryPATTestLimits(QSqlQuery &lDbQuery,
                                                         QList<PATTestLimits> &lTestLimits)
{
    int             lPreviousRunID          = -1;
    int             lRunID                  = -1;
    int             lPreviousTestNumber     = -1;
    int             lTestNumber             = -1;
    int             lPreviousPinIndex       = -1;
    int             lPinIndex               = -1;
    int             lPreviousRetestIndex    = -1;
    int             lRetestIndex            = -1;
    PATTestLimits   lPATTestLimits;

    while (lDbQuery.next())
    {
        lRunID                              = lDbQuery.value(5).toInt();
        lTestNumber                         = lDbQuery.value(1).toInt();
        lPinIndex                           = lDbQuery.value(2).toInt();
        lRetestIndex                        = lDbQuery.value(17).toInt();

        if (lTestNumber == lPreviousTestNumber && lPinIndex == lPreviousPinIndex &&
            lRunID == lPreviousRunID && lRetestIndex == lPreviousRetestIndex)
        {
            lPATTestLimits.mPATLL = qMin(lPATTestLimits.mPATLL, lDbQuery.value(7).toDouble());
            lPATTestLimits.mPATHL = qMax(lPATTestLimits.mPATHL, lDbQuery.value(8).toDouble());
        }
        else
        {
            if (lPreviousTestNumber != -1)
                lTestLimits.append(lPATTestLimits);

            lPATTestLimits.mTestNumber          = lDbQuery.value(1).toInt();
            lPATTestLimits.mPinIndex            = lDbQuery.value(2).toInt();
            lPATTestLimits.mTestName            = lDbQuery.value(3).toString();
            lPATTestLimits.mSite                = lDbQuery.value(4).toInt();
            lPATTestLimits.mPATLL               = lDbQuery.value(7).toDouble();
            lPATTestLimits.mPATHL               = lDbQuery.value(8).toDouble();
            lPATTestLimits.mDistributionShape   = lDbQuery.value(9).toString();
            lPATTestLimits.mNFactor             = lDbQuery.value(10).toDouble();
            lPATTestLimits.mTFactor             = lDbQuery.value(11).toDouble();
            lPATTestLimits.mMean                = lDbQuery.value(12).toDouble();
            lPATTestLimits.mSigma               = lDbQuery.value(13).toDouble();
            lPATTestLimits.mQ1                  = lDbQuery.value(14).toDouble();
            lPATTestLimits.mMedian              = lDbQuery.value(15).toDouble();
            lPATTestLimits.mQ3                  = lDbQuery.value(16).toDouble();
            lPATTestLimits.mRunID               = lRunID;
            lPATTestLimits.mRetestIndex         = lRetestIndex;
        }

        lPreviousRunID          = lRunID;
        lPreviousTestNumber     = lTestNumber;
        lPreviousPinIndex       = lPinIndex;
        lPreviousRetestIndex    = lRetestIndex;
    }

    if (lPreviousTestNumber != -1)
        lTestLimits.append(lPATTestLimits);

    return true;
}

bool PATDbTraceabilityAbstract::ParseQueryRollingLimits(QSqlQuery &lDbQuery, QStringList &lWarnings)
{
    bool                lFindRun                = false;
    int                 lPreviousSite           = -1;
    int                 lSite                   = -1;
    int                 lRunID                  = -1;
    int                 lTestNumber             = -1;
    int                 lPinIndex               = -1;
    QString             lTestName;
    QString             lPartID;
    QString             lPreviousPartID;
    CTest *             lTestCell           = NULL;
    CGexGroupOfFiles *  lGroup              = NULL;
    CGexFileInGroup *   lFile               = NULL;
    QMap<int, QPair<int, QString> > lSitesInfo;
    QPair<int, QString>             lSiteInfo;

    // Reset site info
    lSitesInfo.clear();

    while (lDbQuery.next())
    {
        lTestNumber     = lDbQuery.value(1).toInt();
        lPinIndex       = lDbQuery.value(2).toInt();
        lTestName       = lDbQuery.value(3).toString();
        lSite           = lDbQuery.value(4).toInt();
        lPartID         = lDbQuery.value(5).toString();

        if (lPreviousSite != lSite)
        {
            int lGroupId = GetReportContext()->getGroupForSite(lSite);

            if (lGroupId != -1)
                lGroup = GetReportContext()->getGroupsList().at(lGroupId);

            if (lGroup)
                lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
            else
                lFile = NULL;

            if (lSitesInfo.contains(lSite))
                lSiteInfo   = lSitesInfo.value(lSite);
            else
            {
                lSiteInfo.first     = 0;
                lSiteInfo.second    = "";

                lSitesInfo.insert(lSite, lSiteInfo);
            }
        }

        if (lFile)
        {
            if (lSiteInfo.second != lPartID)
            {
                lFindRun = false;
                int lIdx;

                for(lIdx = lSiteInfo.first; lIdx < lFile->pPartInfoList.count(); ++lIdx)
                {
                    if (lSite == lFile->pPartInfoList.at(lIdx)->m_site &&
                        lPartID == lFile->pPartInfoList.at(lIdx)->getPartID())
                    {
                        lFindRun    = true;
                        break;
                    }
                }

                lSiteInfo.first     = lIdx;
                lSiteInfo.second    = lPartID;

                lSitesInfo.insert(lSite, lSiteInfo);
            }
            else
                lFindRun = true;

            if (lFindRun == true)
            {
                lRunID      = lSiteInfo.first+1;

                // HTH: Don't use test name for now!!! as FT PAT only support test# as key
                if (lFile->FindTestCell(lTestNumber, lPinIndex, &lTestCell, true, false) == 1)
                {
                    lTestCell->GetRollingLimits().AddLimits(lRunID,
                                                            lDbQuery.value(6).toDouble(),
                                                            lDbQuery.value(7).toDouble());
                }
            }
            else
            {
                QString lMessage;

                lMessage = QString("Unable to set new rolling limits for test# %1")
                           .arg(TestNumberLabel(lTestNumber, lPinIndex));
                lMessage += QString(" with site %1 and part_id %2")
                            .arg(lSite).arg(lPartID);
                lMessage += " for Initial Test: no matching run found.";

                lWarnings.append(lMessage);

                GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().constData());
            }
        }

        lPreviousSite           = lSite;
        lPreviousPartID         = lPartID;
    }

    return true;
}

bool PATDbTraceabilityAbstract::QueryWarningsLog(QStringList &lWarnings)
{
    QSqlQuery   lDbQuery(QSqlDatabase::database(mConnectionName));
    QString     lQuery;

    // Query warning logs
    lQuery = "select ft_event.event_message ";
    lQuery += "from ft_event ";
    lQuery += "where ft_event.event_type='GTM_NOTIFICATION' and ";
    lQuery += "ft_event.event_message like '%Too many outliers%'";

    if (lDbQuery.exec(lQuery) == false)
    {
        mErrorMessage = QString("Failed to execute query %1 on PAT traceability DB %2: %3")
                        .arg(lQuery).arg(GetDbFilename()).arg(lDbQuery.lastError().text());

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    while (lDbQuery.next())
    {
        lWarnings.append(lDbQuery.value(0).toString());
    }

    return true;
}

}
}
