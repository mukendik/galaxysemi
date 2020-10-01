#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include <gqtl_log.h>

//////////////////////////////////////////////////////////////////////
// Fill Splitlot info object for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::FillSplitlotInfo(GexDbPlugin_Query & clGexDbQuery_Splitlot,
                                          GexDbPlugin_Galaxy_SplitlotInfo *pclSplitlotInfo,
                                          const GexDbPlugin_Filter & cFilters,
                                          const QVector<int>* pvHbinsToExclude/*=NULL*/)
{
    if (!pclSplitlotInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "FillSplitlotInfo : SplitlotInfo NULL");
        return "error : SplitlotInfo NULL";
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Fill splitlot info for %1...").arg( m_strTestingStage).toLatin1().constData());
    int		nIndex = 0;

    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        pclSplitlotInfo->m_lSplitlotID				= clGexDbQuery_Splitlot.value(nIndex++).toLongLong();	// <splitlot table>.SPLITLOT_ID
        pclSplitlotInfo->m_strLotID					= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.LOT_ID
        pclSplitlotInfo->m_strSublotID				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.SUBLOT_ID
        pclSplitlotInfo->m_uiStartTime				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.START_T
        pclSplitlotInfo->m_uiFinishTime				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.FINISH_T
        pclSplitlotInfo->m_strTesterName			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.TESTER_NAME
        pclSplitlotInfo->m_strTesterType			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.TESTER_TYPE
        pclSplitlotInfo->m_uiFlags					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.SPLITLOT_FLAGS
        pclSplitlotInfo->m_uiNbParts				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.NB_PARTS
        pclSplitlotInfo->m_uiNbParts_Good			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.NB_PARTS_GOOD
        pclSplitlotInfo->m_strDataProvider			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.DATA_PROVIDER
        pclSplitlotInfo->m_strDataType				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.DATA_TYPE
        pclSplitlotInfo->m_cProdData				= clGexDbQuery_Splitlot.GetChar(nIndex++);				// <splitlot table>.PROD_DATA
        pclSplitlotInfo->m_strJobName				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.JOB_NAME
        pclSplitlotInfo->m_strJobRev				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.JOB_REV
        pclSplitlotInfo->m_strOperator				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.OPER_NAM
        pclSplitlotInfo->m_strExecType				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.EXEC_TYP
        pclSplitlotInfo->m_strExecVer				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.EXEC_VER
        pclSplitlotInfo->m_strFacilID				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.FACIL_ID
        pclSplitlotInfo->m_strPartTyp				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.PART_TYP
        pclSplitlotInfo->m_strUserTxt				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.USER_TXT
        pclSplitlotInfo->m_strFamlyID				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.FAMLY_ID
        pclSplitlotInfo->m_strProcID				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.PROC_ID
        pclSplitlotInfo->m_strSpecNam				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.SPEC_NAM
        pclSplitlotInfo->m_strSpecVer				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.SPEC_VER
        pclSplitlotInfo->m_uiFileHostID				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.FILE_HOST_ID
        pclSplitlotInfo->m_strFilePath				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.FILE_PATH
        pclSplitlotInfo->m_strFileName				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.FILE_NAME
        pclSplitlotInfo->m_cValidSplitlot			= clGexDbQuery_Splitlot.GetChar(nIndex++);				// <splitlot table>.VALID_SPLITLOT
        pclSplitlotInfo->m_uiInsertionTime			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.INSERTION_TIME
        pclSplitlotInfo->m_strSubconLotID			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.SUBCON_LOT_ID
        pclSplitlotInfo->m_strWaferID				= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.WAFER_ID
        pclSplitlotInfo->m_strIncremental_Update	= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.INCREMENTAL_UPDATE
        pclSplitlotInfo->m_lSyaID					= clGexDbQuery_Splitlot.value(nIndex++).toLongLong();	// <splitlot table>.SYA_ID
        pclSplitlotInfo->m_strDay					= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.DAY
        pclSplitlotInfo->m_uiWeekNb					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.WEEK_NB
        pclSplitlotInfo->m_uiMonthNb				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.MONTH_NB
        pclSplitlotInfo->m_uiQuarterNb				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.QUARTER_NB
        pclSplitlotInfo->m_uiYearNb					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();		// <splitlot table>.YEAR_NB
        pclSplitlotInfo->m_strYearAndWeek			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.YEAR_AND_WEEK
        pclSplitlotInfo->m_strYearAndMonth			= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.YEAR_AND_MONTH
        pclSplitlotInfo->m_strYearAndQuarter		= clGexDbQuery_Splitlot.value(nIndex++).toString();		// <splitlot table>.YEAR_AND_QUARTER
        pclSplitlotInfo->m_nWaferNb	= -1;
        if(!clGexDbQuery_Splitlot.value(nIndex).isNull())
            pclSplitlotInfo->m_nWaferNb				= clGexDbQuery_Splitlot.value(nIndex).toInt();			// <splitlot table>.WAFER_NB
        nIndex++;
        pclSplitlotInfo->m_strEtestSiteConfig		= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.ETEST_SITE_CONFIG
    }
    else
    {
        pclSplitlotInfo->m_lSplitlotID				= clGexDbQuery_Splitlot.value(nIndex++).toLongLong();		// <splitlot table>.SPLITLOT_ID
        pclSplitlotInfo->m_strLotID					= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.LOT_ID
        pclSplitlotInfo->m_strSublotID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SUBLOT_ID
        pclSplitlotInfo->m_uiSetupTime				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.SETUP_T
        pclSplitlotInfo->m_uiStartTime				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.START_T
        pclSplitlotInfo->m_uiFinishTime				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.FINISH_T
        pclSplitlotInfo->m_uiStationNb				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.STAT_NUM
        pclSplitlotInfo->m_strTesterName			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.TESTER_NAME
        pclSplitlotInfo->m_strTesterType			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.TESTER_TYPE
        pclSplitlotInfo->m_uiFlags					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.SPLITLOT_FLAGS
        pclSplitlotInfo->m_uiNbParts				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.NB_PARTS
        pclSplitlotInfo->m_uiNbParts_Good			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.NB_PARTS_GOOD
        pclSplitlotInfo->m_uiNbPartsSamples			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// (14) <splitlot table>.NB_PARTS_SAMPLES
        pclSplitlotInfo->m_uiNbPartsSamples_Good	= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.NB_PARTS_SAPLES_GOOD
        pclSplitlotInfo->m_uiNbPartsSummary			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// (16) <splitlot table>.NB_PARTS_SUMMARY
        pclSplitlotInfo->m_uiNbPartsSummary_Good	= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.NB_PARTS_SUMMARY_GOOD
        pclSplitlotInfo->m_strDataProvider			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DATA_PROVIDER
        pclSplitlotInfo->m_strDataType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DATA_TYPE
        pclSplitlotInfo->m_cProdData				= clGexDbQuery_Splitlot.GetChar(nIndex++);	// <splitlot table>.PROD_DATA
        pclSplitlotInfo->m_strRetestPhase			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.TEST_INSERTION
        pclSplitlotInfo->m_uiRetestIndex			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.RETEST_INDEX
        pclSplitlotInfo->m_strRetestedHbins			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.RETEST_HBINS
        pclSplitlotInfo->m_uiReworkCode				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.REWORK_CODE
        pclSplitlotInfo->m_strJobName				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.JOB_NAME
        pclSplitlotInfo->m_strJobRev				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.JOB_REV
        pclSplitlotInfo->m_strOperator				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.OPER_NAM
        pclSplitlotInfo->m_strExecType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.EXEC_TYP
        pclSplitlotInfo->m_strExecVer				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.EXEC_VER
        pclSplitlotInfo->m_strTestCode				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.TEST_COD
        pclSplitlotInfo->m_strFacilID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FACIL_ID
        pclSplitlotInfo->m_strTestTemp				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.TST_TEMP
        pclSplitlotInfo->m_cModeCod					= clGexDbQuery_Splitlot.GetChar(nIndex++);	// <splitlot table>.MODE_COD
        pclSplitlotInfo->m_cRtstCod					= clGexDbQuery_Splitlot.GetChar(nIndex++);	// <splitlot table>.RTST_COD
        pclSplitlotInfo->m_cProtCod					= clGexDbQuery_Splitlot.GetChar(nIndex++);	// <splitlot table>.PROT_COD
        pclSplitlotInfo->m_uiBurnTim				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.BURN_TIM;
        pclSplitlotInfo->m_cCmodCod					= clGexDbQuery_Splitlot.GetChar(nIndex++);	// <splitlot table>.CMOD_COD
        pclSplitlotInfo->m_strPartTyp				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.PART_TYP
        pclSplitlotInfo->m_strUserTxt				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.USER_TXT
        pclSplitlotInfo->m_strAuxFile				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.AUX_FILE
        pclSplitlotInfo->m_strPkgTyp				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.PKG_TYP
        pclSplitlotInfo->m_strFamlyID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FAMLY_ID
        pclSplitlotInfo->m_strDateCod				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DATE_COD
        pclSplitlotInfo->m_strFloorID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FLOOR_ID
        pclSplitlotInfo->m_strProcID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.PROC_ID
        pclSplitlotInfo->m_strOperFrq				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.OPER_FRQ
        pclSplitlotInfo->m_strSpecNam				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SPEC_NAM
        pclSplitlotInfo->m_strSpecVer				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SPEC_VER
        pclSplitlotInfo->m_strFlowID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FLOW_ID
        pclSplitlotInfo->m_strSetupID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SETUP_ID
        pclSplitlotInfo->m_strDsgnRev				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DSGN_REV
        pclSplitlotInfo->m_strEngID					= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.ENG_ID
        pclSplitlotInfo->m_strRomCod				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.ROM_COD
        pclSplitlotInfo->m_strSerlNum				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SERL_NUM
        pclSplitlotInfo->m_strSuprNam				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SUPR_NAM
        pclSplitlotInfo->m_uiNbSites				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.NB_SITES
        pclSplitlotInfo->m_uiHeadNum				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.HEAD_NUM
        pclSplitlotInfo->m_strHandlerType			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.HANDLER_TYP
        pclSplitlotInfo->m_strHandlerID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.HANDLER_ID
        pclSplitlotInfo->m_strCardType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CARD_TYP
        pclSplitlotInfo->m_strCardID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CARD_ID
        pclSplitlotInfo->m_strLoadboardType			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.LOADBOARD_TYP
        pclSplitlotInfo->m_strLoadboardID			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.LOADBOARD_ID
        pclSplitlotInfo->m_strDibType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DIB_TYP
        pclSplitlotInfo->m_strDibID					= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DIB_ID
        pclSplitlotInfo->m_strCableType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CABLE_TYP
        pclSplitlotInfo->m_strCableID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CABLE_ID
        pclSplitlotInfo->m_strContactorType			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CONTACTOR_TYP
        pclSplitlotInfo->m_strContactorID			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.CONTACTOR_ID
        pclSplitlotInfo->m_strLaserType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.LASER_TYP
        pclSplitlotInfo->m_strLaserID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.LASER_ID
        pclSplitlotInfo->m_strExtraType				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.EXTRA_TYP
        pclSplitlotInfo->m_strExtraID				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.EXTRA_ID
        pclSplitlotInfo->m_uiFileHostID				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.FILE_HOST_ID
        pclSplitlotInfo->m_strFilePath				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FILE_PATH
        pclSplitlotInfo->m_strFileName				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.FILE_NAME
        pclSplitlotInfo->m_cValidSplitlot			= clGexDbQuery_Splitlot.GetChar(nIndex++);					// <splitlot table>.VALID_SPLITLOT
        pclSplitlotInfo->m_uiInsertionTime			= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.INSERTION_TIME
        pclSplitlotInfo->m_strSubconLotID			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.SUBCON_LOT_ID
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
            pclSplitlotInfo->m_strWaferID			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.WAFER_ID
        pclSplitlotInfo->m_strIncremental_Update	= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.INCREMENTAL_UPDATE
        pclSplitlotInfo->m_lSyaID					= clGexDbQuery_Splitlot.value(nIndex++).toLongLong();		// <splitlot table>.SYA_ID
        pclSplitlotInfo->m_strDay					= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.DAY
        pclSplitlotInfo->m_uiWeekNb					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.WEEK_NB
        pclSplitlotInfo->m_uiMonthNb				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.MONTH_NB
        pclSplitlotInfo->m_uiQuarterNb				= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.QUARTER_NB
        pclSplitlotInfo->m_uiYearNb					= clGexDbQuery_Splitlot.value(nIndex++).toUInt();			// <splitlot table>.YEAR_NB
        pclSplitlotInfo->m_strYearAndWeek			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.YEAR_AND_WEEK
        pclSplitlotInfo->m_strYearAndMonth			= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.YEAR_AND_MONTH
        pclSplitlotInfo->m_strYearAndQuarter		= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <splitlot table>.YEAR_AND_QUARTER
        pclSplitlotInfo->m_nWaferNb = -1;
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        {
            if(!clGexDbQuery_Splitlot.value(nIndex).isNull())
                pclSplitlotInfo->m_nWaferNb			= clGexDbQuery_Splitlot.value(nIndex).toInt();				// <splitlot table>.WAFER_NB
            nIndex++;
        }
    }

    // GCORE-1200: Checked [SC]
    pclSplitlotInfo->m_strProductName				= clGexDbQuery_Splitlot.value(nIndex++).toString();			// <sublot or wafer>.PRODUCT_NAME

    // Site Filter
    pclSplitlotInfo->m_strSiteFilterValue			= cFilters.strSiteFilterValue;

    /*
    pclSplitlotInfo->m_gexQueries.clear();
    pclSplitlotInfo->m_gexQueries = cFilters.m_gexQueries;
    */

    for(int i=0; i<cFilters.m_gexQueries.size(); i++ )
    {
        const QList< QString > ls=cFilters.m_gexQueries.at(i);
        if (ls.size()==0)
            continue;

        //pclSplitlotInfo->mAttributes.insert(ls.at(0), ls.size()>1?QVariant(ls.at(1)):QVariant());
        //if (ls.size()>2)
          //  pclSplitlotInfo->mAttributes.insert(ls.at(1), QVariant(ls.at(2)));

    }

    // For consolidated mode
    pclSplitlotInfo->m_bConsolidatedExtraction		= cFilters.bConsolidatedExtraction;
    if(pvHbinsToExclude)
        pclSplitlotInfo->m_vHbinsToExclude			= (*pvHbinsToExclude);

  return "ok";
}


