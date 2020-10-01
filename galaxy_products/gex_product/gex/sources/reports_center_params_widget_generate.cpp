#include <stdio.h>
#include <stdlib.h>
#include <QVBoxLayout>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDomDocument>

#include "browser_dialog.h"
#include <gqtl_log.h>
#include "reports_center_params_widget.h"
#include "reports_center_multifields.h"
#include "qttreepropertybrowser.h"
#include "script_wizard.h"
//#include "options_dialog.h"
#include "settings_dialog.h"
#include "db_transactions.h"
#include "report_options.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

QString CReportsCenterParamsWidget::GenerateGRT(QString outputgrtfile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" using GRT '%1'...").arg( m_atts["GRT"]).toLatin1().constData() );

    QString grt=m_grxml_filename.section('/',0,
               m_grxml_filename.count('/')-1).append("/"+m_atts["GRT"]);
    QFile grtf(grt);
    if (!grtf.exists())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" grt file unfindable '%1' ").arg(  grt).toLatin1().constData());
        return "error : grt file unfindable !";
    }

    QDomDocument doc;
    QString errorMsg; int errorLine=0;
    if (!doc.setContent(&grtf, &errorMsg, &errorLine))
    {
        grtf.close();
        GSLOG(SYSLOG_SEV_WARNING,
              QString("error : xml '%1' not compliant at line %2 : %3").
              arg(m_atts["GRT"]).arg(errorLine).arg(errorMsg).
              toLatin1().constData());
        return "error : xml not compliant !";
    }
    grtf.close();

    QDomElement gtElem = doc.firstChildElement("galaxy_template");
    if (gtElem.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : can't find at least one 'galaxy_template' element !");
        return "error : can't find at least one 'galaxy_template' element in grt !";
    }

    QDomElement  reportElem=gtElem.firstChildElement("report");
    if (reportElem.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : can't find at least one 'report' element in template !");
        return "error : can't find at least one 'report' element in template !";
    }

    QDomElement  dsElem=reportElem.firstChildElement("datasets");
    if (dsElem.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : can't find at least one 'datasets' element in template !");
        return "error : can't find at least one 'datasets' element in template !";
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" exporting %1 datasets...").
          arg(m_DatasetIDToDatasetMap.size()).toLatin1().constData());
    QMap<QString, CReportsCenterDataset*>::iterator it=m_DatasetIDToDatasetMap.begin();
    for (it=m_DatasetIDToDatasetMap.begin(); it!=m_DatasetIDToDatasetMap.end(); it++)
    {
        if (!it.value())
            continue;

        //QString dsid=it.value();

        if (!it.value()->ExportToDom(doc, dsElem))
            GSLOG(SYSLOG_SEV_NOTICE, "warning : dataset export to xml failed.");
    }

    QString outputFileName=outputgrtfile; //"temp.grt";
    QString of=QDir::homePath()+QDir::separator()+GEX_DEFAULT_DIR+QDir::separator()+"temp";
    QDir::cleanPath(of);
    QDir d(of);
    if (!d.exists())
     if (!d.mkdir(of))
        {
            GSLOG(SYSLOG_SEV_NOTICE, " error : cant create GalaxySemi dir in user folder !");
            return "error : cant create GalaxySemi dir in user folder !";
        }

    QFile outfile(of+QDir::separator()+outputFileName ); // "output.grt"

    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("error : cant open output grt file %1 !").arg(outfile.fileName()).toLatin1().constData() );
        return QString("error : cant open output grt file '%1' !").arg(outfile.fileName());
    }

    QTextStream outstream(&outfile);
    QByteArray ba=doc.toByteArray();


    //
    if (m_pb)
    {
        QMap<int, QMap< QString, QString> >::iterator oit=m_options_properties.begin();
        for (oit=m_options_properties.begin(); oit!=m_options_properties.end(); oit++)
        {
            QString ID=oit.value()["ID"];
            if (ID.isEmpty())
                continue;
            int pid=oit.key();
            if (pid<0)
                continue;
            ID.prepend("$P{"); ID.append("}");
            QVariant cv=m_pb->GetCurrentValue(pid);
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("replace property %1 : %2 with '%3'").
                  arg(pid).arg(ID).arg(cv.toString()).toLatin1().constData());

            ba.replace(ID, cv.toString().toLatin1().constData());

        }
    }

    outstream << ba;
    outfile.close();

    return outfile.fileName();
}

