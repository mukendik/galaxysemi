#include <QMimeData>

#include "tree_model_report.h"

const QString HistoStr  = "histo";
const QString ProbaStr  = "proba";
const QString TrendStr  = "trend";
const QString BoxStr    = "box";
const QString WaferStr  = "wafer";
const QString Default  = "default";

static QMap<T_Component, QIcon> initIcons() {
    QMap<T_Component, QIcon>map;
    map.insert(T_HISTO,                         QIcon(":/gex/icons/options_advhisto.png"));
    map.insert(T_PROBA,                         QIcon(":/gex/icons/options_probabilityplot.png"));
    map.insert(T_TREND,                         QIcon(":/gex/icons/options_trend.png"));
    map.insert(T_BOXPLOT,                       QIcon(":/gex/icons/options_boxmeanrange.png"));
    map.insert(T_WAFER,                         QIcon(":/gex/icons/options_wafermap.png"));
    map.insert(T_TABLE,              QIcon(":/gex/icons/options_statistics.png"));
    map.insert(T_HISTO_CONNECTED,               QIcon(":/gex/icons/options_advhisto.png"));
    map.insert(T_PROBA_CONNECTED,               QIcon(":/gex/icons/options_probabilityplot.png"));
    map.insert(T_TREND_CONNECTED,               QIcon(":/gex/icons/options_trend.png"));
    map.insert(T_BOXPLOT_CONNECTED,             QIcon(":/gex/icons/options_boxmeanrange.png"));
    map.insert(T_WAFER_CONNECTED,               QIcon(":/gex/icons/options_wafermap.png"));
//    map.insert(T_HISTO_CONNECTED,               QIcon(":/gex/icons/options_advhisto_connected.png"));
//    map.insert(T_PROBA_CONNECTED,               QIcon(":/gex/icons/options_probabilityplot_connected.png"));
//    map.insert(T_TREND_CONNECTED,               QIcon(":/gex/icons/options_trend_connected.png"));
//    map.insert(T_BOXPLOT_CONNECTED,             QIcon(":/gex/icons/options_boxmeanrange_connected.png"));
//    map.insert(T_WAFER_CONNECTED,               QIcon(":/gex/icons/options_wafermap_connected.png"));
    map.insert(T_CAPABILITY_TABLE_CONNECTED,    QIcon(":/gex/icons/options_statistics.png"));
    map.insert(T_NONE,                          QIcon());
    return map;
}

QMap<T_Component, QIcon>    TreeModelReport::mIcons;

TreeModelReport::TreeModelReport(QObject* parent): QAbstractItemModel(parent)
{
    mRoot.SetItemType(T_ROOT_ITEM);
    if(mIcons.isEmpty())
        mIcons = initIcons();
}

TreeModelReport::~TreeModelReport()
{
}


TreeItemReport* TreeModelReport::addSection(const QString& sectionName, Component* component)
{
    //-- search if the section already exist, if not create a new one
    TreeItemReport* lSection =mRoot.FindChild(sectionName);
    if(lSection == 0)
    {
        int lRow = mRoot.ChildCount();
        lSection = insertNewItem(lRow, sectionName, QIcon(""), QFont());
    }
    lSection->SetParent(&mRoot);
    lSection->SetComponent(component);
    lSection->SetItemType(T_SECTION_ITEM);

    return lSection;
}

TreeItemReport* TreeModelReport::addItemToSection(TreeItemReport* section, const QString& elementName, Component *component)
{
    QModelIndex lIndexParent = indexOfItem(section);
    if(lIndexParent.isValid() == false)
        return 0;

    TreeItemReport* itemReport = insertNewItem(section->ChildCount(), elementName, initIcon(component->GetType()), initFont(component->GetType()), lIndexParent);
    itemReport->SetComponent(component);
    itemReport->SetItemType(T_ELEMENT_ITEM);
    return itemReport;
}

QModelIndex TreeModelReport::indexOfItem( TreeItemReport* item, const QModelIndex& parent)
{
    if(item == 0)
        return QModelIndex();

    return index(item->Row(), 0, parent);
}

QIcon& TreeModelReport::initIcon(T_Component type)
{
    if(mIcons.contains(type))
        return mIcons[type];
    else
        return mIcons[T_NONE] ;
}

