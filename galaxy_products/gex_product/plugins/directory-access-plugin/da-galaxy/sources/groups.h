#ifndef GROUPS_H
#define GROUPS_H

/*! \class Groups
 * \brief
 *
 */

#include <QDomNode>

#include "dir_access_base.h"

#define GROUP_NODE     "group"
#define GROUP_ID       "uid"
#define GROUP_NAME     "name"
#define GROUP_DESC     "description"
#define GROUP_USERS    "users"
#define GROUP_USER     "user"
#define GROUP_USER_ID  "uid"

namespace GS
{
namespace DAPlugin
{
class Groups : public GroupsBase
{
    Q_OBJECT
public:
    /// \brief Constructor
    Groups();
    /// \brief Destructor
    virtual ~Groups();
    /// \brief return group attribute
    QString GetGroupAttribute(const QString& groupId, const QString& groupAttribute, bool &ok);
    /// \brief return group attribute
    QStringList GetGroupAttributes(const QString& groupId, bool &ok);
    /// \brief update grou pattribute
    bool UpdateGroupAttribute(const QString& groupId, const QString& groupAttribute, const QString& newValue);
    /// \brief add group
    bool Add(const QString &groupId);
    /// \brief remove group
    bool Remove(const QString &groupId);
    /// \brief add user to group
    bool AddUser(const QString &groupId, const QString &userId);
    /// \brief remove user from group
    bool RemoveUser(const QString &groupId, const QString &userId);
    /// \brief true if group exists
    bool Exists(const QString &groupId);
    /// \brief return last error
    QString GetLastError();
    /// \brief return list of groups
    QStringList GetGroupsList();
    /// \brief returns list of groups containing user
    QStringList GetGroupsList(const QString& userId);
    /// \brief return user list in group
    QStringList GetUsersList(const QString& groupId);

signals:
    /// \brief emitted when group is updated
    void sUpdated();

public slots:
    /// \brief Load data
    bool Load(const QDomNode& groupsNode);

private:
    Q_DISABLE_COPY(Groups);

    /// \brief return group node
    bool GetGroupNode(const QString &groupId, QDomNode &matchingNode);
    /// \brief return dom document node
    QDomDocument GetDocNode(const QDomNode& node);

    QString mLastError;     ///< Holds last error
    QDomNode mGroupsNode;   ///< Holds groups data

};
} // END DAPlugin
} // END GS

#endif // GROUPS_H
