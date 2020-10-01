#include <QDomDocument>
#include "reports_center_widget.h"
#include "reports_center_params_dialog.h"
#include "calendar_dialog.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "pickfilter_dialog.h"
#include <gqtl_log.h>


extern const char *		szAppFullName;
extern GexMainwindow *	pGexMainWindow;
extern QString			strApplicationDir;
extern QString			strUserFolder;

// in classes.cpp
extern bool				SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);


///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'From' date
///////////////////////////////////////////////////////////
void	ReportsCenterParamsDialog::OnFromDateCalendar(void)
{
	GSLOG(SYSLOG_SEV_DEBUG, " ");

	// Create Calendar object
	CalendarDialog *pCalendar = new CalendarDialog();
	pCalendar->setDate(this->m_FromDateEdit->date()); //pCalendar->setDate(m_From);

	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;

	// Read date picked by the user
	m_From = pCalendar->getDate();

	// Reset HTML sections to create flag: ALL pages to create.
	OnFilterChange("");

	// Update GUI accordingly
	UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'To' date
///////////////////////////////////////////////////////////
void	ReportsCenterParamsDialog::OnToDateCalendar(void)
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::OnToDateCalendar");
	// Create Calendar object
	CalendarDialog *pCalendar = new CalendarDialog();
	pCalendar->setDate(this->m_ToDateEdit->date()); //pCalendar->setDate(m_To);
	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;
	// Read date picked by the user
	m_To = pCalendar->getDate();
	// Reset HTML sections to create flag: ALL pages to create.
	OnFilterChange("");
	// Update GUI accordingly
	UpdateFromToFields(false);
}


///////////////////////////////////////////////////////////
// Swap dates if needed, update fields.
///////////////////////////////////////////////////////////
void	ReportsCenterParamsDialog::UpdateFromToFields(bool bCheckForSwap)
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::UpdateFromToFields");
	QDate		Date;
	QTime		Time;
	QDateTime	clDateTime_From, clDateTime_To;

	// Check if need to swap dates!
	if((bCheckForSwap == true) && (m_From > m_To))
	{
		Date = m_To;
		m_To= m_From;
		m_From= Date;
	}

	// Check if need to swap times!
	if((bCheckForSwap == true) && (m_FromTime > m_ToTime))
	{
		Time = m_ToTime;
		m_ToTime = m_FromTime;
		m_FromTime = Time;
	}

	// Update 'From' fields + comment
	m_FromDateEdit->setDate(m_From);
	//FromTimeEdit->setTime(m_FromTime);
	clDateTime_From.setDate(m_From);
	//clDateTime_From.setTime(m_FromTime);
	//TextLabelFromDate->setText(clDateTime_From.toString(Qt::TextDate));

	// Update 'To' fields + comment
	m_ToDateEdit->setDate(m_To);
	//ToTimeEdit->setTime(m_ToTime);
	clDateTime_To.setDate(m_To);
	clDateTime_To.setTime(m_ToTime);
	//TextLabelToDate->setText(clDateTime_To.toString(Qt::TextDate));
}

void	ReportsCenterParamsDialog::OnSerieChanged(QString s)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString("ReportsCenterParamsDialog::OnSerieChanged %1").arg( s).toLatin1().constData());

	if (!QObject::sender())
	{
		GSLOG(SYSLOG_SEV_WARNING, "ReportsCenterParamsDialog::OnSerieChanged: unable to retrieve sender !");
		return;
	}

	m_GSF_vol_comboBox->clear(); m_GSF_vol_comboBox->insertItem(0,"*");
	m_GSF_yield_comboBox->clear(); m_GSF_yield_comboBox->insertItem(0,"*");

	QComboBox* cb=(QComboBox*)QObject::sender();
	if (cb==m_serie_comboBox)
	{
		GSLOG(SYSLOG_SEV_DEBUG, QString("serie changed to %1 !").arg( cb->currentText()).toLatin1().constData());
		m_serieFilter_comboBox->clear();
		m_serieFilter_comboBox->insertItem(0,"*");
		if (cb->currentText()==SELECT_SERIE_INVIT)
			return;
	}

	// lets clear all the filters to be sure there is no conflict between serie and filters
	for (int i=0; i<m_filters.size(); i++)
	{
		//if (m_filters[i].m_assigned)
		//	continue;

		m_filters[i].m_filtervalue_cb->clear();

		if (m_filters[i].m_multiselect)
		{
			m_filters[i].m_filtervalue_cb->insertItem(0,"*");
			m_filters[i].m_filtervalue_cb->setCurrentIndex(0);
		}
		else if (m_filters[i].m_GRXMLparam)
			if (m_filters[i].m_GRXMLparam->m_attributes["defaultValueExpression"]!="")
			{
				m_filters[i].m_filtervalue_cb->insertItem(0, m_filters[i].m_GRXMLparam->m_attributes["defaultValueExpression"]);
				m_filters[i].m_filtervalue_cb->setCurrentIndex(0);
			}
	}

}

