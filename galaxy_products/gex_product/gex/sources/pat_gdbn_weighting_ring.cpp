#include "pat_gdbn_weighting_ring.h"

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnWeightingRing - Stores weighting for adjacent and diagonal die
//                              for a ring. Used in Gdbn algorithm
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnWeightingRing::PatGdbnWeightingRing()
    : m_adjacentWeight(0),
      m_diagonalWeight(0)
{
}

PatGdbnWeightingRing::PatGdbnWeightingRing(unsigned int adjacentWeight, unsigned int diagonalWeight)
    : m_adjacentWeight(adjacentWeight),
      m_diagonalWeight(diagonalWeight)
{
}

PatGdbnWeightingRing::PatGdbnWeightingRing(const PatGdbnWeightingRing &other)
{
    *this = other;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnWeightingRing::~PatGdbnWeightingRing()
{
}

///////////////////////////////////////////////////////////
// Operator
///////////////////////////////////////////////////////////
PatGdbnWeightingRing& PatGdbnWeightingRing::operator=(const PatGdbnWeightingRing& other)
{
    if (this != &other)
    {
        m_adjacentWeight    = other.m_adjacentWeight;
        m_diagonalWeight    = other.m_diagonalWeight;
    }

    return *this;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
/*!
  Returns the weigth to apply to a bad adjacent die.
*/
unsigned int PatGdbnWeightingRing::adjacentWeight() const
{
    return m_adjacentWeight;
}

/*!
  Returns the weigth to apply to a bad diagonal die.
*/
unsigned int PatGdbnWeightingRing::diagonalWeight() const
{
    return m_diagonalWeight;
}

/*!
  Sets the weigth to apply to a bad adjacent die.
*/
void PatGdbnWeightingRing::setAdjacentWeight(unsigned int adjacentWeight)
{
    m_adjacentWeight = adjacentWeight;
}

/*!
  Sets the weigth to apply to a bad diagonal die.
*/
void PatGdbnWeightingRing::setDiagonalWeight(unsigned int diagonalWeight)
{
    m_diagonalWeight = diagonalWeight;
}
