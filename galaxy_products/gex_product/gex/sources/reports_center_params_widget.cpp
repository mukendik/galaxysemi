#include <QVBoxLayout>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDomDocument>
#include "browser_dialog.h"
#include "engine.h"
#include <gqtl_log.h>
#include "reports_center_params_widget.h"
#include "reports_center_multifields.h"
#include "scripting_io.h"
#include "browser.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "gex_scriptengine.h"
#include "libgexpb.h"

int CReportsCenterParamsWidget::n_instances=0;
extern GexMainwindow *	pGexMainWindow;
extern GexScriptEngine* pGexScriptEngine;

CReportsCenterParamsWidget::CReportsCenterParamsWidget(QWidget *p,
                                                       QString grxml,
                                                       QMap<QString,QString> atts
                                                       )
    : QWidget(p,0), m_layout(this), m_ok_pb(this),
    //m_tpb(NULL),  m_vpm(NULL),
    m_atts(atts),
    m_grxml_filename(grxml)
{
    m_pb=NULL;
    n_instances++;
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("creating a new ParamsWidget for '%1' (total %2 instances)").
          arg(grxml).arg(n_instances).toLatin1().constData());
    //m_grt_filename=m_atts["GRT"];

    setObjectName(grxml);
    setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint ); // Qt::WindowContextHelpButtonHint
    //setWindowTitle("");
    m_tabwidget.setParent(this);
    m_tabwidget.setObjectName("m_tabwidget");

    //GSLOG(SYSLOG_SEV_DEBUG, " requesting a PropBrowser...");
    m_pb=CGexPB::CreatePropertyBrowser(pGexScriptEngine, &m_tabwidget, 20);
    if (!m_pb)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : cant create a GexPB for this Widget !");
        GEX_ASSERT(m_pb!=NULL);
    }
    else
    {
        m_tabwidget.addTab(m_pb, "Options");
    }
    m_pb->setObjectName("m_pb");

    setLayout(&m_layout);
    m_layout.setObjectName("m_layout");
    m_layout.addWidget(&m_tabwidget);


    //m_layout.addItem(new QSpacerItem(1,1));
    //m_layout.addSpacerItem(new QSpacerItem(1,1));
    //
    m_ok_pb.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_ok_pb.setText("Run report...");
    m_layout.addWidget(&m_ok_pb);

    //m_tabwidget.setContentsMargins(4,5,4,5);
    m_tabwidget.setSizePolicy(QSizePolicy::Minimum, //QSizePolicy::Fixed,	//
                              QSizePolicy::Minimum ); //QSizePolicy::Fixed);	//);

    setSizePolicy(QSizePolicy::Minimum,	//QSizePolicy::Fixed, //
                  QSizePolicy::Fixed //QSizePolicy::Minimum
                  );

    GexMainwindow::applyPalette(this);


    QObject::connect(&m_ok_pb, SIGNAL(released()), this, SLOT(OnRunReport()));

    //if (HasAtLeastOneMandatoryFieldUnknown())
    //	m_ok_pb.setEnabled(false);

}

CReportsCenterParamsWidget::~CReportsCenterParamsWidget()
{
    n_instances--;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" (still %1 instances)").
          arg(n_instances).toLatin1().constData());

    if (!CGexPB::DestroyPropertyBrowser(m_pb))
        GSLOG(SYSLOG_SEV_WARNING, "failed to destroy PropBrowser !");
    m_pb=NULL;

    QMap<QString, CReportsCenterDataset*>::iterator i=m_DatasetIDToDatasetMap.begin();
    for (i=m_DatasetIDToDatasetMap.begin(); i!=m_DatasetIDToDatasetMap.end(); i++)
    {
        if (i.value())
            delete i.value();
    }

}

bool CReportsCenterParamsWidget::IsEnabled()
{
    if (m_atts["EnableIf"]=="")
        return true;
    QScriptValue scriptValue = pGexScriptEngine->evaluate( m_atts["EnableIf"] );
    if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("cannot evaluate '%1' : '%2'")
                .arg(m_atts["EnableIf"])
                .arg(pGexScriptEngine->uncaughtException().toString())
                .toLatin1().data()
                 );
        return false;
    }
    bool b=scriptValue.toBool();
    return b;
}

