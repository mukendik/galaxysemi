// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Limit file size to exclude table for immediate conversion and use batch transfert
// in Mo
#define LIMIT_FILE_SIZE 1024.0
// Limit number splitlots per single transfert query
#define LIMIT_NB_SPLITLOTS 100

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "import_constants.h"

// Standard includes
#include <math.h>

// Qt includes
#include <QProgressDialog>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QDir>
#include <QProgressBar>
#include <QApplication>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>
#include <gqtl_log.h>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// getFreeTotalSpace
////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#elif (defined __sun__) // Solaris stuff
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/statvfs.h>
#elif defined __MACH__
    #include <sys/uio.h>
    #include <sys/mount.h>
    #include <sys/stat.h>
#else // linux stuff
    #include <sys/vfs.h>
    #include <sys/stat.h>
#endif // _WIN32

bool getFreeTotalSpace(const QString& sDirPath,double& fTotal, double& fFree)
{
#ifdef _WIN32

  QString sCurDir = QDir::current().absolutePath();
  QDir::setCurrent( sDirPath );

  ULARGE_INTEGER free,total;
  bool bRes = ::GetDiskFreeSpaceExA( 0 , &free , &total , NULL );
  if ( !bRes ) return false;

  QDir::setCurrent( sCurDir );

  fFree = static_cast<double>( static_cast<__int64>(free.QuadPart) );
  fTotal = static_cast<double>( static_cast<__int64>(total.QuadPart) );

#elif (defined __sun__) // Solaris

  struct stat stst;
  struct statvfs stfs;

  if ( ::stat(sDirPath.local8Bit(),&stst) == -1 ) return false;
  if ( ::statvfs(sDirPath.local8Bit(),&stfs) == -1 ) return false;

  fFree = stfs.f_bavail * ( stst.st_blksize );
  fTotal = stfs.f_blocks * ( stst.st_blksize );

#else //linux

  struct stat stst;
  struct statfs stfs;

  if (::stat(sDirPath.toLocal8Bit().constData(),
      &stst) == -1 )  // local8Bit() ??
      return false;
  if (::statfs(sDirPath.toLocal8Bit().constData(),
      &stfs) == -1 ) // Sandrine ?
      return false;

  fFree = stfs.f_bavail * ( stst.st_blksize );
  fTotal = stfs.f_blocks * ( stst.st_blksize );

#endif // _WIN32

   return true;
}





///////////////////////////////////////////////////////////
// QueryThread
// Create a thread to execute QSqlQuery
// Emit processEvents and progressBar control events
// Display progressDialog after 2 secondes and progressBar < 30%
///////////////////////////////////////////////////////////
// progressBar control
// nQueryType = QUERYTHREAD_TYPE_DEFAULT
//		no progressBar control
//		progressBar incremented each 2 secondes
// nQueryType = QUERYTHREAD_TYPE_PARTITIONING OR QUERYTHREAD_TYPE_INNODB OR QUERYTHREAD_TYPE_BARRACUDA
//		progressBar control
//		check the evolution of MySql data file size
///////////////////////////////////////////////////////////
bool QueryThread::exec(QSqlQuery *pQuery, int nType, QString strDir, QString strFile)
{
  QTime	clLastTimeUpdate;
  float	fDataFilesSize;
  float	fMyIsamSize;
  int		nStep;
  int		nQuoteSize;
  int		nQueryType;
  int		nProgressStep;

  QString		strMsg;
  QDir		clDir;
  QFileInfo	clFile;

  m_pQuery=pQuery;
  nQueryType = nQuoteSize = nType;
  nStep = 0;
  nProgressStep = 0;
  fMyIsamSize = 1;
  fDataFilesSize = 1;

  // Check the driver version
  // File size only for MySql
  QString strDriverName;
  if (!m_pQuery)
    return false;
  QVariant v = m_pQuery->driver()->handle();
  if (v.isValid())
    strDriverName = v.typeName();

  if(!strDriverName.contains("MYSQL",Qt::CaseInsensitive))
    nQueryType = QUERYTHREAD_TYPE_DEFAULT;

  if(nQueryType==QUERYTHREAD_TYPE_DEFAULT)
    emit newText ("Execute SqlQuery:\n wait one moment... ");
  else
  if(nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
    emit newText ("Partition creation for "+strFile+":\n wait one moment... ");
  else
  if(nQueryType==QUERYTHREAD_TYPE_INNODB)
    emit newText ("Transfert table "+strFile+" to InnoDb engine:\n wait one moment... ");
  else
  if(nQueryType==QUERYTHREAD_TYPE_BARRACUDA)
    emit newText ("Transfert table "+strFile+" to InnoDb Barracuda engine:\n wait one moment... ");

  if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
  || (nQueryType==QUERYTHREAD_TYPE_INNODB)
  || (nQueryType==QUERYTHREAD_TYPE_BARRACUDA))
  {
    // For MySql
    // The progress for table conversion can be check with the size of MyIsam data file

    // progressBar use the Data file size
    // Get the original size
    clDir.setPath(strDir);
    clDir.setFilter(QDir::Files);
    // MyIsam files size (data/index)
    fDataFilesSize = 1;
    QStringList lstDataFiles = clDir.entryList(QStringList(strFile+"*.MY*"));
    QStringList::iterator it;
    for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
    {
      if((*it).contains("_myisam",Qt::CaseInsensitive))
        continue;
      if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
      && ((*it).contains("#P#",Qt::CaseInsensitive)))
        continue;

      clFile.setFile(strDir,*it);
      fDataFilesSize+= clFile.size();
    }
    fMyIsamSize = fDataFilesSize;

    if(nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
      strMsg = "Partitioning MyIsam table "+strFile;
    else
    if(nQueryType==QUERYTHREAD_TYPE_INNODB)
      strMsg = "Converting MyIsam table "+strFile+" to InnoDb";
    else
      strMsg = "Converting MyIsam table "+strFile+" to InnoDB Barracuda format";
    strMsg+= ".\nPlease wait ...";

    emit newText(strMsg);
  }

  clLastTimeUpdate.start();

  // Execute the Query
  start();

  // If cannot retrieve the size
  // Change to a standard progressBar
  if(fMyIsamSize <= 1)
    nQueryType = QUERYTHREAD_TYPE_DEFAULT;

  // Wait the end of the query
  // Update the progressDialog with event
  while(isRunning() || !isFinished())
  {

    // Each 2 secondes
    if((clLastTimeUpdate.elapsed() > 2000)
    && (nProgressStep <= 100))
    {
      nStep++;

      if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
      || (nQueryType==QUERYTHREAD_TYPE_INNODB)
      || (nQueryType==QUERYTHREAD_TYPE_BARRACUDA))
      {
        // progressBar use the Data file size
        // Get the original size
        clDir.setPath(strDir);
        clDir.setFilter(QDir::Files);
        // MyIsam files size (data/index)
        fDataFilesSize = 0;
        QStringList lstDataFiles;

        // check file as #P# for partitioning
        if(nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
          lstDataFiles = clDir.entryList(QStringList("#sql*.MY*"));
        else
        // Check temporary file as #sql for InnoDb
        if((nQueryType==QUERYTHREAD_TYPE_INNODB)|| (nQueryType==QUERYTHREAD_TYPE_BARRACUDA))
          lstDataFiles = clDir.entryList(QStringList("#sql*.ibd"));

        QStringList::iterator it;
        for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
        {
          clFile.setFile(strDir,*it);
          fDataFilesSize+= clFile.size();
        }
        nProgressStep = (int)((fDataFilesSize)/(fMyIsamSize*nQuoteSize)*100.0);

        // Probably the Estimation QuoteSize between table1 to table2 is not good
        // Example:
        // Partition is 1 to 1
        // InnoDb is 1 to 4
        if(nProgressStep > 100)
        {
          nQuoteSize++;
          nProgressStep = (int)((fDataFilesSize)/(fMyIsamSize*nQuoteSize)*100.0);
        }
      }
      else
      {
        nProgressStep++;
        if(nProgressStep>100)
          nProgressStep=0;
      }

      if((nStep==1) && (nProgressStep<30))
      {
        // very long query execution
        // display progress bar
        emit showDlg();
      }
      emit progressValue(nProgressStep);

      clLastTimeUpdate.start();
    }

    QCoreApplication::processEvents();

    this->msleep(200);

  }

  // End of the query
  // Return the result
  return m_bError;
}

