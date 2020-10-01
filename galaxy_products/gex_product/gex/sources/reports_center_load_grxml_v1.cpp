#include <QFile>
#include <QDomDocument>
#include <QDomNodeList>
#include <gqtl_log.h>
#include "reports_center_widget.h"
#include "reports_center_multifields.h"
#include "reports_center_params_widget.h"

bool ReportsCenterWidget::LoadGRXML_v1(QString filepath, ReportsCenterItem *item)
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("load %1...").arg( filepath).toLatin1().constData());

	QFile f(filepath);
	if (!f.open(QIODevice::ReadOnly))
	{
		GSLOG(SYSLOG_SEV_WARNING, QString("cant open %1 !").arg( filepath).toLatin1().constData());
		return false;
	}

	QString path=item->m_fullpath+"/"+filepath.section('/',-1);

	if (s_ReportsCenterParamsWidgets.find(filepath)!=s_ReportsCenterParamsWidgets.end())
	{
		GSLOG(SYSLOG_SEV_NOTICE, QString("template '%1' already inserted !").arg( filepath).toLatin1().constData() );
		return false;
	}

	QDomDocument doc("grxml");
	QString errorMsg; int errorLine=0;
	if (!doc.setContent(&f, &errorMsg, &errorLine))
	{
		 f.close();
         GSLOG(SYSLOG_SEV_WARNING,
               QString("xml '%1' not compliant at line %2 : %3").arg(filepath).
               arg(errorLine).arg(errorMsg).toLatin1().constData());
		 return false;
	}
	f.close();

	//QDomElement docElem = doc.documentElement();
	QDomElement grElem = doc.firstChildElement("GalaxyReport");
	if (grElem.isNull())
	{
		GSLOG(SYSLOG_SEV_WARNING, "cant find at least one GalaxyReport element !");
		return false;
	}

	QMap<QString, QString> atts;
	QDomNamedNodeMap dnl=grElem.attributes();
	for (int i=0; i<dnl.size(); i++)
		atts.insert(dnl.item(i).nodeName(), dnl.item(i).nodeValue());

	QString sTitle=grElem.attribute("Title");
	if (sTitle.isEmpty())
	{
		GSLOG(SYSLOG_SEV_WARNING, "no Title found for this GRXML !");
		return false;
	}

	if (atts["Type"].isEmpty())
	{
		GSLOG(SYSLOG_SEV_WARNING, "no 'Type' found in GalaxyReport Element !");
		return false;
	}

	QString grt=grElem.attribute("GRT");
	if (grt.isEmpty() && ( grElem.attribute("Type")=="AER"))
	{
		GSLOG(SYSLOG_SEV_WARNING, "no 'GRT' target found for this AER type GRXML !");
		return false;
	}

	/*
	// Will be done later...
	if (!QFile::exists(grt))
	{
		GSLOG(SYSLOG_SEV_WARNING, (char*)QString("error : GRT file unfindable for this GRXML : '%1' !").arg(
			   filepath.section('/',-1).append(grt).toLatin1().data() );
		return false;
	}
	*/

	QString sDesc=grElem.attribute("Description");
	QList<QVariant> data; data<<sTitle<<sDesc;

	ReportsCenterItem* newitem=new ReportsCenterItem( data, item, filepath, "GalaxyReport" ); // or path ?
	item->appendChild(newitem);

    // since Qt5, if no parent, widget wont be auto deleted at app exit ?
    // if parenting to this, GUI is all dirty
    CReportsCenterParamsWidget *w=new CReportsCenterParamsWidget(0, filepath, atts );
    //QObject::connect(qApp, SIGNAL(aboutToQuit()), w, SLOT(Close()) ); // nor really usefull but anyway
    if (qApp)
    {
        QObject::connect(qApp, SIGNAL(aboutToQuit()), w, SLOT(deleteLater()) ); // does not work
        QObject::connect(qApp, SIGNAL(destroyed()), w, SLOT(deleteLater()) ); // does not work
    }

    w->setWindowTitle(sTitle);

	for(QDomNode n = grElem.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		if (n.nodeName()=="dataset")
		{
			QDomNamedNodeMap atts=n.attributes();
			if ( (!atts.contains("ID")) || (!atts.contains("stage")) )
			{
				GSLOG(SYSLOG_SEV_NOTICE, "Warning : dataset without 'ID' and/or 'stage' found. Ignoring.");
				continue;
			}

			QMap<QString, QString> ds_atts_map;
			for (int i=0; i<atts.size(); i++)
				ds_atts_map.insert(atts.item(i).nodeName(), atts.item(i).nodeValue());

			if (ds_atts_map["ID"]=="")
			{
				GSLOG(SYSLOG_SEV_NOTICE, "Warning : dataset without or bad ID found. Ignoring.");
				continue;
			}

			if (!w->AddDataSet(ds_atts_map))
			{
				GSLOG(SYSLOG_SEV_NOTICE, QString("Warning : dataset '%1' parsing failed. Ignoring.").arg( ds_atts_map["ID"]).toLatin1().constData());
				continue;
			}

			for (QDomNode dsc = n.firstChild(); !dsc.isNull(); dsc = dsc.nextSibling())
			{
				if (dsc.nodeName()=="multifield")
				{
                    GSLOG(SYSLOG_SEV_DEBUG, QString(" '%1' : %2 atts...").
                          arg(dsc.nodeName()).
                          arg(dsc.attributes().size()).toLatin1().constData());

					if (dsc.attributes().size()<1)
					{
						GSLOG(SYSLOG_SEV_NOTICE, " warning : incomplete multifield found in this GRXML. Ignoring.");
						continue;
					}

					QDomNamedNodeMap mf_atts=dsc.attributes();
					QMap<QString, QString> mf_atts_map;
					for (int i=0; i<mf_atts.size(); i++)
						mf_atts_map.insert(mf_atts.item(i).nodeName(), mf_atts.item(i).nodeValue());

					QVector< QMap<QString, QString> > children;

					for (QDomNode mfc = dsc.firstChild(); !mfc.isNull(); mfc = mfc.nextSibling())
					{
						if (mfc.nodeName()!="field")
							continue;
						QDomNamedNodeMap f_atts=mfc.attributes();
						QMap<QString, QString> f_atts_map;
						for (int j=0; j<f_atts.size(); j++)
							f_atts_map.insert(f_atts.item(j).nodeName(), f_atts.item(j).nodeValue());
						children.push_back(f_atts_map);
					}

					w->AddMultiFields(mf_atts_map, ds_atts_map.value("ID"), children);

				}

			}
		}
		else if (n.nodeName()=="options")
		{
			for (QDomNode dsc = n.firstChild(); !dsc.isNull(); dsc = dsc.nextSibling())
			{
				if (dsc.nodeName()=="option")
				{
					QMap<QString, QString> a;
					for (int i=0; i<dsc.attributes().size(); i++)
						a.insert(dsc.attributes().item(i).nodeName(), dsc.attributes().item(i).nodeValue());

					if (!w->AddOptionToPropBrowser(a))
						GSLOG(SYSLOG_SEV_NOTICE, QString("failed to add option '%1'").arg( a["label"]).toLatin1().constData());
				}
			}
		}

	}

	w->CheckForMandatoryValues();

	w->resize(w->sizeHint());

	// For example :
	// path = //WaferSort/yield_trend.grxml
	// filepath = C:/Program Files/GalaxyExaminator/gamaxy_reports/WaferSort/yield_trend.grxml
	//s_ReportsCenterParamsWidgets[path]=w;
	s_ReportsCenterParamsWidgets[filepath]=w;

	return true;
}