QFont TreeModelReport::initFont(T_Component type)
{
    QFont lFont;
    lFont.setItalic(false);
    if(type == T_HISTO_CONNECTED || type == T_BOXPLOT_CONNECTED || type == T_TREND_CONNECTED ||
            type== T_PROBA_CONNECTED || type == T_WAFER_CONNECTED)
        lFont.setItalic(true);

    return lFont;
}

void TreeModelReport::clear()
{
    int lNbChilds = mRoot.ChildCount();
    for(int i = 0; i< lNbChilds; ++i)
    {
        TreeItemReport* lSection = mRoot.Child(i);

        QModelIndex lIndexSection = index(i, 0, QModelIndex());
        removeRows(0, lSection->ChildCount(), lIndexSection );

        lSection->Clear();
    }

    if(lNbChilds > 0)
        removeRows(0, lNbChilds);
    mRoot.Clear();
}

int TreeModelReport::columnCount(const QModelIndex &parent) const
{

    if (parent.isValid())
        return static_cast<TreeItemReport*>(parent.internalPointer())->ColumnCount();
    else
        return mRoot.ColumnCount();
}


QVariant TreeModelReport::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::FontRole)
        return QVariant();

    TreeItemReport *lItem = static_cast<TreeItemReport*>(index.internalPointer());

    if(role == Qt::DisplayRole)
        return lItem->Data(index.column());
    else if (role == Qt::DecorationRole)
        return lItem->Icon();
    else if(role == Qt::FontRole)
        return lItem->Font();

    return QVariant();
}


bool TreeModelReport::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0 )
        return false;

    const TreeMimeData *lItemToMove = qobject_cast<const TreeMimeData *>(data);
    if(lItemToMove == 0)
        return false;

    if(lItemToMove->mType == T_ELEMENT_ITEM)
    {  
        if (!parent.isValid())
            return false;

        TreeItemReport* lItemParent = static_cast<TreeItemReport*>(parent.internalPointer());
        if(lItemParent == 0)
            return false;

        // -- can't drop out of the lItemToMove's section
        // -- 1/ drop on a section
        // -- 2/ drop inside another section
        // -- 3/ drop on an item of anotehr section
        if( (lItemParent->GetType() == T_SECTION_ITEM && row == -1) ||
                (lItemParent->GetType() == T_SECTION_ITEM && lItemParent != lItemToMove->mParent) ||
                (lItemParent->GetType() == T_ELEMENT_ITEM && lItemParent->GetParent() != lItemToMove->mParent)
                )
        {
            return false;
        }

        int lRowToInsert = row;
        QModelIndex lParentIndex = parent;
        // -- Drop on a item
        if(row == -1)
        {
            TreeItemReport* lItem   = static_cast<TreeItemReport*>(parent.internalPointer());
            lRowToInsert            = lItem->Row();
            lParentIndex            = parent.parent();

            // --  need this for an item that is drop on a spot under its current index
            if(lItemToMove->mRow < lRowToInsert)
                ++lRowToInsert;
        }

        //-- extract name
        QByteArray encodedData = data->data("application/vnd.text.list");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QString lName;
        stream >> lName;

        TreeItemReport* lItemReport = insertNewItem(lRowToInsert, lName, data->imageData().value<QIcon>(), lItemToMove->mFont, lParentIndex);
        lItemReport->SetComponent(lItemToMove->mComponent);
        lItemReport->SetItemType(T_ELEMENT_ITEM);
        emit changedHasBeenMade();
    }
    else if(lItemToMove->mType == T_SECTION_ITEM)
    {
        int lRowToInsert = row;
        QModelIndex lParentIndex = parent;
        // -- Drop on a item
        if(row == -1)
        {

            if (parent.isValid() == false)
                return false;

            TreeItemReport* lItemParent = static_cast<TreeItemReport*>(parent.internalPointer());
            if(lItemParent->GetType() == T_ELEMENT_ITEM)
                return false;

            TreeItemReport* lItem   = static_cast<TreeItemReport*>(parent.internalPointer());

            lRowToInsert            = lItem->Row();
            lParentIndex            = parent.parent();

            // --  need this for an item that is drop on a spot under its current index
            if(lItemToMove->mRow < lRowToInsert)
                ++lRowToInsert;
        }
        else
        {
            // -- mean that we drop inside anoter section between items (forbiden)
            if (parent.isValid())
            {
                return false;
            }
        }

        //-- extract name
        QByteArray encodedData = data->data("application/vnd.text.list");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QString lName;
        stream >> lName;

        TreeItemReport* lItemReport = insertNewItem(lRowToInsert, lName, QIcon(), QFont(), lParentIndex);
        lItemReport->SetComponent(lItemToMove->mComponent);
        lItemReport->SetItemType(T_SECTION_ITEM);

        TreeItemReport* lItemDragged = lItemToMove->mDragged;

        for(int i = 0; i < lItemDragged->ChildCount(); ++i)
        {
            addItemToSection(lItemReport, lItemDragged->Child(i)->Data(0).toString(), lItemDragged->Child(i)->GetComponent());
        }
        emit changedHasBeenMade();

    }
    return true;
}

