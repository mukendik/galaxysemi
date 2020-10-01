CREATE TABLE "az_consolidated_tl" (
  "ft_tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "ft_product_name" varchar(255) NOT NULL DEFAULT '',
  "ft_lot_id" varchar(255) NOT NULL DEFAULT '',
  "ft_sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "ft_dib_id" varchar(255) DEFAULT NULL,
  "ft_dib_typ" varchar(255) DEFAULT NULL,
  "ft_data_provider" varchar(255) DEFAULT NULL,
  "ft_data_type" varchar(255) DEFAULT NULL,
  "ft_facil_id" varchar(255) NOT NULL DEFAULT '',
  "ft_famly_id" varchar(255) DEFAULT NULL,
  "ft_floor_id" varchar(255) DEFAULT NULL,
  "ft_oper_frq" varchar(255) DEFAULT NULL,
  "ft_loadboard_id" varchar(255) DEFAULT NULL,
  "ft_loadboard_typ" varchar(255) DEFAULT NULL,
  "ft_oper_nam" varchar(255) NOT NULL DEFAULT '',
  "ft_pkg_typ" varchar(255) DEFAULT NULL,
  "ft_handler_id" varchar(255) DEFAULT NULL,
  "ft_handler_typ" varchar(255) DEFAULT NULL,
  "ft_proc_id" varchar(255) DEFAULT NULL,
  "ft_job_nam" varchar(255) NOT NULL DEFAULT '',
  "ft_job_rev" varchar(255) NOT NULL DEFAULT '',
  "ft_tst_temp" varchar(255) NOT NULL DEFAULT '',
  "ft_tester_name" varchar(255) NOT NULL DEFAULT '',
  "ft_tester_type" varchar(255) NOT NULL DEFAULT '',
  "ft_test_cod" varchar(255) NOT NULL DEFAULT '',
  "ft_die_id" varchar(255) DEFAULT NULL,
  "ft_wt_product_id" varchar(255) DEFAULT NULL,
  "ft_wt_sublot_id" varchar(255) DEFAULT NULL,
  "ft_wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  "wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  "wt_product_name" varchar(255) DEFAULT NULL,
  "wt_lot_id" varchar(255) DEFAULT NULL,
  "wt_sublot_id" varchar(255) DEFAULT NULL,
  "wt_wafer_id" varchar(255) DEFAULT NULL,
  "wt_dib_id" varchar(255) DEFAULT NULL,
  "wt_dib_typ" varchar(255) DEFAULT NULL,
  "wt_data_provider" varchar(255) DEFAULT NULL,
  "wt_data_type" varchar(255) DEFAULT NULL,
  "wt_facil_id" varchar(255) DEFAULT NULL,
  "wt_famly_id" varchar(255) DEFAULT NULL,
  "wt_floor_id" varchar(255) DEFAULT NULL,
  "wt_oper_frq" varchar(255) DEFAULT NULL,
  "wt_loadboard_id" varchar(255) DEFAULT NULL,
  "wt_loadboard_typ" varchar(255) DEFAULT NULL,
  "wt_oper_nam" varchar(255) DEFAULT NULL,
  "wt_pkg_typ" varchar(255) DEFAULT NULL,
  "wt_handler_id" varchar(255) DEFAULT NULL,
  "wt_handler_typ" varchar(255) DEFAULT NULL,
  "wt_proc_id" varchar(255) DEFAULT NULL,
  "wt_job_nam" varchar(255) DEFAULT NULL,
  "wt_job_rev" varchar(255) DEFAULT NULL,
  "wt_tst_temp" varchar(255) DEFAULT NULL,
  "wt_tester_name" varchar(255) DEFAULT NULL,
  "wt_tester_type" varchar(255) DEFAULT NULL,
  "wt_test_cod" varchar(255) DEFAULT NULL,
  "production_stage" varchar(2) NOT NULL DEFAULT '',
  "nb_parts" decimal(52,0) DEFAULT NULL,
  "gross_die" decimal(52,0) DEFAULT NULL,
  "nb_parts_good" decimal(52,0) DEFAULT NULL,
  PRIMARY KEY ("ft_tracking_lot_id","ft_product_name","production_stage")
);
CREATE TABLE "az_consolidated_tl_data" (
  "ft_tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "ft_product_name" varchar(255) NOT NULL DEFAULT '',
  "ft_lot_id" varchar(255) NOT NULL DEFAULT '',
  "ft_sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "ft_dib_id" varchar(255) DEFAULT NULL,
  "ft_dib_typ" varchar(255) DEFAULT NULL,
  "ft_data_provider" varchar(255) DEFAULT NULL,
  "ft_data_type" varchar(255) DEFAULT NULL,
  "ft_facil_id" varchar(255) NOT NULL DEFAULT '',
  "ft_famly_id" varchar(255) DEFAULT NULL,
  "ft_floor_id" varchar(255) DEFAULT NULL,
  "ft_oper_frq" varchar(255) DEFAULT NULL,
  "ft_loadboard_id" varchar(255) DEFAULT NULL,
  "ft_loadboard_typ" varchar(255) DEFAULT NULL,
  "ft_oper_nam" varchar(255) NOT NULL DEFAULT '',
  "ft_pkg_typ" varchar(255) DEFAULT NULL,
  "ft_handler_id" varchar(255) DEFAULT NULL,
  "ft_handler_typ" varchar(255) DEFAULT NULL,
  "ft_proc_id" varchar(255) DEFAULT NULL,
  "ft_job_nam" varchar(255) NOT NULL DEFAULT '',
  "ft_job_rev" varchar(255) NOT NULL DEFAULT '',
  "ft_tst_temp" varchar(255) NOT NULL DEFAULT '',
  "ft_tester_name" varchar(255) NOT NULL DEFAULT '',
  "ft_tester_type" varchar(255) NOT NULL DEFAULT '',
  "ft_test_cod" varchar(255) NOT NULL DEFAULT '',
  "ft_die_id" varchar(255) DEFAULT NULL,
  "ft_wt_product_id" varchar(255) DEFAULT NULL,
  "ft_wt_sublot_id" varchar(255) DEFAULT NULL,
  "ft_wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  "wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  "wt_product_name" varchar(255) DEFAULT NULL,
  "wt_lot_id" varchar(255) DEFAULT NULL,
  "wt_sublot_id" varchar(255) DEFAULT NULL,
  "wt_wafer_id" varchar(255) DEFAULT NULL,
  "wt_dib_id" varchar(255) DEFAULT NULL,
  "wt_dib_typ" varchar(255) DEFAULT NULL,
  "wt_data_provider" varchar(255) DEFAULT NULL,
  "wt_data_type" varchar(255) DEFAULT NULL,
  "wt_facil_id" varchar(255) DEFAULT NULL,
  "wt_famly_id" varchar(255) DEFAULT NULL,
  "wt_floor_id" varchar(255) DEFAULT NULL,
  "wt_oper_frq" varchar(255) DEFAULT NULL,
  "wt_loadboard_id" varchar(255) DEFAULT NULL,
  "wt_loadboard_typ" varchar(255) DEFAULT NULL,
  "wt_oper_nam" varchar(255) DEFAULT NULL,
  "wt_pkg_typ" varchar(255) DEFAULT NULL,
  "wt_handler_id" varchar(255) DEFAULT NULL,
  "wt_handler_typ" varchar(255) DEFAULT NULL,
  "wt_proc_id" varchar(255) DEFAULT NULL,
  "wt_job_nam" varchar(255) DEFAULT NULL,
  "wt_job_rev" varchar(255) DEFAULT NULL,
  "wt_tst_temp" varchar(255) DEFAULT NULL,
  "wt_tester_name" varchar(255) DEFAULT NULL,
  "wt_tester_type" varchar(255) DEFAULT NULL,
  "wt_test_cod" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("ft_tracking_lot_id","ft_product_name")
);
CREATE TABLE "az_consolidated_tl_facts" (
  "ft_tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "ft_product_name" varchar(255) NOT NULL,
  "production_stage" varchar(2) NOT NULL DEFAULT '',
  "nb_parts" decimal(52,0) DEFAULT NULL,
  "gross_die" decimal(52,0) DEFAULT NULL,
  "nb_parts_good" decimal(52,0) DEFAULT NULL,
  PRIMARY KEY ("ft_tracking_lot_id","ft_product_name","production_stage")
);
CREATE TABLE "background_transfer_history" (
  "update_id" int(11) NOT NULL,
  "min_index" int(10)  DEFAULT NULL,
  "max_index" int(10)  DEFAULT NULL,
  "current_index" int(10)  DEFAULT NULL,
  "current_sid" int(10)  DEFAULT NULL,
  "start_time" datetime DEFAULT NULL,
  "end_time" datetime DEFAULT NULL,
  "status" varchar(45) DEFAULT NULL,
  PRIMARY KEY ("update_id")
);
CREATE TABLE "background_transfer_logs" (
  "log_id" int(11) NOT NULL ,
  "update_id" int(11) NOT NULL,
  "date" datetime DEFAULT NULL,
  "action" varchar(255) DEFAULT NULL,
  "description" text,
  "status" varchar(45) DEFAULT NULL,
  "message" text,
  PRIMARY KEY ("log_id")
);
CREATE TABLE "background_transfer_partitions" (
  "update_id" int(11) NOT NULL,
  "table_name" varchar(255) NOT NULL,
  "partition_name" varchar(255) NOT NULL,
  "min_index" int(11) NOT NULL,
  "max_index" int(11) NOT NULL,
  "start_time" datetime DEFAULT NULL,
  "end_time" datetime DEFAULT NULL,
  "status" varchar(45) DEFAULT NULL,
  PRIMARY KEY ("update_id","table_name","partition_name")
);
CREATE TABLE "background_transfer_settings" (
  "update_id" int(11) NOT NULL,
  "setting_key" varchar(255) NOT NULL,
  "setting_value" varchar(255) NOT NULL,
  PRIMARY KEY ("update_id","setting_key","setting_value")
);
CREATE TABLE "background_transfer_tables" (
  "update_id" int(11) NOT NULL,
  "table_name" varchar(255) NOT NULL,
  "min_index" int(11) NOT NULL,
  "max_index" int(11) NOT NULL,
  "current_index" int(11) DEFAULT NULL,
  "current_sid" int(10)  DEFAULT NULL,
  "start_time" datetime DEFAULT NULL,
  "end_time" datetime DEFAULT NULL,
  "status" varchar(45) DEFAULT NULL,
  PRIMARY KEY ("update_id","table_name")
);
CREATE TABLE "background_transfer_tables_description" (
  "update_id" int(11) NOT NULL,
  "build_version" int(11) NOT NULL,
  "table_name" varchar(255) NOT NULL,
  "ordinal_position" int(11) DEFAULT NULL,
  "column_name" varchar(255) NOT NULL,
  "expression" text,
  PRIMARY KEY ("update_id","table_name","column_name")
);
CREATE TABLE "et_dtr" (
  "splitlot_id" int(10)  NOT NULL,
  "run_id" mediumint(7) NOT NULL DEFAULT '0',
  "order_id" mediumint(8)  NOT NULL,
  "dtr_type" varchar(255) NOT NULL,
  "dtr_text" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("splitlot_id","run_id","order_id")
);
CREATE TABLE "et_hbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","hbin_no")
);
CREATE TABLE "et_lot" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" int(10) NOT NULL DEFAULT '0',
  "nb_parts_good" int(10) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "et_lot_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","hbin_no")
);
CREATE TABLE "et_lot_metadata" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "et_lot_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","sbin_no")
);
CREATE TABLE "et_metadata_link" (
  "link_name" varchar(255) NOT NULL,
  "gexdb_table1_name" varchar(255) NOT NULL,
  "gexdb_field1_fullname" varchar(255) NOT NULL,
  "gexdb_table2_name" varchar(255) NOT NULL,
  "gexdb_field2_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("link_name")
);
CREATE TABLE "et_metadata_mapping" (
  "meta_name" varchar(255) NOT NULL,
  "gex_name" varchar(255) DEFAULT NULL,
  "gexdb_table_name" varchar(255) NOT NULL,
  "gexdb_field_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  "gex_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "bintype_field" char(1) NOT NULL DEFAULT 'N',
  "time_field" char(1) NOT NULL DEFAULT 'N',
  "custom_field" char(1) NOT NULL DEFAULT 'Y',
  "numeric_field" char(1) NOT NULL DEFAULT 'N',
  "fact_field" char(1) NOT NULL DEFAULT 'N',
  "consolidated_field" char(1) NOT NULL DEFAULT 'Y',
  "er_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "az_field" char(1) NOT NULL DEFAULT 'N',
  PRIMARY KEY ("meta_name")
);
CREATE TABLE "et_prod_alarm" (
  "splitlot_id" int(10)  NOT NULL,
  "alarm_cat" varchar(255) NOT NULL,
  "alarm_type" varchar(255) NOT NULL,
  "item_no" int(10)  NOT NULL,
  "item_name" varchar(255) DEFAULT NULL,
  "flags" binary(2) NOT NULL,
  "lcl" float NOT NULL DEFAULT '0',
  "ucl" float NOT NULL DEFAULT '0',
  "value" float NOT NULL DEFAULT '0',
  "units" varchar(10) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","alarm_cat","alarm_type","item_no")
);
CREATE TABLE "et_product_hbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "et_product_sbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "et_ptest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "units" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  "testseq" smallint(5)  DEFAULT NULL,
  "spec_ll" float DEFAULT NULL,
  "spec_hl" float DEFAULT NULL,
  "spec_target" float DEFAULT NULL,
  "res_scal" tinyint(3) DEFAULT NULL,
  "ll_scal" tinyint(3) DEFAULT NULL,
  "hl_scal" tinyint(3) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id")
);
CREATE TABLE "et_ptest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" smallint(5)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "value" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id","testseq")
);
CREATE TABLE "et_ptest_stats" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "exec_count" smallint(5)  DEFAULT NULL,
  "fail_count" smallint(5)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id")
);
CREATE TABLE "et_run" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "run_id" smallint(5)  NOT NULL DEFAULT '0',
  "part_id" varchar(255) DEFAULT NULL,
  "part_x" smallint(6) DEFAULT NULL,
  "part_y" smallint(6) DEFAULT NULL,
  "part_status" char(1) DEFAULT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  "tests_executed" smallint(5)  NOT NULL DEFAULT '0',
  "tests_failed" smallint(5)  NOT NULL DEFAULT '0',
  "firstfail_tnum" int(10)  DEFAULT NULL,
  "firstfail_tname" varchar(255) DEFAULT NULL,
  "retest_index" tinyint(3)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "part_txt" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","run_id")
);
CREATE TABLE "et_sbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","sbin_no")
);
CREATE TABLE "et_sbl" (
  "sya_id" int(10)  NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "rule_type" varchar(255) DEFAULT NULL,
  "n1_parameter" float DEFAULT NULL,
  "n2_parameter" float DEFAULT NULL,
  PRIMARY KEY ("sya_id","bin_no")
);
CREATE TABLE "et_sdr" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_grp" smallint(5)  NOT NULL DEFAULT '0',
  "site_index" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5)  NOT NULL DEFAULT '0',
  "hand_typ" varchar(255) DEFAULT NULL,
  "hand_id" varchar(255) DEFAULT NULL,
  "card_typ" varchar(255) DEFAULT NULL,
  "card_id" varchar(255) DEFAULT NULL,
  "load_typ" varchar(255) DEFAULT NULL,
  "load_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "dib_id" varchar(255) DEFAULT NULL,
  "cabl_typ" varchar(255) DEFAULT NULL,
  "cabl_id" varchar(255) DEFAULT NULL,
  "cont_typ" varchar(255) DEFAULT NULL,
  "cont_id" varchar(255) DEFAULT NULL,
  "lasr_typ" varchar(255) DEFAULT NULL,
  "lasr_id" varchar(255) DEFAULT NULL,
  "extr_typ" varchar(255) DEFAULT NULL,
  "extr_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","site_grp","site_no")
);
CREATE TABLE "et_splitlot" (
  "splitlot_id" int(10)  NOT NULL ,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) NOT NULL DEFAULT '0',
  "finish_t" int(10) NOT NULL DEFAULT '0',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(2) NOT NULL DEFAULT '\0\0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "data_provider" varchar(255) DEFAULT '',
  "data_type" varchar(255) DEFAULT '',
  "prod_data" char(1) NOT NULL DEFAULT 'Y',
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "exec_typ" varchar(255) NOT NULL DEFAULT '',
  "exec_ver" varchar(255) NOT NULL DEFAULT '',
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "part_typ" varchar(255) DEFAULT NULL,
  "user_txt" varchar(255) DEFAULT NULL,
  "famly_id" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "file_host_id" int(10)  DEFAULT '0',
  "file_path" varchar(255) NOT NULL DEFAULT '',
  "file_name" varchar(255) NOT NULL DEFAULT '',
  "valid_splitlot" char(1) NOT NULL DEFAULT 'N',
  "insertion_time" int(10)  NOT NULL DEFAULT '0',
  "subcon_lot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) DEFAULT NULL,
  "incremental_update" varchar(255) DEFAULT NULL,
  "sya_id" int(10)  DEFAULT '0',
  "day" varchar(10) NOT NULL,
  "week_nb" tinyint(2)  NOT NULL,
  "month_nb" tinyint(2)  NOT NULL,
  "quarter_nb" tinyint(1)  NOT NULL,
  "year_nb" smallint(4) NOT NULL,
  "year_and_week" varchar(7) NOT NULL,
  "year_and_month" varchar(7) NOT NULL,
  "year_and_quarter" varchar(7) NOT NULL,
  "wafer_nb" tinyint(3)  DEFAULT NULL,
  "site_config" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "et_splitlot_metadata" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "et_sya_set" (
  "sya_id" int(10)  NOT NULL ,
  "product_id" varchar(255) NOT NULL,
  "creation_date" datetime NOT NULL,
  "user_comment" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "start_date" datetime NOT NULL,
  "expiration_date" date NOT NULL,
  "expiration_email_date" datetime DEFAULT NULL,
  "rule_type" varchar(255) NOT NULL,
  "n1_parameter" float NOT NULL DEFAULT '-1',
  "n2_parameter" float NOT NULL DEFAULT '-1',
  "computation_fromdate" date NOT NULL,
  "computation_todate" date NOT NULL,
  "min_lots_required" smallint(5) NOT NULL DEFAULT '-1',
  "min_data_points" smallint(5) NOT NULL DEFAULT '-1',
  "options" tinyint(3)  NOT NULL DEFAULT '0',
  "flags" tinyint(3)  NOT NULL DEFAULT '0',
  "rule_name" varchar(255) DEFAULT NULL,
  "bin_type" tinyint(1) DEFAULT NULL,
  PRIMARY KEY ("sya_id")
);
CREATE TABLE "et_test_conditions" (
  "splitlot_id" int(10)  NOT NULL,
  "test_info_id" smallint(5)  NOT NULL,
  "test_type" char(1) NOT NULL,
  "condition_1" varchar(255) DEFAULT NULL,
  "condition_2" varchar(255) DEFAULT NULL,
  "condition_3" varchar(255) DEFAULT NULL,
  "condition_4" varchar(255) DEFAULT NULL,
  "condition_5" varchar(255) DEFAULT NULL,
  "condition_6" varchar(255) DEFAULT NULL,
  "condition_7" varchar(255) DEFAULT NULL,
  "condition_8" varchar(255) DEFAULT NULL,
  "condition_9" varchar(255) DEFAULT NULL,
  "condition_10" varchar(255) DEFAULT NULL,
  "condition_11" varchar(255) DEFAULT NULL,
  "condition_12" varchar(255) DEFAULT NULL,
  "condition_13" varchar(255) DEFAULT NULL,
  "condition_14" varchar(255) DEFAULT NULL,
  "condition_15" varchar(255) DEFAULT NULL,
  "condition_16" varchar(255) DEFAULT NULL,
  "condition_17" varchar(255) DEFAULT NULL,
  "condition_18" varchar(255) DEFAULT NULL,
  "condition_19" varchar(255) DEFAULT NULL,
  "condition_20" varchar(255) DEFAULT NULL,
  "condition_21" varchar(255) DEFAULT NULL,
  "condition_22" varchar(255) DEFAULT NULL,
  "condition_23" varchar(255) DEFAULT NULL,
  "condition_24" varchar(255) DEFAULT NULL,
  "condition_25" varchar(255) DEFAULT NULL,
  "condition_26" varchar(255) DEFAULT NULL,
  "condition_27" varchar(255) DEFAULT NULL,
  "condition_28" varchar(255) DEFAULT NULL,
  "condition_29" varchar(255) DEFAULT NULL,
  "condition_30" varchar(255) DEFAULT NULL,
  "condition_31" varchar(255) DEFAULT NULL,
  "condition_32" varchar(255) DEFAULT NULL,
  "condition_33" varchar(255) DEFAULT NULL,
  "condition_34" varchar(255) DEFAULT NULL,
  "condition_35" varchar(255) DEFAULT NULL,
  "condition_36" varchar(255) DEFAULT NULL,
  "condition_37" varchar(255) DEFAULT NULL,
  "condition_38" varchar(255) DEFAULT NULL,
  "condition_39" varchar(255) DEFAULT NULL,
  "condition_40" varchar(255) DEFAULT NULL,
  "condition_41" varchar(255) DEFAULT NULL,
  "condition_42" varchar(255) DEFAULT NULL,
  "condition_43" varchar(255) DEFAULT NULL,
  "condition_44" varchar(255) DEFAULT NULL,
  "condition_45" varchar(255) DEFAULT NULL,
  "condition_46" varchar(255) DEFAULT NULL,
  "condition_47" varchar(255) DEFAULT NULL,
  "condition_48" varchar(255) DEFAULT NULL,
  "condition_49" varchar(255) DEFAULT NULL,
  "condition_50" varchar(255) DEFAULT NULL,
  "condition_51" varchar(255) DEFAULT NULL,
  "condition_52" varchar(255) DEFAULT NULL,
  "condition_53" varchar(255) DEFAULT NULL,
  "condition_54" varchar(255) DEFAULT NULL,
  "condition_55" varchar(255) DEFAULT NULL,
  "condition_56" varchar(255) DEFAULT NULL,
  "condition_57" varchar(255) DEFAULT NULL,
  "condition_58" varchar(255) DEFAULT NULL,
  "condition_59" varchar(255) DEFAULT NULL,
  "condition_60" varchar(255) DEFAULT NULL,
  "condition_61" varchar(255) DEFAULT NULL,
  "condition_62" varchar(255) DEFAULT NULL,
  "condition_63" varchar(255) DEFAULT NULL,
  "condition_64" varchar(255) DEFAULT NULL,
  "condition_65" varchar(255) DEFAULT NULL,
  "condition_66" varchar(255) DEFAULT NULL,
  "condition_67" varchar(255) DEFAULT NULL,
  "condition_68" varchar(255) DEFAULT NULL,
  "condition_69" varchar(255) DEFAULT NULL,
  "condition_70" varchar(255) DEFAULT NULL,
  "condition_71" varchar(255) DEFAULT NULL,
  "condition_72" varchar(255) DEFAULT NULL,
  "condition_73" varchar(255) DEFAULT NULL,
  "condition_74" varchar(255) DEFAULT NULL,
  "condition_75" varchar(255) DEFAULT NULL,
  "condition_76" varchar(255) DEFAULT NULL,
  "condition_77" varchar(255) DEFAULT NULL,
  "condition_78" varchar(255) DEFAULT NULL,
  "condition_79" varchar(255) DEFAULT NULL,
  "condition_80" varchar(255) DEFAULT NULL,
  "condition_81" varchar(255) DEFAULT NULL,
  "condition_82" varchar(255) DEFAULT NULL,
  "condition_83" varchar(255) DEFAULT NULL,
  "condition_84" varchar(255) DEFAULT NULL,
  "condition_85" varchar(255) DEFAULT NULL,
  "condition_86" varchar(255) DEFAULT NULL,
  "condition_87" varchar(255) DEFAULT NULL,
  "condition_88" varchar(255) DEFAULT NULL,
  "condition_89" varchar(255) DEFAULT NULL,
  "condition_90" varchar(255) DEFAULT NULL,
  "condition_91" varchar(255) DEFAULT NULL,
  "condition_92" varchar(255) DEFAULT NULL,
  "condition_93" varchar(255) DEFAULT NULL,
  "condition_94" varchar(255) DEFAULT NULL,
  "condition_95" varchar(255) DEFAULT NULL,
  "condition_96" varchar(255) DEFAULT NULL,
  "condition_97" varchar(255) DEFAULT NULL,
  "condition_98" varchar(255) DEFAULT NULL,
  "condition_99" varchar(255) DEFAULT NULL,
  "condition_100" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","test_info_id","test_type")
);
CREATE TABLE "et_wafer_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","wafer_id","hbin_no")
);
CREATE TABLE "et_wafer_info" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "fab_id" varchar(255) DEFAULT NULL,
  "frame_id" varchar(255) DEFAULT NULL,
  "mask_id" varchar(255) DEFAULT NULL,
  "wafer_size" float DEFAULT NULL,
  "die_ht" float DEFAULT NULL,
  "die_wid" float DEFAULT NULL,
  "wafer_units" tinyint(3)  DEFAULT NULL,
  "wafer_flat" char(1) DEFAULT NULL,
  "center_x" smallint(5)  DEFAULT NULL,
  "center_y" smallint(5)  DEFAULT NULL,
  "pos_x" char(1) DEFAULT NULL,
  "pos_y" char(1) DEFAULT NULL,
  "gross_die" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  "wafer_nb" tinyint(3)  DEFAULT NULL,
  "site_config" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("lot_id","wafer_id")
);
CREATE TABLE "et_wafer_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","wafer_id","sbin_no")
);
CREATE TABLE "et_wyr" (
  "wyr_id" int(11) NOT NULL ,
  "site_name" varchar(255) NOT NULL,
  "week_nb" tinyint(3) DEFAULT NULL,
  "year" smallint(5) DEFAULT NULL,
  "date_in" datetime DEFAULT NULL,
  "date_out" datetime DEFAULT NULL,
  "product_name" varchar(255) DEFAULT NULL,
  "program_name" varchar(255) DEFAULT NULL,
  "tester_name" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) DEFAULT NULL,
  "subcon_lot_id" varchar(255) DEFAULT NULL,
  "user_split" varchar(1024) DEFAULT NULL,
  "yield" float DEFAULT '0',
  "parts_received" int(10) DEFAULT '0',
  "pretest_rejects" int(10) DEFAULT '0',
  "pretest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_tested" int(10) DEFAULT '0',
  "parts_pass" int(10) DEFAULT '0',
  "parts_pass_split" varchar(1024) DEFAULT NULL,
  "parts_fail" int(10) DEFAULT '0',
  "parts_fail_split" varchar(1024) DEFAULT NULL,
  "parts_retest" int(10) DEFAULT '0',
  "parts_retest_split" varchar(1024) DEFAULT NULL,
  "insertions" int(10) DEFAULT '0',
  "posttest_rejects" int(10) DEFAULT '0',
  "posttest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_shipped" int(10) DEFAULT '0',
  PRIMARY KEY ("wyr_id")
);
CREATE TABLE "et_wyr_format" (
  "site_name" varchar(255) NOT NULL,
  "column_id" tinyint(3) NOT NULL,
  "column_nb" tinyint(3) NOT NULL,
  "column_name" varchar(255) NOT NULL,
  "data_type" varchar(255) NOT NULL,
  "display" char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY ("site_name","column_id","data_type")
);
CREATE TABLE "file_host" (
  "file_host_id" int(10)  NOT NULL ,
  "host_name" varchar(255) NOT NULL DEFAULT '',
  "host_ftpuser" varchar(255) NOT NULL DEFAULT '',
  "host_ftppassword" varchar(255) NOT NULL DEFAULT '',
  "host_ftppath" varchar(255) DEFAULT NULL,
  "host_ftpport" smallint(5)  NOT NULL DEFAULT '21',
  PRIMARY KEY ("file_host_id")
);
CREATE TABLE "ft_consolidated_sublot" (
  "product_name" varchar(255) DEFAULT NULL,
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_parts" mediumint(8) DEFAULT NULL,
  "nb_parts_good" mediumint(8) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  "die_id" varchar(255) DEFAULT NULL,
  "wt_product_id" varchar(255) DEFAULT NULL,
  "wt_sublot_id" varchar(255) DEFAULT NULL,
  "wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("lot_id","sublot_id","consolidation_name")
);
CREATE TABLE "ft_consolidated_sublot_inter" (
  "product_name" varchar(255) DEFAULT NULL,
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_parts" mediumint(8) DEFAULT NULL,
  "nb_parts_good" mediumint(8) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  "die_id" varchar(255) DEFAULT NULL,
  "wt_product_id" varchar(255) DEFAULT NULL,
  "wt_sublot_id" varchar(255) DEFAULT NULL,
  "wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("lot_id","sublot_id","consolidation_name")
);
CREATE TABLE "ft_consolidated_tl" (
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_lots" bigint(21) NOT NULL DEFAULT '0',
  "nb_parts" decimal(30,0) DEFAULT NULL,
  "nb_parts_good" decimal(30,0) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  "die_id" varchar(255) DEFAULT NULL,
  "wt_product_id" varchar(255) DEFAULT NULL,
  "wt_sublot_id" varchar(255) DEFAULT NULL,
  "wt_tracking_lot_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("product_name","tracking_lot_id")
);
CREATE TABLE "ft_die_tracking" (
  "ft_tracking_lot_id" varchar(255) NOT NULL,
  "wt_product_id" varchar(255) NOT NULL,
  "wt_tracking_lot_id" varchar(255) NOT NULL,
  "wt_sublot_id" varchar(255) NOT NULL,
  "die_id" varchar(255) NOT NULL,
  PRIMARY KEY ("ft_tracking_lot_id","wt_tracking_lot_id","die_id")
);
CREATE TABLE "ft_dietrace_config" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "die_config_id" smallint(5)  NOT NULL DEFAULT '0',
  "die_index" smallint(5) NOT NULL DEFAULT '1',
  "product" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) DEFAULT NULL,
  "wafer_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","die_config_id","die_index")
);
CREATE TABLE "ft_dtr" (
  "splitlot_id" int(10)  NOT NULL,
  "run_id" mediumint(7) NOT NULL DEFAULT '0',
  "order_id" mediumint(8)  NOT NULL,
  "dtr_type" varchar(255) NOT NULL,
  "dtr_text" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("splitlot_id","run_id","order_id")
);
CREATE TABLE "ft_event" (
  "splitlot_id" int(10) NOT NULL,
  "event_id" int(11) NOT NULL,
  "run_id" mediumint(8) NOT NULL,
  "event_type" varchar(255) DEFAULT NULL,
  "event_subtype" varchar(255) DEFAULT NULL,
  "event_time_local" datetime DEFAULT NULL,
  "event_time_utc" datetime DEFAULT NULL,
  "event_message" blob,
  PRIMARY KEY ("splitlot_id","event_id")
);
CREATE TABLE "ft_ftest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "testseq" smallint(5)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id")
);
CREATE TABLE "ft_ftest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL,
  "run_id" mediumint(8)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "vect_nam" varchar(255) NOT NULL DEFAULT '',
  "vect_off" smallint(6) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","run_id","testseq")
);
CREATE TABLE "ft_ftest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","site_no")
);
CREATE TABLE "ft_ftest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  DEFAULT NULL,
  "fail_count" mediumint(8)  DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","site_no")
);
CREATE TABLE "ft_hbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "subfamily" varchar(255) DEFAULT NULL,
  "bin_subfamily" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","hbin_no")
);
CREATE TABLE "ft_hbin_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","hbin_no")
);
CREATE TABLE "ft_hbin_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","hbin_no")
);
CREATE TABLE "ft_lot" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "ft_lot_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","hbin_no")
);
CREATE TABLE "ft_lot_metadata" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "ft_lot_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","sbin_no")
);
CREATE TABLE "ft_metadata_link" (
  "link_name" varchar(255) NOT NULL,
  "gexdb_table1_name" varchar(255) NOT NULL,
  "gexdb_field1_fullname" varchar(255) NOT NULL,
  "gexdb_table2_name" varchar(255) NOT NULL,
  "gexdb_field2_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("link_name")
);
CREATE TABLE "ft_metadata_mapping" (
  "meta_name" varchar(255) NOT NULL,
  "gex_name" varchar(255) DEFAULT NULL,
  "gexdb_table_name" varchar(255) NOT NULL,
  "gexdb_field_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  "gex_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "bintype_field" char(1) NOT NULL DEFAULT 'N',
  "time_field" char(1) NOT NULL DEFAULT 'N',
  "custom_field" char(1) NOT NULL DEFAULT 'Y',
  "numeric_field" char(1) NOT NULL DEFAULT 'N',
  "fact_field" char(1) NOT NULL DEFAULT 'N',
  "consolidated_field" char(1) NOT NULL DEFAULT 'Y',
  "er_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "az_field" char(1) NOT NULL DEFAULT 'N',
  PRIMARY KEY ("meta_name")
);
CREATE TABLE "ft_mptest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "tpin_arrayindex" smallint(6) NOT NULL DEFAULT '0',
  "units" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  "testseq" smallint(5)  DEFAULT NULL,
  "spec_ll" float DEFAULT NULL,
  "spec_hl" float DEFAULT NULL,
  "spec_target" float DEFAULT NULL,
  "res_scal" tinyint(3) DEFAULT NULL,
  "ll_scal" tinyint(3) DEFAULT NULL,
  "hl_scal" tinyint(3) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","tnum","tpin_arrayindex")
);
CREATE TABLE "ft_mptest_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "LL" float DEFAULT NULL,
  "HL" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "ft_mptest_outliers" (
  "splitlot_id" int(10) NOT NULL,
  "mptest_info_id" smallint(5) NOT NULL,
  "run_id" mediumint(8) NOT NULL,
  "run_index" int(11) NOT NULL,
  "limits_run_id" mediumint(8) NOT NULL,
  "limit_type" char(1) NOT NULL,
  "value" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","run_id","run_index")
);
CREATE TABLE "ft_mptest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "value" float DEFAULT NULL,
  "tpin_pmrindex" int(10) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","run_id","testseq")
);
CREATE TABLE "ft_mptest_rollinglimits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8) NOT NULL,
  "limit_index" int(11) NOT NULL,
  "limit_type" char(1) NOT NULL,
  "limit_mode" int(11) NOT NULL,
  "LL" float DEFAULT NULL,
  "HL" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","run_id","limit_index")
);
CREATE TABLE "ft_mptest_rollingstats" (
  "splitlot_id" int(10) NOT NULL,
  "mptest_info_id" smallint(5) NOT NULL,
  "run_id" mediumint(8) NOT NULL,
  "distribution_shape" varchar(255) DEFAULT NULL,
  "n_factor" float DEFAULT NULL,
  "t_factor" float DEFAULT NULL,
  "mean" float DEFAULT NULL,
  "sigma" float DEFAULT NULL,
  "min" float DEFAULT NULL,
  "q1" float DEFAULT NULL,
  "median" float DEFAULT NULL,
  "q3" float DEFAULT NULL,
  "max" float DEFAULT NULL,
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","mptest_info_id","run_id")
);
CREATE TABLE "ft_mptest_static_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "limit_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) DEFAULT NULL,
  "hbin_no" smallint(5)  DEFAULT NULL,
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","limit_id")
);
CREATE TABLE "ft_mptest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "ft_mptest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  NOT NULL DEFAULT '0',
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "ft_parts_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no")
);
CREATE TABLE "ft_parts_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_good" mediumint(8) DEFAULT NULL,
  "nb_rtst" mediumint(8) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","site_no")
);
CREATE TABLE "ft_pin_map" (
  "splitlot_id" int(10)  NOT NULL,
  "tpin_pmrindex" int(10) NOT NULL,
  "chan_typ" smallint(5)  DEFAULT '0',
  "chan_nam" varchar(255) DEFAULT '',
  "phy_nam" varchar(255) DEFAULT '',
  "log_nam" varchar(255) DEFAULT '',
  "head_num" tinyint(4)  NOT NULL DEFAULT '1',
  "site_num" tinyint(4)  NOT NULL DEFAULT '1',
  PRIMARY KEY ("splitlot_id","tpin_pmrindex","head_num","site_num")
);
CREATE TABLE "ft_prod_alarm" (
  "splitlot_id" int(10)  NOT NULL,
  "alarm_cat" varchar(255) NOT NULL,
  "alarm_type" varchar(255) NOT NULL,
  "item_no" int(10)  NOT NULL,
  "item_name" varchar(255) DEFAULT NULL,
  "flags" binary(2) NOT NULL,
  "lcl" float NOT NULL DEFAULT '0',
  "ucl" float NOT NULL DEFAULT '0',
  "value" float NOT NULL DEFAULT '0',
  "units" varchar(10) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","alarm_cat","alarm_type","item_no")
);
CREATE TABLE "ft_product_hbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "ft_product_sbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "ft_ptest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "units" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  "testseq" smallint(5)  DEFAULT NULL,
  "spec_ll" float DEFAULT NULL,
  "spec_hl" float DEFAULT NULL,
  "spec_target" float DEFAULT NULL,
  "res_scal" tinyint(3) DEFAULT NULL,
  "ll_scal" tinyint(3) DEFAULT NULL,
  "hl_scal" tinyint(3) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id")
);
CREATE TABLE "ft_ptest_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "LL" float DEFAULT NULL,
  "HL" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "ft_ptest_outliers" (
  "splitlot_id" int(10) NOT NULL,
  "ptest_info_id" smallint(5) NOT NULL,
  "run_id" mediumint(8) NOT NULL,
  "run_index" int(11) NOT NULL,
  "limits_run_id" mediumint(8) NOT NULL,
  "limit_type" char(1) NOT NULL,
  "value" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id","run_index")
);
CREATE TABLE "ft_ptest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "value" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id","testseq")
);
CREATE TABLE "ft_ptest_rollinglimits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8) NOT NULL,
  "limit_index" int(11) NOT NULL,
  "limit_type" char(1) NOT NULL,
  "limit_mode" int(11) NOT NULL,
  "LL" float DEFAULT NULL,
  "HL" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id","limit_index")
);
CREATE TABLE "ft_ptest_rollingstats" (
  "splitlot_id" int(10) NOT NULL,
  "ptest_info_id" smallint(5) NOT NULL,
  "run_id" mediumint(8) NOT NULL,
  "distribution_shape" varchar(255) DEFAULT NULL,
  "n_factor" float DEFAULT NULL,
  "t_factor" float DEFAULT NULL,
  "mean" float DEFAULT NULL,
  "sigma" float DEFAULT NULL,
  "min" float DEFAULT NULL,
  "q1" float DEFAULT NULL,
  "median" float DEFAULT NULL,
  "q3" float DEFAULT NULL,
  "max" float DEFAULT NULL,
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id")
);
CREATE TABLE "ft_ptest_static_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "limit_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) DEFAULT NULL,
  "hbin_no" smallint(5)  DEFAULT NULL,
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","limit_id")
);
CREATE TABLE "ft_ptest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "ft_ptest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  DEFAULT NULL,
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "ft_run" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "part_id" varchar(255) DEFAULT NULL,
  "part_x" smallint(6) DEFAULT NULL,
  "part_y" smallint(6) DEFAULT NULL,
  "part_status" char(1) DEFAULT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  "tests_executed" smallint(5)  NOT NULL DEFAULT '0',
  "tests_failed" smallint(5)  NOT NULL DEFAULT '0',
  "firstfail_tnum" int(10)  DEFAULT NULL,
  "firstfail_tname" varchar(255) DEFAULT NULL,
  "retest_index" tinyint(3)  NOT NULL DEFAULT '0',
  "wafer_id" varchar(255) DEFAULT NULL,
  "part_txt" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","run_id","site_no")
);
CREATE TABLE "ft_run_dietrace" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "run_id" mediumint(8)  NOT NULL DEFAULT '0',
  "die_config_id" smallint(5) NOT NULL DEFAULT '1',
  "part_id" varchar(255) DEFAULT NULL,
  "part_x" smallint(6) DEFAULT NULL,
  "part_y" smallint(6) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","run_id","die_config_id")
);
CREATE TABLE "ft_sbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "subfamily" varchar(255) DEFAULT NULL,
  "bin_subfamily" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","sbin_no")
);
CREATE TABLE "ft_sbin_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","sbin_no")
);
CREATE TABLE "ft_sbin_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","sbin_no")
);
CREATE TABLE "ft_sbl" (
  "sya_id" int(10)  NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "rule_type" varchar(255) DEFAULT NULL,
  "n1_parameter" float DEFAULT NULL,
  "n2_parameter" float DEFAULT NULL,
  PRIMARY KEY ("sya_id","bin_no")
);
CREATE TABLE "ft_sdr" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_grp" smallint(5)  NOT NULL DEFAULT '0',
  "site_index" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5)  NOT NULL DEFAULT '0',
  "hand_typ" varchar(255) DEFAULT NULL,
  "hand_id" varchar(255) DEFAULT NULL,
  "card_typ" varchar(255) DEFAULT NULL,
  "card_id" varchar(255) DEFAULT NULL,
  "load_typ" varchar(255) DEFAULT NULL,
  "load_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "dib_id" varchar(255) DEFAULT NULL,
  "cabl_typ" varchar(255) DEFAULT NULL,
  "cabl_id" varchar(255) DEFAULT NULL,
  "cont_typ" varchar(255) DEFAULT NULL,
  "cont_id" varchar(255) DEFAULT NULL,
  "lasr_typ" varchar(255) DEFAULT NULL,
  "lasr_id" varchar(255) DEFAULT NULL,
  "extr_typ" varchar(255) DEFAULT NULL,
  "extr_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","site_grp","site_no")
);
CREATE TABLE "ft_splitlot" (
  "splitlot_id" int(10)  NOT NULL ,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "setup_t" int(10) NOT NULL DEFAULT '0',
  "start_t" int(10) NOT NULL DEFAULT '0',
  "finish_t" int(10) NOT NULL DEFAULT '0',
  "stat_num" tinyint(3)  NOT NULL DEFAULT '0',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(2) NOT NULL DEFAULT '\0\0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_samples" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_samples_good" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_summary" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_summary_good" mediumint(8) NOT NULL DEFAULT '0',
  "data_provider" varchar(255) DEFAULT '',
  "data_type" varchar(255) DEFAULT '',
  "prod_data" char(1) NOT NULL DEFAULT 'Y',
  "retest_phase" varchar(255) DEFAULT NULL,
  "retest_index" tinyint(3)  NOT NULL DEFAULT '0',
  "retest_hbins" varchar(255) DEFAULT NULL,
  "rework_code" tinyint(3)  NOT NULL DEFAULT '0',
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "exec_typ" varchar(255) NOT NULL DEFAULT '',
  "exec_ver" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "mode_cod" char(1) DEFAULT NULL,
  "rtst_cod" char(1) DEFAULT NULL,
  "prot_cod" char(1) DEFAULT NULL,
  "burn_tim" int(10) DEFAULT NULL,
  "cmod_cod" char(1) DEFAULT NULL,
  "part_typ" varchar(255) DEFAULT NULL,
  "user_txt" varchar(255) DEFAULT NULL,
  "aux_file" varchar(255) DEFAULT NULL,
  "pkg_typ" varchar(255) DEFAULT NULL,
  "famly_id" varchar(255) DEFAULT NULL,
  "date_cod" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "spec_nam" varchar(255) DEFAULT NULL,
  "spec_ver" varchar(255) DEFAULT NULL,
  "flow_id" varchar(255) DEFAULT NULL,
  "setup_id" varchar(255) DEFAULT NULL,
  "dsgn_rev" varchar(255) DEFAULT NULL,
  "eng_id" varchar(255) DEFAULT NULL,
  "rom_cod" varchar(255) DEFAULT NULL,
  "serl_num" varchar(255) DEFAULT NULL,
  "supr_nam" varchar(255) DEFAULT NULL,
  "nb_sites" tinyint(3)  NOT NULL DEFAULT '1',
  "head_num" tinyint(3)  DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "card_typ" varchar(255) DEFAULT NULL,
  "card_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "dib_id" varchar(255) DEFAULT NULL,
  "cable_typ" varchar(255) DEFAULT NULL,
  "cable_id" varchar(255) DEFAULT NULL,
  "contactor_typ" varchar(255) DEFAULT NULL,
  "contactor_id" varchar(255) DEFAULT NULL,
  "laser_typ" varchar(255) DEFAULT NULL,
  "laser_id" varchar(255) DEFAULT NULL,
  "extra_typ" varchar(255) DEFAULT NULL,
  "extra_id" varchar(255) DEFAULT NULL,
  "file_host_id" int(10)  DEFAULT '0',
  "file_path" varchar(255) NOT NULL DEFAULT '',
  "file_name" varchar(255) NOT NULL DEFAULT '',
  "valid_splitlot" char(1) NOT NULL DEFAULT 'N',
  "insertion_time" int(10)  NOT NULL DEFAULT '0',
  "subcon_lot_id" varchar(255) NOT NULL DEFAULT '',
  "incremental_update" varchar(255) DEFAULT NULL,
  "sya_id" int(10)  DEFAULT '0',
  "day" varchar(10) NOT NULL,
  "week_nb" tinyint(2)  NOT NULL,
  "month_nb" tinyint(2)  NOT NULL,
  "quarter_nb" tinyint(1)  NOT NULL,
  "year_nb" smallint(4) NOT NULL,
  "year_and_week" varchar(7) NOT NULL,
  "year_and_month" varchar(7) NOT NULL,
  "year_and_quarter" varchar(7) NOT NULL,
  "recipe_id" int(11) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "ft_splitlot_metadata" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "ft_sublot_consolidation" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "consolidated_data_type" varchar(255) NOT NULL,
  "consolidation_name" varchar(255) NOT NULL,
  "consolidation_prod_flow" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","sublot_id","consolidation_name")
);
CREATE TABLE "ft_sublot_consolidation_inter" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "consolidated_data_type" varchar(255) NOT NULL,
  "consolidation_name" varchar(255) NOT NULL,
  "consolidation_prod_flow" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","sublot_id","consolidation_name")
);
CREATE TABLE "ft_sublot_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","sublot_id","hbin_no")
);
CREATE TABLE "ft_sublot_hbin_inter" (
  "lot_id" varchar(255) NOT NULL,
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  "consolidation_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","sublot_id","hbin_no","consolidation_name")
);
CREATE TABLE "ft_sublot_info" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "product_name" varchar(255) DEFAULT NULL,
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  "consolidation_status" varchar(255) DEFAULT NULL,
  "consolidation_ref_date" datetime DEFAULT NULL,
  PRIMARY KEY ("lot_id","sublot_id")
);
CREATE TABLE "ft_sublot_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","sublot_id","sbin_no")
);
CREATE TABLE "ft_sublot_sbin_inter" (
  "lot_id" varchar(255) NOT NULL,
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  "consolidation_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","sublot_id","sbin_no","consolidation_name")
);
CREATE TABLE "ft_sya_set" (
  "sya_id" int(10)  NOT NULL ,
  "product_id" varchar(255) NOT NULL,
  "creation_date" datetime NOT NULL,
  "user_comment" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "start_date" datetime NOT NULL,
  "expiration_date" date NOT NULL,
  "expiration_email_date" datetime DEFAULT NULL,
  "rule_type" varchar(255) NOT NULL,
  "n1_parameter" float NOT NULL DEFAULT '-1',
  "n2_parameter" float NOT NULL DEFAULT '-1',
  "computation_fromdate" date NOT NULL,
  "computation_todate" date NOT NULL,
  "min_lots_required" smallint(5) NOT NULL DEFAULT '-1',
  "min_data_points" smallint(5) NOT NULL DEFAULT '-1',
  "options" tinyint(3)  NOT NULL DEFAULT '0',
  "flags" tinyint(3)  NOT NULL DEFAULT '0',
  "rule_name" varchar(255) DEFAULT NULL,
  "bin_type" tinyint(1) DEFAULT NULL,
  PRIMARY KEY ("sya_id")
);
CREATE TABLE "ft_test_conditions" (
  "splitlot_id" int(10)  NOT NULL,
  "test_info_id" smallint(5)  NOT NULL,
  "test_type" char(1) NOT NULL,
  "condition_1" varchar(255) DEFAULT NULL,
  "condition_2" varchar(255) DEFAULT NULL,
  "condition_3" varchar(255) DEFAULT NULL,
  "condition_4" varchar(255) DEFAULT NULL,
  "condition_5" varchar(255) DEFAULT NULL,
  "condition_6" varchar(255) DEFAULT NULL,
  "condition_7" varchar(255) DEFAULT NULL,
  "condition_8" varchar(255) DEFAULT NULL,
  "condition_9" varchar(255) DEFAULT NULL,
  "condition_10" varchar(255) DEFAULT NULL,
  "condition_11" varchar(255) DEFAULT NULL,
  "condition_12" varchar(255) DEFAULT NULL,
  "condition_13" varchar(255) DEFAULT NULL,
  "condition_14" varchar(255) DEFAULT NULL,
  "condition_15" varchar(255) DEFAULT NULL,
  "condition_16" varchar(255) DEFAULT NULL,
  "condition_17" varchar(255) DEFAULT NULL,
  "condition_18" varchar(255) DEFAULT NULL,
  "condition_19" varchar(255) DEFAULT NULL,
  "condition_20" varchar(255) DEFAULT NULL,
  "condition_21" varchar(255) DEFAULT NULL,
  "condition_22" varchar(255) DEFAULT NULL,
  "condition_23" varchar(255) DEFAULT NULL,
  "condition_24" varchar(255) DEFAULT NULL,
  "condition_25" varchar(255) DEFAULT NULL,
  "condition_26" varchar(255) DEFAULT NULL,
  "condition_27" varchar(255) DEFAULT NULL,
  "condition_28" varchar(255) DEFAULT NULL,
  "condition_29" varchar(255) DEFAULT NULL,
  "condition_30" varchar(255) DEFAULT NULL,
  "condition_31" varchar(255) DEFAULT NULL,
  "condition_32" varchar(255) DEFAULT NULL,
  "condition_33" varchar(255) DEFAULT NULL,
  "condition_34" varchar(255) DEFAULT NULL,
  "condition_35" varchar(255) DEFAULT NULL,
  "condition_36" varchar(255) DEFAULT NULL,
  "condition_37" varchar(255) DEFAULT NULL,
  "condition_38" varchar(255) DEFAULT NULL,
  "condition_39" varchar(255) DEFAULT NULL,
  "condition_40" varchar(255) DEFAULT NULL,
  "condition_41" varchar(255) DEFAULT NULL,
  "condition_42" varchar(255) DEFAULT NULL,
  "condition_43" varchar(255) DEFAULT NULL,
  "condition_44" varchar(255) DEFAULT NULL,
  "condition_45" varchar(255) DEFAULT NULL,
  "condition_46" varchar(255) DEFAULT NULL,
  "condition_47" varchar(255) DEFAULT NULL,
  "condition_48" varchar(255) DEFAULT NULL,
  "condition_49" varchar(255) DEFAULT NULL,
  "condition_50" varchar(255) DEFAULT NULL,
  "condition_51" varchar(255) DEFAULT NULL,
  "condition_52" varchar(255) DEFAULT NULL,
  "condition_53" varchar(255) DEFAULT NULL,
  "condition_54" varchar(255) DEFAULT NULL,
  "condition_55" varchar(255) DEFAULT NULL,
  "condition_56" varchar(255) DEFAULT NULL,
  "condition_57" varchar(255) DEFAULT NULL,
  "condition_58" varchar(255) DEFAULT NULL,
  "condition_59" varchar(255) DEFAULT NULL,
  "condition_60" varchar(255) DEFAULT NULL,
  "condition_61" varchar(255) DEFAULT NULL,
  "condition_62" varchar(255) DEFAULT NULL,
  "condition_63" varchar(255) DEFAULT NULL,
  "condition_64" varchar(255) DEFAULT NULL,
  "condition_65" varchar(255) DEFAULT NULL,
  "condition_66" varchar(255) DEFAULT NULL,
  "condition_67" varchar(255) DEFAULT NULL,
  "condition_68" varchar(255) DEFAULT NULL,
  "condition_69" varchar(255) DEFAULT NULL,
  "condition_70" varchar(255) DEFAULT NULL,
  "condition_71" varchar(255) DEFAULT NULL,
  "condition_72" varchar(255) DEFAULT NULL,
  "condition_73" varchar(255) DEFAULT NULL,
  "condition_74" varchar(255) DEFAULT NULL,
  "condition_75" varchar(255) DEFAULT NULL,
  "condition_76" varchar(255) DEFAULT NULL,
  "condition_77" varchar(255) DEFAULT NULL,
  "condition_78" varchar(255) DEFAULT NULL,
  "condition_79" varchar(255) DEFAULT NULL,
  "condition_80" varchar(255) DEFAULT NULL,
  "condition_81" varchar(255) DEFAULT NULL,
  "condition_82" varchar(255) DEFAULT NULL,
  "condition_83" varchar(255) DEFAULT NULL,
  "condition_84" varchar(255) DEFAULT NULL,
  "condition_85" varchar(255) DEFAULT NULL,
  "condition_86" varchar(255) DEFAULT NULL,
  "condition_87" varchar(255) DEFAULT NULL,
  "condition_88" varchar(255) DEFAULT NULL,
  "condition_89" varchar(255) DEFAULT NULL,
  "condition_90" varchar(255) DEFAULT NULL,
  "condition_91" varchar(255) DEFAULT NULL,
  "condition_92" varchar(255) DEFAULT NULL,
  "condition_93" varchar(255) DEFAULT NULL,
  "condition_94" varchar(255) DEFAULT NULL,
  "condition_95" varchar(255) DEFAULT NULL,
  "condition_96" varchar(255) DEFAULT NULL,
  "condition_97" varchar(255) DEFAULT NULL,
  "condition_98" varchar(255) DEFAULT NULL,
  "condition_99" varchar(255) DEFAULT NULL,
  "condition_100" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","test_info_id","test_type")
);
CREATE TABLE "ft_wyr" (
  "wyr_id" int(11) NOT NULL ,
  "site_name" varchar(255) NOT NULL,
  "week_nb" tinyint(3) DEFAULT NULL,
  "year" smallint(5) DEFAULT NULL,
  "date_in" datetime DEFAULT NULL,
  "date_out" datetime DEFAULT NULL,
  "product_name" varchar(255) DEFAULT NULL,
  "program_name" varchar(255) DEFAULT NULL,
  "tester_name" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) DEFAULT NULL,
  "subcon_lot_id" varchar(255) DEFAULT NULL,
  "user_split" varchar(1024) DEFAULT NULL,
  "yield" float DEFAULT '0',
  "parts_received" int(10) DEFAULT '0',
  "pretest_rejects" int(10) DEFAULT '0',
  "pretest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_tested" int(10) DEFAULT '0',
  "parts_pass" int(10) DEFAULT '0',
  "parts_pass_split" varchar(1024) DEFAULT NULL,
  "parts_fail" int(10) DEFAULT '0',
  "parts_fail_split" varchar(1024) DEFAULT NULL,
  "parts_retest" int(10) DEFAULT '0',
  "PARTS_RETEST_SPLIT" varchar(1024) DEFAULT NULL,
  "insertions" int(10) DEFAULT '0',
  "posttest_rejects" int(10) DEFAULT '0',
  "posttest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_shipped" int(10) DEFAULT '0',
  PRIMARY KEY ("wyr_id")
);
CREATE TABLE "ft_wyr_format" (
  "site_name" varchar(255) NOT NULL,
  "column_id" tinyint(3) NOT NULL,
  "column_nb" tinyint(3) NOT NULL,
  "column_name" varchar(255) NOT NULL,
  "data_type" varchar(255) NOT NULL,
  "display" char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY ("site_name","column_id","data_type")
);
CREATE TABLE "gexdb_log" (
  "log_id" int(11) NOT NULL ,
  "log_date" datetime NOT NULL,
  "log_type" varchar(255) NOT NULL,
  "log_string" varchar(1024) NOT NULL,
  PRIMARY KEY ("log_id")
);
CREATE TABLE "global_files" (
  "file_id" int(10)  NOT NULL ,
  "file_name" varchar(255) NOT NULL,
  "file_type" varchar(255) NOT NULL,
  "file_format" varchar(255) NOT NULL,
  "file_content" mediumtext NOT NULL,
  "file_checksum" int(10)  NOT NULL,
  "file_last_update" datetime NOT NULL,
  PRIMARY KEY ("file_id")
);
CREATE TABLE "global_info" (
  "db_version_name" varchar(255) NOT NULL,
  "db_version_nb" smallint(5) NOT NULL,
  "db_version_build" smallint(5) NOT NULL,
  "incremental_splitlots" int(9) NOT NULL DEFAULT '0',
  "db_status" varchar(255) DEFAULT NULL,
  "db_type" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("db_version_build")
);
CREATE TABLE "global_options" (
  "option_name" varchar(255) NOT NULL,
  "option_value" varchar(255) NOT NULL,
  PRIMARY KEY ("option_name")
);
CREATE TABLE "global_settings" (
  "key" VARCHAR(255) PRIMARY KEY NOT NULL,
  "value" VARCHAR(255)
);
CREATE TABLE "incremental_update" (
  "db_update_name" varchar(255) NOT NULL,
  "initial_splitlots" int(9) NOT NULL DEFAULT '0',
  "remaining_splitlots" int(9) NOT NULL DEFAULT '0',
  "db_version_build" smallint(5) DEFAULT NULL,
  PRIMARY KEY ("db_update_name")
);
CREATE TABLE "product" (
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "description" varchar(1000) DEFAULT NULL,
  PRIMARY KEY ("product_name")
);
CREATE TABLE "tdr_update_history" (
  "update_id" int(11) NOT NULL ,
  "update_from" int(11) NOT NULL,
  "update_to" int(11) DEFAULT NULL,
  "start_time" datetime DEFAULT NULL,
  "end_time" datetime DEFAULT NULL,
  "status" varchar(45) DEFAULT NULL,
  PRIMARY KEY ("update_id")
);
CREATE TABLE "tdr_update_logs" (
  "log_id" int(11) NOT NULL ,
  "update_id" int(11) NOT NULL,
  "build_version" int(11) DEFAULT NULL,
  "date" datetime DEFAULT NULL,
  "action" varchar(255) DEFAULT NULL,
  "description" text,
  "status" varchar(45) DEFAULT NULL,
  "message" text,
  PRIMARY KEY ("log_id")
);
CREATE TABLE "tdr_update_partitions_state" (
  "update_id" int(11) NOT NULL,
  "table_name" varchar(255) NOT NULL,
  "partition_name" varchar(255) NOT NULL,
  "partition_ordinal_position" int(11) NOT NULL,
  "partition_method" varchar(255) NOT NULL,
  "partition_expression" varchar(255) NOT NULL,
  "min_index" int(11) NOT NULL,
  "max_index" int(11) NOT NULL,
  "partition_rows" int(11) DEFAULT NULL,
  PRIMARY KEY ("update_id","table_name","partition_name")
);
CREATE TABLE "tdr_update_tables_state" (
  "update_id" int(11) NOT NULL,
  "table_name" varchar(255) NOT NULL,
  "table_type" varchar(255) NOT NULL,
  "engine" varchar(255) NOT NULL,
  "version" int(11) NOT NULL,
  "row_format" varchar(255) NOT NULL,
  "" int(11) DEFAULT NULL,
  "table_collation" varchar(255) NOT NULL,
  "table_rows" int(11) DEFAULT NULL,
  "create_time" datetime DEFAULT NULL,
  PRIMARY KEY ("update_id","table_name")
);
CREATE TABLE "token" (
  "start_time" datetime NOT NULL,
  "name" varchar(256) NOT NULL,
  "key_value" varchar(512) NOT NULL,
  "sql_id" int(10) NOT NULL,
  PRIMARY KEY ("name","key_value")
);
CREATE TABLE "wt_consolidated_tl" (
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_wafers" bigint(21) NOT NULL DEFAULT '0',
  "nb_parts" decimal(30,0) DEFAULT NULL,
  "nb_parts_good" decimal(30,0) DEFAULT NULL,
  "gross_die" decimal(30,0) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("product_name","tracking_lot_id")
);
CREATE TABLE "wt_consolidated_wafer" (
  "product_name" varchar(255) DEFAULT NULL,
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_parts" mediumint(8) DEFAULT NULL,
  "nb_parts_good" mediumint(8) DEFAULT NULL,
  "gross_die" mediumint(8) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("lot_id","wafer_id","consolidation_name")
);
CREATE TABLE "wt_consolidated_wafer_inter" (
  "product_name" varchar(255) DEFAULT NULL,
  "tracking_lot_id" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "start_t" int(10) DEFAULT NULL,
  "day" varchar(10) DEFAULT NULL,
  "week_nb" int(2)  DEFAULT NULL,
  "month_nb" int(2)  DEFAULT NULL,
  "quarter_nb" varchar(1) DEFAULT NULL,
  "year_nb" int(4)  DEFAULT NULL,
  "year_and_week" varchar(7) DEFAULT NULL,
  "year_and_month" varchar(7) DEFAULT NULL,
  "year_and_quarter" varchar(6) DEFAULT NULL,
  "nb_parts" mediumint(8) DEFAULT NULL,
  "nb_parts_good" mediumint(8) DEFAULT NULL,
  "gross_die" mediumint(8) DEFAULT NULL,
  "consolidated_data_type" varchar(255) NOT NULL DEFAULT '',
  "consolidation_name" varchar(255) NOT NULL DEFAULT '',
  "consolidation_prod_flow" varchar(255) NOT NULL DEFAULT '',
  "dib_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "data_provider" varchar(255) DEFAULT NULL,
  "data_type" varchar(255) DEFAULT NULL,
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "famly_id" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "pkg_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("lot_id","wafer_id","consolidation_name")
);
CREATE TABLE "wt_dtr" (
  "splitlot_id" int(10)  NOT NULL,
  "run_id" mediumint(7) NOT NULL DEFAULT '0',
  "order_id" mediumint(8)  NOT NULL,
  "dtr_type" varchar(255) NOT NULL,
  "dtr_text" varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY ("splitlot_id","run_id","order_id")
);
CREATE TABLE "wt_ftest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "testseq" smallint(5)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id")
);
CREATE TABLE "wt_ftest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL,
  "run_id" mediumint(7)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "vect_nam" varchar(255) NOT NULL DEFAULT '',
  "vect_off" smallint(6) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","run_id","testseq")
);
CREATE TABLE "wt_ftest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","site_no")
);
CREATE TABLE "wt_ftest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ftest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  DEFAULT NULL,
  "fail_count" mediumint(8)  DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ftest_info_id","site_no")
);
CREATE TABLE "wt_hbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","hbin_no")
);
CREATE TABLE "wt_hbin_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","hbin_no")
);
CREATE TABLE "wt_hbin_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","hbin_no")
);
CREATE TABLE "wt_lot" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "tracking_lot_id" varchar(255) NOT NULL DEFAULT '',
  "product_name" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "wt_lot_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","hbin_no")
);
CREATE TABLE "wt_lot_metadata" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","product_name")
);
CREATE TABLE "wt_lot_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","product_name","sbin_no")
);
CREATE TABLE "wt_metadata_link" (
  "link_name" varchar(255) NOT NULL,
  "gexdb_table1_name" varchar(255) NOT NULL,
  "gexdb_field1_fullname" varchar(255) NOT NULL,
  "gexdb_table2_name" varchar(255) NOT NULL,
  "gexdb_field2_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("link_name")
);
CREATE TABLE "wt_metadata_mapping" (
  "meta_name" varchar(255) NOT NULL,
  "gex_name" varchar(255) DEFAULT NULL,
  "gexdb_table_name" varchar(255) NOT NULL,
  "gexdb_field_fullname" varchar(255) NOT NULL,
  "gexdb_link_name" varchar(255) DEFAULT NULL,
  "gex_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "bintype_field" char(1) NOT NULL DEFAULT 'N',
  "time_field" char(1) NOT NULL DEFAULT 'N',
  "custom_field" char(1) NOT NULL DEFAULT 'Y',
  "numeric_field" char(1) NOT NULL DEFAULT 'N',
  "fact_field" char(1) NOT NULL DEFAULT 'N',
  "consolidated_field" char(1) NOT NULL DEFAULT 'Y',
  "er_display_in_gui" char(1) NOT NULL DEFAULT 'Y',
  "az_field" char(1) NOT NULL DEFAULT 'N',
  PRIMARY KEY ("meta_name")
);
CREATE TABLE "wt_mptest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "tpin_arrayindex" smallint(6) NOT NULL DEFAULT '0',
  "units" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  "testseq" smallint(5)  DEFAULT NULL,
  "spec_ll" float DEFAULT NULL,
  "spec_hl" float DEFAULT NULL,
  "spec_target" float DEFAULT NULL,
  "res_scal" tinyint(3) DEFAULT NULL,
  "ll_scal" tinyint(3) DEFAULT NULL,
  "hl_scal" tinyint(3) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id")
);
CREATE TABLE "wt_mptest_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "LL" float DEFAULT NULL,
  "HL" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "wt_mptest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(7)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "value" float DEFAULT NULL,
  "tpin_pmrindex" int(10) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","run_id","testseq")
);
CREATE TABLE "wt_mptest_static_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "limit_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) DEFAULT NULL,
  "hbin_no" smallint(5)  DEFAULT NULL,
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","limit_id")
);
CREATE TABLE "wt_mptest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "wt_mptest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "mptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  NOT NULL DEFAULT '0',
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","mptest_info_id","site_no")
);
CREATE TABLE "wt_parts_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no")
);
CREATE TABLE "wt_parts_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_good" mediumint(8) DEFAULT NULL,
  "nb_rtst" mediumint(8) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","site_no")
);
CREATE TABLE "wt_pin_map" (
  "splitlot_id" int(10)  NOT NULL,
  "tpin_pmrindex" int(10) NOT NULL,
  "chan_typ" smallint(5)  DEFAULT '0',
  "chan_nam" varchar(255) DEFAULT '',
  "phy_nam" varchar(255) DEFAULT '',
  "log_nam" varchar(255) DEFAULT '',
  "head_num" tinyint(4)  NOT NULL DEFAULT '1',
  "site_num" tinyint(4)  NOT NULL DEFAULT '1',
  PRIMARY KEY ("splitlot_id","tpin_pmrindex","head_num","site_num")
);
CREATE TABLE "wt_prod_alarm" (
  "splitlot_id" int(10)  NOT NULL,
  "alarm_cat" varchar(255) NOT NULL,
  "alarm_type" varchar(255) NOT NULL,
  "item_no" int(10)  NOT NULL,
  "item_name" varchar(255) DEFAULT NULL,
  "flags" binary(2) NOT NULL,
  "lcl" float NOT NULL DEFAULT '0',
  "ucl" float NOT NULL DEFAULT '0',
  "value" float NOT NULL DEFAULT '0',
  "units" varchar(10) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","alarm_cat","alarm_type","item_no")
);
CREATE TABLE "wt_product_hbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "wt_product_sbin" (
  "product_name" varchar(255) NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "bin_cat" char(1) DEFAULT NULL,
  "nb_parts" bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY ("product_name","bin_no")
);
CREATE TABLE "wt_ptest_info" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "tnum" int(10)  NOT NULL DEFAULT '0',
  "tname" varchar(255) NOT NULL DEFAULT '',
  "units" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  "testseq" smallint(5)  DEFAULT NULL,
  "spec_ll" float DEFAULT NULL,
  "spec_hl" float DEFAULT NULL,
  "spec_target" float DEFAULT NULL,
  "res_scal" tinyint(3) DEFAULT NULL,
  "ll_scal" tinyint(3) DEFAULT NULL,
  "hl_scal" tinyint(3) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id")
);
CREATE TABLE "wt_ptest_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "wt_ptest_results" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "run_id" mediumint(7)  NOT NULL DEFAULT '0',
  "testseq" smallint(5)  NOT NULL DEFAULT '0',
  "flags" binary(1) NOT NULL DEFAULT '\0',
  "value" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","run_id","testseq")
);
CREATE TABLE "wt_ptest_static_limits" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "limit_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) DEFAULT NULL,
  "hbin_no" smallint(5)  DEFAULT NULL,
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ll" float DEFAULT NULL,
  "hl" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","limit_id")
);
CREATE TABLE "wt_ptest_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  NOT NULL DEFAULT '0',
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "wt_ptest_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "ptest_info_id" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "exec_count" mediumint(8)  DEFAULT NULL,
  "fail_count" mediumint(8)  DEFAULT NULL,
  "min_value" float DEFAULT NULL,
  "max_value" float DEFAULT NULL,
  "sum" float DEFAULT NULL,
  "square_sum" float DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","ptest_info_id","site_no")
);
CREATE TABLE "wt_run" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "run_id" mediumint(7)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "part_id" varchar(255) DEFAULT NULL,
  "part_x" smallint(6) DEFAULT NULL,
  "part_y" smallint(6) DEFAULT NULL,
  "part_status" char(1) DEFAULT NULL,
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  DEFAULT NULL,
  "ttime" int(10)  DEFAULT NULL,
  "tests_executed" smallint(5)  NOT NULL DEFAULT '0',
  "tests_failed" smallint(5)  NOT NULL DEFAULT '0',
  "firstfail_tnum" int(10)  DEFAULT NULL,
  "firstfail_tname" varchar(255) DEFAULT NULL,
  "retest_index" tinyint(3)  NOT NULL DEFAULT '0',
  "part_txt" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","run_id","site_no")
);
CREATE TABLE "wt_sbin" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","sbin_no")
);
CREATE TABLE "wt_sbin_stats_samples" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","sbin_no")
);
CREATE TABLE "wt_sbin_stats_summary" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_no" smallint(5) NOT NULL DEFAULT '1',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "bin_count" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id","site_no","sbin_no")
);
CREATE TABLE "wt_sbl" (
  "sya_id" int(10)  NOT NULL,
  "bin_no" smallint(5)  NOT NULL,
  "bin_name" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "rule_type" varchar(255) DEFAULT NULL,
  "n1_parameter" float DEFAULT NULL,
  "n2_parameter" float DEFAULT NULL,
  PRIMARY KEY ("sya_id","bin_no")
);
CREATE TABLE "wt_sdr" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  "site_grp" smallint(5)  NOT NULL DEFAULT '0',
  "site_index" smallint(5)  NOT NULL DEFAULT '0',
  "site_no" smallint(5)  NOT NULL DEFAULT '0',
  "hand_typ" varchar(255) DEFAULT NULL,
  "hand_id" varchar(255) DEFAULT NULL,
  "card_typ" varchar(255) DEFAULT NULL,
  "card_id" varchar(255) DEFAULT NULL,
  "load_typ" varchar(255) DEFAULT NULL,
  "load_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "dib_id" varchar(255) DEFAULT NULL,
  "cabl_typ" varchar(255) DEFAULT NULL,
  "cabl_id" varchar(255) DEFAULT NULL,
  "cont_typ" varchar(255) DEFAULT NULL,
  "cont_id" varchar(255) DEFAULT NULL,
  "lasr_typ" varchar(255) DEFAULT NULL,
  "lasr_id" varchar(255) DEFAULT NULL,
  "extr_typ" varchar(255) DEFAULT NULL,
  "extr_id" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","site_grp","site_no")
);
CREATE TABLE "wt_splitlot" (
  "splitlot_id" int(10)  NOT NULL ,
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "sublot_id" varchar(255) NOT NULL DEFAULT '',
  "setup_t" int(10) NOT NULL DEFAULT '0',
  "start_t" int(10) NOT NULL DEFAULT '0',
  "finish_t" int(10) NOT NULL DEFAULT '0',
  "stat_num" tinyint(3)  NOT NULL DEFAULT '0',
  "tester_name" varchar(255) NOT NULL DEFAULT '',
  "tester_type" varchar(255) NOT NULL DEFAULT '',
  "flags" binary(2) NOT NULL DEFAULT '\0\0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_samples" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_samples_good" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_summary" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_summary_good" mediumint(8) NOT NULL DEFAULT '0',
  "data_provider" varchar(255) DEFAULT '',
  "data_type" varchar(255) DEFAULT '',
  "prod_data" char(1) NOT NULL DEFAULT 'Y',
  "retest_index" tinyint(3)  NOT NULL DEFAULT '0',
  "retest_hbins" varchar(255) DEFAULT NULL,
  "rework_code" tinyint(3)  NOT NULL DEFAULT '0',
  "job_nam" varchar(255) NOT NULL DEFAULT '',
  "job_rev" varchar(255) NOT NULL DEFAULT '',
  "oper_nam" varchar(255) NOT NULL DEFAULT '',
  "exec_typ" varchar(255) NOT NULL DEFAULT '',
  "exec_ver" varchar(255) NOT NULL DEFAULT '',
  "test_cod" varchar(255) NOT NULL DEFAULT '',
  "facil_id" varchar(255) NOT NULL DEFAULT '',
  "tst_temp" varchar(255) NOT NULL DEFAULT '',
  "mode_cod" char(1) DEFAULT NULL,
  "rtst_cod" char(1) DEFAULT NULL,
  "prot_cod" char(1) DEFAULT NULL,
  "burn_tim" int(10) DEFAULT NULL,
  "cmod_cod" char(1) DEFAULT NULL,
  "part_typ" varchar(255) DEFAULT NULL,
  "user_txt" varchar(255) DEFAULT NULL,
  "aux_file" varchar(255) DEFAULT NULL,
  "pkg_typ" varchar(255) DEFAULT NULL,
  "famly_id" varchar(255) DEFAULT NULL,
  "date_cod" varchar(255) DEFAULT NULL,
  "floor_id" varchar(255) DEFAULT NULL,
  "proc_id" varchar(255) DEFAULT NULL,
  "oper_frq" varchar(255) DEFAULT NULL,
  "spec_nam" varchar(255) DEFAULT NULL,
  "spec_ver" varchar(255) DEFAULT NULL,
  "flow_id" varchar(255) DEFAULT NULL,
  "setup_id" varchar(255) DEFAULT NULL,
  "dsgn_rev" varchar(255) DEFAULT NULL,
  "eng_id" varchar(255) DEFAULT NULL,
  "rom_cod" varchar(255) DEFAULT NULL,
  "serl_num" varchar(255) DEFAULT NULL,
  "supr_nam" varchar(255) DEFAULT NULL,
  "nb_sites" tinyint(3)  NOT NULL DEFAULT '1',
  "head_num" tinyint(3)  DEFAULT NULL,
  "handler_typ" varchar(255) DEFAULT NULL,
  "handler_id" varchar(255) DEFAULT NULL,
  "card_typ" varchar(255) DEFAULT NULL,
  "card_id" varchar(255) DEFAULT NULL,
  "loadboard_typ" varchar(255) DEFAULT NULL,
  "loadboard_id" varchar(255) DEFAULT NULL,
  "dib_typ" varchar(255) DEFAULT NULL,
  "dib_id" varchar(255) DEFAULT NULL,
  "cable_typ" varchar(255) DEFAULT NULL,
  "cable_id" varchar(255) DEFAULT NULL,
  "contactor_typ" varchar(255) DEFAULT NULL,
  "contactor_id" varchar(255) DEFAULT NULL,
  "laser_typ" varchar(255) DEFAULT NULL,
  "laser_id" varchar(255) DEFAULT NULL,
  "extra_typ" varchar(255) DEFAULT NULL,
  "extra_id" varchar(255) DEFAULT NULL,
  "file_host_id" int(10)  DEFAULT '0',
  "file_path" varchar(255) NOT NULL DEFAULT '',
  "file_name" varchar(255) NOT NULL DEFAULT '',
  "valid_splitlot" char(1) NOT NULL DEFAULT 'N',
  "insertion_time" int(10)  NOT NULL DEFAULT '0',
  "subcon_lot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) DEFAULT NULL,
  "incremental_update" varchar(255) DEFAULT NULL,
  "sya_id" int(10)  DEFAULT '0',
  "day" varchar(10) NOT NULL,
  "week_nb" tinyint(2)  NOT NULL,
  "month_nb" tinyint(2)  NOT NULL,
  "quarter_nb" tinyint(1)  NOT NULL,
  "year_nb" smallint(4) NOT NULL,
  "year_and_week" varchar(7) NOT NULL,
  "year_and_month" varchar(7) NOT NULL,
  "year_and_quarter" varchar(7) NOT NULL,
  "wafer_nb" tinyint(3)  DEFAULT NULL,
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "wt_splitlot_metadata" (
  "splitlot_id" int(10)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("splitlot_id")
);
CREATE TABLE "wt_sya_set" (
  "sya_id" int(10)  NOT NULL ,
  "product_id" varchar(255) NOT NULL,
  "creation_date" datetime NOT NULL,
  "user_comment" varchar(255) DEFAULT NULL,
  "ll_1" float NOT NULL DEFAULT '-1',
  "hl_1" float NOT NULL DEFAULT '-1',
  "ll_2" float NOT NULL DEFAULT '-1',
  "hl_2" float NOT NULL DEFAULT '-1',
  "start_date" datetime NOT NULL,
  "expiration_date" date NOT NULL,
  "expiration_email_date" datetime DEFAULT NULL,
  "rule_type" varchar(255) NOT NULL,
  "n1_parameter" float NOT NULL DEFAULT '-1',
  "n2_parameter" float NOT NULL DEFAULT '-1',
  "computation_fromdate" date NOT NULL,
  "computation_todate" date NOT NULL,
  "min_lots_required" smallint(5) NOT NULL DEFAULT '-1',
  "min_data_points" smallint(5) NOT NULL DEFAULT '-1',
  "options" tinyint(3)  NOT NULL DEFAULT '0',
  "flags" tinyint(3)  NOT NULL DEFAULT '0',
  "rule_name" varchar(255) DEFAULT NULL,
  "bin_type" tinyint(1) DEFAULT NULL,
  PRIMARY KEY ("sya_id")
);
CREATE TABLE "wt_test_conditions" (
  "splitlot_id" int(10)  NOT NULL,
  "test_info_id" smallint(5)  NOT NULL,
  "test_type" char(1) NOT NULL,
  "condition_1" varchar(255) DEFAULT NULL,
  "condition_2" varchar(255) DEFAULT NULL,
  "condition_3" varchar(255) DEFAULT NULL,
  "condition_4" varchar(255) DEFAULT NULL,
  "condition_5" varchar(255) DEFAULT NULL,
  "condition_6" varchar(255) DEFAULT NULL,
  "condition_7" varchar(255) DEFAULT NULL,
  "condition_8" varchar(255) DEFAULT NULL,
  "condition_9" varchar(255) DEFAULT NULL,
  "condition_10" varchar(255) DEFAULT NULL,
  "condition_11" varchar(255) DEFAULT NULL,
  "condition_12" varchar(255) DEFAULT NULL,
  "condition_13" varchar(255) DEFAULT NULL,
  "condition_14" varchar(255) DEFAULT NULL,
  "condition_15" varchar(255) DEFAULT NULL,
  "condition_16" varchar(255) DEFAULT NULL,
  "condition_17" varchar(255) DEFAULT NULL,
  "condition_18" varchar(255) DEFAULT NULL,
  "condition_19" varchar(255) DEFAULT NULL,
  "condition_20" varchar(255) DEFAULT NULL,
  "condition_21" varchar(255) DEFAULT NULL,
  "condition_22" varchar(255) DEFAULT NULL,
  "condition_23" varchar(255) DEFAULT NULL,
  "condition_24" varchar(255) DEFAULT NULL,
  "condition_25" varchar(255) DEFAULT NULL,
  "condition_26" varchar(255) DEFAULT NULL,
  "condition_27" varchar(255) DEFAULT NULL,
  "condition_28" varchar(255) DEFAULT NULL,
  "condition_29" varchar(255) DEFAULT NULL,
  "condition_30" varchar(255) DEFAULT NULL,
  "condition_31" varchar(255) DEFAULT NULL,
  "condition_32" varchar(255) DEFAULT NULL,
  "condition_33" varchar(255) DEFAULT NULL,
  "condition_34" varchar(255) DEFAULT NULL,
  "condition_35" varchar(255) DEFAULT NULL,
  "condition_36" varchar(255) DEFAULT NULL,
  "condition_37" varchar(255) DEFAULT NULL,
  "condition_38" varchar(255) DEFAULT NULL,
  "condition_39" varchar(255) DEFAULT NULL,
  "condition_40" varchar(255) DEFAULT NULL,
  "condition_41" varchar(255) DEFAULT NULL,
  "condition_42" varchar(255) DEFAULT NULL,
  "condition_43" varchar(255) DEFAULT NULL,
  "condition_44" varchar(255) DEFAULT NULL,
  "condition_45" varchar(255) DEFAULT NULL,
  "condition_46" varchar(255) DEFAULT NULL,
  "condition_47" varchar(255) DEFAULT NULL,
  "condition_48" varchar(255) DEFAULT NULL,
  "condition_49" varchar(255) DEFAULT NULL,
  "condition_50" varchar(255) DEFAULT NULL,
  "condition_51" varchar(255) DEFAULT NULL,
  "condition_52" varchar(255) DEFAULT NULL,
  "condition_53" varchar(255) DEFAULT NULL,
  "condition_54" varchar(255) DEFAULT NULL,
  "condition_55" varchar(255) DEFAULT NULL,
  "condition_56" varchar(255) DEFAULT NULL,
  "condition_57" varchar(255) DEFAULT NULL,
  "condition_58" varchar(255) DEFAULT NULL,
  "condition_59" varchar(255) DEFAULT NULL,
  "condition_60" varchar(255) DEFAULT NULL,
  "condition_61" varchar(255) DEFAULT NULL,
  "condition_62" varchar(255) DEFAULT NULL,
  "condition_63" varchar(255) DEFAULT NULL,
  "condition_64" varchar(255) DEFAULT NULL,
  "condition_65" varchar(255) DEFAULT NULL,
  "condition_66" varchar(255) DEFAULT NULL,
  "condition_67" varchar(255) DEFAULT NULL,
  "condition_68" varchar(255) DEFAULT NULL,
  "condition_69" varchar(255) DEFAULT NULL,
  "condition_70" varchar(255) DEFAULT NULL,
  "condition_71" varchar(255) DEFAULT NULL,
  "condition_72" varchar(255) DEFAULT NULL,
  "condition_73" varchar(255) DEFAULT NULL,
  "condition_74" varchar(255) DEFAULT NULL,
  "condition_75" varchar(255) DEFAULT NULL,
  "condition_76" varchar(255) DEFAULT NULL,
  "condition_77" varchar(255) DEFAULT NULL,
  "condition_78" varchar(255) DEFAULT NULL,
  "condition_79" varchar(255) DEFAULT NULL,
  "condition_80" varchar(255) DEFAULT NULL,
  "condition_81" varchar(255) DEFAULT NULL,
  "condition_82" varchar(255) DEFAULT NULL,
  "condition_83" varchar(255) DEFAULT NULL,
  "condition_84" varchar(255) DEFAULT NULL,
  "condition_85" varchar(255) DEFAULT NULL,
  "condition_86" varchar(255) DEFAULT NULL,
  "condition_87" varchar(255) DEFAULT NULL,
  "condition_88" varchar(255) DEFAULT NULL,
  "condition_89" varchar(255) DEFAULT NULL,
  "condition_90" varchar(255) DEFAULT NULL,
  "condition_91" varchar(255) DEFAULT NULL,
  "condition_92" varchar(255) DEFAULT NULL,
  "condition_93" varchar(255) DEFAULT NULL,
  "condition_94" varchar(255) DEFAULT NULL,
  "condition_95" varchar(255) DEFAULT NULL,
  "condition_96" varchar(255) DEFAULT NULL,
  "condition_97" varchar(255) DEFAULT NULL,
  "condition_98" varchar(255) DEFAULT NULL,
  "condition_99" varchar(255) DEFAULT NULL,
  "condition_100" varchar(255) DEFAULT NULL,
  PRIMARY KEY ("splitlot_id","test_info_id","test_type")
);
CREATE TABLE "wt_wafer_consolidation" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "consolidated_data_type" varchar(255) NOT NULL,
  "consolidation_name" varchar(255) NOT NULL,
  "consolidation_prod_flow" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","wafer_id","consolidation_name")
);
CREATE TABLE "wt_wafer_consolidation_inter" (
  "lot_id" varchar(255) NOT NULL DEFAULT '',
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "consolidated_data_type" varchar(255) NOT NULL,
  "consolidation_name" varchar(255) NOT NULL,
  "consolidation_prod_flow" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","wafer_id","consolidation_name")
);
CREATE TABLE "wt_wafer_hbin" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","wafer_id","hbin_no")
);
CREATE TABLE "wt_wafer_hbin_inter" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "hbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "hbin_name" varchar(255) NOT NULL DEFAULT '',
  "hbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  "consolidation_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","wafer_id","hbin_no","consolidation_name")
);
CREATE TABLE "wt_wafer_info" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL,
  "product_name" varchar(255) NOT NULL,
  "fab_id" varchar(255) DEFAULT NULL,
  "frame_id" varchar(255) DEFAULT NULL,
  "mask_id" varchar(255) DEFAULT NULL,
  "wafer_size" float DEFAULT NULL,
  "die_ht" float DEFAULT NULL,
  "die_wid" float DEFAULT NULL,
  "wafer_units" tinyint(3)  DEFAULT NULL,
  "wafer_flat" char(1) DEFAULT NULL,
  "center_x" smallint(5)  DEFAULT NULL,
  "center_y" smallint(5)  DEFAULT NULL,
  "pos_x" char(1) DEFAULT NULL,
  "pos_y" char(1) DEFAULT NULL,
  "gross_die" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts" mediumint(8) NOT NULL DEFAULT '0',
  "nb_parts_good" mediumint(8) NOT NULL DEFAULT '0',
  "flags" binary(2) DEFAULT NULL,
  "wafer_nb" tinyint(3)  DEFAULT NULL,
  "consolidation_status" varchar(255) DEFAULT NULL,
  "consolidation_ref_date" datetime DEFAULT NULL,
  PRIMARY KEY ("lot_id","wafer_id")
);
CREATE TABLE "wt_wafer_sbin" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  PRIMARY KEY ("lot_id","wafer_id","sbin_no")
);
CREATE TABLE "wt_wafer_sbin_inter" (
  "lot_id" varchar(255) NOT NULL,
  "wafer_id" varchar(255) NOT NULL DEFAULT '',
  "sbin_no" smallint(5)  NOT NULL DEFAULT '0',
  "sbin_name" varchar(255) NOT NULL DEFAULT '',
  "sbin_cat" char(1) DEFAULT NULL,
  "nb_parts" mediumint(8)  NOT NULL DEFAULT '0',
  "consolidation_name" varchar(255) NOT NULL,
  PRIMARY KEY ("lot_id","wafer_id","sbin_no","consolidation_name")
);
CREATE TABLE "wt_wyr" (
  "wyr_id" int(11) NOT NULL ,
  "site_name" varchar(255) NOT NULL,
  "week_nb" tinyint(3) DEFAULT NULL,
  "year" smallint(5) DEFAULT NULL,
  "date_in" datetime DEFAULT NULL,
  "date_out" datetime DEFAULT NULL,
  "product_name" varchar(255) DEFAULT NULL,
  "program_name" varchar(255) DEFAULT NULL,
  "tester_name" varchar(255) DEFAULT NULL,
  "lot_id" varchar(255) DEFAULT NULL,
  "subcon_lot_id" varchar(255) DEFAULT NULL,
  "user_split" varchar(1024) DEFAULT NULL,
  "yield" float DEFAULT '0',
  "parts_received" int(10) DEFAULT '0',
  "pretest_rejects" int(10) DEFAULT '0',
  "pretest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_tested" int(10) DEFAULT '0',
  "parts_pass" int(10) DEFAULT '0',
  "parts_pass_split" varchar(1024) DEFAULT NULL,
  "parts_fail" int(10) DEFAULT '0',
  "parts_fail_split" varchar(1024) DEFAULT NULL,
  "parts_retest" int(10) DEFAULT '0',
  "parts_retest_split" varchar(1024) DEFAULT NULL,
  "insertions" int(10) DEFAULT '0',
  "posttest_rejects" int(10) DEFAULT '0',
  "posttest_rejects_split" varchar(1024) DEFAULT NULL,
  "parts_shipped" int(10) DEFAULT '0',
  PRIMARY KEY ("wyr_id")
);
CREATE TABLE "wt_wyr_format" (
  "site_name" varchar(255) NOT NULL,
  "column_id" tinyint(3) NOT NULL,
  "column_nb" tinyint(3) NOT NULL,
  "column_name" varchar(255) NOT NULL,
  "data_type" varchar(255) NOT NULL,
  "display" char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY ("site_name","column_id","data_type")
);
CREATE INDEX "ft_run_dietrace_ftrundietrace_part" ON "ft_run_dietrace" ("part_id");
CREATE INDEX "ft_lot_ftlot_tl" ON "ft_lot" ("tracking_lot_id");
CREATE INDEX "ft_lot_ftlot_product_lot" ON "ft_lot" ("product_name","lot_id");
CREATE INDEX "et_sya_set_etsyaset_product_creationdate" ON "et_sya_set" ("product_id","creation_date");
CREATE INDEX "wt_wafer_info_wtwaferinfo_wafer" ON "wt_wafer_info" ("wafer_id");
CREATE INDEX "wt_splitlot_wtsplitlot_wafer" ON "wt_splitlot" ("wafer_id");
CREATE INDEX "wt_splitlot_wtsplitlot_lot" ON "wt_splitlot" ("lot_id");
CREATE INDEX "wt_splitlot_wtsplitlot_startt" ON "wt_splitlot" ("start_t");
CREATE INDEX "et_splitlot_etsplitlot_lot" ON "et_splitlot" ("lot_id");
CREATE INDEX "et_splitlot_etsplitlot_startt" ON "et_splitlot" ("start_t");
CREATE INDEX "ft_splitlot_ftsplitlot_lot" ON "ft_splitlot" ("lot_id");
CREATE INDEX "ft_splitlot_ftsplitlot_startt" ON "ft_splitlot" ("start_t");
CREATE INDEX "et_lot_etlot_product_lot" ON "et_lot" ("product_name","lot_id");
CREATE INDEX "ft_event_ftevent_splitlot_run" ON "ft_event" ("splitlot_id","run_id");
CREATE INDEX "ft_ptest_rollingstats_ftptestrollingstats_splitlot_run" ON "ft_ptest_rollingstats" ("splitlot_id","run_id");
CREATE INDEX "ft_ptest_outliers_ftptestoutliers_splitlot_run" ON "ft_ptest_outliers" ("splitlot_id","run_id");
CREATE INDEX "ft_ptest_outliers_ftptestoutliers_splitlot_ptest_limitsrun" ON "ft_ptest_outliers" ("splitlot_id","ptest_info_id","limits_run_id");
CREATE INDEX "wt_consolidated_wafer_wtconsolidatedwafer_tl" ON "wt_consolidated_wafer" ("tracking_lot_id");
CREATE INDEX "ft_ptest_rollinglimits_ftptestrollinglimits_splitlot_run" ON "ft_ptest_rollinglimits" ("splitlot_id","run_id");
CREATE INDEX "ft_consolidated_tl_ftconsolidatedtl_tl" ON "ft_consolidated_tl" ("tracking_lot_id");
CREATE INDEX "ft_sya_set_ftsyaset_product_creationdate" ON "ft_sya_set" ("product_id","creation_date");
CREATE INDEX "ft_mptest_outliers_ftmptestoutliers_splitlot_run" ON "ft_mptest_outliers" ("splitlot_id","run_id");
CREATE INDEX "ft_mptest_outliers_ftmptestoutliers_splitlot_mptest_limitsrun" ON "ft_mptest_outliers" ("splitlot_id","mptest_info_id","limits_run_id");
CREATE INDEX "ft_consolidated_sublot_ftconsolidatedsublot_sublot" ON "ft_consolidated_sublot" ("sublot_id");
CREATE INDEX "ft_consolidated_sublot_ftconsolidatedsublot_tl" ON "ft_consolidated_sublot" ("tracking_lot_id");
CREATE INDEX "wt_lot_wtlot_tl" ON "wt_lot" ("tracking_lot_id");
CREATE INDEX "wt_lot_wtlot_product_lot" ON "wt_lot" ("product_name","lot_id");
CREATE INDEX "wt_consolidated_tl_wtconsolidatedtl_tl" ON "wt_consolidated_tl" ("tracking_lot_id");
CREATE INDEX "ft_die_tracking_ftdietracking_wttl" ON "ft_die_tracking" ("wt_tracking_lot_id");
CREATE INDEX "wt_sya_set_wtsyaset_product_creationdate" ON "wt_sya_set" ("product_id","creation_date");
CREATE INDEX "global_files_globalfiles_file_filetype_fileformat" ON "global_files" ("file_name","file_type","file_format");
CREATE INDEX "ft_sublot_info_ftsublotinfo_sublot" ON "ft_sublot_info" ("sublot_id");
CREATE INDEX "ft_mptest_rollingstats_ftmptestrollingstats_splitlot_run" ON "ft_mptest_rollingstats" ("splitlot_id","run_id");
CREATE INDEX "ft_dietrace_config_ftdietraceconfig_product" ON "ft_dietrace_config" ("product");
CREATE INDEX "ft_dietrace_config_ftdietraceconfig_lot" ON "ft_dietrace_config" ("lot_id");
CREATE INDEX "ft_dietrace_config_ftdietraceconfig_wafer" ON "ft_dietrace_config" ("wafer_id");
CREATE INDEX "ft_dietrace_config_ftdietraceconfig_die" ON "ft_dietrace_config" ("die_index");
CREATE INDEX "ft_mptest_rollinglimits_ftmptestrollinglimits_splitlot_run" ON "ft_mptest_rollinglimits" ("splitlot_id","run_id");
