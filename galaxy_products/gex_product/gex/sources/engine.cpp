#include <gtl_core.h> // for GTL_DEFAULT_SERVER_PORT
#include "engine.h"
#include "csl/csl_engine.h"
#include "engine_private.h"
#include "gex_version.h"
#include "gex_errors.h"
#include "product_info.h"
#include "gex_scriptengine.h"
#include "gex_shared.h"
#include "cryptofile.h"
#include "license_provider_profile.h"
#include "gqtl_filelock.h"
#include "admin_engine.h"
#include "script_wizard.h"
#include "gex_pixmap_extern.h"
#include "gex_errors.h"
#include "gex_report.h"
#include "report_options.h"
#include "pat_info.h"
#include "message.h"
#include "license_provider_common.h"
#include "license_provider_manager.h"
#include "license_provider.h"
#include "user_input_filter.h"
#include "pat_recipe_io.h"
#include "pat_recipe.h"
#include "dir_access_base.h"

#ifndef GSDAEMON
    #include "browser_dialog.h"
    #include <QMessageBox>
#else
    #include "daemon.h"
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <tlhelp32.h>
    #undef CopyFile
#endif

#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScriptEngineDebugger>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>
#include <QProgressBar>
#include <QDateTime>

#include <sys/stat.h>

// ********** TO BE REMOVED ***********
#include "pickuser_dialog.h"

#ifndef GSDAEMON
    extern GexMainwindow*       pGexMainWindow;
#endif

// Script engine has to be moved as a member of the GS::Gex::Engine
extern CReportOptions       ReportOptions;
extern GexScriptEngine *	pGexScriptEngine;
extern QString				strIniClientSection;
//extern CPatInfo *           pPatInfo;

extern bool     UpdateGexScriptEngine(CReportOptions * pReportOptions); // todo : move me in engine
extern QString  LaunchShellScript(QString &strCommand);
extern QString  LaunchQProcess(QProcess*process, const QString &program, const QString& arguments, QByteArray& output, QByteArray& error, bool synchrone, int msecond = 3000);


extern CGexReport *         gexReport;

namespace GS
{
namespace Gex
{

Engine* Engine::mInstance=0;

Engine::Engine(QObject* parent): QObject(parent)
{
    setObjectName("GSEngine");
    mPrivate= new EnginePrivate(this);
    GSLOG(SYSLOG_SEV_NOTICE, "Init Gex engine...");
    // Client node is initializing
    SetClientState(eState_Init);
    launchScriptProcess = new QProcess(this);
}

/******************************************************************************!
 * \fn OnExit
 ******************************************************************************/
bool Engine::OnExit()
{
    GSLOG(3, "Engine::OnExit");
    return Exit(EXIT_SUCCESS);
}

/******************************************************************************!
 * \fn Exit
 ******************************************************************************/
bool Engine::Exit(int lExitCode)
{
    GSLOG(5, QString("Engine Exit %1").arg(lExitCode).toLatin1().constData());
    QString strMessage = "Exiting ";
    strMessage += GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    strMessage += ".";
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );

#ifdef GSDAEMON
    GSLOG(5, QString("Stopping Admin engine...").toLatin1().data());

    bool lR=GetAdminEngine().DisconnectCurrentUser();
    GSLOG(5, QString("Disconnect Current User: %1").arg(lR).toLatin1().data() );
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && GetAdminEngine().IsActivated())
    {
        // Turn off the Sheduler loop
        GetSchedulerEngine().OnStopAllRunningTask();
        // Execute StopProcess on all DBs
        // OR Emit signal to stop insertion?
        GetSchedulerEngine().StopProcess("");
        // Wait the end of all tasks
        GetTaskManager().StopAllTask();
        GSLOG(7, QString("End of stopping DB Process").toLatin1().constData());
//        while(true)
//            if(!GetSchedulerEngine().isSchedulerRunning())
//                break;

        GSLOG(7, QString("End of stopping schedular").toLatin1().constData());
        GetAdminEngine().DisconnectNode();
    }
#else
    GetAdminEngine().DisconnectToAdminServer();
    GetAdminEngine().DeleteAdminServerConnector();
#endif

    GSLOG(5, "Sending signal exit...");
    emit sExit(lExitCode);
    QCoreApplication::processEvents();

    // Set client node status to exit
    mPrivate->mExitCode = lExitCode;
    SetClientState(GS::Gex::Engine::eState_Exit);

    // In case the application event loop is already started, exit it
    QCoreApplication::exit(mPrivate->mExitCode);

    GSLOG(5, "Engine::Exit end.");
    return true;
}

Engine::~Engine()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Deleting the engine...");

    // Remove all temporary STDF files EDITED and CREATED
    GetTempFilesManager().removeAll();
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        // If some PAT performed, ensure to clean all memory buffers
        PATEngine::GetInstance().DeleteContext();
//        if(pPatInfo)
//        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Deleting PatInfo...");
//            delete pPatInfo;
//            pPatInfo = NULL;
//        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of CReportOptions.").
          arg(CReportOptions::GetNumberOfInstances()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of CTest.").
          arg(CTest::GetNumberOfInstances()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of CTestResult.").
          arg(CTestResult::GetNumberOfInstances()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of TemporaryFile.").
          arg(TemporaryFile::GetNumberOfInstances()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of GexDatabaseQuery.").
          arg(GexDatabaseQuery::GetNumberOfInstances()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString(" still %1 instances of GexDbPlugin_Connector.").
          arg(GexDbPlugin_Connector::GetNumberOfInstances()).
          toLatin1().constData());

    // Make sure socket connection gets free'ed
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Closing LM connection before closing application.");
    GSLOG(SYSLOG_SEV_DEBUG, "emit GEXMessage::eExit");
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eExit,
                                                 "Closing application: ~Engine"));

//    GS::LPPlugin::GEXMessage lReqMessage(GS::LPPlugin::GEXMessage::eExit,QString("Closing application: ~Engine"));
//    GS::LPPlugin::LPMessage lResMessage;
//    GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->DirectRequest(lReqMessage, lResMessage);

}


void Engine::OnStart()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Engine OnStart");
    connect(&CSLEngine::GetInstance(), SIGNAL(scriptFinished(GS::Gex::CSLStatus)),
            this, SLOT(OnCSLScriptFinished(GS::Gex::CSLStatus)));
    connect(&GetSchedulerEngine(), SIGNAL(sStartTask(int)),
            this, SLOT(OnStartTask(int)));
    connect(&GetSchedulerEngine(), SIGNAL(sStoppedTask(int)),
            &GetTaskManager(), SLOT(OnStoppedTask(int)));
    connect(&GetTaskManager(), SIGNAL(sStopAllRunningTask()),
            &GetSchedulerEngine(), SLOT(OnStopAllRunningTask()));
#ifdef GSDAEMON
    if (!QObject::connect(GS::Gex::Daemon::GetInstance(), SIGNAL(sStopping()), this, SLOT(OnExit())))
    {
        GSLOG(3, "Cannot connect daemon stopping signal");
    }
    else
    {
        GSLOG(5, "Connect daemon stopping signal ok");
    }
#endif

    // Builds path+name of scripts & .ini setup file used/created by GEX
    QString lR = mPrivate->InitScriptsPath();
    if (lR.startsWith("err"))
        GSLOG(SYSLOG_SEV_ERROR, QString("InitScriptsPath failed: %1").arg(lR).toLatin1().data());

    // Init the local config file path
    mPrivate->InitLocalConfigFilePath();

    // Make sure Options definition file is correctly loaded. If so, reset options.
    if(!GetOptionsHandler().IsReady())
    {
        GSLOG(SYSLOG_SEV_CRITICAL,"Failed to start the application: options definition file is not loaded");

        // Exit application
        Message::information(Get("AppFullName").toString(),
                             "ERROR: The options definition file is not valid.\nPlease contact Quantix support at " +
                             QString(GEX_EMAIL_SUPPORT) + ".\n\nThe software will now exit!");
        Exit(EXIT_FAILURE);
        return;
    }

    ReportOptions.Reset(true);
    SetOptionsMap(ReportOptions.GetOptionsHandler().GetOptionsMap());

    // Set global Company details for Registry key creation
    QCoreApplication::setOrganizationName("GalaxySemi");
    QCoreApplication::setOrganizationDomain("galaxysemi.com");
    if(QCoreApplication::applicationName().isEmpty())
        QCoreApplication::setApplicationName("GalaxySemi software");

    // Change client state
    SetClientState(eState_Connect);
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eLicenseRequest,"Engine OnStart :OnStart"));

    // Emit a signal to tell to receiver objects like Widget, the engine ios starting
    emit sStarting();
}

/******************************************************************************!
 * \fn stdoutMessageHandler
 ******************************************************************************/
void stdoutMessageHandler(QtMsgType ,
                          const QMessageLogContext& ,
                          const QString& msg)
{
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}

/******************************************************************************!
 * \fn SetStdoutHandle
 ******************************************************************************/
void Engine::SetStdoutHandler(bool lSet)
{
    static QtMessageHandler oldHandler = NULL;

    if (lSet)
    {
        if (oldHandler == NULL)
        {
            oldHandler = qInstallMessageHandler(stdoutMessageHandler);
        }
        else
        {
            qInstallMessageHandler(stdoutMessageHandler);
        }
    }
    else if (oldHandler != NULL)
    {
        qInstallMessageHandler(oldHandler);
    }
}

/******************************************************************************!
 * \fn OnLicenseReady
 ******************************************************************************/