Qt::ItemFlags TreeModelReport::flags(const QModelIndex &index) const
{
    Qt::ItemFlags lDefaultFlags = QAbstractItemModel::flags(index);

    if (!index.isValid())
        return  Qt::ItemIsDropEnabled | lDefaultFlags;

    return  Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | lDefaultFlags;
}

TreeItemReport* TreeModelReport::getItemAt(QModelIndex index)
{
    if(index.isValid())
        return static_cast<TreeItemReport*>(index.internalPointer());
    return 0;
}

QVariant TreeModelReport::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRoot.Data(section);

    return QVariant();
}


QModelIndex TreeModelReport::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItemReport *lParentItem = 0;

    if (!parent.isValid())
        lParentItem = const_cast<TreeItemReport*>(&mRoot);
    else
        lParentItem = static_cast<TreeItemReport*>(parent.internalPointer());

    TreeItemReport *lChildItem = lParentItem->Child(row);
    if (lChildItem)
        return createIndex(row, column, lChildItem);
    else
        return QModelIndex();
}


TreeItemReport* TreeModelReport::insertNewItem(int row,
                                               const QString& name,
                                               const QIcon& icon,
                                               const QFont& font,
                                               const QModelIndex& parent/*=QModelIndex()*/)
{
   insertRow(row, parent);
   QModelIndex lIndex = index(row, 0, parent);

   QModelIndex indexParent = lIndex.parent();
   if(indexParent.isValid() == false)
      indexParent = indexParent;

    setData(lIndex, name, Qt::DisplayRole);
    setData(lIndex, icon, Qt::DecorationRole);
    setData(lIndex, font, Qt::FontRole);
    return static_cast<TreeItemReport*>(lIndex.internalPointer());
}


bool TreeModelReport::insertRow (int row, const QModelIndex & parent)
{
     beginInsertRows(parent, row, row);

     TreeItemReport* lTreeItemParent;
     if(parent.isValid())
       lTreeItemParent = static_cast<TreeItemReport*> (parent.internalPointer());
     else
        lTreeItemParent = &mRoot;

     lTreeItemParent->InsertChild(row, new TreeItemReport("", QIcon(), lTreeItemParent));
     endInsertRows();
     return true;
}

QStringList TreeModelReport::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData *TreeModelReport::mimeData(const QModelIndexList &indexes) const
{
    TreeMimeData *mimeData = new TreeMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    const QModelIndex &index = indexes[0];
    if (index.isValid()) {
        QString lText = data(index, Qt::DisplayRole).toString();
        stream << lText;

        TreeItemReport* lItemReport = static_cast<TreeItemReport*>(index.internalPointer());

        mimeData->setImageData(data(index, Qt::DecorationRole));
        mimeData->mType         = lItemReport->GetType();
        mimeData->mText         = lText;
        mimeData->mDragged      = lItemReport;
        mimeData->mParent       = lItemReport->GetParent();
        mimeData->mComponent    = lItemReport->GetComponent();
        mimeData->mFont         = data(index, Qt::FontRole).value<QFont>();
        mimeData->mRow          = lItemReport->Row();
        mimeData->setData("application/vnd.text.list", encodedData);
        return mimeData;
    }

    return 0;
}

QModelIndex TreeModelReport::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    QModelIndex lIndex = index;

    TreeItemReport *lChildItem = static_cast<TreeItemReport*>(lIndex.internalPointer());
    TreeItemReport *lParentItem = lChildItem->GetParent();

    if (lParentItem == 0)
         return QModelIndex();

    if (lParentItem == &mRoot)
        return QModelIndex();

    return createIndex(lParentItem->Row(), 0, lParentItem);
}

