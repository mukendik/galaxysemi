
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPainter>

#include "gexdb_plugin_base.h"
#include "limits_model.h"
#include "limits_delegate.h"

namespace GS
{
namespace Gex
{

LimitsDelegate::LimitsDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * LimitsDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (!index.isValid())
        return NULL;
    QWidget* lEditor = 0;

    const LimitsModel* lModel = static_cast<const LimitsModel*>(
                static_cast<const QSortFilterProxyModel*>(
                    index.model()
                    )->sourceModel()
                );
    TableItem::ObjectType lObjectType = lModel->type(index);

    switch (lObjectType)
    {
        case TableItem::STD_LINEEDIT:
        {
            lEditor = new QLineEdit(parent);
            break;
        }
        case TableItem::COMBOBOX_ALGO:
        {
            QComboBox* lCombo = new QComboBox(parent);
            for (int lIt = 0; lIt < mAlgoValues.size(); ++lIt)
            {
                lCombo->addItem(mAlgoNames.at(lIt), mAlgoValues.at(lIt));
            }
            lEditor = lCombo;
            break;
        }
        case TableItem::COMBOBOX_ENABLED:
        {
            QComboBox* lCombo = new QComboBox(parent);
            lCombo->addItem("Enabled", 1);
            lCombo->addItem("Disabled", 0);
            lEditor = lCombo;
            break;
        }
        case TableItem::STD_CHECKBOX:
        {
            lEditor = new QCheckBox(parent);
            break;
        }
        default:
        {
            lEditor = new QLabel(parent);
        }
    }

    lEditor->installEventFilter(const_cast<LimitsDelegate*>(this));
    return lEditor;
}


void LimitsDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
    if (!index.isValid())
        return;
    QVariant lValue;

    const LimitsModel*  lModel = static_cast<const LimitsModel*>(
                static_cast<const QSortFilterProxyModel*>(
                    index.model()
                    )->sourceModel()
                );
    TableItem::ObjectType lObjectType = lModel->type(index);

    switch (lObjectType)
    {
        case TableItem::STD_LINEEDIT:
        {
            lValue = index.model()->data(index, Qt::EditRole);
            QLineEdit* lLineEdit = static_cast<QLineEdit*>(editor);
            lLineEdit->setText(lValue.toString());
            break;
        }
        case TableItem::COMBOBOX_ALGO:
        case TableItem::COMBOBOX_ENABLED:
        {
            lValue = index.model()->data(index, Qt::UserRole);
            QComboBox* lCombo = static_cast<QComboBox*>(editor);
            lCombo->setCurrentIndex(lCombo->findData(lValue.toString()));
            break;
        }
        case TableItem::STD_CHECKBOX:
        {
            QCheckBox* lCheck = static_cast<QCheckBox*>(editor);
            lCheck->setChecked(lValue.toInt());
            break;
        }
        default:
        {
            lValue = index.model()->data(index, Qt::EditRole);
            QLabel* lLabel = static_cast<QLabel*>(editor);
            lLabel->setText(lValue.toString());
            break;
        }
    }
}

void LimitsDelegate::setModelData(QWidget *editor,
                                     QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    if (!index.isValid())
        return;
    QVariant lValue;

    const LimitsModel*  lModel = static_cast<const LimitsModel*>(
                static_cast<const QSortFilterProxyModel*>(
                    index.model()
                    )->sourceModel()
                );
    TableItem::ObjectType lObjectType = lModel->type(index);

    switch (lObjectType)
    {
        case TableItem::STD_LINEEDIT:
        {
            lValue = static_cast<QLineEdit*>(editor)->text();
            break;
        }
        case TableItem::COMBOBOX_ALGO:
        case TableItem::COMBOBOX_ENABLED:
        {
            lValue = static_cast<QComboBox*>(editor)->currentData();
            break;
        }
        case TableItem::STD_CHECKBOX:
        {
            lValue = (int)static_cast<QCheckBox*>(editor)->isChecked();
            break;
        }
        default:
        {
            lValue = static_cast<QLabel*>(editor)->text();
        }
    }

    model->setData(index, lValue);
}

void LimitsDelegate::setAlgoValues(QStringList values)
{
    mAlgoValues = values;
}

void LimitsDelegate::setAlgoNames(QStringList names)
{
    mAlgoNames = names;
}

} // namespace Gex
} // namespace GS
