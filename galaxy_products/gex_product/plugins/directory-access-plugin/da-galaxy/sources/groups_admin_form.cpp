
#include <QInputDialog>
#include <QMessageBox>

#include "ui_groups_admin_form.h"
#include "groups_admin_form.h"
#include "connector.h"
#include "groups.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{

GroupsAdminForm::GroupsAdminForm(Connector *conn, QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::GroupsAdminForm)
{
    mUi->setupUi(this);
    mCurrentGroupId = "";
    mGroups = 0;
    mUsers = 0;

    mIsEditable = false;
    mConnector = conn;
    mEditableAttributes << GROUP_NAME << GROUP_DESC;
    mIsAdminWriter = false;
    mIsAdminViewer = false;

    UpdateUi();

    connect(mUi->pushButtonNew, SIGNAL(clicked()), this, SLOT(OnCreateGroup()));
    connect(mUi->pushButtonDelete, SIGNAL(clicked()), this, SLOT(OnDeleteGroup()));
    connect(mUi->pushButtonAddUsers, SIGNAL(clicked()), this, SLOT(OnAddUsers()));
    connect(mUi->pushButtonRemoveUsers, SIGNAL(clicked()), this, SLOT(OnRemoveUsers()));
}

GroupsAdminForm::~GroupsAdminForm()
{
    ClearAttributes();
    ClearGroups();
    ClearUsers();
    ClearAllUsers();
    // do not delete mGroups
    delete mUi;
}

bool GroupsAdminForm::LoadData(Groups *groups, Users *users)
{
    if (!groups || !users)
        return false;
    mGroups = groups;
    mUsers = users;
    return Init();
}

QString GroupsAdminForm::GetSelectedGroup() const
{
    if (mUi->listWidgetGroups->currentItem())
        return mUi->listWidgetGroups->currentItem()->data(Qt::DisplayRole).toString();
    else
        return QString();
}

void GroupsAdminForm::SetEditable(bool isEditable)
{
    mIsEditable = isEditable;
    UpdateUi();
}

void GroupsAdminForm::SetAdminViewer(bool isAdminViewer)
{
    mIsAdminViewer = isAdminViewer;
    UpdateUi();
}

void GroupsAdminForm::SetAdminWriter(bool isAdminWriter)
{
    mIsAdminWriter = isAdminWriter;
    UpdateUi();
}

void GroupsAdminForm::SelectGroup(const QString &groupId)
{
    QListWidgetItem* lItem = GetGroupItem(groupId);
    if (lItem)
        mUi->listWidgetGroups->setCurrentItem(lItem);
}

