#ifndef ACCESS_ITEM_H
#define ACCESS_ITEM_H

/*! \class AccessItem
 * \brief holds items used in AccessModel class
 *
 */

#include <QStandardItem>
#include <QDomNode>
#include <QHash>

namespace GS
{
namespace DAPlugin
{

class AccessItem : public QStandardItem
{
public:
    /// \brief
    enum PrivilegesType
    {
        GROUP = 1,
        USER = 2
    };
    /// \brief Constructor
    AccessItem(QDomNode &node, int privilegesType, QString targetedId, int row, AccessItem *parent = 0);
    /// \brief Destructor
    virtual ~AccessItem();
    /// \brief return data value
    QVariant        Data(int column, int role=-1);
    /// \brief define data value
    bool            SetData(const QModelIndex & index, const QVariant & value);
    /// \brief return child (index i) of item
    AccessItem*     child(int i);
    /// \brief return parent item
    AccessItem*     parent();
    /// \brief return the number of children
    int             childCount() const;
    /// \brief return the row of the item
    int             row() const;

private:
    /// \brief return the domnode linked to the item
    QDomNode    GetNode() const;
    /// \brief return the id of user or group linked to the item
    QString     GetTargetedId() const;
    /// \brief  to know if the item define group or user privilege
    int         GetPrivilegesType() const;
    /// \brief recursive function to update node privileges according to checked column
    bool        SetNodePrivilege(AccessItem* item, int column);
    /// \brief Update node privileges according to unchecked column
    bool        UnsetNodePrivilege(AccessItem* item, int column);
    /// \brief return true if the privileges linked to the column is set
    int         HasNodePrivilege(AccessItem *item, int column);
    /// \brief return privilege flag link to column #
    int         GetPrivilegeFlag(int column);

    QHash<int,AccessItem*>  mChildItems;    ///< Holds child item
    PrivilegesType          mPrivilegesType;///< Holds privileges type (User or Group)
    AccessItem*             mParentItem;    ///< Holds parent item
    QDomNode                mDomNode;       ///< Holds linked dom node (entry node)
    QString                 mTargetedId;    ///< Holds id linked (Group or User)
    int                     mRowNumber;     ///< Holds the row in the model
};

} // END DAPlugin
} // END GS

#endif // ACCESS_ITEM_H
