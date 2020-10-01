--
-- Create schema gexdb for sqlite. Version B9.
--

-- PRAGMA encoding="UTF-8"; -- choose between UTF-8, UTF-16, UTF-16le, UTF-16be

--
-- GLOBAL
--
DROP TABLE IF EXISTS global_info;
CREATE TABLE global_info (
  db_version_name						varchar(255)	NOT NULL,
  db_version_nb							smallint(5)		NOT NULL,
  db_version_build						smallint(5)		NOT NULL,
  incremental_splitlots					int(9)			NOT NULL DEFAULT 0
);

INSERT INTO global_info VALUES('GEXDB V1.0 B9 (SQLite)', 10, 9, 0);

DROP TABLE IF EXISTS incremental_update;
CREATE TABLE incremental_update (
  db_update_name		varchar(255)	NOT NULL,
  initial_splitlots		int(9)			NOT NULL DEFAULT 0,
  remaining_splitlots	int(9)			NOT NULL DEFAULT 0,
  db_version_build		smallint(5)		NOT NULL
);

DROP TABLE IF EXISTS product;
CREATE TABLE product (
  product_name 		varchar(255)	NOT NULL DEFAULT '',
  description 		varchar(1000)	DEFAULT NULL
);

/*!40000 ALTER TABLE product DISABLE KEYS */;
/*!40000 ALTER TABLE product ENABLE KEYS */;

DROP TABLE IF EXISTS file_host;
CREATE TABLE file_host (
  file_host_id 		INTEGER	PRIMARY KEY	AUTOINCREMENT	NOT NULL,	-- With MySQL :  FILE_HOST_ID 	int(10) unsigned	NOT NULL auto_increment,
  host_name 		varchar(255)			NOT NULL DEFAULT '',
  host_ftpuser 		varchar(255)			NOT NULL DEFAULT '',
  host_ftppassword 	varchar(255)			NOT NULL DEFAULT '',
  host_ftppath 		varchar(255)			DEFAULT NULL,
  host_ftpport 		smallint(5)				NOT NULL DEFAULT 21
  -- PRIMARY KEY  (FILE_HOST_ID)
); 

/*!40000 ALTER TABLE file_host DISABLE KEYS */;
/*!40000 ALTER TABLE file_host ENABLE KEYS */;

--
-- ELECTRICAL TEST
--
DROP TABLE IF EXISTS et_sya_set;
CREATE TABLE et_sya_set (
 sya_id					INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL , -- SYA_ID	int(10) unsigned	NOT NULL auto_increment,
 product_id				varchar(255)		NOT NULL,
 creation_date			datetime			NOT NULL,
 user_comment			varchar(255)		DEFAULT NULL,
 ll_1					float				NOT NULL DEFAULT -1,
 hl_1					float				NOT NULL DEFAULT -1,
 ll_2					float				NOT NULL DEFAULT -1,
 hl_2					float				NOT NULL DEFAULT -1,
 start_date				datetime			NOT NULL,
 expiration_date			date				NOT NULL,
 expiration_email_date	datetime			DEFAULT NULL,
 rule_type				varchar(255)		NOT NULL,
 n1_parameter			float				NOT NULL DEFAULT -1,
 n2_parameter			float				NOT NULL DEFAULT -1,
 computation_fromdate	date				NOT NULL,
 computation_todate		date				NOT NULL,
 min_lots_required		smallint(5)			NOT NULL DEFAULT -1,
 min_data_points			smallint(5)			NOT NULL DEFAULT -1,
 options					tinyint(3)			NOT NULL DEFAULT 0,
 flags					tinyint(3)			NOT NULL DEFAULT 0
 -- PRIMARY KEY  (SYA_ID)
) ;

DROP TABLE IF EXISTS et_sbl;
CREATE TABLE et_sbl (
 sya_id					INTEGER		NOT NULL, 		-- SYA_ID	int(10) unsigned	NOT NULL,
 bin_no					smallint(5)		NOT NULL,
 bin_name				varchar(255)			DEFAULT NULL,
 ll_1					float					NOT NULL DEFAULT -1,
 hl_1					float					NOT NULL DEFAULT -1,
 ll_2					float					NOT NULL DEFAULT -1,
 hl_2					float					NOT NULL DEFAULT -1,
 PRIMARY KEY  (sya_id, bin_no),
 FOREIGN KEY  (sya_id) REFERENCES et_sya_set(sya_id)
); 

DROP TABLE IF EXISTS et_prod_alarm;
CREATE TABLE et_prod_alarm (
  splitlot_id			int(10)				NOT NULL, -- was unsigned
  alarm_cat				varchar(255)		NOT NULL,
  alarm_type			varchar(255)		NOT NULL,
  item_no				int unsigned		NOT NULL,
  item_name				varchar(255)		DEFAULT NULL,
  flags					binary(2)			NOT NULL,
  lcl					float				NOT NULL DEFAULT 0,
  ucl					float				NOT NULL DEFAULT 0,
  value					float				NOT NULL DEFAULT 0,
  units					varchar(10)			DEFAULT NULL
); 

DROP TABLE IF EXISTS et_wyr;
CREATE TABLE et_wyr (
	site_name				varchar(255)	NOT NULL,
	week_nb					tinyint(3)		DEFAULT NULL,
	year					smallint(5)		DEFAULT NULL,
	date_in					datetime		DEFAULT NULL,
	date_out				datetime		DEFAULT NULL,
	product_name			varchar(255)	DEFAULT NULL,
	program_name			varchar(255)	DEFAULT NULL,
	tester_name				varchar(255)	DEFAULT NULL,
	lot_id					varchar(255)	DEFAULT NULL,
	subcon_lot_id			varchar(255)	DEFAULT NULL,
	user_split				varchar(1024)	DEFAULT NULL,
	yield					float			DEFAULT 0,
	parts_received			int(10)			DEFAULT 0,
	pretest_rejects			int(10)			DEFAULT 0,
	pretest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_tested			int(10)			DEFAULT 0,
	parts_pass				int(10)			DEFAULT 0,
	parts_pass_split		varchar(1024)	DEFAULT NULL,
	parts_fail				int(10)			DEFAULT 0,
	parts_fail_split		varchar(1024)	DEFAULT NULL,
	parts_retest			int(10)			DEFAULT 0,
	parts_retest_split		varchar(1024)	DEFAULT NULL,
	insertions				int(10)			DEFAULT 0,
	posttest_rejects		int(10)			DEFAULT 0,
	posttest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_shipped			int(10)			DEFAULT 0
);

DROP TABLE IF EXISTS et_wyr_format;
CREATE TABLE et_wyr_format (
	site_name				varchar(255)	NOT NULL,
	column_id				tinyint(3)		NOT NULL,
	column_nb				tinyint(3)		NOT NULL,
	column_name				varchar(255)	NOT NULL,
	data_type				varchar(255)	NOT NULL,
	display					char(1)			DEFAULT 'Y' NOT NULL
);

DROP TABLE IF EXISTS et_metadata_mapping;
CREATE TABLE et_metadata_mapping (
  meta_name					varchar(255)	NOT NULL,
  gex_name					varchar(255)	DEFAULT NULL,
  gexdb_table_name			varchar(255)	NOT NULL,
  gexdb_field_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL,
  gex_display_in_gui		char(1)			NOT NULL DEFAULT 'Y'
); 

