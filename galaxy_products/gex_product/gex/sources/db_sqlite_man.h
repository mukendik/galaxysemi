#ifndef DB_SQLITE_MAN_H
#define DB_SQLITE_MAN_H

#include <QString>
class QStringList;
class QSqlDatabase;
class QSqlQuery;
class QObject;

class DbSqliteMan
{

public:
	DbSqliteMan(const QString& strWorkingFolder, const QString& strFileDBName, const QString& strDBName = "", QObject *parent = 0);
	~DbSqliteMan();
	QSqlQuery execQuery(const QString& strQuery, QString* strErrorMsg = NULL);
	QString connectionName() {return m_strConnectionName;}
	bool execQuery(const QStringList& strQueries);
	bool execMultiStatementsQuery(const QString& strQuery);
	QSqlDatabase* m_pDataBase;

private:
	bool createConnection();
	bool closeConnection();
	QString m_strConnectionName;
	QString m_strFileDBName;
	QString m_strWorkingFolder;
};



#endif // DB_SQLITE_MAN_H
