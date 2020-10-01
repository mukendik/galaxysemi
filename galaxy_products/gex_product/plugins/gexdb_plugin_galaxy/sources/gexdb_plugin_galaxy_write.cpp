#include "gexdb_plugin_base.h"
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include <gqtl_log.h>

#include <QSqlError>

bool	GexDbPlugin_Galaxy::WritePart(
        GQTL_STDF::StdfParse & clStdfParse,
        GexDbPlugin_RunInfo *pCurrentPart,
        bool write_all_other_RunInfo_with_same_run_id/*=false*/ )
{
    if (!pCurrentPart)
        return false;

    bool ok=false;
    ok=WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
    if (!ok)
        return false;

    ok=WriteTestInfo(clStdfParse, pCurrentPart);
    if (!ok)
        return false;

    ok=WritePRR(clStdfParse, pCurrentPart);
    if (!ok)
        return false;
    pCurrentPart->m_bWrittenToStdf = true;

    if (write_all_other_RunInfo_with_same_run_id)
    {
        GexDbPlugin_RunInfo *pPart=m_pRunInfoArray; //pCurrentPart->m_next;

        // ToDo : cascade to all others GexDbPlugin_RunInfo* (next and previous)
        while (pPart)
        {
            if (pPart==pCurrentPart)
            {
                pPart=pPart->m_next;
                continue;
            }
            if(pPart->m_bPartExcluded)
            {
                pPart=pPart->m_next;
                continue;
            }
            if (pPart->m_nRunID==pCurrentPart->m_nRunID)
                WritePart(clStdfParse, pPart, false);
            pPart=pPart->m_next;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write STDF MIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteMIR(GQTL_STDF::StdfParse & clStdfParse,
                                  const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString("for splitlot %1").arg( clSplitlotInfo.m_lSplitlotID));
    GQTL_STDF::Stdf_MIR_V4 clMIR;

    // Set MIR fields
    clMIR.SetSETUP_T(clSplitlotInfo.m_uiSetupTime);
    clMIR.SetSTART_T(clSplitlotInfo.m_uiStartTime);
    clMIR.SetSTAT_NUM(clSplitlotInfo.m_uiStationNb);
    clMIR.SetMODE_COD(clSplitlotInfo.m_cModeCod);
    clMIR.SetRTST_COD(clSplitlotInfo.m_cRtstCod);
    clMIR.SetPROT_COD(clSplitlotInfo.m_cProtCod);
    clMIR.SetBURN_TIM(clSplitlotInfo.m_uiBurnTim);
    clMIR.SetCMOD_COD(clSplitlotInfo.m_cCmodCod);
    clMIR.SetLOT_ID(clSplitlotInfo.m_strLotID.toLatin1().constData());
    clMIR.SetPART_TYP(clSplitlotInfo.m_strProductName.toLatin1().constData());
    clMIR.SetNODE_NAM(clSplitlotInfo.m_strTesterName.toLatin1().constData());
    clMIR.SetTSTR_TYP(clSplitlotInfo.m_strTesterType.toLatin1().constData());
    clMIR.SetJOB_NAM(clSplitlotInfo.m_strJobName.toLatin1().constData());
    clMIR.SetJOB_REV(clSplitlotInfo.m_strJobRev.toLatin1().constData());
    clMIR.SetSBLOT_ID(clSplitlotInfo.m_strSublotID.toLatin1().constData());
    clMIR.SetOPER_NAM(clSplitlotInfo.m_strOperator.toLatin1().constData());
    clMIR.SetEXEC_TYP(clSplitlotInfo.m_strExecType.toLatin1().constData());
    clMIR.SetEXEC_VER(clSplitlotInfo.m_strExecVer.toLatin1().constData());
    clMIR.SetTEST_COD(clSplitlotInfo.m_strTestCode.toLatin1().constData());
    clMIR.SetTST_TEMP(clSplitlotInfo.m_strTestTemp.toLatin1().constData());
    clMIR.SetUSER_TXT(clSplitlotInfo.m_strUserTxt.toLatin1().constData());
    clMIR.SetAUX_FILE(clSplitlotInfo.m_strAuxFile.toLatin1().constData());
    clMIR.SetPKG_TYP(clSplitlotInfo.m_strPkgTyp.toLatin1().constData());
    clMIR.SetFAMLY_ID(clSplitlotInfo.m_strFamlyID.toLatin1().constData());
    clMIR.SetDATE_COD(clSplitlotInfo.m_strDateCod.toLatin1().constData());
    clMIR.SetFACIL_ID(clSplitlotInfo.m_strFacilID.toLatin1().constData());
    clMIR.SetFLOOR_ID(clSplitlotInfo.m_strFloorID.toLatin1().constData());
    clMIR.SetPROC_ID(clSplitlotInfo.m_strProcID.toLatin1().constData());
    clMIR.SetSPEC_NAM(clSplitlotInfo.m_strSpecNam.toLatin1().constData());
    clMIR.SetSPEC_VER(clSplitlotInfo.m_strSpecVer.toLatin1().constData());

    // Write record
    return clStdfParse.WriteRecord(&clMIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF SDR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteSDR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    GexDbPlugin_Query           clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString                     strQuery, strCondition, strFieldSpec;
    GQTL_STDF::Stdf_SDR_V4      lSDR;

    GSLOG(SYSLOG_SEV_DEBUG, QString("WriteSDR").toLatin1().constData());

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // SDR
    ///////////////////////////////////////////////////////////////////////////////////////

    // Get pin map list
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "sdr.site_grp");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.site_index");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.hand_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.hand_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.card_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.card_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.load_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.load_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.dib_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.dib_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.cabl_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.cabl_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.cont_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.cont_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.lasr_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.lasr_id");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.extr_typ");
    m_strlQuery_Fields.append(strFieldSpec + "sdr.extr_id");
    strCondition = m_strTablePrefix + "sdr.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site #
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "sdr.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }


    // Build query
    bool b=Query_BuildSqlString(strQuery, false);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed");

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through sdr records
    GSLOG(SYSLOG_SEV_DEBUG, QString("WriteSDR: %1 records retrieved")
          .arg( clGexDbQuery.size()).toLatin1().constData());
    bool lHasSDRTabbeResult(false);
    while(clGexDbQuery.Next())
    {
        lHasSDRTabbeResult = true;
        lSDR.Reset();
        lSDR.SetSITE_GRP(clGexDbQuery.value("site_grp").toInt());
        lSDR.SetSITE_CNT(1);
        // Always add only one site at index 0
        lSDR.SetSITE_NUM(0, clGexDbQuery.value("site_no").toInt());
        lSDR.SetHAND_TYP(clGexDbQuery.value("hand_typ").toString());
        lSDR.SetHAND_ID(clGexDbQuery.value("hand_id").toString());
        lSDR.SetCARD_TYP(clGexDbQuery.value("card_typ").toString());
        lSDR.SetCARD_ID(clGexDbQuery.value("card_id").toString());
        lSDR.SetLOAD_TYP(clGexDbQuery.value("load_typ").toString());
        lSDR.SetLOAD_ID(clGexDbQuery.value("load_id").toString());
        lSDR.SetDIB_TYP(clGexDbQuery.value("dib_typ").toString());
        lSDR.SetDIB_ID(clGexDbQuery.value("dib_id").toString());
        lSDR.SetCABL_TYP(clGexDbQuery.value("cabl_typ").toString());
        lSDR.SetCABL_ID(clGexDbQuery.value("cabl_id").toString());
        lSDR.SetCONT_TYP(clGexDbQuery.value("cont_typ").toString());
        lSDR.SetCONT_ID(clGexDbQuery.value("cont_id").toString());
        lSDR.SetLASR_TYP(clGexDbQuery.value("lasr_typ").toString());
        lSDR.SetLASR_ID(clGexDbQuery.value("lasr_id").toString());
        lSDR.SetEXTR_TYP(clGexDbQuery.value("extr_typ").toString());
        lSDR.SetEXTR_ID(clGexDbQuery.value("extr_id").toString());
        clStdfParse.WriteRecord(&lSDR);
    }

    if (!lHasSDRTabbeResult)
    {
        // Set SDR fields
        lSDR.SetHEAD_NUM(0);
        lSDR.SetSITE_GRP(0);
        if(clSplitlotInfo.m_strSiteFilterValue.isEmpty())
            lSDR.SetSITE_CNT(0);
        else
        {
            QStringList lSites = clSplitlotInfo.m_strSiteFilterValue.split(",", QString::SkipEmptyParts);
            lSDR.SetSITE_CNT(lSites.count());
            for(int i=0; i<lSites.count(); i++)
                lSDR.SetSITE_NUM(i, lSites[i].toInt());
        }
        lSDR.SetHAND_TYP(clSplitlotInfo.m_strHandlerType);
        lSDR.SetHAND_ID(clSplitlotInfo.m_strHandlerID);
        lSDR.SetCARD_TYP(clSplitlotInfo.m_strCardType);
        lSDR.SetCARD_ID(clSplitlotInfo.m_strCardID);
        lSDR.SetLOAD_TYP(clSplitlotInfo.m_strLoadboardType);
        lSDR.SetLOAD_ID(clSplitlotInfo.m_strLoadboardID);
        lSDR.SetDIB_TYP(clSplitlotInfo.m_strDibType);
        lSDR.SetDIB_ID(clSplitlotInfo.m_strDibID);
        lSDR.SetCABL_TYP(clSplitlotInfo.m_strCableType);
        lSDR.SetCABL_ID(clSplitlotInfo.m_strCableID);
        lSDR.SetCONT_TYP(clSplitlotInfo.m_strContactorType);
        lSDR.SetCONT_ID(clSplitlotInfo.m_strContactorID);
        lSDR.SetLASR_TYP(clSplitlotInfo.m_strLaserType);
        lSDR.SetLASR_ID(clSplitlotInfo.m_strLaserID);
        lSDR.SetEXTR_TYP(clSplitlotInfo.m_strExtraType);
        lSDR.SetEXTR_ID(clSplitlotInfo.m_strExtraID);

        // Write record
        return clStdfParse.WriteRecord(&lSDR);
    }

    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);
        // Write partial performance
        WritePartialPerformance((char*)"WritePinMapRecords()");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write STDF MRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteMRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    GQTL_STDF::Stdf_MRR_V4 clMRR;

    // Set MRR fields
    clMRR.SetFINISH_T(clSplitlotInfo.m_uiFinishTime);
    clMRR.SetDISP_COD(' ');

    // Write record
    return clStdfParse.WriteRecord(&clMRR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteWIR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo, const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo)
{
    GQTL_STDF::Stdf_WIR_V4 clWIR;

    // Set WIR fields
    clWIR.SetHEAD_NUM(1);
    clWIR.SetSITE_GRP(255);
    clWIR.SetSTART_T(clSplitlotInfo.m_uiStartTime);
    clWIR.SetWAFER_ID(pclWaferInfo->m_strWaferID);

    // Write record
    return clStdfParse.WriteRecord(&clWIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteWRR(
        GQTL_STDF::StdfParse & clStdfParse,
        const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
        const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo)
{
    GQTL_STDF::Stdf_WRR_V4 clWRR;

    // Set WRR fields
    clWRR.SetHEAD_NUM(1);
    clWRR.SetSITE_GRP(255);
    clWRR.SetFINISH_T(clSplitlotInfo.m_uiFinishTime);
    if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
        clWRR.SetPART_CNT(clSplitlotInfo.m_uiNbParts);
    else
        clWRR.SetPART_CNT(0);
    clWRR.SetRTST_CNT(4294967295u);
    clWRR.SetABRT_CNT(4294967295u);
    clWRR.SetGOOD_CNT(clSplitlotInfo.m_uiNbParts_Good);
    clWRR.SetFUNC_CNT(4294967295u);
    clWRR.SetWAFER_ID(pclWaferInfo->m_strWaferID);
    clWRR.SetFABWF_ID(pclWaferInfo->m_strFabID);
    clWRR.SetFRAME_ID(pclWaferInfo->m_strFrameID);
    clWRR.SetMASK_ID(pclWaferInfo->m_strMaskID);

    // Write record
    return clStdfParse.WriteRecord(&clWRR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteWCR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo)
{
    GQTL_STDF::Stdf_WCR_V4	clWCR;

    // Set WRR fields
    clWCR.SetWAFR_SIZ(pclWaferInfo->m_fWaferSize);
    clWCR.SetDIE_HT(pclWaferInfo->m_fDieHt);
    clWCR.SetDIE_WID(pclWaferInfo->m_fDieWid);
    clWCR.SetWF_UNITS(pclWaferInfo->m_uiWaferUnits);
    clWCR.SetWF_FLAT(pclWaferInfo->m_cWaferFlat);
    clWCR.SetCENTER_X(pclWaferInfo->m_nCenterX);
    clWCR.SetCENTER_Y(pclWaferInfo->m_nCenterY);
    clWCR.SetPOS_X(pclWaferInfo->m_cPosX);
    clWCR.SetPOS_Y(pclWaferInfo->m_cPosY);

    // Write record
    return clStdfParse.WriteRecord(&clWCR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF PIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WritePIR(GQTL_STDF::StdfParse & clStdfParse, int nSiteNum)
{
    GQTL_STDF::Stdf_PIR_V4 clPIR;

    // Set PIR fields
    clPIR.SetHEAD_NUM(1);
    clPIR.SetSITE_NUM(nSiteNum);

    // Write record
    return clStdfParse.WriteRecord(&clPIR);
}

bool GexDbPlugin_Galaxy::WritePRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo *pCurrentPart)
{
    GQTL_STDF::Stdf_PRR_V4 clPRR;

    // Set PRR fields
    clPRR.SetHEAD_NUM(1);
    clPRR.SetSITE_NUM(pCurrentPart->m_nSiteNb);
    if(pCurrentPart->m_bPartFailed)
        clPRR.SetPART_FLG(0x08);
    else
        clPRR.SetPART_FLG(0);
    clPRR.SetNUM_TEST(pCurrentPart->m_iTestExecuted); // need to put the right value
    clPRR.SetHARD_BIN(pCurrentPart->m_nHardBin);
    clPRR.SetSOFT_BIN(pCurrentPart->m_nSoftBin);
    clPRR.SetX_COORD(pCurrentPart->m_nX);
    clPRR.SetY_COORD(pCurrentPart->m_nY);
    clPRR.SetTEST_T(pCurrentPart->m_lnTestTime);
    clPRR.SetPART_ID(pCurrentPart->m_strPartID);
    if(!pCurrentPart->m_strPartTxt.isEmpty())
        clPRR.SetPART_TXT(pCurrentPart->m_strPartTxt);

    // Write record
    return clStdfParse.WriteRecord(&clPRR);
}


bool GexDbPlugin_Galaxy::WritePinMapRecords(GQTL_STDF::StdfParse & clStdfParse,const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    // Only for GEXDB TDR > B24
    if(m_uiDbVersionBuild <= GEXDB_DB_VERSION_BUILD_B24)
        return true;

    // Only for Wafer-Sort and Final-Test!!
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        return true;

    GexDbPlugin_Query           clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString                     strQuery, strCondition, strFieldSpec;
    GQTL_STDF::Stdf_PMR_V4     clPMR;

    GSLOG(SYSLOG_SEV_DEBUG, QString("WritePinMapRecords").toLatin1().constData());

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // PIN MAP RECORDS (PMR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Get pin map list
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.tpin_pmrindex");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.chan_typ");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.chan_nam");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.phy_nam");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.log_nam");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.head_num");
    m_strlQuery_Fields.append(strFieldSpec + "pin_map.site_num");
    strCondition = m_strTablePrefix + "pin_map.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Build query
    bool b=Query_BuildSqlString(strQuery, false);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed");

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through pin map records
    GSLOG(SYSLOG_SEV_DEBUG, QString("WritePinMapRecords: %1 records retrieved")
          .arg( clGexDbQuery.size()).toLatin1().constData());
    int iIndex = 0;
    while(clGexDbQuery.Next())
    {
        iIndex = 0;
        clPMR.Reset();
        clPMR.SetPMR_INDX(clGexDbQuery.value(iIndex).toInt()); iIndex++;
        clPMR.SetCHAN_TYP(clGexDbQuery.value(iIndex).toInt()); iIndex++;
        clPMR.SetCHAN_NAM(clGexDbQuery.value(iIndex).toString().toLatin1().constData()); iIndex++;
        clPMR.SetPHY_NAM(clGexDbQuery.value(iIndex).toString().toLatin1().constData()); iIndex++;
        clPMR.SetLOG_NAM(clGexDbQuery.value(iIndex).toString().toLatin1().constData()); iIndex++;
        clPMR.SetHEAD_NUM(clGexDbQuery.value(iIndex).toInt()); iIndex++;
        clPMR.SetSITE_NUM(clGexDbQuery.value(iIndex).toInt()); iIndex++;
        clStdfParse.WriteRecord(&clPMR);
    }

    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);
        // Write partial performance
        WritePartialPerformance((char*)"WritePinMapRecords()");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write PAT DTR records
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WritePatDtrRecords(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    // Only for Wafer-Sort!!
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_WTEST)
        return true;

    GSLOG(SYSLOG_SEV_DEBUG, QString("WritePatDtrRecords on %1").arg( m_strTestingStage).toLatin1().constData());

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    // Query PAT DTR records
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strQuery, strCommand;
    GQTL_STDF::Stdf_DTR_V4		clDTR;

    strQuery =  "SELECT dtr_text FROM wt_dtr where splitlot_id=" + QString::number(clSplitlotInfo.m_lSplitlotID);
    strQuery += " AND dtr_type='logPAT' ORDER BY order_id ASC";
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through DTRs
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 DTRs retrieved").arg( clGexDbQuery.size()).toLatin1().constData());
    while(clGexDbQuery.Next())
    {
        strCommand = "<cmd> logPAT " + clGexDbQuery.value(0).toString();
        clDTR.SetTEXT_DAT(strCommand);
        clStdfParse.WriteRecord(&clDTR);
    }

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);

        // Write partial performance
        WritePartialPerformance((char*)"WritePatDtrRecords()");
    }

    return true;
}

