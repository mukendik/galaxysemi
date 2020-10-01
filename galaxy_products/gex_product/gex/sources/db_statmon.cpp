#include "browser_dialog.h"
#include "db_engine.h"
#include "gex_database_entry.h"
#include "gex_shared.h"
#include "gexmo_constants.h"
#include "engine.h"
#include "db_external_database.h"
#include "mo_email.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "statisticalMonitoring/statistical_monitoring_task.h"
#include "spm/spm_task.h"
#include "sya/sya_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include <gqtl_log.h>
#include "admin_engine.h"

#include <QBuffer>

extern QString LaunchShellScript(QString &strCommand);

void ExecuteShellOnError(QString Shell)
{
    if (!Shell.isEmpty())
    {
        Shell.replace("$Status", "0");
        QString lRunStatus=LaunchShellScript(Shell);
        if (lRunStatus.startsWith("error"))
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Launch script failed : %1")
                  .arg( lRunStatus)
                  .toLatin1().constData() );
    }

}

int GS::Gex::DatabaseEngine::IsConsolidationInProgress(
        QString databaseName,
        QString testingStage,
        QString lot,
        QString sublots,
        QString wafers,
        QString consoType,
        QString testFlow,
        QString consoLevel,
        QString testInsertion)
{
    GexDatabaseEntry *databaseEntry=NULL;
    databaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(databaseName,false);

    if(!databaseEntry)
    {
        return 2;
    }

    return databaseEntry->m_pExternalDatabase->IsConsolidationInProgress(testingStage,
                                                                         lot,
                                                                         sublots,
                                                                         wafers,
                                                                         consoType,
                                                                         testFlow,
                                                                         consoLevel,
                                                                         testInsertion);
}

// \brief Check the SPM of given params
// \param TestingStage
// \param Product
// \param Lot
// \param WaferID
// \param Shell : can contain $ISODate, $Date, $Time, $UserFolder
// \param RuleName : can be empty ?
// \param LogFile
// \param TriggerFileName : the trigger requesting this check if any
// \param lFullSourceScript : the source script in order to log it in the SPM log file
// \return fail, delay, passed
int GS::Gex::DatabaseEngine::CheckSPMForJS(QString DatabaseName, QString TestingStage, QString Product, QString Lot, QString Sublot, QString Wafer,
                                               QString Shell, QString RuleName,
                                               QString LogFileName, QString TriggerFileName, QString TriggerSource)
{
    QtLib::DatakeysContent dbKeysContent;
    dbKeysContent.Set("Product", Product);
    dbKeysContent.Set("DatabaseName",DatabaseName);
    dbKeysContent.Set("TestingStage", TestingStage);
    dbKeysContent.Set("Lot", Lot);
    dbKeysContent.Set("SubLot", Sublot);
    dbKeysContent.Set("Wafer", Wafer);
    dbKeysContent.Set("TaskName", RuleName);
    dbKeysContent.Set("TaskType", "SPM");

    int lStatus = CheckStatMon(dbKeysContent, Shell, LogFileName, TriggerFileName, TriggerSource);

    return lStatus;

}

// \brief Check the SYA of given params
// \param TestingStage
// \param Product
// \param Lot
// \param WaferID
// \param Shell : can contain $ISODate, $Date, $Time, $UserFolder
// \param RuleName : can be empty ?
// \param LogFile
// \param TriggerFileName : the trigger requesting this check if any
// \param lFullSourceScript : the source script in order to log it in the SYA log file
// \return fail, delay, passed
int GS::Gex::DatabaseEngine::CheckSYAForJS(QString DatabaseName, QString TestingStage, QString Product, QString Lot, QString Sublot, QString Wafer,
                                               QString Shell, QString RuleName,
                                               QString LogFileName, QString TriggerFileName, QString TriggerSource)
{
    QtLib::DatakeysContent dbKeysContent;
    dbKeysContent.Set("Product", Product);
    dbKeysContent.Set("DatabaseName",DatabaseName);
    dbKeysContent.Set("TestingStage", TestingStage);
    dbKeysContent.Set("Lot", Lot);
    dbKeysContent.Set("SubLot", Sublot);
    dbKeysContent.Set("Wafer", Wafer);
    dbKeysContent.Set("TaskName", RuleName);
    dbKeysContent.Set("TaskType", "SYA");

    int lStatus = CheckStatMon(dbKeysContent, Shell, LogFileName, TriggerFileName, TriggerSource);

    return lStatus;

}

