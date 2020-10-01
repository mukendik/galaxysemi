
#include <QSplitter>
#include <QMessageBox>
#include <QCloseEvent>

#include "ui_administration_dialog.h"
#include "administration_dialog.h"
#include "privileges_admin_form.h"
#include "groups_admin_form.h"
#include "users_admin_form.h"
#include "app_entries.h"
#include "connector.h"
#include "groups.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{

AdministrationDialog::AdministrationDialog(Connector *conn, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    mUi(new Ui::AdministrationDialog)
{
    mUi->setupUi(this);
    setModal(true);

    mUsers = 0;
    mGroups = 0;
    mAppEntries = 0;
    mUserPrivilegesForm = 0;
    mGroupPrivilegesForm = 0;
    mGroupsForm = 0;
    mUsersForm = 0;
    mAreUnsavedChanges = false;
    mConnector = conn;
    mEditSessionOpened = false;
    mIsAdminViewer = false;
    mIsAdminWriter = false;
    mOtherSessionOpened = "";
    mConnectionIsOk = mConnector->IsConnected();

    QSplitter *lUserSplitter = new QSplitter();
    lUserSplitter->setOrientation(Qt::Vertical);
    mUsersForm = new UsersAdminForm(mConnector, this);
    lUserSplitter->addWidget(mUsersForm);
    mUserPrivilegesForm = new PrivilegesAdminForm(this);
    mUserPrivilegesForm->setHidden(true);
    lUserSplitter->addWidget(mUserPrivilegesForm);
    lUserSplitter->setStretchFactor(0,1);
    lUserSplitter->setStretchFactor(1,1);
    lUserSplitter->setCollapsible(0, false);
    lUserSplitter->setCollapsible(1, false);
    mUi->frameUsers->layout()->addWidget(lUserSplitter);

    QSplitter *lGroupSplitter = new QSplitter();
    lGroupSplitter->setOrientation(Qt::Vertical);
    mGroupsForm = new GroupsAdminForm(mConnector, this);
    lGroupSplitter->addWidget(mGroupsForm);
    mGroupPrivilegesForm = new PrivilegesAdminForm(this);
    lGroupSplitter->addWidget(mGroupPrivilegesForm);
    lGroupSplitter->setStretchFactor(0,1);
    lGroupSplitter->setStretchFactor(1,1);
    lGroupSplitter->setCollapsible(0, false);
    lGroupSplitter->setCollapsible(1, false);
    mUi->frameGroups->layout()->addWidget(lGroupSplitter);

    mUi->tabWidget->setCurrentIndex(0);

//    connect(mUi->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(LoadData()));
    connect(mUi->pushButtonConnect, SIGNAL(clicked()), this, SIGNAL(sConnectionRequested()));
    connect(mUi->pushButtonSave, SIGNAL(clicked()), this, SLOT(OnSaveRequested()));
    connect(mUi->pushButtonCancel, SIGNAL(clicked()), this, SLOT(OnDiscardRequested()));
    connect(mUi->pushButtonEditSession, SIGNAL(clicked()), this, SLOT(OnChangeSessionStateRequested()));
    connect(mUi->pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));

    connect(mGroupPrivilegesForm, SIGNAL(sPrivilegesChanged()), this, SLOT(OnManualChanges()));
    connect(mGroupsForm, SIGNAL(sGroupsChanged()), this, SLOT(OnManualChanges()));

    connect(mUserPrivilegesForm, SIGNAL(sPrivilegesChanged()), this, SLOT(OnManualChanges()));
    connect(mUsersForm, SIGNAL(sUsersUpdated()), this, SLOT(OnManualChanges()));
    connect(mUsersForm, SIGNAL(sGroupsUpdated()), this, SLOT(LoadGroupsData()));
    connect(mUsersForm, SIGNAL(sShowCustomPrivileges(bool)), mUserPrivilegesForm, SLOT(setVisible(bool)));
    setWindowTitle("Users/Groups and Privileges Manager");
    setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
}

AdministrationDialog::~AdministrationDialog()
{
    // do not delete mUsers;
    // do not delete mGroups;
    // do not delete mAppEntries;
    // do not delete mConnector;

    delete mUsersForm;
    delete mGroupsForm;
    delete mUserPrivilegesForm;
    delete mGroupPrivilegesForm;
    delete mUi;
}

