PRAGMA foreign_keys=OFF
;




BEGIN
;




CREATE TABLE 'global_files'
(
'file_id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL CHECK ( 'file_id'>=0 )
, 'file_name' VARCHAR ( 255 ) NOT NULL
, 'file_type' VARCHAR ( 255 ) NOT NULL
, 'file_format' VARCHAR ( 255 ) NOT NULL
, 'file_content' MEDIUMBLOB NOT NULL
, 'file_checksum' INTEGER NOT NULL CHECK ( 'file_checksum'>=0 )
, 'file_last_update' DATETIME NOT NULL
)
;




CREATE TABLE 'ft_gtl_info'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'gtl_key' VARCHAR ( 255 ) NOT NULL
, 'gtl_value' VARCHAR ( 255 )
, PRIMARY KEY ( 'splitlot_id','gtl_key' )
)
;




CREATE TABLE 'ft_splitlot'
(
'splitlot_id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'lot_id' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'sublot_id' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'setup_t' INTEGER NOT NULL DEFAULT '0'
, 'start_t' INTEGER NOT NULL DEFAULT '0'
, 'finish_t' INTEGER NOT NULL DEFAULT '0'
, 'stat_num' INTEGER NOT NULL CHECK ( 'stat_num'>=0 ) DEFAULT '0'
, 'tester_name' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'tester_type' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'flags' BINARY ( 2 ) NOT NULL DEFAULT '\\0\\0'
, 'nb_parts' INTEGER NOT NULL DEFAULT '0'
, 'nb_parts_good' INTEGER NOT NULL DEFAULT '0'
, 'nb_parts_samples' INTEGER NOT NULL DEFAULT '0'
, 'nb_parts_samples_good' INTEGER NOT NULL DEFAULT '0'
, 'nb_parts_summary' INTEGER NOT NULL DEFAULT '0'
, 'nb_parts_summary_good' INTEGER NOT NULL DEFAULT '0'
, 'data_provider' VARCHAR ( 255 ) DEFAULT ''
, 'data_type' VARCHAR ( 255 ) DEFAULT ''
, 'prod_data' CHAR ( 1 ) NOT NULL DEFAULT 'Y'
, 'retest_phase' varchar(255) DEFAULT NULL
, 'retest_index' INTEGER NOT NULL CHECK ( 'retest_index'>=0 ) DEFAULT '0'
, 'retest_hbins' VARCHAR ( 255 ) DEFAULT NULL
, 'rework_code' INTEGER NOT NULL CHECK ( 'rework_code'>=0 ) DEFAULT '0'
, 'job_nam' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'job_rev' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'oper_nam' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'exec_typ' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'exec_ver' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'test_cod' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'facil_id' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'tst_temp' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'mode_cod' CHAR ( 1 ) DEFAULT NULL
, 'rtst_cod' CHAR ( 1 ) DEFAULT NULL
, 'prot_cod' CHAR ( 1 ) DEFAULT NULL
, 'burn_tim' INTEGER DEFAULT NULL
, 'cmod_cod' CHAR ( 1 ) DEFAULT NULL
, 'part_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'user_txt' VARCHAR ( 255 ) DEFAULT NULL
, 'aux_file' VARCHAR ( 255 ) DEFAULT NULL
, 'pkg_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'famly_id' VARCHAR ( 255 ) DEFAULT NULL
, 'date_cod' VARCHAR ( 255 ) DEFAULT NULL
, 'floor_id' VARCHAR ( 255 ) DEFAULT NULL
, 'proc_id' VARCHAR ( 255 ) DEFAULT NULL
, 'oper_frq' VARCHAR ( 255 ) DEFAULT NULL
, 'spec_nam' VARCHAR ( 255 ) DEFAULT NULL
, 'spec_ver' VARCHAR ( 255 ) DEFAULT NULL
, 'flow_id' VARCHAR ( 255 ) DEFAULT NULL
, 'setup_id' VARCHAR ( 255 ) DEFAULT NULL
, 'dsgn_rev' VARCHAR ( 255 ) DEFAULT NULL
, 'eng_id' VARCHAR ( 255 ) DEFAULT NULL
, 'rom_cod' VARCHAR ( 255 ) DEFAULT NULL
, 'serl_num' VARCHAR ( 255 ) DEFAULT NULL
, 'supr_nam' VARCHAR ( 255 ) DEFAULT NULL
, 'nb_sites' INTEGER NOT NULL CHECK ( 'nb_sites'>=0 ) DEFAULT '1'
, 'head_num' INTEGER CHECK ( 'head_num'>=0 ) DEFAULT NULL
, 'handler_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'handler_id' VARCHAR ( 255 ) DEFAULT NULL
, 'card_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'card_id' VARCHAR ( 255 ) DEFAULT NULL
, 'loadboard_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'loadboard_id' VARCHAR ( 255 ) DEFAULT NULL
, 'dib_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'dib_id' VARCHAR ( 255 ) DEFAULT NULL
, 'cable_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'cable_id' VARCHAR ( 255 ) DEFAULT NULL
, 'contactor_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'contactor_id' VARCHAR ( 255 ) DEFAULT NULL
, 'laser_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'laser_id' VARCHAR ( 255 ) DEFAULT NULL
, 'extra_typ' VARCHAR ( 255 ) DEFAULT NULL
, 'extra_id' VARCHAR ( 255 ) DEFAULT NULL
, 'file_host_id' INTEGER CHECK ( 'file_host_id'>=0 ) DEFAULT '0'
, 'file_path' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'file_name' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'valid_splitlot' CHAR ( 1 ) NOT NULL DEFAULT 'N'
, 'insertion_time' INTEGER NOT NULL CHECK ( 'insertion_time'>=0 ) DEFAULT '0'
, 'subcon_lot_id' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'incremental_update' VARCHAR ( 255 ) DEFAULT NULL
, 'sya_id' INTEGER CHECK ( 'sya_id'>=0 ) DEFAULT '0'
, 'day' VARCHAR ( 10 ) NOT NULL
, 'week_nb' INTEGER NOT NULL CHECK ( 'week_nb'>=0 )
, 'month_nb' INTEGER NOT NULL CHECK ( 'month_nb'>=0 )
, 'quarter_nb' INTEGER NOT NULL CHECK ( 'quarter_nb'>=0 )
, 'year_nb' INTEGER NOT NULL
, 'year_and_week' VARCHAR ( 7 ) NOT NULL
, 'year_and_month' VARCHAR ( 7 ) NOT NULL
, 'year_and_quarter' VARCHAR ( 7 ) NOT NULL
, 'recipe_id' INTEGER CHECK ( 'recipe_id'>=0 )
, CONSTRAINT 'fk_ft_splitlot_global_files1' FOREIGN KEY ( 'recipe_id' ) REFERENCES 'global_files' ( 'file_id' )
)
;




