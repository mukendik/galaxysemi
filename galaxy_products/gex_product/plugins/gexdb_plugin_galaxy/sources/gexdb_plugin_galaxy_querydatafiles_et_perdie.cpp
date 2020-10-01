#include <QSqlError>
#include <QDir>
#include <QDomDocument>

#include "gexdb_plugin_base.h"
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "test_filter.h"
#include "abstract_query_progress.h"
#include <gqtl_log.h>

bool RecursiveExtractColorsToZone(const QDomNode &n, QMap<QRgb, int> &color_to_site_no)
{
    if (n.isNull())
        return false;

    QDomNode c=n.firstChild();
    while (!c.isNull())
    {
        bool ok=true;
        QColor color;
        int z=-1;

        if (c.hasChildNodes())
            ok = RecursiveExtractColorsToZone(c, color_to_site_no);

        if(!ok)
            return false;

        QDomElement e=c.toElement();
        if (e.isNull())
            goto next;
        if (!e.hasAttribute("zoneid"))
            goto next;
        if (!e.hasAttribute("fill"))
            goto next;

        z=e.attribute("zoneid").toInt(&ok);
        if (!ok)
            goto next;

        color.setNamedColor(e.attribute("fill"));
        if (!color.isValid())
            goto next;

        if(color_to_site_no.contains(color.rgb())
                && (color_to_site_no[color.rgb()] != z))
        {
            GSLOG(SYSLOG_SEV_CRITICAL, (QString("FillColor %1 for ZoneId %2 was already used for ZoneId %3")
                                             .arg( color.rgb()).arg(z).arg(color_to_site_no[color.rgb()])).toLatin1().constData());
            return false;
        }
        color_to_site_no.insert(color.rgb(), z);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("%1 is zone %2").arg( color.rgb()).arg(z)).toLatin1().constData());

        next:
        c=c.nextSibling(); continue;
    }
    return true;
}

bool ExtractColorsToZoneMapFromDomDoc(QDomDocument& doc, QMap<QRgb, int> &color_to_site_no)
{
    QDomNode xmln=doc.firstChild();
    if (xmln.isNull())
        return false; // first child should be 'xml'

    QDomNode svgn=xmln.nextSibling();
    if (svgn.isNull())
        return false; // second child should be 'svg'

    return RecursiveExtractColorsToZone(svgn, color_to_site_no);
}

