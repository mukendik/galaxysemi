#include "reports_center_params_dialog.h"
#include "reports_center_widget.h"
#include "calendar_dialog.h"
#include "db_transactions.h"	// DatabaseEntry
#include "browser_dialog.h"	// for GexMainwindow
#include "gex_constants.h"	// for gexLabelFilterChoices, ProcessPartsItems,...
#include <gqtl_log.h>


extern const char *		szAppFullName;
extern GexMainwindow *	pGexMainWindow;
extern QString			strApplicationDir;
extern void				FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

int ReportsCenterParamsDialog::s_num_instances=0;

ReportsCenterParamsDialog::ReportsCenterParamsDialog(QWidget* parent, bool modal, Qt::WFlags fl) : QDialog(parent, fl)
{
	s_num_instances++;

	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ReportsCenterParamsDialog::ReportsCenterParamsDialog (total %1 instances)").arg( s_num_instances));

	//setWindowFlags( Qt::WindowCloseButtonHint); //  1|!Qt::WindowContextHelpButtonHint);

	setupUi(this);

	m_Save_pushButton->hide();

	m_label_split->hide(); m_split_comboBox->hide();

	//FromTimeEdit->hide(); ToTimeEdit->hide();

	setModal(modal);

	m_filters.push_back(SFilter(comboBoxFilter1, m_operator_1, FilterString1, PickFilter1));
	m_filters.push_back(SFilter(comboBoxFilter2, m_operator_2, FilterString2, PickFilter2));
	m_filters.push_back(SFilter(comboBoxFilter3, m_operator_3, FilterString3, PickFilter3));
	m_filters.push_back(SFilter(comboBoxFilter4, m_operator_4, FilterString4, PickFilter4));
	m_filters.push_back(SFilter(comboBoxFilter5, m_operator_5, FilterString5, PickFilter5));
	m_filters.push_back(SFilter(comboBoxFilter6, m_operator_6, FilterString6, PickFilter6));
	m_filters.push_back(SFilter(comboBoxFilter7, m_operator_7, FilterString7, PickFilter7));
	m_filters.push_back(SFilter(comboBoxFilter8, m_operator_8, FilterString8, PickFilter8));

	//this->GroupBoxFilters->hide();

	this->HideAllFilters();
	m_filters.first().Show();

	//pReportsCenter->ResetAll();

	this->setMaximumSize(this->sizeHint());

	QObject::connect(m_comboBoxTimePeriod,	SIGNAL(currentIndexChanged(QString)), this, SLOT(OnTimePeriodChanged(QString)) );
	QObject::connect(FromDateCalendar,		SIGNAL(clicked()),	this, SLOT(OnFromDateCalendar()) );
	QObject::connect(ToDateCalendar,		SIGNAL(clicked()),	this, SLOT(OnToDateCalendar()));
	QObject::connect(PickFilter1,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter2,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter3,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter4,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter5,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter6,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter7,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
	QObject::connect(PickFilter8,			SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );

	QObject::connect(m_GSF_yield_toolButton,	SIGNAL(clicked()), this, SLOT(OnPickFilter()) );
	QObject::connect(m_GSF_vol_toolButton,		SIGNAL(clicked()), this, SLOT(OnPickFilter()) );

	QObject::connect(comboBoxFilter1,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter2,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter3,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter4,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter5,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter6,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter7,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));
	QObject::connect(comboBoxFilter8,		SIGNAL(currentIndexChanged(QString)),	this, SLOT(OnFilterChange(QString)));

	QObject::connect(m_serieFilter_toolButton,		SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );

	QObject::connect(m_serie_comboBox,		SIGNAL(currentIndexChanged(QString)), this, SLOT(OnSerieChanged(QString)) );

	QObject::connect(m_bin_no_toolButton,	SIGNAL(clicked()),	this, SLOT(OnPickBinNo()) );
	QObject::connect(m_button_OK,			SIGNAL(clicked()),	this, SLOT(OnOK()) );
	QObject::connect(m_Save_pushButton,		SIGNAL(clicked()),	this, SLOT(OnOK()) );

	QObject::connect(m_hardbin_radioButton, SIGNAL(toggled(bool)), this, SLOT(OnHardBinTypeChanged(bool)));
}

ReportsCenterParamsDialog::~ReportsCenterParamsDialog()
{
	s_num_instances--;
	GSLOG( SYSLOG_SEV_DEBUG, QString(" (still %1 instances)").arg( s_num_instances));

	//delete m_ui;
}


