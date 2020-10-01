// gexdb_plugin_galaxy_insertion_sqlite.cpp: implementation of the GexDbPlugin_Galaxy class for insertion.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "import_constants.h"

// Standard includes

// Qt includes
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QProgressBar>
#include <QProcess>
#include <QDir>
#include <QSqlDriver>

#if defined(__WIN32__) || defined(__CYGWIN32__)
#include <qtbase/src/3rdparty/sqlite/sqlite3.h>
#else
#include <qtbase/src/3rdparty/sqlite/sqlite3.h>
#endif


// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>

int sqlite_callback(void*,int,char**,char**)
{
	qDebug("sqlite_callback");
	return 0;
}

int	GexDbPlugin_Galaxy::GetNextSplitlotIndex_SQLite()
{
	qDebug("GexDbPlugin_Galaxy::UpdateSplitLotTableSQLite : ");
	int new_id=-1;
	// TO DO : find a better way to find an unused ID.
	QString	strTableName = NormalizeTableName("_SPLITLOT");

	QString	strQuery = "SELECT max(SPLITLOT_ID) FROM ";
	strQuery +=	strTableName;

	QSqlQuery	clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
	if ( !clQuery.exec(strQuery) || (!clQuery.first()) )
	{
		qDebug("GexDbPlugin_Galaxy::UpdateSplitLotTableSQLite : query execution error !");
		*m_pbDelayInsertion = true;
		GSET_ERROR1(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData());
		return -1;
	}
	int r=clQuery.value(0).toInt();
	//int m=clQuery.value(0).toInt();
	QDate d=QDate::currentDate();
	int today=d.year()-((d.year()/100)*100); today*=100000000;
	int month=d.month()*1000000; today+=month;
	int day=d.day()*10000; today+=day;
	qDebug("GexDbPlugin_Galaxy::UpdateSplitLotTableSQLite : today=%d max=%d", today, r);
	if (r>=today)
		new_id = r+1;
	else
		new_id = today;
	return new_id;
}

