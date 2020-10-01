#include "gexdb_plugin_base.h"
#include "gexdb_plugin_querybuilder.h"

using namespace GexDbPluginSQL;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLField
//
// Description	:	Class which represents a field in the SQL Query
//
///////////////////////////////////////////////////////////////////////////////////
GexDbSQLField::GexDbSQLField(const QString &strField, sqlField eField, const QString &strAlias, const GexDbSQLTable &sqlTable, bool bIsNumeric /*= false*/) : m_eFunction(SqlFunctionNone), m_bIsNumeric(bIsNumeric), m_bGrouped(false)
{
    m_strField	= strField;
    m_strAlias	= strAlias;
    m_strTable	= sqlTable.name();
    m_eField	= eField;
}

GexDbSQLField::GexDbSQLField(const QString &strField, sqlField eField, const QString &strAlias, const QString& strTable, bool bIsNumeric /*= false*/)  : m_eFunction(SqlFunctionNone), m_bIsNumeric(bIsNumeric), m_bGrouped(false)
{
    m_strField	= strField;
    m_strAlias	= strAlias;
    m_strTable	= strTable;
    m_eField	= eField;
}

GexDbSQLField::GexDbSQLField(const GexDbSQLField &sqlField)
{
    m_strField		= sqlField.m_strField;
    m_strAlias		= sqlField.m_strAlias;
    m_strTable		= sqlField.m_strTable;
    m_eField		= sqlField.m_eField;
    m_eFunction		= sqlField.m_eFunction;
    m_bIsNumeric	= sqlField.m_bIsNumeric;
    m_bGrouped		= sqlField.m_bGrouped;
}

GexDbSQLField::GexDbSQLField() : m_eField(SqlFieldName), m_eFunction(SqlFunctionNone), m_bIsNumeric(false), m_bGrouped(false)
{
}

QString GexDbSQLField::buildSQLString(GexDbPlugin_Connector * pSqlConnector) const
{
    QString strSQLString;

    if (m_eField == GexDbSQLField::SqlFieldName)
    {
        if (m_strTable.isEmpty() == false)
            strSQLString = QString("%1.%2").arg(m_strTable).arg(m_strField);
        else
            strSQLString = QString("%1").arg(m_strField);
    }
    else if (m_eField == GexDbSQLField::SqlFieldNumeric)
        strSQLString = QString("%1").arg(m_strField);
    else if (m_eField == GexDbSQLField::SqlFieldExpression)
        strSQLString = QString("%1").arg(m_strField);
    else
        strSQLString = QString("'%1'").arg(m_strField);

    switch(m_eFunction)
    {
    case SqlFunctionSum		:	strSQLString = QString("sum(%1)").arg(strSQLString);
        break;

    case SqlFunctionMax		:	strSQLString = QString("max(%1)").arg(strSQLString);
        break;

    case SqlFunctionCount	:	strSQLString = QString("count(%1)").arg(strSQLString);
        break;

    default					:	if (m_bGrouped == false && pSqlConnector && pSqlConnector->IsOracleDB())
            strSQLString = QString("max(%1)").arg(strSQLString);

        break;
    }

    if (m_strAlias.isEmpty() == false)
        strSQLString += QString(" as %1").arg(m_strAlias);

    // Prepend with a line feed
    strSQLString.prepend("\n");

    return strSQLString;
}

QString GexDbSQLField::name() const
{
    QString strName;

    if (m_eField == GexDbSQLField::SqlFieldName)
    {
        if (m_strTable.isEmpty() == false)
            strName = QString("%1.%2").arg(m_strTable).arg(m_strField);
        else
            strName = QString("%1").arg(m_strField);
    }
    else if (m_strAlias.isEmpty() == false)
        strName = m_strAlias;
    else if (m_eField == GexDbSQLField::SqlFieldNumeric)
        strName = QString("%1").arg(m_strField);
    else
        strName = QString("'%1'").arg(m_strField);

    return strName;
}

