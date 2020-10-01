#include <QString>
#include <QApplication>
#include <QDir>
#include <QDomComment>
#include <QDomDocument>
#include <QTextStream>
#include <gqtl_log.h>
#include "gqtl_sysutils.h"
#include "gqtl_archivefile.h"

// using qzipwriter from Qt sources
//#include <qtbase/src/gui/text/qzipwriter_p.h> // used for example in qtextodfwriter.cpp
// or using quazip
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#ifdef QT_DEBUG
    QString AddDir(const QDir &cDir, QStringList &sl, bool recursif)
    {
        if (!cDir.exists())
            return "error : dir does not exist"+cDir.absolutePath();
        //cDir.setFilter(QDir::Files);
        QStringList strDataFiles = cDir.entryList(QStringList("*"));	// Files extensions to look for...: *.*
        for(QStringList::Iterator it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
        {
            if((*it == ".")  || (*it == ".."))
                continue;
            QFileInfo fi(cDir.absolutePath()+"/"+*it);
            if (fi.isDir())
            {
                if(recursif)
                    AddDir( QDir(cDir.absolutePath() + "/" + (*it) ), sl, true);
                continue;
            }
            //QDir::exists()
            sl.push_back(cDir.absolutePath()+"/" + *it);
        }
        return "ok";
    }

    // archive all the content of folder dir in the zip file filePath
    static QString archiveFolder(const QString & filePath, const QDir & dir, const QString & comment = QString(""))
    {
        QString r="error";
        if (QFile::exists(filePath))
            if (!QFile::remove(filePath))
                return "error : cant remove old output file";
        QuaZip zip(filePath);
        zip.setFileNameCodec("IBM866");

        if (!zip.open(QuaZip::mdCreate))
        {
            r=QString("error : zip.open(): %1").arg(zip.getZipError());
            GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
            return r;
        }

        if (!dir.exists())
        {
            r=QString("error : dir.exists(%1)=false").arg(dir.absolutePath());
            GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
            return r;
        }

        QFile inFile;

        QStringList sl;
        r=AddDir(dir, sl, true);
        if (r.startsWith("error") )
        {
            GSLOG(SYSLOG_SEV_WARNING, "Adding files in archive failed");
            return r;
        }

        QFileInfoList files;
        foreach (const QString &fn, sl)
            files << QFileInfo(fn);

        QuaZipFile outFile(&zip);

        char c;
        foreach(const QFileInfo &fileInfo, files)
        {
            if (!fileInfo.isFile())
                continue;

            // ???? ???? ? ?????????????, ?? ????????? ??? ???? ????????????? ? ?????? ??????
            // ????????: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
            // ????? ????? ???????? ????? ?????? fileNameWithSubFolders ????? ????? "Folder\123.opn" ? ?.?.
            QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

            inFile.setFileName(fileInfo.filePath());

            if (!inFile.open(QIODevice::ReadOnly))
            {
                r=QString("error : inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData());
                GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
                return r;
            }

            if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath())))
            {
                r=QString("error : outFile.open(): %1").arg(outFile.getZipError());
                GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
                return r;
            }

            while (inFile.getChar(&c) && outFile.putChar(c)) ;

            if (outFile.getZipError() != UNZ_OK)
            {
                r=QString("error : outFile.putChar(): %1").arg(outFile.getZipError());
                GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
                return r;
            }

            outFile.close();

            if (outFile.getZipError() != UNZ_OK)
            {
                r=QString("error : outFile.close(): %1").arg(outFile.getZipError());
                GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data() );
                return r;
            }

            inFile.close();
        }

        // + ???????????
        if (!comment.isEmpty())
            zip.setComment(comment);

        zip.close();

        if (zip.getZipError() != 0)
        {
            r=QString("error : zip.close(): %1").arg(zip.getZipError());
            GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data() );
            return r;
        }

        return "ok";
    }
#endif

