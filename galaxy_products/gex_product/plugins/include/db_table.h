#ifndef DB_TABLE_H
#define DB_TABLE_H

#include <QObject>
#include <QDomElement>
#include <QPair>
#include <QStringList>

#include "db_table_dependency.h"
#include "db_table_link.h"


namespace GS
{
namespace DbPluginBase
{

class DbTable : public QObject
{
    Q_OBJECT

public:
    enum TestingStage
    {
        ANY,
        ETEST,
        WSORT,
        FTEST
    };
    /// \brief Constructor
    DbTable(QObject *parent = 0);
    /// \brief Destructor
    virtual ~DbTable();
    /// \brief Copy constructor
    DbTable(const DbTable &source);
    /// \brief = operator
    DbTable & operator=(const DbTable &source);
    /// \return table name
    const QString                       Name() const;
    /// \return primary key field list
    const QStringList                   PrimaryKey() const;
    /// \return true if the primary key field list is equal
    bool                                IsPkIncludedIn(const QStringList &key) const;
    /// \brief define the testing stage of the table
    void                                SetTestingStage(const QString &testingStage);
    /// \brief define the name of the table
    void                                SetName(const QString &name);
    /// \brief add a dependency to the table
    bool                                AddDependency(const DbTableDependency &dependency);
    /// \brief Load the table from a dom element
    bool                                LoadFromDom(const QDomElement &node);
    /// \brief load table dependencies from a dom element
    bool                                LoadDependenciesFromDom(const QDomElement &node);
    /// \brief load table primary key from a dom element
    bool                                LoadPrimaryKeyFromDom(const QDomElement &node);
    /// \brief load table links content
    bool                                LoadTableLinksContent(const QMap<QString, DbTableLink> &tablesLinks);
    /// \brief true if the table as some dependencies with
    bool                                HasDependencyWith(const QString &table);
    /// \brief true if the table as some functional dependencies with
    bool                                HasFunctionalDependencyWith(const QString &table);
    /// \brief true if the table as ONE functional dependencies with
    bool                                HasSimpleFDWith(const QString &table);
    /// \brief true if the table as SEVERAL functional dependencies with
    bool                                HasMultipleFDWith(const QString &table);
    /// \brief return non functional dependencies with
    QList<DbTableDependency *>          NonFDsWith(const QString &table);
    /// \brief return simple functional dependencies with
    QList<DbTableDependency *>          SimpleFDsWith(const QString &table);
    /// \brief return multiple functional depenecies with
    QList<DbTableDependency *>          MultipleFDsWith(const QString &table);
    /// \brief return all dependencies with
    QList<DbTableDependency *>          DependenciesWith(const QString &table,
                                                         bool functionalOnly);
    /// \brief similar to homonyme but with a set of table
    QList<QPair<QString, DbTableDependency *> >   NonFDsWith(const QStringList &tables);
    /// \brief similar to homonyme but with a set of table
    QList<QPair<QString, DbTableDependency *> > SimpleFDsWith(const QStringList &tables);
    /// \brief similar to homonyme but with a set of table
    QList<QPair<QString, DbTableDependency *> > MultipleFDsWith(const QStringList &tables);
    /// \brief similar to homonyme but with a set of table
    QList<QPair<QString, DbTableDependency *> >   FDsWith(const QStringList &tables);
    /// \brief similar to homonyme but with a set of table
    QList<QPair<QString, DbTableDependency *> >   DependenciesWith(const QStringList &tables,
                                                                   bool functionalOnly);
    /// \brief return all functional dependencies
    QList<const DbTableDependency *>    FunctionalDependencies() const;
    /// \brief return all depdencies
    QList<const DbTableDependency *>    Dependencies() const;
    
private:
    /// \brief clear dependencies list
    void                        ClearDependencies();
    /// \brief check if node exist and log otherwise
    bool                        NodeIsValid(const QDomElement &node) const;

    QString                     mName;          ///< holds table name
    QString                     mDatabaseName;  ///< holds table database name
    TestingStage                mTestingStage;  ///< holds table testing stage
    QStringList                 mPrimaryKey;    ///< holds list of fields which compose the primary key
    QList<DbTableDependency>    mDependencies;  ///< holds table dependencies list
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // DB_TABLE_H
