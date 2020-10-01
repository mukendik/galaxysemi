#include <QApplication>
#include <QComboBox>
#include <QDateTimeEdit>

#include "gex_shared.h"
#include "gex_constants.h"	// for gexTimePeriodChoices
#include <gqtl_log.h>
#include "pickfilter_dialog.h"
#include "pickbin_single_dialog.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "reports_center_params_widget.h"
#include "reports_center_widget.h"
#include "reports_center_multifields.h"
#include "stdlib.h"
#include "stdio.h"
#include "picktest_dialog.h"

int CReportsCenterDataset::n_instances=0;
extern const char*		gexTimePeriodChoices[];		// tables for the time period combobox
extern bool				SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

#define LAST_N_X_LABEL "Last..."

CReportsCenterDataset::CReportsCenterDataset(QWidget* w, QMap<QString, QString> atts)
	: m_timeperiodGB(NULL), m_timeperiodCB(NULL),
	  m_binningGB(NULL), m_hardbinRB(NULL), m_softbinRB(NULL), m_dualcalendarW(NULL),
	  m_FromDTE(NULL), m_ToDTE(NULL)

{
	CReportsCenterDataset::n_instances++;
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" ctor (%1 instances)").arg( CReportsCenterDataset::n_instances));
	m_widget=w;
	m_atts=atts;

	//setParent(m_widget);

	//if (m_atts["time_period_visible"]=="true")
	//{
		m_timeperiodGB=new QGroupBox(m_atts["time_period_label"], m_widget);
		m_timeperiodGB->setLayout(new QVBoxLayout(m_timeperiodGB));
		m_timeperiodGB->setContentsMargins(2,5,2,0);
		m_timeperiodGB->setSizePolicy(QSizePolicy::Minimum, //QSizePolicy::Fixed,
									  QSizePolicy::Minimum);	// Expanding Maximum MinimumExpanding  Minimum Fixed Preferred //);

			//___________________________________________________________
            m_timeperiodCB = new QComboBox(m_timeperiodGB);
            QString lObjectName("Time range");
            m_timeperiodCB->setObjectName(lObjectName);
			m_timeperiodCB->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // Minimum
            m_timeperiodCB->insertItems(0, QStringList()
             << QCoreApplication::translate("reports_center_params_widget", "Today", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Last 2 days (today and yesterday)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Last 3 days (today and last 2 days)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Last 7 days (1 week)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Last 14 days (2 weeks)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Last 31 days (1 full month)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "All days this week", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "All days this month", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "All dates (no restriction on time/date)", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", "Pick dates from calendar...", 0, QCoreApplication::UnicodeUTF8)
             << QCoreApplication::translate("reports_center_params_widget", LAST_N_X_LABEL, 0, QCoreApplication::UnicodeUTF8)
			);
			m_timeperiodCB->setMaxVisibleItems(11);

			m_TimeFactorStepWidget=new QWidget(m_timeperiodGB);
			m_TimeFactorStepWidget->setLayout(new QHBoxLayout( m_TimeFactorStepWidget ));
			m_TimeFactorLE=new QLineEdit("1", m_TimeFactorStepWidget);
			m_TimeFactorLE->setAlignment(Qt::AlignRight);
			m_TimeFactorLE->setValidator(new QIntValidator( 1, 65535, this));
			m_TimeFactorStepWidget->layout()->addWidget(m_TimeFactorLE);
			m_TimeStepCB=new QComboBox(m_TimeFactorStepWidget);
			m_TimeStepCB->insertItems(0, QStringList() << "days" << "weeks" << "months" << "quarters" << "years");
			m_TimeFactorStepWidget->layout()->addWidget(m_TimeStepCB);

			//___________________________________________________
			m_dualcalendarW=new QWidget(m_timeperiodGB);
			m_dualcalendarW->setLayout(new QHBoxLayout(m_dualcalendarW));
			m_dualcalendarW->setContentsMargins(2,1,2,1);
			//m_dualcalendarW->layout()->setSpacing(1);
			m_dualcalendarW->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); //Preferred
                QLabel* fl = new QLabel("From", m_dualcalendarW);
                fl->setAlignment(Qt::AlignRight);
				m_dualcalendarW->layout()->addWidget( fl );

				m_FromDTE = new QDateTimeEdit(m_dualcalendarW);
				//m_FromDTE->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // QSizePolicy::Minimum
				//m_FromDTE->setContentsMargins(1,1,1,1);
				//m_FromDTE->setAlignment();
				m_FromDTE->setDisplayFormat("yyyy.MM.dd");
				m_FromDTE->setCalendarPopup(true);
				m_dualcalendarW->layout()->addWidget(m_FromDTE); //m_timeperiodGB->layout()->addWidget(m_FromDTE);

                QLabel* tl = new QLabel("To", m_dualcalendarW);
                tl->setAlignment(Qt::AlignRight);
				m_dualcalendarW->layout()->addWidget( tl );
				m_ToDTE	= new QDateTimeEdit(QDate::currentDate(), m_dualcalendarW);
				//m_ToDTE->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
				m_ToDTE->setDisplayFormat("yyyy.MM.dd");
				m_ToDTE->setCalendarPopup(true);
				m_dualcalendarW->layout()->addWidget(m_ToDTE);
			//_________________________________________________________

			QObject::connect( (QObject*)m_timeperiodCB, SIGNAL(currentIndexChanged(int)),
								 (QObject*)this, SLOT(SlotTimePeriodChanged(int)) );

			if (m_atts["time_period_defaultValueExpression"]!="")
			{
				// find pos in the table gexTimePeriodChoices
				for (int j=0; j<10; j++)	// this table has 10 elements ? The last is 0
				{
					if (gexTimePeriodChoices[j]==NULL) break;
					if (gexTimePeriodChoices[j]==0) break;
					if (QString(gexTimePeriodChoices[j])==m_atts["time_period_defaultValueExpression"])
					{
						m_timeperiodCB->setCurrentIndex(j);
						break;
					}
				}
			}

		m_timeperiodGB->layout()->addWidget(m_timeperiodCB);
		//m_timeperiodGB->layout()->addWidget(m_TimeFactorLE);
		//m_timeperiodGB->layout()->addWidget(m_TimeStepCB);
		m_timeperiodGB->layout()->addWidget(m_TimeFactorStepWidget);

		m_timeperiodGB->layout()->addWidget(m_dualcalendarW);

		if (m_widget->layout())
			m_widget->layout()->addWidget(m_timeperiodGB);

		if (m_atts["time_period_visible"]!="true")
			m_timeperiodGB->hide();
		//m_timeperiodGB->resize(ds.m_timeperiodGB->sizeHint());
	//}

	if (m_atts["binning_visible"]=="true")
	{
		GexDatabaseEntry* dbe=ReportsCenterWidget::GetInstance()->GetCurrentDatabaseEntry();
		QStringList strlBinTypes;
		if ( (!dbe) || !dbe->m_pExternalDatabase
			|| (!dbe->m_pExternalDatabase->GetSupportedBinTypes(m_atts["stage"], strlBinTypes)
				|| (strlBinTypes.size()==0)
				)
			)
		{
            GSLOG(SYSLOG_SEV_WARNING, " error : cannot retrieve supported Bin types !");
			m_binningGB=new QGroupBox("Failed to retrieve supported bin types from DB !", m_widget);
			m_binningGB->setLayout(new QHBoxLayout(m_binningGB));
			return;
		}

		dbe->m_pExternalDatabase->GetSupportedBinTypes(m_atts["stage"], strlBinTypes);
		QString label(strlBinTypes.size()>1?"Select the bin type here :":QString("Bin type %1").arg(strlBinTypes.at(0)));

		m_binningGB=new QGroupBox(label, m_widget);
		m_binningGB->setLayout(new QHBoxLayout(m_binningGB));

		m_hardbinRB=new QRadioButton("Hard", m_binningGB);
		m_binningGB->layout()->addWidget( m_hardbinRB );
		m_softbinRB=new QRadioButton("Soft", m_binningGB);
		m_binningGB->layout()->addWidget( m_softbinRB );

		if (m_atts["binning"]=="S")
			m_softbinRB->setChecked(true);
		if (m_atts["binning"]=="H")
			m_hardbinRB->setChecked(true);
		if (m_widget->layout())
			m_widget->layout()->addWidget(m_binningGB);

		if (!strlBinTypes.contains("H", Qt::CaseInsensitive))
		{	m_hardbinRB->setChecked(false); m_hardbinRB->setEnabled(false); }
		if (!strlBinTypes.contains("S", Qt::CaseInsensitive))
		{	m_softbinRB->setChecked(false); m_softbinRB->setEnabled(false); }

		if (m_atts["binning_readonly"]=="true")
			m_binningGB->setEnabled(false);


		QObject::connect(m_hardbinRB, SIGNAL(toggled(bool)), this, SLOT(SlotBinTypeChanged(bool)) );	//();
	}
}

