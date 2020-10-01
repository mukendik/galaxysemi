#include <QTextDocumentWriter>
#include <QAbstractTextDocumentLayout>
#include "gex_report.h"
#include "db_engine.h"
#include <gqtl_log.h>
#include "engine.h"
#include "message.h"

int	CGexReport::ConvertHtmlToODT(void)
{
    QString			strDestination;
    QString			strHtmlReportFolder;
    QDir			cDir;
    //int				nStatus=0;

    // HTML flat file is <path>/<query_folder_name>/pages/indexf.htm,
    // output must be: <path>/<query_folder_name>.odt
    strDestination	= BuildTargetReportName(strHtmlReportFolder,".odt");
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Destination = %1").arg( strDestination).toLatin1().constData() );
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("HtmlReportFolder = %1").arg( strHtmlReportFolder).toLatin1().constData() );
    cDir.remove(strDestination);	// Make sure destination doesn't exist.

    // Header & Footer: Examinator version + URL & Lot/Product info if known.
    QString lHeader, lFooter;
    BuildHeaderFooterText(lHeader, lFooter);

    QTextDocument* td=new QTextDocument(this);
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("defaultStylesheet = %1").arg( td->defaultStyleSheet()).toLatin1().constData() ); // default is empty
    td->setDocumentMargin(0.5f); // no effect on odt export
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("doc margin = %1").arg( td->documentMargin()).toLatin1().constData());
    //td->setPageSize(QSizeF());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("size = %1 %2 page size = %3 %4")
          .arg(td->size().width())
          .arg(td->size().height())
          .arg(td->pageSize().width())
          .arg(td->pageSize().height())
          .toLatin1().constData()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL, "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL, "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL.arg( "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL, "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL.arg( "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL.arg( "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width(), td->pageSize().height()); // default is -1 -1.arg(    GSLOG(SYSLOG_SEV_INFORMATIONAL.arg( "size = %1 %2 page size = %3 %4").arg( td->size().width().arg( td->size().height().arg( td->pageSize().width().arg( td->pageSize().height()); // default is -1 -1

    QAbstractTextDocumentLayout* tdl = td->documentLayout();
    if (tdl)
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Layout : size %1 %2")
              .arg( tdl->documentSize().width())
              .arg(tdl->documentSize().height())  // default is 2.0 15.0.arg(        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Layout : size %1 %2").arg( tdl->documentSize().width().arg( tdl->documentSize().height()  // default is 2.0 15.0
              .toLatin1().constData());

    // todo :
    //QTextDocument::DocumentUrl	1	The url of the document. The loadResource() function uses this url as the base when loading relative resources.
    //td->setMetaInformation(QTextDocument::DocumentUrl, strHtmlReportFolder);

    // Let s add all images in TD as a ressource for it to be able to load.
    // in indexf.html the images are referenced as src="../images/xxxxxxx.xxx"
    /*
        QString html;
        QImage img("c:\\temp\\sample.png"); // todo: generate image in memory
        myTextEdit->document()->addResource(QTextDocument::ImageResource, QUrl("sample.png" ), img);
        html.append("<p><img src=\":sample.png\"></p>");
        myTextEdit->setHtml(html);
    */
    cDir.setPath(strHtmlReportFolder+"/images");
    cDir.setFilter(QDir::Files);
    QStringList strDataFiles = cDir.entryList(QStringList("*"));	// Files extensions to look for...: *.*
    for(QStringList::Iterator it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
    {
        if((*it == ".")  || (*it == ".."))
            continue;
        //td->loadResource() ?
        td->addResource(QTextDocument::ImageResource,
                       QUrl("../images/"+*it),
                       QVariant(QImage(strHtmlReportFolder+"/images/"+*it))
                );
    }

    // todo :
    // - fix CGexReport::WriteHistoHtmlTestInfo :
    //    - avoid creating 2 tables instead of 1 : 15% 35% 50% and using rowspan="7"
    //    - impose large chart size and put the chart inside his own table : done
    // - fix Pareto of Tests Cp/Cpk/failures/SoftBin/HardBins
    // - fix List of Individual Maps
    // - fix Soft/Hard Binning Summary

    QFile f( strHtmlReportFolder + "/pages/indexf.htm"); // reportFlatHtmlAbsPath() // strHtmlReportFolder
    if (!f.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Failed to open %1").arg( reportFlatHtmlAbsPath()).toLatin1().constData() );
        return -1;
    }
    td->setHtml(QString(f.readAll()));
    f.close();

    // Adjusts the document to a reasonable size.
    // As it sees only 1 page, the size explodes to 800,57000 and no effect on odt
    //td->adjustSize();
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Size after adjust: %1 %2").arg( td->size().width(), td->size().height());

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("BlockCount=%1 characterCount=%2 pagecount=%3")
          .arg(td->blockCount())
          .arg(td->characterCount())
          .arg(td->pageCount())
          .toLatin1().constData());

    QList<QByteArray> sdf=QTextDocumentWriter::supportedDocumentFormats();
    if (sdf.indexOf(QByteArray("ODF"))==-1)
       GSLOG(SYSLOG_SEV_WARNING, "QTextDocumentWriter::supportedDocumentFormats : odf NOT found !");
    QTextDocumentWriter writer(strDestination);
    writer.setFormat("odf");
    bool success = writer.write(td);
    if (!success)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Cannot write ODT to %1").arg( strDestination).toLatin1().constData() );
        return -2;
    }

    #ifdef QT_DEBUG
        GS::Gex::Message::information(
            "Text Document generation",
            QString("An ODT document (openable with MicrosoftWord, OpenOffice, "
                    "LibreOffice,...) has been successfuy generated in %1\n"
                    "Warning :\n- only large histogram charts size are "
                    "currently supported\n- pareto bars are WIP\n"
                    "- WaferMaps are WIP\n").arg(strDestination));
    #endif
    // Cleanup: erase the HTML folder created for the flat HTML file.
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strHtmlReportFolder);

    // Update the report file name created (from HTML file name to '.doc' file just created
    setLegacyReportName(strDestination);

    return 0;
}
