#ifndef MV_GROUPS_BUILDER_H
#define MV_GROUPS_BUILDER_H

/*! \class MVGroupsBuilder
 * \brief
 *
 */

#include <QList>

#include "r_algo.h"
#include "r_vector.h"

namespace GS
{
namespace SE
{

class StatsData;

class MVGroupsBuilder:public RAlgo
{
public:
    /// \brief Constructor
    MVGroupsBuilder();
    /// \brief Destructor
    ~MVGroupsBuilder();
    /// \brief return number of groups
    int GetGroupsSize();
    /// \brief return group # groupId
    RVector GetGroup(int groupId);
    /// \brief return all groups
    QList< QList<int> > GetGroups(bool& ok);

private:
    /// \brief Init required data
    void InitRequiredData();
    /// \brief Init required parametes
    void InitRequiredParameters();
    /// \brief compute algo
    bool DoExecute(QMap<Parameter, QVariant> params, StatsData *data);
    /// \brief input data
    bool CheckInputData(StatsData* data);
};

} // namespace SE
} // namespace GS
#endif // MV_GROUPS_BUILDER_H
