#include <QLayout>
#include <QDir>
#include <QLabel>
#include <QFile>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "libjasper.h"
#include "reports_center_widget.h"

#include "jasper_threads.h"

#include "jasper_params_widget.h"
#include "browser_dialog.h"


extern GexMainwindow *	pGexMainWindow;
extern QString			strUserFolder;
extern void				WriteDebugMessageFile(const QString & strMessage);

QVector<ParamsWidget*> ParamsWidget::s_ParamsWidgets;

ParamsWidget::ParamsWidget(QWidget* p, char o[MAX_NUM_RESOURCES][6][MAX_STRING_SIZE], int nParams, QString &report_uri)
		:QWidget(p, Qt::Dialog), m_report_uri(report_uri)
{
	qDebug("ParamsWidget::ParamsWidget: %d params to build...", nParams);

	//this->setWindowOpacity(0.9);
	this->setBackgroundRole(QPalette::Base);
	setBackgroundColor(QColor(255, 255, 255));

	QVBoxLayout* l=new QVBoxLayout(this);
	this->setLayout(l);

	if (!o)
		return;

	for (int i=0; i<nParams; i++)
	{
		if (!o[i])
			continue;
		if ( (!o[i][0]) || (!o[i][1]) || (!o[i][2]) || (!o[i][3]) )
		{
			qDebug("JasperParamsWidget::JasperParamsWidget: error ! params %d null ! ignoring...", i);
			continue;
		}
		SParam p;
		char* label=o[i][0];		p.m_label.append(label);
		char* uri=o[i][1];			p.m_uri.append(uri);
		char* ictype=o[i][2];		p.m_IC_type.append(ictype);
		char* datatype=o[i][3];		p.m_DataType.append(datatype);
		const char* desc=o[i][4]?o[i][4]:" "; p.m_description.append(desc);
		const char* defaultV=o[i][5]?o[i][5]:"";

		qDebug(" creating widgets params %s with type %s and datatype %s, default %s", label?label:"?", ictype, datatype, defaultV);

		QHBoxLayout* h=new QHBoxLayout(l);

		h->addWidget(new QLabel(label));

		if (p.m_IC_type=="JS_IC_TYPE_BOOLEAN")
			p.m_value_widget=new QCheckBox(this);
		else if (p.m_IC_type=="JS_IC_TYPE_SINGLE_VALUE")
		{
			if (p.m_DataType=="JS_DT_TYPE_DATE" || p.m_DataType=="JS_DT_TYPE_DATE_TIME" )
			{	p.m_value_widget=new QDateTimeEdit(this); ((QDateTimeEdit*) p.m_value_widget)->setCalendarPopup(true);
			}
			else
			{
				p.m_value_widget=new QLineEdit(this);
				if (p.m_DataType=="JS_DT_TYPE_NUMBER")
				{
					((QLineEdit*) p.m_value_widget)->setText(QString(defaultV).remove(QRegExp("[/D]"))); // ![0-9]
					((QLineEdit*) p.m_value_widget)->setValidator(new QDoubleValidator( -5000000.0, 999999.0, 2, this));
					qDebug("JasperParamsWidget::JasperParamsWidget: set LineEdit to %s",
						   ((QLineEdit*) p.m_value_widget)->text().toLatin1().data());
					//((QLineEdit*) p.m_value_widget)->setInputMask("99999999");
				}
				else ((QLineEdit*) p.m_value_widget)->setText(defaultV);
			}
		}
		//else if (p.m_IC_type=="JS_IC_TYPE_SINGLE_SELECT_LIST_OF_VALUES")
		//	p.m_value_widget=new QComboBox(this);
		else if (p.m_IC_type=="JS_IC_TYPE_SINGLE_SELECT_QUERY"
				 || p.m_IC_type=="JS_IC_TYPE_SINGLE_SELECT_LIST_OF_VALUES"
				 || p.m_IC_type=="JS_IC_TYPE_MULTI_SELECT_QUERY" )
		{
			p.m_value_widget=new QComboBox(this);
			if (p.m_IC_type=="JS_IC_TYPE_MULTI_SELECT_QUERY")
					((QComboBox*)p.m_value_widget)->view()->setSelectionMode(QAbstractItemView::MultiSelection);
			jasper_acquire_values_for_param_f f
					=(jasper_acquire_values_for_param_f)ReportsCenterWidget::pclJasperLibrary->resolve("jasper_acquire_values_for_param");
			if (f)
			{
				char values[MAX_NUM_RESOURCES][2][MAX_STRING_SIZE];
				int r=f(p.m_uri.toLatin1().data(), m_report_uri.toLatin1().data(), (char***)values);
				qDebug(" %s has %d possible values...", uri, r);
				if (r>0)
				{
					QComboBox* cb=(QComboBox*)p.m_value_widget;
					for (int i=0; i<r; i++)
					{
						cb->addItem(QString(values[i][0])); //+" "+QString(values[i][1]));
					}
				}
			}
			else
				qDebug(" error : cant resolve 'jasper_acquire_values_for_param' in libjasper.");
		}
		else if (p.m_IC_type=="JS_IC_TYPE_MULTI_VALUE"
				 || p.m_IC_type=="JS_IC_TYPE_MULTI_SELECT_LIST_OF_VALUES"
				 )
		{
			p.m_value_widget=new QComboBox(this);
			((QComboBox*)p.m_value_widget)->view()->setSelectionMode(QAbstractItemView::MultiSelection);
		}
		else
			WriteDebugMessageFile(QString("JasperParamsWidget::JasperParamsWidget: warning unhandled IC %1. ToDo.").arg(p.m_IC_type));

		h->addWidget(p.m_value_widget);
		m_params.push_back(p);

		//l->addLayout(h);
	}

	m_ok_pb=new QPushButton("OK", this);
	l->addWidget( m_ok_pb);
	connect(m_ok_pb, SIGNAL(clicked()), JasperRunReportThread::s_singleton, SLOT(RequestReportSlot()) );

	LoadParamsFromFile();

	qDebug("JasperParamsWidget::JasperParamsWidget: ok.");

}

