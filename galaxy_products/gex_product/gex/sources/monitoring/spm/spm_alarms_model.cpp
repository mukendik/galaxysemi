#include "spm_alarms_model.h"

#include <QAbstractItemModel>
#include <QColor>

#include "statistical_monitoring_tables.h"

namespace GS
{
namespace Gex
{
SPMAlarmsModel::SPMAlarmsModel(const QStringList &headerData,
                               QObject *parent)
    : QAbstractTableModel(parent),
      mHeaderData(headerData)
{

}

bool SPMAlarmsModel::insertRows(int position,
                                int rows,
                                const QModelIndex &parent)
{
    int columns = columnCount();
    if (columns <= 0)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    for (int lRowIt = 0; lRowIt < rows; ++lRowIt)
    {
        QList<QString> lItems;
        for (int column = 0; column < columns; ++column)
            lItems.append(QString());
        mRowList.insert(position, lItems);
    }
    endInsertRows();

    return true;
}

bool SPMAlarmsModel::insertColumns(int position,
                                   int columns,
                                   const QModelIndex &parent)
{
    int lRows = rowCount();
    beginInsertColumns(parent, position, position + columns - 1);

    for (int lRowIt = 0; lRowIt < lRows; ++lRowIt)
    {
        for (int lColIt = position; lColIt < columns; ++lColIt)
        {
            mRowList[lRowIt].insert(position, QString());
        }
    }

    for (int lColIt = position; lColIt < columns; ++lColIt)
    {
        mHeaderData.append(QString());
        emit headerDataChanged(Qt::Horizontal, position, position);
    }

    endInsertColumns();

    return true;
}

bool SPMAlarmsModel::removeRows(int position, int rows,
                                const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows - 1);

    for (int lRowIt = 0; lRowIt < rows; ++lRowIt)
    {
        mRowList.removeAt(position);
    }

    endRemoveRows();
    return true;
}

bool SPMAlarmsModel::removeColumns(int position,
                                   int columns,
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

void SPMAlarmsModel::clear()
{
    this->beginResetModel();
    reset();
    this->endResetModel();
}

int SPMAlarmsModel::indexOfColumn(const QString &columnName) const
{
    for(int lIt = 0; lIt < mHeaderData.size(); ++lIt)
    {
        if (mHeaderData.at(lIt) == columnName)
        {
            return lIt;
        }
    }
    return -1;
}

void SPMAlarmsModel::reset()
{
    mRowList.clear();
}

int SPMAlarmsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return mRowList.size();
}

int SPMAlarmsModel::columnCount(const QModelIndex &/*parent*/) const
{
    return mHeaderData.size();
}

QVariant SPMAlarmsModel::data(const QModelIndex &index,
                              int role) const
{
    if (!index.isValid() || mRowList.isEmpty())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        // show "all" if site = -1
        if ((mHeaderData.at(index.column()) == SM_ALARM_SITE) &&
                (mRowList[index.row()][index.column()] == "-1"))
                return "all";
        // replace 1,2 by standard, critical
        else if (mHeaderData.at(index.column()) == SM_ALARM_CRIT)
        {
            if (mRowList[index.row()][index.column()] == "1")
                return "standard";
            else
                return "critical";
        }

        return mRowList[index.row()][index.column()];
    }
    else if (role == Qt::BackgroundRole)
    {
        if ((mHeaderData.at(index.column()) == SM_ALARM_LL) ||
                (mHeaderData.at(index.column()) == SM_ALARM_HL))
        {
            QColor lAlarmColor;
            if (mRowList[index.row()][indexOfColumn(SM_ALARM_CRIT)] == "1")
                lAlarmColor = QColor("#FFA500"); // orange
            else
                lAlarmColor = QColor(Qt::red);
            if (((mHeaderData.at(index.column()) == SM_ALARM_LL) &&
                    (mRowList[index.row()][indexOfColumn(SM_ALARM_VALUE)].toDouble() <
                     mRowList[index.row()][indexOfColumn(SM_ALARM_LL)].toDouble())) ||
                    ((mHeaderData.at(index.column()) == SM_ALARM_HL) &&
                     (mRowList[index.row()][indexOfColumn(SM_ALARM_VALUE)].toDouble() >
                      mRowList[index.row()][indexOfColumn(SM_ALARM_HL)].toDouble())))
            {
                return QVariant(lAlarmColor);
            }
        }
    }
    else if (role == Qt::InitialSortOrderRole)
    {
        QVariant lData = mRowList[index.row()][index.column()].data();
        // define sorting rules
        bool lOK = true;
        int lIntValue = mRowList[index.row()][index.column()].toInt(&lOK);
        if (lOK)
            return lIntValue;
        else
        {
            double lDouble = mRowList[index.row()][index.column()].toDouble(&lOK);
            if (lOK)
                return lDouble;
            else
                return mRowList[index.row()][index.column()];
        }
    }

