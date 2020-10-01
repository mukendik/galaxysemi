//#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include <QComboBox>
#include <QScriptEngine> // to evaluate fromdate, todate,...
#include <QXmlStreamReader>

#include "jasper_params_widget.h"
#include "reports_center_widget.h"
#include "reports_center_multifields.h"
#include <gqtl_log.h>

#include "jasper_threads.h"
#include "ui_reports_center_widget.h"
#include "settings_dialog.h"
//#include "libjasper.h"
#include "browser_dialog.h"
#include "gex_web_browser.h"
#include "gex_version.h"	// for GR_VERSION...
#include "db_transactions.h"	// for GexDatabseEntry
#include "engine.h"

extern bool				FillComboBox(QComboBox * pCombo, const char * szTextTab[]);
extern const char*		gexTimePeriodChoices[];		// tables for the time period combobox

bool ReportsCenterWidget::LoadGRXML(QString filepath, ReportsCenterItem *item)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Load GRXML %1...").arg( filepath).toLatin1().constData());

    QFile f(filepath);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    //GSLOG(SYSLOG_SEV_DEBUG, QString("%1 opened...").arg( f.fileName()).toLatin1().constData());
    QXmlStreamReader xml( &f);

    QString sTitle;
    QString sDesc;

    //QVector<SGRXMLParam> params;
    bool GalaxyReportfound=false;

    float version=0.f;

    while (!xml.atEnd())
    {
        xml.readNext();
        GSLOG(SYSLOG_SEV_DEBUG, QString(" element '%1'").arg( xml.name().toString()).toLatin1().constData());
        QStringRef xmlname=xml.name();
        if (xmlname=="GalaxyReport")
        {
            GalaxyReportfound=true;
            QXmlStreamAttributes atts=xml.attributes();
            if (atts.size()==0)
                continue;
            QXmlStreamAttribute a;
            for (int i=0; i<atts.size(); i++)
            {
                a=atts[i];
                //GSLOG(SYSLOG_SEV_DEBUG, QString("%1 %2").arg( a.name().toString()).toLatin1().constData(), a.value().toString()).toLatin1().constData());.arg(                //GSLOG(SYSLOG_SEV_DEBUG, "%1 %2").arg( a.name().toString()).toLatin1().constData().arg( a.value().toString()).toLatin1().constData());
                if (a.name()=="Title")
                    sTitle=a.value().toString();
                else if (a.name()=="Description")
                    sDesc=a.value().toString();
                else if (a.name()=="Version")
                {
                    bool ok=false;
                    version=a.value().toString().toFloat(&ok);
                    if ( (!ok) || (version==0.0f) || (version<(float)GEX_MIN_GRXML_VERSION) || (version>(float)GEX_MAX_GRXML_VERSION) )
                    {
                        GSLOG(SYSLOG_SEV_NOTICE,
                              QString("GRXML version for %1 not supported "
                                      "(%2 not between %3 and %4). Ignoring.").
                              arg(f.fileName().section('/', -1)).
                              arg(version).
                              arg(GEX_MIN_GRXML_VERSION).
                              arg(GEX_MAX_GRXML_VERSION).
                              toLatin1().constData());
                        f.close();
                        return false;
                    }
                }
            }

            if (version>=0.5f)
            {
                f.close();
                //m_rc_thread->m_GRXMLtoLoad.insert(filepath, item);
                //return true;
                return LoadGRXML_v1(filepath, item);
            }
            if (version==0)
            {
                GSLOG(SYSLOG_SEV_NOTICE, "No version attribute found in this GRXML. Ignoring.");
                f.close();
                return false;
            }

            //GSLOG(SYSLOG_SEV_DEBUG, QString("GalaxyReport found : %1 %2").arg( sTitle).toLatin1().constData(), sDesc).toLatin1().constData() );.arg(            //GSLOG(SYSLOG_SEV_DEBUG, "GalaxyReport found : %1 %2").arg( sTitle).toLatin1().constData().arg( sDesc).toLatin1().constData() );
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Loading %1...").arg( sTitle ));
            QCoreApplication::processEvents();
        }
        else
        if (xmlname=="parameter")
        {
            /*
            SGRXMLParam p;
            p.m_readonly=false; p.m_SFilter=0; p.m_mandatory=false;
            QXmlStreamAttributes atts=xml.attributes();
            if (atts.size()==0)
                continue;
            QXmlStreamAttribute a;
            for (int i=0; i<atts.size(); i++)
            {
                a=atts[i];
                p.m_attributes[a.name().toString()]=a.value().toString();
                GSLOG(SYSLOG_SEV_DEBUG, QString("%1 %2").arg( a.name().toString()).toLatin1().constData(), a.value().toString()).toLatin1().constData());.arg(                GSLOG(SYSLOG_SEV_DEBUG, "%1 %2").arg( a.name().toString()).toLatin1().constData().arg( a.value().toString()).toLatin1().constData());
                if (a.name()=="name") p.m_name=a.value().toString();
                else if (a.name()=="filter") p.m_filters=a.value().toString();
                else if (a.name()=="visible") p.m_visible=(a.value().toString()=="true")?true:false;
                else if (a.name()=="mandatory") p.m_mandatory=(a.value().toString()=="true")?true:false;
                else if (a.name()=="readonly")  p.m_readonly=(a.value().toString()=="true")?true:false;
                else if (a.name()=="datatype") p.m_data_type=a.value().toString();
                else if (a.name()=="multivalue") p.m_multivalue=(a.value().toString()=="true")?true:false;
            }

            //GSLOG(SYSLOG_SEV_DEBUG, QString("\tparameter found : %1 %2").arg( p.m_name).toLatin1().constData(), p.m_attributes["defaultValueExpression"]).toLatin1().constData() );.arg(            //GSLOG(SYSLOG_SEV_DEBUG, "\tparameter found : %1 %2").arg( p.m_name).toLatin1().constData().arg( p.m_attributes["defaultValueExpression"]).toLatin1().constData() );

            params.push_back(p);
            */
        }
    }

    if (!GalaxyReportfound)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Warning : strange grxml found ! Ignoring...");
        f.close();
        return false;
    }

    f.close();

    GSLOG(SYSLOG_SEV_WARNING, QString("GRXML version %1 no more supported").
          arg(version).toLatin1().constData());
    return false;

}
