#include "table_item.h"

#include "engine.h"
#include "spm_engine.h"

namespace GS
{
namespace Gex
{

TableItem::TableItem(Qt::ItemFlags flags):
    mFlags(flags)
{

}

QVariant TableItem::data(int role/*= Qt::UserRole */) const
{
    if (role == Qt::UserRole)
        return mData;
    else if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    {
        if (mObjectType == COMBOBOX_ALGO)
        {
           return Engine::GetInstance().GetSPMEngine().GetSupportedOutlierAlgorithms().value(mData.toString());
        }
        else if (mObjectType == COMBOBOX_ENABLED)
        {
            return mData.toInt()?"enabled":"disabled";
        }
        else if (mObjectType == LINEEDIT_CRIT)
        {
            return (mData.toInt()==1)?"standard":"critical";
        }
        else
        {
            return mData;
        }
    }
    else
        return QVariant();
}

Qt::ItemFlags TableItem::flags() const
{
    return mFlags;
}

TableItem::ObjectType TableItem::objectType() const
{
    return mObjectType;
}

void TableItem::setData(const QVariant& data)
{
    mData = data;
}

void TableItem::setFlags(const Qt::ItemFlags& flags)
{
    mFlags = flags;
}

void TableItem::setObjectType(const ObjectType &itemDelegate)
{
    mObjectType = itemDelegate;
}

} // namespace Gex
} // namespace GS
