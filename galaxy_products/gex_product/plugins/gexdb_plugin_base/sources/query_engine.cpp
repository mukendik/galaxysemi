#include "query_engine.h"
#include "db_architecture.h"
#include "db_table.h"
#include "dijkstra.h"
#include <gqtl_log.h>
#include "query_engine_private.h"

#include <QDebug>
#include <QMap>
#include <QPair>

#define JOINTURE_TYPE_PLACEHOLDER "##JointureType##"

namespace GS
{
namespace DbPluginBase
{

QueryEngine::QueryEngine(QObject *parent) :
    QObject(parent)
{
    mPrivate= new QueryEnginePrivate();
    mPrivate->mDbArchitecture = NULL;
    mPrivate->mBuildingRule = QueryEngine::UseFunctionalDep;

    ResetQuery();
}

QueryEngine::~QueryEngine()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
}
}

void QueryEngine::ResetQuery()
{
    mPrivate->mQueryFields.clear();
    mPrivate->mFilters.clear();
    mPrivate->mFinalTableListToQuery.clear();
    mPrivate->mWhereClause = "";
    mPrivate->mJoinClause = "";
    mPrivate->mFromClause = "";
    mPrivate->mValidQuery = false;
}

bool QueryEngine::SetDbArchitecture(DbArchitecture *dbArchitecture)
{
    mPrivate->mDbArchitecture = dbArchitecture;

    if (!dbArchitecture)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Architecture reset");
        mPrivate->mFunctionalGraph.Clear();
        mPrivate->mDependenciesGraph.Clear();
        return true;
    }

    if (!BuildFunctionalGraph())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, "
               "unable to build functional dependencies graph");
        return false;
    }

    if (!BuildDepenciesGraph())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, "
               "unable to build dependencies graph");
        return false;
    }

    return true;
}


bool QueryEngine::ExtractFromFilters(const QString & queryToken,
                                   QString & dbField,
                                   QString & dbTable) const
{
    // Reset input vars
    dbField = "";
    dbTable = "";

    // Make sure token is not empty
    if(queryToken.isEmpty())
        return true;

    // Construct DB field and table strings
    QString lFullTableField = queryToken.section("|", 0, 0);
    int lLastDotIndex = lFullTableField.lastIndexOf('.');
    // minus 1 to remove the "."
    dbField = lFullTableField.right(lFullTableField.size() - lLastDotIndex - 1).toLower();
    dbTable = lFullTableField.left(lLastDotIndex).toLower();
    // remove schema name if needed
    if (dbTable.count(".") == 1)
        dbTable = dbTable.section(".", 1, 1);
    // not handled
    else if (dbTable.count(".") > 1)
        return false;

    return true;
}

void QueryEngine::RemoveDuplicates(
        QList<QPair<QString, DbTableDependency *> > &lDependencies) const
{
    int lListCount = lDependencies.count();
    int lListIndex = 0;
    QList<QPair<QString, DbTableDependency *> > lSeenItems;
    lSeenItems.reserve(lListCount);
    for (int i = 0; i < lListCount; ++i)
    {
        const QPair<QString, DbTableDependency *> &lDpdc = lDependencies.at(i);
        bool lIsContained = false;
        QPair<QString, DbTableDependency *> lDpdcSeen;
        foreach (lDpdcSeen, lSeenItems)
        {
            if (*lDpdcSeen.second == *lDpdc.second)
            {
                lIsContained = true;
                continue;
            }
        }
        if (lIsContained)
            continue;
        lSeenItems.append(lDpdc);
        if (lListIndex != i)
            lDependencies[lListIndex] = lDpdc;
        ++lListIndex;
    }
    if (lListCount != lListIndex)
        lDependencies.erase(lDependencies.begin() + lListIndex,
                            lDependencies.end());
}

void QueryEngine::RemoveRedundantTablesFromFilters(
        const QStringList &queryFieldsTables,
        QStringList &filtersTables) const
{
    foreach (const QString &lQueried, queryFieldsTables)
    {
        if (filtersTables.contains(lQueried))
            filtersTables.removeAll(lQueried);
    }
}

