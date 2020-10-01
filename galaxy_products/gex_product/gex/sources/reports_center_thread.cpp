#include <gqtl_log.h>
#include "reports_center_widget.h"
#include "reports_center_thread.h"

CReportsCenterThread::CReportsCenterThread(ReportsCenterWidget* p) : QThread()
{
	m_rcw=p;
//    GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg( p));
	bStopRequested=false;
}


void CReportsCenterThread::run()
{
    GSLOG(SYSLOG_SEV_NOTICE, " ");
	while(!bStopRequested)
	{
		if (m_GRXMLtoLoad.size()==0)
		{
			sleep(1);
			continue;
		}

		QMap< QString, ReportsCenterItem*>::iterator it=m_GRXMLtoLoad.begin();
		QString grxml=it.key();
		GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("loading %1...").arg( grxml).toLatin1().constData());
		//ReportsCenterItem* item=it.value();
		/*
		if (!item)
			GSLOG(SYSLOG_SEV_WARNING, "error : the parent item is NULL !");
		else
			if (!m_rcw->LoadGRXML_v1(grxml, item))
				GSLOG(SYSLOG_SEV_NOTICE, QString("error while loading %1").arg( grxml).toLatin1().constData() );
		*/
		m_GRXMLtoLoad.remove(grxml);

	}

	return;
}