bool ParamsWidget::GetCurrentParams(QVector< QPair <QString, QString> > &params)
{
	for (int i=0; i<m_params.size(); i++)
	{
		QPair<QString, QString> p;
		p.first=m_params[i].m_uri.section('/',-1);
		p.second= "?"; //m_params[i].m_value_widget ;//"max_id";
		//QComboBox* cb = qobject_cast<QComboBox *>(m_params[i].m_value_widget);
		//if (cb)
		if (QComboBox* cb = qobject_cast<QComboBox *>(m_params[i].m_value_widget))
		{
			p.second= cb->currentText();
		}		
		else if (QCheckBox* b=qobject_cast< QCheckBox*>(m_params[i].m_value_widget))
		{
			if (b->isChecked())
				p.second="true";
			else
				p.second="false";
		}
		else if (QLineEdit* le=qobject_cast< QLineEdit*>(m_params[i].m_value_widget))
		{
			p.second=le->text();
		}
		else if (QDateTimeEdit* e=qobject_cast< QDateTimeEdit*>(m_params[i].m_value_widget))
		{
			// A jasper date must be represented as a number holding the number of MILLISECONDS from Jan 1, 1970. (java standard).
			long t=(long)(e->dateTime().toTime_t()); // num of sec
			long long int llt=((long long)t)*1000;
			//qDebug(" %s = %ld s = %ll ms", e->dateTime().toString().toLatin1().data(), t, llt);
			p.second=QString("%1").arg(llt);
		}

		qDebug("JasperParamsWidget::GetCurrentParams: %d : %s %s", i, p.first.toLatin1().data(), p.second.toLatin1().data());

		params.push_back( p );
	}
	return true; //m_params.size();
}

// public slot
void ParamsWidget::ReportGenerated(const QString report_uri, const QString output)
{
	if (this->m_report_uri!=report_uri)
	 return;
	SaveCurrentParamsToFile();
	qDebug("JasperParamsWidget::ReportGenerated %s", output.toLatin1().data());
	m_ok_pb->setEnabled(true);
}

// static
ParamsWidget* ParamsWidget::GetParamsWidgetForReport(QString report_uri,
													 QString /*report_name*/)
{
	qDebug("JasperParamsWidget::GetParamsWidgetForReport %s", report_uri.toLatin1().data());
	ParamsWidget* w=NULL;
	foreach ( w, s_ParamsWidgets)
	{	if (w->m_report_uri==report_uri)
			return w;
	}
	return NULL;
}

ParamsWidget* ParamsWidget::CreateParamsWidgetForReport(
		QString &report_uri, QString report_label, char data[MAX_NUM_RESOURCES][6][MAX_STRING_SIZE], int nIC)
{
	qDebug("ParamsWidget::CreateParamsWidgetForReport %d %s", nIC, report_uri.toLatin1().data());
	ParamsWidget* w=new ParamsWidget( (QWidget*)pGexMainWindow->pReportsCenter, data, nIC, report_uri);
	w->setWindowTitle(report_label.append(" params"));
	w->m_report_label=report_label;
	s_ParamsWidgets.push_back(w);

	return w;
}

bool ParamsWidget::SaveCurrentParamsToFile()
{
	QString rsuri=m_report_uri; rsuri.replace('/',"_");
	QFile f(strUserFolder+QDir::separator()+REPORTS_FOLDER+QDir::separator()+"params_"+rsuri+".xml");
	if (!f.open(QIODevice::ReadWrite|QIODevice::Truncate))
		return false;
	qDebug("ParamsWidget::SaveCurrentParamsToFile %s", f.fileName().toLatin1().data());

	QXmlStreamWriter stream( &f);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();

	// all params in 1 element style
	stream.writeStartElement("parameters");
	QVector< QPair <QString, QString> > params;
	this->GetCurrentParams(params);
	for (int i=0; i<params.size(); i++)
	{
		stream.writeAttribute(params[i].first, params[i].second);
	}
	stream.writeEndElement();

	/*
	// 1 element per params style
	QVector< QPair <QString, QString> > params;
	this->GetCurrentParams(params);
	for (int i=0; i<params.size(); i++)
	{
		stream.writeStartElement("parameter");
		stream.writeAttribute(params[i].first, params[i].second);
		stream.writeEndElement(); //
	}
	*/

	stream.writeEndDocument();
	f.close();
	return true;
}

