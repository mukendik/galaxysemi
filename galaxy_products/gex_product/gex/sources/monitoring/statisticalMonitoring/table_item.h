#ifndef TABLEITEM_H
#define TABLEITEM_H


#include <QStyledItemDelegate>

namespace GS
{
namespace Gex
{

class TableItem
{
public:
    enum ObjectType
    {
        NONE = 0,
        STD_LINEEDIT = 1,
        STD_CHECKBOX = 2,
        COMBOBOX_ALGO = 3,
        COMBOBOX_ENABLED = 4,
        LINEEDIT_CRIT = 5,
    };
    /// \brief Constructor
    TableItem(Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);

    virtual ~TableItem(){}

    /// \brief Return object data
    QVariant data(int role = Qt::UserRole) const;
    /// \brief Return object flags
    Qt::ItemFlags flags() const;
    /// \brief Return object type
    ObjectType objectType() const;
    /// \brief Set object
    void setData(const QVariant &data);
    /// \brief Set Flags
    void setFlags(const Qt::ItemFlags &flags);
    /// \brief Return Set object type
    void setObjectType(const ObjectType& objectType);

private:
    Qt::ItemFlags   mFlags;         ///< Holds flags
    QVariant        mData;          ///< Holds data
    ObjectType      mObjectType;    ///< Holds object type
};

} // namespace Gex
} // namespace GS

#endif // TABLEITEM_H
