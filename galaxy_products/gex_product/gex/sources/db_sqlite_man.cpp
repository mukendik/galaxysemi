
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QDir>
#include <QVariant>
#include <QSqlDriver>

#include <../src/3rdparty/sqlite/sqlite3.h>

#include <gqtl_log.h>
#include "db_sqlite_man.h"




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DbSqliteMan::DbSqliteMan(const QString& strWorkingFolder,
						 const QString& strFileDBName,
						 const QString& strConnectionName,
						 QObject* /*parent*/)
{
	m_strWorkingFolder = strWorkingFolder;
	m_strFileDBName = strFileDBName;
	if (strConnectionName.isEmpty())
		m_strConnectionName = QDateTime::currentDateTime().toString("ddMMyyyy_hhmmsszzz");
	else
		m_strConnectionName = strConnectionName;
	createConnection();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DbSqliteMan::~DbSqliteMan()
{
	closeConnection();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Create new connection to database
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DbSqliteMan::createConnection()
{
	m_pDataBase = new QSqlDatabase();
	*m_pDataBase= QSqlDatabase::addDatabase("QSQLITE", m_strConnectionName);
	QString strDbPath = QDir::cleanPath(m_strWorkingFolder + "/" + m_strFileDBName);
	m_pDataBase->setDatabaseName(strDbPath);
	if (!m_pDataBase->open())
	{
		GSLOG(SYSLOG_SEV_ERROR, QString("Cannot open database - Unable to establish a database connection to %1.")
		       .arg(m_strFileDBName));
		return false;
	}
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Close connection and remove database vars
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DbSqliteMan::closeConnection()
{
	if (m_pDataBase != NULL)
	{
		m_pDataBase->close();
		delete m_pDataBase; m_pDataBase=NULL;
	}
	if (QSqlDatabase::contains(m_strConnectionName))
		QSqlDatabase::removeDatabase(m_strConnectionName);
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Execute simple query
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSqlQuery DbSqliteMan::execQuery(const QString& strQuery, QString* strErrorMsg)
{
	if (!strQuery.isEmpty())
	{
		QSqlQuery query(*m_pDataBase);
		query.exec("PRAGMA synchronous = OFF;");
		if (!query.exec(strQuery))
		{
			GSLOG(SYSLOG_SEV_ERROR, QString("Unable to exec : %1. Error : %2.")
			       .arg(strQuery)
			       .arg(query.lastError().text()));
			if (strErrorMsg)
				*strErrorMsg = "Unable to exec : " + strQuery + " Error : " + query.lastError().text();
			return NULL;
		}
		return query;
	}
	return NULL;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Execute multistatement query
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DbSqliteMan::execQuery(const QStringList& strQueries)
{
	QStringList::iterator it;
	bool bSqlOk=true;
	QStringList strExecQueries = strQueries;
	QSqlQuery query(QSqlDatabase::database(m_strConnectionName));
	for (it = strExecQueries.begin(); it != strExecQueries.end(); ++it)
	{
		if (!(*it).trimmed().isEmpty())
		{
			bSqlOk = query.exec((*it));
			if (!bSqlOk)
			{
				GSLOG(SYSLOG_SEV_ERROR, QString("Unable to exec: %1. Error: %2.")
				       .arg(*it)
				       .arg(query.lastError().text()));
				return false;
			}
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int callback(void* /*NotUsed*/, int argc, char **argv, char **azColName)
{
	int i =0;
	for(i=0; i<argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief execute multistatement query with no journal backup (very fast)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DbSqliteMan::execMultiStatementsQuery(const QString& strQuery)
{
	bool bSqlOk=true;
	QSqlQuery query(QSqlDatabase::database(m_strConnectionName));
	query.exec("PRAGMA synchronous = OFF;");
//	m_pDataBase->transaction();
	QStringList sql = strQuery.split(";", QString::SkipEmptyParts);
	QStringList::iterator itSql;

	for (itSql = sql.begin(); itSql != sql.end() && bSqlOk; ++itSql)
	{
		if (!(*itSql).trimmed().isEmpty())
			bSqlOk=query.exec(*itSql);
	}

//	if (bSqlOk)
//	{
//		query.clear();
//		bSqlOk = m_pDataBase->commit();
//	}
//	else
//	{
//		qDebug() << "DbSqliteMan::execMultiStatementsQuery: Error: " << query.lastError().text() << " in query: " << (*itSql);
//		query.clear();
//		m_pDataBase->rollback();
//	}


//	bool bSqliteExecOK = false;
//	char *errmsg=NULL;

//	if(!query.exec("begin transaction;\n")) 		// TO DO : "BEGIN ON CONFLICT ABORT;\n";
//	{
//		qDebug() << "DbSqliteMan::execMultiStatementsQuery: ERROR: BEGIN TRANSACTION query execution failed !";
//		qDebug() << query.lastError().text().toLatin1().data();
//		return false;
//	}

//	QVariant v = m_pDataBase->driver()->handle();
//	if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0)
//	{
//		// v.data() returns a pointer to the handle
//		sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
//		if (handle != 0) // check that it is not NULL
//		{
//			int res;
//			res = sqlite3_exec( handle, QString("begin transaction;\n").toLatin1().data(), callback, 0, &errmsg);
//			if (res != SQLITE_OK)
//			{
//				qDebug() << "Error in sqlite3_exec(): " << QString(errmsg) << endl;
//				sqlite3_free(errormsg);
//				return false;
//			}
//			qDebug() << "Begin transaction: OK" << endl;
//			res = sqlite3_exec( handle, strQuery.toLatin1().data(), callback, 0, &errmsg);
//			if (res != SQLITE_OK)
//			{
//				qDebug() << "Error in sqlite3_exec(): " << QString(errmsg) << endl;
//				return false;
//			}
//			qDebug() << "Query: OK" << endl;
//			res = sqlite3_exec( handle, QString("commit;\n").toLatin1().data(), callback, 0, &errmsg);
//			if (res != SQLITE_OK)
//			{
//				qDebug() << "Error in sqlite3_exec(): " << QString(errmsg) << endl;
//				return false;
//			}
//			qDebug() << "Commit: OK" << endl;
//		}
//	}

//	if (!bSqliteExecOK)
//	{
//		qDebug() << "DbSqliteMan::execMultiStatementsQuery: ERROR: sqlite3_exec() " << QString(errmsg);
//
//		execQuery(strQuery.split(";"));
//	}
//
//	if (!query.exec("COMMIT TRANSACTION\n;"))
//	{
//		qDebug() << "DbSqliteMan::execMultiStatementsQuery: ERROR: COMMIT TRANSACTION query execution failed !";
//		qDebug() << query.lastError().text().toLatin1().data();
//		return false;
//	}

	return true;
}
