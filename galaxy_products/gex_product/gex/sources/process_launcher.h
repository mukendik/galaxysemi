///////////////////////////////////////////////////////////
// Class for launching processes
///////////////////////////////////////////////////////////

#ifndef PROCESS_LAUNCHER_H
#define PROCESS_LAUNCHER_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <QProcess>

class CProcessLauncher: public QObject
{
    Q_OBJECT

public:
    CProcessLauncher(const QString& strCommand,
                     const QStringList& strArguments,
                     bool bSynchronous = false);
	~CProcessLauncher();
	
private:
	QProcess*	m_proc;
	QTime		m_Elapsed;
	QString		m_strCommand;
	QStringList	m_strArguments;
	QStringList	m_strFullCommandline;
	int			m_nProcessExitStatus;
	int			m_nProcessDuration;
	QString		m_strOutput;
	QString		m_strError;
	bool		m_bSynchronous;				// true if the launcher has to wait the end of the process before exiting
	
signals:
    void sScriptFinished();
	void sStdoutChanged(const QString &);
	void sStderrChanged(const QString &);

protected slots:
    void OnError(QProcess::ProcessError error);

public slots:
	void		OnScriptFinished(int nExitCode, QProcess::ExitStatus exitStatus);
	void		OnStdoutDataReady(void);
	void		OnStderrDataReady(void);
    bool		Launch(void);
    void		AbortScript(void);
    int			GetScriptStatus(void) const	{ return m_nProcessExitStatus; }
    int			GetScriptDuration(void)	const { return m_nProcessDuration; }
};

#endif // PROCESS_LAUNCHER_H

