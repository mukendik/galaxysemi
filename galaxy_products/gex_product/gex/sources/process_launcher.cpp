///////////////////////////////////////////////////////////
// process_launcher.cpp: implementation of CProcessLauncher class
///////////////////////////////////////////////////////////
#include "process_launcher.h"
#include "processoutput_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "command_line_options.h"
#include "message.h"

CProcessLauncher::CProcessLauncher(
        const QString& strCommand,
        const QStringList& strArguments,
        bool bSynchronous /* = false */)
    : m_proc(NULL), m_nProcessExitStatus(0), m_nProcessDuration(0)
{
    m_bSynchronous	= bSynchronous;
    m_strCommand	= strCommand;
    m_strArguments	= strArguments;
    m_strFullCommandline << strCommand << strArguments;

    if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsCustomDebugMode()
      && (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()))
    {
        QString strCaption = strCommand + ": process output";
        ProcessOutputDialog* pclProcessoutputDialog
          = new ProcessOutputDialog(strCaption, m_strFullCommandline.join(" "));
        connect(this, SIGNAL(sStdoutChanged(const QString &)),
          pclProcessoutputDialog, SLOT(OnStdoutChanged(const QString &)));
        connect(this, SIGNAL(sStderrChanged(const QString &)),
          pclProcessoutputDialog, SLOT(OnStderrChanged(const QString &)));
        pclProcessoutputDialog->show();
    }
}

CProcessLauncher::~CProcessLauncher()
{
    if(m_proc != NULL)
    {
        delete m_proc;
        m_proc = NULL;
    }
}

// Launch the process
bool CProcessLauncher::Launch(void)
{
    m_nProcessExitStatus = 0;
    m_nProcessDuration = 0;
    m_Elapsed.start();

    if(m_proc != NULL)
    {
        delete m_proc;
        m_proc = NULL;
    }

    m_proc = new QProcess( this );

    #ifdef Q_OS_WIN
        m_strArguments << "//B";				// Batch mode
    #endif

    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(OnScriptFinished(int, QProcess::ExitStatus)));
    connect(m_proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(OnStdoutDataReady()));
    connect(m_proc, SIGNAL(readyReadStandardError()),
            this, SLOT(OnStderrDataReady()));
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(OnError(QProcess::ProcessError)));

    GSLOG(SYSLOG_SEV_NOTICE, QString("launching '%1' '%2'...")
        .arg(m_strCommand)
        .arg(m_strArguments.join(" ")).toLatin1().constData());
    QString strMessage;
    strMessage = "> Command   = " + m_strCommand;
    strMessage = "> Arguments = " + m_strArguments.join(" ");

    m_proc->start(m_strCommand, m_strArguments);

    if(!m_proc->waitForStarted())
    {
        QString strMessage = "Error launching process: ";
        strMessage += m_strCommand;
        strMessage += "\n\nPossible causes:\n";
        strMessage += "  o Process not found in Windows paths\n";
        strMessage += "  o A FireWall application denies the process launch\n";
        GSLOG(SYSLOG_SEV_ERROR, strMessage.replace('\n', ' ').toLatin1().data());
        GS::Gex::Message::warning("GEX", strMessage);
        return false;
    }

    if (m_bSynchronous)
        m_proc->waitForFinished(-1);

    return true;
}

// Slot called when the process should be aborted
void CProcessLauncher::AbortScript(void)
{
    if(m_proc != NULL)
    {
        m_proc->kill();
        delete m_proc;
        m_proc = NULL;
    }
}

// Slot called when the process is finished.
void CProcessLauncher::OnScriptFinished(int /*nExitCode*/,
                                        QProcess::ExitStatus exitStatus)
{
	if(m_proc != NULL)
	{
		m_nProcessDuration = m_Elapsed.elapsed();
		m_nProcessExitStatus = (int) exitStatus;
        GSLOG(7, QString("Process finished. Duration: %1 msec, Exit status: %2").
               arg(QString::number(m_nProcessDuration)).
               arg(QString::number(m_nProcessExitStatus))
               .toLatin1().data() );
		emit sScriptFinished();
	}
}

// Slot called when the process sent data to Stdout
void CProcessLauncher::OnStdoutDataReady(void)
{
	QString strString = m_proc->readAllStandardOutput();
	strString.remove('\n');
	strString.remove('\r');
	strString = strString.trimmed();
	strString += '\n';
	m_strOutput += strString;
    GSLOG(7, QString("Standard ouptut: %1").arg(strString).toLatin1().data() );
	emit sStdoutChanged(m_strOutput);
}

// Slot called when the process sent data to Stderr
void CProcessLauncher::OnStderrDataReady(void)
{
	QString strString = m_proc->readAllStandardError();
	strString.remove('\n');
	strString.remove('\r');
	strString = strString.trimmed();
	strString += '\n';
	m_strError += strString;
    GSLOG(7, QString("Standard error: %1").arg(strString).toLatin1().data() );
	emit sStderrChanged(m_strError);
}

void CProcessLauncher::OnError(QProcess::ProcessError error)
{
    QString lErrorMessage;
    switch(error)
    {
    case QProcess::FailedToStart:
        lErrorMessage = "The process failed to start. Either the invoked program is missing, "
                "or you may have insufficient permissions to invoke the program.";
        break;
    case QProcess::Crashed:
        lErrorMessage = "The process crashed some time after starting successfully.";
        break;
    case QProcess::Timedout:
        lErrorMessage = "The last waitFor...() function timed out.";
        break;
    case QProcess::ReadError:
        lErrorMessage = "An error occurred when attempting to read from the process.";
        break;
    case QProcess::WriteError:
        lErrorMessage = "An error occurred when attempting to write to the process.";
        break;
    case QProcess::UnknownError:
    default:
        lErrorMessage = "An unknown error occurred. ";
        break;
    }
    GSLOG(7, QString("Error: %1").arg(lErrorMessage).toLatin1().data() );
}

