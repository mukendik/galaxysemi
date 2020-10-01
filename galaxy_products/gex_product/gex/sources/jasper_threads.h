#ifndef JASPER_THREADS_H
#define JASPER_THREADS_H

#include <QStringList>
#include <QtWidgets/QWidget>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QThread>
#include <QTreeView>
#include <QLibrary>
#include <QMessageBox>

#include "reports_center_item.h"

class JasperRunReportThread : public QThread
{
	Q_OBJECT

	public:
		static bool LaunchRunReportThread();
		static bool RequestReport(QString uri, QString output, QVector< QPair <QString, QString> > params);
		static bool StopAndDeleteThread();
		static JasperRunReportThread* s_singleton;

	private:
		static bool s_stop_request;

		JasperRunReportThread()
		{
			qDebug("JasperRunReportThread::JasperRunReportThread");
			s_stop_request=false;
		}
		~JasperRunReportThread() { qDebug("JasperRunReportThread::~JasperRunReportThread"); }

		void run();

		struct SJob
		{
			QString m_report_uri;
			QString m_output;
			QVector< QPair <QString, QString> > m_params;
		};

		static QMap<QString, SJob> s_reports_to_run; // URI, Output
		//private slots:
		void finished() { qDebug("JasperRunReportThread::finished"); }
		void terminated() { qDebug("JasperRunReportThread::terminated"); }
	signals:
		void SetStatusLabel(const QString s);
		void PopMessageBox(const QString s, QMessageBox::Icon severity);
		void ReportGenerated(const QString report_uri, const QString output);
	public slots:
		bool RequestReportSlot(); // QString uri, QString output, QString params
};

class JasperListDirThread : public QThread
{
	Q_OBJECT

	public:
		static bool LaunchListDirThread(QString, ReportsCenterItem*);
		static bool StopAndDeleteListDirThread();
		static JasperListDirThread* s_singleton;

	private:
		JasperListDirThread()
		{	qDebug("JasperListDirThread::JasperListDirThread");
			//qDebug("JasperListDirThread::JasperListDirThread %s", m_dir.toLatin1().data()); m_dir=dir; m_rootItem=r;
		}
		~JasperListDirThread() { qDebug("JasperListDirThread::~JasperListDirThread"); }

		void run();

		static QMap<QString, ReportsCenterItem*> s_folders_to_scan;
		static bool s_stop_request;

	signals:
		void SetStatusLabel(const QString s);
		void UpdateGUI();
		void PopMessageBox(const QString s, int severity); //
};

#endif // JASPER_THREADS_H