CREATE INDEX 'ft_splitlot.fk_ft_splitlot_global_files1' ON 'ft_splitlot' ( 'recipe_id' )
;




CREATE TABLE 'ft_mptest_info'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'mptest_info_id' INTEGER NOT NULL CHECK ( 'mptest_info_id'>=0 )
, 'tnum' INTEGER NOT NULL CHECK ( 'tnum'>=0 )
, 'tname' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'tpin_arrayindex' INTEGER NOT NULL CHECK ( 'tpin_arrayindex'>=0 )
, 'units' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'flags' BINARY ( 1 ) NOT NULL DEFAULT '\\0'
, 'll' FLOAT DEFAULT NULL
, 'hl' FLOAT DEFAULT NULL
, 'testseq' INTEGER CHECK ( 'testseq'>=0 ) DEFAULT NULL
, 'spec_ll' FLOAT DEFAULT NULL
, 'spec_hl' FLOAT DEFAULT NULL
, 'spec_target' FLOAT DEFAULT NULL
, 'res_scal' INTEGER DEFAULT NULL
, 'll_scal' INTEGER DEFAULT NULL
, 'hl_scal' INTEGER DEFAULT NULL
, PRIMARY KEY ( 'splitlot_id','mptest_info_id' )
, CONSTRAINT 'fk_ft_mptest_info_ft_splitlot10' FOREIGN KEY ( 'splitlot_id' ) REFERENCES 'ft_splitlot' ( 'splitlot_id' )
)
;




