
#include <QStringList>

#include "app_entries.h"
#include "groups.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{

AppEntries::AppEntries()
{
    mAppSeparator = ":";
}

AppEntries::~AppEntries()
{
}

bool AppEntries::Add(const QString &parentPath, const QString &entryId)
{
    QDomNode lParentNode;
    QString lParentPath = parentPath.toLower();
    QString lEntryId = entryId.toLower();
    // check if path is valid and get parent node
    if (!GetEntryNode(mAppEntriesNode, lParentPath, lParentNode))
    {
        mLastError = QString("Unable to find entry %1").arg(lParentPath);
        return false;
    }
    // check is entry already exists
    QDomNode lTmpNode;
    if (GetEntryNode(lParentNode, lEntryId, lTmpNode))
    {
        mLastError = QString("Entry %1 already exists as child of %2").arg(lEntryId).arg(lParentPath);
        return false;
    }
    // add entry
    QDomElement lNewEntryElt = GetDocNode(mAppEntriesNode).createElement(ENTRY_NODE);
    lNewEntryElt.setAttribute(ENTRY_ID, lEntryId);
    QDomElement lNewEntryItemElt = GetDocNode(mAppEntriesNode).createElement(ENTRY_NAME);
    lNewEntryElt.appendChild(lNewEntryItemElt);
    lNewEntryItemElt = GetDocNode(mAppEntriesNode).createElement(ENTRY_DESC);
    lNewEntryElt.appendChild(lNewEntryItemElt);
    lNewEntryItemElt = GetDocNode(mAppEntriesNode).createElement(ENTRY_USERS_ACCESS);
    lNewEntryElt.appendChild(lNewEntryItemElt);
    lNewEntryItemElt = GetDocNode(mAppEntriesNode).createElement(ENTRY_GROUPS_ACCESS);
    lNewEntryElt.appendChild(lNewEntryItemElt);
    lParentNode.appendChild(lNewEntryElt);

    emit sUpdated();
    return true;
}

bool AppEntries::Move(const QString &entryPath, const QString &newParentPath)
{
    // check is entry exists
    QDomNode lEntryNode;
    QString lEntryPath = entryPath.toLower();
    QString lNewParentPath = newParentPath.toLower();
    if (!GetEntryNode(mAppEntriesNode, lEntryPath, lEntryNode))
    {
        mLastError = QString("Entry %1 doesn't exists").arg(lEntryPath);
        return false;
    }
    // check is entry exists
    QDomNode lNewParentEntryNode;
    if (!GetEntryNode(mAppEntriesNode, lNewParentPath, lNewParentEntryNode))
    {
        mLastError = QString("New parent entry %1 doesn't exists").arg(lNewParentPath);
        return false;
    }
    // Add new
    QDomNode lNewNode = lEntryNode.cloneNode();
    lNewParentEntryNode.appendChild(lNewNode);
    // Remove Old
    QDomNode lOldParentEntryNode = lEntryNode.parentNode();
    lOldParentEntryNode.removeChild(lEntryNode);

    emit sUpdated();
    return true;
}

bool AppEntries::Remove(const QString &entryPath)
{
    // check is entry exists
    QDomNode lEntryNode;
    QString lEntryPath = entryPath.toLower();
    if (!GetEntryNode(mAppEntriesNode, lEntryPath, lEntryNode))
    {
        mLastError = QString("Entry %1 doesn't exists").arg(lEntryPath);
        return false;
    }
    // Remove node
    QDomNode lParentEntryNode = lEntryNode.parentNode();
    lParentEntryNode.removeChild(lEntryNode);

    emit sUpdated();
    return true;
}

