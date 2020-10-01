#ifndef STATS_ALGO_PRIVATE_H
#define STATS_ALGO_PRIVATE_H

#include "stats_data.h"
#include "stats_algo.h"

namespace GS
{
namespace SE
{

class StatsAlgoPrivate
{
public:
    StatsAlgoPrivate();
    virtual ~StatsAlgoPrivate();

    QStringList                         mErrors;            ///< holds last
    StatsData*                          mResults;           ///< holds data used by algo
    QList<GS::SE::StatsAlgo::Parameter> mRequiredParameter; ///< holds required parameters
    QMap<StatsData::DataUse, QString>   mRequiredData;      ///< holds required data
};

} // namespace SE
} // namespace GS

#endif // STATS_ALGO_PRIVATE_H
