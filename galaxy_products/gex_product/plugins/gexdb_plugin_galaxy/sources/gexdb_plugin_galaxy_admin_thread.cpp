// gexdb_plugin_galaxy_admin_b13.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// B12->B13 upgrade
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
#include <math.h>

// Qt includes
#include <QProgressDialog>
#include <QThread>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QDir>
#include <QApplication>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>


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
#else // linux stuff
#include <sys/vfs.h>
#include <sys/stat.h>
#endif // _WIN32


///////////////////////////////////////////////////////////
// QueryThread
// Create a thread to execute QSqlQuery
// Emit processEvents and progressBar control events
// Display progressDialog after 2 secondes and progressBar < 30%
///////////////////////////////////////////////////////////
// progressBar control
// nQueryType = QUERYTHREAD_TYPE_DEFAULT
//		no progressBar controm
//		progressBar incremented each 2 secondes
// nQueryType = QUERYTHREAD_TYPE_PARTITIONING OR QUERYTHREAD_TYPE_INNODB
//		progressBar control
//		check the evolution of MySql data file size
///////////////////////////////////////////////////////////
bool QueryThread::exec(QSqlQuery *pQuery, QApplication *pApplication, int nType, QString strDir, QString strFile)
{
    QTime	clLastTimeUpdate;
    float	fDataFilesSize;
    float	fMyIsamSize;
    int		nStep;
    int		nQueryType;
    int		nProgressStep;

    QString strMsg;

    m_pQuery=pQuery;
    nQueryType = nType;
    nStep = 0;
    nProgressStep = 0;
    fMyIsamSize = 1;
    fDataFilesSize = 1;

    // Check the driver version
    // File size only for MySql
    QString strDriverName;
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


    if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
    || (nQueryType==QUERYTHREAD_TYPE_INNODB))
    {
        // For MySql
        // The progress for table conversion can be check with the size of MyIsam data file

        // progressBar use the Data file size
        // Get the original size
        QDir clDir;
        clDir.setPath(strDir);
        clDir.setFilter(QDir::Files);
        // MyIsam files size (data/index)
        fDataFilesSize = 1;
        QStringList lstDataFiles = clDir.entryList(strFile+"*.MY*");
        QStringList::iterator it;
        for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
        {
            if((*it).contains("_myisam"))
                continue;
            if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
            && ((*it).contains("#P#")))
                continue;
            fDataFilesSize+= ut_GetFileSize(QString(strDir+*it).toLatin1().data());
        }
        fMyIsamSize = fDataFilesSize;

        if(nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
            strMsg = "Partitioning MyIsam table "+strFile;
        else
            strMsg = "Converting MyIsam table "+strFile+" to InnoDb";
        strMsg+= ".\nPlease wait ...";

        emit newText(strMsg);
    }

    clLastTimeUpdate.start();

    // Execute the Query
    start();

    // Wait the end of the query
    // Update the progressDialog with event
    while(isRunning() || !isFinished())
    {

        // Each 2 secondes
        if((clLastTimeUpdate.elapsed() > 2000)
        && (nProgressStep < 100))
        {
            nStep++;

            if((nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
            || (nQueryType==QUERYTHREAD_TYPE_INNODB))
            {
                // progressBar use the Data file size
                // Get the original size
                QDir clDir;
                QFileInfo clFile;
                clDir.setPath(strDir);
                clDir.setFilter(QDir::Files);
                // MyIsam files size (data/index)
                fDataFilesSize = 0;
                QStringList lstDataFiles;

                // check file as #P# for partitioning
                if(nQueryType==QUERYTHREAD_TYPE_PARTITIONING)
                    lstDataFiles = clDir.entryList("#sql*#P#*.MY*");
                else
                // Check temporary file as #sql for InnoDb
                if(nQueryType==QUERYTHREAD_TYPE_INNODB)
                    lstDataFiles = clDir.entryList("#sql*.ibd");

                QStringList::iterator it;
                for(it = lstDataFiles.begin(); it != lstDataFiles.end(); ++it )
                {
                    clFile.setFile(strDir,*it);
                    fDataFilesSize+= clFile.size();
                }
                nProgressStep = (int)((fDataFilesSize)/(fMyIsamSize*nQueryType)*100.0);
            }
            else
            {
                nProgressStep++;
                if(nProgressStep>=100)
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

        if(pApplication)
            pApplication->QCoreApplication::processEvents()();

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
    m_bError=m_pQuery->exec();
    this->msleep(200);
}


///////////////////////////////////////////////////////////
// Update DB: B12 -> B13
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13(QTextEdit *pTextEdit_Log, QProgressBar *pProgressBar)
{
    QString				strQuery;
    bool				bStatus = false;
    QString				strLogMessage, strErrorMessage;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nIncrementalUpdates = 0;
    QString				strRootUser, strRootPassword;

    // Check driver
    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB() && !m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        InsertIntoUpdateLog(pTextEdit_Log, "NOT SUPPORTED ...");
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Check if some custom updates to be performed
    //    CHECK THIS FIRST, PRIOR TO CREATING AZ TABLES!!
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    goto updatedb_b12_to_b13_error;

    // Make sure progress bar is set to max
    if(pProgressBar)
    {
        int nPorgressMax = pProgressBar->maximum();
        pProgressBar->setValue(nPorgressMax);
        QApplication::QCoreApplication::processEvents()();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update Global Info
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Updating GLOBAL_INFO table.";
    InsertIntoUpdateLog(pTextEdit_Log, strLogMessage);

    /////////////////////////////////////////
    // GLOBAL_INFO TABLE
    /////////////////////////////////////////

    // Add INCREMENTAL_SPLITLOTS
    // Check if some incremental updates pending
    strQuery = "SELECT SUM(remaining_splitlots) from incremental_update";
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;
    if(clQuery.First())
        nIncrementalUpdates = clQuery.value(0).toInt();

    strQuery = "TRUNCATE TABLE global_info";
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;

    strQuery = "INSERT INTO global_info VALUES('";
    strQuery += GetDbName(GEXDB_DB_VERSION_NAME_B13);
    strQuery += "'," + QString::number(GEXDB_DB_VERSION_NB_B13);
    strQuery += "," + QString::number(GEXDB_DB_VERSION_BUILD_B13);
    strQuery += "," + QString::number(nIncrementalUpdates);
    strQuery += ")";
    if(!clQuery.Execute(strQuery)) goto updatedb_b12_to_b13_error;

    if(m_pclDatabaseConnector->IsOracleDB())
        clQuery.Execute("COMMIT");
    else if(m_pclDatabaseConnector->IsMySqlDB())
        clQuery.Execute("UNLOCK TABLES");

    // Success
    bStatus = true;
    InsertIntoUpdateLog(pTextEdit_Log, " ");
    strLogMessage = "Status = SUCCESS.";
    InsertIntoUpdateLog(pTextEdit_Log, strLogMessage);
    goto updatedb_b12_to_b13_writelog;

updatedb_b12_to_b13_error:
    // Write error message
    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
updatedb_b12_to_b13_error_noset:
    strErrorMessage = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(pTextEdit_Log, " ");
    strLogMessage = "Status = ERROR (";
    strLogMessage+= strErrorMessage.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    strLogMessage+= ").";
    InsertIntoUpdateLog(pTextEdit_Log, strLogMessage);

updatedb_b12_to_b13_writelog:

    InsertIntoUpdateLog(pTextEdit_Log, " ");
    strLogMessage = "Update history saved to log file ";
    strLogMessage+= m_strUpdateDbLogFile;
    strLogMessage+= ".";
    InsertIntoUpdateLog(pTextEdit_Log, strLogMessage);

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update DB: B12 -> B13: custom update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateDb_B12_to_B13_Custom(QTextEdit *pTextEdit_Log, QProgressBar *pProgressBar)
{
    QString		strQuery;
    QSqlQuery	clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);

    return true;
}
