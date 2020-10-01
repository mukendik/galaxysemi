
#include "statistical_monitoring_tables.h"
#include "gex_constants.h"
#include "gexdb_plugin_base.h"

#include "engine.h"
#include "limits_model.h"

namespace GS
{
namespace Gex
{

LimitsModel::LimitsModel(const QMap<QString, QString> & supportedStats,
                         const QList<QPair<QString, QString> >& headerData,
                         QObject *parent) :
    TableModel(headerData, parent),
    mSupportedStats(supportedStats)
{

}

QVariant LimitsModel::data(const QModelIndex &index, int role) const
{
    if ((role == Qt::DisplayRole) || (role == Qt::EditRole) /*|| (role == Qt::UserRole)*/)
    {
        // customize site column
        if (SM_SITE_NO_COL == index.column())
        {
            if (mRowList[index.row()][index.column()].data(role).toInt() == -1)
                return QVariant("all");
        }
        else if (SM_STAT_HAS_UNIT_COL == index.column())
        {
            if (mRowList[index.row()][index.column()].data(role).toInt() == 0)
                return QVariant("n/a");
        }
        else if (SM_STAT_NAME_COL == index.column())
        {
            return mSupportedStats.value(mRowList[index.row()][index.column()].data(role).toString());
        }
        if (((SM_HL_COL == index.column()) ||
             (SM_LL_COL == index.column())) &&
                mRowList[index.row()][index.column()].data(role).toDouble() == GEX_C_DOUBLE_NAN)
            return QVariant("n/a");
        return mRowList[index.row()][index.column()].data(role);
    }
    else if (role == Qt::UserRole)
    {
        QVariant lData = mRowList[index.row()][index.column()].data();
        // define sorting rules
        switch (lData.type())
        {
        case QMetaType::Int:
            return lData.toInt();
            break;
        case QMetaType::Double:
            return lData.toDouble();
            break;
        default:
            return lData;
            break;
        }
    }
    else if (role == Qt::InitialSortOrderRole)
    {
        QVariant lData = mRowList[index.row()][index.column()].data();
        // define sorting rules
        bool lOK = true;
        int lIntValue = lData.toInt(&lOK);
        if (lOK)
            return lIntValue;
        else
        {
            double lDouble = lData.toDouble(&lOK);
            if (lOK)
                return lDouble;
            else
                return lData;
        }
    }
    else if (role == Qt::BackgroundRole)
    {
        // if disabled set it to lightgrey
        if (mRowList[index.row()][SM_STAT_ENABLED_COL].data().toInt() == 0)
        {
            return QVariant(QColor(Qt::lightGray));
        }
        // if has to be recomputed set yellow
        else if (mRowList[index.row()][SM_STAT_RECOMPUTE_COL].data().toInt() == 1)
        {
            return QVariant(QColor(Qt::yellow));
        }
        // if user updated set it to green
        else if (mRowList[index.row()][SM_STAT_RECOMPUTE_COL].data().toInt() == 2)
        {
            return QVariant(QColor("#81ffa2")); // whatif green
        }

        return QVariant();
    }
    else
        return QVariant();
}

bool LimitsModel::setData(const QModelIndex &index,
                             const QVariant &value,
                             int role/* = Qt::EditRole*/)
{
    if (role == Qt::CheckStateRole)
    {
        if ((Qt::CheckState)value.toInt() == Qt::Checked)
        {
            mRowList[index.row()][index.column()].setData((Qt::CheckState)value.toInt());
        }
    }
    else if (role == Qt::EditRole)
    {
        // do not update if same data
        if (mRowList[index.row()][index.column()].data(role) == value)
        {
            return true;
        }
        else if (index.column() == SM_ALGO_COL)
        {
            if (mRowList[index.row()][index.column()].data(Qt::UserRole) == value)
                return true;
        }
        // make sure LL < HL or do not save
        else if (index.column() == SM_LL_COL)
        {
            bool lOkLL, lOkHL;
            double lLL = value.toDouble(&lOkLL);
            double lHL = mRowList[index.row()][SM_HL_COL].data(Qt::UserRole).toDouble(&lOkHL);
            if (lOkLL == false) // wrong format, ignore value
                return true;
            if (lOkHL && (lLL > lHL))  // HL lower than LL, ignore value
                return true;
        }
        else if (index.column() == SM_HL_COL)
        {
            bool lOkLL, lOkHL;
            double lHL = value.toDouble(&lOkHL);
            double lLL = mRowList[index.row()][SM_LL_COL].data(Qt::UserRole).toDouble(&lOkLL);
            if (lOkHL == false) // wrong format, ignore value
                return true;
            if (lOkLL && (lLL > lHL)) // HL lower than LL, ignore value
                return true;
        }

        mRowList[index.row()][index.column()].setData(value);
    }
    else if (role == Qt::DecorationRole)
    {
        mRowList[index.row()][index.column()].setObjectType((TableItem::ObjectType)value.toInt());
    }

    emit dataChanged(index, index);
    return true;
}

QString LimitsModel::rawColumnName(int columnId)
{
    QString lColumnName;
    switch (columnId)
    {
        case SM_ITEM_TYPE_COL:
        lColumnName = SM_MONITORED_ITEM_TYPE;
        break;
        case SM_STAT_ENABLED_COL:
        lColumnName = SM_ENABLED;
        break;
        case SM_ITEM_NUM_COL:
        lColumnName = SM_MONITORED_ITEM_NUM;
        break;
        case SM_ITEM_NAME_COL:
        lColumnName = SM_MONITORED_ITEM_NAME;
        break;
        case SM_ITEM_UNIT_COL:
        lColumnName = SM_MONITORED_ITEM_UNIT;
        break;
        case SM_ITEM_CAT_COL:
        lColumnName = SM_MONITORED_ITEM_CAT;
        break;
        case SM_SITE_NO_COL:
        lColumnName = SM_SITE_NO;
        break;
        case SM_CRIT_LVL_COL:
        lColumnName = SM_CRIT_LVL;
        break;
        case SM_STAT_NAME_COL:
        lColumnName = SM_STAT_NAME;
        break;
        case SM_LL_ENABLED_COL:
        lColumnName = SM_LL_ENABLED;
        break;
        case SM_LL_COL:
        lColumnName = SM_LL;
        break;
        case SM_HL_ENABLED_COL:
        lColumnName = SM_HL_ENABLED;
        break;
        case SM_HL_COL:
        lColumnName = SM_HL;
        break;
        case SM_ALGO_COL:
        lColumnName = SM_ALGO;
        break;
        case SM_N_COL:
        lColumnName = SM_N;
        break;
        case SM_MEAN_COL:
        lColumnName = SM_MEAN;
        break;
        case SM_SIGMA_COL:
        lColumnName = SM_SIGMA;
        break;
        case SM_MEDIAN_COL:
        lColumnName = SM_MEDIAN;
        break;
        case SM_Q1_COL:
        lColumnName = SM_Q1;
        break;
        case SM_Q3_COL:
        lColumnName = SM_Q3;
        break;
        case SM_PERCENTILE_N1_COL:
        lColumnName = SM_NPERCENT;
        break;
        case SM_PERCENTILE_100_N1_COL:
        lColumnName = SM_100MINUSNPERCENT;
        break;
        case SM_COMP_REMOVED_COUNT_COL:
        lColumnName = SM_COMP_OUTLIERS;
        break;
        case SM_COMP_DATAPOINTS_COL:
        lColumnName = SM_COMP_DATAPOINTS;
        break;
        case SM_ID_COL:
        lColumnName = SM_ID;
        break;
        case SM_VERSION_ID_COL:
        lColumnName = SM_VERSION_ID;
        break;
        case SM_STAT_ID_COL:
        lColumnName = SM_LIMIT_ID;
        break;
        case SM_STAT_RECOMPUTE_COL:
        lColumnName = SM_RECOMPUTE;
        break;
        case SM_STAT_HAS_UNIT_COL:
        lColumnName = SM_HAS_UNIT;
        break;
        default:
        lColumnName = "unknown";
        break;
    }
    return lColumnName;
}

QString LimitsModel::decoractedColumnName(int columnId)
{
    QString lColumnName;
    switch (columnId)
    {
        case SM_STAT_ENABLED_COL:
        lColumnName = SM_ENABLED_D;
        break;
        case SM_ITEM_TYPE_COL:
        lColumnName = SM_MONITORED_ITEM_TYPE_D;
        break;
        case SM_ITEM_NUM_COL:
        lColumnName = SM_MONITORED_ITEM_NUM_D;
        break;
        case SM_ITEM_NAME_COL:
        lColumnName = SM_MONITORED_ITEM_NAME_D;
        break;
        case SM_ITEM_UNIT_COL:
        lColumnName = SM_MONITORED_ITEM_UNIT_D;
        break;
        case SM_ITEM_CAT_COL:
        lColumnName = SM_MONITORED_ITEM_CAT_D;
        break;
        case SM_SITE_NO_COL:
        lColumnName = SM_SITE_NO_D;
        break;
        case SM_CRIT_LVL_COL:
        lColumnName = SM_CRIT_LVL_D;
        break;
        case SM_STAT_NAME_COL:
        lColumnName = SM_STAT_NAME_D;
        break;
        case SM_LL_ENABLED_COL:
        lColumnName = SM_LL_ENABLED_D;
        break;
        case SM_LL_COL:
        lColumnName = SM_LL_D;
        break;
        case SM_HL_ENABLED_COL:
        lColumnName = SM_HL_ENABLED_D;
        break;
        case SM_HL_COL:
        lColumnName = SM_HL_D;
        break;
        case SM_ALGO_COL:
        lColumnName = SM_ALGO_D;
        break;
        case SM_N_COL:
        lColumnName = SM_N_D;
        break;
        case SM_MEAN_COL:
        lColumnName = SM_MEAN_D;
        break;
        case SM_SIGMA_COL:
        lColumnName = SM_SIGMA_D;
        break;
        case SM_MEDIAN_COL:
        lColumnName = SM_MEDIAN_D;
        break;
        case SM_Q1_COL:
        lColumnName = SM_Q1_D;
        break;
        case SM_Q3_COL:
        lColumnName = SM_Q3_D;
        break;
        case SM_PERCENTILE_N1_COL:
        lColumnName = SM_NPERCENT_D;
        break;
        case SM_PERCENTILE_100_N1_COL:
        lColumnName = SM_100MINUSNPERCENT_D;
        break;
        case SM_COMP_REMOVED_COUNT_COL:
        lColumnName = SM_COMP_REMOVED_COUNT_D;
        break;
        case SM_COMP_DATAPOINTS_COL:
        lColumnName = SM_COMP_DATAPOINTS_D;
        break;

        default:
        lColumnName = "unknown";
        break;
    }
    return lColumnName;
}


bool LimitsModel::hasLimitsToUpdate() const
{
    for (int lRowIt = 0; lRowIt < mRowCount; ++lRowIt)
    {
        if (mRowList[lRowIt].at(SM_STAT_RECOMPUTE_COL).data().toInt() == 1)
        {
            return true;
        }
    }

    return false;
}

bool LimitsModel::hasDisabledFields() const
{
    for (int lRowIt = 0; lRowIt < mRowCount; ++lRowIt)
    {
        if ((mRowList[lRowIt].at(SM_STAT_ENABLED_COL).data().toInt() == 0) ||
                (mRowList[lRowIt].at(SM_LL_ENABLED_COL).data().toInt() == 0) ||
                (mRowList[lRowIt].at(SM_HL_ENABLED_COL).data().toInt() == 0))
        {
            return true;
        }
    }

    return false;
}

bool LimitsModel::hasManualLimits() const
{
    for (int lRowIt = 0; lRowIt < mRowCount; ++lRowIt)
    {
        if (mRowList[lRowIt].at(SM_ALGO_COL).data() == C_OUTLIERRULE_MANUAL)
        {
            return true;
        }
    }

    return false;
}

bool LimitsModel::hasNonManualLimitsToUpdate() const
{
    for (int lRowIt = 0; lRowIt < mRowCount; ++lRowIt)
    {
        if ((mRowList[lRowIt].at(SM_ALGO_COL).data() != C_OUTLIERRULE_MANUAL) &&
                (mRowList[lRowIt].at(SM_STAT_RECOMPUTE_COL).data().toInt() == 1))
        {
            return true;
        }
    }

    return false;
}


} // namespace Gex
} // namespace GS