bool ReportsCenterParamsDialog::InsertFieldsIntoComboBox(QComboBox *cb,
														 const QString &testingstage,
														 QString BinType, 	// BinType can be H, S or N (or even *)
														 QString Custom,	// Custom can be Y, N or *
														 QString TimeType  // TimeType can be Y, N or *
														 )
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( testingstage).toLatin1().constData());
	if (!cb || testingstage=="")
		return false;
	GexDatabaseEntry* dbe=((ReportsCenterWidget*)this->parent())->GetCurrentDatabaseEntry();
	if (!dbe)
	{
		GSLOG(SYSLOG_SEV_ERROR , " error : DatabaseEntry NULL !");
		//for (int i=0; i<m_filters.size(); i++)
		//	FillComboBox(cb,	gexLabelFilterChoices);	// add default fields ???
		return false;
	}

	//BinType="N";
	if (BinType=="") BinType="*";
	if (Custom=="") Custom="*";
	if (TimeType=="") TimeType="*";

	QStringList strlFilters;
	//dbe->m_pExternalDatabase->GetLabelFilterChoices( QString(testingstage), strlFilters);
	dbe->m_pExternalDatabase->GetRdbFieldsList(QString(testingstage), strlFilters, "Y", BinType,
											   Custom, TimeType, "*","N");

	if (strlFilters.empty())
	{
		GSLOG(SYSLOG_SEV_WARNING, " warning : no fields returned !");
		//FillComboBox(cb,	gexLabelFilterChoices);	// Should nt happen, insert default static fields ???
		//m_filters[0].m_filter_cb->setCurrentItem(GEX_QUERY_FILTER_PRODUCT);
	}
	else
	{
		cb->insertStringList(strlFilters);
	}
	return true;
}



void ReportsCenterParamsDialog::ResetAll()
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::ResetAll");

	GexDatabaseEntry* dbe=((ReportsCenterWidget*)this->parent())->GetCurrentDatabaseEntry();

	if (!dbe)
	{	GSLOG(SYSLOG_SEV_ERROR, " error : DatabaseEntry NULL !");
		return;
	}

	GSLOG(SYSLOG_SEV_WARNING, " code me ?");

	// Reset Binning list ? To Do !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//QString GexOneQueryWizardPage1::SqlPickBinningList(...)
	//pGexMainWindow->pDatabaseCenter->QueryGetBinningList(&cFilter, bSoftBin)

	// Filters
	//ResetAllFilters(this->m_sTestingStage);

}

SFilter* ReportsCenterParamsDialog::FindFreeFilter()
{
	for (int i=0; i<m_filters.size(); i++)
	{
		if (!m_filters[i].m_assigned)
			return &m_filters[i];
	}
	return 0;
}

/*
void ReportsCenterParamsDialog::ResetAllFilters(QString datatype)
{
	GSLOG(SYSLOG_SEV_DEBUG, " %s", datatype.toLatin1().data());
	for (int i=0; i<m_filters.size(); i++)
	{
		if (m_filters[i].m_assigned)
			continue;
		m_filters[i].m_filtervalue_cb->clear(); m_filters[i].m_filtervalue_cb->insertItem(0,"*");
		m_filters[i].m_filter_cb->clear();
		InsertFieldsIntoComboBox( m_filters[i].m_filter_cb, datatype, "*", "*", "*");
		m_filters[i].m_filter_cb->insertItem(0, gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]);
		m_filters[i].m_filter_cb->setCurrentIndex(0);
		m_filters[i].m_assigned=false;
	}
}
*/

void ReportsCenterParamsDialog::HideAllFilters()
{
	for (int i=0; i<m_filters.size(); i++)
		m_filters[i].Hide();
}

QStringList	ReportsCenterParamsDialog::QueryGetBinningList(QString fieldToQuery, QVector<SFilter*> filters, bool bSoftBin)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( fieldToQuery).toLatin1().constData());
	QStringList sl;

	if (fieldToQuery.isEmpty())
	{
		GSLOG(SYSLOG_SEV_ERROR, "\terror : field to query is NULL !");
		return sl;
	}
	// Get database object
	GexDatabaseEntry* pDatabaseEntry=((ReportsCenterWidget*)this->parent())->GetCurrentDatabaseEntry();
	if(pDatabaseEntry == NULL)
	{
		GSLOG(SYSLOG_SEV_ERROR, " error : cant get DB entry !");
		return sl;	// Return empty list
	}
	// Query only applies to External database
	if(!pDatabaseEntry->bExternal)
	{
		GSLOG(SYSLOG_SEV_ERROR, " error : DB not external !");
		return sl;	// Return empty list
	}
	// Fill external database query object
	GexDbPlugin_Filter clPluginFilter;

	clPluginFilter.strQueryField = fieldToQuery; //clPluginFilter.strQueryField = f->m_filter_cb->currentText();
	clPluginFilter.strlQueryFilters.clear();

	QString strField, strValue, strFilter;

	for (int i=0; i<filters.size(); i++)
	{
		strField = filters[i]->m_filter_cb->currentText();
		strValue = filters[i]->m_filtervalue_cb->currentText();
		if(!strField.isEmpty() && (strField != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]) && (strValue != "*"))
		{
			strFilter = strField;
			strFilter += "="; // not like ?
			strFilter += strValue;
			clPluginFilter.strlQueryFilters.append(strFilter);
		}
	}
	clPluginFilter.strDataTypeQuery = this->m_sTestingStage; //pFilter->strDataTypeQuery;
	clPluginFilter.calendarFrom = this->m_FromDateEdit->date();
	clPluginFilter.calendarTo = this->m_ToDateEdit->date();
	//clPluginFilter.calendarFrom_Time = this->FromTimeEdit->time();
	//clPluginFilter.calendarTo_Time = this->ToTimeEdit->time();

	clPluginFilter.bUseTimePeriod=true;
	clPluginFilter.iTimePeriod = m_comboBoxTimePeriod->currentIndex();
	clPluginFilter.bConsolidatedData = true;

	// Execute query on remote database
	pDatabaseEntry->m_pExternalDatabase->QueryBinlist(clPluginFilter,sl, bSoftBin, true, true);

	return sl;
}
