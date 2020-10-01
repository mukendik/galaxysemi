#ifndef PAT_DB_TRACEABILITY_V1_H
#define PAT_DB_TRACEABILITY_V1_H

#include "pat_db_traceability_abstract.h"

namespace GS
{
namespace Gex
{

class PATDbTraceabilityV1 : public PATDbTraceabilityAbstract
{
public:

    PATDbTraceabilityV1(CGexReport *lReportContext, const QString& lDbFilename,
                        const QString &lConnectionName);
    virtual ~PATDbTraceabilityV1();

protected:

    virtual QString BuildQueryOutliers() const;
    virtual QString BuildQueryDPATTestLimits() const;
    virtual QString BuildQuerySPATTestLimits() const;
    virtual QString BuildQueryRollingLimits() const;
};

}   // namespace Gex
}   // namespase GS

#endif // PAT_DB_TRACEABILITY_V1_H
