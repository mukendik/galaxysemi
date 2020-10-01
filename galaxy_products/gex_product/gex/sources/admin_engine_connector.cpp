///////////////////////////////////////////////////////////
// Database admin: Create/Delete database, insert files
///////////////////////////////////////////////////////////
#include <QShortcut>
#include <QMenu>
#include <QInputDialog>
#include <QSqlError>
#include <QSqlRecord>

#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QPluginLoader>


#include "dir_access_base.h"
#include "admin_engine.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_constants.h"
#include "gex_database_entry.h"
#include "engine.h"
#include <gqtl_log.h>
#include "libgexoc.h"
#include "product_info.h"
#include "command_line_options.h"
#include <gqtl_sysutils.h>

// in main.cpp
//extern void                 WriteDebugMessageFile(const QString & strMessage);
extern CGexSkin*            pGexSkin;      // holds the skin settings
extern GexScriptEngine*     pGexScriptEngine;

// report_build.cpp
extern CReportOptions       ReportOptions;    // Holds options (report_build.h)
#include "read_system_info.h"
#include "message.h"

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////
// AdminEngine class: database connection...
// mutex = CONNECTION_ID():HOST:HOST_CONNECTOR
// - CONNECTION_ID is the # allocated by the Sql Server for this connection
// - HOST is the hostName or hostIp of the user who creates this connection
//      * localhost
//      * 192.168.1.27
// - HOST_CONNECTOR is the hostName of the Sql Server
//      * gx-head
//      * Mac.local
// when open a local connection on 'host1' = XX:localhost:host1
// when open a remote connection on 'host1' from 'host2' = XX:host2_IP:host1
// YM on comp1, user = XX:localhost:comp1
// GEX on laptop(192.168.1.55), user = XX:192.168.1.55:head
///////////////////////////////////////////////////////////

/******************************************************************************!
 * \fn IsMutexActive
 ******************************************************************************/
bool AdminEngine::IsMutexActive(QString lMutex)
{
    // If we are not able to know if the Id is active
    // We must return true
    if (! ConnectToAdminServer())
    {
        return false;
    }

    // No mutex
    if (lMutex.isEmpty())
    {
        return false;
    }

    // It's me
    if (lMutex == mMutex)
    {
        return true;
    }

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    QString lUserSID = lMutex.section(":", 0, 0);
    // QString lUserHostName = lMutex.section(":",1,1);
    QString lUserConnectorHostName = lMutex.section(":", 2, 2);
    QString lCurrentHostName = mMutex.section(":", 2, 2);
    // Same server
    // User laptop = XX:192.168.1.55:head
    // you XX:localhost:head
    if (lUserConnectorHostName.toUpper() == lCurrentHostName.toUpper())
    {
        strQuery = "SELECT * FROM information_schema.PROCESSLIST WHERE USER='"
                +m_pDatabaseConnector->m_strUserName_Admin+"' AND ID="
                +lUserSID;
        if (! clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return true;
        }

        if (! clQuery.first())
        {
            GSLOG(SYSLOG_SEV_CRITICAL, QString("EXPIRED mutex %1 for %2").
                  arg(lUserSID).
                  arg(lUserConnectorHostName).toLatin1().constData());
            return false;
        }
    }
    else
    {
        // For spider environement
        // The connection can be force the local HostName
        // for Node connector
        // we need to connect to the user connector host name
        // and check if the ID comes from this IP
        // User laptop = XX:192.168.1.55:head
        // you XX:localhost:comp1

        // Connect to this IP and check
        QString lConnection = m_pDatabaseConnector->m_strConnectionName;
        lConnection += " on ";
        lConnection += lUserConnectorHostName;

        {
            // Add database link: make sure we use a unique name for
            // the database connection
            // On some MySql server with AUTO connection configuration
            // The addDatabase can directly open a default connection
            // (see at ASE, not reproducible ?)
            QSqlDatabase clSqlDatabase;
            if (! QSqlDatabase::contains(lConnection))
            {
                clSqlDatabase =
                    QSqlDatabase::addDatabase(m_pDatabaseConnector->m_strDriver,
                                              lConnection);
                // Force to close the connection if AUTO connection
                if (clSqlDatabase.isOpen())
                {
                    clSqlDatabase.close();
                }
                clSqlDatabase.setHostName(lUserConnectorHostName);
                clSqlDatabase.
                    setDatabaseName(m_pDatabaseConnector->m_strDatabaseName);
                clSqlDatabase.
                    setUserName(m_pDatabaseConnector->m_strUserName_Admin);
                clSqlDatabase.
                    setPassword(m_pDatabaseConnector->m_strPassword_Admin);
                clSqlDatabase.setPort(m_pDatabaseConnector->m_uiPort);
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Create a new connection to "
                              "clean EXPIRED session_id on "
                              "%1 for %2: db[%3]user[%4]port[%5]").
                      arg(lCurrentHostName).
                      arg(lUserConnectorHostName).
                      arg(m_pDatabaseConnector->m_strDatabaseName).
                      arg(m_pDatabaseConnector->m_strUserName_Admin).
                      arg(m_pDatabaseConnector->m_uiPort).
                      toLatin1().constData());
            }
            else
            {
                clSqlDatabase = QSqlDatabase::database(lConnection);
            }

            if (! clSqlDatabase.isOpen())
            {
                clSqlDatabase.open();
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Open the connection to "
                              "clean EXPIRED session_id on "
                              "%1 for %2: db[%3]user[%4]port[%5]").
                      arg(lCurrentHostName).
                      arg(lUserConnectorHostName).
                      arg(m_pDatabaseConnector->m_strDatabaseName).
                      arg(m_pDatabaseConnector->m_strUserName_Admin).
                      arg(m_pDatabaseConnector->m_uiPort).
                      toLatin1().constData());
            }
        }

        QSqlQuery clUserQuery(QSqlDatabase::database(lConnection));
        strQuery = "SELECT * FROM information_schema.PROCESSLIST WHERE USER='"
                +m_pDatabaseConnector->m_strUserName_Admin+"' AND ID="
                +lUserSID;
        if (! clUserQuery.exec(strQuery))
        {
            SetLastError(clUserQuery);
            // Clean the connection if unable to open it
            QSqlDatabase::removeDatabase(lConnection);
            return true;
        }
        if (! clUserQuery.first())
        {
            GSLOG(SYSLOG_SEV_CRITICAL, QString("EXPIRED mutex %1 for %2").
                  arg(lUserSID).
                  arg(lUserConnectorHostName).toLatin1().constData());
            return false;
        }
    }
    return true;
}

bool AdminEngine::ConnectNode()
{


    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    if(m_nNodeId > 0)
        return true;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return true;

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    /////////////////////
    // YIELDMAN NODE
    // Check if the Cpu/HostId/ServerProfile is referenced
    // Check also if type is valid
    QString    Type = "YM";
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        Type = "PAT";

    QString    strCpu;
    QStringList lstCpu = m_strAllCpu.split(";",QString::SkipEmptyParts);

    if(!lstCpu.contains(m_strCpu))
        lstCpu.append(m_strCpu);

    QString strServerProfile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strServerProfile.replace("\\","/");

    strQuery = "SELECT node_id, cpu FROM ym_nodes WHERE ";
    strQuery+= "((LOWER(cpu) LIKE '%" + lstCpu.join("%') OR (LOWER(cpu) LIKE '%") + "%'))";
    strQuery+= " AND (host_id='"+m_strHostId+"' OR (host_id='?') OR (host_id='0') OR (LEFT(host_id,1)='-'))";
    strQuery+= " AND ((LOWER(gex_server_profile)="+NormalizeStringToSql(strServerProfile).toLower()+
            ") OR (LOWER(gex_server_profile)="+NormalizeStringToSql(strServerProfile+"/").toLower()+"))";
    if(IsLoadBalancingMode())
        strQuery+= " AND ((type IS NULL) OR (type='"+Type+"'))";
    if(!clQuery.exec(strQuery))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Query exec failed: %1")
              .arg( clQuery.lastError().text()).toLatin1().constData());
        SetLastError(clQuery);
        return false;
    }
    if(!clQuery.first())
    {
        // Never referenced
        // Have to insert new node_id
        // Send an email
        // have to create and update this new node_id
        QTime clTime = QTime::currentTime();
        strQuery = "INSERT INTO ym_nodes(";
        if(m_pDatabaseConnector->IsOracleDB())
            strQuery+="node_id,";
        strQuery+= "cpu,name) VALUES(";
        if(m_pDatabaseConnector->IsOracleDB())
            strQuery+="ym_nodes_sequence.nextval,";
        strQuery+= "'"+clTime.toString("hhmmsszz")+"',";
        strQuery+= NormalizeStringToSql(m_strHostName+"["+strServerProfile+"]")+")";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        // Retreve the NodeId
        strQuery = "SELECT node_id FROM ym_nodes WHERE cpu='"+clTime.toString("hhmmsszz")+"'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        clQuery.first();
        m_nNodeId = clQuery.value(0).toInt();

    }
    else
    {
        m_nNodeId = clQuery.value(0).toInt();
        // For all cpu referenced
        // check if have to add it
        strCpu = clQuery.value(1).toString().toLower();
        while(!strCpu.isEmpty())
        {
            if(!lstCpu.contains(strCpu.section(";",0,0)))
                lstCpu.append(strCpu.section(";",0,0));
            strCpu = strCpu.section(";",1).simplified();
        }
    }

    // Check if the node isn't already connected
    if(IsNodeConnected(m_nNodeId))
    {
        // Node already connected
        SetLastError(eDB_InvalidEntry, "Node already connected");
        return false;
    }


    // Only for Monitoring
    QString Summary = "Launch "+GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    if(GetServerVersionName(true) != GetCurrentVersionName(true))
        Summary+= "|ServerVersion="+GetServerVersionName(true)
                + "|CurrentVersion="+GetCurrentVersionName(true)+"|";

    Summary += "|" + GS::LPPlugin::ProductInfo::getInstance()->GetProfileVariable() +
               "=" + GS::Gex::Engine::GetInstance().Get("UserFolder").toString();

    // Update the Events flow
    AddNewEvent("APPLICATION",Type,
                "LAUNCH",Summary,GetServerVersionName(true));

    m_strAllCpu = lstCpu.join(";");

    // Then update YieldMan node
    strQuery = "UPDATE ym_nodes SET ";
    strQuery+= "  cpu='"+m_strAllCpu+"'";
    strQuery+= ", host_name='"+m_strHostName+"'";
    strQuery+= ", host_id='"+m_strHostId+"'";
    strQuery+= ", os='"+m_strOs+"'";
    strQuery+= ", os_login='"+m_strOsLogin+"'";
    strQuery+= ", gex_server_profile="+NormalizeStringToSql(strServerProfile)+"";
    strQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        clQuery.exec("COMMIT"); // To unlock any tables
        return false;
    }
    clQuery.exec("COMMIT");

    QString Date = "SYSDATE";
    if(m_pDatabaseConnector->IsMySqlDB())
        Date = "now()";

    // Check if node can start
    strQuery = "SELECT status FROM ym_nodes";
    strQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
    if(clQuery.exec(strQuery) && clQuery.first())
    {
        QString Status = clQuery.value(0).toString();

        if(Status == "STOP_REQUESTED")
        {
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().stopScheduler(true);
            Status = "STOP";
        }
        else
        {
            if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsWelcomeBoxEnabled() ||
                    GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
                Status = "STOP";
            else
                Status = "RUNNING";
        }

        strQuery = "UPDATE ym_nodes SET ";
        strQuery+= "  status='"+Status+"'";
        strQuery+= ", type='"+Type+"'";
        strQuery+= ", last_update="+Date;
        strQuery+= ", mutex='"+mMutex+"'";
        strQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
        clQuery.exec(strQuery);
        clQuery.exec("COMMIT"); // To unlock any tables

        // Update the Events flow
        AddNewEvent("APPLICATION",Type,
                    Status,"Monitoring is in "+Status+" mode","");

    }

    clQuery.exec("COMMIT");

    // To change the level of a module : GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_LOGLEVEL %1").arg( 3));
    LoadServerOptions();
    return true;
}

bool AdminEngine::DisconnectNode()
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        m_nNodeId = -1;
        return true;
    }

    if(m_nNodeId > 0)
    {

        // Stop the scheduler if running
        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerRunning())
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().RunTaskScheduler(false);

        // ShutDown
        QString        lQuery;
        QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

        QString Date = "SYSDATE";
        if(m_pDatabaseConnector->IsMySqlDB())
            Date = "now()";

        lQuery = "UPDATE ym_nodes SET ";
        lQuery+= "  status='SHUTDOWN'";
        lQuery+= ", last_update="+Date;
        lQuery+= ", mutex=null";
        lQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
        clQuery.exec(lQuery);

        clQuery.exec("COMMIT"); // To unlock any tables

        QString    Type = "YM";
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            Type = "PAT";

        // Update the Events flow
        AddNewEvent("APPLICATION",Type,
                    "CLOSE","Close "+GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),"");

    }

    m_nNodeId = -1;
    return true;
}

bool AdminEngine::UpdateNodeStatus()
{
    if(!ConnectToAdminServer())
        return false;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return true;

    QString        strQuery;
    QString strValue;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Check if the ym_node connection is alive
    ConnectNode();

    QString Date = "SYSDATE";
    if(m_pDatabaseConnector->IsMySqlDB())
        Date = "now()";

    // Check if node can start
    QString Status;
    strQuery = "SELECT status FROM ym_nodes";
    strQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
    // STOP_REQUESTED => stop the scheduler if started or running
    // START_REQUESTED => restart the scheduler if stopped
    if(clQuery.exec(strQuery) && clQuery.first())
    {
        Status = clQuery.value(0).toString();

        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()
                || (Status == "STOP_REQUESTED"))
        {
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().stopScheduler(true);
            Status = "STOP";

            if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()
                    && GS::Gex::Engine::GetInstance().HasTasksRunning())
                Status = "STOP_REQUESTED";
        }
        else if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped()
                && (Status == "START_REQUESTED"))
        {
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().stopScheduler(false);
            Status = "RUNNING";
        }
        else
            Status = "RUNNING";

        // If no change
        if(Status == clQuery.value(0).toString())
            return true;

        strQuery = "UPDATE ym_nodes SET ";
        strQuery+= "  status='"+Status+"'";
        strQuery+= ", last_update="+Date;
        if(!mMutex.isEmpty())
            strQuery+= ", mutex='"+mMutex+"'";
        strQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
        clQuery.exec(strQuery);

        QString    Type = "YM";
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            Type = "PAT";

        // Update the Events flow if status changed
        AddNewEvent("APPLICATION",Type,
                    Status,"Monitoring is in "+Status+" mode","");

        clQuery.exec("COMMIT"); // To unlock any tables
    }

    if((Status=="STOP") && !OtherNodesRunning())
    {
        // Stop requested
        // Clean actions list
        CleanActions("mutex IS NULL");
    }

    if((Status=="STOP")
            || (Status=="STOP_REQUESTED"))
        CancelActions();

    return true;
}

