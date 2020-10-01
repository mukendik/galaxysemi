#include <QtCore/QCoreApplication>

#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>
//#include <libgexlog.h>
#include <gqtl_log.h>

#include "QtService.h"

#include "httpservice.h"
#include "httpdaemon.h"


HttpDaemon::HttpDaemon(quint16 port, QObject* parent)
	: QTcpServer(parent), disabled(false)
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("on port %1").arg(port));
	bool b=listen(QHostAddress::Any, port);
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("listen %1").arg(b?"ok":"failed"));
}

void HttpDaemon::readClient()
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" disabled : %1").arg(disabled?"true":"false"));
	if (disabled)
		return;

	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	QTcpSocket* socket = (QTcpSocket*)sender();
	if (socket->canReadLine())
	{
		//qDebug(socket->readLine());
		QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
		//foreach(QString s, tokens)
		//	qDebug(s.toLatin1().data());
		QString auth="";

		if (tokens[0] == "GET")
		{
			qDebug("\tGET %s %s %s ", tokens[1].toLatin1().data(), tokens[2].toLatin1().data(), tokens[3].toLatin1().data()
				   );
			QString line="?";
			while (!line.isEmpty())
			{	line=QString(socket->readLine()).remove('\r').replace('\n','\t'); //line=QString(socket->readLine()).replace('\r',' ').replace('\n','\t');
				qDebug(line.toLatin1().data());
				if (line.startsWith("Authorization: Basic"))
					auth=line.section(' ',-1);	// should be something like : 'Authorization: Basic dG90bzp0aXRp'
			}
			//QStringList tokens2 = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
			//foreach(QString s, tokens2) qDebug(s.toLatin1().data()); //for (int i=0; i<tokens.size(); i++) tokens2

			QTextStream os(socket);

			os.setAutoDetectUnicode(true);

			QString filetosend="index.html";
			if (tokens[1]!="/")
				filetosend=tokens[1];
			if (filetosend.startsWith('/'))
				filetosend.prepend('.');

			// Remove ?xxxx : supported by std HTTP daemons to force reloading file from disk
			if(filetosend.contains("?"))
				filetosend = filetosend.section('?', 0, 0);

			qDebug("try to open %s...", filetosend.toLatin1().data());

			QFile file(filetosend);
			QIODevice::OpenMode om;
			om=QIODevice::ReadOnly;
			if (filetosend.endsWith("htm") || filetosend.endsWith("g1n") || filetosend.endsWith("html") )
				om=om|QIODevice::Text;

			if (!file.open(om)) //if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				qDebug("\tcant open %s", filetosend.toLatin1().data());
				// test : send auth request :
				os << "HTTP/1.0 200 Ok\r\n"
					"Content-Type: text/html; charset=\"utf-8\"\r\n"
					"\r\n"
					"<h1><center>cant open file."
					"</center></h1>\n"
					<< QDateTime::currentDateTime().toString() << "\n";

				/*
				if (auth=="")
				{	qDebug("\tsending auth request...");
					os << "HTTP/1.1 401 Authorization Required\r\n"
						"Content-Type: text/html; charset=\"utf-8\"\r\n"
						"WWW-Authenticate: Basic realm=\"Access restreint\"\r\n"
						"<html><body><h1><center>Please auth to enter Gex http Server</center></h1></body></html>\n"
						<< QDateTime::currentDateTime().toString() << "\n";
				}
				else
				{	qDebug("\tauth = '%s' = '%s'", auth.toLatin1().data(), QByteArray::fromBase64(auth.toLatin1()).data());
					os << "HTTP/1.0 200 Ok\r\n"
						"Content-Type: text/html; charset=\"utf-8\"\r\n"
						"\r\n"
						"<h1><center>Authentification successful ! Welcome !</center></h1>\n"
						<< QDateTime::currentDateTime().toString() << "\n";
				}
				*/
				socket->close();

				return;
			}

			if (filetosend.endsWith("htm") || filetosend.endsWith("g1n") || filetosend.endsWith("html") )
			{
				// ascii file
				while (!file.atEnd())
				{
				   QByteArray line = file.readLine();
				   os<<line;
				   //process_line(line);
				}
			}
			else
			{
				// bin file
				QByteArray ba=file.readAll(); //()
				socket->write(ba);
			}

			/*
			os << "HTTP/1.0 200 Ok\r\n"
				"Content-Type: text/html; charset=\"utf-8\"\r\n"
				"\r\n"
				"<h1><center>Welcome to Gex http Server</center></h1>\n"
				<< QDateTime::currentDateTime().toString() << "\n";
			*/
			socket->close();

			QtServiceBase::instance()->logMessage("Wrote to client");

			if (socket->state() == QTcpSocket::UnconnectedState)
			{
				delete socket;
				QtServiceBase::instance()->logMessage("Connection closed");
			}
		}
	}

}
