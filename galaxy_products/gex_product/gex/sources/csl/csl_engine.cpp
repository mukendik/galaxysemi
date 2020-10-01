#include "csl_engine.h"
#include "engine.h"
#include "gqtl_log.h"
#include "gex_report.h"
#include "command_line_options.h"

#include <QElapsedTimer>
#include <QApplication>
#include <sstream>

extern CGexReport*     gexReport;

namespace GS
{
namespace Gex
{

class CSLEnginePrivate
{
public:

    CSLEnginePrivate();
    ~CSLEnginePrivate();

    bool                mAbortRequested;
    bool                mProcessEvents;
    CSLEngine::CslType  mType;
    bool                mRunning;
    QString             mErrorMessage;
    QString             mScriptName;
};

CSLEnginePrivate::CSLEnginePrivate()
    : mAbortRequested(false), mProcessEvents(false), mType(CSLEngine::CSL_REGULAR), mRunning(false)
{

}

CSLEnginePrivate::~CSLEnginePrivate()
{

}

CSLEngine * CSLEngine::mInstance = NULL;

CSLEngine::CSLEngine(QObject * parent)
    : QObject(parent), mPrivate(new CSLEnginePrivate)
{
}

CSLEngine::~CSLEngine()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

CSLStatus CSLEngine::RunScript(const QString &scriptName)
{
    mPrivate->mType             = CSL_REGULAR;
    mPrivate->mProcessEvents    = true;

    return Run(scriptName);
}

QString CSLEngine::RunCSL(const QString &scriptName)
// fix me: cant use CSLStatus* as arg:
// the script engine returns: arg 2 has unknown type 'CSLStatus*' (register with qScriptRegisterMetaType())
// even if CSLStatus* has been registered...
// for the moment let s put the root cause error if any in the returned string
{
    //if (!lStatus)
      //  return QString("error: Status param null");
    CSLStatus lS;
    mPrivate->mType             = CSL_REGULAR;
    mPrivate->mProcessEvents    = true;

    lS=Run(scriptName);
    return lS.IsFailed()?QString("error: ")+lS.GetErrorMessage():QString("ok");
}

CSLStatus CSLEngine::RunArgumentScript(const QString &scriptName)
{
    mPrivate->mType             = CSL_ARGUMENT;
    mPrivate->mProcessEvents    = true;

    return Run(scriptName);
}

CSLStatus CSLEngine::RunStartupScript(const QString &scriptName)
{
    mPrivate->mType  = CSL_STARTUP;

    return Run(scriptName);
}

bool CSLEngine::IsRunning(CslType lType /*= CSL_ANY*/) const
{
    return (mPrivate->mRunning && (mPrivate->mType & lType));
}

bool CSLEngine::IsAbortRequested() const
{
    if (mPrivate->mProcessEvents)
        QCoreApplication::processEvents();

    return mPrivate->mAbortRequested;
}

const QString &CSLEngine::GetScriptName() const
{
    return mPrivate->mScriptName;
}

void CSLEngine::CheckCSLVersion(const QString& scriptName)
{
    QFile lScriptFile(scriptName);

    if(lScriptFile.open(QFile::ReadOnly))
    {
        QTextStream lTxtStream(&lScriptFile);

        if (lTxtStream.readAll().contains("gexCslVersion('") == false)
        {
            // no gexCslVersion() call in this csl.
            // Should be a very old csl. Let's save it before being overwritten
            if (!QFile::copy(QDir::cleanPath(QString("%1").arg(scriptName)),
                             QDir::cleanPath(QString("%1.bak").arg(scriptName))))
            {
                QString lLog;

                lLog = "Warning : the version of this CSL file (" + scriptName;
                lLog += ") version is too old for this version of the product.\n";
                lLog += "It will be updated. We have tried to do a backup but it was impossible ";
                lLog += "(perhaps the backup file already exists or the folder is unwritable).";

                emit scriptLog(lLog);

                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Error: cannot save a backup for this (old) CSL : %1")
                      .arg(scriptName).toLatin1().data());
            }
            else
            {
                QString lLog;

                lLog = "A backup copy of this CSL (" + scriptName;
                lLog += ") has been created because its format is too old.";

                emit scriptLog(lLog);

                GSLOG(SYSLOG_SEV_INFORMATIONAL, "Backup copy of the CSL has been created.");
            }
        }

        // Close CSL file
        lScriptFile.close();
    }
    else
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Can't open CSL script '%1'").arg(scriptName).toLatin1().data());
}

CSLStatus CSLEngine::Run(const QString &scriptName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Running script: %1").arg(scriptName).toLatin1().data());

    CSLStatus lStatus;

    // Set startup script status
    lStatus.mStartup = (mPrivate->mType & CSL_STARTUP) ? true : false;

    // Clear error message
    mPrivate->mErrorMessage.clear();
    mPrivate->mAbortRequested   = false;
    mPrivate->mScriptName       = scriptName;

    if (scriptName.isEmpty())
    {
        lStatus.mFailed       = true;
        lStatus.mErrorMessage = "CSL Script name is empty.";

        GSLOG(SYSLOG_SEV_WARNING,
              QString("Unable to execute CSL Script, file name is empty.")
              .arg(mPrivate->mErrorMessage).toLatin1().data());

        return lStatus;
    }

    if (IsRunning())
    {
        lStatus.mFailed       = true;
        lStatus.mErrorMessage = "CSL script engine is already running.";

        GSLOG(SYSLOG_SEV_WARNING,
              QString("Unable to execute CSL Script, another script is already running.")
              .arg(mPrivate->mErrorMessage).toLatin1().data());
        return lStatus;
    }

    if (IsRunnable() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Script cannot be executed: %1")
              .arg(mPrivate->mErrorMessage).toLatin1().data());

        lStatus.mFailed       = true;
        lStatus.mErrorMessage = mPrivate->mErrorMessage;

        return lStatus;
    }

