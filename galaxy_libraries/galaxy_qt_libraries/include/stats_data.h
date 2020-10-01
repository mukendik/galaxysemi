#ifndef STATS_DATA_H
#define STATS_DATA_H

/*! \class StatsData
 * \brief
 *
 */

#include <QStringList>

namespace GS
{
namespace SE
{

class StatsDataPrivate;

class StatsData
{
public:
    enum DataUse
    {
        MVGROUP_IN_1 = 0,
        MVOUTLIER_IN_1 = 1,
        MVOUTLIER_IN_2 = 2,
        MVGROUP_OUT_1 = 3,
        MVOUTLIER_OUT_1 = 4,
        SHAPEIDENTIFIER_IN = 5,
        SHAPEIDENTIFIER_OUT = 6
    };
    /// \brief Destructor
    virtual ~StatsData();
    /// \brief return last error
    QString GetLastError() const;
    /// \brief return errors list
    QStringList GetErrors() const;
    /// \brief clean error stack
    void ClearErrors();

private:
    Q_DECLARE_PRIVATE_D(mPrivate, StatsData)

protected:
    /// \brief Constructor
    StatsData(StatsDataPrivate & lPrivateData);

    StatsDataPrivate* mPrivate;   ///< ptr to private members
};

} // namespace SE
} // namespace GS

#endif // STATS_DATA_H
