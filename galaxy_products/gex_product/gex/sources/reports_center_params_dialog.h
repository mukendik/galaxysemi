#ifndef REPORTS_CENTER_PARAMS_DIALOG_H
#define REPORTS_CENTER_PARAMS_DIALOG_H

#include <QVector>
#include "ui_reports_center_params_dialog.h"

/* A filter struct contains the 3 widgets for a filter :
	- the field combobox, ('Lot Id',...)
	- the operator label (=, !=,...),
	- the field value combobox ('*',....)
	- and the small pick button
	When assigned, then the filter struct is linked to an SGRXMLParam. Dont modify.
	When multiselect, then the value can be multiple.
*/
struct SFilter
{
	SFilter(QComboBox* f, QLabel *l, QComboBox* fv, QToolButton* tb)
	{ m_filter_cb=f; m_filtervalue_cb=fv; m_pickbutton=tb; m_assigned=false; m_multiselect=true; m_operator=l; m_GRXMLparam=0;
	}
	SFilter(const SFilter& f)
	{ m_filter_cb=f.m_filter_cb; m_filtervalue_cb=f.m_filtervalue_cb; m_pickbutton=f.m_pickbutton; m_operator=f.m_operator; m_GRXMLparam=NULL;
	}
	SFilter()
	{ m_filter_cb=NULL; m_filtervalue_cb=NULL; m_pickbutton=NULL; m_assigned=false; m_GRXMLparam=NULL;
	}
	void Hide() { if (m_filter_cb) m_filter_cb->hide(); if (m_operator) m_operator->hide(); if (m_filtervalue_cb) m_filtervalue_cb->hide(); if (m_pickbutton) m_pickbutton->hide(); }
	void Show() { if (m_filter_cb) m_filter_cb->show(); if (m_operator) m_operator->show(); if (m_filtervalue_cb) m_filtervalue_cb->show(); if (m_pickbutton) m_pickbutton->show(); }
	bool isVisible() { if (m_filter_cb->isVisible()) return true; return false; }
	QComboBox* m_filter_cb;
	QComboBox* m_filtervalue_cb;
	QToolButton* m_pickbutton;
	QLabel* m_operator;
	bool m_assigned;	// if true, this filter is assigned to a particular variable, read only or not
	bool m_multiselect;
	struct SGRXMLParam* m_GRXMLparam;
};

/*
	The GRXML param struct contains info regarding a param found in the GRXML.
	It can be linked to a Filter, but if not, m_SFilter will be NULL.
	These structs are built at init. Dont modify it.
*/
struct SGRXMLParam
{
	QString m_name, m_data_type, m_filters; //m_defaultValueExpr,
	bool m_visible, m_mandatory, m_readonly, m_multivalue;
	SFilter* m_SFilter;	// Will be NULL if it is not a filter param
	QMap<QString, QString> m_attributes;	// operator, name, data_type,... strings.
};

class ReportsCenterParamsDialog : public QDialog, public Ui::reports_center_params_dialog
{
	Q_OBJECT

public:
	ReportsCenterParamsDialog(QWidget* parent=0, bool modal=FALSE, Qt::WFlags fl=0 );
	~ReportsCenterParamsDialog();
	//
	void HideAllFilters();	// Hide all filters. Would be easier to hide the m_GroupBoxFilters...
	void HideAll();			// Hide all widgets and groups.
	void ResetAll();		// clear and refill all comboboxes (useful after a bin type change)
	//void ResetAllFilters(QString datatype);		// clear comboboxes and reinsert fields
	bool InsertFieldsIntoComboBox(QComboBox *cb,
								const QString &testingstage, // "Wafer Sort",...
								QString BinType="N", 	// BinType can be H, S or N (or even *)
								QString Custom="*",	// Custom can be Y, N or *
								QString TimeType="*"  // TimeType can be Y, N or *
								  );
	// check if we have to enabled/disabled the ok button because of mandatory params
	bool	CheckOKButtonEnabling();
	// current selected timeline checkbox (Year, Month,...)
	QString GetCurrentTimeline();

	SFilter* FindFreeFilter();	// find a free (not assigned) filter
	QString m_sTestingStage;	// "WaferSort","AtoZ"...	REMOVE ME ?
	QString m_sTitle;			// "Yield by time"...

	QVector<SGRXMLParam> m_params;	// params found in GRXML
	QString m_GRXMLsource;
	static int GetNumberOfInstances() { return s_num_instances; }
private:

	QDate	m_From;
	QDate	m_To;
	QTime	m_FromTime;
	QTime	m_ToTime;
	void	UpdateFromToFields(bool);
	QVector<SFilter> m_filters;
	// PickFilter
	bool	PickFilter(QComboBox *field_cb, QComboBox *value_cb, bool multiselect);
	// Launchs the select dialog widget. Return false on cancel. Fill the pFilter->QueryValue. ReCode Me.
	bool	PickFilterFromLiveList(class GexDatabaseFilter *pFilter, bool bMultiselect); // retrun false on Cancel
	// Launchs the select bin dialog. Returns string containing the selected bin(s).
	QString	PickBinList(bool bMultiSelection, QString strDataType, QComboBox *binno_cb, bool bSoftBin=false);
	// Launchs the select bin dialog. Returns string containing the selected bins. Use an ugly GexDatabaseFilter.
	//QString	SqlPickBinningList(bool bMultiSelection, QString strDataType, QComboBox *binno_cb, bool bSoftBin=false);
	// return string list of bin
	QStringList	QueryGetBinningList(QString fieldToQuery, QVector<SFilter*>, bool bSoftBin);

	// generate a grt file named 'outputFile' in userfolder/galaxysemi following the current GUI choices
	// will return a string starting with 'error' on error.
	// else return the full path to the generated grt.
	QString	GenerateGRTfile(QString outputFile);

	static int s_num_instances;

public slots:
	// Calendar related slots
	void	OnTimePeriodChanged(QString s);
	void	OnFromDateCalendar(void);
	void	OnToDateCalendar(void);	
	void	OnPickFilter();	// On clicked a filter small tool button
	void	OnFilterChange(const QString&);
	bool	OnPickBinNo(QComboBox* cb=NULL, bool multiselect=false);	// fill the specified combobox with selected bin no
	bool	OnHardBinTypeChanged(bool b);
	bool	OnOK();
	bool	OnSave();
	void	OnSerieChanged(QString s);

	//void	OnTimePeriod(void);
};

#endif // REPORTS_CENTER_PARAMS_DIALOG_H
