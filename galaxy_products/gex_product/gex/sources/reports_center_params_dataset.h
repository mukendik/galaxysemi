#ifndef REPORTS_CENTER_PARAMS_DATASET_H
#define REPORTS_CENTER_PARAMS_DATASET_H

#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QMap>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QRadioButton>
#include <QDomElement>
//#include "reports_center_params_widget.h"

class CReportsCenterDataset : public QObject
{
Q_OBJECT

	QWidget *m_widget;
	QGroupBox* m_timeperiodGB;
	QComboBox* m_timeperiodCB;
	QWidget* m_TimeFactorStepWidget;
	QLineEdit*	m_TimeFactorLE;
	QComboBox* m_TimeStepCB;

	QGroupBox* m_binningGB;
	QRadioButton* m_hardbinRB;
	QRadioButton* m_softbinRB;

	QWidget* m_dualcalendarW;
	QDateTimeEdit* m_FromDTE;
	QDateTimeEdit* m_ToDTE;
	QMap<QString, class CReportsCenterMultiFields*> m_MultiFieldsMap;
	static int n_instances;

public slots:
	bool SlotTimePeriodChanged(int i);
	bool SlotBinTypeChanged(bool);

public:
	// the parent is NOT the CReportsCenterParamsWidget, but just a widget with a layout
	CReportsCenterDataset(QWidget* w, QMap<QString, QString> atts);
	~CReportsCenterDataset();
	QMap<QString, QString> m_atts;
	// Add a multifields to this dataset.
	// It will fail if a MF with the same 'role' has already been inserted
	bool AddMultiFields(QMap<QString, QString> atts, QVector< QMap<QString, QString> > children);
	// open a Dialog for the user to pick one or several distinct(s) value(s) for the given field
	// return false if user click cancel
	bool PickDistinct(QComboBox *field_cb, QComboBox *value_cb, bool multiselect);
	// export to dom
	bool ExportToDom(QDomDocument&, QDomElement &de);
	// write csl DataSetting for this gorup
	// Will use the given group id in gexGroup('insert_query','DS', groupID); if not empty
	// else will simply write gexGroup('insert_query','DS')
	QString ExportToCsl(FILE* f, QString group_id);
	//
	bool HasAtLeastOneMandatoryFieldUnknown();
	//
	//CReportsCenterParamsWidget* GetParamsWidget() { return (CReportsCenterParamsWidget*)m_widget->parent(); }
	static int GetNumberOfInstances() { return n_instances; }
};

#endif // REPORTS_CENTER_PARAMS_DATASET_H