bool AppEntries::UpdateEntryAttribute(const QString &entryPath, const QString &entryAttribute, const QString &newValue)
{
    QDomNode lEntryNode;
    QString lEntryPath = entryPath.toLower();
    QString lNewValue = newValue;
    if (!GetEntryNode(mAppEntriesNode, lEntryPath, lEntryNode))
    {
        mLastError = QString("Entry %1 doesn't exists").arg(lEntryPath);
        return false;
    }

    if (lEntryNode.firstChildElement(entryAttribute).isNull())
    {
        QDomNode lNewAttribute = GetDocNode(mAppEntriesNode).createElement(entryAttribute);
        lEntryNode.appendChild(lNewAttribute);
    }

    QDomText lTextElt = lEntryNode.firstChildElement(entryAttribute).firstChild().toText();
    if (!lTextElt.isNull())
        lTextElt.setData(lNewValue);
    else
    {
        QDomText lTextElt = GetDocNode(mAppEntriesNode).createTextNode(lNewValue);
        lEntryNode.firstChildElement(entryAttribute).appendChild(lTextElt);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::AddUserPrivilege(const QString &entryPath, const QString &userId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lUserId = userId.toLower();
    if (!GetUserAccessNode(lEntryPath, lUserId, lPrivilegesNode))
        return false;

    QDomText userAccessNode = lPrivilegesNode.firstChild().toText();
    if (!userAccessNode.isNull())
    {
        bool lOk = true;
        int lCurrentPrivileges = userAccessNode.data().toInt(&lOk);
        if (!lOk)
        {
            mLastError = QString("Invalid value of privilege: %1").arg(userAccessNode.data());
            return false;
        }
        lCurrentPrivileges = AddPrivilege(lCurrentPrivileges, privilege);
        userAccessNode.setData(QString::number(lCurrentPrivileges));
    }
    else
    {
        userAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(userAccessNode);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::RemoveUserPrivilege(const QString &entryPath, const QString &userId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lUserId = userId.toLower();
    if (!GetUserAccessNode(lEntryPath, lUserId, lPrivilegesNode))
        return false;

    QDomText userAccessNode = lPrivilegesNode.firstChild().toText();
    if (!userAccessNode.isNull())
    {
        bool lOk = true;
        int lCurrentPrivileges = userAccessNode.data().toInt(&lOk);
        if (!lOk)
        {
            mLastError = QString("Invalid value of privilege: %1").arg(userAccessNode.data());
            return false;
        }
        lCurrentPrivileges = RemovePrivilege(lCurrentPrivileges, privilege);
        userAccessNode.setData(QString::number(lCurrentPrivileges));
    }
    else
    {
        userAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(userAccessNode);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::UpdateUserPrivilege(const QString &entryPath, const QString &userId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lUserId = userId.toLower();
    if (!GetUserAccessNode(lEntryPath, lUserId, lPrivilegesNode))
        return false;

    QDomText userAccessNode = lPrivilegesNode.firstChild().toText();
    if (!userAccessNode.isNull())
        userAccessNode.setData(QString::number(privilege));
    else
    {
        userAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(userAccessNode);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::AddGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lGroupId = groupId.toLower();
    if (!GetGroupAccessNode(lEntryPath, lGroupId, lPrivilegesNode))
        return false;

    QDomText groupAccessNode = lPrivilegesNode.firstChild().toText();
    if (!groupAccessNode.isNull())
    {
        bool lOk = true;
        int lCurrentPrivileges = groupAccessNode.data().toInt(&lOk);
        if (!lOk)
        {
            mLastError = QString("Invalid value of privilege: %1").arg(groupAccessNode.data());
            return false;
        }
        lCurrentPrivileges = AddPrivilege(lCurrentPrivileges, privilege);
        groupAccessNode.setData(QString::number(lCurrentPrivileges));
    }
    else
    {
        groupAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(groupAccessNode);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::RemoveGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lGroupId = groupId.toLower();
    if (!GetGroupAccessNode(lEntryPath, lGroupId, lPrivilegesNode))
        return false;

    QDomText groupAccessNode = lPrivilegesNode.firstChild().toText();
    if (!groupAccessNode.isNull())
    {
        bool lOk = true;
        int lCurrentPrivileges = groupAccessNode.data().toInt(&lOk);
        if (!lOk)
        {
            mLastError = QString("Invalid value of privilege: %1").arg(groupAccessNode.data());
            return false;
        }
        lCurrentPrivileges = RemovePrivilege(lCurrentPrivileges, privilege);
        groupAccessNode.setData(QString::number(lCurrentPrivileges));
    }
    else
    {
        groupAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(groupAccessNode);
    }

    emit sUpdated();
    return true;
}

bool AppEntries::UpdateGroupPrivilege(const QString &entryPath, const QString &groupId, int privilege)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lGroupId = groupId.toLower();
    if (!GetGroupAccessNode(lEntryPath, lGroupId, lPrivilegesNode))
        return false;

    QDomText groupAccessNode = lPrivilegesNode.firstChild().toText();
    if (!groupAccessNode.isNull())
        groupAccessNode.setData(QString::number(privilege));
    else
    {
        groupAccessNode = GetDocNode(mAppEntriesNode).createTextNode(QString::number(privilege));
        lPrivilegesNode.appendChild(groupAccessNode);
    }

    emit sUpdated();
    return true;
}

int AppEntries::GetUserPrivileges(const QString &entryPath, const QString &userId)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lUserId = userId.toLower();
    if (!GetUserAccessNode(lEntryPath, lUserId, lPrivilegesNode))
        return 0;

    QDomText userAccessNode = lPrivilegesNode.firstChild().toText();
    if (userAccessNode.isNull())
        return 0;
    else
    {
        bool lOk = true;
        int lCurrentPrivileges = userAccessNode.data().toInt(&lOk);
        if (!lOk)
            return 0;

        return lCurrentPrivileges;
    }
}

int AppEntries::GetUserPrivileges(const QDomNode &entryNode, const QString &userId)
{
    QString lUserId = userId.toLower();
    // Check if user exists and add it if needed
    QDomNode lUsersNode = entryNode.firstChildElement(ENTRY_USERS_ACCESS);
    if (lUsersNode.isNull())
        return false;

    QDomNode lUserNode;
    if (!GetUserNode(lUsersNode, lUserId, lUserNode))
        return 0;

    QDomText userAccessNode = lUserNode.firstChildElement(ENTRY_ACCESS).firstChild().toText();

    if (userAccessNode.isNull())
        return 0;
    else
    {
        bool lOk = true;
        int lCurrentPrivileges = userAccessNode.data().toInt(&lOk);
        if (!lOk)
            return 0;

        return lCurrentPrivileges;
    }
}

bool AppEntries::UpdateUserPrivilege(const QDomNode &entryNode, const QString &userId, int privilege)
{
    QString lUserId = userId.toLower();
    // Check if user exists
    QDomNode lUsersNode = entryNode.firstChildElement(ENTRY_USERS_ACCESS);
    if (lUsersNode.isNull())
        return false;

    QDomNode lUserNode;
    if (!GetUserNode(lUsersNode, lUserId, lUserNode) &&
            !AddUserNode(lUsersNode, lUserId, lUserNode))
        return false;

    QDomNode lUserAccessNode = lUserNode.firstChildElement(ENTRY_ACCESS);
    QDomText lUserAccessValue = lUserAccessNode.firstChild().toText();

    if (!lUserAccessValue.isNull())
    {
        if ((entryNode.toElement().attribute(ENTRY_ID) == "users_groups_administrator") &&
                (lUserId == "admin") &&
                (privilege < lUserAccessValue.data().toInt()))
            return false;
        lUserAccessValue.setData(QString::number(privilege));
    }
    else
    {
        lUserAccessValue = GetDocNode(entryNode).createTextNode(QString::number(privilege));
        lUserAccessNode.appendChild(lUserAccessValue);
    }

    return true;
}

int AppEntries::GetGroupPrivileges(const QString &entryPath, const QString &groupId)
{
    QDomNode lPrivilegesNode;
    QString lEntryPath = entryPath.toLower();
    QString lGroupId = groupId.toLower();
    if (!GetGroupAccessNode(lEntryPath, lGroupId, lPrivilegesNode))
        return 0;

    QDomText groupAccessNode = lPrivilegesNode.firstChild().toText();
    if (groupAccessNode.isNull())
        return 0;
    else
    {
        bool lOk = true;
        int lCurrentPrivileges = groupAccessNode.data().toInt(&lOk);
        if (!lOk)
            return 0;

        return lCurrentPrivileges;
    }
}

int AppEntries::GetGroupPrivileges(const QDomNode &entryNode, const QString &groupId)
{
    QString lGroupId = groupId.toLower();
    // Check if group exists and add it if needed
    QDomNode lGroupsNode = entryNode.firstChildElement(ENTRY_GROUPS_ACCESS);
    if (lGroupsNode.isNull())
        return false;
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupsNode, lGroupId, lGroupNode))
        return 0;

    QDomText groupAccessNode = lGroupNode.firstChildElement(ENTRY_ACCESS).firstChild().toText();

    if (groupAccessNode.isNull())
        return 0;
    else
    {
        bool lOk = true;
        int lCurrentPrivileges = groupAccessNode.data().toInt(&lOk);
        if (!lOk)
            return 0;

        return lCurrentPrivileges;
    }
}

bool AppEntries::UpdateGroupPrivilege(const QDomNode &entryNode, const QString &groupId, int privilege)
{
    QString lGroupId = groupId.toLower();
    // Check if group exists
    QDomNode lGroupsNode = entryNode.firstChildElement(ENTRY_GROUPS_ACCESS);
    if (lGroupsNode.isNull())
        return false;
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupsNode, lGroupId, lGroupNode) &&
            !AddGroupNode(lGroupsNode, lGroupId, lGroupNode))
        return false;

    QDomNode lGroupAccessNode = lGroupNode.firstChildElement(ENTRY_ACCESS);
    QDomText lGroupAccessValue = lGroupAccessNode.firstChild().toText();

    if (!lGroupAccessValue.isNull())
        lGroupAccessValue.setData(QString::number(privilege));
    else
    {
        lGroupAccessValue = GetDocNode(entryNode).createTextNode(QString::number(privilege));
        lGroupAccessNode.appendChild(lGroupAccessValue);
    }

    return true;
}

bool AppEntries::IsAllowedTo(const QString &entryPath, const QString &userId, int privilege)
{
    QString lEntryPath = entryPath.toLower();
    QString lUserId = userId.toLower();
    QDomDocument lDoc = GetDocNode(mAppEntriesNode);

    // Check if user is allowed by his id
    int lUserPrivilege = GetUserPrivileges(lEntryPath, lUserId);
    if ((privilege & lUserPrivilege) == privilege)
        return true;

    // Check if user is allowed by his group
    Groups lGroups;
    if (!lGroups.Load(lDoc.firstChildElement("galaxy_dir_file").firstChildElement("groups")))
    {
        mLastError = lGroups.GetLastError();
        return false;
    }
    QStringList lUserGroups = lGroups.GetGroupsList(lUserId);
    QStringListIterator lGroupIdIter(lUserGroups);
    while (lGroupIdIter.hasNext())
    {
        // get parent node id to search
        QString lGroupId = lGroupIdIter.next();
        int lGroupPrivilege = GetGroupPrivileges(lEntryPath, lGroupId);
        if ((privilege & lGroupPrivilege) == privilege)
            return true;
    }

    return false;
}



bool AppEntries::Load(const QDomNode &appEntriesNode)
{
    if (appEntriesNode.isNull())
    {
        mLastError = "Entries list is NULL";
        mAppEntriesNode = QDomNode();
        return false;
    }

    mAppEntriesNode = appEntriesNode;

    return true;
}

bool AppEntries::GetEntryNode(const QDomNode &startNode, const QString &entryPath, QDomNode &matchingNode) const
{
    matchingNode = QDomNode();
    QStringList lParentIdList = entryPath.split(mAppSeparator);
    QStringListIterator lParentIdIter(lParentIdList);
    // check if node exists
    QDomNode lEntryNode = startNode;
    while (lParentIdIter.hasNext())
    {
        // get parent node id to search
        QString lTmpParentId = lParentIdIter.next();
        // get first child of last found
        lEntryNode = lEntryNode.firstChildElement(ENTRY_NODE);
        // search this parent
        bool lFound = false;
        while (!lEntryNode.isNull() && !lFound)
        {
            if (lEntryNode.toElement().attribute(QString(ENTRY_ID)) == lTmpParentId)
                lFound = true;
            else
                lEntryNode = lEntryNode.nextSibling();
        }

        if (lEntryNode.isNull())
            return false;
    }

    matchingNode = lEntryNode;
    return true;
}

bool AppEntries::GetUserNode(const QDomNode &usersNode, const QString &userId, QDomNode &matchingNode)
{
    QDomNode lNode = usersNode.firstChild();
    while (!lNode.isNull())
    {
        if (lNode.toElement().attribute(QString(ENTRY_USER_ID)) == userId)
        {
            matchingNode = lNode;
            return true;
        }
        lNode = lNode.nextSibling();
    }

    return false;
}

bool AppEntries::AddUserNode(QDomNode &usersNode, const QString &userId, QDomNode &userNode)
{
    if (usersNode.isNull() || userId.isEmpty())
        return false;

    QDomElement lUserElt = GetDocNode(usersNode).createElement(ENTRY_USER_NODE);
    lUserElt.setAttribute(ENTRY_USER_ID, userId);
    QDomElement lNewUserItemElt = GetDocNode(usersNode).createElement(ENTRY_ACCESS);
    lUserElt.appendChild(lNewUserItemElt);
    usersNode.appendChild(lUserElt);

    userNode = lUserElt;

    return true;
}

bool AppEntries::GetUserAccessNode(const QString &entryPath, const QString &userId, QDomNode &userAccessNode)
{
    // Check entry exists
    QDomNode lEntryNode;
    if (!GetEntryNode(mAppEntriesNode, entryPath, lEntryNode))
    {
        mLastError = QString("Entry %1 doesn't exists").arg(entryPath);
        return false;
    }
    // Check if user exists and add it if needed
    QDomNode lUsersNode = lEntryNode.firstChildElement(ENTRY_USERS_ACCESS);
    if (lUsersNode.isNull())
        return false;
    QDomNode lUserNode;
    if (!GetUserNode(lUsersNode, userId, lUserNode) &&
            !AddUserNode(lUsersNode, userId, lUserNode))
    {
        mLastError = QString("Unable to find/create user %1 in entry %2").arg(userId).arg(entryPath);
        return false;
    }

    userAccessNode = lUserNode.firstChildElement(ENTRY_ACCESS);
    return true;
}

bool AppEntries::GetGroupNode(const QDomNode &groupsNode, const QString &groupId, QDomNode &matchingNode)
{
    QDomNode lNode = groupsNode.firstChild();
    while (!lNode.isNull())
    {
        if (lNode.toElement().attribute(QString(ENTRY_GROUP_ID)) == groupId)
        {
            matchingNode = lNode;
            return true;
        }
        lNode = lNode.nextSibling();
    }

    return false;
}

bool AppEntries::AddGroupNode(QDomNode &groupsNode, const QString &groupId, QDomNode &groupNode)
{
    if (groupsNode.isNull() || groupId.isEmpty())
        return false;

    QDomElement lGoupElt = GetDocNode(groupsNode).createElement(ENTRY_GROUP_NODE);
    lGoupElt.setAttribute(ENTRY_GROUP_ID, groupId);
    QDomElement lNewGroupItemElt = GetDocNode(groupsNode).createElement(ENTRY_ACCESS);
    lGoupElt.appendChild(lNewGroupItemElt);
    groupsNode.appendChild(lGoupElt);

    groupNode = lGoupElt;

    return true;
}

bool AppEntries::GetGroupAccessNode(const QString &entryPath, const QString &groupId, QDomNode &groupAccessNode)
{
    // Check entry exists
    QDomNode lEntryNode;
    if (!GetEntryNode(mAppEntriesNode, entryPath, lEntryNode))
    {
        mLastError = QString("Entry %1 doesn't exists").arg(entryPath);
        return false;
    }
    // Check if group exists and add it if needed
    QDomNode lGroupsNode = lEntryNode.firstChildElement(ENTRY_GROUPS_ACCESS);
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupsNode, groupId, lGroupNode) &&
            !AddGroupNode(lGroupsNode, groupId, lGroupNode))
    {
        mLastError = QString("Unable to find/create user %1 in entry %2").arg(groupId).arg(entryPath);
        return false;
    }

    groupAccessNode = lGroupNode.firstChildElement(ENTRY_ACCESS);
    return true;
}

int AppEntries::AddPrivilege(int oldPrivilege, int privilegeToAdd)
{
    return (oldPrivilege |= privilegeToAdd);
}

int AppEntries::RemovePrivilege(int oldPrivilege, int privilegeToRemove)
{
    return (oldPrivilege &= ~privilegeToRemove);
}

QDomDocument AppEntries::GetDocNode(const QDomNode &node)
{
    QDomNode lRootNode = node;

    while (!lRootNode.parentNode().isNull())
        lRootNode = lRootNode.parentNode();

    return lRootNode.toDocument();
}

QString AppEntries::GetLastError() const
{
    return mLastError;
}

QDomNode AppEntries::GetEntriesNode()
{
    return mAppEntriesNode;
}

bool AppEntries::Exists(const QString &entryPath)
{
    QDomNode lEntryNode;
    QString lEntryPath = entryPath.toLower();
    if (!GetEntryNode(mAppEntriesNode, lEntryPath, lEntryNode))
        return false;

    return true;
}
} // END DAPlugin
} // END GS