/*!40000 ALTER TABLE et_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE et_metadata_mapping ENABLE KEYS */;

DROP TABLE IF EXISTS et_metadata_link;
CREATE TABLE et_metadata_link (
  link_name					varchar(255)	NOT NULL,
  gexdb_table1_name			varchar(255)	NOT NULL,
  gexdb_field1_fullname		varchar(255)	NOT NULL,
  gexdb_table2_name			varchar(255)	NOT NULL,
  gexdb_field2_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL
);

/*!40000 ALTER TABLE et_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE et_metadata_link ENABLE KEYS */;

DROP TABLE IF EXISTS et_lot;
CREATE TABLE et_lot (
  lot_id 			varchar(255)		NOT NULL DEFAULT '',
  tracking_lot_id	varchar(255)		DEFAULT NULL,
  product_name 		varchar(255)		DEFAULT NULL,
  nb_parts 			int(10)				NOT NULL DEFAULT '0',
  nb_parts_good 	int(10)				NOT NULL DEFAULT '0',
  flags 			binary(2)
);

CREATE INDEX et_lot_index ON et_lot (lot_id);

/*!40000 ALTER TABLE et_lot DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot ENABLE KEYS */;

DROP TABLE IF EXISTS et_lot_hbin;
CREATE TABLE et_lot_hbin (
  lot_id			varchar(255)			NOT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0',  -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0'  -- unsigned
);

/*!40000 ALTER TABLE et_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS et_lot_sbin;
CREATE TABLE et_lot_sbin (
  lot_id			varchar(255)			NOT NULL,
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0'  -- unsigned
);

/*!40000 ALTER TABLE et_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot_sbin ENABLE KEYS */;

DROP TABLE IF EXISTS et_splitlot;
CREATE TABLE et_splitlot (
  splitlot_id 		INTEGER PRIMARY KEY	AUTOINCREMENT NOT NULL, -- int(10) unsigned  auto_increment
  lot_id 			varchar(255)			NOT NULL DEFAULT '',
  sublot_id 		varchar(255)			NOT NULL DEFAULT '',
  start_t 			int(10)					NOT NULL DEFAULT '0',
  finish_t 			int(10)					NOT NULL DEFAULT '0',
  week_nb			tinyint(3)				NOT NULL DEFAULT '0',
  year				smallint(5)				NOT NULL DEFAULT '0',
  tester_name 		varchar(255)			NOT NULL DEFAULT '',
  tester_type 		varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(2)				NOT NULL DEFAULT '\0\0',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0',
  data_provider 	varchar(255)			DEFAULT '',
  data_type 		varchar(255)			DEFAULT '',
  prod_data			char(1)					NOT NULL DEFAULT 'Y',
  job_nam 			varchar(255)			NOT NULL DEFAULT '',
  job_rev 			varchar(255)			NOT NULL DEFAULT '',
  oper_nam 			varchar(255)			NOT NULL DEFAULT '',
  exec_typ 			varchar(255)			NOT NULL DEFAULT '',
  exec_ver 			varchar(255)			NOT NULL DEFAULT '',
  facil_id 			varchar(255)			NOT NULL DEFAULT '',
  part_typ 			varchar(255)			DEFAULT NULL,
  user_txt 			varchar(255)			DEFAULT NULL,
  famly_id 			varchar(255)			DEFAULT NULL,
  proc_id 			varchar(255)			DEFAULT NULL,
  file_host_id 		int(10)					DEFAULT '0', -- unsigned
  file_path 		varchar(255)			NOT NULL DEFAULT '',
  file_name 		varchar(255)			NOT NULL DEFAULT '',
  valid_splitlot 	char(1)					NOT NULL DEFAULT 'N',
  insertion_time 	int(10)					NOT NULL DEFAULT '0', -- unsigned
  subcon_lot_id		varchar(255)			NOT NULL DEFAULT '',
  wafer_id 			varchar(255)			DEFAULT NULL,
  incremental_update	varchar(255)		DEFAULT NULL,
  sya_id			int(10)					DEFAULT '0' -- unsigned
  --  PRIMARY KEY  (SPLITLOT_ID)
);

CREATE INDEX et_splitlot_index ON et_splitlot (lot_id);

/*!40000 ALTER TABLE et_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE et_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS et_wafer_info;
CREATE TABLE et_wafer_info (
  lot_id			varchar(255)			NOT NULL DEFAULT '',
  wafer_id 			varchar(255)			DEFAULT NULL,
  fab_id 			varchar(255)			DEFAULT NULL,
  frame_id 			varchar(255)			DEFAULT NULL,
  mask_id 			varchar(255)			DEFAULT NULL,
  wafer_size 		float					DEFAULT NULL,
  die_ht 			float					DEFAULT NULL,
  die_wid 			float					DEFAULT NULL,
  wafer_units 		tinyint(3)				DEFAULT NULL, -- unsigned
  wafer_flat 		char(1)					DEFAULT NULL,
  center_x 			smallint(5)				DEFAULT NULL, -- unsigned
  center_y 			smallint(5)				DEFAULT NULL, -- unsigned
  pos_x 			char(1)					DEFAULT NULL,
  pos_y 			char(1)					DEFAULT NULL,
  gross_die			mediumint(8)			NOT NULL DEFAULT 0,
  nb_parts			mediumint(8)			NOT NULL DEFAULT 0,
  nb_parts_good		mediumint(8)			NOT NULL DEFAULT 0,
  flags 			binary(2)
);

CREATE INDEX et_wafer_info_index ON et_wafer_info (lot_id, wafer_id);

/*!40000 ALTER TABLE et_wafer_info DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_info ENABLE KEYS */;

DROP TABLE IF EXISTS et_wafer_hbin;
CREATE TABLE et_wafer_hbin (
  lot_id			varchar(255)			NOT NULL,
  wafer_id 			varchar(255)			DEFAULT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0'  -- unsigned
);

CREATE INDEX et_wafer_hbin_index ON et_wafer_hbin (lot_id, wafer_id);


/*!40000 ALTER TABLE et_wafer_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS et_wafer_sbin;
CREATE TABLE et_wafer_sbin (
  lot_id			varchar(255)			NOT NULL,
  wafer_id 			varchar(255)			DEFAULT NULL,
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX et_wafer_sbin_index ON et_wafer_sbin (lot_id, wafer_id);

/*!40000 ALTER TABLE et_wafer_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_hbin;
CREATE TABLE et_hbin (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  bin_count			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX et_hbin_index ON et_hbin (splitlot_id);

/*!40000 ALTER TABLE et_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_sbin;
CREATE TABLE et_sbin (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  bin_count			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX et_sbin_index ON et_sbin (splitlot_id);


/*!40000 ALTER TABLE et_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_run;
CREATE TABLE et_run (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  part_id 			varchar(255)			DEFAULT NULL,
  part_x 			smallint(6)				DEFAULT NULL,
  part_y 			smallint(6)				DEFAULT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				DEFAULT NULL, -- unsigned
  ttime 			int(10)					DEFAULT NULL, -- unsigned
  tests_executed 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tests_failed 		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  firstfail_tnum 	int(10)					DEFAULT NULL, -- unsigned
  firstfail_tname 	varchar(255)			DEFAULT NULL,
  retest_index 		tinyint(3)				NOT NULL DEFAULT '0' -- unsigned
); 

CREATE INDEX et_run_index ON et_run	(splitlot_id);

/*!40000 ALTER TABLE et_run DISABLE KEYS */;
/*!40000 ALTER TABLE et_run ENABLE KEYS */;


