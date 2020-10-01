#ifndef SUBSET_LIMITS_H
#define SUBSET_LIMITS_H


///////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////
#include "gex_site_limits.h"

#include <QMap>
#include <QVariant>
#include <QVector>
#include <QPair>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	SubsetLimits
//
// Description	:	Class holding subset limits by run id for a test
//
///////////////////////////////////////////////////////////////////////////////////

namespace GS
{
namespace Gex
{

typedef QVector< CGexSiteLimits > RunIdLimits;                      ///<  Vector of limits of one run id
typedef QMap < int, QVector< CGexSiteLimits > > TestSubsetLimits;   ///<  Type containing all subset Limits of one test

/*! \class SubsetLimits
 * \brief Class to manage subset limits
 *
 * This class give method to manipulate the test sebset limits.
 *
 *
 */
class SubsetLimits : public QObject
{
    Q_OBJECT

public :
    /// \brief Destructor
    ~SubsetLimits();

    /// \brief This function searches the corresponding limits to a given runId
    ///        It return true if the map contains the corresponding runId. Otherwise, return false.
    /// \param runId used to get the value of the run id
    /// \param limits used to contains subset Limits which correspond to the run id
    /// \return true if the run id has limits, otherwise return false
    bool GetLimits(int runId, RunIdLimits &limits);


    /// \brief Add high and low limits to the limit map for the runId paremeter.
    ///        If this element not exist, create it.
    /// \param runId used to get the value of the run id
    /// \param ll used to contains the low limit to insert
    /// \param hl used to contains the high limit to insert
    void AddLimits(int runId, double ll, double hl);

    /// \brief Add high and low limits to the limit map for the runId paremeter.
    ///        If this element not exist, create it.
    /// \param runId used to get the value of the run id
    /// \param limit used to contains the limit to insert
    void AddSubsetLimitsByRunId(int runId, CGexSiteLimits limit);

    /// \brief Add high and low limits to the limit map for the list of runId (firstRange <= runId <= lastRange).
    ///        If one element not exist, create it.
    /// \param firstRange used to get the value of the first run id
    /// \param lastRange used to get the value of the last run id
    /// \param limit used to contains the limit to insert
    void AddSubsetLimitsByRunRange(int firstRange, int lastRange, CGexSiteLimits limit);

    /// \brief Remove all limits from the litmit map
    void RemoveAllLimits();

    /// \brief Find if the current test has subset limit or no
    /// \return true if the current test has subset limit, otherwise, false
    inline bool HasSubsetLimit() const {return (mLimits.size() != 0);}

    /// \brief Return the list of subset limit
    /// \return the list of subset limit
    inline TestSubsetLimits& GetTestLimits() {return mLimits;}

private:

    TestSubsetLimits mLimits;       ///<  List of subset limits

};

}
}
#endif // SUBSET_LIMITS_H
