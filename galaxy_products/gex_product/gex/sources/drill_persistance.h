#if !defined(DRILL_PERSISTANCE_H__INCLUDED_)
#define DRILL_PERSISTANCE_H__INCLUDED_

#include "ctest.h"

//////////////////////////////////////////////////////
// Drill persistance container (ie per test markers...)
//////////////////////////////////////////////////////
/*! \class  DrillPersistance
    \brief  The DrillPersistance class holds information to ensure persistence for a given object when navigating
    in interactive mode (ie for a test).
*/
class DrillPersistenceItem
{
public:

    DrillPersistenceItem();
    virtual ~DrillPersistenceItem();

    /*! \brief Initialize the persistence item from a CTest.
        Returns true if the marker is a PAT N,M,F marker, false else.
    */
    void    InitItem(const CTest & testItem);

protected:
    QList<bool>     mMlMarkersEnabled;
};

class DrillPersistence
{
public:
    DrillPersistence();
    virtual ~DrillPersistence();

    bool    AddPersistanceItem(const CTest & testItem, const bool resetIfExists = false);

protected:
    QMap<CTest*, DrillPersistenceItem*>     mTestPersistenceMap;
};

#endif	// #ifdef DRILL_PERSISTANCE_H__INCLUDED_
