#include "report_build.h" // CGexReport
#include "reports_center_widget.h"

#include "jasper_threads.h"
#include "jasper_params_widget.h"
#include "ui_reports_center_widget.h"
#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include "libjasper.h"
#include "browser_dialog.h"


extern void	WriteDebugMessageFile(const QString & strMessage);
extern CGexReport	*gexReport;
extern GexMainwindow *	pGexMainWindow;
extern QString			strUserFolder;

// static
JasperRunReportThread* JasperRunReportThread::s_singleton=NULL;
QMap<QString, JasperRunReportThread::SJob> JasperRunReportThread::s_reports_to_run;
bool JasperRunReportThread::s_stop_request=false;

// static
bool JasperRunReportThread::LaunchRunReportThread()
{
	GSLOG(SYSLOG_SEV_DEBUG, " ");

    if (!s_singleton)
        s_singleton=new JasperRunReportThread();

	s_stop_request=false;

    if (!s_singleton->isRunning())
    {
        s_singleton->start();
    }
    return true;
}

bool JasperRunReportThread::RequestReportSlot()
{
	if (!QObject::sender())
	{
		qDebug("JasperRunReportThread::RequestReportSlot: unable to retrieve sender.");
		return false;
	}
	QPushButton* pb=(QPushButton*)QObject::sender();

	ParamsWidget* pw=(ParamsWidget*)pb->parent();
	qDebug("JasperRunReportThread::RequestReportSlot %s", pw->m_report_uri.toLatin1().data());

	if (s_reports_to_run.find(pw->m_report_uri)!=s_reports_to_run.end())
	{
		qDebug("JasperRunReportThread::RequestReportSlot: already in queue...Please wait.");
		return false;
	}

	QVector< QPair <QString, QString> > params;
	pw->GetCurrentParams(params);

	SJob j;
	j.m_output=strUserFolder+QDir::separator()+REPORTS_FOLDER+QDir::separator() + pw->m_report_uri.section('/',-1);

	QString outputf=pGexMainWindow->pReportsCenter->GetCurrentOutputFormat();
	if (outputf.startsWith("PDF"))
		j.m_output.append(".pdf");
	else 	if (outputf.startsWith("ODT"))
		j.m_output.append(".odt");
	else 	if (outputf.startsWith("RTF"))
		j.m_output.append(".rtf");
	else 	if (outputf.startsWith("DOC"))
		j.m_output.append(".docx");
	else 	if (outputf.startsWith("HTML"))
		j.m_output.append(".html");
	else 	if (outputf.startsWith("XLS"))
		j.m_output.append(".xls");
	else 	if (outputf.startsWith("CSV"))
		j.m_output.append(".csv");

	j.m_report_uri=pw->m_report_uri;
	j.m_params=params;
	pw->m_ok_pb->setEnabled(false);

	connect(this, SIGNAL(ReportGenerated(QString, QString)), pw, SLOT(ReportGenerated(QString, QString)));

	s_reports_to_run[j.m_report_uri]=j;

	/*
	if (JasperRunReportThread::RequestReport( pw->m_report_uri,
		strUserFolder+QDir::separator()+REPORTS_FOLDER+QDir::separator()+"o.pdf",
		params)) //for html: "o.html"
		pw->m_ok_pb->setEnabled(false);
	else
		qDebug("JasperRunReportThread::RequestReportSlot: ?");
	*/
	return true;
}

bool JasperRunReportThread::RequestReport(QString uri, QString output, QVector< QPair< QString, QString> > params)
{
	WriteDebugMessageFile("JasperRunReportThread::RequestReport: "+uri+" in "+output+QString(" with %1").arg(params.size())+" params");

	if (s_reports_to_run.find(uri)!=s_reports_to_run.end())
		return false;

	SJob j;
	j.m_output=output;
	j.m_report_uri=uri;
	j.m_params=params;
	s_reports_to_run[uri]=j;

	return true;
}

void JasperRunReportThread::run()
{
	qDebug("JasperRunReportThread::run");

	if (!ReportsCenterWidget::pclJasperLibrary)
	{
		WriteDebugMessageFile("JasperRunReportThread::run: error : JasperLib NULL !");
		return;
	}

	run_report_f rrf=(run_report_f)ReportsCenterWidget::pclJasperLibrary->resolve("jasper_run_report");
	if (!rrf)
	{
		WriteDebugMessageFile("JasperRunReportThread::run: cant resolve function !");
		return;
	}

	do
	{
		if (s_reports_to_run.size()==0)
		{
			this->msleep(1000);
			continue;
		}

		QMap <QString, SJob>::const_iterator	it=s_reports_to_run.begin();

		QString report_uri=it.key();
		SJob j=it.value();
		QString output=j.m_output;

		char params[MAX_NUM_RESOURCES][2][MAX_STRING_SIZE];
		for (int i=0; i<j.m_params.size(); i++)
		{
			snprintf(params[i][0], MAX_STRING_SIZE, j.m_params[i].first.toLatin1().data());
			snprintf(params[i][1], MAX_STRING_SIZE, j.m_params[i].second.toLatin1().data());
			//snprintf(params[i][0], MAX_STRING_SIZE, m_params[i].m_value_widget->cu);
		}

		WriteDebugMessageFile(QString("JasperRunReportThread::run: requesting %1 to %2 for those %3 params...")
							  .arg(report_uri).arg(output).arg(j.m_params.size()) );
		SetStatusLabel("Requesting "+report_uri+"...");

		if (!rrf(report_uri.toLatin1().data(), output.toLatin1().data(), (char***)params, j.m_params.size()))
		{
			WriteDebugMessageFile(QString("JasperRunReportThread::run: failed requesting %1 (%2)").arg(report_uri).arg(output) );
			SetStatusLabel("Failed to generate "+report_uri);
			s_reports_to_run.remove(it.key());
			emit ReportGenerated(report_uri, output); // just to reenable the ok button. To do : create a signal/slot : ReportGenerationFailed
			continue;
		}

		SetStatusLabel(report_uri+" generated !");
		WriteDebugMessageFile(QString("JasperRunReportThread::run: report %1 generated !").arg(report_uri));

		/*
		if (gexReport)
		{
			gexReport->strReportName=output; Wizard_OpenReport(); // Check me with Hervé
		}
		else
			WriteDebugMessageFile(QString("JasperRunReportThread::run: cant open report because gexReport NULL !"));
		*/

		emit ReportGenerated(report_uri, output);
		//pGexMainWindow->LoadHtmlPage(output);

		//pGexMainWindow->Wizard_OpenReportFile(output);

		s_reports_to_run.remove(it.key());

		this->msleep(1000);
	}
	while(!JasperRunReportThread::s_stop_request);

	WriteDebugMessageFile("JasperRunReportThread::run end.");

}

bool JasperRunReportThread::StopAndDeleteThread()
{
	qDebug("JasperRunReportThread::StopAndDeleteListDirThread");
	s_stop_request=true;
	if (!s_singleton)
	 return true;
	while (s_singleton->isRunning())
	 s_stop_request=true;
	delete s_singleton;
	s_singleton=NULL;
	WriteDebugMessageFile("JasperRunReportThread::StopAndDeleteListDirThread ok.");
	return true;
}
