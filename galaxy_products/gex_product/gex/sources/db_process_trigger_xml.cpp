#include <QFile>
#include <QDomDocument>

#include "browser_dialog.h"
#include "file.h"
#include "gex_version.h"
#include "gexmo_constants.h"
#include "db_engine.h"
#include "scheduler_engine.h"
#include "yield/yield_taskdata.h"
#include "status/status_taskdata.h"
#include "mo_email.h"
#include "mo_task.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include <gqtl_log.h>
#include "report_build.h"
#include "report_options.h"
#include "gex_scriptengine.h"
#include "gex_shared.h"
#include "gex_report.h"
#include "product_info.h"
#include "engine.h"

extern CGexReport*		gexReport;			// Handle to report class if yield check failed
// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern GexScriptEngine*	pGexScriptEngine;
extern bool UpdateGexScriptEngine(CReportOptions * pReportOptions);


QString  LaunchQProcess(QProcess* process, const QString &program, const QString& arguments, QByteArray& output, QByteArray& error, bool synchrone, int msecond = 3000)
{
    QString r="ok";
    QStringList lArguments = arguments.split(" ");

    // Use trimmed method to remove extra spaces at beginning and end of the command
    // On Linux, if the command has an extra space at the end, the process cannot be started.

    process->start(program.trimmed(), lArguments);

    if(synchrone)
    {
        if(process->waitForStarted() == true )
        {
            if(process->waitForFinished(msecond) == false)
            {
                output = process->readAllStandardOutput();
                process->kill();
                error = "Timeout, script execution aborted";
                r = "error:" + error;
                return r;
            }
            else if (process->exitCode() != 0)
            {
                output = process->readAllStandardOutput();
                error  = process->readAllStandardError();

                r = QString("error: code %1 returned").arg(process->exitCode());

                if (error.isEmpty() == false)
                    r = r + " with message " + error;

                return r;
            }
        }
        else
        {
             r = "error: Cannot start " + program;
             return r;
        }
    }

    output = process->readAllStandardOutput();
    return r;
}

// Warning : on windows the command path must NOT contains space char ' ' !
QString LaunchShellScript(QString &strCommand)
{
    QString r="ok";
    if(strCommand.isEmpty())
        return r;

    // Launch Shell command (minimized)
    #ifdef _WIN32
        // Replace '/' to '\' to avoid MS-DOS compatibility issues
        strCommand = strCommand.replace('/','\\');
        QString strArgs = strCommand.section(' ',1, -1);

        HINSTANCE i=ShellExecuteA(NULL,
               "open",
                strCommand.section(' ',0, 0).toLatin1().constData(),
                strArgs.toLatin1().constData(),
                NULL, SW_SHOWMINIMIZED);
        if (!i)
            r="error : shell script execution failed";
    #else
        // QString strCommandLine = strCommand + " " + strArgs;
        int i = system(strCommand.toLatin1().constData());
        if (i==-1 || i==127) {
            r = QString("error : %1 returned %2").
                    arg(strCommand).arg(i).toLatin1().data();
        }
    #endif

    return r;
}