QList< QPair<QString, DbTableDependency*> > QueryEngine::GetFirstJoinSet(
        QStringList &queriedTables,
        QStringList &filteredTables,
        bool isFunctionalOnly)
{
    bool lFound = false;
    int lSolutionWeight;
    QMap< int , QList< QPair<QString, DbTableDependency*> > > lPossibleDpdcs;


    QList< QPair<QString, DbTableDependency*> > lDpdcs;
    // check filtered items compared to queried items
    foreach(const QString &queryItem, queriedTables)
    {
        foreach(const QString &filterItem, filteredTables)
        {
            if (queryItem != filterItem)
            {
                lSolutionWeight = 0;
                lDpdcs = DependenciesBetween(queryItem,
                                             filterItem,
                                             lSolutionWeight,
                                             isFunctionalOnly);
                if (!lDpdcs.isEmpty())
                {
                    lPossibleDpdcs.insertMulti(lSolutionWeight, lDpdcs);
                    lFound = true;
                }
            }
        }
    }
    // check queried items compared to queried items
    if (!lFound)
    {
        foreach(const QString &queryItem1, queriedTables)
        {
            foreach(const QString &queryItem2, queriedTables)
            {
                if (queryItem1 != queryItem2)
                {
                    lSolutionWeight = 0;
                    lDpdcs = DependenciesBetween(queryItem1,
                                                 queryItem2,
                                                 lSolutionWeight,
                                                 isFunctionalOnly);
                    if (!lDpdcs.isEmpty())
                    {
                        lPossibleDpdcs.insertMulti(lSolutionWeight, lDpdcs);
                        lFound = true;
                    }
                }
            }
        }
    }

    // Get best solution
    int lLowestCostSolution = 9999;
    lDpdcs.clear();
    QMapIterator< int , QList< QPair<QString, DbTableDependency*> > > lItSol(lPossibleDpdcs);
    while (lItSol.hasNext())
    {
        lItSol.next();
        if (lItSol.key() < lLowestCostSolution)
        {
            lLowestCostSolution = lItSol.key();
            lDpdcs = lItSol.value();
        }
    }

    QStringList lNewTablesInSet;
    QPair<QString, DbTableDependency *> lDepItem;
    foreach (lDepItem, lDpdcs)
        lNewTablesInSet << lDepItem.second->IncludedTables();
    lNewTablesInSet.removeDuplicates();

    foreach(const QString &lTable, lNewTablesInSet)
    {
        queriedTables.removeAll(lTable);
        filteredTables.removeAll(lTable);
    }

    return lDpdcs;
}

QList< QPair<QString, DbTableDependency*> > QueryEngine::JoinSetExtent(
        const QList<QPair<QString, DbTableDependency *> > &joinSet,
        QStringList &queriedTables,
        QStringList &filteredTables,
        bool functionalOnly)
{
    QList< QPair<QString, DbTableDependency*> > lNewDpdcs;
    QStringList lJoinedTables;
    QPair<QString, DbTableDependency *> lDepItem;
    // build list of joined
    foreach (lDepItem, joinSet)
        lJoinedTables << lDepItem.second->IncludedTables();
    lJoinedTables.removeDuplicates();

    QStringList lQueriedJoinedTables;
    QStringList lFilteredJoinedTables;
    QStringList lInitialQueriedTables = GetQueriedTables();
    foreach(const QString &table, lJoinedTables)
    {
        if (lInitialQueriedTables.contains(table))
            lQueriedJoinedTables.append(table);
        else
            lFilteredJoinedTables.append(table);
    }
    // Try to join one FILTER tables with the set
    lNewDpdcs = DependenciesBetween( lQueriedJoinedTables, filteredTables, functionalOnly);
    if (lNewDpdcs.isEmpty())
        lNewDpdcs = DependenciesBetween( lFilteredJoinedTables, filteredTables, functionalOnly);


    // Try to join one QUERY tables with the set
    if (lNewDpdcs.isEmpty())
        lNewDpdcs = DependenciesBetween( lQueriedJoinedTables, queriedTables, functionalOnly);
    if (lNewDpdcs.isEmpty())
        lNewDpdcs = DependenciesBetween( lFilteredJoinedTables, queriedTables, functionalOnly);

    QStringList lNewTablesInSet;
    foreach (lDepItem, lNewDpdcs)
        lNewTablesInSet << lDepItem.second->IncludedTables();
    lNewTablesInSet.removeDuplicates();

    foreach(const QString &lTable, lNewTablesInSet)
    {
        queriedTables.removeAll(lTable);
        filteredTables.removeAll(lTable);
    }

    return lNewDpdcs;
}