CReportsCenterDataset::~CReportsCenterDataset()
{
	CReportsCenterDataset::n_instances--;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" : (still %1 instances)").
          arg(CReportsCenterDataset::n_instances).toLatin1().constData());
	if (m_FromDTE)
		delete m_FromDTE;
	if (m_ToDTE)
		delete m_ToDTE;
}

bool CReportsCenterDataset::SlotTimePeriodChanged(int i)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" time period changed to %1").
          arg(i).toLatin1().constData());
	if (i!=9)
		m_dualcalendarW->hide();
	else
		m_dualcalendarW->show();

	if (i==10)
	{
		m_TimeFactorStepWidget->show();
		//m_TimeFactorLE->show();
		//m_TimeStepCB->show();
	}
	else
	{
		m_TimeFactorStepWidget->hide();
		//m_TimeFactorLE->hide();
		//m_TimeStepCB->hide();
	}

	return true;
}

bool CReportsCenterDataset::SlotBinTypeChanged(bool b)
{
	// true = hard  false = soft
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1").arg(b).
          toLatin1().constData());

	this->m_atts.insert("binning", b?"H":"S");

	QMap<QString, CReportsCenterMultiFields*>::iterator i=m_MultiFieldsMap.begin();
	for (i=m_MultiFieldsMap.begin(); i!=m_MultiFieldsMap.end(); i++)
	{
		if (!i.value())
			continue;
		CReportsCenterMultiFields* mf=i.value();
		mf->ReplaceBinCatInComboBox(b?"H":"S");
	}

	CReportsCenterParamsWidget* pw=(CReportsCenterParamsWidget*) parent();
	if (pw)
		pw->CheckForMandatoryValues();

	return true;
}