bool
GexDbPlugin_Galaxy::
ExecuteMultiInsertQuery_SQLite(const QString& /*strTableName*/,
							   const QString& strQuery)
{
	QSqlQuery	clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
	QString		strTruncatedQuery, strTruncatedSqlError;

        qDebug("GexDbPlugin_Galaxy::ExecuteMultiInsertQuery with SQLite: query length=%i", strQuery.length());

	// have to verify if not bigger than the max_packet_size
	if((int)strQuery.length() > m_nMaxPacketSize)
	{
		qDebug("GexDbPlugin_Galaxy::ExecuteMultiInsertQuery: SQLite : strQuery too big !");
		if(strQuery.length() >= 146)
		{
			strTruncatedQuery = strQuery.left(146);
			strTruncatedQuery += " ...";
		}
		else
			strTruncatedQuery = strQuery;
		*m_pbDelayInsertion = true;
		GSET_ERROR1(GexDbPlugin_Base, eDB_PacketSizeOverflow, NULL, strTruncatedQuery.toLatin1().constData());
		return false;
	}

	// BEGIN;
	//		INSERT INTO table VALUES(1,2,3);
	//		INSERT INTO table VALUES(4,5,6);
	// END;

	if(!clQuery.exec("BEGIN TRANSACTION;")) 		// TO DO : "BEGIN ON CONFLICT ABORT;\n";
	{
                qDebug("%s", clQuery.lastError().text().toLatin1().data());
		*m_pbDelayInsertion = true;
		GSET_ERROR2( GexDbPlugin_Base, eDB_Query, NULL, clQuery.lastQuery().toLatin1().data(),
					 clQuery.lastError().text().toLatin1().data() );
		return false;
	}

	QStringList sl;

	//strString += strQuery; strString += ";";

	/* Fast insertion : multi statement query :
		fix me : does not work anymore probably because of :
		- multithreading (QTHREAD define added in .pro)
		- or new version of sqlite

	QVariant v = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).driver()->handle();
	if ( !v.isValid() )
		qDebug("\tsqlite handle qvariant not valid !");
	else
	if ( strcmp(v.typeName(), "sqlite3*") != 0 )
		qDebug(" wrong handle type name = %s", v.typeName());
	else
	{
		// v.data() returns a pointer to the handle
		sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
		if (handle != 0)
		{
			QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).close();

			//sqlite3_open("gexdb_b9.sqlite", &handle); // To do !!! find the path to the sqlite gexdb
			//char errmsg[1024]="";
			qDebug("\tSQLite version = %s",sqlite3_libversion());
			// check that it is not NULL
			int r=sqlite3_exec( handle, strQuery.toLatin1().data(),
			  NULL, //&sqlite_callback, //int (*callback)(void*,int,char**,char**),  // Callback function
			  NULL,   // 1st argument to callback
			  NULL    // Error msg written here
			);
			//sqlite3_close(handle);
			if (r!=SQLITE_OK)
			 qDebug("sqlite3_exec error : return %d.", r);
			QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).open();
			goto commit_transaction;
		}
		else
			qDebug(" cant get sqlite handle !");
	}
	*/

	// Slow version : 1 statement per query
	sl=strQuery.split(QRegExp(";"), QString::SkipEmptyParts);
	//if (sl.count()>1)
	// qDebug(QString(" found %1 statements in query string...exple: %2").arg(sl.count()).arg(sl.at(1)).toLatin1().data());
        for (int i=0; i<sl.count(); i++)
	{
		if(!clQuery.exec(sl.at(i)))
		{
                    qDebug("%s", " query execution failed :");
                        qDebug("%s", sl.at(i).toLatin1().data());
                        qDebug("%s", clQuery.lastError().text().toLatin1().data());
			if(sl.at(i).length() >= 146)
			{
				strTruncatedQuery = sl.at(i).left(146);
				strTruncatedQuery += " ...";
			}
			else
				strTruncatedQuery = sl.at(i);
                        qDebug("%s", strTruncatedQuery.toLatin1().data());
			/*
			strString = clQuery.lastError().text();
			if(strString.length() >= 146)
			{
				strTruncatedSqlError = strString.left(146);
				strTruncatedSqlError += " ...";
			}
			else
				strTruncatedSqlError = strString;
			*/
			strTruncatedSqlError =  clQuery.lastError().text();
			*m_pbDelayInsertion = true;
			GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strTruncatedQuery.toLatin1().constData(), strTruncatedSqlError.toLatin1().constData());
			return false;
		}
		//else
		//	qDebug(QString(" query execution ok : %1 rows affected.").arg(clQuery.numRowsAffected()).toLatin1().data());
	}

//commit_transaction:

	QString strString = "COMMIT TRANSACTION;\n";		//strString += ";\nEND;\n";
	if(!clQuery.exec(strString))
	{
		qDebug("GexDbPlugin_Galaxy::ExecuteMultiInsertQuery_SQLite: COMMIT query execution failed !");
                qDebug("%s", clQuery.lastError().text().toLatin1().data());
		if(strString.length() >= 146)
		{
			strTruncatedQuery = strString.left(146);
			strTruncatedQuery += " ...";
		}
		else
			strTruncatedQuery = strString;
		strString = clQuery.lastError().text();
		if(strString.length() >= 146)
		{
			strTruncatedSqlError = strString.left(146);
			strTruncatedSqlError += " ...";
		}
		else
			strTruncatedSqlError = strString;
		*m_pbDelayInsertion = true;
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strTruncatedQuery.toLatin1().constData(), strTruncatedSqlError.toLatin1().constData());
		return false;
	}
	return true;
}