QString QueryEngine::NormalizeTableName(const QString &tableName)
{
    if (!mPrivate->mDbArchitecture)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, NULL pointer");
        return QString();
    }
    // Make sure table name is not empty
    if(tableName.isEmpty())
        return QString();

    QString lNormalizedTableName;
    // Add the schema specification, as we are logged in as a standard user,
    // unless the table already has a schema specification (ie external table referenced
    // through the metadata mapping feature)
    if(!mPrivate->mDbArchitecture->SchemaName().isEmpty() && !tableName.contains("."))
        lNormalizedTableName = mPrivate->mDbArchitecture->SchemaName() + "." + tableName;

    return lNormalizedTableName;
}

bool QueryEngine::BuildFunctionalGraph()
{
    if (!mPrivate->mDbArchitecture)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, NULL pointer");
        return false;
    }
    QMap<QString, DbTable> lTables = mPrivate->mDbArchitecture->GetTables();
    if (lTables.isEmpty())
        return false;

    mPrivate->mFunctionalGraph.SetVertices(lTables.keys());
    if (mPrivate->mFunctionalGraph.IsEmpty())
        return false;

    // Build graph
    QMapIterator<QString, DbTable> it(lTables);
    // for each table of the schema
    while (it.hasNext())
    {
        it.next();
        // Get all functional dependencies of the table
        QList<const DbTableDependency *> lDependencies = it.value().
                FunctionalDependencies();
        foreach (const DbTableDependency * lDependency, lDependencies)
        {
            QStringList lIncludedTables = lDependency->IncludedTables();
            // remove origin table of the dependency
            lIncludedTables.removeAll(it.key());
            foreach(const QString &lIncludedTable, lIncludedTables)
            {
                int lEdgeWeight = 4;
                // if 1 to 1 relationship
                DbTable lIncludedDbTable = lTables.value(lIncludedTable);
                if (lIncludedDbTable.HasFunctionalDependencyWith(it.key()))
                    lEdgeWeight = 1;
                mPrivate->mFunctionalGraph.AddEdge(it.key(), lIncludedTable, lEdgeWeight);
            }
        }
    }

    return true;
}

bool QueryEngine::BuildDepenciesGraph()
{
    if (!mPrivate->mDbArchitecture)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, NULL pointer");
        return false;
    }
    QMap<QString, DbTable> lTables = mPrivate->mDbArchitecture->GetTables();
    if (lTables.isEmpty())
        return false;

    mPrivate->mDependenciesGraph.SetVertices(lTables.keys());
    if (mPrivate->mDependenciesGraph.IsEmpty())
        return false;

    // Build graph
    QMapIterator<QString, DbTable> it(lTables);
    // for each table of the schema
    while (it.hasNext())
    {
        it.next();
        // Get all functional dependencies of the table
        QList<const DbTableDependency *> lDependencies = it.value().Dependencies();
        foreach (const DbTableDependency * lDependency, lDependencies)
        {
            QStringList lIncludedTables = lDependency->IncludedTables();
            // remove origin table of the dependency
            lIncludedTables.removeAll(it.key());
            foreach(const QString &lIncludedTable, lIncludedTables)
            {
                mPrivate->mDependenciesGraph.AddEdge(it.key(), lIncludedTable, 4);
                mPrivate->mDependenciesGraph.AddEdge(lIncludedTable, it.key(), 4);
            }
        }
    }

    return true;
}