// Not enough generic... To review
void TreeModelReport::removeItem(TreeItemReport* item)
{
    // -- section
    if(item->GetType() == T_SECTION_ITEM)
    {
        QModelIndex lIndexSection = index(item->Row(), 0, QModelIndex());
        // -- remove all item below
        removeRows(0, item->ChildCount(), lIndexSection);
        // -- remove the section from the view
        removeRow(item->Row());
        item->Clear();
    }
    else if(item->GetType() ==T_ELEMENT_ITEM)
    {
        // -- Item inside section
        QModelIndex lIndexSection = indexOfItem(item->GetParent());
        // -- remove only the item
        removeRows(item->Row(), 1, lIndexSection);
        item->Clear();
    }
}

bool TreeModelReport::removeRows( int row, int count, const QModelIndex & parent )
{
    TreeItemReport *lParentItem;
    if (!parent.isValid())
       lParentItem = &mRoot   ;
    else
      lParentItem  = static_cast<TreeItemReport*>(parent.internalPointer());

    int lRowEnd = row + (count);
    if(lRowEnd < 0)
        return false;

    beginRemoveRows(parent, row, lRowEnd - 1);
    for(int i = lRowEnd - 1; i >= row; --i)
    {
        lParentItem->RemoveChild(i);
    }

    endRemoveRows();

    return true;
}

void TreeModelReport::retrieveIndexPosition()
{
    int lNbSections = mRoot.ChildCount();
    for(int i =0; i < lNbSections; ++i)
    {
        TreeItemReport* lSection = mRoot.Child(i);
        lSection->GetComponent()->mIndexPosition = i;

        int lNbItems = lSection->ChildCount();
        for(int j =0; j < lNbItems; ++j)
        {
            Component *lComponent = lSection->Child(j)->GetComponent();
            lComponent->mIndexPosition = j;
        }
    }
}

int TreeModelReport::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    const TreeItemReport *lParentItem = 0;
    if (!parent.isValid())
        lParentItem = &mRoot;
    else
        lParentItem = static_cast<TreeItemReport*>(parent.internalPointer());

    return lParentItem->ChildCount();
}

bool TreeModelReport::setData(const QModelIndex & index, const QVariant & value, int role)
{
    static_cast<TreeItemReport*>(index.internalPointer())->SetData(value, role);
    emit dataChanged(index, index);
    return true;
}

Qt::DropActions TreeModelReport::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void    TreeModelReport::updateDataSection(QModelIndex indexSection)
{
    if(indexSection.isValid() ) {
        TreeItemReport* lItem = static_cast< TreeItemReport*>(indexSection.internalPointer());

        //-- loop over child and reload the icon
        for(int i = 0; i < lItem->ChildCount(); ++i)
        {
            QModelIndex lIndexChild = index(lItem->Child(i)->Row(), 0, indexSection);
             setData(lIndexChild, initFont(lItem->Child(i)->GetComponent()->GetType()), Qt::FontRole);
        }
    }
}

void    TreeModelReport::updateDataName(QModelIndex index, const QString& name, T_VIEW_ITEM_TYPE type)
{
    if(index.isValid() && type != T_NO_ITEM) {
        TreeItemReport* lItem = static_cast< TreeItemReport*>(index.internalPointer());

        //-- index is dirrectly updated
        if(lItem->GetType() == type) {
            setData(index, name, Qt::DisplayRole);
        }
        else
        {
            //-- we modified the parent name of the index
            setData(index.parent(), name, Qt::DisplayRole);
        }
    }
}

void TreeModelReport::reloadData(QModelIndex index, Qt::ItemDataRole role)
{
    if(index.isValid())
    {
        TreeItemReport* lItem = static_cast< TreeItemReport*>(index.internalPointer());
        if(lItem && lItem->GetComponent())
        {
            if(role == Qt::DisplayRole)
                setData(index, lItem->GetComponent()->GetName(), Qt::DisplayRole);
            else if(role == Qt::DecorationRole)
                setData(index, initIcon(lItem->GetComponent()->GetType()), Qt::DecorationRole);
            else if(role == Qt::FontRole)
            {
                setData(index, initFont(lItem->GetComponent()->GetType()), Qt::FontRole);
            }
        }
    }
}

TreeItemReport* TreeModelReport::sectionAt (int index)
{
    if(index < mRoot.ChildCount())
    {
        return mRoot.Child(index);
    }
    return 0;
}

int TreeModelReport::sectionsCount () const
{
    return mRoot.ChildCount();
}
