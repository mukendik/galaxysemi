#ifndef R_ALGO_PRIVATE_H
#define R_ALGO_PRIVATE_H

#include "stats_algo_private.h"

namespace GS
{
namespace SE
{

class RAlgoPrivate: public StatsAlgoPrivate
{
public:
    QStringList mScripts; ///< Holds R script path
};

} // namespace SE
} // namespace GS


#endif // R_ALGO_PRIVATE_H
