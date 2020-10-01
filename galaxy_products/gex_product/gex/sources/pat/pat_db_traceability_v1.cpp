#include "pat_db_traceability_v1.h"
#include "cpart_info.h"
#include "gex_report.h"
#include "pat_outliers.h"
#include "gqtl_log.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QPair>
#include <QVariant>

namespace GS
{
namespace Gex
{

PATDbTraceabilityV1::PATDbTraceabilityV1(CGexReport * lReportContext,
                                         const QString &lDbFilename, const QString &lConnectionName)
    : PATDbTraceabilityAbstract(lReportContext, lDbFilename, lConnectionName)
{
}

PATDbTraceabilityV1::~PATDbTraceabilityV1()
{
}

QString PATDbTraceabilityV1::BuildQueryOutliers() const
{
    QString lQuery;

    lQuery =  "select ft_run.run_id as runid, ft_splitlot.lot_id as lotid, ft_hbin.hbin_no as hbinno, ";
    lQuery += "ft_hbin.bin_subfamily as binsubfamily, ft_run.site_no, ft_run.part_id as partid, ";
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
    lQuery += "order by retest_index, ft_run.run_id";

    return lQuery;
}

QString PATDbTraceabilityV1::BuildQueryDPATTestLimits() const
{
    QString lQuery;

    lQuery =  "select 'P' as ttype, ft_ptest_info.tnum, '-1' as tpinindex, ft_ptest_info.tname, ";
    lQuery += "ft_run.site_no, ft_run.run_id, ft_ptest_rollinglimits.limit_mode, ";
    lQuery += "ft_ptest_rollinglimits.LL, ft_ptest_rollinglimits.HL, ";
    lQuery += "ft_ptest_rollingstats.distribution_shape, ft_ptest_rollingstats.n_factor, ";
    lQuery += "ft_ptest_rollingstats.t_factor, ft_ptest_rollingstats.mean, ft_ptest_rollingstats.sigma, ";
    lQuery += "ft_ptest_rollingstats.q1, ft_ptest_rollingstats.median, ";
    lQuery += "ft_ptest_rollingstats.q3, '0' as retest_index ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_ptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_ptest_rollingstats ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollingstats.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollingstats.ptest_info_id and ft_ptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "where ft_ptest_rollinglimits.limit_type='D' ";
    lQuery += "order by ft_ptest_info.tnum, tpinindex, ft_ptest_info.tname, ft_run.site_no, ft_run.run_id";

    return lQuery;
}

QString PATDbTraceabilityV1::BuildQuerySPATTestLimits() const
{
    QString lQuery;

    lQuery =  "select 'P' as ttype, ft_ptest_info.tnum, '-1' as tpinindex, ft_ptest_info.tname, ";
    lQuery += "ft_run.site_no, ft_run.run_id, ft_ptest_rollinglimits.limit_mode, ";
    lQuery += "ft_ptest_rollinglimits.LL, ft_ptest_rollinglimits.HL, ";
    lQuery += "ft_ptest_rollingstats.distribution_shape, ft_ptest_rollingstats.n_factor, ";
    lQuery += "ft_ptest_rollingstats.t_factor, ft_ptest_rollingstats.mean, ft_ptest_rollingstats.sigma, ";
    lQuery += "ft_ptest_rollingstats.q1, ft_ptest_rollingstats.median, ";
    lQuery += "ft_ptest_rollingstats.q3, '0' as retest_index ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_ptest_rollinglimits.splitlot_id=ft_run.splitlot_id and ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "inner join ft_ptest_rollingstats ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollingstats.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollingstats.ptest_info_id and ft_ptest_rollingstats.run_id=ft_run.run_id ";
    lQuery += "where ft_ptest_rollinglimits.limit_type='S' ";
    lQuery += "order by ft_ptest_info.tnum, tpinindex, ft_ptest_info.tname, ft_run.site_no, ft_run.run_id";

    return lQuery;
}

QString PATDbTraceabilityV1::BuildQueryRollingLimits() const
{
    QString lQuery;

    lQuery =  "select 'P' as ttype, ft_ptest_info.tnum, '-1' as tpinindex, ft_ptest_info.tname, ";
    lQuery += "ft_run.site_no, ft_run.part_id, ft_ptest_rollinglimits.LL, ft_ptest_rollinglimits.HL, ";
    lQuery += "ft_run.run_id as runid ";
    lQuery += "from ft_ptest_info ";
    lQuery += "inner join ft_ptest_rollinglimits ";
    lQuery += "on ft_ptest_info.splitlot_id=ft_ptest_rollinglimits.splitlot_id and ";
    lQuery += "ft_ptest_info.ptest_info_id=ft_ptest_rollinglimits.ptest_info_id ";
    lQuery += "inner join ft_run ";
    lQuery += "on ft_run.splitlot_id= ft_ptest_info.splitlot_id and ";
    lQuery += "ft_ptest_rollinglimits.run_id=ft_run.run_id ";
    lQuery += "order by ft_run.run_id, ft_run.site_no, ft_run.part_id;";

    return lQuery;
}

}
}