CREATE INDEX 'ft_mptest_info.fk_ft_mptest_info_ft_splitlot1' ON 'ft_mptest_info' ( 'splitlot_id' )
;




CREATE TABLE 'ft_hbin'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'hbin_no' INTEGER NOT NULL CHECK ( 'hbin_no'>=0 )
, 'hbin_name' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'hbin_cat' CHAR ( 1 ) DEFAULT NULL
, 'bin_family' VARCHAR ( 255 )
, 'bin_subfamily' VARCHAR ( 255 )
, PRIMARY KEY ( 'splitlot_id','hbin_no' )
, CONSTRAINT 'fk_ft_hbin_ft_splitlot1' FOREIGN KEY ( 'splitlot_id' ) REFERENCES 'ft_splitlot' ( 'splitlot_id' )
)
;




CREATE TABLE 'ft_ptest_info'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'ptest_info_id' INTEGER NOT NULL CHECK ( 'ptest_info_id'>=0 )
, 'tnum' INTEGER NOT NULL CHECK ( 'tnum'>=0 )
, 'tname' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'units' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'flags' BINARY ( 1 ) NOT NULL DEFAULT '\\0'
, 'll' FLOAT DEFAULT NULL
, 'hl' FLOAT DEFAULT NULL
, 'testseq' INTEGER CHECK ( 'testseq'>=0 ) DEFAULT NULL
, 'spec_ll' FLOAT DEFAULT NULL
, 'spec_hl' FLOAT DEFAULT NULL
, 'spec_target' FLOAT DEFAULT NULL
, 'res_scal' INTEGER DEFAULT NULL
, 'll_scal' INTEGER DEFAULT NULL
, 'hl_scal' INTEGER DEFAULT NULL
, PRIMARY KEY ( 'splitlot_id','ptest_info_id' )
, CONSTRAINT 'fk_ft_ptest_info_ft_splitlot1' FOREIGN KEY ( 'splitlot_id' ) REFERENCES 'ft_splitlot' ( 'splitlot_id' )
)
;




CREATE INDEX 'ft_ptest_info.fk_ft_ptest_info_ft_splitlot1' ON 'ft_ptest_info' ( 'splitlot_id' )
;




CREATE TABLE 'ft_sbin'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'sbin_no' INTEGER NOT NULL CHECK ( 'sbin_no'>=0 )
, 'sbin_name' VARCHAR ( 255 ) NOT NULL DEFAULT ''
, 'sbin_cat' CHAR ( 1 ) DEFAULT NULL
, 'bin_family' VARCHAR ( 255 )
, 'bin_subfamily' VARCHAR ( 255 )
, PRIMARY KEY ( 'splitlot_id','sbin_no' )
, CONSTRAINT 'fk_ft_sbin_ft_splitlot1' FOREIGN KEY ( 'splitlot_id' ) REFERENCES 'ft_splitlot' ( 'splitlot_id' )
)
;