bool CReportsCenterDataset::AddMultiFields(QMap<QString, QString> atts,
											QVector< QMap<QString,QString> > children
														  )
{
	if ( m_MultiFieldsMap.find(atts["role"])!=m_MultiFieldsMap.end() )
	{
		GSLOG(SYSLOG_SEV_NOTICE, " warning : multifield with such role already exist ! Ignoring.");
		return false;
	}

	if (atts.size()<2)
	{
		GSLOG(SYSLOG_SEV_WARNING, QString("error : not enough attributes for the MultiField with role '%1' !").arg( atts["role"]).toLatin1().constData() );
		return false;
	}

	// impose some attributes to the MultiField
	atts["stage"]=m_atts["stage"];
	atts["consolidated"]=m_atts["consolidated"];

	if ( (m_atts["binning"]!="") && (atts["binning"]=="") )
		atts["binning"]=m_atts["binning"]; 	// impose the DataSet binning
	else
	if ( (m_atts["binning"]=="") && (atts["binning"]=="") )
		atts["binning"]="N";	// binning attribute not found in GRXML mean 'None'
	else
	if ( (m_atts["binning"]!="") &&  (atts["binning"]!="") && (m_atts["binning"]!=atts["binning"]) )
		atts["binning"]="N";	// binning different between MF and DS : impose 'None'


	CReportsCenterMultiFields* mf=new CReportsCenterMultiFields(m_widget, this, atts);
	if (!mf)
		return false;

	m_MultiFieldsMap.insert(atts["role"], mf);

	for (int i=0; i<children.size(); i++)
		mf->AddField(children.at(i));

	// add a default field
	mf->AddField(atts);

	if ( (atts["field_mandatory"]=="true") && (atts["autorepeat"]=="true") )
	{
		QMap<QString, QString> a=atts;
		a.insert("field_mandatory","false");
		mf->AddField(a);
	}

	m_widget->layout()->addWidget(mf);
	if (atts["visible"]=="false")
		mf->hide();
	//m_widget->setMinimumSize(m_widget->sizeHint());
	return true;
}

