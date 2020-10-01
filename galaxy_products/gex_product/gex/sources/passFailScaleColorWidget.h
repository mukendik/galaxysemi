/******************************************************************************!
 * \file passFailScaleColorWidget.h
 ******************************************************************************/
#ifndef _PASS_FAIL_SCALE_COLOR_WIDGET_H_
#define _PASS_FAIL_SCALE_COLOR_WIDGET_H_

#include "ui_bin_scale_color_widget.h"

class CGexFileInGroup;

namespace Gex {

/******************************************************************************!
 * \class PassFailScaleColorWidget
 * \brief Class to display pass/fail scale colors
 ******************************************************************************/
class PassFailScaleColorWidget :
  public QWidget,
  public Ui_BinScaleColorWidget
{
public:
  /*!
   * \fn PassFailScaleColorWidget
   */
  PassFailScaleColorWidget(QWidget* pParent = NULL);
  /*!
   * \fn ~PassFailScaleColorWidget
   */
  ~PassFailScaleColorWidget();
  /*!
   * \fn setFile
   */
  void setFile(CGexFileInGroup* pFile);
  /*!
   * \fn updateScale
   * \brief update the content of the scale
   */
  void updateScale();

protected:
  /*!
   * \fn clearList
   * \brief allows to clear the tree widget
   */
  void clearList();
  /*!
   * \fn fillList
   * \brief call the right method to fill the binning list
   */
  void fillList();

private:
  /*!
   * \var m_bNeedUpdate
   * \brief true if the widget need to be updated
   */
  bool m_bNeedUpdate;
  /*!
   * \var m_pFile
   * \brief File associated with the binning list
   */
  CGexFileInGroup* m_pFile;
};

}  // namespace Gex

#endif  // _PASS_FAIL_SCALE_COLOR_WIDGET_H_
