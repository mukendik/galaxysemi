#ifndef DB_ARCHITECTURE_H
#define DB_ARCHITECTURE_H

#include <QObject>
#include <QDomElement>
#include <QMap>

#include "db_table.h"
#include "db_table_link.h"
#include "gexdb_plugin_base.h"

namespace GS
{
namespace DbPluginBase
{

class DbTable;
class DbTableLink;

class DbArchitecture : public QObject
{
    Q_OBJECT
public:
    /// \brief Constructor
    DbArchitecture(QString schemaName, QObject *parent = 0);
    /// \brief Destructor
    virtual ~DbArchitecture();
    /// \brief Load architecture from file
    bool                            LoadFromFile(const QString &filePath);
    /// \brief load architecture from dom element
    bool                            LoadFromDom(QDomDocument &domDocument);
    /// \brief return map of tables
    const QMap<QString, DbTable>&   GetTables();
    /// \brief return table object
    DbTable*                        GetTable(const QString &table);
    /// \brief add link between two table and one field using legacy link struct
    bool                            AddSimpleLinkFromLegacy(
            const GexDbPlugin_Mapping_Link &mappingLink,
            const QString &testingStage);
    /// \brief add link between two table on several keys using legacy link struct
    bool                            AddMultipleKeyLinkFromLegacy(
            const QList<GexDbPlugin_Mapping_Link> &mappingLinks,
            const QString &testingStage);
    /// \brief add link between several table on several keys using legacy link struct
    /// will only work if beween each table on ly one link exists
    bool                            AddMultipleTableLinkFromLegacy(
            const QList<GexDbPlugin_Mapping_Link> &mappingLinks,
            const QString &testingStage);
    QString                         SchemaName() const;
    
private:
    Q_DISABLE_COPY(DbArchitecture);
    /// \brief clear table list
    void ClearTables();
    /// \brief clear table link list
    void ClearTableLinks();
    /// \brief load tables from dom elements
    bool LoadTablesFromDom(const QDomElement &node);
    /// \brief load table links from dom elements
    bool LoadTableLinksFromDom(const QDomElement &node);
    /// \brief check if node exist and log otherwise
    bool NodeIsValid(const QDomElement &node) const;

    QString                    mVersion;        ///< holds the version of architecture file
    QMap<QString, DbTable>     mTdrTables;      ///< holds the list of tables in the database
    QMap<QString, DbTableLink> mTablesLinks;    ///< holds the list of links in the database
    QString                    mSchemaName;     ///< holds the schema name
};

} //END namespace DbPluginBase
} //END namespace GS


#endif // DB_ARCHITECTURE_H
