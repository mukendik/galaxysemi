#ifndef DB_TABLE_DEPENDENCY_H
#define DB_TABLE_DEPENDENCY_H

#include <QObject>
#include <QDomElement>
#include <QMap>

#include "db_table_link.h"

namespace GS
{
namespace DbPluginBase
{

class DbTableDependency: public QObject
{
    Q_OBJECT
public:
    /// \brief Constructor
    DbTableDependency(QObject *parent = 0);
    /// \brief Destructor
    virtual ~DbTableDependency();
    /// \brief Copy constructor
    DbTableDependency(const DbTableDependency &source);
    /// \brief = operator
    DbTableDependency & operator=(const DbTableDependency &source);
    /// \brief == operator
    bool operator==(const DbTableDependency &source) const;
    /// \brief != operator
    bool operator!=(const DbTableDependency &source) const;

    /// \brief load dependency from dom element
    bool                        LoadFromDom(const QDomElement &node);
    /// \brief load table link id from dom element
    bool                        LoadTableLinkIdsFromDom(const QDomElement &node);
    /// \brief add table link to dependency
    void                        AddTableLink(const DbTableLink &tableLink);
    /// \brief load table link list content
    bool                        LoadTableLinksContent(const QMap<QString, DbTableLink> &tablesLinks);
    /// \brief true if the dependency is functional
    bool                        IsFunctional() const;
    /// \brief true if the dependency contains the table
    bool                        Contains(const QString& table) const;
    /// \brief true if the dependency is with one table
    bool                        IsSimple() const;
    /// \brief true if the dependency is on several tables
    bool                        IsMultiple() const;
    /// \brief number of link in the dependency
    bool                        LinkCount() const;
    /// \brief tablesincluded in the dependency
    QStringList                 IncludedTables() const;
    /// \brief list of table links in normalized format
    QStringList                 NormalizedTablesLinks() const;
    /// \brief list of table links
    const QList<DbTableLink>&   TableLinks() const;
    /// \brief return true if each link contains table
    bool IsValidDependencyOfTable(const QString &tableName);

private:
    /// \brief clear table links list
    void                ClearTableLink();
    /// \brief check if node exist and log otherwise
    bool                NodeIsValid(const QDomElement &node) const;

    bool                mIsFunctional; ///< holds true if the dependency is functional
    QList<DbTableLink>  mTableLinks;   ///< holds the list of table links
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // DB_TABLE_DEPENDENCY_H