DROP TABLE IF EXISTS et_ptest_info;
CREATE TABLE et_ptest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  units 			varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  ll 				float					DEFAULT NULL,
  hl 				float					DEFAULT NULL,
  testseq			smallint(5)				DEFAULT NULL, -- unsigned
  spec_ll			float					DEFAULT NULL,
  spec_hl			float					DEFAULT NULL,
  spec_target		float					DEFAULT NULL
 ); 

CREATE INDEX et_ptest_info_index ON et_ptest_info (splitlot_id);

/*!40000 ALTER TABLE et_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS et_ptest_stats;
CREATE TABLE et_ptest_stats (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  exec_count 		smallint(5)				DEFAULT NULL, -- unsigned
  fail_count 		smallint(5)				DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL
);

/*!40000 ALTER TABLE et_ptest_stats DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_stats ENABLE KEYS */;

DROP TABLE IF EXISTS et_ptest_results;
CREATE TABLE et_ptest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  value 			float					DEFAULT NULL
);

CREATE INDEX et_ptest_results_index ON et_ptest_results	(splitlot_id, ptest_info_id);

/*!40000 ALTER TABLE et_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_results ENABLE KEYS */;


--
-- WAFER SORT
--
DROP TABLE IF EXISTS wt_sya_set;
CREATE TABLE wt_sya_set (
 sya_id					INTEGER	PRIMARY KEY AUTOINCREMENT	NOT NULL, -- auto_increment unsigned int(10)
 product_id				varchar(255)		NOT NULL,
 creation_date			datetime			NOT NULL,
 user_comment			varchar(255)		DEFAULT NULL,
 ll_1					float				NOT NULL DEFAULT -1,
 hl_1					float				NOT NULL DEFAULT -1,
 ll_2					float				NOT NULL DEFAULT -1,
 hl_2					float				NOT NULL DEFAULT -1,
 start_date				datetime			NOT NULL,
 expiration_date		date				NOT NULL,
 expiration_email_date	datetime			DEFAULT NULL,
 rule_type				varchar(255)		NOT NULL,
 n1_parameter			float				NOT NULL DEFAULT -1,
 n2_parameter			float				NOT NULL DEFAULT -1,
 computation_fromdate	date				NOT NULL,
 computation_todate		date				NOT NULL,
 min_lots_required		smallint(5)			NOT NULL DEFAULT -1,
 min_data_points		smallint(5)			NOT NULL DEFAULT -1,
 options				tinyint(3)			NOT NULL DEFAULT 0, -- unsigned
 flags					tinyint(3)			NOT NULL DEFAULT 0 -- unsigned
 -- PRIMARY KEY  (SYA_ID)
); 

DROP TABLE IF EXISTS wt_sbl;
CREATE TABLE wt_sbl (
 sya_id					int(10)		NOT NULL, -- int(10) unsigned
 bin_no					smallint(5)		NOT NULL, -- unsigned
 bin_name				varchar(255)			DEFAULT NULL,
 ll_1					float					NOT NULL DEFAULT -1,
 hl_1					float					NOT NULL DEFAULT -1,
 ll_2					float					NOT NULL DEFAULT -1,
 hl_2					float					NOT NULL DEFAULT -1,
 PRIMARY KEY  (sya_id, bin_no),
 FOREIGN KEY  (sya_id) REFERENCES wt_sya_set(sya_id)
);

DROP TABLE IF EXISTS wt_prod_alarm;
CREATE TABLE wt_prod_alarm (
  splitlot_id			int(10)				NOT NULL, -- unsigned
  alarm_cat				varchar(255)		NOT NULL,
  alarm_type			varchar(255)		NOT NULL,
  item_no				int					NOT NULL, -- unsigned
  item_name				varchar(255)		DEFAULT NULL,
  flags					binary(2)			NOT NULL,
  lcl					float				NOT NULL DEFAULT 0,
  ucl					float				NOT NULL DEFAULT 0,
  value					float				NOT NULL DEFAULT 0,
  units					varchar(10)			DEFAULT NULL
);

DROP TABLE IF EXISTS wt_wyr;
CREATE TABLE wt_wyr (
	site_name				varchar(255)	NOT NULL,
	week_nb					tinyint(3)		DEFAULT NULL,
	year					smallint(5)		DEFAULT NULL,
	date_in					datetime		DEFAULT NULL,
	date_out				datetime		DEFAULT NULL,
	product_name			varchar(255)	DEFAULT NULL,
	program_name			varchar(255)	DEFAULT NULL,
	tester_name				varchar(255)	DEFAULT NULL,
	lot_id					varchar(255)	DEFAULT NULL,
	subcon_lot_id			varchar(255)	DEFAULT NULL,
	user_split				varchar(1024)	DEFAULT NULL,
	yield					float			DEFAULT 0,
	parts_received			int(10)			DEFAULT 0,
	pretest_rejects			int(10)			DEFAULT 0,
	pretest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_tested			int(10)			DEFAULT 0,
	parts_pass				int(10)			DEFAULT 0,
	parts_pass_split		varchar(1024)	DEFAULT NULL,
	parts_fail				int(10)			DEFAULT 0,
	parts_fail_split		varchar(1024)	DEFAULT NULL,
	parts_retest			int(10)			DEFAULT 0,
	parts_retest_split		varchar(1024)	DEFAULT NULL,
	insertions				int(10)			DEFAULT 0,
	posttest_rejects		int(10)			DEFAULT 0,
	posttest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_shipped			int(10)			DEFAULT 0
); 

DROP TABLE IF EXISTS wt_wyr_format;
CREATE TABLE wt_wyr_format (
	site_name				varchar(255)	NOT NULL,
	column_id				tinyint(3)		NOT NULL,
	column_nb				tinyint(3)		NOT NULL,
	column_name				varchar(255)	NOT NULL,
	data_type				varchar(255)	NOT NULL,
	display					char(1)			DEFAULT 'Y' NOT NULL
);

DROP TABLE IF EXISTS wt_metadata_mapping;
CREATE TABLE wt_metadata_mapping (
  meta_name					varchar(255)	NOT NULL,
  gex_name					varchar(255)	DEFAULT NULL,
  gexdb_table_name			varchar(255)	NOT NULL,
  gexdb_field_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL,
  gex_display_in_gui		char(1)			NOT NULL DEFAULT 'Y'
);

/*!40000 ALTER TABLE wt_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE wt_metadata_mapping ENABLE KEYS */;

DROP TABLE IF EXISTS wt_metadata_link;
CREATE TABLE wt_metadata_link (
  link_name					varchar(255)	NOT NULL,
  gexdb_table1_name			varchar(255)	NOT NULL,
  gexdb_field1_fullname		varchar(255)	NOT NULL,
  gexdb_table2_name			varchar(255)	NOT NULL,
  gexdb_field2_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL
);

/*!40000 ALTER TABLE wt_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE wt_metadata_link ENABLE KEYS */;