/******************************************************************************!
 * \fn OtherNodesRunning
 * \brief To manage other nodes
 * \return false on error
 ******************************************************************************/
bool AdminEngine::OtherNodesRunning()
{
    if (! ConnectToAdminServer())
    {
        return false;
    }

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));
    // Check if nodes running
    strQuery = "SELECT mutex FROM ym_nodes";
    strQuery +=
        " WHERE NOT(status='SHUTDOWN') "
        "AND NOT(status='STOP') "
        "AND (type='YM') "
        "AND (mutex IS NOT NULL) "
        "AND (mutex <> '') ";
    strQuery += " AND (node_id!=" + QString::number(m_nNodeId) + ") ";

    if (! clQuery.exec(strQuery))
    {
        return false;
    }

    bool lActiveNodes = false;
    QString lMutex;
    QStringList lMutexes;
    while (clQuery.next())
    {
        lMutexes << clQuery.value(0).toString();
    }
    while (! lMutexes.isEmpty())
    {
        lMutex = lMutexes.takeFirst();
        // Check the validity of the mutex
        if (IsMutexActive(lMutex))
        {
            lActiveNodes = true;
        }
        else
        {
            // Update the DB if the mutex is obsolete
            // and the status is not good
            strQuery =
                "UPDATE ym_nodes SET mutex=NULL, status='SHUTDOWN' "
                "WHERE mutex='" + lMutex + "'";
            clQuery.exec(strQuery);
        }
    }

    clQuery.exec("COMMIT");  // To unlock any tables
    return lActiveNodes;
}

/******************************************************************************!
 * \fn StopAllOtherNodes
 * \return false on error
 ******************************************************************************/
bool AdminEngine::StopAllOtherNodes()
{
    if (! ConnectToAdminServer())
    {
        return false;
    }

    if (! GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        return true;
    }

    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    QString Date = "SYSDATE";
    if (m_pDatabaseConnector->IsMySqlDB())
    {
        Date = "now()";
    }

    strQuery = "UPDATE ym_nodes SET ";
    strQuery += "  status='STOP_REQUESTED'";
    strQuery += ", last_update=" + Date;
    strQuery +=
        " WHERE NOT(status='SHUTDOWN') "
        "AND NOT(status='STOP') "
        "AND NOT(status='STOP_REQUESTED') "
        "AND (mutex IS NOT NULL) ";
    strQuery += " AND (node_id!=" + QString::number(m_nNodeId) + ") ";
    if (! clQuery.exec(strQuery))
    {
        clQuery.exec("COMMIT");  // To unlock any tables
        return false;
    }

    clQuery.exec("COMMIT");  // To unlock any tables

    // Wait until all nodes are STOPped
    QTime cTime, cWait;
    cTime = QTime::currentTime();
    while (true)
    {
        if (! OtherNodesRunning())
        {
            return true;
        }
        // Then wait 150msec & retry
        cWait = cTime.addMSecs(150);
        do
        {
            cTime = QTime::currentTime();
        }
        while (cTime < cWait);
    }

    return true;
}

bool AdminEngine::LoadServerSettings()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Load YieldMan Db Settings...");

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

#ifdef DA_GALAXY_REGENERATE

    AdminServerUpdate_dagalaxy();
    return false;
#endif

    if(!ConnectToAdminServer())
        return false;

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    /////////////////////
    // YIELDMAN NODE
    if(!ConnectNode())
        return false;

    // For the first connection
    // Load directory access plugin
    if (!LoadDirectoryAccessPlugin())
        return false;

    if(!LoadUsersList())
        return false;

    // Check if root access is updated
    // Then display the welcome message for first activation
    bool bNewInstall = false;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        strQuery = "SELECT last_access FROM ym_users WHERE lower(login)='root' OR lower(login)='admin'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if(clQuery.first())
        {
            if(clQuery.isNull(0) || clQuery.value(0).toString().isEmpty())
                bNewInstall = GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();
        }
        if(bNewInstall)
            m_bFirstActivation = true;
    }

    if(bNewInstall)
    {
        // to display summary about YieldManDb for User
        // and auto connect the root;

        QString strPwd = "ymadmin";

        strQuery = "SELECT pwd, user_id FROM ym_users WHERE lower(login)='root' OR lower(login)='admin'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if(clQuery.first())
        {
#ifdef QT_DEBUG
            // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
            // DO NOT ADD THIS GSLOG in RELEASE VERSION
            //GSLOG(SYSLOG_SEV_EMERGENCY, QString("Decrypting password for ym_users.user_id=1 from string: %1").arg(clQuery.value(0).toString()));
#endif
            if(!clQuery.isNull(0) && !clQuery.value(0).toString().isEmpty())
                GexDbPlugin_Base::DecryptPassword(clQuery.value(0).toString(),strPwd);
        }

        // And then connect to Yield-Man root
        ConnectUser(clQuery.value(1).toInt(),strPwd,true);
    }

    // Enabled or Disabled some access
    emit sEnableGexAccess();

    return true;
}

bool AdminEngine::LoadServerOptions()
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    // System identification

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    QStringList lstLogDefinedByUser;
    QString strTableName;
    QString strClauseWhere;

    // NODE/USER OPTIONS
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        strTableName = "ym_nodes_options";
        strClauseWhere = " AND (node_id="+QString::number(m_nNodeId)+" OR node_id IS NULL OR node_id=0) ORDER BY node_id";
    }
    else
    {
        strTableName = "ym_users_options";
        if(m_pCurrentUser)
            strClauseWhere = " AND (user_id="+QString::number(m_pCurrentUser->m_nUserId)+" OR user_id IS NULL OR user_id=0) ORDER BY user_id";
        else
            strClauseWhere = " AND (user_id IS NULL  OR user_id=0)";
    }

    // Load LOGLEVEL Options from database
    if(!strTableName.isEmpty())
    {
        // to have the null value after
        // order DESC for MySql
        // First value is the user settings
        // Second value is the global settings (null id)
        if(m_pDatabaseConnector->IsMySqlDB())
            strClauseWhere += " DESC";

        strQuery = "SELECT UPPER(field), UPPER(value) FROM "+strTableName;
        strQuery+= " WHERE field LIKE '%_LOGLEVEL'";
        strQuery+= strClauseWhere;
        if(clQuery.exec(strQuery))
        {
            while(clQuery.next())
            {
                // Check if already defined by the user
                if(lstLogDefinedByUser.contains(clQuery.value(0).toString()))
                    continue;

                lstLogDefinedByUser.append(clQuery.value(0).toString());
                if(!m_mapLogLevel.contains(clQuery.value(0).toString()) || (m_mapLogLevel[clQuery.value(0).toString()] != clQuery.value(1).toString()))
                {
                    GSLOG(SYSLOG_SEV_DEBUG, QString(clQuery.value(0).toString()+" %1")
                          .arg( clQuery.value(1).toInt())
                          .toLatin1().constData());
                    m_mapLogLevel[clQuery.value(0).toString()] = clQuery.value(1).toString();
                }
            }
        }
    }


    // GLOBAL OPTIONS
    // Load LOGLEVEL Options from database
    strQuery = "SELECT UPPER(field), value FROM ym_settings";
    strQuery+= " WHERE field LIKE '%_LOGLEVEL'";
    if(clQuery.exec(strQuery))
    {
        while(clQuery.next())
        {
            if(lstLogDefinedByUser.contains(clQuery.value(0).toString()))
                continue;

            if(!m_mapLogLevel.contains(clQuery.value(0).toString()) || (m_mapLogLevel[clQuery.value(0).toString()] != clQuery.value(1).toString()))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString(clQuery.value(0).toString()+" %1")
                      .arg( clQuery.value(1).toInt())
                      .toLatin1().constData());
                m_mapLogLevel[clQuery.value(0).toString()] = clQuery.value(1).toString();
            }
        }
    }

    return true;
}

bool AdminEngine::LoadUsersList()
{

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    // Load users privileges list from da_galaxy
    GS::DAPlugin::UsersBase * users = NULL;
    GS::DAPlugin::GroupsBase * groups = NULL;
    GS::DAPlugin::AppEntriesBase * applications = NULL;
    if(mDirAccessPlugin)
    {
        users = mDirAccessPlugin->GetUsers();
        groups = mDirAccessPlugin->GetGroups();
        applications = mDirAccessPlugin->GetAppEntries();
        if(!users || !applications || !groups)
        {
            QString strMessage;
            GetLastError(strMessage);
            GSLOG(SYSLOG_SEV_EMERGENCY, QString("Cannot load da_galaxy: %1")
                  .arg(strMessage)
                  .toLatin1().constData());
            //return false;
        }
    }

    int             iIndex;
    QString         strQuery;
    QSqlQuery       clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    AdminUser       *pUser=0;

    // Load all users
    bool bUsersGroupsAdminConnected = HasUserGroupAdminPrivileges();
    bool bConnected = IsUserConnected();

    // User list is already up-to-date, nothing to do
    bool ReloadUsersFromDb = false;
    // Save timestamp of Task file loaded
    if(m_pDatabaseConnector->IsMySqlDB())
        strQuery = "SELECT SUM(CRC32(user_id)), SUM(CRC32(last_update)), count(*) FROM ym_users";
    else
        strQuery = "SELECT SUM(ORA_HASH(user_id, POWER(2,16))) , SUM(ORA_HASH(last_update, POWER(2,16))), count(*) FROM ym_users";

    clQuery.exec(strQuery);
    // Check for all users
    if(clQuery.first() && (clQuery.value(2).toInt() > 0))
    {
        if(m_LastDbUsersIdChecksum != clQuery.value(0).toLongLong())
            ReloadUsersFromDb = true;
        m_LastDbUsersIdChecksum = clQuery.value(0).toLongLong();
        if(m_LastDbUsersUpdateChecksum != clQuery.value(1).toLongLong())
            ReloadUsersFromDb = true;
        m_LastDbUsersUpdateChecksum = clQuery.value(1).toLongLong();
    }
    else
    {
        ReloadUsersFromDb = true;
        m_LastDbUsersIdChecksum = 0;
        m_LastDbUsersUpdateChecksum = 0;
    }

    if(ReloadUsersFromDb)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "LoadUserList : User list is not up-to-date");
        /////////////////////
        // YIELDMAN USERS
        // Check if YieldMan admin user is created
        strQuery = "SELECT count(*) FROM ym_users";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        clQuery.first();
        if(clQuery.value(0).toInt() == 0)
        {
            // Then create the first user as root
            AdminUser *pUser = new AdminUser;
            pUser->m_nUserId = 1;
            pUser->m_strLogin = "admin";
            pUser->m_strName = "Yield-Man Server Administrator";
            pUser->m_strEmail = "";
            pUser->m_nGroupId = 0;
            pUser->m_strPwd = "ymadmin";
            pUser->m_nType = YIELDMANDB_USERTYPE_MASTER_ADMIN;
            pUser->m_nProfileId = 0;

            if(mDirAccessPlugin && users && groups && applications)
            {
                // Connect 'admin'
                // 'ymadmin'
                QMap<QString,QString> lConParam;
                lConParam.insert("dir_user","admin");
                lConParam.insert("dir_pass","ymadmin");

                if (!mDirAccessPlugin->GetConnector()->Connect(lConParam))
                {
                    QString strMessage;
                    GetLastError(strMessage);
                    GSLOG(SYSLOG_SEV_EMERGENCY, QString("Cannot save the root user: %1")
                          .arg(strMessage)
                          .toLatin1().constData());
                    return false;
                }

                if(!users->Add(pUser->m_strLogin, pUser->m_strPwd))
                    qDebug() << users->GetLastError();
                // Update pwd if user already exists
                users->UpdateUserAttribute(pUser->m_strLogin, "password", pUser->m_strPwd);
                users->UpdateUserAttribute(pUser->m_strLogin, "email", pUser->m_strEmail);
                users->UpdateUserAttribute(pUser->m_strLogin, "name", pUser->m_strName);
                users->UpdateUserAttribute(pUser->m_strLogin, "creation_date", GetServerDateTime().toString("yyyy-MM-dd HH:mm:ss"));

                // Add users to group
                groups->AddUser("public",pUser->m_strLogin);

                applications->AddUserPrivilege("galaxy",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:users_groups_administrator",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:examinator",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:examinator:administrator",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:examinator:databases",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:yieldman",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:yieldman:administrator",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:patman",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);
                applications->AddUserPrivilege("galaxy:patman:administrator",pUser->m_strLogin,GS::DAPlugin::READACCESS|GS::DAPlugin::WRITEACCESS);

                if(!mDirAccessPlugin->SaveChanges())
                {
                    QString strMessage;
                    GetLastError(strMessage);
                    GSLOG(SYSLOG_SEV_EMERGENCY, QString("Cannot save the root user: %1")
                          .arg(strMessage)
                          .toLatin1().constData());
                    return false;
                }

                lConParam.insert("dir_user","anonymous");
                lConParam.insert("dir_pass","");

                mDirAccessPlugin->GetConnector()->Connect(lConParam);

            }

            if(!SaveUser(pUser))
            {
                QString strMessage;
                GetLastError(strMessage);
                GSLOG(SYSLOG_SEV_EMERGENCY, QString("Cannot save the root user: %1")
                      .arg(strMessage)
                      .toLatin1().constData());
                return false;
            }
        }

        GSLOG(SYSLOG_SEV_DEBUG, QString("LoadUserList : total %1 to (re)load")
              .arg( clQuery.value(0).toString())
              .toLatin1().constData() );

        if(m_nServerBuild > 7)
            strQuery = "SELECT user_id, group_id, login, pwd, name, email, type, os_login, profile_id, creation_date, expiration_date, last_access, last_update FROM ym_users";
        else
            strQuery = "SELECT user_id, group_id, login, pwd, name, email, type, os_login, profile_id, creation_date, expiration_date, last_access FROM ym_users";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        QMap<QString,int> lMapLoginId; // for update from ym_users
        while(clQuery.next())
        {
            iIndex = 0;

            // Check if already in the list
            if(m_mapUsers.contains(clQuery.value(0).toInt()))
                pUser = m_mapUsers[clQuery.value(0).toInt()];
            else
                pUser = new AdminUser;

            pUser->m_nUserId = clQuery.value(iIndex++).toInt();
            pUser->m_nGroupId = clQuery.value(iIndex++).toInt();
            pUser->m_strLogin = clQuery.value(iIndex++).toString();
#ifdef QT_DEBUG
            // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
            // DO NOT ADD THIS GSLOG in RELEASE VERSION
            //GSLOG(SYSLOG_SEV_EMERGENCY, QString("Decrypting password for ym_users.user_id=%1, ym_users.login=%2 from string: %3").arg(QString::number(pUser->m_nUserId)).arg(pUser->m_strLogin).arg(clQuery.value(iIndex).toString()));
#endif
            GexDbPlugin_Base::DecryptPassword(clQuery.value(iIndex++).toString(),pUser->m_strPwd);
            pUser->m_strName = clQuery.value(iIndex++).toString();
            pUser->m_strEmail = clQuery.value(iIndex++).toString();
            pUser->m_nType = clQuery.value(iIndex++).toInt();
            pUser->m_strOsLogin = clQuery.value(iIndex++).toString();
            pUser->m_nProfileId = clQuery.value(iIndex++).toInt();
            pUser->m_clCreationDate = clQuery.value(iIndex++).toDateTime();
            pUser->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();
            pUser->m_clAccessDate = clQuery.value(iIndex++).toDateTime();
            if(m_nServerBuild > 7)
                pUser->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();

            m_mapUsers[pUser->m_nUserId] = pUser;

            lMapLoginId[pUser->m_strLogin.toLower()] = pUser->m_nUserId;

            // DA Galaxy
            if(users && users->Exists(pUser->m_strLogin))
            {
                bool lValid;
                pUser->m_strLogin = pUser->m_strLogin.toLower();
                pUser->m_strName = users->GetUserAttribute(pUser->m_strLogin,"name",lValid);
                pUser->m_strEmail = users->GetUserAttribute(pUser->m_strLogin,"email",lValid);
                // Check Admin
                bool bAdminUser = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser);
                if(bAdminUser)
                    pUser->m_nType = YIELDMANDB_USERTYPE_MASTER_ADMIN;
                else
                    pUser->m_nType = YIELDMANDB_USERTYPE_USER;
            }
        }

        // TRY TO OPEN A SESSION TO UPDATE THE XML ?
        /*
        if(users && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            bool ok;
            // Synchronize the 2 objects
            foreach(QString login, users->GetUsersList())
            {
                if(!lMapLoginId.contains(login))
                {
                    pUser = new AdminUser;
                    pUser->m_strName = users->GetUserAttribute(login,"name",ok);
                    pUser->m_strEmail = users->GetUserAttribute(login,"email",ok);
                    pUser->m_nType = YIELDMANDB_USERTYPE_USER;
                    pUser->m_strOsLogin = "";
                    pUser->m_nProfileId = 0;
                    pUser->m_strPwd = "INCOMPATIBLE WITH V7.0 - NEED V7.1 FOR UPDATE";
                    pUser->m_clCreationDate = QDateTime::fromString(users->GetUserAttribute(login,"creation_date",ok),"yyyy-MM-dd HH:mm:ss");
                    SaveUser(pUser);
               }
            }
        }
        */
    }


    if(bConnected)
    {
        /////////////////////
        // USER OPTIONS
        GSLOG(SYSLOG_SEV_DEBUG, "LoadUserList : Update users options");
        foreach(pUser, m_mapUsers)
        {
            if(!bUsersGroupsAdminConnected)
            {
                // User connected but not the master
                if(m_pCurrentUser->m_nUserId!=pUser->m_nUserId)
                    continue;
            }

            // Reset user options
            pUser->ResetAllAttributes();
            // Load all user options
            strQuery = "SELECT field, value FROM ym_users_options ";
            strQuery+= "WHERE user_id="+QString::number(pUser->m_nUserId);
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
            while(clQuery.next())
            {
                pUser->SetAttribute(clQuery.value(0).toString(),QVariant(clQuery.value(1).toString()));
            }
        }
        //emit sShowUserList();
    }

    return true;
}