void Engine::OnLicenseReady()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "License is ready");

    //Option are read initiliaze Timer Here
    bool allowAutoClose = (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                           && !GS::LPPlugin::ProductInfo::getInstance()->isGTM());
    if(allowAutoClose && UserInputFilter::geLastInstance()
            && (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_CLIENT))
        UserInputFilter::geLastInstance()->initAutoCloseTimer();

    emit sReady();

    // Once the client node if fully ready, check if a script has to be launched at startup time (from command line argument)
    if((GetClientState() == eState_NodeReady) &&
            mPrivate->mCommandLineOptions.GetRunScript().isEmpty() == false)
    {
        // Run startup script : argv[2]
        // Checks if GEX data analysis scripts can be executed (only Evaluation and Server GEX can do it!)
        // also if ExaminatorWeb is running, accept running script!

        // User Scripting not allowed for GEX standalone, standard Edition...OEM versions too.
        if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
        {
            QString lMessage;

            lMessage = "This function (script execution) is not supported by this version of the ";
            lMessage += "product.\nIf you would like to upgrade your license, please contact ";
            lMessage += "quantix_sales@mentor.com for more information.";

            Message::information(Get("AppFullName").toString(), lMessage);
        }
        else
        {
            // Launch script!
            CSLEngine::GetInstance().RunArgumentScript(mPrivate->mCommandLineOptions.GetRunScript());
        }
    }

    if (!UpdateGexScriptEngine(&ReportOptions))
        GSLOG(SYSLOG_SEV_WARNING, "Failed to update Scripting Engine");

    bool lOk=false;
    int lPort=ReportOptions.GetOption("webservice", "port").toInt(&lOk);
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            || GS::LPPlugin::ProductInfo::getInstance()->isGTM()
            // for example to allow Examinator to run the WebService for example for drill-downing from Sisense
            || getenv("GS_FORCE_WEBSERVICE") )
    {
        mPrivate->mWebServer.SetScriptEngine(pGexScriptEngine);
        if (!lOk)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Cannot retrieve the desired webservice"
                  " port in the options system and/or csl");
#           ifdef GSDAEMON
                GSLOG(SYSLOG_SEV_ERROR, "Webservice port unknown. The soft is going to exit.");
                Exit(EXIT_FAILURE);
#           endif
        }
        else if (!mPrivate->mWebServer.IsListening())
        {
            QString lRes=mPrivate->mWebServer.Listen(QHostAddress::Any, lPort);
            if (lRes.startsWith("err"))
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Cannot start the web server on port %1: %2").
                      arg(lPort).arg(lRes).toLatin1().data());
#               ifdef GSDAEMON
                    GSLOG(SYSLOG_SEV_ERROR,
                      "Webservice port unknown."
                      " The soft is going to exit.");
                    Exit(EXIT_FAILURE);
#               endif
            }
            GSLOG(5, QString("WebServer successfully listening to port %1").arg(lPort).toLatin1().data() );
        }
    }

    if (!GetCommandLineOptions().GetTriggerFile().isEmpty())
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("running trigger file %1...").arg(GetCommandLineOptions().GetTriggerFile())
              .toLatin1().data());

        // "-trigger='...'"
        QFile f(GetCommandLineOptions().GetTriggerFile());
        if (!f.open(QIODevice::ReadOnly))
        {
            GSLOG(SYSLOG_SEV_CRITICAL,
                  QString("cannot open %1").arg(GetCommandLineOptions().GetTriggerFile())
                  .toLatin1().data());
            Exit(EXIT_FAILURE);
            return;
        }

        QDomDocument doc;
        QString errorMsg;
        int errorLine, errorColumn=0;
        if (!doc.setContent(&f, &errorMsg, &errorLine, &errorColumn))
        {
            QString error = QString("file %1 is not xml compliant (line %2 col %3) : %4 !")
                    .arg(GetCommandLineOptions().GetTriggerFile())
                    .arg(errorLine).arg(errorColumn).arg(errorMsg);

            GSLOG(SYSLOG_SEV_CRITICAL, error.toLatin1().data());
            f.close();

            Exit(EXIT_FAILURE);
            return;
        }

        QDomElement docElem = doc.documentElement();
        if (docElem.nodeName()!="GalaxyTrigger")
        {
            GSLOG(SYSLOG_SEV_CRITICAL,
                  QString("GalaxyTrigger tag unfindable : found '%1'").arg(docElem.nodeName())
                  .toLatin1().data());

            Exit(EXIT_FAILURE);
            return;
        }
        bool ok=false;
        float version=docElem.attribute("Version").toFloat(&ok);
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("gtf file version is %1").arg(version).toLatin1().constData());

        if ( (!ok) || (version==0.0f) || (version<(float)GEX_MIN_GTF_VERSION) ||
             (version>(float)GEX_MAX_GTF_VERSION) )
        {
            GSLOG(SYSLOG_SEV_CRITICAL,
                  QString("illegal gtf version : should be between %1 and %2")
                  .arg(GEX_MIN_GTF_VERSION).arg(GEX_MAX_GTF_VERSION).toLatin1().constData());

            Exit(EXIT_FAILURE);
            return;
        }

        QString JSAction=docElem.attribute("JSAction");
        if (!JSAction.isEmpty())
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("JS action: Starting with %1").arg(JSAction).toLatin1().data());

            // Force to run Yield-Man Monitoring Scheduler !!!!!!!
            GetSchedulerEngine().stopScheduler(false);

            QScriptValue scriptValue = pGexScriptEngine->evaluate(JSAction, Get("TempFolder").toString() +
                                                                  QDir::separator()+"js_log.txt" );

            if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("cannot evaluate '%1' : '%2'").arg(JSAction)
                      .arg(pGexScriptEngine->uncaughtException().toString()).toLatin1().data());
                return;
            }

            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("JS action: '%1' returned %2").arg(JSAction)
                  .arg(scriptValue.toBool() ? "true" : "false").toLatin1().constData());
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "Unfindable or empty JSAction");
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Each time the solution is launched, update the Intranet web site so to always be up-to-date.
        GetSchedulerEngine().ExecuteStatusTask();
        // Force Running status at startup: so no task is taking place when launched in GUI mode.
        //pGexMoScheduler->RunTaskScheduler(true);
        // Allow to start in Pause if WelcomePage if true
        GetSchedulerEngine().RunTaskScheduler(GetCommandLineOptions().IsWelcomeBoxEnabled() == false);
    }

    if (GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
#ifndef GCORE15334
        GSLOG(SYSLOG_SEV_ERROR, "Feature not available in this version, please contact support for more informations");
        Exit(EXIT_FAILURE);
        return;
#else
        if (!mPrivate->mTesterServer.isListening())
        {
            int portnumber = GTL_DEFAULT_SERVER_PORT;
            // Check if port specified in env var
            char *ptChar = getenv("GTM_PORT");
            if(ptChar != NULL)
            {
                int nTmpPortNb=GTL_DEFAULT_SERVER_PORT;
                // Get port nb
                if(sscanf(ptChar,"%d",&nTmpPortNb) == 1)
                    portnumber = nTmpPortNb;
            }

            QString lRes=mPrivate->mTesterServer.Start(portnumber);
            if (lRes.startsWith("err"))
            {
                Message::critical(Get("AppFullName").toString(),
                                  "The software cannot listen the port "+QString::number(portnumber) + ".\n"+
                                  "Another application is probably using it. \nThe software will now exit.");
                GSLOG(3, lRes.toLatin1().constData());
                // emit sExit(); // will be called by Exit
                Exit(GS::Error::CANNOT_START_WEBSERVICE); // EXIT_FAILURE
                return;
            }
            GSLOG(5, QString("TesterServer successfully listening to port %1").
                  arg(portnumber).toLatin1().constData());
        }
#endif
    }

    QDir lOnstartFolder("scripts/onstart");
    QStringList lJSFiles = lOnstartFolder.entryList(QDir::nameFiltersFromString("*.js"), QDir::Files);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 js files found in onstart folder").arg(lJSFiles.count()).toLatin1().data() );
    for (int i=0; i<lJSFiles.size(); i++)
    //foreach(QString lFileName, lJSFiles)
    {
        QString lFileName=lJSFiles.at(i);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Evaluating '%1'...").arg(lFileName).toLatin1().data());
        QFile lFile("scripts/onstart/"+lFileName);
        if (lFile.open(QIODevice::ReadOnly))
        {
            QScriptValue lSV=pGexScriptEngine->evaluate(lFile.readAll());
            if (pGexScriptEngine->hasUncaughtException())
                GSLOG(SYSLOG_SEV_WARNING, QString("Exception in %1: %2 line %3")
                      .arg(lFileName).arg(pGexScriptEngine->uncaughtException().toString())
                      .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                      .toLatin1().data() );
            lFile.close();
        }
        else
            GSLOG(SYSLOG_SEV_WARNING,QString("Cannot open '%1'").arg(lFileName).toLatin1().data() );
    }


    if (pGexScriptEngine && QFile::exists(Get("ApplicationDir").toString()+"/scripts/on_start.js"))
    {
        QFile scriptFile(Get("ApplicationDir").toString()+"/scripts/on_start.js" );
        if (scriptFile.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&scriptFile);
            QString contents = stream.readAll();
            scriptFile.close();
            QScriptValue v=pGexScriptEngine->evaluate(contents);
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Execution of on_start.js script : %1").arg(v.toString()).toLatin1().constData());
            if (pGexScriptEngine->hasUncaughtException())
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Exception line %1: %2").arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                      .arg(pGexScriptEngine->uncaughtException().toString())
                      .toLatin1().data());
        }
    }
}