DROP TABLE IF EXISTS wt_lot;
CREATE TABLE wt_lot (
  lot_id 			varchar(255)			NOT NULL DEFAULT '',
  tracking_lot_id	varchar(255)			DEFAULT NULL,
  product_name 		varchar(255)			DEFAULT NULL,
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0',
  flags 			binary(2)
); 

CREATE INDEX wt_lot_index ON wt_lot (lot_id);

/*!40000 ALTER TABLE wt_lot DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot ENABLE KEYS */;

DROP TABLE IF EXISTS wt_lot_hbin;
CREATE TABLE wt_lot_hbin (
  lot_id			varchar(255)			NOT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); 

/*!40000 ALTER TABLE wt_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_lot_sbin;
CREATE TABLE wt_lot_sbin (
  lot_id			varchar(255)			NOT NULL,
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

/*!40000 ALTER TABLE wt_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_splitlot;
CREATE TABLE wt_splitlot (
  splitlot_id 		INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, -- int(10) unsigned		 auto_increment,
  lot_id 			varchar(255)			NOT NULL DEFAULT '',
  sublot_id 		varchar(255)			NOT NULL DEFAULT '',
  setup_t 			int(10)					NOT NULL DEFAULT '0',
  start_t 			int(10)					NOT NULL DEFAULT '0',
  finish_t 			int(10)					NOT NULL DEFAULT '0',
  week_nb			tinyint(3)				NOT NULL DEFAULT '0',
  year				smallint(5)				NOT NULL DEFAULT '0',
  stat_num 			tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  tester_name 		varchar(255)			NOT NULL DEFAULT '',
  tester_type 		varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(2)				NOT NULL DEFAULT '\0\0',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_samples	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_samples_good mediumint(8)		NOT NULL DEFAULT '0',
  nb_parts_summary 	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_summary_good mediumint(8)		NOT NULL DEFAULT '0',
  data_provider 	varchar(255)			DEFAULT '',
  data_type 		varchar(255)			DEFAULT '',
  prod_data 		char(1)					NOT NULL DEFAULT 'Y',
  retest_index 		tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  retest_hbins 		varchar(255)			DEFAULT NULL,
  rework_code 		tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  job_nam 			varchar(255)			NOT NULL DEFAULT '',
  job_rev 			varchar(255)			NOT NULL DEFAULT '',
  oper_nam 			varchar(255)			NOT NULL DEFAULT '',
  exec_typ 			varchar(255)			NOT NULL DEFAULT '',
  exec_ver 			varchar(255)			NOT NULL DEFAULT '',
  test_cod 			varchar(255)			NOT NULL DEFAULT '',
  facil_id 			varchar(255)			NOT NULL DEFAULT '',
  tst_temp 			varchar(255)			NOT NULL DEFAULT '',
  mode_cod 			char(1)					DEFAULT NULL,
  rtst_cod 			char(1)					DEFAULT NULL,
  prot_cod 			char(1)					DEFAULT NULL,
  burn_tim 			int(10) 				DEFAULT NULL,
  cmod_cod 			char(1)					DEFAULT NULL,
  part_typ 			varchar(255)			DEFAULT NULL,
  user_txt 			varchar(255)			DEFAULT NULL,
  aux_file 			varchar(255)			DEFAULT NULL,
  pkg_typ 			varchar(255)			DEFAULT NULL,
  famly_id 			varchar(255)			DEFAULT NULL,
  date_cod 			varchar(255)			DEFAULT NULL,
  floor_id 			varchar(255)			DEFAULT NULL,
  proc_id 			varchar(255)			DEFAULT NULL,
  oper_frq 			varchar(255)			DEFAULT NULL,
  spec_nam 			varchar(255)			DEFAULT NULL,
  spec_ver 			varchar(255)			DEFAULT NULL,
  flow_id 			varchar(255)			DEFAULT NULL,
  setup_id 			varchar(255)			DEFAULT NULL,
  dsgn_rev 			varchar(255)			DEFAULT NULL,
  eng_id 			varchar(255)			DEFAULT NULL,
  rom_cod 			varchar(255)			DEFAULT NULL,
  serl_num 			varchar(255)			DEFAULT NULL,
  supr_nam 			varchar(255)			DEFAULT NULL,
  nb_sites 			tinyint(3)				NOT NULL DEFAULT '1', -- unsigned
  head_num 			tinyint(3)				DEFAULT NULL, -- unsigned
  handler_typ		varchar(255)			DEFAULT NULL,
  handler_id		varchar(255)			DEFAULT NULL,
  card_typ			varchar(255)			DEFAULT NULL,
  card_id			varchar(255)			DEFAULT NULL,
  loadboard_typ		varchar(255)			DEFAULT NULL,
  loadboard_id		varchar(255)			DEFAULT NULL,
  dib_typ			varchar(255)			DEFAULT NULL,
  dib_id			varchar(255)			DEFAULT NULL,
  cable_typ			varchar(255)			DEFAULT NULL,
  cable_id			varchar(255)			DEFAULT NULL,
  contactor_typ		varchar(255)			DEFAULT NULL,
  contactor_id		varchar(255)			DEFAULT NULL,
  laser_typ			varchar(255)			DEFAULT NULL,
  laser_id			varchar(255)			DEFAULT NULL,
  extra_typ			varchar(255)			DEFAULT NULL,
  extra_id			varchar(255)			DEFAULT NULL,
  file_host_id 		int(10)					DEFAULT '0', -- unsigned
  file_path 		varchar(255)			NOT NULL DEFAULT '',
  file_name 		varchar(255)			NOT NULL DEFAULT '',
  valid_splitlot 	char(1)					NOT NULL DEFAULT 'N',
  insertion_time 	int(10)					NOT NULL DEFAULT '0', -- unsigned
  subcon_lot_id		varchar(255)			NOT NULL DEFAULT '',
  wafer_id 			varchar(255)			DEFAULT NULL,
  incremental_update	varchar(255)		DEFAULT NULL,
  sya_id			int(10)					DEFAULT '0' -- unsigned
  --  PRIMARY KEY  (SPLITLOT_ID)
);

CREATE INDEX wt_splitlot_index ON wt_splitlot (lot_id);

/*!40000 ALTER TABLE wt_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE wt_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS wt_wafer_info;
CREATE TABLE wt_wafer_info (
  lot_id			varchar(255)			NOT NULL DEFAULT '',
  wafer_id 			varchar(255)			DEFAULT NULL,
  fab_id 			varchar(255)			DEFAULT NULL,
  frame_id 			varchar(255)			DEFAULT NULL,
  mask_id 			varchar(255)			DEFAULT NULL,
  wafer_size 		float					DEFAULT NULL,
  die_ht 			float					DEFAULT NULL,
  die_wid 			float					DEFAULT NULL,
  wafer_units 		tinyint(3)				DEFAULT NULL, -- unsigned
  wafer_flat 		char(1)					DEFAULT NULL,
  center_x 			smallint(5)				DEFAULT NULL, -- unsigned
  center_y 			smallint(5)				DEFAULT NULL, -- unsigned
  pos_x 			char(1)					DEFAULT NULL,
  pos_y 			char(1)					DEFAULT NULL,
  gross_die			mediumint(8)			NOT NULL DEFAULT 0,
  nb_parts			mediumint(8)			NOT NULL DEFAULT 0,
  nb_parts_good		mediumint(8)			NOT NULL DEFAULT 0,
  flags 			binary(2)
);

CREATE INDEX wt_wafer_info_index ON wt_wafer_info (lot_id, wafer_id);

/*!40000 ALTER TABLE wt_wafer_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_info ENABLE KEYS */;