///////////////////////////////////////////////////////////
// Run the thread
///////////////////////////////////////////////////////////
void QueryThread::run()
{
  // Execute the query in an other thread
  while(!m_pQuery)
  {
    this->msleep(1000);
  }

  m_bError=m_pQuery->exec();
  this->msleep(200);
}



bool GexDbPlugin_Galaxy::UpdateDb_To_InnoDB(bool bToBarracudaCompressed)
{
  int			nTime=0;
  bool		bStatus = false;
  QTime		clStartTime;
  QString		strQuery;
  QString		strLogMessage, strErrorMessage;
  if (!m_pclDatabaseConnector)
    return false;
  QSqlQuery	clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

  // There is 2 possibilities
  // Database is MyISAM
  //			-> to InnoDB Barracuda Compressed
  // Database is amready in InnoDB
  //			-> to Barracuda Compressed

  bool		bFromMyIsam = true;

  QString strEngine,strFormat;
  GetStorageEngineName(strEngine,strFormat);
  if(strEngine.toUpper() != "MYISAM")
    bFromMyIsam = false;

  strLogMessage = "Update DB to InnoDB";
  if(bToBarracudaCompressed)
    strLogMessage+=" Barracuda Compressed";
  strLogMessage+="...";

  GSLOG(SYSLOG_SEV_NOTICE, strLogMessage.toLatin1().constData());

  clStartTime.start();

  // Init progress bar
  int progress_value = 0;
  ResetProgress(false);

  if(!m_pclDatabaseConnector->IsMySqlDB())
    return false;

  SetMaxProgress(100);

  /////////////////////////////////////////
  //
  // RESULT PARTITIONING
  //
  // MYISAM => INNODB
  //
  /////////////////////////////////////////
  // * Check the MySql Server configuration
  //		- GLOBAL_VARIABLES[VERSION] > 5.1
  //		- GLOBAL_VARIABLES[HAVE_PARTITIONING] = YES
  //		- GLOBAL_VARIABLES[HAVE_INNODB] = YES
  //		- GLOBAL_VARIABLES[INNODB_FILE_PER_TABLE] = ON
  //
  // * Partitioning
  //		- Only run/results tables
  //		- Check the size:
  //		  for very big table, create a clone table
  //		  with all need partitions
  //		  and a script for data transfert
  //
  // * Convert MyIsam tables to InnoDB
  //		- Disk space > (MyIsam Table)*10
  //
  /////////////////////////////////////////

  double		fDiskSpace	=0;
  double		fDataFilesSize=0;

  QDir		clDir;
  QFileInfo	clFile;
  QString		strValue;
  QString		strTable;
  QString		strBaseDir;
  QString		strDataDir;
  QStringList	lstDataFiles;
  QStringList lstTablesPartitioned;
  QStringList lstTablesForPartition;
  QStringList lstTablesForConvertion;
  QStringList lstTablesForScript;

  QString		strMySqlVersion;

  bool		bHaveTablesForScript;

  QStringList::Iterator	it;

  ///////////////////////////////////////////
  // * Check the MySql Server configuration
  ///////////////////////////////////////////
  InsertIntoUpdateLog(" ");
  strLogMessage =  "o Check MySql server configuration: InnoDB/Partitioning";
  if(bToBarracudaCompressed)
    strLogMessage+="/Compressed";

  InsertIntoUpdateLog(strLogMessage);

  // Check MySql version >= 5.1
  strQuery = "SELECT @@version";
  if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;

  clQuery.first();
  // 5.1.40-community
  strMySqlVersion = clQuery.value(0).toString();
  strLogMessage = " - MYSQL VERSION = "+strMySqlVersion;
  InsertIntoUpdateLog(strLogMessage);
  strValue = strMySqlVersion.section(".",0,1).remove('.')+"."+strMySqlVersion.section(".",2).section("-",0,0);
  if(strValue.toFloat() < 51.4)
  {
    strErrorMessage = "UNSUPPORTED MYSQL VERSION ";
    goto updatedb_innodb_error_noset;
  }

  // Check if have INNODB engine
  //strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
  //strQuery+= "   WHERE VARIABLE_NAME = 'HAVE_INNODB'";
  // MariaDb 10 - HAVE_INNODB doesn't exist
  // Check instead on engines
  strQuery = "SELECT support FROM information_schema.engines";
  strQuery+= "   WHERE UPPER(engine) = 'INNODB'";
  if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;

  clQuery.first();
  // YES
  strValue = clQuery.value(0).toString();
  strLogMessage = " - HAVE_INNODB = "+strValue;
  InsertIntoUpdateLog(strLogMessage);
  if(!strValue.startsWith("YES",Qt::CaseInsensitive)
          && !strValue.startsWith("DEFAULT",Qt::CaseInsensitive))
  {
    strErrorMessage = "'HAVE_INNODB' disabled. This option must be enabled in order to update the DB.";
    goto updatedb_innodb_error_noset;
  }

  // Check if have INNODB_FILE_PER_TABLE
  strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
  strQuery+= "   WHERE VARIABLE_NAME = 'INNODB_FILE_PER_TABLE'";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;

  clQuery.first();
  // ON
  strValue = clQuery.value(0).toString();
  strLogMessage = " - INNODB_FILE_PER_TABLE = "+strValue;
  InsertIntoUpdateLog(strLogMessage);
  if(!strValue.startsWith("ON",Qt::CaseInsensitive))
  {
    strErrorMessage = "'INNODB_FILE_PER_TABLE' disabled. This option must be enabled in order to update the DB.";
    goto updatedb_innodb_error_noset;
  }

  // Check if have partitioning
  //strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
  //strQuery+= "   WHERE VARIABLE_NAME = 'HAVE_PARTITIONING'";
  // MySql 5.6, MariaDb 10 HAVE_PARTITIONING doesn't exist
  strQuery = "SELECT plugin_status FROM information_schema.plugins";
  strQuery+= "   WHERE UPPER(plugin_name) = 'PARTITION'";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;

  clQuery.first();
  // YES
  strValue = clQuery.value(0).toString();
  strLogMessage = " - HAVE_PARTITIONING = "+strValue;
  InsertIntoUpdateLog(strLogMessage);
  if(!strValue.startsWith("YES",Qt::CaseInsensitive)
          && !strValue.startsWith("ACTIVE",Qt::CaseInsensitive))
  {
    strErrorMessage = "'HAVE_PARTITIONING' disabled. This option must be enabled in order to update the DB.";
    goto updatedb_innodb_error_noset;
  }

  if(bToBarracudaCompressed)
  {
    // Check if have INNODB engine
    strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
    strQuery+= "   WHERE VARIABLE_NAME = 'INNODB_FILE_FORMAT'";
    if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;

    clQuery.first();
    // YES
    strValue = clQuery.value(0).toString();
    strLogMessage = " - INNODB_FILE_FORMAT = "+strValue;
    InsertIntoUpdateLog(strLogMessage);
    if(!strValue.startsWith("Barracuda",Qt::CaseInsensitive))
    {
      strErrorMessage = "'INNODB_FILE_FORMAT' not appropriate. This option must be set to Barracuda in order to update the DB.";
      goto updatedb_innodb_error_noset;
    }
  }

  ///////////////////////////////////////////
  // START PARTITIONING/INNODB
  ///////////////////////////////////////////

  // Select the good DataDir
  strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
  strQuery+= "   WHERE VARIABLE_NAME = 'DATADIR'";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;

  clQuery.first();
  strDataDir = clQuery.value(0).toString() + m_pclDatabaseConnector->m_strSchemaName + "\\";

  clDir.setPath(strDataDir);
  clDir.setFilter(QDir::Files);

  InsertIntoUpdateLog(" ");
  strLogMessage = "o Check GexDb tables:";
  InsertIntoUpdateLog(strLogMessage);


  if(bFromMyIsam)
  {
    // Special case
    // If GexDb12 crash between the rename to _myisam and the innodb creation
    // no test_results but test_results_myisam
    strQuery = "SELECT DISTINCT lower(TABLE_NAME) FROM information_schema.TABLES ";
    strQuery+= "   WHERE (TABLE_SCHEMA) = '" + m_pclDatabaseConnector->m_strSchemaName + "' ";
    strQuery+= "   AND (TABLE_NAME) LIKE '%test_results%'";
    strQuery+= "   ORDER BY TABLE_NAME DESC";
    if(!clQuery.exec(strQuery))
      goto updatedb_innodb_error;
    lstDataFiles.clear();
    // Get tables list
    while(clQuery.next())
      lstDataFiles.append(clQuery.value(0).toString());

    while(!lstDataFiles.isEmpty())
    {
      strTable = lstDataFiles.takeFirst();

      if(!strTable.endsWith("_myisam"))
        continue;

      // Check if InnoDb exists
      strQuery = "SELECT count(*) FROM " + strTable.section("_myisam",0,0);
      if(!clQuery.exec(strQuery))
      {
        // MyIsam exists
        // InnoDb not exists

        // Rename MyIsam
        strQuery = "ALTER TABLE "+strTable+" RENAME TO "+strTable.section("_myisam",0,0);
        if(!clQuery.exec(strQuery))
          goto updatedb_innodb_error;

        continue;
      }

      // InnoDb exists
      // Check if it is empty
      clQuery.first();
      if(clQuery.value(0).toInt() == 0)
      {
        // MyIsam exists
        // InnoDb is empty

        // Drop InnoDb
        strQuery = "DROP TABLE "+strTable.section("_myisam",0,0);
        clQuery.exec(strQuery);

        // Rename MyIsam
        strQuery = "ALTER TABLE "+strTable+" RENAME TO "+strTable.section("_myisam",0,0);
        if(!clQuery.exec(strQuery))
          goto updatedb_innodb_error;

        continue;
      }

      // InnoDb exists
      // It is not empty
      // Check if MyIsam is empty
      strQuery = "SELECT count(*) FROM " + strTable;
      if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
      clQuery.first();
      if(clQuery.value(0).toInt() == 0)
      {
        // MyIsam is empty
        // Transfert is completed
        // Drop MyIsam
        strQuery = "DROP TABLE "+strTable;
        clQuery.exec(strQuery);

        continue;
      }

      // InnoDb is not empty
      // MyIsam is not empty
      // Transfert was incompleted

      // Add this table for Script
      lstTablesForScript.append(strTable.section("_myisam",0,0));
    }

    // Special case from WES
    // InnoDB conversion is not complete
    // test_results is already partionned
    // test_results was renamed to test_results_myisam
    // new insertion was processed
    // test_results not empty AND test_results_myisam not empty

    // Check if have partitions to do
    // only for tables results (Xt_Xtest_results)
    strQuery = "SELECT lower(TABLE_NAME), MIN(PARTITION_NAME) FROM information_schema.PARTITIONS ";
    strQuery+= "   WHERE (TABLE_SCHEMA) = '" + m_pclDatabaseConnector->m_strSchemaName + "' ";
    strQuery+= "   AND ((TABLE_NAME) LIKE '%test_results' OR (TABLE_NAME) LIKE '%_run')";
    strQuery+= "   GROUP BY TABLE_NAME";
    strQuery+= "   ORDER BY TABLE_NAME";
    if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
    while(clQuery.next())
    {
      strTable = clQuery.value(0).toString();
      strValue = clQuery.value(1).toString();

      if(strValue.isEmpty())
        lstTablesForPartition.append(strTable);
      else
        lstTablesPartitioned.append(strTable);
    }
  }

  // Check if have InnoDb convertion to do
  // for all tables
  // Order by table size
  strQuery = "SELECT lower(TABLE_NAME), SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024) AS TABLE_SIZE ";
  strQuery+= "   FROM information_schema.TABLES ";
  strQuery+= "   WHERE (TABLE_SCHEMA) = '" + m_pclDatabaseConnector->m_strSchemaName + "' ";
  strQuery+= "   AND (";
  strQuery+= " (lower(ENGINE) != 'innodb') ";
  if(bToBarracudaCompressed)
    strQuery+= "   OR (lower(ENGINE) != 'compressed')";
  strQuery+= "   )";
  strQuery+= "   GROUP BY TABLE_NAME";
  strQuery+= "   ORDER BY TABLE_SIZE DESC";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;
  while(clQuery.next())
  {
    strTable = clQuery.value(0).toString();

    // If it is a _myisam table for transfer, ignore it
    if(strTable.endsWith("_myisam",Qt::CaseInsensitive))
      continue;

    lstTablesForConvertion.append(strTable);

    // Check the size of the test_results tables
    if(strTable.endsWith("test_results",Qt::CaseInsensitive))
    {

      // Special case from WES
      // test_results is in MyIsam
      // test_results is already partitioned
      // test_results is very big and must be use the script delay
      // test_results_myisam already exists
      // add test_results for script => rename to test_results_myisam (if not already exists)
      // add test_results for partitioning => create good partitions

      // MyIsam files size (data/index)
      fDataFilesSize = clQuery.value(1).toFloat();
      // Script for tables when size > 1024Mo
      // Or if test_results_myisam already exists
      if((fDataFilesSize > LIMIT_FILE_SIZE)
      || (lstTablesForScript.contains(strTable.toLower())))
      {
        if(!lstTablesForScript.contains(strTable.toLower()))
          lstTablesForScript.append(strTable.toLower());

        // If script delay
        // Add table for partitioning
        if(!lstTablesForPartition.contains(strTable.toLower()))
          lstTablesForPartition.append(strTable.toLower());
      }
    }
  }

  strLogMessage = " - Tables for partitioning = "+QString::number(lstTablesForPartition.count());
  InsertIntoUpdateLog(strLogMessage);

  strLogMessage = " - Tables for InnoDb engine = "+QString::number(lstTablesForConvertion.count());
  InsertIntoUpdateLog(strLogMessage);

  strLogMessage = " - Tables for script delay = "+QString::number(lstTablesForScript.count());
  InsertIntoUpdateLog(strLogMessage);

  bHaveTablesForScript = !lstTablesForScript.isEmpty();


  if(!lstTablesForPartition.isEmpty()
  || !lstTablesForConvertion.isEmpty()
  || !lstTablesForScript.isEmpty())
  {
    InsertIntoUpdateLog(" ");
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "* UPGRADE QUANTIX GEXDB TO QUANTIX GEXDB INNODB/PARTITIONING";
    if(bToBarracudaCompressed)
      strLogMessage+="/COMPRESSED";
    InsertIntoUpdateLog(strLogMessage);
    strLogMessage = "************************************************************************* ";
    InsertIntoUpdateLog(strLogMessage);
    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("o Starting upgrade at "+QDateTime::currentDateTime().toString());


    int			nNbSplitlots;
    int			nLastSplitlot=0;
    int			nLastPartSplitlot;
    int			nYearMonth;
    int			nLastYearMonth=0;
    QString		strPartitions;
    QString		strTestingStage;

    SetMaxProgress(lstTablesForPartition.count() + lstTablesForConvertion.count() + 30);

    ///////////////////////////////////////////
    // * Partitioning
    ///////////////////////////////////////////
    // Special case
    // InnoDb already exist
    // MyIsam alway exist
    // Add queries in Script

    if(bHaveTablesForScript)
    {
      // Create if not exist table contains all info for transfert
      strQuery = "SELECT * FROM innodb_transfer_option_from_myisam";
      if(!clQuery.exec(strQuery))
      {
        // Create this table innodb_table, myisam_table, current_index, max_index, index_increment
        strQuery = "CREATE TABLE innodb_transfer_option_from_myisam (";
        strQuery+= " innodb_table varchar(255) NOT NULL,";
        strQuery+= " myisam_table varchar(255) NOT NULL,";
        strQuery+= " current_index int(10) unsigned NOT NULL,";
        strQuery+= " max_index int(10) unsigned NOT NULL,";
        strQuery+= " index_increment int(9) NOT NULL";
        strQuery+= " ) ENGINE=InnoDB";
        if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
      }
      // Create if not exist procedure for the data transfert
      strQuery = "SHOW CREATE PROCEDURE innodb_transfert_data";
      if(!clQuery.exec(strQuery))
      {
        strQuery =  "CREATE PROCEDURE innodb_transfert_data(\n";
        strQuery+= "	OUT Message				VARCHAR(1024),\n";
        strQuery+= "	OUT Status				INT\n";
        strQuery+= ")\n";
        strQuery+= "BEGIN\n";
        strQuery+= "	DECLARE InnoDbTable			VARCHAR(1024);\n";
        strQuery+= "	DECLARE MyIsamTable			VARCHAR(1024);\n";
        strQuery+= "	DECLARE CurrentIndex		INT;\n";
        strQuery+= "	DECLARE MaxIndex			INT;\n";
        strQuery+= "	DECLARE IndexIncrement		INT;\n";
        strQuery+= "\n";
        strQuery+= "	DECLARE EXIT HANDLER FOR 1109\n";
        strQuery+= "	BEGIN\n";
        strQuery+= "		SELECT 'TABLE innodb_transfer_option_from_myisam NOT EXISTS' INTO Message FROM dual;\n";
        strQuery+= "		SELECT -1 INTO Status FROM dual;\n";
        strQuery+= "	END;\n";
        strQuery+= "\n";
        strQuery+= "	SELECT -1 INTO Status FROM dual;\n";
        strQuery+= "	SELECT 'UNKNOWN ERROR' INTO Message FROM dual;\n";
        strQuery+= "\n";
        strQuery+= "	SELECT count(*) FROM innodb_transfer_option_from_myisam WHERE current_index>0 INTO IndexIncrement;\n";
        strQuery+= "	IF (IndexIncrement=0) THEN\n";
        strQuery+= "		SELECT 1 INTO Status FROM dual;\n";
        strQuery+= "		SELECT 'TRANSFERT DATA COMPLETED' INTO Message FROM dual;\n";
        strQuery+= "	ELSE\n";
        strQuery+= "\n";
        strQuery+= "		SELECT innodb_table FROM innodb_transfer_option_from_myisam WHERE current_index>0 LIMIT 1 INTO InnoDbTable;\n";
        strQuery+= "		SELECT myisam_table FROM innodb_transfer_option_from_myisam WHERE innodb_table=InnoDbTable INTO MyIsamTable;\n";
        strQuery+= "		SELECT current_index FROM innodb_transfer_option_from_myisam WHERE innodb_table=InnoDbTable INTO CurrentIndex;\n";
        strQuery+= "		SELECT max_index FROM innodb_transfer_option_from_myisam WHERE innodb_table=InnoDbTable INTO MaxIndex;\n";
        strQuery+= "		SELECT index_increment FROM innodb_transfer_option_from_myisam WHERE innodb_table=InnoDbTable INTO IndexIncrement;\n";
        strQuery+= "\n";
        strQuery+= "		SET @SqlQuery = CONCAT('DELETE FROM ',InnoDbTable,' WHERE splitlot_id>',(CurrentIndex-IndexIncrement),' AND splitlot_id<=',CurrentIndex);\n";
        strQuery+= "		PREPARE Stmt FROM @SqlQuery;\n";
        strQuery+= "		EXECUTE Stmt;\n";
        strQuery+= "\n";
        strQuery+= "		SET @SqlQuery = CONCAT('INSERT INTO ',InnoDbTable,' SELECT * FROM ',MyIsamTable,' WHERE splitlot_id>',(CurrentIndex-IndexIncrement),' AND splitlot_id<=',CurrentIndex);\n";
        strQuery+= "		PREPARE Stmt FROM @SqlQuery;\n";
        strQuery+= "		EXECUTE Stmt;\n";
        strQuery+= "\n";
        strQuery+= "		IF (CurrentIndex < IndexIncrement) THEN\n";
        strQuery+= "			SELECT 0 INTO Status FROM dual;\n";
        strQuery+= "			SELECT CONCAT(CONCAT('TRANSFERT DATA FROM ',MyIsamTable),' COMPLETED') INTO Message FROM dual;\n";
        strQuery+= "			DELETE FROM innodb_transfer_option_from_myisam WHERE innodb_table=InnoDbTable;\n";
        strQuery+= "		ELSE\n";
        strQuery+= "			SELECT 0 INTO Status FROM dual;\n";
        strQuery+= "			SELECT CONCAT(CONCAT(CONCAT(CONCAT(CONCAT('TRANSFERT DATA FROM ',MyIsamTable),' WHERE splitlot_id>'),(CurrentIndex-IndexIncrement)),' AND splitlot_id<='),CurrentIndex) INTO Message FROM dual;\n";
        strQuery+= "			UPDATE innodb_transfer_option_from_myisam SET current_index=(CurrentIndex-IndexIncrement) WHERE innodb_table=InnoDbTable;\n";
        strQuery+= "		END IF;\n";
        strQuery+= "	END IF;\n";
        strQuery+= "END";
        if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
      }
    }

    if(!lstTablesForPartition.isEmpty())
    {

      InsertIntoUpdateLog(" ");
      strLogMessage =  "o Tables partitioning:";
      InsertIntoUpdateLog(strLogMessage);

      while(!lstTablesForPartition.isEmpty())
      {
        nTime = -clStartTime.elapsed();
        strTable = lstTablesForPartition.takeFirst();

        // Check if this table must be cloned for delayed transfert
        if(lstTablesForScript.contains(strTable.toLower()))
        {
          // Check if not exist
          QStringList strlTables;
          m_pclDatabaseConnector->EnumTables(strlTables);
          if(!strlTables.contains(strTable+"_myisam", Qt::CaseInsensitive))
          {
            // Rename the current table
            strQuery = "ALTER TABLE "+strTable+" RENAME TO "+strTable+"_myisam";
            if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
            // Clone the table
            strQuery = "CREATE TABLE "+strTable+" LIKE "+strTable+"_myisam";
            if(!clQuery.exec(strQuery))
            {
              strErrorMessage = clQuery.lastError().text();
              // Undo
              strQuery = "ALTER TABLE "+strTable+"_myisam RENAME TO "+strTable;
              clQuery.exec(strQuery);

              goto updatedb_innodb_error_noset;
            }

          }
          // Force RE-partitioning even if already partitioned
          // REMOVE PARTITIONING was introduced in MySQL 5.1.8
          // removing a table's partitioning without otherwise affecting the table or its data.
          strQuery = "ALTER TABLE "+strTable+" REMOVE PARTITIONING";
          if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
        }

        strLogMessage = " - Partitioning table " + strTable + " ... ";
        InsertIntoUpdateLog(strLogMessage);

        // Determine the list of partitions to create
        // The same for all tables from one testingstage
        if(strTestingStage != strTable.left(2))
        {
          strTestingStage = strTable.left(2);

          // Have to check OLD convention and NEW convention from splitlot_id and insertion_time
          // Convention is SPLIT BY MONTH
          // Old convention is PYYMM
          // New convention is MYYddd00000
          // During the InnoDb update, keep the old convention for partition reinsertion
          // PYYMM
          strQuery = "SELECT splitlot_id, date_format(from_unixtime(insertion_time),'%Y%m') AS insertion_year_month";
          strQuery+= " FROM " + strTestingStage + "_splitlot";
          strQuery+= " WHERE valid_splitlot='Y'"; // ignore invalid line that can have an invalid spltilot
          strQuery+= " ORDER BY splitlot_id";

          if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;

          nNbSplitlots = nLastSplitlot = nLastPartSplitlot = 0;
          nYearMonth = nLastYearMonth = 0;
          strPartitions = "";
          lstDataFiles.clear();

          while(clQuery.next())
          {
            nNbSplitlots++;

            // For the first partition
            if(nYearMonth == 0)
              nYearMonth = clQuery.value(1).toInt();

            nLastYearMonth = clQuery.value(1).toInt();
            nLastSplitlot = clQuery.value(0).toInt();

            // Stop at the current date
            // This partition will be create with limit = MAXVALUE
            // With the new partition convention, create the last partition for this month
            //if(nYearMonth == QDate::currentDate().toString("yyMM").toInt())
            //  break;

            if(nYearMonth != clQuery.value(1).toInt())
            {
              // 32, 201005
              // 33, 201005
              // 34, 201006
              // => #P1005 lessthan 34

              nLastPartSplitlot = clQuery.value(0).toInt();
              if(!strPartitions.isEmpty())
                strPartitions+= ",";
              strValue = "P" + QString::number(nYearMonth).right(4);
              // Check if this partition already exist (Vishay MERGE)
              if(strPartitions.count(strValue) > 0)
                strValue+= "_" + QString::number(strPartitions.count(strValue));
              strPartitions+= " PARTITION " + strValue;
              strPartitions+= " VALUES LESS THAN (" + clQuery.value(0).toString() + ")";
              nYearMonth = clQuery.value(1).toInt();
              // Save the list of splitlot limit
              lstDataFiles.prepend(clQuery.value(0).toString());
              nNbSplitlots = 0;
            }
          }

          // If only one splitlot for the last month
          // 51355, 201004
          // 51385, 201005
          // #P1004 LESS THAN 51385
          // #P1005 LESS THAN 51386

          // 51355, 201004
          // 51385, 201005
          // 51386, 201005
          // #P1004 LESS THAN 51385
          // #P1005 LESS THAN 51387
          nLastSplitlot++;

          // Add the last partition
          if(nLastYearMonth > 0)
          {
            if(nLastYearMonth != QDate::currentDate().toString("yyyyMM").toInt())
            {
              // No data for the current date
              // Add partition for this month
              if(!strPartitions.isEmpty())
                strPartitions+= ",";

              nLastPartSplitlot = nLastSplitlot;

              strValue = "P" + QString::number(nLastYearMonth).right(4);

              // Check if this partition already exist (Vishay MERGE)
              if(strPartitions.count(strValue) > 0)
                strValue+= "_" + QString::number(strPartitions.count(strValue));

              strPartitions+= " PARTITION " + strValue;
              strPartitions+= " VALUES LESS THAN (" + QString::number(nLastSplitlot) + ")";

              // Save the list of splitlot limit
              lstDataFiles.prepend(QString::number(nLastSplitlot));

            }
          }
        }

        //
        strQuery = "ALTER TABLE " + strTable ;
        strQuery+= " PARTITION BY RANGE (splitlot_id)";
        strQuery+= "(" + strPartitions;
        // For script
        // With the new partition convention, create the last partition for this month
        /*
        if((nLastYearMonth == QDate::currentDate().toString("yyyyMM").toInt())
        && (lstTablesForScript.contains(strTable.toLower())))
        {
          // Data exist for the current month
          // If table to Script
          // then have to create a partition only for this data
          if(!strPartitions.isEmpty())
            strQuery+= ",";

          // Check if nLastSplitlot not already used for preview partitions
          if(lstDataFiles.count(QString::number(nLastSplitlot)) == 0)
          {
            strValue = "P" + QString::number(nLastYearMonth).right(4)+"_1";

            // Check if this partition already exist (Vishay MERGE)
            if(strPartitions.count(strValue) > 0)
              strValue+= "_" + QString::number(strPartitions.count(strValue));

            // One for actual data use for script partition
            strQuery+= "PARTITION " + strValue;
            strQuery+= " VALUES LESS THAN (" + QString::number(nLastSplitlot) + ")";

            // Save the list of splitlot limit
            lstDataFiles.prepend(QString::number(nLastSplitlot));

            // One for new data
            strQuery+= ",";

          }
          strQuery+= "PARTITION P" + QDate::currentDate().toString("yyMM") + " VALUES LESS THAN (" + QDate::currentDate().addMonths(1).toString("yyMM000000") + "),";
          strQuery+= "PARTITION LASTPART VALUES LESS THAN MAXVALUE";
        }
        else*/
        {
          // One for new data
          if(!strPartitions.isEmpty())
            strQuery+= ",";
          // New convention
          // Sequence start at the first day of the month
          QDate Date = QDate::currentDate();
          Date = QDate(Date.year(),Date.month(),1);
          UINT lMinSequence = QString(Date.toString("yy")
                                 + QString::number(Date.dayOfYear()).rightJustified(3,'0'))
                  .leftJustified(10,'0').toUInt();
          // Sequence end at the last day of the month
          Date = Date.addMonths(1);
          UINT lMaxSequence = QString(Date.toString("yy")
                                 + QString::number(Date.dayOfYear()).rightJustified(3,'0'))
                  .leftJustified(10,'0').toUInt() -1;

          strQuery+= "PARTITION M" + QString::number(lMinSequence) + " VALUES LESS THAN ("
                  + QString::number(lMaxSequence) + "),";
          strQuery+= "PARTITION LASTPART VALUES LESS THAN MAXVALUE";

        }

        strQuery+= ")";

        // Apply partitioning
        // Thread for ProgressBar
        QueryThread clQueryThread;

        QProgressDialog dlg;
        dlg.setWindowTitle("PARTITIONING");
        dlg.setCancelButton(NULL);

        QObject::connect(&clQueryThread, SIGNAL(progressValue(int)), &dlg, SLOT(setValue(int)));
        QObject::connect(&clQueryThread, SIGNAL(newText(QString)), &dlg, SLOT(setLabelText(QString)));
        QObject::connect(&clQueryThread, SIGNAL(showDlg(QString)), &dlg, SLOT(show()));

        clQuery.prepare(strQuery);

        if(!clQueryThread.exec(&clQuery,QUERYTHREAD_TYPE_PARTITIONING,strDataDir,strTable))
          goto updatedb_innodb_error;
        dlg.cancel();
        // Thread for ProgressBar

        nTime += clStartTime.elapsed();
        InsertIntoUpdateLog("DONE in "+ QString::number(nTime/1000.0) + "s ", false);
        SetProgress(++progress_value);
      }

      InsertIntoUpdateLog(" ");
    }

    if(bHaveTablesForScript)
    {

      // Check if this table must be cloned for delayed transfert
      while(!lstTablesForScript.isEmpty())
      {
        strTable = lstTablesForScript.takeFirst();

        strQuery = "DELETE FROM innodb_transfer_option_from_myisam WHERE innodb_table='"+strTable+"'";
        if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
        // Get the max splitlot_id
        strQuery = "SELECT MAX(splitlot_id) FROM "+strTable+"_myisam";
        if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
        clQuery.first();
        // Insert new ligne in innodb_transfer_option_from_myisam
        strQuery = "INSERT INTO innodb_transfer_option_from_myisam VALUES(";
        strQuery+= "'"+strTable+"','"+strTable+"_myisam',"+clQuery.value(0).toString()+","+clQuery.value(0).toString()+","+QString::number(LIMIT_NB_SPLITLOTS)+")";
        if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;
      }
    }

    SetMaxProgress(70);

    ///////////////////////////////////////////
    // * Convert MyIsam tables to InnoDB
    ///////////////////////////////////////////
    if(!lstTablesForConvertion.isEmpty())
    {

      InsertIntoUpdateLog(" ");
      strLogMessage =  "o Tables convertion to InnoDB";
      if(bToBarracudaCompressed)
        strLogMessage+= " Barracuda Compressed: ";
      else
        strLogMessage+= ": ";
      InsertIntoUpdateLog(strLogMessage);

      strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
      strQuery+= "   WHERE VARIABLE_NAME = 'DATADIR'";
      if(!clQuery.exec(strQuery)) goto updatedb_innodb_error;

      clQuery.first();
      strDataDir = clQuery.value(0).toString() + m_pclDatabaseConnector->m_strSchemaName + "\\";

      clDir.setPath(strDataDir);
      clDir.setFilter(QDir::Files);

      while(!lstTablesForConvertion.isEmpty())
      {
        nTime = -clStartTime.elapsed();
        strTable = lstTablesForConvertion.takeFirst();

        // InnoDB
        strLogMessage = " - Converting table " + strTable + " to InnoDB";
        if(bToBarracudaCompressed)
          strLogMessage+= " Barracuda Compressed ... ";
        else
          strLogMessage+= "engine ... ";
        InsertIntoUpdateLog(strLogMessage);

        // Free disk space
        fDiskSpace = 0;
        getFreeTotalSpace(strDataDir,fDataFilesSize,fDiskSpace);

        // MyIsam files size (data/index)
        fDataFilesSize = 0;
        lstDataFiles = clDir.entryList(QStringList(strTable+".*"));
        for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
        {
          if((*it).contains("myisam"))
            continue;
          clFile.setFile(strDataDir,*it);
          fDataFilesSize+= clFile.size();
        }
        lstDataFiles = clDir.entryList(QStringList(strTable+"#*.*"));
        for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
        {
          if((*it).contains("myisam"))
            continue;
          clFile.setFile(strDataDir,*it);
          fDataFilesSize+= clFile.size();
        }

        // InnoDB need 5*MyIsam space
        // Check if have 6*MyIsam
        // InnoDB Barracuda need 3*MyIsam space
        // Check if have 4*MyIsam

        if((fDiskSpace > 0) && (fDiskSpace < (fDataFilesSize*4)))
        {
          strLogMessage = "Not enough free disk space ("+QString::number(fDiskSpace/1024)+"Mo)";
          InsertIntoUpdateLog(strLogMessage);
          strLogMessage = "Table "+strTable+" ("+QString::number(fDataFilesSize/1024)+"Mo)";
          InsertIntoUpdateLog(strLogMessage);
          strLogMessage = "Estimation size ("+QString::number((fDataFilesSize*4)/1024)+"Mo)";
          InsertIntoUpdateLog(strLogMessage);

          strErrorMessage = "Not enough free disk space";
          goto updatedb_innodb_error_noset;
        }

        strQuery = "ALTER TABLE " + strTable + " ENGINE=INNODB";
        if(bToBarracudaCompressed)
            strQuery += " ROW_FORMAT=COMPRESSED";

        // Thread for ProgressBar
        QueryThread clQueryThread;

        QProgressDialog dlg;
        if(bToBarracudaCompressed)
          dlg.setWindowTitle("InnoDB Barracuda Compressed Convertion");
        else
          dlg.setWindowTitle("InnoDB Convertion");

        dlg.setCancelButton(NULL);

        QObject::connect(&clQueryThread, SIGNAL(progressValue(int)), &dlg, SLOT(setValue(int)));
        QObject::connect(&clQueryThread, SIGNAL(newText(QString)), &dlg, SLOT(setLabelText(QString)));
        QObject::connect(&clQueryThread, SIGNAL(showDlg(QString)), &dlg, SLOT(show()));

        clQuery.prepare(strQuery);
        if(bToBarracudaCompressed)
        {
          if(!clQueryThread.exec(&clQuery,QUERYTHREAD_TYPE_BARRACUDA,strDataDir,strTable))
            goto updatedb_innodb_error;
        }
        else
        {
          if(!clQueryThread.exec(&clQuery,QUERYTHREAD_TYPE_INNODB,strDataDir,strTable))
            goto updatedb_innodb_error;
        }
        dlg.cancel();
        // Thread for ProgressBar

        nTime += clStartTime.elapsed();
        InsertIntoUpdateLog("DONE in "+ QString::number(nTime/1000.0) + "s ", false);
        if(fDataFilesSize > 0)
        {
          InsertIntoUpdateLog(" ("+QString::number(fDataFilesSize/(1024*1024),'f',2), false);
          fDataFilesSize = 0;
          lstDataFiles = clDir.entryList(QStringList(strTable+".*"));
          for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
          {
            if((*it).contains("myisam"))
              continue;

            clFile.setFile(strDataDir,*it);
            fDataFilesSize+= clFile.size();
          }
          lstDataFiles = clDir.entryList(QStringList(strTable+"#*.*"));
          for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
          {
            if((*it).contains("myisam"))
              continue;

            clFile.setFile(strDataDir,*it);
            fDataFilesSize+= clFile.size();
          }
          InsertIntoUpdateLog("Mo -> "+QString::number(fDataFilesSize/(1024*1024),'f',2)+"Mo)", false);
        }

        SetProgress(++progress_value);
      }

      InsertIntoUpdateLog(" ");
    }

    ///////////////////////////////////////////
    // * Script for transfert tables data to InnoDB
    ///////////////////////////////////////////
    if(bHaveTablesForScript)
    {
      InsertIntoUpdateLog(" ");
      strLogMessage =  "o Batch script creation:";
      InsertIntoUpdateLog(strLogMessage);

      strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
      strQuery+= "   WHERE VARIABLE_NAME = 'BASEDIR'";
      if(!clQuery.exec(strQuery))
        goto updatedb_innodb_error;

      clQuery.first();
      strBaseDir = QDir::cleanPath(clQuery.value(0).toString() + "/bin");

      QString			strBatchFile;
      QString			strFullBatchFile;
      CGexSystemUtils	clSysUtils;
      QString strDatabaseFolder = m_strDBFolder;
      strBatchFile = "gexdb_update_"+m_pclDatabaseConnector->m_strSchemaName.toLower()+"_transfert_data_to_innodb";
      strFullBatchFile = strDatabaseFolder + "/" + strBatchFile + ".bat";
      clSysUtils.NormalizePath(strFullBatchFile);

      // Open log file
      FILE *pBatFile = fopen(strFullBatchFile.toLatin1().constData(), "w+");
      if(!pBatFile)
      {
        InsertIntoUpdateLog(" ");
        strLogMessage = "Error writing to bat file ";
        strLogMessage+= strFullBatchFile;
        strLogMessage+= ".";
        InsertIntoUpdateLog(strLogMessage);
        goto updatedb_innodb_error;
      }

      fprintf(pBatFile, "@ECHO off\n");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "TITLE Quantix InnoDb transfert");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "REM ============================================================\n");
      fprintf(pBatFile, "REM script to transfer a MyIsam gexdb tables to InnoDb tables. \n");
      fprintf(pBatFile, "REM ============================================================\n");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "ECHO Starting gexdb transfert \n");
                        fprintf(pBatFile, "ECHO Starting gexdb transfert >> \"%s.log\"\n", strBatchFile.toLatin1().constData());
                        fprintf(pBatFile, "ECHO %%date%% %%time%% >> \"%s.log\"\n", strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "REM Sql queries \n");
      fprintf(pBatFile, ":DataTransfertLoop\n");
      fprintf(pBatFile, "REM ADD MYSQL BIN PATH\n");
                        fprintf(pBatFile, "PATH %%PATH%%;%s\n", strBaseDir.toLatin1().constData());
      fprintf(pBatFile, "REM EXECUTE ONE DATA TRANSFERT\n");
      strValue = "mysql --user="+m_pclDatabaseConnector->m_strUserName_Admin+" --password="+m_pclDatabaseConnector->m_strPassword_Admin;
      if(m_pclDatabaseConnector->m_strHost_IP.isEmpty())
        strValue+= " --host="+m_pclDatabaseConnector->m_strHost_Name;
      else
        strValue+= " --host="+m_pclDatabaseConnector->m_strHost_IP;
      strValue+= " --port="+QString::number(m_pclDatabaseConnector->m_uiPort);
      strValue+= " --database="+m_pclDatabaseConnector->m_strSchemaName;
      strValue+= " --execute=\"CALL innodb_transfert_data(@Message,@Status);";
      strValue+= "SELECT CONCAT('Message=\"',@Message,'\"') AS Results FROM dual ";
      strValue+= "UNION ";
      strValue+= "SELECT CONCAT('Status=\"',@Status,'\"') FROM dual ;";
      strValue+= "\" 2> \""+strBatchFile+".err\" >\""+strBatchFile+".query_result\"\n";
                        fprintf(pBatFile, "%s", strValue.toLatin1().constData());
      fprintf(pBatFile, "REM CHECK IF HAVE SOME ERRORS\n");
                        fprintf(pBatFile, "IF EXIST \"%s.err\" SET /p error=<\"%s.err\"\n",
                                strBatchFile.toLatin1().constData(), strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "IF DEFINED error                   SET ErrorMessage=%%error%%         & goto Error\n");
      fprintf(pBatFile, "REM CHECK IF HAVE SOME RESULTS\n");
                        fprintf(pBatFile, "IF NOT EXIST \"%s.query_result\"                   SET ErrorMessage=Query return no result         & goto Error\n",
                                strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "REM CATCH RESULTS\n");
                        fprintf(pBatFile, "for /f \"delims=\" %%%%a in ('type \"%s.query_result\"^|find \"=\"^|find /v \"#\"') do SET %%%%a\n",
                                strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "IF NOT DEFINED Message		SET ErrorMessage=Query return no result message         & goto Error\n");
      fprintf(pBatFile, "IF NOT DEFINED Status		SET ErrorMessage=Query return no result status         & goto Error\n");
      fprintf(pBatFile, "IF \"%%Status%%\"==\"-1\"			SET ErrorMessage=\"%%Message%%\"         & goto Error\n");
                        fprintf(pBatFile, "IF \"%%Status%%\"== \"1\"			@echo \"%%Message%%\"	&@echo %%date%% %%time%% \"%%Message%%\" >> \"%s.log\"	& goto Exit\n",
                                strBatchFile.toLatin1().constData());
                        fprintf(pBatFile, "IF \"%%Status%%\"== \"0\"			@echo \"%%Message%%\"	&@echo %%date%% %%time%% \"%%Message%%\" >> \"%s.log\"	& goto DataTransfertLoop\n",
                                strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "goto :Exit\n");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "REM ============================================================\n");
      fprintf(pBatFile, "REM EXIT SECTION\n");
      fprintf(pBatFile, "REM ============================================================\n");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, ":Error %%\n");
      fprintf(pBatFile, "ECHO   ERROR:%%ErrorMessage%%\n");
                        fprintf(pBatFile, "ECHO   ERROR:%%ErrorMessage%% >> \"%s.log\"\n",
                                strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "ECHO .\n");
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, ":Exit\n");
      fprintf(pBatFile, "ECHO gexdb transfert finished\n");
                        fprintf(pBatFile, "ECHO gexdb transfert finished >> \"%s.log\"\n",
                                strBatchFile.toLatin1().constData());
                        fprintf(pBatFile, "ECHO %%date%% %%time%% >> \"%s.log\"\n",
                                strBatchFile.toLatin1().constData());
      fprintf(pBatFile, "\n");
      fprintf(pBatFile, "pause\n");

      fclose(pBatFile);
      strLogMessage = "- Sql script to transfert data saved into BATCH file ";
      InsertIntoUpdateLog(strLogMessage);
      strLogMessage = " "+strFullBatchFile;
      strLogMessage+= ".";
      InsertIntoUpdateLog(strLogMessage);
      strLogMessage = "<b><u>===> EXECUTE THIS BATCH TO FINALIZE DATA TRANSFERT</u></b>";
      InsertIntoUpdateLog(strLogMessage);
      InsertIntoUpdateLog(" ");
    }
  }

  InsertIntoUpdateLog("");
  InsertIntoUpdateLog("o Upgraded at "+QDateTime::currentDateTime().toString());
  InsertIntoUpdateLog(" ");
  strLogMessage = "************************************************************************* ";
  InsertIntoUpdateLog(strLogMessage);
  strLogMessage = "* QUANTIX GEXDB UPGRADED TO INNODB/PARTITIONS";
  if(bToBarracudaCompressed)
    strLogMessage+="/COMPRESSED";

  InsertIntoUpdateLog(strLogMessage);
  strLogMessage = "************************************************************************* ";
  InsertIntoUpdateLog(strLogMessage);

  ResetProgress(true);

  // Update global info
  // Update progress
  InsertIntoUpdateLog(" ");
  strLogMessage = "o Updating GLOBAL_INFO table.";
  InsertIntoUpdateLog(strLogMessage);


  // update GEXDB_MYSQL_ENGINE option in global_options table

  strQuery = "DELETE FROM "+NormalizeTableName("global_options",false)+" WHERE option_name='GEXDB_MYSQL_ENGINE'";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;

  strQuery = "INSERT INTO "+NormalizeTableName("global_options",false)+" VALUES('GEXDB_MYSQL_ENGINE','InnoDB')";
  if(!clQuery.exec(strQuery))
    goto updatedb_innodb_error;

  if(bToBarracudaCompressed)
  {

    strQuery = "DELETE FROM "+NormalizeTableName("global_options",false)+" WHERE option_name='GEXDB_MYSQL_ROWDATA'";
    if(!clQuery.exec(strQuery))
      goto updatedb_innodb_error;

    strQuery = "INSERT INTO "+NormalizeTableName("global_options",false)+" VALUES('GEXDB_MYSQL_ROWDATA','Compressed')";
    if(!clQuery.exec(strQuery))
      goto updatedb_innodb_error;

  }

  clQuery.exec("UNLOCK TABLES"); // Do we have to ?

  // Success
  bStatus = true;
  InsertIntoUpdateLog(" ");
  strLogMessage = "Status = SUCCESS.";
  InsertIntoUpdateLog(strLogMessage);
  goto updatedb_innodb_writelog;

updatedb_innodb_error:
  // Write error message
  GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
updatedb_innodb_error_noset:
  if(strErrorMessage.isEmpty())
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
  InsertIntoUpdateLog(" ");
  strLogMessage = "Status = ERROR (";
  strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
  strLogMessage+= ").";
  InsertIntoUpdateLog(strLogMessage);

updatedb_innodb_writelog:
  InsertIntoUpdateLog(" ");
  strLogMessage = "Update history saved to log file ";
  strLogMessage+= m_strUpdateDbLogFile;
  strLogMessage+= ".";
  InsertIntoUpdateLog(strLogMessage);

  return bStatus;
}