bool CReportsCenterDataset::PickDistinct(QComboBox *field_cb, QComboBox *value_cb, bool multiselect)
{
	if ( (!field_cb) || (!value_cb) || (field_cb->currentText()=="")
		|| (field_cb->currentText()==SELECT_SERIE_INVIT)
		|| (field_cb->currentText()==SELECT_FIELD_INVIT))
	{
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" field combobox anormal ('%1').")
              .arg( field_cb?field_cb->currentText():"NULL").toLatin1().constData());
		return false;
	}
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" on '%1' ").arg( field_cb->currentText()).toLatin1().constData());

	GexDatabaseEntry* dbe=ReportsCenterWidget::GetInstance()->GetCurrentDatabaseEntry();
	if (!dbe)
	{
		GSLOG(SYSLOG_SEV_WARNING, " error : cant retrieve DatabaseEntry !");
		return false;
	}

    GexDbPlugin_Filter	clPluginFilter(this);
	clPluginFilter.SetFields(QStringList(field_cb->currentText()));
	clPluginFilter.strDataTypeQuery=this->m_atts["stage"];
	clPluginFilter.strlQueryFilters.clear();
	clPluginFilter.calendarFrom = m_FromDTE->date();
	clPluginFilter.calendarTo = m_ToDTE->date();
	clPluginFilter.iTimePeriod = m_timeperiodCB->currentIndex();
	clPluginFilter.iTimeNFactor = m_TimeFactorLE->text().toInt();
	clPluginFilter.m_eTimeStep = (GexDbPlugin_Filter::eTimeStep)m_TimeStepCB->currentIndex();
	clPluginFilter.bConsolidatedData = (this->m_atts["consolidated"]=="false")?false:true;

	QMap<QString, CReportsCenterMultiFields*>::iterator i=m_MultiFieldsMap.begin();
	for (i=m_MultiFieldsMap.begin(); i!=m_MultiFieldsMap.end(); i++)
	{
		if (!i.value())
			continue;
		CReportsCenterMultiFields* mf=i.value();
		QStringList sl=mf->GetFiltersStrings();
		clPluginFilter.strlQueryFilters.append(sl);
	}

	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("filters : '%1'").arg( clPluginFilter.strlQueryFilters.join("|")).toLatin1().constData());

	QStringList			cMatchingFilters;	// output

	if ( (field_cb->currentText().toLower()=="bin no")
		|| (field_cb->currentText().toLower().contains("bin#", Qt::CaseInsensitive)) )
	{
		QString fieldToQuery="Sbin#";
		if (m_atts["binning"]=="S") fieldToQuery="Sbin#";
		else if (m_atts["binning"]=="H") fieldToQuery="Hbin#";
		dbe->m_pExternalDatabase->QueryBinlist(clPluginFilter, cMatchingFilters,
											   (m_atts["binning"]=="S")?true:false,
											   true, true);

		if (cMatchingFilters.count() == 0)
		{
			GSLOG(SYSLOG_SEV_INFORMATIONAL, " QueryBinList has returned no result. ");
			return false;
        }

		PickBinSingleDialog dPickFilter;
		dPickFilter.fillList(cMatchingFilters);
		dPickFilter.setMultipleSelection(multiselect);
		// Prompt dialog box, let user pick Filter string from the list
		if(dPickFilter.exec() != 1)
			return false;	// User 'Abort'
        value_cb->insertItem(0, dPickFilter.getBinsList(), dPickFilter.getBinsNameList());
		value_cb->setCurrentIndex(0);
		//value_cb->setCurrentText(dPickFilter.getBinsList()); //.replace('|',',');
		return true;
	}
	else if (field_cb->currentText().toLower()=="test#")
	{
		dbe->m_pExternalDatabase->QueryTestlist(clPluginFilter, cMatchingFilters, true);
		if (cMatchingFilters.count() == 0)
		{
			GSLOG(SYSLOG_SEV_INFORMATIONAL, " QueryTestList has returned no result.");
			return false;
		}

		PickTestDialog dPickTest;
		dPickTest.setMultipleSelection(true);

		if (!dPickTest.fillParameterList(cMatchingFilters))
		{
			GSLOG(SYSLOG_SEV_INFORMATIONAL, "cant fill a PickTestDialog with the returned tests list !");
			return false;
		}
		// Prompt dialog box, let user pick tests from the list
		if(dPickTest.exec() == QDialog::Accepted)
		{
			value_cb->insertItem(0, dPickTest.testList());
			value_cb->setCurrentIndex(0);
			return true;
		}
		return false;
	}
	else
	if (!dbe->m_pExternalDatabase->QueryField(clPluginFilter, cMatchingFilters,
											  (m_atts["binning"]=="S")?true:false))
	{
		GSLOG(SYSLOG_SEV_ERROR, " QueryField failed !");
		return false;
	}

	if (cMatchingFilters.size()==0)
	{
		GSLOG(SYSLOG_SEV_NOTICE, "QueryField returned no result. Quite unusual.");
		return false;
	}

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

	if (value_cb->findText(strSelection)>-1)
	{
		value_cb->setCurrentIndex(value_cb->findText(strSelection));
		return true;
	}

	/*
	for(int i=0; i<value_cb->count(); i++)
	{
		if (strSelection == value_cb->text(i))
		{
			// Selection already in combo...simply select it.
			SetCurrentComboItem(value_cb, strSelection);
			return true;
		}
	}
	*/

	value_cb->insertItem(0, strSelection);
	value_cb->setCurrentIndex(0);
	//SetCurrentComboItem(value_cb, strSelection);

	return true;
}

