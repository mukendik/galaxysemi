#ifndef REPORTS_CENTER_PARAMS_WIDGET_H
#define REPORTS_CENTER_PARAMS_WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QMap>
#include "reports_center_multifields.h"
#include "reports_center_params_dataset.h"
#include "libgexpb.h"

class CReportsCenterParamsWidget : public QWidget
{
	Q_OBJECT

	static int n_instances;

	QMap<QString, CReportsCenterDataset*> m_DatasetIDToDatasetMap;

	QTabWidget m_tabwidget;
	QVBoxLayout m_layout;
	QPushButton m_ok_pb;

	//
	//QString m_grt_filename;
	QMap<QString, QString> m_atts;
	QString m_grxml_filename;	

	// generate the grt
	QString GenerateGRT(QString outputgrtfilename);

	//
	QString GenerateCSL(QString outputformat); // as in csl !
	// generate csl DataSet(s)
	// type is 'm' for merge, 'c' for compare
	QString GenerateProcessDataCSL(FILE* f, QString outputformat, char type );
	//QString GenerateCompareCSL(QString output);

	// Open the FILE ASSITANT and write first part of the csl untill Data Section
	// f will point to the opened FILE, NULL if failed
	// returns "ok" or "error..."
	QString WriteBeginningOfCSL(FILE* &f, QString startup_page="home");

public:
	// parent, grxml_filename, grt
	CReportsCenterParamsWidget(QWidget *p, QString grxml_filename, QMap<QString,QString> atts);
	~CReportsCenterParamsWidget();

	//	as defined by xml attribute EnableIf
	bool IsEnabled();
	// Add a dataset to this params widget
	bool AddDataSet(QMap<QString, QString> atts);
	// Add a multifield
	// The dataset with this ID must have been created with  AddDataSet(...)
	// This multifield MUST have at least a 'role' attribute
	bool AddMultiFields(QMap<QString, QString> atts, QString datasetID,
						QVector< QMap<QString, QString> > children);
	// Add an option in the property browser
	bool AddOption( QMap< QString, QString> atts);
	// Add an option to the GexPB
	bool AddOptionToPropBrowser( QMap< QString, QString> atts);

	//
	bool HasAtLeastOneMandatoryFieldUnknown();
	//	check if any fields are mandatory and not yet selected and then disable the 'run' button
	bool CheckForMandatoryValues()
	{
		//GSLOG(SYSLOG_SEV_DEBUG, (char*)"");
		if (HasAtLeastOneMandatoryFieldUnknown())	m_ok_pb.setEnabled(false);
		else m_ok_pb.setEnabled(true);
		return true;
	}

	// Gex PB
	CGexPB* m_pb;
	QMap<int, QMap< QString, QString> > m_options_properties;

	//
	static int GetNumberOfInstances() { return n_instances; }
public slots:
	// send when user click RunReport
	bool OnRunReport();

};

#endif // REPORTS_CENTER_PARAMS_WIDGET_H
