#include <gqtl_log.h>
#include "reports_center_multifields.h"
#include "reports_center_params_dataset.h"
#include <QHBoxLayout>

int CReportsCenterMultiFields::n_instances=0;

CReportsCenterMultiFields::CReportsCenterMultiFields(QWidget* p,
													 CReportsCenterDataset* ds,
													 QMap<QString, QString> atts)
	: QGroupBox(p), m_pDataset(ds), m_layout(this), m_full_range(NULL), m_Attributes(atts)
{
	n_instances++;
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" (total %1 instances)").arg( n_instances));
	//m_Attributes=atts;

	if (m_Attributes["label"]=="")
		setTitle(QString("MultiFields %1").arg(n_instances));
	else
		setTitle(m_Attributes["label"]);


	//m_layout.getContentsMargins();
	m_layout.setContentsMargins(3,1,3,1);
	m_layout.setParent(this);
	m_layout.setSpacing(0);
	//setLayout(&m_layout);

	if (m_Attributes["allow_full_range_choice"]=="true")
	{
		m_full_range=new QCheckBox("Full range (all possible values will be shown)", this);
        m_full_range->setToolTip("Use of the full range option displays the full range of the time period on the aggregation even if your data is filtered to a subset of that range");
        m_layout.addWidget(m_full_range);
		if (m_Attributes["full_range"]=="true")
			m_full_range->setChecked(true);
    }

	setAlignment(Qt::AlignLeft);

	setSizePolicy(QSizePolicy::Minimum, //QSizePolicy::Fixed,	//
				  QSizePolicy::Fixed);  //QSizePolicy::Minimum);
}

CReportsCenterMultiFields::~CReportsCenterMultiFields()
{
	n_instances--;
	//GSLOG(SYSLOG_SEV_DEBUG, QString(" %1 fields to delete...").arg( m_Fields.size());
	QList<CReportsCenterField*>::iterator i=m_Fields.begin();
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{	if (*i==NULL)
			continue;
		delete (*i);
	}
	m_Fields.clear();
}

bool CReportsCenterMultiFields::HasAtLeastOneMandatoryFieldUnknown()
{
	QList<CReportsCenterField*>::iterator i=m_Fields.begin();
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{	if (*i==NULL)
			continue;
		if (!((CReportsCenterField*)*i)->IsValueMandatory())
			continue;
		if (!((CReportsCenterField*)*i)->IsValueSelected())
			return true;
	}

	return false;
}

QStringList CReportsCenterMultiFields::GetFiltersStrings()
{
	QStringList sl;
	// for all fields...
	QList<CReportsCenterField*>::iterator i=m_Fields.begin();
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{	if (*i==NULL)
			continue;
		QString f=((CReportsCenterField*)*i)->GetFilterString();
		if (!f.isEmpty())
			sl.append(f);
	}
	return sl;
}

bool CReportsCenterMultiFields::AddField(QMap<QString,QString> atts)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" add a field following '%1' atts.").arg( atts.size());

	// impose some attributes from the parent
	atts.insert("stage", m_Attributes["stage"]);
	atts.insert("binning", m_Attributes["binning"]);
	atts.insert("time", m_Attributes["time"]);
	atts.insert("orderable", m_Attributes["orderable"]);
	atts.insert("filterable", m_Attributes["filterable"]);
	atts.insert("consolidated", m_Attributes["consolidated"]);

	if (atts["extra_fields"]=="")
	 atts.insert("extra_fields", m_Attributes["extra_fields"]);

	CReportsCenterField* f=new CReportsCenterField(
				this, atts,
				((m_Attributes["indent"]=="false")?0:m_Fields.size())
							 );
	m_Fields.push_back(f);
	m_layout.addWidget(f);

	resize(sizeHint());
	return true;
}


bool CReportsCenterMultiFields::ExportToDom(QDomDocument& doc, QDomElement &de)
{
	QDomElement e=doc.createElement(m_Attributes["role"]);
	de.appendChild(e);

	if (m_full_range)
		e.setAttribute("FullRange", m_full_range->isChecked()?"true":"false");

	int j=-1;
	QList< CReportsCenterField* >::iterator i=this->m_Fields.begin();
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{
		j++;
		if (!(*i))
			continue;

		(*i)->ExportToDom(doc, e, j);
	}

	return true;
}

QString CReportsCenterMultiFields::ExportToCsl(FILE* hFile)
{
	if (!hFile)
		return "error : FILE Null !";
	QList< CReportsCenterField* >::iterator i=this->m_Fields.begin();
	QString r;
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{
		if (!(*i))
			continue;
		CReportsCenterField* f=(*i);
		r=f->ExportToCsl(hFile);
		if (r.startsWith("error"))
			return r;
	}
	return "ok";
}

bool CReportsCenterMultiFields::ReplaceBinCatInComboBox(QString bincat)
{
	QList< CReportsCenterField* >::iterator i=this->m_Fields.begin();
	for (i=m_Fields.begin(); i!=m_Fields.end(); i++)
	{
		if (!(*i))
			continue;
		(*i)->ReplaceBinCatInComboBox(bincat);
	}
	return true;
}
