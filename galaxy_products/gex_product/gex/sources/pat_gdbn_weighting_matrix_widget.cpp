#include "pat_gdbn_weighting_matrix_widget.h"
#include "ui_pat_gdbn_weighting_matrix_widget.h"
#include <QStandardItemModel>
#include <QSpinBox>

PatGdbnWeightingRingDelegate::PatGdbnWeightingRingDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

QWidget * PatGdbnWeightingRingDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
 {
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(0);
     editor->setMaximum(100);

     return editor;
 }

void PatGdbnWeightingRingDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
 {
     int value = index.model()->data(index, Qt::EditRole).toInt();

     QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->setValue(value);
 }

void PatGdbnWeightingRingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
     QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->interpretText();
     int value = spinBox->value();

     model->setData(index, value, Qt::EditRole);
 }

void PatGdbnWeightingRingDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }

PatGdbnWeightingMatrixWidget::PatGdbnWeightingMatrixWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatGdbnWeightingMatrixWidget)
{
    ui->setupUi(this);

    QStandardItemModel * pModel = new QStandardItemModel(2,0);
    ui->tableViewMatrix->setModel(pModel);
    ui->tableViewMatrix->setItemDelegateForRow(0, new PatGdbnWeightingRingDelegate());
    ui->tableViewMatrix->setItemDelegateForRow(1, new PatGdbnWeightingRingDelegate());

//    ui->tableViewMatrix->horizontalHeader()->setStretchLastSection(true);

    pModel->setHeaderData(0, Qt::Vertical, QVariant("Adjacent"));
    pModel->setHeaderData(1, Qt::Vertical, QVariant("Diagonal"));

    updateMatrixSize(1);

    ui->comboBoxMatrix->setCurrentIndex(0);
    ui->labelMatrixNxN->setVisible(false);
    ui->spinBoxMatrixNxN->setVisible(false);

    connect(ui->comboBoxMatrix,     SIGNAL(activated(int)),     this,   SLOT(onComboMatrixChanged(int)));
    connect(ui->spinBoxMatrixNxN,   SIGNAL(valueChanged(int)),  this,   SLOT(onMatrixNxNChanged(int)));
}

void PatGdbnWeightingMatrixWidget::setRings(const QList<PatGdbnWeightingRing> rings)
{
    QStandardItemModel *    pModel = (QStandardItemModel*)ui->tableViewMatrix->model();

    updateMatrixSize(rings.count());

    for (int i = 0; i < rings.count(); i++)
    {
        QModelIndex index = pModel->index(0, i, QModelIndex());
        pModel->setData(index, QVariant(rings.at(i).adjacentWeight()));

        index = pModel->index(1, i, QModelIndex());
        pModel->setData(index, QVariant(rings.at(i).diagonalWeight()));
    }

    ui->comboBoxMatrix->setCurrentIndex(rings.count()-1);

    ui->tableViewMatrix->resizeRowsToContents();
}

void PatGdbnWeightingMatrixWidget::updateMatrixSize(unsigned int matrixSize)
{
    QStandardItemModel *    pModel = (QStandardItemModel*)ui->tableViewMatrix->model();
    QStandardItem *         pItem = NULL;
    QList<QStandardItem*>   lstItem;
    int                     nRow;

    if ((int) matrixSize < pModel->columnCount())
        pModel->removeColumns(matrixSize, pModel->columnCount()- matrixSize);

    while ((int) matrixSize > pModel->columnCount())
    {
        nRow = pModel->columnCount();
        lstItem.clear();

        for (int nColumn = 0; nColumn < pModel->rowCount(); nColumn++)
        {
            pItem = new QStandardItem("1");
            pItem->setTextAlignment(Qt::AlignCenter);

            lstItem << pItem;
        }

        pModel->appendColumn(lstItem);
        pModel->setHeaderData(nRow, Qt::Horizontal, QVariant(QString("Ring %1").arg(nRow+1)));
    }

    ui->tableViewMatrix->resizeColumnsToContents();
    ui->tableViewMatrix->resizeRowsToContents();
}

void PatGdbnWeightingMatrixWidget::onComboMatrixChanged(int nIndex)
{
    if (nIndex == 5)
    {
        ui->labelMatrixNxN->setVisible(true);
        ui->spinBoxMatrixNxN->setVisible(true);
    }
    else
    {
        ui->labelMatrixNxN->setVisible(false);
        ui->spinBoxMatrixNxN->setVisible(false);
    }

    updateMatrixSize(nIndex+1);
}

void PatGdbnWeightingMatrixWidget::onMatrixNxNChanged(int nMatrixSize)
{
    updateMatrixSize(nMatrixSize / 2);
}

PatGdbnWeightingMatrixWidget::~PatGdbnWeightingMatrixWidget()
{
    delete ui;
}
