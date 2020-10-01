
#include <QVariant>
#include <QInputDialog>
#include <QMessageBox>

#include "ui_users_admin_form.h"
#include "users_admin_form.h"
#include "connector.h"
#include "groups.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{

UsersAdminForm::UsersAdminForm(Connector *conn, QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::UsersAdminForm)
{
    mUi->setupUi(this);
    mCurrentUserId = "";
    mConnector = conn;
    mIsEditable = false;
    mIsAdminWriter = false;
    mIsAdminViewer = false;
    mUsers = 0;
    mGroups = 0;
    mUi->pushButtonDelete->setHidden(true); // hidden because no rule has been defined on how handled tasks of removed user


    mEditableAttributes << USER_EMAIL << USER_NAME << USER_PASS;
    UpdateUi();

    connect(mUi->pushButtonNew, SIGNAL(clicked()), this, SLOT(OnCreateUser()));
    connect(mUi->pushButtonDelete, SIGNAL(clicked()), this, SLOT(OnDeleteUser()));
    connect(mUi->checkBoxCustomPrivileges, SIGNAL(toggled(bool)), this, SIGNAL(sShowCustomPrivileges(bool)));
}

UsersAdminForm::~UsersAdminForm()
{
    ClearAttributes();
    ClearUsers();
    // do not delete mUsers
    delete mUi;
}

bool UsersAdminForm::LoadData(Users *users, Groups *groups)
{
    if (!users || !groups)
        return false;
    mUsers = users;
    mGroups = groups;

    return Init();
}

void UsersAdminForm::SetEditable(bool isEditable)
{
    mIsEditable = isEditable;
    UpdateUi();
}

void UsersAdminForm::SetAdminWriter(bool isAdminWriter)
{
    mIsAdminWriter = isAdminWriter;
    UpdateUi();
}

void UsersAdminForm::SetAdminViewer(bool isAdminViewer)
{
    mIsAdminViewer = isAdminViewer;
    UpdateUi();
}

void UsersAdminForm::SelectUser(const QString &userId)
{
    QListWidgetItem* lItem = GetUserItem(userId);
    if (lItem)
        mUi->listWidgetUsers->setCurrentItem(lItem);
}

bool UsersAdminForm::Init()
{
    // User list
    mUi->listWidgetUsers->setSelectionMode(QAbstractItemView::SingleSelection);

    mUi->listWidgetUsers->setStyleSheet(
                "QListView  {"
                    "show-decoration-selected: 1;"
                "}"
                "QListView::item:selected  {"
                    "border: 1px solid #69B2F6;"
                "}"
                "QListView::item:selected:!active  {"
                    "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                "stop: 0 #69B2F6, stop: 1 #50A5F5);"
                "}"
                "QListView::item:selected:active  {"
                    "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                "stop: 0 #69B2F6, stop: 1 #50A5F5);"
                "}"
                                        );

    // Attributes Table
    mUi->tableWidgetUserAttribute->setSelectionMode(QAbstractItemView::SingleSelection);
    mUi->tableWidgetUserAttribute->setColumnCount(2);
    mUi->tableWidgetUserAttribute->verticalHeader()->hide();
    mUi->tableWidgetUserAttribute->horizontalHeader()->show();
    LoadUserList();

    mUi->listWidgetUsers->setCurrentItem(mUi->listWidgetUsers->item(0));

    return true;
}

bool UsersAdminForm::LoadUserList()
{
    QStringList lUsers = mUsers->GetUsersList();

    int lUserCount = lUsers.count();

    ClearUsers();

    for (int i = 0; i < lUserCount; ++i)
    {
        QString lUserId = lUsers.at(i);
        if (!(mIsAdminWriter || mIsAdminViewer) && (mConnector->GetCurrentUser() != lUserId))
            continue;
        mUi->listWidgetUsers->insertItem(i, GetNewUserItem(lUserId));
    }

    mUi->listWidgetUsers->sortItems();

    connect(mUi->listWidgetUsers, SIGNAL(itemSelectionChanged()), this, SLOT(OnUserSelectionChanged()));

    return true;
}

void UsersAdminForm::ClearUsers()
{
    disconnect(mUi->listWidgetUsers, SIGNAL(itemSelectionChanged()), this, SLOT(OnUserSelectionChanged()));
    int lItemsCount = mUi->listWidgetUsers->count();
    for (int i = lItemsCount - 1; i >= 0; --i)
        delete mUi->listWidgetUsers->takeItem(i);

    mUi->listWidgetUsers->clear();
}