void Engine::OnLicenseGranted()
{
    /**
     * Modified method for the GCORE-11262, we need to make sure we didn't break something by getting out some code
     * responsible of changing the client state from eGrantedLicense to eNodeReady
     */
    mPrivate->mlicenseReleaseRequest = false;

    // Initialization startup scripts sequence not yet done, execute required startup scripts
    if (mPrivate->mInitializationSequenceDone == false)
    {
        emit sLicenseGranted();

        // Check if we have to exit the application (no license granted, wrong starting mode...)
        if(GetClientState() == eState_Exit)
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Client state is exit. Closing and quiting...");
            Exit(EXIT_FAILURE);         // will call ExitGexApp through signal...
            return;
        }

        // Load profile CSL
        bool lFinalStartupScriptExecuted=false;
        if(!GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            if (!GetCommandLineOptions().GetProfileScript().isEmpty())
            {
                CSLEngine::GetInstance().RunStartupScript(GetCommandLineOptions().GetProfileScript());
                lFinalStartupScriptExecuted = true;
            }
            else if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                // no profile
            }
            else
            {
                // Admin Server
                // Update the database with current files saved
                // Synchronize with YieldManDB profiles
                if(IsAdminServerMode(true))
                    GetAdminEngine().SynchronizeProfiles(GetDefaultUserProfile());

                // Check if multi-user options available.
                // If so, ask which one to load...unless running in hidden mode
                if(IsExistingDefaultProfile())
                {
                    // Get default username
                    QString lUserScript = GetDefaultProfileScript();

                    SetStartupScript(lUserScript);

                    // Load options: run startup script : $HOME/.<profile>.csl
                    CSLEngine::GetInstance().RunStartupScript(lUserScript);

                    lFinalStartupScriptExecuted = true;
                }
                else
                {
                    QStringList lProfiles = GetProfiles();

                    if (lProfiles.count() == 1)
                    {
                        // Single custom profile, use it!!
                        QString lUserScript = lProfiles.first();

                        SetStartupScript(lUserScript);

                        // Load options: run startup script : $HOME/.<profile>.csl
                        CSLEngine::GetInstance().RunStartupScript(lUserScript);

                        lFinalStartupScriptExecuted = true;
                    }
                    else if((GetCommandLineOptions().IsHidden() == false) && lProfiles.count() > 1)
                    {
    #ifndef GSDAEMON
                        PickUserDialog cPickUser(pGexMainWindow);

                        // Set GUI in 'Load' mode
                        cPickUser.setLoadMode(true);

                        // Display dialog box.
                        if(cPickUser.exec() == 1)
                        {
                            // Get username selected
                            QString lUserName;
                            QString lUserScript = GetStartupScript();
                            cPickUser.getSelectedUserName(lUserName, lUserScript);

                            SetStartupScript(lUserScript);

                            // Load options: run startup script : $HOME/.<profile>.csl
                            CSLEngine::GetInstance().RunStartupScript(lUserScript);

                            lFinalStartupScriptExecuted = true;
                        }
    #endif
                    }
                }
            }
        }

        // If final startup script not executed (GTM, hide mode...), execute it
        if(!lFinalStartupScriptExecuted)
        {
            CSLEngine::GetInstance().RunStartupScript(GetStartupScript());
            lFinalStartupScriptExecuted=true;
        }

        // If Running in Client mode, activate reqired engines
        QString lRes=mPrivate->ActivateEngines();
        if (lRes.startsWith("err"))
        {
            GSLOG(3, lRes.toLatin1().constData());
            Exit(EXIT_FAILURE);         // will call ExitGexApp through signal...
            return;
        }

        mPrivate->mInitializationSequenceDone = true;
    }

    // Once the client node if fully ready AND after all databases were loaded,
    // Script (from command line argument) has to be launched at startup time ()
    // Try to load database entry now
    GetDatabaseEngine().LoadDatabasesListIfEmpty();

    // Active-Passive license
    // Received an license
    // In Active Mode => Node Ready
    // In Passive Mode => License Granted
    if(!mPrivate->mlicensePassive)
    {

        // Node is fully ready (License granted, all startup scripts executed!!)
        SetClientState(eState_NodeReady);
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(EVENT_LICENSE_READY)));
    }
}

/*
void Engine::OnClose()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Engine OnClose");
}
*/

bool Engine::Destroy()
{
    if (mInstance)
    {
#ifndef Q_OS_MAC

        if(GS::LPPlugin::LicenseProviderManager::getInstance())
        {
            GS::LPPlugin::LicenseProviderManager::getInstance()->stopLicensingThread();
            GS::LPPlugin::LicenseProviderManager::getInstance()->waitUntilThreadFinshed();
        }
        mInstance->FreeLicenseProvider();
#endif
        delete mInstance;
    }
    mInstance=0;
    return true;
}

bool Engine::RemoveFileFromDisk(const QString &lFileName)
{
    bool lStatus = true;
    int lResult = -1;

    // Check if the file exists
    // If not, nothing to do
    if(!QFile::exists(lFileName))
        return true;

    // First make sure the file is not read-only!
#if defined unix || __MACH__
    lResult = chmod(lFileName.toLatin1().constData(),0777);
#else
    lResult = _chmod(lFileName.toLatin1().constData(), _S_IREAD | _S_IWRITE);
#endif

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Deleting file %1").arg(lFileName).toLatin1().data());

    // Erase file
    lStatus = QFile::remove(lFileName);
    if(!lStatus &&  (lResult != 0))
    {
        // display the message only if we cannot remove the file
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Cannot change file permissions %1").arg(lFileName).toLatin1().data());
    }

    return lStatus;
}

Engine& Engine::GetInstance()
{
    if (mInstance)
        return *mInstance;
    // Even if the parent is the app, the Engine is not deleted at exit...
    mInstance=new Engine(QCoreApplication::instance());
    /*
    // Let s try to connect to deleteLater, Destroy() ?
    // does not work bacause QApplication not yet ready...
    bool b=QObject::connect(QCoreApplication::instance(),
                            SIGNAL(aboutToQuit()),
                            mInstance,
                            //SLOT(deleteLater())
                            SLOT(Destroy())
                            );
    if (!b)
        GSLOG(SYSLOG_SEV_WARNING, "Get instance: cannot connect aboutToQuit with Destroy");
    */
    return *mInstance;
}

void Engine::customEvent(QEvent *e)
{
    if(e->type()!=1004)
        GSLOG(6, QString("customEvent type %1").arg(e?e->type():-1).toLatin1().data() );
    if(e->type() == QEvent::Type(EVENT_START))
        OnStart();
    else if(e->type() == QEvent::Type(EVENT_LICENSE_GRANTED))
        OnLicenseGranted();
    else if(e->type() == QEvent::Type(EVENT_LICENSE_READY))
        OnLicenseReady();
    else if(e->type() == QEvent::Type(EVENT_USER_ACTIVITY))
        SetLastUserActivity();
    else if(e->type() == QEvent::User+10)
    {
        // Received Pause
        GSLOG(4, "Custom event User+10 (Pause) received");
    }
    else if(e->type() == QEvent::User+20)
    {
        GSLOG(4, "Custom event User+20 (Resume) received");
        // Received Resume
    }
}

QString Engine::OverloadUserFolder()
{
    return mPrivate->OverloadUserFolder();
}

QNetworkAccessManager& Engine::GetNAM()
{
    return mPrivate->mNetworkAccessManager;
}
QNetworkCookieJar&  Engine::GetNetworkCookieJar()
{
    return mPrivate->mNetworkCookieJar;
}
QScriptEngineDebugger& Engine::GetScriptEngineDebbugger()
{
    return mPrivate->mScriptEngineDebugger;
}

SchedulerEngine& Engine::GetSchedulerEngine()
{
    return mPrivate->mSchedulerEngine;
}

DatabaseEngine& Engine::GetDatabaseEngine()
{
    return mPrivate->mDatabaseEngine;
}

SYAEngine& Engine::GetSYAEngine()
{
    return mPrivate->mSYAEngine;
}

SPMEngine& Engine::GetSPMEngine()
{
    return mPrivate->mSPMEngine;
}

TemporaryFilesManager& Engine::GetTempFilesManager()
{
    return mPrivate->mTempFilesManager;
}

TasksManagerEngine &Engine::GetTaskManager()
{
    return mPrivate->mTasksManager;
}

ReadSystemInfo&	Engine::GetSystemInfo()
{
    return mPrivate->mSystemInfo;
}

AdminEngine& Engine::GetAdminEngine()
{
    return mPrivate->mAdminEngine;
}

#ifdef GCORE15334
TesterServer&  Engine::GetTesterServer()
{
    return mPrivate->mTesterServer;
}
#endif

const OptionsHandler &Engine::GetOptionsHandler() const
{
    return mPrivate->mOptionsHandler;
}

QVariant Engine::Get(const QString s)
{
    QVariant &lV=mPrivate->Get(s);

    return lV;
}

bool Engine::Set(const QString key, QVariant value)
{
    QVariant &v=mPrivate->Set(key, value);
    return v.isValid();
}

QVariant Engine::ExecProcess(const QString &program, const QString& arguments, bool synchrone, int msecond)
{
    // Let s check the first arg, the exec to launch really exist ?
    if (program.isEmpty())
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecProcess: terminated in error", "program empty");
        return "error : program empty";
    }

    if(synchrone)
    {
        GSLOG(5, QString("Executing : %1 in synchronous mode. Timoute set to %2 mseconds").arg(program).arg( QString::number(msecond)).toLatin1().data() );
    }
    else
    {
        GSLOG(5, QString("Executing : %1 in asynchronous mode. No logs avalaible").arg(program).toLatin1().data() );
    }

    QByteArray output, error;
    QString lRes =  LaunchQProcess(launchScriptProcess, program, arguments, output, error, synchrone,  msecond);

    QString lOutput = QString(output);
    QString lError = QString(error);
    if(lOutput.isEmpty() == false)
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Script Log: %1").arg(QString(output)).toLatin1().data() );
    if(lError.isEmpty() == false)
        GSLOG(SYSLOG_SEV_ERROR, QString("Script Error Log: %1").arg(QString(error)).toLatin1().data() );

    if(lRes=="ok")
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","ExecProcess: terminated with no error", lOutput);
    }
    else
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecProcess: terminated in error", lError);
    }
    return lRes;

}