bool GroupsAdminForm::Init()
{
    // Group list
    mUi->listWidgetGroups->setSelectionMode(QAbstractItemView::SingleSelection);
    mUi->listWidgetGroups->setStyleSheet(
                "QListView  {"
                    "show-decoration-selected: 1; "
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
    mUi->tableWidgetGroupAttribute->setSelectionMode(QAbstractItemView::SingleSelection);
    mUi->tableWidgetGroupAttribute->setColumnCount(2);
    // Users list
    mUi->listWidgetGroupUsers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // All users list
    mUi->listWidgetAllUsers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mUi->tableWidgetGroupAttribute->verticalHeader()->hide();
    mUi->tableWidgetGroupAttribute->horizontalHeader()->show();
    LoadGroupList();

    mUi->listWidgetGroups->setCurrentItem(mUi->listWidgetGroups->item(0));

    return true;
}

void GroupsAdminForm::UpdateUi()
{
    mUi->pushButtonNew->setEnabled(mIsEditable);
    mUi->pushButtonDelete->setEnabled(mIsEditable);
    mUi->pushButtonAddUsers->setEnabled(mIsEditable);
    mUi->pushButtonRemoveUsers->setEnabled(mIsEditable);

    // lock attribute values
    if (!mIsEditable)
    {
        mUi->tableWidgetGroupAttribute->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mUi->pushButtonNew->setToolTip("Open an edit session with Users/Groups \n"
                                       "administrator privileges to enable this action");
        mUi->pushButtonDelete->setToolTip("Open an edit session with Users/Groups \n"
                                          "administrator privileges to enable this action");
        mUi->pushButtonAddUsers->setToolTip("Open an edit session with Users/Groups \n"
                                            "administrator privileges to enable this action");
        mUi->pushButtonRemoveUsers->setToolTip("Open an edit session with Users/Groups \n"
                                               "administrator privileges to enable this action");
    }
    else
    {
        mUi->tableWidgetGroupAttribute->setEditTriggers(QAbstractItemView::AllEditTriggers);
        mUi->pushButtonNew->setToolTip("Create a new group");
        mUi->pushButtonDelete->setToolTip("Remove selected group");
        mUi->pushButtonAddUsers->setToolTip("Add selected user(s) to the current group");
        mUi->pushButtonRemoveUsers->setToolTip("Remove selected user(s) from the current group");
    }
}

bool GroupsAdminForm::LoadGroupList()
{
    QStringList lGroups = mGroups->GetGroupsList();
    int lGroupCount = lGroups.count();
    ClearGroups();
    for (int i = 0; i < lGroupCount; ++i)
    {
        QString lGroupId = lGroups.at(i);
        if (!(mIsAdminWriter || mIsAdminViewer) &&
                !mGroups->GetUsersList(lGroupId).contains(mConnector->GetCurrentUser()))
            continue;
        mUi->listWidgetGroups->insertItem(i, GetNewGroupItem(lGroupId));
    }
    mUi->listWidgetGroups->sortItems();

    connect(mUi->listWidgetGroups, SIGNAL(itemSelectionChanged()), this, SLOT(OnGroupSelectionChanged()));

    return true;
}

bool GroupsAdminForm::LoadAllUsers()
{
    QStringList lUsers = mUsers->GetUsersList();
    int lUserCount = lUsers.count();
    ClearAllUsers();
    for (int i = 0; i < lUserCount; ++i)
    {
        QListWidgetItem* lNewItem = new QListWidgetItem(lUsers.at(i));
        lNewItem->setFlags(lNewItem->flags () & ~Qt::ItemIsEditable);
        lNewItem->setIcon(QIcon(":/icons/yieldmandb_users.png"));
        mUi->listWidgetAllUsers->insertItem(i, lNewItem);
    }
    mUi->listWidgetAllUsers->sortItems();

    return true;
}

bool GroupsAdminForm::LoadGroupUsers(const QString& groupId)
{
    QStringList lUsers = mGroups->GetUsersList(groupId);
    int lUsersCount = lUsers.count();
    ClearUsers();
    for (int i = 0; i < lUsersCount; ++i)
    {
        QListWidgetItem* lNewItem = new QListWidgetItem(lUsers.at(i));
        lNewItem->setFlags(lNewItem->flags () & ~Qt::ItemIsEditable);
        lNewItem->setIcon(QIcon(":/icons/yieldmandb_users.png"));
        mUi->listWidgetGroupUsers->insertItem(i, lNewItem);
    }
    mUi->listWidgetGroupUsers->sortItems();

    return true;
}

void GroupsAdminForm::ClearGroups()
{
    disconnect(mUi->listWidgetGroups, SIGNAL(itemSelectionChanged()), this, SLOT(OnGroupSelectionChanged()));
    int lItemsCount = mUi->listWidgetGroups->count();
    for (int i = lItemsCount - 1; i >= 0; --i)
        delete mUi->listWidgetGroups->takeItem(i);
    mUi->listWidgetGroups->clear();
}

void GroupsAdminForm::ClearUsers()
{
    int lItemsCount = mUi->listWidgetGroupUsers->count();
    for (int i = lItemsCount - 1; i >= 0; --i)
        delete mUi->listWidgetGroupUsers->takeItem(i);
    mUi->listWidgetGroupUsers->clear();
}

void GroupsAdminForm::ClearAllUsers()
{
    int lItemsCount = mUi->listWidgetAllUsers->count();
    for (int i = lItemsCount - 1; i >= 0; --i)
        delete mUi->listWidgetAllUsers->takeItem(i);
    mUi->listWidgetAllUsers->clear();
}

void GroupsAdminForm::ClearAttributes()
{
    disconnect(mUi->tableWidgetGroupAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));
    int lItemsCount = mUi->tableWidgetGroupAttribute->rowCount();
    for (int i = lItemsCount - 1; i >= 0; --i)
    {
        delete mUi->tableWidgetGroupAttribute->takeItem(i, 0);
        delete mUi->tableWidgetGroupAttribute->takeItem(i, 1);
    }
    mUi->tableWidgetGroupAttribute->clear();
}