void UsersAdminForm::ClearAttributes()
{
    disconnect(mUi->tableWidgetUserAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));
    int lItemsCount = mUi->tableWidgetUserAttribute->rowCount();
    for (int i = lItemsCount - 1; i >= 0; --i)
    {
        delete mUi->tableWidgetUserAttribute->takeItem(i, 0);
        delete mUi->tableWidgetUserAttribute->takeItem(i, 1);
    }

    mUi->tableWidgetUserAttribute->clear();
}

bool UsersAdminForm::LoadUserAttributes(const QString &userId)
{
    bool ok = false;
    mCurrentUserId = userId;
    QStringList lUserAttributes = mUsers->GetUserAttributes(userId, ok);
    ClearAttributes();
    QStringList lHeaderValues;
    lHeaderValues << "Attribute" << "Value";
    mUi->tableWidgetUserAttribute->setHorizontalHeaderLabels(lHeaderValues);

    mUi->tableWidgetUserAttribute->setRowCount(lUserAttributes.count());

    for (int i = 0; i < lUserAttributes.count(); ++i)
    {
        QString lAttName = lUserAttributes.at(i);
        // Set attribute name
        QTableWidgetItem* lNewItemName = new QTableWidgetItem(lAttName);
        lNewItemName->setFlags(lNewItemName->flags () & ~Qt::ItemIsEditable);
        mUi->tableWidgetUserAttribute->setItem(i, 0, lNewItemName);
        // Set Attribute value
        QString lAttValue = mUsers->GetUserAttribute(userId, lUserAttributes.at(i), ok);
        if (lAttName == USER_PASS)
            lAttValue = "********";
        QTableWidgetItem* lNewItemValue = new QTableWidgetItem(lAttValue);
        if (!mEditableAttributes.contains(lAttName) || userId == "anonymous")
            lNewItemValue->setFlags(lNewItemValue->flags () & ~Qt::ItemIsEditable);
        mUi->tableWidgetUserAttribute->setItem(i, 1, lNewItemValue);
    }

    mUi->tableWidgetUserAttribute->resizeColumnsToContents();

    UpdateUi();

    connect(mUi->tableWidgetUserAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));

    return true;
}

QString UsersAdminForm::GetSelectedUser() const
{
    if (!mUi->listWidgetUsers->currentItem())
        return QString();

    QString lUserId = mUi->listWidgetUsers->currentItem()->data(Qt::DisplayRole).toString();

    return lUserId;
}

void UsersAdminForm::UpdateUi()
{
    mUi->pushButtonNew->setEnabled(mIsEditable && mIsAdminWriter);
    mUi->pushButtonDelete->setEnabled(mIsEditable && mIsAdminWriter);

    // lock attribute values
    if (mIsEditable)
    {
        mUi->tableWidgetUserAttribute->setEditTriggers(QAbstractItemView::AllEditTriggers);
        mUi->pushButtonNew->setToolTip("Create a new user");
        mUi->pushButtonDelete->setToolTip("Remove selected user");
    }
    else
    {
        mUi->tableWidgetUserAttribute->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mUi->pushButtonNew->setToolTip("Open an edit session with Users/Groups \n"
                                       "administrator privileges to enable this action");
        mUi->pushButtonDelete->setToolTip("Open an edit session with Users/Groups \n"
                                          "administrator privileges to enable this action");
    }
}

QListWidgetItem *UsersAdminForm::GetNewUserItem(const QString &userId)
{
    QListWidgetItem* lNewItem = new QListWidgetItem(userId);
    lNewItem->setFlags(lNewItem->flags () & ~Qt::ItemIsEditable);
    if (!mIsAdminWriter && (mConnector->GetCurrentUser() != userId))
        lNewItem->setForeground(Qt::gray);
    lNewItem->setIcon(QIcon(":/icons/yieldmandb_users.png"));
    return lNewItem;
}

QListWidgetItem *UsersAdminForm::GetUserItem(const QString &userId)
{
    QList<QListWidgetItem*> lItemsList = mUi->listWidgetUsers->findItems(userId, Qt::MatchFixedString);
    if (lItemsList.isEmpty())
        return 0;
    return lItemsList.at(0);
}

void UsersAdminForm::OnUserSelectionChanged()
{
    QString lUserId = GetSelectedUser();
    if (lUserId.isEmpty())
    {
        ClearAttributes();
        emit sUserSelectionChanged("");
        return;
    }

    LoadUserAttributes(lUserId);
    emit sUserSelectionChanged(lUserId);
}