bool RemoveDecoratorFromDomDoc(QDomDocument& doc)
{
    //QDomElement e=doc.firstChildElement("g");
    int nr=0;
    QDomNode xmln=doc.firstChild();
    if (xmln.isNull())
        return false; // first child should be 'xml'

    QDomNode svgn=xmln.nextSibling();
    if (svgn.isNull())
        return false; // second child should be 'svg'

    QDomNode n=svgn.firstChild();
    while (!n.isNull())
    {
        QDomElement e=n.toElement();
        if (e.isNull())
        { n=n.nextSibling(); continue; }

        if (!e.hasAttribute("decoration"))
        { n=n.nextSibling(); continue; }	//{	e=e.nextSiblingElement("g"); continue; }

        if (e.attribute("decoration")!="true")
        { n=n.nextSibling(); continue; }	// { e=e.nextSiblingElement("g"); continue; }

        svgn.removeChild(n);
        n=svgn.firstChild();
        nr++;
        //e=doc.firstChildElement("g");
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 decorators removed.").arg( nr).toLatin1().constData());
    return true;
}

QString PreComputeSvg(const QString &svgfile, QMap<QRgb, int> &color_to_site_no, QString &outputsvg)
{
    QString waferzonefile=QDir::cleanPath( svgfile );
    //QDir d; waferzonefile=d.absoluteFilePath(waferzonefile);
    QFileInfo fi(waferzonefile);
    waferzonefile= fi.absoluteFilePath();
    if (!QFile::exists( waferzonefile ))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("the file '%1' does not exist").arg( waferzonefile).toLatin1().constData() );
        return QString("error : waferzone file '%1' does not exist !").arg(waferzonefile);
    }

    //m_pQueryProgressDlg->m_pLogTE->append(QString("using waferzone file '%1'").arg(waferzonefile));
    QFile f(waferzonefile);
    if (!f.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("the file '%1' cant be opened ! Check you are allowed to read in this folder. Will use the config in DB...")
              .arg(waferzonefile).toLatin1().data() );
        return "error : waferzone file cant be opened !";
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("successfully opened '%1'").arg( waferzonefile).toLatin1().constData());
    QDomDocument doc("svg");
    QString errorMsg; int errorLine=0;
    if (!doc.setContent(&f, &errorMsg, &errorLine))
    {
         f.close();
         GSLOG(SYSLOG_SEV_ERROR, (QString("xml '%1' not compliant at line %2 : %3")
                  .arg(waferzonefile.toLatin1().data())
                  .arg(errorLine)
                  .arg(errorMsg)).toLatin1().constData());
         waferzonefile.clear();
         return "error : xml not compliant !";
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("successfully created QDomDocument from %1").arg( waferzonefile).toLatin1().constData());
    if (!RemoveDecoratorFromDomDoc(doc))
        return "error : cant remove decorator from svg DomDocument !";

    //QMap<QRgb, int> color_to_site_no;
    if (!ExtractColorsToZoneMapFromDomDoc(doc, color_to_site_no))
        return QString("error : can't extract colors to zones number from svg dom doc !");

    QByteArray ba=doc.toByteArray();

    //QString tmpsvg=QDir::tempPath()+QDir::separator()+"tmp.svg";
    QDir d;
    if (!d.mkpath(QDir::homePath() + QDir::separator() + "GalaxySemi" + QDir::separator()+"temp"))
        GSLOG(SYSLOG_SEV_ERROR, QString("can't create path %1").arg( d.absolutePath()).toLatin1().constData());

    QString tmpsvg=QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp"+QDir::separator()+"tmp.svg";

    QFile tmp(tmpsvg);
    if (!tmp.open(QIODevice::WriteOnly))
    {
        return QString("error : cant open temp file %1 !").arg(tmpsvg);
    }

    if (tmp.write(ba)==-1)
        return QString("error : cant write into generated waferzone temp file %1 !").arg(tmpsvg);

    tmp.close();

    f.close();

    outputsvg=tmpsvg;

    return "ok";
}