QVariant Engine::ExecShellScript(const QString& lLine, bool /*assynchrone*/)
{
    // Let s check the first arg, the exec to launch really exist ?
    if (lLine.isEmpty())
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecShellScript: terminated in error", "command empty");
        return "error : command empty";
    }

    // The QProcess does not behave like OS native function specially ShellExecuteA(...) on win32
    // Consequently, let s use LaunchShellScript for the moment
    QString lCommand=lLine;
    QString execResult = LaunchShellScript(lCommand);
    if(execResult == "ok")
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","ExecShellScript: terminated with no error", "");
    }
    else
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecShellScript: terminated in error", execResult);
    }
    return execResult;
}

QString Engine::GetLastAccessedFodler()
{
    if (mPrivate->mLastAccessedFolder.isEmpty())
        mPrivate->mLastAccessedFolder = mPrivate->mUserFolder.toString();
    return mPrivate->mLastAccessedFolder;
}

void Engine::UpdateLastAccessedFolder(const QString &folder)
{
    mPrivate->mLastAccessedFolder = folder;
}

/*
QString Engine::GetTempFolder()
{
    return mPrivate->mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp";
}
*/

const QString &Engine::GetAssistantScript() const
{
    return mPrivate->mAssistantScript;
}

const QString &Engine::GetStartupScript() const
{
    return mPrivate->mStartupScript;
}

const QString &Engine::GetOldTasksFileName() const
{
    return mPrivate->mMoTasksFile;
}

const QString &Engine::GetTasksXmlFileName() const
{
    return mPrivate->mMoTasksXmlFile;
}

const QString &Engine::GetLocalConfigFileName() const
{
    return mPrivate->mLocalConfigFile;
}

CommandLineOptions &Engine::GetCommandLineOptions()
{
    return mPrivate->mCommandLineOptions;
}

const QDateTime &Engine::GetLastUserActivity() const
{
    return mPrivate->mLastUserActivity;
}

QStringList Engine::GetProfiles()
{
    // Try to load a user profile
    QDir    lDir(Get("UserFolder").toString());

    // Non-recursive: ONLY import this folder
    lDir.setFilter(QDir::Files);

    QStringList nameFilters;
    QString nameFilter("*_user_profile.csl");
    nameFilters.push_back(nameFilter);

    // Find all files with name <name>_user_profile.csl
    QStringList lProfiles = lDir.entryList(nameFilters);
    QStringList lProfilesPath;
    for (int lIt=0; lIt<lProfiles.size(); ++lIt)
    {
        lProfilesPath.append(QDir::cleanPath(
                                 lDir.absolutePath() +
                                 QDir::separator() +
                                 lProfiles.at(lIt)));
    }

    return lProfilesPath;
}

QString Engine::GetDefaultProfileScript()
{
    QString lUserProfile = GetDefaultUserProfile();
    QString lProfile;

    if (lUserProfile.isEmpty() == false)
    {
        // Build full file name from User name
        lProfile  = Get("UserFolder").toString() + "/";
        lProfile += lUserProfile + "_user_profile.csl";
    }

    return lProfile;
}

QString Engine::GetDefaultUserProfile()
{
    char    lBuffer[256+1];
    QString lName;

    get_private_profile_string("Startup", "profile", "", lBuffer, 256,
                               GetLocalConfigFileName().toLatin1().constData());
    lName = lBuffer;

    return lName;
}

bool Engine::IsExistingDefaultProfile()
{
    return QFile::exists(GetDefaultProfileScript());
}

void Engine::SetLastUserActivity()
{
    QDateTime lCurrentDate = GS::Gex::Engine::GetInstance().GetClientDateTime();
    // Only every 1s
    if(mPrivate->mLastUserActivity.addSecs(1) < lCurrentDate)
    {
        mPrivate->mLastUserActivity = lCurrentDate;
        if(GetClientState() == eState_NodeReady)
            activityNotification("User Active");
    }
}

void Engine::SetStartupScript(const QString &scriptName)
{
    mPrivate->mStartupScript = scriptName;
}

bool Engine::IsAdminServerMode(bool userConnected /*= false*/)
{
    // Check if we are connected to GexLm
    if(!HasNodeValidLicense())
        return false;

    // If YieldManDb is activated
    if(GetAdminEngine().IsActivated()
            &&  GetAdminEngine().ConnectToAdminServer()
            &&  (!userConnected || GetAdminEngine().IsUserConnected()))
        return true;

    return false;
}

bool Engine::HasTasksRunning()
{
    return (GetTaskManager().RunningTasksCount() > 0);
}

///////////////////////////////////////////////////////////
// Return a common DateTime (MySql server DateTime or local if not)
///////////////////////////////////////////////////////////
QDateTime Engine::GetServerDateTime()
{
    if(GetAdminEngine().IsActivated())
        return GetAdminEngine().GetServerDateTime();
    return QDateTime::currentDateTime();
}

QDateTime Engine::GetClientDateTime()
{
    return QDateTime::currentDateTime();
}

QDate Engine::GetExpirationDate()
{
    return Get("ExpirationDate").toDate();
}

void Engine::SetExpirationDate(const QDate &d)
{
    // Add 1 day because the expiration date is the day at 00::00:00 and it is expected to be 23:59:59
    Set("ExpirationDate", d);
}

void Engine::SetMaintenanceExpirationDate(const QDate &d)
{
    // Add 1 day because the expiration date is the day at 00::00:00 and it is expected to be 23:59:59
    Set("MaintenanceExpirationDate", d);
}

QDate Engine::GetMaintenanceExpirationDate()
{
    return Get("MaintenanceExpirationDate").toDate();
}

bool Engine::SetOptionsMap(const OptionsMap &optionsMap)
{
    return mPrivate->mOptionsHandler.SetOptionsMap(optionsMap);
}

#ifdef VFEI_SERVER
void Engine::StartVFEIServer()
{
    if (mPrivate->mVFEIServer.IsRunning() == false)
    {
        // Start Server socket listener.
        QString lVFEIConfigFile = Get("ApplicationDir").toString() + "/patman_vfei.conf";

        if (mPrivate->mVFEIServer.Init(lVFEIConfigFile))
        {
            if (mPrivate->mVFEIServer.Start() == false)
                exit(EXIT_FAILURE);
        }
    }
}
#endif

QString Engine::Download(QString url, QString targetfile)
{
    QNetworkRequest nr;
    nr.setUrl(QUrl(url));
    QNetworkReply* reply=GetNAM().get(nr);
    if(!reply)
        return "error";
    if (reply->error()!=QNetworkReply::NoError)
        return "error"+reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    while (!reply->isFinished()) // reply->isFinished() or isRunning() never ends...
    {
        QCoreApplication::instance()->QCoreApplication::processEvents();
        //QThread::currentThread()->wait(100);
    }

    if (nr.url().scheme()=="http")
    {
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==404)
            return "error: "+reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    }
    QFile f(targetfile);
    if (!f.open(QIODevice::WriteOnly))
        return "error";
    f.write(reply->readAll());
    f.close();
    return "ok";
}

QString Engine::NetworkGet(const QString &url)
{
    return mPrivate->NetworkGet(url);
    //GSLOG(6, QString("Network get %1").arg(url).toLatin1().data());
    //return "error : code me";
}

QString Engine::NetworkPost(const QString &url, const QString &data, const QString &contentType)
{
    QString execRes = mPrivate->NetworkPost(url, data, contentType);
    if(execRes=="ok")
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","NetworkPost: terminated with no error", execRes);
    }
    else
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","NetworkPost: terminated in error", execRes);
    }
    return execRes;
}


QString	Engine::UpdateProductStringName()
{
    return mPrivate->UpdateProductStringName();
}

QString Engine::LoginToWeb(const QString &url, const QString &post_data)
{
    return mPrivate->LoginToWeb(url, post_data);
}

int Engine::CheckForLicense(void)
{
    // Check if expiration date reached
    // Or if STDF file is too recent
    QDate CurrentDate = QDate::currentDate();
    if((CurrentDate > GetInstance().GetExpirationDate()) /*|| (CurrentDate < GS::Gex::Engine::GetInstance().GetReleaseDate())*/)
    {
        Message::information(Get("AppFullName").toString(),
                             "Your License has expired, you need to renew it...\nPlease contact " +
                             QString(GEX_EMAIL_SALES));
        return GS::StdLib::Stdf::TooRecent; // Refuse to process STDF file too recent!
    }

    if(GS::Gex::Engine::GetInstance().GetClientState() != GS::Gex::Engine::eState_NodeReady)
    {
        QString strMessage = "Waiting for client to be ready (ClientState = " + GS::Gex::Engine::GetInstance().GetClientStateName()+")";
        Message::information(Get("AppFullName").toString(),
                             strMessage);
        return GS::StdLib::Stdf::RunningRestriction;
    }

    return GS::StdLib::Stdf::NoError;
}



void Engine::UpdateProgressStatus(bool lSet, int lRange, int lStep, bool lShow /*= true*/)
{
    emit sProgressStatus(lSet, lRange, lStep, lShow);
}

void Engine::UpdateLabelStatus(const QString &lMessage /*= ""*/)
{
    emit sLabelStatus(lMessage);
}

void Engine::HideProgress()
{
    emit sHideProgress();
}

