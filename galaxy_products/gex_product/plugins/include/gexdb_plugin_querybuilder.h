#ifndef GEXDB_PLUGIN_QUERYBUILDER_H
#define GEXDB_PLUGIN_QUERYBUILDER_H

#include <QString>
#include <QList>

class GexDbPlugin_Connector;

namespace GexDbPluginSQL
{
	class GexDbSQLTable;
	class GexDbSQLJoin;

	class GexDbSQLField
	{
	public:
		enum sqlField
		{
			SqlFieldName	= 0,
			SqlFieldText,
            SqlFieldNumeric,
            SqlFieldExpression
		};

		enum sqlFunction
		{
			SqlFunctionNone = -1,
			SqlFunctionSum = 0,
			SqlFunctionMax = 1,
			SqlFunctionCount
		};

		GexDbSQLField();
		GexDbSQLField(const QString& strField, sqlField eField, const QString& strAlias, const GexDbSQLTable& sqlTable, bool bIsNumeric = false);
		GexDbSQLField(const QString& strField, sqlField eField, const QString& strAlias, const QString& strTable, bool bIsNumeric = false);
		GexDbSQLField(const GexDbSQLField& sqlField);
		~GexDbSQLField() {};

		QString			name() const;
		QString			buildSQLString(GexDbPlugin_Connector * pSqlConnector) const;
		const QString&	alias() const								{ return m_strAlias; }
		bool			isNumeric() const							{ return m_bIsNumeric; }

		void			setSqlFunction(sqlFunction eSqlFunction)	{ m_eFunction = eSqlFunction; }
		void			setGroupBy(bool bGrouped)					{ m_bGrouped = bGrouped; }

		bool			operator==(const GexDbSQLField& sqlField) const;

	private:

		QString			m_strField;
		QString			m_strAlias;
		QString			m_strTable;
		sqlField		m_eField;
		sqlFunction		m_eFunction;
		bool			m_bIsNumeric;
		bool			m_bGrouped;
	};

	class GexDbSQLCondition
	{
	public:

		enum sqlCondition
		{
			sqlConditionStringEqual = 0,
			sqlConditionStringNotEqual,
			sqlConditionField,
			sqlConditionNumericEqual,
			sqlConditionNumericNotEqual,
			sqlConditionBetween,
			sqlConditionIsNull,
			sqlConditionIsNotNull
		};

		GexDbSQLCondition(const GexDbSQLField &sqlField, const QString &strValue, sqlCondition eCondition);
		GexDbSQLCondition(const GexDbSQLField& sqlField, const GexDbSQLField& sqlFieldValue);
		GexDbSQLCondition(const GexDbSQLField& sqlField, uint uiTimestampFrom, uint uiTimestampTo);
		~GexDbSQLCondition()	{};

		void					addCondition(const GexDbSQLField &sqlField, const QString &lstValue, sqlCondition eCondition);
		void					addCondition(const GexDbSQLField& sqlField, const GexDbSQLField& sqlFieldValue);
		void					addCondition(const GexDbSQLField& sqlField, uint uiTimestampFrom, uint uiTimestampTo);

		QString					buildSQLString() const;

	private:

		class GexDbSQLPrivateCondition
		{
		public:

			GexDbSQLPrivateCondition(const GexDbSQLField& sqlField, const QString& strValue, sqlCondition eCondition);
			~GexDbSQLPrivateCondition() {};

            sqlCondition        GetSqlCondition() const;
			QString				buildSQLString() const;


		private:

			GexDbSQLField		m_sqlField;
			QString				m_strValue;
			sqlCondition		m_eCondition;
		};

		QList<GexDbSQLPrivateCondition>		m_lstPrivateCondition;
	};

	class GexDbSQLQuery
	{
	public:

		enum sqlOrderBy
		{
			SqlOrderByAsc = 0,
			SqlOrderByDesc
		};

	private:

		class GexDbSQLOrderBy
		{
		public:

			GexDbSQLOrderBy(const GexDbSQLField& sqlField, sqlOrderBy eOrderBy = SqlOrderByAsc);
			~GexDbSQLOrderBy() {};

			QString			buildSQLString() const;

		private:

			GexDbSQLField	m_sqlField;
			sqlOrderBy		m_eOrderBy;
		};

	public:

		GexDbSQLQuery(GexDbPlugin_Connector * pPluginConnector);
        GexDbSQLQuery(const GexDbSQLQuery& other);
		~GexDbSQLQuery() {};

		void	addField(const GexDbSQLField& sqlField);
		void	addTable(const GexDbSQLTable& sqlTable);
		void	addCondition(const GexDbSQLCondition& sqlCondition);
		void	addGroupBy(const GexDbSQLField& sqlField);
		void	addOrderBy(const GexDbSQLField& sqlField, sqlOrderBy eOrderBy = SqlOrderByAsc);
		void	addJoin(const GexDbSQLJoin& sqlJoin);
		void	addUnion(const GexDbSQLQuery& sqlQuery);

		void	setDistinct(bool bDistinct)				{ m_bDistinct = bDistinct; }

		QString	buildSQLString() const;

	private:

		QList<GexDbSQLField>		m_lstSQLField;
		QList<GexDbSQLTable>		m_lstSQLTable;
		QList<GexDbSQLField>		m_lstSQLGroupBy;
		QList<GexDbSQLOrderBy>		m_lstSQLOrderBy;
		QList<GexDbSQLCondition>	m_lstSQLCondition;
		QList<GexDbSQLJoin>			m_lstSQLJoin;
		QList<GexDbSQLQuery>		m_lstSQLUnion;

		bool						m_bDistinct;

	private:

		GexDbPlugin_Connector *	m_pSqlConnector;
	};

	class GexDbSQLTable
	{
	public:

        GexDbSQLTable() {}
        GexDbSQLTable(const QString& strTable, const QString& strAlias, const QString& strSchema);
        GexDbSQLTable(const QString& sqlQuery, const QString& strAlias);
        GexDbSQLTable(const GexDbSQLQuery& sqlQuery, const QString& strAlias);
        GexDbSQLTable(const QList<GexDbSQLQuery>& sqlQueries, const QString& strAlias);
		GexDbSQLTable(const GexDbSQLTable& sqlTable);

        ~GexDbSQLTable() {}

		QString                 buildSQLString() const;

		QString                 name() const;

	private:

		QString                 m_strName;
		QString                 m_strAlias;
		QString                 m_strSchema;
        QList<QString>          m_sqlQueries;
	};

	class GexDbSQLJoin
	{
	public:

		enum sqlJoin
		{
			SqlInnerJoin = 0,
			SqlLeftJoin,
			SqlRightJoin,
			SqlLeftOuterJoin,
			SqlRightOuterJoin,
			SqlCrossJoin
		};

		GexDbSQLJoin(const GexDbSQLTable& sqlTable, sqlJoin eJoinType);
		~GexDbSQLJoin();

		void			addCondition(const GexDbSQLCondition& sqlCondition);

		QString			buildSQLString() const;

	private:

		sqlJoin							m_eJoin;
		GexDbSQLTable					m_sqlTable;
		QList<GexDbSQLCondition>		m_lstCondition;
	};
};

#endif // GEXDB_PLUGIN_QUERYBUILDER_H
