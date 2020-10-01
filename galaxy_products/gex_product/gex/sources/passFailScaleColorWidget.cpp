/******************************************************************************!
 * \file passFailScaleColorWidget.cpp
 ******************************************************************************/
#include "passFailScaleColorWidget.h"
#include "gex_report.h"
#include <gqtl_log.h>

namespace Gex {

/******************************************************************************!
 * \fn GexPassFailScaleColorWidget
 ******************************************************************************/
PassFailScaleColorWidget::PassFailScaleColorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  // setup the ui
  setupUi(this);

  // Customize tree widget
  QStringList lstHeaderLabel;
  lstHeaderLabel << "" << "Status" << "Count";
  treeWidget->setHeaderLabels(lstHeaderLabel);
  treeWidget->setColumnWidth(0, 40);
  treeWidget->setColumnWidth(1, 40);
  treeWidget->setColumnWidth(2, 40);

  label->setText("Pass/Fail colors");

  m_pFile       = NULL;
  m_bNeedUpdate = true;
}

/******************************************************************************!
 * \fn ~PassFailScaleColorWidget
 ******************************************************************************/
PassFailScaleColorWidget::~PassFailScaleColorWidget()
{
}

/******************************************************************************!
 * \fn fillList
 ******************************************************************************/
void PassFailScaleColorWidget::fillList()
{
  int nFailCount = 0;
  int nPassCount = 0;

  // Merge this wafermap to the stacked array.
  for (int nIndex = 0;
       m_pFile && nIndex < m_pFile->getWaferMapData().SizeX *
       m_pFile->getWaferMapData().SizeY;
       nIndex++) {
    // Get die value (Bin# or Parametric % value)
    if (m_pFile->getWaferMapData().getWafMap()[nIndex].getBin() ==
        GEX_WAFMAP_FAIL_CELL) {
      nFailCount++;
    } else if (m_pFile->getWaferMapData().getWafMap()[nIndex].getBin() ==
               GEX_WAFMAP_PASS_CELL) {
      nPassCount++;
    }
  }

  QTreeWidgetItem* pItem = NULL;
  QPixmap pixmap(17, 17);  // Create pixmap to show Bin color

  // add fail item
  pItem = new QTreeWidgetItem(treeWidget);
  pixmap.fill(QColor(252, 90, 90));

  pItem->setIcon(0, pixmap);
  pItem->setText(1, "Fail");
  pItem->setText(2, QString::number(nFailCount));

  // add pass item
  pItem = new QTreeWidgetItem(treeWidget);
  pixmap.fill(QColor(134, 242, 134));

  pItem->setIcon(0, pixmap);
  pItem->setText(1, "Pass");
  pItem->setText(2, QString::number(nPassCount));

  treeWidget->resizeColumnToContents(0);
  treeWidget->resizeColumnToContents(1);
  treeWidget->resizeColumnToContents(2);
}

/******************************************************************************!
 * \fn clearList
 ******************************************************************************/
void PassFailScaleColorWidget::clearList()
{
  treeWidget->clear();
}

/******************************************************************************!
 * \fn updateScale
 ******************************************************************************/
void PassFailScaleColorWidget::updateScale()
{
  if (m_bNeedUpdate) {
    // Clear the previous list
    clearList();

    // fill the list with new binning list
    fillList();

    // Set update to false
    m_bNeedUpdate = false;
  }
}

/******************************************************************************!
 * \fn setFile
 ******************************************************************************/
void PassFailScaleColorWidget::setFile(CGexFileInGroup* pFile)
{
  m_pFile       = pFile;
  m_bNeedUpdate = true;
}

}  // namespace Gex
