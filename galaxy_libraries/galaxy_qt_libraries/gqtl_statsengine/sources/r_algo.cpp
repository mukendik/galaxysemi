#include "r_algo.h"
#include "r_algo_private.h"
#include "r_data.h"

namespace GS
{
namespace SE
{


RAlgo::RAlgo()
    :StatsAlgo(* new RAlgoPrivate)
{
}

RAlgo::RAlgo(RAlgoPrivate &lPrivateData)
    :StatsAlgo(lPrivateData)
{

}

RAlgo::~RAlgo()
{

}

QStringList RAlgo::GetScripts()
{
    Q_D(RAlgo);
    return d->mScripts;
}

} // namespace SE
} // namespace GS

