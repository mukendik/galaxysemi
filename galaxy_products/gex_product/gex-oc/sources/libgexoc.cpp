#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <QDate>
#include <QDir>
#include <QDomDocument>
#include <QWidget>

#include "libgexoc.h"
#include <gqtl_log.h>
#include "options_center_widget.h"
#include "gex_version.h"
#include "gex_scriptengine.h"

static QString s_ErrorMessage;

static OptionsCenterWidget* s_ocw=NULL;
GexScriptEngine* s_gse=NULL;

extern "C" LIBGEXOCSHARED_EXPORT
OptionsCenterWidget* GetOCInstance(
  QWidget* parent, GexScriptEngine *gse, GexMainwindow* gmw, int loglevel, QString& strOutputMsg)
{
	// ToDo : remove loglevel arg
    GSLOG(SYSLOG_SEV_NOTICE, QString("GetOCInstance for GEX version '%1' GexScriptingEengine '%2' loglevel=%3")
             .arg(GEX_APP_VERSION)
             .arg(gse?gse->objectName().toLatin1().data():"NULL")
             .arg(loglevel).toLatin1().constData());

	if (!parent)
    {
        GSLOG(SYSLOG_SEV_WARNING,"parent Widget NULL ! Aborting...");
		return NULL;
	}

	if (!gse)
    {
        GSLOG(SYSLOG_SEV_WARNING,"GexScriptEngine NULL ! Aborting...");
		return NULL;
	}
	s_gse=gse;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("AppFullName found in ScriptEngine : %1")
            .arg(s_gse->evaluate("GexEngine.GetAppFullName()").toString()).toLatin1().constData());

    GSLOG(SYSLOG_SEV_NOTICE, QString("GexProduct found in ScriptEngine : %1")
             .arg(s_gse->globalObject().property(PROP_GEXPRODUCT).toString()).toLatin1().constData() );

	if (!s_ocw)
		s_ocw=new OptionsCenterWidget(gmw, parent);

    QScriptValue OCobject = s_gse->newQObject((QObject*) s_ocw);
        if (!OCobject.isNull())
            s_gse->globalObject().setProperty("GexOptionsCenter", OCobject);

	QString ret= s_ocw->BuildFromGOXML(":/gex/xml/gex_options.xml");
	if (ret.startsWith("error"))
	{
		strOutputMsg = ret + QString("\nerror : can't build options tree from options xml file !");
        GSLOG(SYSLOG_SEV_ERROR, QString("build from xml failed : %1").arg(ret).toLatin1().constData());
		GEX_ASSERT(false);
	}

	return s_ocw;
}

extern "C" LIBGEXOCSHARED_EXPORT
bool SetOption(QString s, QString n, QString v)
{
	if (!s_ocw)
		return false;
	return s_ocw->SetOption(s,n,v);
}