QString
GexDbPlugin_Galaxy::
QueryDataFilesForETestPerDie(GexDbPlugin_Filter& cFilters,
              GexDbPlugin_Galaxy_TestFilter& clTestFilter,
              tdGexDbPluginDataFileList& cMatchingFiles,
              const QString& /*strDatabasePhysicalPath*/,
              const QString& strLocalDir,
              bool* /*pbFilesCreatedInFinalLocation*/,
              GexDbPlugin_Base::StatsSource /*eStatsSource*/)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("for tests %1").arg( clTestFilter.getFullTestNblist()).toLatin1().constData());

    if (mQueryProgress)
    {
        mQueryProgress->ClearLogs();
        mQueryProgress->Start(1);
#ifdef QT_DEBUG
        mQueryProgress->SetAutoClose(false);
#endif
    }

    QString waferzonefile;
    QString desired_et_notch="from_db";
    QString desired_ws_notch="from_db";
    QString exclude_wafers_not_available_at;

    QList<QList <QString> >::iterator i;
    for (i=cFilters.m_gexQueries.begin(); i!=cFilters.m_gexQueries.end(); i++)
    {
        QList<QString> l=*i;
        if (l.first()=="db_waferzonesfile")
        {
            if (l.size()>1)
                waferzonefile=l.at(1);
            else
                GSLOG(SYSLOG_SEV_WARNING, " gexQuery db_waferzonesfile command incomplete : no waferzone file specified !");
            continue;
        }

        if (l.first()=="db_et_notch")
        {
            if (l.size()>1)
                desired_et_notch=l.at(1);
            continue;
        }

        if (l.first()=="db_ws_notch")
        {
            if (l.size()>1)
                desired_ws_notch=l.at(1);
            continue;
        }

        if (l.first()=="db_exclude_wafers_not_available_at")
        {
            exclude_wafers_not_available_at=l.at(1); // Should be probably 'Wafer Sort'
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Desired ETest notch is '%1'").arg( desired_et_notch).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Desired WS notch is '%1'").arg( desired_ws_notch).toLatin1().constData());
    //m_pQueryProgressDlg->m_pLogTE->append(QString("desired ETest notch '%1'").arg(desired_et_notch));

    QMap<QRgb, int> color_to_site_no;
    QString generated_svgfile;

    if (waferzonefile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "no db_waferzonesfile specified/found in csl gexQuery() commands : will use the one in DB");
        //return "error : no waferzones file specified !";
    }
    else
    {
        QString r=PreComputeSvg(waferzonefile, color_to_site_no, generated_svgfile);
        if (r.startsWith("error"))
            GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data());
    }

    SetTestingStage(eElectTest);

    // Construct SQL query
    QString strQuery;
    //unsigned int	uiGexDbBuild=0;

    strQuery.clear();
    Query_Empty();
    ConstructSplitlotQuery_Fields();
    m_strlQuery_OrderFields.append(m_strTablePrefix + "splitlot.wafer_nb");
    Query_AddFilters(cFilters);
    Query_AddTimePeriodCondition(cFilters);
    Query_BuildSqlString(strQuery);

    // Execute splitlot query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
    {
        // Following line is not executed with MySQL DB, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
        // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
        clGexDbQuery.setForwardOnly(true);
    }

    //GSLOG(SYSLOG_SEV_DEBUG, strQuery.replace('\n',' ').toLatin1().data());

    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return "error : cant exec ET splitlot query";
    }

    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("m_strTestingStage = %1").arg( m_strTestingStage).toLatin1().constData() ));

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 E-Test splitlots/wafers to handle")
          .arg( clGexDbQuery.size()).toLatin1().constData());
    if (mQueryProgress)
        mQueryProgress->AddLog(
                    QString("%1 E-Test wafers to extract...").arg(clGexDbQuery.size()) );

    GexDbPlugin_Galaxy_SplitlotInfo	clETestSplitlotInfo;
    GexDbPlugin_DataFile			*pStdfFile;
    int								total_extracted_size=0; // sum of all stdf file size

    while(clGexDbQuery.Next())    // for each ET splitlots i.e. wafers
    {
        FillSplitlotInfo(clGexDbQuery, &clETestSplitlotInfo, cFilters);
        QString lotid=clETestSplitlotInfo.m_strLotID;

        QString m=QString("Computing wafer '%1' (%2) from Lot '%3' (eTest splitlot '%4') config '%5'...")
                  .arg(clETestSplitlotInfo.m_strWaferID)
                  .arg(clETestSplitlotInfo.m_nWaferNb)
                  .arg(clETestSplitlotInfo.m_strLotID)
                  .arg(clETestSplitlotInfo.m_lSplitlotID)
                  .arg(clETestSplitlotInfo.m_strEtestSiteConfig.toLower());
        if (mQueryProgress)
            mQueryProgress->AddLog(m);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, m.toLatin1().data() );

        /*
        QString siteconf;

        QString r=GetETestSiteConfig(clETestSplitlotInfo.m_strLotID, clETestSplitlotInfo.m_nWaferNb, siteconf);
        if (r.startsWith("error"))
        {
            GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data());
            //if (generated_svgfile.isEmpty())
            return QString("can't retrieve site config : %1").arg(r);
        }
        */
        QString svg_template=QDir::cleanPath(
                                m_strApplicationPath +
                                QDir::separator() +
                                QString("samples/waferzones/%1.svg").
                                    arg(clETestSplitlotInfo.m_strEtestSiteConfig.toLower()));
        QString r=PreComputeSvg(svg_template, color_to_site_no, generated_svgfile);
        if (r.startsWith("error"))
            return QString("error computing site config image : %1").arg(r);

        QString TLID;
        r=GetTrackingLotID(lotid, m_strTestingStage, TLID);
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              (QString("GetTrackingLotID(...) : %1  : TLID = %2")
              .arg( r)
              .arg(TLID)).toLatin1().constData() );
        if (r.startsWith("error"))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("error : cant retrieve TrackingLotID : %1 !").arg( r).toLatin1().constData());
            return "error : cant retrieve TrackingLotID for given ET LotID !";
        }

        QString WLID;
        QString WWID;

        r=GetMatchingLotWafer(TLID, clETestSplitlotInfo.m_nWaferNb, GEXDB_PLUGIN_GALAXY_WTEST, WLID,WWID);
        if (r.startsWith("error"))
        {
            if (exclude_wafers_not_available_at=="Wafer Sort")
            {

                QString m=QString("Ignoring bachelor wafer %1 (not available at WaferSort)...").arg(clETestSplitlotInfo.m_nWaferNb);
                GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data() );
                if (mQueryProgress)
                    mQueryProgress->AddLog(m);
                SetTestingStage(eElectTest); // because GetMatchingLotWafer(...) has changed it !
                continue;
            }
            QString m=QString("error : cannot retrieve corresponding WS LotWafer for TrackingLot %1 WaferID %2 : %3 ")
                      .arg(TLID).arg(clETestSplitlotInfo.m_strWaferID).arg(r);
            GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().data() );
            GSET_ERROR0(GexDbPlugin_Base, eDB_ExtractionError, NULL);
            return m;
        }

        if (clETestSplitlotInfo.m_strWaferID.toInt()!=WWID.toInt())
        {
            QString m=QString(" found WS WaferID (%1) is probably NOT equal to ETest WaferID (%2) !")
                      .arg(WWID).arg(clETestSplitlotInfo.m_strWaferID);
            GSLOG(SYSLOG_SEV_NOTICE, m.toLatin1().constData() );
            if (mQueryProgress)
                mQueryProgress->AddLog(m);
        }

        GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("WS LotID=%1 WS WaferID=%2")
              .arg( WLID)
              .arg(WWID)).toLatin1().constData() );
        if (mQueryProgress)
            mQueryProgress->AddLog(QString("WaferSort LotID=%1 & WaferSort WaferID=%2")
                                   .arg(WLID).arg(WWID)
                                   );

        // Here it is we have the WS Lot and WaferID
        // Let s enum the splitlots

        QMap< QPair<int, int>, QMap<QString, QVariant> > xys;

        if (!GetDistinctXY(WLID, WWID, cFilters, xys))
        {
            QString lError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            return QString("error : GetDistinctXY() failed : %1").arg(lError);
        }

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 distinct XY found").arg( xys.size() ).toLatin1().constData());

        QString	strStdfFilename;

        r=CreateStdfFileForETestDieGranularity(
              clETestSplitlotInfo, clTestFilter, strLocalDir, strStdfFilename,
              WWID, WLID, xys,
              generated_svgfile,
              desired_et_notch, desired_ws_notch,
              color_to_site_no, cFilters);

        if (r.startsWith("error"))
        {
            GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data());
            return r;
        }

        // STDF file successfully created
        pStdfFile = new GexDbPlugin_DataFile;
        pStdfFile->m_strFileName = strStdfFilename;
        pStdfFile->m_strFilePath = strLocalDir;
        pStdfFile->m_bRemoteFile = false;

        QString	strStdfFullFileName = strLocalDir + strStdfFilename;
        if (QFile::exists(strStdfFullFileName))
        {
            QFile f(strStdfFullFileName);
            total_extracted_size+=f.size();
        }
        // Append to list
        cMatchingFiles.append(pStdfFile);
    }

    if(cMatchingFiles.count() < 1)
    {
        QString lError = "error : no matching ET splitlots found ! Probably an error at insertion time.";
        if (mQueryProgress)
        {
            mQueryProgress->AddLog(lError);
        }
        GSLOG(SYSLOG_SEV_ERROR, QString("error : 0 corresponding ET splitlots found !").toLatin1().constData());
        GSET_ERROR1(GexDbPlugin_Base, eDB_ExtractionError, NULL, "Incorrect number of ET splitlots found (should be 1)");
        return lError;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "ok");

    return "ok";
}