bool CReportsCenterParamsWidget::HasAtLeastOneMandatoryFieldUnknown()
{
    QMap<QString, CReportsCenterDataset*>::iterator i=m_DatasetIDToDatasetMap.begin();
    for (i=m_DatasetIDToDatasetMap.begin(); i!=m_DatasetIDToDatasetMap.end(); i++)
    {
        if (!i.value())
            continue;
        CReportsCenterDataset* ds=i.value();
        if (ds->HasAtLeastOneMandatoryFieldUnknown())
            return true;
    }
    return false;
}



bool CReportsCenterParamsWidget::OnRunReport()
{
    QString of=ReportsCenterWidget::GetInstance()->GetCurrentOutputFormat();

    // save the latest to reshow it later
    ReportsCenterWidget::s_pCurrentParamsWidget=this;
    //this->hide();

    if (!m_atts["RunnableIf"].isEmpty() && pGexScriptEngine)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Run report requested on '%1' : "
                      "checking if runnable : %2").
              arg(m_atts["Title"]).arg(m_atts["RunnableIf"]).
              toLatin1().constData());

        // Let s first register this widget in our JS engine
        QScriptValue objectValue = pGexScriptEngine->newQObject( this );
        if (objectValue.isNull())
        {
            GSLOG(SYSLOG_SEV_ERROR, " ScripteEngine newQObject failed !");
            GS::Gex::Message::warning(
                "Warning",
                QString("This template is not runnable due to internal "
                        "error.\nPlease contact the support team."));
            return false;
        }

        pGexScriptEngine->globalObject().setProperty("ParamsWidget", objectValue);

        QScriptValue sv=pGexScriptEngine->evaluate(m_atts["RunnableIf"]);
        if (sv.isError() || pGexScriptEngine->hasUncaughtException() )
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Error when evaluating '%1' : '%2'")
                    .arg( m_atts["RunnableIf"] )
                    .arg(pGexScriptEngine->uncaughtException().toString())
                    .toLatin1().data()
                     );
            GS::Gex::Message::warning(
                "Warning",
                QString("This template has an incorrect "
                        "runnable condition : %1\n%2").
                arg(m_atts["RunnableIf"]).
                arg(pGexScriptEngine->uncaughtException().toString()));
            return false;
        }

        if(!sv.toBool())
        {
            GS::Gex::Message::warning(
                "Warning",
                QString("Please check that all parameters are correct before "
                        "run.\nThis template would be runnable if : %1").
                arg(m_atts["RunnableIf"]) );
            return false;
        }
    }

    QString GRTfilename;
    if (m_atts["Type"]=="AER")
    {
        GRTfilename=GenerateGRT("temp.grt");
        if (GRTfilename.startsWith("error", Qt::CaseInsensitive))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString(" error while generating GRT file : %1").arg( GRTfilename).toLatin1().constData());
            return false;
        }

        pGexMainWindow->SetWizardType(GEX_JASPER_WIZARD);
        // pGexMainWindow->iWizardType = GEX_JASPER_WIZARD;		// PYC, 27,05/2011
        QString r=pGexMainWindow->BuildReportNow(false, "home", true, of, GRTfilename);
        if (r.startsWith("error"))
        {
            GS::Gex::Message::warning("Error", r);
            return false;
        }
        else
            return true;
    }
    else
    {
        // Not grt based template but a usual csl based one.
        QString r=GenerateCSL(of);
        // Launch script!
        if (!r.startsWith("ok"))
        {
            GS::Gex::Message::warning("Error while preparing report script !", r);
            return false;
        }

        GS::Gex::CSLStatus lS=GS::Gex::CSLEngine::GetInstance()
                .RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript());
        if (lS.IsFailed())
        {
            GSLOG(4, QString("csl run failed : %1").
                  arg(lS.GetErrorMessage()).toLatin1().constData());
            GS::Gex::Message::critical("Failed to run report", lS.GetErrorMessage());
        }

        // If report created, next HTML report may not need any HTML page to rebuild...
        if (!lS.IsFailed())	//(bCreateReport)
        {
            pGexMainWindow->m_bDatasetChanged	= false;
            pGexMainWindow->iHtmlSectionsToSkip	= GEX_HTMLSECTION_ALL;
        }
    }

    return true;
}

/*
bool	CReportsCenterParamsWidget::PickValue(QComboBox *field_cb, QComboBox *value_cb, bool multiselect)
{
    if (!field_cb || !value_cb)
        return false;
    GSLOG(SYSLOG_SEV_DEBUG,(char*)QString(" on '%1'...").arg( field_cb->currentText()).toLatin1().constData());
    if (field_cb->currentText()=="")
        return false;

    return false;
}
*/
