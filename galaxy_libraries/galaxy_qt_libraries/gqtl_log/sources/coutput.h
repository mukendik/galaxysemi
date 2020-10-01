#ifndef COUTPUT_H
#define COUTPUT_H

#include <QDate>
#include <QMap>
#include <QUdpSocket>
#include <QTcpSocket>

struct SMessage
{
	int m_sev;
	QDate m_date;
	QTime m_time;
	QMap< QString, QString> m_atts;
};

class COutput : public QObject
{
	Q_OBJECT

public:
	COutput(QMap< QString, QString > a)
	{
		m_atts=a;
		s_ListCOutput.push_back(this);
    }
    virtual ~COutput() { m_buffer.clear(); }
	QList< SMessage > m_buffer;
	QMap< QString, QString> m_atts;
	QString Flush(); // { while (!m_buffer.isEmpty()) PopFront(); return "ok"; };

    virtual bool PopFront()=0;
	static QList<COutput*> s_ListCOutput;
    void newMessage(const SMessage& m) { emit newMessageSignal(m); }
signals:
	void newMessageSignal(const SMessage& m);
};

class CConsoleOutput : public COutput
{
public:
	CConsoleOutput(QMap< QString, QString > a) : COutput(a)
	{
    }
    bool PopFront();
};

class CRtfOutput : public COutput
{
public:
	CRtfOutput(QMap< QString, QString > a);
    bool PopFront();
	QString m_fullpath;
};

class CCsvOutput : public COutput
{
public:
    CCsvOutput(QMap< QString, QString > a) : COutput(a) { }
    bool PopFront();
};

class CSqlOutput : public COutput
{
public:
    CSqlOutput(QMap< QString, QString > a) : COutput(a) { }
    bool PopFront();
};

class CSyslogOutput : public COutput
{
    Q_OBJECT

    QString msg; // rename me
public:
	CSyslogOutput(QMap< QString, QString > a);
    virtual ~CSyslogOutput();
    bool PopFront();
	QUdpSocket* m_udpSocket;
	QTcpSocket* m_tcpSocket;
	QHostAddress m_hostAddress;
	quint16 m_port;
public slots:
    void error(QAbstractSocket::SocketError);
    void stateChanged(QAbstractSocket::SocketState lSS);
    void hostFound();
    void connected();
    void disconnected();
    void readyRead();
    void onConnectionClosed();
};

class CTxtOutput : public COutput
{
public:
	CTxtOutput(QMap< QString, QString > a);
    bool PopFront();
	//QString m_fullpath;
};

class CXmlOutput : public COutput
{
public:
	CXmlOutput(QMap< QString, QString > a);
    bool PopFront();
	QString m_fullpath;
};

/*
class CHttpDaemonOutput : public COutput
{
	QString PopFront();
};
*/

#endif // COUTPUT_H