void Engine::OnCSLScriptFinished(const GS::Gex::CSLStatus& lStatus)
{
    if (lStatus.IsFailed() == false && lStatus.IsStartup() == true)
        SetOptionsMap(ReportOptions.GetOptionsHandler().GetOptionsMap());

    // Report handle not available yet!
    if(gexReport)
    {
        // Check if must display an error message...unless running mode is HIDE!
        switch(gexReport->iScriptExitCode)
        {
        default:
        case GS::StdLib::Stdf::NoError:
            break;

        case GS::StdLib::Stdf::FileOpened:
            Message::information(Get("AppFullName").toString(),
                                 "Error opening file");
            break;

        case GS::StdLib::Stdf::FileClosed:
            Message::information(Get("AppFullName").toString(),
                                 "Error closing file");
            break;

        case GS::StdLib::Stdf::ErrorOpen:
            Message::information(Get("AppFullName").toString(),
                                 "Error opening/creating file");
            break;

        case GS::StdLib::Stdf::ErrorMemory:
            Message::information(Get("AppFullName").toString(),
                                 "Memory allocation failure");
            break;

        case GS::StdLib::Stdf::WriteError:
            Message::information(Get("AppFullName").toString(),
                                 "Error writing to file");
            break;

        case GS::StdLib::Stdf::ReportFile:
            Message::information(Get("AppFullName").toString(),
                                 "Can't create report file (output file).\nPossible cause: Disk is full, disk quota reached, folder write protected...");
            break;

        case GS::StdLib::Stdf::TooRecent:
            Message::information(Get("AppFullName").toString(),
                                 "Your License has expired, you need to renew it...\nPlease contact " +
                                 QString(GEX_EMAIL_SALES));
            break;

        case GS::StdLib::Stdf::PlatformRestriction:
            Message::information(Get("AppFullName").toString(),
                                 "Your License is restricted to only process STDF files\nfrom a specific tester brand and OS revision.\nTo remove this limitation and analyse STDF files from any tester brand,\nyou need to upgrade your license.\n\nPlease contact "+QString(GEX_EMAIL_SALES));
            // Delete all data structure so that even in In teractive mode, uer can't see data!
            delete gexReport;
            gexReport = NULL;
            break;

        case GS::StdLib::Stdf::RunningRestriction:
            Message::information(Get("AppFullName").toString(),
                                 "This is a 'Standalone' GEX release: it doesn't allow scripting execution.\nTo run user scripts, you need to upgrade your Quantix Examinator.\n\nPlease contact "+QString(GEX_EMAIL_SALES));
            break;
        }

        if (lStatus.IsFailed() == false)
            gexReport->setCompleted(true);
    }

    // Check if command line argument for closing application once script argv[2] (not startup setup script!) is done.
    if((GetClientState() == eState_NodeReady)
            && (mPrivate->mCommandLineOptions.CloseAfterRunScript() == true)
            && (lStatus.IsStartup() == false))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "emit GEXMessage::eExit");
        emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eExit,
                                                     "CloseAfterScript true. Exiting ... : OnCSLScriptFinished"));
//        GS::LPPlugin::GEXMessage lReqMessage(GS::LPPlugin::GEXMessage::eExit,QString("Closing application: ~Engine"));
//        GS::LPPlugin::LPMessage lResMessage;
//        GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->DirectRequest(lReqMessage, lResMessage);

        GSLOG(SYSLOG_SEV_NOTICE, "CloseAfterScript true. Exiting...");
        Exit(EXIT_SUCCESS);
    }
}

void Engine::SetClientState(ClientState eNewClientState)
{
    QString strMessage = "Changing client state: " + GetClientStateName() + " => " + GetClientStateName(eNewClientState);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().constData());
    if (QString::compare(GetClientStateName(eNewClientState), "eState_LicenseGranted" ) == 0
            && QString::compare(GetClientStateName(),"eState_ReConnect") == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Reconnection success");
        // YM history log
        strMessage = "'connection opened' signal received.";
        GetSchedulerEngine().AppendMoHistoryLog("","License provider connection found", strMessage);
    }

    mPrivate->m_ePreviousClientState = mPrivate->m_eClientState;
    mPrivate->m_eClientState = eNewClientState;
}

Engine::ClientState Engine::GetClientState(void)
{
    return (ClientState)mPrivate->m_eClientState;
}

QString Engine::GetClientStateName(void)
{
    return GetClientStateName((ClientState)mPrivate->m_eClientState);
}

QString Engine::GetClientStateName(ClientState eClientState)
{
    switch(eClientState)
    {
    case eState_Init: return QString("eState_Init");
    case eState_Connect: return QString("eState_Connect");
    case eState_ReConnect: return QString("eState_ReConnect");
    case eState_LicenseGranted: return QString("eState_LicenseGranted");
    case eState_NodeReady: return QString("eState_NodeReady");
    case eState_Exit: return QString("eState_Exit");
    };

    return QString("Unknown");
}

bool Engine::HasNodeValidLicense()
{
    // Check if we have a connection with GexLm
    return ((mPrivate->m_eClientState == eState_NodeReady)
            || (mPrivate->m_eClientState == eState_LicenseGranted));
}

///////////////////////////////////////////////////////////
// Waits until client node is in one of the specified states
///////////////////////////////////////////////////////////
void Engine::WaitForClientState(unsigned int uiStates)
{
    // Loop until state reached...
    while(1)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        if(mPrivate->m_eClientState & uiStates)
            return;
    };
}

///////////////////////////////////////////////////////////
// Waits until client node is in specified state
///////////////////////////////////////////////////////////
void Engine::WaitForClientState(GS::Gex::Engine::ClientState eState)
{
    // Loop until state reached...
    while(1)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        if(mPrivate->m_eClientState == (unsigned int) eState)
            return;
    };
}

void Engine::OnStartTask(int lType)
{
    // Retrieved an id from the task manager
    GetSchedulerEngine().setLastTaskId(GetTaskManager().AddTask(lType));
}


/******************************************************************************!
 * \fn ShouldConnectAgain
 ******************************************************************************/
bool Engine::ShouldConnectAgain(const int errnb, const QString& lMessage)
{
    QString lTitle;
    switch (errnb)
    {
        case GS::Error::SERVER_CONNECTION_CLOSE:
            lTitle = "Connection closed";
            break;
        case GS::Error::NETWORK_TIMEOUT:
            lTitle = "Network Timeout";
            break;
        case GS::Error::SOFTWARE_IDLE:
            lTitle = "Software Idle";
            break;
        case GS::Error::USER_LICENSE_RELEASE:
            lTitle = "User license release";
            break;
        case GS::Error::ALL_LICENSES_USED:
            lTitle = "All licenses used";
            break;
        default:
            lTitle = "Unkown error";
    }
    GSLOG(SYSLOG_SEV_WARNING, QString("ShouldConnectAgain %1%2").arg(lMessage).arg(lTitle).toLatin1().constData());
//#ifdef GSDAEMON
    ClientState lastStableState = GetClientState();
//#endif
    if (GetClientState() == eState_NodeReady || GetClientState() == eState_ReConnect || GetClientState() == eState_LicenseGranted)
    {
        SetClientState(eState_ReConnect);
    }
    else if(GetClientState() == eState_Init || GetClientState() == eState_Connect  )
    {
        SetClientState(Engine::eState_Connect);
    }

//#else
//    // Change client state
//    if (GetClientState() == eState_NodeReady)
//    {
//        SetClientState(eState_ReConnect);
//    }
//    else
//    {
//        SetClientState(Engine::eState_Connect);
//    }
//#endif



#ifndef GSDAEMON
    if(GetCommandLineOptions().IsHidden() == true)
    {
        GSLOG(SYSLOG_SEV_DEBUG, lMessage.toLatin1().constData());
        Exit(errnb);
    }
    else
    {
        // 7961
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() && !GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            // Ask question if try to connect again
            QMessageBox mb(lTitle,
                           lMessage,
                           QMessageBox::Question,
                           QMessageBox::Yes  | QMessageBox::Default,
                           QMessageBox::No,
                           QMessageBox::NoButton,
                           pGexMainWindow);
            mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
            mb.setButtonText( QMessageBox::Yes, "&Connect again..." );
            mb.setButtonText( QMessageBox::No, "&Close application" );
            if(mb.exec() == QMessageBox::Yes)
            {
                //SetClientState(eState_Connect);
                emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eReconnect,
                                                             "Should connect again : ShouldConnectAgain"));
                // Tell will try again
                return true;
            }
            else
            {
                GSLOG(SYSLOG_SEV_DEBUG, "emit GEXMessage::eExit");
                emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eExit,
                                                             "Close application : ShouldConnectAgain"));
                // Tell will try again
                return false;
            }
        }
        else
        {
            if(lastStableState == Engine::eState_NodeReady || lastStableState == Engine::eState_ReConnect)
            {
                if(pGexMainWindow)
                {
                    pGexMainWindow->ReloadProductSkin(true);
                }
                QTimer::singleShot(30000, this, SLOT(ConnectToLPAgain()));
                return true;
            }
            else
            {
                return false;
            }
        }
    }
#else

    if(lastStableState == Engine::eState_NodeReady || lastStableState == Engine::eState_ReConnect)
    {
        QTimer::singleShot(30000, this, SLOT(ConnectToLPAgain()));
        return true;
    }
    else
    {
        return false;
    }

#endif

    // Do NOT try again
    return false;
}

