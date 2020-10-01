#include "db_table_link.h"
#include <gqtl_log.h>

#include <QStringList>

namespace GS
{
namespace DbPluginBase
{

DbTableLink::DbTableLink(QObject *parent) :
    QObject(parent)
{
    mId = "0";
}

DbTableLink::DbTableLink(const QString &id, QObject *parent) :
    QObject(parent)
{
    mId = id;
}

DbTableLink::DbTableLink(const QString &table1,
                         const QString &table2,
                         const QList<KeysLink> &keysLinks,
                         QObject *parent):
    QObject(parent)
{
    mId = "0";
    mTable1 = table1;
    mTable2 = table2;
    mKeysLinks = keysLinks;
}

DbTableLink::DbTableLink(const DbTableLink &source) :
    QObject(source.parent())
{
    *this = source;
}

DbTableLink &DbTableLink::operator =(const DbTableLink &source)
{
    if (this != &source)
    {
        mId = source.mId;
        mTable1 = source.mTable1;
        mTable2 = source.mTable2;
        for (int i = 0; i < source.mKeysLinks.count(); ++i)
            mKeysLinks.append(source.mKeysLinks.at(i));
    }

    return *this;
}

bool DbTableLink::operator==(const DbTableLink &source) const
{
    if ((mId !="0") && (source.mId !="0") && (mId != source.mId))
        return false;

    if (mKeysLinks.count() != source.mKeysLinks.count())
        return false;

    bool lIsEqualSym = false;
    bool lIsEqualNotSym = false;

    if ((mTable1 == source.mTable1) && (mTable2 == source.mTable2))
        lIsEqualSym = true;

    if ((mTable1 == source.mTable2) && (mTable2 == source.mTable1))
        lIsEqualNotSym = true;

    if (!lIsEqualSym && !lIsEqualNotSym)
        return false;

    for (int i = 0; i < source.mKeysLinks.count(); ++i)
    {
        if (!KeysLink::IsEqual(mKeysLinks.at(i), source.mKeysLinks.at(i), lIsEqualSym))
            return false;
    }

    return true;
}

bool DbTableLink::operator!=(const DbTableLink &source) const
{
    return !((*this) == source);
}

void DbTableLink::SetId(const QString &id)
{
    if (id.trimmed().isEmpty())
        return;

    mId = id.trimmed();
}

DbTableLink::~DbTableLink()
{
    ClearTableLinkElt();
}

const QString &DbTableLink::Id() const
{
    return mId;
}

const QString &DbTableLink::Table1() const
{
    return mTable1;
}

const QString &DbTableLink::Table2() const
{
    return mTable2;
}

QStringList DbTableLink::NormalizedTablesLinks() const
{
    QStringList lTableLinks;
    for (int i = 0; i < mKeysLinks.count(); ++i)
    {
        QString lLink;
        if (mKeysLinks[i].mTable1Key.mType == Key::FIELD)
            lLink = mTable1 + "." + mKeysLinks[i].mTable1Key.mValue;
        else
            lLink = "\'" + mKeysLinks[i].mTable1Key.mValue + "\'";
        lLink += " = ";
        if (mKeysLinks[i].mTable2Key.mType == Key::FIELD)
            lLink += mTable2 + "." + mKeysLinks[i].mTable2Key.mValue;
        else
            lLink += "\'" + mKeysLinks[i].mTable2Key.mValue + "\'";
        lTableLinks << lLink;
    }
    return lTableLinks;
}

QString DbTableLink::Conditions() const
{
    QString lFieldClause;
    for (int i = 0; i < mKeysLinks.count(); ++i)
    {
        QString lLink;
        if (i > 0)
            lFieldClause += " AND ";
        if (mKeysLinks[i].mTable1Key.mType == Key::FIELD)
            lLink = mTable1 + "." + mKeysLinks[i].mTable1Key.mValue;
        else
            lLink = "\'" + mKeysLinks[i].mTable1Key.mValue + "\'";
        lLink += " = ";
        if (mKeysLinks[i].mTable2Key.mType == Key::FIELD)
            lLink += mTable2 + "." + mKeysLinks[i].mTable2Key.mValue;
        else
            lLink += "\'" + mKeysLinks[i].mTable2Key.mValue + "\'";
        lFieldClause += lLink;
    }

    if (!lFieldClause.isEmpty())
    {
        lFieldClause.prepend("(");
        lFieldClause.append(")");
    }

    return lFieldClause;
}

bool DbTableLink::LoadFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    QDomElement lNodeTmp;

    mId = node.attribute("id");

    // Load table 1
    lNodeTmp = node.firstChildElement("table1");
    if (NodeIsValid(lNodeTmp))
        mTable1 = lNodeTmp.text();

    // Load table 1
    lNodeTmp = node.firstChildElement("table2");
    if (NodeIsValid(lNodeTmp))
        mTable2 = lNodeTmp.text();

    // Load table link elts
    lNodeTmp = node.firstChildElement("key_link_list");
    if (NodeIsValid(lNodeTmp))
        LoadKeyLinksFromDom(lNodeTmp);

    return true;
}

bool DbTableLink::LoadKeyLinksFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    ClearTableLinkElt();
    QDomElement lNodeTmp;

    QDomNode lNodeKeyLink = node.firstChildElement("key_link");

    while (!lNodeKeyLink.isNull())
    {
        if (lNodeKeyLink.isElement())
        {
            QDomElement lEltKeyLink = lNodeKeyLink.toElement();
            if (NodeIsValid(lEltKeyLink))
            {
                Key lTable1Key, lTable2Key;
                lNodeTmp = lEltKeyLink.firstChildElement("table1_key");
                if (NodeIsValid(lNodeTmp))
                {
                    lTable1Key.mValue = lNodeTmp.text();
                    QString lKeyType = lNodeTmp.attribute("type","field").toLower();
                    lTable1Key.mType = (lKeyType == "string") ? Key::STRING : Key::FIELD;
                }
                lNodeTmp = lEltKeyLink.firstChildElement("table2_key");
                if (NodeIsValid(lNodeTmp))
                {
                    lTable2Key.mValue = lNodeTmp.text();
                    QString lKeyType = lNodeTmp.attribute("type","field").toLower();
                    lTable2Key.mType = (lKeyType == "string") ? Key::STRING : Key::FIELD;
                }

                mKeysLinks.append(KeysLink(lTable1Key, lTable2Key));
            }
        }
        lNodeKeyLink = lNodeKeyLink.nextSibling();
    }

    return true;
}

void DbTableLink::ClearTableLinkElt()
{
    mKeysLinks.clear();
}

bool DbTableLink::NodeIsValid(const QDomElement &node) const
{
    if (node.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("No '%1' node in table_link"
                                         " element #%2").
               arg(node.tagName()).
               arg(mId).toLatin1().data());
        return false;
    }

    return true;
}

} //END namespace DbPluginBase
} //END namespace GS