bool CReportsCenterDataset::ExportToDom(QDomDocument &doc, QDomElement &datasetsElem)
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1").arg( m_atts["ID"]).toLatin1().constData());

	QDomElement c=datasetsElem.firstChildElement("dataset");
	while (!c.isNull())
	{
		if (c.attribute("ID")=="")
		{
			c=c.nextSiblingElement("dataset"); continue;
		}
		if (c.attribute("ID")!=this->m_atts["ID"])
		{
			c=c.nextSiblingElement("dataset"); continue;
		}
		//QDomNodeList QDomNode::childNodes();
		//c=c.nextSiblingElement("dataset");
		break;
	}

	if (c.isNull())
	{
		//c=QDomNode();
		c=doc.createElement("dataset");
		//c.setTagName("dataset");

		QDomNode n=datasetsElem.appendChild(c);
		if (n.isNull())
		{	GSLOG(SYSLOG_SEV_NOTICE, " appendChild failed !");
			return false;
		}
	}

	QString strDatabaseLogicalName = ReportsCenterWidget::GetInstance()->GetDatabaseComboBox()->currentText();
	if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
		strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();

	c.setAttribute("DatabaseName", strDatabaseLogicalName);
	c.setAttribute("ID", this->m_atts["ID"]);
	c.setAttribute("BinType", this->m_atts["binning"]);
	c.setAttribute("Stage", this->m_atts["stage"]);
	c.setAttribute("TimePeriod", gexTimePeriodChoices[this->m_timeperiodCB->currentIndex()]);
	if (m_timeperiodCB->currentText()==LAST_N_X_LABEL)
	{
		c.setAttribute("TimeNFactor", m_TimeFactorLE->text());
		c.setAttribute("TimeStep", m_TimeStepCB->currentText());
	}
	c.setAttribute("CalendarFrom", m_FromDTE->date().toString("yyyy-MM-dd"));
	c.setAttribute("CalendarTo", m_ToDTE->date().toString("yyyy-MM-dd"));

	QMap<QString, CReportsCenterMultiFields*>::iterator	i=m_MultiFieldsMap.begin();
	for (i=m_MultiFieldsMap.begin(); i!=m_MultiFieldsMap.end(); i++)
	{
		if (!i.value())
			continue;
		if (!i.value()->ExportToDom(doc, c))
		{
			GSLOG(SYSLOG_SEV_NOTICE, " failed to export multifield to xml.");
			//continue;
		}
	}

	return true;
}

