#ifndef ACCESS_MODEL_H
#define ACCESS_MODEL_H

/*! \class AccessModel
 * \brief Model that holds application entries arborescence
 *
 */

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QDomNode>
#include <QVariant>


namespace GS
{
namespace DAPlugin
{

class AccessItem;

class AccessModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    /// \brief Constructor
    AccessModel(QDomNode &appEntriesNode, int privilegesType, QString targetedId, QObject *parent = 0);
    /// \brief Destructor
    virtual ~AccessModel();
    /// \brief Define if model is editable or not
    void            SetEditable(bool isEditable);
    /// \brief return data
    QVariant        data(const QModelIndex &index, int role) const;
    /// \brief set data
    bool            setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    /// \brief return flag
    Qt::ItemFlags   flags(const QModelIndex &index) const;
    /// \brief return headerdata
    QVariant        headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    /// \brief return index
    QModelIndex     index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    /// \brief return parent of index
    QModelIndex     parent(const QModelIndex &child) const;
    /// \brief return row count
    int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    /// \brief return column count
    int             columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    Q_DISABLE_COPY(AccessModel);

    QDomNode    mAppEntries;    ///< Holds application entries DomNode
    AccessItem* mRootItem;      ///< Hold root item
    bool        mIsEditable;    ///< true if model is editable
};

} // END DAPlugin
} // END GS

#endif // ACCESS_MODEL_H