CREATE TABLE 'ft_run'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'site_no' INTEGER NOT NULL DEFAULT '1'
, 'part_id' VARCHAR ( 255 ) DEFAULT NULL
, 'part_x' INTEGER DEFAULT NULL
, 'part_y' INTEGER DEFAULT NULL
, 'part_status' char(1) DEFAULT NULL
, 'hbin_no' INTEGER NOT NULL CHECK ( 'hbin_no'>=0 )
, 'sbin_no' INTEGER CHECK ( 'sbin_no'>=0 ) DEFAULT NULL
, 'ttime' INTEGER CHECK ( 'ttime'>=0 ) DEFAULT NULL
, 'tests_executed' INTEGER NOT NULL CHECK ( 'tests_executed'>=0 ) DEFAULT '0'
, 'tests_failed' INTEGER NOT NULL CHECK ( 'tests_failed'>=0 ) DEFAULT '0'
, 'firstfail_tnum' INTEGER CHECK ( 'firstfail_tnum'>=0 ) DEFAULT NULL
, 'firstfail_tname' VARCHAR ( 255 ) DEFAULT NULL
, 'retest_index' INTEGER NOT NULL CHECK ( 'retest_index'>=0 ) DEFAULT '0'
, 'wafer_id' VARCHAR ( 255 ) DEFAULT NULL
, 'part_txt' VARCHAR ( 255 ) DEFAULT NULL
, PRIMARY KEY ( 'splitlot_id','run_id','site_no' )
, CONSTRAINT 'fk_ft_run_ft_hbin1' FOREIGN KEY ( 'splitlot_id','hbin_no' ) REFERENCES 'ft_hbin' ( 'splitlot_id','hbin_no' )
, CONSTRAINT 'fk_ft_run_ft_sbin1' FOREIGN KEY ( 'splitlot_id','sbin_no' ) REFERENCES 'ft_sbin' ( 'splitlot_id','sbin_no' )
, CONSTRAINT 'fk_ft_run_ft_splitlot1' FOREIGN KEY ( 'splitlot_id' ) REFERENCES 'ft_splitlot' ( 'splitlot_id' )
)
;




CREATE INDEX 'ft_run.fk_ft_run_ft_hbin1' ON 'ft_run' ( 'splitlot_id','hbin_no' )
;




CREATE INDEX 'ft_run.fk_ft_run_ft_sbin1' ON 'ft_run' ( 'splitlot_id','sbin_no' )
;




CREATE TABLE 'ft_event'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'event_id' INTEGER NOT NULL
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'event_type' VARCHAR ( 255 )
, 'event_subtype' VARCHAR ( 255 )
, 'event_time_local' DATETIME
, 'event_time_utc' DATETIME
, 'event_message' BLOB
, PRIMARY KEY ( 'splitlot_id','event_id' )
, CONSTRAINT 'fk_ft_event_ft_run1' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
)
;




CREATE INDEX 'ft_event.fk_ft_event_ft_run1' ON 'ft_event' ( 'splitlot_id','run_id' )
;




CREATE TABLE 'ft_ptest_rollingstats'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'ptest_info_id' INTEGER NOT NULL CHECK ( 'ptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'distribution_shape' VARCHAR ( 255 )
, 'n_factor' FLOAT
, 't_factor' FLOAT
, 'mean' FLOAT
, 'sigma' FLOAT
, 'min' FLOAT
, 'q1' FLOAT
, 'median' FLOAT
, 'q3' FLOAT
, 'max' FLOAT
, 'exec_count' INTEGER NOT NULL CHECK ( 'exec_count'>=0 ) DEFAULT '0'
, 'fail_count' INTEGER NOT NULL CHECK ( 'fail_count'>=0 ) DEFAULT '0'
, PRIMARY KEY ( 'splitlot_id','ptest_info_id','run_id' )
, CONSTRAINT 'fk_ft_ptest_rollingstats_ft_ptest_info1' FOREIGN KEY ( 'splitlot_id','ptest_info_id' ) REFERENCES 'ft_ptest_info' ( 'splitlot_id','ptest_info_id' )
, CONSTRAINT 'fk_ft_ptest_rollingstats_ft_run1' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
)
;




CREATE INDEX 'ft_ptest_rollingstats.fk_ft_ptest_rollingstats_ft_ptest_info1' ON 'ft_ptest_rollingstats' ( 'splitlot_id','ptest_info_id' )
;




CREATE INDEX 'ft_ptest_rollingstats.fk_ft_ptest_rollingstats_ft_run1' ON 'ft_ptest_rollingstats' ( 'splitlot_id','run_id' )
;




