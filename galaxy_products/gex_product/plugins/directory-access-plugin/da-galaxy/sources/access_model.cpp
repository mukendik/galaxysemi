
#include <QDomDocument>
#include <QStringList>

#include "access_model.h"
#include "access_item.h"

namespace GS
{
namespace DAPlugin
{

AccessModel::AccessModel(QDomNode &appEntriesNode, int privilegesType, QString targetedId, QObject *parent)
    : QAbstractItemModel(parent), mAppEntries(appEntriesNode)
{
    mRootItem = new AccessItem(appEntriesNode, privilegesType, targetedId, 0);
    mIsEditable = false;
}

AccessModel::~AccessModel()
{
    delete mRootItem;
}

void AccessModel::SetEditable(bool isEditable)
{
    mIsEditable = isEditable;
}

QVariant AccessModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    AccessItem *lItem = static_cast<AccessItem*>(index.internalPointer());
    if ((role == Qt::DisplayRole) && ((index.column() == 0) || (index.column() == 3)))
        return lItem->Data(index.column(), role);
    else if ((role == Qt::CheckStateRole) && ((index.column() > 0) && (index.column() < 3)))
        return QVariant(lItem->Data(index.column()).toBool() ? Qt::Checked : Qt::Unchecked );
    else if ((role == Qt::DecorationRole) && (index.column() == 0))
    {
        if (lItem->Data(index.column()).toString() == "galaxy")
            return QIcon(":/icons/options_gexapplication.png");
        else if (lItem->Data(index.column()).toString() == "databases")
            return QIcon(":/icons/database.png");
        else if (lItem->Data(index.column()).toString() == "users_groups_administrator")
            return QIcon(":/icons/yieldmandb_groups_mng.png");
        else if (lItem->Data(index.column()).toString() == "administrator")
            return QIcon(":/icons/admin-icon.png");
    }
    return QVariant();
}

bool AccessModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return true;
    AccessItem *lItem = static_cast<AccessItem*>(index.internalPointer());
    if (((index.column() > 0) && (index.column() < 3)) && (role == Qt::CheckStateRole))
    {
        emit dataChanged(index,index.child(index.row() + 1, index.column()));
        return lItem->SetData(index, (Qt::CheckState)value.toInt());
    }
    return QAbstractItemModel::setData(index, value, role );
}

Qt::ItemFlags AccessModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags lFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
    if ((index.column() > 0) && (index.column() < 3))
    {
        lFlags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsTristate;

        if (mIsEditable)
            lFlags |= Qt::ItemIsEnabled;
        else
            lFlags &= ~Qt::ItemIsEnabled;
    }

    return lFlags;
}

QVariant AccessModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return "Application entry";
            case 1:
                return "Read";
            case 2:
                return "Write";
            case 3:
                return "Description";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QModelIndex AccessModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    AccessItem *lParentItem = 0;
    if (!parent.isValid())
        lParentItem = mRootItem;
    else
        lParentItem = static_cast<AccessItem*>(parent.internalPointer());
    AccessItem *lChildItem = lParentItem->child(row);
    if (lChildItem)
        return createIndex(row, column, lChildItem);
    else
        return QModelIndex();
}

QModelIndex AccessModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    AccessItem *lChildItem = static_cast<AccessItem*>(child.internalPointer());
    AccessItem *lParentItem = lChildItem->parent();
    if (!lParentItem || (lParentItem == mRootItem))
        return QModelIndex();
    return createIndex(lParentItem->row(), 0, lParentItem);
}

int AccessModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    AccessItem *lParentItem;
    if (!parent.isValid())
        lParentItem = mRootItem;
    else
        lParentItem = static_cast<AccessItem*>(parent.internalPointer());
    return lParentItem->childCount();
}

int AccessModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 4;
}

} // END DAPlugin
} // END GS


