#include <QSqlError>
#include <QString>
#include <gqtl_sysutils.h>
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "gexdb_plugin_datafile.h"
#include "test_filter.h"
#include <gqtl_log.h>
#include "abstract_query_progress.h"

bool GexDbPlugin_Galaxy::HasDieTraceData()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Has DieTrace Data ?");

    // Check if already have this information
    /////////////////
    // HAVE TO UPDATE GALAXY_PLUGIN CONNECT/DISCONNECT BEFORE
    // DO NOT CHECK - ALWAYS CHECK WITH QUERY
    mAttributes.clear();
    /////////////////
    if(!GetAttribute("HasDieTraceData").isValid())
    {
        // Query the database
        // On MySql : count(*) on millions rows takes XXX s
        // On MySQL : faster with : select * from ... LIMIT 1
        // On Oracle : count(*) should be ok
        GexDbPlugin_Query lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString lQuery="SELECT COUNT(*) FROM ft_dietrace_config";
        if(m_pclDatabaseConnector->IsMySqlDB())
            lQuery = "SELECT splitlot_id FROM ft_dietrace_config LIMIT 1";
        // 1758 on db_multi on wkd
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                        lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        SetAttribute("HasDieTraceData",QVariant(!( !lGexDbQuery.First() // for MySql
                                                   || (lGexDbQuery.value(0).toInt() == 0) // for Oracle
                                                   )));
    }
    return GetAttribute("HasDieTraceData").toBool();
}

QString GexDbPlugin_Galaxy
::GetListOfFTSplitlotsContainingWSProductLotWafer(
        const QMap<QString,QVariant> &PLW,
        GexDbPlugin_Filter & cFilters,
        QList<GexDbPlugin_Galaxy_SplitlotInfo> &/*list*/)
{
    QString lQuery;
    Query_Empty();
    m_strlQuery_Fields.append("Field|ft_splitlot.splitlot_id");
    // Lets filter on this PLW
    m_strlQuery_ValueConditions.append( "ft_dietrace_config.product|String|"+PLW["product"].toString() );
    m_strlQuery_ValueConditions.append( "ft_dietrace_config.lot_id|String|"+PLW["lot"].toString() );
    m_strlQuery_ValueConditions.append( "ft_dietrace_config.wafer_id|String|"+PLW["wafer"].toString() );
    // Be sure to order by RI in order to write results in stdf in the right order
    // With multi Retest phase, the retest_index is reset between each and cannot be used for the order
    // m_strlQuery_OrderFields.append("ft_splitlot.retest_index");
    m_strlQuery_OrderFields.append("ft_splitlot.start_t");

    bool lB=Query_AddFilters(cFilters); // in case of any other filters
    if (!lB)
        return "error: cant add filters to query";
    lB=Query_AddTimePeriodCondition(cFilters);
    if (!lB)
        return "error: cant add time filters";

    lB=Query_BuildSqlString_UsingJoins(lQuery, true);
    if (!lB)
        return "error: cant build splitlots identification query";

    GexDbPlugin_Query lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!lGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return "error: splitlots identification query failed: "
                + lGexDbQuery.lastError().text() +" :" + lQuery;
    }

    //list.clear();
    while(lGexDbQuery.Next())
    {
        bool ok=false;
        //list.append( lGexDbQuery.value(0).toUInt(&ok) );
        if (!ok)
            return "error: cannot translate splitlot id into integer";
    }

    return "ok";
}

