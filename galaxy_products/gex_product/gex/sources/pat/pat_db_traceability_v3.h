#ifndef PAT_DB_TRACEABILITY_V3_H
#define PAT_DB_TRACEABILITY_V3_H

#include "pat_db_traceability_abstract.h"

namespace GS
{
namespace Gex
{

class PATDbTraceabilityV3 : public PATDbTraceabilityAbstract
{
public:

    PATDbTraceabilityV3(CGexReport *lReportContext, const QString& lDbFilename,
                        const QString &lConnectionName);
    virtual ~PATDbTraceabilityV3();

protected:

    virtual QString BuildQueryOutliers() const;
    virtual QString BuildQueryDPATTestLimits() const;
    virtual QString BuildQuerySPATTestLimits() const;
    virtual QString BuildQueryRollingLimits() const;
};

}   // namespace Gex
}   // namespase GS

#endif // PAT_DB_TRACEABILITY_V3_H
