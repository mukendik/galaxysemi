// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "test_filter.h"
#include "abstract_query_progress.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>

// Standard includes
#include <math.h>

// Qt includes
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QDateTime>
#include <QMap>
#include <QDir>
#include <QMessageBox>

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <gqtl_sysutils.h>
#include "gstdl_md5checksum_c.h"

bool GexDbPlugin_Galaxy::CreateStdfFileName(
    QString& fileName,
    const GexDbPlugin_Galaxy_SplitlotInfo& clSplitlotInfo,
    const GexDbPlugin_Galaxy_TestFilter & clTestFilter,
    const GexDbPlugin_Filter& dbFilters,
    const GexDbPlugin_Galaxy_WaferInfo* gdpgwiPtrWaferInfo
    ) const
{
    fileName.clear();

    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        fileName = "GEXDB_ET";
    else if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        fileName = "GEXDB_WT";
    else if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        fileName = "GEXDB_FT";
    else
        return false;

    if(!clSplitlotInfo.m_strProductName.isEmpty())
        fileName += "_P" + clSplitlotInfo.m_strProductName;
    if(!clSplitlotInfo.m_strTesterName.isEmpty())
        fileName += "_T" + clSplitlotInfo.m_strTesterName;
    if(!clSplitlotInfo.m_strLotID.isEmpty())
        fileName += "_L" + clSplitlotInfo.m_strLotID;

    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        if( (gdpgwiPtrWaferInfo) &&
                (!gdpgwiPtrWaferInfo->m_strWaferID.isEmpty()) )
            fileName += "_W" + gdpgwiPtrWaferInfo->m_strWaferID;
    }

    // Add site information
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        QString lSiteFilterValue = clSplitlotInfo.m_strSiteFilterValue;
        fileName += "_S" + lSiteFilterValue.remove(",");
    }

    // Add splitlot_ID to make sure we have a different file for each splitlot
    fileName += "_" + QString::number(clSplitlotInfo.m_lSplitlotID);


    // Add md5 hash key if test list and/or test conditions are defined
    QString lTestList = clTestFilter.getCleanTestNblist();
    if(!lTestList.isEmpty() || !dbFilters.strlQueryFilters.isEmpty())
    {
        QString lMD5String;
        // Add test conditions if any
        QString	lFieldName;
        QString	lFieldValue;

        foreach(const QString &filter, dbFilters.strlQueryFilters)
        {
            if (filter.isEmpty())
                continue;
            lFieldName   = filter.section("=", 0, 0);
            lFieldValue  = filter.section("=", 1, 1);
            if(lFieldName.isEmpty() || lFieldValue.isEmpty())
                continue;
            const GexDbPlugin_Mapping_FieldMap::const_iterator itMapping = m_pmapFields_GexToRemote->find(lFieldName);
            if(itMapping != m_pmapFields_GexToRemote->end())
            {
                lMD5String += "_" + lFieldName + "-" + lFieldValue;
            }
        }

        if (!lTestList.isEmpty())
        {
            // ensure key unicity
            QStringList qslTestList = lTestList.split(QString(","),
                                                        QString::SkipEmptyParts);
            qslTestList.sort();
            lMD5String += qslTestList.join(QString(","));
        }

        /// TO CHECK : can it be > 64 ?
        UINT uintTstListSize = lMD5String.size();
        BYTE bytePtrTstList[uintTstListSize];

        for(UINT ii=0; ii<uintTstListSize; ii++)
            bytePtrTstList[ii]= (lMD5String.at(ii)).toLatin1();

        char chrPtrCheckSumRslt[MD5_RESULT_SIZE];
        int nCheckSumRslt = GetMD5Buffer(bytePtrTstList,
                                         uintTstListSize,
                                         chrPtrCheckSumRslt);

        if(nCheckSumRslt!=MD5_OK)
        {
            GEX_ASSERT(false);
            return false;
        }

        QString strCheckSumRslt;
        strCheckSumRslt.clear();
        for(int ii=0; ii<MD5_RESULT_SIZE-1 ; ii++)  // -1 for \0 last character
            strCheckSumRslt.append(QChar(chrPtrCheckSumRslt[ii]));

        fileName += QString("_") + strCheckSumRslt;
    }

    // everything went well !
    return true;
}


