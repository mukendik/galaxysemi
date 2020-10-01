#ifndef PAT_GDBN_SQUEEZE_ALGO_H
#define PAT_GDBN_SQUEEZE_ALGO_H

#include "pat_gdbn_abstract_algorithm.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnSqueezeAlgo
//
// Description	:	Class implementing GDBN algorithm with squeeze rule
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnSqueezeAlgo : public PatGdbnAbstractAlgorithm
{
public:

    PatGdbnSqueezeAlgo();
    virtual ~PatGdbnSqueezeAlgo();

    // PROPERTIES
    void                setClusterSize(int nClusterSize)                                { m_nClusterSize = nClusterSize; }
    void                setMinimumFailCount(int nMinimumFailCount)                      { m_nMinimumFailCount = nMinimumFailCount; }
    void                setFailEdgeDies(bool bFail)                                     { m_bFailEdgeDies = bFail; }

    // METHODS
    bool                excludeDie(const CWaferMap * pWafermap, int dieIndex) const;

private:

    int                 m_nClusterSize;
    int                 m_nMinimumFailCount;
    bool                m_bFailEdgeDies;

};

#endif // PAT_GDBN_SQUEEZE_ALGO_H
