#ifndef STATS_DATA_PRIVATE_H
#define STATS_DATA_PRIVATE_H

#include <QStringList>


namespace GS
{
namespace SE
{

class StatsDataPrivate
{
public:
    StatsDataPrivate();
    virtual ~StatsDataPrivate();

    QStringList mErrors; ///< Holds error
};

} // namespace SE
} // namespace GS

#endif // STATS_DATA_PRIVATE_H