void Engine::processLPMessage(const GS::LPPlugin::LPMessage &lpMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("LPMessage(%1,%2)").arg(lpMessage.getType()).arg(lpMessage.getData()).toLatin1().constData());

    if(lpMessage.getType() == GS::LPPlugin::LPMessage::eAccept)
    {
        // Active-Passive license
        // Received an license
        // In Active Mode => Node Ready
        // In Passive Mode => License Granted
        if(lpMessage.getData().contains("License:PassiveMode"))
        {
            mPrivate->mlicensePassive = true;
            // case 7663
            // Update Application Name - add/remove (Passive License)
            UpdateProductStringName();
        }

#ifndef QT_NO_DEBUG
        QVariantMap &fullLPData = GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->getFullLPData();
        foreach(const QString &key,fullLPData.keys())
        {
            qDebug()<< QString("Key (%1) => Val(%2)").arg(key).arg(fullLPData[key].toString());
        }
#endif
        SetClientState(eState_LicenseGranted);
        if(!GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("ExpirationDate").isNull())
            SetExpirationDate(GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("ExpirationDate").toDate());
        if(!GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("MaintenanceExpirationDate").isNull())
            SetMaintenanceExpirationDate(GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("MaintenanceExpirationDate").toDate());

        // No more useful since we know the product desired from the command line
//        GSLOG(SYSLOG_SEV_DEBUG, QString("UpdateProductStringName(%1,%2)")
//              .arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
//              .arg(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()).toLatin1().constData());
        UpdateProductStringName();

        // If GEX running as a Client node...do all server connection.
        switch(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode())
        {
        case GEX_RUNNINGMODE_STANDALONE:
        case GEX_RUNNINGMODE_EVALUATION:
            // Check that we don't have another instance running!
            if(!mPrivate->mlicenseReleaseRequest && mPrivate->CheckValidInstance(false) == false)
            {
                // Exit application
                GSLOG(SYSLOG_SEV_CRITICAL, "CheckValidInstance failed");
                Exit(EXIT_FAILURE);
                return;
            }

            break;

        case GEX_RUNNINGMODE_CLIENT:
            // create user lock
            if(!mPrivate->mlicenseReleaseRequest)
                mPrivate->CheckValidInstance(true);
#ifndef GSDAEMON
            if(pGexMainWindow)
            {
                pGexMainWindow->enableLicRelease();
            }
#endif
            break;
        }

        // Lock lock file if YM or PATMAN
        // Case 7536 YM and Pat Man should reconnect when loosing connection
        if( ! (mPrivate->m_ePreviousClientState == eState_ReConnect
               && mPrivate->m_eClientState == eState_LicenseGranted )
            && !mPrivate->mlicenseReleaseRequest
            && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            CGFileLock clFileLock;
            QString    strLockFile = GetInstance().Get("UserFolder").toString() + "/.gexmo_lockfile.txt";
            CGexSystemUtils::NormalizePath(strLockFile);
            QByteArray pString = strLockFile.toLatin1();
            clFileLock.SetLockFileName(pString.data());
            if(clFileLock.Lock() == 0)
            {
                // Couldn't lock lock file, an instance of the application is already running
                QString strServerMessage = "Couldn't lock lock file " + strLockFile;
                strServerMessage += ", an instance of the application is probably already running.";
                GSLOG(SYSLOG_SEV_EMERGENCY,strServerMessage.toLatin1().data());

                if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
                    Message::information(Get("AppFullName").toString(),
                                         "An instance of the application is already running.\nOnly one instance of PAT-Man can be run on a system.\nContact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+" for more details.\n\nGEX will now exit.");
                else
                    Message::information(Get("AppFullName").toString(),
                                         "An instance of the application is already running.\nOnly one instance of Yield-Man can be run on a system.\nContact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+" for more details.\n\nGEX will now exit.");

                Exit(GS::Error::APPLICATION_ALREADY_RUNNING);
                return;
            }

#ifdef VFEI_SERVER // Legacy for ST: deprecated. Should be removed, or standardized (so to support generic VFEI protocol)
            if (GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
                GetInstance().StartVFEIServer();
#endif
        }

        if(!mPrivate->mlicenseReleaseRequest
                && GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed()
                && GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_CLIENT)
        {
            emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eGetYMAdminDBConfig,""));
            QCoreApplication::processEvents();
        }
        else
            QCoreApplication::postEvent(this, new QEvent(QEvent::Type(EVENT_LICENSE_GRANTED)));
    }
    else if(lpMessage.getType() == GS::LPPlugin::LPMessage::eReject)
    {
        ClientState lastStableState = GetClientState();
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() && !GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            if(!lpMessage.getData().isEmpty())
                Message::critical(Get("AppFullName").toString(), lpMessage.getData());
            Exit(GS::Error::CANNOT_GET_LICENSE);
        }
        else
        {
            if(lastStableState == Engine::eState_NodeReady || lastStableState == Engine::eState_ReConnect)
            {
                handleDisconnection(lpMessage.getData());
            }
            else
            {
                if(!lpMessage.getData().isEmpty())
                    Message::critical(Get("AppFullName").toString(), lpMessage.getData());
                Exit(GS::Error::CANNOT_GET_LICENSE);
            }

        }
    }
    else if(lpMessage.getType() == GS::LPPlugin::LPMessage::eDisconnected)
    {
        handleDisconnection(lpMessage.getData());
    }
    else if(lpMessage.getType() == GS::LPPlugin::LPMessage::eGetYMAdminDBConfig )
    {
        sendYMAdminDBConfig();
    }
    else if( lpMessage.getType() == GS::LPPlugin::LPMessage::eSetYMAdminDBConfig)
    {
        setYMAdminDBConfig(lpMessage.getData());//Sett the string in the connector
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(EVENT_LICENSE_GRANTED)));
    }
    else if(lpMessage.getType() == GS::LPPlugin::LPMessage::eExtended)
    {
        handleExtendedMessage(lpMessage.getData());
    }

    QCoreApplication::processEvents();
}

void Engine::handleDisconnection(const QString &lpMessage)
{
    GSLOG(SYSLOG_SEV_NOTICE,
            QString("handleDisconnection: %1").
            arg(lpMessage).toLatin1().constData())

    int lpError  = lpMessage.section("|",0,0).isEmpty() ? GS::Error::SERVER_CONNECTION_CLOSE : lpMessage.section("|",0,0).toInt();
    QString lpAddMessage = lpMessage.section("|",1,1);

    QString strMessage;

    // Hide process bar
    UpdateProgressStatus(true, 0, -1, false);
    UpdateLabelStatus();

    // YM history log
    strMessage = "'connection closed' signal received.";
    GetSchedulerEngine().AppendMoHistoryLog("","License provider connection lost", strMessage);

    // Trace Massage
    strMessage = "License provider connection lost: " + strMessage;
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().constData());

    strMessage = QString("License provider connection lost!\n"
                         "And send the following message :\n"
                         "%1").arg(lpAddMessage);
    if (ShouldConnectAgain((const int)lpError, strMessage))
    {
        // Do not close application,
        // user confirmed to Try to connect to server again
        QCoreApplication::processEvents();
        return;
    }


    // User asked to abord applilcation (no try to connect to server...)
    QCoreApplication::processEvents();
    Exit(EXIT_SUCCESS);
}