bool GroupsAdminForm::LoadGroupAttributes(const QString &groupId)
{
    bool lOk = false;
    mCurrentGroupId = groupId;
    QStringList lGroupAttributes = mGroups->GetGroupAttributes(groupId, lOk);
    ClearAttributes();
    QStringList lHeaderValues;
    lHeaderValues << "Attribute" << "Value";
    mUi->tableWidgetGroupAttribute->setHorizontalHeaderLabels(lHeaderValues);
    mUi->tableWidgetGroupAttribute->setRowCount(lGroupAttributes.count() - 1); // do not inclue users nod
    for (int i = 0; i < lGroupAttributes.count(); ++i)
    {
        if (lGroupAttributes.at(i) != GROUP_USERS)
        {
            QString lAttName = lGroupAttributes.at(i);
            QTableWidgetItem* lNewItemName = new QTableWidgetItem(lAttName);
            lNewItemName->setFlags(lNewItemName->flags () & ~Qt::ItemIsEditable);
            mUi->tableWidgetGroupAttribute->setItem(i, 0, lNewItemName);
            QTableWidgetItem* lNewItemValue = new QTableWidgetItem(mGroups->GetGroupAttribute(groupId, lAttName, lOk));
            if (!mEditableAttributes.contains(lAttName))
                lNewItemValue->setFlags(lNewItemValue->flags () & ~Qt::ItemIsEditable);
            mUi->tableWidgetGroupAttribute->setItem(i, 1, lNewItemValue);
        }
    }
    mUi->tableWidgetGroupAttribute->resizeColumnsToContents();
    connect(mUi->tableWidgetGroupAttribute, SIGNAL(cellChanged(int,int)), this, SLOT(OnAttributeChanged()));

    return true;
}

QListWidgetItem *GroupsAdminForm::GetNewGroupItem(const QString &groupId)
{
    QListWidgetItem* lNewItem = new QListWidgetItem(groupId);
    lNewItem->setFlags(lNewItem->flags () & ~Qt::ItemIsEditable);
    lNewItem->setIcon(QIcon(":/icons/yieldmandb_groups.png"));
    return lNewItem;
}

QListWidgetItem *GroupsAdminForm::GetGroupItem(const QString &groupId)
{
    QList<QListWidgetItem* > lItemsList = mUi->listWidgetGroups->findItems(groupId, Qt::MatchFixedString);
    if (lItemsList.isEmpty())
        return 0;
    return lItemsList.at(0);
}


void GroupsAdminForm::OnGroupSelectionChanged()
{
    if (!mUi->listWidgetGroups->currentItem())
    {
        ClearUsers();
        ClearAllUsers();
        ClearAttributes();
        emit sGroupSelectionChanged("");
        return;
    }
    QString lGroupId = mUi->listWidgetGroups->currentItem()->data(Qt::DisplayRole).toString();
    LoadGroupAttributes(lGroupId);
    LoadGroupUsers(lGroupId);
    LoadAllUsers();
    emit sGroupSelectionChanged(lGroupId);
}

