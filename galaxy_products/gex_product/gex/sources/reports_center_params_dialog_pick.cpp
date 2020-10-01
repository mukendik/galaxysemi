#include "reports_center_widget.h"
#include "reports_center_params_dialog.h"
#include "calendar_dialog.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "pickfilter_dialog.h"
#include <gqtl_log.h>


extern bool				SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);
extern GexMainwindow*	pGexMainWindow;


bool	ReportsCenterParamsDialog::PickFilter(QComboBox *field_cb, QComboBox *value_cb, bool multiselect)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString("ReportsCenterParamsDialog::PickFilter on %1...").arg( field_cb->currentText()).toLatin1().constData());
	if (field_cb->currentText()=="")
		return false;

	if ( (field_cb->currentText()==gexLabelFilterChoices[GEX_QUERY_FILTER_NONE])
		|| (field_cb->currentText()==SELECT_SERIE_INVIT) )
	{
		GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("\tfield combobox anormal (%1).").arg( field_cb->currentText()).toLatin1().constData() );
		return false;
	}

	QComboBox* cb=((ReportsCenterWidget*)this->parent())->GetDatabaseComboBox();
	QString strDatabaseLogicalName = cb->currentText();
	if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
		strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();

	GexDatabaseEntry	*pDatabaseEntry =
			pGexMainWindow->pDatabaseCenter->FindDatabaseEntry(strDatabaseLogicalName );
	if (!pDatabaseEntry)
	{
		GSLOG(SYSLOG_SEV_ERROR, "\tcant find DB entry !");
		return false;
	}
	if(!pDatabaseEntry->bExternal)
	{
		GSLOG(SYSLOG_SEV_ERROR, QString("\t%1 is not an external DB !").arg( cb->currentText()).toLatin1().constData() );
		return false;
	}

	GexDbPlugin_Filter	clPluginFilter;
	clPluginFilter.strQueryField = field_cb->currentText();
	clPluginFilter.strDataTypeQuery=this->m_sTestingStage;
	clPluginFilter.strlQueryFilters.clear();
	clPluginFilter.calendarFrom = this->m_FromDateEdit->date();
	clPluginFilter.calendarTo = this->m_ToDateEdit->date();
	clPluginFilter.iTimePeriod = this->m_comboBoxTimePeriod->currentIndex();
	clPluginFilter.bConsolidatedData = true;

	// add the serie filter ?
	if ( (m_serie_comboBox!=field_cb) || (value_cb==m_GSF_vol_comboBox) || (value_cb==m_GSF_yield_comboBox) ) //&& (field_cb!=m_GSF_yield_comboBox) && (field_cb!=m_GSF_vol_comboBox) )
	{
		if ( (m_serie_comboBox->currentText()!="") && (m_serieFilter_comboBox->currentText()!="")
			&& (m_serieFilter_comboBox->currentText()!="*") )
		 clPluginFilter.strlQueryFilters.append( m_serie_comboBox->currentText() + "=" + m_serieFilter_comboBox->currentText() );
	}

	for (int i=0; i<m_filters.size(); i++)
	{
		// if (!m_filters[i].isVisible())	// when a widget is in another tab, it is considered as not visible.
		if (!m_filters[i].m_assigned)
			continue;
		if (m_filters[i].m_filter_cb==field_cb)
			continue;
		if (m_filters[i].m_filter_cb->currentText()=="")
			continue;
		if (m_filters[i].m_filter_cb->currentText()==SELECT_FILTER_INVIT)
			continue;
		if (m_filters[i].m_filtervalue_cb->currentText()=="*")
			continue;
		if (m_filters[i].m_operator->text()!="=")
			continue;
		clPluginFilter.strlQueryFilters.append(
				m_filters[i].m_filter_cb->currentText() +"="+m_filters[i].m_filtervalue_cb->currentText() );
	}

	QStringList			cMatchingFilters;	// output

	if (!pDatabaseEntry->m_pExternalDatabase->QueryField(clPluginFilter, cMatchingFilters, m_hardbin_radioButton?!m_hardbin_radioButton->isChecked():false))
	{	GSLOG(SYSLOG_SEV_ERROR, "\tDB QueryField failed !");
		return false;
	}

	if (cMatchingFilters.size()==0)
		return false;

	PickFilterDialog dPickFilter;
	dPickFilter.fillList(cMatchingFilters, multiselect);	// with or without star ?
	dPickFilter.setMultipleSelection(multiselect);

	// Prompt dialog box, let user pick Filter string from the list
	if(dPickFilter.exec() != QDialog::Accepted)
	{
		return false;	// User 'Abort'
	}

	// Save the list selected into the edit field...unless it is already in!
	QString strSelection = dPickFilter.filterList();
	for(int i=0; i<value_cb->count(); i++)
	{
		if (strSelection == value_cb->text(i))
		{
			// Selection already in combo...simply select it.
			SetCurrentComboItem(value_cb, strSelection);
			return true;
		}
	}

	value_cb->insertItem(strSelection);
	SetCurrentComboItem(value_cb, strSelection);

	// Reset HTML sections to create flag: ALL pages to create.
	OnFilterChange("");

	return true;
}

