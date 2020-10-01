#ifndef PAT_GDBN_WEIGHTING_RING_H
#define PAT_GDBN_WEIGHTING_RING_H

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnWeightingRing
//
// Description	:	Stores weighting for adjacent and diagonal die for a ring.
//                  Used in Gdbn algorithm
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnWeightingRing
{
public:

    // Constructors - Destructor
    PatGdbnWeightingRing();
    PatGdbnWeightingRing(unsigned int adjacentWeight, unsigned int diagonalWeight);
    PatGdbnWeightingRing(const PatGdbnWeightingRing& other);
    ~PatGdbnWeightingRing();

    // Operators
    PatGdbnWeightingRing& operator=(const PatGdbnWeightingRing& other);

    // Methods
    unsigned int                adjacentWeight() const;
    unsigned int                diagonalWeight() const;

    void                        setAdjacentWeight(unsigned int adjacentWeight);
    void                        setDiagonalWeight(unsigned int diagonalWeight);

private:

    unsigned int        m_adjacentWeight;
    unsigned int        m_diagonalWeight;
};


#endif // PAT_GDBN_WEIGHTING_RING_H
