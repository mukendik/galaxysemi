#ifndef GEXLOGTHREAD_H
#define GEXLOGTHREAD_H
#include <QCoreApplication>
#include <QThread>
#include <QMap>
#include <QDir>
#include <QDate>
#include <QUdpSocket>
#include <QTcpSocket>
#include "coutput.h"

//#include "httpdaemon.h"

extern bool ReplaceWithUsualVariables(QString &s, QMap<QString, QString>);
extern QFile s_gexloglogfile;

class CGexLogThread : public QThread
{
	Q_OBJECT

 public:
	CGexLogThread() //: m_exit_requested(false)
	{
		//QObject::connect(this, SIGNAL(newMessageSignal(SMessage)), this, SLOT(newMessage(SMessage)));
		//s_httpdaemon=0;
	};
  ~CGexLogThread();
	void run();

	//void exit() { qDebug("GexLogThread::exit"); };
	//void terminate() { qDebug("GexLogThread::terminate"); };
	void quit(){ qDebug("GexLogThread::quit"); };

	//void emitNewMessage(const SMessage &m) { emit newMessageSignal(m); };

	/*
	bool event(QEvent* e)
	{
		qDebug("GexLogThread::event %d %d", e->type(), (this==QThread::currentThread()) );
		e->setAccepted(true);
		COutput* o=0;
		foreach(o, COutput::s_ListCOutput)
		{
			o->PopFront();
		}
		msleep(500);
		return true;
	};	// from QObject
	*/

	static bool m_exit_requested;
	static void AtExit();

	static class HttpDaemon* s_httpdaemon;
signals:
	void newMessageSignal(const SMessage &m);
public slots:
	void newMessage(const SMessage &m);

};

#endif // GEXLOGTHREAD_H