int GS::Gex::DatabaseEngine::ProcessTriggerFile_XML(CGexMoTaskItem* /*ptTask*/,
                                          QString strTriggerFileName,
                                          bool* /*pbDeleteTrigger*/,
                                          QString& strErrorMessage,
                                          QString& strShortErrorMsg)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Process xml trigger file  %1...")
        .arg(strTriggerFileName).toLatin1().data() );

    QFile file(strTriggerFileName); // Read the text from a file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        strErrorMessage=strShortErrorMsg=m_strInsertionShortErrorMessage="cant open .gtf file !";
        return Failed;	// Failed opening Trigger file.
    }

    QDomDocument doc("gtf");

    QString errorMsg;
    int errorLine, errorColumn=0;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        QString error=QString("file %1 is not xml compliant (line %2 col %3) : %4 !")
                      .arg(strTriggerFileName).arg(errorLine).arg(errorColumn).arg(errorMsg);
        GSLOG(SYSLOG_SEV_ERROR, error.toLatin1().data());

        file.close();

        strErrorMessage=error;
        strShortErrorMsg=m_strInsertionShortErrorMessage
                =QString("error parsing corrupted .gtf file (line %1 col %2)").arg(errorLine).arg(errorColumn);

        return Failed;
    }
    file.close();

    // print out the element names of all elements that are direct children of the outermost element.
    QDomElement docElem = doc.documentElement();


    // Check we support this version of .gtf
    bool ok=false;
    float version=docElem.attribute("Version").toFloat(&ok);
    GSLOG(SYSLOG_SEV_DEBUG, QString("gtf file version is %1")
          .arg( version)
          .toLatin1().constData());
    if ( (!ok) || (version==0.0f) || (version<(float)GEX_MIN_GTF_VERSION) || (version>(float)GEX_MAX_GTF_VERSION) )
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("illegal version : should be between %1 and %2")
              .arg(GEX_MIN_GTF_VERSION)
              .arg(GEX_MAX_GTF_VERSION)
              .toLatin1().constData());
        QString error=QString();
        strErrorMessage=error;
        if (version<(float)GEX_MIN_GTF_VERSION)
         strShortErrorMsg=m_strInsertionShortErrorMessage=".gtf version too old";
        if (version>(float)GEX_MAX_GTF_VERSION)
         strShortErrorMsg=m_strInsertionShortErrorMessage=".gtf version not yet supported";
        return Failed;
    }

    QString JSAction=docElem.attribute("JSAction");
    if (!JSAction.isEmpty())
    {
      GSLOG(SYSLOG_SEV_NOTICE, QString("JS action: Starting with %1")
            .arg( JSAction)
            .toLatin1().constData());

      QScriptSyntaxCheckResult scr= pGexScriptEngine->checkSyntax( JSAction );
      if (scr.state()!=QScriptSyntaxCheckResult::Valid)
        GSLOG(SYSLOG_SEV_WARNING, "Syntax error in script");

      QFileInfo fi(strTriggerFileName);
      QString CurrentScriptPath=fi.absolutePath();
      pGexScriptEngine->globalObject().setProperty("CurrentScriptPath",
        CurrentScriptPath, QScriptValue::ReadOnly);
      pGexScriptEngine->globalObject().setProperty("CurrentScriptFileName",
              fi.fileName(), QScriptValue::ReadOnly);

      QString lToBeConcatenated;
      QString lJSConcatenateFile=docElem.attribute("JSConcatenateFile");
      if (!lJSConcatenateFile.isEmpty())
      {
          QFile lCJS(lJSConcatenateFile);
          if (!lCJS.open(QIODevice::ReadOnly))
          {
              lCJS.setFileName(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
                               +"/GalaxySemi/scripts/"+lJSConcatenateFile);
              if (!lCJS.open(QIODevice::ReadOnly))
              {
                  lCJS.setFileName(CurrentScriptPath+"/"+lJSConcatenateFile);
                  lCJS.open(QIODevice::ReadOnly);
              }
          }
          if (lCJS.isOpen())
          {
              QTextStream lTS(&lCJS);
              lToBeConcatenated=lTS.readAll();
          }
          else
          {
              strErrorMessage=strShortErrorMsg=m_strInsertionShortErrorMessage
                =QString("error: cannot open file %1").arg(lJSConcatenateFile);
              GSLOG(SYSLOG_SEV_ERROR, strShortErrorMsg.toLatin1().data());
              return Failed;
          }
      }


      if (!UpdateGexScriptEngine(&ReportOptions))
        GSLOG(SYSLOG_SEV_WARNING, "Failed to update Scripting Engine");

      QString lScript=JSAction;
      if (!lToBeConcatenated.isEmpty())
          lScript.append("\n// concatenated from "+ lJSConcatenateFile +"\n"+lToBeConcatenated);

      QScriptValue scriptValue = pGexScriptEngine->evaluate( lScript,
        GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator()+"js_log.txt" );
      if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
      {
          QString m=QString("JavaScript evaluation : at line %1: '%2'")
                  .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                  .arg(pGexScriptEngine->uncaughtException().toString());
          GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
          // TODO case 5863 : send an email about this error
          strErrorMessage=strShortErrorMsg=m_strInsertionShortErrorMessage=m;
          return Failed;
      }
      GSLOG(SYSLOG_SEV_NOTICE, QString("JS action: '%1' returned %2")
            .arg(JSAction.replace('\n',' ').toLatin1().data())
            .arg(scriptValue.toBool()?"true":"false")
            .toLatin1().constData());

      // Check if the script returns a status
      QString lResult = scriptValue.toString();

      // Check if the Result is an integer
      int     lStatus = Passed;
      bool    lIsNum;
      lStatus = lResult.toInt(&lIsNum);
      if(!lIsNum && (lStatus<0) && (lStatus>3))
          lStatus = Passed; // Unknow status, consider Passed

      // Check the type of the Result message
      if(lResult.startsWith("ok",Qt::CaseInsensitive)
              || lResult.startsWith("delay",Qt::CaseInsensitive)
              || lResult.startsWith("error",Qt::CaseInsensitive)
              || lResult.isEmpty())
      {
          if(lResult.isEmpty())
              lResult = "ok:";
          QString lResultStatus = lResult.section(":",0,0).toLower().simplified();
          QString lMessageStatus = lResult.section(":",1).simplified();
          if(lMessageStatus.isEmpty())
              m_strInsertionShortErrorMessage = lMessageStatus;

          if(lResultStatus == "error")
              lStatus=Failed;
          if(lResultStatus == "delay")
              lStatus=Delay;
          if(lResultStatus == "ok")
              lStatus=Passed;
      }
      strErrorMessage = strShortErrorMsg = m_strInsertionShortErrorMessage;

      return lStatus;
    } // JSAction. Do we continue ?

    QString Action=docElem.attribute("Action");
    QString RuleTitle=docElem.attribute("RuleName");
    QString DatabaseName=docElem.attribute("DatabaseName");
    QString TestingStage=docElem.attribute("TestingStage");
    QString Shell=docElem.attribute("Shell");
    QString Product=docElem.attribute("Product");
    QString Lot=docElem.attribute("Lot");
    QString WaferID=docElem.attribute("WaferID");
    QString SubLot=docElem.attribute("SubLot");
    QString LogFile=docElem.attribute("LogFile");
    QString TriggerSource=doc.toString();

    int lStatus = 0;
    if(Action.toLower() == "checkspm")
    {
        QtLib::DatakeysContent dbKeysContent;
        dbKeysContent.Set("Product", Product);
        dbKeysContent.Set("DatabaseName",DatabaseName);
        dbKeysContent.Set("TestingStage", TestingStage);
        dbKeysContent.Set("Lot", Lot);
        dbKeysContent.Set("SubLot", SubLot);
        dbKeysContent.Set("Wafer", WaferID);
        dbKeysContent.Set("TaskName", RuleTitle);
        dbKeysContent.Set("TaskType", "SPM");

        lStatus=CheckStatMon(dbKeysContent, Shell, LogFile, strTriggerFileName,TriggerSource);
    }
    else if(Action.toLower() == "checksya")
    {
        QtLib::DatakeysContent dbKeysContent;
        dbKeysContent.Set("Product", Product);
        dbKeysContent.Set("DatabaseName",DatabaseName);
        dbKeysContent.Set("TestingStage", TestingStage);
        dbKeysContent.Set("Lot", Lot);
        dbKeysContent.Set("SubLot", SubLot);
        dbKeysContent.Set("Wafer", WaferID);
        dbKeysContent.Set("TaskName", RuleTitle);
        dbKeysContent.Set("TaskType", "SYA");

        lStatus=CheckStatMon(dbKeysContent, Shell, LogFile, strTriggerFileName,TriggerSource);
    }
    else
    {
        strErrorMessage=strShortErrorMsg = m_strInsertionShortErrorMessage = QString("Action '%1' not supported !").arg(Action);
        GSLOG(SYSLOG_SEV_ERROR, m_strInsertionShortErrorMessage.toLatin1().data());
        return Failed;
    }

    strShortErrorMsg=m_strInsertionShortErrorMessage;
    return lStatus;
}
