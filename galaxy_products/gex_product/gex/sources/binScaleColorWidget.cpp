/******************************************************************************!
 * \file binScaleColorWidget.cpp
 ******************************************************************************/
#include "binScaleColorWidget.h"
#include "cbinning.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>

extern CGexReport* gexReport;  // Handle to report class

namespace Gex {

/******************************************************************************!
 * \fn BinScaleColorWidget
 ******************************************************************************/
BinScaleColorWidget::BinScaleColorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  // setup the ui
  setupUi(this);

  setBinType(typeSoftBin);

  // Customize tree widget
  QStringList lstHeaderLabel;
  lstHeaderLabel << "" << "Bin#" << "Bin Name"
                 << "Filter" << "Bin count" << "Percent";
  treeWidget->setHeaderLabels(lstHeaderLabel);
  treeWidget->setColumnWidth(0, 40);
  treeWidget->setColumnWidth(1, 40);
  treeWidget->setColumnWidth(2, 100);
  treeWidget->setColumnWidth(3, 40);
  treeWidget->setColumnWidth(4, 50);
  treeWidget->setColumnWidth(5, 50);

  m_pGroup      = NULL;
  m_pFile       = NULL;
  m_bNeedUpdate = false;
}

/******************************************************************************!
 * \fn ~BinScaleColorWidget
 ******************************************************************************/
BinScaleColorWidget::~BinScaleColorWidget()
{
}

/******************************************************************************!
 * \fn setBinType
 ******************************************************************************/
void BinScaleColorWidget::setBinType(binningType eType)
{
  m_eBinType    = eType;
  m_bNeedUpdate = true;
}

/******************************************************************************!
 * \fn setGroup
 ******************************************************************************/
void BinScaleColorWidget::setGroup(CGexGroupOfFiles* pGroup)
{
  m_pGroup      = pGroup;
  m_bNeedUpdate = true;
}

/******************************************************************************!
 * \fn setFile
 ******************************************************************************/
void BinScaleColorWidget::setFile(CGexFileInGroup* pFile)
{
  m_pFile       = pFile;
  m_bNeedUpdate = true;
}

/******************************************************************************!
 * \fn updateScale
 ******************************************************************************/
void BinScaleColorWidget::updateScale()
{
  if (m_bNeedUpdate) {
    // Clear the previous list
    clearBinningList();

    // fill the list with new binning list
    fillBinningList();

    // Set update to false
    m_bNeedUpdate = false;
  }
}

/******************************************************************************!
 * \fn fillBinningList
 ******************************************************************************/
void BinScaleColorWidget::fillBinningList()
{
  if (m_pGroup && m_pFile) {
    float fPercent;
    long lTotalBins = 0;
    long lBinCount  = 0;
    QString strBinName;
    QString strBinCount;
    QString strPercent;
    CBinning* pBinCell    = NULL;
    CBinning* pBinCellTmp = NULL;

    // Get the list of binning
    if (m_eBinType == typeSoftBin) {
      pBinCell = m_pGroup->cMergedData.ptMergedSoftBinList;
    } else {
      pBinCell = m_pGroup->cMergedData.ptMergedHardBinList;
    }

    pBinCellTmp = pBinCell;

    // Compute the total binning for this wafer
    while (pBinCellTmp != NULL) {
      lTotalBins += m_pFile->GetWafermapBinOccurance(pBinCellTmp->iBinValue);

      pBinCellTmp = pBinCellTmp->ptNextBin;
    }

    // Fill the widget
    while (pBinCell != NULL) {
      //if (pBinCell->ldTotalCount)
      lBinCount = m_pFile->GetWafermapBinOccurance(pBinCell->iBinValue);

      if (lBinCount) {
        // Fill Bin name
        if (pBinCell->strBinName.isEmpty()) {
          strBinName = "-";
        } else {
          strBinName = pBinCell->strBinName;
        }

        // Fill bin Count
        strBinCount = QString::number(lBinCount);

        // Percentage
        fPercent = (100.0 * lBinCount) / lTotalBins;

        if ((fPercent < 0.0) || (fPercent > 100.0) || (lTotalBins <= 0)) {
          // Happens when corrupted binning data
          // (as seen on LTX Fusion systems !)
          strPercent = GEX_NA;
        } else {
          strPercent.sprintf("%.2f %%", fPercent);
        }

        addBinning(gexReport->cDieColor.GetWafmapDieColor(pBinCell->iBinValue, m_eBinType == typeSoftBin),
                   QString::number(pBinCell->iBinValue),
                   strBinName,
                   strBinCount,
                   strPercent);
      }

      pBinCell = pBinCell->ptNextBin;
    }

    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
  }
}

/******************************************************************************!
 * \fn clearBinningList
 ******************************************************************************/
void BinScaleColorWidget::clearBinningList()
{
  treeWidget->clear();
}

/******************************************************************************!
 * \fn addBinning
 * \brief Read Bin range & color and add it to the Bin list GUI
 ******************************************************************************/
void BinScaleColorWidget::addBinning(const QColor& clrBinning,
                                     QString strBinCode,
                                     const QString& strBinName,
                                     const QString& strBinCount,
                                     const QString& strPercent)
{
  treeWidget->blockSignals(true);
  QTreeWidgetItem* pItem = new QTreeWidgetItem(treeWidget);

  if (pItem) {
    // Create pixmap to show Bin color
    QPixmap pixmap(17, 17);
    pixmap.fill(clrBinning);

    pItem->setIcon(0, pixmap);
    pItem->setText(1, strBinCode);
    pItem->setText(2, strBinName);
    pItem->setText(4, strBinCount);
    pItem->setText(5, strPercent);

    pItem->setTextAlignment(4, Qt::AlignRight);
    pItem->setTextAlignment(5, Qt::AlignRight);

    // Checkbox "Filter"
    pItem->setTextAlignment(3, Qt::AlignCenter);
    pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
    pItem->setCheckState(3, Qt::Checked);
  }
  treeWidget->blockSignals(false);
}

/******************************************************************************!
 * \fn BinFilterState
 ******************************************************************************/
Qt::CheckState BinScaleColorWidget::BinFilterState(int bin)
{
  QString strBinCode;
  strBinCode.setNum(bin);

  QTreeWidgetItemIterator it(treeWidget);
  while (*it) {
    if ((*it)->text(1) == strBinCode) {
      Qt::CheckState st = (*it)->checkState(3);
      //GSLOG(SYSLOG_SEV_DEBUG, "bin " + strBinCode +
      //       ((st == Qt::Checked) ? " checked" : " unchecked"));
      return st;
    }
    ++it;
  }
  //GSLOG(SYSLOG_SEV_DEBUG, "bin " + strBinCode + " not found");
  return Qt::Checked;
}

/******************************************************************************!
 * \fn SetAllBinFilterState
 ******************************************************************************/
void BinScaleColorWidget::SetAllBinFilterState()
{
  QTreeWidgetItemIterator it(treeWidget);
  while (*it) {
    (*it)->setCheckState(3, Qt::Checked);
    ++it;
  }
}

}  // namepace Gex