bool AdminEngine::LoadDirectoryAccessPlugin()
{
    if(m_nServerBuild < 13)
        return true;

    // Do not allow Examinator-PRO 7.1 to load DA_GALAXY without the YM update
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && m_nServerBuild < 13)
        return true;

    if(!mDirAccessPlugin)
    {
        // get plugin dir
        QString lApplicationDir;
        CGexSystemUtils::GetApplicationDirectory(lApplicationDir);
        QDir lPluginsDir = QDir(lApplicationDir);
        lPluginsDir.cd("plugins/da");

        // get file name
        QString lPluginFileName;
#if defined QT_DEBUG
        lPluginFileName = "dagalaxyd.";
#else
        lPluginFileName = "dagalaxy.";
#endif

#if defined unix
        lPluginFileName.append("so");
        lPluginFileName.prepend("lib");
#elif defined __MACH__
        lPluginFileName.append("dylib");
        lPluginFileName.prepend("lib");
#else
        lPluginFileName.append("dll");
#endif

        if(lPluginFileName.isEmpty())
        {
            QString lMsg = "Directory Access Plugin not loaded";
            SetLastError(eDB_InvalidEntry, lMsg);
            return false;
        }

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Load plugin '" + lPluginsDir.absoluteFilePath(lPluginFileName) + "'...")
              .toLatin1().constData());

        QPluginLoader lPluginLoader(lPluginsDir.absoluteFilePath(lPluginFileName));
        QObject *lPlugin = lPluginLoader.instance();
        if (!lPluginLoader.isLoaded())
        {
            QString lMsg = "Directory Access Plugin not loaded: " + lPluginLoader.errorString();
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unable to load " + lPluginsDir.absoluteFilePath(lPluginFileName))
                  .toLatin1().constData());
            SetLastError(eDB_InvalidEntry, lMsg);
            return false;
        }

        if (lPlugin)
            mDirAccessPlugin = qobject_cast<GS::DAPlugin::DirAccessBase *>(lPlugin);
        else
            return false;

        GSLOG(SYSLOG_SEV_NOTICE,
              QString(mDirAccessPlugin->Name() + ", version " + mDirAccessPlugin->Version() + ": Loaded")
              .toLatin1().constData());

    }

    // Check if host already resolved and used with success
    if(m_pDatabaseConnector->m_strHost_LastUsed.isEmpty())
    {
        QString lMsg = "Unable to connect: no valid host found";
        SetLastError(eDB_InvalidEntry, lMsg);
        return false;
    }

    // auto connect as anonymous
    if (mDirAccessPlugin
            && mDirAccessPlugin->GetConnector()
            && !mDirAccessPlugin->GetConnector()->IsConnected())
    {
        // Make sure to close the connection
        mDirAccessPlugin->GetConnector()->Disconnect();

        QMap<QString,QString> lConParam;
        lConParam.insert("dir_user","anonymous");
        lConParam.insert("dir_pass","");

        lConParam.insert("sql_host",m_pDatabaseConnector->m_strHost_LastUsed);
        lConParam.insert("sql_port",QString::number(m_pDatabaseConnector->m_uiPort));
        lConParam.insert("sql_user",m_pDatabaseConnector->m_strUserName_Admin);
        lConParam.insert("sql_pass",m_pDatabaseConnector->m_strPassword_Admin);
        lConParam.insert("sql_database_sid",m_pDatabaseConnector->m_strDatabaseName);
        lConParam.insert("sql_shema",m_pDatabaseConnector->m_strSchemaName);
        lConParam.insert("sql_driver",m_pDatabaseConnector->m_strDriver);
        // Share the connection with the plugin (use only one instead of 2)
        // lConParam.insert("sql_connection_id",m_pDatabaseConnector->m_strConnectionName);

        // Open anonymous connection
        if (!mDirAccessPlugin->GetConnector()->Connect(lConParam))
        {
            QString lMsg = "Unable to connect: " +
                    mDirAccessPlugin->GetConnector()->GetLastError();
            SetLastError(eDB_InvalidEntry, lMsg);
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::SaveGroup(AdminUserGroup* /*pGroup*/)
{

    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::SaveUser(AdminUser* pUser)
{

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    AdminUser* pUserToSave = pUser;
    if(pUserToSave == NULL)
        pUserToSave = m_pCurrentUser;

    if(pUserToSave == NULL)
    {
        // Invalid user
        SetLastError(eDB_InvalidEntry, "Invalid user");
        return false;
    }

    QString strPwd;
    GexDbPlugin_Base::CryptPassword(pUserToSave->m_strPwd,strPwd);

    bool        bUpdateUser = false;
    bool        bUpdateDirectAccess = false;
    bool        bCreateUser = false;
    QString     strQuery;
    QSqlQuery   clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strQuery = "SELECT count(*) FROM ym_users ";
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }

    if(clQuery.first() && (clQuery.value(0).toInt() == 0))
    {
        bUpdateUser = bCreateUser = true;
    }
    else
    {
        // When user exists, check some DA GALAXY field update
        strQuery = "SELECT login, pwd, name, email, os_login, profile_id FROM ym_users WHERE user_id=" + QString::number(pUserToSave->m_nUserId);
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if(clQuery.first())
        {
            if(pUserToSave->m_strName != clQuery.value(2).toString()) bUpdateDirectAccess=true;
            if(pUserToSave->m_strEmail != clQuery.value(3).toString()) bUpdateDirectAccess=true;

            bUpdateUser = bUpdateDirectAccess;
            QString lPwd;
            GexDbPlugin_Base::DecryptPassword(clQuery.value(1).toString(),lPwd);

            if(pUserToSave->m_strPwd != lPwd) bUpdateUser=true;
            if(pUserToSave->m_strLogin != clQuery.value(0).toString()) bUpdateUser=true;
            if(pUserToSave->m_strOsLogin != clQuery.value(4).toString()) bUpdateUser=true;
            if(pUserToSave->m_nProfileId != clQuery.value(5).toInt()) bUpdateUser=true;
        }
        else
            bCreateUser = bUpdateUser = true;
    }


    if(bCreateUser)
    {
        // have to create and update this new user
        QTime clTime = QTime::currentTime();
        if(m_pDatabaseConnector->IsOracleDB())
        {
            QString strSqlDate = "SYSDATE";
            if(pUserToSave->m_clCreationDate.isValid())
                strSqlDate = NormalizeDateToSql(pUserToSave->m_clCreationDate);

            strQuery = "INSERT INTO ym_users(user_id,login,name,type,creation_date) VALUES(";
            strQuery+= "ym_users_sequence.nextval,'"+pUserToSave->m_strLogin+"',";
            strQuery+= "'"+clTime.toString("hhmmsszz")+"',"+QString::number(pUserToSave->m_nType)+","+strSqlDate+")";
        }
        else
        {
            QString strSqlDate = "NOW()";
            if(pUserToSave->m_clCreationDate.isValid())
                strSqlDate = NormalizeDateToSql(pUserToSave->m_clCreationDate);
            strQuery = "INSERT INTO ym_users(login,name,type,creation_date) VALUES(";
            strQuery+= "'"+pUserToSave->m_strLogin+"','"+clTime.toString("hhmmsszz")+"',"+QString::number(pUserToSave->m_nType)+","+strSqlDate+")";
        }
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        // Retreve the GroupId
        strQuery = "SELECT user_id, creation_date FROM ym_users WHERE name='"+clTime.toString("hhmmsszz")+"'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        clQuery.first();
        pUserToSave->m_nUserId = clQuery.value(0).toInt();
        pUserToSave->m_clCreationDate = clQuery.value(1).toDateTime();

    }

#ifdef QT_DEBUG
    // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
    // DO NOT ADD THIS GSLOG in RELEASE VERSION
#ifdef CASE4882
    GSLOG(SYSLOG_SEV_EMERGENCY,
          QString("Crypting password %1 for YM user %2 before updating YMDB (ym_users)")
          .arg(pUserToSave->m_strPwd)
          .arg(pUserToSave->m_strName)
          .toLatin1().constData());
#endif
#endif

    if(bUpdateUser)
    {

        // Then Save current pUserToSave
        strQuery = "UPDATE ym_users SET ";

        strQuery+= " login="+NormalizeStringToSql(pUserToSave->m_strLogin)+"";
        // PWD must be compatible with V7.0 and V7.1
        // but use DA XML when it is present
        if(!strPwd.isEmpty())
            strQuery+= " , pwd='"+strPwd+"'";

        // No group id
        strQuery+= " , group_id=" + QString(m_pDatabaseConnector->IsOracleDB() ? "''" : "null");

        strQuery+= " , name="+NormalizeStringToSql(pUserToSave->m_strName)+"";
        strQuery+= " , email="+NormalizeStringToSql(pUserToSave->m_strEmail)+"";

        // with DA Galaxy
        // privileges are saved into the XML
        // Force ym_users.type to standard user so admin must use a V7.1 to connect as admin
        if(!mDirAccessPlugin)
            strQuery+= " , type="+QString::number(pUserToSave->m_nType);
        else
            strQuery+= " , type="+QString::number(YIELDMANDB_USERTYPE_USER);
        if(m_pCurrentUser == pUserToSave)
            strQuery+= ", os_login="+NormalizeStringToSql(m_strOsLogin)+"";

        // Save profile whit Examinator-PRO even if user is admin
        //if(pUserToSave->m_nType < YIELDMANDB_USERTYPE_MASTER_ADMIN)
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                && (pUserToSave->m_nProfileId>0))
            strQuery+= ", profile_id="+QString::number(pUserToSave->m_nProfileId);

        if(m_pDatabaseConnector->IsOracleDB())
            strQuery+= ", last_update=SYSDATE";
        else
            strQuery+= ", last_update=NOW()";
        strQuery+= ", mutex=null";
        strQuery+= " WHERE user_id="+QString::number(pUserToSave->m_nUserId);
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            clQuery.exec("COMMIT"); // To unlock any tables
            return false;
        }
        clQuery.exec("COMMIT"); // To unlock any tables

        // Select new update time
        strQuery = "SELECT last_update FROM  ym_users";
        strQuery+= " WHERE user_id="+QString::number(pUserToSave->m_nUserId);
        if(clQuery.exec(strQuery) && clQuery.first())
            pUserToSave->m_clUpdateDate = clQuery.value(0).toDateTime();
    }


    // Delete user options
    strQuery = "DELETE FROM ym_users_options WHERE user_id="+QString::number(pUserToSave->m_nUserId);
    clQuery.exec(strQuery);

    //////////////////////
    // Save user options
    if(!pUserToSave->GetAttributes().isEmpty())
    {
        // Save attributes
        QMap<QString, QVariant> mapAttributes = pUserToSave->GetAttributes();
        foreach(const QString& key, mapAttributes.keys())
        {
            strQuery = "INSERT INTO ym_users_options (user_id,field,value) VALUES("
                    + QString::number(pUserToSave->m_nUserId)
                    + ","+NormalizeStringToSql(key).toUpper()+","
                    + NormalizeStringToSql(mapAttributes.value(key).toString()) + ")";
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
    }

    clQuery.exec("COMMIT");

    if(!m_mapUsers.contains(pUserToSave->m_nUserId))
        m_mapUsers[pUserToSave->m_nUserId] = pUserToSave;

    if(bUpdateDirectAccess
            && mDirAccessPlugin
            && mDirAccessPlugin->GetUsers())
    {
        GS::DAPlugin::UsersBase * users = mDirAccessPlugin->GetUsers();
        users->UpdateUserAttribute(pUserToSave->m_strLogin, "email", pUserToSave->m_strEmail);
        users->UpdateUserAttribute(pUserToSave->m_strLogin, "name", pUserToSave->m_strName);
        mDirAccessPlugin->SaveChanges();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::SaveProfile(AdminUserProfile* pProfile)
{

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    if(pProfile == NULL)
    {
        // Invalid Profile
        SetLastError(eDB_InvalidEntry, "Invalid Profile");
        return false;
    }

    // the admin User must exist
    if(!m_mapUsers.contains(pProfile->m_nUserId))
    {
        // Invalid user
        SetLastError(eDB_InvalidEntry, "Invalid user");
        return false;
    }

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strQuery = "SELECT * FROM ym_users_profiles WHERE profile_id=" + QString::number(pProfile->m_nProfileId);
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }

    if(!clQuery.first())
    {
        // have to create and update this new Profile
        QTime clTime = QTime::currentTime();
        if(m_pDatabaseConnector->IsOracleDB())
        {
            strQuery = "INSERT INTO ym_users_profiles(profile_id,user_id,name,creation_date) VALUES(";
            strQuery+="ym_profiles_sequence.nextval,";
            strQuery+= ""+QString::number(pProfile->m_nUserId)+",'"+clTime.toString("hhmmsszz")+"',SYSDATE)";
        }
        else
        {
            strQuery = "INSERT INTO ym_users_profiles(user_id,name,creation_date) VALUES(";
            strQuery+= ""+QString::number(pProfile->m_nUserId)+",'"+clTime.toString("hhmmsszz")+"',NOW())";
        }
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        // Retreve the ProfileId
        strQuery = "SELECT profile_id FROM ym_users_profiles WHERE name='"+clTime.toString("hhmmsszz")+"'";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if(!clQuery.first())
        {
            SetLastError(clQuery);
            return false;
        }
        pProfile->m_nProfileId = clQuery.value(0).toInt();

    }

    // Then Save current pProfile
    strQuery = "UPDATE ym_users_profiles SET ";
    strQuery+= " user_id="+QString::number(pProfile->m_nUserId)+", ";
    strQuery+= " name="+NormalizeStringToSql(pProfile->m_strName)+", ";
    strQuery+= " description="+NormalizeStringToSql(pProfile->m_strDescription)+", ";
    strQuery+= " os_login='"+pProfile->m_strOsLogin+"', ";
    strQuery+= " permisions="+QString::number(pProfile->m_nPermissions)+", ";
    strQuery+= " script_name="+NormalizeStringToSql(pProfile->m_strScriptName)+", ";
    strQuery+= " script_content="+NormalizeScriptStringToSql(pProfile->m_strScriptContent)+", ";
    QString strSqlDate = NormalizeDateToSql(pProfile->m_clUpdateDate);
    strQuery+= " last_update=" + strSqlDate;
    strQuery+= " WHERE profile_id="+QString::number(pProfile->m_nProfileId);
    if(!clQuery.exec(strQuery))
    {
        //WriteDebugMessageFile(strQuery);
        //WriteDebugMessageFile(clQuery.lastError().text());
        SetLastError(clQuery);
        return false;
    }

    clQuery.exec("COMMIT");

    if(!m_mapProfiles.contains(pProfile->m_nProfileId))
        m_mapProfiles[pProfile->m_nProfileId] = pProfile;

    return true;
}

//////////////////////////////////////////////////////////////////////
bool AdminEngine::DeleteProfile(AdminUserProfile* pProfile)
{

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    if(pProfile == NULL)
    {
        // Invalid Profile
        SetLastError(eDB_InvalidEntry, "Invalid Profile");
        return false;
    }

    // the admin User must exist
    if(!m_mapUsers.contains(pProfile->m_nUserId))
    {
        // Invalid user
        SetLastError(eDB_InvalidEntry, "Invalid user");
        return false;
    }

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Then Save current pProfile
    strQuery = "DELETE FROM ym_users_profiles ";
    strQuery+= " WHERE profile_id="+QString::number(pProfile->m_nProfileId);
    if(!clQuery.exec(strQuery))
    {
        //WriteDebugMessageFile(strQuery);
        //WriteDebugMessageFile(clQuery.lastError().text());
        SetLastError(clQuery);
        return false;
    }

    clQuery.exec("COMMIT");

    if(m_mapProfiles.contains(pProfile->m_nProfileId))
        m_mapProfiles.remove(pProfile->m_nProfileId);

    delete pProfile;

    return true;
}

bool AdminEngine::IsUserAdmin(AdminUser *pUser)
{
    if(!pUser)
        return false;

    bool lIsMaster = false;
    // Use DA to know if the user is Admin
    if(mDirAccessPlugin)
    {
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();
        if(applications)
        {
            if((GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
                    && applications->IsAllowedTo("galaxy:examinator:administrator",pUser->m_strLogin,GS::DAPlugin::WRITEACCESS))
                lIsMaster = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan()
                    && applications->IsAllowedTo("galaxy:yieldman:administrator",pUser->m_strLogin,GS::DAPlugin::WRITEACCESS))
                lIsMaster = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan()
                    && applications->IsAllowedTo("galaxy:patman:administrator",pUser->m_strLogin,GS::DAPlugin::WRITEACCESS))
                lIsMaster = true;
        }
    }
    else if(pUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        lIsMaster = true;

    return lIsMaster;
}

bool AdminEngine::IsUserAdmin(bool bWithReadOnly)
{
    AdminUser *lUser = m_pCurrentUser;
    if(!lUser)
        return false;


    bool lIsMaster = false;
    int lPrivileges = 1;
    if(!bWithReadOnly)
        lPrivileges = 1|2;

    // Use DA to know if the user is Admin
    if(mDirAccessPlugin)
    {
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();
        if(applications)
        {
            if((GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
                    && applications->IsAllowedTo("galaxy:examinator:administrator",lUser->m_strLogin,lPrivileges))
                lIsMaster = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan()
                    && applications->IsAllowedTo("galaxy:yieldman:administrator",lUser->m_strLogin,lPrivileges))
                lIsMaster = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan()
                    && applications->IsAllowedTo("galaxy:patman:administrator",lUser->m_strLogin,lPrivileges))
                lIsMaster = true;
        }
    }
    else if(lUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        lIsMaster = true;

    return lIsMaster;
}

bool AdminEngine::HasUserGroupAdminPrivileges(bool bWithReadOnly)
{
    AdminUser *lUser = m_pCurrentUser;
    if(!lUser)
        return false;

    bool lIsMaster = false;
    int lPrivileges = 1;
    if(!bWithReadOnly)
        lPrivileges = 1|2;

    // Use DA to know if the user is Admin
    if(mDirAccessPlugin)
    {
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();
        if(applications)
        {
            if(applications->IsAllowedTo("galaxy:users_groups_administrator",lUser->m_strLogin,lPrivileges))
                lIsMaster = true;
        }
    }
    else if(lUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
        lIsMaster = true;

    return lIsMaster;
}

//////////////////////////////////////////////////////////////////////
// Check if a user is SQL connected (through the ym_admin_db link)
// a user mutex = CONNECTION_ID():HOST(:SOCKET)
// and for multi-connection (only for users)
// ym_users = mutex(|mutex)* (ie CONNECTION_ID_1:HOST_1|CONNECTION_ID_2:HOST_2)
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsUserConnected(AdminUser  *pUser)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return true;

    if(!ConnectToAdminServer())
        return false;

    if(pUser == NULL)
        return false;

    QString        lQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    // To know if the user is connected
    // Check the validity ym_users.mutex
    // If empty = not connected
    // If valid = connected

    // Update the attribute 'USER_ACTIVE_SESSIONS' for multi-connection
    QString lActiveSessions;

    lQuery = "SELECT mutex FROM ym_users WHERE user_id="+QString::number(pUser->m_nUserId);
    lQuery+= " AND NOT(mutex IS NULL OR mutex='')";
    if(!clQuery.exec(lQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if(clQuery.first())
    {
        // User seems to be connected
        QStringList lMutexList = clQuery.value(0).toString().split("|",QString::SkipEmptyParts);
        QString lMutex;
        while(!lMutexList.isEmpty())
        {
            lMutex = lMutexList.takeFirst();
            // Check the validity of the mutex
            if(IsMutexActive(lMutex))
            {
                // User already connected
                if(!lActiveSessions.isEmpty())
                    lActiveSessions += "|";
                lActiveSessions += lMutex;
            }
        }
    }
    pUser->SetAttribute("USER_ACTIVE_SESSIONS",lActiveSessions);

    return !lActiveSessions.isEmpty();
}

//////////////////////////////////////////////////////////////////////
// Check if the current object user is valid
// Check if the current object user is admin
// Check if the DirAccess is connected not with 'anonymous'
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsUserConnected(bool asAdmin)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return true;

    // Must be connected into GEX
    if(m_pCurrentUser == NULL)
        return false;
    // Must be connected into ym_admin_db
    if(!m_pDatabaseConnector->IsConnected())
        return false;

    // Must be connected into DA
    if(mDirAccessPlugin && !mDirAccessPlugin->GetConnector()->IsConnected())
        return false;

    if(mDirAccessPlugin && mDirAccessPlugin->GetCurrentUser() == "anonymous")
        return false;

    if(asAdmin)
        return IsUserAdmin();

    return true;
}

//////////////////////////////////////////////////////////////////////
// User request connection
// pwd managed through DirAccess
// Check for multi-connection
//////////////////////////////////////////////////////////////////////
bool AdminEngine::ConnectUser(int nUserId, QString strPwd, bool bAsAdmin)
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    AdminUser  *pUser=0;

    if(!m_mapUsers.contains(nUserId))
    {
        // invalid user
        SetLastError(eDB_InvalidEntry, "User doesn't exist");
        return false;
    }

    pUser = m_mapUsers[nUserId];

    if(bAsAdmin && !IsUserAdmin(pUser))
    {
        // Invalid passWord
        SetLastError(eDB_InvalidEntry, "Connection allowed only with the Admin YieldManDb account");
        return false;
    }

    // Check if have some Pwd
    if(strPwd.isEmpty())
    {
        // Invalid passWord
        SetLastError(eDB_InvalidEntry, "Invalid password");
        return false;
    }

    if(!mDirAccessPlugin)
    {
        // Check if have some Pwd
        if(strPwd.isEmpty())
        {
            // Invalid passWord
            SetLastError(eDB_InvalidEntry, "Invalid password");
            return false;
        }
        if(!pUser->m_strPwd.isEmpty() && (pUser->m_strPwd != strPwd))
        {
            // Invalid passWord
            SetLastError(eDB_InvalidEntry, "Invalid password");
            return false;
        }
    }

    QString        lQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    GSLOG(SYSLOG_SEV_DEBUG, "ConnectUser to the ym_admin_db");
    bool  bAllowMultiConnection = false;
    // ADMIN multi connection for YieldMan only
    // Allow ADMIN multi connection only on the same machine
    // YieldMan is started => other PC or other ServerProfile
    // If ADMIN first connect to YieldMan => ExaminatorPRO rejects the new connection
    // If ADMIN first connect to ExaminatorPRO => YieldMan allows this new connection and ExaminatorPRO still running
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() && IsUserAdmin(pUser))
        bAllowMultiConnection = true;

    // Check if the user isn't already connected
    QStringList lActiveSessions;
    if(IsUserConnected(pUser))
    {
        // User already connected
        bool    bAcceptConnection = bAllowMultiConnection;
        QString lMutexId;
        QString lMachine;

        lActiveSessions = pUser->GetAttribute("USER_ACTIVE_SESSIONS").toString().split("|",QString::SkipEmptyParts);

        while(!lActiveSessions.isEmpty())
        {
            lMutexId = lActiveSessions.takeFirst();
            lMachine = lMutexId.section(":",1,1);
            if(lMachine == "localhost")
            {
                lMachine = lMutexId.section(":",2,2);
                if(lMachine.isEmpty())
                    lMachine = m_pDatabaseConnector->m_strHost_Name;
            }
            // check if User HostName is the same as MACHINE
            if(bAllowMultiConnection && (lMachine.toLower() != m_strHostName.toLower()))
                bAcceptConnection = false;
        }

        if(!bAcceptConnection)
        {
            GSLOG(SYSLOG_SEV_DEBUG, "ConnectUser: Check if the user is already connected");
            SetLastError(eDB_InvalidEntry, QString("User already connected on "+lMachine).toLatin1().data());
            return false;
        }
    }

    // Then allow this connection
    // First disconnect the user to YieldManDb
    DisconnectCurrentUser();
    // Load users privileges list from da_galaxy
    if(mDirAccessPlugin && mDirAccessPlugin->GetConnector())
    {
        QMap<QString,QString> lConParam;
        lConParam.insert("dir_user",pUser->m_strLogin);
        lConParam.insert("dir_pass",strPwd);
        if (!mDirAccessPlugin->GetConnector()->ChangeUser(lConParam))
        {
            SetLastError(eDB_InvalidEntry, mDirAccessPlugin->GetConnector()->GetLastError().toLatin1().data());
            return false;
        }

        // Check Admin
        bool bAdminUser = IsUserAdmin(pUser);
        if(bAdminUser)
            pUser->m_nType = YIELDMANDB_USERTYPE_MASTER_ADMIN;
        else
            pUser->m_nType = YIELDMANDB_USERTYPE_USER;
    }

    // Connected as user
    QString strCryptedPwd;
#ifdef QT_DEBUG
#ifdef CASE4882
    // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
    // DO NOT ADD THIS GSLOG in RELEASE VERSION
    GSLOG(SYSLOG_SEV_EMERGENCY,
          QString("Crypting password %1 for YM user %2 before updating YMDB (ym_users)")
          .arg(strPwd)
          .arg(pUser->m_strName)
          .toLatin1().constData());
#endif
#endif

    lActiveSessions << mMutex;
    GexDbPlugin_Base::CryptPassword(strPwd,strCryptedPwd);  // Update the YieldManDb
    lQuery = "UPDATE ym_users SET";
    if(pUser->m_strPwd.isEmpty())
    {
        // Save the new passWord
        pUser->m_strPwd = strPwd;
        lQuery+= " pwd='"+strCryptedPwd + "', ";
    }
    lQuery+= " os_login="+NormalizeStringToSql(m_strOsLogin)+", ";
    lQuery+= " profile_id="+QString::number(pUser->m_nProfileId);
    if(m_nServerBuild > 7)
    {
        lQuery+= ", ";
        lQuery+= " mutex='"+lActiveSessions.join("|")+"', ";
        if(m_pDatabaseConnector->IsOracleDB())
            lQuery+= " last_access=SYSDATE";
        else
            lQuery+= " last_access=NOW()";
    }
    lQuery+= " WHERE user_id="+QString::number(nUserId);
    if(!clQuery.exec(lQuery))
    {
        SetLastError(clQuery);
        clQuery.exec("COMMIT"); // To unlock any tables
        return false;
    }

    clQuery.exec("COMMIT");

    m_pCurrentUser = m_mapUsers[nUserId];
    m_pCurrentGroup = NULL;
    m_pCurrentProfile = NULL;

    if(m_mapGroups.contains(m_pCurrentUser->m_nGroupId))
    {
        m_pCurrentGroup = m_mapGroups[m_pCurrentUser->m_nGroupId];
    }


    // Update the Events flow
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        AddNewEvent("CONNECTION","USER",
                    "LOGIN","User is connected|Login="+m_pCurrentUser->m_strLogin,"");



    if (m_pCurrentUser && pGexScriptEngine)
    {
        QScriptValue lSv = pGexScriptEngine->newQObject((QObject*) m_pCurrentUser);
        if (!lSv.isNull())
            pGexScriptEngine->globalObject().setProperty("GSCurrentUser", lSv);

        QFile scriptFile( GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()+"/scripts/on_login.js" );
        if (scriptFile.exists())
        {
            if (scriptFile.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&scriptFile);
                QString contents = stream.readAll();
                if (!contents.isEmpty())
                {
                    scriptFile.close();
                    QScriptValue v=pGexScriptEngine->evaluate(contents);
                    GSLOG(SYSLOG_SEV_NOTICE,
                          QString("Execution of on_login.js script returned : %1").arg( v.toString())
                          .toLatin1().constData() );
                    // GCORE-499
                    pGexScriptEngine->collectGarbage();
                }
            }
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Connected user %1")
          .arg(m_pCurrentUser->m_strLogin)
          .toLatin1().data() );

    // Enabled or Disabled some access
    emit sEnableGexAccess();

    // To change the level of a module : GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_LOGLEVEL %1").arg( 3));
    LoadServerOptions();
    // Synchronize with YieldManDB profiles
    //QString strNoProfile;
    //SynchronizeProfiles(strNoProfile);

    return true;

}

bool AdminEngine::DisconnectCurrentUser()
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(m_pCurrentUser == NULL)
        return true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Disconnect Current User %1")
          .arg(m_pCurrentUser->m_strLogin)
          .toLatin1().data() );

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strQuery = "UPDATE ym_users SET";
    strQuery+= " mutex=(CASE WHEN mutex='"+mMutex+"' THEN NULL ELSE REPLACE(mutex,'"+mMutex+"','') END)";
    strQuery+= " WHERE user_id="+QString::number(m_pCurrentUser->m_nUserId);
    clQuery.exec(strQuery);
    clQuery.exec("COMMIT"); // To unlock any tables

    // Switch to DA anonymous
    QMap<QString,QString> lConParam;
    lConParam.insert("dir_user","anonymous");
    lConParam.insert("dir_pass","");

    if(mDirAccessPlugin && mDirAccessPlugin->GetConnector())
        mDirAccessPlugin->GetConnector()->ChangeUser(lConParam);

    // Reset LogLevel from user
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        m_mapLogLevel.clear();

    if (pGexScriptEngine)
    {
        //QScriptValue lSv = pGexScriptEngine->newQObject((QObject*) m_pCurrentUser);
        pGexScriptEngine->globalObject().setProperty("GSCurrentUser", QScriptValue());
    }

    // Update the Events flow
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        AddNewEvent("CONNECTION","USER",
                    "LOGOUT","User is disconnected|Login="+m_pCurrentUser->m_strLogin,"");

    m_pCurrentUser = NULL;
    m_pCurrentGroup = NULL;
    m_pCurrentProfile = NULL;

    // Enabled or Disabled some access
    emit sEnableGexAccess();
    // To change the level of a module : GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_LOGLEVEL %1").arg( 3));
    LoadServerOptions();

    return true;

}