void UsersAdminForm::OnAttributeChanged()
{
    int lChangedColumn = mUi->tableWidgetUserAttribute->currentColumn();
    if (lChangedColumn != 1)
        return;
    int lChangedRow = mUi->tableWidgetUserAttribute->currentRow();
    QString lAttribute = mUi->tableWidgetUserAttribute->item(lChangedRow, 0)->text();
    QString lNewValue = mUi->tableWidgetUserAttribute->item(lChangedRow, 1)->text();
    if (lAttribute == USER_PASS)
    {
        disconnect(mUi->tableWidgetUserAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));
        mUi->tableWidgetUserAttribute->item(lChangedRow, 1)->setData(Qt::DisplayRole, "********");
        connect(mUi->tableWidgetUserAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));
        if (mCurrentUserId == "anonymous")
        {
            QMessageBox lMsgBox;
            lMsgBox.setWindowTitle("Error");
            lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
            lMsgBox.setText("Password of anonymous user can not be changed!");
            lMsgBox.setStandardButtons(QMessageBox::Ok);
            lMsgBox.exec();
            return;
        }
        bool ok;
        QString lConfirmedPass = QInputDialog::getText(this, "Change password", "Please confirm password:", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            if (lConfirmedPass != lNewValue)
            {
                QMessageBox lMsgBox;
                lMsgBox.setWindowTitle("Error");
                lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                lMsgBox.setText("Second password does not match\nPassword not updated!");
                lMsgBox.setStandardButtons(QMessageBox::Ok);
                lMsgBox.exec();
                return;
            }
        }
    }
    if (lNewValue.toUtf8() != lNewValue.toLatin1())
    {
        ErrorMsgBox();
        return;
    }
    mUsers->UpdateUserAttribute(mCurrentUserId, lAttribute, lNewValue);
    mUi->tableWidgetUserAttribute->resizeColumnToContents(1);

    emit sUsersUpdated();
}

void UsersAdminForm::ErrorMsgBox()
{
    QMessageBox lMsgBox;
    lMsgBox.setWindowTitle("Error");
    lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    lMsgBox.setText("Login support ASCII characters only.");
    lMsgBox.setStandardButtons(QMessageBox::Ok);
    lMsgBox.exec();
}

void UsersAdminForm::OnCreateUser()
{
    bool ok;
    QString lUserId = QInputDialog::getText(this, "New user", "Define unique name:", QLineEdit::Normal, "username", &ok);
    if (ok && lUserId.toUtf8() != lUserId.toLatin1())
    {
        ErrorMsgBox();
        return;
    }
    if (ok && !lUserId.isEmpty())
    {
        if (!mUsers->Add(lUserId, "1234"))
            return;
        mUsers->UpdateUserAttribute(lUserId, USER_CREATION_DATE, mConnector->GetServerDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        mGroups->AddUser("public", lUserId);
        mUi->listWidgetUsers->insertItem(0, GetNewUserItem(lUserId));
        if (!GetUserItem(lUserId))
            return;
        mUi->listWidgetUsers->setCurrentItem(GetUserItem(lUserId));
        mUi->listWidgetUsers->setFocus();
        emit sGroupsUpdated();
        emit sUsersUpdated();
    }
}

void UsersAdminForm::OnDeleteUser()
{
    if (!mUi->listWidgetUsers->currentItem())
        return ;

    QString lUserId = mUi->listWidgetUsers->currentItem()->data(Qt::DisplayRole).toString();
    if (lUserId.isEmpty())
        return;

    QMessageBox lMsgBox;
    lMsgBox.setWindowTitle(" ");
    lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    if (lUserId == "admin" || lUserId == "anonymous")
    {
        lMsgBox.setText("It is not allowed to delete user: " + lUserId + "!");
        lMsgBox.setStandardButtons(QMessageBox::Ok);
        lMsgBox.exec();
        return;
    }

    lMsgBox.setText("Do you want to delete user: " + lUserId + "?");
    lMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    lMsgBox.setDefaultButton(QMessageBox::Ok);
    int lReturn = lMsgBox.exec();
    if (lReturn == QMessageBox::Ok)
    {
        mUsers->Remove(lUserId);

        LoadUserList();

        mUi->listWidgetUsers->setCurrentItem(mUi->listWidgetUsers->item(0));
    }
    emit sUsersUpdated();
}

} // END DAPlugin
} // END GS

