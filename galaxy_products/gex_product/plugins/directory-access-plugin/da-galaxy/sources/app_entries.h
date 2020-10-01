#ifndef APP_ENTRIES_H
#define APP_ENTRIES_H

/*! \class AppEntries
 * \brief
 *
 */

#include <QDomNode>

#include "dir_access_base.h"

#define ENTRY_NODE              "entry"
#define ENTRY_ID                "uid"
#define ENTRY_NAME              "name"
#define ENTRY_DESC              "description"
#define ENTRY_USERS_ACCESS      "users_access"
#define ENTRY_GROUPS_ACCESS     "groups_access"
#define ENTRY_USER_NODE         "user"
#define ENTRY_USER_ID           "uid"
#define ENTRY_GROUP_NODE        "group"
#define ENTRY_GROUP_ID          "uid"
#define ENTRY_ACCESS            "access"

namespace GS
{
namespace DAPlugin
{

class AppEntries : public AppEntriesBase
{
    Q_OBJECT
public:
    /// \brief Constructor
    AppEntries();
    /// \brief Desctructor
    virtual ~AppEntries();
    // entries
    /// \brief add new entry
    bool Add(const QString &parentPath, const QString &entryId);
    /// \brief move entry
    bool Move(const QString &entryPath, const QString &newParentPath);
    /// \brief remove entry
    bool Remove(const QString &entryPath);
    /// \brief update entry attribute
    bool UpdateEntryAttribute(const QString &entryPath, const QString& entryAttribute, const QString& newValue);
    // Privileges
    /// \brief add privilege to user
    bool AddUserPrivilege(const QString &entryPath, const QString &userId, int privilege);
    /// \brief remove privilege to user
    bool RemoveUserPrivilege(const QString &entryPath, const QString &userId, int privilege);
    /// \brief update user privilege
    bool UpdateUserPrivilege(const QString &entryPath, const QString &userId, int privilege);
    /// \brief return user privileges
    int GetUserPrivileges(const QString &entryPath, const QString &userId);
    /// \brief return user privileges
    static int GetUserPrivileges(const QDomNode &entryNode, const QString &userId);
    /// \brief update user privileges
    static bool UpdateUserPrivilege(const QDomNode &entryNode, const QString &userId, int privilege);
    /// \brief add group privileges
    bool AddGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege);
    /// \brief remove group privileges
    bool RemoveGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege);
    /// \brief update group privileges
    bool UpdateGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege);
    /// \brief return group privileges
    int GetGroupPrivileges(const QString &entryPath, const QString &groupId);
    /// \brief return group privilges
    static int GetGroupPrivileges(const QDomNode &entryNode, const QString &groupId);
    /// \brief update group privileges
    static bool UpdateGroupPrivilege(const QDomNode &entryNode, const QString &groupId, int privilege);
    /// \brief true if user has privileges
    bool IsAllowedTo(const QString &entryPath, const QString &userId, int privilege);
    /// \brief return last error
    QString GetLastError() const;
    /// \brief retun entry node
    QDomNode GetEntriesNode();
    /// \brief true is entry exists
    bool Exists(const QString &entryPath);

signals:
    /// \brief emitted when data are updated
    void sUpdated();

public slots:
    /// \brief load data
    bool Load(const QDomNode& appEntriesNode);

private:
    Q_DISABLE_COPY(AppEntries);

    // entry
    /// \brief return entry node
    bool GetEntryNode(const QDomNode &startNode, const QString &entryPath, QDomNode &matchingNode) const;
    // user
    /// \brief return user node
    static bool GetUserNode(const QDomNode &usersNode, const QString &userId, QDomNode &matchingNode);
    /// \brief add user node
    static bool AddUserNode(QDomNode &usersNode, const QString &userId, QDomNode &userNode);
    /// \brief return user access node
    bool GetUserAccessNode(const QString &entryPath, const QString &userId, QDomNode &userAccessNode);
    // group
    /// \brief return group node
    static bool GetGroupNode(const QDomNode &groupsNode, const QString &groupId, QDomNode &matchingNode);
    /// \brief add group node
    static bool AddGroupNode(QDomNode &groupsNode, const QString &groupId, QDomNode &groupNode);
    /// \brief return grroup access node
    bool GetGroupAccessNode(const QString &entryPath, const QString &groupId, QDomNode &groupAccessNode);
    // Privileges
    /// \brief add privilege
    int AddPrivilege(int oldPrivilege, int privilegeToAdd);
    /// \brief remove privilege
    int RemovePrivilege(int oldPrivilege, int privilegeToRemove);
    // misc
    /// \brief return dom documeet
    static QDomDocument GetDocNode(const QDomNode& node);

    QString     mLastError;         ///< Holds last error
    QDomNode    mAppEntriesNode;    ///< Holds application entries data
    QString     mAppSeparator;      ///< Holds separator between entries
};

} // END DAPlugin
} // END GS

#endif // APP_ENTRIES_H