QString CReportsCenterDataset::ExportToCsl(FILE* hFile, QString group_id)
{
	if (!hFile)
		return "error : csl file handle NULL !";

	fprintf(hFile, "\t// Dataset %s '%s'\n",
			m_atts["ID"].toLatin1().data(),
			m_atts["label"].toLatin1().data());
	fprintf(hFile, "\tgexQuery('db_data_type','%s');\n",
			m_atts["stage"].toLatin1().data() );
	fprintf(hFile, "\tgexQuery('db_consolidated','%s');\n",
			(m_atts["consolidated"]=="false")?"false":"true" );

	CReportsCenterParamsWidget* pw=(CReportsCenterParamsWidget*) parent();
	if (!pw)
	{
		QString t("error : cant retrieve ParamsWidget for this DataSet !");
		GSLOG(SYSLOG_SEV_WARNING, t.toLatin1().data());
		return t;
	}

	if (m_atts["export"]!="" || m_atts["gexQuery"]!="")
	{
		QString t=m_atts["export"];
		if (t.contains("$O{"))
		{
			QMap<int, QMap< QString, QString> >::iterator oit=pw->m_options_properties.begin();
			for (oit=pw->m_options_properties.begin(); oit!=pw->m_options_properties.end(); oit++)
			{
				QString ID=oit.value()["ID"];
				if (ID.isEmpty())
					continue;
				int pid=oit.key();
				if (pid<0)
					continue;
				ID.prepend("$O{"); ID.append("}");
				QVariant cv=pw->m_pb->GetCurrentValue(pid);
				t.replace(ID, cv.toString());
			}
		}
		if (t.isEmpty())
			t=m_atts["gexQuery"];
		fprintf(hFile, "\t%s\n", t.toLatin1().data() );
	}

	// gexQuery('db_period','<Period>','From_day From_Month From_Year To_day To_Month To_Year');"
	//fprintf(hFile, "\tgexQuery('db_period','all_dates','2011 1 20 2011 1 20 00:00:00 23:59:59');" );

	if (m_timeperiodCB->currentIndex()!=GEX_QUERY_TIMEPERIOD_LAST_N_X)
	 fprintf(hFile,"\tgexQuery('db_period','%s','%d %d %d %d %d %d');\n",
			gexTimePeriodChoices[ (m_timeperiodCB)?m_timeperiodCB->currentIndex():0 ],
			this->m_FromDTE->date().year(),
			this->m_FromDTE->date().month(),
			this->m_FromDTE->date().day(),
			this->m_ToDTE->date().year(),
			this->m_ToDTE->date().month(),
			this->m_ToDTE->date().day()
			);
	else
	{
		fprintf(hFile,"\tgexQuery('db_period','%s','%s %s');\n",
			gexTimePeriodChoices[ m_timeperiodCB->currentIndex()],
			m_TimeFactorLE->text().toLatin1().data(),
			m_TimeStepCB->currentText().toLatin1().data()
				);
		GSLOG(SYSLOG_SEV_WARNING, "check me");
	}

	// gexQuery('dbf_sql','...
	QMap<QString, CReportsCenterMultiFields*>::iterator i=m_MultiFieldsMap.begin();
	for (i=m_MultiFieldsMap.begin(); i!=m_MultiFieldsMap.end(); i++)
	{
		if (!i.value())
			continue;
		CReportsCenterMultiFields* mf=i.value();
		QString r=mf->ExportToCsl(hFile);
	}

	fprintf(hFile,"\tgexGroup('insert_query','%s' %s );\n\n", m_atts["ID"].toLatin1().data(),
                        (group_id!="")?(QString(", ")+group_id).toLatin1().data():""
			);

	return "ok";
}

bool CReportsCenterDataset::HasAtLeastOneMandatoryFieldUnknown()
{
	QMap<QString, CReportsCenterMultiFields*>::iterator	i=m_MultiFieldsMap.begin();
	for (i=m_MultiFieldsMap.begin(); i!=m_MultiFieldsMap.end(); i++)
	{
		if (!i.value())
			continue;
		CReportsCenterMultiFields* mf=i.value();
		if ( mf->HasAtLeastOneMandatoryFieldUnknown() )
			return true;
	}
	return false;
}