    // Set script running flag to true
    mPrivate->mRunning = true;

    // Emit a signal when CSL engine run a script
    emit scriptStarted(scriptName, (mPrivate->mType & CSL_STARTUP) ? true : false);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Checking CSL version of '%1'...").arg(scriptName).toLatin1().data() );
    // Check CSL version of the script file
    CheckCSLVersion(scriptName);

    ZCsl * lZCSL = NULL;

    try
    {
        // GEX module name...
        ZString lModule("gex");

        // create csl instance
        lZCSL = new ZCsl();

        QStringList lArgs;
        // Creates the argc,argv[] list
        if (!qApp)
        {
            lStatus.mFailed=true;
            lStatus.mErrorMessage=QString("Q(Core)Application null");
            return lStatus;
        }
        else
        {
            //            #ifdef QT_DEBUG
            //              GSLOG(4, "Temporary workaround to allow to run daemon in debug");
            //              lArgs.insert(0, QCoreApplication::applicationFilePath());
            //
            #ifdef QT_DEBUG
                lArgs = GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetArguments();
            #else
                lArgs = QGuiApplication::arguments(); // Strangely this line is crashing in daemon debug not in release...
            #endif
        }
        ZString lInit("const mainArgVals[");
        lInit += ZString(lArgs.count());
        lInit += "] = {\n";

        for(int lIdx = 0; lIdx < lArgs.count(); ++lIdx)
        {
            // Add each argv[] argument to the list
            if (!lArgs.at(lIdx).isEmpty())
            {
                lInit += "  '" + ZString(lArgs.at(lIdx).toLatin1().data()).change("\\","\\\\")+"'";

                if(lIdx + 1 < lArgs.count())
                    lInit += ",";	// If we have not reached the last argument
            }
        }

        // Close structure
        lInit += '\n';
        lInit += "};\n";

        ZTRACE_DEVELOP(lInit);
        std::istringstream lInitStream((char*) lInit);

        GSLOG(SYSLOG_SEV_DEBUG, "Initializing CSL engine");

        // load initialization
        lZCSL->loadScript(lModule, &lInitStream);

        GSLOG(SYSLOG_SEV_DEBUG, "CSL engine initialized.");

        // Starts given script.
        GSLOG(SYSLOG_SEV_DEBUG, "Loading CSL script into CSL engine");

        lInit = scriptName.toLatin1().data();
        lZCSL->loadScript(lInit);

        GSLOG(SYSLOG_SEV_DEBUG, "CSL script loaded into CSL engine");

        // execute main
        GSLOG(SYSLOG_SEV_DEBUG, "Calling main function of CSL script");

        lZCSL->call(lModule, "main").asInt();

        GSLOG(SYSLOG_SEV_DEBUG, "Main function executed");

    } // try
    catch(ZString msg)
    {
        lStatus.mFailed         = true;
        lStatus.mErrorMessage   = msg.constBuffer();

        GSLOG(SYSLOG_SEV_ERROR,
              QString("CSL exception caught: %1").arg(msg.constBuffer()).toLatin1().data());
    }
    catch (const ZException& err)
    {
        lStatus.mFailed         = true;
        lStatus.mErrorMessage   = (err.count() > 0) ? err[0] : "No message";

        GSLOG(SYSLOG_SEV_ERROR, lStatus.mErrorMessage.toLatin1().data());

        emit scriptLog(lStatus.mErrorMessage);
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
           QString("CSL Script '%1' executed...").arg(scriptName).toLatin1().data());

