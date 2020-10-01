#include "pat_db_traceability_v3.h"
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

PATDbTraceabilityV3::PATDbTraceabilityV3(CGexReport * lReportContext,
                                         const QString &lDbFilename, const QString &lConnectionName)
    : PATDbTraceabilityAbstract(lReportContext, lDbFilename, lConnectionName)
{
}

PATDbTraceabilityV3::~PATDbTraceabilityV3()
{
}

QString PATDbTraceabilityV3::BuildQueryOutliers() const
{
    return PATDbTraceabilityAbstract::BuildQueryOutliers();
}

QString PATDbTraceabilityV3::BuildQueryDPATTestLimits() const
{
    return PATDbTraceabilityAbstract::BuildQueryDPATTestLimits();
}

QString PATDbTraceabilityV3::BuildQuerySPATTestLimits() const
{
    return PATDbTraceabilityAbstract::BuildQuerySPATTestLimits();
}

QString PATDbTraceabilityV3::BuildQueryRollingLimits() const
{
    return PATDbTraceabilityAbstract::BuildQueryRollingLimits();
}

}
}