CREATE TABLE 'ft_mptest_rollinglimits'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'mptest_info_id' INTEGER NOT NULL CHECK ( 'mptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'limit_index' INTEGER NOT NULL CHECK ( 'limit_index'>=0 )
, 'limit_type' CHAR NOT NULL
, 'limit_mode' INTEGER NOT NULL CHECK ( 'limit_mode'>=0 )
, 'LL' FLOAT DEFAULT NULL
, 'HL' FLOAT DEFAULT NULL
, PRIMARY KEY ( 'splitlot_id','mptest_info_id','run_id','limit_index' )
, CONSTRAINT 'fk_ft_mptest_rollinglimits_ft_mptest_info10' FOREIGN KEY ( 'splitlot_id','mptest_info_id' ) REFERENCES 'ft_mptest_info' ( 'splitlot_id','mptest_info_id' )
, CONSTRAINT 'fk_ft_mptest_rollinglimits_ft_run10' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
)
;




CREATE INDEX 'ft_mptest_rollinglimits.fk_ft_mptest_rollinglimits_ft_run1' ON 'ft_mptest_rollinglimits' ( 'splitlot_id','run_id' )
;




CREATE TABLE 'ft_mptest_rollingstats'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'mptest_info_id' INTEGER NOT NULL CHECK ( 'mptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'distribution_shape' VARCHAR ( 255 )
, 'n_factor' FLOAT
, 't_factor' FLOAT
, 'mean' FLOAT
, 'sigma' FLOAT
, 'min' FLOAT
, 'q1' FLOAT
, 'median' FLOAT
, 'q3' FLOAT
, 'max' FLOAT
, 'exec_count' INTEGER NOT NULL CHECK ( 'exec_count'>=0 ) DEFAULT '0'
, 'fail_count' INTEGER NOT NULL CHECK ( 'fail_count'>=0 ) DEFAULT '0'
, PRIMARY KEY ( 'splitlot_id','mptest_info_id','run_id' )
, CONSTRAINT 'fk_ft_mptest_rollingstats_ft_run10' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
, CONSTRAINT 'fk_ft_mptest_rollingstats_ft_mptest_info' FOREIGN KEY ( 'splitlot_id','mptest_info_id' ) REFERENCES 'ft_mptest_info' ( 'splitlot_id','mptest_info_id' )
)
;




CREATE INDEX 'ft_mptest_rollingstats.fk_ft_mptest_rollingstats_ft_run1' ON 'ft_mptest_rollingstats' ( 'splitlot_id','run_id' )
;




CREATE INDEX 'ft_mptest_rollingstats.fk_ft_mptest_rollingstats_ft_mptest_info' ON 'ft_mptest_rollingstats' ( 'splitlot_id','mptest_info_id' )
;




CREATE TABLE 'ft_mptest_outliers'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'mptest_info_id' INTEGER NOT NULL CHECK ( 'mptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'run_index' INTEGER NOT NULL CHECK ( 'run_index'>=0 )
, 'limits_run_id' INTEGER NOT NULL CHECK ( 'limits_run_id'>=0 )
, 'limit_type' CHAR NOT NULL
, 'value' FLOAT
, PRIMARY KEY ( 'splitlot_id','mptest_info_id','run_id','run_index' )
, CONSTRAINT 'fk_ft_mptest_outliers_ft_run10' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
, CONSTRAINT 'fk_ft_mptest_outliers_ft_mptest_info10' FOREIGN KEY ( 'splitlot_id','mptest_info_id' ) REFERENCES 'ft_mptest_info' ( 'splitlot_id','mptest_info_id' )
, CONSTRAINT 'fk_ft_mptest_outliers_ft_mptest_rollinglimits10' FOREIGN KEY ( 'splitlot_id','mptest_info_id','limits_run_id' ) REFERENCES 'ft_mptest_rollinglimits' ( 'splitlot_id','mptest_info_id','run_id' )
)
;




