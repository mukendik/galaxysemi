#include <QDir>
#include <QSqlError>
#include <QProgressBar>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QMessageBox>
#include "gexdb_plugin_galaxy.h"
#include "gexdb_getroot_dialog.h"
#include "gex_shared.h"

QString GexDbPlugin_Galaxy::GetCreationScriptName(const DataBaseType &type)
{
    QString lScriptName("gexdb*install*.sql");
    if(type == eYmAdminDb)
        lScriptName = "ym_admin_db*install*.sql";
    else if((type == eAdrDb) || (type == eAdrLocalDb))
        lScriptName = "adr*install*.sql";

    return lScriptName;
}

QString GexDbPlugin_Galaxy::GetUpdateScriptName(const DataBaseType &type)
{
    QString lScriptName("gexdb*update*.sql");
    if(type == eYmAdminDb)
        lScriptName = "ym_admin_db*update*.sql";
    else if((type == eAdrDb) || (type == eAdrLocalDb))
        lScriptName = "adr*update*.sql";

    return lScriptName;
}

bool GexDbPlugin_Galaxy::CreateDatabase(const GexDbPlugin_Connector settingConnector,
                                        const GexDbPlugin_Galaxy::DataBaseType &dbType,
                                        const QString &rootName, const QString &rootPwd,
                                        QStringList& lstUninstall,
                                        QProgressBar *progressBar,
                                        bool lActiveKeepComment /*= false*/)
{
    QString     strSqlPath;
    QDir        clDir;
    QString     strMessage;
    bool        bStatus = false;
    bool        bCreateDatabase = false;
    QString     lDBName = settingConnector.m_strDatabaseName;

    // Connect to MySql/SQLite as Root
    QString strPluginName;
    GetPluginName(strPluginName);
    GexDbPlugin_Connector	clDbRootConnector(strPluginName+"_Root", this);

    clDbRootConnector = settingConnector;
    clDbRootConnector.m_strUserName = rootName;
    clDbRootConnector.m_strPassword = rootPwd;
    clDbRootConnector.m_strUserName_Admin = rootName;
    clDbRootConnector.m_strPassword_Admin = rootPwd;
    clDbRootConnector.m_strDatabaseName = "information_schema";
    clDbRootConnector.m_strSchemaName = "information_schema";
    clDbRootConnector.m_strConnectionName = strPluginName+"_Root";

    strSqlPath = m_strApplicationPath+QDir::separator();

    // Check if the source SQL file exist
    if(clDbRootConnector.IsMySqlDB())
        strSqlPath += "install/mysql/";
    else
    {
        return false;
    }

    clDir.setPath(strSqlPath);

    int         nValue;
    QFile       clFile;
    QString     strValue;
    QTextStream hFile(&clFile);
    QStringList lstFiles;
    QString     lSqlFile  = GetCreationScriptName(dbType);
    lstFiles = clDir.entryList( QStringList(lSqlFile), QDir::Files, QDir::Name);
    if(lstFiles.isEmpty())
    {
        // error
        InsertIntoUpdateLog("<b> ***********************************************************</b>",true);
        InsertIntoUpdateLog("<b> -----------------------   WARNING   -----------------------</b>",true);
        InsertIntoUpdateLog("<b> Sql install script not found ("+strSqlPath+lSqlFile+") !</b>",true);
        InsertIntoUpdateLog("<b> ***********************************************************</b>",true);
        return bStatus;
    }

    // take the last version
    while(!lstFiles.isEmpty())
    {
        strValue = lstFiles.takeFirst();
        if(!lSqlFile.isEmpty())
        {
            nValue = lSqlFile.toLower().section("_b",1).section(".",0,0).toInt();
            if(nValue <= strValue.toLower().section("_b",1).section(".",0,0).toInt())
                lSqlFile = strSqlPath + strValue;
        }
        else
            lSqlFile = strSqlPath + strValue;
    }

    // traces

    InsertIntoUpdateLog("<b>** "+ settingConnector.m_strDatabaseName.toUpper()+" DATABASE CREATION **</b>");
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog("<b>o Host Name / IP</b> " + settingConnector.m_strHost_IP);
    InsertIntoUpdateLog("<b>o Port</b> " + QString::number(settingConnector.m_uiPort));
    InsertIntoUpdateLog("<b>o DB Name</b> "+ settingConnector.m_strDatabaseName);
    InsertIntoUpdateLog("<b>o Admin Name</b> " + settingConnector.m_strUserName_Admin);
    if(settingConnector.m_strUserName != settingConnector.m_strUserName_Admin)
        InsertIntoUpdateLog("<b>o User Name</b> " + settingConnector.m_strUserName);

    InsertIntoUpdateLog("<b>o Sql Script: </b>" + lSqlFile + "\n");

    clDbRootConnector.SetAdminLogin(true);
    if(!clDbRootConnector.Connect())
    {
        clDbRootConnector.GetLastError(strMessage);
        InsertIntoUpdateLog("Root connection failed:\n"+strMessage,true);

        // to be modified
        //        button(QWizard::BackButton)->setEnabled(true);
        clDbRootConnector.Disconnect();

        return false;
    }

    clFile.setFileName(lSqlFile);
    if(!clFile.open(QIODevice::ReadOnly))
    {
        // Connect as admin
        strMessage = clFile.errorString();
        InsertIntoUpdateLog(strMessage,true);
        return false;
    }

    QString		strQuery;
    QSqlQuery	*clQuery = new QSqlQuery(QSqlDatabase::database(clDbRootConnector.m_strConnectionName));

    strQuery ="SHOW DATABASES LIKE '" + lDBName + "'";

    if(!clQuery->exec(strQuery))
    {
        strMessage = "Error executing SQL query.\n";
        strMessage+= "QUERY=" + strQuery + "\n";
        strMessage+= "ERROR=" + clQuery->lastError().text();
        InsertIntoUpdateLog(strMessage,true);
        delete clQuery;clQuery = 0;clQuery = 0;
        goto labelError;
    }

    if(!clQuery->first())
        bCreateDatabase = true;

    if(!bCreateDatabase)
    {
        if(dbType == eYmAdminDb)
        {
            InsertIntoUpdateLog("<b>** "+ settingConnector.m_strDatabaseName.toUpper()+" DATABASE CREATION **</b>");
            InsertIntoUpdateLog(" ");
            InsertIntoUpdateLog("YieldMan Administration Database already exist !");
            InsertIntoUpdateLog(" ");
            InsertIntoUpdateLog("DATABASE CREATION : <b>Already DONE</b>");
            InsertIntoUpdateLog(" ");

            bStatus = true;
            goto labelExit;
        }
        // If already exist then warning
        strMessage = "Host Name / IP " + settingConnector.m_strHost_IP;
        strMessage+= ", Port " + QString::number(settingConnector.m_uiPort);
        InsertIntoUpdateLog(strMessage);
        strMessage = "DB Name "+lDBName;
        strMessage+= ", Admin Name " + settingConnector.m_strUserName_Admin;
        if(settingConnector.m_strUserName_Admin != settingConnector.m_strUserName)
            strMessage+= ", User Name " + settingConnector.m_strUserName;
        InsertIntoUpdateLog(strMessage);
        if(clDbRootConnector.IsMySqlDB())
            strMessage = "o Database already exists:";
        else
            strMessage = "o Users already exist:";
        InsertIntoUpdateLog(strMessage);
        goto labelError;
    }
    else
    {
        SetUpdateDbLogFile("gexdb_update_"+
                           settingConnector.m_strUserName_Admin.toLower()+
                           "_creation_"+
                           QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")+".log");

        if(!UpdateDb_CheckServerVersion(dbType, clDbRootConnector.m_strConnectionName))
            goto labelError;


        // ============================================================
        // CREATE AN EMPTY GEXDB
        // ============================================================

        InsertIntoUpdateLog("o Creating database...");

        strQuery = "CREATE DATABASE " + lDBName;
        if(!clQuery->exec(strQuery))
        {
            strMessage = "Error executing SQL query.\n";
            strMessage+= "QUERY=" + strQuery + "\n";
            strMessage+= "ERROR=" + clQuery->lastError().text();
            InsertIntoUpdateLog(strMessage,true);
            goto labelError;
        }
        lstUninstall.append("DROP DATABASE " + lDBName);

        InsertIntoUpdateLog("o Creating users...");
        // ============================================================
        // CREATE GEXDB USERS
        // ============================================================
        if(!CreateUpdateDatabaseUsers(settingConnector, clDbRootConnector.m_strConnectionName, dbType, lstUninstall, strMessage))
        {
            InsertIntoUpdateLog(strMessage,true);
            goto labelError;
        }

        // Connect as GexDb admin
        clDbRootConnector.m_strDatabaseName = lDBName;
        clDbRootConnector.m_strSchemaName = settingConnector.m_strUserName_Admin;
        clDbRootConnector.m_strUserName = settingConnector.m_strUserName_Admin;
        clDbRootConnector.m_strPassword = settingConnector.m_strPassword_Admin;
        clDbRootConnector.m_strUserName_Admin = settingConnector.m_strUserName_Admin;
        clDbRootConnector.m_strPassword_Admin = settingConnector.m_strPassword_Admin;

        if(!clDbRootConnector.Connect())
        {
            clDbRootConnector.GetLastError(strMessage);
            InsertIntoUpdateLog(strMessage,true);
            goto labelError;
        }

        strQuery = "USE " + lDBName;
        if(!clQuery->exec(strQuery))
        {
            strMessage = "Error executing SQL query.\n";
            strMessage+= "QUERY=" + strQuery + "\n";
            strMessage+= "ERROR=" + clQuery->lastError().text();
            InsertIntoUpdateLog(strMessage,true);
            goto labelError;
        }
        lstUninstall.append("USE information_schema");

        InsertIntoUpdateLog("o Creating tables...");


        // ============================================================
        // CREATE ALL GEXDB TABLES
        // ============================================================

        // Create GexDb tables

        QString			strDelimiter;
        QString			strLine;
        bool            lKeepProcedureComment = false;

        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        int iProgressStep = 0;
        int iNextFilePos = 0;
        int iFileSize = clFile.size() + 1;
        progressBar->show();
        progressBar->setValue(0);
        progressBar->setMaximum(100);

        strQuery = "";
        strDelimiter = ";";
        while(! hFile.atEnd())
        {
            strLine = hFile.readLine().simplified();
            if(strLine.isEmpty())
                continue;
            if(strLine.startsWith("--") && lKeepProcedureComment == false)
                continue;
            if(strLine.startsWith("exit;"))
                break;
            if(strLine.startsWith("DEFINE",Qt::CaseInsensitive))
                continue;
            if(strLine.startsWith("DELIMITER",Qt::CaseInsensitive))
            {
                strDelimiter = strLine.section(" ",1,1);
                if(lActiveKeepComment == true)
                    lKeepProcedureComment = true;
                continue;
            }

            while((int) hFile.device()->pos() > iNextFilePos)
            {
                iProgressStep += 100/iFileSize + 1;
                iNextFilePos  += iFileSize/100 + 1;
                progressBar->setValue(iProgressStep);
                QCoreApplication::processEvents();
            }

            strQuery += strLine;
            if(strLine.indexOf(strDelimiter) >= 0)
            {
                strQuery = strQuery.remove(strDelimiter);
                // Execute the query
                if(!clQuery->exec(strQuery))
                {
                    strMessage = "Error executing SQL query.\n";
                    strMessage+= "QUERY=" + strQuery + "\n";
                    strMessage+= "ERROR=" + clQuery->lastError().text();
                    InsertIntoUpdateLog(strMessage,true);
                    goto labelError;
                }
                if((strQuery.startsWith("CREATE ",Qt::CaseInsensitive))
                        && (!strQuery.contains(" DATABASE ",Qt::CaseInsensitive)))
                {
                    strQuery = strQuery.remove("TEMPORARY").simplified();
                    strQuery = strQuery.remove("OR REPLACE").simplified();
                    if(strQuery.count("(") > 0)
                        strQuery = strQuery.section("(",0,0);
                    InsertIntoUpdateLog(strQuery.section(" ",0,2) + "... DONE");

                    strQuery = "DROP " + strQuery.section(" ",1,2);
                    if(strQuery.indexOf(" TABLESPACE ") > 0)
                        strQuery+= " INCLUDING CONTENTS AND DATAFILES";
                    if(strQuery.indexOf(" USER ") > 0)
                        strQuery+= " CASCADE";
                    lstUninstall.append(strQuery);
                }
                strQuery = "";
            }
            else
                strQuery += "\n";
        }

        if(!(dbType == eYmAdminDb))
        {
            // Connect as gexdb_admin
            // TDR Update TDR DB type in global info
            if (!UpdateDbSetTdrType(lDBName, dbType, clQuery))
                goto labelError;

            // Connect to DB (admin user)
            clDbRootConnector.SetAdminLogin(true);
            if(!clDbRootConnector.Connect())
            {
                clDbRootConnector.GetLastError(strMessage);
                InsertIntoUpdateLog(strMessage,true);
                goto labelError;
            }

            *m_pclDatabaseConnector = clDbRootConnector;
            UpdateDatabase();


            InsertIntoUpdateLog(" ");
            InsertIntoUpdateLog("<b>** DATABASE CREATION SUCCESS **</b>");
            InsertIntoUpdateLog(" ");
            InsertIntoUpdateLog("<b>o Host Name / IP</b> " + settingConnector.m_strHost_IP);
            InsertIntoUpdateLog("<b>o Port</b> " + QString::number(settingConnector.m_uiPort));
            InsertIntoUpdateLog("<b>o DB Name</b> "+ settingConnector.m_strDatabaseName);
            InsertIntoUpdateLog("<b>o Admin Name</b> " + settingConnector.m_strUserName_Admin);
            if(settingConnector.m_strUserName_Admin != settingConnector.m_strUserName)
                InsertIntoUpdateLog("<b>o User Name</b> " + settingConnector.m_strUserName);

            InsertIntoUpdateLog("<b>o Sql Script: </b>" + lSqlFile + "\n");

            InsertIntoUpdateLog("o Database creation completed");

            progressBar->setValue(0);
            progressBar->setMaximum(50);

            clDbRootConnector.Disconnect();
        }

        progressBar->setValue(100);
    }

    bStatus = true;
    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("DATABASE CREATION : <b>Success</b>");
    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("");

    progressBar->hide();
    goto labelExit;

labelError:

    if((strMessage.toLower().indexOf("'root'@'%'") > 0)
            && (settingConnector.m_strHost_IP.toLower() != "localhost")
            && (m_strHostName.toLower() != settingConnector.m_strHost_IP.toLower()))
    {
        strMessage = "Connection probably necessary on localhost...";
        InsertIntoUpdateLog(strMessage);
    }

    InsertIntoUpdateLog("");
    InsertIntoUpdateLog("DATABASE CREATION : <b>Error</b>",true);
    InsertIntoUpdateLog("");

    if(!lstUninstall.isEmpty())
    {
        if(clDbRootConnector.m_strUserName != rootName)
        {
            if(clQuery){
                delete clQuery ;
                clQuery = 0;
            }
            clDbRootConnector.Disconnect();

            // Connect as root again
            clDbRootConnector.m_strUserName = rootName;
            clDbRootConnector.m_strPassword = rootPwd;
            clDbRootConnector.m_strUserName_Admin = rootName;
            clDbRootConnector.m_strPassword_Admin = rootPwd;
            if(clDbRootConnector.IsMySqlDB())
            {
                clDbRootConnector.m_strDatabaseName = "information_schema";
                clDbRootConnector.m_strSchemaName = "information_schema";
            }
            else
            {
                clDbRootConnector.m_strDatabaseName = lDBName;
                clDbRootConnector.m_strSchemaName = "SYSTEM";
            }

            clDbRootConnector.Connect();
            clQuery = new QSqlQuery(QSqlDatabase::database(clDbRootConnector.m_strConnectionName));
        }
        else if(!clDbRootConnector.IsConnected())
            clDbRootConnector.Connect();

        while(!lstUninstall.isEmpty())
            clQuery->exec(lstUninstall.takeLast());
    }

labelExit:
    if(clQuery)
    {
        delete clQuery ;
    }
    clDbRootConnector.Disconnect();

    clFile.close();
    return bStatus;
}


///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::CreateUpdateDatabaseUsers
////// \param IN settingConnector: the Database info for users
////// \param IN rootConnection: the SqlDatabase connection name if available
////// \param IN dbType: type of the Database created/updated
////// \param OUT lstUninstall: list of queries for an undo if needed
////// \param OUT errorMessage: error message if any
////// \return
bool GexDbPlugin_Galaxy::CreateUpdateDatabaseUsers(const GexDbPlugin_Connector settingConnector,
                                                   const QString rootConnection,
                                                   const GexDbPlugin_Galaxy::DataBaseType &dbType,
                                                   QStringList& lstUninstall, QString& errorMessage)
{
    QString         lDatabaseName = settingConnector.m_strDatabaseName;
    QString         lConnectionName = rootConnection;
    QSqlDatabase    lSqlDatabase = QSqlDatabase::database(lConnectionName);
    GexDbPlugin_Connector   lDatabaseConnector(settingConnector.m_strPluginName, this);

    // Check if a root connection is specified
    if(lConnectionName.isEmpty())
    {
        // Ask for root and pwd
        GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

        if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
        {
            errorMessage = "Action was canceled";
            return false;
        }

        // Retrieve values from dialog
        QString lRootName = clGexdbGetrootDialog.GetRootUsername();
        QString lRootPwd = clGexdbGetrootDialog.GetRootPassword();

        // Start a new connection
        lDatabaseConnector = settingConnector;
        lDatabaseConnector.m_strUserName_Admin = lRootName;
        lDatabaseConnector.m_strPassword_Admin = lRootPwd;
        lDatabaseConnector.m_strDatabaseName = "information_schema";
        lDatabaseConnector.m_strSchemaName = "information_schema";

        lDatabaseConnector.m_strConnectionName = lDatabaseName+"_CreateUpdateDatabaseUsers";

        lDatabaseConnector.SetAdminLogin(true);
        if(!lDatabaseConnector.Connect())
            return false;

        lSqlDatabase = QSqlDatabase::database(lDatabaseConnector.m_strConnectionName);
    }

    GexDbPlugin_Query   clQuery(this, lSqlDatabase);
    QString             lQuery;


    errorMessage = "";


    // ============================================================
    // CREATE GEXDB USERS
    // ============================================================

    // Check if the user already exist
    QMap<QString, QStringList> lUserHosts;
    lQuery = "SELECT DISTINCT USER, HOST FROM mysql.user WHERE User='" + settingConnector.m_strUserName_Admin + "'";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY=" + lQuery + "\n";
        errorMessage+= "ERROR=" + clQuery.lastError().text();
        return false;
    }
    while(clQuery.next())
    {
        lUserHosts[clQuery.value("USER").toString()] << clQuery.value("HOST").toString();
    }

    // The user must be created before any GRANT
    // it is possible that a GRANT cannot create a user if the NO_AUTO_CREATE_USER is set in sql_mode
    // First create the user
    if(!lUserHosts.contains(settingConnector.m_strUserName_Admin)
            || !lUserHosts[settingConnector.m_strUserName_Admin].contains("localhost"))
    {
        // New user for localhost
        lQuery = "CREATE USER '" + settingConnector.m_strUserName_Admin + "'@'localhost'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        lstUninstall.append("DROP USER '" + settingConnector.m_strUserName_Admin + "'@'localhost'");
    }

    if(!lUserHosts.contains(settingConnector.m_strUserName_Admin)
            || !lUserHosts[settingConnector.m_strUserName_Admin].contains("%"))
    {
        // New user for %
        lQuery = "CREATE USER '" + settingConnector.m_strUserName_Admin + "'@'%'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        lstUninstall.append("DROP USER '" + settingConnector.m_strUserName_Admin + "'@'%'");
    }

    // Give ALL PRIVILEGES on the schema
    // on localhost
    lQuery = "GRANT ALL PRIVILEGES ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName_Admin + "'@'localhost' IDENTIFIED BY '" + settingConnector.m_strPassword_Admin + "'";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY=" + lQuery + "\n";
        errorMessage+= "ERROR=" + clQuery.lastError().text();
        return false;
    }

    // for all other hosts
    lQuery = "GRANT ALL PRIVILEGES ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName_Admin + "'@'%' IDENTIFIED BY '" + settingConnector.m_strPassword_Admin + "'";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY=" + lQuery + "\n";
        errorMessage+= "ERROR=" + clQuery.lastError().text();
        return false;
    }

    // Give some Administrative roles for LOAD DATA INFILE and PROCESSLIST
    // on localhost
    lQuery = "GRANT PROCESS, FILE ON *.* TO '" + settingConnector.m_strUserName_Admin + "'@'localhost'";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY = " + lQuery + "\n";
        errorMessage+= "ERROR = " + clQuery.lastError().text();
        return false;
    }

    // for all other hosts
    lQuery = "GRANT PROCESS, FILE ON *.* TO '" + settingConnector.m_strUserName_Admin + "'@'%'";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY = " + lQuery + "\n";
        errorMessage+= "ERROR = " + clQuery.lastError().text();
        return false;
    }

    if(dbType == eYmAdminDb)
    {
        // on localhost
        lQuery = "GRANT GRANT OPTION ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName_Admin + "'@'localhost'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        // for all other hosts
        lQuery = "GRANT GRANT OPTION ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName_Admin + "'@'%'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        // on localhost
        lQuery = "GRANT CREATE USER ON *.* TO '" + settingConnector.m_strUserName_Admin + "'@'localhost'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        // for all other hosts
        lQuery = "GRANT CREATE USER ON *.* TO '" + settingConnector.m_strUserName_Admin + "'@'%'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
    }
    else if(settingConnector.m_strUserName_Admin != settingConnector.m_strUserName)
    {
        // Check if the user already exist
        lQuery = "SELECT DISTINCT USER, HOST FROM mysql.user WHERE User='" + settingConnector.m_strUserName + "'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        while(clQuery.next())
        {
            lUserHosts[clQuery.value("USER").toString()] << clQuery.value("HOST").toString();
        }

        // The user must be created before any GRANT
        // it is possible that a GRANT cannot create a user if the NO_AUTO_CREATE_USER is set in sql_mode
        // First create the user
        if(!lUserHosts.contains(settingConnector.m_strUserName)
                || !lUserHosts[settingConnector.m_strUserName].contains("localhost"))
        {
            // New user for localhost
            lQuery = "CREATE USER '" + settingConnector.m_strUserName + "'@'localhost'";
            if(!clQuery.exec(lQuery))
            {
                errorMessage = "Error executing SQL query.\n";
                errorMessage+= "QUERY=" + lQuery + "\n";
                errorMessage+= "ERROR=" + clQuery.lastError().text();
                return false;
            }
            lstUninstall.append("DROP USER '" + settingConnector.m_strUserName + "'@'localhost'");
        }

        if(!lUserHosts.contains(settingConnector.m_strUserName)
                || !lUserHosts[settingConnector.m_strUserName].contains("%"))
        {
            // New user for %
            lQuery = "CREATE USER '" + settingConnector.m_strUserName + "'@'%'";
            if(!clQuery.exec(lQuery))
            {
                errorMessage = "Error executing SQL query.\n";
                errorMessage+= "QUERY=" + lQuery + "\n";
                errorMessage+= "ERROR=" + clQuery.lastError().text();
                return false;
            }
            lstUninstall.append("DROP USER '" + settingConnector.m_strUserName + "'@'%'");
        }

        // on localhost
        lQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName + "'@'localhost' IDENTIFIED BY '" + settingConnector.m_strPassword + "'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
        // for all other hosts
        lQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON " + lDatabaseName + ".* TO '" + settingConnector.m_strUserName + "'@'%' IDENTIFIED BY '" + settingConnector.m_strPassword + "'";
        if(!clQuery.exec(lQuery))
        {
            errorMessage = "Error executing SQL query.\n";
            errorMessage+= "QUERY=" + lQuery + "\n";
            errorMessage+= "ERROR=" + clQuery.lastError().text();
            return false;
        }
    }

    lQuery = "FLUSH PRIVILEGES";
    if(!clQuery.exec(lQuery))
    {
        errorMessage = "Error executing SQL query.\n";
        errorMessage+= "QUERY=" + lQuery + "\n";
        errorMessage+= "ERROR=" + clQuery.lastError().text();
        return false;
    }
    return true;
}