bool GexDbSQLField::operator==(const GexDbSQLField& sqlField) const
{
    return name() == sqlField.name();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLTable
//
// Description	:	Class which represents a table in the SQL Query
//
///////////////////////////////////////////////////////////////////////////////////

GexDbSQLTable::GexDbSQLTable(const QString &sqlQuery, const QString &strAlias)
{
    m_strAlias	= strAlias;
    m_sqlQueries.append(sqlQuery);
}

GexDbSQLTable::GexDbSQLTable(const GexDbSQLQuery& sqlQuery, const QString &strAlias)
{
    m_sqlQueries.append(sqlQuery.buildSQLString());
    m_strAlias	= strAlias;
}

GexDbSQLTable::GexDbSQLTable(const QList<GexDbSQLQuery> &sqlQueries, const QString &strAlias)
{
    for (int lIdx = 0; lIdx < sqlQueries.count(); ++lIdx)
        m_sqlQueries.append(sqlQueries.at(lIdx).buildSQLString());

    m_strAlias      = strAlias;
}

GexDbSQLTable::GexDbSQLTable(const QString &strTable, const QString &strAlias, const QString &strSchema)
{
    m_strName	= strTable;
    m_strAlias	= strAlias;
    m_strSchema	= strSchema;
}

GexDbSQLTable::GexDbSQLTable(const GexDbSQLTable &sqlTable)
{
    m_strName       = sqlTable.m_strName;
    m_strAlias      = sqlTable.m_strAlias;
    m_strSchema     = sqlTable.m_strSchema;
    m_sqlQueries	= sqlTable.m_sqlQueries;
}

QString GexDbSQLTable::name() const
{
    QString strTableName;

    if (m_strAlias.isEmpty() == false)
        strTableName = m_strAlias;
    else if (m_strName.isEmpty() == false)
    {
        if (m_strSchema.isEmpty() == false)
            strTableName = QString("%1.%2").arg(m_strSchema).arg(m_strName);
        else
            strTableName = m_strName;
    }

    return strTableName;
}

QString GexDbSQLTable::buildSQLString() const
{
    QString strSQLString = "\n";

    if (m_strName.isEmpty() == false)
    {
        if (m_strSchema.isEmpty())
            strSQLString =	m_strName;
        else
            strSQLString = QString("%1.%2").arg(m_strSchema).arg(m_strName);

        strSQLString += " ";
    }
    else
    {
        strSQLString =	" ( ";

        for (int idx = 0; idx < m_sqlQueries.count(); idx++)
        {
            if (idx == 0)
                strSQLString +=	" ( ";
            else
                strSQLString += "\nunion \n(";

            strSQLString +=	m_sqlQueries.at(idx);
            strSQLString += " )";
        }

        strSQLString += " ) ";
    }

    strSQLString += m_strAlias;

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLQuery
//
// Description	:	Class which represents an SQL Query
//
///////////////////////////////////////////////////////////////////////////////////
GexDbSQLQuery::GexDbSQLQuery(GexDbPlugin_Connector *pPluginConnector) : m_bDistinct(false), m_pSqlConnector(pPluginConnector)
{
}

GexDbSQLQuery::GexDbSQLQuery(const GexDbSQLQuery &other)
{
    m_lstSQLField       = other.m_lstSQLField;
    m_lstSQLTable       = other.m_lstSQLTable;
    m_lstSQLGroupBy     = other.m_lstSQLGroupBy;
    m_lstSQLOrderBy     = other.m_lstSQLOrderBy;
    m_lstSQLCondition   = other.m_lstSQLCondition;
    m_lstSQLJoin        = other.m_lstSQLJoin;
    m_lstSQLUnion       = other.m_lstSQLUnion;
    m_bDistinct         = other.m_bDistinct;
    m_pSqlConnector     = other.m_pSqlConnector;
}

void GexDbSQLQuery::addField(const GexDbSQLField &sqlField)
{
    int nIndexField = m_lstSQLField.indexOf(sqlField);

    if (nIndexField == -1)
    {
        m_lstSQLField.append(sqlField);
        nIndexField = m_lstSQLField.count()-1;
    }

    if (m_lstSQLGroupBy.indexOf(sqlField) != -1)
        m_lstSQLField[nIndexField].setGroupBy(true);
}

void GexDbSQLQuery::addTable(const GexDbSQLTable& sqlTable)
{
    m_lstSQLTable.append(sqlTable);
}

void GexDbSQLQuery::addGroupBy(const GexDbSQLField &sqlField)
{
    m_lstSQLGroupBy.append(sqlField);

    int nIndexField = m_lstSQLField.indexOf(sqlField);

    if (nIndexField != -1)
        m_lstSQLField[nIndexField].setGroupBy(true);
}

void GexDbSQLQuery::addOrderBy(const GexDbSQLField &sqlField, sqlOrderBy eOrderBy)
{
    m_lstSQLOrderBy.append(GexDbSQLOrderBy(sqlField, eOrderBy));
}

void GexDbSQLQuery::addCondition(const GexDbSQLCondition& sqlCondition)
{
    m_lstSQLCondition.append(sqlCondition);
}

void GexDbSQLQuery::addJoin(const GexDbSQLJoin& sqlJoin)
{
    m_lstSQLJoin.append(sqlJoin);
}

void GexDbSQLQuery::addUnion(const GexDbSQLQuery& sqlQuery)
{
    m_lstSQLUnion.append(sqlQuery);
}

QString GexDbSQLQuery::buildSQLString() const
{
    QString strSQLString = "select ";

    if (m_bDistinct)
        strSQLString += "distinct ";

    if (m_lstSQLField.count() == 0)
        strSQLString += "*";
    else
    {
        for (int nField = 0; nField < m_lstSQLField.count(); nField++)
        {
            strSQLString += m_lstSQLField.at(nField).buildSQLString(m_pSqlConnector);

            if (nField != m_lstSQLField.count()-1)
                strSQLString += ", ";
        }
    }

    strSQLString += "\nfrom ";

    for (int nTable = 0; nTable < m_lstSQLTable.count(); nTable++)
    {
        strSQLString += m_lstSQLTable.at(nTable).buildSQLString();

        if (nTable != m_lstSQLTable.count()-1)
            strSQLString += ", ";
    }

    for (int nJoin = 0; nJoin < m_lstSQLJoin.count(); nJoin++)
    {
        strSQLString += m_lstSQLJoin.at(nJoin).buildSQLString();
    }

    if (m_lstSQLCondition.count() > 0)
    {
        strSQLString += "\nwhere ";

        for (int nCondition = 0; nCondition < m_lstSQLCondition.count(); nCondition++)
        {
            strSQLString += m_lstSQLCondition.at(nCondition).buildSQLString();

            if (nCondition != m_lstSQLCondition.count()-1)
                strSQLString += " and ";
        }

    }

    if (m_lstSQLGroupBy.count() > 0)
    {
        strSQLString += "\ngroup by ";

        for (int nGroupBy = 0; nGroupBy < m_lstSQLGroupBy.count(); nGroupBy++)
        {
            strSQLString += m_lstSQLGroupBy.at(nGroupBy).name();

            if (nGroupBy != m_lstSQLGroupBy.count()-1)
                strSQLString += ", ";
        }
    }

    if (m_lstSQLOrderBy.count() > 0)
    {
        strSQLString += "\norder by \n";

        for (int nOrderBy = 0; nOrderBy < m_lstSQLOrderBy.count(); nOrderBy++)
        {
            strSQLString += m_lstSQLOrderBy.at(nOrderBy).buildSQLString();

            if (nOrderBy != m_lstSQLOrderBy.count()-1)
                strSQLString += ", ";
        }
    }

    if (m_lstSQLUnion.count() > 0)
    {
        strSQLString.prepend("(");
        strSQLString.append(")");

        for (int nUnion = 0; nUnion < m_lstSQLUnion.count(); nUnion++)
        {
            strSQLString += "\nunion ";
            strSQLString += "\n(";
            strSQLString += m_lstSQLUnion.at(nUnion).buildSQLString();
            strSQLString += ")";
        }
    }

    // qDebug are forbidden in release, and boring in debug for coders who are not interested by your qDebug.
    // Dev rules 6.6 imposes to use GSLOG
    // By spliting gex logs per MODULE (in GSLOG.xml), you will have logs for PluginBase only
    //qDebug("%s",QString("GexDbSQLQuery : %1").arg(strSQLString).toLatin1().data());

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLQuery::GexDbSQLOrderBy
//
// Description	:	Class which represents an ordered field in the query
//
///////////////////////////////////////////////////////////////////////////////////
GexDbSQLQuery::GexDbSQLOrderBy::GexDbSQLOrderBy(const GexDbSQLField& sqlField, sqlOrderBy eOrderBy /*= SqlOrderByAsc*/) : m_sqlField(sqlField)
{
    m_eOrderBy	= eOrderBy;
}

QString GexDbSQLQuery::GexDbSQLOrderBy::buildSQLString() const
{
    QString strSQLString = m_sqlField.name();

    if (m_eOrderBy == GexDbSQLQuery::SqlOrderByDesc)
        strSQLString += " desc";

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLCondition
//
// Description	:	Class which represents a condition in the query
//
///////////////////////////////////////////////////////////////////////////////////
GexDbSQLCondition::GexDbSQLCondition(const GexDbSQLField &sqlField, const QString &strValue, sqlCondition eCondition)
{
    addCondition(sqlField, strValue, eCondition);
}

GexDbSQLCondition::GexDbSQLCondition(const GexDbSQLField& sqlField, const GexDbSQLField& sqlFieldValue)
{
    addCondition(sqlField, sqlFieldValue);
}

GexDbSQLCondition::GexDbSQLCondition(const GexDbSQLField& sqlField, uint uiTimestampFrom, uint uiTimestampTo)
{
    addCondition(sqlField, uiTimestampFrom, uiTimestampTo);
}

void GexDbSQLCondition::addCondition(const GexDbSQLField &sqlField, const QString &strValue, sqlCondition eCondition)
{
    m_lstPrivateCondition.append(GexDbSQLCondition::GexDbSQLPrivateCondition(sqlField, strValue, eCondition));
}

void GexDbSQLCondition::addCondition(const GexDbSQLField& sqlField, const GexDbSQLField& sqlFieldValue)
{
    m_lstPrivateCondition.append(GexDbSQLCondition::GexDbSQLPrivateCondition(sqlField, sqlFieldValue.name(), GexDbSQLCondition::sqlConditionField));
}

void GexDbSQLCondition::addCondition(const GexDbSQLField& sqlField, uint uiTimestampFrom, uint uiTimestampTo)
{
    QString strValue;

    strValue = QString::number(uiTimestampFrom);
    strValue += "|";
    strValue += QString::number(uiTimestampTo);

    m_lstPrivateCondition.append(GexDbSQLCondition::GexDbSQLPrivateCondition(sqlField, strValue, GexDbSQLCondition::sqlConditionBetween));
}

QString GexDbSQLCondition::buildSQLString() const
{
    QString strSQLString = "\n( ";

    for (int nCondition = 0; nCondition < m_lstPrivateCondition.count(); ++nCondition)
    {
        GexDbSQLPrivateCondition lCondition = m_lstPrivateCondition.at(nCondition);
        strSQLString += lCondition.buildSQLString();

        if (nCondition < m_lstPrivateCondition.count()-1)
        {
            if (lCondition.GetSqlCondition() == sqlConditionStringNotEqual
                    || lCondition.GetSqlCondition() == sqlConditionNumericNotEqual)
                strSQLString += " and ";
            else
                strSQLString += " or ";
        }
    }

    strSQLString += " )";

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLCondition::GexDbSQLPrivateCondition
//
// Description	:	Class which represents a condition in the query
//
///////////////////////////////////////////////////////////////////////////////////
GexDbSQLCondition::GexDbSQLPrivateCondition::GexDbSQLPrivateCondition(const GexDbSQLField& sqlField, const QString &strValue, sqlCondition eCondition)
{
    m_sqlField		= sqlField;
    m_strValue		= strValue;
    m_eCondition	= eCondition;
}

GexDbSQLCondition::sqlCondition GexDbSQLCondition::GexDbSQLPrivateCondition::GetSqlCondition() const
{
    return m_eCondition;
}
QString GexDbSQLCondition::GexDbSQLPrivateCondition::buildSQLString() const
{
    QString strSQLString;

    switch(m_eCondition)
    {
    case	sqlConditionStringEqual		:
        if (m_strValue.contains("*") || m_strValue.contains("?"))
        {
            strSQLString = m_sqlField.name();
            strSQLString += " LIKE '";
            strSQLString += GexDbPlugin_Base::Query_BuildSqlStringExpression(m_strValue, true);
            strSQLString += "'";
        }
        else if (m_sqlField.isNumeric())
        {
            strSQLString = m_sqlField.name();
            strSQLString += " = ";
            strSQLString += m_strValue;
        }
        else
        {
            strSQLString = m_sqlField.name();
            strSQLString += " = '";
            strSQLString += GexDbPlugin_Base::Query_BuildSqlStringExpression(m_strValue, false);;
            strSQLString += "'";
        }
        break;

    case	sqlConditionStringNotEqual	:
        if (m_strValue.contains("*") || m_strValue.contains("?"))
        {
            strSQLString = m_sqlField.name();
            strSQLString += " NOT LIKE '";
            strSQLString += GexDbPlugin_Base::Query_BuildSqlStringExpression(m_strValue, true);;
            strSQLString += "'";
        }
        else if (m_sqlField.isNumeric())
        {
            strSQLString = m_sqlField.name();
            strSQLString += " != ";
            strSQLString += m_strValue;
        }
        else
        {
            strSQLString = m_sqlField.name();
            strSQLString += " != '";
            strSQLString += GexDbPlugin_Base::Query_BuildSqlStringExpression(m_strValue, false);;
            strSQLString += "'";
        }
        break;

    case	sqlConditionField			:
        strSQLString = m_sqlField.name();
        strSQLString += " = ";
        strSQLString += m_strValue;
        break;

    case	sqlConditionBetween			:
        strSQLString += m_sqlField.name();
        strSQLString += " BETWEEN ";
        strSQLString += m_strValue.section("|", 0, 0);
        strSQLString += " AND ";
        strSQLString += m_strValue.section("|", 1, 1);
        break;

    case	sqlConditionIsNull			:
        strSQLString += m_sqlField.name();
        strSQLString += " IS NULL";
        break;

    case	sqlConditionIsNotNull		:
        strSQLString += m_sqlField.name();
        strSQLString += " IS NOT NULL";
        break;

    default								:
        qDebug("**** WARNING **** QString GexDbSQLPrivateCondition::buildSQLString() const : case not managed...");
        break;
    }

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLJoin::GexDbSQLJoin
//
// Description	:	Class which represents a SQL Join
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbSQLJoin::GexDbSQLJoin(const GexDbSQLTable& sqlTable, sqlJoin eJoinType) : m_eJoin(eJoinType), m_sqlTable(sqlTable)
{
    //	addJoin(sqlTable, eJoinType);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbSQLJoin::~GexDbSQLJoin()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbSQLJoin::addJoin(const GexDbSQLTable& sqlTable, sqlJoin eJoinType)
//
// Description	:	Add a Join
//
///////////////////////////////////////////////////////////////////////////////////
//void GexDbSQLJoin::addJoin(const GexDbSQLTable& sqlTable, sqlJoin eJoinType)
//{
//	m_lstPrivateJoin.append(GexDbSQLJoin::GexDbSQLPrivateJoin(sqlTable, eJoinType));
//}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbSQLJoin::addCondition(const GexDbSQLCondition& sqlCondition)
//
// Description	:	Add a condition to the Join
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbSQLJoin::addCondition(const GexDbSQLCondition& sqlCondition)
{
    m_lstCondition.append(sqlCondition);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString GexDbSQLJoin::buildSQLString() const
//
// Description	:	Build the SQL string
//
///////////////////////////////////////////////////////////////////////////////////
QString GexDbSQLJoin::buildSQLString() const
{
    //	QString strSQLString;
    //
    //	for (int nJoin = 0; nJoin < m_lstPrivateJoin.count(); ++nJoin)
    //		strSQLString += m_lstPrivateJoin.at(nJoin).buildSQLString();

    QString strSQLString = "\n";

    switch (m_eJoin)
    {
    default					:
    case SqlInnerJoin		:	strSQLString += "inner join ";
        break;

    case SqlLeftJoin		:	strSQLString += "left join ";
        break;

    case SqlRightJoin		:	strSQLString += "right join ";
        break;

    case SqlLeftOuterJoin	:	strSQLString += "left outer join ";
        break;

    case SqlRightOuterJoin	:	strSQLString += "right outer join ";
        break;

    case SqlCrossJoin		:	strSQLString += "cross join ";
        break;
    }

    strSQLString += "\n";
    strSQLString += m_sqlTable.buildSQLString();

    if (m_lstCondition.count() > 0)
    {
        strSQLString += "\non ";

        for (int nCondition = 0; nCondition < m_lstCondition.count(); ++nCondition)
        {
            strSQLString += m_lstCondition.at(nCondition).buildSQLString();

            if (nCondition != m_lstCondition.count()-1)
                strSQLString += "\nand ";
        }
    }

    return strSQLString;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbSQLCondition::GexDbSQLPrivateCondition
//
// Description	:	Class which represents a condition in the query
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
//GexDbSQLJoin::GexDbSQLPrivateJoin::GexDbSQLPrivateJoin(const GexDbSQLTable& sqlTable, sqlJoin eJoinType)
//{
//	m_sqlTable	= sqlTable;
//	m_eJoin		= eJoinType;
//}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
//GexDbSQLJoin::GexDbSQLPrivateJoin::~GexDbSQLPrivateJoin()
//{
//
//}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString GexDbSQLJoin::GexDbSQLPrivateJoin::buildSQLString() const
//
// Description	:	Build the SQL string
//
///////////////////////////////////////////////////////////////////////////////////
//QString GexDbSQLJoin::GexDbSQLPrivateJoin::buildSQLString() const
//{
//	QString strSQLString = " ";
//
//	switch (m_eJoin)
//	{
//		default					:
//		case SqlInnerJoin		:	strSQLString += "inner join ";
//									break;
//
//		case SqlLeftJoin		:	strSQLString += "left join ";
//									break;
//
//		case SqlRightJoin		:	strSQLString += "right join ";
//									break;
//
//		case SqlLeftOuterJoin	:	strSQLString += "left outer join ";
//									break;
//
//		case SqlRightOuterJoin	:	strSQLString += "right outer join ";
//									break;
//
//		case SqlCrossJoin		:	strSQLString += "cross join ";
//									break;
//	}
//
//	strSQLString += m_sqlTable.buildSQLString();
//
//	return strSQLString;
//}
