#include "db_architecture.h"
#include <gqtl_log.h>

#include <QFile>

namespace GS
{
namespace DbPluginBase
{

DbArchitecture::DbArchitecture(QString schemaName, QObject *parent) :
    QObject(parent)
{
    mSchemaName = schemaName;
}

DbArchitecture::~DbArchitecture()
{
    ClearTables();
}

void DbArchitecture::ClearTables()
{
    mTdrTables.clear();
}

void DbArchitecture::ClearTableLinks()
{
    mTablesLinks.clear();
}

bool DbArchitecture::LoadFromFile(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    QFile lFile(filePath);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to open %1").
               arg(filePath).
               toLatin1().data());
        return false;
    }

    QString lErrorMsg;
    int lErrorLine, lErrorColumn;
    QDomDocument lDomDocument;

    if (!lDomDocument.setContent(&lFile, &lErrorMsg, &lErrorLine, &lErrorColumn))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1 at line %2, column %3.").
               arg(lErrorMsg).
               arg(QString::number(lErrorLine)).
               arg(QString::number(lErrorColumn)).
               toLatin1().data());
        return false;
    }

    // Load root elmt
    bool lStatus = LoadFromDom(lDomDocument);

    lFile.close();

    return lStatus;
}

bool DbArchitecture::LoadFromDom(QDomDocument &domDocument)
{
    if (domDocument.isNull())
        return false;

    bool lIsValidArch = true;

    QDomElement lNodeRoot = domDocument.
            firstChildElement("galaxy_tables_dependencies");

    mVersion = lNodeRoot.attribute("version");

    QDomElement lNodeTmp = lNodeRoot.firstChildElement("table_list");
    if (!NodeIsValid(lNodeTmp))
        lIsValidArch = false;

    if (lIsValidArch && !LoadTablesFromDom(lNodeTmp))
        lIsValidArch = false;

    lNodeTmp = lNodeRoot.firstChildElement("table_link_list");
    if (lIsValidArch && !NodeIsValid(lNodeTmp))
        lIsValidArch = false;

    if (lIsValidArch && !LoadTableLinksFromDom(lNodeTmp))
        lIsValidArch = false;

    if (!lIsValidArch)
        GSLOG(SYSLOG_SEV_ERROR, "Invalid database architecture, unable to load");

    return lIsValidArch;
}

const QMap<QString, DbTable> &DbArchitecture::GetTables()
{
    return mTdrTables;
}

DbTable* DbArchitecture::GetTable(const QString &table)
{
    if (!mTdrTables.contains(table))
        return NULL;

    return &mTdrTables[table];
}

bool DbArchitecture::LoadTablesFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    ClearTables();

    QDomNode lNodeTable = node.firstChildElement("table");
    while (!lNodeTable.isNull())
    {
        if (lNodeTable.isElement())
        {
            QDomElement lEltTable = lNodeTable.toElement();
            if (NodeIsValid(lEltTable))
            {
                DbTable lTable;
                lTable.LoadFromDom(lEltTable);
                mTdrTables.insert(lTable.Name(), lTable);
            }
        }
        lNodeTable = lNodeTable.nextSibling();
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 tables loaded").
           arg(mTdrTables.count()).toLatin1().data());

    return true;
}

bool DbArchitecture::LoadTableLinksFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    ClearTableLinks();

    QDomNode lNodeTableLink = node.firstChildElement("table_link");
    while (!lNodeTableLink.isNull())
    {
        if (lNodeTableLink.isElement())
        {
            QDomElement lEltTableLink = lNodeTableLink.toElement();
            if (NodeIsValid(lEltTableLink))
            {
                DbTableLink lTableLink;
                lTableLink.LoadFromDom(lEltTableLink);
                mTablesLinks.insert(lTableLink.Id(), lTableLink);
            }
        }
        lNodeTableLink = lNodeTableLink.nextSibling();
    }

    bool lValidLinks = true;
    QMapIterator<QString, DbTable> it(mTdrTables);
    while (it.hasNext())
    {
        it.next();
        if (!mTdrTables[it.key()].LoadTableLinksContent(mTablesLinks))
            lValidLinks = false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 tables links loaded").
           arg(mTablesLinks.count()).toLatin1().data());

    return lValidLinks;
}