void GroupsAdminForm::OnAttributeChanged()
{
    int lChangedColumn = mUi->tableWidgetGroupAttribute->currentColumn();
    if (lChangedColumn != 1)
        return;
    int lChangedRow = mUi->tableWidgetGroupAttribute->currentRow();
    QString lAttribute = mUi->tableWidgetGroupAttribute->item(lChangedRow, 0)->text();
    QString lNewValue = mUi->tableWidgetGroupAttribute->item(lChangedRow, 1)->text();
    mGroups->UpdateGroupAttribute(mCurrentGroupId, lAttribute, lNewValue);
    mUi->tableWidgetGroupAttribute->resizeColumnToContents(1);
    emit sGroupsChanged();
}

void GroupsAdminForm::OnCreateGroup()
{
    bool ok;
    QString lGroupId = QInputDialog::getText
            (this, "New group", "Define unique name:", QLineEdit::Normal, "groupname", &ok);
    if (ok && lGroupId.toUtf8() != lGroupId.toLatin1())
    {
        QMessageBox lMsgBox;
        lMsgBox.setWindowTitle("Error");
        lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        lMsgBox.setText("Login support ASCII characters only.");
        lMsgBox.setStandardButtons(QMessageBox::Ok);
        lMsgBox.exec();
        return;
    }
    if (ok && !lGroupId.isEmpty())
    {
        if (!mGroups->Add(lGroupId))
            return;
        mUi->listWidgetGroups->insertItem(0, GetNewGroupItem(lGroupId));
        if (!GetGroupItem(lGroupId))
            return;
        mUi->listWidgetGroups->setCurrentItem(GetGroupItem(lGroupId));
        mUi->listWidgetGroups->setFocus();
    }
    emit sGroupsChanged();
}

void GroupsAdminForm::OnDeleteGroup()
{
    if (!mUi->listWidgetGroups->currentItem())
        return ;
    QString lGroupId = mUi->listWidgetGroups->currentItem()->data(Qt::DisplayRole).toString();
    if (lGroupId.isEmpty())
        return;

    QMessageBox lMsgBox;
    lMsgBox.setWindowTitle(" ");
    lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    if (lGroupId == "public")
    {
        lMsgBox.setText("It is not allowed to delete group: " + lGroupId + "!");
        lMsgBox.setStandardButtons(QMessageBox::Ok);
        lMsgBox.exec();
        return;
    }

    lMsgBox.setText("Do you want to delete group: " + lGroupId + "?");
    lMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    lMsgBox.setDefaultButton(QMessageBox::Ok);
    int lReturn = lMsgBox.exec();
    if (lReturn == QMessageBox::Ok)
    {
        mGroups->Remove(lGroupId);
        LoadGroupList();
        mUi->listWidgetGroups->setCurrentItem(mUi->listWidgetGroups->item(0));
    }
    emit sGroupsChanged();
}

void GroupsAdminForm::OnAddUsers()
{
    QList<QListWidgetItem*> lSelectedItems = mUi->listWidgetAllUsers->selectedItems();

    for (int i = 0; i < lSelectedItems.count(); ++i)
    {
        QString lUserId = lSelectedItems.at(i)->data(Qt::DisplayRole).toString();
        if (lUserId == "anonymous")
        {
            QMessageBox lMsgBox;
            lMsgBox.setWindowTitle("Error");
            lMsgBox.setText("Anonymous user can not be added to a group!");
            lMsgBox.setStandardButtons(QMessageBox::Ok);
            lMsgBox.exec();
        }
        else
            mGroups->AddUser(mCurrentGroupId, lSelectedItems.at(i)->data(Qt::DisplayRole).toString());
    }

    LoadGroupUsers(mCurrentGroupId);
    emit sGroupsChanged();
}

void GroupsAdminForm::OnRemoveUsers()
{
    QList<QListWidgetItem*> lSelectedItems = mUi->listWidgetGroupUsers->selectedItems();

    for (int i = 0; i < lSelectedItems.count(); ++i)
        mGroups->RemoveUser(mCurrentGroupId, lSelectedItems.at(i)->data(Qt::DisplayRole).toString());

    LoadGroupUsers(mCurrentGroupId);
    emit sGroupsChanged();
}

} // END DAPlugin
} // END GS

