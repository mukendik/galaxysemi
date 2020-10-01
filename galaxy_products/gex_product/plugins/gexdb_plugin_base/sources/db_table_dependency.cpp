#include "db_table_dependency.h"
#include <gqtl_log.h>

#include <QStringList>

namespace GS
{
namespace DbPluginBase
{

DbTableDependency::DbTableDependency(QObject *parent) :
    QObject(parent)
{
    mIsFunctional = false;
}

DbTableDependency::~DbTableDependency()
{
    ClearTableLink();
}

void DbTableDependency::ClearTableLink()
{
    mTableLinks.clear();
}

bool DbTableDependency::NodeIsValid(const QDomElement &node) const
{
    if (node.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("No '%1' node in table_dependency"
                                         " element").
               arg(node.tagName()).
               toLatin1().data());
        return false;
    }

    return true;
}

DbTableDependency::DbTableDependency(const DbTableDependency &source) :
    QObject(source.parent())
{
    *this = source;
}

DbTableDependency &DbTableDependency::operator =(
        const DbTableDependency &source)
{
    if (this != &source)
    {
        mIsFunctional = source.mIsFunctional;
        for (int i = 0; i < source.mTableLinks.count(); ++i)
            mTableLinks.append(source.mTableLinks.at(i));
    }

    return *this;
}

bool DbTableDependency::operator==(const DbTableDependency &source) const
{
//    if (mIsFunctional != source.mIsFunctional)
//        return false;

    if (mTableLinks.count() != source.TableLinks().count())
        return false;

    for (int i = 0; i < mTableLinks.count(); ++i)
    {
        if (mTableLinks.at(i) != source.mTableLinks.at(i))
            return false;
    }

    return true;
}

bool DbTableDependency::operator!=(const DbTableDependency &source) const
{
    return !((*this) == source);
}

bool DbTableDependency::IsFunctional() const
{
    return mIsFunctional;
}

bool DbTableDependency::LoadFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    QDomElement lNodeTmp;
    bool lOk;

    // Load is Functional
    lNodeTmp = node.firstChildElement("is_functional");
    if (NodeIsValid(lNodeTmp))
    {
        mIsFunctional = node.text().toInt(&lOk);
        if (!lOk)
            return false;
    }

    // Load tables Links
    lNodeTmp = node.firstChildElement("table_link_id_list");
    if (NodeIsValid(lNodeTmp))
        LoadTableLinkIdsFromDom(lNodeTmp);

    return true;
}

bool DbTableDependency::LoadTableLinkIdsFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    ClearTableLink();

    QDomNode lNodeTableLink = node.firstChildElement("table_link");
    while (!lNodeTableLink.isNull())
    {
        if (lNodeTableLink.isElement())
        {
            QDomElement lEltTableLink = lNodeTableLink.toElement();
            if (NodeIsValid(lEltTableLink))
            {
                DbTableLink lTableLink;
                lTableLink.SetId(lEltTableLink.attribute("id"));
                if (!lTableLink.Id().isEmpty())
                    mTableLinks.append(lTableLink);
            }
        }
        lNodeTableLink = lNodeTableLink.nextSibling();
    }

    return true;
}

void DbTableDependency::AddTableLink(const DbTableLink &tableLink)
{
    mTableLinks.append(tableLink);
}

bool DbTableDependency::LoadTableLinksContent(
        const QMap<QString, DbTableLink> &tablesLinks)
{
    bool lIsValid = true;

    for (int i = 0; i < mTableLinks.count(); ++i)
    {
        QString lId = mTableLinks.at(i).Id();
        if (!tablesLinks.contains(lId))
        {
            GSLOG(SYSLOG_SEV_ERROR,
                   QString("Table link #%1 not found in dependency").
                   arg(lId).
                   toLatin1().data());
            lIsValid = false;
        }
        else
            mTableLinks[i] = tablesLinks.value(lId);
    }

    return lIsValid;
}

bool DbTableDependency::Contains(const QString &table) const
{
    bool lContainsTable = false;
    for (int i = 0; i < mTableLinks.count() && !lContainsTable; ++i)
    {
        if (mTableLinks.at(i).Table1() == table ||
                mTableLinks.at(i).Table2() == table)
            lContainsTable = true;
    }

    return lContainsTable;
}

bool DbTableDependency::IsSimple() const
{
    return (mTableLinks.count() == 1);
}

bool DbTableDependency::IsMultiple() const
{
    return (mTableLinks.count() > 1);
}

bool DbTableDependency::LinkCount() const
{
    return mTableLinks.count();
}

QStringList DbTableDependency::IncludedTables() const
{
    QStringList lIncludedTables;
    for (int i = 0; i < mTableLinks.count(); ++i)
    {
        lIncludedTables.append(mTableLinks.at(i).Table1());
        lIncludedTables.append(mTableLinks.at(i).Table2());
    }

    lIncludedTables.removeDuplicates();

    return lIncludedTables;
}

const QList<DbTableLink> &DbTableDependency::TableLinks() const
{
    return mTableLinks;
}

bool DbTableDependency::IsValidDependencyOfTable(const QString &tableName)
{
    bool lIsValid = true;
    for (int i = 0; i < mTableLinks.count(); ++i)
    {
        if (mTableLinks.at(i).Table1() != tableName &&
                mTableLinks.at(i).Table2() != tableName)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Wrong link in table %1 dependency: "
                                             "link #%2 %3 does not include the good table.").
                   arg(tableName).
                   arg(mTableLinks.at(i).Id()).
                   arg(mTableLinks.at(i).Conditions()).
                   toLatin1().data());
            lIsValid = false;
        }
    }

    return lIsValid;
}

QStringList DbTableDependency::NormalizedTablesLinks() const
{
    QStringList lTableLinks;
    for (int i = 0; i < mTableLinks.count(); ++i)
        lTableLinks << mTableLinks[i].NormalizedTablesLinks();

    return lTableLinks;
}

} //END namespace DbPluginBase
} //END namespace GS

