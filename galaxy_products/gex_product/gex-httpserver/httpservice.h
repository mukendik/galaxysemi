#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H

#include "httpdaemon.h"

#include <QtCore/QCoreApplication>
#include "QtService.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>

class HttpService : public QtService<QCoreApplication>
{
 public:
	 HttpService(int argc, char **argv)
		 : QtService<QCoreApplication>(argc, argv, "Gex HTTP Daemon")
	 {
		 qDebug("HttpService");
		 setServiceDescription("GalaxySemi HTTP service");
		 setServiceFlags(QtServiceBase::CanBeSuspended);
	 }

 protected:
	 void start()
	 {
		 qDebug("HttpService::start");
		 QCoreApplication *app = application();

		 quint16 port = (app->argc() > 1) ?
				 QString::fromLocal8Bit(app->argv()[1]).toUShort() : 8080;
		 daemon = new HttpDaemon(port, app);

		 if (!daemon->isListening())
		 {
			 logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
			 app->quit();
		 }
	 }

	 void pause()
	 {
		 qDebug("HttpService::pause");
		 daemon->pause();
	 }

	 void resume()
	 {
		 qDebug("HttpService::resume");
		 daemon->resume();
	 }

 private:
	HttpDaemon *daemon;

};

#endif // HTTPSERVICE_H
