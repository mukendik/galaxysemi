#ifndef DBC_TRANSACTION_H
#define DBC_TRANSACTION_H



#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QSqlQuery>
#include <QMap>
#include <QList>


// ** CHARACTERIZATION DATABASE CONSTANTS **

// TABLES
#define DBC_PTEST_INFO_T				"ptest_info"
#define DBC_PTEST_RESULT_T				"ptest_result"
#define DBC_FTEST_INFO_T				"ftest_info"
#define DBC_FTEST_RESULT_T				"ftest_result"
#define DBC_STEST_INFO_T				"stest_info"
#define DBC_STEST_RESULT_T				"stest_result"
#define DBC_MPTEST_INFO_T				"mptest_info"
#define DBC_MPTEST_RESULT_T				"mptest_result"
#define DBC_SESSION_T					"session"
#define DBC_FILE_T						"file"
#define DBC_TEST_T						"test"
#define DBC_STEP_T						"step"
#define DBC_PGROUP_T					"pgroup"
#define DBC_L_TEST_PGROUP_T				"l_test_pgroup"
#define DBC_L_PGROUP_PGROUP_T			"l_test_pgroup"

// FIELDS
// session fields
#define DBC_SESSION_F_ID				"id"
#define DBC_SESSION_F_NAME				"name"
#define DBC_SESSION_F_DATE				"date"

// file fields
#define DBC_FILE_F_ID					"id"
#define DBC_FILE_F_SESSIONID			"session_id"
#define DBC_FILE_F_NAME					"name"
#define DBC_FILE_F_DATE					"date"
#define DBC_FILE_F_COMMENT				"comment"
#define DBC_FILE_F_PATH					"path"

// step fields
#define DBC_STEP_F_ID					"id"
#define DBC_STEP_F_SESSIONID			"session_id"
#define DBC_STEP_F_FILEID				"file_id"

// test fields
#define DBC_TEST_F_ID					"id"
#define DBC_TEST_F_NUMBER				"number"
#define DBC_TEST_F_NAME					"name"
#define DBC_TEST_F_TEST_TYPE			"test_type"
#define DBC_TEST_F_PARAM_TYPE			"param_type"
#define DBC_TEST_F_ORIGIN				"origin"
#define DBC_TEST_F_SESSIONID			"session_id"

// ptest_info fields
#define DBC_PTEST_INFO_F_ID				"id"
#define DBC_PTEST_INFO_F_TESTID			"test_id"
#define DBC_PTEST_INFO_F_LOWL			"lowl"
#define DBC_PTEST_INFO_F_HIGHL			"highl"
#define DBC_PTEST_INFO_F_UNITS			"units"
#define DBC_PTEST_INFO_F_FLAG			"flag"

// ptest_result fields
#define DBC_PTEST_RESULT_F_STEPID		"step_id"
#define DBC_PTEST_RESULT_F_TESTINFOID	"test_info_id"
#define DBC_PTEST_RESULT_F_FLOWID		"flow_id"
#define DBC_PTEST_RESULT_F_FAILED		"failed"
#define DBC_PTEST_RESULT_F_VALUE		"value"

// mptest_info fields
#define DBC_MPTEST_INFO_F_ID			"id"
#define DBC_MPTEST_INFO_F_TESTID		"test_id"
#define DBC_MPTEST_INFO_F_PIN			"pin"
#define DBC_MPTEST_INFO_F_LOWL			"lowl"
#define DBC_MPTEST_INFO_F_HIGHL			"highl"
#define DBC_MPTEST_INFO_F_UNITS			"units"
#define DBC_MPTEST_INFO_F_FLAG			"flag"

// mptest_result fields
#define DBC_MPTEST_RESULT_F_STEPID		"step_id"
#define DBC_MPTEST_RESULT_F_TESTINFOID	"test_info_id"
#define DBC_MPTEST_RESULT_F_FLOWID		"flow_id"
#define DBC_MPTEST_RESULT_F_FAILED		"failed"
#define DBC_MPTEST_RESULT_F_VALUE		"value"

// ftest_info fields
#define DBC_FTEST_INFO_F_ID				"id"
#define DBC_FTEST_INFO_F_TESTID			"test_id"
#define DBC_FTEST_INFO_F_FLAG			"flag"

// ftest_result fields
#define DBC_FTEST_RESULT_F_STEPID		"step_id"
#define DBC_FTEST_RESULT_F_TESTINFOID	"test_info_id"
#define DBC_FTEST_RESULT_F_FLOWID		"flow_id"
#define DBC_FTEST_RESULT_F_FAILED		"failed"
#define DBC_FTEST_RESULT_F_VECTOR		"vector"
#define DBC_FTEST_RESULT_F_OFFSET		"offset"

// stest_info fields
#define DBC_STEST_INFO_F_ID				"id"
#define DBC_STEST_INFO_F_TESTID			"test_id"

