#include <QDomDocument>
//#include <gqtl_log.h>
#include "reports_center_widget.h"
#include "reports_center_params_dialog.h"
#include "calendar_dialog.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "pickfilter_dialog.h"
#include <gqtl_log.h>


extern const char*		gexTimePeriodChoices[];		// tables for the time period combobox
extern const char *		szAppFullName;
extern GexMainwindow *	pGexMainWindow;
extern QString			strApplicationDir;
extern QString			strUserFolder;

extern void				WriteDebugMessageFile(const QString & strMessage);
// in classes.cpp
extern bool				SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

QString	ReportsCenterParamsDialog::GenerateGRTfile(QString outputFileName)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString("ReportsCenterParamsDialog::GenerateGRTfile to %1").arg( outputFileName).toLatin1().constData());

	//search for P_GRT_FILE param :
	QString grtfile;
	for (int i=0;i<m_params.size(); i++)
		if (m_params[i].m_name=="P_GRT_FILE")
			grtfile=m_params[i].m_attributes["defaultValueExpression"];

	if (grtfile=="")
		return QString("error : no GRT defined for this template !");

	QString p=this->m_GRXMLsource.section('/',0,-2)+QDir::separator()+grtfile;
		//strUserFolder+QDir::separator()+ "GalaxySemi" +QDir::separator()+"GRXML"

	QFile file(p);
	if (!file.open(QIODevice::ReadOnly))
		return QString("error : cant open file %1 !").arg(p.toLatin1().data());
	GSLOG(SYSLOG_SEV_DEBUG, QString(" using template '%1'").arg( p).toLatin1().constData());

	QDomDocument doc("grt");
	if (!doc.setContent(&file))
	{
		file.close();
		return QString("error : cant apply xml document content !");
	}
	file.close();

	QDomElement gte=doc.firstChildElement("galaxy_template");
	if (gte.isNull())
		return QString("error : cant find galaxy_template element !");

	QDomElement report_elem=gte.firstChildElement("report");
	if (report_elem.isNull())
		return QString("error : cant find report element !");

	QDomElement dataset_elem=report_elem.firstChildElement("dataset");
	if (dataset_elem.isNull())
		return QString("error : cant find a dataset element !");

	dataset_elem.setAttribute("CalendarFrom", m_FromDateEdit->date().toString("yyyy-MM-dd"));
	dataset_elem.setAttribute("CalendarTo", m_ToDateEdit->date().toString("yyyy-MM-dd"));

	QDomElement filters_elem=dataset_elem.firstChildElement("filters");
	if (filters_elem.isNull())
	{
		filters_elem=doc.createElement("filters");
		dataset_elem.appendChild(filters_elem);
	}


	// Adding serie filter
	for (int i=0; i<m_params.size(); i++)
	{
		if (m_params[i].m_name!="P_SERIE")
			continue;
		if (m_serie_comboBox->currentText()==SELECT_SERIE_INVIT)
			break;
		if ( (m_serieFilter_comboBox->currentText()=="*") || (m_serieFilter_comboBox->currentText()=="") )
			break;
		QDomElement f=doc.createElement("filter");
		f.setAttribute("field", m_serie_comboBox->currentText());
		f.setAttribute("op", "=");
		f.setAttribute("value", m_serieFilter_comboBox->currentText());
		if (f.isNull())
			GSLOG(SYSLOG_SEV_ERROR, "\terror : null filter node !");
		else
			filters_elem.appendChild(f);
		break;
	}
	// Adding filters...
	for (int i=0; i<m_filters.size(); i++)
	{
		if (!m_filters[i].m_assigned)
			continue;
		// a 'visible' widget is not visible if in a hidden tab...
		//if (!m_filters[i].isVisible())
		//	continue;
		if (m_filters[i].m_filtervalue_cb->currentText()=="*")
			continue;
		if (m_filters[i].m_filtervalue_cb->currentText()=="")
			continue;
		if ( (m_filters[i].m_filter_cb->currentText()=="") || (m_filters[i].m_filter_cb->currentText()==SELECT_FILTER_INVIT) )
			continue;
		GSLOG(SYSLOG_SEV_DEBUG, QString("\tadding filter %1").arg( m_filters[i].m_filter_cb->currentText()).toLatin1().constData());
		QDomElement f=doc.createElement("filter");
		f.setAttribute("field", m_filters[i].m_filter_cb->currentText());
		if (m_filters[i].m_operator->text()=="=")
			f.setAttribute("op", "=");
		else
			f.setAttribute("op", "!=");
		f.setAttribute("value", m_filters[i].m_filtervalue_cb->currentText());
		filters_elem.appendChild(f);
	}

	// removing tables or charts
	for (int i=0; i<m_params.size(); i++)
	{
		if (m_params[i].m_name=="P_SHOW_TABLES")
		{	if (!m_showTable_checkBox->isChecked())
			{
				QDomElement g=report_elem.firstChildElement("group");
				while (!g.isNull())
				{
					//QDomElement g=report_elem.firstChildElement("group");
					while (!g.firstChildElement("table").isNull())
						g.removeChild(g.firstChildElement("table"));
					g=g.nextSiblingElement("group");
				}
			}
			continue;
		}
		if (m_params[i].m_name=="P_SHOW_CHARTS")
		{	if (!m_showChart_checkBox->isChecked())
			{
				QDomElement g=report_elem.firstChildElement("group");
				while (!g.isNull())
				{
					while (!g.firstChildElement("chart").isNull())
						g.removeChild(g.firstChildElement("chart"));
					g=g.nextSiblingElement("group");
				}
			}
			continue;
		}
	}

	// write ouput grt file
	//QFile outfile(this->m_GRXMLsource.section('/',0,-2)+QDir::separator()+"output.grt");
	QDir d(strUserFolder+QDir::separator()+"GalaxySemi");
	if (!d.exists())
	 if (!d.mkdir(strUserFolder+QDir::separator()+"GalaxySemi"))
		return QString("error : cant create GalaxySemi dir in user folder !");

	QFile outfile(strUserFolder+QDir::separator()+"GalaxySemi"+QDir::separator()+outputFileName ); // "output.grt"

	if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
		return QString("error : cant open output grt file %1 !").arg(outfile.fileName());

	QTextStream outstream(&outfile);
	QByteArray ba=doc.toByteArray();

	// raw replace !
	for (int i=0; i<m_params.size(); i++)
	{
		QString strDatabaseLogicalName = ((ReportsCenterWidget*)this->parent())->GetDatabaseComboBox()->currentText();
		if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
			strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
		if (m_params[i].m_name=="P_DATABASE_NAME")
		 ba.replace("$P{P_DATABASE_NAME}", strDatabaseLogicalName);
		else if (m_params[i].m_name=="P_DATA_TYPE")
		 ba.replace("$P{P_DATA_TYPE}", m_params[i].m_data_type);
		else if (m_params[i].m_name=="P_TIME_PERIOD")
		 ba.replace("$P{P_TIME_PERIOD}", gexTimePeriodChoices[m_comboBoxTimePeriod->currentIndex()]); //ba.replace("$P{P_TIME_PERIOD}", comboBoxTimePeriod->currentText());
		else if (m_params[i].m_name=="P_BIN_TYPE")
		 ba.replace("$P{P_BIN_TYPE}", this->m_hardbin_radioButton->isChecked()?"hard":"soft");
		else if (m_params[i].m_name=="P_BIN_NO")
		 ba.replace("$P{P_BIN_NO}", this->m_bin_no_comboBox->currentText());
		else if (m_params[i].m_name=="P_SPLIT")
			ba.replace("$P{P_SPLIT}", m_split_comboBox->currentText() );
		else if (m_params[i].m_name=="P_AGGREGATE")
			ba.replace("$P{P_AGGREGATE}", m_aggregate_comboBox->currentText() );
		else if (m_params[i].m_name=="P_FULL_AGG_RANGE")
			ba.replace("$P{P_FULL_AGG_RANGE}", m_FullAggRange_checkBox->isChecked()?"true":"false" );
		else if (m_params[i].m_name=="P_SERIE")
			ba.replace("$P{P_SERIE}", m_serie_comboBox->currentText()==SELECT_SERIE_INVIT?"":m_serie_comboBox->currentText());
		else if (m_params[i].m_name=="P_CHART_LEFTAXIS_MIN")
			ba.replace("$P{P_CHART_LEFTAXIS_MIN}", m_LeftMin_lineEdit->text() );
		else if (m_params[i].m_name=="P_CHART_LEFTAXIS_MAX")
			ba.replace("$P{P_CHART_LEFTAXIS_MAX}", m_LeftMax_lineEdit->text() );
		else if (m_params[i].m_name=="P_CHART_RIGHTAXIS_MIN")
			ba.replace("$P{P_CHART_RIGHTAXIS_MIN}", m_RightMin_lineEdit->text() );
		else if (m_params[i].m_name=="P_CHART_RIGHTAXIS_MAX")
			ba.replace("$P{P_CHART_RIGHTAXIS_MAX}", m_RightMax_lineEdit->text() );

		else if (m_params[i].m_name=="P_OPTIONS")
		{
		}
		else if (m_params[i].m_name=="P_MAX_NUM_AGGREGATES_IN_CHART")
		{
			ba.replace("$P{P_MAX_NUM_AGGREGATES_IN_CHART}", m_chartXlimit_lineEdit->text() );
		}
		else if (m_params[i].m_name=="P_SHOW_VOL_IN_CHART")
		{
			ba.replace("$P{P_SHOW_VOL_IN_CHART}", m_ShowVolInChart_checkBox->isChecked()?"true":"false");
		}
		else if (m_params[i].m_name=="P_SERIE_GRAPH_FILTERS_ON_YIELD")
			ba.replace( "$P{P_SERIE_GRAPH_FILTERS_ON_YIELD}", m_GSF_yield_comboBox->currentText() );
		else if (m_params[i].m_name=="P_SERIE_GRAPH_FILTERS_ON_VOL")
			ba.replace( "$P{P_SERIE_GRAPH_FILTERS_ON_VOL}", m_GSF_vol_comboBox->currentText() );
		else if (m_params[i].m_name=="P_YIELD_AXIS_STYLE")
		{
			ba.replace( "$P{P_YIELD_AXIS_STYLE}", m_YieldAxisStyle_comboBox->currentText() );
		}
		else if (m_params[i].m_name=="P_VOL_AXIS_STYLE")
		{
			ba.replace( "$P{P_VOL_AXIS_STYLE}", m_VolAxisStyle_comboBox->currentText() );
		}
		else if (m_params[i].m_attributes["rawReplace"]=="true") // P_OVERALL_YIELD, P_YIELD, P_VOLUME...
		{	ba.replace(	QString("$P{%1}").arg(m_params[i].m_name).toLatin1().data(),
					   m_params[i].m_attributes["defaultValueExpression"]);
		}
	}

	outstream << ba;
	outfile.close();

	return outfile.fileName();
}
