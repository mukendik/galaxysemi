#include <QSqlError>
#include <QDir>

#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "abstract_query_progress.h"

extern QString ConvertSvgToQImageUsingQSvgLib(QString svgfilepath, QImage& i);

QString
GexDbPlugin_Galaxy::
InitMapsForETestDieGranularity(const GexDbPlugin_Galaxy_SplitlotInfo& clETSplitlotInfo,
                               const QString& WSWaferID,
                               const QString& WSLotID,
                               const QMap< QPair<int, int>, QMap<QString, QVariant> > &xys,
//                               const QList< QPair<int, int> >& xys,
                               const QString &waferzonefilepath,
                               int rotation_in_degree,
                               const QMap<QRgb, int> color_to_site_no,
                               bool bClearBinningMaps)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("splitlotID=%1 TotalRun=%2 WaferLotID=%3 WaferID=%4 waferzonefilepath=%5")
            .arg(clETSplitlotInfo.m_lSplitlotID)
            .arg(this->m_uiTotalRuns)
            .arg(WSLotID)
            .arg(WSWaferID)
            .arg(waferzonefilepath)
             .toLatin1().data() );

    if (xys.size()<1)
        return QString("error : no dies (XY pair) to generate!");

    SetTestingStage(GEXDB_PLUGIN_GALAXY_ETEST);

    QString							strQuery, strCondition, strFieldSpec, strDbField0;
    GexDbPlugin_Query				clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int								nArrayOffset=0;
    //int								nIndex=0, nHardBin=0, nSoftBin=0;
    GexDbPlugin_BinMap::Iterator	itBinning;

    int min_x   = xys.begin().key().first;
    int max_x   = xys.begin().key().first;
    int min_y   = xys.begin().key().second;
    int max_y   = xys.begin().key().second;

    QPair<int, int> lXYPair;

    foreach(lXYPair, xys.keys())
    {
        if (min_x > lXYPair.first)    min_x = lXYPair.first;
        if (max_x < lXYPair.first)    max_x = lXYPair.first;
        if (min_y > lXYPair.second)   min_y = lXYPair.second;
        if (max_y < lXYPair.second)   max_y = lXYPair.second;
    }