QList<QPair<QString, DbTableDependency *> > QueryEngine::DependenciesBetween(
        const QString &tableFrom,
        const QString &tableTo,
        int &solutionWeight,
        bool functionalOnly) const
{
    if (functionalOnly)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
           QString("Search link from %1 to %2 using functional dependencies ").
           arg(tableFrom).
           arg(tableTo).
           toLatin1().data());
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG,
           QString("Search link from %1 to %2 ignoring functional dependencies ").
           arg(tableFrom).
           arg(tableTo).
           toLatin1().data());
    }

    QList<QPair<QString, DbTableDependency*> > lMatchingTables;

    if (!mPrivate->mDbArchitecture)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, NULL pointer");
        return lMatchingTables;
    }

    if (mPrivate->mFunctionalGraph.IsEmpty())
        return lMatchingTables;

    GS::DbPluginBase::Dijkstra lPathFinder;
    // Get the shortest path from one table to another
    if (functionalOnly)
        lPathFinder.SetGraph(mPrivate->mFunctionalGraph);
    else
        lPathFinder.SetGraph(mPrivate->mDependenciesGraph);

    QStringList lShortestPath = lPathFinder.ShortestPath(tableFrom, tableTo, solutionWeight);

    QString lCurrentTableName, lNextTableName;
    if (!lShortestPath.isEmpty())
    {
        lCurrentTableName = lShortestPath.first();
        for (int i = 1; i < lShortestPath.count(); ++i)
        {
            lNextTableName = lShortestPath.at(i);
            DbTable* lCurrentTable = mPrivate->mDbArchitecture->GetTable(lCurrentTableName);
            if (lCurrentTable)
            {
                 QList<QPair<QString, DbTableDependency*> > lDependencies;
                 lDependencies = lCurrentTable->DependenciesWith(
                                                 QStringList(lNextTableName),
                                                 functionalOnly);
                 lMatchingTables.append(lDependencies);
            }
            else
                GSLOG(SYSLOG_SEV_ERROR, QString("Table %1 not found").
                       arg(lCurrentTableName).toLatin1().data());
            lCurrentTableName = lNextTableName;
        }
    }

    return lMatchingTables;
}

QList<QPair<QString, DbTableDependency *> > QueryEngine::DependenciesBetween(
        const QStringList &tablesFrom,
        const QStringList &tablesTo,
        bool functionalOnly) const
{
    QList<QPair<QString, DbTableDependency *> > lNewDpdcs;

    foreach(const QString &lTableToItem, tablesTo)
    {
        foreach (const QString &lTableFromItem, tablesFrom)
        {
            int lSolWeight = 0;
            lNewDpdcs = DependenciesBetween(
                        lTableFromItem,
                        lTableToItem,
                        lSolWeight,
                        functionalOnly);
            if (!lNewDpdcs.isEmpty())
                return lNewDpdcs;
        }
    }

    return lNewDpdcs;
}

QStringList QueryEngine::GetUnlinkedTables(
        QStringList tableToLink,
        QList<QPair<QString, DbTableDependency *> > dependencies) const
{
    QStringList lUnlinkedTable = tableToLink;
    QPair<QString, DbTableDependency *> lDpdc;
    foreach(lDpdc, dependencies)
    {
        QStringList lLinkedTables = lDpdc.second->IncludedTables();
        // Remove linked table
        foreach(const QString &lLinked, lLinkedTables)
            lUnlinkedTable.removeAll(lLinked);
    }

    return lUnlinkedTable;
}

QStringList QueryEngine::GetFilteredTables() const
{
    QStringList lFilteredTables;
    for (int i = 0; i < mPrivate->mFilters.count(); ++i)
    {
        QString lGexElt, lField, lTable;
        lGexElt = mPrivate->mFilters[i];
        if (!ExtractFromFilters(lGexElt, lField, lTable))
            return QStringList();
        if (!lFilteredTables.contains(lTable))
            lFilteredTables.append(lTable);
    }

    return lFilteredTables;
}

