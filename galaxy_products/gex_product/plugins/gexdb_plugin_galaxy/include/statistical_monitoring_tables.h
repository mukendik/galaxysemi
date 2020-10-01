#ifndef SPM_TABLES_H
#define SPM_TABLES_H

#include <QString>

// Common Keys
const QString SM_ID                      = "task_id";
const QString SM_VERSION_ID              = "version_id";
const QString SM_MONITORED_ITEM_ID       = "monitored_item_id";
const QString SM_LIMIT_ID                = "limit_id";
const QString SM_LOG_ID                  = "log_id";

// SM ym_sm
const QString SM_TABLE_MAIN               = "";

// SM ym_sm_filters
const QString SM_TABLE_FILTERS       = "_filters";

const QString SM_FILTER_NAME  = "field";
const QString SM_FILTER_VALUE = "value";

// SM ym_sm_default_params
const QString SM_TABLE_DEF_PARAMS    = "_default_params";

// SM ym_sm_version
const QString SM_TABLE_VERSION       = "_version";

// SM ym_sm_limit
const QString SM_TABLE_LIMIT          = "_limit";

const QString SM_SITE_NO                 = "site_no";
const QString SM_CRIT_LVL                = "criticity_level";
const QString SM_STAT_NAME               = "stat_name";
const QString SM_LL_ENABLED              = "ll_enabled";
const QString SM_LL                      = "ll";
const QString SM_HL_ENABLED              = "hl_enabled";
const QString SM_HL                      = "hl";
const QString SM_ALGO                    = "algorithm";
const QString SM_COMP_DATAPOINTS         = "computation_datapoints";
const QString SM_COMP_OUTLIERS           = "computation_outliers";
const QString SM_ENABLED                 = "enabled";
const QString SM_RECOMPUTE               = "recompute";
const QString SM_HAS_UNIT                = "has_unit";
const QString SM_MEAN                    = "mean";
const QString SM_SIGMA                   = "sigma";
const QString SM_MEDIAN                  = "median";
const QString SM_Q1                      = "q1";
const QString SM_Q3                      = "q3";
const QString SM_NPERCENT                = "percent_n";
const QString SM_100MINUSNPERCENT        = "percent_100_min_n";

// SM ym_sm_alarm
const QString  SM_ALARM_PRODUCT        = "product";
const QString  SM_ALARM_LOT            = "lot";
const QString  SM_ALARM_SUBLOT         = "sublot";
const QString  SM_ALARM_WAFER          = "wafer";
const QString  SM_ALARM_SPLITLOT       = "splitlot";
const QString  SM_ALARM_CRIT           = "criticity";
const QString  SM_ALARM_TEST_NUM       = "test_number";
const QString  SM_ALARM_TEST_NAME      = "test_name";
const QString  SM_ALARM_SITE           = "site";
const QString  SM_ALARM_STAT           = "stat";
const QString  SM_ALARM_EXEC_COUNT     = "total_parts";
const QString  SM_ALARM_FAIL_COUNT     = "parts_in_item";
const QString  SM_ALARM_LL             = "ll";
const QString  SM_ALARM_HL             = "hl";
const QString  SM_ALARM_VALUE          = "outlier_value";
const QString  SM_ALARM_UNIT           = "unit";


// SM _sm_stat_param
const QString SM_TABLE_LIMIT_PARAM    = "_limit_param";

const QString SM_PARAM_NAME  = "param_name";
const QString SM_PARAM_VALUE = "param_value";
const QString SM_N           = "param_value";

// SM _sm_test
const QString SM_TABLE_MONITORED_ITEM    = "_monitored_item";

const QString SM_MONITORED_ITEM_TYPE = "monitored_item_type";
const QString SM_MONITORED_ITEM_NUM  = "GROUP_CONCAT(monitored_item_num SEPARATOR '&')";
const QString SM_MONITORED_ITEM_NAME = "GROUP_CONCAT(monitored_item_name SEPARATOR '&')";
const QString SM_MONITORED_ITEM_UNIT = "monitored_item_unit";
const QString SM_MONITORED_ITEM_CAT  = "GROUP_CONCAT(monitored_item_cat SEPARATOR '&')";

// SM _sm_alarm
const QString SM_TABLE_ALARM         = "_alarm";

// SM _sm_log
const QString SM_TABLE_LOG           = "_log";


// Decorated fields names
const QString SM_ENABLED_D              = "status";
const QString SM_MONITORED_ITEM_TYPE_D  = "type";
const QString SM_MONITORED_ITEM_NUM_D   = "#";
const QString SM_MONITORED_ITEM_NAME_D  = "name";
const QString SM_MONITORED_ITEM_UNIT_D  = "unit";
const QString SM_MONITORED_ITEM_CAT_D   = "category";
const QString SM_SITE_NO_D              = "site#";
const QString SM_CRIT_LVL_D             = "level";
const QString SM_STAT_NAME_D            = "stats";
const QString SM_LL_ENABLED_D           = "low limit status";
const QString SM_LL_D                   = "low limit";
const QString SM_HL_ENABLED_D           = "high limit status";
const QString SM_HL_D                   = "high limit";
const QString SM_ALGO_D                 = "algorithm";
const QString SM_COMP_REMOVED_COUNT_D   = "removed count";
const QString SM_COMP_DATAPOINTS_D      = "wafer/sublot";
const QString SM_PARAM_NAME_D           = "parameter";
const QString SM_PARAM_VALUE_D          = "value";
const QString SM_UNKNOWN_D             = "unknown";

const QString SM_ALARM_PRODUCT_D        = "product";
const QString SM_ALARM_LOT_D            = "lot";
const QString SM_ALARM_SUBLOT_D         = "sublot";
const QString SM_ALARM_WAFER_D          = "wafer";
const QString SM_ALARM_SPLITLOT_D       = "splitlot";
const QString SM_ALARM_CRIT_D           = "criticity";
const QString SM_ALARM_TEST_NUM_D       = "#";
const QString SM_ALARM_TEST_NAME_D      = "name";
const QString SM_ALARM_SITE_D           = "site";
const QString SM_ALARM_STAT_D           = "stat";
const QString SM_ALARM_EXEC_COUNT_D     = "exec_count";
const QString SM_ALARM_FAIL_COUNT_D     = "fail_count";
const QString SM_ALARM_LL_D             = "low limit";
const QString SM_ALARM_HL_D             = "high limit";
const QString SM_ALARM_VALUE_D          = "value";
const QString SM_ALARM_UNIT_D           = "unit";

const QString SM_N_D                    = "N";
const QString SM_MEAN_D                 = "mean";
const QString SM_SIGMA_D                = "sigma";
const QString SM_MEDIAN_D               = "median";
const QString SM_Q1_D                   = "Q1";
const QString SM_Q3_D                   = "Q3";
const QString SM_NPERCENT_D             = "N percentile";
const QString SM_100MINUSNPERCENT_D     = "100 - N percentile";


#endif // SPM_TABLES_H