void Engine::setYMAdminDBConfig(const QString &strConf)
{
    QString strString = strConf;

    // if mPrivate->mYieldManDb already initialised, nothing to do
    if((strString.isEmpty() == false) && (strString != "GEX_YieldManDbSettings|NotSupported") &&
            !GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        UpdateProgressStatus(true, 0, -1, false);
        UpdateLabelStatus();
        bool bHaveToSendConfigToLP = false;
        // Check if already connected
        // Then ignore
        if(!mPrivate->mAdminEngine.IsActivated())
        {
            mPrivate->mAdminEngine.InitAdminServerConnector();
            //GS::LPPlugin::

            //We are in this mode lpMessage.getType() == GS::LPPlugin::LPMessage::eSetYMAdminDBConfig
            if(strString != "GEX_YieldManDbSettings|NotConfigured")
            {
                GSLOG(SYSLOG_SEV_DEBUG,"Start YieldMan Server");
                UpdateLabelStatus("Start YieldMan Server");

                // host|port|admin_name|admin_pwd
                QString strPassWord;
#ifdef QT_DEBUG
                // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
                // DO NOT ADD THIS GSLOG in RELEASE VERSION
                //GSLOG(SYSLOG_SEV_EMERGENCY, QString("Decrypting password from section 4 of string: %1").arg(strString));
#endif
                QString lDbHost(::getenv("DBHOST"));
                if (! lDbHost.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_NOTICE, QString("string = %1").
                          arg(strString).toUtf8().constData());
                    GSLOG(SYSLOG_SEV_NOTICE, QString("DBHOST = %1").
                          arg(lDbHost).toUtf8().constData());
                    strString = lDbHost;
                    strPassWord = strString.section("|", 4, 4).simplified();
                }
                else
                {
                    GexDbPlugin_Base::
                        DecryptPassword(strString.section("|", 4, 4).
                                        simplified(), strPassWord);
                }

                mPrivate->mAdminEngine.m_pDatabaseConnector->m_bAdminUser = true;
                QString strHost = strString.section("|",0,0);
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_IP = mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Name = mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Unresolved = strHost;
                if(strHost.contains("["))
                {
                    // HostName[HostIP]HostUnresolved
                    mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Name = strHost.section("[",0,0);
                    mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_IP = strHost.section("[",1).section("]",0,0);
                    mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Unresolved = strHost.section("]",1);

                    if(mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Unresolved.isEmpty())
                    {
                        bHaveToSendConfigToLP = true;
                        mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Unresolved = mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Name;
                    }
                }
                else
                    bHaveToSendConfigToLP = true;

                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strDriver = strString.section("|",1,1).simplified();
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strDatabaseName = strString.section("|",2,2).simplified();
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strUserName_Admin = strString.section("|",3,3).simplified();
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strPassword_Admin = strPassWord;
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strUserName = strString.section("|",3,3).simplified();
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strPassword = strPassWord;
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_strSchemaName = strString.section("|",5,5).simplified();
                mPrivate->mAdminEngine.m_pDatabaseConnector->m_uiPort = strString.section("|",6,6).simplified().toInt();
            }

            // Test the connection to the database
            //We are in this mode lpMessage.getType() == GS::LPPlugin::LPMessage::eGetYMAdminDBConfig)
            if((strString == "GEX_YieldManDbSettings|NotConfigured")
                    || (!mPrivate->mAdminEngine.ConnectToAdminServer()))
            {

                AdminEngine::ErrorCodeAdminEngine eLastError;
                eLastError = GGET_LASTERRORCODE(AdminEngine,&mPrivate->mAdminEngine);

                QString strCriticalError;
                if(eLastError == AdminEngine::eDB_NotUpToDate)
                {
                    strCriticalError = " version is not compatible.";
                    GSLOG(SYSLOG_SEV_ALERT, "Admin DB not compatible");
                }
                else if(eLastError == AdminEngine::eDB_UnderMaintenance)
                {
                    strCriticalError = " version is under maintenance.";
                    GSLOG(SYSLOG_SEV_ALERT, "Admin DB under maintenance");
                }
                else
                {
                    strCriticalError = " is not accessible.";
                    GSLOG(SYSLOG_SEV_ALERT, "Admin DB not accessible");
                }
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Line From Socket%1").arg(strString).
                      toLatin1().constData());
                // For the first installation
                // GexLm doesn't have the good connection settings
                // Ask for the user admin
                // Create the YieldManDb database
                // Send to GexLm the new connection settings
                // GexLm saves this settings in a local settings file
                // Then for the next connection
                // GexLm send this settings
                // And YieldManDb starts
                if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    bool bCreateYieldManDb = false;
                    bool bUpdateYieldManDb = false;
                    // if GexLM is configurate but yieldManDb not accessible
                    if(strString != "GEX_YieldManDbSettings|NotConfigured")
                    {
                        // Check why the connection was reject
                        // If the DB is not Up-To-Date or Under Maintenance
                        // Try an update
                        if((eLastError == AdminEngine::eDB_NotUpToDate)
                                || (eLastError == AdminEngine::eDB_UnderMaintenance))
                        {
                            if(GetCommandLineOptions().IsWelcomeBoxEnabled()
                                    && mPrivate->mAdminEngine.WelcomeDialog("update"))
                                bUpdateYieldManDb = true;
                        }
                        else
                        {
                            // Display an error message
                            mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,true);
                            // ALWAYS EXIT
                            GSLOG(SYSLOG_SEV_CRITICAL,"Admin DB not configured. System will exit now.");
                            Exit(EXIT_FAILURE);
                            return;
                        }
                    }
                    else if(GetCommandLineOptions().IsWelcomeBoxEnabled())
                    {
                        // Display information about YieldManDb for User
                        if(mPrivate->mAdminEngine.WelcomeDialog("install"))
                        {
                            bCreateYieldManDb = true;
                        }
                        else
                        {
                            // case 5787 - Make YM admin DB mandatory (Monitoring mode)
                            // Display an error message
                            //strCriticalError += "\nYield-Man Administration Server installation was aborted.";
                            //mPrivate->mYieldManDb->YieldManDbConnectionErrorDialog(strCriticalError,true);
                            // ALWAYS EXIT
                            //pGexMainWindow->ExitGexApplication(EXIT_FAILURE);
                            //return;
                            // Case 5787 - Make YmAdminDb server mandatory for RDB
                            // accept to start Yield-Man in NO RDB mode
                            strCriticalError = " installation was aborted.";
                            strCriticalError += "\nYou chose to skip Yield-Man Administration Server installation.";
                            strCriticalError += "\nYour Yield-Man version does not support RDB tools.";
                            strCriticalError += "\nTo install Yield-Man Administration Server,";
                            strCriticalError += " start Yield-Man with the arg -W.";
                            mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,false);
                        }
                    }

                    if(bCreateYieldManDb)
                    {
                        strCriticalError = "";
                        if(mPrivate->mAdminEngine.CreationAdminServerDb())
                        {
                            // Update the Admin Server is needed
                            bUpdateYieldManDb = true;
                            bHaveToSendConfigToLP = true;
                        }
                        else
                        {
                            strCriticalError += "\nYield-Man Administration Server installation was aborted.";
                            mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,true);
                            // ALWAYS EXIT
                            GSLOG(SYSLOG_SEV_CRITICAL,strCriticalError.toLatin1().data());
                            Exit(EXIT_FAILURE);
                            return;
                        }
                    }
                    if(bUpdateYieldManDb)
                    {
                        strCriticalError = "";
                        if(!mPrivate->mAdminEngine.UpdateAdminServerDb())
                        {
                            strCriticalError += "\nYield-Man Administration Server update was aborted.";
                            mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,true);
                            // ALWAYS EXIT
                            GSLOG(SYSLOG_SEV_CRITICAL,strCriticalError.toLatin1().data());
                            Exit(EXIT_FAILURE);
                            return;
                        }
                    }
                    else
                    {
                        // Check if we can start the Application
                        if(!strCriticalError.isEmpty() && GS::LPPlugin::ProductInfo::getInstance()->isYieldMan())
                        {
                            mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,true);
                            // ALWAYS EXIT
                            GSLOG(SYSLOG_SEV_CRITICAL,strCriticalError.toLatin1().data());
                            Exit(EXIT_FAILURE);
                            return;
                        }
                        // No connection
                        // Delete
//                        if(mPrivate->mAdminEngine.m_pDatabaseConnector)
//                            delete mPrivate->mAdminEngine.m_pDatabaseConnector;
                        mPrivate->mAdminEngine.DisconnectToAdminServer();
                        mPrivate->mAdminEngine.DeleteAdminServerConnector();
                    }


                }
                else
                {
                    // No connection
                    if(strString != "GEX_YieldManDbSettings|NotConfigured")
                    {
                        // Due to an error from the connection
                        // Display the last error

                        mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,false);

                        // Display an error message
                        QString strError;
                        QString strMessage = "";

                        mPrivate->mAdminEngine.GetLastError(strError);
                        strMessage = "Yield-Man Administration Server is not accessible.";
                        strMessage+= "\n";
                        if(!strError.isEmpty())
                            strMessage+= strError + "\n";
                        strMessage+= strString+"\n";

                        GSLOG(SYSLOG_SEV_EMERGENCY,
                              strMessage.toLatin1().constData());
                        GSLOG(SYSLOG_SEV_DEBUG,
                              strString.toLatin1().constData());
                    }
                    // Disconnect
                    mPrivate->mAdminEngine.DisconnectToAdminServer();
                    mPrivate->mAdminEngine.DeleteAdminServerConnector();
                }

            }

            if(mPrivate->mAdminEngine.m_pDatabaseConnector
                    && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                    && !mPrivate->mAdminEngine.IsServerUpToDate())
            {
                if(GetCommandLineOptions().IsWelcomeBoxEnabled()
                        && mPrivate->mAdminEngine.WelcomeDialog("update"))
                {
                    if(!mPrivate->mAdminEngine.UpdateAdminServerDb())
                    {
                        QString strCriticalError;
                        mPrivate->mAdminEngine.GetLastError(strCriticalError);
                        mPrivate->mAdminEngine.ConnectionErrorDialog(strCriticalError,true);
                        // ALWAYS EXIT
                        GSLOG(SYSLOG_SEV_CRITICAL,strCriticalError.toLatin1().data());
                        Exit(EXIT_FAILURE);
                        return;
                    }
                }
            }

            // Validate the connection
            if(mPrivate->mAdminEngine.m_pDatabaseConnector && mPrivate->mAdminEngine.ConnectToAdminServer())
            {
                if(bHaveToSendConfigToLP)
                {
                    sendYMAdminDBConfig();
                }

                // Check if YieldMan
                // Only manage the ADMIN_LOGIN on YieldMan
                // For other exec like Gex, no connection is requiered
                // but ADMIN_LOGIN can connect on Examinator to manage User accounts
                UpdateLabelStatus("Load YieldMan Server settings");

                if(!mPrivate->mAdminEngine.LoadServerSettings())
                {
                    bool lIsCritical = GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();
                    QString strError;
                    strError = " cannot start.";
                    mPrivate->mAdminEngine.ConnectionErrorDialog(strError,lIsCritical);
                    mPrivate->mAdminEngine.GetLastError(strError);
                    GSLOG(SYSLOG_SEV_EMERGENCY,
                          strError.toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());
                    if (lIsCritical)
                    {
                        Exit(EXIT_FAILURE);
                        return;
                    }
                    // Disconnect
                    mPrivate->mAdminEngine.DisconnectToAdminServer();
                    mPrivate->mAdminEngine.DeleteAdminServerConnector();
                }
            }
            else
            {
                // Disconnect
                mPrivate->mAdminEngine.DisconnectToAdminServer();
                mPrivate->mAdminEngine.DeleteAdminServerConnector();
            }
        }
    }
    else
    {
        // Disconnect
        mPrivate->mAdminEngine.DisconnectToAdminServer();
        mPrivate->mAdminEngine.DeleteAdminServerConnector();
    }

    // If all is ok
    // Start the ValidityCheck
    if(mPrivate->mAdminEngine.m_pDatabaseConnector)
        mPrivate->mAdminEngine.OnValidityCheck();
}

void Engine::sendYMAdminDBConfig()
{

    QString strString ;

    if(mPrivate->mAdminEngine.IsActivated())
    {
        // Gex-lm must be updated before to use this code
        // Update GexLm
        QString strPassWord;
#ifdef QT_DEBUG
#ifdef CASE4882
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        GSLOG(SYSLOG_SEV_EMERGENCY, QString("Crypting password %1 for user %2 before sending to Gex-LM")
              .arg(mPrivate->mYieldManDb->m_pDatabaseConnector->m_strPassword_Admin)
              .arg(mPrivate->mYieldManDb->m_pDatabaseConnector->m_strUserName_Admin));
#endif
#endif
        GexDbPlugin_Base::CryptPassword(mPrivate->mAdminEngine.m_pDatabaseConnector->m_strPassword_Admin, strPassWord);

        // Send connection with HostName[HostIP]HostUnresolved

        strString = "GEX_YieldManDbSettings;";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Name;
        strString+="["+mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_IP+"]";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strHost_Unresolved;
        strString+="|";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strDriver;
        strString+="|";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strDatabaseName;
        strString+="|";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strUserName_Admin;
        strString+="|";
        strString+= strPassWord;
        strString+="|";
        strString+= mPrivate->mAdminEngine.m_pDatabaseConnector->m_strSchemaName;
        strString+="|";
        strString+= QString::number(mPrivate->mAdminEngine.m_pDatabaseConnector->m_uiPort);

        //WriteLineToSocket(strString);
    }

    // send YMAdminDB config information
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eSetYMAdminDBConfig,
                                                 strString));
    QCoreApplication::processEvents();

}