bool GexDbPlugin_Galaxy::ExecuteLoadDataInfile_SQLite(QString*	pstrTestResults, int*	pNbResults) //Stdf_Record::RecordTypes nRecordType)
{
        qDebug("GexDbPlugin_Galaxy::ExecuteLoadDataInfile: with SQLite (string size=%i)", pstrTestResults->count());

	QSqlQuery	clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

	QString	strString = "BEGIN TRANSACTION;\n"; // To Do : try "BEGIN EXCLUSIVE"
	if(!clQuery.exec("BEGIN TRANSACTION;\n"))
	{
                qDebug("GexDbPlugin_Galaxy::ExecuteLoadDataInfile: with SQLite : query BEGIN TRANSACTION failed !");
		*m_pbDelayInsertion = true;
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "BEGIN TRANSACTION", clQuery.lastError().text().toLatin1().constData());
		return false;
	}

	QStringList sl;
	QVariant v = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName).driver()->handle();
	if ( !v.isValid() )
		qDebug(" sqlite handle qvariant not valid !");
	else
	if ( strcmp(v.typeName(), "sqlite3*") != 0 )
		qDebug( " wrong handle type name = %s", v.typeName() );
	else
	{
		// v.data() returns a pointer to the handle
		sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
		if (handle != 0)
		{
			//char errmsg[1024]="";
			//qDebug(QString(" SQLite version %1").arg(sqlite3_libversion()));
			// check that it is not NULL
			int r=sqlite3_exec( handle, pstrTestResults->toLatin1().data(),
			  NULL,	//&sqlite_callback, //int (*callback)(void*,int,char**,char**),  /* Callback function */
			  NULL, /* 1st argument to callback */
              NULL  /* Err  ²                                           ²²²²²²or msg written here */
			);
			if (r!=SQLITE_OK)
			 qDebug("GexDbPlugin_Galaxy::ExecuteLoadDataInfile_SQLite: sqlite3_exec error : return %d.", r);
			goto commit_transaction;
		}
		else
			qDebug(" cant get sqlite handle !");
	}

	// Splitting the big string is many small simple query strings
	sl=(*pstrTestResults).split(QRegExp(";"), QString::SkipEmptyParts);
	if (sl.count()>1)
         qDebug(" found %i statements in query string...exple: %s", sl.count(), sl.at(1).toLatin1().data());
	for (int i=0; i<sl.count(); i++)
	{
		if (sl.at(i)=="" || sl.at(i)==";" || sl.at(i)=="  " || sl.at(i)=="\n")
			continue;
		if(!clQuery.exec(sl.at(i)))		// if(!clQuery.exec(strQuery))
		{
                        qDebug("GexDbPlugin_Galaxy::ExecuteLoadDataInfile: with SQLite : query failed : '%s'", sl.at(i).toLatin1().constData());
			*m_pbDelayInsertion = true;
			GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, sl.at(i).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
			return false;
		}
	}

commit_transaction:
	strString = "COMMIT TRANSACTION;\n";		//strString += ";\nEND;\n";
	if(!clQuery.exec(strString))
	{
		QString strTruncatedQuery, strTruncatedSqlError ;
		qDebug(" query execution failed !");
                qDebug("%s", clQuery.lastError().text().toLatin1().data());
		if(strString.length() >= 146)
		{
			strTruncatedQuery = strString.left(146);
			strTruncatedQuery += " ...";
		}
		else
			strTruncatedQuery = strString;
		strString = clQuery.lastError().text();
		if(strString.length() >= 146)
		{
			strTruncatedSqlError = strString.left(146);
			strTruncatedSqlError += " ...";
		}
		else
			strTruncatedSqlError = strString;
		*m_pbDelayInsertion = true;
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strTruncatedQuery.toLatin1().constData(), strTruncatedSqlError.toLatin1().constData());
		return false;
	}

	(*pstrTestResults)="";
	(*pNbResults)=0;

	qDebug("GexDbPlugin_Galaxy::ExecuteLoadDataInfile_SQLite finished.");

	return true;
}
