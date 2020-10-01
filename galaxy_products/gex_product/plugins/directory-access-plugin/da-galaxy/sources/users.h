#ifndef USERS_H
#define USERS_H

/*! \class Users
 * \brief
 *
 */

#include <QStringList>
#include <QDomNode>

#include "dir_access_base.h"

#define USER_NODE           "user"
#define USER_ID             "uid"
#define USER_NAME           "name"
#define USER_PASS           "password"
#define USER_EMAIL          "email"
#define USER_CREATION_DATE  "creation_date"

namespace GS
{
namespace DAPlugin
{
class Users : public UsersBase
{
    Q_OBJECT
public:
    /// \brief Constructor
    Users();
    /// \brief Destructor
    virtual ~Users();
    /// \brief return user attribute
    QString GetUserAttribute(const QString& userId, const QString& userAttribute, bool &ok);
    /// \brief return user attribute
    QStringList GetUserAttributes(const QString& userId, bool &ok);
    /// \brief update user attribute
    bool UpdateUserAttribute(const QString& userId, const QString& userAttribute, const QString& newValue);
    /// \brief add user
    bool Add(const QString& userId, const QString& userPass);
    /// \brief remove user
    bool Remove(const QString &userId);
    /// \brief true if user exists
    bool Exists(const QString &userId);
    /// \brief return user list
    QStringList GetUsersList();
    /// \brief return last error
    QString GetLastError();
    /// \brief return last changes
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    QStringList YmAdbV2SupportGetLastChanges();
    /// \brief clear last changes list
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    void YmAdbV2SupportClearLastChanges();

signals:
    /// \brief emitted when data upated
    void sUpdated();

public slots:
    /// \brief load data
    bool Load(const QDomNode& usersNode);

private:
    Q_DISABLE_COPY(Users);

    /// \brief return user node
    bool GetUserNode(const QString &userId, QDomNode &matchingNode);
    /// \brief return dom document
    QDomDocument GetDocNode(const QDomNode& node);

    QString         mLastError;                         ///< Holds last error
    QDomNode        mUsersNode;                         ///< Holds users data
    QStringList     mYmAdminDbV2SupportChanges;         ///< Holds last changes
                                                        ///has to be removed when no compatibility needed
};
} // END DAPlugin
} // END GS

#endif // USERS_H
