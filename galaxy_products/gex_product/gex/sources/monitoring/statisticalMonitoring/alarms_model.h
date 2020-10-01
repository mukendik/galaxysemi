#ifndef ALARMSMODEL_H
#define ALARMSMODEL_H

#include "common_widgets/table_model.h"

#define SM_ALARM_PRODUCT_COL       0
#define SM_ALARM_LOT_COL           1
#define SM_ALARM_SUBLOT_COL        2
#define SM_ALARM_WAFER_COL         3
#define SM_ALARM_SPLITLOT_COL      4
#define SM_ALARM_CRIT_COL          5
#define SM_ALARM_TEST_NUM_COL      6
#define SM_ALARM_TEST_NAME_COL     7
#define SM_ALARM_SITE_COL          8
#define SM_ALARM_STAT_COL          9
#define SM_ALARM_EXEC_COUNT_COL    10
#define SM_ALARM_FAIL_COUNT_COL    11
#define SM_ALARM_LL_COL            12
#define SM_ALARM_HL_COL            13
#define SM_ALARM_VALUE_COL         14
#define SM_ALARM_UNIT_COL          15

#define SM_ALARMS_MODEL_NUM_TOT_COL     16
#define SM_ALARMS_MODEL_NUM_SHOWN_COL   16

class StatMonAlarm;

namespace GS
{
namespace Gex
{

class AlarmsModel : public TableModel
{
    Q_OBJECT

public:
    /// \brief Constructor
    AlarmsModel(QList<QPair<QString, QString> > &headerData, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    /// \brief return column name matching with column id
    static QString rawColumnName(int columnId);
    /// \brief return column name matching with column id
    static QString decoractedColumnName(int columnId);
    /// \brief return field value extracted from alarm object
    static QVariant GetValue(const StatMonAlarm &alarm, const QString &field);
protected:
    static QString GetDecoratedAlarmName(const QString &raw);
};

} // namespace Gex
} // namespace GS

#endif // ALARMSMODEL_H