QStringList QueryEngine::GetFilteredTablesOnPK() const
{
    QStringList lFilteredTablesOnPK;
    if (! mPrivate->mDbArchitecture)

    {
        GSLOG(SYSLOG_SEV_ERROR, "Architecture not loaded, NULL pointer");
        return lFilteredTablesOnPK;
    }
    QMap<QString, QStringList> lTableFilters;
    // retrieve all filters
    for (int i = 0; i < mPrivate->mFilters.count(); ++i)
    {
        QString lGexElt, lField, lTable;
        lGexElt = mPrivate->mFilters[i];
        if (!ExtractFromFilters(lGexElt, lField, lTable))
            return QStringList();
        QStringList lCurrentFilters = lTableFilters.value(lTable);
        lCurrentFilters << lField;
        lTableFilters.insert(lTable, lCurrentFilters);
    }
    // for each table check if pk is included
    QMapIterator<QString, QStringList> lTableIter(lTableFilters);
    while (lTableIter.hasNext())
    {
        lTableIter.next();
        DbTable* lTable = mPrivate->mDbArchitecture->GetTable(lTableIter.key());
        if (!lTable)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unable to find table %1").
                   arg(lTableIter.key()).toLatin1().data());
            continue;
        }

        if (lTable->IsPkIncludedIn(lTableIter.value()) &&
                !lFilteredTablesOnPK.contains(lTableIter.key()))
            lFilteredTablesOnPK.append(lTableIter.key());
    }

    return lFilteredTablesOnPK;
}

QStringList QueryEngine::GetQueriedTables() const
{
    QStringList lQueriedTables;
    for (int i = 0; i < mPrivate->mQueryFields.count(); ++i)
    {
        QStringList lGexElt = mPrivate->mQueryFields[i].split("|");
        QString lTable = lGexElt[1];
        lTable = lTable.section(QString("."), 0, 0);
        if (!lQueriedTables.contains(lTable))
            lQueriedTables.append(lTable);
    }

    return lQueriedTables;
}

bool QueryEngine::RemoveUselessDep(QList<QPair<QString, DbTableDependency *> > &lJoinSet) const
{
    bool lDepRemoved = false;
    // Extract tables with filters on PK
    QStringList lFilteredTablesOnPK = GetFilteredTablesOnPK();
    mPrivate->mFinalTableListToQuery.clear();

    if (lFilteredTablesOnPK.isEmpty())
        return false;

    // Get dependencies where these tables are involved
    QStringList lTableToRemove = GetUselessTableLinkOn(lFilteredTablesOnPK, lJoinSet);
    if (!lTableToRemove.isEmpty())
    {
        // for each dependency if it contains only table not to remove: remove dependency
        for (int lIndex = 0; lIndex < lJoinSet.size(); ++lIndex)
        {
            QPair<QString, DbTableDependency *> lDep = lJoinSet.at(lIndex);
            int lUselessCount = 0;
            QStringList lIncluded = lDep.second->IncludedTables();
            foreach (const QString &lTable, lIncluded)
            {
                if (lTableToRemove.contains(lTable))
                    ++lUselessCount;
}
            // If one table is connected only to table to remove, then remove dependency
            if ((lUselessCount > 0) && (lUselessCount >= (lIncluded.size() - 1)))
            {
                lJoinSet.removeAt(lIndex);
                lDepRemoved = true;
                lIndex = std::max(lIndex - 1, 0);
            }
        }
    }

    // Retrieve final table list to query
    for (int lIndex = 0; lIndex < lJoinSet.size(); ++lIndex)
{
        QPair<QString, DbTableDependency *> lDep = lJoinSet.at(lIndex);
        mPrivate->mFinalTableListToQuery.append(lDep.second->IncludedTables());
}
    mPrivate->mFinalTableListToQuery.removeDuplicates();

    return lDepRemoved;
}


QStringList QueryEngine::GetUselessTableLinkOn(const QStringList &tablesToCheck,
                                         const QList<QPair<QString, DbTableDependency *> > &lJoinSet) const
{
    AdjacencyGraph lJoinGraph = BuildJoinSetGraph(lJoinSet);
    Dijkstra lPathFinder;
    lPathFinder.SetGraph(lJoinGraph);

    QStringList lTableToRemove;
    QStringList lQueriedTables = GetQueriedTables();
    QStringList lJoinedTables = lJoinGraph.GetVertices();
    // Remove queried tables from joined tables as they can't be removed (we need them)
    foreach(const QString &lQueriedTable, lQueriedTables)
        lJoinedTables.removeAll(lQueriedTable);
    // Remove pointed table from tables to check (we don't want to remove them here)
    foreach(const QString &lTable, tablesToCheck)
        lJoinedTables.removeAll(lTable);

    // For all tables to check
    foreach (const QString &lJoinedTable, lJoinedTables)
    {
        bool lIsPkTableUsed = true;
        foreach (const QString &lQueriedTable, lQueriedTables)
        {
            // get path from joined table to queried
            int lWeight = 0;
            QStringList lPathTables = lPathFinder.ShortestPath(lQueriedTable, lJoinedTable, lWeight);
            // check if path include tables to check
            foreach (const QString &lTable, tablesToCheck)
            {
                if (!lPathTables.contains(lTable))
                    lIsPkTableUsed = false;
}
        }
        // if always pass by pk table: remove from query
        if (lIsPkTableUsed)
            lTableToRemove.append(lJoinedTable);

    }

    return lTableToRemove;
}


