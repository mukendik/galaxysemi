#include "table_model.h"

namespace GS
{
namespace Gex
{

TableModel::TableModel(const QList<QPair<QString, QString> >& headerData,
                       QObject *parent)
    : QAbstractTableModel(parent),
      mHeaderData(headerData)
{
    reset();
    for (int lIt = 0; lIt < mHeaderData.size(); ++lIt)
    {
       mRawHeaderIndex.insert(mHeaderData.at(lIt).first, lIt);
       mDecoractedHeaderIndex.insert(mHeaderData.at(lIt).second, lIt);
    }
    mRowCount = 0;
}

void TableModel::clear()
{
    this->beginResetModel();
    reset();
    this->endResetModel();
}

int TableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return mRowCount;
}


int TableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return mHeaderData.size();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    return mRowList[index.row()][index.column()].flags();
}

bool TableModel::setFlags(const QModelIndex &index, Qt::ItemFlags flags)
{
    mRowList[index.row()][index.column()].setFlags(flags);

    return true;
}

bool TableModel::setHeaderData(int section,
                                   Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
    if (((role != Qt::DisplayRole) ||
         (orientation != Qt::Horizontal)) ||
            (section >= mHeaderData.count()) ||
            mHeaderData.isEmpty())
        return false;

    mHeaderData[section].second = value.toString();

    emit headerDataChanged(orientation, section, section);

    return true;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (mHeaderData.isEmpty())
        return QVariant();

    if ((role == Qt::DisplayRole && orientation == Qt::Horizontal) &&
            (section < mHeaderData.size()))
    {
        return mHeaderData.at(section).second;
    }
    else if ((role == Qt::UserRole && orientation == Qt::Horizontal) &&
             (section < mHeaderData.size()))
    {
        return mHeaderData.at(section).first;
    }

    return QVariant();
}


bool TableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    int columns = columnCount();
    if (columns <= 0)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    for (int lRowIt = 0; lRowIt < rows; ++lRowIt)
    {
        QList<TableItem> lItems;
        for (int column = 0; column < columns; ++column)
            lItems.append(TableItem());
        mRowList.insert(position, lItems);
    }
    endInsertRows();

    mRowCount += rows;

    return true;
}

bool TableModel::insertColumns(int position, int columns,
                               const QModelIndex &parent)
{
    int lRows = rowCount();
    beginInsertColumns(parent, position, position + columns - 1);

    for (int lRowIt = 0; lRowIt < lRows; ++lRowIt)
    {
        for (int lColIt = position; lColIt < columns; ++lColIt)
        {
            mRowList[lRowIt].insert(position, TableItem());
        }
    }

    for (int lColIt = position; lColIt < columns; ++lColIt)
    {
        mHeaderData.append(qMakePair(QString(),QString()));
        emit headerDataChanged(Qt::Horizontal, position, position);
    }

    endInsertColumns();

    return true;
}

bool TableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows - 1);

    for (int lRowIt = 0; lRowIt < rows; ++lRowIt)
    {
        mRowList.removeAt(position);
    }

    endRemoveRows();

    mRowCount -= rows;

    return true;
}

bool TableModel::removeColumns(int position, int columns,
                               const QModelIndex &parent)
{
    int lRowCount = rowCount();
    beginRemoveColumns(parent, position, position + columns - 1);

    for (int lRowIt = 0; lRowIt < lRowCount; ++lRowIt)
    {
        for (int lColIt = 0; lColIt < columns; ++lColIt)
        {
            mRowList[lRowIt].removeAt(position);
        }
    }

    for (int lColIt = 0; lColIt < columns; ++lColIt)
    {
        mHeaderData.removeAt(position);
        emit headerDataChanged(Qt::Horizontal, position, position);
    }

    endRemoveColumns();
    return true;
}

QStringList TableModel::headerData() const
{
    QStringList lHeaderData;
    for (int i = 0; i < mHeaderData.size(); ++i)
    {
        lHeaderData.append(mHeaderData.at(i).first);
    }
    return lHeaderData;
}

void TableModel::setRowCount(int rowCount)
{
    clear();
    beginInsertRows(QModelIndex(), 0, rowCount-1);
    QList<TableItem> lItems;
    int lColCount = columnCount();
    for (int column = 0; column < lColCount; ++column)
        lItems.append(TableItem());

    for (int lRowIt = 0; lRowIt < rowCount; ++lRowIt)
    {
        mRowList.insert(lRowIt, lItems);
    }
    endInsertRows();
    mRowCount = rowCount;
}

TableItem::ObjectType TableModel::type(const QModelIndex &index) const
{
    return mRowList[index.row()][index.column()].objectType();
}

void TableModel::reset()
{
    mRowList.clear();
    mRowCount = 0;
}

int TableModel::indexOfColumn(const QString &columnName, bool rawName/* = true*/) const
{
    if (rawName)
        return mRawHeaderIndex.value(columnName);
    else
        return mDecoractedHeaderIndex.value(columnName);
}

} // namespace Gex
} // namespace GS

