#ifndef PAT_GDBN_GENERIC_BADDIES_H
#define PAT_GDBN_GENERIC_BADDIES_H

#include "pat_gdbn_abstract_baddies.h"

namespace GS
{
    namespace QtLib
    {
        class Range;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnGenericBadDies
//
// Description	:	Class to determine if a die should be taken into
//                  account in a gdbn algorithm using the generic method (bad bin list)
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnGenericBadDies : public PatGdbnAbstractBadDies
{
public:
    PatGdbnGenericBadDies(GS::QtLib::Range * pBadBinList);
    virtual ~PatGdbnGenericBadDies();

    bool    isDieIncluded(int bin, int dieIndex) const;

private:

    GS::QtLib::Range *     m_pBadBinList;
};

#endif // PAT_GDBN_GENERIC_BADDIES_H
