#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "LogFile.h"
#include "gexlogthread.h"
#include <QString>
#include <QTime>

QString _log_to_rtf(int sev, const char* file, const char* m, const char* func, QString path)
{
	if (!bRtfLoaded)
		RtfInit(QString(path).toStdString());

	if (bRtfLoaded)
	{
		std::string t=QTime::currentTime().toString(Qt::ISODate).toStdString();
		// MSG_SUCCESS = 1 MSG_INFO = 2 MSG_WARN = 3, MSG_ERROR = 4
		RtfLogMessage(
			t,
			std::string(func?func:"?"),
			std::string(m),
			(SYSLOG_SEV)sev );

		if (sev<SYSLOG_SEV_WARNING)
			RtfLogMessage(t, " in file : ", file, (SYSLOG_SEV)sev);
	}

	//RtfClose(); // will be done atExit(...)

	return "ok";
}

CRtfOutput::CRtfOutput(QMap< QString, QString > a) : COutput(a)
{
	if (!a["output"].isEmpty())
	{
		m_fullpath=a["outputfile"];
		ReplaceWithUsualVariables(m_fullpath, a);
		QFileInfo fi(m_fullpath);
		QDir d( fi.absolutePath() );
		if (!d.exists())
			d.mkpath( fi.absolutePath() );
	}
	else
	{
		m_fullpath=QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"logs"+QDir::separator();
		m_fullpath += QDate::currentDate().toString(Qt::ISODate);
		QDir d(m_fullpath);
		if (!d.exists())
			d.mkpath(m_fullpath);
		m_fullpath += QDir::separator();
        m_fullpath += "gslog_";
		m_fullpath += QDate::currentDate().toString(Qt::ISODate);
        m_fullpath += "_"+QString::number(QCoreApplication::applicationPid());
		m_fullpath += ".rtf";
	}
	#ifdef QT_DEBUG
	 qDebug("CRtfOutput : outputfile = %s", m_fullpath.toLatin1().data());
	#endif
}

bool CRtfOutput::PopFront()
{
	if (m_buffer.size()==0)
        return true;
	SMessage m=m_buffer.takeLast();	// takeFirst ?

	_log_to_rtf(m.m_sev, m.m_atts["file"].toLatin1().constData(),
				m.m_atts["msg"].toLatin1().constData(),
				m.m_atts["func"].toLatin1().constData(),
				m_fullpath
				);

    return true;
}