QString AdminEngine::GetSettingsValue(const QString &OptionName)
{
    QString lValue;
    if (GetSettingsValue(OptionName, lValue))
        return lValue;
    return "";
}

/******************************************************************************!
 * \fn GetSettingsValue
 * \return false on error
 ******************************************************************************/
bool AdminEngine::GetSettingsValue(const QString& strOptionName,
                                   QString& strOptionValue)
{
    if (! IsSettingName(strOptionName))
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Get custom option value for %1").arg(strOptionName).
              toLatin1().constData());
    }

    QString strQuery;
    QSqlQuery clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                                 m_strConnectionName));

    strOptionValue = "";

    strQuery = "SELECT value FROM ym_settings ";
    strQuery +=
        " WHERE field=" + NormalizeStringToSql(strOptionName.toUpper()) + " ";
    if (! clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    if (clNodeQuery.first())
    {
        strOptionValue = clNodeQuery.value(0).toString().trimmed();
        return true;
    }

    return false;
}

bool AdminEngine::SetSettingsValue(const QString &strSettingName, QString &strSettingValue)
{
    if(!IsSettingName(strSettingName))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Set a custom option key: %1=%2").arg(strSettingName).arg(strSettingValue).toLatin1().constData());
    }
    else if(!IsValidSettingValue(strSettingName,strSettingValue))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Set an option key with an invalid value: %1=%2. Allowed value is %3")
              .arg(strSettingName)
              .arg(strSettingValue)
              .arg(GetNodeSettingType(strSettingName)).toLatin1().constData());
    }

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // Delete entry for current node_id
    strQuery = "DELETE FROM ym_settings ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strSettingName.toUpper()) + " ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }


    strQuery = "INSERT INTO ym_settings ";
    strQuery+= " VALUE(" + NormalizeStringToSql(strSettingName.toUpper()) + "," + NormalizeStringToSql(strSettingValue) + ")";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }

    clNodeQuery.exec("COMMIT");
    return true;
}

