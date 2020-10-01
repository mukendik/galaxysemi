#include <QSqlError>

#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "abstract_query_progress.h"

bool GexDbPlugin_Galaxy::InitMaps(
        const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
        const GexDbPlugin_Filter& cFilters,
        bool bClearBinningMaps/*=true*/,
        QMap< QPair<int, int>, QMap<QString, QVariant> > *xys,
        unsigned int sampling
        )
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Init maps for splitlotID=%1 filtering on %2 totalruns=%3 (XY:%4) at sampling %5")
           .arg(clSplitlotInfo.m_lSplitlotID).arg(cFilters.strlQueryFilters.join(","))
           .arg(this->m_uiTotalRuns).arg(xys?xys->size():-1).arg(sampling)
           .toLatin1().data());

    QString	strQuery, strCondition, strFieldSpec, strDbField0;
    GexDbPlugin_Query clGexDbQuery(this,
      QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int	nArrayOffset, nIndex, nHardBin, nSoftBin;
    GexDbPlugin_BinMap::Iterator itBinning;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Init Maps for splitlot %1...")
           .arg(clSplitlotInfo.m_lSplitlotID).toLatin1().data() );
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    // Delete array if allocated
    if(m_pRunInfoArray)
    {
        delete [] m_pRunInfoArray;
        m_pRunInfoArray = NULL;
    }

    // Reset nb of runs
    m_uiSplitlotNbRuns = 0;

    // Create binning maps
    if (!CreateBinningMaps(clSplitlotInfo, bClearBinningMaps))
        GSLOG(SYSLOG_SEV_ERROR, "CreateBinningMaps failed");

    if(mBackgroundTransferActivated
            && (mBackgroundTransferInProgress > 0))
    {
        if(!BackgroundTransferLazyMode(m_strTablePrefix + "run",clSplitlotInfo.m_lSplitlotID))
            return false;
    }

    // Create part results array
    Query_Empty();
    strFieldSpec = "Function|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "run.run_id|MAX");

    strCondition = m_strTablePrefix + "run.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    bool b=Query_BuildSqlString(strQuery, false);
    if (!b)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot build sql string");
        GSET_ERROR2(GexDbPlugin_Base, eDB_ExtractionError, NULL, "Build sql failed", "Build sql failed");
        return false;
    }

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery) || !clGexDbQuery.Next())
    {
        GSLOG(SYSLOG_SEV_ERROR, clGexDbQuery.lastError().text().toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    if(m_bCustomerDebugMode)
        clGexDbQuery.DumpPerformance();

    // Set RUN array dimension variables
    m_nRunInfoArray_NbItems = clGexDbQuery.value(0).toInt() + 1;

    // Allocate RUN array
    //try
    m_pRunInfoArray = new GexDbPlugin_RunInfo[m_nRunInfoArray_NbItems]; // size of MAX(run_id)
    // catch

    if(!m_pRunInfoArray)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" cant create m_pRunInfoArray = new GexDbPlugin_RunInfo[%1]; !")
              .arg( m_nRunInfoArray_NbItems).toLatin1().constData());
        return false;
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

    // Query part information
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "run.run_id");
    m_strlQuery_Fields.append(strFieldSpec + "run.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.part_id");

    // 7067 & 3520: if FT and we are doing a dietraced extraction by wafer, let's use the dietraced table x&y
    QString lEGB=m_clOptions.mOptions.value(GS::DbPluginGalaxy::RdbOptions::mOptionExtractionGroupBy).toString();
    if (m_strTablePrefix=="ft_" && (lEGB=="wafer"||lEGB=="automatic") && HasDieTraceData())
    {
        // Field part_x
        strDbField0 = m_strTablePrefix+"run_dietrace.part_x";
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
        strDbField0 = m_strTablePrefix+"run_dietrace.part_y";
        strFieldSpec = "Expression";
        strFieldSpec += "|";
        strFieldSpec += strDbField0;
        strFieldSpec += "|(case when ";
        strFieldSpec += strDbField0;
        strFieldSpec += " IS NULL then -32768 else ";
        strFieldSpec += strDbField0;
        strFieldSpec += " end)";
        m_strlQuery_Fields.append(strFieldSpec);
    }
    else
    {
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
    }

    //m_strlQuery_Fields.append(strFieldSpec + "run.part_x");
    //m_strlQuery_Fields.append(strFieldSpec + "run.part_y");
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "run.hbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.sbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "run.tests_executed");
    m_strlQuery_Fields.append(strFieldSpec + "run.ttime");
    m_strlQuery_Fields.append(strFieldSpec + "run.part_txt");

    strCondition = m_strTablePrefix + "run.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    // Filter list without test condition filter
    QStringList   lQueryFilters;

    // Add other conditions
    // when not test condition
    // when linked to the xx_run table (already have a filter for the splitlot)
    // The goal is to have the filter on a xx_run.site_no for exemple
    // but not on xx_splitlot.lot_id='AA' or xx_lot.tracking_id
    foreach(const QString &filter, cFilters.strlQueryFilters)
    {
        if (filter.isEmpty())
            continue;

        QStringList	strlElements = filter.split("=");

        if (strlElements.size()!=2)
            continue;

        QString   strField = strlElements[0];

        GexDbPlugin_Mapping_FieldMap::Iterator itMapping = m_pmapFields_GexToRemote->find(strField);

        if(itMapping == m_pmapFields_GexToRemote->end())
         continue;

        GexDbPlugin_Mapping_Field mf = itMapping.value();

        if(mf.m_strSqlTable.toLower() != QString(m_strTablePrefix + "run").toLower())
            continue;

        if (mf.isTestCondition() == false)
            lQueryFilters.append(filter);
        else
            GSLOG(SYSLOG_SEV_DEBUG, QString("Init maps: Remove test condition filter on %1")
                  .arg( filter).toLatin1().constData());
    }

    // case 4343
    InsertFilters(lQueryFilters);

    b=Query_BuildSqlString(strQuery, false);
    if (!b)
    {
        GSLOG(SYSLOG_SEV_WARNING, "build sql string failed");
        return false;
    }

    //GSLOG(SYSLOG_SEV_DEBUG, QString("Init Maps : runs=%1: %2").arg( this->m_uiTotalRuns, strQuery.replace('\n'.arg( ' ')).toLatin1().constData()));

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, clGexDbQuery.lastError().text().toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // For all run_id
    while(clGexDbQuery.Next())
    {
        // Check for user abort
        if(mQueryProgress->IsAbortRequested())
        {
            GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
            return false;
        }

        nIndex = 0;
        nArrayOffset = clGexDbQuery.value(nIndex++).toUInt(); // run_id

        // Update RUN array
        m_pRunInfoArray[nArrayOffset].m_nRunID = nArrayOffset;
        m_pRunInfoArray[nArrayOffset].m_bPartExcluded = false;
#if 0
        // Case 3169: Since GEXDB B10, et_run table also has a run_id field
        if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
#endif
        m_pRunInfoArray[nArrayOffset].m_nSiteNb = clGexDbQuery.value(nIndex++).toInt();
        m_pRunInfoArray[nArrayOffset].m_strPartID = clGexDbQuery.value(nIndex++).toString();
        if(clGexDbQuery.isNull(nIndex))
            m_pRunInfoArray[nArrayOffset].m_nX = -32768;
        else
            m_pRunInfoArray[nArrayOffset].m_nX = clGexDbQuery.value(nIndex).toInt();
        nIndex++;
        if(clGexDbQuery.isNull(nIndex))
            m_pRunInfoArray[nArrayOffset].m_nY = -32768;
        else
            m_pRunInfoArray[nArrayOffset].m_nY = clGexDbQuery.value(nIndex).toInt();
        nIndex++;
        m_pRunInfoArray[nArrayOffset].m_nHardBin = nHardBin = clGexDbQuery.value(nIndex++).toInt();
        m_pRunInfoArray[nArrayOffset].m_nSoftBin = nSoftBin = clGexDbQuery.value(nIndex++).toInt();
        m_pRunInfoArray[nArrayOffset].m_iTestExecuted = clGexDbQuery.value(nIndex++).toInt();
        m_pRunInfoArray[nArrayOffset].m_lnTestTime = clGexDbQuery.value(nIndex++).toInt();
        if(!clGexDbQuery.isNull(nIndex))
            m_pRunInfoArray[nArrayOffset].m_strPartTxt = clGexDbQuery.value(nIndex).toString();
        nIndex++;


        // Set fail flag, depending on Hard bin
        itBinning = m_mapHardBins.find(nHardBin);
        if(itBinning != m_mapHardBins.end())
        {
            if(itBinning.value().m_cBinCat == 'f' || itBinning.value().m_cBinCat == 'F')
                m_pRunInfoArray[nArrayOffset].m_bPartFailed = true;
        }
        else if(nHardBin != 1)
            m_pRunInfoArray[nArrayOffset].m_bPartFailed = true;

        // Exclude Part Here with m_bPartIgnored flag
        if ( xys && (!xys->isEmpty()) )
        {
            int run_id=clGexDbQuery.value(0).toUInt();
            int x=clGexDbQuery.isNull(3)?-1:clGexDbQuery.value(3).toInt();
            int y=clGexDbQuery.isNull(4)?-1:clGexDbQuery.value(4).toInt();

            QPair<int, int> p; p.first=x; p.second=y;
            QMap< QPair<int, int>, QMap<QString, QVariant> >::iterator it=xys->find(p);
            if (it!=xys->end())
            {
                int splitlot_id=it.value()["splitlot_id"].toInt();
                if (clSplitlotInfo.m_lSplitlotID!=splitlot_id)
                    m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;

                int runid=it.value()["run_id"].toInt();
                if (run_id!=runid)
                    m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
            }
        }

        // Add filter on site # ?
        if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
        {
            m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
            if(clSplitlotInfo.m_strSiteFilterValue.split(",",QString::SkipEmptyParts).contains(QString::number(m_pRunInfoArray[nArrayOffset].m_nSiteNb)))
            // if(QString::number(m_pRunInfoArray[nArrayOffset].m_nSiteNb) == clSplitlotInfo.m_strSiteFilterValue)
                m_pRunInfoArray[nArrayOffset].m_bPartExcluded = false;
        }
        if (!clSplitlotInfo.m_vHbinsToExclude.isEmpty())
        {
            for (int i=0; i<clSplitlotInfo.m_vHbinsToExclude.size(); i++)
            {
                if(m_pRunInfoArray[nArrayOffset].m_nHardBin == clSplitlotInfo.m_vHbinsToExclude.at(i))
                {
                    m_pRunInfoArray[nArrayOffset].m_bPartExcluded = true;
                    break;
                }
            }
        }

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

    // Update query progress dialog
    GSLOG(SYSLOG_SEV_NOTICE, QString("Init Maps: TotalRuns=%1").arg(m_uiTotalRuns).toLatin1().data() );

    if (mQueryProgress)
        mQueryProgress->SetRetrievedRuns(m_uiTotalRuns, true);

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);
        WritePartialPerformance((char*)"InitMaps()");
    }

    return true;
}

bool GexDbPlugin_Galaxy::InsertFilters(QStringList queryFilters)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Insert query filters: %1").arg(queryFilters.join("|")).toLatin1().constData());

    QString			lQueryField;
    QString         lQueryValue;
    int				lNbFields;
    QStringList		lFields;

    foreach(const QString &lFilters, queryFilters)
    {
        lQueryField = lFilters.section('=', 0, 0);
        lQueryValue = lFilters.section('=', 1, 1);
        lNbFields   = Query_FieldsToQuery(lQueryField, lFields);

        if(lNbFields == -1)
            continue;

        if(lNbFields > 1)
        {
            if(!Query_AddValueCondition_MultiField(lFields, lQueryValue))
                return false;
        }
        else
        {
            if(!Query_AddValueCondition(lQueryField, lQueryValue))
                return false;
        }
    }

    return true;
}