bool GexDbPlugin_Galaxy::UpdateDatabase()
{
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog("<b>** UPDATING "+ m_pclDatabaseConnector->m_strSchemaName.toUpper()+" DATABASE **</b>");
    InsertIntoUpdateLog(" ");
    // No consolidation process for char DB
    if (IsYmProdTdr() || IsManualProdTdr())
    {
        InsertIntoUpdateLog("o Consolidation tree");
        InsertIntoUpdateLog("o Consolidation process");
        InsertIntoUpdateLog("o Consolidation data");
    }


    // Updating Meta-Data mapping and consolidated process
    if(!UpdateDb())
    {
        // Ignore any error during the Update
        //QString strError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        //QMessageBox::warning(this,"** DATABASE UPDATE WARNING **",strError);

        InsertIntoUpdateLog(" ");
        InsertIntoUpdateLog("<b>** DATABASE UPDATE WARNING **</b>");
        InsertIntoUpdateLog(" ");
        InsertIntoUpdateLog("o Your database was successfully created but the consolidation process has been disabled.");
        InsertIntoUpdateLog("o You need to update your database again using the HouseKeeping tool");
    }
    else
    {
        InsertIntoUpdateLog(" ");
        InsertIntoUpdateLog("<b>** DATABASE UPDATE SUCCESS **</b>");
        InsertIntoUpdateLog(" ");
        // No consolidation process for char DB
        if (IsYmProdTdr() || IsManualProdTdr())
        {
            InsertIntoUpdateLog("o Consolidation tree ... DONE");
            InsertIntoUpdateLog("o Consolidation process ... DONE");
            InsertIntoUpdateLog("o Consolidation data ... DONE");
        }
    }
    return true;
}