bool AdminEngine::GetNodeSettingsValues(QMap<QString,QString>& mapSettings, bool ForAll)
{
    if(m_nNodeId < 0)
        return false;

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strQuery = "SELECT field,value FROM ym_nodes_options ";
    if(ForAll)
        strQuery+= " WHERE (node_id IS NULL OR node_id=0) ";
    else
        strQuery+= " WHERE (node_id = " + QString::number(m_nNodeId) + ") ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    while(clNodeQuery.next())
        mapSettings[clNodeQuery.value(0).toString().trimmed()] = clNodeQuery.value(1).toString().trimmed();

    return true;
}

QVariant AdminEngine::GetNodeSetting(const QString &lOptionName)
{
    QString lValue;
    bool lRet=GetNodeSettingsValue(lOptionName, lValue);
    if (!lRet)
        return QVariant();
    return QVariant(lValue);
}

bool AdminEngine::GetNodeSettingsValue(const QString &strOptionName, QString &strOptionValue)
{
    if(m_nNodeId < 0)
        return false;

    if(!IsNodeSettingName(strOptionName))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Get custom option value for %1").arg(strOptionName).toLatin1().constData());
    }

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strOptionValue = "";

    strQuery = "SELECT value FROM ym_nodes_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    strQuery+= " AND (node_id = " + QString::number(m_nNodeId) + ") ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    if(clNodeQuery.first())
    {
        strOptionValue = clNodeQuery.value(0).toString().trimmed();
        return true;
    }
    strQuery = "SELECT value FROM ym_nodes_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    strQuery+= " AND (node_id IS NULL OR node_id=0) ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    if(clNodeQuery.first())
    {
        strOptionValue = clNodeQuery.value(0).toString().trimmed();
        return true;
    }
    return false;
}

bool AdminEngine::SetNodeSettingsValue(const QString &strOptionName, const QString &strOptionValue, bool ForAll)
{
    if(m_nNodeId < 0)
        return false;

    if(!IsNodeSettingName(strOptionName))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Set a custom option key: %1=%2").arg(strOptionName).arg(strOptionValue).toLatin1().constData());
    }
    else if(!IsValidNodeSettingValue(strOptionName,strOptionValue))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Set an option key with an invalid value: %1=%2. Allowed value is %3")
              .arg(strOptionName)
              .arg(strOptionValue)
              .arg(GetNodeSettingType(strOptionName)).toLatin1().constData());
    }

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // if ForAll is true
    // Apply this option for all users
    // Delete entry for current node_id
    strQuery = "DELETE FROM ym_nodes_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    if(!ForAll)
        strQuery+= " AND (node_id = " + QString::number(m_nNodeId) + ") ";
    else
        strQuery+= " AND (node_id IS NULL OR node_id=0) ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }


    strQuery = "INSERT INTO ym_nodes_options (node_id,field,value) ";
    strQuery+= " VALUE(";
    if(ForAll)
        strQuery+= "0,";
    else
        strQuery+= QString::number(m_nNodeId) + ",";
    strQuery+=NormalizeStringToSql(strOptionName.toUpper()) + "," + NormalizeStringToSql(strOptionValue) + ")";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }

    clNodeQuery.exec("COMMIT");
    return true;
}

QStringList AdminEngine::GetNodeSettingsName()
{
    return GenericGetSettingsName(sNodeSettingsString);
}

QString     AdminEngine::GetNodeSettingType(QString optionName)
{
    return GenericGetSettingAttribute(sNodeSettingsString,"TYPE",optionName);
}

bool        AdminEngine::IsNodeSettingName(QString optionName)
{
    return GenericIsSettingName(sNodeSettingsString,optionName);
}

bool        AdminEngine::IsValidNodeSettingValue(QString optionName, QString optionValue)
{
    return GenericIsValidSettingValue(sNodeSettingsString,optionName,optionValue);
}

QStringList AdminEngine::GetSettingsName()
{
    return GenericGetSettingsName(sSettingsString);
}

QString     AdminEngine::GetSettingType(QString optionName)
{
    return GenericGetSettingAttribute(sSettingsString,"TYPE",optionName);
}

/******************************************************************************!
 * \fn IsSettingName
 ******************************************************************************/
bool AdminEngine::IsSettingName(QString optionName)
{
    return GenericIsSettingName(sSettingsString, optionName);
}

bool        AdminEngine::IsValidSettingValue(QString optionName, QString optionValue)
{
    return GenericIsValidSettingValue(sSettingsString,optionName,optionValue);
}

QStringList AdminEngine::GetUserSettingsName()
{
    return GenericGetSettingsName(sUserSettingsString);
}

QString     AdminEngine::GetUserSettingType(QString optionName)
{
    return GenericGetSettingAttribute(sUserSettingsString,"TYPE",optionName);
}

QString     AdminEngine::GetUserSettingDescription(QString optionName)
{
    return GenericGetSettingAttribute(sUserSettingsString,"DESCRIPTION",optionName);
}

QString     AdminEngine::GetUserSettingDefaultValue(QString optionName)
{
    return GenericGetSettingAttribute(sUserSettingsString,"DEFAULT",optionName);
}

bool        AdminEngine::IsUserSettingName(QString optionName)
{
    return GenericIsSettingName(sUserSettingsString,optionName);
}

bool        AdminEngine::IsValidUserSettingValue(QString optionName, QString optionValue)
{
    return GenericIsValidSettingValue(sUserSettingsString,optionName,optionValue);
}

/******************************************************************************!
 * \fn GenericGetSettingsName
 ******************************************************************************/
QStringList AdminEngine::GenericGetSettingsName(const QStringList lSettingsList)
{
    QStringList lstSettings;
    foreach(QString optionDescription, lSettingsList)
    {
        lstSettings << optionDescription.section("#", 0, 0).simplified();
    }
    return lstSettings;
}

QString AdminEngine::GenericGetSettingAttribute(const QStringList lSettingsList,
                                                QString attributeName,
                                                QString optionName)
{
    QString lAttributeValue = "";

    if(optionName.isEmpty())
        return lAttributeValue;

    QString lAttributeFlag = QString("#%1=").arg(attributeName.toUpper());
    foreach(QString optionDescription, lSettingsList)
    {
        if(optionDescription.startsWith(optionName,Qt::CaseInsensitive))
        {
            if(optionDescription.contains(lAttributeFlag))
                lAttributeValue = optionDescription.section(lAttributeFlag,1).remove(lAttributeFlag).section("#",0,0);
            return lAttributeValue;
        }
    }
    return lAttributeValue;
}

/******************************************************************************!
 * \fn GenericIsSettingName
 ******************************************************************************/
bool AdminEngine::GenericIsSettingName(const QStringList lSettingsList,
                                       QString optionName)
{
    return GenericGetSettingsName(lSettingsList).contains(optionName,
                                                          Qt::CaseInsensitive);
}

bool AdminEngine::GenericIsValidSettingValue(const QStringList lSettingsList,QString optionName, QString optionValue)
{
    bool lStatus = true;

    if(optionName.isEmpty())
        return lStatus;

    QString lType = GenericGetSettingAttribute(lSettingsList,"TYPE",optionName).toUpper();
    if(lType == "READONLY")
        lStatus = false;
    else if(lType == "NUMBER")
        optionValue.toFloat(&lStatus);
    else if(lType == "BOOLEAN")
    {
        QStringList lBoolean = QStringList()
                << "Y" << "N"
                << "YES" << "NO"
                << "TRUE"  << "FALSE"
                << "1" << "0";
        lStatus = lBoolean.contains(optionValue.simplified().toUpper());
    }
    else if(lType == "CHAR")
        lStatus = (optionValue.length()==1);
    else if(lType == "STRING")
        lStatus = true;
    else
    {
        // FOR ENUM
        if(lType.contains("|"))
            lStatus = lType.toUpper().split("|").contains(optionValue.simplified().toUpper());
        else
            lStatus = true;
    }

    return lStatus;
}

