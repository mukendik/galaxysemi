#include <QSqlError>
#include <QDir>
#include <gqtl_sysutils.h>
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "test_filter.h"
#include "abstract_query_progress.h"

#include <gqtl_log.h>

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryDataFiles(GexDbPlugin_Filter & cFilters,
                                        const QString & strTestlist,
                                        tdGexDbPluginDataFileList & cMatchingFiles,
                                        const QString & strDatabasePhysicalPath,
                                        const QString & strLocalDir,
                                        bool *pbFilesCreatedInFinalLocation,
                                        GexDbPlugin_Base::StatsSource eStatsSource )
{
    // reset error message to be sure no false error is returned to GEX when error
    // Have to reset Plugin Error Message (no reset after the last insertion)
    m_clLastErrorGexDbPlugin_Base.Reset();
    //GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
    if (mQueryProgress)
    {
        mQueryProgress->ClearLogs();
        mQueryProgress->SetLogsTextColor(Qt::black);
    }

    // Debug message
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Query Data Files for %1 results for tests '%2'...")
                          .arg(cFilters.bConsolidatedExtraction?"consolidated":"not consolidated")
                          .arg(strTestlist).toLatin1().data() );

    unsigned int sampling=1;
    QStringList sl;
    foreach(const QList<QString> &ls, cFilters.m_gexQueries)
    {
        sl=QStringList(ls);
        if (sl.size()==0)
            continue;
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("gexQuery : %1").arg( sl.join(" ")).toLatin1().constData());
        if (mQueryProgress)
            mQueryProgress->AddLog(
                        QString("Extraction option : %1 %2...")
                        .arg(sl.at(0)).arg(sl.size()>1?sl.at(1):""));
        if (sl.at(0)=="db_extraction_mode")
        {
            if (sl.size()>1)
            {
                if(sl.at(1)=="1_out_of_N_samples")
                {
                    if (sl.size()>2)
                        sampling=sl.at(2).toInt();
                }
            }
         }
    }

    // Init plug-in options
    m_clOptions.Init(cFilters.strOptionsString);

    // Init some variables
    m_eStatsSource = eStatsSource;
    m_bSkipAllIndexDialogs = false;
    m_bAbortForUnusableIndex = false;

    // Clear returned list of matching files
    qDeleteAll(cMatchingFiles);
    cMatchingFiles.clear();

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild,
      strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }

    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        if (mQueryProgress)
            mQueryProgress->AddLog("DataBase not compatible for extraction !");
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check testingstage to query
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        SetTestingStage(eElectTest);
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        SetTestingStage(eWaferTest);
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        SetTestingStage(eFinalTest);
    }
    else
    {
        QString m=QString("Unknown Testing Stage '%1'").arg(cFilters.strDataTypeQuery);
        GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
        if (mQueryProgress)
            mQueryProgress->AddLog(m);
        return false;
    }

    if (mQueryProgress)
        mQueryProgress->AddLog(QString("Extracting stage %1").arg(cFilters.strDataTypeQuery));

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
    {
        if (mQueryProgress)
            mQueryProgress->AddLog("Error : cannot connect to DB !");
        return false;
    }

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Init Test filter with testlist
    GexDbPlugin_Galaxy_TestFilter clTestFilter(strTestlist);

    // Extraction group by: wafer, automatic or splitlot
    QString lEGB=m_clOptions.mOptions.value(GS::DbPluginGalaxy::RdbOptions::mOptionExtractionGroupBy).toString();

    // 7067
    if (cFilters.bConsolidatedExtraction &&
        (cFilters.strDataTypeQuery==GEXDB_PLUGIN_GALAXY_WTEST ||
            (cFilters.strDataTypeQuery==GEXDB_PLUGIN_GALAXY_FTEST &&
              ( lEGB=="splitlot" ||
                (lEGB=="automatic" && !HasDieTraceData()) || // automatic and no dietrace data (i.e. splitlot extraction)
                (lEGB=="wafer" && !HasDieTraceData()) // no dietrace data, no per wafer extraction
              )
            )
         )
       )
    {
        m_uiTotalRuns = 0;
        m_uiTotalTestResults = 0;
        if (cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        {
            bool lStatus=QueryDataFilesConsolidatedForFinalTest(
                cFilters, clTestFilter, cMatchingFiles, strLocalDir);

            // Hide if no error or if abort requested
            if(mQueryProgress && mQueryProgress->IsAbortRequested())
                mQueryProgress->SetAutoClose(true);
            if(mQueryProgress &&
                    (lStatus || mQueryProgress->IsAbortRequested()))
                mQueryProgress->Stop();

            return lStatus;
        }

        if (cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        {
            bool lStatus = QueryDataFilesConsolidatedForWaferSort(cFilters, clTestFilter, cMatchingFiles, strDatabasePhysicalPath, strLocalDir,
                                          pbFilesCreatedInFinalLocation, eStatsSource, uiGexDbBuild);
            // Hide if no error or if abort requested
            if(mQueryProgress && mQueryProgress->IsAbortRequested())
                mQueryProgress->SetAutoClose(true);
            if(mQueryProgress &&
                    (lStatus || mQueryProgress->IsAbortRequested()))
                mQueryProgress->Stop();

            return lStatus;
        }

        GSLOG(SYSLOG_SEV_NOTICE, QString("Consolidation algorithm for %1 not implemented. Aborting.").arg(
               cFilters.strDataTypeQuery.toLatin1().data()).toLatin1().constData());
        return false;
    }

    if (cFilters.strDataTypeQuery==GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_uiTotalRuns = 0;
        m_uiTotalTestResults = 0;
        QList<QString> gexquery_granularitydie;
        gexquery_granularitydie.push_back("db_granularity");
        gexquery_granularitydie.push_back("die");

        if (cFilters.m_gexQueries.contains(gexquery_granularitydie))
        {
            QString r=QueryDataFilesForETestPerDie
                    (cFilters, clTestFilter, cMatchingFiles, strDatabasePhysicalPath,
                     strLocalDir, pbFilesCreatedInFinalLocation, eStatsSource);
            if (r.startsWith("error"))
            {
                GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data() );
                if (mQueryProgress)
                {
                    mQueryProgress->SetLogsTextColor(Qt::red);
                    mQueryProgress->AddLog(r);
                    //m_pQueryProgressDlg->Stop();
                    mQueryProgress->EndFileProgress();
                    // Hide if no error or if abort requested
                    if(mQueryProgress && mQueryProgress->IsAbortRequested())
                        mQueryProgress->SetAutoClose(true);
                    if(mQueryProgress->IsAbortRequested())
                        mQueryProgress->Stop();
                }
                return false;
            }
            else
            {
                // Hide if no error or if abort requested
                if(mQueryProgress)
                    mQueryProgress->Stop();

                return true;
        }
    }
    }

    // 3520 : if FT and this DB has die trace data, do not extract splitlot by splitlot but wafers by wafers
    if (cFilters.strDataTypeQuery==GEXDB_PLUGIN_GALAXY_FTEST)
    {
        if (HasDieTraceData())
        {
            // could be either "automatic" or "wafer" or empty
            if (lEGB== "automatic" || lEGB=="wafer" || lEGB.isEmpty())
            {
                bool lHasResult = false;
                QString lR=QueryDataFilesForFTwithDieTrace(
                  cFilters, clTestFilter,
                  cMatchingFiles, strDatabasePhysicalPath, strLocalDir,
                  pbFilesCreatedInFinalLocation,
                  eStatsSource, lHasResult );

                // -- no result and in automatic we try to extract by splitlot
                if(lHasResult == false && lEGB == "automatic")
                {
                    lEGB = "splitlot";
                    m_clOptions.mOptions[GS::DbPluginGalaxy::RdbOptions::mOptionExtractionGroupBy] = "splitlot";
                }
                else
                {
                    GSLOG(SYSLOG_SEV_NOTICE,
                      QString("Query DataFiles for FT using DieTrace returned: %1")
                      .arg(lR).toLatin1().constData());

                    // Hide if no error or if abort requested
                    if(mQueryProgress && mQueryProgress->IsAbortRequested())
                        mQueryProgress->SetAutoClose(true);
                    if(mQueryProgress &&
                            (!lR.startsWith("error") || mQueryProgress->IsAbortRequested()))
                        mQueryProgress->Stop();

                    return (lR.startsWith("error")?false:true);
                }
            }
            //else
                GSLOG(SYSLOG_SEV_NOTICE, QString("Die trace data detected but performing extraction by %1")
                      .arg( lEGB).toLatin1().constData());
        }
    }

    // Construct SQL query
    QString strQuery;
    if(!ConstructSplitlotQuery(cFilters, strQuery, uiGexDbBuild))
        return false;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
    {
        // Following line is not executed with MySQL DB, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
        // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
        clGexDbQuery.setForwardOnly(true);
    }
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    m_uiTotalRuns = 0;
    m_uiTotalTestResults = 0;

    // Create 1 STDF file for each Splitlot retrieved
    GexDbPlugin_DataFile			*pStdfFile;
    QString							strStdfFilename;
    QDir							cDir;
    int								total_extracted_size=0; // sum of all stdf file size
    CGexSystemUtils					clSysUtils;
    GexDbPlugin_Galaxy_SplitlotInfo	clSplitlotInfo;

    while(clGexDbQuery.Next())
    {
        if(cMatchingFiles.isEmpty())
        {
            // Get nb of files to generate
            int nNbFiles = clGexDbQuery.size();
            if(nNbFiles < 0)
                nNbFiles = 1;

            GSLOG(SYSLOG_SEV_NOTICE, QString("Query data files : %1 splitlots to extract...")
                  .arg( nNbFiles ).toLatin1().constData());

            if (mQueryProgress)
                mQueryProgress->AddLog(QString("Extracting %1 splitlots...").arg(nNbFiles));

            // Show query progress dialog
            if (mQueryProgress)
                mQueryProgress->Start(nNbFiles);
        }
        // Fill SplitlotInfo object
        FillSplitlotInfo(clGexDbQuery, &clSplitlotInfo, cFilters);

        if (mQueryProgress)
            mQueryProgress->AddLog(QString("Extracting splitlot '%1'...").arg(clSplitlotInfo.m_lSplitlotID));

        // Create STDF file
        if(CreateStdfFile(clSplitlotInfo, clTestFilter, strLocalDir, strStdfFilename, cFilters, sampling))
        {
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
        else
        {
            // Remove STDF file if exist
            QString	strStdfFullFileName = strLocalDir + strStdfFilename;
            clSysUtils.NormalizePath(strStdfFullFileName);
            if(cDir.exists(strStdfFullFileName))
                cDir.remove(strStdfFullFileName);
            // Display error message if not user abort
            if(!mQueryProgress->IsAbortRequested() && !m_bAbortForUnusableIndex)
                DisplayErrorMessage();
            // Clear matching files
            qDeleteAll(cMatchingFiles);
            cMatchingFiles.clear();

            // Hide if no error or if abort requested
            if(mQueryProgress && mQueryProgress->IsAbortRequested())
                mQueryProgress->SetAutoClose(true);
            if(mQueryProgress &&
                    (mQueryProgress->IsAbortRequested()))
                mQueryProgress->Stop();
            return false;
        }
    }

    // Hide Query progress dialog
    if (mQueryProgress)
        mQueryProgress->Stop();

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Query Data Files : total %1b extracted\n").
          arg(total_extracted_size).toLatin1().constData());

    return cMatchingFiles.count() > 0;
}

