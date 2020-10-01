#include "gexdb_plugin_galaxy.h"

///////////////////////////////////////////////////////////
// Query: Add all splitlot fields
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::ConstructSplitlotQuery_Fields(bool bPurge)
{
    QString strCondition, strFieldSpec;

    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.lot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.sublot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.start_t");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.finish_t");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.tester_name");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.tester_type");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_flags");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_good");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.data_provider");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.data_type");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.prod_data");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.job_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.job_rev");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.oper_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.exec_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.exec_ver");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.facil_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.part_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.user_txt");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.famly_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.proc_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.spec_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.spec_ver");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_host_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_path");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_name");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.valid_splitlot");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.insertion_time");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.subcon_lot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.incremental_update");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.sya_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.day");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.week_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.month_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.quarter_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_week");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_month");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_quarter");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.site_config");
    }
    else
    {
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.lot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.sublot_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.setup_t");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.start_t");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.finish_t");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.stat_num");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.tester_name");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.tester_type");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_flags");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_good");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_samples");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_samples_good");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_summary");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_summary_good");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.data_provider");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.data_type");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.prod_data");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.test_insertion");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.retest_index");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.retest_hbins");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.rework_code");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.job_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.job_rev");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.oper_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.exec_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.exec_ver");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.test_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.facil_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.tst_temp");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.mode_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.rtst_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.prot_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.burn_tim");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.cmod_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.part_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.user_txt");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.aux_file");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.pkg_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.famly_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.date_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.floor_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.proc_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.oper_frq");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.spec_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.spec_ver");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.flow_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.setup_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.dsgn_rev");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.eng_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.rom_cod");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.serl_num");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.supr_nam");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_sites");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.head_num");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.handler_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.handler_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.card_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.card_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.loadboard_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.loadboard_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.dib_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.dib_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.cable_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.cable_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.contactor_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.contactor_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.laser_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.laser_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.extra_typ");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.extra_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_host_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_path");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.file_name");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.valid_splitlot");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.insertion_time");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.subcon_lot_id");
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
            m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.incremental_update");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.sya_id");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.day");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.week_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.month_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.quarter_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_nb");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_week");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_month");
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.year_and_quarter");
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
            m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_nb");
    }

    // Add product_name, and common conditions
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.product_name");

    if(!bPurge){
        strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
        m_strlQuery_ValueConditions.append(strCondition);
    }
}


///////////////////////////////////////////////////////////
// Query: construct query string for Splitlot query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructSplitlotQuery(GexDbPlugin_Filter &cFilters,QString & strQuery,
                                                unsigned int /*uiGexDbBuild*/,bool bClearQueryFirst/*=true*/,bool bPurge)
{
    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    if(bClearQueryFirst)
        Query_Empty();

    // Fill splitlot fields
    ConstructSplitlotQuery_Fields(bPurge);

    // Order by lot_id, start_t
    m_strlQuery_OrderFields.append(m_strTablePrefix + "splitlot.lot_id");
    m_strlQuery_OrderFields.append(m_strTablePrefix + "splitlot.start_t");

    // Set filters
    Query_AddFilters(cFilters);

    // Add time period condition
    Query_AddTimePeriodCondition(cFilters);

    // Add min part condition
    Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_PARTS, QString(">=%1").arg(QString::number(cFilters.mMinimumPartsInFile)));

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for Splitlot query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructSplitlotQuery(long lSplitlotID, QString & strQuery, bool bClearQueryFirst/*=true*/)
{
    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    QString strCondition, strFieldSpec;
    if(bClearQueryFirst)
        Query_Empty();

    // Fill splitlot fields
    ConstructSplitlotQuery_Fields();

    // Add other fields and conditions
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    strCondition = m_strTablePrefix + "splitlot.splitlot_id|Numeric|" + QString::number(lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    return true;
}

