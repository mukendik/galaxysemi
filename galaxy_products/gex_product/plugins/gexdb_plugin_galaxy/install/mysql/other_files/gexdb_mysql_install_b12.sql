-- MySQL Administrator dump 1.4
--
-- ------------------------------------------------------
-- Server version	5.1.40-community


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE=NO_AUTO_VALUE_ON_ZERO */;

--
-- Table structure for table global_info
--

DROP TABLE IF EXISTS global_info;
CREATE TABLE global_info (
  db_version_name varchar(255) NOT NULL,
  db_version_nb smallint(5) NOT NULL,
  db_version_build smallint(5) NOT NULL,
  incremental_splitlots int(9) NOT NULL DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

INSERT INTO global_info VALUES ('GEXDB V1.7 B12 (MySQL)', 17, 12, 0);

--
-- Table structure for table global_options
--

DROP TABLE IF EXISTS global_options;
CREATE TABLE global_options (
  option_name varchar(255) NOT NULL,
  option_value varchar(255) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

INSERT INTO global_options VALUES ('GEXDB_MYSQL_ENGINE', 'MyISAM');

--
-- Table structure for table gexdb_log
--

DROP TABLE IF EXISTS gexdb_log;
CREATE TABLE gexdb_log (
  log_date datetime NOT NULL,
  log_type varchar(255) NOT NULL,
  log_string varchar(1024) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table incremental_update
--

DROP TABLE IF EXISTS incremental_update;
CREATE TABLE incremental_update (
  db_update_name varchar(255) NOT NULL,
  initial_splitlots int(9) NOT NULL DEFAULT 0,
  remaining_splitlots int(9) NOT NULL DEFAULT 0,
  db_version_build smallint(5) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table product
--

DROP TABLE IF EXISTS product;
CREATE TABLE product (
  product_name varchar(255) NOT NULL DEFAULT '',
  description varchar(1000) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_dtr
--

DROP TABLE IF EXISTS et_dtr;
CREATE TABLE et_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT NULL,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  KEY et_dtr (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_hbin
--

DROP TABLE IF EXISTS et_hbin;
CREATE TABLE et_hbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY et_hbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_lot
--

DROP TABLE IF EXISTS et_lot;
CREATE TABLE et_lot (
  lot_id varchar(255) NOT NULL DEFAULT '',
  tracking_lot_id varchar(255) DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  nb_parts int(10) NOT NULL DEFAULT 0,
  nb_parts_good int(10) NOT NULL DEFAULT 0,
  flags binary(2) DEFAULT NULL,
  KEY et_lot (lot_id),
  KEY etlot_productname_lot (product_name,lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;


--
-- Table structure for table et_lot_hbin
--

DROP TABLE IF EXISTS et_lot_hbin;
CREATE TABLE et_lot_hbin (
  lot_id varchar(255) NOT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY etlothbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_lot_sbin
--

DROP TABLE IF EXISTS et_lot_sbin;
CREATE TABLE et_lot_sbin (
  lot_id varchar(255) NOT NULL,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY etlotsbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_metadata_link
--

DROP TABLE IF EXISTS et_metadata_link;
CREATE TABLE et_metadata_link (
  link_name varchar(255) NOT NULL,
  gexdb_table1_name varchar(255) NOT NULL,
  gexdb_field1_fullname varchar(255) NOT NULL,
  gexdb_table2_name varchar(255) NOT NULL,
  gexdb_field2_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_metadata_mapping
--

DROP TABLE IF EXISTS et_metadata_mapping;
CREATE TABLE et_metadata_mapping (
  meta_name varchar(255) NOT NULL,
  gex_name varchar(255) DEFAULT NULL,
  gexdb_table_name varchar(255) NOT NULL,
  gexdb_field_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL,
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  bintype_field char(1) NOT NULL DEFAULT 'N',
  time_field char(1) NOT NULL DEFAULT 'N',
  custom_field char(1) NOT NULL DEFAULT 'Y',
  numeric_field char(1) NOT NULL DEFAULT 'N',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_prod_alarm
--

DROP TABLE IF EXISTS et_prod_alarm;
CREATE TABLE et_prod_alarm (
  splitlot_id int(10) unsigned NOT NULL,
  alarm_cat varchar(255) NOT NULL,
  alarm_type varchar(255) NOT NULL,
  item_no int(10) unsigned NOT NULL,
  item_name varchar(255) DEFAULT NULL,
  flags binary(2) NOT NULL,
  lcl float NOT NULL DEFAULT 0,
  ucl float NOT NULL DEFAULT 0,
  `value` float NOT NULL DEFAULT 0,
  units varchar(10) DEFAULT NULL,
  KEY etprodalarm_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;


--
-- Table structure for table et_product_hbin
--

DROP TABLE IF EXISTS et_product_hbin;
CREATE TABLE et_product_hbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY et_product_hbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_product_sbin
--

DROP TABLE IF EXISTS et_product_sbin;
CREATE TABLE et_product_sbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY et_product_sbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_ptest_info
--

DROP TABLE IF EXISTS et_ptest_info;
CREATE TABLE et_ptest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  units varchar(255) NOT NULL DEFAULT '',
  flags binary(1) NOT NULL DEFAULT '\0',
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  testseq smallint(5) unsigned DEFAULT NULL,
  spec_ll float DEFAULT NULL,
  spec_hl float DEFAULT NULL,
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  KEY etptest_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_ptest_results
--

DROP TABLE IF EXISTS et_ptest_results;
CREATE TABLE et_ptest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  run_id smallint(5) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  `value` float DEFAULT NULL,
  KEY et_ptest_results (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;


--
-- Table structure for table et_ptest_stats
--

DROP TABLE IF EXISTS et_ptest_stats;
CREATE TABLE et_ptest_stats (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  exec_count smallint(5) unsigned DEFAULT NULL,
  fail_count smallint(5) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  KEY etpteststats_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_run
--

DROP TABLE IF EXISTS et_run;
CREATE TABLE et_run (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  run_id smallint(5) unsigned NOT NULL DEFAULT 0,
  part_id varchar(255) DEFAULT NULL,
  part_x smallint(6) DEFAULT NULL,
  part_y smallint(6) DEFAULT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0,
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0,
  firstfail_tnum int(10) unsigned DEFAULT NULL,
  firstfail_tname varchar(255) DEFAULT NULL,
  retest_index tinyint(3) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  part_txt varchar(255) DEFAULT NULL,
  KEY et_run (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_sbin
--

DROP TABLE IF EXISTS et_sbin;
CREATE TABLE et_sbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY et_sbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_sbl
--

DROP TABLE IF EXISTS et_sbl;
CREATE TABLE et_sbl (
  sya_id int(10) unsigned NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_splitlot
--

DROP TABLE IF EXISTS et_splitlot;
CREATE TABLE et_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  start_t int(10) NOT NULL DEFAULT 0,
  finish_t int(10) NOT NULL DEFAULT 0,
  tester_name varchar(255) NOT NULL DEFAULT '',
  tester_type varchar(255) NOT NULL DEFAULT '',
  flags binary(2) NOT NULL DEFAULT '\0\0',
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  data_provider varchar(255) DEFAULT '',
  data_type varchar(255) DEFAULT '',
  prod_data char(1) NOT NULL DEFAULT 'Y',
  job_nam varchar(255) NOT NULL DEFAULT '',
  job_rev varchar(255) NOT NULL DEFAULT '',
  oper_nam varchar(255) NOT NULL DEFAULT '',
  exec_typ varchar(255) NOT NULL DEFAULT '',
  exec_ver varchar(255) NOT NULL DEFAULT '',
  facil_id varchar(255) NOT NULL DEFAULT '',
  part_typ varchar(255) DEFAULT NULL,
  user_txt varchar(255) DEFAULT NULL,
  famly_id varchar(255) DEFAULT NULL,
  proc_id varchar(255) DEFAULT NULL,
  file_host_id int(10) unsigned DEFAULT 0,
  file_path varchar(255) NOT NULL DEFAULT '',
  file_name varchar(255) NOT NULL DEFAULT '',
  valid_splitlot char(1) NOT NULL DEFAULT 'N',
  insertion_time int(10) unsigned NOT NULL DEFAULT 0,
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) DEFAULT NULL,
  incremental_update varchar(255) DEFAULT NULL,
  sya_id int(10) unsigned DEFAULT 0,
  `day` varchar(10) NOT NULL,
  week_nb tinyint(2) unsigned NOT NULL,
  month_nb tinyint(2) unsigned NOT NULL,
  quarter_nb tinyint(1) unsigned NOT NULL,
  year_nb smallint(4) NOT NULL,
  year_and_week varchar(7) NOT NULL,
  year_and_month varchar(7) NOT NULL,
  year_and_quarter varchar(7) NOT NULL,
  PRIMARY KEY (splitlot_id),
  KEY et_splitlot (lot_id),
  KEY etsplitlot_startt (start_t)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_sya_set
--

DROP TABLE IF EXISTS et_sya_set;
CREATE TABLE et_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  product_id varchar(255) NOT NULL,
  creation_date datetime NOT NULL,
  user_comment varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  start_date datetime NOT NULL,
  expiration_date date NOT NULL,
  expiration_email_date datetime DEFAULT NULL,
  rule_type varchar(255) NOT NULL,
  n1_parameter float NOT NULL DEFAULT -1,
  n2_parameter float NOT NULL DEFAULT -1,
  computation_fromdate date NOT NULL,
  computation_todate date NOT NULL,
  min_lots_required smallint(5) NOT NULL DEFAULT -1,
  min_data_points smallint(5) NOT NULL DEFAULT -1,
  `options` tinyint(3) unsigned NOT NULL DEFAULT 0,
  flags tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (sya_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_wafer_hbin
--

DROP TABLE IF EXISTS et_wafer_hbin;
CREATE TABLE et_wafer_hbin (
  lot_id varchar(255) NOT NULL,
  wafer_id varchar(255) DEFAULT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY et_wafer_hbin (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_wafer_info
--

DROP TABLE IF EXISTS et_wafer_info;
CREATE TABLE et_wafer_info (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) DEFAULT NULL,
  fab_id varchar(255) DEFAULT NULL,
  frame_id varchar(255) DEFAULT NULL,
  mask_id varchar(255) DEFAULT NULL,
  wafer_size float DEFAULT NULL,
  die_ht float DEFAULT NULL,
  die_wid float DEFAULT NULL,
  wafer_units tinyint(3) unsigned DEFAULT NULL,
  wafer_flat char(1) DEFAULT NULL,
  center_x smallint(5) unsigned DEFAULT NULL,
  center_y smallint(5) unsigned DEFAULT NULL,
  pos_x char(1) DEFAULT NULL,
  pos_y char(1) DEFAULT NULL,
  gross_die mediumint(8) NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  flags binary(2) DEFAULT NULL,
  KEY et_wafer_info (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_wafer_sbin
--

DROP TABLE IF EXISTS et_wafer_sbin;
CREATE TABLE et_wafer_sbin (
  lot_id varchar(255) NOT NULL,
  wafer_id varchar(255) DEFAULT NULL,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY et_wafer_sbin (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_wyr
--

DROP TABLE IF EXISTS et_wyr;
CREATE TABLE et_wyr (
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  `year` smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT 0,
  parts_received int(10) DEFAULT 0,
  pretest_rejects int(10) DEFAULT 0,
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT 0,
  parts_pass int(10) DEFAULT 0,
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT 0,
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT 0,
  parts_retest_split varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT 0,
  posttest_rejects int(10) DEFAULT 0,
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table et_wyr_format
--

DROP TABLE IF EXISTS et_wyr_format;
CREATE TABLE et_wyr_format (
  site_name varchar(255) NOT NULL,
  column_id tinyint(3) NOT NULL,
  column_nb tinyint(3) NOT NULL,
  column_name varchar(255) NOT NULL,
  data_type varchar(255) NOT NULL,
  display char(1) NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table file_host
--

DROP TABLE IF EXISTS file_host;
CREATE TABLE file_host (
  file_host_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  host_name varchar(255) NOT NULL DEFAULT '',
  host_ftpuser varchar(255) NOT NULL DEFAULT '',
  host_ftppassword varchar(255) NOT NULL DEFAULT '',
  host_ftppath varchar(255) DEFAULT NULL,
  host_ftpport smallint(5) unsigned NOT NULL DEFAULT 21,
  PRIMARY KEY (file_host_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_dtr
--

DROP TABLE IF EXISTS ft_dtr;
CREATE TABLE ft_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT NULL,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  KEY ft_dtr (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_die_tracking
--

DROP TABLE IF EXISTS ft_die_tracking;
CREATE TABLE ft_die_tracking (
  ft_tracking_lot_id varchar(255) NOT NULL,
  wt_product_id varchar(255) NOT NULL,
  wt_tracking_lot_id varchar(255) NOT NULL,
  wt_sublot_id varchar(255) NOT NULL,
  die_id varchar(255) NOT NULL,
  KEY ft_multidie_tracking_1 (ft_tracking_lot_id),
  KEY ft_multidie_tracking_2 (wt_tracking_lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ftest_info
--

DROP TABLE IF EXISTS ft_ftest_info;
CREATE TABLE ft_ftest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  testseq smallint(5) unsigned DEFAULT NULL,
  KEY ft_ftest_info (splitlot_id),
  KEY ftftest_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ftest_results
--

DROP TABLE IF EXISTS ft_ftest_results;
CREATE TABLE ft_ftest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  vect_nam varchar(255) NOT NULL DEFAULT '',
  vect_off smallint(6) DEFAULT NULL,
  KEY ft_ftest_results (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ftest_stats_samples
--

DROP TABLE IF EXISTS ft_ftest_stats_samples;
CREATE TABLE ft_ftest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  KEY ftftestsamp_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ftest_stats_summary
--

DROP TABLE IF EXISTS ft_ftest_stats_summary;
CREATE TABLE ft_ftest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned DEFAULT NULL,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY ftftestsum_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_hbin
--

DROP TABLE IF EXISTS ft_hbin;
CREATE TABLE ft_hbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  KEY ft_hbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_hbin_stats_samples
--

DROP TABLE IF EXISTS ft_hbin_stats_samples;
CREATE TABLE ft_hbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  KEY ft_hbin_stats_samples (splitlot_id,site_no,hbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_hbin_stats_summary
--

DROP TABLE IF EXISTS ft_hbin_stats_summary;
CREATE TABLE ft_hbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY ft_hbin_stats_summary (splitlot_id,site_no,hbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_lot
--

DROP TABLE IF EXISTS ft_lot;
CREATE TABLE ft_lot (
  lot_id varchar(255) NOT NULL DEFAULT '',
  tracking_lot_id varchar(255) DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  flags binary(2) DEFAULT NULL,
  KEY ft_lot (lot_id),
  KEY ftlot_productname_lot (product_name,lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_lot_hbin
--

DROP TABLE IF EXISTS ft_lot_hbin;
CREATE TABLE ft_lot_hbin (
  lot_id varchar(255) NOT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY ftlothbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_lot_sbin
--

DROP TABLE IF EXISTS ft_lot_sbin;
CREATE TABLE ft_lot_sbin (
  lot_id varchar(255) NOT NULL,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY ftlotsbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_metadata_link
--

DROP TABLE IF EXISTS ft_metadata_link;
CREATE TABLE ft_metadata_link (
  link_name varchar(255) NOT NULL,
  gexdb_table1_name varchar(255) NOT NULL,
  gexdb_field1_fullname varchar(255) NOT NULL,
  gexdb_table2_name varchar(255) NOT NULL,
  gexdb_field2_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_metadata_mapping
--

DROP TABLE IF EXISTS ft_metadata_mapping;
CREATE TABLE ft_metadata_mapping (
  meta_name varchar(255) NOT NULL,
  gex_name varchar(255) DEFAULT NULL,
  gexdb_table_name varchar(255) NOT NULL,
  gexdb_field_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL,
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  bintype_field char(1) NOT NULL DEFAULT 'N',
  time_field char(1) NOT NULL DEFAULT 'N',
  custom_field char(1) NOT NULL DEFAULT 'Y',
  numeric_field char(1) NOT NULL DEFAULT 'N',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_mptest_info
--

DROP TABLE IF EXISTS ft_mptest_info;
CREATE TABLE ft_mptest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  tpin_arrayindex smallint(6) NOT NULL DEFAULT 0,
  units varchar(255) NOT NULL DEFAULT '',
  flags binary(1) NOT NULL DEFAULT '\0',
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  testseq smallint(5) unsigned DEFAULT NULL,
  spec_ll float DEFAULT NULL,
  spec_hl float DEFAULT NULL,
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  KEY ft_mptest_info (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_mptest_limits
--

DROP TABLE IF EXISTS ft_mptest_limits;
CREATE TABLE ft_mptest_limits (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  LL float DEFAULT NULL,
  HL float DEFAULT NULL,
  KEY ftmptestlimits_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_mptest_results
--

DROP TABLE IF EXISTS ft_mptest_results;
CREATE TABLE ft_mptest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(8) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  `value` float NOT NULL DEFAULT 0,
  tpin_pmrindex smallint(6) DEFAULT NULL,
  KEY ft_mptest_results (splitlot_id,mptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_mptest_stats_samples
--

DROP TABLE IF EXISTS ft_mptest_stats_samples;
CREATE TABLE ft_mptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  KEY ftmptestsamp_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_mptest_stats_summary
--

DROP TABLE IF EXISTS ft_mptest_stats_summary;
CREATE TABLE ft_mptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY ftmptestsum_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_parts_stats_samples
--

DROP TABLE IF EXISTS ft_parts_stats_samples;
CREATE TABLE ft_parts_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  KEY ft_parts_stats_samples (splitlot_id,site_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_parts_stats_summary
--

DROP TABLE IF EXISTS ft_parts_stats_summary;
CREATE TABLE ft_parts_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_good mediumint(8) DEFAULT NULL,
  nb_rtst mediumint(8) DEFAULT NULL,
  KEY ft_parts_stats_summary (splitlot_id,site_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_prod_alarm
--

DROP TABLE IF EXISTS ft_prod_alarm;
CREATE TABLE ft_prod_alarm (
  splitlot_id int(10) unsigned NOT NULL,
  alarm_cat varchar(255) NOT NULL,
  alarm_type varchar(255) NOT NULL,
  item_no int(10) unsigned NOT NULL,
  item_name varchar(255) DEFAULT NULL,
  flags binary(2) NOT NULL,
  lcl float NOT NULL DEFAULT 0,
  ucl float NOT NULL DEFAULT 0,
  `value` float NOT NULL DEFAULT 0,
  units varchar(10) DEFAULT NULL,
  KEY ftprodalarm_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_product_hbin
--

DROP TABLE IF EXISTS ft_product_hbin;
CREATE TABLE ft_product_hbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY ft_product_hbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_product_sbin
--

DROP TABLE IF EXISTS ft_product_sbin;
CREATE TABLE ft_product_sbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY ft_product_sbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ptest_info
--

DROP TABLE IF EXISTS ft_ptest_info;
CREATE TABLE ft_ptest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  units varchar(255) NOT NULL DEFAULT '',
  flags binary(1) NOT NULL DEFAULT '\0',
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  testseq smallint(5) unsigned DEFAULT NULL,
  spec_ll float DEFAULT NULL,
  spec_hl float DEFAULT NULL,
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  KEY ftptest_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ptest_limits
--

DROP TABLE IF EXISTS ft_ptest_limits;
CREATE TABLE ft_ptest_limits (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  LL float DEFAULT NULL,
  HL float DEFAULT NULL,
  KEY ftptestlimits_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ptest_results
--

DROP TABLE IF EXISTS ft_ptest_results;
CREATE TABLE ft_ptest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(8) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  `value` float DEFAULT NULL,
  KEY ft_ptest_results (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ptest_stats_samples
--

DROP TABLE IF EXISTS ft_ptest_stats_samples;
CREATE TABLE ft_ptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  KEY ftptestsamp_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_ptest_stats_summary
--

DROP TABLE IF EXISTS ft_ptest_stats_summary;
CREATE TABLE ft_ptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned DEFAULT NULL,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY ftptestsum_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_run
--

DROP TABLE IF EXISTS ft_run;
CREATE TABLE ft_run (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(8) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  part_id varchar(255) DEFAULT NULL,
  part_x smallint(6) DEFAULT NULL,
  part_y smallint(6) DEFAULT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0,
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0,
  firstfail_tnum int(10) unsigned DEFAULT NULL,
  firstfail_tname varchar(255) DEFAULT NULL,
  retest_index tinyint(3) unsigned NOT NULL DEFAULT 0,
  wafer_id varchar(255) DEFAULT NULL,
  part_txt varchar(255) DEFAULT NULL,
  KEY ft_run (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_sbin
--

DROP TABLE IF EXISTS ft_sbin;
CREATE TABLE ft_sbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  KEY ft_sbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_sbin_stats_samples
--

DROP TABLE IF EXISTS ft_sbin_stats_samples;
CREATE TABLE ft_sbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  KEY ft_sbin_stats_samples (splitlot_id,site_no,sbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_sbin_stats_summary
--

DROP TABLE IF EXISTS ft_sbin_stats_summary;
CREATE TABLE ft_sbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY ft_sbin_stats_summary (splitlot_id,site_no,sbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_sbl
--

DROP TABLE IF EXISTS ft_sbl;
CREATE TABLE ft_sbl (
  sya_id int(10) unsigned NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_splitlot
--

DROP TABLE IF EXISTS ft_splitlot;
CREATE TABLE ft_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  setup_t int(10) NOT NULL DEFAULT 0,
  start_t int(10) NOT NULL DEFAULT 0,
  finish_t int(10) NOT NULL DEFAULT 0,
  stat_num tinyint(3) unsigned NOT NULL DEFAULT 0,
  tester_name varchar(255) NOT NULL DEFAULT '',
  tester_type varchar(255) NOT NULL DEFAULT '',
  flags binary(2) NOT NULL DEFAULT '\0\0',
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_samples mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_summary mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT 0,
  data_provider varchar(255) DEFAULT '',
  data_type varchar(255) DEFAULT '',
  prod_data char(1) NOT NULL DEFAULT 'Y',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT 0,
  retest_hbins varchar(255) DEFAULT NULL,
  rework_code tinyint(3) unsigned NOT NULL DEFAULT 0,
  job_nam varchar(255) NOT NULL DEFAULT '',
  job_rev varchar(255) NOT NULL DEFAULT '',
  oper_nam varchar(255) NOT NULL DEFAULT '',
  exec_typ varchar(255) NOT NULL DEFAULT '',
  exec_ver varchar(255) NOT NULL DEFAULT '',
  test_cod varchar(255) NOT NULL DEFAULT '',
  facil_id varchar(255) NOT NULL DEFAULT '',
  tst_temp varchar(255) NOT NULL DEFAULT '',
  mode_cod char(1) DEFAULT NULL,
  rtst_cod char(1) DEFAULT NULL,
  prot_cod char(1) DEFAULT NULL,
  burn_tim int(10) DEFAULT NULL,
  cmod_cod char(1) DEFAULT NULL,
  part_typ varchar(255) DEFAULT NULL,
  user_txt varchar(255) DEFAULT NULL,
  aux_file varchar(255) DEFAULT NULL,
  pkg_typ varchar(255) DEFAULT NULL,
  famly_id varchar(255) DEFAULT NULL,
  date_cod varchar(255) DEFAULT NULL,
  floor_id varchar(255) DEFAULT NULL,
  proc_id varchar(255) DEFAULT NULL,
  oper_frq varchar(255) DEFAULT NULL,
  spec_nam varchar(255) DEFAULT NULL,
  spec_ver varchar(255) DEFAULT NULL,
  flow_id varchar(255) DEFAULT NULL,
  setup_id varchar(255) DEFAULT NULL,
  dsgn_rev varchar(255) DEFAULT NULL,
  eng_id varchar(255) DEFAULT NULL,
  rom_cod varchar(255) DEFAULT NULL,
  serl_num varchar(255) DEFAULT NULL,
  supr_nam varchar(255) DEFAULT NULL,
  nb_sites tinyint(3) unsigned NOT NULL DEFAULT 1,
  head_num tinyint(3) unsigned DEFAULT NULL,
  handler_typ varchar(255) DEFAULT NULL,
  handler_id varchar(255) DEFAULT NULL,
  card_typ varchar(255) DEFAULT NULL,
  card_id varchar(255) DEFAULT NULL,
  loadboard_typ varchar(255) DEFAULT NULL,
  loadboard_id varchar(255) DEFAULT NULL,
  dib_typ varchar(255) DEFAULT NULL,
  dib_id varchar(255) DEFAULT NULL,
  cable_typ varchar(255) DEFAULT NULL,
  cable_id varchar(255) DEFAULT NULL,
  contactor_typ varchar(255) DEFAULT NULL,
  contactor_id varchar(255) DEFAULT NULL,
  laser_typ varchar(255) DEFAULT NULL,
  laser_id varchar(255) DEFAULT NULL,
  extra_typ varchar(255) DEFAULT NULL,
  extra_id varchar(255) DEFAULT NULL,
  file_host_id int(10) unsigned DEFAULT 0,
  file_path varchar(255) NOT NULL DEFAULT '',
  file_name varchar(255) NOT NULL DEFAULT '',
  valid_splitlot char(1) NOT NULL DEFAULT 'N',
  insertion_time int(10) unsigned NOT NULL DEFAULT 0,
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  incremental_update varchar(255) DEFAULT NULL,
  sya_id int(10) unsigned DEFAULT 0,
  `day` varchar(10) NOT NULL,
  week_nb tinyint(2) unsigned NOT NULL,
  month_nb tinyint(2) unsigned NOT NULL,
  quarter_nb tinyint(1) unsigned NOT NULL,
  year_nb smallint(4) NOT NULL,
  year_and_week varchar(7) NOT NULL,
  year_and_month varchar(7) NOT NULL,
  year_and_quarter varchar(7) NOT NULL,
  PRIMARY KEY (splitlot_id),
  KEY ft_splitlot (lot_id),
  KEY ftsplitlot_startt (start_t)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_sya_set
--

DROP TABLE IF EXISTS ft_sya_set;
CREATE TABLE ft_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  product_id varchar(255) NOT NULL,
  creation_date datetime NOT NULL,
  user_comment varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  start_date datetime NOT NULL,
  expiration_date date NOT NULL,
  expiration_email_date datetime DEFAULT NULL,
  rule_type varchar(255) NOT NULL,
  n1_parameter float NOT NULL DEFAULT -1,
  n2_parameter float NOT NULL DEFAULT -1,
  computation_fromdate date NOT NULL,
  computation_todate date NOT NULL,
  min_lots_required smallint(5) NOT NULL DEFAULT -1,
  min_data_points smallint(5) NOT NULL DEFAULT -1,
  `options` tinyint(3) unsigned NOT NULL DEFAULT 0,
  flags tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (sya_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_wyr
--

DROP TABLE IF EXISTS ft_wyr;
CREATE TABLE ft_wyr (
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  `year` smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT 0,
  parts_received int(10) DEFAULT 0,
  pretest_rejects int(10) DEFAULT 0,
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT 0,
  parts_pass int(10) DEFAULT 0,
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT 0,
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT 0,
  PARTS_RETEST_SPLIT varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT 0,
  posttest_rejects int(10) DEFAULT 0,
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table ft_wyr_format
--

DROP TABLE IF EXISTS ft_wyr_format;
CREATE TABLE ft_wyr_format (
  site_name varchar(255) NOT NULL,
  column_id tinyint(3) NOT NULL,
  column_nb tinyint(3) NOT NULL,
  column_name varchar(255) NOT NULL,
  data_type varchar(255) NOT NULL,
  display char(1) NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_dtr
--

DROP TABLE IF EXISTS wt_dtr;
CREATE TABLE wt_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT NULL,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  KEY wt_dtr (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ftest_info
--

DROP TABLE IF EXISTS wt_ftest_info;
CREATE TABLE wt_ftest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  testseq smallint(5) unsigned DEFAULT NULL,
  KEY wtftest_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ftest_results
--

DROP TABLE IF EXISTS wt_ftest_results;
CREATE TABLE wt_ftest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(7) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  vect_nam varchar(255) NOT NULL DEFAULT '',
  vect_off smallint(6) DEFAULT NULL,
  KEY wt_ftest_results (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ftest_stats_samples
--

DROP TABLE IF EXISTS wt_ftest_stats_samples;
CREATE TABLE wt_ftest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  KEY wtftestsamp_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ftest_stats_summary
--

DROP TABLE IF EXISTS wt_ftest_stats_summary;
CREATE TABLE wt_ftest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ftest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned DEFAULT NULL,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY wtftestsum_splitlot_ftest (splitlot_id,ftest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_hbin
--

DROP TABLE IF EXISTS wt_hbin;
CREATE TABLE wt_hbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  KEY wt_hbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_hbin_stats_samples
--

DROP TABLE IF EXISTS wt_hbin_stats_samples;
CREATE TABLE wt_hbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  KEY wt_hbin_stats_samples (splitlot_id,site_no,hbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_hbin_stats_summary
--

DROP TABLE IF EXISTS wt_hbin_stats_summary;
CREATE TABLE wt_hbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wt_hbin_stats_summary (splitlot_id,site_no,hbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_lot
--

DROP TABLE IF EXISTS wt_lot;
CREATE TABLE wt_lot (
  lot_id varchar(255) NOT NULL DEFAULT '',
  tracking_lot_id varchar(255) DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  flags binary(2) DEFAULT NULL,
  KEY wt_lot (lot_id),
  KEY wtlot_productname_lot (product_name,lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_lot_hbin
--

DROP TABLE IF EXISTS wt_lot_hbin;
CREATE TABLE wt_lot_hbin (
  lot_id varchar(255) NOT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wtlothbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_lot_sbin
--

DROP TABLE IF EXISTS wt_lot_sbin;
CREATE TABLE wt_lot_sbin (
  lot_id varchar(255) NOT NULL,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wtlotsbin_lot (lot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_metadata_link
--

DROP TABLE IF EXISTS wt_metadata_link;
CREATE TABLE wt_metadata_link (
  link_name varchar(255) NOT NULL,
  gexdb_table1_name varchar(255) NOT NULL,
  gexdb_field1_fullname varchar(255) NOT NULL,
  gexdb_table2_name varchar(255) NOT NULL,
  gexdb_field2_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_metadata_mapping
--

DROP TABLE IF EXISTS wt_metadata_mapping;
CREATE TABLE wt_metadata_mapping (
  meta_name varchar(255) NOT NULL,
  gex_name varchar(255) DEFAULT NULL,
  gexdb_table_name varchar(255) NOT NULL,
  gexdb_field_fullname varchar(255) NOT NULL,
  gexdb_link_name varchar(255) DEFAULT NULL,
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  bintype_field char(1) NOT NULL DEFAULT 'N',
  time_field char(1) NOT NULL DEFAULT 'N',
  custom_field char(1) NOT NULL DEFAULT 'Y',
  numeric_field char(1) NOT NULL DEFAULT 'N',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_mptest_info
--

DROP TABLE IF EXISTS wt_mptest_info;
CREATE TABLE wt_mptest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  tpin_arrayindex smallint(6) NOT NULL DEFAULT 0,
  units varchar(255) NOT NULL DEFAULT '',
  flags binary(1) NOT NULL DEFAULT '\0',
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  testseq smallint(5) unsigned DEFAULT NULL,
  spec_ll float DEFAULT NULL,
  spec_hl float DEFAULT NULL,
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  KEY wt_mptest_info (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_mptest_limits
--

DROP TABLE IF EXISTS wt_mptest_limits;
CREATE TABLE wt_mptest_limits (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  LL float DEFAULT NULL,
  HL float DEFAULT NULL,
  KEY wtmptestlimits_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_mptest_results
--

DROP TABLE IF EXISTS wt_mptest_results;
CREATE TABLE wt_mptest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(7) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  `value` float NOT NULL DEFAULT 0,
  tpin_pmrindex smallint(6) DEFAULT NULL,
  KEY wt_mptest_results (splitlot_id,mptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_mptest_stats_samples
--

DROP TABLE IF EXISTS wt_mptest_stats_samples;
CREATE TABLE wt_mptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  KEY wtmptestsamp_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_mptest_stats_summary
--

DROP TABLE IF EXISTS wt_mptest_stats_summary;
CREATE TABLE wt_mptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  mptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY wtmptestsum_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_parts_stats_samples
--

DROP TABLE IF EXISTS wt_parts_stats_samples;
CREATE TABLE wt_parts_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  KEY wt_parts_stats_samples (splitlot_id,site_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_parts_stats_summary
--

DROP TABLE IF EXISTS wt_parts_stats_summary;
CREATE TABLE wt_parts_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_good mediumint(8) DEFAULT NULL,
  nb_rtst mediumint(8) DEFAULT NULL,
  KEY wt_parts_stats_summary (splitlot_id,site_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_prod_alarm
--

DROP TABLE IF EXISTS wt_prod_alarm;
CREATE TABLE wt_prod_alarm (
  splitlot_id int(10) unsigned NOT NULL,
  alarm_cat varchar(255) NOT NULL,
  alarm_type varchar(255) NOT NULL,
  item_no int(10) unsigned NOT NULL,
  item_name varchar(255) DEFAULT NULL,
  flags binary(2) NOT NULL,
  lcl float NOT NULL DEFAULT 0,
  ucl float NOT NULL DEFAULT 0,
  `value` float NOT NULL DEFAULT 0,
  units varchar(10) DEFAULT NULL,
  KEY wtprodalarm_splitlot (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_product_hbin
--

DROP TABLE IF EXISTS wt_product_hbin;
CREATE TABLE wt_product_hbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY wt_product_hbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_product_sbin
--

DROP TABLE IF EXISTS wt_product_sbin;
CREATE TABLE wt_product_sbin (
  product_name varchar(255) NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) NOT NULL DEFAULT '',
  bin_cat char(1) DEFAULT NULL,
  nb_parts bigint(20) NOT NULL DEFAULT 0,
  KEY wt_product_sbin_bin (product_name,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ptest_info
--

DROP TABLE IF EXISTS wt_ptest_info;
CREATE TABLE wt_ptest_info (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  tnum int(10) unsigned NOT NULL DEFAULT 0,
  tname varchar(255) NOT NULL DEFAULT '',
  units varchar(255) NOT NULL DEFAULT '',
  flags binary(1) NOT NULL DEFAULT '\0',
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  testseq smallint(5) unsigned DEFAULT NULL,
  spec_ll float DEFAULT NULL,
  spec_hl float DEFAULT NULL,
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  KEY wtptest_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ptest_limits
--

DROP TABLE IF EXISTS wt_ptest_limits;
CREATE TABLE wt_ptest_limits (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  ll float DEFAULT NULL,
  hl float DEFAULT NULL,
  KEY wtptestlimits_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ptest_results
--

DROP TABLE IF EXISTS wt_ptest_results;
CREATE TABLE wt_ptest_results (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(7) unsigned NOT NULL DEFAULT 0,
  flags binary(1) NOT NULL DEFAULT '\0',
  `value` float DEFAULT NULL,
  KEY wt_ptest_results (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ptest_stats_samples
--

DROP TABLE IF EXISTS wt_ptest_stats_samples;
CREATE TABLE wt_ptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  KEY wtptestsamp_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_ptest_stats_summary
--

DROP TABLE IF EXISTS wt_ptest_stats_summary;
CREATE TABLE wt_ptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  ptest_info_id smallint(5) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  exec_count mediumint(8) unsigned DEFAULT NULL,
  fail_count mediumint(8) unsigned DEFAULT NULL,
  min_value float DEFAULT NULL,
  max_value float DEFAULT NULL,
  sum float DEFAULT NULL,
  square_sum float DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  KEY wtptestsum_splitlot_ptest (splitlot_id,ptest_info_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_run
--

DROP TABLE IF EXISTS wt_run;
CREATE TABLE wt_run (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  run_id mediumint(7) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  part_id varchar(255) DEFAULT NULL,
  part_x smallint(6) DEFAULT NULL,
  part_y smallint(6) DEFAULT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned DEFAULT NULL,
  ttime int(10) unsigned DEFAULT NULL,
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0,
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0,
  firstfail_tnum int(10) unsigned DEFAULT NULL,
  firstfail_tname varchar(255) DEFAULT NULL,
  retest_index tinyint(3) unsigned NOT NULL DEFAULT 0,
  part_txt varchar(255) DEFAULT NULL,
  KEY wt_run (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_sbin
--

DROP TABLE IF EXISTS wt_sbin;
CREATE TABLE wt_sbin (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  KEY wt_sbin (splitlot_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_sbin_stats_samples
--

DROP TABLE IF EXISTS wt_sbin_stats_samples;
CREATE TABLE wt_sbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  KEY wt_sbin_stats_samples (splitlot_id,site_no,sbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_sbin_stats_summary
--

DROP TABLE IF EXISTS wt_sbin_stats_summary;
CREATE TABLE wt_sbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL DEFAULT 0,
  site_no smallint(5) NOT NULL DEFAULT 1,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  bin_count mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wt_sbin_stats_summary (splitlot_id,site_no,sbin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_sbl
--

DROP TABLE IF EXISTS wt_sbl;
CREATE TABLE wt_sbl (
  sya_id int(10) unsigned NOT NULL,
  bin_no smallint(5) unsigned NOT NULL,
  bin_name varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_splitlot
--

DROP TABLE IF EXISTS wt_splitlot;
CREATE TABLE wt_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  setup_t int(10) NOT NULL DEFAULT 0,
  start_t int(10) NOT NULL DEFAULT 0,
  finish_t int(10) NOT NULL DEFAULT 0,
  stat_num tinyint(3) unsigned NOT NULL DEFAULT 0,
  tester_name varchar(255) NOT NULL DEFAULT '',
  tester_type varchar(255) NOT NULL DEFAULT '',
  flags binary(2) NOT NULL DEFAULT '\0\0',
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_samples mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_summary mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT 0,
  data_provider varchar(255) DEFAULT '',
  data_type varchar(255) DEFAULT '',
  prod_data char(1) NOT NULL DEFAULT 'Y',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT 0,
  retest_hbins varchar(255) DEFAULT NULL,
  rework_code tinyint(3) unsigned NOT NULL DEFAULT 0,
  job_nam varchar(255) NOT NULL DEFAULT '',
  job_rev varchar(255) NOT NULL DEFAULT '',
  oper_nam varchar(255) NOT NULL DEFAULT '',
  exec_typ varchar(255) NOT NULL DEFAULT '',
  exec_ver varchar(255) NOT NULL DEFAULT '',
  test_cod varchar(255) NOT NULL DEFAULT '',
  facil_id varchar(255) NOT NULL DEFAULT '',
  tst_temp varchar(255) NOT NULL DEFAULT '',
  mode_cod char(1) DEFAULT NULL,
  rtst_cod char(1) DEFAULT NULL,
  prot_cod char(1) DEFAULT NULL,
  burn_tim int(10) DEFAULT NULL,
  cmod_cod char(1) DEFAULT NULL,
  part_typ varchar(255) DEFAULT NULL,
  user_txt varchar(255) DEFAULT NULL,
  aux_file varchar(255) DEFAULT NULL,
  pkg_typ varchar(255) DEFAULT NULL,
  famly_id varchar(255) DEFAULT NULL,
  date_cod varchar(255) DEFAULT NULL,
  floor_id varchar(255) DEFAULT NULL,
  proc_id varchar(255) DEFAULT NULL,
  oper_frq varchar(255) DEFAULT NULL,
  spec_nam varchar(255) DEFAULT NULL,
  spec_ver varchar(255) DEFAULT NULL,
  flow_id varchar(255) DEFAULT NULL,
  setup_id varchar(255) DEFAULT NULL,
  dsgn_rev varchar(255) DEFAULT NULL,
  eng_id varchar(255) DEFAULT NULL,
  rom_cod varchar(255) DEFAULT NULL,
  serl_num varchar(255) DEFAULT NULL,
  supr_nam varchar(255) DEFAULT NULL,
  nb_sites tinyint(3) unsigned NOT NULL DEFAULT 1,
  head_num tinyint(3) unsigned DEFAULT NULL,
  handler_typ varchar(255) DEFAULT NULL,
  handler_id varchar(255) DEFAULT NULL,
  card_typ varchar(255) DEFAULT NULL,
  card_id varchar(255) DEFAULT NULL,
  loadboard_typ varchar(255) DEFAULT NULL,
  loadboard_id varchar(255) DEFAULT NULL,
  dib_typ varchar(255) DEFAULT NULL,
  dib_id varchar(255) DEFAULT NULL,
  cable_typ varchar(255) DEFAULT NULL,
  cable_id varchar(255) DEFAULT NULL,
  contactor_typ varchar(255) DEFAULT NULL,
  contactor_id varchar(255) DEFAULT NULL,
  laser_typ varchar(255) DEFAULT NULL,
  laser_id varchar(255) DEFAULT NULL,
  extra_typ varchar(255) DEFAULT NULL,
  extra_id varchar(255) DEFAULT NULL,
  file_host_id int(10) unsigned DEFAULT 0,
  file_path varchar(255) NOT NULL DEFAULT '',
  file_name varchar(255) NOT NULL DEFAULT '',
  valid_splitlot char(1) NOT NULL DEFAULT 'N',
  insertion_time int(10) unsigned NOT NULL DEFAULT 0,
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) DEFAULT NULL,
  incremental_update varchar(255) DEFAULT NULL,
  sya_id int(10) unsigned DEFAULT 0,
  `day` varchar(10) NOT NULL,
  week_nb tinyint(2) unsigned NOT NULL,
  month_nb tinyint(2) unsigned NOT NULL,
  quarter_nb tinyint(1) unsigned NOT NULL,
  year_nb smallint(4) NOT NULL,
  year_and_week varchar(7) NOT NULL,
  year_and_month varchar(7) NOT NULL,
  year_and_quarter varchar(7) NOT NULL,
  PRIMARY KEY (splitlot_id),
  KEY wt_splitlot (lot_id),
  KEY wtsplitlot_startt (start_t)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_sya_set
--

DROP TABLE IF EXISTS wt_sya_set;
CREATE TABLE wt_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  product_id varchar(255) NOT NULL,
  creation_date datetime NOT NULL,
  user_comment varchar(255) DEFAULT NULL,
  ll_1 float NOT NULL DEFAULT -1,
  hl_1 float NOT NULL DEFAULT -1,
  ll_2 float NOT NULL DEFAULT -1,
  hl_2 float NOT NULL DEFAULT -1,
  start_date datetime NOT NULL,
  expiration_date date NOT NULL,
  expiration_email_date datetime DEFAULT NULL,
  rule_type varchar(255) NOT NULL,
  n1_parameter float NOT NULL DEFAULT -1,
  n2_parameter float NOT NULL DEFAULT -1,
  computation_fromdate date NOT NULL,
  computation_todate date NOT NULL,
  min_lots_required smallint(5) NOT NULL DEFAULT -1,
  min_data_points smallint(5) NOT NULL DEFAULT -1,
  `options` tinyint(3) unsigned NOT NULL DEFAULT 0,
  flags tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (sya_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_wafer_hbin
--

DROP TABLE IF EXISTS wt_wafer_hbin;
CREATE TABLE wt_wafer_hbin (
  lot_id varchar(255) NOT NULL,
  wafer_id varchar(255) DEFAULT NULL,
  hbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  hbin_name varchar(255) NOT NULL DEFAULT '',
  hbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wt_wafer_hbin (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_wafer_info
--

DROP TABLE IF EXISTS wt_wafer_info;
CREATE TABLE wt_wafer_info (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) DEFAULT NULL,
  fab_id varchar(255) DEFAULT NULL,
  frame_id varchar(255) DEFAULT NULL,
  mask_id varchar(255) DEFAULT NULL,
  wafer_size float DEFAULT NULL,
  die_ht float DEFAULT NULL,
  die_wid float DEFAULT NULL,
  wafer_units tinyint(3) unsigned DEFAULT NULL,
  wafer_flat char(1) DEFAULT NULL,
  center_x smallint(5) unsigned DEFAULT NULL,
  center_y smallint(5) unsigned DEFAULT NULL,
  pos_x char(1) DEFAULT NULL,
  pos_y char(1) DEFAULT NULL,
  gross_die mediumint(8) NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT 0,
  nb_parts_good mediumint(8) NOT NULL DEFAULT 0,
  flags binary(2) DEFAULT NULL,
  KEY wt_wafer_info (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_wafer_sbin
--

DROP TABLE IF EXISTS wt_wafer_sbin;
CREATE TABLE wt_wafer_sbin (
  lot_id varchar(255) NOT NULL,
  wafer_id varchar(255) DEFAULT NULL,
  sbin_no smallint(5) unsigned NOT NULL DEFAULT 0,
  sbin_name varchar(255) NOT NULL DEFAULT '',
  sbin_cat char(1) DEFAULT NULL,
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT 0,
  KEY wt_wafer_sbin (lot_id,wafer_id)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_wyr
--

DROP TABLE IF EXISTS wt_wyr;
CREATE TABLE wt_wyr (
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  `year` smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT 0,
  parts_received int(10) DEFAULT 0,
  pretest_rejects int(10) DEFAULT 0,
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT 0,
  parts_pass int(10) DEFAULT 0,
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT 0,
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT 0,
  parts_retest_split varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT 0,
  posttest_rejects int(10) DEFAULT 0,
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table wt_wyr_format
--

DROP TABLE IF EXISTS wt_wyr_format;
CREATE TABLE wt_wyr_format (
  site_name varchar(255) NOT NULL,
  column_id tinyint(3) NOT NULL,
  column_nb tinyint(3) NOT NULL,
  column_name varchar(255) NOT NULL,
  data_type varchar(255) NOT NULL,
  display char(1) NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Procedure et_filearchive_settings
--

DELIMITER $$

DROP PROCEDURE IF EXISTS et_filearchive_settings$$
CREATE PROCEDURE  et_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT UseArchiveSettings INT, -- Return 1 if the Archivesettings should be used, 0 else
OUT MovePath VARCHAR(1024), -- Return the path to use if the file should be moved after insertion (DataPump settings)
OUT FtpPort INT, -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpServer VARCHAR(1024), -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpUser VARCHAR(1024), -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPassword VARCHAR(1024), -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPath VARCHAR(1024))
BEGIN
SELECT '' INTO MovePath From dual;
SELECT '' INTO FtpServer From dual;
SELECT 21 INTO FtpPort FROM dual;
SELECT '' INTO FtpUser From dual;
SELECT '' INTO FtpPassword From dual;
SELECT '' INTO FtpPath From dual;
SELECT 0 INTO UseArchiveSettings FROM dual;
END $$

DELIMITER ;

--
-- Procedure et_insertion_status
--
DELIMITER $$

DROP PROCEDURE IF EXISTS et_insertion_status$$
CREATE PROCEDURE  et_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT)
BEGIN
END $$

DELIMITER ;
--
-- Procedure et_insertion_validation
--
DELIMITER $$

DROP PROCEDURE IF EXISTS et_insertion_validation$$
CREATE PROCEDURE  et_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$

DELIMITER ;

--
-- Procedure et_insertion_postprocessing
--

DELIMITER $$

DROP PROCEDURE IF EXISTS et_insertion_postprocessing$$
CREATE PROCEDURE  et_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$

DELIMITER ;
--
-- Procedure ft_filearchive_settings
--

DELIMITER $$

DROP PROCEDURE IF EXISTS ft_filearchive_settings$$
CREATE PROCEDURE  ft_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
OUT UseArchiveSettings INT, -- Return 1 if the Archivesettings should be used, 0 else
OUT MovePath VARCHAR(1024), -- Return the path to use if the file should be moved after insertion (DataPump settings)
OUT FtpPort INT, -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpServer VARCHAR(1024), -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpUser VARCHAR(1024), -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPassword VARCHAR(1024), -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPath VARCHAR(1024))
BEGIN
SELECT '' INTO MovePath From dual;
SELECT '' INTO FtpServer From dual;
SELECT 21 INTO FtpPort FROM dual;
SELECT '' INTO FtpUser From dual;
SELECT '' INTO FtpPassword From dual;
SELECT '' INTO FtpPath From dual;
SELECT 0 INTO UseArchiveSettings FROM dual;
END $$

DELIMITER ;
--
-- Procedure ft_insertion_status
--

DELIMITER $$

DROP PROCEDURE IF EXISTS ft_insertion_status$$
CREATE PROCEDURE ft_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT)
BEGIN
END $$

DELIMITER ;

--
-- Procedure ft_insertion_validation
--

DELIMITER $$

DROP PROCEDURE IF EXISTS ft_insertion_validation$$
CREATE PROCEDURE  ft_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$


--
-- Procedure ft_insertion_postprocessing
--

DELIMITER $$

DROP PROCEDURE IF EXISTS ft_insertion_postprocessing$$
CREATE PROCEDURE  ft_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$

DELIMITER ;
--
-- Procedure purge_invalid_splitlots
--
DELIMITER $$

DROP PROCEDURE IF EXISTS purge_invalid_splitlots$$
CREATE PROCEDURE  purge_invalid_splitlots()
BEGIN
CALL purge_splitlots(null, null, null);
END $$

DELIMITER ;
--
-- Procedure purge_splitlots
--

DELIMITER $$

DROP PROCEDURE IF EXISTS purge_splitlots$$
CREATE PROCEDURE  purge_splitlots(
SamplesNbWeeks INT, # Nb of weeks before delete samples
StatsNbWeeks INT, # Nb of weeks before delete stats
EntriesNbWeeks INT # Nb of weeks before delete entries
)
BEGIN
DECLARE DateEraseEntries, DateEraseStats, DateEraseSamples INT;
DECLARE DateSetup BIGINT;
DECLARE HaveSplitlotIdForErase, SplitlotForErase INT;
SELECT (SamplesNbWeeks*7) * (24*60*60) INTO DateEraseSamples;
SELECT (StatsNbWeeks *7) * (24*60*60) INTO DateEraseStats;
SELECT (EntriesNbWeeks*7) * (24*60*60) INTO DateEraseEntries;
# Total purge
# verify if have TotalPurge information else skip this step
# FOR FT_TABLES
# Verify if have Purge to do
IF (EntriesNbWeeks IS NULL) THEN
# Only erase Invalid Splitlot older than 3mn
SELECT 0 INTO DateSetup;
ELSE
SELECT UNIX_TIMESTAMP() - DateEraseEntries INTO DateSetup;
END IF;
SELECT max(splitlot_id) FROM ft_splitlot
WHERE (DateSetup > INSERTION_TIME) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_parts_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_parts_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_prod_alarm WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_splitlot WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR WT_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM wt_splitlot
WHERE (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_parts_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_parts_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_prod_alarm WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_splitlot WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR ET_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM et_splitlot
WHERE (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_ptest_stats WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_prod_alarm WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_splitlot WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# stats purge
# verify if have stats information else skip this step
IF NOT (StatsNbWeeks IS NULL)
THEN
# FOR FT_TABLES
# Verify if have Purge to do
SELECT UNIX_TIMESTAMP() - DateEraseStats INTO DateSetup;
SELECT max(splitlot_id) FROM ft_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR WT_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM wt_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR E_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM et_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_ptest_stats WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
END IF;
# SamplesNbWeeks purge
# verify if have Samplesge information else skip this step
IF NOT (SamplesNbWeeks IS NULL)
THEN
# FOR FT_TABLES
# Verify if have Purge to do
SELECT UNIX_TIMESTAMP() - DateEraseSamples INTO DateSetup;
SELECT max(splitlot_id) FROM ft_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR WT_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM wt_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
# FOR E_TABLES
# Verify if have Purge to do
SELECT max(splitlot_id) FROM et_splitlot
WHERE DateSetup > insertion_time
INTO HaveSplitlotIdForErase;
IF NOT (HaveSplitlotIdForErase IS NULL)
THEN BEGIN
DECLARE not_found BOOLEAN DEFAULT false;
DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE (DateSetup > insertion_time);
DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
OPEN curSplitlot;
SET not_found = false;
REPEAT
FETCH curSplitlot INTO SplitlotForErase;
DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
UNTIL not_found = true END REPEAT;
CLOSE curSplitlot;
END; END IF;
END IF;
END $$

DELIMITER ;

--
-- Procedure wt_filearchive_settings
--
DELIMITER $$

DROP PROCEDURE IF EXISTS wt_filearchive_settings$$
CREATE PROCEDURE wt_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT UseArchiveSettings INT, -- Return 1 if the Archivesettings should be used, 0 else
OUT MovePath VARCHAR(1024), -- Return the path to use if the file should be moved after insertion (DataPump settings)
OUT FtpPort INT, -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpServer VARCHAR(1024), -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpUser VARCHAR(1024), -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPassword VARCHAR(1024), -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
OUT FtpPath VARCHAR(1024))
BEGIN
SELECT '' INTO MovePath From dual;
SELECT '' INTO FtpServer From dual;
SELECT 21 INTO FtpPort FROM dual;
SELECT '' INTO FtpUser From dual;
SELECT '' INTO FtpPassword From dual;
SELECT '' INTO FtpPath From dual;
SELECT 0 INTO UseArchiveSettings FROM dual;
END $$

DELIMITER ;
--
-- Procedure wt_insertion_status
--
DELIMITER $$

DROP PROCEDURE IF EXISTS wt_insertion_status$$
CREATE PROCEDURE  wt_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT)
BEGIN
END $$

DELIMITER ;

--
-- Procedure wt_insertion_validation
--
DELIMITER $$

DROP PROCEDURE IF EXISTS wt_insertion_validation$$
CREATE PROCEDURE  wt_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$

DELIMITER ;

--
-- Procedure wt_insertion_postprocessing
--
DELIMITER $$

DROP PROCEDURE IF EXISTS wt_insertion_postprocessing$$
CREATE PROCEDURE  wt_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END $$

DELIMITER ;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
