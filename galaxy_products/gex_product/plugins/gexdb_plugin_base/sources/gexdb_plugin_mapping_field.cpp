#include "gexdb_plugin_common.h"
#include "gexdb_plugin_base.h"

///////////////////////////////////////////////////////////
// GexDbPlugin_Mapping_Field class:
// Meta-data field mapping
///////////////////////////////////////////////////////////
GexDbPlugin_Mapping_Field::GexDbPlugin_Mapping_Field()
{
    m_bDisplayInQueryGui    = false;
    m_bNumeric              = false;
    m_bTime                 = false;
    m_bCustom               = false;
    m_bFact                 = false;
    m_bConsolidated         = false;
    m_bDisplayInERGui       = false;
    m_bAZ                   = false;
    mTestCondition          = false;
    mStaticMetaData         = true;
}

GexDbPlugin_Mapping_Field::GexDbPlugin_Mapping_Field(const GexDbPlugin_Mapping_Field &other)
{
    *this = other;
}

GexDbPlugin_Mapping_Field::GexDbPlugin_Mapping_Field(
        QString strMetaDataName, QString strGexName, QString strSqlTable, QString strSqlFullField, QString strSqlLinkName,
        bool bDisplayInQueryGui,
        QString sBinType,
        bool bTime, bool bCustom, bool bFact, bool bConsolidated, bool bDisplayInERGui, bool bNumeric, bool bAZ,
        bool bStaticMetaData)
{
    m_strMetaDataName       = strMetaDataName;
    m_strGexName            = strGexName;
    m_strNormalizedName     = strSqlFullField.section('.', 1, 1);
//		m_strNormalizedName =	strMetaDataName.simplified().toLower();
//		m_strNormalizedName.replace(QRegExp("[^0-9a-zA-Z_]"), "_");
    m_strSqlTable           = strSqlTable;
    m_strSqlFullField       = strSqlFullField;
    m_strSqlLinkName        = strSqlLinkName;
    m_bDisplayInQueryGui    = bDisplayInQueryGui;
    m_bNumeric              = bNumeric;
    m_sBinType              = sBinType;
    m_bTime                 = bTime;
    m_bCustom               = bCustom;
    m_bFact                 = bFact;
    m_bConsolidated         = bConsolidated;
    m_bDisplayInERGui       = bDisplayInERGui;
    m_bAZ                   = bAZ;
    mTestCondition          = false;
    mStaticMetaData         = bStaticMetaData;
}

GexDbPlugin_Mapping_Field& GexDbPlugin_Mapping_Field::operator=(const GexDbPlugin_Mapping_Field& other)
{
    if (this != &other)
    {
        m_strMetaDataName       = other.m_strMetaDataName;
        m_strGexName            = other.m_strGexName;
        m_strNormalizedName     = other.m_strNormalizedName;
        m_strSqlTable           = other.m_strSqlTable;
        m_strSqlFullField       = other.m_strSqlFullField;
        m_strSqlLinkName        = other.m_strSqlLinkName;
        m_bDisplayInQueryGui    = other.m_bDisplayInQueryGui;
        m_bNumeric              = other.m_bNumeric;
        m_sBinType              = other.m_sBinType;
        m_bTime                 = other.m_bTime;
        m_bCustom               = other.m_bCustom;
        m_bFact                 = other.m_bFact;
        m_bConsolidated         = other.m_bConsolidated;
        m_bDisplayInERGui       = other.m_bDisplayInERGui;
        m_bAZ                   = other.m_bAZ;
        mTestCondition          = other.mTestCondition;
        mStaticMetaData         = other.mStaticMetaData;
    }

    return *this;
}

bool GexDbPlugin_Mapping_Field::isTestCondition() const
{
    return mTestCondition;
}

bool GexDbPlugin_Mapping_Field::isStaticMetaData() const
{
    return mStaticMetaData;
}