bool AdminEngine::GetUserSettingsValues(QMap<QString,QString>& mapSettings, bool ForAll)
{
    if(m_pCurrentUser == NULL)
        return false;

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strQuery = "SELECT field,value FROM ym_users_options ";
    if(ForAll)
        strQuery += " WHERE (user_id IS NULL OR user_id=0) ";
    else
        strQuery += " WHERE (user_id = " + QString::number(m_pCurrentUser->m_nUserId) + ") ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    while(clNodeQuery.next())
        mapSettings[clNodeQuery.value(0).toString().trimmed()] = clNodeQuery.value(1).toString().trimmed();

    return true;
}

bool AdminEngine::GetUserSettingsValue(const QString &strOptionName, QString &strOptionValue)
{
    if(m_pCurrentUser == NULL)
        return false;

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    strOptionValue = "";

    strQuery = "SELECT value FROM ym_users_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    strQuery += " AND (user_id = " + QString::number(m_pCurrentUser->m_nUserId) + ") ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    if(clNodeQuery.first())
    {
        strOptionValue = clNodeQuery.value(0).toString().trimmed();
        return true;
    }
    strQuery = "SELECT value FROM ym_users_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    strQuery += " AND (user_id IS NULL OR user_id=0) ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }
    if(clNodeQuery.first())
    {
        strOptionValue = clNodeQuery.value(0).toString().trimmed();
        return true;
    }
    return false;
}

bool AdminEngine::SetUserSettingsValue(const QString &strOptionName, QString &strOptionValue, bool ForAll)
{
    if(m_pCurrentUser == NULL)
        return false;

    QString        strQuery;
    QSqlQuery      clNodeQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    // if ForAll is true
    // Apply this option for all users
    // Delete entry for current user_id
    strQuery = "DELETE FROM ym_users_options ";
    strQuery+= " WHERE field=" +  NormalizeStringToSql(strOptionName.toUpper()) + " ";
    if(!ForAll)
        strQuery+= " AND (user_id = " + QString::number(m_pCurrentUser->m_nUserId) + ") ";
    else
        strQuery += " AND (user_id IS NULL OR user_id=0) ";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }

    if(strOptionValue.isEmpty())
        return true;

    strQuery = "INSERT INTO ym_users_options (user_id,field,value)";
    strQuery+= " VALUE(";
    if(ForAll)
        strQuery+= "0,";
    else
        strQuery+= QString::number(m_pCurrentUser->m_nUserId) + ",";
    strQuery+=NormalizeStringToSql(strOptionName.toUpper()) + "," + NormalizeStringToSql(strOptionValue) + ")";
    if(!clNodeQuery.exec(strQuery))
    {
        SetLastError(clNodeQuery);
        return false;
    }

    clNodeQuery.exec("COMMIT");
    return true;
}

