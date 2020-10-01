#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QtCore/QCoreApplication>
#include "QtService.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QDateTime>

// HttpDaemon is the the class that implements the simple HTTP server.
 class HttpDaemon : public QTcpServer
 {
	 Q_OBJECT
 public:
	 HttpDaemon(quint16 port, QObject* parent = 0);
	/*		 : QTcpServer(parent), disabled(false)
	 {
		 qDebug("HttpDaemon %d", port);
		 listen(QHostAddress::Any, port);
	 }
	*/

	 void incomingConnection(int socket)
	 {
		 qDebug("HttpDaemon::incomingConnection");
		 if (disabled)
			 return;

		 // When a new client connects, the server constructs a QTcpSocket and all
		 // communication with the client is done over this QTcpSocket. QTcpSocket
		 // works asynchronously, this means that all the communication is done
		 // in the two slots readClient() and discardClient().
		 QTcpSocket* s = new QTcpSocket(this);
		 connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
		 connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
		 s->setSocketDescriptor(socket);

		 QtServiceBase::instance()->logMessage("New Connection");
	 }

	 void pause()
	 {
		 qDebug("httpdaemon::pause");
		 disabled = true;
	 }

	 void resume()
	 {
		 qDebug("httpdaemon::resume");
		 disabled = false;
	 }

 private slots:
	 void readClient(); // called when a client asks something ('GET' or not)

	 void discardClient()
	 {
		 QTcpSocket* socket = (QTcpSocket*)sender();
		 socket->deleteLater();

		 QtServiceBase::instance()->logMessage("Connection closed");
	 }

 private:
	 bool disabled;
 };

#endif // HTTPDAEMON_H
