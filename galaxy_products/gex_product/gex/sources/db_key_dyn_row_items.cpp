#include "db_key_dyn_row_items.h"
#include "db_key_data.h"

#include <QStandardItemModel>

namespace GS
{
namespace Gex
{
DbKeyDynRowItems::DbKeyDynRowItems(DbKeyData &keyData,
                                   const QStringList &allowedNames):
    mKeyData(keyData)
{
    mAllowedNames = allowedNames;

    initWidgets();

    mLineName.setText(mKeyData.Name());
    mLineExpression.setText(mKeyData.Expression());

    connect(&mLineName, SIGNAL(textChanged(QString)),
            this, SLOT(OnNameChanged(QString)));

    connect(&mLineExpression, SIGNAL(textChanged(QString)),
            this, SLOT(OnExpressionChanged(QString)));

    connect(&mAddRow, SIGNAL(clicked()),
            this, SLOT(OnAddRowRequested()));

    connect(&mRemoveRow, SIGNAL(clicked()),
            this, SLOT(OnRemoveRowRequested()));
}

DbKeyDynRowItems::~DbKeyDynRowItems()
{
}

void DbKeyDynRowItems::SetNameToolTips(QMap<QString, QString> toolTips)
{
    mLineName.SetToolTipsMap(toolTips);
}

void DbKeyDynRowItems::SetExpressionToolTips(QMap<QString, QString> toolTips)
{
    mLineExpression.SetToolTipsMap(toolTips);
}

void DbKeyDynRowItems::initWidgets()
{
    // Name
    mLineName.setMinimumHeight(18);
    mLineName.SetCompleterSeparator("[");
    mLineName.SetCompleterModel(BuildNameTreeModelCompleter());
    // Expression
    mLineExpression.setMinimumHeight(18);
    mLineExpression.SetCompleterSeparator(".");
    mLineExpression.SetCompleterModel(BuildExpressionTreeModelCompleter());
    // Button add
    mAddRow.setMaximumSize(18, 18);
    mAddRow.setIcon(QIcon (":/gex/icons/add.png"));
    QPalette lPalette = mAddRow.palette();
    lPalette.setColor(mAddRow.backgroundRole(), Qt::white);
    mAddRow.setPalette(lPalette);
    mAddRow.setFlat(true);
    mAddRow.setToolTip("New row");
    // Button remove
    mRemoveRow.setMaximumSize(18, 18);
    mRemoveRow.setIcon(QIcon (":/gex/icons/remove.png"));
    lPalette = mRemoveRow.palette();
    lPalette.setColor(mRemoveRow.backgroundRole(), Qt::white);
    mRemoveRow.setPalette(lPalette);
    mRemoveRow.setFlat(true);
    mRemoveRow.setToolTip("Delete this row");
    // Insert into layout
    insertWidget(0, &(mLineName));
    insertWidget(1, &(mLineExpression));
    insertWidget(2, &(mAddRow));
    insertWidget(3, &(mRemoveRow));
}

QStandardItemModel *DbKeyDynRowItems::BuildNameTreeModelCompleter()
{
    QStringList allowedNames;
    allowedNames << "test[name]" << "test[number]" << "testCondition[]";

    QStandardItemModel *model = new QStandardItemModel();

    QStandardItem *parentItem = model->invisibleRootItem();

    // Set all first level items
    for (int i = 0; i < allowedNames.count(); ++i)
    {
        QStandardItem *item = new QStandardItem;
        item->setText(allowedNames.at(i).trimmed());
        parentItem->appendRow(item);
    }

    return model;
}

QStandardItemModel *DbKeyDynRowItems::BuildExpressionTreeModelCompleter()
{
    QList<QStringList> itemsLists;
    mAllowedNames << "test[name]" << "test[number]" << "testCondition[]";
    itemsLists.append(mAllowedNames);
    QStringList allowedFunctions;
    allowedFunctions << "Section()" << "RegExp()" << "Date()";
    itemsLists.append(allowedFunctions);

    QStandardItemModel *model = new QStandardItemModel();

    QStandardItem *parentItem = model->invisibleRootItem();

    // Set all first level items
    QStringList firstLevelItems = itemsLists.at(0);
    for (int i = 0; i < firstLevelItems.count(); ++i)
    {
        QStandardItem *item = new QStandardItem;
        item->setText(firstLevelItems.at(i).trimmed());
        parentItem->appendRow(item);
    }

    // Set all first level items
    QStringList secondLevelItems = itemsLists.at(1);
    for (int childRow = 0; childRow < parentItem->rowCount(); childRow++)
    {
        for (int itemId = 0; itemId < secondLevelItems.count(); ++itemId)
        {
            QStandardItem *item = new QStandardItem;
            item->setText(secondLevelItems.at(itemId).trimmed());
            parentItem->child(childRow)->appendRow(item);
        }
    }

    return model;
}

void DbKeyDynRowItems::OnNameChanged(QString newName)
{
    mKeyData.SetName(newName);
}

void DbKeyDynRowItems::OnExpressionChanged(QString newExpression)
{
    mKeyData.SetExpression(newExpression);
}

void DbKeyDynRowItems::OnAddRowRequested()
{
    // Add row after the selected one
    emit AddRow(mKeyData.FlowId() + 1);
}

void DbKeyDynRowItems::OnRemoveRowRequested()
{
    emit RemoveRow(mKeyData.FlowId());
}

void DbKeyDynRowItems::CheckValidity()
{
    if (!mKeyData.IsValidName()) // If not valid set color to red
    {
        QPalette palette = mLineName.palette();
        palette.setColor(QPalette::Text, Qt::red);
        mLineName.setPalette(palette);
    }
    else // Else restore original color
    {
        QPalette palette = mLineName.palette();
        palette.setColor(QPalette::Text, Qt::black);
        mLineName.setPalette(palette);
    }

    if (!mKeyData.IsValidResult()) // If not valid set color to red
    {
        QPalette palette = mLineExpression.palette();
        palette.setColor(QPalette::Text, Qt::red);
        mLineExpression.setPalette(palette);
    }
    else // Else restore original color
    {
        QPalette palette = mLineExpression.palette();
        palette.setColor(QPalette::Text, Qt::black);
        mLineExpression.setPalette(palette);
    }
}

} // END Gex
} // END GS
