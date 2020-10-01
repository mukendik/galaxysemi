#include "browser_dialog.h"
#include "db_transactions.h"
#include <gqtl_log.h>
#include "scheduler_engine.h"
#include "file.h"
#include "gex_scriptengine.h"
#include "db_engine.h"
#include "report_options.h"
#include "engine.h"

extern GexScriptEngine*	pGexScriptEngine;
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern bool UpdateGexScriptEngine(CReportOptions * pReportOptions);

int	GS::Gex::DatabaseEngine::ProcessScriptFile(
        QString strFileName, bool */*pbDeleteFile*/,
        QString &strErrorMessage,
        QString &strShortErrorMsg)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Process script file %1...").arg( strFileName).toLatin1().constData());

    QFile sf(strFileName);
    if (!sf.open(QIODevice::ReadOnly))
    {
        strErrorMessage=strShortErrorMsg=m_strInsertionShortErrorMessage="Cannot open file '"+strFileName+"'";
        return Failed;
    }

    QTextStream ts(&sf);
    QString JSAction=ts.readAll();
    sf.close();

    QScriptSyntaxCheckResult scr= pGexScriptEngine->checkSyntax( JSAction );
    if (scr.state()!=QScriptSyntaxCheckResult::Valid)
        GSLOG(SYSLOG_SEV_WARNING, "Syntax error in script");

    QFileInfo fi(strFileName);
    QString CurrentScriptPath=fi.absolutePath();
    pGexScriptEngine->globalObject().setProperty("CurrentScriptPath",
            CurrentScriptPath, QScriptValue::ReadOnly);
    pGexScriptEngine->globalObject().setProperty("CurrentScriptFileName",
            fi.fileName(), QScriptValue::ReadOnly);

    if (!UpdateGexScriptEngine(&ReportOptions))
        GSLOG(SYSLOG_SEV_WARNING, "Failed to update Scripting Engine");

    QScriptValue scriptValue = pGexScriptEngine->evaluate( JSAction,
      GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator()+"js_log.txt" );

    QString Result;
    if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
    {
        Result=pGexScriptEngine->uncaughtException().toString();
        GSLOG(SYSLOG_SEV_WARNING, QString("Script evaluation : at line %1: '%2'")
               .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
               .arg(Result)
               .toLatin1().data());
        // TODO case 5863 : send an email about this error
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Script evaluation returned : %1")
              .arg(scriptValue.toString().toLatin1().data() )
              .toLatin1().constData());
        // error: Insertion failed: Error in plugin: Galaxy SQL D
        Result = scriptValue.toString();
    }

    // GCORE-499
    pGexScriptEngine->collectGarbage();

    // Check if the Result is an integer
    int     lStatus = Passed;
    bool    lIsNum;
    lStatus = Result.toInt(&lIsNum);
    if (lIsNum)
    {
        if ((lStatus < 0) || (lStatus > 3))
        {
            lStatus = Failed; // Unknow status, consider Failed
            QString lResultStatus = Result.section(":",0,0).toLower().simplified();

            m_strInsertionShortErrorMessage = "Script has returned " + lResultStatus + ". "
                "It must return either 0, 1, 2 or 3 " ;
        }
    }
    else if(Result.startsWith("ok",Qt::CaseInsensitive)
            || Result.startsWith("delay",Qt::CaseInsensitive)
            || Result.startsWith("error",Qt::CaseInsensitive)
            || Result.isEmpty())
    {
        if(Result.isEmpty())
            Result = "ok:";
        QString lResultStatus = Result.section(":",0,0).toLower().simplified();
        QString lMessageStatus = Result.section(":",1).simplified();
        if(lMessageStatus.isEmpty())
            m_strInsertionShortErrorMessage = lMessageStatus;

        if(lResultStatus == "error")
            lStatus=Failed;
        if(lResultStatus == "delay")
            lStatus=Delay;
        if(lResultStatus == "ok")
            lStatus=Passed;
    }
    else
    {
        lStatus = Failed;
        QString lResultStatus = Result.section(":",0,0).toLower().simplified();

        m_strInsertionShortErrorMessage = "Script has returned \"" + lResultStatus + "\". "
            "It must return either \"ok\", \"error\" or \"delay\" " ;
    }


    strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;

    return lStatus;
}
