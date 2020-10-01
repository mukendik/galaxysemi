#include "db_table.h"
#include <gqtl_log.h>

#include <QStringList>

namespace GS
{
namespace DbPluginBase
{

DbTable::DbTable(QObject *parent) :
    QObject(parent)
{
    mTestingStage = ANY;
}

DbTable::DbTable(const DbTable &source) :
    QObject(source.parent())
{
    *this = source;
}

DbTable &DbTable::operator =(const DbTable &source)
{
    if (this != &source)
    {
        mName = source.mName;
        mTestingStage = source.mTestingStage;
        mDatabaseName = source.mDatabaseName;
        mPrimaryKey = source.mPrimaryKey;
        for (int i = 0; i < source.mDependencies.count(); ++i)
            mDependencies.append(source.mDependencies.at(i));
    }

    return *this;
}

DbTable::~DbTable()
{
    ClearDependencies();
}

void DbTable::ClearDependencies()
{
    mDependencies.clear();
}

bool DbTable::NodeIsValid(const QDomElement &node) const
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

const QString DbTable::Name() const
{
    return mName;
}

const QStringList DbTable::PrimaryKey() const
{
    return mPrimaryKey;
}

bool DbTable::IsPkIncludedIn(const QStringList &key) const
{
    QStringList lComparedKey = key;
    lComparedKey.removeDuplicates();

    if (mPrimaryKey.isEmpty() || (mPrimaryKey.count() > lComparedKey.count()))
        return false;

    foreach(const QString &lField, mPrimaryKey)
    {
        if (!lComparedKey.contains(lField, Qt::CaseInsensitive))
            return false;
    }

    return true;
}

void DbTable::SetTestingStage(const QString &testingStage)
{
    if (testingStage == "e-test")
        mTestingStage = ETEST;
    if (testingStage == "w-sort")
        mTestingStage = WSORT;
    if (testingStage == "f-test")
        mTestingStage = FTEST;
}

void DbTable::SetName(const QString &name)
{
    mName = name;
}

bool DbTable::AddDependency(const DbTableDependency &dependency)
{
    // check if already exists
    if (mDependencies.contains(dependency))
        return false;

    mDependencies.append(dependency);
    return true;
}

bool DbTable::LoadFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    QDomElement lNodeTmp;
    // Load table name
    mName = node.attribute("name");

    // LoadTestingStage
    lNodeTmp = node.firstChildElement("testing_stage");
    if (NodeIsValid(lNodeTmp))
        SetTestingStage(lNodeTmp.text());

    // Load dependencies <dependencies_list>
    lNodeTmp = node.firstChildElement("dependency_list");
    if (NodeIsValid(lNodeTmp))
        LoadDependenciesFromDom(lNodeTmp);

    // Load primary key
    lNodeTmp = node.firstChildElement("primary_key");
    if (!lNodeTmp.isNull())
        LoadPrimaryKeyFromDom(lNodeTmp);

    return true;
}

bool DbTable::LoadDependenciesFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    ClearDependencies();

    QDomNode lNodeDependency = node.firstChildElement("dependency");
    while (!lNodeDependency.isNull())
    {
        if (lNodeDependency.isElement())
        {
            QDomElement lEltNodeDependency = lNodeDependency.toElement();
            if (NodeIsValid(lEltNodeDependency))
            {
                DbTableDependency lDependency;
                lDependency.LoadFromDom(lEltNodeDependency);
                if (lDependency.LinkCount() > 0)
                    mDependencies.append(lDependency);
            }
        }
        lNodeDependency = lNodeDependency.nextSibling();
    }

    return true;
}

bool DbTable::LoadPrimaryKeyFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    mPrimaryKey.clear();

    QDomNode lNodeField = node.firstChildElement("field");
    while (!lNodeField.isNull())
    {
        if (lNodeField.isElement())
        {
            QDomElement lEltNodeField = lNodeField.toElement();
            if (NodeIsValid(lEltNodeField))
                mPrimaryKey << lEltNodeField.attribute("name");
        }
        lNodeField = lNodeField.nextSibling();
    }

    mPrimaryKey.removeDuplicates();

    return true;
}

bool DbTable::LoadTableLinksContent(
        const QMap<QString, DbTableLink> &tablesLinks)
{
    bool lValid = true;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (!mDependencies[i].LoadTableLinksContent(tablesLinks))
            lValid = false;
        if (!mDependencies[i].IsValidDependencyOfTable(mName))
            lValid = false;
    }
    return lValid;
}

bool DbTable::HasDependencyWith(const QString &table)
{
    for (int i = 0; i < mDependencies.count(); ++i)
        if (mDependencies.at(i).Contains(table))
            return true;

    return false;
}