QString GexDbPlugin_Galaxy
::GetListOfDieTracedProductLotWafers(
        GexDbPlugin_Filter & cFilters,
        QList< QMap <QString, QVariant> > &list)
{
    QString lQuery;
    Query_Empty();
    m_strlQuery_Fields.append("Field|ft_dietrace_config.product");
    m_strlQuery_Fields.append("Field|ft_dietrace_config.lot_id");
    m_strlQuery_Fields.append("Field|ft_dietrace_config.wafer_id");
    // 7055: make sure extractions will be ordered by product,lot,wafer
    m_strlQuery_OrderFields.append("ft_dietrace_config.product");
    m_strlQuery_OrderFields.append("ft_dietrace_config.lot_id");
    m_strlQuery_OrderFields.append("ft_dietrace_config.wafer_id");
    bool lB=Query_AddFilters(cFilters);
    if (!lB)
        return "error: cant add filters to query";
    lB=Query_AddTimePeriodCondition(cFilters);
    if (!lB)
        return "error: cant add time filters";
    lB=Query_BuildSqlString_UsingJoins(lQuery, true);
    if (!lB)
        return "error: cant build prodlotwafer SQL query";

    GexDbPlugin_Query lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!lGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return "error: prodlotwafer identification query failed: "
                + QString(lGexDbQuery.lastError().text()) +" :" + lQuery ;
    }

    while(lGexDbQuery.Next())
    {
        QMap< QString, QVariant > plw;
        plw.insert("product", lGexDbQuery.value(0).toString() );
        plw.insert("lot_id", lGexDbQuery.value(1).toString() );
        plw.insert("wafer_id", lGexDbQuery.value(2).toString() );
        list.append(plw);
    }

    return "ok";
}

QString GexDbPlugin_Galaxy
::QueryDataFilesForFTwithDieTrace(GexDbPlugin_Filter & cFilters,
        GexDbPlugin_Galaxy_TestFilter & lTestFilter,
        tdGexDbPluginDataFileList & lMatchingFiles,
        const QString & lDatabasePhysicalPath,
        const QString & lLocalDir,
        bool* lFilesCreatedInFinalLocation,
        GexDbPlugin_Base::StatsSource lStatsSource, bool &hasResult)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
             (QString("Query DataFiles For FT with DieTrace: %1 filters, for tests %2 into DB %3. at local dir %4 statsSource %5")
             .arg(cFilters.strlQueryFilters.size())
             .arg(lTestFilter.getFullTestNblist().toLatin1().data())
             .arg(lDatabasePhysicalPath.toLatin1().data())
             .arg(lLocalDir.toLatin1().data())
             .arg(lStatsSource))
             .toLatin1().constData());

    lMatchingFiles.clear(); // ?

    if (lFilesCreatedInFinalLocation)
        *lFilesCreatedInFinalLocation=true;

    QList< QMap <QString, QVariant> > lPLWs; // list of ProductLotWafers
    QString lR=GetListOfDieTracedProductLotWafers(cFilters, lPLWs);

    if(lPLWs.isEmpty())
       hasResult = false;
    else
       hasResult = true;

    if (lR.startsWith("error"))
        return lR;

    if (mQueryProgress)
        mQueryProgress->Start( lPLWs.size() );

    // let s first identify the prod/lot/wafers (i.e. file) to extract
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 ProductLotWafer(s) to extract...")
          .arg( lPLWs.size()).toLatin1().constData());
    QMap<QString,QVariant> PLW;
    foreach(PLW, lPLWs)
    {
        lR=ExtractDieTracedPLW(PLW, cFilters, lTestFilter, lMatchingFiles, lLocalDir);
        if (lR.startsWith("error"))
            return lR;
    }

    return "ok";
}

