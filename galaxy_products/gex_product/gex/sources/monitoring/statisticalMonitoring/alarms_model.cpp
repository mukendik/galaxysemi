
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_tables.h"
#include "alarms_model.h"


namespace GS
{
namespace Gex
{

AlarmsModel::AlarmsModel(QList<QPair<QString, QString> > &headerData, QObject *parent)
    : TableModel(headerData, parent)
{

}

QVariant AlarmsModel::data(const QModelIndex &index,
                              int role) const
{
    if (!index.isValid() || mRowList.isEmpty())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        // show "all" if site = -1
        if ((index.column() == SM_ALARM_SITE_COL) &&
                (mRowList[index.row()][index.column()].data().toInt() == -1))
                return "all";
        // replace 1,2 by standard, critical
        else if (index.column()== SM_ALARM_CRIT_COL)
        {
            if (mRowList[index.row()][index.column()].data().toInt() == 1)
                return "standard";
            else
                return "critical";
        }

        return mRowList[index.row()][index.column()].data();
    }
    else if (role == Qt::BackgroundRole)
    {
        if ((index.column() == SM_ALARM_LL_COL) ||
                (index.column() == SM_ALARM_HL_COL))
        {
            QColor lAlarmColor;
            if (mRowList[index.row()][SM_ALARM_CRIT_COL].data().toInt() == 1)
                lAlarmColor = QColor("#FFA500"); // orange
            else
                lAlarmColor = QColor(Qt::red);
            if (((index.column() == SM_ALARM_LL_COL) &&
                    (mRowList[index.row()][SM_ALARM_VALUE_COL].data().toDouble() <
                     mRowList[index.row()][SM_ALARM_LL_COL].data().toDouble())) ||
                    ((index.column() == SM_ALARM_HL_COL) &&
                     (mRowList[index.row()][SM_ALARM_VALUE_COL].data().toDouble() >
                      mRowList[index.row()][SM_ALARM_HL_COL].data().toDouble())))
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
        int lIntValue = mRowList[index.row()][index.column()].data().toInt(&lOK);
        if (lOK)
            return lIntValue;
        else
        {
            double lDouble = mRowList[index.row()][index.column()].data().toDouble(&lOK);
            if (lOK)
                return lDouble;
            else
                return mRowList[index.row()][index.column()].data();
        }
    }

    return QVariant();
}

bool AlarmsModel::setData(const QModelIndex &index,
                             const QVariant &value,
                             int /*role*/)
{
    if (!index.isValid() || mRowList.isEmpty())
        return false;

    mRowList[index.row()][index.column()].setData(value);

    return true;
}

QString AlarmsModel::rawColumnName(int columnId)
{
    QString lColumnName;
    switch (columnId)
    {
        case SM_ALARM_PRODUCT_COL:
        lColumnName = SM_ALARM_PRODUCT;
        break;
        case SM_ALARM_LOT_COL:
        lColumnName = SM_ALARM_LOT;
        break;
        case SM_ALARM_SUBLOT_COL:
        lColumnName = SM_ALARM_SUBLOT;
        break;
        case SM_ALARM_WAFER_COL:
        lColumnName = SM_ALARM_WAFER;
        break;
        case SM_ALARM_SPLITLOT_COL:
        lColumnName = SM_ALARM_SPLITLOT;
        break;
        case SM_ALARM_CRIT_COL:
        lColumnName = SM_ALARM_CRIT;
        break;
        case SM_ALARM_TEST_NUM_COL:
        lColumnName = SM_ALARM_TEST_NUM;
        break;
        case SM_ALARM_TEST_NAME_COL:
        lColumnName = SM_ALARM_TEST_NAME;
        break;
        case SM_ALARM_SITE_COL:
        lColumnName = SM_ALARM_SITE;
        break;
        case SM_ALARM_STAT_COL:
        lColumnName = SM_ALARM_STAT;
        break;
        case SM_ALARM_EXEC_COUNT_COL:
        lColumnName = SM_ALARM_EXEC_COUNT;
        break;
        case SM_ALARM_FAIL_COUNT_COL:
        lColumnName = SM_ALARM_FAIL_COUNT;
        break;
        case SM_ALARM_LL_COL:
        lColumnName = SM_ALARM_LL;
        break;
        case SM_ALARM_HL_COL:
        lColumnName = SM_ALARM_HL;
        break;
        case SM_ALARM_VALUE_COL:
        lColumnName = SM_ALARM_VALUE;
        break;
        case SM_ALARM_UNIT_COL:
        lColumnName = SM_ALARM_UNIT;
        break;
        default:
        lColumnName = "unknown";
        break;
    }

    return lColumnName;
}

QString AlarmsModel::decoractedColumnName(int columnId)
{
    return GetDecoratedAlarmName(rawColumnName(columnId));
}

QString AlarmsModel::GetDecoratedAlarmName(const QString &raw)
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

QVariant AlarmsModel::GetValue(const StatMonAlarm &alarm,
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