DROP TABLE IF EXISTS wt_wafer_hbin;
CREATE TABLE wt_wafer_hbin (
  lot_id			varchar(255)			NOT NULL,
  wafer_id 			varchar(255)			DEFAULT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); 

CREATE INDEX wt_wafer_hbin_index ON wt_wafer_hbin (lot_id, wafer_id);

/*!40000 ALTER TABLE wt_wafer_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_wafer_sbin;
CREATE TABLE wt_wafer_sbin (
  lot_id			varchar(255)			NOT NULL,
  wafer_id 			varchar(255)			DEFAULT NULL,
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX wt_wafer_sbin_index ON wt_wafer_sbin (lot_id, wafer_id);

/*!40000 ALTER TABLE wt_wafer_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_sbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_parts_stats_samples;
CREATE TABLE wt_parts_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX wt_parts_stats_samples_index ON wt_parts_stats_samples (splitlot_id, site_no);

/*!40000 ALTER TABLE wt_parts_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_parts_stats_samples ENABLE KEYS */;

DROP TABLE IF EXISTS wt_parts_stats_summary;
CREATE TABLE wt_parts_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_good 			mediumint(8)			DEFAULT NULL,
  nb_rtst 			mediumint(8)			DEFAULT NULL
);

CREATE INDEX wt_parts_stats_summary_index ON wt_parts_stats_summary (splitlot_id, site_no);

/*!40000 ALTER TABLE wt_parts_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_parts_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_hbin;
CREATE TABLE wt_hbin (
  splitlot_id		int(10)					NOT NULL DEFAULT '0', -- unsigned
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL
);

CREATE INDEX wt_hbin_index ON wt_hbin (splitlot_id);

/*!40000 ALTER TABLE wt_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_hbin_stats_samples;
CREATE TABLE wt_hbin_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX wt_hbin_stats_samples_index ON wt_hbin_stats_samples (splitlot_id, site_no, hbin_no);

/*!40000 ALTER TABLE wt_hbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin_stats_samples ENABLE KEYS */;



DROP TABLE IF EXISTS wt_hbin_stats_summary;
CREATE TABLE wt_hbin_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  bin_count 		mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); 

CREATE INDEX wt_hbin_stats_summary_index ON wt_hbin_stats_summary (splitlot_id, site_no, hbin_no);

/*!40000 ALTER TABLE wt_hbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_sbin;
CREATE TABLE wt_sbin (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL
); 

CREATE INDEX wt_sbin_index ON wt_sbin (splitlot_id);

/*!40000 ALTER TABLE wt_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_sbin_stats_samples;
CREATE TABLE wt_sbin_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX wt_sbin_stats_samples_index ON wt_sbin_stats_samples (splitlot_id, site_no, sbin_no);

/*!40000 ALTER TABLE wt_sbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin_stats_samples ENABLE KEYS */;

DROP TABLE IF EXISTS wt_sbin_stats_summary;
CREATE TABLE wt_sbin_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  bin_count 		mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX wt_sbin_stats_summary_index ON wt_sbin_stats_summary (splitlot_id, site_no, sbin_no);

/*!40000 ALTER TABLE wt_sbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin_stats_summary ENABLE KEYS */;