QString GexDbPlugin_Galaxy::ExtractDieTracedPLW(
        const QMap<QString,QVariant> &PLW,
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_Galaxy_TestFilter &lTestFilter,
        tdGexDbPluginDataFileList &lMatchingFiles,
        const QString &lLocalDir)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
             (QString("ExtractDieTraced ProductLotWafer %1 tests %2, already %3 files")
             .arg(PLW["wafer_id"].toString().toLatin1().data())
             .arg(lTestFilter.getFullTestNblist().toLatin1().data())
             .arg(lMatchingFiles.size() )).toLatin1().constData());

    // Identify the ft splitlot(s) that contains at least 1 die from this wafer
    GexDbPlugin_SplitlotList lSplitlotList;
    GexDbPlugin_Filter lTempFilters=cFilters;
    // Add filters to this PLW
    GexDbPlugin_Mapping_Field lWaferFieldMapping;
    if (!m_mapFields_GexToRemote_Ft.ContainsSqlFullField(
                "ft_dietrace_config.wafer_id", lWaferFieldMapping))
        return "error: impossible to find FT metadata for WS wafer ID";
    lTempFilters.strlQueryFilters.append(lWaferFieldMapping.m_strGexName+"="+PLW["wafer_id"].toString());

    GexDbPlugin_Mapping_Field lLotFieldMapping;
    if (!m_mapFields_GexToRemote_Ft.ContainsSqlFullField(
                "ft_dietrace_config.lot_id", lLotFieldMapping))
        return "error: impossible to find FT metadata for WS Lot ID";
    lTempFilters.strlQueryFilters.append(lLotFieldMapping.m_strGexName+"="+PLW["lot_id"].toString());

    GexDbPlugin_Mapping_Field lProductFieldMapping;
    if (!m_mapFields_GexToRemote_Ft.ContainsSqlFullField(
                "ft_dietrace_config.product", lProductFieldMapping))
        return "error: impossible to find FT metadata for WS Product";
    lTempFilters.strlQueryFilters.append(lProductFieldMapping.m_strGexName+"="+PLW["product"].toString());

    // 7067 : add filter on splitlots if consolidated
    if (lTempFilters.bConsolidatedExtraction)
    {
        lTempFilters.strlQueryFilters.append( QString(GEXDB_PLUGIN_DBFIELD_PROD_DATA)+ "=Y");
        lTempFilters.strlQueryFilters.append( QString(GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT)+ "=Y");
        m_strlQuery_ValueConditions.append( "ft_splitlot.prod_data='Y'");
        m_strlQuery_ValueConditions.append( "ft_splitlot.valid_splitlot<>'N'");
    }

    //Use ConstructSplitlotQuery(...) in order to get Galaxy_SplitlotInfo

    bool lB=QuerySplitlots(lTempFilters, lSplitlotList);
    if (!lB)
        return "error: failed to query splitlots for this wafer";

    QString lM=QString("Wafer %1 spread over %2 FT splitlots:").arg(PLW["wafer_id"].toString()).arg(lSplitlotList.count());
    mQueryProgress->AddLog(lM);
    GSLOG(SYSLOG_SEV_NOTICE, lM.toLatin1().data());

    /*
    QList<GexDbPlugin_Galaxy_SplitlotInfo> lSplitlots;
    QString lR=GetListOfSplitlotsContainingProductLotWafer(PLW, cFilters, lSplitlots);
    if (lR.startsWith("error"))
        return lR;
    */

    // Open Stdf
    QString lStdfFileName="GEXDB_FT_P"+PLW["product"].toString()+"_L"
            +PLW["lot_id"].toString()
            +"_W"+PLW["wafer_id"].toString()+".stdf";

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
             (QString("%1 splitlot(s) to extract (partially or not) for PLW: %2 %3 %4: %5")
             .arg(lSplitlotList.count())
             .arg(PLW["product"].toString().toLatin1().data())
             .arg(PLW["lot_id"].toString().toLatin1().data())
             .arg(PLW["wafer_id"].toString().toLatin1().data())
             .arg(lStdfFileName.toLatin1().data()))
          .toLatin1().constData());

    QString	lStdfFullFileName = lLocalDir + lStdfFileName;
    CGexSystemUtils::NormalizePath(lStdfFullFileName);

    GQTL_STDF::StdfParse lStdfParse;
    // Open STDF file
    if(!lStdfParse.Open(lStdfFullFileName.toLatin1().data(), STDF_WRITE))
    {
        /*
            if(pclWaferInfo)
                delete pclWaferInfo;
            pclWaferInfo=0;
        */
        QString lE="error: failed to open for writing "+ lStdfFullFileName;
        GSLOG(SYSLOG_SEV_WARNING, lE.toLatin1().data() );
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Open, GGET_LASTERROR(StdfParse, &lStdfParse));
        return lE;
    }

    //BeginStdf(...) : todo ?
    // Create a faked splitlotinfo just to write some global records
    GexDbPlugin_Galaxy_SplitlotInfo lGSI;
    lGSI.m_cValidSplitlot='Y';
    bool ok=false;
    int lWID=PLW["wafer_id"].toInt(&ok);
    if (ok)
        lGSI.m_nWaferNb=lWID;
    lGSI.m_strFileName=lStdfFileName;
    lGSI.m_strLotID=PLW["lot_id"].toString();
    lGSI.m_strProductName=PLW["product"].toString();
    lGSI.m_strWaferID=PLW["wafer_id"].toString();
    // write MIR
    if (!WriteMIR(lStdfParse, lGSI))
    {
        lStdfParse.Close();
        return "error: failed to write MIR";
    }

    // Let s create a faked WaferInfo
    GexDbPlugin_Galaxy_WaferInfo lWI;
    // 7026
    QString WSSimulation("simulatews"); // lower case !!!!
    if (m_clOptions.mOptions.contains(m_clOptions.mOptionSimulateWSifDieTracedFTExtraction))
        WSSimulation=m_clOptions.mOptions.value(m_clOptions.mOptionSimulateWSifDieTracedFTExtraction).toString().toLower();

    if (WSSimulation=="simulatews")
    {
        lWI.m_strWaferID=PLW["wafer_id"].toString();
        bool b=WriteWIR(lStdfParse, lGSI, &lWI);
        if (!b)
        {
            lStdfParse.Close();
            return "error: cant write WIR";
        }
    }

    //foreach(unsigned splitlot_id, lSplitlotList)
    GexDbPlugin_SplitlotInfo* lSI=0;
    foreach(lSI, lSplitlotList)
    {
        // 7055:
        // Pass lTempFilters instead of cFilters to make sure the splitlot extraction will filter required runs
        // if some run level filters are defined
        QString lR=ExtractSplitlotIntoFile(*lSI, PLW, lTempFilters, lTestFilter, lStdfParse);
        if (lR.startsWith("error"))
        {
            lStdfParse.Close();
            return lR;
        }
    }

    // 7026
    if (WSSimulation=="simulatews")
    {
        bool b=WriteWRR(lStdfParse, lGSI, &lWI);
        if (!b)
        {
            lStdfParse.Close();
            return "error: cant write WRR";
        }
    }

    if (!WriteMRR(lStdfParse, lGSI))
    {
        lStdfParse.Close();
        return "error: failed to write MRR";
    }

    // Close Stdf
    lStdfParse.Close();
    m_clTestList.ClearData(true);

    // Stop progress bar for current file
    if (mQueryProgress)
        mQueryProgress->EndFileProgress();

    // Add stdf to lMatchingFiles : STDF file successfully created
    GexDbPlugin_DataFile	*pStdfFile= new GexDbPlugin_DataFile;
    pStdfFile->m_strFileName = lStdfFileName;
    pStdfFile->m_strFilePath = lLocalDir;
    pStdfFile->m_bRemoteFile = false;
    // Append to list
    lMatchingFiles.append(pStdfFile);

    return "ok";
}

