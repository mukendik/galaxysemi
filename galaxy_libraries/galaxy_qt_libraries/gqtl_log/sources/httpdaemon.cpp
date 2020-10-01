#include <QtCore/QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>

#include "httpdaemon.h"
#include "gexlogthread.h"


HttpDaemon::HttpDaemon(QMap<QString, QString> atts,  quint16 port, QObject* parent)
	: QTcpServer(parent),
	  // QThread(parent),
	  // COutput(atts),
	  disabled(false)
{
	s_gexloglogfile.write( QString("HttpDaemon::HttpDaemon: threaded = %1").arg(atts["threaded"])
						  .toLatin1().data()
						  );
	#ifdef QT_DEBUG
	 qDebug("HttpDaemon::HttpDaemon on port %d %s", port, atts["threaded"].toLatin1().data() );
	#endif
	if (!listen(QHostAddress::Any, port))
	{
		qDebug("HttpDaemon::HttpDaemon: listen failed !");
	}
	#ifdef QT_DEBUG
		qDebug("HttpDaemon::HttpDaemon ok");
	#endif
}

void HttpDaemon::incomingConnection(int socket)
{
	qDebug("HttpDaemon::incomingConnection");
	if (disabled)
		 return;

	// When a new client connects, the server constructs a QTcpSocket and all
	// communication with the client is done over this QTcpSocket. QTcpSocket
	// works asynchronously, this means that all the communication is done
	// in the two slots readClient() and discardClient().
	QTcpSocket* s = new QTcpSocket(this);
	QObject::connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
	QObject::connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
	s->setSocketDescriptor(socket);
}

QString HttpDaemon::PopFront()
{
	//_log_to_syslog(m.m_atts["msg"]);
	return "ok";
}


void HttpDaemon::discardClient()
{
	QTcpSocket* socket = (QTcpSocket*)sender();
	if (socket)
	{
		qDebug("httpdaemon::discardClient %s", socket->peerAddress().toString().toLatin1().data());
		socket->deleteLater();
	}
}

void HttpDaemon::readClient()
{
	qDebug("HttpDaemon::readClient: disabled=%s", disabled?"true":"false");
	if (disabled)
		return;

	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	QTcpSocket* socket = (QTcpSocket*)sender();
	if (!socket->canReadLine())
	{
		qDebug("HttpDaemon::readClient: cant read line on socket !");
		return;
	}

	//qDebug(socket->readLine());
	QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
	if (tokens.size()==0)
		return;

	//foreach(QString s, tokens)
	//	qDebug(s.toLatin1().data());
	QString auth="";

	if (tokens[0] != "GET")
	{
		qDebug("token %s not supported !", tokens[0].toLatin1().data() );
		return;
	}

	qDebug("HttpDaemon::readClient : GET %s %s %s ",
		   tokens[1].toLatin1().data(), tokens[2].toLatin1().data(), tokens[3].toLatin1().data()
		   );
	QString line="?";
	while (!line.isEmpty())
	{
		line=QString(socket->readLine()).remove('\r').replace('\n','\t'); //line=QString(socket->readLine()).replace('\r',' ').replace('\n','\t');
		//qDebug(line.toLatin1().data());
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

	if (filetosend=="index.html")
	{
		os << "HTTP/1.0 200 Ok\r\n"
			"Content-Type: text/html; charset=\"utf-8\"\r\n"
			"\r\n"
			"<h1><center>Logs at ";
		os << QDateTime::currentDateTime().toString();
		os <<"</center></h1>\n";

		os << "<table>";
		SMessage m;
		foreach (m, m_buffer)
		{
			os << "<tr>\n";
			os << "<td>" << m.m_date.toString(Qt::ISODate) << "</td>";
			os << "<td>" << m.m_time.toString(Qt::ISODate) << "</td>";
			os << "<td>" << QString::number(m.m_sev) << "</td>";
			os << "<td>" << m.m_atts["msg"] << "</td>";
			os << "</tr>\n";
		}
		os << "\n";
		os << "</table>";

		socket->close();
		if (socket->state() == QTcpSocket::UnconnectedState)
		{
			delete socket;
		}
		return;
	}

	qDebug("try to open %s...", filetosend.toLatin1().data());
	QFile file(filetosend);
	QIODevice::OpenMode om=QIODevice::ReadOnly;
	if (filetosend.endsWith("htm") || filetosend.endsWith("g1n") || filetosend.endsWith("html") )
		om=om|QIODevice::Text;

	if (!file.open(om)) //if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
    #ifdef QT_DEBUG
      qDebug("HttpDaemon::readClient(): cant open %s", filetosend.toLatin1().data());
    #endif
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

	//QtServiceBase::instance()->logMessage("Wrote to client");

	if (socket->state() == QTcpSocket::UnconnectedState)
	{
		delete socket;
		//QtServiceBase::instance()->logMessage("Connection closed");
	}

}
