#ifndef PAT_GDBN_WEIGHTING_ALGO_H
#define PAT_GDBN_WEIGHTING_ALGO_H

#include "pat_gdbn_abstract_algorithm.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnWeightingAlgo
//
// Description	:	Class implementing GDBN algorithm with weighting rule
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnWeightingAlgo : public PatGdbnAbstractAlgorithm
{
public:

    PatGdbnWeightingAlgo();
    virtual ~PatGdbnWeightingAlgo();

    // PROPERTIES
    void                setEdgeDieWeighting(int nEdgeDieWeighting)				{ m_nEdgeDieWeighting		= nEdgeDieWeighting; }
	void                setEdgeDieWeightingScale(double dEdgeDieWeightingScale)	{ m_dEdgeDieWeightingScale	= dEdgeDieWeightingScale; }
	void                setAdjacentWeighting(const QList<int>& lstWeight);
	void                setDiagonalWeighting(const QList<int>& lstWeight);
	void                setMinimumWeighting(int nWeight)						{ m_nMinimumWeighting	= nWeight; }

    // METHODS
    bool                excludeDie(const CWaferMap * pWafermap, int dieIndex) const;

protected:

    double              computeDieWeight(const CWaferMap * pWafermap, const QList<int>& lstWeight, CWaferMap::AroundDie aroundDie, int dieIndex, bool bEdgeDie) const;

    int					m_nEdgeDieWeighting;			// Weighting rule for edge die
	double				m_dEdgeDieWeightingScale;		// Weighting scale to apply on edge die
	int					m_nMinimumWeighting;			// Minimum weight to exclude a die
	QList<int>			m_lstAdjacentWeighting;			// Weight for an adjacent failed die (same line or column)
	QList<int>			m_lstDiagonalWeighting;			// Weight for a diagonal failed die
};

#endif // PAT_GDBN_WEIGHTING_ALGO_H
