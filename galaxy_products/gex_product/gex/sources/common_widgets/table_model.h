#ifndef TABLEMODEL_H
#define TABLEMODEL_H


#include <QAbstractItemModel>
#include <QString>

#include "table_item.h"

namespace GS
{
namespace Gex
{

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /// \brief Constructor
    TableModel(const QList<QPair<QString, QString> > &headerData, QObject *parent = 0);
    virtual ~TableModel(){}

    /// \brief implement QAbstractTableModel functions
    int             rowCount     (const QModelIndex &parent = QModelIndex()) const;
    int             columnCount  (const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags   flags        (const QModelIndex &index) const;
    bool            setFlags     (const QModelIndex &index, Qt::ItemFlags flags);
    QVariant        headerData   (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool            setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    bool            insertRows   (int position, int rows, const QModelIndex &parent = QModelIndex());
    bool            insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
    bool            removeRows   (int position, int rows, const QModelIndex &parent = QModelIndex());
    bool            removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());

    /// \brief return index of column in header
    int indexOfColumn(const QString& rawColumnName, bool rawName = true) const;
    /// \brief clear the model
    void clear ();
    /// \brief return headerData
    QStringList headerData() const;

    void setRowCount(int rowCount);
    TableItem::ObjectType type(const QModelIndex &index) const;

protected:

    void reset();
    virtual void resetSpecialized(){}

    QList<QList<TableItem > >            mRowList;              ///< Holds model data
    QList<QPair<QString, QString > >     mHeaderData;           ///< Holds model header data
    QMap<QString, int>                   mRawHeaderIndex;       ///< Holds header index
    QMap<QString, int>                   mDecoractedHeaderIndex;///< Holds header index
    int                                  mRowCount;             ///< Holds rows count
};

} // namespace Gex
} // namespace GS

#endif // TABLEMODEL_H