bool GexDbPlugin_Galaxy::WriteSummaryRecords_Consolidated(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    int												nBinning, nSiteNum;
    GexDbPlugin_BinMap::Iterator					itBinning;
    GexDbPlugin_Query								clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString											strQuery, strCondition, strFieldSpec;
    GQTL_STDF::Stdf_SBR_V4							clSBR;
    GQTL_STDF::Stdf_HBR_V4							clHBR;
    GQTL_STDF::Stdf_PCR_V4							clPCR;
    unsigned int									uiBinCount;

    GSLOG(SYSLOG_SEV_DEBUG, "WriteSummaryRecords_Consolidated");

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // CONSOLIDATED EXTRACTION
    //
    // FT Consolidation is done on HBins, so if we are in FT consolidated mode:
    // 1) If full datalog (samples=summary), then create HBR+SBR records with PRR results
    // 2) Else create only HBR with hbin summary records, but no SBR
    // 3) In any case, create no PCR, TSR
    //
    // WT Consolidation is done stacking wafer maps, so if we are in WT consolidated mode:
    // 1) Create HBR+SBR records with PRR results
    // 2) Create no PCR, TSR
    ///////////////////////////////////////////////////////////////////////////////////////

    // 1) Check if WT testing stage or full datalog
    if((m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST) || (clSplitlotInfo.m_uiNbPartsSamples == clSplitlotInfo.m_uiNbPartsSummary))
    {
        int lNbParts = 0;
        int lNbPartsGood = 0;
        GexDbPlugin_BinInfo	clBinInfo;

        // Create HBR from PRR results
        for(itBinning = m_mapHardBins.begin(); itBinning != m_mapHardBins.end(); itBinning++)
        {
            clBinInfo = itBinning.value();

            if(clBinInfo.m_nBinCount > 0)
            {
                lNbParts += clBinInfo.m_nBinCount;
                if(clBinInfo.m_cBinCat == 'P')
                    lNbPartsGood += clBinInfo.m_nBinCount;

                // Write HBR record
                clHBR.SetHEAD_NUM(255);
                clHBR.SetSITE_NUM(255);
                clHBR.SetHBIN_NUM(itBinning.key());
                clHBR.SetHBIN_CNT(clBinInfo.m_nBinCount);
                clHBR.SetHBIN_PF(clBinInfo.m_cBinCat.toLatin1());
                clHBR.SetHBIN_NAM(clBinInfo.m_strBinName);
                clStdfParse.WriteRecord(&clHBR);
            }
        }

        // Create SBR from PRR results
        for(itBinning = m_mapSoftBins.begin(); itBinning != m_mapSoftBins.end(); itBinning++)
        {
            clBinInfo = itBinning.value();

            if(clBinInfo.m_nBinCount > 0)
            {
                // Write SBR record
                clSBR.SetHEAD_NUM(255);
                clSBR.SetSITE_NUM(255);
                clSBR.SetSBIN_NUM(itBinning.key());
                clSBR.SetSBIN_CNT(clBinInfo.m_nBinCount);
                clSBR.SetSBIN_PF(clBinInfo.m_cBinCat.toLatin1());
                clSBR.SetSBIN_NAM(clBinInfo.m_strBinName);
                clStdfParse.WriteRecord(&clSBR);
            }
        }

        if(lNbParts > 0)
        {
            // Write PCR record
            clPCR.SetHEAD_NUM(255);
            clPCR.SetSITE_NUM(255);
            clPCR.SetPART_CNT(lNbParts);
            clPCR.SetRTST_CNT(0);
            clPCR.SetABRT_CNT(0);
            clPCR.SetGOOD_CNT(lNbPartsGood);
            clStdfParse.WriteRecord(&clPCR);
        }

        return true;
    }

    // 2) If not full datalog, reate only HBR with hbin summary records, but no SBR
    // Get hard bin summary
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.hbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.bin_count");
    strCondition = m_strTablePrefix + "hbin_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "hbin_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    // Add filter on Hbins ?
    if (!clSplitlotInfo.m_vHbinsToExclude.isEmpty())
    {
        for (int i=0; i<clSplitlotInfo.m_vHbinsToExclude.size(); i++)
            m_strlQuery_ValueConditions.append(
                        m_strTablePrefix + "hbin_stats_summary.hbin_no|Numeric|"+ QString("!=%1").arg(clSplitlotInfo.m_vHbinsToExclude.at(i)) ); // ?
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through binning summary
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 hbin_stats_summary retrieved").arg( clGexDbQuery.size()).toLatin1().constData());
    uiBinCount = 0;
    while(clGexDbQuery.Next())
    {
        uiBinCount++;
        nBinning = clGexDbQuery.value(0).toInt();
        nSiteNum = clGexDbQuery.value(1).toInt();
        itBinning = m_mapHardBins.find(nBinning);
        if(itBinning != m_mapHardBins.end())
        {
            // Write HBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clHBR.SetHEAD_NUM(255);
            else
                clHBR.SetHEAD_NUM(1);
            clHBR.SetSITE_NUM(nSiteNum);
            clHBR.SetHBIN_NUM(nBinning);
            clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
            clHBR.SetHBIN_PF((*itBinning).m_cBinCat.toLatin1());
            clHBR.SetHBIN_NAM((*itBinning).m_strBinName);
            clStdfParse.WriteRecord(&clHBR);
        }
        else
        {
            char cBinCat = 'F';
            if(nBinning == 1)
                cBinCat = 'P';
            // Write HBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clHBR.SetHEAD_NUM(255);
            else
                clHBR.SetHEAD_NUM(1);
            clHBR.SetSITE_NUM(nSiteNum);
            clHBR.SetHBIN_NUM(nBinning);
            clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
            clHBR.SetHBIN_PF(cBinCat);
            clHBR.SetHBIN_NAM("");
            clStdfParse.WriteRecord(&clHBR);
        }
    }

    // If no Bin summary available, use samples
    if(uiBinCount == 0)
    {
        GSLOG(SYSLOG_SEV_DEBUG,"uiBinCount=0");
        // Get hard bin samples
        Query_Empty();
        strFieldSpec = "Field|";
        strFieldSpec += m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.hbin_no");
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.site_no");
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.nb_parts");
        strCondition = m_strTablePrefix + "hbin_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
        m_strlQuery_ValueConditions.append(strCondition);
        // Add filter on site # ?
        if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
        {
            strCondition = m_strTablePrefix + "hbin_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        // Add filter on Hbins ?
        if (!clSplitlotInfo.m_vHbinsToExclude.isEmpty())
        {
            for (int i=0; i<clSplitlotInfo.m_vHbinsToExclude.size(); i++)
                m_strlQuery_ValueConditions.append(
                            m_strTablePrefix + "hbin_stats_samples.hbin_no|Numeric|"+ QString("!=%1").arg(clSplitlotInfo.m_vHbinsToExclude.at(i)) ); // ?
        }
        Query_BuildSqlString(strQuery, false);

        clGexDbQuery.setForwardOnly(true);
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Iterate through binning samples
        GSLOG(SYSLOG_SEV_DEBUG, QString("%1 hbin_stats_samples.").arg( clGexDbQuery.size()).toLatin1().constData());
        while(clGexDbQuery.Next())
        {
            uiBinCount++;
            nBinning = clGexDbQuery.value(0).toInt();
            nSiteNum = clGexDbQuery.value(1).toInt();
            itBinning = m_mapHardBins.find(nBinning);
            if(itBinning != m_mapHardBins.end())
            {
                // Write HBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clHBR.SetHEAD_NUM(255);
                else
                    clHBR.SetHEAD_NUM(1);
                clHBR.SetSITE_NUM(nSiteNum);
                clHBR.SetHBIN_NUM(nBinning);
                clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
                clHBR.SetHBIN_PF((*itBinning).m_cBinCat.toLatin1());
                clHBR.SetHBIN_NAM((*itBinning).m_strBinName);
                clStdfParse.WriteRecord(&clHBR);
            }
            else
            {
                char cBinCat = 'F';
                if(nBinning == 1)
                    cBinCat = 'P';
                // Write HBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clHBR.SetHEAD_NUM(255);
                else
                    clHBR.SetHEAD_NUM(1);
                clHBR.SetSITE_NUM(nSiteNum);
                clHBR.SetHBIN_NUM(nBinning);
                clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
                clHBR.SetHBIN_PF(cBinCat);
                clHBR.SetHBIN_NAM("");
                clStdfParse.WriteRecord(&clHBR);
            }
        }
    }

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);

        // Write partial performance
        WritePartialPerformance((char*)"WriteSummaryRecords_Consolidated()");
    }

    return true;
}

