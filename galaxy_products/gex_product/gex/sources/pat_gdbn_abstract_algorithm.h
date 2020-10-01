#ifndef PAT_GDBN_ABSTRACT_ALGORITHM_H
#define PAT_GDBN_ABSTRACT_ALGORITHM_H

#include "wafermap.h"

class PatGdbnAbstractBadDies;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnAbstractAlgorithm
//
// Description	:	Abstrast class to implement a gdbn algorithm
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnAbstractAlgorithm
{
public:

    PatGdbnAbstractAlgorithm();
    virtual ~PatGdbnAbstractAlgorithm();

    // PROPERTIES
    void               SetEdgeDieType(int nEdgeDieType)						{ m_nEdgeDieType    = nEdgeDieType; }
    void               SetRingRadius(int nRadius)							{ m_nRingRadius     = nRadius; }
    void               SetRingArea(CWaferMap::Ring eArea)					{ m_eRingArea       = eArea; }
    void               SetRuleName(const QString& strName)                  { m_strRuleName     = strName; }
    void               SetSoftBin(const int softBin)                        { mSoftBin          = softBin;}
    void               SetHardBin(const int hardBin)                        { mHardBin          = hardBin;}

    void               SetBadDies(PatGdbnAbstractBadDies * pBadDies);

    const QString&     GetRuleName() const                                  { return m_strRuleName; }
    int                GetSoftBin() const                                   { return mSoftBin;}
    int                GetHardBin() const                                   { return mHardBin;}

    // METHODS
    virtual bool                    excludeDie(const CWaferMap * pWafermap, int dieIndex) const = 0;

protected:

    bool                            isValidDie(const CWaferMap * pWafermap, int dieIndex) const;
    bool                            isAroundBadDie(int bin, int dieIndex) const;

    int                             m_nEdgeDieType;					// Edge die type to analyze
    int                             m_nRingRadius;					// Mask Radius for the ring
    QString                         m_strRuleName;                  // Rule name
    CWaferMap::Ring                 m_eRingArea;					// Ring area
    PatGdbnAbstractBadDies *        m_pBadDies;
    int                             mSoftBin;
    int                             mHardBin;
};

#endif // PAT_GDBN_ABSTRACT_ALGORITHM_H