DROP TABLE IF EXISTS wt_run;
CREATE TABLE wt_run (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  part_id 			varchar(255)			DEFAULT NULL,
  part_x 			smallint(6)				DEFAULT NULL,
  part_y	 		smallint(6)				DEFAULT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				DEFAULT NULL, -- unsigned
  ttime 			int(10)					DEFAULT NULL, -- unsigned
  tests_executed 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tests_failed 		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  firstfail_tnum 	int(10)					DEFAULT NULL, -- unsigned
  firstfail_tname 	varchar(255)			DEFAULT NULL,
  retest_index 		tinyint(3)				NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX wt_run_index ON wt_run (splitlot_id);

/*!40000 ALTER TABLE wt_run DISABLE KEYS */;
/*!40000 ALTER TABLE wt_run ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_info;
CREATE TABLE wt_ptest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  units 			varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  ll 				float					DEFAULT NULL,
  hl 				float					DEFAULT NULL,
  testseq 			smallint(5)				DEFAULT NULL, -- unsigned
  spec_ll			float					DEFAULT NULL,
  spec_hl			float					DEFAULT NULL,
  spec_target		float					DEFAULT NULL
);

CREATE INDEX wt_ptest_info_index ON wt_ptest_info (splitlot_id);

/*!40000 ALTER TABLE wt_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_limits;
CREATE TABLE wt_ptest_limits (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  ll 				float					NOT NULL DEFAULT '0',
  hl 				float					NOT NULL DEFAULT '0'
);

/*!40000 ALTER TABLE wt_ptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_results;
CREATE TABLE wt_ptest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  value 			float					DEFAULT NULL
);

CREATE INDEX wt_ptest_results_index ON wt_ptest_results (splitlot_id, ptest_info_id);

/*!40000 ALTER TABLE wt_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_stats_samples;
CREATE TABLE wt_ptest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum		float					DEFAULT NULL
);

/*!40000 ALTER TABLE wt_ptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_stats_summary;
CREATE TABLE wt_ptest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL,
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE wt_ptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_info;
CREATE TABLE wt_mptest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  tpin_arrayindex	smallint(6)				NOT NULL DEFAULT '0',
  units 			varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  ll 				float					DEFAULT NULL,
  hl 				float					DEFAULT NULL,
  testseq 			smallint(5)				DEFAULT NULL, -- unsigned
  spec_ll			float					DEFAULT NULL,
  spec_hl			float					DEFAULT NULL,
  spec_target		float					DEFAULT NULL
);

CREATE INDEX wt_mptest_info_index ON wt_mptest_info (splitlot_id);

/*!40000 ALTER TABLE wt_mptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_limits;
CREATE TABLE wt_mptest_limits (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no			smallint(5)				NOT NULL DEFAULT '1',
  ll 				float					NOT NULL DEFAULT '0',
  hl 				float					NOT NULL DEFAULT '0'
);

/*!40000 ALTER TABLE wt_mptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_results;
CREATE TABLE wt_mptest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  value 			float					NOT NULL DEFAULT '0',
  tpin_pmrindex		smallint(6)				DEFAULT NULL
);

CREATE INDEX wt_mptest_results_index ON wt_mptest_results (splitlot_id, mptest_info_id);

/*!40000 ALTER TABLE wt_mptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_stats_samples;
CREATE TABLE wt_mptest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL
);

/*!40000 ALTER TABLE wt_mptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_stats_summary;
CREATE TABLE wt_mptest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL,
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE wt_mptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_info;
CREATE TABLE wt_ftest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  testseq 			smallint(5)				DEFAULT NULL -- unsigned
);

CREATE INDEX wt_ftest_info_index ON wt_ftest_info (splitlot_id);

/*!40000 ALTER TABLE wt_ftest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_results;
CREATE TABLE wt_ftest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL, -- unsigned
  run_id 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  vect_nam 			varchar(255)			NOT NULL DEFAULT '',
  vect_off 			smallint(6)				DEFAULT NULL
);

CREATE INDEX wt_ftest_results_index ON wt_ftest_results (splitlot_id, ftest_info_id);

/*!40000 ALTER TABLE wt_ftest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_stats_samples;
CREATE TABLE wt_ftest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE wt_ftest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_stats_summary;
CREATE TABLE wt_ftest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE wt_ftest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_stats_summary ENABLE KEYS */;

--
-- FINAL TEST
--
DROP TABLE IF EXISTS ft_multidie_tracking;
CREATE TABLE ft_multidie_tracking (
  ft_tracking_lot_id		varchar(255)		NOT NULL,
  wt_product_id			varchar(255)		NOT NULL,
  wt_tracking_lot_id		varchar(255)		NOT NULL,
  wt_sublot_id			varchar(255)		NOT NULL,
  die_id					varchar(255)		NOT NULL
);

CREATE INDEX ft_multidie_tracking_1 ON ft_multidie_tracking (ft_tracking_lot_id);
CREATE INDEX ft_multidie_tracking_2 ON ft_multidie_tracking (wt_tracking_lot_id);

DROP TABLE IF EXISTS ft_sya_set;
CREATE TABLE ft_sya_set (
 sya_id					INTEGER PRIMARY KEY AUTOINCREMENT	NOT NULL, -- int(10) unsigned auto_increment,
 product_id				varchar(255)		NOT NULL,
 creation_date			datetime			NOT NULL,
 user_comment			varchar(255)		DEFAULT NULL,
 ll_1					float				NOT NULL DEFAULT -1,
 hl_1					float				NOT NULL DEFAULT -1,
 ll_2					float				NOT NULL DEFAULT -1,
 hl_2					float				NOT NULL DEFAULT -1,
 start_date				datetime			NOT NULL,
 expiration_date		date				NOT NULL,
 expiration_email_date	datetime			DEFAULT NULL,
 rule_type				varchar(255)		NOT NULL,
 n1_parameter			float				NOT NULL DEFAULT -1,
 n2_parameter			float				NOT NULL DEFAULT -1,
 computation_fromdate	date				NOT NULL,
 computation_todate		date				NOT NULL,
 min_lots_required		smallint(5)			NOT NULL DEFAULT -1,
 min_data_points		smallint(5)			NOT NULL DEFAULT -1,
 options				tinyint(3)			NOT NULL DEFAULT 0, -- unsigned
 flags					tinyint(3)			NOT NULL DEFAULT 0 -- unsigned
 -- PRIMARY KEY  (SYA_ID)
);

DROP TABLE IF EXISTS ft_sbl;
CREATE TABLE ft_sbl (
 sya_id					int(10) NOT NULL, 	-- int(10) unsigned
 bin_no					smallint(5)	NOT NULL, 	-- unsigned
 bin_name				varchar(255)			DEFAULT NULL,
 ll_1					float					NOT NULL DEFAULT -1,
 hl_1					float					NOT NULL DEFAULT -1,
 ll_2					float					NOT NULL DEFAULT -1,
 hl_2					float					NOT NULL DEFAULT -1,
 PRIMARY KEY  (sya_id, bin_no),
 FOREIGN KEY  (sya_id) REFERENCES ft_sya_set(sya_id)
); 

DROP TABLE IF EXISTS ft_wyr;
CREATE TABLE ft_wyr (
	site_name				varchar(255)	NOT NULL,
	week_nb					tinyint(3)		DEFAULT NULL,
	year					smallint(5)		DEFAULT NULL,
	date_in					datetime		DEFAULT NULL,
	date_out				datetime		DEFAULT NULL,
	product_name			varchar(255)	DEFAULT NULL,
	program_name			varchar(255)	DEFAULT NULL,
	tester_name				varchar(255)	DEFAULT NULL,
	lot_id					varchar(255)	DEFAULT NULL,
	subcon_lot_id			varchar(255)	DEFAULT NULL,
	user_split				varchar(1024)	DEFAULT NULL,
	yield					float			DEFAULT 0,
	parts_received			int(10)			DEFAULT 0,
	pretest_rejects			int(10)			DEFAULT 0,
	pretest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_tested			int(10)			DEFAULT 0,
	parts_pass				int(10)			DEFAULT 0,
	parts_pass_split		varchar(1024)	DEFAULT NULL,
	parts_fail				int(10)			DEFAULT 0,
	parts_fail_split		varchar(1024)	DEFAULT NULL,
	parts_retest			int(10)			DEFAULT 0,
	parts_retest_split		varchar(1024)	DEFAULT NULL,
	insertions				int(10)			DEFAULT 0,
	posttest_rejects		int(10)			DEFAULT 0,
	posttest_rejects_split	varchar(1024)	DEFAULT NULL,
	parts_shipped			int(10)			DEFAULT 0
);

DROP TABLE IF EXISTS ft_wyr_format;
CREATE TABLE ft_wyr_format (
	site_name				varchar(255)	NOT NULL,
	column_id				tinyint(3)		NOT NULL,
	column_nb				tinyint(3)		NOT NULL,
	column_name				varchar(255)	NOT NULL,
	data_type				varchar(255)	NOT NULL,
	display					char(1)			DEFAULT 'Y' NOT NULL
);

DROP TABLE IF EXISTS ft_metadata_mapping;
CREATE TABLE ft_metadata_mapping (
  meta_name					varchar(255)	NOT NULL,
  gex_name					varchar(255)	DEFAULT NULL,
  gexdb_table_name			varchar(255)	NOT NULL,
  gexdb_field_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL,
  gex_display_in_gui		char(1)			NOT NULL DEFAULT 'Y'
);

/*!40000 ALTER TABLE ft_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE ft_metadata_mapping ENABLE KEYS */;

DROP TABLE IF EXISTS ft_metadata_link;
CREATE TABLE ft_metadata_link (
  link_name					varchar(255)	NOT NULL,
  gexdb_table1_name			varchar(255)	NOT NULL,
  gexdb_field1_fullname		varchar(255)	NOT NULL,
  gexdb_table2_name			varchar(255)	NOT NULL,
  gexdb_field2_fullname		varchar(255)	NOT NULL,
  gexdb_link_name			varchar(255)	DEFAULT NULL
);

/*!40000 ALTER TABLE ft_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE ft_metadata_link ENABLE KEYS */;

INSERT INTO ft_metadata_link VALUES('ft_multidie_tracking-ft_lot','ft_multidie_tracking','ft_multidie_tracking.ft_tracking_lot_id','ft_lot','ft_lot.tracking_lot_id','ft_lot-ft_splitlot');
INSERT INTO ft_metadata_mapping VALUES('Wafer Die ID', '', 'ft_multidie_tracking', 'ft_multidie_tracking.die_id','ft_multidie_tracking-ft_lot','Y');
INSERT INTO ft_metadata_mapping VALUES('Wafer Product ID', '', 'ft_multidie_tracking', 'ft_multidie_tracking.wt_product_id','ft_multidie_tracking-ft_lot','Y');
INSERT INTO ft_metadata_mapping VALUES('Wafer Tracking Lot ID', '', 'ft_multidie_tracking', 'ft_multidie_tracking.wt_tracking_lot_id','ft_multidie_tracking-ft_lot','Y');
INSERT INTO ft_metadata_mapping VALUES('Wafer Sublot ID', '', 'ft_multidie_tracking', 'ft_multidie_tracking.wt_sublot_id','ft_multidie_tracking-ft_lot','Y');


DROP TABLE IF EXISTS ft_lot;
CREATE TABLE ft_lot (
  lot_id 			varchar(255)			NOT NULL DEFAULT '',
  tracking_lot_id	varchar(255)			DEFAULT NULL,
  product_name 		varchar(255)			DEFAULT NULL,
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0',
  flags 			binary(2)
);

CREATE INDEX ft_lot_index ON ft_lot (lot_id);

/*!40000 ALTER TABLE ft_lot DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot ENABLE KEYS */;

DROP TABLE IF EXISTS ft_lot_hbin;
CREATE TABLE ft_lot_hbin (
  lot_id			varchar(255)			NOT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); 

/*!40000 ALTER TABLE ft_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS ft_lot_sbin;
CREATE TABLE ft_lot_sbin (
  lot_id			varchar(255)			NOT NULL,
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL,
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); -- ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_splitlot;
CREATE TABLE ft_splitlot (
  splitlot_id 		INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, -- int(10) unsigned		 auto_increment,
  lot_id 			varchar(255)			NOT NULL DEFAULT '',
  sublot_id 		varchar(255)			NOT NULL DEFAULT '',
  setup_t 			int(10)					NOT NULL DEFAULT '0',
  start_t 			int(10)					NOT NULL DEFAULT '0',
  finish_t 			int(10)					NOT NULL DEFAULT '0',
  week_nb			tinyint(3)				NOT NULL DEFAULT '0',
  year				smallint(5)				NOT NULL DEFAULT '0',
  stat_num 			tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  tester_name 		varchar(255)			NOT NULL DEFAULT '',
  tester_type 		varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(2)				NOT NULL DEFAULT '\0\0',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_samples	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_samples_good mediumint(8)		NOT NULL DEFAULT '0',
  nb_parts_summary 	mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_summary_good mediumint(8)		NOT NULL DEFAULT '0',
  data_provider 	varchar(255)			DEFAULT '',
  data_type 		varchar(255)			DEFAULT '',
  prod_data 		char(1)					NOT NULL DEFAULT 'Y',
  retest_index 		tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  retest_hbins 		varchar(255)			DEFAULT NULL,
  rework_code 		tinyint(3)				NOT NULL DEFAULT '0', -- unsigned
  job_nam 			varchar(255)			NOT NULL DEFAULT '',
  job_rev 			varchar(255)			NOT NULL DEFAULT '',
  oper_nam 			varchar(255)			NOT NULL DEFAULT '',
  exec_typ 			varchar(255)			NOT NULL DEFAULT '',
  exec_ver 			varchar(255)			NOT NULL DEFAULT '',
  test_cod 			varchar(255)			NOT NULL DEFAULT '',
  facil_id 			varchar(255)			NOT NULL DEFAULT '',
  tst_temp 			varchar(255)			NOT NULL DEFAULT '',
  mode_cod 			char(1)					DEFAULT NULL,
  rtst_cod 			char(1)					DEFAULT NULL,
  prot_cod 			char(1)					DEFAULT NULL,
  burn_tim 			int(10) 				DEFAULT NULL,
  cmod_cod 			char(1)					DEFAULT NULL,
  part_typ 			varchar(255)			DEFAULT NULL,
  user_txt 			varchar(255)			DEFAULT NULL,
  aux_file 			varchar(255)			DEFAULT NULL,
  pkg_typ 			varchar(255)			DEFAULT NULL,
  famly_id 			varchar(255)			DEFAULT NULL,
  date_cod 			varchar(255)			DEFAULT NULL,
  floor_id 			varchar(255)			DEFAULT NULL,
  proc_id 			varchar(255)			DEFAULT NULL,
  oper_frq 			varchar(255)			DEFAULT NULL,
  spec_nam 			varchar(255)			DEFAULT NULL,
  spec_ver 			varchar(255)			DEFAULT NULL,
  flow_id 			varchar(255)			DEFAULT NULL,
  setup_id 			varchar(255)			DEFAULT NULL,
  dsgn_rev 			varchar(255)			DEFAULT NULL,
  eng_id 			varchar(255)			DEFAULT NULL,
  rom_cod 			varchar(255)			DEFAULT NULL,
  serl_num 			varchar(255)			DEFAULT NULL,
  supr_nam 			varchar(255)			DEFAULT NULL,
  nb_sites 			tinyint(3)				NOT NULL DEFAULT '1', -- unsigned
  head_num 			tinyint(3)				DEFAULT NULL, -- unsigned
  handler_typ		varchar(255)			DEFAULT NULL,
  handler_id		varchar(255)			DEFAULT NULL,
  card_typ			varchar(255)			DEFAULT NULL,
  card_id			varchar(255)			DEFAULT NULL,
  loadboard_typ		varchar(255)			DEFAULT NULL,
  loadboard_id		varchar(255)			DEFAULT NULL,
  dib_typ			varchar(255)			DEFAULT NULL,
  dib_id			varchar(255)			DEFAULT NULL,
  cable_typ			varchar(255)			DEFAULT NULL,
  cable_id			varchar(255)			DEFAULT NULL,
  contactor_typ		varchar(255)			DEFAULT NULL,
  contactor_id		varchar(255)			DEFAULT NULL,
  laser_typ			varchar(255)			DEFAULT NULL,
  laser_id			varchar(255)			DEFAULT NULL,
  extra_typ			varchar(255)			DEFAULT NULL,
  extra_id			varchar(255)			DEFAULT NULL,
  file_host_id 		int(10)					DEFAULT '0', -- unsigned
  file_path 		varchar(255)			NOT NULL DEFAULT '',
  file_name 		varchar(255)			NOT NULL DEFAULT '',
  valid_splitlot 	char(1)					NOT NULL DEFAULT 'N',
  insertion_time 	int(10)					NOT NULL DEFAULT '0', -- unsigned
  subcon_lot_id		varchar(255)			NOT NULL DEFAULT '',
  incremental_update	varchar(255)		DEFAULT NULL,
  sya_id			int(10)					DEFAULT '0' -- unsigned
  -- PRIMARY KEY  (SPLITLOT_ID)
); 

CREATE INDEX ft_splitlot_index ON ft_splitlot (lot_id);

/*!40000 ALTER TABLE ft_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE ft_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS ft_parts_stats_samples;
CREATE TABLE ft_parts_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  nb_parts			mediumint(8)			NOT NULL DEFAULT '0',
  nb_parts_good 	mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX ft_parts_stats_samples_index ON ft_parts_stats_samples (splitlot_id, site_no);

/*!40000 ALTER TABLE ft_parts_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_parts_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_parts_stats_summary;
CREATE TABLE ft_parts_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0',
  nb_good 			mediumint(8)			DEFAULT NULL,
  nb_rtst 			mediumint(8)			DEFAULT NULL
); 

CREATE INDEX ft_parts_stats_summary_index ON ft_parts_stats_summary (splitlot_id, site_no);

/*!40000 ALTER TABLE ft_parts_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_parts_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_hbin;
CREATE TABLE ft_hbin (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  hbin_name 		varchar(255)			NOT NULL DEFAULT '',
  hbin_cat 			char(1)					DEFAULT NULL
);

CREATE INDEX ft_hbin_index ON ft_hbin (splitlot_id);

/*!40000 ALTER TABLE ft_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_hbin_stats_samples;
CREATE TABLE ft_hbin_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX ft_hbin_stats_samples_index ON ft_hbin_stats_samples (splitlot_id, site_no, hbin_no);

/*!40000 ALTER TABLE ft_hbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin_stats_samples ENABLE KEYS */;

DROP TABLE IF EXISTS ft_hbin_stats_summary;
CREATE TABLE ft_hbin_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  bin_count 		mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
); 

