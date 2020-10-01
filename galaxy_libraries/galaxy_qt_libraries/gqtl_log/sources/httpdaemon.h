#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QtCore/QCoreApplication>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QHostAddress>
#include <QDateTime>
#include "gexlogthread.h"

// HttpDaemon is the the class that implements the simple HTTP server.
 class HttpDaemon : public QTcpServer
		 //, public QThread
		 //, public COutput
 {
	 Q_OBJECT

 public:
	 HttpDaemon(QMap<QString, QString> atts, quint16 port, QObject* parent = 0);
		 //: QTcpServer(parent),
		//COutput(),
		//disabled(false)

	QList< SMessage > m_buffer;

	QString PopFront();

	void incomingConnection(int socket);

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
	 void discardClient();

 private:
	 bool disabled;
 };

#endif // HTTPDAEMON_H