void	ReportsCenterParamsDialog::OnTimePeriodChanged(QString s)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( s).toLatin1().constData());
	if (!s.startsWith("Pick dates from"))
	{
		TextLabelFrom->hide();
		m_FromDateEdit->hide();
		FromDateCalendar->hide();
		TextLabelTo->hide();
		m_ToDateEdit->hide();
		ToDateCalendar->hide();
	}
	else
	{
		TextLabelFrom->show();
		m_FromDateEdit->show();
		FromDateCalendar->show();
		TextLabelTo->show();
		m_ToDateEdit->show();
		ToDateCalendar->show();
	}
}

///////////////////////////////////////////////////////////
// User has changed a Query filter...
void	ReportsCenterParamsDialog::OnFilterChange(const QString &strString)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strString).toLatin1().constData());
	// Check if user combining ? or * with the '|' character: it's not allowed!
	if(((strString.indexOf("*") >= 0) || (strString.indexOf("?") >= 0)) &&
		(strString.indexOf("|") >= 0))
	{
        GS::Gex::Message::information(
            "", "You can't combine the '*' or '?' wildcar\n"
            "with the OR '|' character. Use either one grammar.");
	}

	if (!QObject::sender())
	{
		GSLOG(SYSLOG_SEV_ERROR, " unable to retrieve sender !");
		return;
	}

	QComboBox* cb=(QComboBox*)QObject::sender();

	for (int i=0; i<m_filters.size(); i++)
	{
		if (m_filters[i].m_filter_cb!=cb)
			continue;
		m_filters[i].m_filtervalue_cb->clear();
		if (m_filters[i].m_multiselect)
		{
			m_filters[i].m_filtervalue_cb->insertItem(0,"*");
			m_filters[i].m_filtervalue_cb->setCurrentIndex(0);
		}
		else if (m_filters[i].m_GRXMLparam)
			if (m_filters[i].m_GRXMLparam->m_attributes["defaultValueExpression"]!="")
			{
				m_filters[i].m_filtervalue_cb->insertItem(0, m_filters[i].m_GRXMLparam->m_attributes["defaultValueExpression"]);
				m_filters[i].m_filtervalue_cb->setCurrentIndex(0);
			}
	}

	// Reset HTML sections to create flag: ALL pages to create. ????????????????????????????????
	/*
	if(pGexMainWindow != NULL)
	{
		pGexMainWindow->iHtmlSectionsToSkip = 0;
		pGexMainWindow->m_bDatasetChanged	= true;
	}
	*/
}