bool DbArchitecture::NodeIsValid(const QDomElement &node) const
{
    if (node.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("No '%1' node in table_architecture").
               arg(node.tagName())
               .toLatin1().data());
        return false;
    }

    return true;
}

bool DbArchitecture::AddSimpleLinkFromLegacy(
        const GexDbPlugin_Mapping_Link &mappingLink,
        const QString &testingStage)
{
    QString lField1, lField2, lTable1, lTable2;
    lTable1 = mappingLink.m_strSqlTable1;
    lTable2 = mappingLink.m_strSqlTable2;
    lField1 = mappingLink.m_strSqlFullField1.section(".", -1);
    lField2 = mappingLink.m_strSqlFullField2.section(".", -1);

    // Add table(s) to architecture
    if (GetTable(lTable1) == NULL)
    {
        GS::DbPluginBase::DbTable lTable;
        lTable.SetTestingStage(testingStage);
        lTable.SetName(lTable1);
        mTdrTables.insert(lTable.Name(), lTable);
    }
    if (GetTable(lTable2) == NULL)
    {
        GS::DbPluginBase::DbTable lTable;
        lTable.SetTestingStage(testingStage);
        lTable.SetName(lTable2);
        mTdrTables.insert(lTable.Name(), lTable);
    }

    QList<GS::DbPluginBase::DbTableLink::KeysLink> lKeyList;
    GS::DbPluginBase::DbTableLink::KeysLink lKeyLink(lField1, lField2);
    lKeyList.append(lKeyLink);
    GS::DbPluginBase::DbTableLink lTableLink(lTable1, lTable2, lKeyList);
    GS::DbPluginBase::DbTableDependency lDependency;
    lDependency.AddTableLink(lTableLink);
    if (!GetTable(lTable1)->AddDependency(lDependency))
        return false;
    if (!GetTable(lTable2)->AddDependency(lDependency))
        return false;

    mTablesLinks.insertMulti(lTableLink.Id(), lTableLink);

    return true;
}

bool DbArchitecture::AddMultipleKeyLinkFromLegacy(
        const QList<GexDbPlugin_Mapping_Link> &mappingLinks,
        const QString &testingStage)
{
    if (mappingLinks.isEmpty())
        return false;

    QString lTable1, lTable2;
    lTable1 = mappingLinks.isEmpty() ? "" : mappingLinks.first().m_strSqlTable1;
    lTable2 = mappingLinks.isEmpty() ? "" : mappingLinks.first().m_strSqlTable2;

    // Validity check
    foreach(const GexDbPlugin_Mapping_Link &mappingLink, mappingLinks)
    {
        if (!(((lTable1 == mappingLink.m_strSqlTable1) &&
             (lTable2 == mappingLink.m_strSqlTable2)) ||
                ((lTable2 == mappingLink.m_strSqlTable1) &&
                 (lTable1 == mappingLink.m_strSqlTable2))))
            return false;
    }
    // Add table(s) to architecture
    if (GetTable(lTable1) == NULL)
    {
        GS::DbPluginBase::DbTable lTable;
        lTable.SetTestingStage(testingStage);
        lTable.SetName(lTable1);
        mTdrTables.insert(lTable.Name(), lTable);
    }
    if (GetTable(lTable2) == NULL)
    {
        GS::DbPluginBase::DbTable lTable;
        lTable.SetTestingStage(testingStage);
        lTable.SetName(lTable2);
        mTdrTables.insert(lTable.Name(), lTable);
    }

    QList<GS::DbPluginBase::DbTableLink::KeysLink> lKeyList;
    GS::DbPluginBase::DbTableLink::KeysLink lKeyLink;
    foreach(const GexDbPlugin_Mapping_Link &mappingLink, mappingLinks)
    {
        QString lField1, lField2;
        lField1 = mappingLink.m_strSqlFullField1.section(".", -1);
        lField2 = mappingLink.m_strSqlFullField2.section(".", -1);
        if (mappingLink.m_strSqlTable1 == lTable1)
        {
            lKeyLink.mTable1Key.mValue = lField1;
            lKeyLink.mTable2Key.mValue = lField2;
        }
        else
        {
            lKeyLink.mTable1Key.mValue = lField2;
            lKeyLink.mTable2Key.mValue = lField1;
        }
        lKeyList.append(lKeyLink);
    }
    GS::DbPluginBase::DbTableLink lTableLink(lTable1, lTable2, lKeyList);
    GS::DbPluginBase::DbTableDependency lDependency;
    lDependency.AddTableLink(lTableLink);
    if (!GetTable(lTable1)->AddDependency(lDependency))
        return false;
    if (!GetTable(lTable2)->AddDependency(lDependency))
        return false;

    mTablesLinks.insertMulti(lTableLink.Id(), lTableLink);

    return true;
}

