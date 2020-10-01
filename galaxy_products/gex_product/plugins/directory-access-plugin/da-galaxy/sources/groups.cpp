
#include <QStringList>

#include "groups.h"

namespace GS
{
namespace DAPlugin
{
Groups::Groups()
{
}

Groups::~Groups()
{
}

bool Groups::Load(const QDomNode &groupsNode)
{
    if (groupsNode.isNull())
    {
        mLastError = "Groups list is NULL";
        mGroupsNode = QDomNode();
        return false;
    }
    mGroupsNode = groupsNode;
    return true;
}

bool Groups::GetGroupNode(const QString &groupId, QDomNode &matchingNode)
{
    QDomNode lNode = mGroupsNode.firstChild();
    while (!lNode.isNull())
    {
        if (lNode.toElement().attribute(QString(GROUP_ID)) == groupId)
        {
            matchingNode = lNode;
            return true;
        }
        lNode = lNode.nextSibling();
    }
    return false;
}

QDomDocument Groups::GetDocNode(const QDomNode &node)
{
    QDomNode lRootNode = node;

    while (!lRootNode.parentNode().isNull())
        lRootNode = lRootNode.parentNode();

    return lRootNode.toDocument();
}

QString Groups::GetGroupAttribute(const QString &groupId, const QString &groupAttribute, bool &ok)
{
    QString lGroupId = groupId.toLower();
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group %1").arg(lGroupId);
        ok = false;
        return QString();
    }
    ok = true;
    return lGroupNode.firstChildElement(groupAttribute).text();
}

QStringList Groups::GetGroupAttributes(const QString &groupId, bool &ok)
{
    QString lGroupId = groupId.toLower();
    ok = false;
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find user %1").arg(lGroupId);
        return QStringList();
    }
    ok = true;
    QStringList lGroupAttributes;
    QDomNode lAttNode = lGroupNode.firstChild();
    while(!lAttNode.isNull())
    {
        lGroupAttributes << lAttNode.nodeName();
        lAttNode = lAttNode.nextSibling();
    }

    return lGroupAttributes;
}

bool Groups::UpdateGroupAttribute(const QString &groupId, const QString &groupAttribute, const QString &newValue)
{
    QString lGroupId = groupId.toLower();
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group %1").arg(lGroupId);
        return false;
    }

    if (lGroupNode.firstChildElement(groupAttribute).isNull())
    {
        QDomNode lNewAttribute = GetDocNode(mGroupsNode).createElement(groupAttribute);
        lGroupNode.appendChild(lNewAttribute);
    }

    QDomText lTextElt = lGroupNode.firstChildElement(groupAttribute).firstChild().toText();
    if (!lTextElt.isNull())
        lTextElt.setData(newValue);
    else
    {
        QDomText lTextElt = GetDocNode(mGroupsNode).createTextNode(newValue);
        lGroupNode.firstChildElement(groupAttribute).appendChild(lTextElt);
    }

    emit sUpdated();
    return true;
}

bool Groups::Add(const QString &groupId)
{
    if (groupId.isEmpty())
    {
        mLastError = "Empty group id";
        return false;
    }
    QString lGroupId = groupId.toLower();
    // check if groupid exists
    QDomNode lNode;
    if (GetGroupNode(lGroupId, lNode))
    {
        mLastError = QString("Group %1 already exists").arg(lGroupId);
        return false;
    }
    // if not add new
    QDomElement lNewGroupElt = GetDocNode(mGroupsNode).createElement(GROUP_NODE);
    lNewGroupElt.setAttribute(GROUP_ID, lGroupId);
    QDomElement lNewGroupItemElt = GetDocNode(mGroupsNode).createElement(GROUP_NAME);
    lNewGroupElt.appendChild(lNewGroupItemElt);
    lNewGroupItemElt = GetDocNode(mGroupsNode).createElement(GROUP_DESC);
    lNewGroupElt.appendChild(lNewGroupItemElt);
    lNewGroupItemElt = GetDocNode(mGroupsNode).createElement(GROUP_USERS);
    lNewGroupElt.appendChild(lNewGroupItemElt);
    mGroupsNode.appendChild(lNewGroupElt);

    emit sUpdated();
    return true;
}

bool Groups::Remove(const QString &groupId)
{
    if (groupId.isEmpty())
    {
        mLastError = "Empty group id";
        return false;
    }
    QString lGroupId = groupId.toLower();
    QDomNode lGroupNode;
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group: %1").arg(lGroupId);
        return false;
    }

    mGroupsNode.removeChild(lGroupNode);
    emit sUpdated();
    return true;
}

