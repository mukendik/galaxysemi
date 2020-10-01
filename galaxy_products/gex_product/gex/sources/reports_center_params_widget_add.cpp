#include <stdio.h>
#include <stdlib.h>
#include <QVBoxLayout>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDomDocument>
#include "browser_dialog.h"
#include <gqtl_log.h>
#include "reports_center_params_widget.h"
#include "reports_center_multifields.h"
#include "qttreepropertybrowser.h"
#include "script_wizard.h"
#include "settings_dialog.h"

extern GexMainwindow *	pGexMainWindow;

bool CReportsCenterParamsWidget::AddDataSet(QMap<QString, QString> atts)
{
	if ( (atts["ID"]=="") || (atts["stage"]=="") )
		return false;

	if (m_DatasetIDToDatasetMap.find(atts["ID"])!=m_DatasetIDToDatasetMap.end())
	{
		GSLOG(SYSLOG_SEV_WARNING, QString("error : DataSet ID '%1' already inserted !").arg( atts["ID"]).toLatin1().constData() );
		return false;
	}

	GSLOG(SYSLOG_SEV_DEBUG, QString(" adding the dataset ID '%1'").arg( atts["ID"]).toLatin1().constData());
	QWidget *w=new QWidget(this); //w=new QWidget(&m_tabwidget);
	QVBoxLayout* l=new QVBoxLayout(w);
	w->setLayout(l);
	w->setSizePolicy(QSizePolicy::Minimum, //QSizePolicy::Fixed,
					 QSizePolicy::Fixed		//QSizePolicy::Minimum
					 );

	if (atts["consolidated"]=="")
		atts.insert("consolidated","true");

	CReportsCenterDataset* ds=new CReportsCenterDataset(w, atts);
	ds->setParent(this);

	m_tabwidget.insertTab(0, w, atts["label"]);
	m_tabwidget.setCurrentIndex(0);


	m_DatasetIDToDatasetMap.insert(atts["ID"], ds);
	//w->resize(w->sizeHint());
	//m_tabwidget.resize(m_tabwidget.sizeHint());

	return true;
}

bool CReportsCenterParamsWidget::AddMultiFields(QMap<QString, QString> atts, QString datasetID,
												QVector< QMap<QString, QString> > children)
{
	if (atts["role"].isEmpty())
	{
		GSLOG(SYSLOG_SEV_NOTICE, " no 'role' found for this multifield ! Ignoring.");
		return false;
	}

	if (m_DatasetIDToDatasetMap.find(datasetID)==m_DatasetIDToDatasetMap.end())
	{
		GSLOG(SYSLOG_SEV_NOTICE, QString(" no dataset '%1' found ! Ignoring.").arg( datasetID).toLatin1().constData());
		return false;
	}

    GSLOG(SYSLOG_SEV_DEBUG,
          QString(" adding a '%1' multifield with %2 children.").
          arg(atts["role"]).arg(children.size()).toLatin1().constData());
	m_DatasetIDToDatasetMap.value(datasetID)->AddMultiFields(atts, children);

	//m_tabwidget.resize(m_tabwidget.sizeHint());

	resize(sizeHint());

	return false;
}

bool CReportsCenterParamsWidget::AddOptionToPropBrowser( QMap< QString, QString> atts)
{
	if (!m_pb)
		return false;

	CGexPublicProperty cgppProperty(atts);
	int nPropertyID =  m_pb->AddProperty(cgppProperty);
	if (nPropertyID <0)
	{
        GSLOG(SYSLOG_SEV_DEBUG,
              QString(" failed to add a Property to PB for this option !").
              arg(nPropertyID).toLatin1().constData());
		return false;
	}

	m_options_properties.insert(nPropertyID, atts);

        if (atts.find("defaultValueExpression")!=atts.end() )
            GSLOG(SYSLOG_SEV_NOTICE, "It seems that ReportCenter .grxml file has not been well updated : 'defaultValueExpression' not used anymore" );

	return true;
}