bool AdminEngine::IsNodeConnected(int nNodeId)
{
    if(!ConnectToAdminServer())
        return false;

    if(nNodeId < 0)
        return false;

    QString        lQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    // To know if the user is connected
    // Check the validity ym_users.mutex
    // If empty = not connected
    // If valid = connected
    lQuery = "SELECT mutex FROM ym_nodes WHERE node_id="+QString::number(m_nNodeId);
    lQuery+= " AND NOT(mutex IS NULL OR mutex='')";
    if(!clQuery.exec(lQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    if(clQuery.first())
    {
        // User seems to be connected
        // Check the validity of the mutex
        if(IsMutexActive(clQuery.value(0).toString()))
        {
            // Node already connected
            return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////
// Read user info
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToRead(AdminUser *pUser)
{
    if(pUser == NULL)
        return false;

    // True if it is me
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser == pUser)
        return true;

    return GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges(true);
}

//////////////////////////////////////////////////////////////////////
// Modify user info
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToModify(AdminUser *pUser)
{
    if(pUser == NULL)
        return false;

    // True if it is me
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser == pUser)
        return true;

    return GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges();
}

//////////////////////////////////////////////////////////////////////
// Read group info
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToRead(AdminUserGroup *pGroup)
{
    if(pGroup == NULL)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////
// Modify group info
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToModify(AdminUserGroup *pGroup)
{
    if(pGroup == NULL)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////
// show or hide task in taskList
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToRead(CGexMoTaskItem* /*pTask*/)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // All task loaded
        return true;
    }

    // Display task list in Examinator
    // TODO

    return false;

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToModify(CGexMoTaskItem* pTask)
{
    bool bIsAllowedToModify = false;

    // Need user connection
    if(!IsUserConnected())
        return false;

    // If Super Admin, always true
    if(IsUserConnected(true))
        bIsAllowedToModify = true;
    // For other User
    else if(IsUserConnected())
    {
        // Check if admin is the owner of the task
        if(pTask->m_iUserId == m_pCurrentUser->m_nUserId)
            bIsAllowedToModify =  true;
        else
        {
            // different users
            QString strUserPermissions = QString::number(pTask->m_iPermissions).left(3);
            bIsAllowedToModify =  strUserPermissions.mid(1,1) == "1";
        }
    }

    return bIsAllowedToModify;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToLock(CGexMoTaskItem* pTask)
{
    // Need user connection
    if(!IsUserConnected())
        return false;

    // If Super Admin, always true
    if(IsUserConnected(true))
        return true;

    if(IsUserConnected())
    {
        // Check if admin is the owner of the task
        if(pTask->m_iUserId == m_pCurrentUser->m_nUserId)
            return true;
    }

    return false;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToExecute(CGexMoTaskItem* pTask)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Check if have Admin connection
        if(IsUserConnected(true))
            return true;

        if(IsUserConnected())
        {
            // Check if admin is the owner of the task
            if(pTask->m_iUserId == m_pCurrentUser->m_nUserId)
                return true;

            // Check if admin group is the same of the task user group
            if(pTask->m_iGroupId == m_pCurrentUser->m_nGroupId)
                return true;

            return false;
        }

        return true;
    }

    // Execute task list in Examinator
    // TODO

    return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsInRestrictedMode(GexDatabaseEntry *pDatabaseEntry)
{
    if(!pDatabaseEntry->m_pExternalDatabase) // (!pDatabaseEntry->IsStoredInDb())
        return false;

    bool bSecuredMode = false;

    // Check from gexdb if connected
    // do not force connection
    if(pDatabaseEntry->m_pExternalDatabase && pDatabaseEntry->m_pExternalDatabase->IsDbConnected())
    {
        bSecuredMode = pDatabaseEntry->m_pExternalDatabase->IsSecuredMode();
        pDatabaseEntry->UnsetStatusFlags(STATUS_SECURED_MODE);
        if(bSecuredMode)
            pDatabaseEntry->SetStatusFlags(STATUS_SECURED_MODE);
    }
    else if(mDirAccessPlugin)
    {
        // if not connected, check into da_galaxy
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();
        if(applications && applications->Exists("galaxy:examinator:databases:"+pDatabaseEntry->LogicalName()))
            bSecuredMode = true;
        if(applications && applications->Exists("galaxy:yieldman:databases:"+pDatabaseEntry->LogicalName()))
            bSecuredMode = true;
        if(applications && applications->Exists("galaxy:patman:databases:"+pDatabaseEntry->LogicalName()))
            bSecuredMode = true;
        pDatabaseEntry->UnsetStatusFlags(STATUS_SECURED_MODE);
        if(bSecuredMode)
            pDatabaseEntry->SetStatusFlags(STATUS_SECURED_MODE);
    }
    else if(pDatabaseEntry->StatusFlags() & STATUS_SECURED_MODE)
        // if cannot use da_galaxy for any reason
        // Check if already see this DB as SECURED
        bSecuredMode = true;


    return bSecuredMode;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToRead(GexDatabaseEntry *pDatabaseEntry)
{
    if(!IsInRestrictedMode(pDatabaseEntry))
        return true;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // All database loaded
        return true;
    }

    if(IsUserConnected(true))
        return true;

    GSLOG(SYSLOG_SEV_NOTICE, QString(" Restricted Access Mode for '%1'")
          .arg(pDatabaseEntry->LogicalName().toLatin1().constData() )
          .toLatin1().constData());

    if(mDirAccessPlugin)
    {
        bool bIsAllowed = false;
        // if not connected, check into da_galaxy
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();

        if(applications)
        {
            if ((GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                 GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()) &&
                    applications->IsAllowedTo("galaxy:examinator:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::READACCESS))
                bIsAllowed = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan() &&
                    applications->IsAllowedTo("galaxy:yieldman:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::READACCESS))
                bIsAllowed = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan() &&
                    applications->IsAllowedTo("galaxy:patman:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::READACCESS))
                bIsAllowed = true;
        }
        return bIsAllowed;
    }

    return false;

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool AdminEngine::IsAllowedToModify(GexDatabaseEntry *pDatabaseEntry)
{
    if(!IsInRestrictedMode(pDatabaseEntry))
        return true;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // All database loaded
        return true;
    }

    if(IsUserConnected(true))
        return true;

    GSLOG(SYSLOG_SEV_NOTICE, QString(" Restriction Mode for '%1'")
          .arg(pDatabaseEntry->LogicalName().toLatin1().constData() )
          .toLatin1().constData());

    if(mDirAccessPlugin)
    {
        bool bIsAllowed = false;
        // if not connected, check into da_galaxy
        GS::DAPlugin::AppEntriesBase * applications = mDirAccessPlugin->GetAppEntries();

        if(applications)
        {
            if((GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()) &&
                    applications->IsAllowedTo("galaxy:examinator:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::WRITEACCESS))
                bIsAllowed = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan() &&
                    applications->IsAllowedTo("galaxy:yieldman:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::WRITEACCESS))
                bIsAllowed = true;
            if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan() &&
                    applications->IsAllowedTo("galaxy:patman:databases:"+pDatabaseEntry->LogicalName(),
                                              mDirAccessPlugin->GetCurrentUser(),
                                              GS::DAPlugin::WRITEACCESS))
                bIsAllowed = true;
        }
        return bIsAllowed;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Lock item in ym_admin_db for update
//////////////////////////////////////////////////////////////////////
bool AdminEngine::Lock(CGexMoTaskItem *ptTask)
{
    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    if(ptTask == NULL)
        return false;

    if(ptTask->IsLocal())
        return true;

    QSqlQuery  clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;
    // Check if the current Task is locked
    strQuery = "SELECT mutex FROM ym_tasks";
    strQuery+= " WHERE task_id="+QString::number(ptTask->GetID());
    if(!clQuery.exec(strQuery)) return clQuery.lastError().text().contains("mutex"); // FOR B7 NOT ALREADY IN B8 ...
    if(!clQuery.first()) return false;

    QString lMutex;
    lMutex = clQuery.value(0).toString();

    // Check if the lock is always active
    if(!lMutex.isEmpty() && IsMutexActive(lMutex))
    {
        SetLastError(eDB_InvalidEntry,"This task is currently updated on "+lMutex.section(":",1,1)+"!\n Please try later.");
        return false;
    }

    // Try to Lock the task
    strQuery = "UPDATE ym_tasks";
    strQuery+= " SET mutex='"+mMutex+"'";
    strQuery+= " WHERE task_id="+QString::number(ptTask->GetID());
    if(lMutex.isEmpty())
        strQuery+= " AND mutex IS NULL";
    else
        strQuery+= " AND mutex='"+lMutex+"'";
    if(!clQuery.exec(strQuery))
    {
        clQuery.exec("COMMIT"); // To unlock any tables
        return false;
    }

    clQuery.exec("COMMIT");

    // Check if task is locked
    strQuery = "SELECT mutex FROM ym_tasks";
    strQuery+= " WHERE task_id="+QString::number(ptTask->GetID());
    strQuery+= " AND mutex='"+mMutex+"'";
    if(!clQuery.exec(strQuery)) return false;
    if(!clQuery.first()) return false;

    // Only lock one task
    // Then Unlock all other tasks
    strQuery = "UPDATE ym_tasks";
    strQuery+= " SET mutex=" + QString(m_pDatabaseConnector->IsOracleDB() ? "''" : "null");
    strQuery+= " WHERE task_id!="+QString::number(ptTask->GetID());
    strQuery+= " AND mutex='"+mMutex+"'";
    clQuery.exec(strQuery);

    clQuery.exec("COMMIT");

    //WriteDebugMessageFile("AdminEngine::LockTask("+QString::number(ptTask->GetID())+")");
    return true;
}

bool AdminEngine::Unlock(CGexMoTaskItem *ptTask)
{
    if(ptTask == NULL)
        return false;

    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    QSqlQuery  clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;

    // UnLock the task
    strQuery = "UPDATE ym_tasks";
    strQuery+= " SET mutex=" + QString(m_pDatabaseConnector->IsOracleDB() ? "''" : "null");
    strQuery+= " WHERE mutex='"+mMutex+"'";
    if(ptTask && ptTask->IsUploaded())
        strQuery+= " AND task_id="+QString::number(ptTask->GetID());
    if(!clQuery.exec(strQuery))
    {
        clQuery.exec("COMMIT"); // To unlock any tables
        return false;
    }

    clQuery.exec("COMMIT");
    //WriteDebugMessageFile("AdminEngine::UnlockTask("+QString::number(ptTask->GetID())+")");
    return true;
}

///////////////////////////////////////////////////////////
bool AdminEngine::Lock(AdminUser *ptUser)
{
    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    if(ptUser == NULL)
        return false;

    return true;
}

bool AdminEngine::Unlock(AdminUser *ptUser)
{
    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    if(ptUser == NULL)
        return false;

    return true;
}

///////////////////////////////////////////////////////////
bool AdminEngine::Lock(GexDatabaseEntry *pDatabaseEntry)
{
    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    if(pDatabaseEntry == NULL)
        return false;

    if(!pDatabaseEntry->IsStoredInDb())
        return true;

    QSqlQuery  clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;
    // Check if the current Database is locked
    strQuery = "SELECT mutex FROM ym_databases";
    strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    if(!clQuery.exec(strQuery)) return clQuery.lastError().text().contains("mutex"); // FOR B7 NOT ALREADY IN B8 ...
    if(!clQuery.first()) return false;

    QString lMutex;
    lMutex = clQuery.value(0).toString();

    // Check if the lock is always active
    if(!lMutex.isEmpty() && IsMutexActive(lMutex))
    {
        SetLastError(eDB_InvalidEntry,"This database is currently updated on "+lMutex.section(":",1,1)+"!\n Please try later.");
        return false;
    }

    // Try to Lock the database
    strQuery = "UPDATE ym_databases";
    strQuery+= " SET mutex='"+mMutex+"'";
    strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    if(lMutex.isEmpty())
        strQuery+= " AND (mutex IS NULL OR mutex='')";
    else
        strQuery+= " AND mutex='"+lMutex+"'";
    if(!clQuery.exec(strQuery)) return false;

    clQuery.exec("COMMIT");

    // Check if database is locked
    strQuery = "SELECT mutex FROM ym_databases";
    strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    strQuery+= " AND mutex='"+mMutex+"'";
    if(!clQuery.exec(strQuery)) return false;
    if(!clQuery.first()) return false;

    // Only lock one database
    // Then Unlock all other databases
    strQuery = "UPDATE ym_databases";
    strQuery+= " SET mutex=" + QString(m_pDatabaseConnector->IsOracleDB() ? "''" : "null");
    strQuery+= " WHERE database_id!="+QString::number(pDatabaseEntry->m_nDatabaseId);
    strQuery+= " AND mutex='"+mMutex+"'";
    clQuery.exec(strQuery);

    clQuery.exec("COMMIT");

    //WriteDebugMessageFile("AdminEngine::LockDatabase("+QString::number(pDatabaseEntry->m_nDatabaseId)+")");
    return true;
}

bool AdminEngine::Unlock(GexDatabaseEntry *pDatabaseEntry)
{
    // Check if yieldman is connected
    if(!ConnectToAdminServer())
        return false;

    if(m_pDatabaseConnector == NULL)
        return false;

    QSqlQuery  clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    QString strQuery;

    // UnLock the database
    strQuery = "UPDATE ym_databases";
    strQuery+= " SET mutex=" + QString(m_pDatabaseConnector->IsOracleDB() ? "''" : "null");
    strQuery+= " WHERE mutex='"+mMutex+"'";
    if(pDatabaseEntry && pDatabaseEntry->IsStoredInDb())
        strQuery+= " AND database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    if(!clQuery.exec(strQuery)) return false;

    clQuery.exec("COMMIT");
    //WriteDebugMessageFile("AdminEngine::UnlockDatabase("+QString::number(pDatabaseEntry->m_nDatabaseId)+")");
    return true;
}

QString AdminEngine::GetDatabaseFolder()
{
    QDir clDir;
    // Get option
    // If empty, create it and save
    QString strDatabasesPath = ReportOptions.GetOption("databases","yieldman_path").toString();
    if(!strDatabasesPath.isEmpty())
    {
        QString strGlobalPath = strDatabasesPath;
        // Check if GalaxySemi exists
        if(strGlobalPath.contains("databases",Qt::CaseInsensitive))
        {
            // Supp subFolder /databases/yieldman
            strGlobalPath = strGlobalPath.replace("databases","databases",Qt::CaseInsensitive);
            strGlobalPath = strGlobalPath.section("databases",0,0);
        }
        if(!clDir.exists(strGlobalPath))
            strDatabasesPath = "";
    }
    if(strDatabasesPath.isEmpty())
    {
        strDatabasesPath = QDir::cleanPath(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                                           + QString(GEX_DATABASE_FOLDER)
                                           + QString(GEX_DATABASE_YIELDMAN));
        ReportOptions.SetOption("databases","yieldman_path",strDatabasesPath);
    }

    clDir.mkpath(strDatabasesPath);
    return strDatabasesPath;
}

bool AdminEngine::SaveDatabase(GexDatabaseEntry* pDatabaseEntry)
{

    // Check if database access configured
    if(m_pDatabaseConnector == NULL)
        return false;

    if(!ConnectToAdminServer())
        return false;

    if(pDatabaseEntry == NULL)
    {
        // Invalid database
        SetLastError(eDB_InvalidEntry, "Invalid database entry");
        return false;
    }

    if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase == NULL)
    {
        // Invalid database
        SetLastError(eDB_InvalidEntry, "Invalid database entry");
        return false;
    }

    GexDbPlugin_ID *pPlugin = NULL;
    if(pDatabaseEntry->IsExternal())
    {
        pPlugin = pDatabaseEntry->m_pExternalDatabase->GetPluginID();
        if(pPlugin == NULL)
        {
            // Invalid database
            SetLastError(eDB_InvalidEntry, "Invalid database entry");
            return false;
        }

        if(pPlugin->m_pPlugin == NULL)
        {
            // Invalid database
            SetLastError(eDB_InvalidEntry, "Invalid database entry");
            return false;
        }

        if(pPlugin->m_pPlugin->m_pclDatabaseConnector == NULL)
        {
            // Invalid database
            SetLastError(eDB_InvalidEntry, "Invalid database entry");
            return false;
        }
    }

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    if(!pDatabaseEntry->IsStoredInDb())
    {
        // Check if database is already referenced
        strQuery = "SELECT * FROM ym_databases DB WHERE DB.name='" + pDatabaseEntry->LogicalName()+"'";
        if(pPlugin)
        {
            //strQuery+= " AND DB.plugin_file='"+ pPlugin->m_strFileName +"'";
            strQuery+= " AND (DB.host LIKE '%"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name+"%'";
            strQuery+= " OR DB.host LIKE '%"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP+"%'";
            strQuery+= " OR DB.host LIKE '%"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved+"%')";
            strQuery+= " AND DB.port="+ QString::number(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_uiPort) ;
            strQuery+= " AND DB.driver='"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDriver+"'";
            strQuery+= " AND DB.schema='"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName +"'";
            strQuery+= " AND DB.database='"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDatabaseName +"'";
        }
        // node_id ???
        //strQuery+= " AND node_id="+QString::number(m_nNodeId);
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }
        if(!clQuery.first())
        {
            // have to create and update this new database
            QTime clTime = QTime::currentTime();
            if(m_pDatabaseConnector->IsOracleDB())
            {
                strQuery = "INSERT INTO ym_databases(database_id,name,creation_date) VALUES(";
                strQuery+="ym_databases_sequence.nextval,'"+clTime.toString("hhmmsszz")+"',SYSDATE)";
            }
            else
            {
                strQuery = "INSERT INTO ym_databases(name,creation_date) VALUES(";
                strQuery+= "'"+clTime.toString("hhmmsszz")+"',NOW())";
            }
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

            // Retreve the GroupId
            strQuery = "SELECT DB.database_id FROM ym_databases DB WHERE DB.name='"+clTime.toString("hhmmsszz")+"'";
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
            clQuery.first();
            pDatabaseEntry->m_nDatabaseId = clQuery.value(0).toInt();
        }
    }

    // Then Save current database
    // Save the Release Plugin file name instead to the Debug
    QString strPluginFileName;
    QString strUserPwd, strAdminPwd;
    QString  strDateNow = "now()";

    if(m_pDatabaseConnector->IsOracleDB())
        strDateNow = "SYSDATE";

    if(pPlugin)
    {
        strPluginFileName = pPlugin->pluginFileName();
        if(strPluginFileName.contains("d."))
        {
            // try with release plugin
            strPluginFileName = strPluginFileName.replace("d.",".");
        }

#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
#ifdef CASE4882
        GSLOG(SYSLOG_SEV_EMERGENCY,
              QString("Crypting password %1 for YM user %2 before updating YMDB (ym_databases)")
              .arg(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strPassword)
              .arg(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strUserName)
              .toLatin1().constData());
#endif
#endif
        GexDbPlugin_Base::CryptPassword(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strPassword    , strUserPwd);
#ifdef QT_DEBUG
#ifdef CASE4882
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        GSLOG(SYSLOG_SEV_EMERGENCY,
              QString("Crypting password %1 for YM user %2 before updating YMDB (ym_databases)")
              .arg(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strPassword_Admin)
              .arg(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strUserName_Admin)
              .toLatin1().constData());
#endif
#endif
        GexDbPlugin_Base::CryptPassword(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strPassword_Admin , strAdminPwd);
    }


    strQuery = "UPDATE ym_databases DB SET ";
    strQuery+= " DB.name=" + NormalizeStringToSql(pDatabaseEntry->LogicalName())+"";
    if(m_nNodeId > 0)
        strQuery+= " , DB.node_id="+QString::number(m_nNodeId);
    strQuery+= " , DB.description="+ NormalizeStringToSql(pDatabaseEntry->Description()) +"";
    strQuery+= " , last_update="+strDateNow;
    strQuery+= " , mutex=null";
    if(pPlugin)
    {
        strQuery+= " , DB.plugin_name='"+ pPlugin->pluginName().section("[",0,0).simplified() +"'";
        strQuery+= " , DB.plugin_file='"+ strPluginFileName + "'";
        strQuery+= " , DB.plugin_version='"+ QString::number(pPlugin->pluginBuild()) + "'";
        strQuery+= " , DB.host='"+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name;
        strQuery+= "["+ pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP + "]";
        strQuery+= pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved + "'";
        strQuery+= " , DB.port="+ QString::number(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_uiPort) ;
        strQuery+= " , DB.driver="+ NormalizeStringToSql(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDriver)+"";
        strQuery+= " , DB.schema="+ NormalizeStringToSql(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName) +"";
        strQuery+= " , DB.database="+ NormalizeStringToSql(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDatabaseName) +"";
        strQuery+= " , DB.user_name="+ NormalizeStringToSql(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strUserName) +"";
        strQuery+= " , DB.user_pwd="+ NormalizeStringToSql(strUserPwd) +"";
        strQuery+= " , DB.admin_name="+ NormalizeStringToSql(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strUserName_Admin) +"";
        strQuery+= " , DB.admin_pwd="+ NormalizeStringToSql(strAdminPwd) +"";
        strQuery+= " , DB.type="+ NormalizeStringToSql(pDatabaseEntry->TdrTypeRecorded()) +"";
    }
    else
    {
        strQuery+= " , DB.plugin_name=null , DB.plugin_file=null , DB.plugin_version=null";
        strQuery+= " , DB.host=null , DB.port=null , DB.driver=null";
        strQuery+= " , DB.schema=null , DB.database=null";
        strQuery+= " , DB.user_name=null , DB.user_pwd=null , DB.admin_name=null , DB.admin_pwd=null";
        strQuery+= " , DB.type='"+ pDatabaseEntry->StorageType() +"'";
    }
    strQuery+= " WHERE DB.database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }

    // Then update ym_databases_options
    strQuery = "DELETE FROM ym_databases_options ";
    strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    clQuery.exec(strQuery);

    if(pPlugin)
    {
        // Startup Type option
        if(pPlugin->m_pPlugin->IsAutomaticStartup())
            strQuery = "INSERT INTO ym_databases_options (database_id,field,value) VALUES("+QString::number(pDatabaseEntry->m_nDatabaseId)+",'StartupType','1')";
        else
            strQuery = "INSERT INTO ym_databases_options (database_id,field,value) VALUES("+QString::number(pDatabaseEntry->m_nDatabaseId)+",'StartupType','0')";
        clQuery.exec(strQuery);
        // ADR link for TDR
        if (pPlugin->m_pPlugin->MustHaveAdrLink())
        {
            strQuery = "INSERT INTO ym_databases_options (database_id,field,value) VALUES("
                       + QString::number(pDatabaseEntry->m_nDatabaseId)+",'AdrLink','"
                       + pPlugin->m_pPlugin->GetAdrLinkName() + "')";
            clQuery.exec(strQuery);
        }
    }

    clQuery.exec("COMMIT");

    // Then get the update date
    strQuery = "SELECT last_update FROM ym_databases ";
    strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
    clQuery.exec(strQuery);
    clQuery.first();
    /// TODO Sandrine: what to do if first return false
    pDatabaseEntry->m_clLastUpdate = clQuery.value(0).toDateTime();


    return true;
}

QDomElement AdminEngine::GetDatabaseEntryDomElt(int nDatabaseId)
{
    QDomDocument doc;

    QDomElement domEltRoot = doc.createElement("gex_database_entry");
    QDomElement domEltTmp;
    QDomText    domTextTmp;
    int iFieldNo;

    QString     strQuery = "";
    QSqlQuery   clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));


    // Check if database access configured
    if(m_pDatabaseConnector == NULL)
        return domEltRoot;

    if(!ConnectToAdminServer())
        return domEltRoot;

    strQuery = "SELECT DB.database_id, DB.node_id, DB.name, DB.description, DB.plugin_name, DB.plugin_file, DB.plugin_version, DB.host,";
    strQuery+= " DB.port, DB.driver, DB.schema, DB.database, DB.user_name, DB.user_pwd, DB.admin_name, DB.admin_pwd, DB.incremental_updates,";
    strQuery+= " DB.creation_date, DB.expiration_date, DB.type";
    strQuery+= " FROM ym_databases DB";
    strQuery+= " WHERE DB.database_id="+QString::number(nDatabaseId);

    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return domEltRoot;
    }

    if(!clQuery.first())
    {
        // Invalid database
        SetLastError(eDB_InvalidEntry, "Invalid database entry");
        return domEltRoot;
    }

    bool bIsCorporateDatabase = false;
    iFieldNo = clQuery.record().indexOf("plugin_name");
    bIsCorporateDatabase = !clQuery.value(iFieldNo).toString().isEmpty();

    /////////////////////////////////////
    // <admindb> lvl 0
    QDomElement domEltAdminDb = doc.createElement("admindb");
    // DB ID
    domEltTmp = doc.createElement("databaseid");
    iFieldNo = clQuery.record().indexOf("database_id");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltAdminDb.appendChild(domEltTmp);
    // Node ID
    domEltTmp = doc.createElement("nodeid");
    iFieldNo = clQuery.record().indexOf("node_id");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltAdminDb.appendChild(domEltTmp);
    // Creation date
    domEltTmp = doc.createElement("creationdate");
    iFieldNo = clQuery.record().indexOf("creation_date");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltAdminDb.appendChild(domEltTmp);
    // Expiration date
    domEltTmp = doc.createElement("expirationdate");
    iFieldNo = clQuery.record().indexOf("expiration_date");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltAdminDb.appendChild(domEltTmp);

    domEltRoot.appendChild(domEltAdminDb);
    // </admindb> lvl 0
    /////////////////////////////////////

    /////////////////////////////////////
    // <database> lvl 0
    QDomElement domEltDatabase = doc.createElement("database");
    // name
    domEltTmp = doc.createElement("Name");
    iFieldNo = clQuery.record().indexOf("name");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltDatabase.appendChild(domEltTmp);
    // ExternalDatabase
    domEltTmp = doc.createElement("ExternalDatabase");
    domTextTmp = doc.createTextNode((bIsCorporateDatabase?"1":"0"));
    domEltTmp.appendChild(domTextTmp);
    domEltDatabase.appendChild(domEltTmp);
    // desc
    domEltTmp = doc.createElement("Description");
    iFieldNo = clQuery.record().indexOf("description");
    domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
    domEltTmp.appendChild(domTextTmp);
    domEltDatabase.appendChild(domEltTmp);
    // Storage engine (spider...). Not saved in AdminDB, add an empty element.
    // Will be populated when connecting to TDR.
    domEltTmp = doc.createElement("StorageEngine");
    domEltDatabase.appendChild(domEltTmp);

    if(bIsCorporateDatabase)
    {
        // tdr db type charac/prod/ym
        domEltTmp = doc.createElement("TdrType");
        iFieldNo = clQuery.record().indexOf("type");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltDatabase.appendChild(domEltTmp);

        // Storage
        domEltTmp = doc.createElement("Storage");
        domTextTmp = doc.createTextNode("Copy");
        domEltTmp.appendChild(domTextTmp);
        domEltDatabase.appendChild(domEltTmp);
    }
    else
    {
        // Storage
        domEltTmp = doc.createElement("Storage");
        iFieldNo = clQuery.record().indexOf("type");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltDatabase.appendChild(domEltTmp);
    }

    domEltRoot.appendChild(domEltDatabase);
    // </database> lvl 0
    /////////////////////////////////////

    if(bIsCorporateDatabase)
    {

        /////////////////////////////////////
        // <ExternalDatabase> lvl 0
        QDomElement domEltExtDatabase = doc.createElement("ExternalDatabase");
        /////////////////////////////////////
        //     <Identification> lvl 1
        QDomElement domEltIdentification = doc.createElement("Identification");
        // file
        domEltTmp = doc.createElement("PluginFile");
        iFieldNo = clQuery.record().indexOf("plugin_file");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltIdentification.appendChild(domEltTmp);
        // name
        domEltTmp = doc.createElement("PluginName");
        iFieldNo = clQuery.record().indexOf("plugin_name");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltIdentification.appendChild(domEltTmp);
        // build
        domEltTmp = doc.createElement("PluginBuild");
        iFieldNo = clQuery.record().indexOf("plugin_version");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltIdentification.appendChild(domEltTmp);

        domEltExtDatabase.appendChild(domEltIdentification);
        //     </Identification> lvl 1
        /////////////////////////////////////
        /////////////////////////////////////
        //     <PluginConfig>  lvl 1
        QDomElement domEltPluConfig = doc.createElement("PluginConfig");
        /////////////////////////////////////
        //         <Connection>  lvl 2
        QDomElement domEltConnection = doc.createElement("Connection");
        //Host IP NAME UNRESOLVED
        QString strHost, strHostName, strHostIP, strHostUnresolved;
        iFieldNo = clQuery.record().indexOf("host");
        strHost = clQuery.value(iFieldNo).toString();
        // Host
        domEltTmp = doc.createElement("Host");
        domTextTmp = doc.createTextNode(strHost);
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        if(strHost.contains("["))
        {
            // HostName[HostIP]HostUnresolved
            strHostName = strHost.section("[",0,0);
            strHostIP = strHost.section("[",1).section("]",0,0);
            strHostUnresolved = strHost.section("]",1);

            if(strHostUnresolved.isEmpty())
                strHostUnresolved = strHostName;
        }
        // Host name
        domEltTmp = doc.createElement("HostName");
        domTextTmp = doc.createTextNode(strHostName);
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        // Host name
        domEltTmp = doc.createElement("HostIP");
        domTextTmp = doc.createTextNode(strHostIP);
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        // Host name
        domEltTmp = doc.createElement("UnresolvedHostName");
        domTextTmp = doc.createTextNode(strHostUnresolved);
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //Driver
        domEltTmp = doc.createElement("Driver");
        iFieldNo = clQuery.record().indexOf("driver");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //DatabaseName
        domEltTmp = doc.createElement("DatabaseName");
        iFieldNo = clQuery.record().indexOf("database");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //UserName
        domEltTmp = doc.createElement("UserName");
        iFieldNo = clQuery.record().indexOf("user_name");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //UserPassword
        domEltTmp = doc.createElement("UserPassword");
        iFieldNo = clQuery.record().indexOf("user_pwd");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //UserName_Admin
        domEltTmp = doc.createElement("UserName_Admin");
        iFieldNo = clQuery.record().indexOf("admin_name");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //UserPassword_Adm = doc.createTextNodein
        domEltTmp = doc.createElement("UserPassword_Admin");
        iFieldNo = clQuery.record().indexOf("admin_pwd");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //SchemaName
        domEltTmp = doc.createElement("SchemaName");
        iFieldNo = clQuery.record().indexOf("schema");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);
        //Port
        domEltTmp = doc.createElement("Port");
        iFieldNo = clQuery.record().indexOf("port");
        domTextTmp = doc.createTextNode(clQuery.value(iFieldNo).toString());
        domEltTmp.appendChild(domTextTmp);
        domEltConnection.appendChild(domEltTmp);

        domEltPluConfig.appendChild(domEltConnection);
        //         </Connection> lvl 2
        /////////////////////////////////////
        /////////////////////////////////////
        //         <Options> lvl 2
        QDomElement domEltOptions = doc.createElement("Options");
        strQuery = "select DB.field,DB.value FROM ym_databases_options DB";
        strQuery+= " WHERE DB.database_id="+QString::number(nDatabaseId);
        clQuery.exec(strQuery);

        QString strValue, strField;
        while(clQuery.next())
        {
            strField = clQuery.value(0).toString();
            strValue = clQuery.value(1).toString();
            if (!strField.isEmpty())
            {
                domEltTmp = doc.createElement(strField);
                domTextTmp = doc.createTextNode(strValue);
                domEltTmp.appendChild(domTextTmp);
                domEltOptions.appendChild(domEltTmp);
            }
        }

        domEltPluConfig.appendChild(domEltOptions);
        //         </Options> lvl 2
        /////////////////////////////////////
        domEltExtDatabase.appendChild(domEltPluConfig);
        //     </PluginConfig> lvl 1
        /////////////////////////////////////
        domEltRoot.appendChild(domEltExtDatabase);
        // </ExternalDatabase> lvl 0
        /////////////////////////////////////
    }


    return domEltRoot;
}

///////////////////////////////////////////////////////////
// Database entry
///////////////////////////////////////////////////////////
void AdminEngine::SynchronizeProfiles(const QString & strDefaultUserName)
{
    if(!IsUserConnected())
        return;

    // Save profile whit Examinator-PRO even if user is admin
    //if(m_pCurrentUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return;

    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles");

    // Check if all profile entry from the YieldManDatabase table are in the os
    // If not, have to copy all user profile available for this osLogin
    AdminUserProfile *pProfile;
    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
    strQuery = "SELECT profile_id, user_id, name, description, os_login, permisions, script_name, script_content, creation_date, last_update FROM ym_users_profiles";
    strQuery+= " WHERE user_id="+QString::number(m_pCurrentUser->m_nUserId);
    //strQuery+= " AND os_login='"+m_strOsLogin+"'";
    strQuery+= " ORDER BY profile_id";

    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles - load from DB");
    if(clQuery.exec(strQuery))
    {
        while(clQuery.next())
        {
            if(!QFile::exists(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+"/"+clQuery.value(6).toString()))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("SynchronizeProfiles - create from DB %1")
                      .arg(clQuery.value(6).toString().toLatin1().constData())
                      .toLatin1().constData());
                QFile clFile(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
                             +"/"+clQuery.value(6).toString()); // Write the text to the file
                if(!clFile.open(QIODevice::WriteOnly))
                    continue;
                QTextStream hFile(&clFile);  // Assign file handle to data stream
                hFile << NormalizeSqlToScriptString(clQuery.value(7).toString());
                clFile.close();

                //WriteDebugMessageFile("    " + clQuery.value(6).toString() + " <= from YieldMan Server");
            }
            // Then update YieldManDb
            // Check if already in the list
            if(m_mapProfiles.contains(clQuery.value(0).toInt()))
                pProfile = m_mapProfiles[clQuery.value(0).toInt()];
            else
                pProfile = new AdminUserProfile;

            int iIndex = 0;
            pProfile->m_nProfileId = clQuery.value(iIndex++).toInt();
            pProfile->m_nUserId = clQuery.value(iIndex++).toInt();
            pProfile->m_strName = clQuery.value(iIndex++).toString();
            pProfile->m_strDescription = clQuery.value(iIndex++).toString();
            pProfile->m_strOsLogin = clQuery.value(iIndex++).toString();
            pProfile->m_nPermissions = clQuery.value(iIndex++).toInt();
            pProfile->m_strScriptName = clQuery.value(iIndex++).toString();
            pProfile->m_strScriptContent = NormalizeSqlToScriptString(clQuery.value(iIndex++).toString());
            pProfile->m_clCreationDate = clQuery.value(iIndex++).toDateTime();
            pProfile->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();

            m_mapProfiles[pProfile->m_nProfileId] = pProfile;
        }
    }
    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles - load from DB DONE");

    // Then check if all profile files in the os are in the YieldManDatabase and up to date

    // Find all files with name <name>_user_profile.csl
    QDir cDir(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    cDir.setFilter(QDir::Files);  // Non-recursive: ONLY import this folder
    QStringList strFiles = cDir.entryList(QStringList() << "*_user_profile.csl");  // Files extensions to look for...

    // Extract users names from file name
    QStringList::Iterator it;
    QString strFileName;

    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles - check from files");
    for(it = strFiles.begin(); it != strFiles.end(); ++it )
    {
        // Get file name
        strFileName = GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+"/"+*it;
        QFileInfo clFileInfo(strFileName);

        AdminUserProfile *pOsProfile = NULL;
        pProfile = NULL;
        foreach(pOsProfile,m_mapProfiles)
        {
            if(clFileInfo.fileName() == pOsProfile->m_strScriptName)
            {
                pProfile = pOsProfile;
                // Can have multi entries with differents Os
                // Old bug
                if((m_strOsLogin == pProfile->m_strOsLogin))
                    break;
            }
        }

        if(pProfile && (clFileInfo.fileName() == pProfile->m_strScriptName))
        {
            // Check is have to update this profile
            QString strDate1 = clFileInfo.lastModified().toString("dd/MM/yy hh:mm:ss");
            QString strDate2 = pProfile->m_clUpdateDate.toString("dd/MM/yy hh:mm:ss");
            if(strDate1 == strDate2)
                continue;
            else if(clFileInfo.lastModified() > pProfile->m_clUpdateDate)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("SynchronizeProfiles - update from file %1")
                      .arg(strFileName.toLatin1().constData())
                      .toLatin1().constData());
                QFile clFile(strFileName);
                if(!clFile.open(QIODevice::ReadOnly))
                    continue;
                QTextStream hFile(&clFile);  // Assign file handle to data stream

                // Then update YieldManDb
                pProfile->m_strScriptContent = hFile.readAll();
                clFile.close();

                //WriteDebugMessageFile("    " + clProfile.m_strScriptName + " => to YieldMan Server");

                QFileInfo clFileUpdated(strFileName);
                pProfile->m_clUpdateDate = clFileUpdated.lastModified();

                SaveProfile(pProfile);

            }
            else
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("SynchronizeProfiles - update from DB %1")
                      .arg(strFileName.toLatin1().constData())
                      .toLatin1().constData());
                // Update file
                QFile clFile(strFileName); // Write the text to the file
                if(!clFile.open(QIODevice::WriteOnly))
                    continue;
                QTextStream hFile(&clFile);  // Assign file handle to data stream
                hFile << NormalizeSqlToScriptString(clQuery.value(7).toString());
                clFile.close();


                QFileInfo clFileUpdated(strFileName);
                pProfile->m_clUpdateDate = clFileUpdated.lastModified();

                //WriteDebugMessageFile("    " + clProfile.m_strScriptName + " <= from YieldMan Server");
                SaveProfile(pProfile);
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("SynchronizeProfiles - create from file %1")
                  .arg(strFileName.toLatin1().constData())
                  .toLatin1().constData());
            QFile clFile(strFileName);
            if(!clFile.open(QIODevice::ReadOnly))
                continue;
            QTextStream hFile(&clFile);  // Assign file handle to data stream

            // Create a new entry
            pProfile = new AdminUserProfile;
            pProfile->m_nProfileId = 0;
            pProfile->m_nUserId = m_pCurrentUser->m_nUserId;
            pProfile->m_strName = clFileInfo.fileName().section("_user_profile.csl",0,0);
            pProfile->m_strDescription = clFileInfo.fileName();
            pProfile->m_strOsLogin = m_strOsLogin;
            pProfile->m_nPermissions = 0;
            pProfile->m_strScriptName = clFileInfo.fileName();
            pProfile->m_strScriptContent = hFile.readAll();
            pProfile->m_clUpdateDate = clFileInfo.lastModified();

            clFile.close();

            //WriteDebugMessageFile("    " + pProfile->m_strScriptName + " => to YieldMan Server");

            SaveProfile(pProfile);
        }
    }
    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles - check from files DONE");

    // Check if have a default profile and save it
    if(!strDefaultUserName.isEmpty())
    {
        strQuery = "SELECT profile_id FROM ym_users_profiles";
        strQuery+= " WHERE name='"+strDefaultUserName+"'";
        strQuery+= " AND user_id="+QString::number(m_pCurrentUser->m_nUserId);

        if(clQuery.exec(strQuery) && clQuery.first())
        {
            m_pCurrentUser->m_nProfileId = clQuery.value(0).toInt();
            SaveUser(m_pCurrentUser);

        }
    }
    GSLOG(SYSLOG_SEV_DEBUG, "SynchronizeProfiles DONE");
}

///////////////////////////////////////////////////////////
// Synchronize user profiles between YieldmanDB and the
// local folder.
///////////////////////////////////////////////////////////
void AdminEngine::DeleteProfiles(const QString & strDefaultUserName)
{
    if(!IsUserConnected())
        return;

    // Save profile whit Examinator-PRO even if user is admin
    //if(m_pCurrentUser->m_nType == YIELDMANDB_USERTYPE_MASTER_ADMIN)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return;

    //WriteDebugMessageFile("AdminEngine::DeleteProfiles()");

    // Check if have a default profile and save it
    if(!strDefaultUserName.isEmpty())
    {
        AdminUserProfile *pProfile=NULL;
        QString strQuery;
        QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));
        strQuery = "SELECT profile_id FROM ym_users_profiles";
        strQuery+= " WHERE name='"+strDefaultUserName+"'";
        strQuery+= " AND user_id="+QString::number(m_pCurrentUser->m_nUserId);

        if(clQuery.exec(strQuery) && clQuery.first())
        {
            if(m_mapProfiles.contains(clQuery.value(0).toInt()))
                pProfile = m_mapProfiles[clQuery.value(0).toInt()];
            if(pProfile)
            {
                if(m_pCurrentUser->m_nProfileId == pProfile->m_nProfileId)
                    m_pCurrentUser->m_nProfileId = 0;
                DeleteProfile(pProfile);
            }
        }
    }
}


}
}
