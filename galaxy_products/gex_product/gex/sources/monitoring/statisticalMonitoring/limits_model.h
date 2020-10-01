#ifndef LIMITSMODEL_H
#define LIMITSMODEL_H

#include <QAbstractItemModel>
#include <QString>

#include "common_widgets/table_model.h"

// defines columns order
#define SM_STAT_ENABLED_COL      0
#define SM_ITEM_TYPE_COL         1
#define SM_ITEM_NUM_COL          2
#define SM_ITEM_NAME_COL         3
#define SM_ITEM_UNIT_COL         4
#define SM_ITEM_CAT_COL          5
#define SM_SITE_NO_COL           6
#define SM_CRIT_LVL_COL          7
#define SM_STAT_NAME_COL         8
#define SM_LL_ENABLED_COL        9
#define SM_LL_COL                10
#define SM_HL_ENABLED_COL        11
#define SM_HL_COL                12
#define SM_ALGO_COL              13
#define SM_N_COL                 14
#define SM_MEAN_COL              15
#define SM_SIGMA_COL             16
#define SM_MEDIAN_COL            17
#define SM_Q1_COL                18
#define SM_Q3_COL                19
#define SM_PERCENTILE_N1_COL     20
#define SM_PERCENTILE_100_N1_COL 21
#define SM_COMP_REMOVED_COUNT_COL 22
#define SM_COMP_DATAPOINTS_COL   23
#define SM_ID_COL                24
#define SM_VERSION_ID_COL        25
#define SM_STAT_ID_COL           26
#define SM_STAT_RECOMPUTE_COL    27
#define SM_STAT_HAS_UNIT_COL     28

#define SM_LIMITS_MODEL_NUM_TOT_COL   29

#define SM_LIMITS_MODEL_NUM_SHOWN_COL 24


namespace GS
{
namespace Gex
{

class LimitsModel : public TableModel
{
    Q_OBJECT

public:
    /// \brief Constructor
    LimitsModel(const QMap<QString, QString> &supportedStats,
                const QList<QPair<QString, QString> > &headerData,
                QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    /// \brief return column name matching with column id
    static QString rawColumnName(int columnId);
    /// \brief return column name matching with column id
    static QString decoractedColumnName(int columnId);

    /// \brief true if the model has some limits to update
    bool hasLimitsToUpdate() const;
    /// \brief true if the model has disabled fields
    bool hasDisabledFields() const;
    /// \brief true if the model has manual limits
    bool hasManualLimits() const;
    /// \brief true if the model has manual limits
    bool hasNonManualLimitsToUpdate() const;

private:
    QMap<QString, QString> mSupportedStats; ///< Holds list of supported stats
};

} // namespace Gex
} // namespace GS

#endif // LIMITSMODEL_H
