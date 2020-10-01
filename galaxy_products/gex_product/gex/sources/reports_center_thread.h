#ifndef REPORTS_CENTER_THREAD_H
#define REPORTS_CENTER_THREAD_H

#include <QThread>
#include <QMap>

class CReportsCenterThread : public QThread
{
	Q_OBJECT

public:

	bool bStopRequested;

	CReportsCenterThread(class ReportsCenterWidget* p);

	void run();


	QMap< QString, class ReportsCenterItem* > m_GRXMLtoLoad;

	//private slots:
	//void finished() { GSLOG(7, " "); }
	//void terminated() { GSLOG(7, " "); }

	ReportsCenterWidget* m_rcw;

	// scan usual folders (galaxy_reports,...)
	//QString ScanUsualFolders();

	// Load GRXML version 0.5 and upper
	//bool LoadGRXML_v1(QString filepath, ReportsCenterItem *parentitem);


};

#endif // REPORTS_CENTER_THREAD_H
