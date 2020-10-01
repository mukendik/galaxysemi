#undef QT3_SUPPORT

#include "pick_fields_dialog.h"
#include "ui_pick_fields_dialog.h"

struct PickFieldsData
{
    QStringList     mFieldList;
    QStringList     mSelectedFieldList;

    inline PickFieldsData(const QStringList& fieldList)
        : mFieldList(fieldList) {}
};

PickFieldsDialog::PickFieldsDialog(const QStringList &fieldList, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PickFieldsDialog), mpPrivate(new PickFieldsData(fieldList))
{
    ui->setupUi(this);

    // Fill the treewidget with the field list
    QTreeWidgetItem * pItem = NULL;

    foreach(const QString &field, mpPrivate->mFieldList)
    {
        pItem = new QTreeWidgetItem(ui->treeWidgetFieldList);

        if (mpPrivate->mSelectedFieldList.contains(field))
            pItem->setCheckState(0, Qt::Checked);
        else
            pItem->setCheckState(0, Qt::Unchecked);

        pItem->setText(1, field);
    }

    connect(ui->treeWidgetFieldList,    SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this,                       SLOT(onItemChanged(QTreeWidgetItem*, int)));
    connect(ui->treeWidgetFieldList,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this,                       SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
}

PickFieldsDialog::~PickFieldsDialog()
{
    delete ui;
}

const QStringList &PickFieldsDialog::selectedFieldList() const
{
    return mpPrivate->mSelectedFieldList;
}

void PickFieldsDialog::setSelectedFieldList(const QStringList &fieldList)
{
    QTreeWidgetItem * pItem = NULL;

    for (int idx = 0; idx < ui->treeWidgetFieldList->topLevelItemCount(); idx++)
    {
        pItem = ui->treeWidgetFieldList->topLevelItem(idx);

        if (pItem && fieldList.contains(pItem->text(1)))
            ui->treeWidgetFieldList->topLevelItem(idx)->setCheckState(0, Qt::Checked);
        else
            ui->treeWidgetFieldList->topLevelItem(idx)->setCheckState(0, Qt::Unchecked);
    }
}

void PickFieldsDialog::onItemChanged(QTreeWidgetItem *pItem, int nColumn)
{
    if (nColumn == 0 && pItem)
    {
        // Get the state of the item
		int nState = pItem->checkState(nColumn);

        // state change into true, clear all other checkbox in the list
		if (nState)
		{
            mpPrivate->mSelectedFieldList.append(pItem->text(1));
		}
        else
        {
            mpPrivate->mSelectedFieldList.removeAll(pItem->text(1));
        }

        ui->lineEditFields->setText(mpPrivate->mSelectedFieldList.join(","));
    }
}

void PickFieldsDialog::onItemDoubleClicked(QTreeWidgetItem * pItem, int /*nColumn*/)
{
    if (pItem)
    {
        // Get the state of the item
		int nState = pItem->checkState(0);

        // state change into true, clear all other checkbox in the list
		if (nState)
            pItem->setCheckState(0, Qt::Unchecked);
        else
            pItem->setCheckState(0, Qt::Checked);
    }
}