bool Groups::AddUser(const QString &groupId, const QString &userId)
{
    QDomNode lGroupNode;
    QString lGroupId = groupId.toLower();
    QString lUserId = userId.toLower();
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group: %1").arg(lGroupId);
        return false;
    }
    QDomNode lUsersNode = lGroupNode.firstChildElement(GROUP_USERS);
    // check if user exists
    QDomNode lUserNode = lUsersNode.firstChild();
    while (!lUserNode.isNull())
    {
        QDomElement lElt = lUserNode.toElement();
        if (lElt.attribute(GROUP_USER_ID) == lUserId)
        {
            mLastError = QString("user %1 already exists in group %2").arg(lUserId).arg(lGroupId);
            return false;
        }
        lUserNode = lUserNode.nextSibling();
    }
    // if not add new
    QDomElement lNewUserElt = GetDocNode(mGroupsNode).createElement(GROUP_USER);
    lNewUserElt.setAttribute(GROUP_USER_ID, lUserId);
    lUsersNode.appendChild(lNewUserElt);

    emit sUpdated();
    return true;
}

bool Groups::RemoveUser(const QString &groupId, const QString &userId)
{
    QDomNode lGroupNode;
    QString lGroupId = groupId.toLower();
    QString lUserId = userId.toLower();
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group %1").arg(lGroupId);
        return false;
    }
    QDomNode lUsersNode = lGroupNode.firstChildElement(GROUP_USERS);
    // check if user exists
    QDomNode lUserNode = lUsersNode.firstChild();
    while (!lUserNode.isNull())
    {
        QDomElement lElt = lUserNode.toElement();
        if (lElt.attribute(GROUP_USER_ID) == lUserId)
        {
            lUsersNode.removeChild(lUserNode);
            emit sUpdated();
            return true;
        }
        lUserNode = lUserNode.nextSibling();
    }

    mLastError = QString("Unable to find user %1").arg(lUserId);
    return false;
}

bool Groups::Exists(const QString &groupId)
{
    return GetGroupsList().contains(groupId.toLower());
}

QStringList Groups::GetGroupsList()
{
    QStringList lGroupsList;
    QDomNode lNode = mGroupsNode.firstChild();
    while(!lNode.isNull())
    {
        QDomElement lElt = lNode.toElement();
        lGroupsList << lElt.attribute(QString(GROUP_ID));
        lNode = lNode.nextSibling();
    }

    return lGroupsList;
}

QStringList Groups::GetUsersList(const QString &groupId)
{
    QStringList lUsersList;
    QDomNode lGroupNode;
    QString lGroupId = groupId.toLower();
    if (!GetGroupNode(lGroupId, lGroupNode))
    {
        mLastError = QString("Unable to find group %1").arg(lGroupId);
        return lUsersList;
    }
    QDomNode lUsersNode = lGroupNode.firstChildElement(GROUP_USERS);
    // check if user exists
    QDomNode lUserNode = lUsersNode.firstChild();
    while (!lUserNode.isNull())
    {
        QDomElement lElt = lUserNode.toElement();
        lUsersList << lElt.attribute(GROUP_USER_ID);
        lUserNode = lUserNode.nextSibling();
    }
    return lUsersList;
}

QStringList Groups::GetGroupsList(const QString &userId)
{
    QStringList lGroupsList;
    QString lUserId = userId.toLower();
    QDomNode lGroupNode = mGroupsNode.firstChild();
    while(!lGroupNode.isNull())
    {
        QDomNode lUsersNode = lGroupNode.firstChildElement(GROUP_USERS);
        // check if user exists
        QDomNode lUserNode = lUsersNode.firstChild();
        while (!lUserNode.isNull())
        {
            QDomElement lEltUser = lUserNode.toElement();
            if (lEltUser.attribute(GROUP_USER_ID) == lUserId)
            {
                QDomElement lEltGroup = lGroupNode.toElement();
                lGroupsList << lEltGroup.attribute(QString(GROUP_ID));
            }
            lUserNode = lUserNode.nextSibling();
        }
        lGroupNode = lGroupNode.nextSibling();
    }

    return lGroupsList;
}

QString Groups::GetLastError()
{
    return mLastError;
}
} // END DAPlugin
} // END GS

