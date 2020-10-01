
#include "r_engine.h"
#include "Rinternals.h"
#include "Rembedded.h"
#include <QDir>
#include <QTemporaryFile>

#ifdef _WIN32
    #include <windows.h>
#endif

extern Rboolean R_Interactive;

namespace GS
{
namespace SE
{

REngine::REngine()
{

}

REngine::~REngine()
{
    Rf_endEmbeddedR(0);
}

bool REngine::Init(const QString& appDir)
{
    QString lRHomePath;
// check if GALAXY_R_HOME is defined then use it
    lRHomePath = QString(qgetenv("GALAXY_R_HOME"));
// else use
    if (lRHomePath.simplified().isEmpty())
        lRHomePath = QDir::cleanPath(appDir + QDir::separator() + "r");

    QDir lTestDir(lRHomePath);
    if (!lTestDir.exists())
    {
        mErrors.append(QString("cannot find GALAXY_R_HOME directory: %1").arg(lRHomePath));
        return false;
    }

    const char *R_argv[] = {"--gui=none", "--no-restore","--no-save", "--no-readline", "--silent", "--vanilla", "--slave"};
    int R_argc = sizeof(R_argv) / sizeof(R_argv[0]);

    // Set R_HOME
    if (!qputenv("R_HOME", lRHomePath.toLatin1().data()))
    {
        mErrors.append(QString("cannot set R_HOME path %1").arg(lRHomePath));
        return false;
    }
    // Init R
    if (!Rf_initEmbeddedR(R_argc, (char**)R_argv))
    {
        mErrors.append(QString("cannot init Embedded R engine"));
        return false;
    }

    R_ReplDLLinit();

    R_Interactive = static_cast<Rboolean>(FALSE);

    return true;
}

bool REngine::SourceScripts(const QStringList &scripts)
{
    for (int lIdx = 0; lIdx < scripts.size(); ++lIdx)
    {
        QString lScript = scripts.at(lIdx);
        if (!QFile::exists(lScript))
        {
            mErrors.append(QString("cannot find resource: %1").arg(lScript));
            return false;
        }
        // cp script into temp file
        QTemporaryFile *lTmpScriptFile = QTemporaryFile::createNativeFile(lScript);
        QString lFilePath = lTmpScriptFile->fileName();
        // source script
        SEXP lCall = PROTECT(Rf_lang2(
                                Rf_install("try"),
                                Rf_lang2(
                                    Rf_install("source"),
                                    Rf_mkString(lFilePath.toLatin1().data()))
                                ));
        if (!lCall)
        {
            mErrors.append(QString("Error while sourcing %1").arg(lScript));
            return false;
        }

        int lErrorOccurred;
        SEXP res = PROTECT( R_tryEvalSilent( lCall, R_GlobalEnv, &lErrorOccurred ));
        UNPROTECT(2);

        if (lErrorOccurred || !res || Rf_inherits( res, "try-error"))
        {
            mErrors.append(QString("cannot source %1: %2").arg(lScript).arg(QString(R_curErrorBuf())));
            return false;
        }
        delete lTmpScriptFile;
    }

    return true;
}

QString REngine::GetLastError()
{
    if (mErrors.isEmpty())
        return QString();

    return mErrors.last();
}

QStringList REngine::GetErrors()
{
    return mErrors;
}

void REngine::ClearErrors()
{
    mErrors.clear();
}

} // namespace SE
} // namespace GS

