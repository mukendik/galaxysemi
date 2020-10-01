#ifndef GROUPS_ADMIN_FORM_H
#define GROUPS_ADMIN_FORM_H

/*! \class GroupsAdminForm
 * \brief
 *
 */

#include <QWidget>

namespace Ui {
class GroupsAdminForm;
}

class QListWidgetItem;

namespace GS
{
namespace DAPlugin
{
class Groups;
class Users;
class Connector;

class GroupsAdminForm : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    GroupsAdminForm(Connector* conn, QWidget *parent = 0);
    /// \brief Destructor
    virtual ~GroupsAdminForm();
    /// \brief load data
    bool LoadData(Groups* groups, Users *users);
    /// \brief return selected group id
    QString GetSelectedGroup() const;
    /// \brief set if gui is editable or not
    void SetEditable(bool isEditable);
    /// \brief set if user is admin viewer or not
    void SetAdminViewer(bool isAdminViewer);
    /// \brief set if user is admin writer or not
    void SetAdminWriter(bool isAdminWriter);
    /// \brief select the group in ui
    void SelectGroup(const QString &groupId);

signals:
    /// \brief emitted when group selection change
    void sGroupSelectionChanged(const QString& groupId);
    /// \brief emitted group data is changed by user
    void sGroupsChanged();

private slots:
    /// \brief called when selected group change
    void OnGroupSelectionChanged();
    /// \brief called when attribute value changed
    void OnAttributeChanged();
    /// \brief open a dialog to create a group
    void OnCreateGroup();
    /// \brief remove the selected group
    void OnDeleteGroup();
    /// \brief add selected users to selected group
    void OnAddUsers();
    /// \brief remove selected user from selected group
    void OnRemoveUsers();

private:
    Q_DISABLE_COPY(GroupsAdminForm);

    /// \brief init widget
    bool Init();
    /// \brief update gui
    void UpdateUi();
    /// \brief clear all groups from the widget
    void ClearGroups();
    /// \brief clear group users widget
    void ClearUsers();
    /// \brief clear all users widget
    void ClearAllUsers();
    /// \brief clear group attributes
    void ClearAttributes();
    /// \brief load group list
    bool LoadGroupList();
    /// \brief load list of all users
    bool LoadAllUsers();
    /// \brief load list of group users
    bool LoadGroupUsers(const QString &groupId);
    /// \brief load list of group attributes
    bool LoadGroupAttributes(const QString& groupId);
    /// \brief return new group item
    QListWidgetItem* GetNewGroupItem(const QString& groupId);
    /// \brief return group item
    QListWidgetItem* GetGroupItem(const QString& groupId);

    Ui::GroupsAdminForm*    mUi;                    ///< Holds UI
    Users*                  mUsers;                 ///< Holds users data
    Groups*                 mGroups;                ///< Holds groups data
    Connector*              mConnector;             ///< Holds connector
    bool                    mIsEditable;            ///< Holds true if gui is editable
    bool                    mIsAdminViewer;         ///< Holds true if user connected has admin viewer privileges
    bool                    mIsAdminWriter;         ///< Holds true if user connected has admin writer privileges
    QString                 mCurrentGroupId;        ///< Holds selected group id
    QStringList             mEditableAttributes;    ///< Holds list of editable attribute of a group
};

} // END DAPlugin
} // END GS

#endif // GROUPS_ADMIN_FORM_H