//    QPair< int, int> p;
//    foreach(p, xys)
//    {
//        if (min_x>p.first) min_x=p.first;
//        if (max_x<p.first) max_x=p.first;
//        if (min_y>p.second) min_y=p.second;
//        if (max_y<p.second) max_y=p.second;
//    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Waferzone image size will be from (%1,%2) to (%3.arg(%4)")
          .arg( min_x)
          .arg(min_y)
          .arg( max_x)
          .arg(max_y)).toLatin1().constData());
    if (mQueryProgress)
        mQueryProgress->AddLog(
                    QString("found dies for this wafer are from (%1,%2) to (%3,%4)...")
                 .arg(min_x).arg(min_y).arg(max_x).arg(max_y)
                                          );
    int wafer_width=max_x-min_x+1;
    int wafer_height=max_y-min_y+1;
    QImage i(wafer_width, wafer_height, QImage::Format_RGB32); // QImage::Format_RGB16 is not enough rich for SVG Kate's colors

    QString r=ConvertSvgToQImageUsingQSvgLib(waferzonefilepath, i);
    if (r.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, (char*)QString("ConvertSvgToQImage returned : %1").arg( r).toLatin1().constData());
        return r;
    }

    // To Do : rotate image here :
    if (mQueryProgress)
        mQueryProgress->AddLog(
                    QString("rotating waferzone image by %1° counterclockwise...").arg(rotation_in_degree));

    if (rotation_in_degree!=0)
    {
        GSLOG(6, QString("Rotating waferzone image by %1 degrees").arg(rotation_in_degree).toLatin1().data() );
        QMatrix m; m.rotate(rotation_in_degree);
        i=i.transformed(m);
        i=i.scaled(wafer_width, wafer_height);
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Generated waferzone image size is %1 %2")
          .arg( i.width())
          .arg(i.height() )).toLatin1().constData());

    QDir d;
    if (!d.mkpath(QDir::homePath() + QDir::separator() + "GalaxySemi" + QDir::separator()+"temp"))
        GSLOG(SYSLOG_SEV_ERROR, QString("can't create path %1").arg( d.absolutePath()).toLatin1().constData());

    i.save(QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp"+QDir::separator() + "waferzone.bmp" );

    // Delete array if allocated
    if(m_pRunInfoArray)
    {
        delete [] m_pRunInfoArray;
        m_pRunInfoArray = NULL;
    }

    // Reset nb of runs
    m_uiSplitlotNbRuns = 0;

    // Create binning maps
    if (!CreateBinningMaps(clETSplitlotInfo, bClearBinningMaps))
        return QString("error : cant create binning maps!");

    // Set RUN array dimension variables
    m_nRunInfoArray_NbItems = xys.size()+1;

    // Allocate RUN array
    m_pRunInfoArray = new GexDbPlugin_RunInfo[m_nRunInfoArray_NbItems];
    if(!m_pRunInfoArray)
    {
        GSLOG(SYSLOG_SEV_ERROR, " cant create m_pRunInfoArray!");
        return QString("error : cant create RunInfo array!");
    }

    // Init matrix
    for(nArrayOffset=0; nArrayOffset<m_nRunInfoArray_NbItems; nArrayOffset++)
    {
        m_pRunInfoArray[nArrayOffset].m_nRunID = nArrayOffset;
        m_pRunInfoArray[nArrayOffset].m_nX = -32768;
        m_pRunInfoArray[nArrayOffset].m_nY = -32768;
        m_pRunInfoArray[nArrayOffset].m_nSiteNb = -1;
        m_pRunInfoArray[nArrayOffset].m_nSoftBin = -1;
        m_pRunInfoArray[nArrayOffset].m_nHardBin = -1;
        m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
        m_pRunInfoArray[nArrayOffset].m_bWrittenToStdf = false;
        m_pRunInfoArray[nArrayOffset].m_lnTestTime = 0;

        if (nArrayOffset==(m_nRunInfoArray_NbItems-1))
            m_pRunInfoArray[nArrayOffset].m_next = NULL;
        else
            m_pRunInfoArray[nArrayOffset].m_next = &m_pRunInfoArray[nArrayOffset+1];

        m_pRunInfoArray[nArrayOffset].m_previous = (nArrayOffset==0)?NULL:&m_pRunInfoArray[nArrayOffset-1];

    }

    if(mBackgroundTransferActivated
            && (mBackgroundTransferInProgress > 0))
    {
        if(!BackgroundTransferLazyMode(m_strTablePrefix + "run",clETSplitlotInfo.m_lSplitlotID))
            return QString("error : cant create RunInfo array!");
    }

    // Soft, Hard bin are the etest one
    // RunId must be from et.wt_run
    // siteNb = zone

    // Query part information
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("preparing run query on %1").arg( m_strTablePrefix).toLatin1().constData());
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "run.run_id");
    m_strlQuery_Fields.append(strFieldSpec + "run.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.part_id");
    // Field part_x
    strDbField0 = m_strTablePrefix+"run.part_x";
    strFieldSpec = "Expression";
    strFieldSpec += "|";
    strFieldSpec += strDbField0;
    strFieldSpec += "|(case when ";
    strFieldSpec += strDbField0;
    strFieldSpec += " IS NULL then -32768 else ";
    strFieldSpec += strDbField0;
    strFieldSpec += " end)";
    m_strlQuery_Fields.append(strFieldSpec);
    // Field part_y
    strDbField0 = m_strTablePrefix+"run.part_y";
    strFieldSpec = "Expression";
    strFieldSpec += "|";
    strFieldSpec += strDbField0;
    strFieldSpec += "|(case when ";
    strFieldSpec += strDbField0;
    strFieldSpec += " IS NULL then -32768 else ";
    strFieldSpec += strDbField0;
    strFieldSpec += " end)";
    m_strlQuery_Fields.append(strFieldSpec);
    //m_strlQuery_Fields.append(strFieldSpec + "run.part_x");
    //m_strlQuery_Fields.append(strFieldSpec + "run.part_y");
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "run.hbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.sbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.ttime");
    m_strlQuery_Fields.append(strFieldSpec + "run.part_txt");
    strCondition = m_strTablePrefix + "run.splitlot_id|Numeric|" + QString::number(clETSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    Query_BuildSqlString(strQuery, false);

    //GSLOG(SYSLOG_SEV_DEBUG, " InitMaps (runs=%d): %s", this->m_uiTotalRuns, strQuery.replace('\n', ' ').toLatin1().data());

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, " query exec failed!");
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return QString("error : failed executing query to fill RunInfo array!");
    }

    //GSLOG(SYSLOG_SEV_DEBUG, strQuery.replace('\n',' ').toLatin1().data());

    int nIndex=0;

    if (mQueryProgress)
        mQueryProgress->AddLog(QString("%1 runs found for this splitlot...")
                                          .arg(clGexDbQuery.size()) );

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 runs found for this splitlot")
          .arg( clGexDbQuery.size()).toLatin1().constData());

    //QList < QMap< QString, QVariant >  > m_runids;
    QMap< int, QMap< QString, QVariant > > site2run_ids;

    // For all run_id
    while(clGexDbQuery.Next())
    {
        // Check for user abort
        if(mQueryProgress->IsAbortRequested())
        {
            GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
            return QString("error : extraction aborted by user!");
        }

        nIndex = 0;
        if (clGexDbQuery.value(nIndex).isNull())
            GSLOG(SYSLOG_SEV_WARNING, "Found a null run id in this run table!");
        unsigned runid = clGexDbQuery.value(nIndex++).toUInt(); // run_id

        if (clGexDbQuery.value(nIndex).isNull())
            GSLOG(SYSLOG_SEV_WARNING, "Found a null site no in this run table!");
        int site_no = clGexDbQuery.value(nIndex++).toInt();

        if (color_to_site_no.key(site_no, Qt::black)==Qt::black)
        {
            QString m=QString("The site %1 is NOT in your waferzone file. The extraction/generation will probably fail.")
              .arg(site_no);
            mQueryProgress->AddLog(m);
            GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
        }

        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              (QString(" run_id=%1 site_no=%2").arg( runid).arg(site_no)).toLatin1().constData());
        QMap< QString, QVariant > a;

        a.insert("run_id", QVariant(runid) );
        a.insert("part_id", clGexDbQuery.value(nIndex++));
        a.insert("part_x", clGexDbQuery.value(nIndex++));
        a.insert("part_y", clGexDbQuery.value(nIndex++));
        a.insert("hbin_no", clGexDbQuery.value(nIndex++));
        a.insert("sbin_no", clGexDbQuery.value(nIndex++));
        a.insert("ttime", clGexDbQuery.value(nIndex++));
        a.insert("part_txt", clGexDbQuery.value(nIndex++));

        site2run_ids.insert(site_no, a);
    }

    nArrayOffset=1;
    int nHardBin=0, nSoftBin=0;

    foreach(lXYPair, xys.keys())
    {
        QRgb lColor = i.pixel(lXYPair.first - min_x, lXYPair.second - min_y);

        if (color_to_site_no.find(lColor)==color_to_site_no.end())
        {
            // The ET WaferMap is represented by site2run_ids
            // The WS WaferMap is in lXYPair
            // The Mapping is done by color_to_site_no
            // It is mandatory to have for each color defined in the SVG associated with a WS WaferMap,
            // a SiteNo from the ET WaferMap
            QString lError=QString("error : the die at (%1,%2) with color %3 does not belong to any known zone according to the given waferzone svg file.\n")
                .arg(lXYPair.first).arg(lXYPair.second).arg(lColor);
            lError+= QString("\n%1 E-Test parts vs %2 SVG ZoneId.")
                    .arg(site2run_ids.count())
                    .arg(color_to_site_no.count());
            lError+= "\nPlease check that the svg file corresponds to the eTest configuration.";
            GSET_ERROR1(GexDbPlugin_Base, eDB_ExtractionError, NULL, lError.toLatin1().data());
            GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().data() );
            return lError;
        }

        int lSite=color_to_site_no[lColor];
        if (site2run_ids.find(lSite)==site2run_ids.end())
        {
            // The ET WaferMap is represented by site2run_ids
            // The WS WaferMap is in lXYPair
            // The Mapping is done by color_to_site_no
            // If the ET WaferMap is truncated (missing runs), this means missing sites
            // we can chose to allow the generation of a truncated WaferMap or not
            // Add a FAKE site_no with EMPTY QMap
            QMap< QString, QVariant > a;
            site2run_ids.insert(lSite, a);

            QString lWarning=QString("error : the site %1 does not correspond to any runID (no test results).").arg(lSite);
            lWarning+= QString("\n%1 E-Test parts vs %2 SVG ZoneId.")
                    .arg(site2run_ids.count())
                    .arg(color_to_site_no.count());
            lWarning+= "\nYou could check that the waferzone eTest configuration is correct.";
            // GSET_ERROR1(GexDbPlugin_Base, eDB_ExtractionError, NULL, lWarning.toLatin1().data());
            GSLOG(SYSLOG_SEV_ERROR, lWarning.toLatin1().data() );
            // And continue
        }

        // Update RUN array
        QMap< QString, QVariant >   lDieProperties       = site2run_ids.find(lSite).value();
        QMap<QString, QVariant>     lXYAttr = xys.value(lXYPair);

        m_pRunInfoArray[nArrayOffset].m_nRunID = lDieProperties["run_id"].toInt();
        m_pRunInfoArray[nArrayOffset].m_bPartExcluded = false;
        m_pRunInfoArray[nArrayOffset].m_nSiteNb = lSite;
        m_pRunInfoArray[nArrayOffset].m_strPartID = lXYAttr["part_id"].toString();
        m_pRunInfoArray[nArrayOffset].m_nX = lXYPair.first;
        m_pRunInfoArray[nArrayOffset].m_nY = lXYPair.second;
        m_pRunInfoArray[nArrayOffset].m_nHardBin = nHardBin = lDieProperties["hbin_no"].toInt();
        m_pRunInfoArray[nArrayOffset].m_nSoftBin = nSoftBin = lDieProperties["sbin_no"].toInt();
        m_pRunInfoArray[nArrayOffset].m_lnTestTime = lDieProperties["ttime"].toULongLong();
        m_pRunInfoArray[nArrayOffset].m_strPartTxt = lDieProperties["part_txt"].toString();

        // Set fail flag, depending on Hard bin
        itBinning = m_mapHardBins.find(nHardBin);
        if(itBinning != m_mapHardBins.end())
        {
            if(itBinning.value().m_cBinCat == 'f' || itBinning.value().m_cBinCat == 'F')
                m_pRunInfoArray[nArrayOffset].m_bPartFailed = true;
        }
        else if(nHardBin != 1)
            m_pRunInfoArray[nArrayOffset].m_bPartFailed = true;

        // Add filter on site # ?
        if(!clETSplitlotInfo.m_strSiteFilterValue.isEmpty())
        {
            m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
            if(clETSplitlotInfo.m_strSiteFilterValue.split(",",QString::SkipEmptyParts).contains(QString::number(m_pRunInfoArray[nArrayOffset].m_nSiteNb)))
                m_pRunInfoArray[nArrayOffset].m_bPartExcluded = false;
        }
        if (!clETSplitlotInfo.m_vHbinsToExclude.isEmpty())
        {
            for (int i=0; i<clETSplitlotInfo.m_vHbinsToExclude.size(); i++)
            {
                if(m_pRunInfoArray[nArrayOffset].m_nHardBin == clETSplitlotInfo.m_vHbinsToExclude.at(i))
                {
                    m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
                    break;
                }
            }
        }

        nArrayOffset++;

        if(m_pRunInfoArray[nArrayOffset].m_bPartExcluded)
            continue;

        m_uiSplitlotNbRuns++;
        m_uiTotalRuns++;
        // Increase bin counts
        (m_mapSoftBins[nSoftBin].m_nBinCount)++;
        (m_mapHardBins[nHardBin].m_nBinCount)++;

        // Update query progress dialog
        if (mQueryProgress)
            mQueryProgress->SetRetrievedRuns(m_uiTotalRuns);

    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 total runs").arg( m_uiTotalRuns).toLatin1().constData());

    if (mQueryProgress)
        mQueryProgress->SetRetrievedRuns(m_uiTotalRuns, true);

    return QString("ok");
}
