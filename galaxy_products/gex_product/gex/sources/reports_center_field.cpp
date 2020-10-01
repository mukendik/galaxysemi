#include <gqtl_log.h>
#include "reports_center_multifields.h"
#include "reports_center_params_widget.h"
#include "reports_center_params_dataset.h"
#include "reports_center_widget.h"
#include "gexdb_plugin_base.h"

int CReportsCenterField::n_instances=0;

CReportsCenterField::CReportsCenterField(QWidget* p, QMap<QString, QString> atts, int indent)
    : QWidget(p),
    m_layout(this), m_field(this), m_value(this),  m_operator(this) // m_spacer(this),
{
    n_instances++;
    m_atts=atts;
    m_layout.setContentsMargins(2,0,2,0);
    setLayout(&m_layout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //
    m_indent.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (indent==0)
        m_indent.hide();
    else
    {	for (int i=0; i<indent;i++)
            m_indent.setText(m_indent.text().append(' ')); // // 0x21b3=flecheretourdroit
    }
    m_layout.addWidget(&m_indent);

    m_layout.addWidget(&m_field);

    // the field combobox
    // ConsolidatedType = Y N or *

    if (!ReportsCenterWidget::GetInstance()->InsertFieldsIntoComboBox(
        &m_field, m_atts["stage"], m_atts["binning"],"*", m_atts["time"],
        m_atts["consolidated"]=="false"?"*":"Y" )
        )
    {
        GSLOG(4, QString("InsertFieldsIntoComboBox failed: stage='%1' bin='%2' time='%3' conso='%4' !")
                 .arg(m_atts["stage"])
                 .arg(m_atts["binning"])
                 .arg(m_atts["time"])
                 .arg(m_atts["consolidated"])
                .toLatin1().data()
                 );
        //m_field.insertItem("(RDB error)", 0);
        m_field.setEnabled(false);
        m_atts.insert("field_mandatory", "false");
        m_atts.insert("default_field", "");
        m_atts.insert("field_readonly","true");
        m_atts.insert("filterable", "true");
        m_atts.insert("value_mandatory","true");
        //return;
    }

    if (m_atts["field_mandatory"]=="false")
    {
        m_field.insertItem(0, SELECT_FIELD_INVIT);	//SELECT_GROUPBY_INVIT);
        m_field.setCurrentIndex(0);
    }

    if (m_atts["default_field"]!="")
    {
        int j=m_field.findText(m_atts["default_field"]);
        if (j!=-1)
            m_field.setCurrentIndex(j);
        else // insert anyway in the combobox even if it is not listed by DB
        {	m_field.insertItem(0, m_atts["default_field"]);
            m_field.setCurrentIndex(0);
        }
    }

    if ( (m_atts["extra_fields"]!="") )
    {
        m_field.insertItems(m_field.count(), m_atts["extra_fields"].split('|') );
    }

    if (m_atts["field_readonly"]=="true")
        m_field.setEnabled(false);
    QObject::connect(&m_field, SIGNAL(currentIndexChanged(QString)), this,SLOT(SlotFieldChanged(QString)) );


    //
    if (m_atts["orderable"]=="true")
    {
        m_orderdir.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        if (m_atts["orderdir"]=="asc")
            m_orderdir.setText(QChar(0x2191)); //2191:Up  // 0x2193=down //0x21E7=FatUp  0x21E9=FatDown // 0x21c8=doubleUp
        else
            m_orderdir.setText(QChar(0x2193));
        //m_orderdir.c	//setOpenExternalLinks();
        m_layout.addWidget(&m_orderdir);
        //m_orderdir.setTextInteractionFlags();
        QObject::connect(&m_orderdir, SIGNAL(clicked()), this, SLOT(OnOrderClicked()) );
        //m_orderdir.setMouseTracking(true);

    }
    else
    {	m_orderdir.hide();
    }

    if (m_atts["filterable"]!="false")
    {
        if (m_atts["operator"]=="")
            m_operator.setText("=");
        else
            m_operator.setText(m_atts["operator"]);
        m_operator.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_operator.show();
        m_layout.addWidget(&m_operator);

        if (m_atts["value_readonly"]=="false")
            m_value.setEnabled(true);
        else
            m_value.setEnabled(false);

        m_value.setEditable(true);
        if (m_atts["value_editable"]=="false")
            m_value.setEditable(false);

        //m_value.setSizePolicy(	QSizePolicy::Maximum,	//Maximum, //Minimum, //Maximum, //Expanding, //MinimumExpanding	//
        //					QSizePolicy::Fixed);

        if (m_atts["value"]!="")
            m_value.insertItem(0, m_atts["value"]);
        if ( (m_atts["value_mandatory"]=="false") && (m_atts["value"]=="") )
            m_value.insertItem(0, "*");

        if (m_atts["value_mandatory"]=="true")
        {
            //m_value.setBackgroundColor(QColor(Qt::red)); // does not work ?
            QPalette palette;
            palette.setColor(m_value.backgroundRole(), Qt::red);
            palette.setColor(m_value.foregroundRole(), Qt::red);
            m_value.setPalette(palette);
            this->setPalette(palette);
        }

        m_value.show();
        m_layout.addWidget(&m_value);

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/gex/icons/combo.png"), QSize(), QIcon::Normal, QIcon::On);
        icon1.addFile(QString::fromUtf8(":/gex/icons/combo.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_pickbutton.setParent(this);
        m_pickbutton.setIcon(icon1);
        m_pickbutton.setAutoRaise(true);
        m_layout.addWidget(&m_pickbutton);
        if (m_atts["value_readonly"]=="true")
            m_pickbutton.setEnabled(false);
        QObject::connect(&m_pickbutton,	SIGNAL(clicked()),	this, SLOT(OnPickFilter()) );
    }
    else
    {
        m_operator.hide();
        m_value.hide();
        m_pickbutton.hide();
    }


}

CReportsCenterField::~CReportsCenterField()
{
    n_instances--;
}

void CReportsCenterField::Hide()
{
    m_field.hide(); m_orderdir.hide(); m_operator.hide(); m_value.hide(); m_pickbutton.hide();
}

void CReportsCenterField::Show()
{
    m_field.show(); m_orderdir.show();
    m_operator.show(); m_value.show();
    if (pick_enabled) m_pickbutton.show();
}

void CReportsCenterField::SlotFieldChanged(QString newf)
{
    GSLOG(7, QString("Field changed: %1").arg(newf).toLatin1().data());
    //GSLOG(SYSLOG_SEV_DEBUG,(char*)QString(" %1").arg( newf).toLatin1().constData());
    m_value.clear();
    if (m_atts["multivalue"]=="true")
        m_value.addItem("*");

    CReportsCenterMultiFields* mf=(CReportsCenterMultiFields*)this->parent();
    if (!mf)
    {
        GSLOG(4, "This field has no multifield parent !");
        return;
    }

    // let s add another field if this one is the last
    if (mf->IsLastField(this))
    {
        if (mf->IsAutoRepeat())
            if (!mf->AddField(m_atts))
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("can't add a new field to this multifield !").
                      toLatin1().constData());
    }

}

bool CReportsCenterField::ExportToDom(QDomDocument& doc, QDomElement &de, int index)
{
    if ( (m_field.currentText()==SELECT_FIELD_INVIT)
        //|| (m_value.currentText()=="*")
        //|| (m_value.currentText()=="")
        )
    return false;

    QDomElement e=doc.createElement("field");

    e.setAttribute("index", index);
    e.setAttribute("label", m_field.currentText() );
    e.setAttribute("operator", this->m_operator.text() );
    e.setAttribute("value", m_value.currentText().replace(',','|') );
    if(!m_value.itemData(m_value.currentIndex()).isNull())
        e.setAttribute("additional_data", m_value.itemData(m_value.currentIndex()).toString() );
    if (m_atts["orderable"]=="true")
     e.setAttribute("order", (m_orderdir.text()==QChar(0x2191))?"asc":"desc");
    de.appendChild(e);

    return true;
}

QString CReportsCenterField::ExportToCsl(FILE* hFile)
{
    if (!hFile)
        return "error : FILE Null !";

    if (m_field.currentText()==SELECT_FIELD_INVIT)
        return "ok";

    QString csl_export=m_atts["csl_export"];
    if (csl_export.isEmpty())
    {
        CReportsCenterMultiFields* mf=(CReportsCenterMultiFields*)parent();
        if (mf)
         csl_export=mf->GetAttribute("csl_export");
    }

    if (csl_export!="")
    {
        // probably csl_export = gexQuery('db_split','dbf_sql','%1');
        QString d(m_atts["csl_export"]);
        d=d.arg(m_field.currentText());
        d.prepend('\t');
        d.append('\n');
                fprintf(hFile, "%s", d.toLatin1().data() );
    }

    // FILTERS
    if ( (this->m_value.currentText()=="") || (this->m_value.currentText()=="*") )
        return "ok";

    // bin number are not gexQuery('dbf_sql'...) but gexQuery('db_data','hbins','0-5');

    if (m_field.currentText()=="HBin#")
    {
        fprintf(hFile,"\tgexQuery('db_data','hbins','%s');\n",
            this->m_value.currentText().toLatin1().data() );
        return "ok";
    }

    if (m_field.currentText()=="SBin#")
    {
        fprintf(hFile,"\tgexQuery('db_data','bins','%s');\n",
                this->m_value.currentText().toLatin1().data() );
        return "ok";
    }

    if (m_field.currentText().toLower()=="test#")
    {
        fprintf(hFile,"\tgexQuery('db_testlist','%s');\n",
                    this->m_value.currentText().toLatin1().data() );
        return "ok";
    }

    QString filter_value = this->m_value.currentText();

    fprintf(hFile,"\tgexQuery('dbf_sql','%s','%s');\n",
            this->m_field.currentText().toLatin1().data(),
            filter_value.toLatin1().data() );

    return "ok";
}

QString CReportsCenterField::GetFilterString()
{
    if ( (m_field.currentText() == SELECT_FIELD_INVIT)
        || (m_value.currentText() == "*")
        || (m_value.currentText() == "") )
        return QString("");
    QString lOperator("=");
    if (m_operator.text() == "!=")
        lOperator = "!=";
    return QString("%1%2%3").arg(m_field.currentText()).arg(lOperator).arg(m_value.currentText());
}

void	CReportsCenterField::OnPickFilter()
{
    if (!QObject::sender())
    {
        GSLOG(3, " unable to retrieve signal sender !");
        return;
    }
    QToolButton* tb=(QToolButton*)QObject::sender();
    CReportsCenterField *lCurrentFieldReport=(CReportsCenterField*)tb->parent();
    GSLOG(6, QString("Pick for '%1'...").arg(lCurrentFieldReport->m_field.currentText()).toLatin1().data());

    CReportsCenterMultiFields* mf=(CReportsCenterMultiFields*)lCurrentFieldReport->parent();
    if (!mf)
    {
        GSLOG(4, " this field has no multifield parent !");
        return;
    }

    CReportsCenterDataset* ds=mf->GetDataset();
    if (!ds)
    {
        GSLOG(4, " this field has no dataset grandparent !");
        return;
    }

    QString lCurrentValue=lCurrentFieldReport->m_value.currentText();
    lCurrentFieldReport->m_value.setItemText(lCurrentFieldReport->m_value.currentIndex(), "");
    if (!ds->PickDistinct(&(lCurrentFieldReport->m_field),
                          &(lCurrentFieldReport->m_value),
                          ((mf->GetAttribute("multivalue")=="true")?true:false)))
    {
        GSLOG(5, " warning : DataSet PickDistinct() failed.");
        // return to the old value because the pick failed
        lCurrentFieldReport->m_value.setItemText(lCurrentFieldReport->m_value.currentIndex(), lCurrentValue);
    }

    // the selection was successfull
    if ((lCurrentFieldReport->m_value.currentText() != "*") && (lCurrentFieldReport->m_value.currentText() != ""))
    {
        // let s add another field if this one is the last
        if ( (mf->IsAutoRepeat()) && (mf->IsLastField(this) ) )
        {
            if (!mf->AddField(m_atts))
                GSLOG(4, "can't add a new field to this multifield !");
        }
    }

    CReportsCenterParamsWidget* pw=(CReportsCenterParamsWidget*) ds->parent();
    if (pw)
        pw->CheckForMandatoryValues();

    return;
}

bool CReportsCenterField::ReplaceBinCatInComboBox(QString bincat)
{

    if (m_field.currentText().contains("bin#", Qt::CaseInsensitive))
    {
        m_value.clear();
        m_value.setCurrentText("1");
    }

    for (int j=0; j<m_field.count(); j++)
    {
        if (m_field.itemText(j).contains("bin P/F status", Qt::CaseInsensitive))
        {
            m_field.setItemText(j,bincat+"bin P/F status");
            continue;
        }
        if (m_field.itemText(j).contains("bin#", Qt::CaseInsensitive))
        {
            m_field.setItemText(j,bincat+"bin#");
            continue;
        }

    }
    return true;
}
