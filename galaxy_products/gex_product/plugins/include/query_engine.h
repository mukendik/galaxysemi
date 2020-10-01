#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

// Qt
#include <QObject>
#include <QPair>

namespace GS
{
namespace DbPluginBase
{

class DbTableDependency;
class DbArchitecture;
class QueryEnginePrivate;
class AdjacencyGraph;

class QueryEngine : public QObject
{
    Q_OBJECT
public:

    enum JointureType
    {
        INNER,
        LEFTOUTER
    };

    enum BuildingRule
    {
        UseFunctionalDep,
        IgnoreFunctionalDep
    };

    /// \brief Constructor
    QueryEngine(QObject *parent = 0);
    /// \brief Destructor
    virtual ~QueryEngine();
    /// \brief Reset
    void ResetQuery();
    /// \brief set the database architecture
    bool SetDbArchitecture(DbArchitecture *dbArchitecture);
    /// \brief set filters of the query
    void SetFormatedFilters(const QStringList &filters);
    /// \brief set fileds to be queried
    void SetFormatedQueryFields(const QStringList &fields);
    /// \brief set Building rule
    void SetBuildingRule(const BuildingRule rule);
    /// \brief remove value condition on tables that are not included in the final query list
    QStringList SimplifiedValueConditions();
    /// \brief build query
    bool BuildQuery();
    /// \brief true if query succesfully built
    bool ValidQuery() const;
    /// \brief return where clause
    QString WhereClause() const;
    /// \brief return join clause
    QString JoinClause(JointureType join = QueryEngine::INNER) const;
    /// \brief return from clause
    QString FromClause() const;

private:
    Q_DISABLE_COPY(QueryEngine);
    /// \brief build the graph of functional dependencies
    bool                                        BuildFunctionalGraph();
    /// \brief build the graph of all dependencies
    bool                                        BuildDepenciesGraph();
    /// \brief map of depencies between tablefrom and tableto (functional or not)
    QList<QPair<QString, DbTableDependency *> > DependenciesBetween(
                                                const QString &tableFrom,
                                                const QString &tableTo,
                                                int &solutionWeight,
                                                bool functionalOnly) const;
    QList<QPair<QString, DbTableDependency *> > DependenciesBetween(
                                                const QStringList &tablesFrom,
                                                const QStringList &tablesTo,
                                                bool functionalOnly) const;
    /// \brief return list of tables from table to link not included in links
    QStringList GetUnlinkedTables(
            QStringList tableToLink,
            QList<QPair<QString, DbTableDependency *> > dependencies) const;
    /// \brief build dependencies list knowing filters and fields to query
    bool    BuildOrderedDependenciesList(
            QList<QPair<QString, DbTableDependency *> > &lDependencies);
    /// \brief return the table from the list which is in each db table link of the dependency
    QString GetJoinedTable(
            QStringList lJoinedTables,
            DbTableDependency* dependency) const;
    bool BuildClauses(
            const QList<QPair<QString, DbTableDependency *> > lDependencies);
    /// \brief return join clause knowing filters and fields to query
    bool BuildJoinClause(
            const QList<QPair<QString, DbTableDependency *> > lDependencies);
    /// \brief return where clause knowing filters and fields to query
    bool BuildWhereClause(
            const QList<QPair<QString, DbTableDependency *> > lDependencies);
    /// \brief return filter part of the where clause
    bool GetWhereClauseFilter();
    /// \brief Query: normalize table name
    QString NormalizeTableName(const QString & tableName);
    /// \brief Query: extract normalized field name and table name from
    // the DB field specifier retrieved from the mapping
    // Example:
    // wt_lot.product_name
    // MySQL/OCI:	field = wt_lot.product_name
    //				table = wt_lot
    // GEXDB.wt_lot.product_name
    // MySQL/OCI:	field = GEXDB.wt_lot.product_name
    //				table = GEXDB.wt_lot
    bool ExtractFromFilters(const QString & queryToken,
                        QString & dbField,
                        QString & dbTable) const;
    /// \brief remove duplicate dependencies
    void RemoveDuplicates(
            QList<QPair<QString, DbTableDependency *> > &lDependencies) const;
    /// \brief remove from filter tables list table which are already in queried tables list
    void RemoveRedundantTablesFromFilters(
            const QStringList &queryFieldsTables,
            QStringList &filtersTables) const;
    /// \brief build and return the first table jointure used to build the query
    QList< QPair<QString, DbTableDependency*> > GetFirstJoinSet(
            QStringList &queriedTables,
            QStringList &filteredTables,
            bool isFunctionalOnly);
    /// \brief build an extent to current table jointure
    QList< QPair<QString, DbTableDependency*> > JoinSetExtent(
            const QList<QPair<QString, DbTableDependency *> > &joinSet,
            QStringList &queriedTables,
            QStringList &filteredTables,
            bool functionalOnly);
    /// \brief build the full jointure used for the query if possible
    bool BuildJoinTableSet(QList<QPair<QString, DbTableDependency *> > &lJoinSet);
    /// \return list filtered tables in the query
    QStringList GetFilteredTables() const;
    /// \return list of filtered tables on primary key
    QStringList GetFilteredTablesOnPK() const;
    /// \return list queried tables in the query
    QStringList GetQueriedTables() const;
    /// \return true if useless dep for the query are removed (which does not give more info)
    bool        RemoveUselessDep(QList<QPair<QString, DbTableDependency *> > &lJoinSet) const;
    /// \return return useless dep (connected to table) for the query (which does not give more info)
    QStringList GetUselessTableLinkOn(const QStringList &tablesToCheck,
                                const QList<QPair<QString, DbTableDependency *> > &lJoinSet) const;
    /// \brief build the graph of the jointure
    AdjacencyGraph BuildJoinSetGraph(const QList<QPair<QString, DbTableDependency *> > &lJoinSet) const;

    QueryEnginePrivate      *mPrivate;          ///< holds pointer to private members
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // QUERY_ENGINE_H
