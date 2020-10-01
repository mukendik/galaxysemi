#include <QUdpSocket>
#include <QCoreApplication>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include "gexlogthread.h"

static QUdpSocket* s_udpSocket=NULL;
static int s_syslog_port=514;
static QString s_ErrorMessage;

CSyslogOutput::CSyslogOutput(QMap< QString, QString > a) : COutput(a)
{
    m_tcpSocket=NULL;
    m_udpSocket=NULL;

	if (!a["target"].isEmpty())
		m_hostAddress.setAddress(a["target"]);

	if (m_hostAddress.isNull())
		m_hostAddress.setAddress(QHostAddress::LocalHost);

    bool ok=false;
    m_port=a["port"].toInt(&ok);
    if (!ok)
        m_port=514;

	if (a["protocol"]=="tcp")
    {
        qDebug("Creating TCP socket...");

        m_tcpSocket=new QTcpSocket(this);

        //m_tcpSocket->open(QIODevice::WriteOnly);
        bool b=false;
        b=(bool)connect( m_tcpSocket, SIGNAL(connectionClosed()), this, SLOT(onConnectionClosed()) );
        b=(bool)connect( m_tcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()) );
        b=b&(bool)connect( m_tcpSocket, SIGNAL(connected()), this, SLOT(connected()) );
        b=b&(bool)connect( m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()) );
        b=b&(bool)connect( m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(error(QAbstractSocket::SocketError)) );
        b=b&(bool)connect( m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                              this, SLOT(stateChanged(QAbstractSocket::SocketState)) );
        connect( m_tcpSocket, SIGNAL(readyRead()),this, SLOT(readyRead()) );

        if (!b)
            qDebug("Cannot connect some TCP socket signal");

        qDebug("CSyslogOutput : trying to connect to %s on %d...", a["target"].toLatin1().data(), m_port);
        m_tcpSocket->connectToHost(a["target"], m_port); // 514, 5140 or 1468 ?
        if (m_tcpSocket->waitForConnected(1000))
            qDebug("CSyslogOutput : connected");

        //m_tcpSocket->setSocketOption();
    }
	else
        m_udpSocket=new QUdpSocket();

    #ifdef QT_DEBUG
        qDebug("CSyslogOutput : target:%s port:%d prot=%s",
               m_hostAddress.toString().toLatin1().data(), m_port, a["protocol"].toLatin1().data() );
    #endif
}

CSyslogOutput::~CSyslogOutput()
{
    if (m_tcpSocket != NULL)
    {
        delete m_tcpSocket;
    }
    if (m_udpSocket != NULL)
    {
        delete m_udpSocket;
    }
}

void CSyslogOutput::readyRead()
{
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::readyRead ");
    #endif
}

void CSyslogOutput::stateChanged(QAbstractSocket::SocketState lSS)
{
    Q_UNUSED(lSS)
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::stateChanged: %d", lSS);
    #endif
}

void CSyslogOutput::connected()
{
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::connected tcp");
    #endif

    QTextStream lTS(m_tcpSocket);
    lTS << "srzerze\n";
    lTS << "<165>May 18 14:46:18 127.0.0.1 connected\n";
    lTS.flush();
    m_tcpSocket->flush();
    m_tcpSocket->waitForBytesWritten(100);
}

void CSyslogOutput::disconnected()
{
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::disconnected tcp");
    #endif
}

void CSyslogOutput::onConnectionClosed()
{
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::onConnectionClosed ");
    #endif
}

void CSyslogOutput::hostFound()
{
    //QTextStream lTS(m_tcpSocket);
    //lTS << "<165>May 18 14:46:18 127.0.0.1 Un message Syslog classique\n";

    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::hostFound tcp");
    #endif
}

void CSyslogOutput::error(QAbstractSocket::SocketError lSE)
{
    Q_UNUSED(lSE)
    #ifdef QT_DEBUG
        qDebug("CSyslogOutput::socketError %d tcp", (int)lSE);
    #endif
}