CREATE INDEX ft_hbin_stats_summary_index ON ft_hbin_stats_summary (splitlot_id, site_no, hbin_no);

/*!40000 ALTER TABLE ft_hbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin;
CREATE TABLE ft_sbin (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_name 		varchar(255)			NOT NULL DEFAULT '',
  sbin_cat 			char(1)					DEFAULT NULL
); 

CREATE INDEX ft_sbin_index ON ft_sbin (splitlot_id);

/*!40000 ALTER TABLE ft_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin_stats_samples;
CREATE TABLE ft_sbin_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  nb_parts 			mediumint(8)			NOT NULL DEFAULT '0'
);

CREATE INDEX ft_sbin_stats_samples_index ON ft_sbin_stats_samples (splitlot_id, site_no, sbin_no);

/*!40000 ALTER TABLE ft_sbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin_stats_summary;
CREATE TABLE ft_sbin_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  sbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  bin_count 		mediumint(8)			NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX ft_sbin_stats_summary_index ON ft_sbin_stats_summary (splitlot_id, site_no, sbin_no);

/*!40000 ALTER TABLE ft_sbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_run;
CREATE TABLE ft_run (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  run_id 			mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  part_id 			varchar(255)			DEFAULT NULL,
  part_x			smallint(6)				DEFAULT NULL,
  part_y 			smallint(6)				DEFAULT NULL,
  hbin_no 			smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  sbin_no 			smallint(5)				DEFAULT NULL, -- unsigned
  ttime 			int(10)					DEFAULT NULL, -- unsigned
  tests_executed 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tests_failed 		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  firstfail_tnum 	int(10)					DEFAULT NULL, -- unsigned
  firstfail_tname 	varchar(255)			DEFAULT NULL,
  retest_index 		tinyint(3)				NOT NULL DEFAULT '0' -- unsigned
);

CREATE INDEX ft_run_index ON ft_run (splitlot_id);

/*!40000 ALTER TABLE ft_run DISABLE KEYS */;
/*!40000 ALTER TABLE ft_run ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_info;
CREATE TABLE ft_ptest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  units 			varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  ll 				float					DEFAULT NULL,
  hl 				float					DEFAULT NULL,
  testseq 			smallint(5)				DEFAULT NULL, -- unsigned
  spec_ll			float					DEFAULT NULL,
  spec_hl			float					DEFAULT NULL,
  spec_target		float					DEFAULT NULL
);

CREATE INDEX ft_ptest_info_index ON ft_ptest_info (splitlot_id);

/*!40000 ALTER TABLE ft_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_limits;
CREATE TABLE ft_ptest_limits (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  ll 				float					NOT NULL DEFAULT '0',
  hl 				float					NOT NULL DEFAULT '0'
);

/*!40000 ALTER TABLE ft_ptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_results;
CREATE TABLE ft_ptest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  run_id 			mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  value 			float					DEFAULT NULL
);

CREATE INDEX ft_ptest_results_index ON ft_ptest_results (splitlot_id, ptest_info_id);

/*!40000 ALTER TABLE ft_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_stats_samples;
CREATE TABLE ft_ptest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  min_value	 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL
);

/*!40000 ALTER TABLE ft_ptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_stats_summary;
CREATE TABLE ft_ptest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  fail_count	 	mediumint(8)			DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL,
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE ft_ptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_info;
CREATE TABLE ft_mptest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  tpin_arrayindex	smallint(6)				NOT NULL DEFAULT '0',
  units 			varchar(255)			NOT NULL DEFAULT '',
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  ll 				float					DEFAULT NULL,
  hl 				float					DEFAULT NULL,
  testseq 			smallint(5)				DEFAULT NULL, -- unsigned
  spec_ll			float					DEFAULT NULL,
  spec_hl			float					DEFAULT NULL,
  spec_target		float					DEFAULT NULL
);

CREATE INDEX ft_mptest_info_index ON ft_mptest_info (splitlot_id);

/*!40000 ALTER TABLE ft_mptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_limits;
CREATE TABLE ft_mptest_limits (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  ll 				float					NOT NULL DEFAULT '0',
  hl 				float					NOT NULL DEFAULT '0'
);

/*!40000 ALTER TABLE ft_mptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_results;
CREATE TABLE ft_mptest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  run_id 			mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  value 			float					NOT NULL DEFAULT '0',
  tpin_pmrindex		smallint(6)				DEFAULT NULL
);

CREATE INDEX ft_mptest_results_index ON ft_mptest_results (splitlot_id, mptest_info_id);

/*!40000 ALTER TABLE ft_mptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_stats_samples;
CREATE TABLE ft_mptest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL
);

/*!40000 ALTER TABLE ft_mptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_stats_summary;
CREATE TABLE ft_mptest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  mptest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  min_value 		float					DEFAULT NULL,
  max_value 		float					DEFAULT NULL,
  sum 				float					DEFAULT NULL,
  square_sum 		float					DEFAULT NULL,
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE ft_mptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_info;
CREATE TABLE ft_ftest_info (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id		smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  tnum 				int(10)					NOT NULL DEFAULT '0', -- unsigned
  tname 			varchar(255)			NOT NULL DEFAULT '',
  testseq 			smallint(5)				DEFAULT NULL -- unsigned
);

CREATE INDEX ft_ftest_info_index ON ft_ftest_info (splitlot_id);

/*!40000 ALTER TABLE ft_ftest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_info ENABLE KEYS */;

DROP TABLE IF EXISTS ft_ftest_results;
CREATE TABLE ft_ftest_results (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL, -- unsigned
  run_id 			mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  flags 			binary(1)				NOT NULL DEFAULT '\0',
  vect_nam 			varchar(255)			NOT NULL DEFAULT '',
  vect_off 			smallint(6)				DEFAULT NULL
);

CREATE INDEX ft_ftest_results_index ON ft_ftest_results (splitlot_id, ftest_info_id);

/*!40000 ALTER TABLE ft_ftest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_stats_samples;
CREATE TABLE ft_ftest_stats_samples (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			NOT NULL DEFAULT '0', -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE ft_ftest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_stats_summary;
CREATE TABLE ft_ftest_stats_summary (
  splitlot_id 		int(10)					NOT NULL DEFAULT '0', -- unsigned
  ftest_info_id 	smallint(5)				NOT NULL DEFAULT '0', -- unsigned
  site_no 			smallint(5)				NOT NULL DEFAULT '1',
  exec_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  fail_count 		mediumint(8)			DEFAULT NULL, -- unsigned
  ttime 			int(10)					DEFAULT NULL -- unsigned
);

/*!40000 ALTER TABLE ft_ftest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_stats_summary ENABLE KEYS */;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