void Engine::handleExtendedMessage(const QString &lpMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Handling extended message: %1").
          arg(lpMessage).toLatin1().constData());
    if(lpMessage.startsWith("Message::information"))
    {
        GS::Gex::Message::information(lpMessage.section("|",1,1), lpMessage.section("|",2,2));
    }
    else if(lpMessage.startsWith("Message::warning"))
    {
        GS::Gex::Message::warning(lpMessage.section("|",1,1), lpMessage.section("|",2,2));
    }
    else if(lpMessage.startsWith("Message::critical"))
    {
        GS::Gex::Message::critical(lpMessage.section("|",1,1), lpMessage.section("|",2,2));
    }
    else if(lpMessage.startsWith("UpdateProgressStatus"))
    {
        UpdateProgressStatus(lpMessage.section("|",1,1).toInt(),
                             lpMessage.section("|",2,2).toInt(),
                             lpMessage.section("|",3,3).toInt(),
                             lpMessage.section("|",4,4).toInt());
    }
    else if(lpMessage.startsWith("UpdateLabelStatus"))
    {
        UpdateLabelStatus(lpMessage.section("|",1,1));
    }
    else if(lpMessage.startsWith("AppendMoHistoryLog"))
    {
        GetSchedulerEngine().AppendMoHistoryLog(lpMessage.section("|",1,1),
                                                lpMessage.section("|",2,2),
                                                lpMessage.section("|",3,3));
    }
//    else if(lpMessage.startsWith("InitScriptsPath"))
//    {
//        QString lR=InitScriptsPath((bool)lpMessage.section("|",1,1).toInt());
//        if (lR.startsWith("err"))
//            GSLOG(3, QString("InitScriptsPath failed : %1").arg(lR).toLatin1().data() );
//    }
    else if(lpMessage.startsWith("License:"))
    {
        // Active-Passive license
        // Received an license
        // In Active Mode => Node Ready
        // In Passive Mode => License Granted
        bool bStartWithPassiveMode = mPrivate->mlicensePassive;
        if(lpMessage == "License:PassiveMode")
            mPrivate->mlicensePassive = true;
        else
            mPrivate->mlicensePassive = false;

        if(bStartWithPassiveMode != mPrivate->mlicensePassive)
        {
            // case 6384 - force Scheduler to start (not PAUSE mode)
            // when Active will be accepted
            GetCommandLineOptions().SetWelcomeBoxEnabled(false);
            // case 7663
            // Update Application Name - add/remove (Passive License)
            UpdateProductStringName();
        }

        // emit a signal for NodeReady
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(EVENT_LICENSE_GRANTED)));
    }
    else
    {
        GEX_ASSERT(false);
    }
}

void Engine::disconnectFromLP(const QString &str)
{
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eDisconnect, str));
    QCoreApplication::processEvents();
}

void Engine::reconnectToLP(const QString &str)
{
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eReconnect, str));
    QCoreApplication::processEvents();
}

void Engine::autoCloseReached(QString &message)
{
    // Make sure there isn't a pending license release request
    if(!mPrivate->mlicenseReleaseRequest)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "autoCloseReached" );

        bool restart = false;
        if(!GetSchedulerEngine().isSchedulerStopped())
        {
            GetSchedulerEngine().OnStopAllRunningTask();
            restart = true;
        }

        mPrivate->mlicenseReleaseRequest = true;
        disconnectFromLP(message);
        bool ret = ShouldConnectAgain(GS::Error::SOFTWARE_IDLE, message);

        if(ret && restart)
        {
            GetSchedulerEngine().OnRestartAllPendingTask();
        }

        if(!ret)
        {
            QCoreApplication::processEvents();
            Exit(EXIT_SUCCESS);
        }
    }
}

void Engine::ConnectToLPAgain()
{
    reconnectToLP("Should connect again automatic");
}

void Engine::licenseReleaseRequest()
{
    if(!sender())
        return;

    bool restart = false;
    if(!GetSchedulerEngine().isSchedulerStopped())
    {
        GetSchedulerEngine().OnStopAllRunningTask();
        restart = true;
    }

    mPrivate->mlicenseReleaseRequest = true;
    disconnectFromLP("User request to release the license momentarily");
    bool ret = ShouldConnectAgain(GS::Error::USER_LICENSE_RELEASE,
               "User request to release the license momentarily: license has been released to other users.");
    if(ret && restart)
    {
        GetSchedulerEngine().OnRestartAllPendingTask();
    }

    if(!ret)
    {
        QCoreApplication::processEvents();
        Exit(EXIT_SUCCESS);
    }

}

void Engine::activityNotification(const QString &str)
{
//    GSLOG(SYSLOG_SEV_DEBUG, "activityNotification" );
    emit sendGEXMessage(GS::LPPlugin::GEXMessage(GS::LPPlugin::GEXMessage::eActive, str));
}

bool Engine::GetLicensePassive()
{
    return mPrivate->mlicensePassive;
}



QList<QString> Engine::GetListNotNotifiyedFile()
{
    return mListNotNotifiyedFile;
}


///< brief Add the file name to the list of file names that shouldn't be notifiyed
void Engine::AddFileToNotNotifiyed(QString fileName)
{
    mListNotNotifiyedFile.append(fileName);
}

void Engine::RequestGTLLicense(qintptr libraryID, bool &status, QString &info)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start RequestGTLLicense %1").arg(libraryID).toLatin1().constData());

    if(GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider())
    {
        QString lRequest = QString("%1%2%3").arg(GS::LPPlugin::GTLOperations).arg(GS::LPPlugin::GTLRequestLicense)
                .arg(libraryID);
        GSLOG(SYSLOG_SEV_DEBUG, QString("RequestGTLLicense Request %1").arg(lRequest).toLatin1().constData());

        GS::LPPlugin::GEXMessage lReqMessage(GS::LPPlugin::GEXMessage::eExtended,lRequest);
        GS::LPPlugin::LPMessage lResMessage;
        GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->DirectRequest(lReqMessage,
                                                                                                 lResMessage);

        GSLOG(SYSLOG_SEV_DEBUG, QString("RequestGTLLicense Response %1")
              .arg( QString("LPMessage(%1 , %2)").arg(lResMessage.getType())
              .arg(lResMessage.getData())).toLatin1().constData());

        if(lResMessage.getType() == GS::LPPlugin::LPMessage::eExtended)
        {
            QString lMessage = lResMessage.getData();
            if(lMessage.startsWith(GS::LPPlugin::GTLOperations))
            {
                QString lResponse = lMessage.section(GS::LPPlugin::GTLOperations,1,1);
                if(lResponse.startsWith(GS::LPPlugin::GTLResponseLicense))
                {
                    status = true;
                    QString lData = lResponse.section(GS::LPPlugin::GTLResponseLicense,1,1);
                    info = lData.section(":",2,2);
                }
                else if(lResponse.startsWith(GS::LPPlugin::GTLResponseNOLicense))
                {
                    status = false;
                    QString lData = lResponse.section(GS::LPPlugin::GTLResponseNOLicense,1,1);
                    info = lData.section(":",2,2);
                }
            }
            else
            {
                status = false;
                info = QString("Internal error invalid license provider");
                GSLOG(SYSLOG_SEV_WARNING, QString("RequestGTLLicense %1 %2").arg(status).arg(info).toLatin1().constData());
            }

        }
        else
        {
            status = false;
            info = QString("Internal error unexpected response from the license provide %1")
                    .arg(QString("LPMessage(%1 , %2)").arg(lResMessage.getType()).arg(lResMessage.getData()));
            GSLOG(SYSLOG_SEV_WARNING, QString("RequestGTLLicense %1 %2").arg(status).arg(info).toLatin1().constData());
        }
    }
    else
    {
        status = false;
        info = QString("Internal error invalid license provider");
        GSLOG(SYSLOG_SEV_WARNING, QString("RequestGTLLicense %1 %2").arg(status).arg(info).toLatin1().constData());
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("End RequestGTLLicense %1").arg(libraryID).toLatin1().constData());
}

void Engine::ReleaseGTLLicense(int libraryID)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start ReleaseGTLLicense %1").arg(libraryID).toLatin1().constData());
    if(GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider())
    {
        QString lRequest = QString("%1%2%3").arg(GS::LPPlugin::GTLOperations).arg(GS::LPPlugin::GTLReleaseLicense).arg(libraryID);
        GSLOG(SYSLOG_SEV_DEBUG, QString("ReleaseGTLLicense Request %1").arg(lRequest).toLatin1().constData());

        GS::LPPlugin::GEXMessage lReqMessage(GS::LPPlugin::GEXMessage::eExtended,lRequest);
        GS::LPPlugin::LPMessage lResMessage;
        GS::LPPlugin::LicenseProviderManager::getInstance()->getCurrentProvider()->DirectRequest(lReqMessage, lResMessage);

        GSLOG(SYSLOG_SEV_DEBUG, QString("RequestGTLLicense Response %1")
              .arg( QString("LPMessage(%1 , %2)").arg(lResMessage.getType()).arg(lResMessage.getData())).toLatin1().constData());
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("ReleaseGTLLicense  %1").arg("Internal error invalid license provider").toLatin1().constData());
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("End ReleaseGTLLicense %1").arg(libraryID).toLatin1().constData());
}


void Engine::FreeLicenseProvider()
{
    GS::LPPlugin::LicenseProviderManager::Destroy();

}

int Engine::GetServerGlobalTimeout()
{
    //Check if the License server return a valid value.
    if(GS::LPPlugin::LicenseProviderManager::getInstance() && !GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("GlobalTimeout").isNull())
    {
        //Check if the product is not a monitoring produc to allow this.
        bool lAllowGlobalTimeout = (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                                    && !GS::LPPlugin::ProductInfo::getInstance()->isGTM());
        // if it is allowed to be used by the product return the valid value else return -1
        if(lAllowGlobalTimeout)
        {
            return GS::LPPlugin::LicenseProviderManager::getInstance()->getLPData("GlobalTimeout").toInt();
        }
    }

    return -1;
}

}
}
