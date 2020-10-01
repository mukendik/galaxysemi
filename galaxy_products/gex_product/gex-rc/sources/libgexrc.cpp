#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <QDate>
#include <QDir>
#include <QDomDocument>
#include <QWidget>

#include "libgexrc.h"
//#include "libgexlog.h"
#include <gqtl_log.h>
#include "gex_version.h"


static QString s_ErrorMessage;

/*
static OptionsCenterWidget* s_ocw=NULL;

extern "C" LIBGEXOCSHARED_EXPORT
OptionsCenterWidget* GetOCInstance(QWidget* parent, GexMainwindow* gmw)
{
	//gexlog_level=6;
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("GetOCInstance for version %1").arg(GEX_APP_VERSION));
	if (!s_ocw)
		s_ocw=new OptionsCenterWidget(gmw, parent);
	return s_ocw;
}

extern "C" LIBGEXOCSHARED_EXPORT
bool SetOption(QString s, QString n, QString v)
{
	if (!s_ocw)
		return false;
	return s_ocw->SetOption(s,n,v);
}

*/
