/******************************************************************************!
 * \file drillObjectCell.h
 * \brief Interactive drill: 3D Data Mining
 ******************************************************************************/
#ifndef DRILL_OBJECT_CELL_H
#define DRILL_OBJECT_CELL_H

#include <qgl.h>
#if __MACH__
#include "OpenGL/glu.h"
#else
#include <GL/glu.h>
#endif

#include "ui_drill_3d_dialog.h"
#include "drill_3d_viewer.h"

#include <QTextBrowser>
#include <QtWidgets/QTreeWidget>

#include "font_3d.h"

#ifndef DRILL_NO_DATA_MINING
# include "vec.h"
#endif

namespace Gex {

/******************************************************************************!
 * \class DrillObjectCell
 * \brief Contains information about OpenGL object
 ******************************************************************************/
class DrillObjectCell
{
public:
  /*!
   * \fn DrillObjectCell
   */
  DrillObjectCell();
  /*!
   * \fn DrillObjectCell
   */
  DrillObjectCell(int nIndex, long lRun, int nDieX, int nDieY);
  /*!
   * \fn DrillObjectCell
   */
  DrillObjectCell(const DrillObjectCell& objectCell);
  /*!
   * \fn index
   */
  int index() const {
    return m_nIndex;
  }
  /*!
   * \fn run
   */
  long run() const {
    return m_lRun;
  }
  /*!
   * \fn dieX
   */
  int dieX() const {
    return m_nDieX;
  }
  /*!
   * \fn dieY
   */
  int dieY() const {
    return m_nDieY;
  }
  /*!
   * \fn operator=
   */
  DrillObjectCell& operator=(const DrillObjectCell& objectCell);
  /*!
   * \fn operator==
   */
  bool operator==(const DrillObjectCell& objectCell) const;

private:
  /*!
   * \var m_nIndex
   * \brief Index of the object selected in the OpenGL viewer
   */
  int m_nIndex;
  /*!
   * \var m_lRun
   * \brief Index of the run corresponding to the object selected
   */
  int m_lRun;
  /*!
   * \var m_nDieX
   * \brief X Die coordinate
   */
  int m_nDieX;
  /*!
   * \var m_nDieY
   * \brief Y Die coordinate
   */
  int m_nDieY;
};

}  // namespace Gex

#endif
