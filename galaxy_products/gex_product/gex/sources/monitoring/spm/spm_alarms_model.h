#ifndef SPMALARMSMODEL_H
#define SPMALARMSMODEL_H

#include <QAbstractItemModel>

#include "statistical_monitoring_alarm_struct.h"

namespace GS
{
namespace Gex
{

class SPMAlarmsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SPMAlarmsModel(const QStringList &headerData, QObject *parent = 0);

    // implement QAbstractTableModel functions
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());

    /// \brief clear model content
    void clear();
    /// \brief return index of column in header
    int indexOfColumn(const QString& columnName) const;

    static QVariant GetValue(const StatMonAlarm &alarm, const QString &field);
protected:
    QString GetDecoratedAlarmName(const QString &raw) const;
private:
    /// \brief reset the model
    void reset();

    QList<QList<QString > >     mRowList;      ///< Holds model data
    QStringList                 mHeaderData;   ///< Holds model header data
};

} // namespace Gex
} // namespace GS

#endif // SPMALARMSMODEL_H
