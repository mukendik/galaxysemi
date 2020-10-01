#include <QSqlError>
#include <gqtl_sysutils.h>

#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "abstract_query_progress.h"

QString	GexDbPlugin_Galaxy::CreateStdfFileForETestDieGranularity(
        GexDbPlugin_Galaxy_SplitlotInfo & clETSplitlotInfo,
        GexDbPlugin_Galaxy_TestFilter & clTestFilter,
        const QString & strLocalDir,
        QString & strStdfFileName,
        const QString &WSWaferID, const QString &WSLotID,
        const QMap< QPair<int, int>, QMap<QString, QVariant> > &xys,
//		const QList< QPair<int, int> > &xys,
        const QString& waferzonefilepath,
        const QString desired_et_notch, const QString desired_ws_notch,
        QMap<QRgb, int> color_to_site_no,
        const GexDbPlugin_Filter & cFilters)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("splitlotID=%1 WSLotID=%2 WSWaferID=%3")
        .arg(clETSplitlotInfo.m_lSplitlotID)
        .arg(WSLotID).arg(WSWaferID)
           .toLatin1().data() );

    //QDateTime							clDateTime;
    QString								strMessage;
    //QTime								tTimer;

    // Clear some variables
    strStdfFileName = "";
    m_mapHardBins.clear();
    m_mapSoftBins.clear();

    GSLOG(SYSLOG_SEV_DEBUG, QString(" runs=%1 sublotID='%2' T=%3 good=%4")
        .arg(m_uiTotalRuns).arg(clETSplitlotInfo.m_strSublotID)
        .arg(clETSplitlotInfo.m_uiNbParts).arg(clETSplitlotInfo.m_uiNbParts_Good)
        .toLatin1().data() );

    // Get Wafer info and build file name
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
        if (!SetTestingStage(GEXDB_PLUGIN_GALAXY_ETEST))
                return "error : cant set Etest TS !";

    strStdfFileName = "ET";

    if(!clETSplitlotInfo.m_strProductName.isEmpty())
        strStdfFileName += "_P" + clETSplitlotInfo.m_strProductName;
    if(!clETSplitlotInfo.m_strTesterName.isEmpty())
        strStdfFileName += "_T" + clETSplitlotInfo.m_strTesterName;
    if(!clETSplitlotInfo.m_strLotID.isEmpty())
        strStdfFileName += "_L" + clETSplitlotInfo.m_strLotID;

    QString strWaferID;
    GexDbPlugin_Galaxy_WaferInfo *pclETestWaferInfo = GetWaferInfo(clETSplitlotInfo);
    if(!pclETestWaferInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "can't retrieve ET WaferInfo !");
        return "error : can't retrieve ET WaferInfo !";
    }

    if(pclETestWaferInfo->m_strWaferID.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "ETest wafer ID empty");
        return "error : ETest wafer ID empty";
    }

    strWaferID = pclETestWaferInfo->m_strWaferID;
    strStdfFileName += "_W" + strWaferID;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("ET WaferID = %1 Flat = '%2'")
          .arg( strWaferID)
           .arg(pclETestWaferInfo->m_cWaferFlat)).toLatin1().constData());

    if (desired_et_notch=="from_db" && pclETestWaferInfo->m_cWaferFlat==' ')
    {
        QString m=QString("error : the eTest notch for this lot (%1) wafer (%2) is unknown. You must first fix your DB.")
                .arg(clETSplitlotInfo.m_strLotID)
                .arg(pclETestWaferInfo->m_strWaferID);
        GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().data());
        return m;
    }

    // Try to retrieve WaferSort WaferInfo
    GexDbPlugin_Galaxy_SplitlotInfo  clWSSplitlotInfo;
    clWSSplitlotInfo.m_strLotID=WSLotID;
    clWSSplitlotInfo.m_strWaferID=WSWaferID;
    SetTestingStage(GEXDB_PLUGIN_GALAXY_WTEST);
    GexDbPlugin_Galaxy_WaferInfo *pclWSWaferInfo = GetWaferInfo(clWSSplitlotInfo);
    if (!pclWSWaferInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("error : cant retrieve WaferSort WaferInfo for Lot %1")
              .arg( WSLotID).toLatin1().constData());
        return QString("error : cant retrieve WaferSort WaferInfo for Lot %1 !").arg(WSLotID);
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Corresponding WaferSort wafer has a '%1' flat according to DB...")
          .arg( pclWSWaferInfo->m_cWaferFlat).toLatin1().constData());

    if (mQueryProgress)
        mQueryProgress->AddLog(
        QString("correspondind Wafer at WaferSort stage has a '%1' flat...").arg(pclWSWaferInfo->m_cWaferFlat));

    if (pclWSWaferInfo->m_cWaferFlat==' ' && desired_ws_notch=="from_db")
    {
        QString m=QString("error : the WaferSort notch of lot (%1) wafer (%2) is unknown. You must first fix your DB to continue.")
                .arg(WSLotID)
                .arg(WSWaferID);
        GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().constData());
        return m;
    }

    // Add site information
    if(!clETSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strMessage = clETSplitlotInfo.m_strSiteFilterValue;
        strStdfFileName += "_S" + strMessage.remove(",");
    }

    // Add splitlot_ID to make sure we have a different file for each splitlot
    CGexSystemUtils	clSysUtils;
    strStdfFileName += "_" + QString::number(clETSplitlotInfo.m_lSplitlotID);
    clSysUtils.NormalizeString(strStdfFileName);
    strStdfFileName += ".stdf";

    // Update query progress dialog
    if (mQueryProgress)
        mQueryProgress->SetFileInfo(clETSplitlotInfo.m_strProductName,
                                        clETSplitlotInfo.m_strLotID,
                                        clETSplitlotInfo.m_strSublotID, strWaferID);

    // Create STDF file
    QString	strStdfFullFileName = strLocalDir + strStdfFileName;
    clSysUtils.NormalizePath(strStdfFullFileName);

    // Create STDF records
    GQTL_STDF::StdfParse clStdfParse;

    // Open STDF file
    if(!clStdfParse.Open(strStdfFullFileName.toLatin1().constData(), STDF_WRITE))
    {
        if (pclWSWaferInfo)			delete pclWSWaferInfo;
        if(pclETestWaferInfo)			delete pclETestWaferInfo;
        GSLOG(SYSLOG_SEV_ERROR, QString("can't open STDF '%1'").arg( strStdfFullFileName).toLatin1().constData());
        return QString("error : cant open STDF output file %1 !").arg(strStdfFullFileName);
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("successfully opened %1").arg( strStdfFullFileName).toLatin1().constData());

    int d=0;
    int et_flat=0, ws_flat=0;

    char etn=' ';
    if (desired_et_notch=="from_db")
        etn=pclETestWaferInfo->m_cWaferFlat;
    else
        etn=desired_et_notch.at(0).toLatin1();
    switch (etn)
    {
        case ' ' : GSLOG(SYSLOG_SEV_ERROR, "unknown eTest notch !"); et_flat=0; break;
        case 'U' : et_flat=180; break;
        case 'D' : et_flat=0; break;
        case 'R' : et_flat=-90; break;
        case 'L' : et_flat=90; break;
    }

    char wsn=' ';
    if (desired_ws_notch=="from_db")
        wsn=pclWSWaferInfo->m_cWaferFlat;
    else
        wsn=desired_ws_notch.at(0).toLatin1();
    switch (wsn)
    {
        case ' ' : GSLOG(SYSLOG_SEV_ERROR, "error : unknown WaferSort notch !"); ws_flat=0; break;
        case 'U' : ws_flat=180; break;
        case 'D' : ws_flat=0; break;
        case 'R' : ws_flat=-90; break;
        case 'L' : ws_flat=90; break;
    }

    d=ws_flat-et_flat;

    QString r=InitMapsForETestDieGranularity(clETSplitlotInfo, WSWaferID, WSLotID, xys,
                                             waferzonefilepath, d,
                                             color_to_site_no);
    if (r.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data());
        if (pclWSWaferInfo)			delete pclWSWaferInfo;
        if(pclETestWaferInfo)		delete pclETestWaferInfo;
        return r;
    }
    GSLOG(SYSLOG_SEV_DEBUG, "successfully InitMaps.");

    // Create testlist
    if(!CreateTestlist(clETSplitlotInfo, clTestFilter, cFilters))
    {
        if(pclETestWaferInfo)			delete pclETestWaferInfo;
        if (pclWSWaferInfo)			delete pclWSWaferInfo;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in CreateTestlist");
        return "error in CreateTestlist";
    }

    // Start progress bar for current file
    mQueryProgress->StartFileProgress(m_uiSplitlotNbRuns);

    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1 tests found for this splitlot.")
          .arg( m_clTestList.m_uiNbTests).toLatin1().constData());

    // if the notch has been overloaded overwrite it
    if (desired_et_notch!="from_db")
        pclETestWaferInfo->m_cWaferFlat=desired_et_notch.at(0).toLatin1();

    // Write MIR
    if(!WriteMIR(clStdfParse, clETSplitlotInfo))
    {
        if (pclWSWaferInfo)			delete pclWSWaferInfo;
        if(pclETestWaferInfo)			delete pclETestWaferInfo;
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteMIR ! Aborting...");
        return "error in WriteMIR";
    }

    // Write SDR
    if(!WriteSDR(clStdfParse, clETSplitlotInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteSDR ! Aborting...");
        return "error in WriteSDR ";
    }

    // Write PMR
    if(!WritePinMapRecords(clStdfParse, clETSplitlotInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WritePinMapRecords ! Aborting...");
        return "error in WritePinMapRecords ";
    }

    // Write DTR with gross_die
    if(pclETestWaferInfo
            && (pclETestWaferInfo->m_uiGrossDie > 0)
            && (pclETestWaferInfo->m_uiFlags & FLAG_WAFERINFO_GROSSDIEOVERLOADED))
    {
        GQTL_STDF::Stdf_DTR_V4 clDTR;
        QString strUserTxt = "<cmd> gross_die=" + QString::number(pclETestWaferInfo->m_uiGrossDie);
        // Set DTR fields
        clDTR.SetTEXT_DAT(strUserTxt);
        // Write record
        clStdfParse.WriteRecord(&clDTR);
    }

    // Write WCR

    if(pclETestWaferInfo && !WriteWCR(clStdfParse, pclETestWaferInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteWCR ! Aborting...");
        return "error in WriteWCR";
    }

    // Write WIR
    if(pclETestWaferInfo && !WriteWIR(clStdfParse, clETSplitlotInfo, pclETestWaferInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWIR ! Aborting...");
        return "error in WriteWIR ";
    }

    // Write static test info, if no raw data extracted
    // Otherwise, they are written with the first part result
    if(!m_clOptions.mOptions.value(GS::DbPluginGalaxy::RdbOptions::mOptionExtractRawData).toBool())    //m_clOptions.m_bExtractRawData)
        WriteStaticTestInfo(clStdfParse, clETSplitlotInfo);

    // Write test results
    r=WriteTestResults(clStdfParse, clETSplitlotInfo, clTestFilter, NULL, true);
    if(r.startsWith("error"))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteTestResults");
        return "error in WriteTestResults";
    }

    // Write WRR
    if(pclETestWaferInfo && !WriteWRR(clStdfParse, clETSplitlotInfo, pclETestWaferInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWRR ");
        return "error in WriteWRR ";
    }

    // Write PAT DTRs
    if((m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST) && !WritePatDtrRecords(clStdfParse, clETSplitlotInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WritePatDtrRecords");
        return "error in WritePatDtrRecords";
    }

    // Write summary records
    // 4343 : if any filters on run table, do not write all SummaryRecords
    bool any_run_filters=false;
// To restore to disable case 6408
    /*
    QString res=cFilters.ContainsFilterOnTable( m_strTablePrefix + "run", (GexDbPlugin_Base*)this, any_run_filters);
    if (res.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot check metadata");
        return "error : "+res;
    }
    */

    if((m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
        && !WriteSummaryRecords(clStdfParse, clETSplitlotInfo, any_run_filters, cFilters))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, "WriteSummaryRecords failed !");
        return "error in WriteSummaryRecords";
    }

    // Write MRR
    if(!WriteMRR(clStdfParse, clETSplitlotInfo))
    {
        if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
        if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }
        clStdfParse.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteMRR ! Aborting...");
        return "error in WriteMRR ";
    }

    // Free ressources
    if(pclWSWaferInfo) { delete pclWSWaferInfo; pclWSWaferInfo=0; }
    if(pclETestWaferInfo) { delete pclETestWaferInfo; pclETestWaferInfo=0; }

    clStdfParse.Close();
    m_clTestList.ClearData(true);

    // Stop progress bar for current file
    if (mQueryProgress)
        mQueryProgress->EndFileProgress();

    // Clear maps
    m_mapHardBins.clear();
    m_mapSoftBins.clear();

    return "ok";
}
