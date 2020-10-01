#ifndef STATS_ALGO_H
#define STATS_ALGO_H

/*! \class
 * \brief
 *
 */

#include <QMap>
#include <QString>
#include <QVariant>

namespace GS
{
namespace SE
{

class StatsAlgoPrivate;
class StatsData;

class StatsAlgo
{
public:
    enum Algorithm
    {
        MV_GROUPS_BUILDER,
        MV_OUTLIERS_FINDER,
        SHAPE_IDENTIFIER
    };
    enum Parameter
    {
        SIGMA = 1,
        CORR_THRESHOLD = 2,
        MAX_COMPONENT = 3,
        DISTINCT_VALUES = 4
    };

    /// \brief Destructor
    virtual ~StatsAlgo();
    /// \brief init algo
    void Init();
    /// \brief init required data
    virtual void InitRequiredData() = 0;
    /// \brief init required parameter
    virtual void InitRequiredParameters() = 0;
    /// \brief call do excute
    bool Execute(QMap<Parameter, QVariant> params, StatsData* data);
    /// \brief return last error
    QString GetLastError();
    /// \brief return errors list
    QStringList GetErrors();
    /// \brief clean error stack
    void ClearErrors();

private:
    Q_DECLARE_PRIVATE_D(mPrivate, StatsAlgo)


protected:
    /// \brief Constructor
    StatsAlgo(StatsAlgoPrivate & lPrivateData);
    /// \brief compute algo
    virtual bool DoExecute(QMap<Parameter, QVariant> params, StatsData *data)=0;
    /// \brief check input parameters
    bool CheckInputParameters(QMap<Parameter, QVariant> params);
    /// \brief check input data
    virtual bool CheckInputData(StatsData* data);

    StatsAlgoPrivate* mPrivate; ///< ptr to private members
};

} // namespace SE
} // namespace GS

#endif // STATS_ALGO_H