AdjacencyGraph QueryEngine::BuildJoinSetGraph(const QList<QPair<QString, DbTableDependency *> > &lJoinSet) const
{
    AdjacencyGraph lJoinGraph;

    // Set vertices
    QStringList lVertices;
    for (int lDepIndex = 0; lDepIndex < lJoinSet.size(); ++lDepIndex)
        lVertices << lJoinSet.at(lDepIndex).second->IncludedTables();

    lVertices.removeDuplicates();
    lJoinGraph.SetVertices(lVertices);

    // Set edges
    for (int lDepIndex = 0; lDepIndex < lJoinSet.size(); ++lDepIndex)
    {
        QList<DbTableLink> lTableLinks = lJoinSet.at(lDepIndex).second->TableLinks();
        foreach (const DbTableLink &lTableLink, lTableLinks)
        {
            lJoinGraph.AddEdge( lTableLink.Table1(), lTableLink.Table2(), 1);
            lJoinGraph.AddEdge( lTableLink.Table2(), lTableLink.Table1(), 1);
    }
    }

    return lJoinGraph;
}

void QueryEngine::SetFormatedFilters(const QStringList &filters)
{
    mPrivate->mFilters = filters;
}

void QueryEngine::SetFormatedQueryFields(const QStringList &fields)
{
    mPrivate->mQueryFields = fields;
}

void QueryEngine::SetBuildingRule(const QueryEngine::BuildingRule rule)
{
    mPrivate->mBuildingRule = rule;
}

QStringList QueryEngine::SimplifiedValueConditions()
{
    QStringList lSimplifiedValueCondictions;
    foreach (const QString &lFilter, mPrivate->mFilters)
    {
        QString lGexElt, lField, lTable;
        lGexElt = lFilter;
        if (ExtractFromFilters(lGexElt, lField, lTable))
        {
            if (mPrivate->mFinalTableListToQuery.contains(lTable))
                lSimplifiedValueCondictions.append(lFilter);
        }
        else
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Unable to extract field and/or table from %1").
                  arg(lGexElt).toLatin1().constData());
    }

    return lSimplifiedValueCondictions;
}

bool QueryEngine::BuildQuery()
{
    mPrivate->mValidQuery = false;

    QList< QPair<QString, DbTableDependency*> > lDependencies;

    QStringList lFilteredTables = GetFilteredTables();
    QStringList lQueriedTables = GetQueriedTables();

    if ((lQueriedTables.count() == 1) && (lFilteredTables.count() == 0))
    {
        mPrivate->mFromClause = NormalizeTableName(lQueriedTables.at(0));
        return true;
    }
    else if ((lQueriedTables.count() == 1) &&
             (lFilteredTables.count() == 1) &&
             (lQueriedTables.at(0) == lFilteredTables.at(0)))
    {
        mPrivate->mFromClause = NormalizeTableName(lQueriedTables.at(0));
        return true;
    }

    if (!BuildOrderedDependenciesList(lDependencies))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Query not built, "
               "unable to build dependencies list");
        return false;
    }

    if (!BuildClauses(lDependencies))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Query not built");
        return false;
    }

    mPrivate->mValidQuery = true;

    return true;
}

bool QueryEngine::ValidQuery() const
{
    return mPrivate->mValidQuery;
}

QString QueryEngine::WhereClause() const
{
    return mPrivate->mWhereClause;
}