bool ParamsWidget::LoadParamsFromFile()
{
	qDebug("ParamsWidget::LoadParamsFromFile for %s", this->m_report_uri.toLatin1().data());
	QString rsuri=m_report_uri; rsuri.replace('/',"_");
	QFile f(strUserFolder+QDir::separator()+REPORTS_FOLDER+QDir::separator()+"params_"+rsuri+".xml");
	if (!f.open(QIODevice::ReadOnly))
		return false;

	qDebug("\t%s opened...", f.fileName().toLatin1().data());
	QXmlStreamReader xml( &f);

	while (!xml.atEnd())
	{
		xml.readNext();
		//qDebug("ParamsWidget::LoadParamsFromFile: element '%s'", xml.name().toString().toLatin1().data());
		if (xml.name()=="parameters")
		{
			QXmlStreamAttributes att=xml.attributes();
			qDebug("ParamsWidget::LoadParamsFromFile: parameters found with %d attributes.", att.size() );

			for (int na=0; na<att.size(); na++)
			{
				QXmlStreamAttribute a=att[na];
				QString attname=a.name().toString();
				QString attvalue=a.value().toString();
				qDebug("\tattr %s found with value %s", attname.toLatin1().data(), attvalue.toLatin1().data());
				//SParam *p=NULL;
				for (int i=0; i<m_params.size(); i++)
					if (m_params[i].m_uri.section('/',-1)==attname)
					{
						qDebug("\t\tparam found !");
						QWidget *w=m_params[i].m_value_widget;
						if (QComboBox* cb = qobject_cast<QComboBox *>(w))
						{
							cb->setCurrentIndex(cb->findText(attvalue));;
						}
						else if (QCheckBox* b=qobject_cast< QCheckBox*>(w))
						{
							if (attvalue=="true") b->setChecked(true);
							else b->setChecked(false);
						}
						else if (QLineEdit* le=qobject_cast< QLineEdit*>(w))
						{
							//if (le->inputMask()=="9")
								// ?
							//else
								le->setText(attvalue);

						}
						else if (QDateTimeEdit* e=qobject_cast< QDateTimeEdit*>(w))
						{
							// A jasper date is represented as a number holding the number of MILLISECONDS from Jan 1, 1970. (java standard).
							long long t=attvalue.toLongLong();
							uint uintt=(uint)(t/1000);
							e->setDateTime(QDateTime().fromTime_t(uintt));
							//qDebug("\tdatetimeedit set to %d", uintt);
						}

					}
			} // all attr
		} //parameters element
	}


	/*
	while (!xml.atEnd())
	{
		xml.readNext();
		qDebug("JasperParamsWidget::LoadParamsFromFile: element '%s'", xml.name().toString().toLatin1().data());
		//if (xml.isStartElement())
		{
			//qDebug("JasperParamsWidget::LoadParamsFromFile: startElement %s found", xml.name().toString().toLatin1().data());
			if (xml.name()=="parameter")
			{
				QXmlStreamAttributes att=xml.attributes();
				//readElementText();
				qDebug("JasperParamsWidget::LoadParamsFromFile: parameter found with %d attributes.", att.size() );

				for (int na=0; na<att.size(); na++)
				{
					QXmlStreamAttribute a=att[na];
					QString attname=a.name().toString();
					QString attvalue=a.value().toString();
					qDebug("\tattr %s found with value %s", attname.toLatin1().data(), attvalue.toLatin1().data());
					SParam *p=NULL;
					for (int i=0; i<m_params.size(); i++)
						if (m_params[i].m_uri.section('/',-1)==attname)
						{
							qDebug("\t\tparam found !");
							QWidget *w=m_params[i].m_value_widget;
							if (QComboBox* cb = qobject_cast<QComboBox *>(w))
							{
								cb->setCurrentIndex(cb->findText(attvalue));;
							}
							else if (QCheckBox* b=qobject_cast< QCheckBox*>(w))
							{
								//if (b->isChecked()) p.second="true";
								//else p.second="false";
							}
							else if (QLineEdit* le=qobject_cast< QLineEdit*>(w))
							{
								//p.second=le->text();
							}
							else if (QDateTimeEdit* e=qobject_cast< QDateTimeEdit*>(w))
							{
								// A jasper date must be represented as a number holding the number of MILLISECONDS from Jan 1, 1970. (java standard).
								//long t=(long)(e->dateTime().toTime_t()); // num of sec
								//long long int llt=((long long)t)*1000;
								//qDebug(" %s = %ld s = %ll ms", e->dateTime().toString().toLatin1().data(), t, llt);
								//p.second=QString("%1").arg(llt);
							}

						}
				} // for all attr
			}
		} // isStartElement
	}
	*/

	if (xml.hasError())
	{
		qDebug("ParamsWidget::LoadParamsFromFile: error in xml ! cancelling.");
		f.close();
		return false;
	}

	f.close();
	return true;
}