bool GexDbPlugin_Galaxy::WriteSummaryRecords(
        GQTL_STDF::StdfParse & clStdfParse,
        const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
        const bool run_filtered,
        const GexDbPlugin_Filter& dbFilters
        )
{
    int												nBinning, nSiteNum;
    GexDbPlugin_BinMap::Iterator					itBinning;
    GexDbPlugin_Query								clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString											strQuery, strCondition, strFieldSpec;
    GQTL_STDF::Stdf_SBR_V4									clSBR;
    GQTL_STDF::Stdf_HBR_V4									clHBR;
    GQTL_STDF::Stdf_PCR_V4									clPCR;
    GQTL_STDF::Stdf_TSR_V4									clTSR;
    unsigned int									uiBinCount;

    Q_UNUSED(dbFilters);

    GSLOG(SYSLOG_SEV_DEBUG, (QString("Write Summary Records : ConsolidatedExtraction:%1 run_filtered:%2")
                             .arg(clSplitlotInfo.m_bConsolidatedExtraction?"true":"false")
                             .arg(run_filtered?"true":"false")).toLatin1().constData()
          );

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    // Check if we are in consolidated mode, if so, call dedicated function
    if (clSplitlotInfo.m_bConsolidatedExtraction)
        return WriteSummaryRecords_Consolidated(clStdfParse, clSplitlotInfo);

    ///////////////////////////////////////////////////////////////////////////////////////
    // HARDWARE BINNING (HBR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Get hard bin summary
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.hbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_summary.bin_count");
    strCondition = m_strTablePrefix + "hbin_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "hbin_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    // Add filter on Hbins ?
    if (!clSplitlotInfo.m_vHbinsToExclude.isEmpty())
    {
        for (int i=0; i<clSplitlotInfo.m_vHbinsToExclude.size(); i++)
            m_strlQuery_ValueConditions.append(
                        m_strTablePrefix + "hbin_stats_summary.hbin_no|Numeric|"+ QString("!=%1").arg(clSplitlotInfo.m_vHbinsToExclude.at(i)) ); // ?
    }

    // Build query
    bool b=Query_BuildSqlString(strQuery, false);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed");

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through binning summary
    GSLOG(SYSLOG_SEV_DEBUG, QString("Write summary records : %1 hbin_stats_summary retrieved")
          .arg( clGexDbQuery.size()).toLatin1().constData());
    uiBinCount = 0;
    while(clGexDbQuery.Next())
    {
        uiBinCount++;
        nBinning = clGexDbQuery.value(0).toInt();
        nSiteNum = clGexDbQuery.value(1).toInt();
        itBinning = m_mapHardBins.find(nBinning);
        if(itBinning != m_mapHardBins.end())
        {
            // Write HBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clHBR.SetHEAD_NUM(255);
            else
                clHBR.SetHEAD_NUM(1);
            clHBR.SetSITE_NUM(nSiteNum);
            clHBR.SetHBIN_NUM(nBinning);
            if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
            clHBR.SetHBIN_PF((*itBinning).m_cBinCat.toLatin1());
            clHBR.SetHBIN_NAM((*itBinning).m_strBinName);
            clStdfParse.WriteRecord(&clHBR);
        }
        else
        {
            char cBinCat = 'F';
            if(nBinning == 1)
                cBinCat = 'P';
            // Write HBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clHBR.SetHEAD_NUM(255);
            else
                clHBR.SetHEAD_NUM(1);
            clHBR.SetSITE_NUM(nSiteNum);
            clHBR.SetHBIN_NUM(nBinning);
            if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
            clHBR.SetHBIN_PF(cBinCat);
            clHBR.SetHBIN_NAM("");
            clStdfParse.WriteRecord(&clHBR);
        }
    }

    // If no Bin summary available, use samples
    if(uiBinCount == 0)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Write summary records : uiBinCount=0");
        // Get hard bin samples
        Query_Empty();
        strFieldSpec = "Field|";
        strFieldSpec += m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.hbin_no");
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.site_no");
        m_strlQuery_Fields.append(strFieldSpec + "hbin_stats_samples.nb_parts");
        strCondition = m_strTablePrefix + "hbin_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
        m_strlQuery_ValueConditions.append(strCondition);
        // Add filter on site # ?
        if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
        {
            strCondition = m_strTablePrefix + "hbin_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        // Add filter on Hbins ?
        if (!clSplitlotInfo.m_vHbinsToExclude.isEmpty())
        {
            for (int i=0; i<clSplitlotInfo.m_vHbinsToExclude.size(); i++)
                m_strlQuery_ValueConditions.append(
                            m_strTablePrefix + "hbin_stats_samples.hbin_no|Numeric|"+ QString("!=%1").arg(clSplitlotInfo.m_vHbinsToExclude.at(i)) ); // ?
        }

        // Build query
        b=Query_BuildSqlString(strQuery, false);
        if (!b)
            GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed!");

        clGexDbQuery.setForwardOnly(true);
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Iterate through binning samples
        GSLOG(SYSLOG_SEV_DEBUG, QString("Write summary records : %1 hbin_stats_samples.")
              .arg( clGexDbQuery.size()).toLatin1().constData());
        while(clGexDbQuery.Next())
        {
            uiBinCount++;
            nBinning = clGexDbQuery.value(0).toInt();
            nSiteNum = clGexDbQuery.value(1).toInt();
            itBinning = m_mapHardBins.find(nBinning);
            if(itBinning != m_mapHardBins.end())
            {
                // Write HBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clHBR.SetHEAD_NUM(255);
                else
                    clHBR.SetHEAD_NUM(1);
                clHBR.SetSITE_NUM(nSiteNum);
                clHBR.SetHBIN_NUM(nBinning);
                if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                    clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
                clHBR.SetHBIN_PF((*itBinning).m_cBinCat.toLatin1());
                clHBR.SetHBIN_NAM((*itBinning).m_strBinName);
                clStdfParse.WriteRecord(&clHBR);
            }
            else
            {
                char cBinCat = 'F';
                if(nBinning == 1)
                    cBinCat = 'P';
                // Write HBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clHBR.SetHEAD_NUM(255);
                else
                    clHBR.SetHEAD_NUM(1);
                clHBR.SetSITE_NUM(nSiteNum);
                clHBR.SetHBIN_NUM(nBinning);
                if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                    clHBR.SetHBIN_CNT(clGexDbQuery.value(2).toInt());
                clHBR.SetHBIN_PF(cBinCat);
                clHBR.SetHBIN_NAM("");
                clStdfParse.WriteRecord(&clHBR);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // SOFTWARE BINNING (SBR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Get soft bin summary
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_summary.sbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_summary.bin_count");
    strCondition = m_strTablePrefix + "sbin_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "sbin_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Build query
    b=Query_BuildSqlString(strQuery, false);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed");

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Query exec failed : %1")
              .arg( clGexDbQuery.lastError().text()).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through binning summary
    uiBinCount = 0;
    while(clGexDbQuery.Next())
    {
        uiBinCount++;
        nBinning = clGexDbQuery.value(0).toInt();
        nSiteNum = clGexDbQuery.value(1).toInt();
        itBinning = m_mapSoftBins.find(nBinning);
        if(itBinning != m_mapSoftBins.end())
        {
            // Write SBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clSBR.SetHEAD_NUM(255);
            else
                clSBR.SetHEAD_NUM(1);
            clSBR.SetSITE_NUM(nSiteNum);
            clSBR.SetSBIN_NUM(nBinning);
            if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                clSBR.SetSBIN_CNT(clGexDbQuery.value(2).toInt());
            clSBR.SetSBIN_PF((*itBinning).m_cBinCat.toLatin1());
            clSBR.SetSBIN_NAM((*itBinning).m_strBinName);
            clStdfParse.WriteRecord(&clSBR);
        }
        else
        {
            char cBinCat = 'F';
            if(nBinning == 1)
                cBinCat = 'P';
            // Write SBR record
            if((nSiteNum == 255) || (nSiteNum == -1))
                clSBR.SetHEAD_NUM(255);
            else
                clSBR.SetHEAD_NUM(1);
            clSBR.SetSITE_NUM(nSiteNum);
            clSBR.SetSBIN_NUM(nBinning);
            if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                clSBR.SetSBIN_CNT(clGexDbQuery.value(2).toInt());
            clSBR.SetSBIN_PF(cBinCat);
            clSBR.SetSBIN_NAM("");
            clStdfParse.WriteRecord(&clSBR);
        }
    }

    // If no Bin summary available, use samples
    if(uiBinCount == 0)
    {
        // Get soft bin samples
        Query_Empty();
        strFieldSpec = "Field|";
        strFieldSpec += m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_samples.sbin_no");
        m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_samples.site_no");
        m_strlQuery_Fields.append(strFieldSpec + "sbin_stats_samples.nb_parts");
        strCondition = m_strTablePrefix + "sbin_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
        m_strlQuery_ValueConditions.append(strCondition);
        // Add filter on site # ?
        if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
        {
            strCondition = m_strTablePrefix + "sbin_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
            m_strlQuery_ValueConditions.append(strCondition);
        }

        // Build query
        b=Query_BuildSqlString(strQuery, false);
        if (!b)
            GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed");

        clGexDbQuery.setForwardOnly(true);
        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Iterate through binning samples
        while(clGexDbQuery.Next())
        {
            uiBinCount++;
            nBinning = clGexDbQuery.value(0).toInt();
            nSiteNum = clGexDbQuery.value(1).toInt();
            itBinning = m_mapSoftBins.find(nBinning);
            if(itBinning != m_mapSoftBins.end())
            {
                // Write SBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clSBR.SetHEAD_NUM(255);
                else
                    clSBR.SetHEAD_NUM(1);
                clSBR.SetSITE_NUM(nSiteNum);
                clSBR.SetSBIN_NUM(nBinning);
                if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                    clSBR.SetSBIN_CNT(clGexDbQuery.value(2).toInt());
                clSBR.SetSBIN_PF((*itBinning).m_cBinCat.toLatin1());
                clSBR.SetSBIN_NAM((*itBinning).m_strBinName);
                clStdfParse.WriteRecord(&clSBR);
            }
            else
            {
                char cBinCat = 'F';
                if(nBinning == 1)
                    cBinCat = 'P';
                // Write SBR record
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clSBR.SetHEAD_NUM(255);
                else
                    clSBR.SetHEAD_NUM(1);
                clSBR.SetSITE_NUM(nSiteNum);
                clSBR.SetSBIN_NUM(nBinning);
                if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374
                    clSBR.SetSBIN_CNT(clGexDbQuery.value(2).toInt());
                clSBR.SetSBIN_PF(cBinCat);
                clSBR.SetSBIN_NAM("");
                clStdfParse.WriteRecord(&clSBR);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // PART COUNT (PCR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Get part count
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "parts_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "parts_stats_summary.nb_parts");
    m_strlQuery_Fields.append(strFieldSpec + "parts_stats_summary.nb_good");
    m_strlQuery_Fields.append(strFieldSpec + "parts_stats_summary.nb_rtst");
    strCondition = m_strTablePrefix + "parts_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "parts_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Build query
    b=Query_BuildSqlString(strQuery, false);
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed!");

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Write summary records : %1 parts stats summary.")
          .arg( clGexDbQuery.size()).toLatin1().constData());
    // Iterate through part counts
    while(clGexDbQuery.Next())
    {
        nSiteNum = clGexDbQuery.value(0).toInt();
        // Write PCR record
        if((nSiteNum == 255) || (nSiteNum == -1))
            clPCR.SetHEAD_NUM(255);
        else
            clPCR.SetHEAD_NUM(1);
        clPCR.SetSITE_NUM(nSiteNum);
        //if (!clSplitlotInfo.m_bConsolidatedExtraction) // 6374 : ?
        clPCR.SetPART_CNT(clGexDbQuery.value(1).toInt());
        clPCR.SetRTST_CNT(clGexDbQuery.value(3).toInt());
        clPCR.SetABRT_CNT(0);
        clPCR.SetGOOD_CNT(clGexDbQuery.value(2).toInt());
        clStdfParse.WriteRecord(&clPCR);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // GET STATISTICS (Samples and Summary)
    ///////////////////////////////////////////////////////////////////////////////////////
    // Get test stats records: PTR
    if(!GetTestStats_P(clSplitlotInfo))
        return false;
    // Get test stats records: MPR
    if(!GetTestStats_MP(clSplitlotInfo))
        return false;
    // Get test stats records: FTR
    if(!GetTestStats_F(clSplitlotInfo))
        return false;

    ///////////////////////////////////////////////////////////////////////////////////////
    // WRITE STATISTICS (TSR)
    ///////////////////////////////////////////////////////////////////////////////////////
    GexDbPlugin_TestInfoContainer					*pContainer;
    GexDbPlugin_TestInfo							*pTestInfo;
    QMap<int, GexDbPlugin_TestInfo_Stats>::Iterator	itStats;

    pContainer = m_clTestList.m_pFirstTest;

    bool bWriteStatsFromSamples=false;
    bool bWriteStatsFromSummary=false;
    while(pContainer)
    {
        pTestInfo = pContainer->m_pTestInfo;
        bWriteStatsFromSamples = bWriteStatsFromSummary = false;

        // Check which data to use for TSR (samples, summary)
        // If some samples have been extracted, no need to create TSR records from samples, as the stats can be computed directly using the samlpes,
        // so if summary is available, let's write it to the TSR, if no summary is available, no TSR are written.
        if(pTestInfo->m_bHaveSamples)
        {
            bWriteStatsFromSummary = true;
            bWriteStatsFromSamples = false;
        }
        else
        {
            switch(m_eStatsSource)
            {
            case eStatsFromSummaryOnly:
                bWriteStatsFromSummary = true;
                break;

            case eStatsFromSummaryThenSamples:
                if(!pTestInfo->m_mapStatsFromSummary.isEmpty() && pTestInfo->m_bMinimumStatsFromSummaryAvailable)
                    bWriteStatsFromSummary = true;
                else if(!pTestInfo->m_mapStatsFromSamples.isEmpty())
                    bWriteStatsFromSamples = true;
                break;

            case eStatsFromSamplesOnly:
                bWriteStatsFromSamples=true;
                break;

            case eStatsFromSamplesThenSummary:
                if(!pTestInfo->m_mapStatsFromSamples.isEmpty())
                    bWriteStatsFromSamples=true;
                else if(!pTestInfo->m_mapStatsFromSummary.isEmpty())
                    bWriteStatsFromSummary = true;
                break;
            }
        }

        if(bWriteStatsFromSummary)
        {
            for(itStats=pTestInfo->m_mapStatsFromSummary.begin(); itStats != pTestInfo->m_mapStatsFromSummary.end(); itStats++)
            {
                clTSR.Reset();
                nSiteNum = itStats.key();
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clTSR.SetHEAD_NUM(255);
                else
                    clTSR.SetHEAD_NUM(1);
                clTSR.SetSITE_NUM(nSiteNum);
                clTSR.SetTEST_TYP(pTestInfo->m_cTestType);
                clTSR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                clTSR.SetEXEC_CNT(itStats.value().m_uiExecCount);
                clTSR.SetFAIL_CNT(itStats.value().m_uiFailCount);
                clTSR.SetALRM_CNT(0);
                clTSR.SetTEST_NAM(pTestInfo->m_strTestName);
                clTSR.SetTEST_TIM(itStats.value().m_fTestTime);
                clTSR.SetSEQ_NAME(GEXDB_PLUGIN_TSR_STATS_SOURCE_SUMMARY);
                clTSR.SetTEST_LBL("");
                if(pTestInfo->m_bStatsFromSummaryComplete)
                {
                    clTSR.SetTEST_MIN(itStats.value().m_fMin);
                    clTSR.SetTEST_MAX(itStats.value().m_fMax);
                    clTSR.SetTST_SUMS(itStats.value().m_fSum);
                    clTSR.SetTST_SQRS(itStats.value().m_fSumSquare);
                }

                if(itStats.value().m_uiOptFlag != 0)
                    clTSR.SetOPT_FLAG(itStats.value().m_uiOptFlag);
                clStdfParse.WriteRecord(&clTSR);
            }
        }
        else if(bWriteStatsFromSamples)
        {
            for(itStats=pTestInfo->m_mapStatsFromSamples.begin(); itStats != pTestInfo->m_mapStatsFromSamples.end(); itStats++)
            {
                clTSR.Reset();
                nSiteNum = itStats.key();
                if((nSiteNum == 255) || (nSiteNum == -1))
                    clTSR.SetHEAD_NUM(255);
                else
                    clTSR.SetHEAD_NUM(1);
                clTSR.SetSITE_NUM(nSiteNum);
                clTSR.SetTEST_TYP(pTestInfo->m_cTestType);
                clTSR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                clTSR.SetEXEC_CNT(itStats.value().m_uiExecCount);
                clTSR.SetFAIL_CNT(itStats.value().m_uiFailCount);
                clTSR.SetALRM_CNT(0);
                clTSR.SetTEST_NAM(pTestInfo->m_strTestName);
                clTSR.SetTEST_TIM(itStats.value().m_fTestTime);
                clTSR.SetSEQ_NAME(GEXDB_PLUGIN_TSR_STATS_SOURCE_SAMPLES);
                clTSR.SetTEST_LBL("");
                clTSR.SetTEST_MIN(itStats.value().m_fMin);
                clTSR.SetTEST_MAX(itStats.value().m_fMax);
                clTSR.SetTST_SUMS(itStats.value().m_fSum);
                clTSR.SetTST_SQRS(itStats.value().m_fSumSquare);

                if(itStats.value().m_uiOptFlag != 0)
                    clTSR.SetOPT_FLAG(itStats.value().m_uiOptFlag);
                clStdfParse.WriteRecord(&clTSR);
            }
        }

        pContainer = pContainer->m_pNextTest;
    }

    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);
        // Write partial performance
        WritePartialPerformance((char*)"WriteSummaryRecords()");
    }

    return true;
}