QString QueryEngine::JoinClause(
        JointureType join/*= DbQueryEngine::INNER*/) const
{
    QString lJoinClause = mPrivate->mJoinClause;
    if (join == QueryEngine::INNER)
        lJoinClause = lJoinClause.replace(QString(JOINTURE_TYPE_PLACEHOLDER),
                                          "INNER");
    else if (join == QueryEngine::LEFTOUTER)
        lJoinClause = lJoinClause.replace(QString(JOINTURE_TYPE_PLACEHOLDER),
                                          "LEFT OUTER");
    else
        GSLOG(SYSLOG_SEV_ERROR, "Unknown jointure type");

    return lJoinClause;
}

QString QueryEngine::FromClause() const
{
    return mPrivate->mFromClause;
}

bool QueryEngine::BuildOrderedDependenciesList(
        QList< QPair<QString, DbTableDependency*> > &lDependencies)
{
    if (!BuildJoinTableSet(lDependencies))
        return false;

    GSLOG(SYSLOG_SEV_DEBUG, "LINK(S):");
    for (int i = 0; i < lDependencies.count(); ++i)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Dep #%1: %2").
               arg(i).
               arg(lDependencies.at(i).second->IncludedTables().join(",")).
               toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
               QString(lDependencies.at(i).second->NormalizedTablesLinks()
                       .join("\n")).toLatin1().data());
    }

    return true;
}

QString QueryEngine::GetJoinedTable(QStringList lJoinedTables,
                                      DbTableDependency *dependency) const
{
    QString lJoinedTable;
    QList<DbTableLink> lLinks = dependency->TableLinks();
    foreach(const QString &lTableName, lJoinedTables)
    {
        bool lIsInEachLink = true;
        foreach(const DbTableLink &lLink, lLinks)
        {
            if (lTableName != lLink.Table1() && lTableName != lLink.Table2())
                lIsInEachLink = false;
        }
        if (lIsInEachLink)
            lJoinedTable = lTableName;
    }

    return lJoinedTable;
}

bool QueryEngine::BuildClauses(const QList<QPair<QString, DbTableDependency *> > lDependencies)
{
    if (!BuildWhereClause(lDependencies))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Query not built, "
               "unable to build where clause");
        return false;
    }

    if (!BuildJoinClause(lDependencies))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Query not built, "
               "unable to build join clause");
        return false;
    }
    return true;
}

bool QueryEngine::BuildJoinClause(
        const QList< QPair<QString, DbTableDependency*> > lDependencies)
{
    mPrivate->mJoinClause = "";
    if (lDependencies.isEmpty())
        return false;

    QStringList lJoinedTables;
    QList<DbTableLink> lAddedLinks;
    mPrivate->mJoinClause = NormalizeTableName(lDependencies.first().first);
    lJoinedTables << lDependencies.first().first;
    QPair<QString, DbTableDependency*> lDependency;

    foreach(lDependency, lDependencies)
    {
        mPrivate->mJoinClause += " " + QString(JOINTURE_TYPE_PLACEHOLDER) +" JOIN ";
        QString lJoinedTable = GetJoinedTable(lJoinedTables, lDependency.second);
        QList<DbTableLink> lTableLinks = lDependency.second->TableLinks();
        foreach(const DbTableLink &lLink, lTableLinks)
        {
            if (!lAddedLinks.contains(lLink))
            {
                QString lTable2;
                if (lLink.Table1() == lJoinedTable)
                    lTable2 = lLink.Table2();
                else
                    lTable2 = lLink.Table1();
                mPrivate->mJoinClause += " " + NormalizeTableName(lTable2) + " ON \n";
                mPrivate->mJoinClause += lLink.Conditions() + "\n";
                lAddedLinks.append(lLink);
            }
        }
        lJoinedTables.append(lDependency.second->IncludedTables());
        lJoinedTables.removeDuplicates();
    }

    return true;
}

