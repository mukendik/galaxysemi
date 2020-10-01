#include "pat_gdbn_weighting_algo.h"
#include "patman_lib.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnWeigthingAlgo - Class implementing GDBN algorithm with weighting rule
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnWeightingAlgo::PatGdbnWeightingAlgo() : PatGdbnAbstractAlgorithm()
{
    m_nEdgeDieWeighting			= GEX_TPAT_GPAT_EDGE_SCALE;
    m_dEdgeDieWeightingScale	= 1.0;
    m_nMinimumWeighting			= 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnWeightingAlgo::~PatGdbnWeightingAlgo()
{

}

///////////////////////////////////////////////////////////
// PROPERTIES
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void PatGdbnWeightingAlgo::setAdjacentWeighting(const QList<int>& lstWeight)
//
// Description	:	Sets the weighting for adjacents die
//
///////////////////////////////////////////////////////////////////////////////////
void PatGdbnWeightingAlgo::setAdjacentWeighting(const QList<int>& lstWeight)
{
    m_lstAdjacentWeighting = lstWeight;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void PatGdbnWeightingAlgo::setDiagonalWeighting(const QList<int>& lstWeight)
//
// Description	:	Sets the weighting for diagonals die
//
///////////////////////////////////////////////////////////////////////////////////
void PatGdbnWeightingAlgo::setDiagonalWeighting(const QList<int>& lstWeight)
{
    m_lstDiagonalWeighting = lstWeight;
}

///////////////////////////////////////////////////////////
// METHODS
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool PatGdbnWeightingAlgo::excludeDie(const CWaferMap * pWafermap, int dieIndex) const
//
// Description	:	Determine if a die should be failed as GDBN
//
///////////////////////////////////////////////////////////////////////////////////
bool PatGdbnWeightingAlgo::excludeDie(const CWaferMap * pWafermap, int dieIndex) const
{
    bool    bExcludeDie         = false;

    if (isValidDie(pWafermap, dieIndex))
    {
        bool    bEdgeDieDetected	= false;
        double	dTotalWeighting     = 0;

        // Die is on the edge of the wafer ?
        bEdgeDieDetected = pWafermap->isDieOnEdge(dieIndex, m_nEdgeDieType);

        // Compute weight for adjacent die
        dTotalWeighting += computeDieWeight(pWafermap, m_lstAdjacentWeighting, CWaferMap::AdjacentDie, dieIndex, bEdgeDieDetected);

        // Compute weight for diagonal die
        dTotalWeighting += computeDieWeight(pWafermap, m_lstDiagonalWeighting, CWaferMap::DiagonalDie, dieIndex, bEdgeDieDetected);

        // We found a die on the edge of the wafer, apply the scaling to the total weight
        if (bEdgeDieDetected && m_nEdgeDieWeighting == GEX_TPAT_GPAT_EDGE_SCALE)
            dTotalWeighting *= m_dEdgeDieWeightingScale;

        // Check if total weight of surrounding dies is high enough to exclude center die!
        if (dTotalWeighting >= m_nMinimumWeighting)
            bExcludeDie = true;
    }

    return bExcludeDie;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double PatGdbnWeightingAlgo::computeDieWeight(const CWaferMap * pWafermap, const QList<int>& lstWeight,
//                                                                  CWaferMap::AroundDie aroundDie, int dieIndex, bool bEdgeDie) const
//
// Description	:	Compute weight for a targeted die using adjacent or diagonal die
//
///////////////////////////////////////////////////////////////////////////////////
double PatGdbnWeightingAlgo::computeDieWeight(const CWaferMap * pWafermap, const QList<int>& lstWeight,
                                                CWaferMap::AroundDie aroundDie, int dieIndex, bool bEdgeDie) const
{
    int                         nBinDie     = GEX_WAFMAP_EMPTY_CELL;
    double						dWeight     = 0;
    int                         nWaferSize  = pWafermap->SizeX * pWafermap->SizeY;
    std::vector<int>			vecIndexDie;
    std::vector<int>::iterator	itBegin;
    std::vector<int>::iterator	itEnd;

    for (int nRing = 0; nRing < lstWeight.count(); ++nRing)
    {
        vecIndexDie	= pWafermap->aroundDieCoordinate(dieIndex, aroundDie, nRing, 1, bEdgeDie);
        itBegin		= vecIndexDie.begin();
        itEnd		= vecIndexDie.end();

        while (itBegin != itEnd)
        {
            if (*itBegin >= 0 && *itBegin < nWaferSize)
                nBinDie = ((CWaferMap *)pWafermap)->getWafMap()[*itBegin].getBin();
            else
                nBinDie = GEX_WAFMAP_EMPTY_CELL;

            if (nBinDie == GEX_WAFMAP_EMPTY_CELL && bEdgeDie)	// Means target die in on the edge of the wafer
            {
                // Consider die outside the wafer as a bad die
                if (m_nEdgeDieWeighting == GEX_TPAT_GPAT_EDGE_BAD && pWafermap->isDieOutOfWafer(*itBegin))
                    dWeight += lstWeight.at(nRing);
            }
            else if (isAroundBadDie(nBinDie, (*itBegin)))
                dWeight += lstWeight.at(nRing);

            itBegin++;
        }
    }

    return dWeight;
}