    if(lZCSL != NULL)
    {
        // delete instance
        delete lZCSL;
        lZCSL = NULL;
    }

    // Abort was requested, set the status accordingly
    if (mPrivate->mAbortRequested)
    {
        lStatus.mAborted    = true;
        lStatus.mFailed     = true;
    }

    // Notify client that script is completed
    emit scriptFinished(lStatus);

    // CSL engine is not running anymore
    mPrivate->mRunning = false;

    return lStatus;
}

bool CSLEngine::IsRunnable() const
{
    // Accept to run scripts only if license granted or ready.
    bool            lLicenseReady = false;

    // Check if GEX running as Client node....
    switch(Engine::GetInstance().GetClientState())
    {
        case Engine::eState_LicenseGranted:	// GEX running as Client, and license granted
        case Engine::eState_NodeReady:// GEX running as Standalone or Evaluation
            lLicenseReady = true;
            break;

        default:
            break;
    }

    if (lLicenseReady == false)
    {
        mPrivate->mErrorMessage = "No licence granted or ready. \n\nScripts can't be executed.";
        return false;
    }


    return true;
}

void CSLEngine::AbortScript()
{
    mPrivate->mAbortRequested  = true;
}

CSLEngine &CSLEngine::GetInstance()
{
    if (mInstance)
        return *mInstance;

    // Even if the parent is the app, the Engine is not deleted at exit...
    mInstance = new CSLEngine(qApp);

//    // Let s try to connect to deleteLater, Destroy() but it does not work...
//    bool lSuccess = connect(qApp, SIGNAL(aboutToQuit()), mInstance, SLOT(Destroy()));

//    if (!lSuccess)
//        GSLOG(SYSLOG_SEV_WARNING, "Cannot connect signal aboutToQuit with GS::GEX::CSLEngine::Destroy");

    return *mInstance;
}

void CSLEngine::Destroy()
{
    if (mInstance)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

CSLStatus::CSLStatus(QObject* parent) : QObject(parent)
    , mFailed(false), mAborted(false), mStartup(false)
{
}

CSLStatus::~CSLStatus()
{
}

CSLStatus::CSLStatus(const CSLStatus& other)
   :QObject(other.parent())
{
    *this=other;
}

CSLStatus& CSLStatus::operator=(const CSLStatus& other)
{
    mFailed = other.mFailed;
    mAborted = other.mAborted;
    mStartup = other.mStartup;
    mErrorMessage = other.mErrorMessage;
    return *this;
}

bool CSLStatus::IsFailed() const
{
    return mFailed;
}

bool CSLStatus::IsAborted() const
{
    return mAborted;
}

bool CSLStatus::IsStartup() const
{
    return mStartup;
}

const QString &CSLStatus::GetErrorMessage() const
{
    return mErrorMessage;
}

}   // namespace Gex
}   // namespase GS