CREATE INDEX 'ft_mptest_outliers.fk_ft_mptest_outliers_ft_run1' ON 'ft_mptest_outliers' ( 'splitlot_id','run_id' )
;




CREATE INDEX 'ft_mptest_outliers.fk_ft_mptest_outliers_ft_ptest_info1' ON 'ft_mptest_outliers' ( 'splitlot_id','mptest_info_id' )
;




CREATE INDEX 'ft_mptest_outliers.fk_ft_ptest_outliers_ft_mptest_rollinglimits1' ON 'ft_mptest_outliers' ( 'splitlot_id','mptest_info_id','limits_run_id' )
;




CREATE TABLE 'ft_ptest_rollinglimits'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'ptest_info_id' INTEGER NOT NULL CHECK ( 'ptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'limit_index' INTEGER NOT NULL CHECK ( 'limit_index'>=0 )
, 'limit_type' CHAR NOT NULL
, 'limit_mode' INTEGER NOT NULL CHECK ( 'limit_mode'>=0 )
, 'LL' FLOAT DEFAULT NULL
, 'HL' FLOAT DEFAULT NULL
, PRIMARY KEY ( 'splitlot_id','ptest_info_id','run_id','limit_index' )
, CONSTRAINT 'fk_ft_ptest_rollinglimits_ft_ptest_info1' FOREIGN KEY ( 'splitlot_id','ptest_info_id' ) REFERENCES 'ft_ptest_info' ( 'splitlot_id','ptest_info_id' )
, CONSTRAINT 'fk_ft_ptest_rollinglimits_ft_run1' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
)
;




CREATE INDEX 'ft_ptest_rollinglimits.fk_ft_ptest_rollinglimits_ft_run1' ON 'ft_ptest_rollinglimits' ( 'splitlot_id','run_id' )
;




CREATE TABLE 'ft_ptest_outliers'
(
'splitlot_id' INTEGER NOT NULL CHECK ( 'splitlot_id'>=0 )
, 'ptest_info_id' INTEGER NOT NULL CHECK ( 'ptest_info_id'>=0 )
, 'run_id' INTEGER NOT NULL CHECK ( 'run_id'>=0 )
, 'run_index' INTEGER NOT NULL CHECK ( 'run_index'>=0 )
, 'limits_run_id' INTEGER NOT NULL CHECK ( 'limits_run_id'>=0 )
, 'limit_type' CHAR NOT NULL
, 'value' FLOAT
, PRIMARY KEY ( 'splitlot_id','ptest_info_id','run_id','run_index' )
, CONSTRAINT 'fk_ft_ptest_outliers_ft_run1' FOREIGN KEY ( 'splitlot_id','run_id' ) REFERENCES 'ft_run' ( 'splitlot_id','run_id' )
, CONSTRAINT 'fk_ft_ptest_outliers_ft_ptest_info1' FOREIGN KEY ( 'splitlot_id','ptest_info_id' ) REFERENCES 'ft_ptest_info' ( 'splitlot_id','ptest_info_id' )
, CONSTRAINT 'fk_ft_ptest_outliers_ft_ptest_rollinglimits1' FOREIGN KEY ( 'splitlot_id','ptest_info_id','limits_run_id' ) REFERENCES 'ft_ptest_rollinglimits' ( 'splitlot_id','ptest_info_id','run_id' )
)
;




CREATE INDEX 'ft_ptest_outliers.fk_ft_ptest_outliers_ft_run1' ON 'ft_ptest_outliers' ( 'splitlot_id','run_id' )
;




CREATE INDEX 'ft_ptest_outliers.fk_ft_ptest_outliers_ft_ptest_info1' ON 'ft_ptest_outliers' ( 'splitlot_id','ptest_info_id' )
;




CREATE INDEX 'ft_ptest_outliers.fk_ft_ptest_outliers_ft_ptest_rollinglimits1' ON 'ft_ptest_outliers' ( 'splitlot_id','ptest_info_id','limits_run_id' )
;




COMMIT
;
