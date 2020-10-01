#ifndef PAT_GDBN_ABSTRACT_BADDIES_H
#define PAT_GDBN_ABSTRACT_BADDIES_H

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnAbstractBadDies
//
// Description	:	Abstrast class to determine if a die should be taken into
//                  account in a gdbn algorithm
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnAbstractBadDies
{
public:

    PatGdbnAbstractBadDies()     {}
    virtual ~PatGdbnAbstractBadDies()   {}

    virtual bool    isDieIncluded(int bin, int dieIndex) const = 0;
};

#endif // PAT_GDBN_ABSTRACT_BADDIES_H