    return QVariant();
}

bool SPMAlarmsModel::setData(const QModelIndex &index,
                             const QVariant &value,
                             int /*role*/)
{
    if (!index.isValid() || mRowList.isEmpty())
        return false;

    mRowList[index.row()][index.column()] = value.toString();

    return true;
}

QVariant SPMAlarmsModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    if (mHeaderData.isEmpty())
        return QVariant();

    if ((role == Qt::DisplayRole && orientation == Qt::Horizontal) &&
            (section < mHeaderData.size()))
    {
        return GetDecoratedAlarmName(mHeaderData.at(section));
    }
    else if ((role == Qt::UserRole && orientation == Qt::Horizontal) &&
             (section < mHeaderData.size()))
    {
        return mHeaderData.at(section);
    }

    return QVariant();
}


bool SPMAlarmsModel::setHeaderData(int section,
                                   Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
    if (((role != Qt::DisplayRole) ||
         (orientation != Qt::Horizontal)) ||
            (section >= mHeaderData.count()) ||
            mHeaderData.isEmpty())
        return false;

    mHeaderData[section] = value.toString();

    emit headerDataChanged(orientation, section, section);

    return true;
}

QString SPMAlarmsModel::GetDecoratedAlarmName(const QString &raw) const
{
    if (raw == SM_ALARM_PRODUCT)
        return SM_ALARM_PRODUCT_D;
    else if (raw == SM_ALARM_LOT)
        return SM_ALARM_LOT_D;
    else if (raw == SM_ALARM_SUBLOT)
        return SM_ALARM_SUBLOT_D;
    else if (raw == SM_ALARM_WAFER)
        return SM_ALARM_WAFER_D;
    else if (raw == SM_ALARM_SPLITLOT)
        return SM_ALARM_SPLITLOT_D;
    else if (raw == SM_ALARM_CRIT)
        return SM_ALARM_CRIT_D;
    else if (raw == SM_ALARM_TEST_NUM)
        return SM_ALARM_TEST_NUM_D;
    else if (raw == SM_ALARM_TEST_NAME)
        return SM_ALARM_TEST_NAME_D;
    else if (raw == SM_ALARM_SITE)
        return SM_ALARM_SITE_D;
    else if (raw == SM_ALARM_STAT)
        return SM_ALARM_STAT_D;
    else if (raw == SM_ALARM_EXEC_COUNT)
        return SM_ALARM_EXEC_COUNT_D;
    else if (raw == SM_ALARM_FAIL_COUNT)
        return SM_ALARM_FAIL_COUNT_D;
    else if (raw == SM_ALARM_LL)
        return SM_ALARM_LL_D;
    else if (raw == SM_ALARM_HL)
        return SM_ALARM_HL_D;
    else if (raw == SM_ALARM_VALUE)
        return SM_ALARM_VALUE_D;
    else if (raw == SM_ALARM_UNIT)
        return SM_ALARM_UNIT_D;
    else
        return "Unknown";
}

QVariant SPMAlarmsModel::GetValue(const StatMonAlarm &alarm,
                                  const QString &field)
{
    if (field == SM_ALARM_PRODUCT)
        return QVariant(alarm.productName);
    else if (field == SM_ALARM_LOT)
        return QVariant(alarm.lotId);
    else if (field == SM_ALARM_SUBLOT)
        return QVariant(alarm.sublotId);
    else if (field == SM_ALARM_WAFER)
        return QVariant(alarm.waferId);
    else if (field == SM_ALARM_SPLITLOT)
        return QVariant(alarm.splitlotId);
    else if (field == SM_ALARM_CRIT)
        return QVariant(alarm.criticityLevel);
    else if (field == SM_ALARM_TEST_NUM)
        return QVariant(alarm.monitoredItemNum);
    else if (field == SM_ALARM_TEST_NAME)
        return QVariant(alarm.monitoredItemName);
    else if (field == SM_ALARM_SITE)
        return QVariant(alarm.siteNum);
    else if (field == SM_ALARM_STAT)
        return QVariant(alarm.statName);
    else if (field == SM_ALARM_EXEC_COUNT)
        return QVariant(alarm.execCount);
    else if (field == SM_ALARM_FAIL_COUNT)
        return QVariant(alarm.failCount);
    else if (field == SM_ALARM_LL)
        return QVariant(alarm.lowLimit);
    else if (field == SM_ALARM_HL)
        return QVariant(alarm.highLimit);
    else if (field == SM_ALARM_VALUE)
        return QVariant(alarm.outlierValue);
    else if (field == SM_ALARM_UNIT)
        return QVariant(alarm.monitoredItemUnit);
    else
        return QVariant();
}

} // namespace Gex
} // namespace GS
