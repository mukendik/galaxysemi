#include "multi_select_input_dialog.h"
#include "ui_multi_select_input_dialog.h"

MultiSelectInputDialog::MultiSelectInputDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::MultiSelectInputDialog)
{
    mUi->setupUi(this);
    QHeaderView *lVerticalHeader = mUi->tableWidgetItems->verticalHeader();
    lVerticalHeader->setDefaultSectionSize(20);
}

MultiSelectInputDialog::~MultiSelectInputDialog()
{
    delete mUi;
}

QList<QStringList> MultiSelectInputDialog::GetSelectedItems(QWidget *parent,
                                                     const QString &title,
                                                     const QString &label,
                                                     const QList<QStringList> &inputItems,
                                                     const QStringList &header,
                                                     bool *ok)
{
    MultiSelectInputDialog lDialog(parent);
    lDialog.setWindowTitle(title);
    lDialog.SetLabel(label);
    lDialog.SetHeader(header);
    lDialog.SetInputItems(inputItems);

    int lReturnValue = lDialog.exec();
    if (ok)
        *ok = !!lReturnValue;
    if (lReturnValue)
    {
        return lDialog.GetSelectedItems();
    }
    else
    {
        return QList<QStringList>();
    }
}

void MultiSelectInputDialog::SetLabel(const QString &label)
{
    mUi->labelAction->setText(label);
}

void MultiSelectInputDialog::SetHeader(const QStringList &header)
{
    mHeader = header;
}

void MultiSelectInputDialog::SetInputItems(const QList<QStringList> &items)
{
    if (!(items.size() > 0 ))
        return;

    mUi->tableWidgetItems->clear();
    mUi->tableWidgetItems->setRowCount(items.size());
    mUi->tableWidgetItems->setColumnCount(items.at(0).size());
    mUi->tableWidgetItems->horizontalHeader()->setVisible(true);
    mUi->tableWidgetItems->setHorizontalHeaderLabels(mHeader);

    for (int lRowId = 0; lRowId < items.size(); ++lRowId)
    {
        for (int lColId = 0; lColId < items.at(lRowId).size(); ++lColId)
        {
            mUi->tableWidgetItems->setItem(lRowId, lColId,
                                           new QTableWidgetItem(items.at(lRowId).at(lColId)));
        }
    }
    mUi->tableWidgetItems->resizeColumnsToContents();
}

QList<QStringList> MultiSelectInputDialog::GetSelectedItems()
{
    QModelIndexList lSelectedItems = mUi->tableWidgetItems->selectionModel()->selectedRows();
    QList<QStringList> lStringItems;
    for (int lIter = 0; lIter < lSelectedItems.size(); ++lIter)
    {
        QStringList lRow;
        for (int lColId = 0; lColId < mUi->tableWidgetItems->columnCount(); ++lColId)
        {
            lRow.append(mUi->tableWidgetItems->item(lSelectedItems.at(lIter).row(),
                                                lColId)->
                                                        data(Qt::DisplayRole).toString());
        }
        lStringItems.append(lRow);
    }

    return lStringItems;
}