QString GexDbPlugin_Galaxy::ExtractSplitlotIntoFile(
        const GexDbPlugin_SplitlotInfo &lSplitlotInfo,
        const QMap<QString,QVariant> &PLW,
        GexDbPlugin_Filter &lFilters,
        GexDbPlugin_Galaxy_TestFilter &lTestFilter,
        GQTL_STDF::StdfParse &lStdfParse)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Extracting die from PLW %1 %2 %3 in FT splitlot %4").
          arg(PLW["product"].toString()).
          arg(PLW["lot_id"].toString()).
          arg(PLW["wafer_id"].toString()).
          arg(lSplitlotInfo.m_lSplitlotID).toLatin1().constData());

    if (mQueryProgress)
        mQueryProgress->SetFileInfo(
                    PLW["product"].toString(), PLW["lot_id"].toString(),
                    QString::number(lSplitlotInfo.m_lSplitlotID), PLW["wafer_id"].toString() );

    GexDbPlugin_Galaxy_SplitlotInfo lGSI;
    lGSI.m_bConsolidatedExtraction=lFilters.bConsolidatedExtraction;
    //lGSI.m_cCmodCod= ?
    //lGSI.m_cModeCod= ?
    //lGSI.m_cProdData= ?
    //lGSI.m_cProtCod= lSplitlotInfo. ?
    //lGSI.m_cRtstCod=lSplitlotInfo. ?
    //lGSI.m_cValidSplitlot= lSplitlotInfo. ?
    lGSI.m_lSplitlotID = lSplitlotInfo.m_lSplitlotID;
    //lGSI.m_lSyaID = lSplitlotInfo. ?
    //lGSI.m_nWaferNb = lSplitlotInfo. ?
    //lGSI.m_strAuxFile = lSplitlotInfo. ?
    lGSI.m_strCableID=lSplitlotInfo.m_strCableID;
    lGSI.m_strCableType=lSplitlotInfo.m_strCableType;
    lGSI.m_strCardID=lSplitlotInfo.m_strCardID;
    lGSI.m_strCardType=lSplitlotInfo.m_strCardType;
    lGSI.m_strContactorID=lSplitlotInfo.m_strContactorID;
    lGSI.m_strContactorType=lSplitlotInfo.m_strContactorType;
    //lGSI.m_strDataProvider=lSplitlotInfo. ?
    //lGSI.m_strDataType=lSplitlotInfo. ?
    //lGSI.m_strDateCod=lSplitlotInfo. ?
    lGSI.m_strDay=lSplitlotInfo.m_strDay;
    lGSI.m_strDibID=lSplitlotInfo.m_strDibID;
    lGSI.m_strDibType=lSplitlotInfo.m_strDibType;
    //lGSI.m_strDsgnRev=lSplitlotInfo. ?
    //lGSI.m_strEngID=lSplitlotInfo. ?
    //lGSI.m_strEtestSiteConfig = lSplitlotInfo ?
    lGSI.m_strExecType=lSplitlotInfo.m_strExecType;
    // todo : fill other fields ?

    // Init diferent maps: X,Y matrix with part results, binning map,...
    if(!InitMaps(lGSI, lFilters, true, NULL, 1)) // sampling ?
        return "error: cant init maps";

    if(!CreateTestlist(lGSI, lTestFilter, lFilters))
        return "error: cant create test list";

    // Write test results
    QString r=WriteTestResults(lStdfParse, lGSI, lTestFilter, NULL, false, 1);
    if(r.startsWith("error"))
        return r;

    // GCORE-2449
    /* // no way to use WriteSummaryRecords() because bin counts are all wrong if so
    bool lWritesumRecordResult=false;
    if (!lGSI.m_bConsolidatedExtraction)
        lWritesumRecordResult=WriteSummaryRecords(lStdfParse, lGSI, true, lFilters);
    else
        lWritesumRecordResult=WriteSummaryRecords_Consolidated(lStdfParse, lGSI);
    if (!lWritesumRecordResult)
        return "error: failed to write sumary records";
    */
    // GCORE-2449: let s try to manually write SBR & HBR
    GexDbPlugin_BinMap::Iterator lBinIt;
    GQTL_STDF::Stdf_SBR_V4 lSBR;
    for (lBinIt=m_mapSoftBins.begin(); lBinIt!=m_mapSoftBins.end(); lBinIt++)
    {
        lSBR.Reset();
        lSBR.SetSBIN_NUM(lBinIt.key());
        lSBR.SetSBIN_NAM(lBinIt.value().m_strBinName);
        lSBR.SetSBIN_PF(lBinIt.value().m_cBinCat.toLatin1());
        lSBR.SetSBIN_CNT(lBinIt.value().m_nBinCount);
        // Sandrine is saying that for summary head num must be 255
        lSBR.SetHEAD_NUM(255);
        // Sandrine is saying that site default should be 1
        lSBR.SetSITE_NUM(1);
        lStdfParse.WriteRecord(&lSBR);
    }
    GQTL_STDF::Stdf_HBR_V4 lHBR;
    for (lBinIt=m_mapHardBins.begin(); lBinIt!=m_mapHardBins.end(); lBinIt++)
    {
        lHBR.Reset();
        lHBR.SetHBIN_NUM(lBinIt.key());
        lHBR.SetHBIN_NAM(lBinIt.value().m_strBinName);
        lHBR.SetHBIN_PF(lBinIt.value().m_cBinCat.toLatin1());
        lHBR.SetHBIN_CNT(lBinIt.value().m_nBinCount);
        lHBR.SetHEAD_NUM(255);
        lHBR.SetSITE_NUM(1);
        lStdfParse.WriteRecord(&lHBR);
    }


    return "ok";
}
