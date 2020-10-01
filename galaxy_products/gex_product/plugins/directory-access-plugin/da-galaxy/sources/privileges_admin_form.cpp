
#include <QTreeWidget>
#include <QDomNode>

#include "ui_privileges_admin_form.h"
#include "privileges_admin_form.h"
#include "access_model.h"
#include "app_entries.h"

namespace GS
{
namespace DAPlugin
{
PrivilegesAdminForm::PrivilegesAdminForm(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::PrivilegesAdminForm)
{
    mUi->setupUi(this);
    mModel = 0;
    mUsers = 0;
    mGroups = 0;
    mPrivileges = 0;
    mIsEditable = false;
}

PrivilegesAdminForm::~PrivilegesAdminForm()
{
    delete mUi;
}

bool PrivilegesAdminForm::SetData(Users *users, Groups *groups, AppEntries *appEntries)
{
    if (!users || !groups || !appEntries)
        return false;

    mUsers = users;
    mGroups = groups;
    mPrivileges = appEntries;
    return true;
}

void PrivilegesAdminForm::SetEditable(bool isEditable)
{
    mIsEditable = isEditable;
    // Edit trigger don't works for check boxs
    if (mModel)
        mModel->SetEditable(mIsEditable);
}

void PrivilegesAdminForm::LoadUserPrivileges(const QString &userId)
{
    // Clear model
    if (mModel)
    {
        disconnect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(sPrivilegesChanged()));
        delete mModel;
        mModel = 0;
        mUi->treeViewPrivileges->setModel(0);
    }
    // do nothing if user empty
    if (userId.isEmpty())
        return;

    QDomNode lNode = mPrivileges->GetEntriesNode();
    if (lNode.isNull())
        return;

    int lPrivilegesType = 2;
    mModel = new AccessModel(lNode, lPrivilegesType, userId, this);
    mModel->SetEditable(mIsEditable);
    mUi->treeViewPrivileges->setModel(mModel);
    mUi->treeViewPrivileges->expandToDepth(2);
    mUi->treeViewPrivileges->resizeColumnToContents(0);
    mUi->treeViewPrivileges->resizeColumnToContents(4);
    connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(sPrivilegesChanged()));
}

void PrivilegesAdminForm::LoadGroupPrivileges(const QString &groupId)
{
    // Clear model
    if (mModel)
    {
        disconnect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(sPrivilegesChanged()));
        delete mModel;
        mModel = 0;
        mUi->treeViewPrivileges->setModel(0);
    }
    // do nothing if group empty
    if (groupId.isEmpty())
        return;

    QDomNode lNode = mPrivileges->GetEntriesNode();
    if (lNode.isNull())
        return;

    int lPrivilegesType = 1;
    mModel = new AccessModel(lNode, lPrivilegesType, groupId, this);
    mModel->SetEditable(mIsEditable);
    mUi->treeViewPrivileges->setModel(mModel);
    mUi->treeViewPrivileges->expandToDepth(2);
    mUi->treeViewPrivileges->resizeColumnToContents(0);
    mUi->treeViewPrivileges->resizeColumnToContents(4);
    connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(sPrivilegesChanged()));
}
} // END DAPlugin
} // END GS

