#ifndef DIR_ACCESS_BASE_H
#define DIR_ACCESS_BASE_H

#include <QDateTime>
#include <QtPlugin>
#include <QMap>

class QString;
class QWidget;


namespace GS
{
namespace DAPlugin
{
class GroupsBase;
class UsersBase;
class ConnectorBase;
class AppEntriesBase;

enum AccessPrivileges
{
    NOACCESS = 0,
    READACCESS = 1,
    WRITEACCESS = 2
};

class DirAccessBase: public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    virtual ~DirAccessBase(){}
    /// \brief return plugin version
    virtual QString         Version() = 0;
    /// \brief return plugin name
    virtual QString         Name() = 0;
    /// \brief return pointer to users data
    virtual UsersBase*      GetUsers() = 0;
    /// \brief return pointer to groups data
    virtual GroupsBase*     GetGroups() = 0;
    /// \brief return pointer to connector
    virtual ConnectorBase*  GetConnector() = 0;
    /// \brief return pointer to application entris
    virtual AppEntriesBase* GetAppEntries() = 0;
    /// \brief return current user id
    virtual QString         GetCurrentUser() = 0;
    /// \brief open admin ui
    virtual bool            OpenAdministrationUi()   {return 0;}
    /// \brief saves data changes
    virtual bool            SaveChanges()           {return false;}
    /// \brief return last error
    virtual QString         GetLastError()          {return QString();}
    /// \brief return last changes
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    virtual QStringList     YmAdbV2SupportGetLastUsersChanges() = 0;
    /// \brief clear last changes list
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    virtual void            YmAdbV2SupportClearLastUsersChanges() = 0;

signals:
    /// \brief sent when connection state has changed connected/disconnected
    void sConnectionStateChanged(bool connectionIsOk);

};

class UsersBase : public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    virtual ~UsersBase(){}
    /// \brief user attribute value
    virtual QString GetUserAttribute(const QString& userId, const QString& userAttribute, bool &ok) = 0;
    /// \brief list of user attribute
    virtual QStringList GetUserAttributes(const QString& userId, bool &ok) = 0;
    /// \brief update user attribute
    virtual bool UpdateUserAttribute(const QString& userId, const QString& userAttribute, const QString& newValue) = 0;
    /// \brief add user
    virtual bool Add(const QString &userId, const QString &userPass) = 0;
    /// \brief remove user
    virtual bool Remove(const QString &userId) = 0;
    /// \brief true if user exists
    virtual bool Exists(const QString &userId) = 0;
    /// \brief return last error
    virtual QString GetLastError() = 0;
    /// \brief return user list
    virtual QStringList GetUsersList() = 0;
};

class GroupsBase : public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    virtual ~GroupsBase(){}
    /// \brief return group attribute value
    virtual QString GetGroupAttribute(const QString& groupId, const QString& groupAttribute, bool &ok) = 0;
    /// \brief return group attribute list
    virtual QStringList GetGroupAttributes(const QString& groupId, bool &ok) = 0;
    /// \brief update group attribbute
    virtual bool UpdateGroupAttribute(const QString& groupId, const QString& groupAttribute, const QString& newValue) = 0;
    /// \brief add group
    virtual bool Add(const QString &groupId) = 0;
    /// \brief remov group
    virtual bool Remove(const QString &groupId) = 0;
    /// \brief true if group exists
    virtual bool Exists(const QString &groupId) = 0;
    /// \brief add user to group
    virtual bool AddUser(const QString &groupId, const QString &userId) = 0;
    /// \brief remove user from group
    virtual bool RemoveUser(const QString &groupId, const QString &userId) = 0;
    /// \brief return last erro
    virtual QString GetLastError() = 0;
    /// \brief return group list
    virtual QStringList GetGroupsList() = 0;
    /// \brief return groups that contains user
    virtual QStringList GetGroupsList(const QString& userId) = 0;
    /// \brief return users of group
    virtual QStringList GetUsersList(const QString& groupId) = 0;
};

class ConnectorBase : public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    virtual ~ConnectorBase(){}
    /// \brief connect to da server
    virtual bool Connect(const QMap<QString, QString> &conParams) = 0;
    /// \brief reconnect without specifying parameter
    virtual bool Reconnect() = 0;
    /// \brief disconnect
    virtual bool Disconnect() = 0;
    /// \brief change connected user without deconnection
    virtual bool ChangeUser(const QMap<QString, QString> &parameters) = 0;
    /// \brief true if user is connected
    virtual bool IsConnected() = 0;
    /// \brief return last error
    virtual QString GetLastError() = 0;
    /// \brief return server date time
    virtual QDateTime GetServerDateTime() = 0;
    /// \brief return SQL Session Id
    virtual QString GetSessionId() = 0;
    /// \brief check if evrything is OK and try repair
    virtual bool ValidityCheck() = 0;
 };

class AppEntriesBase : public QObject
{
    Q_OBJECT
public:
    /// \brief Destructor
    virtual ~AppEntriesBase(){}
    /// \brief add entry
    virtual bool Add(const QString &parentPath, const QString &entryId) = 0;
    /// \brief move entry
    virtual bool Move(const QString &entryPath, const QString &newParentPath) = 0;
    /// \brief remove entry
    virtual bool Remove(const QString &entryPath) = 0;
    /// \brief update entry attribute
    virtual bool UpdateEntryAttribute(const QString &entryPath, const QString& entryAttribute, const QString& newValue) = 0;
    /// \brief add privileges to user
    virtual bool AddUserPrivilege(const QString &entryPath, const QString &userId, int privilege) = 0;
    /// \brief remove usr privilege
    virtual bool RemoveUserPrivilege(const QString &entryPath, const QString &userId, int privilege) = 0;
    /// \brief update user privileges
    virtual bool UpdateUserPrivilege(const QString &entryPath, const QString &userId, int privilege) = 0;
    /// \brief add group privileges
    virtual bool AddGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege) = 0;
    /// \brief remove group privileges
    virtual bool RemoveGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege) = 0;
    /// \brief update group privileges
    virtual bool UpdateGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege) = 0;
    /// \brief return user privileges
    virtual int GetUserPrivileges(const QString &entryPath, const QString &userId) = 0;
    /// \brief return group privileges
    virtual int GetGroupPrivileges(const QString &entryPath, const QString &groupId) = 0;
    /// \brief true if app entry exists
    virtual bool Exists(const QString &entryPath) = 0;
    /// \brief true if group has privilege
    virtual bool IsAllowedTo(const QString &entryPath, const QString &userId, int privilege) = 0;
    /// \brief return last error
    virtual QString GetLastError() const = 0;
};

} // END DAPlugin
} // END GS

Q_DECLARE_INTERFACE(GS::DAPlugin::DirAccessBase, "DirectoryAccess/1.0")

#endif // DIR_ACCESS_BASE_H
