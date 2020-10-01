
#include <QCryptographicHash>
#include <QStringList>

#include "users.h"

namespace GS
{
namespace DAPlugin
{
Users::Users()
{
}

Users::~Users()
{
}

bool Users::Load(const QDomNode& usersNode)
{
    if (usersNode.isNull())
    {
        mLastError = "Users list is NULL";
        mUsersNode = QDomNode();
        return false;
    }

    YmAdbV2SupportClearLastChanges();
    mUsersNode = usersNode;

    return true;
}

bool Users::GetUserNode(const QString &userId, QDomNode &matchingNode)
{
    QDomNode lNode = mUsersNode.firstChild();
    while (!lNode.isNull())
    {
        if (lNode.toElement().attribute(QString(USER_ID)) == userId)
        {
            matchingNode = lNode;
            return true;
        }
        lNode = lNode.nextSibling();
    }

    return false;
}

QDomDocument Users::GetDocNode(const QDomNode &node)
{
    QDomNode lRootNode = node;

    while (!lRootNode.parentNode().isNull())
        lRootNode = lRootNode.parentNode();

    return lRootNode.toDocument();
}

QString Users::GetUserAttribute(const QString &userId, const QString &userAttribute, bool &ok)
{
    ok = false;
    QDomNode lUserNode;
    QString lUserId = userId.toLower();
    if (!GetUserNode(lUserId, lUserNode))
    {
        mLastError = QString("Unable to find user %1").arg(lUserId);
        return QString();
    }
    ok = true;
    return lUserNode.firstChildElement(userAttribute).text();
}

QStringList Users::GetUserAttributes(const QString &userId, bool &ok)
{
    ok = false;
    QDomNode lUserNode;
    QString lUserId = userId.toLower();
    if (!GetUserNode(lUserId, lUserNode))
    {
        mLastError = QString("Unable to find user %1").arg(lUserId);
        return QStringList();
    }
    ok = true;
    QStringList lUserAttributes;
    QDomNode lAttNode = lUserNode.firstChild();
    while(!lAttNode.isNull())
    {
        lUserAttributes << lAttNode.nodeName();
        lAttNode = lAttNode.nextSibling();
    }

    return lUserAttributes;
}

bool Users::UpdateUserAttribute(const QString &userId, const QString &userAttribute, const QString &newValue)
{
    QDomNode lUserNode;
    QString lUserId = userId.toLower();
    QString lNewValue = newValue;
    if (!GetUserNode(lUserId, lUserNode))
    {
        mLastError = QString("Unable to find user %1").arg(lUserId);
        return false;
    }

    mYmAdminDbV2SupportChanges.append(QString("update user|%1|attribute|%2|value|%3").
                                           arg(lUserId).
                                           arg(userAttribute).
                                           arg(lNewValue));

    // Encrypt password
    if (userAttribute == USER_PASS)
        lNewValue = QCryptographicHash::hash(lNewValue.toLatin1(),QCryptographicHash::Md5).toHex();

    if (lUserNode.firstChildElement(userAttribute).isNull())
    {
        QDomNode lNewAttribute = GetDocNode(mUsersNode).createElement(userAttribute);
        lUserNode.appendChild(lNewAttribute);
    }

    QDomText lTextElt = lUserNode.firstChildElement(userAttribute).firstChild().toText();
    if (!lTextElt.isNull())
        lTextElt.setData(lNewValue);
    else
    {
        QDomText lTextElt = GetDocNode(mUsersNode).createTextNode(lNewValue);
        lUserNode.firstChildElement(userAttribute).appendChild(lTextElt);
    }

    emit sUpdated();
    return true;
}

bool Users::Add(const QString &userId, const QString &userPass)
{
    if (userId.isEmpty())
    {
        mLastError = "Empty user id";
        return false;
    }
    QString lUserId = userId.toLower();
    // check if user_id exists
    QDomNode lUserNode;
    if (GetUserNode(lUserId, lUserNode))
    {
        mLastError = QString("User %1 already exists").arg(lUserId);
        return false;
    }
    // if not add new
    QDomElement lNewUserElt = GetDocNode(mUsersNode).createElement(USER_NODE);
    lNewUserElt.setAttribute(USER_ID, lUserId);
    mYmAdminDbV2SupportChanges.append(QString("add user|%1").arg(lUserId));
    mYmAdminDbV2SupportChanges.append(QString("update user|%1|attribute|%2|value|%3").
                                           arg(lUserId).
                                           arg(USER_PASS).
                                           arg(userPass));
    // add empty name elt
    QDomElement lNewUserItemElt = GetDocNode(mUsersNode).createElement(USER_NAME);
    lNewUserElt.appendChild(lNewUserItemElt);
    // add empty email elt
    lNewUserItemElt = GetDocNode(mUsersNode).createElement(USER_EMAIL);
    lNewUserElt.appendChild(lNewUserItemElt);
    // add pass elt
    lNewUserItemElt = GetDocNode(mUsersNode).createElement(USER_PASS);
    QString lHashedPass = QCryptographicHash::hash(userPass.toLatin1(),QCryptographicHash::Md5).toHex();
    QDomText lNewTextElt = GetDocNode(mUsersNode).createTextNode(lHashedPass);
    lNewUserItemElt.appendChild(lNewTextElt);
    lNewUserElt.appendChild(lNewUserItemElt);
    mUsersNode.appendChild(lNewUserElt);

    emit sUpdated();
    return true;
}

bool Users::Remove(const QString &userId)
{
    if (userId.isEmpty())
    {
        mLastError = "Empty user id";
        return false;
    }
    QString lUserId = userId.toLower();
    // check if user_id exists
    QDomNode lUserNode;
    if (!GetUserNode(lUserId, lUserNode))
    {
        mLastError = QString("Unable to find user: %1").arg(lUserId);
        return false;
    }

    mUsersNode.removeChild(lUserNode);
    emit sUpdated();
    return true;
}

bool Users::Exists(const QString &userId)
{
    return GetUsersList().contains(userId.toLower());
}

QStringList Users::GetUsersList()
{
    QStringList lUsersList;
    QDomNode lNode = mUsersNode.firstChild();
    while(!lNode.isNull())
    {
        QDomElement lElt = lNode.toElement();
        lUsersList << lElt.attribute(QString(USER_ID));
        lNode = lNode.nextSibling();
    }

    return lUsersList;
}


QString Users::GetLastError()
{
    return mLastError;
}

QStringList Users::YmAdbV2SupportGetLastChanges()
{
    return mYmAdminDbV2SupportChanges;
}

void Users::YmAdbV2SupportClearLastChanges()
{
    mYmAdminDbV2SupportChanges.clear();
}

} // END DAPlugin
} // END GS

