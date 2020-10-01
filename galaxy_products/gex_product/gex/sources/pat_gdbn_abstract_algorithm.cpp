#include "pat_gdbn_abstract_algorithm.h"
#include "pat_gdbn_engine.h"
#include "pat_gdbn_abstract_baddies.h"

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnAbstractAlgorithm - Abstrast class to implement a gdbn algorithm
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnAbstractAlgorithm::PatGdbnAbstractAlgorithm()
{
    m_nEdgeDieType				= CWaferMap::EdgeDieBoth;
    m_nRingRadius				= -1;
    m_eRingArea					= CWaferMap::RingFromEdge;
    m_pBadDies                  = NULL;
    mSoftBin                    = -1;
    mHardBin                    = -1;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnAbstractAlgorithm::~PatGdbnAbstractAlgorithm()
{
    if (m_pBadDies)
    {
        delete m_pBadDies;
        m_pBadDies = NULL;
    }
}

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void PatGdbnAbstractAlgorithm::setBadDies(PatGdbnAbstractBadDies * pBadDies)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void PatGdbnAbstractAlgorithm::SetBadDies(PatGdbnAbstractBadDies * pBadDies)
{
    if (m_pBadDies)
    {
        delete m_pBadDies;
        m_pBadDies = NULL;
    }

    m_pBadDies = pBadDies;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool PatGdbnAbstractAlgorithm::isValidDie(const CWaferMap * pWafermap, int dieIndex)
//
// Description	:	Indicate if a die should be taken into account in the weigth computation
//
///////////////////////////////////////////////////////////////////////////////////
bool PatGdbnAbstractAlgorithm::isValidDie(const CWaferMap * pWafermap, int dieIndex) const
{
    if (pWafermap)
        return pWafermap->isDieInsideRing(m_eRingArea, m_nRingRadius, dieIndex);
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool PatGdbnAbstractAlgorithm::isAroundBadDie(int bin, int /*dieIndex*/)
//
// Description	:	Indicate if a die should be taken into account in the gdbn computation
//
///////////////////////////////////////////////////////////////////////////////////
bool PatGdbnAbstractAlgorithm::isAroundBadDie(int bin, int dieIndex) const
{
    if (m_pBadDies)
        return m_pBadDies->isDieIncluded(bin, dieIndex);
    else
        return false;
}