bool AdministrationDialog::SetData(Users *users, Groups *groups, AppEntries *appEntries)
{
    if (!users || !groups || !appEntries)
        return false;

    mUsers = users;
    mGroups = groups;
    mAppEntries = appEntries;
    mAreUnsavedChanges = false;

    LoadData();

    UpdateUi();

    return true;
}

void AdministrationDialog::SelectUser(const QString &userId)
{
    mUsersForm->SelectUser(userId);
}

void AdministrationDialog::OnInputUpdated()
{
    LoadData();

    UpdateUi();
}

void AdministrationDialog::OnConnectionStateChanged(bool connectionIsOk)
{
    mConnectionIsOk = connectionIsOk;
    mAreUnsavedChanges = false;

    if (mConnectionIsOk)
        LoadData();

    UpdateUi();
}

void AdministrationDialog::OnManualChanges()
{
    mAreUnsavedChanges = true;
    UpdateUi();
}

void AdministrationDialog::OnChangeSessionStateRequested()
{
    emit sChangeSessionStateRequested(!mEditSessionOpened);
}

void AdministrationDialog::OnSessionStateChanged(bool sessionIsOpened)
{
    mEditSessionOpened = sessionIsOpened;
    UpdateUi();
}

void AdministrationDialog::OnOtherSessionStateChanged(QString sessionId)
{
    mOtherSessionOpened = sessionId;
    UpdateUi();
}

bool AdministrationDialog::LoadData()
{
    if (!LoadUserPrivileges())
        return false;

    LoadUsersData();

    LoadGroupsData();

    return true;
}

void AdministrationDialog::LoadGroupsData()
{
    QString lSelectedGroupId = mGroupsForm->GetSelectedGroup();
    // Load groups widgets
    disconnect(mGroupsForm, SIGNAL(sGroupSelectionChanged(QString)), this, SLOT(OnGroupSelectionChanged(QString)));
    mGroupsForm->SetAdminViewer(mIsAdminViewer);
    mGroupsForm->SetAdminWriter(mIsAdminWriter);
    mGroupsForm->LoadData(mGroups, mUsers);
    mGroupPrivilegesForm->SetData(mUsers, mGroups, mAppEntries);
    if (!lSelectedGroupId.isEmpty())
        mGroupsForm->SelectGroup(lSelectedGroupId);
    OnGroupSelectionChanged(mGroupsForm->GetSelectedGroup());
    connect(mGroupsForm, SIGNAL(sGroupSelectionChanged(QString)), this, SLOT(OnGroupSelectionChanged(QString)));
}

void AdministrationDialog::LoadUsersData()
{
    QString lSelectedUserId = mUsersForm->GetSelectedUser();
    // Load users widgets
    disconnect(mUsersForm, SIGNAL(sUserSelectionChanged(QString)), this, SLOT(OnUserSelectionChanged(QString)));
    mUsersForm->SetAdminViewer(mIsAdminViewer);
    mUsersForm->SetAdminWriter(mIsAdminWriter);
    mUsersForm->LoadData(mUsers, mGroups);
    mUserPrivilegesForm->SetData(mUsers, mGroups, mAppEntries);
    if (!lSelectedUserId.isEmpty())
        mUsersForm->SelectUser(lSelectedUserId);
    OnUserSelectionChanged(mUsersForm->GetSelectedUser());
    connect(mUsersForm, SIGNAL(sUserSelectionChanged(QString)), this, SLOT(OnUserSelectionChanged(QString)));
}

void AdministrationDialog::OnDiscardRequested()
{
    mAreUnsavedChanges = false;
    UpdateUi();
    emit sCancelRequested();
}

void AdministrationDialog::OnSaveRequested()
{
    mAreUnsavedChanges = false;
    UpdateUi();
    emit sSaveRequested();
}

void AdministrationDialog::OnUserSelectionChanged(const QString &userId)
{
    if (userId.isEmpty())
        return;

    mUserPrivilegesForm->LoadUserPrivileges(userId);
    UpdateUi();
}

void AdministrationDialog::OnGroupSelectionChanged(const QString &groupId)
{
    if (groupId.isEmpty())
        return;

    mGroupPrivilegesForm->LoadGroupPrivileges(groupId);
    UpdateUi();
}

