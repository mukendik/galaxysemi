#include <component.h>
#include "tree_item_report.h"



TreeItemReport::TreeItemReport(const QString &name, const QIcon& icon, TreeItemReport *parent):
                                                                            mItemType(T_NO_ITEM),
                                                                            mIcon(icon),
                                                                            mParent(parent),
                                                                            mComponent(0)
{
    mData.append(name);
    mFont.setBold(false);
    mFont.setItalic(false);
}


TreeItemReport::~TreeItemReport()
{
    Clear();
}


void TreeItemReport::Clear()
{
    qDeleteAll(mChilds);
    mChilds.clear();
    mParent = 0;
}

void TreeItemReport::AppendChild(TreeItemReport *item)
{
    mChilds.append(item);
}

TreeItemReport* TreeItemReport::FindChild(const QString& name)
{
    QList<TreeItemReport*>::iterator lIterBegin(mChilds.begin()), lIterEnd(mChilds.end());
    for(; lIterBegin < lIterEnd; ++lIterBegin)
    {
        if((*lIterBegin)->Data(0).toString() == name)
            return *lIterBegin;
    }
    return 0;
}

TreeItemReport *TreeItemReport::GetParent() const
{
    return mParent;
}

void TreeItemReport::SetParent(TreeItemReport *parent)
{
    mParent = parent;
}

QIcon TreeItemReport::Icon() const
{
    return mIcon;
}

QFont TreeItemReport::Font() const
{
    return mFont;
}

void TreeItemReport::InsertChild(int indexOf, TreeItemReport *item)
{
    mChilds.insert(indexOf, item);
}

void TreeItemReport::SetData(const QVariant & variant, int role)
{
    if(role == Qt::DisplayRole)
        mData[0] = variant.toString();
    else if(role == Qt::DecorationRole)
        mIcon = variant.value<QIcon>();
    else if(role == Qt::FontRole)
        mFont = variant.value<QFont>();
}

Component *TreeItemReport::GetComponent() const
{
    return mComponent;
}

void TreeItemReport::SetComponent(Component *component)
{
    mComponent = component;
}

T_VIEW_ITEM_TYPE TreeItemReport::GetType() const
{
    return mItemType;
}

void TreeItemReport::SetItemType(T_VIEW_ITEM_TYPE itemType)
{
    mItemType = itemType;
}


int TreeItemReport::ChildCount() const
{
    return mChilds.count();
}

TreeItemReport *TreeItemReport::Child(int row)
{
    return mChilds.value(row);
}


int TreeItemReport::Row() const
{
    if (mParent)
        return mParent->mChilds.indexOf(const_cast<TreeItemReport*>(this));

    return 0;
}

void TreeItemReport::RemoveChild(int indexOf)
{
    mChilds.removeAt(indexOf);
}

int TreeItemReport::ColumnCount() const
{
    return mData.count();
}

QVariant TreeItemReport::Data(int index) const
{
    return mData[index];
}
