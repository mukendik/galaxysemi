#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include "gexlogthread.h"

//static QSqlDatabase s_sqldb;
static bool sqlite_problem=true;

bool CSqlOutput::PopFront()
{
	m_buffer.pop_front();
    return true;
}

QString _log_to_sql(int sev, const char* file, const char* func, const char* m)
{
	QSqlDatabase s_sqldb=QSqlDatabase::database("log.sqlite");
	// SQL log
	if (!QSqlDatabase::contains("log.sqlite") && !sqlite_problem )
		//if (!s_sqldb.isValid())
		if (QSqlDatabase::isDriverAvailable("QSQLITE"))
		{
			#ifdef QT_DEBUG
			 qDebug("Drivers list = %s", QSqlDatabase::drivers().join(",").toLatin1().data());
			 qDebug("adding SQLITE log.sqlite DB...");
			#endif
			s_sqldb=QSqlDatabase::addDatabase("QSQLITE", "log.sqlite");
			#ifdef QT_DEBUG
			 if (!s_sqldb.isValid())
				qDebug("addDatabase() failed !");
			 else
				qDebug("addDatabase() ok.");
			#endif
			//return;
		}

	if (!s_sqldb.isOpen() && !sqlite_problem )
	{
		s_sqldb.setDatabaseName("log.sqlite");
		#ifdef QT_DEBUG
		 qDebug("try to open log DB....");
		#endif
		if (s_sqldb.open())
		{
			#ifdef QT_DEBUG
			 qDebug("sqlite DB opened.");
			#endif
			s_sqldb.exec("create table if not exists LOG ( SEV smallint(1) NOT NULL, MSG varchar(512) NOT NULL, FILE varchar(255) NOT NULL, FUNC varchar(255) NOT NULL )");
			if (s_sqldb.lastError().type()!=QSqlError::NoError)
			{
				//if (s_sqldb.lastError().text().contains("exist"))
				{	s_sqldb.close(); sqlite_problem=true;
				}
			}
		}
		#ifdef QT_DEBUG
		else
			qDebug("error : %s", s_sqldb.lastError().text().toLatin1().data());
		#endif
	}
	else if (!sqlite_problem)
	{
		s_sqldb.exec(QString("insert into LOG VALUES('%1','%2','%3','%4')")
					 .arg(sev).arg(m).arg(file?file:"").arg(func?func:"") );
		if (s_sqldb.lastError().type()!=QSqlError::NoError)
		{
			#ifdef QT_DEBUG
			 qDebug("%s\n", s_sqldb.lastError().text().toLatin1().data());
			#endif
			sqlite_problem=true;
		}
	}

	return "ok";
}