bool GexDbPlugin_Galaxy::UpdateDbSetTdrType(const QString& dbName,
                                            const GexDbPlugin_Galaxy::DataBaseType &dbType,
                                            QSqlQuery *sqlQuery)
{
    QString lTdrDbType, lQuery, lErrorMsg;
    if (!sqlQuery)
        return false;

    lTdrDbType = dbName;
    if (dbType == GexDbPlugin_Galaxy::eCharacTdrDb) // Charac
        lTdrDbType += GEXDB_CHAR_TDR_KEY;
    else if (dbType == GexDbPlugin_Galaxy::eManualProdDb) // Prod
        lTdrDbType += GEXDB_MAN_PROD_TDR_KEY;
    else if (dbType == GexDbPlugin_Galaxy::eProdTdrDb) // Prod
        lTdrDbType += GEXDB_YM_PROD_TDR_KEY;
    else if (dbType == GexDbPlugin_Galaxy::eAdrDb) // ADR
        lTdrDbType += GEXDB_ADR_KEY;
    else if (dbType == GexDbPlugin_Galaxy::eAdrLocalDb) // localADR
        lTdrDbType += GEXDB_ADR_LOCAL_KEY;

    QByteArray lHash = QCryptographicHash::hash(lTdrDbType.toLatin1(),QCryptographicHash::Md5);
    lQuery = "UPDATE " + /*mUi_qleAdminName->text()*/dbName + ".global_info SET db_type='"+ QString(lHash.toHex()) +"'";
    if(!sqlQuery->exec(lQuery))
    {
        lErrorMsg = "Error executing SQL query.\n";
        lErrorMsg += "QUERY=" + lQuery + "\n";
        lErrorMsg += "ERROR=" + sqlQuery->lastError().text();
        InsertIntoUpdateLog(lErrorMsg,true);
        return false;
    }

    return true;
}