///////////////////////////////////////////////////////////
// Pick Filter string from list, update GUI accordingly.
// Under this mode, filter IS contextual: and ONLY lists
// database values matching the other filter criteria
// As we have up-to n filters, we have this function fill
// a list box of the Query filter, but narrowing the fill
// checking all the other 7 filters match the lookup criteria.
///////////////////////////////////////////////////////////
bool	ReportsCenterParamsDialog::PickFilterFromLiveList(GexDatabaseFilter *pFilter, bool bMultiselect)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" for %1").arg( pFilter->pQueryFilter->currentText()).toLatin1().constData());

	QComboBox* cb=((ReportsCenterWidget*)this->parent())->GetDatabaseComboBox();
	// Get Database name we have to look into...get string '[Local] <Database name>'
	QString strDatabaseName = cb->currentText();
	// Skip the [Local]/[Server] info, and extract database name.
	if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
		strDatabaseName = strDatabaseName.section("]",1).trimmed();

	pFilter->strDatabaseLogicalName = strDatabaseName;
	pFilter->iTimePeriod = m_comboBoxTimePeriod->currentIndex();
	// In case of External SQL database, filter will also reduce search to a given data type (eg: wafer sort, final test, e-test..)
	pFilter->strDataTypeQuery = this->m_sTestingStage; //comboBoxDatabaseType->currentText();
	pFilter->bOfflineQuery = false;	// true if query to be executed over local cache database, not remote database!
	pFilter->calendarFrom = m_FromDateEdit->date();			// Filter: From date
	pFilter->calendarTo = m_ToDateEdit->date();				// Filter: To date
	//pFilter->calendarFrom_Time = FromTimeEdit->time();	// Filter: From time
	//pFilter->calendarTo_Time = ToTimeEdit->time();		// Filter: To time

	// Fill Filter list with relevant strings
	PickFilterDialog dPickFilter;
	dPickFilter.fillList(pFilter, bMultiselect);	// with or without star ?
	dPickFilter.setMultipleSelection(bMultiselect);

	// Prompt dialog box, let user pick Filter string from the list
	if(dPickFilter.exec() != 1)
		return false;	// User 'Abort'

	// Save the list selected into the edit field...unless it is already in!
	QString strSelection = dPickFilter.filterList();
	for(int i=0; i<pFilter->pQueryValue->count(); i++)
	{
		if (strSelection == pFilter->pQueryValue->text(i))
		{
			// Selection already in combo...simply select it.
			SetCurrentComboItem(pFilter->pQueryValue, strSelection);
			return true;
		}
	}

	pFilter->pQueryValue->insertItem(strSelection);
	SetCurrentComboItem(pFilter->pQueryValue, strSelection);

	// Reset HTML sections to create flag: ALL pages to create.
	OnFilterChange("");
	return true;
}

QString	ReportsCenterParamsDialog::PickBinList(bool bMultiSelection, QString strDataType, QComboBox *binno_cb, bool bSoftBin)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" for %1").arg( strDataType).toLatin1().constData() );

	SFilter* f=NULL;
	QVector<SFilter*> v;

	for (int i=0; i<m_filters.size(); i++)
	{
		if (!m_filters[i].isVisible())
			continue;
		if (m_filters[i].m_filtervalue_cb==binno_cb)
		{
			f=&m_filters[i];
			//filter_cb=m_filters[i].m_filter_cb;
			//filtervalue_cb=m_filters[i].m_filtervalue_cb;
			//multiselect=m_filters[i].m_multiselect;
			continue;
		}
		//
		v.push_back(&m_filters[i]);
	}

	QString fieldToQuery;
	if (f)
		fieldToQuery=f->m_filter_cb->currentText();
	else
		fieldToQuery=bSoftBin?"Sbin#":"Hbin#";

	QStringList cBinningList = QueryGetBinningList(fieldToQuery, v, bSoftBin);
	if (cBinningList.count() == 0)
	{
		GSLOG(SYSLOG_SEV_INFORMATIONAL, " QueryGetBinningList has returned no result. ");
		return "";
	}
	// Fill Filter list with relevant strings
	PickBinSingleDialog dPickFilter;
	dPickFilter.fillList(cBinningList);
	dPickFilter.setMultipleSelection(bMultiSelection);
	// Prompt dialog box, let user pick Filter string from the list
	if(dPickFilter.exec() != 1)
		return "";	// User 'Abort'

	return dPickFilter.getBinsList(); //.replace('|',',');

	//return "";
}