int GS::Gex::DatabaseEngine::CheckStatMon(QtLib::DatakeysContent& dbKeysContent,
                                      QString& Shell, QString& LogFileName, QString& TriggerFileName, QString& TriggerSource)
{
    int lStatus = Passed;
    m_strInsertionShortErrorMessage = "";

    QString Product = dbKeysContent.Get("Product").toString();
    QString Lot = dbKeysContent.Get("Lot").toString();
    QString Sublot = dbKeysContent.Get("Sublot").toString();
    QString Wafer = dbKeysContent.Get("Wafer").toString();
    QString DatabaseName = dbKeysContent.Get("DatabaseName").toString();
    QString TestingStage = dbKeysContent.Get("TestingStage").toString();
    QString RuleName = dbKeysContent.Get("TaskName").toString();
    QString TaskType = dbKeysContent.Get("TaskType").toString();
    QString entryPoint = "Check" + TaskType + "ForJS";

    GSLOG(SYSLOG_SEV_NOTICE, QString("Check" + TaskType + " %1 %2 %3 %4")
           .arg(Product).arg(Lot).arg(Sublot).arg(Wafer)
          .toLatin1().data());

    if(!Shell.isEmpty())
    {
        Shell.replace("$Action", "Check" + TaskType);
        Shell.replace("$ISODate", QDate::currentDate().toString(Qt::ISODate) ); // 2011-04-12
        Shell.replace("$Date", QDate::currentDate().toString("yyyyMMdd"));
        Shell.replace("$Time", QTime::currentTime().toString("HHmmss") ); // Qt::ISODate = 17:26:43 : not compliant with filename
        Shell.replace("$UserFolder", QDir::homePath());
        Shell.replace("$ProductID", Product);
        Shell.replace("$LotID", Lot);
        Shell.replace("$SublotID", Sublot);
        Shell.replace("$WaferID", Wafer);
        Shell.replace("$RuleName", QString(RuleName).remove(' ') );
        Shell.replace("$DatabaseName", QString(DatabaseName).remove(' '));
        Shell.replace("$TestingStage", QString(TestingStage).remove(' '));
        Shell.replace("$LogFile", LogFileName);
        Shell.replace("$Status", "1");
        Shell.replace("$FailStatus", "0" );
    }
    if(!LogFileName.isEmpty())
    {
        // exemple : G:/$Action-$ProductID-$LotID-$WaferID-$ISODate_$Time.log
        LogFileName.replace("$Action", "Check" + TaskType);
        LogFileName.replace("$ISODate", QDate::currentDate().toString(Qt::ISODate) ); // 2011-04-12
        LogFileName.replace("$Date", QDate::currentDate().toString("yyyyMMdd"));
        LogFileName.replace("$Time", QTime::currentTime().toString("HHmmss") ); // Qt::ISODate = 17:26:43 : not compliant with filename
        LogFileName.replace("$UserFolder", QDir::homePath());
        LogFileName.replace("$ProductID", Product);
        LogFileName.replace("$LotID", Lot);
        LogFileName.replace("$SublotID", Sublot);
        LogFileName.replace("$WaferID", Wafer);
        LogFileName.replace("$RuleName", QString(RuleName).remove(' ') );
        LogFileName.replace("$DatabaseName", QString(DatabaseName).remove(' '));
        LogFileName.replace("$TestingStage", QString(TestingStage).remove(' '));
    }

    dbKeysContent.Set("TriggerFile", TriggerFileName);
    dbKeysContent.Set("TriggerSource", TriggerSource);

    if (!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isActivated())
    {
        m_strInsertionShortErrorMessage="Monitoring scheduler not ready";
        lStatus = Delay;
        ExecuteShellOnError(Shell.replace("$Status", "0"));
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("WARNING", entryPoint + ": task delayed", m_strInsertionShortErrorMessage);
        return lStatus;
    }

    // Check the validity of the attribute
    m_strInsertionShortErrorMessage = "";
    GexDatabaseEntry *pDatabaseEntry=NULL;
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(DatabaseName,false);

    if(pDatabaseEntry == NULL)
        m_strInsertionShortErrorMessage="bad/unfindable check" + TaskType + " 'DatabaseName' attribute in check " + TaskType + " action !";
    if((TestingStage=="") && m_strInsertionShortErrorMessage.isEmpty())
        m_strInsertionShortErrorMessage="bad/unfindable check" + TaskType + " 'TestingStage' attribute in check " + TaskType + " action !";
    if((Product=="") && m_strInsertionShortErrorMessage.isEmpty())
        m_strInsertionShortErrorMessage="bad/unfindable check" + TaskType + " 'Product' attribute in check " + TaskType + " action !";
    if((Lot=="") && m_strInsertionShortErrorMessage.isEmpty())
        m_strInsertionShortErrorMessage="bad/unfindable check" + TaskType + " 'Lot' attribute in check " + TaskType + " action !";

    if(!m_strInsertionShortErrorMessage.isEmpty())
    {
        lStatus = Failed;
        ExecuteShellOnError(Shell.replace("$Status", "0"));
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR", entryPoint + ": task terminated with error", m_strInsertionShortErrorMessage);
        return lStatus;

    }
    QFile LogFile;
    QTextStream LogTS; QBuffer buffer;
    if (!LogFileName.isEmpty())
    {
        QFileInfo fi(LogFileName);
        QDir d; d.mkpath(fi.absolutePath());

        LogFile.setFileName(LogFileName);
        if (!LogFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            m_strInsertionShortErrorMessage=QString("can't open logfile '%1'").arg(LogFileName);
            lStatus = Delay;
            ExecuteShellOnError(Shell.replace("$Status", "0"));
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("WARNING", entryPoint + ": task delayed", m_strInsertionShortErrorMessage);
            return lStatus;
        }
        else
            LogTS.setDevice(&LogFile);

        if(LogTS.device()==NULL)
        {
            // log to our temp folder just in case of
            QString lPath=GS::Gex::Engine::GetInstance().Get("TempFolder").toString();
            QDir lDir(lPath);
            LogFile.setFileName(lPath + QDir::separator() + TaskType + "log.txt");
            if ( (!lDir.mkpath(lPath)) || (!LogFile.open(QIODevice::WriteOnly | QIODevice::Text)) )
                LogTS.setDevice(&buffer);
            else
                LogTS.setDevice(&LogFile);
        }
    }

    if(!LogFileName.isEmpty())
    {
        LogTS << "#### " + TaskType + " log file ##################################################" << endl;
        LogTS << "Date," << GS::Gex::Engine::GetInstance().GetClientDateTime().toString(Qt::ISODate) << endl;
        LogTS << "YieldManRev," << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
        LogTS << "TriggerFile," << TriggerFileName << endl;
        LogTS << "" << endl;
    }


    dbKeysContent.Set("TriggerFile", TriggerFileName);
    QList<CGexMoTaskStatisticalMonitoring *> lTasks =
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductStatMonTask(dbKeysContent);

    if(lTasks.isEmpty() || lTasks.count() > 1)
    {
        if(lTasks.isEmpty())
        {
            m_strInsertionShortErrorMessage = QString("No enabled " + TaskType + " rule found for Product '%1' in TDR %2[%3] %4 ")
                    .arg(Product).arg(DatabaseName).arg(TestingStage).arg(!RuleName.isEmpty() ? "called '"+RuleName+"'" : "");
        }
        else if(lTasks.count() > 1)
        {
            m_strInsertionShortErrorMessage = QString("More than 1 " + TaskType + " rule found for Product '%1' in TDR %2[%3] %4 ")
                    .arg(Product).arg(DatabaseName).arg(TestingStage).arg(!RuleName.isEmpty() ? "called '"+RuleName+"'" : "");
        }

        if(!LogFileName.isEmpty())
        {
            LogTS << "### " + TaskType + " Execution ERROR #######################################" << endl;
            LogTS << m_strInsertionShortErrorMessage  << endl;
            LogTS.setDevice(0); // will flush
            LogFile.close();
        }
        lStatus = Failed;
        ExecuteShellOnError(Shell.replace("$Status", "0"));
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR", entryPoint + ": task terminated with error", m_strInsertionShortErrorMessage);
        return lStatus;
    }

    CGexMoTaskStatisticalMonitoring * ptTask = lTasks.first();

    QString lTaskStatus = GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteStatMonTask(ptTask, dbKeysContent);
    if(!lTaskStatus.isEmpty() && !lTaskStatus.startsWith("ok",Qt::CaseInsensitive))
    {

        m_strInsertionShortErrorMessage=QString(TaskType + " Check failed : %1 !").arg(lTaskStatus);
        GSLOG(SYSLOG_SEV_ERROR, m_strInsertionShortErrorMessage.toLatin1().data());
        if(!LogFileName.isEmpty())
        {
            LogTS << "### " + TaskType + " Execution ERROR #######################################" << endl;
            LogTS <<  m_strInsertionShortErrorMessage << endl;
            LogTS << "#\n";
            LogTS << "# " << TriggerSource.replace("\n","\n# ") << endl;
            LogTS.setDevice(0); // will flush
            LogFile.close();
        }
        if(lTaskStatus.startsWith("delay",Qt::CaseInsensitive))
        {
            lStatus = Delay;
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("WARNING", entryPoint + ": task delayed", m_strInsertionShortErrorMessage);
        }
        else
        {
            lStatus = Failed;
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR", entryPoint + ": task terminated with error", m_strInsertionShortErrorMessage);
        }
        ExecuteShellOnError(Shell.replace("$Status", "0"));
        return lStatus;
    }

    QString lTaskSummary = dbKeysContent.Get(TaskType + "Summary").toString();
    if(dbKeysContent.Get(TaskType + "Status").toString().toUpper() == "ALARM")
    {
        m_strInsertionShortErrorMessage = lTaskStatus.section(":",1).simplified();
        if(!LogFileName.isEmpty())
        {
            LogTS << "### " + TaskType + " Execution Status #######################################" << endl;
            LogTS << TaskType + " ALARM,";
            LogTS << (lTaskSummary.contains("critical")?"critical":"standard") << "\n";
            LogTS << endl;
        }
        Shell.replace( "$FailStatus", (lTaskSummary.contains("critical")?"critical":"standard") );
    }
    else
    {
        m_strInsertionShortErrorMessage="ok";
        if(!LogFileName.isEmpty())
        {
            LogTS << "### " + TaskType + " Execution Status #######################################" << endl;
            LogTS << TaskType + " ALARM, none\n";
            LogTS << endl;
        }
    }

    if(!LogFileName.isEmpty())
    {
        LogTS << lTaskSummary.replace("\n","#endl#").simplified().replace(":",",").replace("#endl#","\n") << endl;
        LogTS << "### Trigger Source #######################################" << endl;
        LogTS << "# " << TriggerSource.replace("\n","\n# ") << endl;
        LogTS.setDevice(0); // will flush
        LogFile.close();
    }
    LaunchShellScript(Shell);

    GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO", entryPoint + ": task terminated successfully", m_strInsertionShortErrorMessage);
    return Passed;
}