bool DbTable::HasFunctionalDependencyWith(const QString &table)
{
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                mDependencies.at(i).IsFunctional())
            return true;
    }

    return false;
}

bool DbTable::HasSimpleFDWith(const QString &table)
{
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                mDependencies.at(i).IsFunctional() &&
                mDependencies.at(i).IsSimple())
            return true;
    }

    return false;
}

bool DbTable::HasMultipleFDWith(const QString &table)
{
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                mDependencies.at(i).IsFunctional() &&
                mDependencies.at(i).IsMultiple())
            return true;
    }

    return false;
}

QList<DbTableDependency *> DbTable::NonFDsWith(const QString &table)
{
    QList<DbTableDependency *> lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                !mDependencies.at(i).IsFunctional())
            lDependencies.append(&mDependencies[i]);
    }

    return lDependencies;
}

QList<DbTableDependency *> DbTable::SimpleFDsWith(const QString &table)
{
    QList<DbTableDependency *> lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                mDependencies.at(i).IsFunctional() &&
                mDependencies.at(i).IsSimple())
            lDependencies.append(&mDependencies[i]);
    }

    return lDependencies;
}

QList<DbTableDependency *> DbTable::MultipleFDsWith(const QString &table)
{
   QList<DbTableDependency *>  lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table) &&
                mDependencies.at(i).IsFunctional() &&
                mDependencies.at(i).IsMultiple())
            lDependencies.append(&mDependencies[i]);
    }

    return lDependencies;
}

QList<DbTableDependency *> DbTable::DependenciesWith(const QString &table,
                                                     bool functionalOnly)
{
    QList<DbTableDependency *> lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).Contains(table))
        {
            if (!functionalOnly)
                lDependencies.append(&mDependencies[i]);
            else if (functionalOnly && mDependencies.at(i).IsFunctional())
                lDependencies.append(&mDependencies[i]);
        }
    }

    return lDependencies;
}

QList<QPair<QString, DbTableDependency *> > DbTable::NonFDsWith(
        const QStringList &tables)
{
    QList<DbTableDependency *> lDependencies;
    QList<QPair<QString, DbTableDependency *> > lDependencyTables;
    for (int i = 0; i < tables.count(); ++i)
    {
        lDependencies = NonFDsWith(tables.at(i));
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
    }

    return lDependencyTables;
}

QList<QPair<QString, DbTableDependency *> > DbTable::SimpleFDsWith(
        const QStringList &tables)
{
    QList<DbTableDependency *> lDependencies;
    QList<QPair<QString, DbTableDependency *> > lDependencyTables;
    for (int i = 0; i < tables.count(); ++i)
    {
        lDependencies = SimpleFDsWith(tables.at(i));
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
    }

    return lDependencyTables;
}

QList<QPair<QString, DbTableDependency *> > DbTable::MultipleFDsWith(
        const QStringList &tables)
{
    QList<DbTableDependency *> lDependencies;
    QList<QPair<QString, DbTableDependency *> > lDependencyTables;
    for (int i = 0; i < tables.count(); ++i)
    {
        lDependencies = MultipleFDsWith(tables.at(i));
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
    }

    return lDependencyTables;
}

QList<QPair<QString, DbTableDependency *> > DbTable::FDsWith(
        const QStringList &tables)
{
    QList<DbTableDependency *> lDependencies;
    QList<QPair<QString, DbTableDependency *> > lDependencyTables;
    for (int i = 0; i < tables.count(); ++i)
    {
        lDependencies = SimpleFDsWith(tables.at(i));
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
        lDependencies = MultipleFDsWith(tables.at(i));
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
    }

    return lDependencyTables;
}

QList<QPair<QString, DbTableDependency *> > DbTable::DependenciesWith(
        const QStringList &tables, bool functionalOnly)
{
    QList<DbTableDependency *> lDependencies;
    QList<QPair<QString, DbTableDependency *> > lDependencyTables;
    for (int i = 0; i < tables.count(); ++i)
    {
        lDependencies = DependenciesWith(tables.at(i), functionalOnly);
        foreach (DbTableDependency * lDpdc, lDependencies)
            lDependencyTables.append(qMakePair(tables.at(i), lDpdc));
    }

    return lDependencyTables;
}


QList<const DbTableDependency *> DbTable::FunctionalDependencies() const
{
    QList<const DbTableDependency *> lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
    {
        if (mDependencies.at(i).IsFunctional())
            lDependencies.append(&mDependencies[i]);
    }
    return lDependencies;
}


QList<const DbTableDependency *> DbTable::Dependencies() const
{
    QList<const DbTableDependency *> lDependencies;
    for (int i = 0; i < mDependencies.count(); ++i)
            lDependencies.append(&mDependencies[i]);
    return lDependencies;
}

} //END namespace DbPluginBase
} //END namespace GS