QString
_log_to_syslog(int sev, const char* file, const char* m, const char* func, QMap<QString, QString> atts)
{
    /*
    // PRI : exemple <5>
    // La partie PRI d'un message Syslog est composée obligatoirement de 3, 4 ou 5 caractères.
    // Le premier caractère est toujours le caractère "<" suivi par un nombre qui représente la priorité (en base 10) du message et suivi par le caractère ">".
    // La seule fois où le caractère "0" peut suivre le caractère "<" est pour coder une priorité dont la valeur est 0 justement. Dans tous les autres cas, le ou les caractères "0" de tête ne doivent pas être présents.

    La priorité d'un message Syslog est définie par sa fonctionnalité et sa sévérité.
    // Cette priorité est un nombre qui est le résultat de la multiplication de la fonctionnalité par 8 auquel est ajoutée la sévérité.
    Ainsi un message de priorité 165 aura pour fonctionnalité 20 (c'est-à-dire local use 4) et pour sévérité 5 (c'est-à-dire Notice).
    Il ne peut y avoir de priorité plus grande que 191 car la fonctionnalité la plus grande définie par les RFC est 23 et la sévérité la plus grande est 7 : (23 * 8) + 7 = 191.
    Le mot priorité est certainement mal choisi car un message de priorité importante ne sera pas traité ou acheminé plus rapidement qu'un message de moindre priorité.
    */
    QString msg;
    msg.append("<"+QString::number(16*8+sev)+">");	// 16 =	local use 0
    /*
      // La partie HEADER d'un message Syslog contient 2 champs :
        // Le champ TIMESTAMP
        Le champ TIMESTAMP doit immédiatement suivre le caractère ">" de la partie HEADER.
        Le format de date utilisé par le champ TIMESTAMP est "Mmm dd hh:mm:ss".
        "Mmm" est l'abréviation anglaise pour le mois sur 3 caractères. Les seules valeurs admises sont : "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec".
        Un caractère espace (" ") doit obligatoirement suivre la valeur du nom du mois.
        "dd" représente le numéro de jour dans le mois. Ce numéro de jour doit toujours être représenté par 2 caractères. Si ce numéro de jour est inférieur à 10, il doit alors être précédé du caractère espace (" ").
        Un caractère espace (" ") doit obligatoirement suivre la valeur numéro de jour.
        "hh:mm:ss" est l'heure locale. L'heure est représentée en utilisant un format sur 24 heures. Les valeurs autorisées vont de "00" à "23". Les valeurs autorisées pour les minutes ("mm") et les secondes ("ss") vont de "00" à "59".
        Un caractère espace (" ") doit obligatoirement suivre le champ TIMESTAMP.
    */
    msg.append(QDateTime::currentDateTime().toString("MM dd hh:mm:ss "));
    //msg.append(QDate::currentDate().toString("MMM dd "));
    /*
        // Le champ HOSTNAME
        Le champ HOSTNAME peut contenir :
        Un nom de machine sans son nom de domaine. Un nom de machine ne doit pas contenir de caractère espace (" ").
        Une adresse IP au format IPv4 (format décimal pointé 192.168.1.1 par exemple).
        Une adresse IP au format IPv6 (voir la RFC RFC 2373 pour les différentes notations supportées).
        Rien, le champ HOSTNAME est facultatif.
        Un caractère espace (" ") doit obligatoirement suivre le champ HOSTNAME (même si ce champ est vide).
    */
    static QString lhn=QHostInfo::localHostName(); // returns 127.0.0.1

    msg.append(QString("%1 %2[%3]: ")
            .arg(lhn.isEmpty()?"?":lhn)
            .arg(atts["module"].toLatin1().data())
            .arg(QCoreApplication::applicationPid())
               );

    msg.append(m);
    msg.append(QString(" in function %1").arg(func?func:"?"));
    msg.append(QString(" in file %1").arg(file?file:"?"));

	if (s_udpSocket)
	{
		QHostAddress ha(atts["target"]);
		if (ha.isNull())
			return "error : cant resolve target!";

		qint64 n=0;
		n=s_udpSocket->writeDatagram(msg.toLatin1().data(),
						(qint64)msg.size(),
						ha,	//QHostAddress::LocalHost,
						s_syslog_port);

		if (n==-1)
		{
			s_ErrorMessage=s_udpSocket->errorString();
			n=s_udpSocket->writeDatagram(
					s_ErrorMessage.toLatin1().data(),
					(qint64)s_ErrorMessage.size(),
					QHostAddress::LocalHost,
					s_syslog_port);
		}
	}

	return "ok";
}

bool CSyslogOutput::PopFront()
{
	if (m_buffer.isEmpty())
        return true;
	SMessage m=m_buffer.takeFirst();

	qint64 n=0;

    //QString msg;
    msg.clear();
    msg.append("<"+QString::number(16*8+ m.m_sev)+">");	// 16 =	local use 0

    static char *month[] = {(char*)"Jan", (char*)"Feb", (char*)"Mar", (char*)"Apr", (char*)"May", (char*)"Jun",
                            (char*)"Jul", (char*)"Aug", (char*)"Sep", (char*)"Oct", (char*)"Nov", (char*)"Dec" };
    // We cannot use localized month name as could be uncompliant with syslog standard as for example the french month name.
    //msg.append(QDateTime::currentDateTime().toString("MMM dd hh:mm:ss "));
    msg.append(QString(month[QDate::currentDate().month()-1])+ QDateTime::currentDateTime().toString(" dd hh:mm:ss "));

	static QString lhn=QHostInfo::localHostName(); // returns 127.0.0.1 or MYMACHINE
    msg.append(QString("%1 %2[%3]: ")
            .arg(lhn.isEmpty()?"?":lhn)
            .arg(m.m_atts["module"])
            .arg(QCoreApplication::applicationPid())
    );
	msg.append(m.m_atts["msg"]);
	msg.append(QString(" in function %1").arg(m.m_atts["func"]));
	msg.append(QString(" in file %1 ").arg(m.m_atts["file"]) );

    if (m_atts["protocol"]=="tcp")
    {
        if (!m_tcpSocket)
            return "error";
        QTextStream lTS(m_tcpSocket);
        lTS<<msg<<"\n\n";
        lTS.flush();
        return "ok";
    }

    if (m_udpSocket)
    {
        // limit to 1000 chars max as standard syslog is anyway limiting to 1Kb per packet
        msg.truncate(1000);
        n=m_udpSocket->writeDatagram(msg.toLatin1().data(),
           (qint64)msg.size(),
           m_hostAddress,	//ha,	//QHostAddress::LocalHost,
           m_port);

        if (n==-1)
        {
            s_ErrorMessage=m_udpSocket->errorString();
            n=m_udpSocket->writeDatagram(
                 s_ErrorMessage.toLatin1().data(),
                 (qint64)s_ErrorMessage.size(),
                 m_hostAddress,	//QHostAddress::LocalHost,
                 m_port);
            #ifdef QT_DEBUG
                printf("gqtl_log: Failed to send syslog packet: %s", m.m_atts["msg"].replace('\n',' ').toLatin1().data());
            #endif
        }
    }

	//_log_to_syslog(m.m_sev, m.m_atts["file"].toLatin1().data(), m.m_atts["msg"].toLatin1().data(),
		//		   m.m_atts["func"].toLatin1().data(), m_atts);
	return "ok";
}