// stest_result fields
#define DBC_STEST_RESULT_F_STEPID		"step_id"
#define DBC_STEST_RESULT_F_TESTINFOID	"test_info_id"
#define DBC_STEST_RESULT_F_FLOWID		"flow_id"
#define DBC_STEST_RESULT_F_FAILED		"failed"
#define DBC_STEST_RESULT_F_VALUE		"value"

// l_test_pgroup fields
#define DBC_L_TEST_PGROUP_F_ID			"id"
#define DBC_L_TEST_PGROUP_F_TESTID		"test_id"
#define DBC_L_TEST_PGROUP_F_PGROUPID	"pgroup_id"
#define DBC_L_TEST_PGROUP_F_SESSIONID	"session_id"

// l_pgroup_pgroup fields
#define DBC_L_PGROUP_PGROUP_F_ID				"id"
#define DBC_L_PGROUP_PGROUP_F_PARENTPGROUPID	"parent_pgroup_id"
#define DBC_L_PGROUP_PGROUP_F_CHILDPGROUPID		"child_pgroup_id"
#define DBC_L_PGROUP_PGROUP_F_SESSIONID			"session_id"

// pgroup fields
#define DBC_PGROUP_F_ID					"id"
#define DBC_PGROUP_F_LABEL				"label"
#define DBC_PGROUP_F_SESSIONID			"session_id"

class Gate_ParameterDef;
class Gate_DataResult;
class DbSqliteMan;
class TestKey;
class DbcStep;
class DbcGroup;


class DbcTransaction : public QObject
{
Q_OBJECT
public:
	DbcTransaction(const QString& strWorkingFolder, const QString& strFileDbName, QObject *parent = 0);
	virtual ~DbcTransaction();
	void initDataBase(const QString &strDbName);
	void loadSessionInfo();
	void openSession();
	void setFileDbName(const QString& strAbsFileName);
	void setDbFolder(const QString& strDbFolder);
	void setSessionId(const QString& strSessionId);
	bool addFile(const QString& strFileName);
	void removeFile(const QString& strFileName);
	void removeTest(Gate_ParameterDef* paramInfo);
	bool loadTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo);
	bool loadPTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo);
	bool loadSTestInfo(QSqlQuery& query, Gate_ParameterDef* paramInfo);
	bool loadPTestResult(const QSqlQuery& query, Gate_DataResult& paramResult);
	
	QStringList getParamList();
	QList<Gate_ParameterDef*> testList();
	QList<QStringList> fileList();
	QList<Gate_DataResult*> testResult(const Gate_ParameterDef& paramInfo);
	QList<DbcStep> stepList(const QStringList &strLstFilter);

	QString fileDbName();
	QString sessionId();
	QString dbFolder();
	QString dbName();
	QString lastInsertedFileId();
	DbSqliteMan *dbConnection() {return m_dbInsertionCon;}
	QDateTime creationDate();
	QString testId(const Gate_ParameterDef &paramInfo);
	QString testInfoId(Gate_ParameterDef &paramInfo);
	Gate_ParameterDef* testInfo(const int & nId);
	Gate_ParameterDef* testInfo(int iNumber, const QString& strName);
	bool pTestInfo(Gate_ParameterDef* paramInfo);
	bool sTestInfo(Gate_ParameterDef* paramInfo);
	QString	addStep();
	QString insertTest(Gate_ParameterDef &paramInfo);
	bool updateTest(Gate_ParameterDef &paramInfo);
	QString insertTestInfo(Gate_ParameterDef &paramInfo);
	bool updateTestInfo(Gate_ParameterDef &paramInfo);
	QString insertGroup(const QString &strGroupName);
	QString groupId(const QString &strGroupName);
	QMap<int, DbcGroup*> groupList();
	bool updateGroupName(const QString &strGroupId, const QString &strGroupName);
	bool addTestToGroup(const QString &strTestId, const QString &strGroupName);
	bool beginTransaction();
	bool rollbackTransaction();
	bool commitTransaction();

signals:


public slots:
	void insertParameterInfo(const QMap<TestKey, Gate_ParameterDef>& mapParamInfo);
	void insertParameterResults(const QMap<TestKey, Gate_DataResult>& mapParamResultsByNumber);

private:
	
	void setDbName(const QString& strName);
	void setCreationDate(uint time_tCreation);
	void setFileListInDb(const QStringList& strLstFile);

	QString		m_strFileDbName;
	QString		m_strLastInsertedFileId;
	QString		m_strSessionId;
	QString		m_strDbFolder;
	QString		m_strDbName;
	QDateTime	m_dtCreation;
	QStringList m_strLstFileInDb;
	QString		m_strTransactionMode;
	DbSqliteMan *m_dbInsertionCon;
	QMap<TestKey, Gate_ParameterDef> m_mapCurrentParamInfo;
};

#endif // DBC_TRANSACTION_H
