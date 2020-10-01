#include <QString>
#include <QDir>
#include <QDate>
#include "gexlogthread.h"

QString _log_to_csv(int sev, const char* file, const char* m, const char* func, QString /*path*/)
{
    FILE *hFile=0;
    QString	strTraceFile;
	strTraceFile = QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"logs"+QDir::separator();
	strTraceFile += QDate::currentDate().toString(Qt::ISODate);
	QDir d(strTraceFile);
	if (!d.exists())
		if (!d.mkpath(strTraceFile))
			return "error";
	strTraceFile += QDir::separator();
	strTraceFile += "gexlog_";
	strTraceFile += QDate::currentDate().toString(Qt::ISODate);
	strTraceFile += ".csv";

	hFile = fopen(strTraceFile.toLatin1().constData(),"a");
	if(hFile == NULL)
		return "error";

	fprintf(hFile,"%d, %s, %s, %s, %s,\n",
		//QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(),
		sev,
		QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(),
		m?QString(m).replace(',',' ').replace('\n',' ').toLatin1().data():"?",
		func?QString(func).replace(',', ' ').toLatin1().data():"?",
		file?file:"?"
		);
	fclose(hFile);
	return "ok";
}

bool CCsvOutput::PopFront()
{
	if (m_buffer.size()==0)
        return true;
	SMessage m=m_buffer.takeLast();	// takeFirst ?
	_log_to_csv(m.m_sev,
				m.m_atts["file"].toLatin1().constData(),
				m.m_atts["msg"].toLatin1().constData(),
				m.m_atts["func"].toLatin1().constData(),
				"");

    return true;
}