void AdministrationDialog::closeEvent(QCloseEvent *event)
{
    if (!mAreUnsavedChanges)
    {
        QDialog::closeEvent(event);
    }
    else
    {
        QMessageBox lMsgBox;
        lMsgBox.setWindowTitle("Save");
        lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        lMsgBox.setText("Save changes?");
        lMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);
        lMsgBox.setDefaultButton(QMessageBox::Yes);
        int lResult = lMsgBox.exec();
        switch (lResult)
        {
        case QMessageBox::Yes:
            OnSaveRequested();
            event->accept();
            break;
        case QMessageBox::Discard:
            OnDiscardRequested();
            event->accept();
            break;
        case QMessageBox::Cancel:
            event->ignore();
            return;
        default:
            QDialog::closeEvent(event);
            break;
        }
    }
    emit sChangeSessionStateRequested(false);
}

void AdministrationDialog::UpdateUi()
{
    mUi->tabWidget->setEnabled(mConnectionIsOk);

    if (!mUsersForm)
        return;

    mUi->labelWarning->setText("edit session opened by " + mOtherSessionOpened.section(":", 0, 0) +
                               " from " + mOtherSessionOpened.section(":", 2, 2));
    mUi->labelWarning->setHidden(mOtherSessionOpened.isEmpty());

    QString lUserId = mUsersForm->GetSelectedUser();
    bool lIsUserConnectedSelected = (mConnector->GetCurrentUser() == lUserId);

    mUsersForm->SetEditable((lIsUserConnectedSelected || mIsAdminWriter) && mEditSessionOpened);
    mUserPrivilegesForm->SetEditable(mIsAdminWriter && mEditSessionOpened && (lUserId != "anonymous"));

    mGroupsForm->SetEditable(mIsAdminWriter && mEditSessionOpened);
    mGroupPrivilegesForm->SetEditable(mIsAdminWriter && mEditSessionOpened);

    if (!mConnectionIsOk)
        mUi->labelConnectionState->setText("disconnected");
    else
        mUi->labelConnectionState->setText(QString("connected as %1").arg(mConnector->GetCurrentUser()));

    UpdateButtons();
}

void AdministrationDialog::UpdateButtons()
{
    // Connect button
    mUi->pushButtonConnect->setHidden(mConnectionIsOk);
    mUi->pushButtonConnect->setToolTip("Connect to Users/Groups directory access");

    // Save/discard buttons
    // clicability
    mUi->pushButtonCancel->setEnabled(mConnectionIsOk && mAreUnsavedChanges);
    mUi->pushButtonCancel->setToolTip("Cancel last changes");

    mUi->pushButtonSave->setEnabled(mConnectionIsOk && mAreUnsavedChanges);
    mUi->pushButtonSave->setToolTip("Save last changes");
    // visibility
    mUi->pushButtonCancel->setVisible(mConnectionIsOk && mEditSessionOpened);
    mUi->pushButtonSave->setVisible(mConnectionIsOk && mEditSessionOpened);

    // Edit session button
    mUi->pushButtonEditSession->setVisible(mConnectionIsOk);
    mUi->pushButtonEditSession->setDisabled(mAreUnsavedChanges || !mOtherSessionOpened.isEmpty());
    if (mEditSessionOpened)
    {
        mUi->pushButtonEditSession->setText("Release the Edit Session");
        mUi->pushButtonEditSession->setToolTip("Release the edit session to allow other users to edit data\n"
                                               "last changes have to be saved or canceled");
    }
    else
    {
        mUi->pushButtonEditSession->setText("Open an Edit Session");
        mUi->pushButtonEditSession->setToolTip("Open an edit session, during this session other users\n"
                                               "will not be able to edit data");
    }
}

bool AdministrationDialog::LoadUserPrivileges()
{
    if (!mAppEntries)
        return false;
    mIsAdminViewer = false;
    mIsAdminWriter = false;
    QString lAdminEntry = "galaxy:users_groups_administrator";
    if (mAppEntries->IsAllowedTo(lAdminEntry, mConnector->GetCurrentUser(), READACCESS))
        mIsAdminViewer = true;
    if (mAppEntries->IsAllowedTo(lAdminEntry, mConnector->GetCurrentUser(), WRITEACCESS))
        mIsAdminWriter = true;

    return true;
}

} // END DAPlugin
} // END GS

