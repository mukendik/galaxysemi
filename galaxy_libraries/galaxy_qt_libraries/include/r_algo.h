#ifndef R_ALGO_H
#define R_ALGO_H

/*! \class RAlgo
 * \brief
 *
 */

#include <QString>

#include "stats_algo.h"

namespace GS
{
namespace SE
{

class RAlgoPrivate;

class RAlgo: public StatsAlgo
{
public:
    /// \brief Constructor
    RAlgo();
    /// \brief Destructor
    ~RAlgo();
    /// \brief return script path
    QStringList GetScripts();

protected:
    Q_DECLARE_PRIVATE_D(mPrivate, RAlgo)
    /// \brief Constructor
    RAlgo(RAlgoPrivate & lPrivateData);
};

} // namespace SE
} // namespace GS

#endif // R_ALGO_H
