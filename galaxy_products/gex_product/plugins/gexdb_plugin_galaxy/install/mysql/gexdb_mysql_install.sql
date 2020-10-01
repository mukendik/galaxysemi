

--
-- Table structure for table global_options
--

DROP TABLE IF EXISTS global_info;

CREATE TABLE global_info (
  db_version_name varchar(255) NOT NULL COMMENT 'commercial name of the database version',
  db_version_nb smallint(5) NOT NULL COMMENT 'technical number of the database version',
  db_version_build smallint(5) NOT NULL COMMENT 'build number of the database version',
  incremental_splitlots int(9) NOT NULL DEFAULT '0' COMMENT 'dynamically stores the number of splitlots that needs to be incrementally updated',
  db_status varchar(255) DEFAULT NULL COMMENT 'current state of the database, stores the tasks that needs to be done if the database is not ready',
  db_type varchar(255) DEFAULT NULL COMMENT 'stores a ciphered description of the database type (yieldman prod, manual prod or characterization db)',
  PRIMARY KEY (db_version_build)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='global information about the database version and status';

INSERT INTO global_info VALUES ('GEXDB V7.01 B75 (MySQL)', 701, 75, 0, 'UPDATING_CONSOLIDATION_TREE|UPDATING_CONSOLIDATION_TRIGGERS|UPDATING_CONSOLIDATION_TABLES|UPDATING_CONSOLIDATION_PROCEDURES|UPDATING_INDEXES', NULL);


--
-- Table structure for table global_options
--

DROP TABLE IF EXISTS global_options;

CREATE TABLE global_options (
  option_name varchar(255) NOT NULL COMMENT 'the key of the option',
  option_value varchar(255) NOT NULL COMMENT 'the value of the option',
  PRIMARY KEY (option_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='global options of the database stored using a key->value format';

INSERT INTO global_options VALUES ('GEXDB_MYSQL_ENGINE', 'InnoDB');


--
-- Table structure for table global_files
--

DROP TABLE IF EXISTS global_files;


CREATE TABLE global_files (
  file_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'unique identifier of the virtual file',
  file_name varchar(255) NOT NULL COMMENT 'virtual file name',
  file_type varchar(255) NOT NULL COMMENT 'virtual file type (ex: "consolidation")',
  file_format varchar(255) NOT NULL COMMENT 'virtual file format (ex: "xml")',
  file_content mediumtext NOT NULL COMMENT 'raw content of the virtual file',
  file_checksum int(10) unsigned NOT NULL COMMENT 'checksum of the file content',
  file_last_update datetime NOT NULL COMMENT 'date of the last content update',
  PRIMARY KEY (file_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='storage for virtual files content, currently used for consolidation tree storage only';


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
  host_ftpport smallint(5) unsigned NOT NULL DEFAULT '21',
  PRIMARY KEY (file_host_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='stores de ftp connexions -defined in the *.gexdbkeys file used at import time- to associate with a splitlot';

--
-- Table structure for table incremental_update
--

DROP TABLE IF EXISTS incremental_update;


CREATE TABLE incremental_update (
  db_update_name varchar(255) NOT NULL COMMENT 'name of the action that must be performed by the incremental update',
  processed_splitlots int(9) NOT NULL DEFAULT '0' COMMENT 'number of splitlots processed for this action',
  remaining_splitlots int(9) NOT NULL DEFAULT '0' COMMENT 'number of splitlots to be processed for this action',
  status varchar(255) DEFAULT 'DISABLED' COMMENT 'Flags whether this action is ENABLED or DISABLED for processing by the scheduler',
  frequency varchar(255) DEFAULT '*/10 * * * *' COMMENT 'CRON-compliant frequency at which this action must be performed',
  max_items int(9) DEFAULT 10 COMMENT 'maximum number of items to be processed in a single scheduler execution',
  last_schedule datetime DEFAULT NULL COMMENT 'indicates when this action was last scheduled by the yield-man scheduler',
  last_execution datetime DEFAULT NULL COMMENT 'indicates when this action was last performed by the yield-man scheduler',
  PRIMARY KEY (db_update_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='summaries of the incremental updates required to run on the database';


--
-- Table structure for table token
--

DROP TABLE IF EXISTS token;

CREATE TABLE token (
  start_time datetime NOT NULL COMMENT 'timestamp at which the product started processing',
  name varchar(256) NOT NULL COMMENT 'name of the action executed by the product which took the token',
  key_value varchar(512) NOT NULL COMMENT 'key of the object on which the product is processing',
  session_id varchar(256) NOT NULL COMMENT 'session id of the product processing',
  PRIMARY KEY (name,key_value)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='stores the tokens used to synchronize several products working in parallel on the database';


--
-- Table structure for table gexdb_log
--

DROP TABLE IF EXISTS gexdb_log;


CREATE TABLE gexdb_log (
  log_id INT NOT NULL AUTO_INCREMENT,
  log_date datetime NOT NULL,
  log_type varchar(255) NOT NULL,
  log_string varchar(1024) NOT NULL,
  PRIMARY KEY (log_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='log entries of the stored procedures debugging process';


--
-- Table structure for table product
--

DROP TABLE IF EXISTS product;


CREATE TABLE product (
  product_name varchar(255) NOT NULL DEFAULT '',
  description varchar(1000) DEFAULT NULL,
  PRIMARY KEY (product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='tested products';


--
-- Table structure for table et_product_sbin
--

DROP TABLE IF EXISTS et_product_sbin;


CREATE TABLE et_product_sbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. software binning at product level';


--
-- Table structure for table et_product_hbin
--

DROP TABLE IF EXISTS et_product_hbin;


CREATE TABLE et_product_hbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. hardware binning at product level';


--
-- Table structure for table et_lot
--

DROP TABLE IF EXISTS et_lot;


CREATE TABLE et_lot (
  lot_id varchar(255) NOT NULL COMMENT 'lot internal identifier',
  tracking_lot_id varchar(255) NOT NULL COMMENT 'lot identifier in the manufacturer\'s tracking system',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  nb_parts int(10) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this lot',
  nb_parts_good int(10) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this lot',
  lot_flags tinyint(2) unsigned DEFAULT NULL COMMENT 'not currently in use',
  PRIMARY KEY (lot_id,product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the lots tested at the E.T. stage';


--
-- Table structure for table et_lot_metadata
--

DROP TABLE IF EXISTS et_lot_metadata;

CREATE TABLE et_lot_metadata (
  lot_id varchar(255) NOT NULL,
  product_name varchar(255) NOT NULL,
  PRIMARY KEY (lot_id, product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the lots tested at the E.T. stage';


--
-- Table structure for table et_lot_sbin
--

DROP TABLE IF EXISTS et_lot_sbin;


CREATE TABLE et_lot_sbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. software binning at lot level';


--
-- Table structure for table et_lot_hbin
--

DROP TABLE IF EXISTS et_lot_hbin;


CREATE TABLE et_lot_hbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. hardware binning at lot level';


--
-- Table structure for table et_wafer_info
--

DROP TABLE IF EXISTS et_wafer_info;


CREATE TABLE et_wafer_info (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) NOT NULL COMMENT 'wafer internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the to the product name',
  fab_id varchar(255) DEFAULT NULL COMMENT 'wafer identifier in the manufacturer''s tracking system',
  frame_id varchar(255) DEFAULT NULL COMMENT 'additional wafer tracking identifier',
  mask_id varchar(255) DEFAULT NULL COMMENT 'identifier of the wafer mask',
  wafer_size float DEFAULT NULL COMMENT 'diameter of wafer in WF_UNITS',
  die_ht float DEFAULT NULL COMMENT 'height of a die in WF_UNITS',
  die_wid float DEFAULT NULL COMMENT 'width of a die in WF_UNITS',
  wafer_units tinyint(3) unsigned DEFAULT NULL COMMENT 'units used for WF_UNITS',
  wafer_flat char(1) DEFAULT NULL COMMENT 'orientation of the wafer flat',
  center_x smallint(5) unsigned DEFAULT NULL COMMENT 'X coordinates of center die on wafer',
  center_y smallint(5) unsigned DEFAULT NULL COMMENT 'Y coordinates of center die on wafer',
  pos_x char(1) DEFAULT NULL COMMENT 'positive X direction of the wafer',
  pos_y char(1) DEFAULT NULL COMMENT 'positive Y direction of the wafer',
  gross_die mediumint(8) NOT NULL DEFAULT '0',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this wafer',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this lot',
  wafer_flags tinyint(2) unsigned DEFAULT NULL,
  wafer_nb tinyint(3) unsigned DEFAULT NULL COMMENT 'wafer number within its lot',
  site_config varchar(255) DEFAULT NULL COMMENT 'additional information on the site configuration retrieved from the *.gexdbkeys file used at import time',
  consolidation_algo varchar(45) DEFAULT NULL,
  consolidation_status char(1) DEFAULT NULL COMMENT 'consolidation status of the wafer',
  consolidation_summary TEXT DEFAULT NULL,
  consolidation_date datetime DEFAULT NULL COMMENT 'date of the consolidation',  
  PRIMARY KEY (lot_id,wafer_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the wafers tested at the E.T. stage';


--
-- Table structure for table et_wafer_sbin
--

DROP TABLE IF EXISTS et_wafer_sbin;


CREATE TABLE et_wafer_sbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,wafer_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. software binning at wafer level';

--
-- Table structure for table wt_wafer_sbin_inter
--

DROP TABLE IF EXISTS et_wafer_sbin_inter;

CREATE TABLE et_wafer_sbin_inter (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the wafer internal identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,sbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the W.T. software binning at wafer level, as specified by the consolidation tree';

--
-- Table structure for table wt_wafer_consolidation
--

DROP TABLE IF EXISTS et_wafer_consolidation;

CREATE TABLE et_wafer_consolidation (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) DEFAULT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='wafer consolidation informations';

--
-- Table structure for table wt_wafer_consolidation_inter
--

DROP TABLE IF EXISTS et_wafer_consolidation_inter;

CREATE TABLE et_wafer_consolidation_inter (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) DEFAULT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='wafer intermediate consolidation informations';

--
-- Table structure for table et_wafer_hbin
--

DROP TABLE IF EXISTS et_wafer_hbin;


CREATE TABLE et_wafer_hbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,wafer_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the E.T. hardware binning at lot level';

--
-- Table structure for table et_wafer_hbin_inter
--

DROP TABLE IF EXISTS et_wafer_hbin_inter;

CREATE TABLE et_wafer_hbin_inter (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the wafer internal identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,hbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the W.T. hardware binning at wafer level, as specified by the consolidation tree';

--
-- Table structure for table et_splitlot
--

DROP TABLE IF EXISTS et_splitlot;


CREATE TABLE et_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'automatically incremented unique identifier of the splitlot. The format is AAdddXXX with AA being the last two digits of the current year, ddd the index of the day in the year when the partition was created, and xxx a counter initialised with the value 000',
  lot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the lot internal identifier',
  sublot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'there is no such et_sublot table, therefore this column may be used or not depending on how the customer works',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer identifier',
  product_name varchar(255) DEFAULT NULL,
  start_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the first part tested',
  finish_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the last part tested',
  tester_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the node that generated the data',
  tester_type varchar(255) NOT NULL DEFAULT '' COMMENT 'type of the tester that generated the data',
  splitlot_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of good parts',
  data_provider varchar(255) DEFAULT '' COMMENT 'company which generated the data',
  data_type varchar(255) DEFAULT '' COMMENT 'defines the type of data: prod, engineering, QA, characterization...',
  prod_data char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether the data type is production (Y) or not (N)',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Retest index within a phase (0 for initial test, 1 for first retest, ...)',
  retest_hbins varchar(255) DEFAULT NULL COMMENT 'Retested hard bins in case of LINEAR consolidation, test/retest consolidation algorithm in case of STACKED consolidation.',
  test_insertion_index tinyint(1) unsigned DEFAULT NULL COMMENT 'Indicates the index of the insertion for multi-insertions',
  test_insertion varchar(255) DEFAULT NULL COMMENT 'Test insertion name in case of multiple insertions to generate the consolidation result',
  test_flow varchar(45) DEFAULT NULL COMMENT 'Test flow name in case of multiple flows. This can allow an additional consolidation granularity.',
  mode_cod char(1) DEFAULT NULL COMMENT 'test mode code (prod, test...)',
  job_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) name',
  job_rev varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) revision',
  oper_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job setup operator name or identifier',
  exec_typ varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer software type',
  exec_ver varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer sotfware version',
  facil_id varchar(255) NOT NULL DEFAULT '' COMMENT 'test facility identifier',
  part_typ varchar(255) DEFAULT NULL COMMENT 'part type (or product identifier)',
  user_txt varchar(255) DEFAULT NULL COMMENT 'generic user text',
  famly_id varchar(255) DEFAULT NULL COMMENT 'product family identifier',
  proc_id varchar(255) DEFAULT NULL COMMENT 'fabrication process identifier',
  spec_nam varchar(255) DEFAULT NULL COMMENT 'test specification name',
  spec_ver varchar(255) DEFAULT NULL COMMENT 'test specification version',
  file_host_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the file_host table',
  file_path varchar(255) NOT NULL DEFAULT '' COMMENT 'path of the file imported as a splitlot',
  file_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the file imported as a splitlot',
  valid_splitlot char(1) NOT NULL DEFAULT 'N' COMMENT 'whether the splitlot is valid (Y) or not (N)',
  insertion_time int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'timestamp of the splitlot insertion process',
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  incremental_update varchar(255) DEFAULT NULL COMMENT 'name of the action that must be performed on this splitlot by an incremental update. Matches the primary key or the "incremental_update" table',
  sya_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the SYA identifier',
  day varchar(10) NOT NULL COMMENT 'day of the start_t timestamp',
  week_nb tinyint(2) unsigned NOT NULL COMMENT 'week number of the start_t timestamp',
  month_nb tinyint(2) unsigned NOT NULL COMMENT 'month number for the start_t timestamp',
  quarter_nb tinyint(1) unsigned NOT NULL COMMENT 'quarter of the start_t timestamp',
  year_nb smallint(4) NOT NULL COMMENT 'year of the start_t timestamp',
  year_and_week varchar(7) NOT NULL COMMENT 'year and week of the start_t timestamp',
  year_and_month varchar(7) NOT NULL COMMENT 'year and month of the start_t timestamp',
  year_and_quarter varchar(7) NOT NULL COMMENT 'year and quarter of the start_t timestamp',
  wafer_nb tinyint(3) unsigned DEFAULT NULL COMMENT 'wafer number within its lot',
  site_config varchar(255) DEFAULT NULL COMMENT 'additional information on the site configuration retrieved from the *.gexdbkeys file used at import time',
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a et_splitlot is the single execution of a set of E.T. tests on a given number of dies. A splitlot matches with one test file.';


--
-- Table structure for table et_splitlot_metadata
--

DROP TABLE IF EXISTS et_splitlot_metadata;

CREATE TABLE et_splitlot_metadata (
  splitlot_id int(10) unsigned NOT NULL,
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the splitlot tested at E.T. stage';


--
-- Table structure for table et_sbin
--

DROP TABLE IF EXISTS et_sbin;


CREATE TABLE et_sbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts in that bin',
  bin_family varchar(255),
  bin_subfamily varchar(255),
  PRIMARY KEY (splitlot_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='E.T. software binning at splitlot level';

--
-- Table structure for table et_hbin
--

DROP TABLE IF EXISTS et_hbin;


CREATE TABLE et_hbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts in that bin',
  bin_family varchar(255),
  bin_subfamily varchar(255),
  PRIMARY KEY (splitlot_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='E.T. hardware binning at splitlot level';


--
-- Table structure for table et_run
--

DROP TABLE IF EXISTS et_run;


CREATE TABLE et_run (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  run_id mediumint(7) unsigned NOT NULL COMMENT 'run unique identifier',
  site_no smallint(5) NOT NULL DEFAULT 1 COMMENT 'test site number',
  part_id varchar(255) DEFAULT NULL COMMENT 'part (die) unique identifier',
  part_x smallint(6) DEFAULT NULL COMMENT 'X coordinates of the part (die) on the wafer',
  part_y smallint(6) DEFAULT NULL COMMENT 'Y coordinates of the part (die) on the wafer',
  part_txt varchar(255) DEFAULT NULL COMMENT 'miscellaneous text related to that die',
  part_retest_index tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT 'index of the die test (0) or retest (1+) status',
  part_status char(1) NOT NULL DEFAULT '' COMMENT 'consolidated P (test passed) or F (test failed) status of the die',
  part_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'BIT1: part retested, BIT2: no full touchdown',
  intermediate tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_insertion',
  final tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_flow',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'hardware binning of the die',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software binning of the die',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'duration of the test on that die, in milliseconds',
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests executed on that die',
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests failed on that die',
  firstfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  firstfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  lastfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  lastfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  PRIMARY KEY (splitlot_id,run_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a et_run is the single execution of a set of E.T. tests on a given die. Runs constitute the decomposition of a splitlot at die level.';


--
-- Table structure for table et_ptest_info
--

DROP TABLE IF EXISTS et_ptest_info;


CREATE TABLE et_ptest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  units varchar(255) NOT NULL DEFAULT '' COMMENT 'units used for the test measure',
  test_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'flags describing the test',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  spec_ll float DEFAULT NULL COMMENT 'test low limit as per the test specification (engineering stage)',
  spec_hl float DEFAULT NULL COMMENT 'test high limit as per the test specification (engineering stage)',
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,ptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the parametric E.T. tests';


--
-- Table structure for table et_ptest_stats
--

DROP TABLE IF EXISTS et_ptest_stats;


CREATE TABLE et_ptest_stats (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  exec_count smallint(5) unsigned DEFAULT NULL COMMENT 'how many times the test has been executed',
  fail_count smallint(5) unsigned DEFAULT NULL COMMENT 'how many times the test execution led to a failing die',
  min_value float DEFAULT NULL COMMENT 'minimum tested value amongst all the test executions',
  max_value float DEFAULT NULL COMMENT 'maximum tested value amongst all the test executions',
  sum float DEFAULT NULL COMMENT 'sum of all the tested values',
  square_sum float DEFAULT NULL COMMENT 'square of the sum of all the tested values',
  PRIMARY KEY (splitlot_id,ptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidated statistics of the parametric tests for a givne splitlot';


--
-- Table structure for table et_ptest_results
--

DROP TABLE IF EXISTS et_ptest_results;


CREATE TABLE et_ptest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test has been found in the stdf file',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  value float DEFAULT NULL COMMENT 'result value of the test',
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT 'results of the parametric tests for each die';

--
-- Table structure for table et_test_conditions (TDR)
--

DROP TABLE IF EXISTS et_test_conditions;


CREATE TABLE et_test_conditions(
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  test_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  test_type char(1) NOT NULL COMMENT 'indicates whether the test description identifier must be looked amongst the parametric, multiparametric or functional test descrpitions',

  condition_1 varchar(255) DEFAULT NULL COMMENT 'description of the first test condition',
  condition_2 varchar(255) DEFAULT NULL COMMENT 'description of the second test condition',
  condition_3 varchar(255) DEFAULT NULL COMMENT 'description of the third test condition',
  condition_4 varchar(255) DEFAULT NULL COMMENT 'and so on...',
  condition_5 varchar(255) DEFAULT NULL,
  condition_6 varchar(255) DEFAULT NULL,
  condition_7 varchar(255) DEFAULT NULL,
  condition_8 varchar(255) DEFAULT NULL,
  condition_9 varchar(255) DEFAULT NULL,

  condition_10 varchar(255) DEFAULT NULL,
  condition_11 varchar(255) DEFAULT NULL,
  condition_12 varchar(255) DEFAULT NULL,
  condition_13 varchar(255) DEFAULT NULL,
  condition_14 varchar(255) DEFAULT NULL,
  condition_15 varchar(255) DEFAULT NULL,
  condition_16 varchar(255) DEFAULT NULL,
  condition_17 varchar(255) DEFAULT NULL,
  condition_18 varchar(255) DEFAULT NULL,
  condition_19 varchar(255) DEFAULT NULL,

  condition_20 varchar(255) DEFAULT NULL,
  condition_21 varchar(255) DEFAULT NULL,
  condition_22 varchar(255) DEFAULT NULL,
  condition_23 varchar(255) DEFAULT NULL,
  condition_24 varchar(255) DEFAULT NULL,
  condition_25 varchar(255) DEFAULT NULL,
  condition_26 varchar(255) DEFAULT NULL,
  condition_27 varchar(255) DEFAULT NULL,
  condition_28 varchar(255) DEFAULT NULL,
  condition_29 varchar(255) DEFAULT NULL,

  condition_30 varchar(255) DEFAULT NULL,
  condition_31 varchar(255) DEFAULT NULL,
  condition_32 varchar(255) DEFAULT NULL,
  condition_33 varchar(255) DEFAULT NULL,
  condition_34 varchar(255) DEFAULT NULL,
  condition_35 varchar(255) DEFAULT NULL,
  condition_36 varchar(255) DEFAULT NULL,
  condition_37 varchar(255) DEFAULT NULL,
  condition_38 varchar(255) DEFAULT NULL,
  condition_39 varchar(255) DEFAULT NULL,

  condition_40 varchar(255) DEFAULT NULL,
  condition_41 varchar(255) DEFAULT NULL,
  condition_42 varchar(255) DEFAULT NULL,
  condition_43 varchar(255) DEFAULT NULL,
  condition_44 varchar(255) DEFAULT NULL,
  condition_45 varchar(255) DEFAULT NULL,
  condition_46 varchar(255) DEFAULT NULL,
  condition_47 varchar(255) DEFAULT NULL,
  condition_48 varchar(255) DEFAULT NULL,
  condition_49 varchar(255) DEFAULT NULL,

  condition_50 varchar(255) DEFAULT NULL,
  condition_51 varchar(255) DEFAULT NULL,
  condition_52 varchar(255) DEFAULT NULL,
  condition_53 varchar(255) DEFAULT NULL,
  condition_54 varchar(255) DEFAULT NULL,
  condition_55 varchar(255) DEFAULT NULL,
  condition_56 varchar(255) DEFAULT NULL,
  condition_57 varchar(255) DEFAULT NULL,
  condition_58 varchar(255) DEFAULT NULL,
  condition_59 varchar(255) DEFAULT NULL,

  condition_60 varchar(255) DEFAULT NULL,
  condition_61 varchar(255) DEFAULT NULL,
  condition_62 varchar(255) DEFAULT NULL,
  condition_63 varchar(255) DEFAULT NULL,
  condition_64 varchar(255) DEFAULT NULL,
  condition_65 varchar(255) DEFAULT NULL,
  condition_66 varchar(255) DEFAULT NULL,
  condition_67 varchar(255) DEFAULT NULL,
  condition_68 varchar(255) DEFAULT NULL,
  condition_69 varchar(255) DEFAULT NULL,

  condition_70 varchar(255) DEFAULT NULL,
  condition_71 varchar(255) DEFAULT NULL,
  condition_72 varchar(255) DEFAULT NULL,
  condition_73 varchar(255) DEFAULT NULL,
  condition_74 varchar(255) DEFAULT NULL,
  condition_75 varchar(255) DEFAULT NULL,
  condition_76 varchar(255) DEFAULT NULL,
  condition_77 varchar(255) DEFAULT NULL,
  condition_78 varchar(255) DEFAULT NULL,
  condition_79 varchar(255) DEFAULT NULL,

  condition_80 varchar(255) DEFAULT NULL,
  condition_81 varchar(255) DEFAULT NULL,
  condition_82 varchar(255) DEFAULT NULL,
  condition_83 varchar(255) DEFAULT NULL,
  condition_84 varchar(255) DEFAULT NULL,
  condition_85 varchar(255) DEFAULT NULL,
  condition_86 varchar(255) DEFAULT NULL,
  condition_87 varchar(255) DEFAULT NULL,
  condition_88 varchar(255) DEFAULT NULL,
  condition_89 varchar(255) DEFAULT NULL,

  condition_90 varchar(255) DEFAULT NULL,
  condition_91 varchar(255) DEFAULT NULL,
  condition_92 varchar(255) DEFAULT NULL,
  condition_93 varchar(255) DEFAULT NULL,
  condition_94 varchar(255) DEFAULT NULL,
  condition_95 varchar(255) DEFAULT NULL,
  condition_96 varchar(255) DEFAULT NULL,
  condition_97 varchar(255) DEFAULT NULL,
  condition_98 varchar(255) DEFAULT NULL,
  condition_99 varchar(255) DEFAULT NULL,

  condition_100 varchar(255) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,test_info_id,test_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='defines the test condition during the characterisation stage';


--
-- Table structure for table et_metadata_mapping
--

DROP TABLE IF EXISTS et_metadata_mapping;


CREATE TABLE et_metadata_mapping (
  meta_name varchar(255) NOT NULL COMMENT 'name of the additional field',
  gex_name varchar(255) DEFAULT NULL COMMENT 'name of the mapped field in gex',
  gexdb_table_name varchar(255) NOT NULL COMMENT 'name of the gex database table to which the additional field is mapped',
  gexdb_field_fullname varchar(255) NOT NULL COMMENT 'name of the column to which the additional field is mapped',
  gexdb_link_name varchar(255) DEFAULT NULL COMMENT 'foreign key to the metadata link name',
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether the field must be displayed in the gex GUI (Y) or not (N)',
  bintype_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a binary type logic (Y) or not (N)',
  time_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a time type logic (Y) or not (N)',
  custom_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be treated with a custom type logic (Y) or not (N)',
  numeric_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a numeric type logic (Y) or not (N)',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be processed for consolidation',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be processed for A-Z consolidation',
  PRIMARY KEY (meta_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='describes the additional fields usable for data filtering or consolidation';


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
  gexdb_link_name varchar(255) DEFAULT NULL,
  PRIMARY KEY (link_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


--
-- Table structure for table et_sya_set
--

DROP TABLE IF EXISTS et_sya_set;


CREATE TABLE et_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'SYA unique identifier',
  product_id varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  creation_date datetime NOT NULL COMMENT 'creation date of the SYA set',
  user_comment varchar(255) DEFAULT NULL COMMENT 'user defined comment',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SYA alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SYA alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SYA alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SYA alarm to trigger',
  start_date datetime NOT NULL COMMENT 'date from which the SYA alarm is valid',
  expiration_date date NOT NULL COMMENT 'date to which the SYA alarm is not valid anymore',
  expiration_email_date datetime DEFAULT NULL COMMENT 'date to which an expiration warning email must be sent',
  rule_type varchar(255) NOT NULL COMMENT 'SYA rule type',
  n1_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  computation_fromdate date NOT NULL COMMENT 'initial date from which the limits should be computed',
  computation_todate date NOT NULL COMMENT 'final date to which the limits should be computed',
  min_lots_required smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of lots for the computed limits to be valid',
  min_data_points smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of calculated yields for the computed limits to be valid',
  options tinyint(3) unsigned NOT NULL DEFAULT '0',
  flags tinyint(3) unsigned NOT NULL DEFAULT '0',
  rule_name varchar(255) DEFAULT NULL COMMENT 'name of the SYA rule',
  bin_type tinyint(1) DEFAULT NULL COMMENT 'whether the alarm should be triggered on a good (P) bin or a bad (F) bin',
  PRIMARY KEY (sya_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SYA rules appliable for the statistical yield analysis alarms';


--
-- Table structure for table et_sbl
--

DROP TABLE IF EXISTS et_sbl;


CREATE TABLE et_sbl (
  sya_id int(10) unsigned NOT NULL COMMENT 'foreign key to the associated SYA rule',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'bin number on which the SBL must be applied',
  bin_name varchar(255) DEFAULT NULL COMMENT 'name of the bin on which the SBL must be applied',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SBL alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SBL alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SBL alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SBL alarm to trigger',
  rule_type varchar(255) DEFAULT NULL COMMENT 'SBL rule type',
  n1_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SBL rules appliable along with SYA alarms';


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
  display char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY (site_name,column_id,data_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports formats';


--
-- Table structure for table et_wyr
--

DROP TABLE IF EXISTS et_wyr;


CREATE TABLE et_wyr (
  wyr_id INT NOT NULL AUTO_INCREMENT,
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  year smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT '0',
  parts_received int(10) DEFAULT '0',
  pretest_rejects int(10) DEFAULT '0',
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT '0',
  parts_pass int(10) DEFAULT '0',
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT '0',
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT '0',
  parts_retest_split varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT '0',
  posttest_rejects int(10) DEFAULT '0',
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT '0',
  PRIMARY KEY (wyr_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports';


--
-- Table structure for table et_dtr
--

DROP TABLE IF EXISTS et_dtr;


CREATE TABLE et_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT 0,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (splitlot_id,run_id,order_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='html summaries of PAT reports';


--
-- Table structure for table et_sdr
--

DROP TABLE IF EXISTS et_sdr;

CREATE TABLE et_sdr (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_grp smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site group number',
  site_index smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site index',
  site_no smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site number',
  hand_typ varchar(255) DEFAULT NULL COMMENT 'handler of prober type',
  hand_id varchar(255) DEFAULT NULL COMMENT 'handler of prober identifier',
  card_typ varchar(255) DEFAULT NULL COMMENT 'prober card type',
  card_id varchar(255) DEFAULT NULL COMMENT 'prober card identifier',
  load_typ varchar(255) DEFAULT NULL COMMENT 'load board type',
  load_id varchar(255) DEFAULT NULL COMMENT 'load board identifier',
  dib_typ varchar(255) DEFAULT NULL COMMENT 'DIB board type',
  dib_id varchar(255) DEFAULT NULL COMMENT 'DIB board identifier',
  cabl_typ varchar(255) DEFAULT NULL COMMENT 'interface cable type',
  cabl_id varchar(255) DEFAULT NULL COMMENT 'interface cable identifier',
  cont_typ varchar(255) DEFAULT NULL COMMENT 'handler contractor type',
  cont_id varchar(255) DEFAULT NULL COMMENT 'handler contractor identifier',
  lasr_typ varchar(255) DEFAULT NULL COMMENT 'laser type',
  lasr_id varchar(255) DEFAULT NULL COMMENT 'laser identifier',
  extr_typ varchar(255) DEFAULT NULL COMMENT 'extra equipment type',
  extr_id varchar(255) DEFAULT NULL COMMENT 'extra equipment identifier',
  PRIMARY KEY (splitlot_id, site_grp, site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the sites used for testing';


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
  alarm_flags tinyint(2) unsigned NOT NULL,
  lcl float NOT NULL DEFAULT '0',
  ucl float NOT NULL DEFAULT '0',
  value float NOT NULL DEFAULT '0',
  units varchar(10) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,alarm_cat,alarm_type,item_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='logs the alarms raised by the datapump tasks';





--
-- Table structure for table wt_product_sbin
--

DROP TABLE IF EXISTS wt_product_sbin;


CREATE TABLE wt_product_sbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. software binning at product level';


--
-- Table structure for table wt_product_hbin
--

DROP TABLE IF EXISTS wt_product_hbin;


CREATE TABLE wt_product_hbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. hardware binning at product level';


--
-- Table structure for table wt_lot
--

DROP TABLE IF EXISTS wt_lot;


CREATE TABLE wt_lot (
  lot_id varchar(255) NOT NULL COMMENT 'lot internal identifier',
  tracking_lot_id varchar(255) NOT NULL COMMENT 'lot identifier in the manufacturer\'s tracking system',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this lot',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this lot',
  lot_flags tinyint(2) unsigned DEFAULT NULL COMMENT 'not currently in use',
  PRIMARY KEY (lot_id,product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the lots tested at the W.T. stage';


--
-- Table structure for table wt_lot_metadata
--

DROP TABLE IF EXISTS wt_lot_metadata;

CREATE TABLE wt_lot_metadata (
  lot_id varchar(255) NOT NULL,
  product_name varchar(255) NOT NULL,
  PRIMARY KEY (lot_id, product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the lots tested at the W.T. stage';


--
-- Table structure for table wt_lot_sbin
--

DROP TABLE IF EXISTS wt_lot_sbin;


CREATE TABLE wt_lot_sbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. software binning at lot level';


--
-- Table structure for table wt_lot_hbin
--

DROP TABLE IF EXISTS wt_lot_hbin;


CREATE TABLE wt_lot_hbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. hardware binning at lot level';


--
-- Table structure for table wt_wafer_info
--

DROP TABLE IF EXISTS wt_wafer_info;


CREATE TABLE wt_wafer_info (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) NOT NULL COMMENT 'wafer internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the to the product name',
  fab_id varchar(255) DEFAULT NULL COMMENT 'wafer identifier in the manufacturer''s tracking system',
  frame_id varchar(255) DEFAULT NULL COMMENT 'additional wafer tracking identifier',
  mask_id varchar(255) DEFAULT NULL COMMENT 'identifier of the wafer mask',
  wafer_size float DEFAULT NULL COMMENT 'diameter of wafer in WF_UNITS',
  die_ht float DEFAULT NULL COMMENT 'height of a die in WF_UNITS',
  die_wid float DEFAULT NULL COMMENT 'width of a die in WF_UNITS',
  wafer_units tinyint(3) unsigned DEFAULT NULL COMMENT 'units used for WF_UNITS',
  wafer_flat char(1) DEFAULT NULL COMMENT 'orientation of the wafer flat',
  center_x smallint(5) unsigned DEFAULT NULL COMMENT 'X coordinates of center die on wafer',
  center_y smallint(5) unsigned DEFAULT NULL COMMENT 'Y coordinates of center die on wafer',
  pos_x char(1) DEFAULT NULL COMMENT 'positive X direction of the wafer',
  pos_y char(1) DEFAULT NULL COMMENT 'positive Y direction of the wafer',
  gross_die mediumint(8) NOT NULL DEFAULT '0',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this wafer',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this lot',
  wafer_flags tinyint(2) unsigned DEFAULT NULL,
  wafer_nb tinyint(3) unsigned DEFAULT NULL COMMENT 'wafer number within its lot',
  consolidation_algo varchar(45) DEFAULT NULL,
  consolidation_status char(1) DEFAULT NULL COMMENT 'consolidation status of the wafer',
  consolidation_summary TEXT DEFAULT NULL,
  consolidation_date datetime DEFAULT NULL COMMENT 'date of the consolidation',
  PRIMARY KEY (lot_id,wafer_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the wafers tested at the W.T. stage';


--
-- Table structure for table wt_wafer_sbin
--

DROP TABLE IF EXISTS wt_wafer_sbin;


CREATE TABLE wt_wafer_sbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,wafer_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. software binning at wafer level';


--
-- Table structure for table wt_wafer_sbin_inter
--

DROP TABLE IF EXISTS wt_wafer_sbin_inter;


CREATE TABLE wt_wafer_sbin_inter (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,sbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the W.T. software binning at wafer level, as specified by the consolidation tree';


--
-- Table structure for table wt_wafer_hbin
--

DROP TABLE IF EXISTS wt_wafer_hbin;


CREATE TABLE wt_wafer_hbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,wafer_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the W.T. hardware binning at wafer level';


--
-- Table structure for table wt_wafer_hbin_inter
--

DROP TABLE IF EXISTS wt_wafer_hbin_inter;


CREATE TABLE wt_wafer_hbin_inter (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  wafer_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,hbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the W.T. hardware binning at wafer level, as specified by the consolidation tree';


--
-- Table structure for table wt_wafer_consolidation
--

DROP TABLE IF EXISTS wt_wafer_consolidation;


CREATE TABLE wt_wafer_consolidation (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) DEFAULT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='wafer consolidation informations';


--
-- Table structure for table wt_wafer_consolidation_inter
--

DROP TABLE IF EXISTS wt_wafer_consolidation_inter;


CREATE TABLE wt_wafer_consolidation_inter (
  lot_id varchar(255) NOT NULL DEFAULT '',
  wafer_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) DEFAULT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='wafer intermediate consolidation informations';


--
-- Table structure for table wt_pin_map
--

DROP TABLE IF EXISTS wt_pin_map;


CREATE TABLE wt_pin_map (
  splitlot_id int(10) unsigned NOT NULL,
  tpin_pmrindex int(10) NOT NULL,
  chan_typ smallint(5) unsigned DEFAULT '0',
  chan_nam varchar(255) DEFAULT '',
  phy_nam varchar(255) DEFAULT '',
  log_nam varchar(255) DEFAULT '',
  head_num tinyint(4) unsigned DEFAULT '1',
  site_num tinyint(4) unsigned DEFAULT '1',
  PRIMARY KEY (splitlot_id,tpin_pmrindex,head_num,site_num)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table wt_splitlot
--

DROP TABLE IF EXISTS wt_splitlot;


CREATE TABLE wt_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'automatically incremented unique identifier of the splitlot. The format is AAdddXXX with AA being the last two digits of the current year, ddd the index of the day in the year when the partition was created, and xxx a counter initialised with the value 000',
  lot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the lot internal identifier',
  sublot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'there is no such wt_sublot table, therefore this column may be used or not depending on how the customer works',
  wafer_id varchar(255) NOT NULL COMMENT 'foreign key to the wafer identifier',
  product_name varchar(255) DEFAULT NULL COMMENT 'foreign key to the to the product name',
  setup_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the test setup',
  start_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the first part tested',
  finish_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the last part tested',
  stat_num tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'tester station number',
  tester_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the node that generated the data',
  tester_type varchar(255) NOT NULL DEFAULT '' COMMENT 'type of the tester that generated the data',
  splitlot_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested according to the X/Y map',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of good parts according to the X/Y map',
  nb_parts_samples mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of tested parts (from the test results)',
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of good parts (from the test results)',
  nb_parts_summary mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested according to the stdf "part count record" field',
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of good parts according to the stdf "part count record" field',
  data_provider varchar(255) DEFAULT '' COMMENT 'company which generated the data',
  data_type varchar(255) DEFAULT '' COMMENT 'defines the type of data: prod, engineering, QA, characterization...',
  prod_data char(1) NOT NULL DEFAULT 'Y' COMMENT 'Flag specifying whether the data is production data (''Y'') or non-production data (''N''). Only production data is part of the final consolidation.',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Retest index within a phase (0 for initial test, 1 for first retest, ...)',
  retest_hbins varchar(255) DEFAULT NULL COMMENT 'Retested hard bins in case of LINEAR consolidation, test/retest consolidation algorithm in case of STACKED consolidation.',
  test_insertion_index tinyint(1) unsigned DEFAULT NULL COMMENT 'Indicates the index of the insertion for multi-insertions',
  test_insertion varchar(255) DEFAULT NULL COMMENT 'Test insertion name in case of multiple insertions to generate the consolidation result',
  test_flow varchar(45) DEFAULT NULL COMMENT 'Test flow name in case of multiple flows. This can allow an additional consolidation granularity.',
  rework_code tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'describes whether the data is original (0) or has been altered by a pat process',
  job_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) name',
  job_rev varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) revision',
  oper_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job setup operator name or identifier',
  exec_typ varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer software type',
  exec_ver varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer sotfware version',
  test_cod varchar(255) NOT NULL DEFAULT '' COMMENT 'test phase or step code',
  facil_id varchar(255) NOT NULL DEFAULT '' COMMENT 'test facility identifier',
  tst_temp varchar(255) NOT NULL DEFAULT '' COMMENT 'test temperature',
  mode_cod char(1) DEFAULT NULL COMMENT 'test mode code (prod, test...)',
  rtst_cod char(1) DEFAULT NULL COMMENT 'lot retest code',
  prot_cod char(1) DEFAULT NULL COMMENT 'data protection code',
  burn_tim int(10) DEFAULT NULL COMMENT 'burn in time (in minutes)',
  cmod_cod char(1) DEFAULT NULL COMMENT 'command mode code',
  part_typ varchar(255) DEFAULT NULL COMMENT 'part type (or product identifier)',
  user_txt varchar(255) DEFAULT NULL COMMENT 'generic user text',
  aux_file varchar(255) DEFAULT NULL COMMENT 'name of auxiliary data file',
  pkg_typ varchar(255) DEFAULT NULL COMMENT 'package type',
  famly_id varchar(255) DEFAULT NULL COMMENT 'product family identifier',
  date_cod varchar(255) DEFAULT NULL COMMENT 'date code',
  floor_id varchar(255) DEFAULT NULL COMMENT 'test floor identifier',
  proc_id varchar(255) DEFAULT NULL COMMENT 'fabrication process identifier',
  oper_frq varchar(255) DEFAULT NULL COMMENT 'operation frequency of step',
  spec_nam varchar(255) DEFAULT NULL COMMENT 'test specification name',
  spec_ver varchar(255) DEFAULT NULL COMMENT 'test specification version',
  flow_id varchar(255) DEFAULT NULL COMMENT 'test flow identifier',
  setup_id varchar(255) DEFAULT NULL COMMENT 'test setup identifier',
  dsgn_rev varchar(255) DEFAULT NULL COMMENT 'device design identifier',
  eng_id varchar(255) DEFAULT NULL COMMENT 'engineering lot identifier',
  rom_cod varchar(255) DEFAULT NULL COMMENT 'ROM code identifier',
  serl_num varchar(255) DEFAULT NULL COMMENT 'tester serial number',
  supr_nam varchar(255) DEFAULT NULL COMMENT 'supervisor name or identifier',
  nb_sites tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'number of test sites found in the stdf file',
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
  file_host_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the file_host table',
  file_path varchar(255) NOT NULL DEFAULT '' COMMENT 'path of the file imported as a splitlot',
  file_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the file imported as a splitlot',
  valid_splitlot char(1) NOT NULL DEFAULT 'N' COMMENT 'whether the splitlot is valid (Y) or not (N)',
  insertion_time int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'timestamp of the splitlot insertion process',
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  incremental_update varchar(255) DEFAULT NULL COMMENT 'name of the action that must be performed on this splitlot by an incremental update. Matches the primary key or the "incremental_update" table',
  sya_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the SYA identifier',
  day varchar(10) NOT NULL COMMENT 'day of the start_t timestamp',
  week_nb tinyint(2) unsigned NOT NULL COMMENT 'week number of the start_t timestamp',
  month_nb tinyint(2) unsigned NOT NULL COMMENT 'month number for the start_t timestamp',
  quarter_nb tinyint(1) unsigned NOT NULL COMMENT 'quarter of the start_t timestamp',
  year_nb smallint(4) NOT NULL COMMENT 'year of the start_t timestamp',
  year_and_week varchar(7) NOT NULL COMMENT 'year and week of the start_t timestamp',
  year_and_month varchar(7) NOT NULL COMMENT 'year and month of the start_t timestamp',
  year_and_quarter varchar(7) NOT NULL COMMENT 'year and quarter of the start_t timestamp',
  wafer_nb tinyint(3) unsigned DEFAULT NULL COMMENT 'wafer number within its lot',
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a wt_splitlot is the single execution of a set of W.T. tests on a given number of dies. A splitlot matches with one test file.';

--
-- Table structure for table wt_splitlot_metadata
--

DROP TABLE IF EXISTS wt_splitlot_metadata;

CREATE TABLE wt_splitlot_metadata (
  splitlot_id int(10) unsigned NOT NULL,
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the splitlot tested at W.T. stage';


--
-- Table structure for table wt_sbin
--

DROP TABLE IF EXISTS wt_sbin;


CREATE TABLE wt_sbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_family varchar(255),
  bin_subfamily varchar(255),
  PRIMARY KEY (splitlot_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. software binning description';


--
-- Table structure for table wt_sbin_stats_samples
--

DROP TABLE IF EXISTS wt_sbin_stats_samples;


CREATE TABLE wt_sbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts consolidated in that bin',
  PRIMARY KEY (splitlot_id,site_no,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. software binning consolidation from the test results';


--
-- Table structure for table wt_sbin_stats_summary
--

DROP TABLE IF EXISTS wt_sbin_stats_summary;


CREATE TABLE wt_sbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts declared in that bin',
  PRIMARY KEY (splitlot_id,site_no,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. software binning stats as found in the stdf file';


--
-- Table structure for table wt_hbin
--

DROP TABLE IF EXISTS wt_hbin;


CREATE TABLE wt_hbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_family varchar(255),
  bin_subfamily varchar(255),
  PRIMARY KEY (splitlot_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. hardware binning description';


--
-- Table structure for table wt_hbin_stats_samples
--

DROP TABLE IF EXISTS wt_hbin_stats_samples;


CREATE TABLE wt_hbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts consolidated in that bin',
  PRIMARY KEY (splitlot_id,site_no,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. hardware binning consolidation from the test results';


--
-- Table structure for table wt_hbin_stats_summary
--

DROP TABLE IF EXISTS wt_hbin_stats_summary;


CREATE TABLE wt_hbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts declared in that bin',
  PRIMARY KEY (splitlot_id,site_no,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='W.T. hardware binning stats as found in the stdf file';


--
-- Table structure for table wt_parts_stats_samples
--

DROP TABLE IF EXISTS wt_parts_stats_samples;


CREATE TABLE wt_parts_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts for which test results were found',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts for which the test results were good',
  PRIMARY KEY (splitlot_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidated number of good/bad parts per splitlot and site';


--
-- Table structure for table wt_parts_stats_summary
--

DROP TABLE IF EXISTS wt_parts_stats_summary;


CREATE TABLE wt_parts_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts declared tested in the stdf file',
  nb_good mediumint(8) DEFAULT NULL COMMENT 'number of parts declared good in the stdf file',
  nb_rtst mediumint(8) DEFAULT NULL COMMENT 'number of parts declared re-tested in the stdf file',
  PRIMARY KEY (splitlot_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='number of good/bad parts as found in the stdf file';


--
-- Table structure for table wt_run
--

DROP TABLE IF EXISTS wt_run;


CREATE TABLE wt_run (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  run_id mediumint(7) unsigned NOT NULL COMMENT 'run unique identifier',
  site_no smallint(5) NOT NULL DEFAULT 1 COMMENT 'test site number',
  part_id varchar(255) DEFAULT NULL COMMENT 'part (die) unique identifier',
  part_x smallint(6) DEFAULT NULL COMMENT 'X coordinates of the part (die) on the wafer',
  part_y smallint(6) DEFAULT NULL COMMENT 'Y coordinates of the part (die) on the wafer',
  part_txt varchar(255) DEFAULT NULL COMMENT 'miscellaneous text related to that die',
  part_retest_index tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT 'index of the die test (0) or retest (1+) status',
  part_status char(1) NOT NULL DEFAULT '' COMMENT 'consolidated P (test passed) or F (test failed) status of the die',
  part_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'BIT1: part retested, BIT2: no full touchdown',
  intermediate tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_insertion',
  final tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_flow',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'hardware binning of the die',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software binning of the die',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'duration of the test on that die, in milliseconds',
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests executed on that die',
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests failed on that die',
  firstfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  firstfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  lastfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  lastfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  PRIMARY KEY (splitlot_id,run_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a wt_run is the single execution of a set of W.T. tests on a given die. Runs constitute the decomposition of a splitlot at die level.';

--
-- Table structure for table wt_ptest_info
--

DROP TABLE IF EXISTS wt_ptest_info;


CREATE TABLE wt_ptest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  units varchar(255) NOT NULL DEFAULT '' COMMENT 'units used for the test measure',
  test_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'flags describing the test',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  spec_ll float DEFAULT NULL COMMENT 'test low limit as per the test specification (engineering stage)',
  spec_hl float DEFAULT NULL COMMENT 'test high limit as per the test specification (engineering stage)',
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,ptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the parametric W.T. tests';


--
-- Table structure for table wt_ptest_limits
--

DROP TABLE IF EXISTS wt_ptest_limits;


CREATE TABLE wt_ptest_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits allowed for each site executing a given test';


--
-- Table structure for table wt_ptest_static_limits
--

DROP TABLE IF EXISTS wt_ptest_static_limits;


CREATE TABLE wt_ptest_static_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  limit_id smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'static limit identifier',
  site_no smallint(5) DEFAULT NULL COMMENT 'test site number',
  hbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'hardware bin number for that limit',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software bin number for that limit',
  ll float DEFAULT NULL COMMENT 'appliable low limit',
  hl float DEFAULT NULL COMMENT 'appliable high limit',
  PRIMARY KEY (splitlot_id,ptest_info_id,limit_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined in a DTR field in order to override the resulting bin depending on the test results';


--
-- Table structure for table wt_ptest_stats_samples
--

DROP TABLE IF EXISTS wt_ptest_stats_samples;


CREATE TABLE wt_ptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table wt_ptest_stats_summary
--

DROP TABLE IF EXISTS wt_ptest_stats_summary;


CREATE TABLE wt_ptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';


--
-- Table structure for table wt_ptest_results
--

DROP TABLE IF EXISTS wt_ptest_results;


CREATE TABLE wt_ptest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(7) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  value float DEFAULT NULL COMMENT 'result value of the test',
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table wt_mptest_info
--

DROP TABLE IF EXISTS wt_mptest_info;


CREATE TABLE wt_mptest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  tpin_arrayindex mediumint NOT NULL COMMENT 'indexes of the tester channels (1 channel for each parameter tested)',
  units varchar(255) NOT NULL DEFAULT '' COMMENT 'units used for the test measure',
  test_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'flags describing the test',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  spec_ll float DEFAULT NULL COMMENT 'test low limit as per the test specification (engineering stage)',
  spec_hl float DEFAULT NULL COMMENT 'test high limit as per the test specification (engineering stage)',
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,mptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the multi-parametric W.T. tests';


--
-- Table structure for table wt_mptest_limits
--

DROP TABLE IF EXISTS wt_mptest_limits;


CREATE TABLE wt_mptest_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  LL float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  HL float DEFAULT NULL COMMENT 'high limit for a measured die to be good',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits allowed for each site executing a given test';


--
-- Table structure for table wt_mptest_static_limits
--

DROP TABLE IF EXISTS wt_mptest_static_limits;


CREATE TABLE wt_mptest_static_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  limit_id smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'static limit identifier',
  site_no smallint(5) DEFAULT NULL COMMENT 'test site number',
  hbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'hardware bin number for that limit',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software bin number for that limit',
  ll float DEFAULT NULL COMMENT 'appliable low limit',
  hl float DEFAULT NULL COMMENT 'appliable high limit',
  PRIMARY KEY (splitlot_id,mptest_info_id,limit_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined in a DTR field in order to override the resulting bin depending on the test results';


--
-- Table structure for table wt_mptest_stats_samples
--

DROP TABLE IF EXISTS wt_mptest_stats_samples;


CREATE TABLE wt_mptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table wt_mptest_stats_summary
--

DROP TABLE IF EXISTS wt_mptest_stats_summary;


CREATE TABLE wt_mptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';


--
-- Table structure for table wt_mptest_results
--

DROP TABLE IF EXISTS wt_mptest_results;


CREATE TABLE wt_mptest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(7) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  value float DEFAULT NULL COMMENT 'result value of the test',
  tpin_pmrindex int(10) DEFAULT NULL COMMENT 'index of the tester channel used to get that value (i.e. parameter tested)',
  PRIMARY KEY (splitlot_id,mptest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table wt_ftest_info
--

DROP TABLE IF EXISTS wt_ftest_info;

CREATE TABLE wt_ftest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  PRIMARY KEY (splitlot_id,ftest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the functional W.T. tests';


--
-- Table structure for table wt_ftest_stats_samples
--

DROP TABLE IF EXISTS wt_ftest_stats_samples;


CREATE TABLE wt_ftest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  PRIMARY KEY (splitlot_id,ftest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table wt_ftest_stats_summary
--

DROP TABLE IF EXISTS wt_ftest_stats_summary;


CREATE TABLE wt_ftest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,ftest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';


--
-- Table structure for table wt_ftest_results
--

DROP TABLE IF EXISTS wt_ftest_results;


CREATE TABLE wt_ftest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(7) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  vect_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'vector module pattern name',
  vect_off smallint(6) DEFAULT NULL COMMENT 'integer offset of the vector from the vector of interest',
  PRIMARY KEY (splitlot_id,ftest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table wt_test_conditions (TDR)
--

DROP TABLE IF EXISTS wt_test_conditions;


CREATE TABLE wt_test_conditions(
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  test_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  test_type char(1) NOT NULL COMMENT 'indicates whether the test description identifier must be looked amongst the parametric, multiparametric or functional test descrpitions',

  condition_1 varchar(255) DEFAULT NULL COMMENT 'description of the first test condition',
  condition_2 varchar(255) DEFAULT NULL COMMENT 'description of the second test condition',
  condition_3 varchar(255) DEFAULT NULL COMMENT 'description of the third test condition',
  condition_4 varchar(255) DEFAULT NULL COMMENT 'and so on...',
  condition_5 varchar(255) DEFAULT NULL,
  condition_6 varchar(255) DEFAULT NULL,
  condition_7 varchar(255) DEFAULT NULL,
  condition_8 varchar(255) DEFAULT NULL,
  condition_9 varchar(255) DEFAULT NULL,

  condition_10 varchar(255) DEFAULT NULL,
  condition_11 varchar(255) DEFAULT NULL,
  condition_12 varchar(255) DEFAULT NULL,
  condition_13 varchar(255) DEFAULT NULL,
  condition_14 varchar(255) DEFAULT NULL,
  condition_15 varchar(255) DEFAULT NULL,
  condition_16 varchar(255) DEFAULT NULL,
  condition_17 varchar(255) DEFAULT NULL,
  condition_18 varchar(255) DEFAULT NULL,
  condition_19 varchar(255) DEFAULT NULL,

  condition_20 varchar(255) DEFAULT NULL,
  condition_21 varchar(255) DEFAULT NULL,
  condition_22 varchar(255) DEFAULT NULL,
  condition_23 varchar(255) DEFAULT NULL,
  condition_24 varchar(255) DEFAULT NULL,
  condition_25 varchar(255) DEFAULT NULL,
  condition_26 varchar(255) DEFAULT NULL,
  condition_27 varchar(255) DEFAULT NULL,
  condition_28 varchar(255) DEFAULT NULL,
  condition_29 varchar(255) DEFAULT NULL,

  condition_30 varchar(255) DEFAULT NULL,
  condition_31 varchar(255) DEFAULT NULL,
  condition_32 varchar(255) DEFAULT NULL,
  condition_33 varchar(255) DEFAULT NULL,
  condition_34 varchar(255) DEFAULT NULL,
  condition_35 varchar(255) DEFAULT NULL,
  condition_36 varchar(255) DEFAULT NULL,
  condition_37 varchar(255) DEFAULT NULL,
  condition_38 varchar(255) DEFAULT NULL,
  condition_39 varchar(255) DEFAULT NULL,

  condition_40 varchar(255) DEFAULT NULL,
  condition_41 varchar(255) DEFAULT NULL,
  condition_42 varchar(255) DEFAULT NULL,
  condition_43 varchar(255) DEFAULT NULL,
  condition_44 varchar(255) DEFAULT NULL,
  condition_45 varchar(255) DEFAULT NULL,
  condition_46 varchar(255) DEFAULT NULL,
  condition_47 varchar(255) DEFAULT NULL,
  condition_48 varchar(255) DEFAULT NULL,
  condition_49 varchar(255) DEFAULT NULL,

  condition_50 varchar(255) DEFAULT NULL,
  condition_51 varchar(255) DEFAULT NULL,
  condition_52 varchar(255) DEFAULT NULL,
  condition_53 varchar(255) DEFAULT NULL,
  condition_54 varchar(255) DEFAULT NULL,
  condition_55 varchar(255) DEFAULT NULL,
  condition_56 varchar(255) DEFAULT NULL,
  condition_57 varchar(255) DEFAULT NULL,
  condition_58 varchar(255) DEFAULT NULL,
  condition_59 varchar(255) DEFAULT NULL,

  condition_60 varchar(255) DEFAULT NULL,
  condition_61 varchar(255) DEFAULT NULL,
  condition_62 varchar(255) DEFAULT NULL,
  condition_63 varchar(255) DEFAULT NULL,
  condition_64 varchar(255) DEFAULT NULL,
  condition_65 varchar(255) DEFAULT NULL,
  condition_66 varchar(255) DEFAULT NULL,
  condition_67 varchar(255) DEFAULT NULL,
  condition_68 varchar(255) DEFAULT NULL,
  condition_69 varchar(255) DEFAULT NULL,

  condition_70 varchar(255) DEFAULT NULL,
  condition_71 varchar(255) DEFAULT NULL,
  condition_72 varchar(255) DEFAULT NULL,
  condition_73 varchar(255) DEFAULT NULL,
  condition_74 varchar(255) DEFAULT NULL,
  condition_75 varchar(255) DEFAULT NULL,
  condition_76 varchar(255) DEFAULT NULL,
  condition_77 varchar(255) DEFAULT NULL,
  condition_78 varchar(255) DEFAULT NULL,
  condition_79 varchar(255) DEFAULT NULL,

  condition_80 varchar(255) DEFAULT NULL,
  condition_81 varchar(255) DEFAULT NULL,
  condition_82 varchar(255) DEFAULT NULL,
  condition_83 varchar(255) DEFAULT NULL,
  condition_84 varchar(255) DEFAULT NULL,
  condition_85 varchar(255) DEFAULT NULL,
  condition_86 varchar(255) DEFAULT NULL,
  condition_87 varchar(255) DEFAULT NULL,
  condition_88 varchar(255) DEFAULT NULL,
  condition_89 varchar(255) DEFAULT NULL,

  condition_90 varchar(255) DEFAULT NULL,
  condition_91 varchar(255) DEFAULT NULL,
  condition_92 varchar(255) DEFAULT NULL,
  condition_93 varchar(255) DEFAULT NULL,
  condition_94 varchar(255) DEFAULT NULL,
  condition_95 varchar(255) DEFAULT NULL,
  condition_96 varchar(255) DEFAULT NULL,
  condition_97 varchar(255) DEFAULT NULL,
  condition_98 varchar(255) DEFAULT NULL,
  condition_99 varchar(255) DEFAULT NULL,

  condition_100 varchar(255) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,test_info_id,test_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='defines the test condition during the characterisation stage';


--
-- Table structure for table wt_metadata_mapping
--

DROP TABLE IF EXISTS wt_metadata_mapping;


CREATE TABLE wt_metadata_mapping (
  meta_name varchar(255) NOT NULL COMMENT 'name of the additional field',
  gex_name varchar(255) DEFAULT NULL COMMENT 'name of the mapped field in gex',
  gexdb_table_name varchar(255) NOT NULL COMMENT 'name of the gex database table to which the additional field is mapped',
  gexdb_field_fullname varchar(255) NOT NULL COMMENT 'name of the column to which the additional field is mapped',
  gexdb_link_name varchar(255) DEFAULT NULL COMMENT 'foreign key to the metadata link name',
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether the field must be displayed in the gex GUI (Y) or not (N)',
  bintype_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a binary type logic (Y) or not (N)',
  time_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a time type logic (Y) or not (N)',
  custom_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be treated with a custom type logic (Y) or not (N)',
  numeric_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a numeric type logic (Y) or not (N)',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be processed for consolidation',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be processed for A-Z consolidation',
  PRIMARY KEY (meta_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='describes the additional fields usable for data filtering or consolidation';


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
  gexdb_link_name varchar(255) DEFAULT NULL,
  PRIMARY KEY (link_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table wt_sya_set
--

DROP TABLE IF EXISTS wt_sya_set;


CREATE TABLE wt_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'SYA unique identifier',
  product_id varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  creation_date datetime NOT NULL COMMENT 'creation date of the SYA set',
  user_comment varchar(255) DEFAULT NULL COMMENT 'user defined comment',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SYA alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SYA alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SYA alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SYA alarm to trigger',
  start_date datetime NOT NULL COMMENT 'date from which the SYA alarm is valid',
  expiration_date date NOT NULL COMMENT 'date to which the SYA alarm is not valid anymore',
  expiration_email_date datetime DEFAULT NULL COMMENT 'date to which an expiration warning email must be sent',
  rule_type varchar(255) NOT NULL COMMENT 'SYA rule type',
  n1_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  computation_fromdate date NOT NULL COMMENT 'initial date from which the limits should be computed',
  computation_todate date NOT NULL COMMENT 'final date to which the limits should be computed',
  min_lots_required smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of lots for the computed limits to be valid',
  min_data_points smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of calculated yields for the computed limits to be valid',
  options tinyint(3) unsigned NOT NULL DEFAULT '0',
  flags tinyint(3) unsigned NOT NULL DEFAULT '0',
  rule_name varchar(255) DEFAULT NULL COMMENT 'name of the SYA rule',
  bin_type tinyint(1) DEFAULT NULL COMMENT 'whether the alarm should be triggered on a good (P) bin or a bad (F) bin',
  PRIMARY KEY (sya_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SYA rules appliable for the statistical yield analysis alarms';


--
-- Table structure for table wt_sbl
--

DROP TABLE IF EXISTS wt_sbl;


CREATE TABLE wt_sbl (
  sya_id int(10) unsigned NOT NULL COMMENT 'foreign key to the associated SYA rule',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'bin number on which the SBL must be applied',
  bin_name varchar(255) DEFAULT NULL COMMENT 'name of the bin on which the SBL must be applied',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SBL alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SBL alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SBL alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SBL alarm to trigger',
  rule_type varchar(255) DEFAULT NULL COMMENT 'SBL rule type',
  n1_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SBL rules appliable along with SYA alarms';


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
  display char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY (site_name,column_id,data_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports formats';


--
-- Table structure for table wt_wyr
--

DROP TABLE IF EXISTS wt_wyr;


CREATE TABLE wt_wyr (
  wyr_id INT NOT NULL AUTO_INCREMENT,
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  year smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT '0',
  parts_received int(10) DEFAULT '0',
  pretest_rejects int(10) DEFAULT '0',
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT '0',
  parts_pass int(10) DEFAULT '0',
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT '0',
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT '0',
  parts_retest_split varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT '0',
  posttest_rejects int(10) DEFAULT '0',
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT '0',
  PRIMARY KEY (wyr_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports';


--
-- Table structure for table wt_dtr
--

DROP TABLE IF EXISTS wt_dtr;


CREATE TABLE wt_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT 0,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (splitlot_id,run_id,order_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='html summaries of PAT reports';

--
-- Table structure for table wt_sdr
--

DROP TABLE IF EXISTS wt_sdr;

CREATE TABLE wt_sdr (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_grp smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site group number',
  site_index smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site index',
  site_no smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site number',
  hand_typ varchar(255) DEFAULT NULL COMMENT 'handler of prober type',
  hand_id varchar(255) DEFAULT NULL COMMENT 'handler of prober identifier',
  card_typ varchar(255) DEFAULT NULL COMMENT 'prober card type',
  card_id varchar(255) DEFAULT NULL COMMENT 'prober card identifier',
  load_typ varchar(255) DEFAULT NULL COMMENT 'load board type',
  load_id varchar(255) DEFAULT NULL COMMENT 'load board identifier',
  dib_typ varchar(255) DEFAULT NULL COMMENT 'DIB board type',
  dib_id varchar(255) DEFAULT NULL COMMENT 'DIB board identifier',
  cabl_typ varchar(255) DEFAULT NULL COMMENT 'interface cable type',
  cabl_id varchar(255) DEFAULT NULL COMMENT 'interface cable identifier',
  cont_typ varchar(255) DEFAULT NULL COMMENT 'handler contractor type',
  cont_id varchar(255) DEFAULT NULL COMMENT 'handler contractor identifier',
  lasr_typ varchar(255) DEFAULT NULL COMMENT 'laser type',
  lasr_id varchar(255) DEFAULT NULL COMMENT 'laser identifier',
  extr_typ varchar(255) DEFAULT NULL COMMENT 'extra equipment type',
  extr_id varchar(255) DEFAULT NULL COMMENT 'extra equipment identifier',
  PRIMARY KEY (splitlot_id, site_grp, site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the sites used for testing';


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
  alarm_flags tinyint(2) unsigned NOT NULL,
  lcl float NOT NULL DEFAULT '0',
  ucl float NOT NULL DEFAULT '0',
  value float NOT NULL DEFAULT '0',
  units varchar(10) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,alarm_cat,alarm_type,item_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='logs the alarms raised by the datapump tasks';




--
-- Table structure for table ft_product_sbin
--

DROP TABLE IF EXISTS ft_product_sbin;


CREATE TABLE ft_product_sbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. software binning at product level';


--
-- Table structure for table ft_product_hbin
--

DROP TABLE IF EXISTS ft_product_hbin;


CREATE TABLE ft_product_hbin (
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  bin_name varchar(255) DEFAULT NULL COMMENT 'verbose name of the bin being consolidated',
  bin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts bigint(20) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (product_name,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. hardware binning at product level';


--
-- Table structure for table ft_lot
--

DROP TABLE IF EXISTS ft_lot;


CREATE TABLE ft_lot (
  lot_id varchar(255) NOT NULL COMMENT 'lot internal identifier',
  tracking_lot_id varchar(255) NOT NULL COMMENT 'lot identifier in the manufacturer\'s tracking system',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this lot',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this lot',
  lot_flags tinyint(2) unsigned DEFAULT NULL COMMENT 'not currently in use',
  PRIMARY KEY (lot_id,product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the lots tested at the F.T. stage';

--
-- Table structure for table ft_lot_metadata
--

DROP TABLE IF EXISTS ft_lot_metadata;

CREATE TABLE ft_lot_metadata (
  lot_id varchar(255) NOT NULL,
  product_name varchar(255) NOT NULL,
  PRIMARY KEY (lot_id, product_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the lots tested at the F.T. stage';


--
-- Table structure for table ft_lot_sbin
--

DROP TABLE IF EXISTS ft_lot_sbin;


CREATE TABLE ft_lot_sbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. software binning at lot level';


--
-- Table structure for table ft_lot_hbin
--

DROP TABLE IF EXISTS ft_lot_hbin;


CREATE TABLE ft_lot_hbin (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  PRIMARY KEY (lot_id,product_name,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. hardware binning at lot level';


--
-- Table structure for table ft_sublot_info
--
DROP TABLE IF EXISTS ft_sublot_info;
CREATE TABLE ft_sublot_info (
  lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
  sublot_id varchar(255) NOT NULL COMMENT 'sublot internal identifier',
  product_name varchar(255) NOT NULL COMMENT 'foreign key to the to the product name',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts processed for this sublot',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts identified as good for this sublot',
  sublot_flags tinyint(2) unsigned DEFAULT NULL,
  consolidation_algo varchar(45) DEFAULT NULL,
  consolidation_status char(1) DEFAULT NULL COMMENT 'consolidation status of the sublot',
  consolidation_summary TEXT DEFAULT NULL,
  consolidation_date datetime DEFAULT NULL COMMENT 'date of the consolidation',
  PRIMARY KEY (lot_id,sublot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the sublots tested at the F.T. stage';


--
-- Table structure for table ft_sublot_sbin
--
DROP TABLE IF EXISTS ft_sublot_sbin;
CREATE TABLE ft_sublot_sbin (
 lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
 sublot_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the sublot internal identifier',
 sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
 sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
 sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
 PRIMARY KEY (lot_id,sublot_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. software binning at sublot level';


--
-- Table structure for table ft_sublot_sbin_inter
--
DROP TABLE IF EXISTS ft_sublot_sbin_inter;
CREATE TABLE ft_sublot_sbin_inter (
 lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
 sublot_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
 sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
 sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
 sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
 PRIMARY KEY (lot_id,sublot_id,sbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the F.T. software binning at sublot level, as specified by the consolidation tree';


--
-- Table structure for table ft_sublot_hbin
--
DROP TABLE IF EXISTS ft_sublot_hbin;
CREATE TABLE ft_sublot_hbin (
 lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
 sublot_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the sublot internal identifier',
 hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
 hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
 hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
 PRIMARY KEY (lot_id,sublot_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='consolidation of the F.T. hardware binning at sublot level';


--
-- Table structure for table ft_sublot_hbin_inter
--
DROP TABLE IF EXISTS ft_sublot_hbin_inter;
CREATE TABLE ft_sublot_hbin_inter (
 lot_id varchar(255) NOT NULL COMMENT 'foreign key to the lot internal identifier',
 sublot_id varchar(255) DEFAULT NULL COMMENT 'foreign key to the wafer internal identifier',
 hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
 hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
 hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'consolidated number of parts in that bin',
 consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
 consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
 PRIMARY KEY (lot_id,sublot_id,hbin_no,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='intermediate consolidation of the F.T. hardware binning at sublot level, as specified by the consolidation tree';


--
-- Table structure for table ft_sublot_consolidation
--
DROP TABLE IF EXISTS ft_sublot_consolidation;
CREATE TABLE ft_sublot_consolidation (
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) NOT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,sublot_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='sublot consolidation informations';


--
-- Table structure for table ft_sublot_consolidation_inter
--
DROP TABLE IF EXISTS ft_sublot_consolidation_inter;
CREATE TABLE ft_sublot_consolidation_inter (
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  consolidated_data_type varchar(45) NOT NULL,
  consolidation_name varchar(255) NOT NULL COMMENT 'name of the consolidation in progress',
  consolidation_flow varchar(45) NOT NULL COMMENT 'name of the consolidation flow in progress',
  PRIMARY KEY (lot_id,sublot_id,consolidation_name,consolidation_flow)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='wafer intermediate consolidation informations';

--
-- Table structure for table ft_pin_map
--

DROP TABLE IF EXISTS ft_pin_map;


CREATE TABLE ft_pin_map (
  splitlot_id int(10) unsigned NOT NULL,
  tpin_pmrindex int(10) NOT NULL,
  chan_typ smallint(5) unsigned DEFAULT '0',
  chan_nam varchar(255) DEFAULT '',
  phy_nam varchar(255) DEFAULT '',
  log_nam varchar(255) DEFAULT '',
  head_num tinyint(4) unsigned DEFAULT '1',
  site_num tinyint(4) unsigned DEFAULT '1',
  PRIMARY KEY (splitlot_id,tpin_pmrindex,head_num,site_num)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_splitlot
--

DROP TABLE IF EXISTS ft_splitlot;
CREATE TABLE ft_splitlot (
  splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'automatically incremented unique identifier of the splitlot. The format is AAdddXXX with AA being the last two digits of the current year, ddd the index of the day in the year when the partition was created, and xxx a counter initialised with the value 000',
  lot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the lot internal identifier',
  sublot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the sublot internal identifier',
  product_name varchar(255) DEFAULT NULL,
  setup_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the test setup',
  start_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the first part tested',
  finish_t int(10) NOT NULL DEFAULT '0' COMMENT 'timestamp of the last part tested',
  stat_num tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'tester station number',
  tester_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the node that generated the data',
  tester_type varchar(255) NOT NULL DEFAULT '' COMMENT 'type of the tester that generated the data',
  splitlot_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of good parts',
  nb_parts_samples mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of tested parts (from the test results)',
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'consolidated number of good parts (from the test results)',
  nb_parts_summary mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested according to the stdf "part count record" field',
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of good parts according to the stdf "part count record" field',
  data_provider varchar(255) DEFAULT '' COMMENT 'company which generated the data',
  data_type varchar(255) DEFAULT '' COMMENT 'defines the type of data: prod, engineering, QA, characterization...',
  prod_data char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether the data type is production (Y) or not (N)',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Retest index within a phase (0 for initial test, 1 for first retest, ...)',
  retest_hbins varchar(255) DEFAULT NULL COMMENT 'Retested hard bins in case of LINEAR consolidation, test/retest consolidation algorithm in case of STACKED consolidation.',
  test_insertion_index tinyint(1) unsigned DEFAULT NULL COMMENT 'Indicates the index of the insertion for multi-insertions',
  test_insertion varchar(255) DEFAULT NULL COMMENT 'Test insertion name in case of multiple insertions to generate the consolidation result',
  test_flow varchar(45) DEFAULT NULL COMMENT 'Test flow name in case of multiple flows. This can allow an additional consolidation granularity.',
  rework_code tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'describes whether the data is original (0) or has been altered by a pat process',
  job_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) name',
  job_rev varchar(255) NOT NULL DEFAULT '' COMMENT 'job (test program) revision',
  oper_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'job setup operator name or identifier',
  exec_typ varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer software type',
  exec_ver varchar(255) NOT NULL DEFAULT '' COMMENT 'tester executer sotfware version',
  test_cod varchar(255) NOT NULL DEFAULT '' COMMENT 'test phase or step code',
  facil_id varchar(255) NOT NULL DEFAULT '' COMMENT 'test facility identifier',
  tst_temp varchar(255) NOT NULL DEFAULT '' COMMENT 'test temperature',
  mode_cod char(1) DEFAULT NULL COMMENT 'test mode code (prod, test...)',
  rtst_cod char(1) DEFAULT NULL COMMENT 'lot retest code',
  prot_cod char(1) DEFAULT NULL COMMENT 'data protection code',
  burn_tim int(10) DEFAULT NULL COMMENT 'burn in time (in minutes)',
  cmod_cod char(1) DEFAULT NULL COMMENT 'command mode code',
  part_typ varchar(255) DEFAULT NULL COMMENT 'part type (or product identifier)',
  user_txt varchar(255) DEFAULT NULL COMMENT 'generic user text',
  aux_file varchar(255) DEFAULT NULL COMMENT 'name of auxiliary data file',
  pkg_typ varchar(255) DEFAULT NULL COMMENT 'package type',
  famly_id varchar(255) DEFAULT NULL COMMENT 'product family identifier',
  date_cod varchar(255) DEFAULT NULL COMMENT 'date code',
  floor_id varchar(255) DEFAULT NULL COMMENT 'test floor identifier',
  proc_id varchar(255) DEFAULT NULL COMMENT 'fabrication process identifier',
  oper_frq varchar(255) DEFAULT NULL COMMENT 'operation frequency of step',
  spec_nam varchar(255) DEFAULT NULL COMMENT 'test specification name',
  spec_ver varchar(255) DEFAULT NULL COMMENT 'test specification version',
  flow_id varchar(255) DEFAULT NULL COMMENT 'test flow identifier',
  setup_id varchar(255) DEFAULT NULL COMMENT 'test setup identifier',
  dsgn_rev varchar(255) DEFAULT NULL COMMENT 'device design identifier',
  eng_id varchar(255) DEFAULT NULL COMMENT 'engineering lot identifier',
  rom_cod varchar(255) DEFAULT NULL COMMENT 'ROM code identifier',
  serl_num varchar(255) DEFAULT NULL COMMENT 'tester serial number',
  supr_nam varchar(255) DEFAULT NULL COMMENT 'supervisor name or identifier',
  nb_sites tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'number of test sites found in the stdf file',
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
  file_host_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the file_host table',
  file_path varchar(255) NOT NULL DEFAULT '' COMMENT 'path of the file imported as a splitlot',
  file_name varchar(255) NOT NULL DEFAULT '' COMMENT 'name of the file imported as a splitlot',
  valid_splitlot char(1) NOT NULL DEFAULT 'N' COMMENT 'whether the splitlot is valid (Y) or not (N)',
  insertion_time int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'timestamp of the splitlot insertion process',
  subcon_lot_id varchar(255) NOT NULL DEFAULT '' COMMENT 'foreign key to the sublot identifier',
  incremental_update varchar(255) DEFAULT NULL COMMENT 'name of the action that must be performed on this splitlot by an incremental update. Matches the primary key or the "incremental_update" table',
  sya_id int(10) unsigned DEFAULT '0' COMMENT 'foreign key to the SYA identifier',
  day varchar(10) NOT NULL COMMENT 'day of the start_t timestamp',
  week_nb tinyint(2) unsigned NOT NULL COMMENT 'week number of the start_t timestamp',
  month_nb tinyint(2) unsigned NOT NULL COMMENT 'month number for the start_t timestamp',
  quarter_nb tinyint(1) unsigned NOT NULL COMMENT 'quarter of the start_t timestamp',
  year_nb smallint(4) NOT NULL COMMENT 'year of the start_t timestamp',
  year_and_week varchar(7) NOT NULL COMMENT 'year and week of the start_t timestamp',
  year_and_month varchar(7) NOT NULL COMMENT 'year and month of the start_t timestamp',
  year_and_quarter varchar(7) NOT NULL COMMENT 'year and quarter of the start_t timestamp',
  recipe_id int(10) unsigned DEFAULT NULL COMMENT 'foreign key to the PAT recipe identifier',
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a ft_splitlot is the single execution of a set of F.T. tests on a given number of dies. A splitlot matches with one test file.';


--
-- Table structure for table ft_splitlot_metadata
--

DROP TABLE IF EXISTS ft_splitlot_metadata;

CREATE TABLE ft_splitlot_metadata (
  splitlot_id int(10) unsigned NOT NULL,
  PRIMARY KEY (splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='additional informations on the splitlot tested at Fs.T. stage';


--
-- Table structure for table ft_sbin
--

DROP TABLE IF EXISTS ft_sbin;


CREATE TABLE ft_sbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  sbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  sbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_family varchar (255),
  bin_subfamily varchar (255),
  PRIMARY KEY (splitlot_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. software binning description';


--
-- Table structure for table ft_sbin_stats_samples
--

DROP TABLE IF EXISTS ft_sbin_stats_samples;


CREATE TABLE ft_sbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts consolidated in that bin',
  PRIMARY KEY (splitlot_id,site_no,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. software binning consolidation from the test results';


--
-- Table structure for table ft_sbin_stats_summary
--

DROP TABLE IF EXISTS ft_sbin_stats_summary;


CREATE TABLE ft_sbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  sbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts declared in that bin',
  PRIMARY KEY (splitlot_id,site_no,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. software binning stats as found in the stdf file';


--
-- Table structure for table ft_hbin
--

DROP TABLE IF EXISTS ft_hbin;


CREATE TABLE ft_hbin (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'number (identifier) of the bin being consolidated',
  hbin_name varchar(255) NOT NULL DEFAULT '' COMMENT 'verbose name of the bin being consolidated',
  hbin_cat char(1) DEFAULT NULL COMMENT 'P (test passed) or F (test failed) category associated with the bin',
  bin_family varchar(255),
  bin_subfamily varchar(255),
  PRIMARY KEY (splitlot_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. hardware binning description';


--
-- Table structure for table ft_hbin_stats_samples
--

DROP TABLE IF EXISTS ft_hbin_stats_samples;


CREATE TABLE ft_hbin_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts consolidated in that bin',
  PRIMARY KEY (splitlot_id,site_no,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. hardware binning consolidation from the test results';


--
-- Table structure for table ft_hbin_stats_summary
--

DROP TABLE IF EXISTS ft_hbin_stats_summary;


CREATE TABLE ft_hbin_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'foreign key to the binning number',
  bin_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts declared in that bin',
  PRIMARY KEY (splitlot_id,site_no,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='F.T. hardware binning stats as found in the stdf file';


--
-- Table structure for table ft_parts_stats_samples
--

DROP TABLE IF EXISTS ft_parts_stats_samples;


CREATE TABLE ft_parts_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tester',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts good',
  PRIMARY KEY (splitlot_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='parts statistics consolidated from the test results';


--
-- Table structure for table ft_parts_stats_summary
--

DROP TABLE IF EXISTS ft_parts_stats_summary;


CREATE TABLE ft_parts_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  nb_parts mediumint(8) NOT NULL DEFAULT '0' COMMENT 'number of parts tested',
  nb_good mediumint(8) DEFAULT NULL COMMENT 'number of parts good',
  nb_rtst mediumint(8) DEFAULT NULL COMMENT 'number of parts declared re-tested in the stdf file',
  PRIMARY KEY (splitlot_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='parts statistics as found in the stdf file';


--
-- Table structure for table ft_die_tracking
--

DROP TABLE IF EXISTS ft_die_tracking;


CREATE TABLE ft_die_tracking (
  ft_tracking_lot_id varchar(255) NOT NULL COMMENT 'F.T. external lot identifier',
  wt_product_id varchar(255) NOT NULL COMMENT 'W.T. product identifier',
  wt_tracking_lot_id varchar(255) NOT NULL COMMENT 'W.T. external lot identifier',
  wt_sublot_id varchar(255) NOT NULL COMMENT 'W.T. sublot identifier',
  die_id varchar(255) NOT NULL COMMENT 'part identifier',
  PRIMARY KEY (ft_tracking_lot_id,wt_tracking_lot_id,die_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='allows tracking a lot issued from the W.T. stage into the F.T. stage';


--
-- Table structure for table ft_dietrace_config
--

DROP TABLE IF EXISTS ft_dietrace_config;


CREATE TABLE ft_dietrace_config (
  splitlot_id int(10) unsigned NOT NULL,
  die_config_id smallint(5) unsigned NOT NULL DEFAULT '0',
  die_index smallint(5) NOT NULL DEFAULT '1',
  product varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  wafer_id varchar(255) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,die_config_id,die_index)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_run
--

DROP TABLE IF EXISTS ft_run;


CREATE TABLE ft_run (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'run unique identifier',
  site_no smallint(5) NOT NULL DEFAULT 1 COMMENT 'test site number',
  part_id varchar(255) DEFAULT NULL COMMENT 'part (die) unique identifier',
  part_x smallint(6) DEFAULT NULL COMMENT 'X coordinates of the part (die) on the wafer',
  part_y smallint(6) DEFAULT NULL COMMENT 'Y coordinates of the part (die) on the wafer',
  part_txt varchar(255) DEFAULT NULL COMMENT 'miscellaneous text related to that die',
  part_retest_index tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT 'index of the die test (0) or retest (1+) status',
  part_status char(1) NOT NULL DEFAULT '' COMMENT 'consolidated P (test passed) or F (test failed) status of the die',
  part_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'BIT1: part retested, BIT2: no full touchdown',
  intermediate tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_insertion',
  final tinyint(1) unsigned NULL DEFAULT NULL COMMENT 'BOOLEAN: identification of  the die for the consolidated test_flow',
  hbin_no smallint(5) unsigned NOT NULL COMMENT 'hardware binning of the die',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software binning of the die',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'duration of the test on that die, in milliseconds',
  tests_executed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests executed on that die',
  tests_failed smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'consolidated number of tests failed on that die',
  firstfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  firstfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the first test failed on that die',
  lastfail_test_type char(1) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  lastfail_test_id smallint(5) DEFAULT NULL COMMENT 'identification of the last test failed on that die',
  wafer_id varchar(255) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,run_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='a ft_run is the single execution of a set of F.T. tests on a given die. Runs constitute the decomposition of a splitlot at die level.';

--
-- Table structure for table ft_run_dietrace
--

DROP TABLE IF EXISTS ft_run_dietrace;


CREATE TABLE ft_run_dietrace (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL DEFAULT '0',
  die_config_id smallint(5) NOT NULL DEFAULT '1',
  part_id varchar(255) NOT NULL DEFAULT '',
  part_x smallint(6) DEFAULT NULL,
  part_y smallint(6) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,run_id,die_config_id,part_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_ptest_info
--

DROP TABLE IF EXISTS ft_ptest_info;


CREATE TABLE ft_ptest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  units varchar(255) NOT NULL DEFAULT '' COMMENT 'units used for the test measure',
  test_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'flags describing the test',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  spec_ll float DEFAULT NULL COMMENT 'test low limit as per the test specification (engineering stage)',
  spec_hl float DEFAULT NULL COMMENT 'test high limit as per the test specification (engineering stage)',
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,ptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the parametric F.T. tests';


--
-- Table structure for table ft_ptest_limits
--

DROP TABLE IF EXISTS ft_ptest_limits;


CREATE TABLE ft_ptest_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  LL float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  HL float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits allowed for each site executing a given test';


--
-- Table structure for table ft_ptest_static_limits
--

DROP TABLE IF EXISTS ft_ptest_static_limits;


CREATE TABLE ft_ptest_static_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  limit_id smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'static limit identifier',
  site_no smallint(5) DEFAULT NULL COMMENT 'test site number',
  hbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'hardware bin number for that limit',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software bin number for that limit',
  ll float DEFAULT NULL COMMENT 'appliable low limit',
  hl float DEFAULT NULL COMMENT 'appliable high limit',
  PRIMARY KEY (splitlot_id,ptest_info_id,limit_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined in a DTR field in order to override the resulting bin depending on the test results';


--
-- Table structure for table ft_ptest_rollinglimits
--

DROP TABLE IF EXISTS ft_ptest_rollinglimits;

CREATE TABLE ft_ptest_rollinglimits
(
  splitlot_id int(10) unsigned NOT NULL,
  ptest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL,
  limit_index int(10) unsigned NOT NULL,
  limit_type char(1) NOT NULL,
  limit_mode int(10) unsigned NOT NULL,
  LL float DEFAULT NULL,
  HL float DEFAULT NULL,
  gtl_splitlot_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id,limit_index)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined during the PAT processing';

--
-- Table structure for table ft_ptest_rollingstats
--

DROP TABLE IF EXISTS ft_ptest_rollingstats;

CREATE TABLE ft_ptest_rollingstats (
  splitlot_id int(10) unsigned NOT NULL,
  ptest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL,
  distribution_shape varchar (255),
  n_factor float,
  t_factor float,
  mean float,
  sigma float,
  min float,
  q1 float,
  median float,
  q3 float,
  max float,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0',
  fail_count mediumint(8) unsigned NOT NULL DEFAULT '0',
  gtl_splitlot_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_ptest_stats_samples
--

DROP TABLE IF EXISTS ft_ptest_stats_samples;


CREATE TABLE ft_ptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table ft_ptest_stats_summary
--

DROP TABLE IF EXISTS ft_ptest_stats_summary;


CREATE TABLE ft_ptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,ptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';


--
-- Table structure for table ft_ptest_results
--

DROP TABLE IF EXISTS ft_ptest_results;


CREATE TABLE ft_ptest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  value float DEFAULT NULL COMMENT 'result value of the test',
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table ft_ptest_outliers
--

DROP TABLE IF EXISTS ft_ptest_outliers;

CREATE TABLE ft_ptest_outliers
(
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  run_index int(10) unsigned NOT NULL,
  limits_run_id mediumint(8) unsigned NOT NULL,
  limit_type char(1) NOT NULL,
  value float COMMENT 'result value of the test',
  gtl_splitlot_id int(10) unsigned DEFAULT NULL COMMENT 'splitlot identifier retrieved from the PAT test',
  PRIMARY KEY (splitlot_id,ptest_info_id,run_id,run_index)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='PAT processed outliers';


--
-- Table structure for table ft_mptest_info
--

DROP TABLE IF EXISTS ft_mptest_info;


CREATE TABLE ft_mptest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  tpin_arrayindex mediumint NOT NULL COMMENT 'indexes of the tester channels (1 channel for each parameter tested)',
  units varchar(255) NOT NULL DEFAULT '' COMMENT 'units used for the test measure',
  test_flags tinyint(2) unsigned NOT NULL DEFAULT 0 COMMENT 'flags describing the test',
  ll float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  hl float DEFAULT NULL COMMENT 'high limit for a measured dit to be good',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  spec_ll float DEFAULT NULL COMMENT 'test low limit as per the test specification (engineering stage)',
  spec_hl float DEFAULT NULL COMMENT 'test high limit as per the test specification (engineering stage)',
  spec_target float DEFAULT NULL,
  res_scal tinyint(3) DEFAULT NULL,
  ll_scal tinyint(3) DEFAULT NULL,
  hl_scal tinyint(3) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,mptest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the multiparametric F.T. tests';


--
-- Table structure for table ft_mptest_limits
--

DROP TABLE IF EXISTS ft_mptest_limits;


CREATE TABLE ft_mptest_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  LL float DEFAULT NULL COMMENT 'low limit for a measured die to be good',
  HL float DEFAULT NULL COMMENT 'high limit for a measured die to be good',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits allowed for each site executing a given test';


--
-- Table structure for table ft_mptest_static_limits
--

DROP TABLE IF EXISTS ft_mptest_static_limits;


CREATE TABLE ft_mptest_static_limits (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  limit_id smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'static limit identifier',
  site_no smallint(5) DEFAULT NULL COMMENT 'test site number',
  hbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'hardware bin number for that limit',
  sbin_no smallint(5) unsigned DEFAULT NULL COMMENT 'software bin number for that limit',
  ll float DEFAULT NULL COMMENT 'appliable low limit',
  hl float DEFAULT NULL COMMENT 'appliable high limit',
  PRIMARY KEY (splitlot_id,mptest_info_id,limit_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined in a DTR field in order to override the resulting bin depending on the test results';


--
-- Table structure for table ft_mptest_rollinglimits
--

DROP TABLE IF EXISTS ft_mptest_rollinglimits;

CREATE TABLE ft_mptest_rollinglimits
(
  splitlot_id int(10) unsigned NOT NULL,
  mptest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL,
  limit_index int(10) unsigned NOT NULL,
  limit_type char(1) NOT NULL,
  limit_mode int(10) unsigned NOT NULL,
  LL float DEFAULT NULL,
  HL float DEFAULT NULL,
  gtl_splitlot_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (splitlot_id,mptest_info_id,run_id,limit_index)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='limits defined during the PAT processing';


--
-- Table structure for table ft_mptest_rollingstats
--

DROP TABLE IF EXISTS ft_mptest_rollingstats;

CREATE TABLE ft_mptest_rollingstats
(
  splitlot_id int(10) unsigned NOT NULL,
  mptest_info_id smallint(5) unsigned NOT NULL,
  run_id mediumint(8) unsigned NOT NULL,
  distribution_shape varchar (255),
  n_factor float,
  t_factor float,
  mean float,
  sigma float,
  min float,
  q1 float,
  median float,
  q3 float,
  max float,
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0',
  fail_count mediumint(8) unsigned NOT NULL DEFAULT '0',
  gtl_splitlot_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (splitlot_id,mptest_info_id,run_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_mptest_stats_samples
--

DROP TABLE IF EXISTS ft_mptest_stats_samples;


CREATE TABLE ft_mptest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table ft_mptest_stats_summary
--

DROP TABLE IF EXISTS ft_mptest_stats_summary;


CREATE TABLE ft_mptest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test failed',
  min_value float DEFAULT NULL COMMENT 'smallest value found in the test results',
  max_value float DEFAULT NULL COMMENT 'highest value found in the test results',
  sum float DEFAULT NULL COMMENT 'sum of the test results',
  square_sum float DEFAULT NULL COMMENT 'square sum of the test results',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,mptest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';

--
-- Table structure for table ft_mptest_results
--

DROP TABLE IF EXISTS ft_mptest_results;


CREATE TABLE ft_mptest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  value float DEFAULT NULL COMMENT 'result value of the test',
  tpin_pmrindex int(10) DEFAULT NULL COMMENT 'index of the tester channel used to get that value (i.e. parameter tested)',
  PRIMARY KEY (splitlot_id,mptest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table ft_mptest_outliers
--

DROP TABLE IF EXISTS ft_mptest_outliers;

CREATE TABLE ft_mptest_outliers
(
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  mptest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  run_index int(10) unsigned NOT NULL,
  limits_run_id mediumint(8) unsigned NOT NULL,
  limit_type char(1) NOT NULL,
  value float COMMENT 'result value of the test',
  gtl_splitlot_id int(10) unsigned DEFAULT NULL COMMENT 'splitlot identifier retrieved from the PAT test',
  PRIMARY KEY (splitlot_id,mptest_info_id,run_id,run_index)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='PAT processed outliers';


--
-- Table structure for table ft_ftest_info
--

DROP TABLE IF EXISTS ft_ftest_info;


CREATE TABLE ft_ftest_info (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'test description unique identifier',
  tnum int(10) unsigned NOT NULL COMMENT 'test number',
  tname varchar(255) NOT NULL DEFAULT '' COMMENT 'test name',
  testseq smallint(5) unsigned DEFAULT NULL COMMENT 'order in which this test has been found in the stdf file',
  PRIMARY KEY (splitlot_id,ftest_info_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the functional F.T. tests';


--
-- Table structure for table ft_ftest_stats_samples
--

DROP TABLE IF EXISTS ft_ftest_stats_samples;


CREATE TABLE ft_ftest_stats_samples (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  PRIMARY KEY (splitlot_id,ftest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics consolidated from the individual results';


--
-- Table structure for table ft_ftest_stats_summary
--

DROP TABLE IF EXISTS ft_ftest_stats_summary;


CREATE TABLE ft_ftest_stats_summary (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  site_no smallint(5) NOT NULL DEFAULT '1' COMMENT 'test site number',
  exec_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test was executed',
  fail_count mediumint(8) unsigned DEFAULT NULL COMMENT 'how many times the test failed',
  ttime int(10) unsigned DEFAULT NULL COMMENT 'average test execution time',
  PRIMARY KEY (splitlot_id,ftest_info_id,site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='test statistics found in the stdf file';


--
-- Table structure for table ft_ftest_results
--

DROP TABLE IF EXISTS ft_ftest_results;


CREATE TABLE ft_ftest_results (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  ftest_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  run_id mediumint(8) unsigned NOT NULL COMMENT 'foreign key to the run identifier',
  testseq smallint(5) unsigned NOT NULL COMMENT 'order in which the test result has been found in the stdf file for this particular splitlot/test/run',
  result_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  vect_nam varchar(255) NOT NULL DEFAULT '' COMMENT 'vector module pattern name',
  vect_off smallint(6) DEFAULT NULL COMMENT 'integer offset of the vector from the vector of interest',
  PRIMARY KEY (splitlot_id,ftest_info_id,run_id,testseq)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='individual test results';


--
-- Table structure for table ft_test_conditions (TDR)
--

DROP TABLE IF EXISTS ft_test_conditions;


CREATE TABLE ft_test_conditions(
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  test_info_id smallint(5) unsigned NOT NULL COMMENT 'foreign key to the test description identifier',
  test_type char(1) NOT NULL COMMENT 'indicates whether the test description identifier must be looked amongst the parametric, multiparametric or functional test descrpitions',

  condition_1 varchar(255) DEFAULT NULL COMMENT 'description of the first test condition',
  condition_2 varchar(255) DEFAULT NULL COMMENT 'description of the second test condition',
  condition_3 varchar(255) DEFAULT NULL COMMENT 'description of the third test condition',
  condition_4 varchar(255) DEFAULT NULL COMMENT 'and so on...',
  condition_5 varchar(255) DEFAULT NULL,
  condition_6 varchar(255) DEFAULT NULL,
  condition_7 varchar(255) DEFAULT NULL,
  condition_8 varchar(255) DEFAULT NULL,
  condition_9 varchar(255) DEFAULT NULL,

  condition_10 varchar(255) DEFAULT NULL,
  condition_11 varchar(255) DEFAULT NULL,
  condition_12 varchar(255) DEFAULT NULL,
  condition_13 varchar(255) DEFAULT NULL,
  condition_14 varchar(255) DEFAULT NULL,
  condition_15 varchar(255) DEFAULT NULL,
  condition_16 varchar(255) DEFAULT NULL,
  condition_17 varchar(255) DEFAULT NULL,
  condition_18 varchar(255) DEFAULT NULL,
  condition_19 varchar(255) DEFAULT NULL,

  condition_20 varchar(255) DEFAULT NULL,
  condition_21 varchar(255) DEFAULT NULL,
  condition_22 varchar(255) DEFAULT NULL,
  condition_23 varchar(255) DEFAULT NULL,
  condition_24 varchar(255) DEFAULT NULL,
  condition_25 varchar(255) DEFAULT NULL,
  condition_26 varchar(255) DEFAULT NULL,
  condition_27 varchar(255) DEFAULT NULL,
  condition_28 varchar(255) DEFAULT NULL,
  condition_29 varchar(255) DEFAULT NULL,

  condition_30 varchar(255) DEFAULT NULL,
  condition_31 varchar(255) DEFAULT NULL,
  condition_32 varchar(255) DEFAULT NULL,
  condition_33 varchar(255) DEFAULT NULL,
  condition_34 varchar(255) DEFAULT NULL,
  condition_35 varchar(255) DEFAULT NULL,
  condition_36 varchar(255) DEFAULT NULL,
  condition_37 varchar(255) DEFAULT NULL,
  condition_38 varchar(255) DEFAULT NULL,
  condition_39 varchar(255) DEFAULT NULL,

  condition_40 varchar(255) DEFAULT NULL,
  condition_41 varchar(255) DEFAULT NULL,
  condition_42 varchar(255) DEFAULT NULL,
  condition_43 varchar(255) DEFAULT NULL,
  condition_44 varchar(255) DEFAULT NULL,
  condition_45 varchar(255) DEFAULT NULL,
  condition_46 varchar(255) DEFAULT NULL,
  condition_47 varchar(255) DEFAULT NULL,
  condition_48 varchar(255) DEFAULT NULL,
  condition_49 varchar(255) DEFAULT NULL,

  condition_50 varchar(255) DEFAULT NULL,
  condition_51 varchar(255) DEFAULT NULL,
  condition_52 varchar(255) DEFAULT NULL,
  condition_53 varchar(255) DEFAULT NULL,
  condition_54 varchar(255) DEFAULT NULL,
  condition_55 varchar(255) DEFAULT NULL,
  condition_56 varchar(255) DEFAULT NULL,
  condition_57 varchar(255) DEFAULT NULL,
  condition_58 varchar(255) DEFAULT NULL,
  condition_59 varchar(255) DEFAULT NULL,

  condition_60 varchar(255) DEFAULT NULL,
  condition_61 varchar(255) DEFAULT NULL,
  condition_62 varchar(255) DEFAULT NULL,
  condition_63 varchar(255) DEFAULT NULL,
  condition_64 varchar(255) DEFAULT NULL,
  condition_65 varchar(255) DEFAULT NULL,
  condition_66 varchar(255) DEFAULT NULL,
  condition_67 varchar(255) DEFAULT NULL,
  condition_68 varchar(255) DEFAULT NULL,
  condition_69 varchar(255) DEFAULT NULL,

  condition_70 varchar(255) DEFAULT NULL,
  condition_71 varchar(255) DEFAULT NULL,
  condition_72 varchar(255) DEFAULT NULL,
  condition_73 varchar(255) DEFAULT NULL,
  condition_74 varchar(255) DEFAULT NULL,
  condition_75 varchar(255) DEFAULT NULL,
  condition_76 varchar(255) DEFAULT NULL,
  condition_77 varchar(255) DEFAULT NULL,
  condition_78 varchar(255) DEFAULT NULL,
  condition_79 varchar(255) DEFAULT NULL,

  condition_80 varchar(255) DEFAULT NULL,
  condition_81 varchar(255) DEFAULT NULL,
  condition_82 varchar(255) DEFAULT NULL,
  condition_83 varchar(255) DEFAULT NULL,
  condition_84 varchar(255) DEFAULT NULL,
  condition_85 varchar(255) DEFAULT NULL,
  condition_86 varchar(255) DEFAULT NULL,
  condition_87 varchar(255) DEFAULT NULL,
  condition_88 varchar(255) DEFAULT NULL,
  condition_89 varchar(255) DEFAULT NULL,

  condition_90 varchar(255) DEFAULT NULL,
  condition_91 varchar(255) DEFAULT NULL,
  condition_92 varchar(255) DEFAULT NULL,
  condition_93 varchar(255) DEFAULT NULL,
  condition_94 varchar(255) DEFAULT NULL,
  condition_95 varchar(255) DEFAULT NULL,
  condition_96 varchar(255) DEFAULT NULL,
  condition_97 varchar(255) DEFAULT NULL,
  condition_98 varchar(255) DEFAULT NULL,
  condition_99 varchar(255) DEFAULT NULL,

  condition_100 varchar(255) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,test_info_id,test_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='defines the test condition during the characterisation stage';


--
-- Table structure for table ft_metadata_mapping
--

DROP TABLE IF EXISTS ft_metadata_mapping;


CREATE TABLE ft_metadata_mapping (
  meta_name varchar(255) NOT NULL COMMENT 'name of the additional field',
  gex_name varchar(255) DEFAULT NULL COMMENT 'name of the mapped field in gex',
  gexdb_table_name varchar(255) NOT NULL COMMENT 'name of the gex database table to which the additional field is mapped',
  gexdb_field_fullname varchar(255) NOT NULL COMMENT 'name of the column to which the additional field is mapped',
  gexdb_link_name varchar(255) DEFAULT NULL COMMENT 'foreign key to the metadata link name',
  gex_display_in_gui char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether the field must be displayed in the gex GUI (Y) or not (N)',
  bintype_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a binary type logic (Y) or not (N)',
  time_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a time type logic (Y) or not (N)',
  custom_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be treated with a custom type logic (Y) or not (N)',
  numeric_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be treated with a numeric type logic (Y) or not (N)',
  fact_field char(1) NOT NULL DEFAULT 'N',
  consolidated_field char(1) NOT NULL DEFAULT 'Y' COMMENT 'whether this field must be processed for consolidation',
  er_display_in_gui char(1) NOT NULL DEFAULT 'Y',
  az_field char(1) NOT NULL DEFAULT 'N' COMMENT 'whether this field must be processed for A-Z consolidation',
  PRIMARY KEY (meta_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='describes the additional fields usable for data filtering or consolidation';


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
  gexdb_link_name varchar(255) DEFAULT NULL,
  PRIMARY KEY (link_name)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_event
--

DROP TABLE IF EXISTS ft_event;

CREATE TABLE ft_event
(
  splitlot_id int(10) unsigned NOT NULL,
  event_id int(10) NOT NULL,
  run_id mediumint(8) NOT NULL,
  event_type varchar (255),
  event_subtype varchar (255),
  event_time_local DATETIME,
  event_time_utc DATETIME,
  event_message blob,
  gtl_splitlot_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (splitlot_id,event_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;


--
-- Table structure for table ft_sya_set
--

DROP TABLE IF EXISTS ft_sya_set;


CREATE TABLE ft_sya_set (
  sya_id int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'SYA unique identifier',
  product_id varchar(255) NOT NULL COMMENT 'foreign key to the product name',
  creation_date datetime NOT NULL COMMENT 'creation date of the SYA set',
  user_comment varchar(255) DEFAULT NULL COMMENT 'user defined comment',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SYA alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SYA alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SYA alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SYA alarm to trigger',
  start_date datetime NOT NULL COMMENT 'date from which the SYA alarm is valid',
  expiration_date date NOT NULL COMMENT 'date to which the SYA alarm is not valid anymore',
  expiration_email_date datetime DEFAULT NULL COMMENT 'date to which an expiration warning email must be sent',
  rule_type varchar(255) NOT NULL COMMENT 'SYA rule type',
  n1_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float NOT NULL DEFAULT '-1' COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  computation_fromdate date NOT NULL COMMENT 'initial date from which the limits should be computed',
  computation_todate date NOT NULL COMMENT 'final date to which the limits should be computed',
  min_lots_required smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of lots for the computed limits to be valid',
  min_data_points smallint(5) NOT NULL DEFAULT '-1' COMMENT 'minimum number of calculated yields for the computed limits to be valid',
  options tinyint(3) unsigned NOT NULL DEFAULT '0',
  flags tinyint(3) unsigned NOT NULL DEFAULT '0',
  rule_name varchar(255) DEFAULT NULL COMMENT 'name of the SYA rule',
  bin_type tinyint(1) DEFAULT NULL COMMENT 'whether the alarm should be triggered on a good (P) bin or a bad (F) bin',
  PRIMARY KEY (sya_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SYA rules appliable for the statistical yield analysis alarms';

--
-- Table structure for table ft_sbl
--

DROP TABLE IF EXISTS ft_sbl;


CREATE TABLE ft_sbl (
  sya_id int(10) unsigned NOT NULL COMMENT 'foreign key to the associated SYA rule',
  bin_no smallint(5) unsigned NOT NULL COMMENT 'bin number on which the SBL must be applied',
  bin_name varchar(255) DEFAULT NULL COMMENT 'name of the bin on which the SBL must be applied',
  ll_1 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the standard SBL alarm to trigger',
  hl_1 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the standard SBL alarm to trigger',
  ll_2 float NOT NULL DEFAULT '-1' COMMENT 'low limit for the critical SBL alarm to trigger',
  hl_2 float NOT NULL DEFAULT '-1' COMMENT 'high limit for the critical SBL alarm to trigger',
  rule_type varchar(255) DEFAULT NULL COMMENT 'SBL rule type',
  n1_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_1 and hl_1 limits',
  n2_parameter float DEFAULT NULL COMMENT 'value of the N parameter to be applied to the statistical rule when computing ll_2 and hl_2 limits',
  PRIMARY KEY (sya_id,bin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='SBL rules appliable along with SYA alarms';


--
-- Table structure for table ft_gtl_info
--

DROP TABLE IF EXISTS ft_gtl_info;


CREATE TABLE ft_gtl_info (
  gtl_splitlot_id int(10) unsigned NOT NULL,
  gtl_key varchar(255) NOT NULL,
  gtl_value varchar(255),
  PRIMARY KEY (gtl_splitlot_id,gtl_key)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='PAT processed splitlot key/value storage';

--
-- Table structure for table ft_gtl_splitlot
--

DROP TABLE IF EXISTS ft_gtl_splitlot;
CREATE TABLE ft_gtl_splitlot (
  gtl_splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  session_id bigint(4) NOT NULL DEFAULT '0',
  sqlite_splitlot_id int(10) unsigned NOT NULL,
  splitlot_id int(10) unsigned NOT NULL,
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  product_name varchar(255) DEFAULT NULL,
  setup_t int(10) NOT NULL DEFAULT '0',
  start_t int(10) NOT NULL DEFAULT '0',
  finish_t int(10) NOT NULL DEFAULT '0',
  stat_num tinyint(3) unsigned NOT NULL DEFAULT '0',
  tester_name varchar(255) NOT NULL DEFAULT '',
  tester_type varchar(255) NOT NULL DEFAULT '',
  splitlot_flags tinyint(2) unsigned NOT NULL DEFAULT 0,
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_samples mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_summary mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT '0',
  data_provider varchar(255) DEFAULT '',
  data_type varchar(255) DEFAULT '',
  prod_data char(1) NOT NULL DEFAULT 'Y',
  retest_index tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Retest index within a phase (0 for initial test, 1 for first retest, ...)',
  retest_hbins varchar(255) DEFAULT NULL COMMENT 'Retested hard bins in case of LINEAR consolidation, test/retest consolidation algorithm in case of STACKED consolidation.',
  test_insertion_index tinyint(1) unsigned DEFAULT NULL COMMENT 'Indicates the index of the insertion for multi-insertions',
  test_insertion varchar(255) DEFAULT NULL COMMENT 'Test insertion name in case of multiple insertions to generate the consolidation result',
  test_flow varchar(45) DEFAULT NULL COMMENT 'Test flow name in case of multiple flows. This can allow an additional consolidation granularity.',
  rework_code tinyint(3) unsigned NOT NULL DEFAULT '0',
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
  nb_sites tinyint(3) unsigned NOT NULL DEFAULT '1',
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
  file_host_id int(10) unsigned DEFAULT '0',
  file_path varchar(255) NOT NULL DEFAULT '',
  file_name varchar(255) NOT NULL DEFAULT '',
  valid_splitlot char(1) NOT NULL DEFAULT 'N',
  insertion_time int(10) unsigned NOT NULL DEFAULT '0',
  subcon_lot_id varchar(255) NOT NULL DEFAULT '',
  incremental_update varchar(255) DEFAULT NULL,
  sya_id int(10) unsigned DEFAULT '0',
  day varchar(10) NOT NULL,
  week_nb tinyint(2) unsigned NOT NULL,
  month_nb tinyint(2) unsigned NOT NULL,
  quarter_nb tinyint(1) unsigned NOT NULL,
  year_nb smallint(4) NOT NULL,
  year_and_week varchar(7) NOT NULL,
  year_and_month varchar(7) NOT NULL,
  year_and_quarter varchar(7) NOT NULL,
  recipe_id int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (gtl_splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='PAT processed splitlot infos';


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
  display char(1) NOT NULL DEFAULT 'Y',
  PRIMARY KEY (site_name,column_id,data_type)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports formats';


--
-- Table structure for table ft_wyr
--

DROP TABLE IF EXISTS ft_wyr;


CREATE TABLE ft_wyr (
  wyr_id INT NOT NULL AUTO_INCREMENT,
  site_name varchar(255) NOT NULL,
  week_nb tinyint(3) DEFAULT NULL,
  year smallint(5) DEFAULT NULL,
  date_in datetime DEFAULT NULL,
  date_out datetime DEFAULT NULL,
  product_name varchar(255) DEFAULT NULL,
  program_name varchar(255) DEFAULT NULL,
  tester_name varchar(255) DEFAULT NULL,
  lot_id varchar(255) DEFAULT NULL,
  subcon_lot_id varchar(255) DEFAULT NULL,
  user_split varchar(1024) DEFAULT NULL,
  yield float DEFAULT '0',
  parts_received int(10) DEFAULT '0',
  pretest_rejects int(10) DEFAULT '0',
  pretest_rejects_split varchar(1024) DEFAULT NULL,
  parts_tested int(10) DEFAULT '0',
  parts_pass int(10) DEFAULT '0',
  parts_pass_split varchar(1024) DEFAULT NULL,
  parts_fail int(10) DEFAULT '0',
  parts_fail_split varchar(1024) DEFAULT NULL,
  parts_retest int(10) DEFAULT '0',
  PARTS_RETEST_SPLIT varchar(1024) DEFAULT NULL,
  insertions int(10) DEFAULT '0',
  posttest_rejects int(10) DEFAULT '0',
  posttest_rejects_split varchar(1024) DEFAULT NULL,
  parts_shipped int(10) DEFAULT '0',
  PRIMARY KEY (wyr_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Weekly yield reports';


--
-- Table structure for table ft_dtr
--

DROP TABLE IF EXISTS ft_dtr;


CREATE TABLE ft_dtr (
  splitlot_id int(10) unsigned NOT NULL,
  run_id mediumint(7) DEFAULT 0,
  order_id mediumint(8) unsigned NOT NULL,
  dtr_type varchar(255) NOT NULL,
  dtr_text varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (splitlot_id,run_id,order_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='html summaries of PAT reports';

--
-- Table structure for table ft_sdr
--

DROP TABLE IF EXISTS ft_sdr;

CREATE TABLE ft_sdr (
  splitlot_id int(10) unsigned NOT NULL COMMENT 'foreign key to the splitlot identifier',
  site_grp smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site group number',
  site_index smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site index',
  site_no smallint unsigned NOT NULL DEFAULT '0' COMMENT 'site number',
  hand_typ varchar(255) DEFAULT NULL COMMENT 'handler of prober type',
  hand_id varchar(255) DEFAULT NULL COMMENT 'handler of prober identifier',
  card_typ varchar(255) DEFAULT NULL COMMENT 'prober card type',
  card_id varchar(255) DEFAULT NULL COMMENT 'prober card identifier',
  load_typ varchar(255) DEFAULT NULL COMMENT 'load board type',
  load_id varchar(255) DEFAULT NULL COMMENT 'load board identifier',
  dib_typ varchar(255) DEFAULT NULL COMMENT 'DIB board type',
  dib_id varchar(255) DEFAULT NULL COMMENT 'DIB board identifier',
  cabl_typ varchar(255) DEFAULT NULL COMMENT 'interface cable type',
  cabl_id varchar(255) DEFAULT NULL COMMENT 'interface cable identifier',
  cont_typ varchar(255) DEFAULT NULL COMMENT 'handler contractor type',
  cont_id varchar(255) DEFAULT NULL COMMENT 'handler contractor identifier',
  lasr_typ varchar(255) DEFAULT NULL COMMENT 'laser type',
  lasr_id varchar(255) DEFAULT NULL COMMENT 'laser identifier',
  extr_typ varchar(255) DEFAULT NULL COMMENT 'extra equipment type',
  extr_id varchar(255) DEFAULT NULL COMMENT 'extra equipment identifier',
  PRIMARY KEY (splitlot_id, site_grp, site_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='description of the sites used for testing';


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
  alarm_flags tinyint(2) unsigned NOT NULL,
  lcl float NOT NULL DEFAULT '0',
  ucl float NOT NULL DEFAULT '0',
  value float NOT NULL DEFAULT '0',
  units varchar(10) DEFAULT NULL,
  PRIMARY KEY (splitlot_id,alarm_cat,alarm_type,item_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='logs the alarms raised by the datapump tasks';




--
-- Dumping routines
--

DELIMITER ;;
CREATE PROCEDURE et_custom_incremental_update(
IN Splitlot INT,
IN IncrementalKeyword VARCHAR(1024),
OUT Message VARCHAR(1024),
OUT Status INT)
BEGIN
SELECT 'Empty custom incremental update stored procedure' INTO Message From dual;
SELECT 0 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Status INT, -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
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
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be post-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the post-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_insertion_preprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be pre-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the pre-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

-- The call to the following stored procedure should be last in this stored procedure.
-- Any customisation should be done above.
CALL et_check_data_integrity(Splitlot, Message, Status);

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_check_data_integrity(
IN Splitlot INT, -- SplitlotId of the splitlot to be checked
OUT Message VARCHAR(1024), -- Return the Error message in case the check fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

-- !!!! DO NOT MODIFY THIS STORED PROCEDURE !!!!
-- !!!! IT CAN POTENTIALLY BE DROPPED/MODIFIED BY TDR UPDATE SCRIPTS !!!!
-- !!!! ANY MODIFICATION COULD BE LOST DURING FUTURE TDR UPDATES !!!!

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE et_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_custom_incremental_update(
IN Splitlot INT,
IN IncrementalKeyword VARCHAR(1024),
OUT Message VARCHAR(1024),
OUT Status INT)
BEGIN
SELECT 'Empty custom incremental update stored procedure' INTO Message From dual;
SELECT 0 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
IN Status INT, -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
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
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be post-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the post-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_insertion_preprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be pre-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the pre-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

-- The call to the following stored procedure should be last in this stored procedure.
-- Any customisation should be done above.
CALL ft_check_data_integrity(Splitlot, Message, Status);

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_check_data_integrity(
IN Splitlot INT, -- SplitlotId of the splitlot to be checked
OUT Message VARCHAR(1024), -- Return the Error message in case the check fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

-- !!!! DO NOT MODIFY THIS STORED PROCEDURE !!!!
-- !!!! IT CAN POTENTIALLY BE DROPPED/MODIFIED BY TDR UPDATE SCRIPTS !!!!
-- !!!! ANY MODIFICATION COULD BE LOST DURING FUTURE TDR UPDATES !!!!

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE ft_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted (not used for FT)
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE purge_invalid_splitlots()
BEGIN
    CALL purge_splitlots();
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE purge_splitlots()
BEGIN
    -- Remove all invalid splitlots into the TDR except current insertions if any
    DECLARE tdr_table_name VARCHAR(255);
    DECLARE not_found BOOLEAN DEFAULT false;

    DECLARE ft_ignore_splitlots TEXT;
    DECLARE wt_ignore_splitlots TEXT;
    DECLARE et_ignore_splitlots TEXT;

	-- Select all Final Test tables linked to the splitlot_id except ft_splitlot tables
	DECLARE ft_tables CURSOR FOR
		SELECT table_name FROM information_schema.columns
		WHERE table_schema=Database() AND table_name like 'ft%' AND column_name='splitlot_id' AND table_name<>'ft_splitlot' and table_name not rlike'view';
	-- Select all Wafer Sort tables linked to the splitlot_id except wt_splitlot tables
	DECLARE wt_tables CURSOR FOR
		SELECT table_name FROM information_schema.columns
		WHERE table_schema=Database() AND table_name like 'wt%' AND column_name='splitlot_id' AND table_name<>'wt_splitlot' and table_name not rlike'view';
	-- Select all Elect Test tables linked to the splitlot_id except et_splitlot tables
	DECLARE et_tables CURSOR FOR
		SELECT table_name FROM information_schema.columns
		WHERE table_schema=Database() AND table_name like 'et%' AND column_name='splitlot_id' AND table_name<>'et_splitlot' and table_name not rlike'view';

    DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;

    -- Clean Final Test tables
    IF EXISTS (SELECT splitlot_id FROM ft_splitlot WHERE valid_splitlot='N') THEN
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Final Test tables - START');
        -- Don't purge current insertion if any
        -- All active insertions are materialized with a splitlot_id < 201000000
        SELECT concat(' AND splitlot_id NOT IN (',group_concat(S.splitlot_id separator ','),')') INTO ft_ignore_splitlots FROM ft_splitlot S WHERE
            lot_id IN (SELECT lot_id FROM ft_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND sublot_id IN (SELECT sublot_id FROM ft_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND start_t IN (SELECT start_t FROM ft_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY));

        IF (ft_ignore_splitlots IS NULL) THEN
            SELECT '' INTO ft_ignore_splitlots;
        END IF;

        OPEN ft_tables;
            SET not_found = false;
            REPEAT
                FETCH ft_tables INTO tdr_table_name;
                IF NOT not_found THEN
                -- Remove any lines from summary and results tables
                SET @ddl=CONCAT('DELETE FROM ',tdr_table_name,
                    ' WHERE splitlot_id IN (SELECT splitlot_id FROM ft_splitlot WHERE valid_splitlot=''N''',ft_ignore_splitlots,')');
                PREPARE stmt FROM @ddl;
                EXECUTE stmt;
                END IF;
            UNTIL not_found = true
            END REPEAT;
        CLOSE ft_tables;
        -- Remove any lines from splitlot table
        SET @ddl=CONCAT('DELETE FROM ft_splitlot WHERE valid_splitlot=''N''',ft_ignore_splitlots);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Final Test tables - DONE');
    END IF;


    -- Clean Wafer Sort tables
    IF EXISTS (SELECT splitlot_id FROM wt_splitlot WHERE valid_splitlot='N') THEN
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Wafer Sort tables - START');
        -- Don't purge current insertion if any
        -- All active insertions are materialized with a splitlot_id < 201000000
        SELECT concat(' AND splitlot_id NOT IN (',group_concat(S.splitlot_id separator ','),')') INTO wt_ignore_splitlots FROM wt_splitlot S WHERE
            lot_id IN (SELECT lot_id FROM wt_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND sublot_id IN (SELECT sublot_id FROM wt_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND wafer_id IN (SELECT wafer_id FROM wt_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND start_t IN (SELECT start_t FROM wt_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY));

        IF (wt_ignore_splitlots IS NULL) THEN
            SELECT '' INTO wt_ignore_splitlots;
        END IF;
        OPEN wt_tables;
            SET not_found = false;
            REPEAT
                FETCH wt_tables INTO tdr_table_name;
                IF NOT not_found THEN
                -- Remove any lines from summary and results tables
                SET @ddl=CONCAT('DELETE FROM ',tdr_table_name,
                    ' WHERE splitlot_id IN (SELECT splitlot_id FROM wt_splitlot WHERE valid_splitlot=''N''',wt_ignore_splitlots,')');
                PREPARE stmt FROM @ddl;
                EXECUTE stmt;
                END IF;
            UNTIL not_found = true
            END REPEAT;
        CLOSE wt_tables;
        -- Remove any lines from splitlot table
        SET @ddl=CONCAT('DELETE FROM wt_splitlot WHERE valid_splitlot=''N''',wt_ignore_splitlots);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Wafer Sort tables - DONE');
    END IF;

    -- Clean Elect Test tables
    IF EXISTS (SELECT splitlot_id FROM et_splitlot WHERE valid_splitlot='N') THEN
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Elect Test tables - START');
        -- Don't purge current insertion if any
        -- All active insertions are materialized with a splitlot_id < 201000000
        SELECT concat(' AND splitlot_id NOT IN (',group_concat(S.splitlot_id separator ','),')') INTO et_ignore_splitlots FROM et_splitlot S WHERE
            lot_id IN (SELECT lot_id FROM et_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND sublot_id IN (SELECT sublot_id FROM et_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND wafer_id IN (SELECT wafer_id FROM et_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY))
            AND start_t IN (SELECT start_t FROM et_splitlot WHERE valid_splitlot='N' AND splitlot_id<201000000 AND from_unixtime(insertion_time)>adddate(now(), INTERVAL -1 DAY));

        IF (et_ignore_splitlots IS NULL) THEN
            SELECT '' INTO et_ignore_splitlots;
        END IF;
        OPEN et_tables;
            SET not_found = false;
            REPEAT
                FETCH et_tables INTO tdr_table_name;
                IF NOT not_found THEN
                -- Remove any lines from summary and results tables
                SET @ddl=CONCAT('DELETE FROM ',tdr_table_name,
                    ' WHERE splitlot_id IN (SELECT splitlot_id FROM et_splitlot WHERE valid_splitlot=''N''',et_ignore_splitlots,')');
                PREPARE stmt FROM @ddl;
                EXECUTE stmt;
                END IF;
            UNTIL not_found = true
            END REPEAT;
        CLOSE et_tables;
        -- Remove any lines from splitlot table
        SET @ddl=CONCAT('DELETE FROM et_splitlot WHERE valid_splitlot=''N''',et_ignore_splitlots);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        INSERT INTO gexdb_log(log_date,log_type,log_string) values(now(),'purge_splitlots','Clean Elect Test tables - DONE');
    END IF;

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_custom_incremental_update(
IN Splitlot INT,
IN IncrementalKeyword VARCHAR(1024),
OUT Message VARCHAR(1024),
OUT Status INT)
BEGIN
SELECT 'Empty custom incremental update stored procedure' INTO Message From dual;
SELECT 0 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_filearchive_settings(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Status INT, -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
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
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_insertion_postprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be post-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the post-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_insertion_preprocessing(
IN Splitlot INT, -- SplitlotId of the splitlot to be pre-processed
OUT Message VARCHAR(1024), -- Return the Error message in case the pre-processing fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

-- The call to the following stored procedure should be last in this stored procedure.
-- Any customisation should be done above.
CALL wt_check_data_integrity(Splitlot, Message, Status);

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_check_data_integrity(
IN Splitlot INT, -- SplitlotId of the splitlot to be checked
OUT Message VARCHAR(1024), -- Return the Error message in case the check fails
OUT Status INT -- Status for the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN

-- !!!! DO NOT MODIFY THIS STORED PROCEDURE !!!!
-- !!!! IT CAN POTENTIALLY BE DROPPED/MODIFIED BY TDR UPDATE SCRIPTS !!!!
-- !!!! ANY MODIFICATION COULD BE LOST DURING FUTURE TDR UPDATES !!!!

SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;

END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_insertion_status(
IN Splitlot INT, -- SplitlotId of the splitlot to be inserted
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be inserted
IN LotID VARCHAR(1024), -- Lot of the splitlot to be inserted
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
IN Message VARCHAR(1024), -- Error message in case the insertion failed
IN Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
END ;;
DELIMITER ;


DELIMITER ;;
CREATE PROCEDURE wt_insertion_validation(
IN Splitlot INT, -- SplitlotId of the splitlot to be validated
IN TrackingLotID VARCHAR(1024), -- Tracking lot of the splitlot to be validated
IN LotID VARCHAR(1024), -- Lot of the splitlot to be validated
IN WaferID VARCHAR(1024), -- WaferID of the splitlot to be inserted
OUT TrackingLotID_Out VARCHAR(1024), -- Tracking lot to be used in GexDB for this splitlot
OUT LotID_Out VARCHAR(1024), -- Lot to be used in GexDB for this splitlot
OUT WaferID_Out VARCHAR(1024), -- Wafer to be used in GexDB for this splitlot
OUT ProductName VARCHAR(1024), -- Return the Product Name if it has to be overloaded
OUT Message VARCHAR(1024), -- Return the Error message in case the validation fails
OUT Status INT -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
BEGIN
SELECT TrackingLotID INTO TrackingLotID_Out From dual;
SELECT LotID INTO LotID_Out From dual;
SELECT WaferID INTO WaferID_Out From dual;
SELECT 'Success' INTO Message From dual;
SELECT 1 INTO Status FROM dual;
END ;;
DELIMITER ;

DELIMITER ;;
CREATE EVENT database_size
ON SCHEDULE EVERY 1 DAY
COMMENT 'Compute the mysql_database size.'
DO
 BEGIN
 DECLARE size BIGINT;
 SELECT count(*) FROM global_options WHERE option_name='GEXDB_MYSQL_SIZE' INTO size;
 IF (size = 0) THEN
	INSERT INTO global_options VALUES('GEXDB_MYSQL_SIZE','0');
 END IF;

 UPDATE global_options
	SET option_value =
		(SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024)
		FROM information_schema.TABLES
		WHERE TABLE_SCHEMA=LEFT(CURRENT_USER(),INSTR(CURRENT_USER(),'@')-1))
	WHERE option_name='GEXDB_MYSQL_SIZE';
 END;;


DELIMITER ;

DELIMITER ;;
CREATE PROCEDURE ft_gtl_insertion_validation(
IN GTL_SplitlotID  INT,            -- SplitlotId of the GTL splitlot to be validated
IN GTL_File        VARCHAR(255),   -- SQLite file of the GTL splitlot to be validated
OUT TDR_SplitlotID INT,            -- Matching TDR SplitlotId in case of success
OUT Message        VARCHAR(1024),  -- Return the Error message in case the validation fails
OUT Status         INT             -- Status of the insertion (0=FAIL , 1=PASS, 2=DELAY)
)
this_proc:BEGIN
-- #######################################################################################
-- Following is a possible implementation of the GTL traceability to TDR splitlot
-- validation and matching stored procedure.
-- If this implementation matches your needs, please uncomment it, and remove the
-- default code at the end of the stored procedure.
-- #######################################################################################
--	DECLARE lLogMessage			VARCHAR(1024);
--	DECLARE lFilePattern		VARCHAR(1024);
--	DECLARE lGTL_Product		VARCHAR(256);
--	DECLARE lGTL_Lot			VARCHAR(256);
--	DECLARE lGTL_Tester			VARCHAR(256);
--	DECLARE	lTDR_SplitlotID		INT;
--	DECLARE lTDR_NbSplitlots	INT;
--	DECLARE lLast_row_fetched	INT;
--
--	-- Cursor to loop over TDR splitlots
--	DECLARE cSplitlots CURSOR FOR (SELECT SL.splitlot_id
--		FROM ft_splitlot SL
--		WHERE SL.part_typ=lGTL_Product
--		AND SL.lot_id=lGTL_Lot
--		AND SL.tester_name=lGTL_Tester
--		AND SL.file_name LIKE lFilePattern
--		AND SL.valid_splitlot='Y'
--		AND SL.prod_data = 'Y');
--	DECLARE CONTINUE HANDLER FOR NOT FOUND SET lLast_row_fetched=1;
--
--	-- Init output variables
--	SET TDR_SplitlotID = null;
--	SELECT 'Unknown error while checking GTL traceability file ' into Message from dual;
--	SELECT concat(Message, GTL_File) into Message from dual;
--	SELECT concat(Message, '.') into Message from dual;
--	SET Status = 2;
--
--	-- Make sure file extension is '.sqlite'
--	IF (GTL_File NOT REGEXP '.sqlite$') THEN
--		SELECT 'Invalid file extension for GTL traceability file ' into Message from dual;
--		SELECT concat(Message, GTL_File) into Message from dual;
--		SELECT concat(Message, ' (.sqlite is expected).') into Message from dual;
--		SET Status = 0;
--		LEAVE this_proc;
--	END IF;
--
--	-- Get file root name
--	IF (GTL_File REGEXP '_[a-z].sqlite$') THEN
--		SELECT SUBSTRING(GTL_File, 1, LENGTH(GTL_File)-9) INTO lFilePattern;
--	ELSE
--		SELECT SUBSTRING(GTL_File, 1, LENGTH(GTL_File)-7) INTO lFilePattern;
--	END IF;
--	SELECT concat(lFilePattern, '.%') INTO lFilePattern;
--
--	-- Get keys from keyscontent table
--	SELECT key_value FROM keyscontent WHERE key_name='Product' INTO lGTL_Product;
--	SELECT key_value FROM keyscontent WHERE key_name='Lot' INTO lGTL_Lot;
--	SELECT key_value FROM keyscontent WHERE key_name='TesterName' INTO lGTL_Tester;
--
--	DROP TABLE IF EXISTS keyscontent_bak;
--	CREATE TABLE keyscontent_bak as SELECT * from keyscontent;
--
--	-- Log matching parameters
--	SELECT 'Matching parameters: part_typ=' INTO lLogMessage;
--	SELECT concat(lLogMessage, lGTL_Product) INTO lLogMessage;
--	SELECT concat(lLogMessage, ', lot_id=') INTO lLogMessage;
--	SELECT concat(lLogMessage, lGTL_Lot) INTO lLogMessage;
--	SELECT concat(lLogMessage, ', tester_name=') INTO lLogMessage;
--	SELECT concat(lLogMessage, lGTL_Tester) INTO lLogMessage;
--	SELECT concat(lLogMessage, ', file_name LIKE like ') INTO lLogMessage;
--	SELECT concat(lLogMessage, lFilePattern) INTO lLogMessage;
--
--	INSERT INTO gexdb_log(log_date, log_type, log_string)
--	VALUES(now(), 'ft_gtl_insertion_validation', lLogMessage);
--
--	-- Loop through matching TDR splitlots
--	SET lTDR_SplitlotID = null;
--	SET lTDR_NbSplitlots = 0;
--	SET lLast_row_fetched = 0;
--	OPEN cSplitlots;
--	cSplitlots_loop:LOOP
--		FETCH cSplitlots INTO lTDR_SplitlotID;
--		IF lLast_row_fetched=1 THEN
--			LEAVE cSplitlots_loop;
--		END IF;
--		-- Increment nb of matching TDR splitlots
--		SET lTDR_NbSplitlots = lTDR_NbSplitlots+1;
--	END LOOP cSplitlots_loop;
--	CLOSE cSplitlots;
--
--	-- Make sure we have only ONE matching TDR splitlot
--	IF (lTDR_NbSplitlots = 0) THEN
--		SELECT 'No TDR splitlot found for GTL traceability file ' into Message from dual;
--		SELECT concat(Message, GTL_File) into Message from dual;
--		SELECT concat(Message, ' (GTL splitlot_id=') into Message from dual;
--		SELECT concat(Message, GTL_SplitlotID) into Message from dual;
--		SELECT concat(Message, ').') into Message from dual;
--		SET Status = 2;
--		LEAVE this_proc;
--	END IF;
--	IF (lTDR_NbSplitlots > 1) THEN
--		SELECT 'More than one TDR splitlot found for GTL traceability file ' into Message from dual;
--		SELECT concat(Message, GTL_File) into Message from dual;
--		SELECT concat(Message, ' (GTL splitlot_id=') into Message from dual;
--		SELECT concat(Message, GTL_SplitlotID) into Message from dual;
--		SELECT concat(Message, ').') into Message from dual;
--		SET Status = 2;
--		LEAVE this_proc;
--	END IF;
--
--	-- Found ONE TDR splitlot: let's get out of here with success
--	SET TDR_SplitlotID = lTDR_SplitlotID;
--	SET Message = 'Success';
--	SET Status = 1;
-- #######################################################################################

	SET TDR_SplitlotID = null;
	SELECT 'Error: no GTL traceability to TDR splitlot validation and matching' INTO Message from dual;
	SELECT concat(Message, ' stored procedure defined.') INTO Message from dual;
	SET Status = 2;

END;;
DELIMITER ;