// copy the source to the dir, recursively or not
QString CopyFolder(QString& source, QString& dest, bool &recursive)
{
    if (!QFile::exists(source))
        return "error : source dir does not exist";

    QDir cDir(dest);
    if (!cDir.mkpath( dest ) )
        return "error : canot create dest "+dest;

    //cDir.setPath(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures");
    cDir.setPath(source);
    cDir.setFilter(QDir::Files);
    QStringList strDataFiles = cDir.entryList(QStringList("*"));	// Files extensions to look for...: *.*
    for(QStringList::Iterator it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
    {
        if((*it == ".")  || (*it == ".."))
            continue;

        if (QFile::exists(dest+QDir::separator()+(*it)))
            if (!QFile::remove(dest+QDir::separator()+(*it)))
                return "error : cannot remove "+dest+"/"+(*it);

        if (!QFile::copy(source+"/"+(*it), dest+"/"+(*it)) )
            return "error : cannot copy " + source+(*it) + "to "+dest+"/"+(*it);
    }

    if (!recursive)
        return "ok";

    cDir.setFilter(QDir::Dirs);
    strDataFiles.clear();
    strDataFiles = cDir.entryList(QStringList("*"));	// Files extensions to look for...: *.*
    for(QStringList::Iterator it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
    {
        if((*it == ".")  || (*it == ".."))
            continue;
        QString s(source+"/"+(*it));
        QString d(dest+"/"+(*it));
        QString r=CopyFolder(s, d, recursive);
        if (r.startsWith("error"))
            return "error : cannot copy "+source+(*it) +" to " + dest+(*it) + " : "+r;
    }
    return "ok";
}

QString GenerateODP(QString reportFlatHtmlAbsPath, QString strDestination)
{
    // reportFlatHtmlAbsPath should be ...pages/indexf.htm
    QFileInfo fi(reportFlatHtmlAbsPath);
    QString imagespath=fi.absolutePath()+"/indexf_files"; // exple : c:/Users/will/GalaxySemi/reports/data_samples/pages/indexf_files
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Generate ODP : path=%1 dest=%2 images=%3")
          .arg(reportFlatHtmlAbsPath)
          .arg(strDestination)
          .arg(imagespath)
          .toLatin1().constData());

    QString strApplicationDir;
    if(CGexSystemUtils::GetApplicationDirectory(strApplicationDir) != CGexSystemUtils::NoError)
    {
        return "error : cant get ApplicationDirectory";
    }

    QDir d(QDir::homePath()+"/GalaxySemi/temp/odp");
    if (!d.mkpath(QDir::homePath()+"/GalaxySemi/temp/odp"))
        return "error : cannot make path "+QDir::homePath()+"/GalaxySemi/temp/odp";
    QString fr(strApplicationDir+"/samples/odp");
    QString t(QDir::homePath()+"/GalaxySemi/temp/odp");
    bool recursive=true;
    QString r=CopyFolder(fr, t, recursive);
    if (r.startsWith("error"))
        return r;

    QFile f(strApplicationDir+"/samples/odp/content_template.xml");
    if (!f.open(QIODevice::ReadOnly))
        return "errro : cant open content_template.xml";

    QDomDocument dd;
    QString errorMsg;
    bool b=dd.setContent(&f, true, &errorMsg);
    if (!b)
        return "error : setContent failed : "+errorMsg;
    f.close();
    //GSLOG(SYSLOG_SEV_NOTICE, QString("Generate ODP : doctype = %1 namespaceURI = %2").arg( dd.doctype().toText().data()).toLatin1().constData(), dd.namespaceURI()).toLatin1().constData() );.arg(    //GSLOG(SYSLOG_SEV_NOTICE, "Generate ODP : doctype = %1 namespaceURI = %2").arg( dd.doctype().toText().data()).toLatin1().constData().arg( dd.namespaceURI()).toLatin1().constData() );

    QDomNode obn;
    QDomNodeList nl=dd.elementsByTagName("office:body");
    if (nl.isEmpty())
        nl=dd.elementsByTagNameNS( "urn:oasis:names:tc:opendocument:xmlns:office:1.0", "body" ); // office:body
    if (nl.isEmpty())
    {
        obn=dd.firstChildElement("office:body"); //office:body
        if (obn.isNull())
            return "error : cant find office:body node in template";
    }
    else
        obn=nl.at(0);

    nl=obn.toElement().elementsByTagNameNS( "urn:oasis:names:tc:opendocument:xmlns:office:1.0", "presentation");
    if (nl.isEmpty())
        return "error : cant find office:presentation node in template";
    QDomNode pn=nl.at(0);   //obn.firstChildElement();    //obn.firstChildElement("presentation");
    if (pn.isNull())
        return "error : office:presentation node null";

    QDomNode template_drawpagenode;
    nl=pn.toElement().elementsByTagNameNS("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "page");
    if (nl.isEmpty())
        return "error : cant find the template page draw:page";
    template_drawpagenode=nl.at(0);
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Template page name = %1")
          .arg(template_drawpagenode.attributes().namedItemNS("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "name").nodeValue())
          .toLatin1().data());

    QString drawnsURI="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
    QString svgnsURI="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    QString xlinknsURI="http://www.w3.org/1999/xlink";

    // copy all png files from cache to temp/odp/Pictures and add an element in xml
    QDir cDir;
    //cDir.setPath(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures");
    cDir.setPath(imagespath);
    cDir.setFilter(QDir::Files);
    QStringList strDataFiles = cDir.entryList(QStringList("*.png"));	// Files extensions to look for...: *.png
    int i=1;
    for(QStringList::Iterator it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
    {
        if((*it == ".")  || (*it == ".."))
            continue;
        // copy to temp
        if (QFile::exists(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures/"+(*it)))
            QFile::remove(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures/"+(*it));
        bool b=QFile::copy(imagespath+"/"+(*it), QDir::homePath()+"/GalaxySemi/temp/odp/Pictures/"+(*it));
        if (!b)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Cannot copy %1 to .../GalaxySemi/temp/odp").arg( (*it)).toLatin1().constData() );
            continue;
        }

        QDomNode newdrawpagenode = template_drawpagenode.cloneNode(true); // deep
        newdrawpagenode.toElement().setAttributeNS(drawnsURI, "name", QString("page%1").arg(i) );
            QDomNode framedn=newdrawpagenode.toElement().elementsByTagNameNS(drawnsURI, "frame").at(0);
            framedn.toElement().removeAttribute("svg:width");
            framedn.toElement().removeAttributeNS(svgnsURI,"width");
            framedn.toElement().setAttribute("svg:width", "28cm"); //framedn.toElement().setAttributeNS(svgnsURI, "width", "28cm");
            framedn.toElement().removeAttributeNS(svgnsURI, "height");
            framedn.toElement().setAttribute("svg:height", "21cm"); //framedn.toElement().setAttributeNS(svgnsURI, "height", "21cm");
            framedn.toElement().removeAttributeNS(svgnsURI, "x");
            framedn.toElement().setAttribute("svg:x", "0cm"); //framedn.toElement().setAttributeNS(svgnsURI, "x", "0cm");
            framedn.toElement().removeAttributeNS(svgnsURI, "y");
            framedn.toElement().setAttribute("svg:y", "0cm"); //framedn.toElement().setAttributeNS(svgnsURI, "y", "0cm");
            framedn.toElement().setAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");

            QDomNode imagedn=framedn.toElement().elementsByTagNameNS("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "image").at(0);
            imagedn.toElement().removeAttributeNS(xlinknsURI, "href");
            imagedn.toElement().setAttribute("xlink:href", QString("Pictures/"+(*it)) );
            //imagedn.toElement().setAttributeNS(xlinknsURI,"href", QString("Pictures/"+(*it)) ); // .arg(i, 10, 4, QChar('0')
        pn.insertAfter(newdrawpagenode, template_drawpagenode);
        i++;
    }

    QFile ocf(QDir::homePath()+"/GalaxySemi/temp/odp/content.xml");
    if (!ocf.open(QIODevice::WriteOnly))
        return "error : cant write to .../GalaxySemi/temp/odp/content.xml";
    QTextStream ts; ts.setDevice(&ocf);
    dd.save(ts, 2);
    ocf.close();

    // CArchiveFile seems to compress in gz only ?
    //CArchiveFile af((QApplication*)QCoreApplication::instance());
    //QString d=QDir::homePath()+"/GalaxySemi/temp/odp/report.odp";
    //b=af.Compress(QDir::homePath()+"/GalaxySemi/temp/odp/content.xml", d);
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ODP compression %1").arg( b?"ok":"failed"));
    //b=af.Compress(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures/logogalaxy.gif", d);
    //b=af.Compress(QDir::homePath()+"/GalaxySemi/temp/odp/Pictures/slide0018_image001.png", d);

    // using qCompress : compresses using zlib meaning .gz only ?
    //qCompress();
    //int r=system("cd %USERPROFILE%/GalaxySemi/temp/odp & %ProgramFiles%/7-Zip/7z a %USERPROFILE%/GalaxySemi/p.odp -r -tzip Pictures content.xml META-INF/manifest.xml");
    //return QString("ok : system 7z returned %1").arg(r) ;

    // using Qt internal Zip ?
    //QFile  m_device("o.odp"); //QIODevice *m_device;
    //QOutputStrategy m_strategy=
    // cant use QZipStreamStrategy : it is internaly defined and implemented in qtextodfwriter !
    //QZipStreamStrategy m_strategy(m_device);

    #ifdef QT_DEBUG
        // Quazip
        r=archiveFolder(strDestination, QDir(QDir::homePath()+"/GalaxySemi/temp/odp"));
        if(r.startsWith("err"))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("archiveFolder failed : ").arg( r).toLatin1().constData());
            return QString("Error : the report has been written in folder homePath/galaxysemi/temp/odp \n"\
                           "but the archiver failed to create zip/odp.\n" \
                           "Use any compresser (7z,WinZip,zip,...) to compress the contents of this folder in a .zip and then rename to .odp");
        }
        return QString("ok : the report has been written in %1").arg(strDestination);
    #endif

    return QString("ok : the report has been written in homePath/galaxysemi/temp/odp\n"\
                   "Use any compresser (7z,winzip,...) to compress the contents of this folder in a .zip and then rename to .odp");
}