bool DbArchitecture::AddMultipleTableLinkFromLegacy(
        const QList<GexDbPlugin_Mapping_Link> &mappingLinks,
        const QString &testingStage)
{
    if (mappingLinks.isEmpty())
        return false;

    // build table list
    QStringList lTableList;
    foreach(const GexDbPlugin_Mapping_Link &lValue, mappingLinks)
    {
        lTableList << lValue.m_strSqlTable1;
        lTableList << lValue.m_strSqlTable2;
    }
    lTableList.removeDuplicates();
    // insert new tables if needed
    foreach(const QString &lTable, lTableList)
    {
        if (GetTable(lTable) == NULL)
        {
            GS::DbPluginBase::DbTable lNewTable;
            lNewTable.SetTestingStage(testingStage);
            lNewTable.SetName(lTable);
            mTdrTables.insert(lNewTable.Name(), lNewTable);
        }
    }

    // get main table targeted
    QString lMainTable = mappingLinks.isEmpty() ? "" : mappingLinks.first().m_strSqlTable1;

    // Validity check
    foreach(const GexDbPlugin_Mapping_Link &mappingLink, mappingLinks)
    {
        if (lMainTable != mappingLink.m_strSqlTable1)
            return false;
    }

    GS::DbPluginBase::DbTableDependency lMainDependency;
    foreach(const GexDbPlugin_Mapping_Link &mappingLink, mappingLinks)
    {
        QList<GS::DbPluginBase::DbTableLink::KeysLink> lKeyList;
        GS::DbPluginBase::DbTableLink::KeysLink lKeyLink;
        QString lField1, lField2, lTable2;
        lField1 = mappingLink.m_strSqlFullField1.section(".", -1);
        lField2 = mappingLink.m_strSqlFullField2.section(".", -1);
        lTable2 = mappingLink.m_strSqlTable2;

        lKeyLink.mTable1Key.mValue = lField1;
        lKeyLink.mTable2Key.mValue = lField2;
        lKeyList.append(lKeyLink);

        GS::DbPluginBase::DbTableLink lTableLink(lMainTable, lTable2, lKeyList);
        lMainDependency.AddTableLink(lTableLink);

        GS::DbPluginBase::DbTableDependency lDependency;
        lDependency.AddTableLink(lTableLink);
        GetTable(lTable2)->AddDependency(lDependency);

        mTablesLinks.insertMulti(lTableLink.Id(), lTableLink);
    }

    GetTable(lMainTable)->AddDependency(lMainDependency);

    return true;
}

QString DbArchitecture::SchemaName() const
{
    return mSchemaName;
}

} //END namespace DbPluginBase
} //END namespace GS

