#ifndef PRIVILEGES_ADMIN_FORM_H
#define PRIVILEGES_ADMIN_FORM_H

/*! \class PrivilegesAdminForm
 * \brief
 *
 */

#include <QWidget>

namespace Ui {
class PrivilegesAdminForm;
}


namespace GS
{
namespace DAPlugin
{

class Users;
class Groups;
class AppEntries;
class AccessModel;

class PrivilegesAdminForm : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    PrivilegesAdminForm(QWidget *parent = 0);
    /// \brief Desctructor
    virtual ~PrivilegesAdminForm();
    /// \brief set data
    bool SetData(Users* users, Groups* groups, AppEntries* appEntries);
    /// \brief set if gui is editable or not
    void SetEditable(bool isEditable);

signals:
    /// \brief emitted when privileges changed by user
    void sPrivilegesChanged();

public slots:
    /// \brief load privileges of user
    void LoadUserPrivileges(const QString& userId);
    /// \brief load privileges of group
    void LoadGroupPrivileges(const QString& groupId);

private:
    Q_DISABLE_COPY(PrivilegesAdminForm);

    Ui::PrivilegesAdminForm*    mUi;            ///< Holds UI
    Users*                      mUsers;         ///< Holds users data
    AccessModel*                mModel;         ///< Holds data model
    Groups*                     mGroups;        ///< Holds groups data
    AppEntries*                 mPrivileges;    ///< Holds application entries data
    bool                        mIsEditable;    ///< Holds true if gui is editable
};

} // END DAPlugin
} // END GS

#endif // PRIVILEGES_ADMIN_FORM_H