bool QueryEngine::BuildWhereClause(
        const QList< QPair<QString, DbTableDependency*> > lDependencies)
{
    mPrivate->mWhereClause = "";
    mPrivate->mFromClause = "";
    if (lDependencies.isEmpty())
        return false;

    QStringList lJoinedTables;
    QList<DbTableLink> lAddedLinks;
    lJoinedTables << lDependencies.first().first;
    QPair<QString, DbTableDependency*> lDependency;
    foreach(lDependency, lDependencies)
    {
        QList<DbTableLink> lTableLinks = lDependency.second->TableLinks();
        foreach(const DbTableLink &lLink, lTableLinks)
        {
            if (!lAddedLinks.contains(lLink))
            {
                if (!mPrivate->mWhereClause.isEmpty())
                    mPrivate->mWhereClause += " AND ";
                mPrivate->mWhereClause += lLink.Conditions() + "\n";
                lAddedLinks.append(lLink);
            }
        }
        lJoinedTables.append(lDependency.second->IncludedTables());
        lJoinedTables.removeDuplicates();
    }

    // build from by adding schema name
    foreach(const QString &table, lJoinedTables)
        mPrivate->mFromClause += NormalizeTableName(table) + "\n,";
    mPrivate->mFromClause.chop(1); // remove last ","

    return true;
}

bool QueryEngine::GetWhereClauseFilter()
{
    /// CODE ME
    return true;
}

bool QueryEngine::BuildJoinTableSet(
        QList< QPair<QString, DbTableDependency*> > &lJoinSet)
{
    QStringList lQueriedTables = GetQueriedTables();
    QStringList lFilteredTables = GetFilteredTables();
    lJoinSet.clear();
    QList< QPair<QString, DbTableDependency*> > lFunctionalJoinSet;
    QList< QPair<QString, DbTableDependency*> > lNotFunctionalJoinSet;
    QList< QPair<QString, DbTableDependency*> > lJoinSetExtent;
    bool lFailed = false;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Table(s) to query: %1").
           arg(lQueriedTables.join(",")).toLatin1().data());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Table(s) to filter: %1").
           arg(lFilteredTables.join(",")).toLatin1().data());

    QueryEngine::BuildingRule lBuildingRule = static_cast<BuildingRule>(mPrivate->mBuildingRule);

    if (lBuildingRule == QueryEngine::UseFunctionalDep)
        lFunctionalJoinSet = GetFirstJoinSet(lQueriedTables, lFilteredTables, true);
    if (lFunctionalJoinSet.isEmpty())
    {
        lNotFunctionalJoinSet = GetFirstJoinSet(lQueriedTables, lFilteredTables, false);
        if (lNotFunctionalJoinSet.isEmpty())
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unable to build initial tables set").toLatin1().data());
            lFailed = true;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("First tables set built").toLatin1().data());

    while (!(lQueriedTables.isEmpty() && lFilteredTables.isEmpty()) && !lFailed)
    {
        // Search functional dependencies with functional set found
        if (lBuildingRule == QueryEngine::UseFunctionalDep)
            lJoinSetExtent = JoinSetExtent(lFunctionalJoinSet, lQueriedTables, lFilteredTables, true);
        if (lJoinSetExtent.isEmpty())
        {
            // Search non functional dependencies with functional set first
            lJoinSetExtent = JoinSetExtent(lFunctionalJoinSet, lQueriedTables, lFilteredTables, false);
            if (lJoinSetExtent.isEmpty())
                // Search non functional dependencies with non functional set
                lJoinSetExtent = JoinSetExtent(lNotFunctionalJoinSet, lQueriedTables, lFilteredTables, false);

            if (lJoinSetExtent.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Unable to link %1, %2 with current tables set").
                       arg(lQueriedTables.join(", ")).
                       arg(lFilteredTables.join(", ")).toLatin1().data());
                lFailed = true;
            }
            else // Dependencies (not functional) found
                lNotFunctionalJoinSet.append(lJoinSetExtent);
        }
        else
        // Functional dependencies found
            lFunctionalJoinSet.append(lJoinSetExtent);

        lJoinSetExtent.clear();
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Full tables set built").toLatin1().data());
    lJoinSet.append(lFunctionalJoinSet);
    lJoinSet.append(lNotFunctionalJoinSet);
    RemoveDuplicates(lJoinSet);

    // Remove useless filters here if needed
    RemoveUselessDep(lJoinSet);

    return !lFailed;
}

} //END namespace DbPluginBase
} //END namespace GS

