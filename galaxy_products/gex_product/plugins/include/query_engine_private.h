#ifndef QUERY_ENGINE_PRIVATE_H
#define QUERY_ENGINE_PRIVATE_H

#include <QObject>
#include <QStringList>
#include "adjacencygraph.h"


namespace GS
{
namespace DbPluginBase
{

class DbArchitecture;

class QueryEnginePrivate: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QueryEnginePrivate)
public:
    QueryEnginePrivate();
    virtual ~QueryEnginePrivate();

    DbArchitecture          *mDbArchitecture;       ///< holds the database architecture
    QStringList             mTargetedTables;        ///< holds targeted tables
    AdjacencyGraph          mFunctionalGraph;       ///< holds the graph of functional dependencies
    AdjacencyGraph          mDependenciesGraph;     ///< holds the graph of all dependencies
    QStringList             mQueryFields;           ///< holds fields to query
    QStringList             mFilters;               ///< holds filters of the query
    QString                 mWhereClause;           ///< holds SQL where clause
    QStringList             mFinalTableListToQuery; ///< holds list of table to query after having removed useless tables
    QString                 mJoinClause;            ///< holds SQL join clause
    QString                 mFromClause;            ///< holds SQL from clause
    bool                    mValidQuery;            ///< holds is query is valid or not
    int                     mBuildingRule;          ///< holds build rule
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // QUERY_ENGINE_PRIVATE_H
