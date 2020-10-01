#ifndef USERS_ADMIN_FORM_H
#define USERS_ADMIN_FORM_H

/*! \class UsersAdminForm
 * \brief
 *
 */

#include <QWidget>

namespace Ui {
class UsersAdminForm;
}

class QListWidgetItem;

namespace GS
{
namespace DAPlugin
{
class Users;
class Groups;
class Connector;

class UsersAdminForm : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    UsersAdminForm(Connector* conn, QWidget *parent = 0);
    /// \brief Desctructor
    virtual ~UsersAdminForm();
    /// \brief load data
    bool LoadData(Users *users, Groups *groups);
    /// \brief return selected user
    QString GetSelectedUser() const;
    /// \brief define if gui is editable
    void SetEditable(bool isEditable);
    /// \brief set if user is admin writer or not
    void SetAdminWriter(bool isAdminWriter);
    /// \brief set if user is admin viewer or not
    void SetAdminViewer(bool isAdminViewer);
    /// \brief select user in gui
    void SelectUser(const QString& userId);

signals:
    /// \brief emitted when user selected change
    void sUserSelectionChanged(const QString& userId);
    /// \brief emitted when user data are updated
    void sUsersUpdated();
    /// \brief emitted when group data are updated
    void sGroupsUpdated();
    /// \brief emitted when user want to see or not user privileges
    void sShowCustomPrivileges(bool show);

private slots:
    /// \brief load user data
    void OnUserSelectionChanged();
    /// \brief load attributes data
    void OnAttributeChanged();
    /// \brief open a dialog to create user
    void OnCreateUser();
    /// \brief remove selected user
    void OnDeleteUser();

private:
    Q_DISABLE_COPY(UsersAdminForm)

    /// \brief init gui
    bool Init();
    /// \brief load user list
    bool LoadUserList();
    /// \brief clear user list
    void ClearUsers();
    /// \brief clear user attributes
    void ClearAttributes();
    /// \brief load user attributes
    bool LoadUserAttributes(const QString& userId);
    /// \brief update ui
    void UpdateUi();
    /// \brief return new user item
    QListWidgetItem* GetNewUserItem(const QString& userId);
    /// \brief return item link to user id
    QListWidgetItem* GetUserItem(const QString& userId);

    Ui::UsersAdminForm* mUi;                    ///< Holds UI
    Users*              mUsers;                 ///< Holds users data
    Groups*             mGroups;                ///< Holds groups data
    Connector*          mConnector;             ///< Holds connector
    bool                mIsEditable;            ///< Holds true if editable
    bool                mIsAdminWriter;         ///< Holds true if user is admin writer
    bool                mIsAdminViewer;         ///< Holds true if user is admin viewer
    QString             mCurrentUserId;         ///< Holds current useer id
    QStringList         mEditableAttributes;    ///< Holds list editable attributes
    void ErrorMsgBox();
};

} // END DAPlugin
} // END GS

#endif // USERS_ADMIN_FORM_H
