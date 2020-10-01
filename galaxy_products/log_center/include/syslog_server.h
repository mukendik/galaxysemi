#ifndef SYSLOG_SERVER_H
#define SYSLOG_SERVER_H

#include <QtCore/QCoreApplication>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QHostAddress>
#include <QDateTime>
#include <QSocketNotifier>

struct SMessage
{
    int m_sev;
    int m_facility;
    QDateTime m_date;
    //QTime m_time;
    QMap< QString, QString> m_atts;
};

class SyslogServer : public QTcpServer
         //, public QThread
{
     Q_OBJECT

    //QUdpSocket mUdpSocket;

 public:
     SyslogServer(quint16 tcp_port, QObject* parent = 0);
         //: QTcpServer(parent),
        //COutput(),
        //disabled(false)
    ~SyslogServer();

    //void incomingConnection(int socket);

     void pause()
     {
         qDebug("pause");
         disabled = true;
     }

     void resume()
     {
         qDebug("resume");
         disabled = false;
     }

 private slots:
     void OnNewConnection();
     void readClient(); // called when a client asks something ('GET' or not)
     void discardClient();
     void clientConnected();
     void OnConnectionClosed();
     void aboutToClose();
     void bytesWritten(qint64);
     void error(QAbstractSocket::SocketError);
     void SetMaxLogLevel(int);

 signals:
     void sNewMessage(SMessage &m);

 private:
     bool disabled;
     int mMaxLogLevel;
};

#endif // SYSLOG_SERVER_H
