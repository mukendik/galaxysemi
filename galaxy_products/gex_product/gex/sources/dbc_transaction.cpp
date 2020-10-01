
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QVariant>
#include <QSqlDriver>

#include <gqtl_log.h>
#include "dbc_group.h"
#include "dbc_step.h"
#include "gex_constants.h"
#include "db_sqlite_man.h"
#include "gate_event_manager.h"

#include "dbc_transaction.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcTransaction::DbcTransaction(const QString& strWorkingFolder, const QString& strFileDbName, QObject *parent) :
	QObject(parent)
{
	m_strLastInsertedFileId = "";
	m_strSessionId = "";
	m_strTransactionMode = "file";
	m_strFileDbName = "";
	setFileDbName(strFileDbName);
	setDbFolder(strWorkingFolder);
	m_dbInsertionCon = new DbSqliteMan(dbFolder(), fileDbName());
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcTransaction::~DbcTransaction()
{
	delete m_dbInsertionCon;
}


///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::beginTransaction()
{
	return m_dbInsertionCon->m_pDataBase->transaction();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::rollbackTransaction()
{
	return m_dbInsertionCon->m_pDataBase->rollback();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::commitTransaction()
{
	return m_dbInsertionCon->m_pDataBase->commit();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcTransaction::initDataBase(const QString &strDbName)
{
	setDbName(strDbName);
	/// TODO: add a clear database
	QString strQuery = "INSERT INTO " + QString(DBC_SESSION_T) + " "
						"(" + QString(DBC_SESSION_F_ID)		+ ", "
						"" + QString(DBC_SESSION_F_NAME)	+ ", "
						"" + QString(DBC_SESSION_F_DATE)	+ ") "
						"VALUES "
						"(\'" +	fileDbName().section(".",0,0)												+ "\', "
						"\'" +	dbName()													+ "\', "
						"\'" +	QString::number(QDateTime::currentDateTime().toTime_t())+ "\')";
	
//	DbSqliteMan dbManager(dbFolder(), fileDbName(), "init_db");
	m_dbInsertionCon->execQuery(strQuery);
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setFileDbName(const QString &strFileDbName)
{
	m_strFileDbName = strFileDbName;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setDbFolder(const QString &strDbFolder)
{
	m_strDbFolder = strDbFolder;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setDbName(const QString& strName)
{
	m_strDbName = strName;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setSessionId(const QString& strSessionId)
{
	m_strSessionId = strSessionId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setCreationDate(uint time_tCreation)
{
	m_dtCreation = QDateTime::fromTime_t(time_tCreation);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::setFileListInDb(const QStringList &strLstFile)
{
	m_strLstFileInDb = strLstFile;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::fileDbName()
{
	return m_strFileDbName;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::sessionId()
{
	return m_strSessionId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::dbFolder()
{
	return m_strDbFolder;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::dbName()
{
	return m_strDbName;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QList<QStringList> DbcTransaction::fileList()
{
	QList<QStringList> lstFileInfo;
	QString strQuery = "SELECT * FROM " + QString(DBC_FILE_T) + " "
					   "WHERE " + QString(DBC_FILE_F_SESSIONID) + "=" + sessionId();
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	while (query.next())
	{
		QStringList strLstInfo;
		int iFieldNo = query.record().indexOf(DBC_FILE_F_ID);
		strLstInfo << query.value(iFieldNo).toString();
		iFieldNo = query.record().indexOf(DBC_FILE_F_NAME);
		strLstInfo << query.value(iFieldNo).toString();
		iFieldNo = query.record().indexOf(DBC_FILE_F_DATE);
		strLstInfo << query.value(iFieldNo).toString();
		
		lstFileInfo << strLstInfo;
	}
	 
	return lstFileInfo;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QDateTime DbcTransaction::creationDate()
{
	return m_dtCreation;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::lastInsertedFileId()
{
	return m_strLastInsertedFileId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::loadSessionInfo()
{
//	DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(), "load_session");
	// Load select select info
	QString strQuery = "SELECT "
					   "" + QString(DBC_SESSION_F_ID)	+ ", "
					   "" + QString(DBC_SESSION_F_NAME)	+ ", "
					   "" + QString(DBC_SESSION_F_DATE)	+ " "
					   "FROM " + QString(DBC_SESSION_T)+ "";
	// Exec query
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	if (query.next())
	{
		// Set session attributs
		int iFieldNo = query.record().indexOf(DBC_SESSION_F_ID);
		setSessionId(query.value(iFieldNo).toString());
		iFieldNo = query.record().indexOf(DBC_SESSION_F_NAME);
		setDbName(query.value(iFieldNo).toString());
		iFieldNo = query.record().indexOf(DBC_SESSION_F_DATE);
		setCreationDate(query.value(iFieldNo).toUInt());
	}
	else
		GSLOG(SYSLOG_SEV_ERROR, QString("unable to load session %1.")
		       .arg(dbName()));
}


///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcTransaction::removeFile(const QString& strFileName)
{
	loadSessionInfo();
	// Get File Id
	QString strFileId = "";
	QString strQuery = "SELECT " + QString(DBC_FILE_F_ID) + " "
					   "FROM " + QString(DBC_FILE_T) + " "
					   "WHERE " + QString(DBC_FILE_F_NAME) + "=\'" + strFileName + "\'";
//	DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(), "file_id");
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	if (query.next())
	{
		int iFieldNo = query.record().indexOf(DBC_FILE_F_ID);
		strFileId = query.value(iFieldNo).toString();
	}

	// Get all linked step id
	QStringList lstStepId;
	strQuery = "SELECT " + QString(DBC_STEP_F_ID) + " "
			   "FROM " + QString(DBC_STEP_T) + " "
			   "WHERE " + QString(DBC_STEP_F_FILEID) + "=\'" + strFileId + "\' ORDER BY " + QString(DBC_STEP_F_ID) + " ASC";
	query = m_dbInsertionCon->execQuery(strQuery);
	while (query.next())
	{
		int iFieldNo = query.record().indexOf(DBC_STEP_F_ID);
		lstStepId << query.value(iFieldNo).toString();
	}

	if (!lstStepId.isEmpty())
	{
		// Delete linked tests
		// P-Test
		strQuery = "DELETE FROM " + QString(DBC_PTEST_RESULT_T) + " "
				   "WHERE "
				   "" + QString(DBC_PTEST_RESULT_F_STEPID) + ">= " + lstStepId.first() + " AND " + QString(DBC_PTEST_RESULT_F_STEPID) + " <= " + lstStepId.last();
		m_dbInsertionCon->execQuery(strQuery);

		// MP-Test
		strQuery = "DELETE FROM " + QString(DBC_MPTEST_RESULT_T) + " "
				   "WHERE "
				   "" + QString(DBC_MPTEST_RESULT_F_STEPID) + ">= " + lstStepId.first() + " AND " + QString(DBC_MPTEST_RESULT_F_STEPID) + " <= " + lstStepId.last();
		m_dbInsertionCon->execQuery(strQuery);		
		// F-Test
		strQuery = "DELETE FROM " + QString(DBC_FTEST_RESULT_T) + " "
				   "WHERE "
				   "" + QString(DBC_FTEST_RESULT_F_STEPID) + ">= " + lstStepId.first() + " AND " + QString(DBC_FTEST_RESULT_F_STEPID) + " <= " + lstStepId.last();
		m_dbInsertionCon->execQuery(strQuery);
		// S-Test
		strQuery = "DELETE FROM " + QString(DBC_STEST_RESULT_T) + " "
				   "WHERE "
				   "" + QString(DBC_STEST_RESULT_F_STEPID) + ">= " + lstStepId.first() + " AND " + QString(DBC_STEST_RESULT_F_STEPID) + " <= " + lstStepId.last();
		m_dbInsertionCon->execQuery(strQuery);
	}

	// Delete steps
	strQuery = "DELETE FROM " + QString(DBC_STEP_T) + " "
					   "WHERE "
					   "" + QString(DBC_STEP_F_FILEID) + "= \'" + strFileId + "\' ";
	m_dbInsertionCon->execQuery(strQuery);

	// Delete files 
	strQuery = "DELETE FROM " + QString(DBC_FILE_T) + " "
					   "WHERE "
					   "" + QString(DBC_FILE_F_ID) + "=\'"  + strFileId + "\'";

	m_dbInsertionCon->execQuery(strQuery);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::removeTest(Gate_ParameterDef* paramInfo)
{
	QString strQuery;
	if (QChar(paramInfo->m_bTestType) == 'P')
	{
		// Delete Results
		strQuery = "DELETE FROM " + QString(DBC_PTEST_RESULT_T) + " WHERE "
		        + QString(DBC_PTEST_RESULT_F_TESTINFOID) + " IN "
		        + "(SELECT " + QString(DBC_PTEST_INFO_F_ID) + " FROM " + QString(DBC_PTEST_INFO_T) 
		        + " WHERE " + QString(DBC_PTEST_INFO_F_TESTID) + "=\'" + QString::number(paramInfo->m_nId) + "\')";
		m_dbInsertionCon->execQuery(strQuery);
		// Delete info
		strQuery = "DELETE FROM " + QString(DBC_PTEST_INFO_T) + " WHERE "
		        + QString(DBC_PTEST_INFO_F_TESTID) + "=\'" + QString::number(paramInfo->m_nId) + "\'";
		m_dbInsertionCon->execQuery(strQuery);
	}
	else if (QChar(paramInfo->m_bTestType) == 'S')
	{
		// Delete Results
		strQuery = "DELETE FROM " + QString(DBC_STEST_RESULT_T) + " WHERE "
		        + QString(DBC_STEST_RESULT_F_TESTINFOID) + " IN "
		        + "(SELECT " + QString(DBC_STEST_INFO_F_ID) + " FROM " + QString(DBC_STEST_INFO_T) 
		        + " WHERE " + QString(DBC_STEST_INFO_F_TESTID) + "=\'" + QString::number(paramInfo->m_nId) + "\')";
		m_dbInsertionCon->execQuery(strQuery);
		// Delete info
		strQuery = "DELETE FROM " + QString(DBC_STEST_INFO_T) + " WHERE "
		        + QString(DBC_STEST_INFO_F_TESTID) + "=\'" + QString::number(paramInfo->m_nId) + "\'";
		m_dbInsertionCon->execQuery(strQuery);
	}
		
	// Delete TEST
	strQuery = "DELETE FROM " + QString(DBC_TEST_T) + " WHERE " + QString(DBC_TEST_F_ID) + "=\'" + QString::number(paramInfo->m_nId) + "\'";
	m_dbInsertionCon->execQuery(strQuery);
	
	
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool DbcTransaction::addFile(const QString &strFileName)
{
	QString strFile = QFileInfo(strFileName).fileName();
	m_strLastInsertedFileId = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
	QString strQuery = "INSERT INTO " + QString(DBC_FILE_T) + ""
			   "(\'" + QString(DBC_FILE_F_ID) + "\', " 
				"\'" + QString(DBC_FILE_F_SESSIONID) + "\', "
				"\'" + QString(DBC_FILE_F_NAME) + "\', "
				"\'" + QString(DBC_FILE_F_COMMENT) + "\', "
				"\'" + QString(DBC_FILE_F_DATE) + "\', "
				"\'" + QString(DBC_FILE_F_PATH) + "\') "
			   "VALUES "
			   "(\'" + m_strLastInsertedFileId + "\', "
			   "\'" + sessionId() + "\', "
			   "\'" + strFile + "\', "
			   "\'imported file\', "
			   "\'" + QString::number(QDateTime::currentDateTime().toTime_t()) + "\', "
			   "\'" + QFileInfo(strFileName).absolutePath() + "\')";
	
//	DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(), "add_file");
	m_dbInsertionCon->execQuery(strQuery);
	return true;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::insertParameterInfo(const QMap<TestKey, Gate_ParameterDef> & mapParamInfo)
{
	m_mapCurrentParamInfo = mapParamInfo;
	QString strQuery = "";
	//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName());
	QMapIterator<TestKey, Gate_ParameterDef> itParamInfo(m_mapCurrentParamInfo);
	while (itParamInfo.hasNext())
	{
		itParamInfo.next();
		Gate_ParameterDef paramDef= itParamInfo.value();
		QString strTestId = testId(paramDef);
		if (strTestId.isEmpty())
		{
			strTestId = insertTest(paramDef);
		}
		paramDef.m_nId = strTestId.toInt();
		QString strTestInfoId = testInfoId(paramDef);
		if (strTestInfoId.isEmpty())
		{
			strTestInfoId = insertTestInfo(paramDef);
		}
		m_mapCurrentParamInfo[itParamInfo.key()].m_nParameterIndex = strTestInfoId.toInt();
	}

	m_dbInsertionCon->execMultiStatementsQuery(strQuery);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void DbcTransaction::insertParameterResults(const QMap<TestKey, Gate_DataResult> &mapParamResultsByNumber)
{
	QString strStepId = addStep();
	QString strQuery = "";
//	DbSqliteMan dbManager(this->dbFolder(), this->fileDbName());
	QMapIterator <TestKey, Gate_DataResult> itResult(mapParamResultsByNumber);

	while (itResult.hasNext())
	{
		itResult.next();
		if (!m_mapCurrentParamInfo.contains(itResult.key()))
		{
			GSLOG(SYSLOG_SEV_ERROR, QString("No info for param: number: %1, name: %2.")
			       .arg(QString::number(itResult.key().number()))
			       .arg(itResult.key().name()));
			return;
		}

		Gate_ParameterDef paramInfo = m_mapCurrentParamInfo.value(itResult.key());
		Gate_DataResult paramResult = itResult.value();
		QString strTestInfoId = QString::number(paramInfo.m_nParameterIndex);
		
		if (QChar(paramInfo.m_bTestType) == 'P')
		{
			strQuery += "INSERT INTO " + QString(DBC_PTEST_RESULT_T) + " "
							   "(" + QString(DBC_PTEST_RESULT_F_STEPID) + ", "
							   "" + QString(DBC_PTEST_RESULT_F_TESTINFOID) + ", "
							   "" + QString(DBC_PTEST_RESULT_F_FLOWID) + ", "
							   "" + QString(DBC_PTEST_RESULT_F_FAILED) + ", "
							   "" + QString(DBC_PTEST_RESULT_F_VALUE) + ") "
							   "VALUES "
							   "(" + strStepId + ", "
							   "" + strTestInfoId + ", "
							   "" + QString::number(paramResult.m_nFlowId) + ", "
							   "" + QString::number(paramResult.m_bTestFailed) + ", "
							   "" + QString::number(paramResult.m_lfValue) + "); ";
		}
		else if (QChar(paramInfo.m_bTestType) == 'F')
		{
			
		}
		else if (QChar(paramInfo.m_bTestType) == 'M')
		{
			
		}
		else if (QChar(paramInfo.m_bTestType) == 'S')
		{
			strQuery += "INSERT INTO " + QString(DBC_STEST_RESULT_T) + " "
							   "(" + QString(DBC_STEST_RESULT_F_STEPID) + ", "
							   "" + QString(DBC_STEST_RESULT_F_TESTINFOID) + ", "
							   "" + QString(DBC_STEST_RESULT_F_FLOWID) + ", "
							   "" + QString(DBC_STEST_RESULT_F_FAILED) + ", "
							   "" + QString(DBC_STEST_RESULT_F_VALUE) + ") "
							   "VALUES "
							   "(" + strStepId + ", "
							   "" + strTestInfoId + ", "
							   "" + QString::number(paramResult.m_nFlowId) + ", "
							   "" + QString::number(paramResult.m_bTestFailed) + ", "
							   "\'" + paramResult.m_strValue + "\'); ";
		}
	}
	
	m_dbInsertionCon->execMultiStatementsQuery(strQuery);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::testId(const Gate_ParameterDef &paramInfo)
{
	QString strTestId = "";
	QString strQuery = "SELECT id FROM " + QString(DBC_TEST_T) + " "
					   "WHERE "
					   "" + QString(DBC_TEST_F_NUMBER) + "  = \'" + QString::number(paramInfo.m_nParameterNumber) + "\' ";
	if (!paramInfo.m_strName.isEmpty())
	{
		strQuery +=    "AND "
					   "" + QString(DBC_TEST_F_NAME) + "  = \'" + paramInfo.m_strName + "\' ";
	}

	//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(), "test_id");
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	if (query.next())
	{
		int iFieldNo = query.record().indexOf("id");
		strTestId = query.value(iFieldNo).toString();
	}
	return strTestId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::testInfoId(Gate_ParameterDef &paramInfo)
{
	if (paramInfo.m_nId == 0)
		paramInfo.m_nId = testId(paramInfo).toInt();
	
	
	QString strTestInfoId = "";
	QString strQuery;
	if (QChar(paramInfo.m_bTestType) == 'P')
	{
		strQuery = "SELECT " + QString(DBC_PTEST_INFO_F_ID) + " FROM " + QString(DBC_PTEST_INFO_T) + " "
							   "WHERE "
							   "" + QString(DBC_PTEST_INFO_F_TESTID) + "  = \'" + QString::number(paramInfo.m_nId) + "\' "
							   "AND "
							   "" + QString(DBC_PTEST_INFO_F_LOWL) + "  = \'" + QString::number(paramInfo.m_lfLowL) + "\' "
							   "AND "
							    "" + QString(DBC_PTEST_INFO_F_HIGHL) + "  = \'" + QString::number(paramInfo.m_lfHighL) + "\' ";
	}
	else if (QChar(paramInfo.m_bTestType) == 'F')
	{

	}
	else if (QChar(paramInfo.m_bTestType) == 'M')
	{

	}
	else if (QChar(paramInfo.m_bTestType) == 'S')
	{
		strQuery = "SELECT " + QString(DBC_STEST_INFO_F_ID) + " FROM " + QString(DBC_STEST_INFO_T) + " "
							   "WHERE "
							   "" + QString(DBC_STEST_INFO_F_TESTID) + "  = \'" + QString::number(paramInfo.m_nId) + "\' ";
	}

	//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(), "test_id");
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	if (query.next())
	{
		int iFieldNo = query.record().indexOf(QString(DBC_PTEST_INFO_F_ID));
		strTestInfoId = query.value(iFieldNo).toString();
	}
	return strTestInfoId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::insertTest(Gate_ParameterDef &paramInfo)
{
	QString strTestId = "";
	QString strQuery = "INSERT INTO " + QString(DBC_TEST_T) + " "
				   "(" + QString(DBC_TEST_F_ID) + ", "
				   "" + QString(DBC_TEST_F_NUMBER) + ", "
				   "" + QString(DBC_TEST_F_NAME) + ", "
				   "" + QString(DBC_TEST_F_TEST_TYPE) + ", "
				   "" + QString(DBC_TEST_F_PARAM_TYPE) + ", "
				   "" + QString(DBC_TEST_F_ORIGIN) + ", "
				   "" + QString(DBC_TEST_F_SESSIONID) + ") "
				   "VALUES "
				   "(NULL, "
				   "\'" + QString::number(paramInfo.m_nParameterNumber) + "\', "
				   "\'" + QString(paramInfo.m_strName) + "\', "
				   "\'" + QString(paramInfo.m_bTestType) + "\', "
				   "\'Generic\', "
				   "\'" + m_strTransactionMode + "\', "
				   "\'" + sessionId() + "\') ";
		//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(),"find_test");
		QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
		strTestId = query.lastInsertId().toString();

	if (!strTestId.isEmpty())
	{
		QString strGroupName = "All";
		// Add Test to group all
		if (addTestToGroup(strTestId, strGroupName) == false)
			GSLOG(SYSLOG_SEV_ERROR, QString("unable to add %1 to %2.")
			       .arg(strTestId)
			       .arg(strGroupName));
	}
	
	return strTestId;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool DbcTransaction::updateTest(Gate_ParameterDef &paramInfo)
{
	if (paramInfo.m_nId == 0)
		paramInfo.m_nId = testId(paramInfo).toInt();
	
	QString strQuery = "UPDATE " + QString(DBC_TEST_T) + " SET "
	        "" + QString(DBC_TEST_F_NUMBER)		+ "=\'" + QString::number(paramInfo.m_nParameterNumber) + "\', "
	        "" + QString(DBC_TEST_F_NAME)		+ "=\'" + QString(paramInfo.m_strName) + "\', "
	        "" + QString(DBC_TEST_F_TEST_TYPE)	+ "=\'" + QString(paramInfo.m_bTestType) + "\', "
	        "" + QString(DBC_TEST_F_PARAM_TYPE) + "=\'Generic\', "
	        "" + QString(DBC_TEST_F_ORIGIN)		+ "=\'" + m_strTransactionMode + "\' "
	        "WHERE "
	        "" + QString(DBC_TEST_F_ID) + "=\'" + QString::number(paramInfo.m_nId) + "\'";
	
	QString strErrorMsg = "";
	m_dbInsertionCon->execQuery(strQuery,&strErrorMsg);
	if (!strErrorMsg.isEmpty())
		return false;
	
	return true;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::insertTestInfo(Gate_ParameterDef &paramInfo)
{
	QString strTestInfoId = "";
	QString strQuery = "";
	if (QChar(paramInfo.m_bTestType) == 'P')
	{
		strQuery = "INSERT INTO " + QString(DBC_PTEST_INFO_T) + " "
				"(" + QString(DBC_PTEST_INFO_F_ID) + ", "
				"" + QString(DBC_PTEST_INFO_F_TESTID) + ", "
				"" + QString(DBC_PTEST_INFO_F_LOWL) + ", "
				"" + QString(DBC_PTEST_INFO_F_HIGHL) + ", "
				"" + QString(DBC_PTEST_INFO_F_UNITS) + ", "
				"" + QString(DBC_PTEST_INFO_F_FLAG) + ") "
				"VALUES "
				"(NULL,"
				"\'" + QString::number(paramInfo.m_nId) + "\', "
				"\'" + QString::number(paramInfo.m_lfLowL) + "\', "
				"\'" + QString::number(paramInfo.m_lfHighL) + "\', "
				"\'" + paramInfo.m_strUnits + "\', "
				"\'" + QString::number(paramInfo.m_uiFlags) + "\') ";
	}
	else if (QChar(paramInfo.m_bTestType) == 'F')
	{
		
	}
	else if (QChar(paramInfo.m_bTestType) == 'M')
	{
		
	}
	else if (QChar(paramInfo.m_bTestType) == 'S')
	{
		strQuery = "INSERT INTO " + QString(DBC_STEST_INFO_T) + " "
				"(" + QString(DBC_STEST_INFO_F_ID) + ", "
				"" + QString(DBC_STEST_INFO_F_TESTID) + ") "
				"VALUES "
				"(NULL, "
				"\'" + QString::number(paramInfo.m_nId) + "\') ";
	}

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	strTestInfoId = query.lastInsertId().toString();

	return strTestInfoId;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool DbcTransaction::updateTestInfo(Gate_ParameterDef &paramInfo)
{
	if (paramInfo.m_nId == 0)
		paramInfo.m_nId = testId(paramInfo).toInt();
	
	QString strQuery = "";
	if (QChar(paramInfo.m_bTestType) == 'P')
	{
		strQuery = "UPDATE " + QString(DBC_PTEST_INFO_T) + " SET "
				"" + QString(DBC_PTEST_INFO_F_LOWL)		+ "=\'" + QString::number(paramInfo.m_lfLowL) + "\', "
				"" + QString(DBC_PTEST_INFO_F_HIGHL)	+ "=\'" + QString::number(paramInfo.m_lfHighL) + "\', "
				"" + QString(DBC_PTEST_INFO_F_UNITS)	+ "=\'" + paramInfo.m_strUnits + "\', "
				"" + QString(DBC_PTEST_INFO_F_FLAG)		+ "=\'" + QString::number(paramInfo.m_uiFlags) + "\' "
		        "WHERE "
		        "" + QString(DBC_PTEST_INFO_F_TESTID)	+ "=\'" + QString::number(paramInfo.m_nId) + "\'";
	}
	else if (QChar(paramInfo.m_bTestType) == 'F')
	{
		
	}
	else if (QChar(paramInfo.m_bTestType) == 'M')
	{
		
	}
	else if (QChar(paramInfo.m_bTestType) == 'S')
	{
		/*strQuery = "UPDATE " + QString(DBC_STEST_INFO_T) + " SET "
				"" + QString(DBC_PTEST_INFO_F_UNITS)	+ "=\'" + paramInfo.m_strUnits + "\', "
		        "WHERE "
		        "" + QString(DBC_STEST_INFO_F_TESTID)	+ "=\'" + QString::number(paramInfo.m_nId) + "\'";*/
		// Nothing to update at the moment
		return true;
	}
	
	QString strErrorMsg = "";
	m_dbInsertionCon->execQuery(strQuery, &strErrorMsg);
	if (!strErrorMsg.isEmpty())
		return false;

	return true;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::insertGroup(const QString &strGroupName)
{
	if (strGroupName.isEmpty())
		return QString();
	
	QString strGroupId = "";
	QString strQuery = "INSERT INTO " + QString(DBC_PGROUP_T) + " "
				   "(" + QString(DBC_PGROUP_F_ID) + ", "
				   "" + QString(DBC_PGROUP_F_LABEL) + ", "
				   "" + QString(DBC_PGROUP_F_SESSIONID) + ") "
				   "VALUES "
				   "(NULL, "
				   "\'" + strGroupName + "\', "
				   "\'" + sessionId() + "\') ";
		//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName(),"find_test");
		QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
		strGroupId = query.lastInsertId().toString();

	return strGroupId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::groupId(const QString &strGroupName)
{
	if (strGroupName.isEmpty())
		return QString();
	
	QString strGroupId = "";
	QString strQuery = "SELECT " + QString(DBC_PGROUP_F_ID) + " FROM " + QString(DBC_PGROUP_T) + " "
					   "WHERE "
					   "" + QString(DBC_PGROUP_F_LABEL) + "  = \'" + strGroupName + "\' ";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	if (query.next())
	{
		int iFieldNo = query.record().indexOf("id");
		strGroupId = query.value(iFieldNo).toString();
	}
	return strGroupId;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool DbcTransaction::updateGroupName(const QString &strGroupId, const QString &strGroupName)
{
	if (strGroupId.isEmpty())
		return false;
	
	QString strQuery = "";

	strQuery = "UPDATE " + QString(DBC_PGROUP_T) + " SET "
			"" + QString(DBC_PGROUP_F_LABEL)	+ "=\'" + strGroupName + "\' "
			"WHERE "
			"" + QString(DBC_PGROUP_F_ID)		+ "=\'" + strGroupId + "\'";
	
	QString strErrorMsg = "";
	m_dbInsertionCon->execQuery(strQuery, &strErrorMsg);
	if (!strErrorMsg.isEmpty())
		return false;

	return true;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool DbcTransaction::addTestToGroup(const QString &strTestId, const QString &strGroupName)
{
	if (strGroupName.isEmpty() || strTestId.isEmpty())
		return false;
	
	QString strGroupId = groupId(strGroupName);
	if (strGroupId.isEmpty())
		strGroupId = insertGroup(strGroupName);
	
	if (strGroupId.isEmpty())
		return false;
	
	QString strQuery = "INSERT INTO " + QString(DBC_L_TEST_PGROUP_T) + " "
				   "(" + QString(DBC_L_TEST_PGROUP_F_ID) + ", "
				   "" + QString(DBC_L_TEST_PGROUP_F_TESTID) + ", "
				   "" + QString(DBC_L_TEST_PGROUP_F_PGROUPID) + ", "
				   "" + QString(DBC_L_TEST_PGROUP_F_SESSIONID) + ") "
				   "VALUES "
				   "(NULL, "
				   "\'" + strTestId + "\', "
				   "\'" + strGroupId + "\', "
				   "\'" + sessionId() + "\') ";
	QString strErrorMsg = "";
	m_dbInsertionCon->execQuery(strQuery,&strErrorMsg);
	
	if (!strErrorMsg.isEmpty())
		return false;
	
	return true;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QMap<int, DbcGroup*> DbcTransaction::groupList()
{
	QMap<int, DbcGroup*> mapGroupParameterList;
	
	QString strQuery = "SELECT " + QString(DBC_L_TEST_PGROUP_F_PGROUPID) + ", "
									+ QString(DBC_PGROUP_T) + "." + QString(DBC_PGROUP_F_LABEL) + " as group_name, " 
									+ QString(DBC_PGROUP_T) + "." + QString(DBC_PGROUP_F_ID) + " as group_id, " 
									+ QString(DBC_L_TEST_PGROUP_F_TESTID) + " " 
					"FROM " + QString(DBC_PGROUP_T) + " "
					"LEFT OUTER JOIN "+ QString(DBC_L_TEST_PGROUP_T) + " "
									"ON " + QString(DBC_L_TEST_PGROUP_T) + "." + QString(DBC_L_TEST_PGROUP_F_PGROUPID) + "=" 
											+ QString(DBC_PGROUP_T) + "." + QString(DBC_PGROUP_F_ID) + " " 
					"LEFT OUTER JOIN "+ QString(DBC_TEST_T) + " "
									"ON " + QString(DBC_L_TEST_PGROUP_T) + "." + QString(DBC_L_TEST_PGROUP_F_TESTID) + "=" 
											+ QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_ID) + " "
					"WHERE " + QString(DBC_PGROUP_T) + "." + QString(DBC_PGROUP_F_SESSIONID) + "=\'" + sessionId() + "\'";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	while (query.next())
	{
		int iFieldNo = query.record().indexOf("group_id");
		int iGroupId = query.value(iFieldNo).toInt();
		if (!mapGroupParameterList.contains(iGroupId))
		{
			DbcGroup *newGroup = new DbcGroup();
			newGroup->setGroupId(iGroupId);
			iFieldNo = query.record().indexOf("group_name");
			QString strGroupName = query.value(iFieldNo).toString();
			newGroup->setGroupName(strGroupName);
			mapGroupParameterList.insert(iGroupId, newGroup);
		}
		
		iFieldNo = query.record().indexOf(DBC_L_TEST_PGROUP_F_TESTID);
		if (!query.value(iFieldNo).toString().trimmed().isEmpty())
		{
			int iTestId = query.value(iFieldNo).toInt();
			mapGroupParameterList.value(iGroupId)->addParameter(iTestId);
		}
	}

	return mapGroupParameterList;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QString DbcTransaction::addStep()
{
	QString strStepId = "";
	QString strQuery = "INSERT INTO " + QString(DBC_STEP_T) + " "
			   "(" + QString(DBC_STEP_F_ID) + ", "
			   "" + QString(DBC_STEP_F_SESSIONID) + ", "
			   "" + QString(DBC_STEP_F_FILEID) + ") "
			   "VALUES "
			   "(NULL, "
			   "\'" + sessionId() + "\', "
			   "\'" + lastInsertedFileId() + "\') ";
	//DbSqliteMan dbManager(this->dbFolder(), this->fileDbName());
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);

	strStepId = query.lastInsertId().toString();
	return strStepId;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QList<Gate_ParameterDef*> DbcTransaction::testList()
{
	QList<Gate_ParameterDef*> lstTestInfo;
	QString strQuery = "SELECT * FROM " + QString(DBC_TEST_T) + " "
						"WHERE " + QString(DBC_TEST_F_SESSIONID) + "=" + sessionId();
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	while (query.next())
	{
		Gate_ParameterDef* paramInfo = new Gate_ParameterDef();
		if (loadTestInfo(query, paramInfo))
			lstTestInfo << paramInfo;
	}

	return lstTestInfo;
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
QList<Gate_DataResult*> DbcTransaction::testResult(const Gate_ParameterDef& paramInfo)
{
	QList<Gate_DataResult*> lstTestResult;
	QString strQuery = "SELECT * FROM ";

	switch (paramInfo.m_bTestType)
	{
		case 'P':
			strQuery += QString(DBC_PTEST_RESULT_T);
			break;
		case 'F':
			strQuery += QString(DBC_FTEST_RESULT_T);
			break;
		case 'M':
			strQuery += QString(DBC_MPTEST_RESULT_T);
			break;
		case 'S':
			strQuery += QString(DBC_STEST_RESULT_T);
			break;
		default:
			break;
	}

	strQuery += " WHERE " + QString(DBC_PTEST_RESULT_F_TESTINFOID) + " = " + QString::number(paramInfo.m_nId);
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	while (query.next())
	{
		Gate_DataResult* paramResult = new Gate_DataResult();
		loadPTestResult(query, *paramResult);
		lstTestResult << paramResult;
	}
	
	return lstTestResult;
}


///////////////////////////////////////////////////////////
// Return step list linked to the active session
///////////////////////////////////////////////////////////
QList<DbcStep> DbcTransaction::stepList(const QStringList &strLstFilter)
{
	QList<DbcStep> lstStep;
	
	// Build request
	QString strQuery = "SELECT * FROM " + QString(DBC_STEP_T) + " ";
	// If existing filters to apply
	if (!strLstFilter.isEmpty())
	{
		QStringListIterator itFilter(strLstFilter);
		strQuery += "WHERE " + itFilter.next() + " ";
		while (itFilter.hasNext())
		{
			strQuery += "AND " + itFilter.next() + " ";
		}
	}
	// Exec query
	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);
	
	// Get all retreiving steps
	while (query.next())
	{
		DbcStep step;
		int iFieldNo = query.record().indexOf(DBC_STEP_F_ID);
		step.setId(query.value(iFieldNo).toInt());
		iFieldNo = query.record().indexOf(DBC_STEP_F_SESSIONID);
		step.setSessionId(query.value(iFieldNo).toString());
		iFieldNo = query.record().indexOf(DBC_STEP_F_FILEID);
		step.setSessionId(query.value(iFieldNo).toString());
		lstStep << step;
	}
	
	return lstStep;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::loadPTestResult(const QSqlQuery& query, Gate_DataResult& paramResult)
{
	int iFieldNo = query.record().indexOf(DBC_PTEST_RESULT_F_VALUE);
	paramResult.m_lfValue = query.value(iFieldNo).toDouble();
	iFieldNo = query.record().indexOf(DBC_PTEST_RESULT_F_FAILED);
	paramResult.m_bTestFailed = query.value(iFieldNo).toBool();
	iFieldNo = query.record().indexOf(DBC_PTEST_RESULT_F_FLOWID);
	paramResult.m_nFlowId = query.value(iFieldNo).toInt();
	iFieldNo = query.record().indexOf(DBC_PTEST_RESULT_F_STEPID);
	paramResult.m_nStepNum = query.value(iFieldNo).toInt();

	return true;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::loadTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo)
{
	int iFieldNo = query.record().indexOf(DBC_TEST_F_ID);
	paramInfo->m_nId = query.value(iFieldNo).toInt();
	iFieldNo = query.record().indexOf(DBC_TEST_F_NAME);
	paramInfo->m_strName = query.value(iFieldNo).toString();
	iFieldNo = query.record().indexOf(DBC_TEST_F_NUMBER);
	paramInfo->m_nParameterNumber = query.value(iFieldNo).toInt();
	iFieldNo = query.record().indexOf(DBC_TEST_F_TEST_TYPE);
	paramInfo->m_bTestType = (BYTE) (query.value(iFieldNo).toString()[0].toLatin1());
	
	return true;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::loadPTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo)
{
	int iFieldNo = query.record().indexOf(DBC_PTEST_INFO_F_TESTID);
	paramInfo->m_nId = query.value(iFieldNo).toInt();
	iFieldNo = query.record().indexOf(DBC_PTEST_INFO_F_UNITS);
	paramInfo->m_strUnits = query.value(iFieldNo).toString();
	iFieldNo = query.record().indexOf(DBC_PTEST_INFO_F_HIGHL);
	paramInfo->m_lfHighL = query.value(iFieldNo).toDouble();
	iFieldNo = query.record().indexOf(DBC_PTEST_INFO_F_LOWL);
	paramInfo->m_lfLowL = query.value(iFieldNo).toDouble();
	iFieldNo = query.record().indexOf(DBC_PTEST_INFO_F_ID);
	
	paramInfo->m_lstnInfoId << query.value(iFieldNo).toInt();
	// Can have several info because o multiple limits
	while (query.next())
		paramInfo->m_lstnInfoId << query.value(iFieldNo).toInt();

	return true;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::loadSTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo)
{
	int iFieldNo = query.record().indexOf(DBC_STEST_INFO_F_TESTID);
	paramInfo->m_nId = query.value(iFieldNo).toInt();
	iFieldNo = query.record().indexOf(DBC_STEST_INFO_F_ID);
	
	paramInfo->m_lstnInfoId << query.value(iFieldNo).toInt();
	// Can have several info because o multiple limits
	while (query.next())
		paramInfo->m_lstnInfoId << query.value(iFieldNo).toInt();

	return true;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
Gate_ParameterDef* DbcTransaction::testInfo(const int & nId)
{
	QString strQuery = "SELECT * FROM " + QString(DBC_TEST_T) + " "
					   "WHERE " + QString(DBC_TEST_F_SESSIONID) + "=" + sessionId() + " "
					   "AND " + QString(DBC_TEST_F_ID) + "=" + QString::number(nId) + "";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);

	if (query.next())
	{
		Gate_ParameterDef* paramInfo = new Gate_ParameterDef();
		if (loadTestInfo(query, paramInfo))
		{
			switch (paramInfo->m_bTestType)
			{
				case 'P':
					pTestInfo(paramInfo);
					break;
				case 'F':
					/// TODO
					break;
				case 'M':
					/// TODO
					break;
				case 'S':
					sTestInfo(paramInfo);
					break;
				default:
					break;
			}
			return paramInfo;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
Gate_ParameterDef* DbcTransaction::testInfo(int iNumber, const QString& strName)
{
	QString strQuery = "SELECT * FROM " + QString(DBC_TEST_T) + " "
					   "WHERE " + QString(DBC_TEST_F_SESSIONID) + "=" + sessionId() + " "
					   "AND " + QString(DBC_TEST_F_NUMBER) + "=\"" + QString::number(iNumber) + "\" "
					   "AND " + QString(DBC_TEST_F_NAME) + "=\"" + strName + "\"";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);

	if (query.next())
	{
		Gate_ParameterDef* paramInfo = new Gate_ParameterDef();
		if (loadTestInfo(query, paramInfo))
		{
			switch (paramInfo->m_bTestType)
			{
				case 'P':
					pTestInfo(paramInfo);
					break;
				case 'F':
					/// TODO
					break;
				case 'M':
					/// TODO
					break;
				case 'S':
					sTestInfo(paramInfo);
					break;
				default:
					break;
			}
			return paramInfo;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::pTestInfo(Gate_ParameterDef* paramInfo)
{
	QString strQuery = "SELECT * FROM "+ QString(DBC_PTEST_INFO_T) + " "
					   "WHERE " + QString(DBC_PTEST_INFO_F_TESTID) + "=" + QString::number(paramInfo->m_nId) + " ";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);

	if (query.next())
	{
		if (loadPTestInfo(query, paramInfo))
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcTransaction::sTestInfo(Gate_ParameterDef* paramInfo)
{
	QString strQuery = "SELECT * FROM "+ QString(DBC_STEST_INFO_T) + " "
					   "WHERE " + QString(DBC_STEST_INFO_F_TESTID) + "=" + QString::number(paramInfo->m_nId) + " ";

	QSqlQuery query = m_dbInsertionCon->execQuery(strQuery);

	if (query.next())
	{
		if (loadSTestInfo(query, paramInfo))
			return true;
	}

	return false;
}