bool GexDbPlugin_Galaxy::CreateStdfFile(
    GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
    GexDbPlugin_Galaxy_TestFilter & clTestFilter,
    const QString & strLocalDir,
    QString & strStdfFileName,
    const GexDbPlugin_Filter& dbFilters,
    const int &sampling
    )
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
      QString("Create stdf file for splitlot %1 tests '%2' sampling at 1 out of %3 (%4 query params)...")
        .arg(clSplitlotInfo.m_lSplitlotID)
        .arg(clTestFilter.getCleanTestNblist())
        .arg(sampling).arg(dbFilters.m_gexQueries.size())
        .toLatin1().data()
        );
        //.arg(clTestFilter.getFullTestNblist() ) );

    /*
    for(int i=0; i<clSplitlotInfo.m_gexQueries.size(); i++)
    {
        //QList<QString> sl=clSplitlotInfo.m_gexQueries.at(i);
        //if (sl.size()==0)
          //  continue;
        //GSLOG(SYSLOG_SEV_DEBUG, QString("Create Stdf file query param : %1 %2").arg(sl.at(0)).arg(sl.size()>1?sl.at(1):"") );
    }
    */

    GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo=NULL;
    QString	strMessage;

    // Clear some variables
    strStdfFileName = "";
    m_mapHardBins.clear();
    m_mapSoftBins.clear();

    GSLOG(SYSLOG_SEV_DEBUG, (QString("Create stdf file for runs=%1 sublotID=%2 T=%3 good=%4")
          .arg(m_uiTotalRuns)
          .arg(clSplitlotInfo.m_strSublotID.toLatin1().data())
          .arg(clSplitlotInfo.m_uiNbParts)
          .arg(clSplitlotInfo.m_uiNbParts_Good)).toLatin1().constData());

    // Get Wafer info
    QString strWaferID;

    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        pclWaferInfo = GetWaferInfo(clSplitlotInfo);
        if(pclWaferInfo && !pclWaferInfo->m_strWaferID.isEmpty())
        {
            strWaferID = pclWaferInfo->m_strWaferID;
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" wafer notch according to DB is '%1'")
                  .arg( pclWaferInfo->m_cWaferFlat).toLatin1().constData());
        }
    }

    // Add site information
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
        strMessage = clSplitlotInfo.m_strSiteFilterValue;

    // Build file name
    if(!CreateStdfFileName(strStdfFileName, clSplitlotInfo,
                           clTestFilter, dbFilters, pclWaferInfo))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("CreateStdfFileName failed : %1").arg( strStdfFileName).toLatin1().constData());
        GEX_ASSERT(false);  /// TO DO : GSLOG
        // return false ?
    }

    // normalize file name
    //CGexSystemUtils	clSysUtils;
    CGexSystemUtils::NormalizeString(strStdfFileName);
    strStdfFileName += ".stdf";

    // Update query progress dialog
    if (mQueryProgress)
        mQueryProgress->SetFileInfo(clSplitlotInfo.m_strProductName,
                                            clSplitlotInfo.m_strLotID,
                                            clSplitlotInfo.m_strSublotID,
                                            strWaferID);

    // Create STDF file
    QString	strStdfFullFileName = strLocalDir + strStdfFileName;
    CGexSystemUtils::NormalizePath(strStdfFullFileName);

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        strMessage = "Extracting splitlot ";
        strMessage += strStdfFileName;
        GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );
        m_clExtractionPerf.Start(true);
    }

    // Create STDF records
    GQTL_STDF::StdfParse clStdfParse;

    // Open STDF file
    if(!clStdfParse.Open(strStdfFullFileName.toLatin1().data(), STDF_WRITE))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        GSLOG(SYSLOG_SEV_WARNING, QString("Error while opening %1 ").arg( strStdfFullFileName).toLatin1().constData());
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Open, GGET_LASTERROR(StdfParse, &clStdfParse));
        return false;
    }

    // Init diferent maps: X,Y matrix with part results, binning map,...
    if(!InitMaps(clSplitlotInfo, dbFilters, true, NULL, sampling))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in InitMaps ! Aborting...");
        return false;
    }

    if(!CreateTestlist(clSplitlotInfo, clTestFilter, dbFilters))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in CreateTestlist ! Aborting...");
        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create stdf file : %1 tests found for this splitlot.")
          .arg( m_clTestList.m_uiNbTests).toLatin1().constData());

    // Start progress bar for current file
    if (mQueryProgress)
        mQueryProgress->StartFileProgress(m_uiSplitlotNbRuns);

    if(!WriteMIR(clStdfParse, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteMIR ! Aborting...");
        return false;
    }

    if(!WriteSDR(clStdfParse, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteSDR ! Aborting...");
        return false;
    }

    // Write PMR
    if(!WritePinMapRecords(clStdfParse, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WritePinMapRecords ! Aborting...");
        return false;
    }

    // Write DTR with gross_die
    if(pclWaferInfo
            && (pclWaferInfo->m_uiGrossDie > 0)
            && (pclWaferInfo->m_uiFlags & FLAG_WAFERINFO_GROSSDIEOVERLOADED))
    {
        GQTL_STDF::Stdf_DTR_V4 clDTR;
        QString strUserTxt = "<cmd> gross_die=" + QString::number(pclWaferInfo->m_uiGrossDie);
        // Set DTR fields
        clDTR.SetTEXT_DAT(strUserTxt);
        // Write record
        clStdfParse.WriteRecord(&clDTR);
    }

    // Write WCR
    if(pclWaferInfo && !WriteWCR(clStdfParse, pclWaferInfo))
    {
        delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWCR ! Aborting...");
        return false;
    }

    // Write WIR
    if(pclWaferInfo && !WriteWIR(clStdfParse, clSplitlotInfo, pclWaferInfo))
    {
        delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWIR ! Aborting...");
        return false;
    }

    // If no raw data extracted, write static test info
    // Otherwise, they are written with the first part result
    if (!m_clOptions.mOptions
          .value( GS::DbPluginGalaxy::RdbOptions::mOptionExtractRawData ).toBool())      //m_clOptions.m_bExtractRawData)
        if (!WriteStaticTestInfo(clStdfParse, clSplitlotInfo))
            GSLOG(SYSLOG_SEV_ERROR, "WriteStaticTestInfo failed");

    // Write test results
    QString r=WriteTestResults(clStdfParse, clSplitlotInfo, clTestFilter, NULL, false, sampling);
    if(r.startsWith("error"))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, QString("WriteTestResults failed : %1").arg( r).toLatin1().constData());
        return false;
    }

    // Write WRR
    if(pclWaferInfo && !WriteWRR(clStdfParse, clSplitlotInfo, pclWaferInfo))
    {
        delete pclWaferInfo;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWRR ! Aborting...");
        return false;
    }


    // Write PAT DTRs
    if((m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST) && !WritePatDtrRecords(clStdfParse, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WritePatDtrRecords ! Aborting...");
        return false;
    }

    // Write summary records
    // 4343 : if any filters on run table, do not write SummaryRecords
    bool any_run_filters=false;

    // To restore to disable case 6408
    /*
    QString res=dbFilters.ContainsFilterOnTable(m_strTablePrefix + "run", (GexDbPlugin_Base*)this, any_run_filters);
    if (res.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot check metadata");
        return false;
    }
    */

    if((m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
        && !WriteSummaryRecords(clStdfParse, clSplitlotInfo, any_run_filters,dbFilters))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteSummaryRecords ! Aborting...");
        return false;
    }

    // Write MRR
    if(!WriteMRR(clStdfParse, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        pclWaferInfo=0;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteMRR ! Aborting...");
        return false;
    }

    // Free ressources
    if(pclWaferInfo)
        delete pclWaferInfo;
    clStdfParse.Close();
    m_clTestList.ClearData(true);

    // Stop progress bar for current file
    if (mQueryProgress)
        mQueryProgress->EndFileProgress();

    // Clear maps
    m_mapHardBins.clear();
    m_mapSoftBins.clear();

    #ifdef NO_DUMP_PERF
     return true;
    #endif

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        QString strMessage, strPerformance;
        m_clExtractionPerf.Stop();

        if(m_clExtractionPerf.m_fTime_Total >= 1000000.0F)
        {
            strPerformance =  "-------- Execution time           = " + QString::number(m_clExtractionPerf.m_fTime_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query execution      = " + QString::number(m_clExtractionPerf.m_fTimer_DbQuery_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query iteration      = " + QString::number(m_clExtractionPerf.m_fTimer_DbIteration_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query extracted rows = " + QString::number(m_clExtractionPerf.m_ulNbRows_Total) + " \n";
            strPerformance += "-------- Nb of runs               = " + QString::number(m_clExtractionPerf.m_ulNbRuns_Total) + " \n";
            strPerformance += "-------- Nb of test results       = " + QString::number(m_clExtractionPerf.m_ulNbTestResults_Total) + " \n";
            strMessage.sprintf(	"-------- Splitlot %s extracted in %.2f seconds:\n%s\n",
                                strStdfFileName.toLatin1().constData(),
                                m_clExtractionPerf.m_fTime_Total/1000000.0F,
                                strPerformance.toLatin1().constData());
        }
        else
        {
            strPerformance =  "-------- Execution time           = " + QString::number(m_clExtractionPerf.m_fTime_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query execution      = " + QString::number(m_clExtractionPerf.m_fTimer_DbQuery_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query iteration      = " + QString::number(m_clExtractionPerf.m_fTimer_DbIteration_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query extracted rows = " + QString::number(m_clExtractionPerf.m_ulNbRows_Total) + " \n";
            strPerformance += "-------- Nb of runs               = " + QString::number(m_clExtractionPerf.m_ulNbRuns_Total) + " \n";
            strPerformance += "-------- Nb of test results       = " + QString::number(m_clExtractionPerf.m_ulNbTestResults_Total) + " \n";
            strMessage.sprintf(	"-------- Splitlot %s extracted in %d ms:\n%s\n",
                                strStdfFileName.toLatin1().constData(),
                                (int)(m_clExtractionPerf.m_fTime_Total/1000.0F),
                                strPerformance.toLatin1().constData());
        }

        WriteDebugMessageFile(strMessage, true);
    }

    return true;
}
