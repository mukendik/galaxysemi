#ifndef ADMINISTRATION_DIALOG_H
#define ADMINISTRATION_DIALOG_H

/*! \class AdministrationDialog
 * \brief
 *
 */

#include <QtWidgets/QDialog>

namespace Ui {
class AdministrationDialog;
}


namespace GS
{
namespace DAPlugin
{
class Users;
class Groups;
class Connector;
class AppEntries;
class UsersAdminForm;
class GroupsAdminForm;
class PrivilegesAdminForm;

class AdministrationDialog : public QDialog
{
    Q_OBJECT
public:
    /// \brief Constructor
    AdministrationDialog(Connector* conn, QWidget *parent = 0);
    /// \brief Destructor
    virtual ~AdministrationDialog();
    /// \brief set data
    bool SetData(Users* users, Groups* groups, AppEntries* appEntries);
    /// \brief select a user and his attributes in the user widget
    void SelectUser(const QString& userId);
    
signals:
    /// \brief emitted when a connection is requested by user
    void sConnectionRequested();
    /// \brief emitted when a save data is requested by user
    void sSaveRequested();
    /// \brief emitted when a discard changes is requested by user
    void sCancelRequested();
    /// \brief emitted when the user want to change stat of the session (open/close)
    void sChangeSessionStateRequested(bool openSession);

public slots:
    /// \brief called when data have been updated by someone else
    /// reload data
    void OnInputUpdated();
    /// \brief called when connection is opened or closed
    /// reload data if opened
    void OnConnectionStateChanged(bool connectionIsOk);
    /// \brief called when user change data
    void OnManualChanges();
    /// \brief called when want to open or close session
    void OnChangeSessionStateRequested();
    /// \brief called when session state change
    void OnSessionStateChanged(bool sessionIsOpened);
    /// \brief called when session of somebody else changed
    void OnOtherSessionStateChanged(QString sessionId);

private slots:
    /// \brief load data Group/User
    bool LoadData();
    /// \brief load Group data
    void LoadGroupsData();
    /// \brief load user data
    void LoadUsersData();
    /// \brief Cancel changes made by user before last save
    void OnDiscardRequested();
    /// \brief save last changes
    void OnSaveRequested();
    /// \brief load user privileges when user selected change
    void OnUserSelectionChanged(const QString& userId);
    /// \brief load group privileges when user selected change
    void OnGroupSelectionChanged(const QString& groupId);

protected:
    /// \brief over load qt dialog close event to manage choice if unsaved changes have been done
    void closeEvent(QCloseEvent * event);

private:
    Q_DISABLE_COPY(AdministrationDialog);
    /// \brief update UI according to all parameter
    void UpdateUi();
    /// \brief update buttons according to all parameter
    void UpdateButtons();
    /// \brief load privileges of connected user
    bool LoadUserPrivileges();

    Ui::AdministrationDialog*   mUi;                    ///< Holds UI
    PrivilegesAdminForm*        mUserPrivilegesForm;    ///< Holds user privileges form
    PrivilegesAdminForm*        mGroupPrivilegesForm;   ///< Holds group privileges form
    GroupsAdminForm*            mGroupsForm;            ///< Holds group form
    UsersAdminForm*             mUsersForm;             ///< Holds user form
    AppEntries*                 mAppEntries;            ///< Holds application entries data
    Connector*                  mConnector;             ///< Holds connector
    Groups*                     mGroups;                ///< Holds groups data
    Users*                      mUsers;                 ///< Holds users data
    bool                        mEditSessionOpened;     ///< Holds true if edit session opened
    bool                        mIsEditable;            ///< Holds true if gui is editable
    bool                        mAreUnsavedChanges;     ///< Holds true if user has no unsaved changes
    bool                        mConnectionIsOk;        ///< Holds true if connection state is good
    bool                        mIsAdminViewer;         ///< Holds true if connected user has admin read privileges
    bool                        mIsAdminWriter;         ///< Holds true if connected user has admin write privileges
    QString                     mOtherSessionOpened;    ///< Holds id of other session opened
};

} // END DAPlugin
} // END GS


#endif // ADMINISTRATION_DIALOG_H
