/******************************************************************************!
 * \file binScaleColorWidget.h
 ******************************************************************************/
#ifndef _BIN_SCALE_COLOR_WIDGET_H_
#define _BIN_SCALE_COLOR_WIDGET_H_

#include "ui_bin_scale_color_widget.h"

class CTest;
class CGexGroupOfFiles;
class CGexFileInGroup;

namespace Gex {

/******************************************************************************!
 * \class BinScaleColorWidget
 * \brief Class to display binning scale colors
 ******************************************************************************/
class BinScaleColorWidget : public QWidget, public Ui_BinScaleColorWidget
{
public:
  /*!
   * \enum binningType
   */
  enum binningType {
    typeSoftBin,
    typeHardBin
  };
  /*!
   * \fn BinScaleColorWidget
   */
  BinScaleColorWidget(QWidget* pParent = NULL);
  /*!
   * \fn ~BinScaleColorWidget
   */
  ~BinScaleColorWidget();
  /*!
   * \fn binType
   */
  binningType binType() const {
    return m_eBinType;
  }
  /*!
   * \fn setBinType
   */
  void setBinType(binningType eType);
  /*!
   * \fn setGroup
   */
  void setGroup(CGexGroupOfFiles* pGroup);
  /*!
   * \fn setFile
   */
  void setFile(CGexFileInGroup* pFile);
  /*!
   * \fn updateScale
   * \brief update the content of the scale
   */
  void updateScale();
  /*!
   * \fn BinFilterState
   */
  Qt::CheckState BinFilterState(int bin);
  /*!
   * \fn SetAllBinFilterState
   */
  void SetAllBinFilterState();

protected:
  /*!
   * \fn clearBinningList
   * \brief allows to clear the tree widget
   */
  void clearBinningList();
  /*!
   * \fn fillBinningList
   * \brief call the right method to fill the binning list
   */
  void fillBinningList();
  /*!
   * \fn fillCustomList
   * \brief fill the list with the custom colors
   */
  void fillCustomList();
  /*!
   * \fn fillDefaultList
   * \brief fill the list with the default colors
   */
  void fillDefaultList();
  /*!
   * \fn addBinning
   * \brief add an item in the list
   */
  void addBinning(const QColor& clrBinning,
                  QString strBinCode,
                  const QString& strBinName,
                  const QString& strBinCount,
                  const QString& strPercent);

private:
  /*!
   * \fn OnChangeBinFilter
   * \brief callback for checkboxes "Filter"
   */
  //void OnChangeBinFilter(QTreeWidgetItem* item, int col);
  /*!
   * \var m_eBinType
   * \brief binning type (soft or hard)
   */
  binningType m_eBinType;
  /*!
   * \var m_bNeedUpdate
   * \brief true if the widget need to be updated
   */
  bool m_bNeedUpdate;
  /*!
   * \var m_pGroup
   * \brief Group associated with the binning list
   */
  CGexGroupOfFiles* m_pGroup;
  /*!
   * \var m_pFile
   * \brief File associated with the binning list
   */
  CGexFileInGroup* m_pFile;
};

}  // namepace Gex

#endif  // _BIN_SCALE_COLOR_WIDGET_H_