void	ReportsCenterParamsDialog::OnPickFilter(void)
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::::OnPickFilter");

	if (!QObject::sender())
	{
		GSLOG(SYSLOG_SEV_ERROR, "ReportsCenterParamsDialog::OnPickFilter: unable to retrieve sender !");
		return;
	}
	QToolButton* tb=(QToolButton*)QObject::sender();

	if (tb==m_serieFilter_toolButton)
	{
		if ( (m_serie_comboBox->currentText().toLower()=="bin no")
			|| (m_serie_comboBox->currentText().toLower().contains("bin#", Qt::CaseInsensitive)) )
		{
			this->OnPickBinNo(m_serieFilter_comboBox, true );
			CheckOKButtonEnabling();
			return;
		}

		if (!PickFilter(m_serie_comboBox, m_serieFilter_comboBox, true))
		 GSLOG(SYSLOG_SEV_WARNING,"PickFilter on serie failed !");
		return;
	}

	if (tb==m_GSF_yield_toolButton)
	{
		if (!PickFilter(m_serie_comboBox, m_GSF_yield_comboBox, true))
			GSLOG(SYSLOG_SEV_WARNING,"PickFilter on yield GSF failed or cancelled !");
		return;
	}
	if (tb==m_GSF_vol_toolButton)
	{
		if (!PickFilter(m_serie_comboBox, m_GSF_vol_comboBox, true))
			GSLOG(SYSLOG_SEV_WARNING,"PickFilter on vol GSF failed or cancelled !");
		return;
	}

	SFilter* sfilter=NULL;

	QComboBox* filter_cb=NULL;
	QComboBox* filtervalue_cb=NULL;
	bool multiselect=true;
	for (int i=0; i<m_filters.size(); i++)
	{
		if (m_filters[i].m_pickbutton==tb)
		{
			sfilter=&m_filters[i];
			filter_cb=m_filters[i].m_filter_cb;
			filtervalue_cb=m_filters[i].m_filtervalue_cb;
			multiselect=m_filters[i].m_multiselect;
			break;
		}
	}

	if ( (filter_cb==NULL) || (filtervalue_cb==NULL) )
	{
		GSLOG(SYSLOG_SEV_ERROR, "\terror : unknown filter !");
		return;
	}

	if ( (filter_cb->currentText().toLower()=="bin no")
		|| (filter_cb->currentText().toLower().contains("bin#", Qt::CaseInsensitive)) )
	{
		this->OnPickBinNo(filtervalue_cb, sfilter->m_multiselect);
		CheckOKButtonEnabling();
		return;
	}

	// test
	if (!PickFilter( filter_cb, filtervalue_cb, multiselect))
	 GSLOG(SYSLOG_SEV_ERROR, "\tPickFilter failed or cancelled !");
	else
	{
		if (filtervalue_cb->currentText()!="*")
		// show next filter
		for (int i=0; i<m_filters.size(); i++)
		{
			if (m_filters[i].m_pickbutton==tb)
			{
				if (i!=m_filters.size()-1)
				{
					// this is the next filter
					// is it already shown ?
					if (m_filters[i+1].isVisible())
						break;
					m_filters[i+1].m_assigned=true;
					if (sfilter->m_multiselect)
					{	m_filters[i+1].m_filtervalue_cb->insertItem(0,"*");
						m_filters[i+1].m_multiselect=true;
					}
					m_filters[i+1].Show();
					// let s take the same kind of bin (H,S or N) than the previous one
					QString Bin="*";
					if (m_filters[i].m_GRXMLparam)
						Bin=m_filters[i].m_GRXMLparam->m_attributes["Binning"];
					//m_params[i].m_attributes
					if (m_filters[i+1].m_filter_cb->count()==0)
					 InsertFieldsIntoComboBox(m_filters[i+1].m_filter_cb, this->m_sTestingStage, Bin,"*","*");
					m_filters[i+1].m_filter_cb->setCurrentIndex(i+1);
				}
				// else ... (no more filter available)
			}
		} // for all filters
	}
	CheckOKButtonEnabling();
	return;

	/*
	if (filter_cb->currentText()=="bin P/F status")
	{
		if (m_hardbin_radioButton->isChecked())
		 dummyCB.insertItem(0, "Hbin P/F status");
		else
		 dummyCB.insertItem(0, "Sbin P/F status");
		dummyCB.setCurrentIndex(0);
		filter_cb=&dummyCB;
	}
	*/

}

