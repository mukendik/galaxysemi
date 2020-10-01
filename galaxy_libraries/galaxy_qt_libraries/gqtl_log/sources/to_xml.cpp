#include <QString>
#include <QDomDocument>
#include <QDir>
#include <QFile>
#include "gexlogthread.h"

static QDomDocument s_by_time_domdoc("log_by_time");
static QDomDocument s_by_file_domdoc("log_by_file");
static QDomDocument s_by_func_domdoc("log_by_func");
static QDomDocument s_by_severity_domdoc("log_by_severity");

CXmlOutput::CXmlOutput(QMap< QString, QString > a) : COutput(a)
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
		m_fullpath += "gexlog_";
		m_fullpath += QDate::currentDate().toString(Qt::ISODate);
        m_fullpath += "_"+QString::number(QCoreApplication::applicationPid());
		m_fullpath += ".xml";
	}
	#ifdef QT_DEBUG
	 qDebug("CXmlOutput : outputfile = %s", m_fullpath.toLatin1().data());
	#endif
}

QString _log_to_xml(int sev, const char* func, const char* m, QString fullpath)
{
	QString msg(m);
	msg=msg.simplified();
	msg.remove(QChar('\t'));
	msg.remove(QChar('\n'));
	msg.remove(QChar('"'));
	if (func)
	{	msg.prepend(": "); msg.prepend(func);
	}

	QDomElement root_by_time=s_by_time_domdoc.firstChildElement("log");
	if (root_by_time.isNull())
	{	s_by_time_domdoc.appendChild(s_by_time_domdoc.createElement("log"));
		root_by_time=s_by_time_domdoc.firstChildElement("log");
	}


	/*
	QDomElement e=doc.firstChildElement(func);
	if (e.isNull())
		e=doc.createElement(func);
	e.appendChild(doc.createElement(m));
	*/
	QDomElement e=s_by_time_domdoc.createElement("M");

	if (func!=NULL)
		e.setAttribute("f", func);
	e.setAttribute("m", QString(m));

	root_by_time.appendChild(e);

	// update file : when ?
	if (sev>6)
		return "ok";

	QFile f(fullpath);
	if (!f.open(QIODevice::Truncate|QIODevice::WriteOnly))
	{
		#ifdef QT_DEBUG
		 qDebug( "gexlog: error : cant open/write file %s !", fullpath.toLatin1().data() );
		#endif
		return "error";
	}

	//bool error=false;
	if (f.write(s_by_time_domdoc.toByteArray())==-1)
	{
		f.close();
		//error = true;
		#ifdef QT_DEBUG
		 qDebug("gexlog: error when writing to xml !");
		#endif
		return "error writing file !";		
	}
	f.close();

	/*
	// order by file containing the message
	if (file)
	{
		QDomElement root_by_file=s_by_file_domdoc.firstChildElement("log");
		if (root_by_file.isNull())
		{
			s_by_file_domdoc.appendChild(s_by_file_domdoc.createElement("log"));
			root_by_file=s_by_file_domdoc.firstChildElement("log");
		}

		QDomElement file_elem=root_by_file.firstChildElement(file);
		if (file_elem.isNull())
		{
			file_elem=s_by_file_domdoc.createElement(file);
			root_by_file.appendChild(file_elem);
		}
		QDomElement e=s_by_file_domdoc.createElement(func);
		e.setAttribute("m", msg);
		file_elem.appendChild(e);

		if (sev<6)
		{
			QFile by_file_f(p + "_by_file.xml" );
			if (by_file_f.open(QIODevice::Truncate|QIODevice::WriteOnly))
			{
				QByteArray byfile_ba=s_by_file_domdoc.toByteArray();
				by_file_f.write(byfile_ba);
				by_file_f.close();
			}
		}
	}
	*/

	/*
	// by severity level
	if ( (sev>=(int)SYSLOG_SEV_EMERGENCY) && (sev<=(int)SYSLOG_SEV_DEBUG) )
	{
		QDomElement root= s_by_severity_domdoc.firstChildElement("log");
		if (root.isNull())
		{
			s_by_severity_domdoc.appendChild(s_by_severity_domdoc.createElement("log"));
			root=s_by_severity_domdoc.firstChildElement("log");
		}

		QDomElement elem=root.firstChildElement( QString("sev_%1").arg(sev) );
		if (elem.isNull())
		{
			elem=s_by_severity_domdoc.createElement( QString("sev_%1").arg(sev) );
			root.appendChild(elem);
		}
		QDomElement e=s_by_severity_domdoc.createElement(func);
		e.setAttribute("m", msg);
		elem.appendChild(e);

		if (sev<6)
		{
			QFile sev_f(p+"_by_sev.xml");
			if (sev_f.open(QIODevice::Truncate|QIODevice::WriteOnly))
			{
				sev_f.write(s_by_severity_domdoc.toByteArray());
				sev_f.close();
			}
		}
	} // by sev
	*/


	/*
	// Order by function
	if (func)
	{
		QDomElement root=s_by_func_domdoc.firstChildElement("log");
		if (root.isNull())
		{
			s_by_func_domdoc.appendChild(s_by_func_domdoc.createElement("log"));
			root=s_by_func_domdoc.firstChildElement("log");
		}
		QDomElement elem=root.firstChildElement( QString(func).simplified() );
		if (elem.isNull())
		{
			elem=s_by_func_domdoc.createElement( QString(func).simplified() );
			root.appendChild(elem);
		}
		QDomElement e=s_by_func_domdoc.createElement(QString("sev_%1").arg(sev));
		e.setAttribute("m", msg);
		elem.appendChild(e);

		if (sev<6)
		{
			QFile f(p+"_by_func.xml");
			if (f.open(QIODevice::Truncate|QIODevice::WriteOnly))
			{
				f.write(s_by_func_domdoc.toByteArray());
				f.close();
			}
		}
	}
	*/


	return "ok";
}

bool CXmlOutput::PopFront()
{
	if (m_buffer.isEmpty())
        return true;
	SMessage m=m_buffer.takeFirst();
	_log_to_xml(m.m_sev,
				m.m_atts["func"].toLatin1().constData(),
				m.m_atts["msg"].toLatin1().constData(),
				m_fullpath
				);
    return true;
}
