#include <QString>
#include <QDir>
#include <QDate>
#include "gexlogthread.h"
extern QFile s_gexloglogfile;

CTxtOutput::CTxtOutput(QMap< QString, QString > a) : COutput(a)
{
  if (a["outputfile"].isEmpty())
	{
     // default outputfile : $HOME/GalaxySemi/logs/$ISODATE/$MODULE_$PID.txt
     m_atts.insert("outputfile", "$HOME/GalaxySemi/logs/$ISODATE/$MODULE_$PID.txt");
	}
	#ifdef QT_DEBUG
	 qDebug("CTxtOutput : outputfile = %s", m_atts["outputfile"].toLatin1().data());
	#endif
  s_gexloglogfile.write(QString("%1 : CTxtOutput : outputfile = %2\n")
                        .arg(QTime::currentTime().toString(Qt::ISODate))
                        .arg( m_atts["outputfile"]).toLatin1().data() );
}

QString _log_to_txt(const SMessage &m, QString fullpath
	//int sev, const char* file, const char* m, const char* func, QString fullpath
	)
{
	FILE* hFile = fopen(fullpath.toLatin1().constData(),"a");
	if(hFile == NULL)
		return "error";
	QString msg=m.m_atts["msg"];
	fprintf(hFile,"[%s %s %s %d] %s  \tin %s  \tin %s\n",
		QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(),
		QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(),
		m.m_atts["module"].toLatin1().data(),
		m.m_sev,
		msg.replace('\n',' ').toLatin1().data(),
		m.m_atts["func"].toLatin1().data(),	//func?func:"?",
		m.m_atts["file"].toLatin1().data() //file?file:"?"
		);
	fclose(hFile);

	return "ok";
}

bool CTxtOutput::PopFront()
{
	if (m_buffer.isEmpty())
        return true;
	SMessage m=m_buffer.takeLast();

	//foreach()
	//this->m_atts.insert()
	QMap<QString, QString> a=this->m_atts;
	a.insert("file", m.m_atts["file"]);
	a.insert("module", m.m_atts["module"]);
	QString p=m_atts["outputfile"];
	ReplaceWithUsualVariables(p, a);

	QFileInfo fi(p);
	QDir d( fi.absolutePath() );
	if (!d.exists())
		d.mkpath( fi.absolutePath() );

	_log_to_txt(m,p);

    return true;
}
