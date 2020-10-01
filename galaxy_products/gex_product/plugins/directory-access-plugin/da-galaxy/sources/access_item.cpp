
#include "access_item.h"
#include "app_entries.h"

namespace GS
{
namespace DAPlugin
{

AccessItem::AccessItem(QDomNode &node, int privilegesType, QString targetedId, int row, AccessItem *parent)
{
    mDomNode = node;
    mRowNumber = row;
    mParentItem = parent;
    mPrivilegesType = static_cast<PrivilegesType>(privilegesType);
    mTargetedId = targetedId;
}

AccessItem::~AccessItem()
{
    QHash<int,AccessItem*>::iterator lIt;
    for (lIt = mChildItems.begin(); lIt != mChildItems.end(); ++lIt)
        delete lIt.value();
}

QDomNode AccessItem::GetNode() const
{
    return mDomNode;
}

int AccessItem::GetPrivilegesType() const
{
    return static_cast<int>(mPrivilegesType);
}

QString AccessItem::GetTargetedId() const
{
    return mTargetedId;
}

AccessItem *AccessItem::parent()
{
    return mParentItem;
}

AccessItem *AccessItem::child(int i)
{
    if (mChildItems.contains(i))
        return mChildItems[i];

    QDomNode lChildNode = mDomNode.firstChild();
    int lCountChildren = 0;
    while(!lChildNode.isNull())
    {
        // only check entry elt
        if (lChildNode.nodeName() == ENTRY_NODE)
        {
            if (lCountChildren == i)
            {
                AccessItem *childItem = new AccessItem(lChildNode, GetPrivilegesType(), GetTargetedId(), i, this);
                mChildItems[i] = childItem;
                return childItem;
            }
            ++lCountChildren;
        }
        lChildNode = lChildNode.nextSibling();
    }
    return 0;
}

int AccessItem::childCount() const
{
    QDomNode lChildNode = mDomNode.firstChild();
    int lChildrenCount = 0;
    while(!lChildNode.isNull())
    {
        // only check entry elt
        if (lChildNode.nodeName() == ENTRY_NODE)
            ++lChildrenCount;
        lChildNode = lChildNode.nextSibling();
    }
    return lChildrenCount;
}

QVariant AccessItem::Data(int column, int role/*=-1*/)
{
    switch (column)
    {
        case 0:
            {
                if (role == Qt::DisplayRole)
                    return GetNode().firstChildElement(ENTRY_NAME).text();
                else
                    return GetNode().toElement().attribute(ENTRY_ID);
            }
        case 1://READ
        case 2://WRITE
            return HasNodePrivilege(this, column);
        case 3:
            return GetNode().firstChildElement(ENTRY_DESC).text();
        default:
            return QVariant();
    }
}

bool AccessItem::SetData(const QModelIndex &index, const QVariant &value)
{
    if ((index.column() > 0) && (index.column() < 3))
    {
        if (((Qt::CheckState)value.toInt() == Qt::Checked) ||
                ((Qt::CheckState)value.toInt() == Qt::PartiallyChecked))
            return SetNodePrivilege(this, index.column());
        else
            return UnsetNodePrivilege(this, index.column());
    }

    return true;
}

int AccessItem::row() const
{
    return mRowNumber;
}

bool AccessItem::SetNodePrivilege(AccessItem *item, int column)
{
    if (column < 1)
        return false;

    for (int i = 0; i < item->childCount(); ++i)
        SetNodePrivilege(item->child(i), column);

    int lPrivilege = GetPrivilegeFlag(column);

    // if write enabled then enable read defaultly
    if (lPrivilege == GS::DAPlugin::WRITEACCESS)
        lPrivilege |= GS::DAPlugin::READACCESS;

    if (mPrivilegesType == GROUP)
        return (AppEntries::UpdateGroupPrivilege(
                    item->GetNode(),
                    mTargetedId,
                    AppEntries::GetGroupPrivileges(item->GetNode(), mTargetedId) | lPrivilege));
    else if (mPrivilegesType == USER)
        return (AppEntries::UpdateUserPrivilege(
                    item->GetNode(),
                    mTargetedId,
                    AppEntries::GetUserPrivileges(item->GetNode(), mTargetedId) | lPrivilege));
    else
        return false;
}

bool AccessItem::UnsetNodePrivilege(AccessItem *item, int column)
{
    if (column < 1)
        return false;

    int lPrivilege = GetPrivilegeFlag(column);

    // if read disabled then disable write
    if (lPrivilege == GS::DAPlugin::READACCESS)
        lPrivilege |= GS::DAPlugin::WRITEACCESS;

    if (mPrivilegesType == GROUP)
        return (AppEntries::UpdateGroupPrivilege(
                    item->GetNode(),
                    mTargetedId,
                    AppEntries::GetGroupPrivileges(item->GetNode(), mTargetedId) & ~lPrivilege));
    else if (mPrivilegesType == USER)
        return (AppEntries::UpdateUserPrivilege(
                    item->GetNode(),
                    mTargetedId,
                    AppEntries::GetUserPrivileges(item->GetNode(), mTargetedId) & ~lPrivilege));
    else
        return false;
}

int AccessItem::HasNodePrivilege(AccessItem* item, int column)
{
    if (column < 1)
        return false;

    int lPrivilege = GetPrivilegeFlag(column);

    if (mPrivilegesType == GROUP)
        return ((AppEntries::GetGroupPrivileges(item->GetNode(), mTargetedId) & lPrivilege) == lPrivilege);
    else if (mPrivilegesType == USER)
        return ((AppEntries::GetUserPrivileges(item->GetNode(), mTargetedId) & lPrivilege) == lPrivilege);
    else
        return false;
}

int AccessItem::GetPrivilegeFlag(int column)
{
    int lBase = 2;
    int lExponent = column - 1;
    int lPrivilege = 1;
    for (int i = 0; i < lExponent; ++i)
        lPrivilege *= lBase;

    return lPrivilege;
}

} // END DAPlugin
} // END GS