QString GexDbPlugin_Mapping_Field::getSqlFieldName() const
{
    QString sqlFieldName;

    if (m_strSqlFullField.indexOf(".") != -1)
        sqlFieldName = m_strSqlFullField.section(".", 1, 1);

    return sqlFieldName;
}

void GexDbPlugin_Mapping_Field::setTestCondition(bool testCondition)
{
    mTestCondition = testCondition;
}

bool GexDbPlugin_Mapping_FieldMap::ContainsGexField(const QString & strGexField, GexDbPlugin_Mapping_Field & clFieldMapping)
{
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = begin();
    while(itMapping != end())
    {
        if((*itMapping).m_strGexName == strGexField)
        {
            clFieldMapping = itMapping.value();// data();
            return true;
        }
        itMapping++;
    }
    return false;
}


bool GexDbPlugin_Mapping_FieldMap::ContainsSqlFullField(
        const QString & strSqlFullField,
        GexDbPlugin_Mapping_Field & clFieldMapping)
{
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = begin();
    while(itMapping != end())
    {
        if((*itMapping).m_strSqlFullField == strSqlFullField)
        {
            clFieldMapping = itMapping.value();
            return true;
        }
        itMapping++;
    }
    return false;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool GexDbPlugin_Mapping_FieldMap::GetSqlFullField(const QString & strMetaDataName, QString & strSqlFullField)
{
    GexDbPlugin_Mapping_Field				clFieldMapping;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;

    itMapping = find(strMetaDataName);
    if(itMapping == end())
        return false;

    clFieldMapping = itMapping.value();
    strSqlFullField = clFieldMapping.m_strSqlFullField;

    return true;
}


QList<GexDbPlugin_Mapping_Link> GexDbPlugin_Mapping_LinkMap::
GetLinkPredecessorList(const GexDbPlugin_Mapping_Link &link)
{
    QList<GexDbPlugin_Mapping_Link> lPredecessorList;
    bool lPredecessorFound = false;

    QMapIterator<QString, GexDbPlugin_Mapping_Link> it(*this);
    while (it.hasNext() && !lPredecessorFound)
    {
        it.next();
        if (it.value().m_strSqlTable2Link == link.m_strLinkName)
        {
            lPredecessorFound = true;
            lPredecessorList << it.value();
            lPredecessorList << GetLinkPredecessorList(it.value());
        }
    }

    return lPredecessorList;
}

QList<GexDbPlugin_Mapping_Link> GexDbPlugin_Mapping_LinkMap::
GetLinksWithSameTables(const GexDbPlugin_Mapping_Link &link) const
{
    QList<GexDbPlugin_Mapping_Link> lLinkList;

    foreach(GexDbPlugin_Mapping_Link lValue, *this)
    {
        if (((lValue.m_strSqlTable1 == link.m_strSqlTable1) &&
                (lValue.m_strSqlTable2 == link.m_strSqlTable2)) ||
                ((lValue.m_strSqlTable1 == link.m_strSqlTable2) &&
                 (lValue.m_strSqlTable2 == link.m_strSqlTable1)))
            lLinkList.append(lValue);
    }

    return lLinkList;
}

bool GexDbPlugin_Mapping_LinkMap::IsLinkedToTable(
        const GexDbPlugin_Mapping_Field & clField, const QString & strTableName)
{
    GexDbPlugin_Mapping_LinkMap::Iterator	itMapping;
    QString									strForwardLink;

    // Is field belonging to the specified table?
    if(clField.m_strSqlTable == strTableName)
        return true;

    // Any foward link?
    strForwardLink = clField.m_strSqlLinkName;
    while(!strForwardLink.isEmpty())
    {
        // Find specified link name in map
        itMapping = find(strForwardLink);
        if(itMapping == end())
            return false;

        // Check if specified table corresponds to dest table of the link
        if((*itMapping).m_strSqlTable2 == strTableName)
            return true;

        // Any forward link?
        strForwardLink = (*itMapping).m_strSqlTable2Link;
    }

    return false;
}

