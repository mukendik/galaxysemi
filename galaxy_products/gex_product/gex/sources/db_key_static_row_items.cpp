#include "db_key_static_row_items.h"
#include "db_key_data.h"
#include "product_info.h"
#include "gex_shared.h"
#include "message.h"

#include <QStandardItemModel>

namespace GS
{
namespace Gex
{
DbKeyStaticRowItems::DbKeyStaticRowItems(DbKeyData &keyData,
                                         const QStringList &allowedNames,
                                         QMap<QString, QString> toolTips):
    mKeyData(keyData)
{
    mAllowedNames = allowedNames;
    mExpressionsToolTips = toolTips;
    mLineExpression.SetToolTipsMap(mExpressionsToolTips);
    initWidgets();

    SetName(mKeyData.Name());
    SetValue(mKeyData.Value());
    SetExpression(mKeyData.Expression());

    connect(&mComboName, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(OnNameChanged(QString)));

    connect(&mLineExpression, SIGNAL(textChanged(QString)),
            this, SLOT(OnExpressionChanged(QString)));

    connect(&mAddRow, SIGNAL(clicked()),
            this, SLOT(OnAddRowRequested()));

    connect(&mRemoveRow, SIGNAL(clicked()),
            this, SLOT(OnRemoveRowRequested()));
}

DbKeyStaticRowItems::~DbKeyStaticRowItems()
{
}

void DbKeyStaticRowItems::SetName(const QString &name)
{
    // set combo current item to the good one
    int comboIndex = mComboName.findText(name, Qt::MatchFixedString);
    if (comboIndex < 0 || comboIndex > mComboName.count())
        comboIndex = 0;
    mComboName.setCurrentIndex(comboIndex);
    if (comboIndex != 0)
        mComboName.setToolTip(mExpressionsToolTips.value(name));
    else
        mComboName.setToolTip("");
}

void DbKeyStaticRowItems::SetValue(const QString &value)
{
    mLineValue.setText(value);
}

void DbKeyStaticRowItems::SetExpression(const QString &expression)
{
    mLineExpression.setText(expression);
}

void DbKeyStaticRowItems::SetEvaluatedValue(const QString &evaluatedValue)
{
    mLineEvaluatedValue.setText(evaluatedValue);
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

void DbKeyStaticRowItems::initWidgets()
{
    // Combo Name
    mComboName.addItem("--Select a key--");
    for (int i = 0; i < mAllowedNames.size(); ++i)
        mComboName.addItem(mAllowedNames.at(i));
    mComboName.setMinimumHeight(18);
    mComboName.model()->sort(0, Qt::AscendingOrder);
    // Value
    QPalette palette = mLineValue.palette();
    palette.setBrush(QPalette::Base,
                     palette.brush(QPalette::Disabled, QPalette::Base));
    mLineValue.setPalette(palette);
    mLineValue.setReadOnly(true);
    mLineValue.setMinimumHeight(18);
    // Expression
    mLineExpression.setMinimumHeight(18);
    mLineExpression.SetCompleterSeparator(".");
    mLineExpression.SetCompleterModel(BuildExpressionTreeModelCompleter());
    // Evaluated expression
    mLineEvaluatedValue.setPalette(palette);
    mLineEvaluatedValue.setReadOnly(true);
    mLineEvaluatedValue.setMinimumHeight(18);
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
    insertWidget(0, &(mComboName));
    insertWidget(1, &(mLineValue));
    insertWidget(2, &(mLineExpression));
    insertWidget(3, &(mLineEvaluatedValue));
    insertWidget(4, &(mAddRow));
    insertWidget(5, &(mRemoveRow));
}

QStandardItemModel *DbKeyStaticRowItems::BuildExpressionTreeModelCompleter()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return NULL;
    }

    QList<QStringList> itemsLists;
    itemsLists.append(mAllowedNames);
    QStringList allowedFunctions;
    allowedFunctions << "Section()" << "RegExp()" << "Date()";
    itemsLists.append(allowedFunctions);

    // CAUTION: Only build 2 levels of items, not enough time...
    // Would be better with a recursive function to do as much level as needed

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

void DbKeyStaticRowItems::OnNameChanged(QString newName)
{
    mKeyData.SetName(newName);
    if (!newName.trimmed().isEmpty())
        mComboName.setToolTip(mExpressionsToolTips.value(newName));
}

void DbKeyStaticRowItems::OnExpressionChanged(QString newExpression)
{
    mKeyData.SetExpression(newExpression);
}

void DbKeyStaticRowItems::OnAddRowRequested()
{
    // Add row after the selected one
    emit AddRow(mKeyData.FlowId() + 1);
}

void DbKeyStaticRowItems::OnRemoveRowRequested()
{
    emit RemoveRow(mKeyData.FlowId());
}

} // END Gex
} // END GS
