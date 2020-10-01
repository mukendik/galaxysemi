/******************************************************************************!
 * \file drillObjectCell.cpp
 * \brief Interactive Drill: 3D Data Mining
 ******************************************************************************/
#include "drillObjectCell.h"

namespace Gex {

/******************************************************************************!
 * \fn DrillObjectCell
 * \brief Constructor 1
 ******************************************************************************/
DrillObjectCell::DrillObjectCell()
{
  m_nIndex = -1;
  m_lRun   = -1;
  m_nDieX  = 0;
  m_nDieY  = 0;
}

/******************************************************************************!
 * \fn DrillObjectCell
 * \brief Constructor 2
 ******************************************************************************/
DrillObjectCell::DrillObjectCell(int nIndex, long lRun, int nDieX, int nDieY)
{
  m_nIndex = nIndex;
  m_lRun   = lRun;
  m_nDieX  = nDieX;
  m_nDieY  = nDieY;
}

/******************************************************************************!
 * \fn DrillObjectCell
 * \brief Copy constructor
 ******************************************************************************/
DrillObjectCell::DrillObjectCell(const DrillObjectCell& objectCell)
{
  *this = objectCell;
}

/******************************************************************************!
 * \fn operator=
 ******************************************************************************/
DrillObjectCell&
DrillObjectCell::operator=(const DrillObjectCell& objectCell)
{
  if (this != &objectCell) {
    m_nIndex = objectCell.m_nIndex;
    m_lRun   = objectCell.m_lRun;
    m_nDieX  = objectCell.m_nDieX;
    m_nDieY  = objectCell.m_nDieY;
  }

  return *this;
}

/******************************************************************************!
 * \fn operator==
 ******************************************************************************/
bool DrillObjectCell::operator==(const DrillObjectCell& objectCell) const
{
  return (m_nIndex == objectCell.m_nIndex &&
          m_lRun   == objectCell.m_lRun   &&
          m_nDieY  == objectCell.m_nDieY  &&
          m_nDieX  == objectCell.m_nDieX);
}

}  // namespace Gex
