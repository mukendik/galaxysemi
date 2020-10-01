--
-- Create schema gexdb
--


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

--
-- GLOBAL
--
DROP TABLE IF EXISTS global_info;
CREATE TABLE global_info (
  DB_VERSION_NAME						varchar(255)	NOT NULL,
  DB_VERSION_NB							smallint(5)		NOT NULL,
  DB_VERSION_BUILD						smallint(5)		NOT NULL,
  INCREMENTAL_SPLITLOTS					int(9)			NOT NULL DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

INSERT INTO GLOBAL_INFO VALUES('GEXDB V1.0 B8', 10, 8, 0);

DROP TABLE IF EXISTS incremental_update;
CREATE TABLE incremental_update (
  DB_UPDATE_NAME		varchar(255)	NOT NULL,
  INITIAL_SPLITLOTS		int(9)			NOT NULL DEFAULT 0,
  REMAINING_SPLITLOTS	int(9)			NOT NULL DEFAULT 0,
  DB_VERSION_BUILD		smallint(5)		NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS product;
CREATE TABLE product (
  PRODUCT_NAME 		varchar(255)	NOT NULL DEFAULT '',
  DESCRIPTION 		varchar(1000)	DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE product DISABLE KEYS */;
/*!40000 ALTER TABLE product ENABLE KEYS */;

DROP TABLE IF EXISTS file_host;
CREATE TABLE file_host (
  FILE_HOST_ID 		int(10) unsigned		NOT NULL auto_increment,
  HOST_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HOST_FTPUSER 		varchar(255)			NOT NULL DEFAULT '',
  HOST_FTPPASSWORD 	varchar(255)			NOT NULL DEFAULT '',
  HOST_FTPPATH 		varchar(255)			DEFAULT NULL,
  HOST_FTPPORT 		smallint(5) unsigned	NOT NULL DEFAULT 21,
  PRIMARY KEY  (FILE_HOST_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE file_host DISABLE KEYS */;
/*!40000 ALTER TABLE file_host ENABLE KEYS */;


--
-- ELECTRICAL TEST
--
DROP TABLE IF EXISTS et_sya_set;
CREATE TABLE et_sya_set (
SYA_ID					int(10) unsigned	NOT NULL auto_increment,
PRODUCT_ID				varchar(255)		NOT NULL,
CREATION_DATE			datetime			NOT NULL,
USER_COMMENT			varchar(255)		DEFAULT NULL,
LL_1					float				NOT NULL DEFAULT -1,
HL_1					float				NOT NULL DEFAULT -1,
LL_2					float				NOT NULL DEFAULT -1,
HL_2					float				NOT NULL DEFAULT -1,
START_DATE				datetime			NOT NULL,
EXPIRATION_DATE			date				NOT NULL,
EXPIRATION_EMAIL_DATE	datetime			DEFAULT NULL,
RULE_TYPE				varchar(255)		NOT NULL,
N1_PARAMETER			float				NOT NULL DEFAULT -1,
N2_PARAMETER			float				NOT NULL DEFAULT -1,
COMPUTATION_FROMDATE	date				NOT NULL,
COMPUTATION_TODATE		date				NOT NULL,
MIN_LOTS_REQUIRED		smallint(5)			NOT NULL DEFAULT -1,
MIN_DATA_POINTS			smallint(5)			NOT NULL DEFAULT -1,
OPTIONS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
FLAGS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
PRIMARY KEY  (SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS et_sbl;
CREATE TABLE et_sbl (
SYA_ID					int(10) unsigned		NOT NULL,
BIN_NO					smallint(5) unsigned	NOT NULL,
BIN_NAME				varchar(255)			DEFAULT NULL,
LL_1					float					NOT NULL DEFAULT -1,
HL_1					float					NOT NULL DEFAULT -1,
LL_2					float					NOT NULL DEFAULT -1,
HL_2					float					NOT NULL DEFAULT -1,
PRIMARY KEY  (SYA_ID, BIN_NO),
FOREIGN KEY  (SYA_ID) REFERENCES et_sya_set(SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS et_prod_alarm;
CREATE TABLE et_prod_alarm (
  SPLITLOT_ID			int(10) unsigned	NOT NULL,
  ALARM_CAT				varchar(255)		NOT NULL,
  ALARM_TYPE			varchar(255)		NOT NULL,
  ITEM_NO				int unsigned		NOT NULL,
  ITEM_NAME				varchar(255)		DEFAULT NULL,
  FLAGS					binary(2)			NOT NULL,
  LCL					float				NOT NULL DEFAULT 0,
  UCL					float				NOT NULL DEFAULT 0,
  VALUE					float				NOT NULL DEFAULT 0,
  UNITS					varchar(10)			DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS et_wyr;
CREATE TABLE et_wyr (
	SITE_NAME				varchar(255)	NOT NULL,
	WEEK_NB					tinyint(3)		DEFAULT NULL,
	YEAR					smallint(5)		DEFAULT NULL,
	DATE_IN					datetime		DEFAULT NULL,
	DATE_OUT				datetime		DEFAULT NULL,
	PRODUCT_NAME			varchar(255)	DEFAULT NULL,
	PROGRAM_NAME			varchar(255)	DEFAULT NULL,
	TESTER_NAME				varchar(255)	DEFAULT NULL,
	LOT_ID					varchar(255)	DEFAULT NULL,
	SUBCON_LOT_ID			varchar(255)	DEFAULT NULL,
	USER_SPLIT				varchar(1024)	DEFAULT NULL,
	YIELD					float			DEFAULT 0,
	PARTS_RECEIVED			int(10)			DEFAULT 0,
	PRETEST_REJECTS			int(10)			DEFAULT 0,
	PRETEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_TESTED			int(10)			DEFAULT 0,
	PARTS_PASS				int(10)			DEFAULT 0,
	PARTS_PASS_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_FAIL				int(10)			DEFAULT 0,
	PARTS_FAIL_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_RETEST			int(10)			DEFAULT 0,
	PARTS_RETEST_SPLIT		varchar(1024)	DEFAULT NULL,
	INSERTIONS				int(10)			DEFAULT 0,
	POSTTEST_REJECTS		int(10)			DEFAULT 0,
	POSTTEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_SHIPPED			int(10)			DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS et_wyr_format;
CREATE TABLE et_wyr_format (
	SITE_NAME				varchar(255)	NOT NULL,
	COLUMN_ID				tinyint(3)		NOT NULL,
	COLUMN_NB				tinyint(3)		NOT NULL,
	COLUMN_NAME				varchar(255)	NOT NULL,
	DATA_TYPE				varchar(255)	NOT NULL,
	DISPLAY					char(1)			DEFAULT 'Y' NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;


DROP TABLE IF EXISTS et_metadata_mapping;
CREATE TABLE et_metadata_mapping (
  META_NAME					varchar(255)	NOT NULL,
  GEX_NAME					varchar(255)	DEFAULT NULL,
  GEXDB_TABLE_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL,
  GEX_DISPLAY_IN_GUI		char(1)			NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE et_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE et_metadata_mapping ENABLE KEYS */;


DROP TABLE IF EXISTS et_metadata_link;
CREATE TABLE et_metadata_link (
  LINK_NAME					varchar(255)	NOT NULL,
  GEXDB_TABLE1_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD1_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_TABLE2_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD2_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE et_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE et_metadata_link ENABLE KEYS */;


DROP TABLE IF EXISTS et_lot;
CREATE TABLE et_lot (
  LOT_ID 			varchar(255)		NOT NULL DEFAULT '',
  TRACKING_LOT_ID	varchar(255)		DEFAULT NULL,
  PRODUCT_NAME 		varchar(255)		DEFAULT NULL,
  NB_PARTS 			int(10)				NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	int(10)				NOT NULL DEFAULT '0',
  FLAGS 			binary(2)
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_lot ON et_lot (LOT_ID);

/*!40000 ALTER TABLE et_lot DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot ENABLE KEYS */;

DROP TABLE IF EXISTS et_lot_hbin;
CREATE TABLE et_lot_hbin (
  LOT_ID			varchar(255)			NOT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE et_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS et_lot_sbin;
CREATE TABLE et_lot_sbin (
  LOT_ID			varchar(255)			NOT NULL,
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE et_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_lot_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_splitlot;
CREATE TABLE et_splitlot (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL auto_increment,
  LOT_ID 			varchar(255)			NOT NULL DEFAULT '',
  SUBLOT_ID 		varchar(255)			NOT NULL DEFAULT '',
  START_T 			int(10)					NOT NULL DEFAULT '0',
  FINISH_T 			int(10)					NOT NULL DEFAULT '0',
  WEEK_NB			tinyint(3)				NOT NULL DEFAULT '0',
  YEAR				smallint(5)				NOT NULL DEFAULT '0',
  TESTER_NAME 		varchar(255)			NOT NULL DEFAULT '',
  TESTER_TYPE 		varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(2)				NOT NULL DEFAULT '\0\0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0',
  DATA_PROVIDER 	varchar(255)			DEFAULT '',
  DATA_TYPE 		varchar(255)			DEFAULT '',
  PROD_DATA			char(1)					NOT NULL DEFAULT 'Y',
  JOB_NAM 			varchar(255)			NOT NULL DEFAULT '',
  JOB_REV 			varchar(255)			NOT NULL DEFAULT '',
  OPER_NAM 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_TYP 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_VER 			varchar(255)			NOT NULL DEFAULT '',
  FACIL_ID 			varchar(255)			NOT NULL DEFAULT '',
  PART_TYP 			varchar(255)			DEFAULT NULL,	
  USER_TXT 			varchar(255)			DEFAULT NULL,	
  FAMLY_ID 			varchar(255)			DEFAULT NULL,	
  PROC_ID 			varchar(255)			DEFAULT NULL,
  FILE_HOST_ID 		int(10) unsigned		DEFAULT '0',
  FILE_PATH 		varchar(255)			NOT NULL DEFAULT '',
  FILE_NAME 		varchar(255)			NOT NULL DEFAULT '',
  VALID_SPLITLOT 	char(1)					NOT NULL DEFAULT 'N',
  INSERTION_TIME 	int(10) unsigned		NOT NULL DEFAULT '0',
  SUBCON_LOT_ID		varchar(255)			NOT NULL DEFAULT '',
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  INCREMENTAL_UPDATE	varchar(255)		DEFAULT NULL,
  SYA_ID			int(10) unsigned		DEFAULT '0',
  PRIMARY KEY  (SPLITLOT_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_splitlot ON et_splitlot (LOT_ID);

/*!40000 ALTER TABLE et_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE et_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS et_wafer_info;
CREATE TABLE et_wafer_info (
  LOT_ID			varchar(255)			NOT NULL DEFAULT '',
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  FAB_ID 			varchar(255)			DEFAULT NULL,
  FRAME_ID 			varchar(255)			DEFAULT NULL,
  MASK_ID 			varchar(255)			DEFAULT NULL,
  WAFER_SIZE 		float					DEFAULT NULL,
  DIE_HT 			float					DEFAULT NULL,
  DIE_WID 			float					DEFAULT NULL,
  WAFER_UNITS 		tinyint(3) unsigned		DEFAULT NULL,
  WAFER_FLAT 		char(1)					DEFAULT NULL,
  CENTER_X 			smallint(5) unsigned	DEFAULT NULL,
  CENTER_Y 			smallint(5) unsigned	DEFAULT NULL,
  POS_X 			char(1)					DEFAULT NULL,
  POS_Y 			char(1)					DEFAULT NULL,
  GROSS_DIE			mediumint(8)			NOT NULL DEFAULT 0,
  NB_PARTS			mediumint(8)			NOT NULL DEFAULT 0,
  NB_PARTS_GOOD		mediumint(8)			NOT NULL DEFAULT 0, 
  FLAGS 			binary(2)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;


CREATE INDEX et_wafer_info ON et_wafer_info (LOT_ID, WAFER_ID);

/*!40000 ALTER TABLE et_wafer_info DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_info ENABLE KEYS */;

DROP TABLE IF EXISTS et_wafer_hbin;
CREATE TABLE et_wafer_hbin (
  LOT_ID			varchar(255)			NOT NULL,
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_wafer_hbin ON et_wafer_hbin (LOT_ID, WAFER_ID); 

/*!40000 ALTER TABLE et_wafer_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS et_wafer_sbin;
CREATE TABLE et_wafer_sbin (
  LOT_ID			varchar(255)			NOT NULL,
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_wafer_sbin ON et_wafer_sbin (LOT_ID, WAFER_ID);

/*!40000 ALTER TABLE et_wafer_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_wafer_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_hbin;
CREATE TABLE et_hbin (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  BIN_COUNT			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_hbin ON et_hbin (SPLITLOT_ID);

/*!40000 ALTER TABLE et_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_sbin;
CREATE TABLE et_sbin (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  BIN_COUNT			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_sbin ON et_sbin (SPLITLOT_ID);

/*!40000 ALTER TABLE et_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE et_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS et_run;
CREATE TABLE et_run (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  PART_ID 			varchar(255)			DEFAULT NULL,
  PART_X 			smallint(6)				DEFAULT NULL,
  PART_Y 			smallint(6)				DEFAULT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL,
  TESTS_EXECUTED 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TESTS_FAILED 		smallint(5) unsigned	NOT NULL DEFAULT '0',
  FIRSTFAIL_TNUM 	int(10) unsigned		DEFAULT NULL,
  FIRSTFAIL_TNAME 	varchar(255)			DEFAULT NULL,
  RETEST_INDEX 		tinyint(3) unsigned		NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_run ON et_run	(SPLITLOT_ID);

/*!40000 ALTER TABLE et_run DISABLE KEYS */;
/*!40000 ALTER TABLE et_run ENABLE KEYS */;


DROP TABLE IF EXISTS et_ptest_info;
CREATE TABLE et_ptest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  UNITS 			varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  LL 				float					DEFAULT NULL,
  HL 				float					DEFAULT NULL,
  TESTSEQ			smallint(5) unsigned	DEFAULT NULL,
  SPEC_LL			float					DEFAULT NULL,
  SPEC_HL			float					DEFAULT NULL,
  SPEC_TARGET		float					DEFAULT NULL
 ) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_ptest_info ON et_ptest_info (SPLITLOT_ID);

/*!40000 ALTER TABLE et_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS et_ptest_stats;
CREATE TABLE et_ptest_stats (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  EXEC_COUNT 		smallint(5) unsigned	DEFAULT NULL,
  FAIL_COUNT 		smallint(5) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE et_ptest_stats DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_stats ENABLE KEYS */;


DROP TABLE IF EXISTS et_ptest_results;
CREATE TABLE et_ptest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VALUE 			float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX et_ptest_results ON et_ptest_results	(SPLITLOT_ID, PTEST_INFO_ID);

/*!40000 ALTER TABLE et_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE et_ptest_results ENABLE KEYS */;


--
-- WAFER SORT
--
DROP TABLE IF EXISTS wt_sya_set;
CREATE TABLE wt_sya_set (
SYA_ID					int(10) unsigned	NOT NULL auto_increment,
PRODUCT_ID				varchar(255)		NOT NULL,
CREATION_DATE			datetime			NOT NULL,
USER_COMMENT			varchar(255)		DEFAULT NULL,
LL_1					float				NOT NULL DEFAULT -1,
HL_1					float				NOT NULL DEFAULT -1,
LL_2					float				NOT NULL DEFAULT -1,
HL_2					float				NOT NULL DEFAULT -1,
START_DATE				datetime			NOT NULL,
EXPIRATION_DATE			date				NOT NULL,
EXPIRATION_EMAIL_DATE	datetime			DEFAULT NULL,
RULE_TYPE				varchar(255)		NOT NULL,
N1_PARAMETER			float				NOT NULL DEFAULT -1,
N2_PARAMETER			float				NOT NULL DEFAULT -1,
COMPUTATION_FROMDATE	date				NOT NULL,
COMPUTATION_TODATE		date				NOT NULL,
MIN_LOTS_REQUIRED		smallint(5)			NOT NULL DEFAULT -1,
MIN_DATA_POINTS			smallint(5)			NOT NULL DEFAULT -1,
OPTIONS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
FLAGS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
PRIMARY KEY  (SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS wt_sbl;
CREATE TABLE wt_sbl (
SYA_ID					int(10) unsigned		NOT NULL,
BIN_NO					smallint(5) unsigned	NOT NULL,
BIN_NAME				varchar(255)			DEFAULT NULL,
LL_1					float					NOT NULL DEFAULT -1,
HL_1					float					NOT NULL DEFAULT -1,
LL_2					float					NOT NULL DEFAULT -1,
HL_2					float					NOT NULL DEFAULT -1,
PRIMARY KEY  (SYA_ID, BIN_NO),
FOREIGN KEY  (SYA_ID) REFERENCES wt_sya_set(SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS wt_prod_alarm;
CREATE TABLE wt_prod_alarm (
  SPLITLOT_ID			int(10) unsigned	NOT NULL,
  ALARM_CAT				varchar(255)		NOT NULL,
  ALARM_TYPE			varchar(255)		NOT NULL,
  ITEM_NO				int unsigned		NOT NULL,
  ITEM_NAME				varchar(255)		DEFAULT NULL,
  FLAGS					binary(2)			NOT NULL,
  LCL					float				NOT NULL DEFAULT 0,
  UCL					float				NOT NULL DEFAULT 0,
  VALUE					float				NOT NULL DEFAULT 0,
  UNITS					varchar(10)			DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS wt_wyr;
CREATE TABLE wt_wyr (
	SITE_NAME				varchar(255)	NOT NULL,
	WEEK_NB					tinyint(3)		DEFAULT NULL,
	YEAR					smallint(5)		DEFAULT NULL,
	DATE_IN					datetime		DEFAULT NULL,
	DATE_OUT				datetime		DEFAULT NULL,
	PRODUCT_NAME			varchar(255)	DEFAULT NULL,
	PROGRAM_NAME			varchar(255)	DEFAULT NULL,
	TESTER_NAME				varchar(255)	DEFAULT NULL,
	LOT_ID					varchar(255)	DEFAULT NULL,
	SUBCON_LOT_ID			varchar(255)	DEFAULT NULL,
	USER_SPLIT				varchar(1024)	DEFAULT NULL,
	YIELD					float			DEFAULT 0,
	PARTS_RECEIVED			int(10)			DEFAULT 0,
	PRETEST_REJECTS			int(10)			DEFAULT 0,
	PRETEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_TESTED			int(10)			DEFAULT 0,
	PARTS_PASS				int(10)			DEFAULT 0,
	PARTS_PASS_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_FAIL				int(10)			DEFAULT 0,
	PARTS_FAIL_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_RETEST			int(10)			DEFAULT 0,
	PARTS_RETEST_SPLIT		varchar(1024)	DEFAULT NULL,
	INSERTIONS				int(10)			DEFAULT 0,
	POSTTEST_REJECTS		int(10)			DEFAULT 0,
	POSTTEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_SHIPPED			int(10)			DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS wt_wyr_format;
CREATE TABLE wt_wyr_format (
	SITE_NAME				varchar(255)	NOT NULL,
	COLUMN_ID				tinyint(3)		NOT NULL,
	COLUMN_NB				tinyint(3)		NOT NULL,
	COLUMN_NAME				varchar(255)	NOT NULL,
	DATA_TYPE				varchar(255)	NOT NULL,
	DISPLAY					char(1)			DEFAULT 'Y' NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS wt_metadata_mapping;
CREATE TABLE wt_metadata_mapping (
  META_NAME					varchar(255)	NOT NULL,
  GEX_NAME					varchar(255)	DEFAULT NULL,
  GEXDB_TABLE_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL,
  GEX_DISPLAY_IN_GUI		char(1)			NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE wt_metadata_mapping ENABLE KEYS */;


DROP TABLE IF EXISTS wt_metadata_link;
CREATE TABLE wt_metadata_link (
  LINK_NAME					varchar(255)	NOT NULL,
  GEXDB_TABLE1_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD1_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_TABLE2_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD2_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE wt_metadata_link ENABLE KEYS */;


DROP TABLE IF EXISTS wt_lot;
CREATE TABLE wt_lot (
  LOT_ID 			varchar(255)			NOT NULL DEFAULT '',
  TRACKING_LOT_ID	varchar(255)			DEFAULT NULL,
  PRODUCT_NAME 		varchar(255)			DEFAULT NULL,
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0',
  FLAGS 			binary(2)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_lot ON wt_lot (LOT_ID);

/*!40000 ALTER TABLE wt_lot DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot ENABLE KEYS */;

DROP TABLE IF EXISTS wt_lot_hbin;
CREATE TABLE wt_lot_hbin (
  LOT_ID			varchar(255)			NOT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_lot_sbin;
CREATE TABLE wt_lot_sbin (
  LOT_ID			varchar(255)			NOT NULL,
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_lot_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_splitlot;
CREATE TABLE wt_splitlot (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL auto_increment,
  LOT_ID 			varchar(255)			NOT NULL DEFAULT '',
  SUBLOT_ID 		varchar(255)			NOT NULL DEFAULT '',
  SETUP_T 			int(10)					NOT NULL DEFAULT '0',
  START_T 			int(10)					NOT NULL DEFAULT '0',
  FINISH_T 			int(10)					NOT NULL DEFAULT '0',
  WEEK_NB			tinyint(3)				NOT NULL DEFAULT '0',
  YEAR				smallint(5)				NOT NULL DEFAULT '0',
  STAT_NUM 			tinyint(3) unsigned		NOT NULL DEFAULT '0',
  TESTER_NAME 		varchar(255)			NOT NULL DEFAULT '',
  TESTER_TYPE 		varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(2)				NOT NULL DEFAULT '\0\0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SAMPLES	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SAMPLES_GOOD mediumint(8)		NOT NULL DEFAULT '0',
  NB_PARTS_SUMMARY 	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SUMMARY_GOOD mediumint(8)		NOT NULL DEFAULT '0',
  DATA_PROVIDER 	varchar(255)			DEFAULT '',
  DATA_TYPE 		varchar(255)			DEFAULT '',
  PROD_DATA 		char(1)					NOT NULL DEFAULT 'Y',
  RETEST_INDEX 		tinyint(3) unsigned		NOT NULL DEFAULT '0',
  RETEST_HBINS 		varchar(255)			DEFAULT NULL,
  REWORK_CODE 		tinyint(3) unsigned		NOT NULL DEFAULT '0',
  JOB_NAM 			varchar(255)			NOT NULL DEFAULT '',
  JOB_REV 			varchar(255)			NOT NULL DEFAULT '',
  OPER_NAM 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_TYP 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_VER 			varchar(255)			NOT NULL DEFAULT '',
  TEST_COD 			varchar(255)			NOT NULL DEFAULT '',
  FACIL_ID 			varchar(255)			NOT NULL DEFAULT '',
  TST_TEMP 			varchar(255)			NOT NULL DEFAULT '',
  MODE_COD 			char(1)					DEFAULT NULL,	
  RTST_COD 			char(1)					DEFAULT NULL,	
  PROT_COD 			char(1)					DEFAULT NULL,	
  BURN_TIM 			int(10) 				DEFAULT NULL,	
  CMOD_COD 			char(1)					DEFAULT NULL,	
  PART_TYP 			varchar(255)			DEFAULT NULL,	
  USER_TXT 			varchar(255)			DEFAULT NULL,	
  AUX_FILE 			varchar(255)			DEFAULT NULL,	
  PKG_TYP 			varchar(255)			DEFAULT NULL,
  FAMLY_ID 			varchar(255)			DEFAULT NULL,	
  DATE_COD 			varchar(255)			DEFAULT NULL,	
  FLOOR_ID 			varchar(255)			DEFAULT NULL,	
  PROC_ID 			varchar(255)			DEFAULT NULL,
  OPER_FRQ 			varchar(255)			DEFAULT NULL,	
  SPEC_NAM 			varchar(255)			DEFAULT NULL,	
  SPEC_VER 			varchar(255)			DEFAULT NULL,
  FLOW_ID 			varchar(255)			DEFAULT NULL,
  SETUP_ID 			varchar(255)			DEFAULT NULL,
  DSGN_REV 			varchar(255)			DEFAULT NULL,
  ENG_ID 			varchar(255)			DEFAULT NULL,
  ROM_COD 			varchar(255)			DEFAULT NULL,
  SERL_NUM 			varchar(255)			DEFAULT NULL,
  SUPR_NAM 			varchar(255)			DEFAULT NULL,
  NB_SITES 			tinyint(3) unsigned		NOT NULL DEFAULT '1',
  HEAD_NUM 			tinyint(3) unsigned		DEFAULT NULL,
  HANDLER_TYP		varchar(255)			DEFAULT NULL,
  HANDLER_ID		varchar(255)			DEFAULT NULL,
  CARD_TYP			varchar(255)			DEFAULT NULL,
  CARD_ID			varchar(255)			DEFAULT NULL,
  LOADBOARD_TYP		varchar(255)			DEFAULT NULL,
  LOADBOARD_ID		varchar(255)			DEFAULT NULL,
  DIB_TYP			varchar(255)			DEFAULT NULL,
  DIB_ID			varchar(255)			DEFAULT NULL,
  CABLE_TYP			varchar(255)			DEFAULT NULL,
  CABLE_ID			varchar(255)			DEFAULT NULL,
  CONTACTOR_TYP		varchar(255)			DEFAULT NULL,
  CONTACTOR_ID		varchar(255)			DEFAULT NULL,
  LASER_TYP			varchar(255)			DEFAULT NULL,
  LASER_ID			varchar(255)			DEFAULT NULL,
  EXTRA_TYP			varchar(255)			DEFAULT NULL,
  EXTRA_ID			varchar(255)			DEFAULT NULL,
  FILE_HOST_ID 		int(10) unsigned		DEFAULT '0',
  FILE_PATH 		varchar(255)			NOT NULL DEFAULT '',
  FILE_NAME 		varchar(255)			NOT NULL DEFAULT '',
  VALID_SPLITLOT 	char(1)					NOT NULL DEFAULT 'N',
  INSERTION_TIME 	int(10) unsigned		NOT NULL DEFAULT '0',
  SUBCON_LOT_ID		varchar(255)			NOT NULL DEFAULT '',
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  INCREMENTAL_UPDATE	varchar(255)		DEFAULT NULL,
  SYA_ID			int(10) unsigned		DEFAULT '0',
  PRIMARY KEY  (SPLITLOT_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_splitlot ON wt_splitlot (LOT_ID);

/*!40000 ALTER TABLE wt_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE wt_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS wt_wafer_info;
CREATE TABLE wt_wafer_info (
  LOT_ID			varchar(255)			NOT NULL DEFAULT '',
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  FAB_ID 			varchar(255)			DEFAULT NULL,
  FRAME_ID 			varchar(255)			DEFAULT NULL,
  MASK_ID 			varchar(255)			DEFAULT NULL,
  WAFER_SIZE 		float					DEFAULT NULL,
  DIE_HT 			float					DEFAULT NULL,
  DIE_WID 			float					DEFAULT NULL,
  WAFER_UNITS 		tinyint(3) unsigned		DEFAULT NULL,
  WAFER_FLAT 		char(1)					DEFAULT NULL,
  CENTER_X 			smallint(5) unsigned	DEFAULT NULL,
  CENTER_Y 			smallint(5) unsigned	DEFAULT NULL,
  POS_X 			char(1)					DEFAULT NULL,
  POS_Y 			char(1)					DEFAULT NULL,
  GROSS_DIE			mediumint(8)			NOT NULL DEFAULT 0,
  NB_PARTS			mediumint(8)			NOT NULL DEFAULT 0,
  NB_PARTS_GOOD		mediumint(8)			NOT NULL DEFAULT 0, 
  FLAGS 			binary(2)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_wafer_info ON wt_wafer_info (LOT_ID, WAFER_ID);

/*!40000 ALTER TABLE wt_wafer_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_info ENABLE KEYS */;

DROP TABLE IF EXISTS wt_wafer_hbin;
CREATE TABLE wt_wafer_hbin (
  LOT_ID			varchar(255)			NOT NULL,
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_wafer_hbin ON wt_wafer_hbin (LOT_ID, WAFER_ID); 

/*!40000 ALTER TABLE wt_wafer_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_wafer_sbin;
CREATE TABLE wt_wafer_sbin (
  LOT_ID			varchar(255)			NOT NULL,
  WAFER_ID 			varchar(255)			DEFAULT NULL,
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_wafer_sbin ON wt_wafer_sbin (LOT_ID, WAFER_ID); 

/*!40000 ALTER TABLE wt_wafer_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_wafer_sbin ENABLE KEYS */;

DROP TABLE IF EXISTS wt_parts_stats_samples;
CREATE TABLE wt_parts_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_parts_stats_samples ON wt_parts_stats_samples (SPLITLOT_ID, SITE_NO); 

/*!40000 ALTER TABLE wt_parts_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_parts_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_parts_stats_summary;
CREATE TABLE wt_parts_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_GOOD 			mediumint(8)			DEFAULT NULL,
  NB_RTST 			mediumint(8)			DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_parts_stats_summary ON wt_parts_stats_summary (SPLITLOT_ID, SITE_NO);

/*!40000 ALTER TABLE wt_parts_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_parts_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_hbin;
CREATE TABLE wt_hbin (
  SPLITLOT_ID		int(10) unsigned		NOT NULL DEFAULT '0',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_hbin ON wt_hbin (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_hbin_stats_samples;
CREATE TABLE wt_hbin_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_hbin_stats_samples ON wt_hbin_stats_samples (SPLITLOT_ID, SITE_NO, HBIN_NO);

/*!40000 ALTER TABLE wt_hbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin_stats_samples ENABLE KEYS */;



DROP TABLE IF EXISTS wt_hbin_stats_summary;
CREATE TABLE wt_hbin_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  BIN_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_hbin_stats_summary ON wt_hbin_stats_summary (SPLITLOT_ID, SITE_NO, HBIN_NO); 

/*!40000 ALTER TABLE wt_hbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_hbin_stats_summary ENABLE KEYS */;

DROP TABLE IF EXISTS wt_sbin;
CREATE TABLE wt_sbin (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_sbin ON wt_sbin (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS wt_sbin_stats_samples;
CREATE TABLE wt_sbin_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_sbin_stats_samples ON wt_sbin_stats_samples (SPLITLOT_ID, SITE_NO, SBIN_NO);

/*!40000 ALTER TABLE wt_sbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_sbin_stats_summary;
CREATE TABLE wt_sbin_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  BIN_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_sbin_stats_summary ON wt_sbin_stats_summary (SPLITLOT_ID, SITE_NO, SBIN_NO);

/*!40000 ALTER TABLE wt_sbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_sbin_stats_summary ENABLE KEYS */;

DROP TABLE IF EXISTS wt_run;
CREATE TABLE wt_run (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  PART_ID 			varchar(255)			DEFAULT NULL,
  PART_X 			smallint(6)				DEFAULT NULL,
  PART_Y	 		smallint(6)				DEFAULT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL,
  TESTS_EXECUTED 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TESTS_FAILED 		smallint(5) unsigned	NOT NULL DEFAULT '0',
  FIRSTFAIL_TNUM 	int(10) unsigned		DEFAULT NULL,
  FIRSTFAIL_TNAME 	varchar(255)			DEFAULT NULL,
  RETEST_INDEX 		tinyint(3) unsigned		NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_run ON wt_run (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_run DISABLE KEYS */;
/*!40000 ALTER TABLE wt_run ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_info;
CREATE TABLE wt_ptest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  UNITS 			varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  LL 				float					DEFAULT NULL,
  HL 				float					DEFAULT NULL,
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL,
  SPEC_LL			float					DEFAULT NULL,
  SPEC_HL			float					DEFAULT NULL,
  SPEC_TARGET		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_ptest_info ON wt_ptest_info (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_limits;
CREATE TABLE wt_ptest_limits (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  LL 				float					NOT NULL DEFAULT '0',
  HL 				float					NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_ptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_results;
CREATE TABLE wt_ptest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID		smallint(5) unsigned	NOT NULL DEFAULT '0',
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VALUE 			float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_ptest_results ON wt_ptest_results (SPLITLOT_ID, PTEST_INFO_ID);

/*!40000 ALTER TABLE wt_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_stats_samples;
CREATE TABLE wt_ptest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID		smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_ptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ptest_stats_summary;
CREATE TABLE wt_ptest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_ptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_info;
CREATE TABLE wt_mptest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  TPIN_ARRAYINDEX	smallint(6)				NOT NULL DEFAULT '0',
  UNITS 			varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  LL 				float					DEFAULT NULL,
  HL 				float					DEFAULT NULL,
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL,
  SPEC_LL			float					DEFAULT NULL,
  SPEC_HL			float					DEFAULT NULL,
  SPEC_TARGET		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_mptest_info ON wt_mptest_info (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_mptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_limits;
CREATE TABLE wt_mptest_limits (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO			smallint(5)				NOT NULL DEFAULT '1',
  LL 				float					NOT NULL DEFAULT '0',
  HL 				float					NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_mptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_results;
CREATE TABLE wt_mptest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VALUE 			float					NOT NULL DEFAULT '0',
  TPIN_PMRINDEX		smallint(6)				DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_mptest_results ON wt_mptest_results (SPLITLOT_ID, MPTEST_INFO_ID);

/*!40000 ALTER TABLE wt_mptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_stats_samples;
CREATE TABLE wt_mptest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_mptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_mptest_stats_summary;
CREATE TABLE wt_mptest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_mptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_mptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_info;
CREATE TABLE wt_ftest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_ftest_info ON wt_ftest_info (SPLITLOT_ID);

/*!40000 ALTER TABLE wt_ftest_info DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_info ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_results;
CREATE TABLE wt_ftest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL,
  RUN_ID 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VECT_NAM 			varchar(255)			NOT NULL DEFAULT '',
  VECT_OFF 			smallint(6)				DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX wt_ftest_results ON wt_ftest_results (SPLITLOT_ID, FTEST_INFO_ID);

/*!40000 ALTER TABLE wt_ftest_results DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_results ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_stats_samples;
CREATE TABLE wt_ftest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_ftest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS wt_ftest_stats_summary;
CREATE TABLE wt_ftest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE wt_ftest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE wt_ftest_stats_summary ENABLE KEYS */;

--
-- FINAL TEST
--
DROP TABLE IF EXISTS ft_sya_set;
CREATE TABLE ft_sya_set (
SYA_ID					int(10) unsigned	NOT NULL auto_increment,
PRODUCT_ID				varchar(255)		NOT NULL,
CREATION_DATE			datetime			NOT NULL,
USER_COMMENT			varchar(255)		DEFAULT NULL,
LL_1					float				NOT NULL DEFAULT -1,
HL_1					float				NOT NULL DEFAULT -1,
LL_2					float				NOT NULL DEFAULT -1,
HL_2					float				NOT NULL DEFAULT -1,
START_DATE				datetime			NOT NULL,
EXPIRATION_DATE			date				NOT NULL,
EXPIRATION_EMAIL_DATE	datetime			DEFAULT NULL,
RULE_TYPE				varchar(255)		NOT NULL,
N1_PARAMETER			float				NOT NULL DEFAULT -1,
N2_PARAMETER			float				NOT NULL DEFAULT -1,
COMPUTATION_FROMDATE	date				NOT NULL,
COMPUTATION_TODATE		date				NOT NULL,
MIN_LOTS_REQUIRED		smallint(5)			NOT NULL DEFAULT -1,
MIN_DATA_POINTS			smallint(5)			NOT NULL DEFAULT -1,
OPTIONS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
FLAGS					tinyint(3) unsigned	NOT NULL DEFAULT 0,
PRIMARY KEY  (SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS ft_sbl;
CREATE TABLE ft_sbl (
SYA_ID					int(10) unsigned		NOT NULL,
BIN_NO					smallint(5) unsigned	NOT NULL,
BIN_NAME				varchar(255)			DEFAULT NULL,
LL_1					float					NOT NULL DEFAULT -1,
HL_1					float					NOT NULL DEFAULT -1,
LL_2					float					NOT NULL DEFAULT -1,
HL_2					float					NOT NULL DEFAULT -1,
PRIMARY KEY  (SYA_ID, BIN_NO),
FOREIGN KEY  (SYA_ID) REFERENCES ft_sya_set(SYA_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS ft_wyr;
CREATE TABLE ft_wyr (
	SITE_NAME				varchar(255)	NOT NULL,
	WEEK_NB					tinyint(3)		DEFAULT NULL,
	YEAR					smallint(5)		DEFAULT NULL,
	DATE_IN					datetime		DEFAULT NULL,
	DATE_OUT				datetime		DEFAULT NULL,
	PRODUCT_NAME			varchar(255)	DEFAULT NULL,
	PROGRAM_NAME			varchar(255)	DEFAULT NULL,
	TESTER_NAME				varchar(255)	DEFAULT NULL,
	LOT_ID					varchar(255)	DEFAULT NULL,
	SUBCON_LOT_ID			varchar(255)	DEFAULT NULL,
	USER_SPLIT				varchar(1024)	DEFAULT NULL,
	YIELD					float			DEFAULT 0,
	PARTS_RECEIVED			int(10)			DEFAULT 0,
	PRETEST_REJECTS			int(10)			DEFAULT 0,
	PRETEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_TESTED			int(10)			DEFAULT 0,
	PARTS_PASS				int(10)			DEFAULT 0,
	PARTS_PASS_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_FAIL				int(10)			DEFAULT 0,
	PARTS_FAIL_SPLIT		varchar(1024)	DEFAULT NULL,
	PARTS_RETEST			int(10)			DEFAULT 0,
	PARTS_RETEST_SPLIT		varchar(1024)	DEFAULT NULL,
	INSERTIONS				int(10)			DEFAULT 0,
	POSTTEST_REJECTS		int(10)			DEFAULT 0,
	POSTTEST_REJECTS_SPLIT	varchar(1024)	DEFAULT NULL,
	PARTS_SHIPPED			int(10)			DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS ft_wyr_format;
CREATE TABLE ft_wyr_format (
	SITE_NAME				varchar(255)	NOT NULL,
	COLUMN_ID				tinyint(3)		NOT NULL,
	COLUMN_NB				tinyint(3)		NOT NULL,
	COLUMN_NAME				varchar(255)	NOT NULL,
	DATA_TYPE				varchar(255)	NOT NULL,
	DISPLAY					char(1)			DEFAULT 'Y' NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

DROP TABLE IF EXISTS ft_metadata_mapping;
CREATE TABLE ft_metadata_mapping (
  META_NAME					varchar(255)	NOT NULL,
  GEX_NAME					varchar(255)	DEFAULT NULL,
  GEXDB_TABLE_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL,
  GEX_DISPLAY_IN_GUI		char(1)			NOT NULL DEFAULT 'Y'
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_metadata_mapping DISABLE KEYS */;
/*!40000 ALTER TABLE ft_metadata_mapping ENABLE KEYS */;


DROP TABLE IF EXISTS ft_metadata_link;
CREATE TABLE ft_metadata_link (
  LINK_NAME					varchar(255)	NOT NULL,
  GEXDB_TABLE1_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD1_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_TABLE2_NAME			varchar(255)	NOT NULL,
  GEXDB_FIELD2_FULLNAME		varchar(255)	NOT NULL,
  GEXDB_LINK_NAME			varchar(255)	DEFAULT NULL
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_metadata_link DISABLE KEYS */;
/*!40000 ALTER TABLE ft_metadata_link ENABLE KEYS */;


DROP TABLE IF EXISTS ft_lot;
CREATE TABLE ft_lot (
  LOT_ID 			varchar(255)			NOT NULL DEFAULT '',
  TRACKING_LOT_ID	varchar(255)			DEFAULT NULL,
  PRODUCT_NAME 		varchar(255)			DEFAULT NULL,
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0',
  FLAGS 			binary(2)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_lot ON ft_lot (LOT_ID);

/*!40000 ALTER TABLE ft_lot DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot ENABLE KEYS */;

DROP TABLE IF EXISTS ft_lot_hbin;
CREATE TABLE ft_lot_hbin (
  LOT_ID			varchar(255)			NOT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_lot_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot_hbin ENABLE KEYS */;

DROP TABLE IF EXISTS ft_lot_sbin;
CREATE TABLE ft_lot_sbin (
  LOT_ID			varchar(255)			NOT NULL,
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL,
  NB_PARTS			mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_lot_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_lot_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_splitlot;
CREATE TABLE ft_splitlot (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL auto_increment,
  LOT_ID 			varchar(255)			NOT NULL DEFAULT '',
  SUBLOT_ID 		varchar(255)			NOT NULL DEFAULT '',
  SETUP_T 			int(10)					NOT NULL DEFAULT '0',
  START_T 			int(10)					NOT NULL DEFAULT '0',
  FINISH_T 			int(10)					NOT NULL DEFAULT '0',
  WEEK_NB			tinyint(3)				NOT NULL DEFAULT '0',
  YEAR				smallint(5)				NOT NULL DEFAULT '0',
  STAT_NUM 			tinyint(3) unsigned		NOT NULL DEFAULT '0',
  TESTER_NAME 		varchar(255)			NOT NULL DEFAULT '',
  TESTER_TYPE 		varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(2)				NOT NULL DEFAULT '\0\0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SAMPLES	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SAMPLES_GOOD mediumint(8)		NOT NULL DEFAULT '0',
  NB_PARTS_SUMMARY 	mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_SUMMARY_GOOD mediumint(8)		NOT NULL DEFAULT '0',
  DATA_PROVIDER 	varchar(255)			DEFAULT '',
  DATA_TYPE 		varchar(255)			DEFAULT '',
  PROD_DATA 		char(1)					NOT NULL DEFAULT 'Y',
  RETEST_INDEX 		tinyint(3) unsigned		NOT NULL DEFAULT '0',
  RETEST_HBINS 		varchar(255)			DEFAULT NULL,
  REWORK_CODE 		tinyint(3) unsigned		NOT NULL DEFAULT '0',
  JOB_NAM 			varchar(255)			NOT NULL DEFAULT '',
  JOB_REV 			varchar(255)			NOT NULL DEFAULT '',
  OPER_NAM 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_TYP 			varchar(255)			NOT NULL DEFAULT '',
  EXEC_VER 			varchar(255)			NOT NULL DEFAULT '',
  TEST_COD 			varchar(255)			NOT NULL DEFAULT '',
  FACIL_ID 			varchar(255)			NOT NULL DEFAULT '',
  TST_TEMP 			varchar(255)			NOT NULL DEFAULT '',
  MODE_COD 			char(1)					DEFAULT NULL,	
  RTST_COD 			char(1)					DEFAULT NULL,	
  PROT_COD 			char(1)					DEFAULT NULL,	
  BURN_TIM 			int(10) 				DEFAULT NULL,	
  CMOD_COD 			char(1)					DEFAULT NULL,	
  PART_TYP 			varchar(255)			DEFAULT NULL,	
  USER_TXT 			varchar(255)			DEFAULT NULL,	
  AUX_FILE 			varchar(255)			DEFAULT NULL,	
  PKG_TYP 			varchar(255)			DEFAULT NULL,
  FAMLY_ID 			varchar(255)			DEFAULT NULL,	
  DATE_COD 			varchar(255)			DEFAULT NULL,	
  FLOOR_ID 			varchar(255)			DEFAULT NULL,	
  PROC_ID 			varchar(255)			DEFAULT NULL,
  OPER_FRQ 			varchar(255)			DEFAULT NULL,	
  SPEC_NAM 			varchar(255)			DEFAULT NULL,	
  SPEC_VER 			varchar(255)			DEFAULT NULL,
  FLOW_ID 			varchar(255)			DEFAULT NULL,
  SETUP_ID 			varchar(255)			DEFAULT NULL,
  DSGN_REV 			varchar(255)			DEFAULT NULL,
  ENG_ID 			varchar(255)			DEFAULT NULL,
  ROM_COD 			varchar(255)			DEFAULT NULL,
  SERL_NUM 			varchar(255)			DEFAULT NULL,
  SUPR_NAM 			varchar(255)			DEFAULT NULL,
  NB_SITES 			tinyint(3) unsigned		NOT NULL DEFAULT '1',
  HEAD_NUM 			tinyint(3) unsigned		DEFAULT NULL,
  HANDLER_TYP		varchar(255)			DEFAULT NULL,
  HANDLER_ID		varchar(255)			DEFAULT NULL,
  CARD_TYP			varchar(255)			DEFAULT NULL,
  CARD_ID			varchar(255)			DEFAULT NULL,
  LOADBOARD_TYP		varchar(255)			DEFAULT NULL,
  LOADBOARD_ID		varchar(255)			DEFAULT NULL,
  DIB_TYP			varchar(255)			DEFAULT NULL,
  DIB_ID			varchar(255)			DEFAULT NULL,
  CABLE_TYP			varchar(255)			DEFAULT NULL,
  CABLE_ID			varchar(255)			DEFAULT NULL,
  CONTACTOR_TYP		varchar(255)			DEFAULT NULL,
  CONTACTOR_ID		varchar(255)			DEFAULT NULL,
  LASER_TYP			varchar(255)			DEFAULT NULL,
  LASER_ID			varchar(255)			DEFAULT NULL,
  EXTRA_TYP			varchar(255)			DEFAULT NULL,
  EXTRA_ID			varchar(255)			DEFAULT NULL,
  FILE_HOST_ID 		int(10) unsigned		DEFAULT '0',
  FILE_PATH 		varchar(255)			NOT NULL DEFAULT '',
  FILE_NAME 		varchar(255)			NOT NULL DEFAULT '',
  VALID_SPLITLOT 	char(1)					NOT NULL DEFAULT 'N',
  INSERTION_TIME 	int(10) unsigned		NOT NULL DEFAULT '0',
  SUBCON_LOT_ID		varchar(255)			NOT NULL DEFAULT '',
  INCREMENTAL_UPDATE	varchar(255)		DEFAULT NULL,
  SYA_ID			int(10) unsigned		DEFAULT '0',
  PRIMARY KEY  (SPLITLOT_ID)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_splitlot ON ft_splitlot (LOT_ID);

/*!40000 ALTER TABLE ft_splitlot DISABLE KEYS */;
/*!40000 ALTER TABLE ft_splitlot ENABLE KEYS */;


DROP TABLE IF EXISTS ft_parts_stats_samples;
CREATE TABLE ft_parts_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  NB_PARTS			mediumint(8)			NOT NULL DEFAULT '0',
  NB_PARTS_GOOD 	mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM	DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_parts_stats_samples ON ft_parts_stats_samples (SPLITLOT_ID, SITE_NO);

/*!40000 ALTER TABLE ft_parts_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_parts_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_parts_stats_summary;
CREATE TABLE ft_parts_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0',
  NB_GOOD 			mediumint(8)			DEFAULT NULL,
  NB_RTST 			mediumint(8)			DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_parts_stats_summary ON ft_parts_stats_summary (SPLITLOT_ID, SITE_NO);

/*!40000 ALTER TABLE ft_parts_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_parts_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_hbin;
CREATE TABLE ft_hbin (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  HBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  HBIN_CAT 			char(1)					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_hbin ON ft_hbin (SPLITLOT_ID);

/*!40000 ALTER TABLE ft_hbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_hbin_stats_samples;
CREATE TABLE ft_hbin_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_hbin_stats_samples ON ft_hbin_stats_samples (SPLITLOT_ID, SITE_NO, HBIN_NO);

/*!40000 ALTER TABLE ft_hbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin_stats_samples ENABLE KEYS */;

DROP TABLE IF EXISTS ft_hbin_stats_summary;
CREATE TABLE ft_hbin_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  BIN_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_hbin_stats_summary ON ft_hbin_stats_summary (SPLITLOT_ID, SITE_NO, HBIN_NO);

/*!40000 ALTER TABLE ft_hbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_hbin_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin;
CREATE TABLE ft_sbin (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NAME 		varchar(255)			NOT NULL DEFAULT '',
  SBIN_CAT 			char(1)					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_sbin ON ft_sbin (SPLITLOT_ID);

/*!40000 ALTER TABLE ft_sbin DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin_stats_samples;
CREATE TABLE ft_sbin_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  NB_PARTS 			mediumint(8)			NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_sbin_stats_samples ON ft_sbin_stats_samples (SPLITLOT_ID, SITE_NO, SBIN_NO);

/*!40000 ALTER TABLE ft_sbin_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_sbin_stats_summary;
CREATE TABLE ft_sbin_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  SBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  BIN_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_sbin_stats_summary ON ft_sbin_stats_summary (SPLITLOT_ID, SITE_NO, SBIN_NO);

/*!40000 ALTER TABLE ft_sbin_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_sbin_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_run;
CREATE TABLE ft_run (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  RUN_ID 			mediumint(8) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  PART_ID 			varchar(255)			DEFAULT NULL,
  PART_X			smallint(6)				DEFAULT NULL,
  PART_Y 			smallint(6)				DEFAULT NULL,
  HBIN_NO 			smallint(5) unsigned	NOT NULL DEFAULT '0',
  SBIN_NO 			smallint(5) unsigned	DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL,
  TESTS_EXECUTED 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TESTS_FAILED 		smallint(5) unsigned	NOT NULL DEFAULT '0',
  FIRSTFAIL_TNUM 	int(10) unsigned		DEFAULT NULL,
  FIRSTFAIL_TNAME 	varchar(255)			DEFAULT NULL,
  RETEST_INDEX 		tinyint(3) unsigned		NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_run ON ft_run (SPLITLOT_ID);

/*!40000 ALTER TABLE ft_run DISABLE KEYS */;
/*!40000 ALTER TABLE ft_run ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_info;
CREATE TABLE ft_ptest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  UNITS 			varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  LL 				float					DEFAULT NULL,
  HL 				float					DEFAULT NULL,
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL,
  SPEC_LL			float					DEFAULT NULL,
  SPEC_HL			float					DEFAULT NULL,
  SPEC_TARGET		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_ptest_info ON ft_ptest_info (SPLITLOT_ID); 

/*!40000 ALTER TABLE ft_ptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_limits;
CREATE TABLE ft_ptest_limits (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  LL 				float					NOT NULL DEFAULT '0',
  HL 				float					NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_ptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_results;
CREATE TABLE ft_ptest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  RUN_ID 			mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VALUE 			float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_ptest_results ON ft_ptest_results (SPLITLOT_ID, PTEST_INFO_ID);

/*!40000 ALTER TABLE ft_ptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_stats_samples;
CREATE TABLE ft_ptest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE	 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_ptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ptest_stats_summary;
CREATE TABLE ft_ptest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  PTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  FAIL_COUNT	 	mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_ptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_info;
CREATE TABLE ft_mptest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  TPIN_ARRAYINDEX	smallint(6)				NOT NULL DEFAULT '0',
  UNITS 			varchar(255)			NOT NULL DEFAULT '',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  LL 				float					DEFAULT NULL,
  HL 				float					DEFAULT NULL,
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL,
  SPEC_LL			float					DEFAULT NULL,
  SPEC_HL			float					DEFAULT NULL,
  SPEC_TARGET		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_mptest_info ON ft_mptest_info (SPLITLOT_ID); 

/*!40000 ALTER TABLE ft_mptest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_info ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_limits;
CREATE TABLE ft_mptest_limits (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  LL 				float					NOT NULL DEFAULT '0',
  HL 				float					NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_mptest_limits DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_limits ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_results;
CREATE TABLE ft_mptest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  RUN_ID 			mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VALUE 			float					NOT NULL DEFAULT '0',
  TPIN_PMRINDEX		smallint(6)				DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_mptest_results ON ft_mptest_results (SPLITLOT_ID, MPTEST_INFO_ID);

/*!40000 ALTER TABLE ft_mptest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_stats_samples;
CREATE TABLE ft_mptest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_mptest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_mptest_stats_summary;
CREATE TABLE ft_mptest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  MPTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  MIN_VALUE 		float					DEFAULT NULL,
  MAX_VALUE 		float					DEFAULT NULL,
  SUM 				float					DEFAULT NULL,
  SQUARE_SUM 		float					DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_mptest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_mptest_stats_summary ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_info;
CREATE TABLE ft_ftest_info (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID		smallint(5) unsigned	NOT NULL DEFAULT '0',
  TNUM 				int(10) unsigned		NOT NULL DEFAULT '0',
  TNAME 			varchar(255)			NOT NULL DEFAULT '',
  TESTSEQ 			smallint(5) unsigned	DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_ftest_info ON ft_ftest_info (SPLITLOT_ID);

/*!40000 ALTER TABLE ft_ftest_info DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_info ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_results;
CREATE TABLE ft_ftest_results (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL,
  RUN_ID 			mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FLAGS 			binary(1)				NOT NULL DEFAULT '\0',
  VECT_NAM 			varchar(255)			NOT NULL DEFAULT '',
  VECT_OFF 			smallint(6)				DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

CREATE INDEX ft_ftest_results ON ft_ftest_results (SPLITLOT_ID, FTEST_INFO_ID);

/*!40000 ALTER TABLE ft_ftest_results DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_results ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_stats_samples;
CREATE TABLE ft_ftest_stats_samples (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	NOT NULL DEFAULT '0',
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_ftest_stats_samples DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_stats_samples ENABLE KEYS */;


DROP TABLE IF EXISTS ft_ftest_stats_summary;
CREATE TABLE ft_ftest_stats_summary (
  SPLITLOT_ID 		int(10) unsigned		NOT NULL DEFAULT '0',
  FTEST_INFO_ID 	smallint(5) unsigned	NOT NULL DEFAULT '0',
  SITE_NO 			smallint(5)				NOT NULL DEFAULT '1',
  EXEC_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  FAIL_COUNT 		mediumint(8) unsigned	DEFAULT NULL,
  TTIME 			int(10) unsigned		DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE ft_ftest_stats_summary DISABLE KEYS */;
/*!40000 ALTER TABLE ft_ftest_stats_summary ENABLE KEYS */;

--
-- PURGE PROCEDURE
--

DROP PROCEDURE IF EXISTS PURGE_INVALID_SPLITLOTS;
DELIMITER $$
CREATE PROCEDURE PURGE_INVALID_SPLITLOTS()
BEGIN
	CALL PURGE_SPLITLOTS(null, null, null);
END $$
DELIMITER ;

DROP PROCEDURE IF EXISTS PURGE_SPLITLOTS;
DELIMITER $$
CREATE PROCEDURE PURGE_SPLITLOTS
(
	SamplesNbWeeks	INT, 	# Nb of weeks before delete samples
	StatsNbWeeks	INT, 	# Nb of weeks before delete stats
	EntriesNbWeeks	INT		# Nb of weeks before delete entries
)
BEGIN
	DECLARE DateEraseEntries, DateEraseStats, DateEraseSamples INT;
	DECLARE DateSetup BIGINT;
	DECLARE HaveSplitlotIdForErase, SplitlotForErase INT;

	SELECT (SamplesNbWeeks*7) * (24*60*60) INTO DateEraseSamples;
	SELECT (StatsNbWeeks  *7) * (24*60*60) INTO DateEraseStats;
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
	SELECT max(Splitlot_ID) FROM ft_splitlot
		WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM ft_splitlot WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM ft_ptest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ftest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_mptest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_run WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ptest_limits WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ptest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ftest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ftest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_ftest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_mptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_mptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_mptest_limits WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_mptest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_sbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_sbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_hbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_hbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_sbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_hbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_parts_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_parts_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_prod_alarm WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM ft_splitlot WHERE Splitlot_Id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# FOR WT_TABLES
	# Verify if have Purge to do
	SELECT max(Splitlot_ID) FROM wt_splitlot
		WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM wt_splitlot WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM wt_ptest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ftest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_mptest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_run WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ptest_limits WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ptest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ftest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ftest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_ftest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_mptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_mptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_mptest_limits WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_mptest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_sbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_sbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_hbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_hbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_sbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_hbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_parts_stats_samples WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_parts_stats_summary WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_prod_alarm WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM wt_splitlot WHERE Splitlot_Id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# FOR ET_TABLES
	# Verify if have Purge to do
	SELECT max(Splitlot_ID) FROM et_splitlot
		WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM et_splitlot WHERE  (DateSetup > INSERTION_TIME) OR ((Valid_Splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > INSERTION_TIME ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM et_ptest_results WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_run WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_ptest_stats WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_ptest_info WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_sbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_hbin WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_prod_alarm WHERE Splitlot_Id = SplitlotForErase;
			DELETE FROM et_splitlot WHERE Splitlot_Id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# stats purge
	# verify if have stats information else skip this step
	IF NOT (StatsNbWeeks IS NULL)
	THEN
		# FOR FT_TABLES
		# Verify if have Purge to do
		SELECT UNIX_TIMESTAMP() - DateEraseStats INTO DateSetup;
		SELECT max(Splitlot_ID) FROM ft_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM ft_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ftest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_run WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ptest_limits WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ptest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ftest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ftest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ftest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_limits WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_sbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_sbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;	
				DELETE FROM ft_hbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_hbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_sbin WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_hbin WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		# FOR WT_TABLES
		# Verify if have Purge to do
		SELECT max(Splitlot_ID) FROM wt_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM wt_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ftest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_run WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ptest_limits WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ptest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ftest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ftest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ftest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_limits WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_sbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_sbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_hbin_stats_samples WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_hbin_stats_summary WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_sbin WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_hbin WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	
		# FOR E_TABLES
		# Verify if have Purge to do
		SELECT max(Splitlot_ID) FROM et_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM et_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_run WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_ptest_stats WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_ptest_info WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_sbin WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_hbin WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
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
		SELECT max(Splitlot_ID) FROM ft_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM ft_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_ftest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_mptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM ft_run WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		# FOR WT_TABLES
		# Verify if have Purge to do
		SELECT max(Splitlot_ID) FROM wt_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM wt_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_ftest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_mptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM wt_run WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	
		# FOR E_TABLES
		# Verify if have Purge to do
		SELECT max(Splitlot_ID) FROM et_splitlot
			WHERE  DateSetup > INSERTION_TIME
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT Splitlot_ID FROM et_splitlot WHERE  (DateSetup > INSERTION_TIME);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_ptest_results WHERE Splitlot_Id = SplitlotForErase;
				DELETE FROM et_run WHERE Splitlot_Id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;	
	END IF;

END $$
DELIMITER ;

DROP PROCEDURE IF EXISTS ET_INSERTION_VALIDATION;
DELIMITER $$
CREATE PROCEDURE ET_INSERTION_VALIDATION(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS ET_INSERTION_STATUS;
DELIMITER $$
CREATE PROCEDURE ET_INSERTION_STATUS(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS ET_FILEARCHIVE_SETTINGS;
DELIMITER $$
CREATE PROCEDURE ET_FILEARCHIVE_SETTINGS(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
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

DROP PROCEDURE IF EXISTS WT_INSERTION_VALIDATION;
DELIMITER $$
CREATE PROCEDURE WT_INSERTION_VALIDATION(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS WT_INSERTION_STATUS;
DELIMITER $$
CREATE PROCEDURE WT_INSERTION_STATUS(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS WT_FILEARCHIVE_SETTINGS;
DELIMITER $$
CREATE PROCEDURE WT_FILEARCHIVE_SETTINGS(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
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

DROP PROCEDURE IF EXISTS FT_INSERTION_VALIDATION;
DELIMITER $$
CREATE PROCEDURE FT_INSERTION_VALIDATION(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS FT_INSERTION_STATUS;
DELIMITER $$
CREATE PROCEDURE FT_INSERTION_STATUS(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS FT_FILEARCHIVE_SETTINGS;
DELIMITER $$
CREATE PROCEDURE FT_FILEARCHIVE_SETTINGS(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
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

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
