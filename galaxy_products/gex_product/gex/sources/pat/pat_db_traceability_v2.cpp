#include "pat_db_traceability_v2.h"
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

PATDbTraceabilityV2::PATDbTraceabilityV2(CGexReport * lReportContext,
                                         const QString &lDbFilename, const QString &lConnectionName)
    : PATDbTraceabilityAbstract(lReportContext, lDbFilename, lConnectionName)
{
}

PATDbTraceabilityV2::~PATDbTraceabilityV2()
{
}

QString PATDbTraceabilityV2::BuildQueryOutliers() const
{
    return PATDbTraceabilityAbstract::BuildQueryOutliers();
}

QString PATDbTraceabilityV2::BuildQueryDPATTestLimits() const
{
    return PATDbTraceabilityAbstract::BuildQueryDPATTestLimits();
}

QString PATDbTraceabilityV2::BuildQuerySPATTestLimits() const
{
    return PATDbTraceabilityAbstract::BuildQuerySPATTestLimits();
}

QString PATDbTraceabilityV2::BuildQueryRollingLimits() const
{
    return PATDbTraceabilityAbstract::BuildQueryRollingLimits();
}


}
}