bool ReportsCenterParamsDialog::CheckOKButtonEnabling()
{
	for (int i=0; i<m_params.size(); i++)
	{
		if (!m_params[i].m_mandatory)
			continue;
		if (m_params[i].m_SFilter==NULL)
			continue;
		GSLOG(SYSLOG_SEV_DEBUG,QString("ReportsCenterParamsDialog::CheckOKButtonEnabling for mandatory %1").arg(
				 m_params[i].m_attributes["name"].toLatin1().data());
		if ( (m_params[i].m_SFilter->m_filtervalue_cb->currentText()=="")
					|| (m_params[i].m_SFilter->m_filtervalue_cb->currentText().isEmpty()) )
		{
			m_button_OK->setEnabled(false);
			QPalette palette;
			palette.setColor(QPalette::Base, QColor(255, 152, 152));
			m_params[i].m_SFilter->m_filtervalue_cb->setPalette(palette);
			return false;
		}
		else
		{
			QPalette palette;
			palette.setColor(QPalette::Base, QColor(255, 255, 255));
			m_params[i].m_SFilter->m_filtervalue_cb->setPalette(palette);
		}
	}
	m_button_OK->setEnabled(true);
	return true;
}

bool	ReportsCenterParamsDialog::OnPickBinNo(QComboBox* cb, bool multiselect)
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::OnPickBinNo");
	//QString GexOneQueryWizardPage1::SqlPickBinningList(...)
	//pGexMainWindow->pDatabaseCenter->QueryGetBinningList(&cFilter, bSoftBin)
	if (cb==NULL)
	{
		cb=this->m_bin_no_comboBox;

		// multiselect or not ?
		// Let s search for the good params
		for (int i=0; i<m_params.size(); i++)
		{
			if (m_params[i].m_name=="P_BIN_NO")
			{
				if (m_params[i].m_multivalue==true)
					multiselect=true;
				break;
			}
		}
	}

	// multiselect or not ?
	// old school GexDatabaseFilter SqlPickBinningList
	//QString bl=SqlPickBinningList(multiselect, this->m_sTestingStage, cb, !m_hardbin_radioButton->isChecked());

	// new implementation try using SFilter struct
	QString bl=PickBinList(multiselect, this->m_sTestingStage, cb, !m_hardbin_radioButton->isChecked());

	if (!bl.isEmpty())
	{
		cb->insertItem(0,bl);
		cb->setCurrentIndex(0);
	}
	else
	{
		// the user cancelled ?
        // Message::information(this, "Warning",
        //  "No bin available. A mandatory filter is probably needed before.");
		//GSLOG(SYSLOG_SEV_DEBUG,"\tno bin available. A mandatory filter is probably needed before choosing a bin.");
	}

	return true;
}

bool ReplaceBinCatInComboBox(QComboBox* cb, QString bincat)
{
	if (!cb)
		return false;
	for (int j=0; j<cb->count(); j++)
	{
		if (cb->itemText(j).contains("bin P/F status", Qt::CaseInsensitive))
		{
			cb->setItemText(j,bincat+"bin P/F status");
			continue;
		}
		if (cb->itemText(j).contains("bin#", Qt::CaseInsensitive))
		{
			cb->setItemText(j,bincat+"bin#");
			continue;
		}
	}
	return true;
}

bool	ReportsCenterParamsDialog::OnHardBinTypeChanged(bool b)
{
	GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterParamsDialog::OnHardBinTypeChanged: BinType changed !");

	// scan all filters and change sbin to hbin or inverse
	for (int i=0; i<m_filters.size(); i++)
	{
		ReplaceBinCatInComboBox(m_filters[i].m_filter_cb, b?"H":"S");
	}

	ReplaceBinCatInComboBox(m_split_comboBox, b?"H":"S");
	ReplaceBinCatInComboBox(m_aggregate_comboBox, b?"H":"S");
	ReplaceBinCatInComboBox(m_serie_comboBox, b?"H":"S");

	return true;
}

bool	ReportsCenterParamsDialog::OnOK()
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 (%2 params) from %3").arg(
			 this->m_sTitle.toLatin1().data(),
			 this->m_params.size(),
		   m_GRXMLsource.toLatin1().data() );

	QString GRTfilename=GenerateGRTfile("output.grt");
	if (GRTfilename.startsWith("error", Qt::CaseInsensitive))
	{
		GSLOG(SYSLOG_SEV_WARNING,"ReportsCenterParamsDialog::OnOK: error generating GRT file !");
		return false;
	}

	if (this->isModal())
		this->hide();

	QString of=((ReportsCenterWidget*)this->parent())->GetCurrentOutputFormat();


	pGexMainWindow->SetWizardType(GEX_JASPER_WIZARD);
	// pGexMainWindow->iWizardType = GEX_JASPER_WIZARD;		// PYC, 27/05/2011
	pGexMainWindow->BuildReportNow(false, "home", true, of, GRTfilename);

	return true;
}

bool	ReportsCenterParamsDialog::OnSave()
{
	GSLOG(SYSLOG_SEV_DEBUG,"ReportsCenterParamsDialog::OnSave: code me !");
	return true;
}
