#include <QtCore/QCoreApplication>

#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>
#include <QDir>

#include "QtService.h"

#include "httpservice.h"
#include "httpdaemon.h"
//#include "libgexlog.h"
#include <gqtl_log.h>


// The directory that will be published by the server
//QString public_dir="../public";
QString public_dir="public";

// 1 - first launch with '-i' argument (in QtCreator : Project > Run params) (Is it really useful ??????)
// 1.5 - set your public_dir
// 2 - then launch with '-e' arg

int main(int argc, char *argv[])
{
	#if !defined(Q_WS_WIN)
	 // QtService stores service settings in SystemScope, which normally require root privileges.
	 // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
	 QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
	 qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
	#endif

	qDebug("main: public dir will be '%s'", public_dir.toLatin1().data());

	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("currentPath=%1").arg(QDir::currentPath().toLatin1().data()));


	QDir::setCurrent(public_dir);

	HttpService service(argc, argv);
	return service.exec();
}
