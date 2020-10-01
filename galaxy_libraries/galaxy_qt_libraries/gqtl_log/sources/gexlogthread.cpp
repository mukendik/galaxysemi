#include <QFile>
#include <QMutex>
#include <stdexcept>
#include <stdlib.h>	// atexit

#include "gexlogthread.h"
#include "httpdaemon.h"


HttpDaemon* CGexLogThread::s_httpdaemon=0;

extern CGexLogThread*	s_pThread;
extern QMutex*			s_pMutex;

bool CGexLogThread::m_exit_requested=false;

QList<COutput*> COutput::s_ListCOutput;

//extern QList<SLog> s_logs;

CGexLogThread::~CGexLogThread()
{
  #ifdef QT_DEBUG
    qDebug("GexLogThread::~GexLogThread()");
  #endif
}

void CGexLogThread::AtExit()
{
	m_exit_requested=true;
	if (s_pThread)
		s_pThread->exit();
	#ifdef QT_DEBUG
	 qDebug("GexLogThread::atexit");
	#endif

	/*
	GexLogThread::s_httpdaemon->pause();
	GexLogThread::s_httpdaemon->close();
	delete GexLogThread::s_httpdaemon;
	GexLogThread::s_httpdaemon=0;
	*/
}

void CGexLogThread::newMessage(const SMessage &m)
{
	#ifdef QT_DEBUG
	 qDebug("GexLogThread::newMessage: %d", QThread::currentThread()==this);
	#endif

	COutput* o=0;
	foreach(o, COutput::s_ListCOutput)
	{
		o->m_buffer.push_back(m);
		o->PopFront();
		msleep(200);
	}
}

void CGexLogThread::run()
{
	#ifdef QT_DEBUG
	 qDebug("GexLogThread::run : %d", QThread::currentThread()==this );
	 qDebug("GexLogThread::run: QThread::idealThreadCount=%d",  QThread::idealThreadCount() );
	 char* gll=getenv("GEX_LOGLEVEL");
	 if (gll)
		qDebug("GexLogThread::run: GEX_LOGLEVEL=%s", gll);
	#endif

	int r=atexit(CGexLogThread::AtExit);
  #ifdef QT_DEBUG
    qDebug("GexLogThread::run: atexit : %d", r);
  #else
   Q_UNUSED(r)
  #endif

	QMap<QString, QString> atts;

	/*
	if (!s_httpdaemon)
		s_httpdaemon = new HttpDaemon(atts, 8080, NULL);
	if (!s_httpdaemon->isListening())
	{
		#ifdef QT_DEBUG
		 qDebug("GexLogThread::run: Failed to bind to port %d", s_httpdaemon->serverPort() );
		#endif
		delete s_httpdaemon;
		s_httpdaemon=0;
		//return;
	}
	*/

	COutput* o;
	foreach(o, COutput::s_ListCOutput)
		QObject::connect(o, SIGNAL(newMessageSignal(SMessage)), this, SLOT(newMessage(SMessage)));

	//s_httpdaemon->pause();
	//s_httpdaemon->resume();

	//r=exec();
	#ifdef QT_DEBUG
	 //qDebug("GexLogThread::run: exec returned %d ", r);
	#endif

	while (!m_exit_requested)	//this->terminate())
	{
    // using thread_priority instead does not work on windows :
    // the thread is using full core even with the lowest priority (idle)
    // Let's sleep even a little for the messages to be written ASAP
    msleep(10);
		COutput *o=NULL;
		foreach(o, COutput::s_ListCOutput)
		{
			s_pMutex->lock();
			o->PopFront();
			s_pMutex->unlock();
		}
		/*
		for (int i=0; i<s_logs.size(); i++)
		{
			if (s_logs.at(i).m_atts["type"]=="httpdaemon")
			{
				if (!m_httpdaemon)		//(!s_logs.at(i).m_output)
				{
					//s_logs[i].m_output=new HttpDaemon(8080, this);
					//HttpDaemon* hd=new HttpDaemon(8080, this);
					m_httpdaemon=new HttpDaemon(8080, NULL);
					if (! m_httpdaemon->isListening())
					{
						qDebug(" Failed to bind HttpDaemon to port %d", m_httpdaemon->serverPort() );
					}

					if (! ((HttpDaemon*)l.m_output)->isListening())
					{
						qDebug(" Failed to bind HttpDaemon to port %d", ((HttpDaemon*)l.m_output)->serverPort() );
						delete l.m_output;
						goto nextnode;
					}
				}
			}
		}
		*/
	}

	#ifdef QT_DEBUG
		qDebug("GexLogThread::run: ok");
	#endif
	//delete m_httpdaemon;
}
