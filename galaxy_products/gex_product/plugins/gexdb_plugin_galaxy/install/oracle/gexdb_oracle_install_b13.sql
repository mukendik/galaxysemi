--
-- First arg is the GexAdmin name
--

DEFINE GEXADMIN=&1
DEFINE GEXADMINPWD=&2
DEFINE GEXUSER=&3
DEFINE GEXUSERPWD=&4
DEFINE LOGGINGMODE=&5

--
-- Create GEXDB tablespaces
--

CREATE TEMPORARY TABLESPACE &GEXADMIN._temp 
  TEMPFILE '&GEXADMIN._temp.dbf' SIZE 2000M REUSE AUTOEXTEND 
  ON NEXT 500M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL;

CREATE TABLESPACE &GEXADMIN. 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN..dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._wt 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._wt.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._wt_d 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._wt_d.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._wt_s 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._wt_s.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._wt_r 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._wt_r.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._ft 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._ft.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._ft_d 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._ft_d.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._ft_s 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._ft_s.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._ft_r 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._ft_r.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._et 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._et.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._et_d 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._et_d.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._et_s 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._et_s.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

CREATE TABLESPACE &GEXADMIN._et_r 
  &LOGGINGMODE. 
  DATAFILE '&GEXADMIN._et_r.dbf' SIZE 5M REUSE AUTOEXTEND 
  ON NEXT 5M MAXSIZE UNLIMITED EXTENT MANAGEMENT LOCAL 
  SEGMENT SPACE MANAGEMENT AUTO 
  DEFAULT COMPRESS;

--
-- Create &GEXADMIN. users
--
 
CREATE USER &GEXADMIN. PROFILE DEFAULT
 IDENTIFIED BY &GEXADMINPWD. 
 DEFAULT TABLESPACE &GEXADMIN.
 TEMPORARY TABLESPACE &GEXADMIN._temp 
 ACCOUNT UNLOCK;
GRANT CONNECT TO &GEXADMIN.;
GRANT RESOURCE TO &GEXADMIN.;
GRANT OEM_MONITOR TO &GEXADMIN.;
GRANT ALTER TABLESPACE TO &GEXADMIN.;
GRANT CREATE TABLESPACE TO &GEXADMIN.;
GRANT DROP TABLESPACE TO &GEXADMIN.;
GRANT MANAGE TABLESPACE TO &GEXADMIN.;
GRANT IMP_FULL_DATABASE TO &GEXADMIN.;
GRANT EXP_FULL_DATABASE TO &GEXADMIN.;
GRANT SCHEDULER_ADMIN TO &GEXADMIN.; 
ALTER USER &GEXADMIN. DEFAULT ROLE ALL;
GRANT CREATE JOB TO &GEXADMIN.;

CREATE USER &GEXUSER. PROFILE DEFAULT
 IDENTIFIED BY &GEXUSERPWD. 
 DEFAULT TABLESPACE &GEXADMIN.
 TEMPORARY TABLESPACE &GEXADMIN._temp
 ACCOUNT UNLOCK;
GRANT CONNECT TO &GEXUSER.;
GRANT OEM_MONITOR TO &GEXUSER.;
GRANT EXP_FULL_DATABASE TO &GEXUSER.;

--
-- Create GEXDB tables and indexes
--

--
-- Table structure for table global_info
--

CREATE TABLE &GEXADMIN..global_info (
 db_version_name   VARCHAR2(255) NOT NULL,
 db_version_nb   NUMBER(6)  NOT NULL,
 db_version_build  NUMBER(6)  NOT NULL,
 incremental_splitlots NUMBER(10)  NOT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

INSERT INTO &GEXADMIN..global_info VALUES('GEXDB V1.8 B13 (Oracle)', 18, 13, 0);

--
-- Table structure for table global_options
--

CREATE TABLE &GEXADMIN..global_options (
 option_name   VARCHAR2(255) NOT NULL,
 option_value   VARCHAR2(255) NOT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table gexdb_log
--

CREATE TABLE &GEXADMIN..gexdb_log (
 log_date   DATE NOT NULL,
 log_type   VARCHAR2(255)  NOT NULL,
 log_string  VARCHAR2(1024)  NOT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table incremental_update
--

CREATE TABLE &GEXADMIN..incremental_update (
  db_update_name  VARCHAR2(255) NOT NULL,
  initial_splitlots  NUMBER(10)  DEFAULT 0 NOT NULL,
  remaining_splitlots NUMBER(10)  DEFAULT 0 NOT NULL,
  db_version_build  NUMBER(6)  NOT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table product
--

CREATE TABLE &GEXADMIN..product (
 product_name  VARCHAR2(255) DEFAULT '' NOT NULL,
 description   VARCHAR2(1000) DEFAULT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_dtr
--

CREATE TABLE &GEXADMIN..et_dtr (
  splitlot_id NUMBER(10) NOT NULL,
  run_id NUMBER(5) DEFAULT NULL,
  order_id NUMBER(8) NOT NULL,
  dtr_type VARCHAR2(255) NOT NULL,
  dtr_text VARCHAR2(255) DEFAULT '' NOT NULL
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_dtr ON &GEXADMIN..et_dtr (splitlot_id);

--
-- Table structure for table file_host
--

CREATE TABLE &GEXADMIN..file_host (
 file_host_id  NUMBER(10)  NOT NULL,
 host_name   VARCHAR2(255) DEFAULT '',
 host_ftpuser  VARCHAR2(255) DEFAULT '',
 host_ftppassword VARCHAR2(255) DEFAULT '',
 host_ftppath  VARCHAR2(255) DEFAULT NULL,
 host_ftpport  NUMBER(4)  DEFAULT 21 NOT NULL,
 CONSTRAINT pk_file_host PRIMARY KEY (file_host_id)
) TABLESPACE &GEXADMIN. PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- ELECTRICAL TEST
--

--
-- Table structure for table et_sya_set
--

CREATE TABLE &GEXADMIN..et_sya_set (
 sya_id     NUMBER(10)   NOT NULL,
 product_id    VARCHAR2(255)  NOT NULL,
 CREATION_DATE   DATE    NOT NULL,
 user_comment   VARCHAR2(255)  DEFAULT NULL,
 ll_1     NUMBER    DEFAULT -1 NOT NULL,
 hl_1     NUMBER    DEFAULT -1 NOT NULL,
 ll_2     NUMBER    DEFAULT -1 NOT NULL,
 hl_2     NUMBER    DEFAULT -1 NOT NULL,
 start_date    DATE    NOT NULL,
 expiration_date   DATE    NOT NULL,
 expiration_email_date DATE    DEFAULT NULL,
 rule_type    VARCHAR2(255)  NOT NULL,
 n1_parameter   NUMBER    DEFAULT -1 NOT NULL,
 n2_parameter   NUMBER    DEFAULT -1 NOT NULL,
 computation_fromdate DATE    NOT NULL,
 computation_todate  DATE    NOT NULL,
 min_lots_required  NUMBER(5)   DEFAULT -1 NOT NULL,
 min_data_points   NUMBER(5)   DEFAULT -1 NOT NULL,
 options     NUMBER(3)   DEFAULT 0 NOT NULL,
 flags     NUMBER(3)   DEFAULT 0 NOT NULL,
 rule_name   VARCHAR2(255)  DEFAULT NULL,
 CONSTRAINT pk_et_sya_set PRIMARY KEY(sya_id)
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table et_sbl
--

CREATE TABLE &GEXADMIN..et_sbl (
 sya_id     NUMBER(10)  NOT NULL,
 bin_no     NUMBER(5)  NOT NULL,
 bin_name    VARCHAR2(255) DEFAULT NULL,
 ll_1     NUMBER   DEFAULT -1 NOT NULL,
 hl_1     NUMBER   DEFAULT -1 NOT NULL,
 ll_2     NUMBER   DEFAULT -1 NOT NULL,
 hl_2     NUMBER   DEFAULT -1 NOT NULL,
 rule_type    VARCHAR2(255)  DEFAULT NULL,
 n1_parameter   NUMBER    DEFAULT NULL,
 n2_parameter   NUMBER    DEFAULT NULL,
 CONSTRAINT pk_et_sbl PRIMARY KEY(sya_id,bin_no),
 CONSTRAINT fk_et_sbl FOREIGN KEY(sya_id) REFERENCES &GEXADMIN..et_sya_set(sya_id)
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_prod_alarm
--

CREATE TABLE &GEXADMIN..et_prod_alarm (
  splitlot_id   NUMBER(10)  NOT NULL,
  alarm_cat    VARCHAR2(255) NOT NULL,
  alarm_type   VARCHAR2(255) NOT NULL,
  item_no    NUMBER   NOT NULL,
  item_name    VARCHAR2(255) DEFAULT NULL,
  flags     CHAR(2)   NOT NULL,
  lcl     NUMBER   DEFAULT 0 NOT NULL,
  ucl     NUMBER   DEFAULT 0 NOT NULL,
  value     NUMBER   DEFAULT 0 NOT NULL,
  units     VARCHAR2(10) DEFAULT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_product_hbin
--

CREATE TABLE &GEXADMIN..et_product_hbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no     NUMBER   NOT NULL,
  bin_name   VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   DEFAULT NULL,
  nb_parts    NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_product_hbin ON &GEXADMIN..et_product_hbin (product_name,bin_no);


--
-- Table structure for table et_product_sbin
--

CREATE TABLE &GEXADMIN..et_product_sbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no     NUMBER   NOT NULL,
  bin_name   VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   DEFAULT NULL,
  nb_parts    NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_product_sbin ON &GEXADMIN..et_product_sbin (product_name,bin_no);

--
-- Table structure for table et_wyr
--

CREATE TABLE &GEXADMIN..et_wyr (
 site_name    VARCHAR2(255) NOT NULL,
 week_nb     NUMBER(3)  DEFAULT NULL,
 year     NUMBER(5)  DEFAULT NULL,
 date_in     DATE   DEFAULT NULL,
 date_out    DATE   DEFAULT NULL,
 product_name   VARCHAR2(255) DEFAULT NULL,
 program_name   VARCHAR2(255) DEFAULT NULL,
 tester_name    VARCHAR2(255) DEFAULT NULL,
 lot_id     VARCHAR2(255) DEFAULT NULL,
 subcon_lot_id   VARCHAR2(255) DEFAULT NULL,
 user_split    VARCHAR2(1024) DEFAULT NULL,
 yield     NUMBER   DEFAULT 0,
 parts_received   NUMBER(10)  DEFAULT 0,
 pretest_rejects   NUMBER(10)  DEFAULT 0,
 pretest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_tested   NUMBER(10)  DEFAULT 0,
 parts_pass    NUMBER(10)  DEFAULT 0,
 parts_pass_split  VARCHAR2(1024) DEFAULT NULL,
 parts_fail    NUMBER(10)  DEFAULT 0,
 parts_fail_split  VARCHAR2(1024) DEFAULT NULL,
 parts_retest   NUMBER(10)  DEFAULT 0,
 parts_retest_split  VARCHAR2(1024) DEFAULT NULL,
 insertions    NUMBER(10)  DEFAULT 0,
 posttest_rejects  NUMBER(10)  DEFAULT 0,
 posttest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_shipped   NUMBER(10)  DEFAULT 0
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_wyr_format
--

CREATE TABLE &GEXADMIN..et_wyr_format (
 site_name    VARCHAR2(255) NOT NULL,
 column_id    NUMBER(3)  NOT NULL,
 column_nb    NUMBER(3)  NOT NULL,
 column_name    VARCHAR2(255) NOT NULL,
 data_type    VARCHAR2(255) NOT NULL,
 display    CHAR(1)  DEFAULT 'Y' NOT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_metadata_mapping
--

CREATE TABLE &GEXADMIN..et_metadata_mapping (
 meta_name    VARCHAR2(255) NOT NULL,
 gex_name    VARCHAR2(255) DEFAULT NULL,
 gexdb_table_name  VARCHAR2(255) NOT NULL,
 gexdb_field_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 gex_display_in_gui  CHAR(1)   DEFAULT 'Y' NOT NULL,
  bintype_field char(1) DEFAULT 'N' NOT NULL,
  time_field char(1) DEFAULT 'N' NOT NULL,
  custom_field char(1) DEFAULT 'Y' NOT NULL,
  numeric_field char(1) DEFAULT 'N' NOT NULL,
  fact_field char(1) DEFAULT 'N' NOT NULL,
  consolidated_field char(1)  DEFAULT 'Y'NOT NULL,
  er_display_in_gui char(1) DEFAULT 'Y' NOT NULL,
  az_field char(1) DEFAULT 'N' NOT NULL,
 CONSTRAINT pk_et_metadata_mapping PRIMARY KEY (meta_name),
 CONSTRAINT uk1_et_metadata_mapping UNIQUE (gex_name),
 CONSTRAINT uk2_et_metadata_mapping UNIQUE (gexdb_field_fullname)
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_metadata_link
--

CREATE TABLE &GEXADMIN..et_metadata_link (
 link_name    VARCHAR2(255) NOT NULL,
 gexdb_table1_name  VARCHAR2(255) NOT NULL,
 gexdb_field1_fullname VARCHAR2(255) NOT NULL,
 gexdb_table2_name  VARCHAR2(255) NOT NULL,
 gexdb_field2_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 CONSTRAINT pk_et_metadata_link PRIMARY KEY (link_name),
 CONSTRAINT uk1_et_metadata_link UNIQUE (gexdb_field1_fullname,gexdb_field2_fullname)
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table et_lot
--

CREATE TABLE &GEXADMIN..et_lot (
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 tracking_lot_id  VARCHAR2(255) DEFAULT NULL,
 product_name  VARCHAR2(255) DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(2)   DEFAULT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_lot ON &GEXADMIN..et_lot (lot_id);

CREATE INDEX &GEXADMIN..et_lot_product ON &GEXADMIN..et_lot (product_name,lot_id);

--
-- Table structure for table et_lot_hbin
--

CREATE TABLE &GEXADMIN..et_lot_hbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 hbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 hbin_name    VARCHAR2(255)   DEFAULT '',
 hbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_lot_hbin ON &GEXADMIN..et_lot_hbin (lot_id);

--
-- Table structure for table et_lot_sbin
--

CREATE TABLE &GEXADMIN..et_lot_sbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 sbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 sbin_name    VARCHAR2(255)   DEFAULT '',
 sbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_lot_sbin ON &GEXADMIN..et_lot_sbin (lot_id);

--
-- Table structure for table et_splitlot
--

CREATE TABLE &GEXADMIN..et_splitlot (
 splitlot_id   NUMBER(10)  NOT NULL,
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 sublot_id   VARCHAR2(255) DEFAULT '',
 start_t    NUMBER(10)  DEFAULT 0 NOT NULL,
 finish_t   NUMBER(10)  DEFAULT 0 NOT NULL,
 tester_name   VARCHAR2(255) DEFAULT '',
 tester_type   VARCHAR2(255) DEFAULT '',
 flags    CHAR(2)   DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 data_provider  VARCHAR2(255) DEFAULT '',
 data_type   VARCHAR2(255) DEFAULT '',
 prod_data   CHAR(1)   DEFAULT 'Y' NOT NULL,
 job_nam    VARCHAR2(255) DEFAULT NULL,
 job_rev    VARCHAR2(255) DEFAULT NULL,
 oper_nam   VARCHAR2(255) DEFAULT NULL,
 exec_typ   VARCHAR2(255) DEFAULT NULL,
 exec_ver   VARCHAR2(255) DEFAULT NULL,
 facil_id   VARCHAR2(255) DEFAULT NULL,
 part_typ   VARCHAR2(256) DEFAULT NULL, 
 user_txt   VARCHAR2(256) DEFAULT NULL, 
 famly_id   VARCHAR2(256) DEFAULT NULL, 
 proc_id    VARCHAR2(256) DEFAULT NULL,
 file_host_id  NUMBER(10),
 file_path   VARCHAR2(255) DEFAULT '',
 file_name   VARCHAR2(255) DEFAULT '',
 valid_splitlot  CHAR(1)   DEFAULT 'N' NOT NULL,
 insertion_time  NUMBER(10)  DEFAULT NULL,
 subcon_lot_id  VARCHAR2(255) DEFAULT '',
 wafer_id    VARCHAR2(255) DEFAULT NULL,
 incremental_update  VARCHAR2(255) DEFAULT NULL,
 sya_id    NUMBER(10)  DEFAULT 0,
 day VARCHAR2(10) NOT NULL,
 week_nb NUMBER(2) NOT NULL,
 month_nb NUMBER(2) NOT NULL,
 quarter_nb NUMBER(1) NOT NULL,
 year_nb NUMBER(4) NOT NULL,
 year_and_week VARCHAR2(7) NOT NULL,
 year_and_month VARCHAR2(7) NOT NULL,
 year_and_quarter VARCHAR2(7) NOT NULL,
 wafer_nb   NUMBER(3)  DEFAULT NULL,
 site_config   VARCHAR2(255) DEFAULT NULL,
 CONSTRAINT pk_et_splitlot PRIMARY KEY (splitlot_id)
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_splitlot ON &GEXADMIN..et_splitlot (lot_id) LOCAL COMPRESS;
CREATE INDEX &GEXADMIN..et_splitlot_start ON &GEXADMIN..et_splitlot (start_t) LOCAL COMPRESS;

--
-- Table structure for table et_wafer_info
--

CREATE TABLE &GEXADMIN..et_wafer_info (
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 wafer_id   VARCHAR2(255) DEFAULT NULL,
 fab_id    VARCHAR2(255) DEFAULT NULL,
 frame_id   VARCHAR2(255) DEFAULT NULL,
 mask_id    VARCHAR2(255) DEFAULT NULL,
 wafer_size   NUMBER   DEFAULT NULL,
 die_ht    NUMBER   DEFAULT NULL,
 die_wid    NUMBER   DEFAULT NULL,
 wafer_units   NUMBER(3)  DEFAULT NULL,
 wafer_flat   CHAR(1)   DEFAULT NULL,
 center_x   NUMBER(5)  DEFAULT NULL,
 center_y   NUMBER(5)  DEFAULT NULL,
 pos_x    CHAR(1)   DEFAULT NULL,
 pos_y    CHAR(1)   DEFAULT NULL,
 gross_die   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(2)   DEFAULT 0,
 wafer_nb   NUMBER(3)  DEFAULT NULL,
 site_config   VARCHAR2(255) DEFAULT NULL
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_wafer_info ON &GEXADMIN..et_wafer_info (lot_id, wafer_id);

--
-- Table structure for table et_wafer_hbin
--

CREATE TABLE &GEXADMIN..et_wafer_hbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 wafer_id   VARCHAR2(255)   DEFAULT NULL,
 hbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 hbin_name    VARCHAR2(255)   DEFAULT '',
 hbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_wafer_hbin ON &GEXADMIN..et_wafer_hbin (lot_id, wafer_id); 

--
-- Table structure for table et_wafer_sbin
--

CREATE TABLE &GEXADMIN..et_wafer_sbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 wafer_id   VARCHAR2(255)   DEFAULT NULL,
 sbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 sbin_name    VARCHAR2(255)   DEFAULT '',
 sbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._et PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..et_wafer_sbin ON &GEXADMIN..et_wafer_sbin (lot_id, wafer_id);

--
-- Table structure for table et_hbin
--

CREATE TABLE &GEXADMIN..et_hbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 hbin_name   VARCHAR2(255) DEFAULT '',
 hbin_cat   CHAR(1)   DEFAULT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_hbin ON &GEXADMIN..et_hbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table et_sbin
--

CREATE TABLE &GEXADMIN..et_sbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_name   VARCHAR2(255) DEFAULT '',
 sbin_cat   CHAR(1)   DEFAULT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_sbin ON &GEXADMIN..et_sbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table et_run
--

CREATE TABLE &GEXADMIN..et_run (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(5)  DEFAULT 0 NOT NULL,
 part_id    VARCHAR2(255) DEFAULT NULL,
 part_x    NUMBER(6)  DEFAULT NULL,
 part_y    NUMBER(6)  DEFAULT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL,
 tests_executed  NUMBER(5)  DEFAULT 0 NOT NULL,
 tests_failed  NUMBER(5)  DEFAULT 0 NOT NULL,
 firstfail_tnum  NUMBER(10)  DEFAULT NULL,
 firstfail_tname  VARCHAR2(255) DEFAULT NULL,
 retest_index  NUMBER(3)  DEFAULT 0 NOT NULL,
 site_no NUMBER(5) DEFAULT 1 NOT NULL,
 part_txt    VARCHAR2(255) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_run ON &GEXADMIN..et_run (splitlot_id) LOCAL COMPRESS;


--
-- Table structure for table et_ptest_info
--

CREATE TABLE &GEXADMIN..et_ptest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 units    VARCHAR2(255) DEFAULT '',
 flags    CHAR(1)   DEFAULT NULL,
 ll     NUMBER   DEFAULT NULL,
 hl     NUMBER   DEFAULT NULL,
 testseq    NUMBER(5)  DEFAULT NULL,
 spec_ll    NUMBER   DEFAULT NULL,
 spec_hl    NUMBER   DEFAULT NULL,
 spec_target   NUMBER   DEFAULT NULL,
  res_scal NUMBER(3) DEFAULT NULL,
  ll_scal NUMBER(3) DEFAULT NULL,
  hl_scal NUMBER(3) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_ptest_info ON &GEXADMIN..et_ptest_info (splitlot_id,ptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table et_ptest_stats
--

CREATE TABLE &GEXADMIN..et_ptest_stats (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 exec_count   NUMBER(5)  DEFAULT NULL,
 fail_count   NUMBER(5)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..et_ptest_stats ON &GEXADMIN..et_ptest_stats (splitlot_id,ptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table et_ptest_results
--

CREATE TABLE &GEXADMIN..et_ptest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(5)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 value    NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._et_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);


CREATE INDEX &GEXADMIN..et_ptest_results ON &GEXADMIN..et_ptest_results (splitlot_id, ptest_info_id) LOCAL COMPRESS;

--
-- WAFER SORT
--

--
-- Table structure for table wt_sya_set
--

CREATE TABLE &GEXADMIN..wt_sya_set (
 sya_id     NUMBER(10)   NOT NULL,
 product_id    VARCHAR2(255)  NOT NULL,
 creation_date   DATE    NOT NULL,
 user_comment   VARCHAR2(255)  DEFAULT NULL,
 ll_1     NUMBER    DEFAULT -1 NOT NULL,
 hl_1     NUMBER    DEFAULT -1 NOT NULL,
 ll_2     NUMBER    DEFAULT -1 NOT NULL,
 hl_2     NUMBER    DEFAULT -1 NOT NULL,
 start_date    DATE    NOT NULL,
 expiration_date   DATE    NOT NULL,
 expiration_email_date DATE    DEFAULT NULL,
 rule_type    VARCHAR2(255)  NOT NULL,
 n1_parameter   NUMBER    DEFAULT -1 NOT NULL,
 n2_parameter   NUMBER    DEFAULT -1 NOT NULL,
 computation_fromdate DATE    NOT NULL,
 computation_todate  DATE    NOT NULL,
 min_lots_required  NUMBER(5)   DEFAULT -1 NOT NULL,
 min_data_points   NUMBER(5)   DEFAULT -1 NOT NULL,
 options     NUMBER(3)   DEFAULT 0 NOT NULL,
 flags     NUMBER(3)   DEFAULT 0 NOT NULL,
 rule_name   VARCHAR2(255)  DEFAULT NULL,
 CONSTRAINT pk_wt_sya_set PRIMARY KEY(sya_id)
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table wt_sbl
--

CREATE TABLE &GEXADMIN..wt_sbl (
 sya_id     NUMBER(10)  NOT NULL,
 bin_no     NUMBER(5)  NOT NULL,
 bin_name    VARCHAR2(255) DEFAULT NULL,
 ll_1     NUMBER   DEFAULT -1 NOT NULL,
 hl_1     NUMBER   DEFAULT -1 NOT NULL,
 ll_2     NUMBER   DEFAULT -1 NOT NULL,
 hl_2     NUMBER   DEFAULT -1 NOT NULL,
 rule_type    VARCHAR2(255)  DEFAULT NULL,
 n1_parameter   NUMBER    DEFAULT NULL,
 n2_parameter   NUMBER    DEFAULT NULL,
 CONSTRAINT pk_wt_sbl PRIMARY KEY(sya_id,bin_no),
 CONSTRAINT fk_wt_sbl FOREIGN KEY(sya_id) REFERENCES &GEXADMIN..wt_sya_set(sya_id)
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table wt_prod_alarm
--

CREATE TABLE &GEXADMIN..wt_prod_alarm (
  splitlot_id   NUMBER(10)  NOT NULL,
  alarm_cat    VARCHAR2(255) NOT NULL,
  alarm_type   VARCHAR2(255) NOT NULL,
  item_no    NUMBER   NOT NULL,
  item_name    VARCHAR2(255) DEFAULT NULL,
  flags     CHAR(2)   NOT NULL,
  lcl     NUMBER   DEFAULT 0 NOT NULL,
  ucl     NUMBER   DEFAULT 0 NOT NULL,
  value     NUMBER   DEFAULT 0 NOT NULL,
  units     VARCHAR2(10) DEFAULT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table wt_product_hbin
--

CREATE TABLE &GEXADMIN..wt_product_hbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no    NUMBER(5)   NOT NULL,
  bin_name    VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   NOT NULL,
  nb_parts     NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..wt_product_hbin ON &GEXADMIN..wt_product_hbin (product_name, bin_no);


--
-- Table structure for table wt_product_sbin
--

CREATE TABLE &GEXADMIN..wt_product_sbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no    NUMBER(5)   NOT NULL,
  bin_name    VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   NOT NULL,
  nb_parts     NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..wt_product_sbin ON &GEXADMIN..wt_product_sbin (product_name, bin_no);

--
-- Table structure for table wt_wyr
--

CREATE TABLE &GEXADMIN..wt_wyr (
 site_name    VARCHAR2(255) NOT NULL,
 week_nb     NUMBER(3)  DEFAULT NULL,
 year     NUMBER(5)  DEFAULT NULL,
 date_in     DATE   DEFAULT NULL,
 date_out    DATE   DEFAULT NULL,
 product_name   VARCHAR2(255) DEFAULT NULL,
 program_name   VARCHAR2(255) DEFAULT NULL,
 tester_name    VARCHAR2(255) DEFAULT NULL,
 lot_id     VARCHAR2(255) DEFAULT NULL,
 subcon_lot_id   VARCHAR2(255) DEFAULT NULL,
 user_split    VARCHAR2(1024) DEFAULT NULL,
 yield     NUMBER   DEFAULT 0,
 parts_received   NUMBER(10)  DEFAULT 0,
 pretest_rejects   NUMBER(10)  DEFAULT 0,
 pretest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_tested   NUMBER(10)  DEFAULT 0,
 parts_pass    NUMBER(10)  DEFAULT 0,
 parts_pass_split  VARCHAR2(1024) DEFAULT NULL,
 parts_fail    NUMBER(10)  DEFAULT 0,
 parts_fail_split  VARCHAR2(1024) DEFAULT NULL,
 parts_retest   NUMBER(10)  DEFAULT 0,
 parts_retest_split  VARCHAR2(1024) DEFAULT NULL,
 insertions    NUMBER(10)  DEFAULT 0,
 posttest_rejects  NUMBER(10)  DEFAULT 0,
 posttest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_shipped   NUMBER(10)  DEFAULT 0
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table wt_wyr_format
--

CREATE TABLE &GEXADMIN..wt_wyr_format (
 site_name    VARCHAR2(255) NOT NULL,
 column_id    NUMBER(3)  NOT NULL,
 column_nb    NUMBER(3)  NOT NULL,
 column_name    VARCHAR2(255) NOT NULL,
 data_type    VARCHAR2(255) NOT NULL,
 display     CHAR(1)   DEFAULT 'Y' NOT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table wt_metadata_mapping
--

CREATE TABLE &GEXADMIN..wt_metadata_mapping (
 meta_name    VARCHAR2(255) NOT NULL,
 gex_name    VARCHAR2(255) DEFAULT NULL,
 gexdb_table_name  VARCHAR2(255) NOT NULL,
 gexdb_field_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 gex_display_in_gui  CHAR(1)   DEFAULT 'Y' NOT NULL,
  bintype_field char(1) DEFAULT 'N' NOT NULL,
  time_field char(1) DEFAULT 'N' NOT NULL,
  custom_field char(1) DEFAULT 'Y' NOT NULL,
  numeric_field char(1) DEFAULT 'N' NOT NULL,
  fact_field char(1) DEFAULT 'N' NOT NULL,
  consolidated_field char(1) DEFAULT 'Y' NOT NULL,
  er_display_in_gui char(1) DEFAULT 'Y' NOT NULL,
  az_field char(1) DEFAULT 'N' NOT NULL,
 CONSTRAINT pk_wt_metadata_mapping PRIMARY KEY (meta_name),
 CONSTRAINT uk1_wt_metadata_mapping UNIQUE (gex_name),
 CONSTRAINT uk2_wt_metadata_mapping UNIQUE (gexdb_field_fullname)
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table wt_metadata_link
--

CREATE TABLE &GEXADMIN..wt_metadata_link (
 link_name    VARCHAR2(255) NOT NULL,
 gexdb_table1_name  VARCHAR2(255) NOT NULL,
 gexdb_field1_fullname VARCHAR2(255) NOT NULL,
 gexdb_table2_name  VARCHAR2(255) NOT NULL,
 gexdb_field2_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 CONSTRAINT pk_wt_metadata_link PRIMARY KEY (link_name),
 CONSTRAINT uk1_wt_metadata_link UNIQUE (gexdb_field1_fullname,gexdb_field2_fullname)
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table wt_lot
--

CREATE TABLE &GEXADMIN..wt_lot (
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 tracking_lot_id  VARCHAR2(255) DEFAULT NULL,
 product_name  VARCHAR2(255) DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(2)   DEFAULT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..wt_lot ON &GEXADMIN..wt_lot (lot_id);
CREATE INDEX &GEXADMIN..wt_lot_product ON &GEXADMIN..wt_lot (product_name,lot_id);

--
-- Table structure for table wt_lot_hbin
--

CREATE TABLE &GEXADMIN..wt_lot_hbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 hbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 hbin_name    VARCHAR2(255)   DEFAULT '',
 hbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..wt_lot_hbin ON &GEXADMIN..wt_lot_hbin (lot_id);

--
-- Table structure for table wt_lot_sbin
--

CREATE TABLE &GEXADMIN..wt_lot_sbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 sbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 sbin_name    VARCHAR2(255)   DEFAULT '',
 sbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..wt_lot_sbin ON &GEXADMIN..wt_lot_sbin (lot_id);


--
-- Table structure for table wt_splitlot
--

CREATE TABLE &GEXADMIN..wt_splitlot (
 splitlot_id   NUMBER(10)  NOT NULL,
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 sublot_id   VARCHAR2(255) DEFAULT '',
 setup_t    NUMBER(10)  DEFAULT 0 NOT NULL,
 start_t    NUMBER(10)  DEFAULT 0 NOT NULL,
 finish_t   NUMBER(10)  DEFAULT 0 NOT NULL,
 stat_num   NUMBER(3)  DEFAULT 0 NOT NULL,
 tester_name   VARCHAR2(255) DEFAULT '',
 tester_type   VARCHAR2(255) DEFAULT '',
 flags    CHAR(2)   DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_samples NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_samples_good NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_summary NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_summary_good NUMBER(8)  DEFAULT 0 NOT NULL,
 data_provider  VARCHAR2(255) DEFAULT '',
 data_type   VARCHAR2(255) DEFAULT '',
 prod_data   CHAR(1)   DEFAULT 'Y' NOT NULL,
 retest_index  NUMBER(3)  DEFAULT 0 NOT NULL,
 retest_hbins  VARCHAR2(255) DEFAULT NULL,
 rework_code   NUMBER(3)  DEFAULT 0 NOT NULL,
 job_nam    VARCHAR2(255) DEFAULT NULL,
 job_rev    VARCHAR2(255) DEFAULT NULL,
 oper_nam   VARCHAR2(255) DEFAULT NULL,
 exec_typ   VARCHAR2(255) DEFAULT NULL,
 exec_ver   VARCHAR2(255) DEFAULT NULL,
 test_cod   VARCHAR2(255) DEFAULT NULL,
 facil_id   VARCHAR2(255) DEFAULT NULL,
 tst_temp   VARCHAR2(255) DEFAULT NULL,
 mode_cod   CHAR(1)   DEFAULT NULL, 
 rtst_cod   CHAR(1)   DEFAULT NULL, 
 prot_cod   CHAR(1)   DEFAULT NULL, 
 burn_tim   NUMBER(5)  DEFAULT NULL, 
 cmod_cod   CHAR(1)   DEFAULT NULL, 
 part_typ   VARCHAR2(256) DEFAULT NULL, 
 user_txt   VARCHAR2(256) DEFAULT NULL, 
 aux_file   VARCHAR2(256) DEFAULT NULL, 
 pkg_typ    VARCHAR2(256) DEFAULT NULL,
 famly_id   VARCHAR2(256) DEFAULT NULL, 
 date_cod   VARCHAR2(256) DEFAULT NULL, 
 floor_id   VARCHAR2(256) DEFAULT NULL, 
 proc_id    VARCHAR2(256) DEFAULT NULL,
 oper_frq   VARCHAR2(256) DEFAULT NULL, 
 spec_nam   VARCHAR2(256) DEFAULT NULL, 
 spec_ver   VARCHAR2(256) DEFAULT NULL,
 flow_id    VARCHAR2(256) DEFAULT NULL,
 setup_id   VARCHAR2(256) DEFAULT NULL,
 dsgn_rev   VARCHAR2(256) DEFAULT NULL,
 eng_id    VARCHAR2(256) DEFAULT NULL,
 rom_cod    VARCHAR2(256) DEFAULT NULL,
 serl_num   VARCHAR2(256) DEFAULT NULL,
 supr_nam   VARCHAR2(256) DEFAULT NULL,
 nb_sites   NUMBER(3)  DEFAULT 1 NOT NULL,
 head_num   NUMBER(3)  DEFAULT NULL,
 handler_typ   VARCHAR2(255) DEFAULT NULL,
 handler_id   VARCHAR2(255) DEFAULT NULL,
 card_typ   VARCHAR2(255) DEFAULT NULL,
 card_id    VARCHAR2(255) DEFAULT NULL,
 loadboard_typ  VARCHAR2(255) DEFAULT NULL,
 loadboard_id  VARCHAR2(255) DEFAULT NULL,
 dib_typ    VARCHAR2(255) DEFAULT NULL,
 dib_id    VARCHAR2(255) DEFAULT NULL,
 cable_typ   VARCHAR2(255) DEFAULT NULL,
 cable_id   VARCHAR2(255) DEFAULT NULL,
 contactor_typ  VARCHAR2(255) DEFAULT NULL,
 contactor_id  VARCHAR2(255) DEFAULT NULL,
 laser_typ   VARCHAR2(255) DEFAULT NULL,
 laser_id   VARCHAR2(255) DEFAULT NULL,
 extra_typ   VARCHAR2(255) DEFAULT NULL,
 extra_id   VARCHAR2(255) DEFAULT NULL,
 file_host_id  NUMBER(10)  DEFAULT 0,
 file_path   VARCHAR2(255) DEFAULT '',
 file_name   VARCHAR2(255) DEFAULT '',
 valid_splitlot  CHAR(1)   DEFAULT 'N' NOT NULL,
 insertion_time  NUMBER(10)  DEFAULT NULL,
 subcon_lot_id  VARCHAR2(255) DEFAULT '',
 wafer_id    VARCHAR2(255) DEFAULT NULL,
 incremental_update  VARCHAR2(255) DEFAULT NULL,
 sya_id    NUMBER(10)  DEFAULT 0,
 day VARCHAR2(10) NOT NULL,
 week_nb NUMBER(2) NOT NULL,
 month_nb NUMBER(2) NOT NULL,
 quarter_nb NUMBER(1) NOT NULL,
 year_nb NUMBER(4) NOT NULL,
 year_and_week VARCHAR2(7) NOT NULL,
 year_and_month VARCHAR2(7) NOT NULL,
 year_and_quarter VARCHAR2(7) NOT NULL,
 wafer_nb   NUMBER(3)  DEFAULT NULL,
 CONSTRAINT pk_wt_splitlot PRIMARY KEY (splitlot_id)
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_splitlot ON &GEXADMIN..wt_splitlot (lot_id) LOCAL COMPRESS;
CREATE INDEX &GEXADMIN..wt_splitlot_start ON &GEXADMIN..wt_splitlot (start_t) LOCAL COMPRESS;

--
-- Table structure for table wt_wafer_info
--

CREATE TABLE &GEXADMIN..wt_wafer_info (
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 wafer_id   VARCHAR2(255) DEFAULT NULL,
 fab_id    VARCHAR2(255) DEFAULT NULL,
 frame_id   VARCHAR2(255) DEFAULT NULL,
 mask_id    VARCHAR2(255) DEFAULT NULL,
 wafer_size   NUMBER   DEFAULT NULL,
 die_ht    NUMBER   DEFAULT NULL,
 die_wid    NUMBER   DEFAULT NULL,
 wafer_units   NUMBER(3)  DEFAULT NULL,
 wafer_flat   CHAR(1)   DEFAULT NULL,
 center_x   NUMBER(5)  DEFAULT NULL,
 center_y   NUMBER(5)  DEFAULT NULL,
 pos_x    CHAR(1)   DEFAULT NULL,
 pos_y    CHAR(1)   DEFAULT NULL,
 gross_die   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(2)   DEFAULT 0,
 wafer_nb   NUMBER(3)  DEFAULT NULL
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..wt_wafer_info ON &GEXADMIN..wt_wafer_info (lot_id, wafer_id);

--
-- Table structure for table wt_wafer_hbin
--

CREATE TABLE &GEXADMIN..wt_wafer_hbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 wafer_id   VARCHAR2(255)   DEFAULT NULL,
 hbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 hbin_name    VARCHAR2(255)   DEFAULT '',
 hbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..wt_wafer_hbin ON &GEXADMIN..wt_wafer_hbin (lot_id, wafer_id); 

--
-- Table structure for table wt_wafer_sbin
--

CREATE TABLE &GEXADMIN..wt_wafer_sbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 wafer_id   VARCHAR2(255) DEFAULT NULL,
 sbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 sbin_name    VARCHAR2(255)   DEFAULT '',
 sbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._wt PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..wt_wafer_sbin ON &GEXADMIN..wt_wafer_sbin (lot_id, wafer_id);

--
-- Table structure for table wt_parts_stats_samples
--

CREATE TABLE &GEXADMIN..wt_parts_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_parts_stats_samples ON &GEXADMIN..wt_parts_stats_samples (splitlot_id, site_no) LOCAL COMPRESS; 

--
-- Table structure for table wt_parts_stats_summary
--

CREATE TABLE &GEXADMIN..wt_parts_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_good    NUMBER(8)  DEFAULT NULL,
 nb_rtst    NUMBER(8)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_parts_stats_summary ON &GEXADMIN..wt_parts_stats_summary (splitlot_id, site_no) LOCAL COMPRESS;

--
-- Table structure for table wt_hbin
--

CREATE TABLE &GEXADMIN..wt_hbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 hbin_name   VARCHAR2(255) DEFAULT '',
 hbin_cat   CHAR(1)   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_hbin ON &GEXADMIN..wt_hbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table wt_hbin_stats_samples
--

CREATE TABLE &GEXADMIN..wt_hbin_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_hbin_stats_samples ON &GEXADMIN..wt_hbin_stats_samples (splitlot_id, site_no, hbin_no) LOCAL COMPRESS;

--
-- Table structure for table wt_hbin_stats_summary
--

CREATE TABLE &GEXADMIN..wt_hbin_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_hbin_stats_summary ON &GEXADMIN..wt_hbin_stats_summary (splitlot_id, site_no, hbin_no) LOCAL COMPRESS; 

--
-- Table structure for table wt_sbin
--

CREATE TABLE &GEXADMIN..wt_sbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_name   VARCHAR2(255) DEFAULT '',
 sbin_cat   CHAR(1)   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_sbin ON &GEXADMIN..wt_sbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table wt_sbin_stats_samples
--

CREATE TABLE &GEXADMIN..wt_sbin_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_sbin_stats_samples ON &GEXADMIN..wt_sbin_stats_samples (splitlot_id, site_no, sbin_no) LOCAL COMPRESS;

--
-- Table structure for table wt_sbin_stats_summary
--

CREATE TABLE &GEXADMIN..wt_sbin_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_sbin_stats_summary ON &GEXADMIN..wt_sbin_stats_summary (splitlot_id, site_no, sbin_no) LOCAL COMPRESS;

--
-- Table structure for table wt_run
--

CREATE TABLE &GEXADMIN..wt_run (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(7)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 part_id    VARCHAR2(255) DEFAULT NULL,
 part_x    NUMBER(6)  DEFAULT NULL,
 part_y    NUMBER(6)  DEFAULT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL,
 tests_executed  NUMBER(5)  DEFAULT 0 NOT NULL,
 tests_failed  NUMBER(5)  DEFAULT 0 NOT NULL,
 firstfail_tnum  NUMBER(10)  DEFAULT NULL,
 firstfail_tname  VARCHAR2(255) DEFAULT NULL,
 retest_index  NUMBER(3)  DEFAULT 0 NOT NULL,
 part_txt    VARCHAR2(255) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_run ON &GEXADMIN..WT_RUN (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table wt_ptest_info
--

CREATE TABLE &GEXADMIN..wt_ptest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 units    VARCHAR2(255) DEFAULT '',
 flags    CHAR(1)   DEFAULT NULL,
 ll     NUMBER   DEFAULT NULL,
 hl     NUMBER   DEFAULT NULL,
 testseq    NUMBER(5)  DEFAULT NULL,
 spec_ll    NUMBER   DEFAULT NULL,
 spec_hl    NUMBER   DEFAULT NULL,
 spec_target   NUMBER   DEFAULT NULL,
  res_scal NUMBER(3) DEFAULT NULL,
  ll_scal NUMBER(3) DEFAULT NULL,
  hl_scal NUMBER(3) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_ptest_info ON &GEXADMIN..wt_ptest_info (splitlot_id,ptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table wt_ptest_limits
--

CREATE TABLE &GEXADMIN..wt_ptest_limits (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 ll     NUMBER   DEFAULT 0 NOT NULL,
 hl     NUMBER   DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_ptest_limits ON &GEXADMIN..wt_ptest_limits (splitlot_id,ptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_ptest_results
--

CREATE TABLE &GEXADMIN..wt_ptest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(7)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 value    NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_ptest_results ON &GEXADMIN..wt_ptest_results (splitlot_id, ptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table wt_ptest_stats_samples
--

CREATE TABLE &GEXADMIN..wt_ptest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_ptest_stats_samples ON &GEXADMIN..wt_ptest_stats_samples (splitlot_id, ptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_ptest_stats_summary
--

CREATE TABLE &GEXADMIN..wt_ptest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_ptest_stats_summary ON &GEXADMIN..wt_ptest_stats_summary (splitlot_id, ptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_mptest_info
--

CREATE TABLE &GEXADMIN..wt_mptest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 tpin_arrayindex  NUMBER(6)  DEFAULT 0 NOT NULL,
 units    VARCHAR2(255) DEFAULT '',
 flags    CHAR(1)   DEFAULT NULL,
 ll     NUMBER   DEFAULT NULL,
 hl     NUMBER   DEFAULT NULL,
 testseq    NUMBER(5)  DEFAULT NULL,
 spec_ll    NUMBER   DEFAULT NULL,
 spec_hl    NUMBER   DEFAULT NULL,
 spec_target   NUMBER   DEFAULT NULL,
  res_scal NUMBER(3) DEFAULT NULL,
  ll_scal NUMBER(3) DEFAULT NULL,
  hl_scal NUMBER(3) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_mptest_info ON &GEXADMIN..wt_mptest_info (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table wt_mptest_limits
--

CREATE TABLE &GEXADMIN..wt_mptest_limits (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 ll     NUMBER   DEFAULT 0 NOT NULL,
 hl     NUMBER   DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_mptest_limits ON &GEXADMIN..wt_mptest_limits (splitlot_id,mptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_mptest_results
--

CREATE TABLE &GEXADMIN..wt_mptest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(7)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 value    NUMBER   DEFAULT NULL,
 tpin_pmrindex  NUMBER(6)  DEFAULT 0
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_mptest_results ON &GEXADMIN..wt_mptest_results (splitlot_id, mptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table wt_mptest_stats_samples
--

CREATE TABLE &GEXADMIN..wt_mptest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_mptest_stats_samples ON &GEXADMIN..wt_mptest_stats_samples (splitlot_id, mptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_mptest_stats_summary
--

CREATE TABLE &GEXADMIN..wt_mptest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_mptest_stats_summary ON &GEXADMIN..wt_mptest_stats_summary (splitlot_id, mptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_dtr
--

CREATE TABLE &GEXADMIN..wt_dtr (
  splitlot_id NUMBER(10) NOT NULL,
  run_id NUMBER(7) DEFAULT NULL,
  order_id NUMBER(8) NOT NULL,
  dtr_type VARCHAR2(255) NOT NULL,
  dtr_text VARCHAR2(255) DEFAULT '' NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..wt_dtr ON &GEXADMIN..wt_dtr (splitlot_id);

--
-- Table structure for table wt_ftest_info
--

CREATE TABLE &GEXADMIN..wt_ftest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 testseq    NUMBER(5)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_ftest_info ON &GEXADMIN..wt_ftest_info (splitlot_id,ftest_info_id) LOCAL COMPRESS;

--
-- Table structure for table wt_ftest_results
--

CREATE TABLE &GEXADMIN..wt_ftest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  NOT NULL,
 run_id    NUMBER(7)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 vect_nam   VARCHAR2(255) DEFAULT '',
 vect_off   NUMBER(6)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..wt_ftest_results ON &GEXADMIN..wt_ftest_results (splitlot_id, ftest_info_id) LOCAL COMPRESS;

--
-- Table structure for table wt_ftest_stats_samples
--

CREATE TABLE &GEXADMIN..wt_ftest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_ftest_stats_samples ON &GEXADMIN..wt_ftest_stats_samples (splitlot_id, ftest_info_id) LOCAL COMPRESS;


--
-- Table structure for table wt_ftest_stats_summary
--

CREATE TABLE &GEXADMIN..wt_ftest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._wt_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..wt_ftest_stats_summary ON &GEXADMIN..wt_ftest_stats_summary (splitlot_id, ftest_info_id) LOCAL COMPRESS;

--
-- FINAL TEST
--

--
-- Table structure for table ft_dtr
--

CREATE TABLE &GEXADMIN..ft_dtr (
  splitlot_id NUMBER(10) NOT NULL,
  run_id NUMBER(8) DEFAULT NULL,
  order_id NUMBER(8) NOT NULL,
  dtr_type VARCHAR2(255) NOT NULL,
  dtr_text VARCHAR2(255) DEFAULT '' NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..ft_dtr ON &GEXADMIN..ft_dtr (splitlot_id);

--
-- Table structure for table ft_die_tracking
--

CREATE TABLE &GEXADMIN..ft_die_tracking (
 ft_tracking_lot_id  VARCHAR2(255)  NOT NULL,
 wt_product_id   VARCHAR2(255)  NOT NULL,
 wt_tracking_lot_id  VARCHAR2(255)  NOT NULL,
 wt_sublot_id   VARCHAR2(255)  NOT NULL,
 die_id     VARCHAR2(255)  NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..ft_multidie_tracking_1 ON &GEXADMIN..ft_die_tracking (ft_tracking_lot_id);
CREATE INDEX &GEXADMIN..ft_multidie_tracking_2 ON &GEXADMIN..ft_die_tracking (wt_tracking_lot_id);

--
-- Table structure for table ft_sya_set
--

CREATE TABLE &GEXADMIN..ft_sya_set (
 sya_id     NUMBER(10)   NOT NULL,
 product_id    VARCHAR2(255)  NOT NULL,
 creation_date   DATE    NOT NULL,
 user_comment   VARCHAR2(255)  DEFAULT NULL,
 ll_1     NUMBER    DEFAULT -1 NOT NULL,
 hl_1     NUMBER    DEFAULT -1 NOT NULL,
 ll_2     NUMBER    DEFAULT -1 NOT NULL,
 hl_2     NUMBER    DEFAULT -1 NOT NULL,
 start_date    DATE    NOT NULL,
 expiration_date   DATE    NOT NULL,
 expiration_email_date DATE    DEFAULT NULL,
 rule_type    VARCHAR2(255)  NOT NULL,
 n1_parameter   NUMBER    DEFAULT -1 NOT NULL,
 n2_parameter   NUMBER    DEFAULT -1 NOT NULL,
 computation_fromdate DATE    NOT NULL,
 computation_todate  DATE    NOT NULL,
 min_lots_required  NUMBER(5)   DEFAULT -1 NOT NULL,
 min_data_points   NUMBER(5)   DEFAULT -1 NOT NULL,
 options     NUMBER(3)   DEFAULT 0 NOT NULL,
 flags     NUMBER(3)   DEFAULT 0 NOT NULL,
 rule_name   VARCHAR2(255)  DEFAULT NULL,
 CONSTRAINT pk_ft_sya_set PRIMARY KEY(sya_id)
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table ft_sbl
--

CREATE TABLE &GEXADMIN..ft_sbl (
 sya_id     NUMBER(10)  NOT NULL,
 bin_no     NUMBER(5)  NOT NULL,
 bin_name    VARCHAR2(255) DEFAULT NULL,
 ll_1     NUMBER   DEFAULT -1 NOT NULL,
 hl_1     NUMBER   DEFAULT -1 NOT NULL,
 ll_2     NUMBER   DEFAULT -1 NOT NULL,
 hl_2     NUMBER   DEFAULT -1 NOT NULL,
 rule_type    VARCHAR2(255)  DEFAULT NULL,
 n1_parameter   NUMBER    DEFAULT NULL,
 n2_parameter   NUMBER    DEFAULT NULL,
 CONSTRAINT pk_ft_sbl PRIMARY KEY(sya_id,bin_no),
 CONSTRAINT fk_ft_sbl FOREIGN KEY(sya_id) REFERENCES &GEXADMIN..ft_sya_set(sya_id)
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table ft_prod_alarm
--

CREATE TABLE &GEXADMIN..ft_prod_alarm (
  splitlot_id   NUMBER(10)  NOT NULL,
  alarm_cat    VARCHAR2(255) NOT NULL,
  alarm_type   VARCHAR2(255) NOT NULL,
  item_no    NUMBER(10) NOT NULL,
  item_name    VARCHAR2(255) DEFAULT NULL,
  flags     CHAR(2)   NOT NULL,
  lcl     NUMBER   DEFAULT 0 NOT NULL,
  ucl     NUMBER   DEFAULT 0 NOT NULL,
  value     NUMBER   DEFAULT 0 NOT NULL,
  units     VARCHAR2(10) DEFAULT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..ft_prod_alarm ON &GEXADMIN..ft_prod_alarm (splitlot_id);

--
-- Table structure for table ft_product_hbin
--

CREATE TABLE &GEXADMIN..ft_product_hbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no    NUMBER(5)   NOT NULL,
  bin_name    VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   DEFAULT NULL,
  nb_parts     NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..ft_product_hbin ON &GEXADMIN..ft_product_hbin (product_name,bin_no);

--
-- Table structure for table ft_product_sbin
--

CREATE TABLE &GEXADMIN..ft_product_sbin (
  product_name    VARCHAR2(255) NOT NULL,
  bin_no    NUMBER(5)   NOT NULL,
  bin_name    VARCHAR2(255) DEFAULT NULL,
  bin_cat     CHAR(1)   DEFAULT NULL,
  nb_parts     NUMBER   DEFAULT 0 NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..ft_product_sbin ON &GEXADMIN..ft_product_sbin (product_name,bin_no);

--
-- Table structure for table ft_wyr
--

CREATE TABLE &GEXADMIN..ft_wyr (
 site_name    VARCHAR2(255) NOT NULL,
 week_nb     NUMBER(3)  DEFAULT NULL,
 year     NUMBER(5)  DEFAULT NULL,
 date_in     DATE   DEFAULT NULL,
 date_out    DATE   DEFAULT NULL,
 product_name   VARCHAR2(255) DEFAULT NULL,
 program_name   VARCHAR2(255) DEFAULT NULL,
 tester_name    VARCHAR2(255) DEFAULT NULL,
 lot_id     VARCHAR2(255) DEFAULT NULL,
 subcon_lot_id   VARCHAR2(255) DEFAULT NULL,
 user_split    VARCHAR2(1024) DEFAULT NULL,
 yield     NUMBER   DEFAULT 0,
 parts_received   NUMBER(10)  DEFAULT 0,
 pretest_rejects   NUMBER(10)  DEFAULT 0,
 pretest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_tested   NUMBER(10)  DEFAULT 0,
 parts_pass    NUMBER(10)  DEFAULT 0,
 parts_pass_split  VARCHAR2(1024) DEFAULT NULL,
 parts_fail    NUMBER(10)  DEFAULT 0,
 parts_fail_split  VARCHAR2(1024) DEFAULT NULL,
 parts_retest   NUMBER(10)  DEFAULT 0,
 parts_retest_split  VARCHAR2(1024) DEFAULT NULL,
 insertions    NUMBER(10)  DEFAULT 0,
 posttest_rejects  NUMBER(10)  DEFAULT 0,
 posttest_rejects_split VARCHAR2(1024) DEFAULT NULL,
 parts_shipped   NUMBER(10)  DEFAULT 0
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table ft_wyr_format
--

CREATE TABLE &GEXADMIN..ft_wyr_format (
 site_name    VARCHAR2(255) NOT NULL,
 column_id    NUMBER(3)  NOT NULL,
 column_nb    NUMBER(3)  NOT NULL,
 column_name    VARCHAR2(255) NOT NULL,
 data_type    VARCHAR2(255) NOT NULL,
 display     CHAR(1)   DEFAULT 'Y' NOT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table ft_metadata_mapping
--

CREATE TABLE &GEXADMIN..ft_metadata_mapping (
 meta_name    VARCHAR2(255) NOT NULL,
 gex_name    VARCHAR2(255) DEFAULT NULL,
 gexdb_table_name  VARCHAR2(255) NOT NULL,
 gexdb_field_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 gex_display_in_gui  CHAR(1)   DEFAULT 'Y' NOT NULL,
  bintype_field char(1) DEFAULT 'N' NOT NULL,
  time_field char(1) DEFAULT 'N' NOT NULL,
  custom_field char(1) DEFAULT 'Y' NOT NULL,
  numeric_field char(1) DEFAULT 'N' NOT NULL,
  fact_field char(1) DEFAULT 'N' NOT NULL,
  consolidated_field char(1) DEFAULT 'Y' NOT NULL,
  er_display_in_gui char(1) DEFAULT 'Y' NOT NULL,
  az_field char(1) DEFAULT 'N' NOT NULL,
 CONSTRAINT pk_ft_metadata_mapping PRIMARY KEY (meta_name),
 CONSTRAINT uk1_ft_metadata_mapping UNIQUE (gex_name),
 CONSTRAINT uk2_ft_metadata_mapping UNIQUE (gexdb_field_fullname)
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;


--
-- Table structure for table ft_metadata_link
--

CREATE TABLE &GEXADMIN..ft_metadata_link (
 link_name    VARCHAR2(255) NOT NULL,
 gexdb_table1_name  VARCHAR2(255) NOT NULL,
 gexdb_field1_fullname VARCHAR2(255) NOT NULL,
 gexdb_table2_name  VARCHAR2(255) NOT NULL,
 gexdb_field2_fullname VARCHAR2(255) NOT NULL,
 gexdb_link_name   VARCHAR2(255) DEFAULT NULL,
 CONSTRAINT pk_ft_metadata_link PRIMARY KEY (link_name),
 CONSTRAINT uk1_ft_metadata_link UNIQUE (gexdb_field1_fullname,gexdb_field2_fullname)
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

--
-- Table structure for table ft_lot
--

CREATE TABLE &GEXADMIN..ft_lot (
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 tracking_lot_id  VARCHAR2(255) DEFAULT NULL,
 product_name  VARCHAR2(255) DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(2)   DEFAULT NULL
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;

CREATE INDEX &GEXADMIN..ft_lot ON &GEXADMIN..ft_lot (lot_id);
CREATE INDEX &GEXADMIN..ft_lot_product ON &GEXADMIN..ft_lot (product_name,lot_id);

--
-- Table structure for table ft_lot_hbin
--

CREATE TABLE &GEXADMIN..ft_lot_hbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 hbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 hbin_name    VARCHAR2(255)   DEFAULT '',
 hbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..ft_lot_hbin ON &GEXADMIN..ft_lot_hbin (lot_id);

--
-- Table structure for table ft_lot_sbin
--

CREATE TABLE &GEXADMIN..ft_lot_sbin (
 lot_id    VARCHAR2(255)   NOT NULL,
 sbin_no    NUMBER(5)     DEFAULT '0' NOT NULL,
 sbin_name    VARCHAR2(255)   DEFAULT '',
 sbin_cat    CHAR(1)     DEFAULT NULL,
 nb_parts   NUMBER(8)    DEFAULT '0' NOT NULL 
) TABLESPACE &GEXADMIN._ft PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS;
CREATE INDEX &GEXADMIN..ft_lot_sbin ON &GEXADMIN..ft_lot_sbin (lot_id);


--
-- Table structure for table ft_splitlot
--

CREATE TABLE &GEXADMIN..ft_splitlot (
 splitlot_id   NUMBER(10)  NOT NULL,
 lot_id    VARCHAR2(255) DEFAULT '' NOT NULL,
 sublot_id   VARCHAR2(255) DEFAULT '',
 setup_t    NUMBER(10)  DEFAULT 0 NOT NULL,
 start_t    NUMBER(10)  DEFAULT 0 NOT NULL,
 finish_t   NUMBER(10)  DEFAULT 0 NOT NULL,
 stat_num   NUMBER(3)  DEFAULT 0 NOT NULL,
 tester_name   VARCHAR2(255) DEFAULT '',
 tester_type   VARCHAR2(255) DEFAULT '',
 flags    CHAR(2)   DEFAULT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_samples NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_samples_good NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_summary NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_summary_good NUMBER(8)  DEFAULT 0 NOT NULL,
 data_provider  VARCHAR2(255) DEFAULT '',
 data_type   VARCHAR2(255) DEFAULT '',
 prod_data   CHAR(1)   DEFAULT 'Y' NOT NULL,
 retest_index  NUMBER(3)  DEFAULT 0 NOT NULL,
 retest_hbins  VARCHAR2(255) DEFAULT NULL,
 rework_code   NUMBER(3)  DEFAULT 0 NOT NULL,
 job_nam    VARCHAR2(255) DEFAULT NULL,
 job_rev    VARCHAR2(255) DEFAULT NULL,
 oper_nam   VARCHAR2(255) DEFAULT NULL,
 exec_typ   VARCHAR2(255) DEFAULT NULL,
 exec_ver   VARCHAR2(255) DEFAULT NULL,
 test_cod   VARCHAR2(255) DEFAULT NULL,
 facil_id   VARCHAR2(255) DEFAULT NULL,
 tst_temp   VARCHAR2(255) DEFAULT NULL,
 mode_cod   CHAR(1)   DEFAULT NULL, 
 rtst_cod   CHAR(1)   DEFAULT NULL, 
 prot_cod   CHAR(1)   DEFAULT NULL, 
 burn_tim   NUMBER(5)  DEFAULT NULL, 
 cmod_cod   CHAR(1)   DEFAULT NULL, 
 part_typ   VARCHAR2(256) DEFAULT NULL, 
 user_txt   VARCHAR2(256) DEFAULT NULL, 
 aux_file   VARCHAR2(256) DEFAULT NULL, 
 pkg_typ    VARCHAR2(256) DEFAULT NULL,
 famly_id   VARCHAR2(256) DEFAULT NULL, 
 date_cod   VARCHAR2(256) DEFAULT NULL, 
 floor_id   VARCHAR2(256) DEFAULT NULL, 
 proc_id    VARCHAR2(256) DEFAULT NULL,
 oper_frq   VARCHAR2(256) DEFAULT NULL, 
 spec_nam   VARCHAR2(256) DEFAULT NULL, 
 spec_ver   VARCHAR2(256) DEFAULT NULL,
 flow_id    VARCHAR2(256) DEFAULT NULL,
 setup_id   VARCHAR2(256) DEFAULT NULL,
 dsgn_rev   VARCHAR2(256) DEFAULT NULL,
 eng_id    VARCHAR2(256) DEFAULT NULL,
 rom_cod    VARCHAR2(256) DEFAULT NULL,
 serl_num   VARCHAR2(256) DEFAULT NULL,
 supr_nam   VARCHAR2(256) DEFAULT NULL,
 nb_sites   NUMBER(3)  DEFAULT 1 NOT NULL,
 head_num   NUMBER(3)  DEFAULT NULL,
 handler_typ   VARCHAR2(255) DEFAULT NULL,
 handler_id   VARCHAR2(255) DEFAULT NULL,
 card_typ   VARCHAR2(255) DEFAULT NULL,
 card_id    VARCHAR2(255) DEFAULT NULL,
 loadboard_typ  VARCHAR2(255) DEFAULT NULL,
 loadboard_id  VARCHAR2(255) DEFAULT NULL,
 dib_typ    VARCHAR2(255) DEFAULT NULL,
 dib_id    VARCHAR2(255) DEFAULT NULL,
 cable_typ   VARCHAR2(255) DEFAULT NULL,
 cable_id   VARCHAR2(255) DEFAULT NULL,
 contactor_typ  VARCHAR2(255) DEFAULT NULL,
 contactor_id  VARCHAR2(255) DEFAULT NULL,
 laser_typ   VARCHAR2(255) DEFAULT NULL,
 laser_id   VARCHAR2(255) DEFAULT NULL,
 extra_typ   VARCHAR2(255) DEFAULT NULL,
 extra_id   VARCHAR2(255) DEFAULT NULL,
 file_host_id  NUMBER(10)  DEFAULT 0,
 file_path   VARCHAR2(255) DEFAULT '',
 file_name   VARCHAR2(255) DEFAULT '',
 valid_splitlot  CHAR(1)   DEFAULT 'N' NOT NULL,
 insertion_time  NUMBER(10)  DEFAULT NULL,
 subcon_lot_id  VARCHAR2(255) DEFAULT '',
 incremental_update  VARCHAR2(255) DEFAULT NULL,
 sya_id    NUMBER(10)  DEFAULT 0,
  day VARCHAR2(10) NOT NULL,
  week_nb NUMBER(2) NOT NULL,
  month_nb NUMBER(2) NOT NULL,
  quarter_nb NUMBER(1) NOT NULL,
  year_nb NUMBER(4) NOT NULL,
  year_and_week VARCHAR2(7) NOT NULL,
  year_and_month VARCHAR2(7) NOT NULL,
  year_and_quarter VARCHAR2(7) NOT NULL,
 CONSTRAINT pk_ft_splitlot PRIMARY KEY (splitlot_id)
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_splitlot ON &GEXADMIN..ft_splitlot (lot_id) LOCAL COMPRESS;
CREATE INDEX &GEXADMIN..ft_splitlot_start ON &GEXADMIN..ft_splitlot (start_t) LOCAL COMPRESS;


--
-- Table structure for table ft_parts_stats_samples
--

CREATE TABLE &GEXADMIN..ft_parts_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_parts_good  NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_parts_stats_samples ON &GEXADMIN..ft_parts_stats_samples (splitlot_id, site_no) LOCAL COMPRESS;

--
-- Table structure for table ft_parts_stats_summary
--

CREATE TABLE &GEXADMIN..ft_parts_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL,
 nb_good    NUMBER(8)  DEFAULT NULL,
 nb_rtst    NUMBER(8)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_d PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_parts_stats_summary ON &GEXADMIN..ft_parts_stats_summary (splitlot_id, site_no) LOCAL COMPRESS;

--
-- Table structure for table ft_hbin
--

CREATE TABLE &GEXADMIN..ft_hbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 hbin_name   VARCHAR2(255) DEFAULT '',
 hbin_cat   CHAR(1)   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_hbin ON &GEXADMIN..ft_hbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table ft_hbin_stats_samples
--

CREATE TABLE &GEXADMIN..ft_hbin_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_hbin_stats_samples ON &GEXADMIN..ft_hbin_stats_samples (splitlot_id, site_no, hbin_no) LOCAL COMPRESS;

--
-- Table structure for table ft_hbin_stats_summary
--

CREATE TABLE &GEXADMIN..ft_hbin_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_hbin_stats_summary ON &GEXADMIN..ft_hbin_stats_summary (splitlot_id, site_no, hbin_no) LOCAL COMPRESS;

--
-- Table structure for table ft_sbin
--

CREATE TABLE &GEXADMIN..ft_sbin (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_name   VARCHAR2(255) DEFAULT '',
 sbin_cat   CHAR(1)   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_sbin ON &GEXADMIN..ft_sbin (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table ft_sbin_stats_samples
--

CREATE TABLE &GEXADMIN..ft_sbin_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 nb_parts   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_sbin_stats_samples ON &GEXADMIN..ft_sbin_stats_samples (splitlot_id, site_no, sbin_no) LOCAL COMPRESS;

--
-- Table structure for table ft_sbin_stats_summary
--

CREATE TABLE &GEXADMIN..ft_sbin_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 bin_count   NUMBER(8)  DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_sbin_stats_summary ON &GEXADMIN..ft_sbin_stats_summary (splitlot_id, site_no, sbin_no) LOCAL COMPRESS;

--
-- Table structure for table ft_run
--

CREATE TABLE &GEXADMIN..ft_run (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(8)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 part_id    VARCHAR2(255) DEFAULT NULL,
 part_x    NUMBER(6)  DEFAULT NULL,
 part_y    NUMBER(6)  DEFAULT NULL,
 hbin_no    NUMBER(5)  DEFAULT 0 NOT NULL,
 sbin_no    NUMBER(5)  DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL,
 tests_executed  NUMBER(5)  DEFAULT 0 NOT NULL,
 tests_failed  NUMBER(5)  DEFAULT 0 NOT NULL,
 firstfail_tnum  NUMBER(10)  DEFAULT NULL,
 firstfail_tname  VARCHAR2(255) DEFAULT NULL,
 retest_index  NUMBER(3)  DEFAULT 0 NOT NULL,
 wafer_id  VARCHAR2(255)  DEFAULT NULL,
 part_txt    VARCHAR2(255) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_run ON &GEXADMIN..ft_run (splitlot_id) LOCAL COMPRESS;

--
-- Table structure for table ft_ptest_info
--

CREATE TABLE &GEXADMIN..ft_ptest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 units    VARCHAR2(255) DEFAULT '',
 flags    CHAR(1)   DEFAULT NULL,
 ll     NUMBER   DEFAULT NULL,
 hl     NUMBER   DEFAULT NULL,
 testseq    NUMBER(5)  DEFAULT NULL,
 spec_ll    NUMBER   DEFAULT NULL,
 spec_hl    NUMBER   DEFAULT NULL,
 spec_target   NUMBER   DEFAULT NULL,
  res_scal NUMBER(3) DEFAULT NULL,
  ll_scal NUMBER(3) DEFAULT NULL,
  hl_scal NUMBER(3) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ptest_info ON &GEXADMIN..ft_ptest_info (splitlot_id,ptest_info_id) LOCAL COMPRESS; 


--
-- Table structure for table ft_ptest_limits
--

CREATE TABLE &GEXADMIN..ft_ptest_limits (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 ll     NUMBER   DEFAULT 0 NOT NULL,
 hl     NUMBER   DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_ptest_limits ON &GEXADMIN..ft_ptest_limits (splitlot_id,ptest_info_id) LOCAL COMPRESS; 


--
-- Table structure for table ft_ptest_results
--

CREATE TABLE &GEXADMIN..ft_ptest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 value    NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ptest_results ON &GEXADMIN..ft_ptest_results (splitlot_id, ptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table ft_ptest_stats_samples
--

CREATE TABLE &GEXADMIN..ft_ptest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_ptest_stats_samples ON &GEXADMIN..ft_ptest_stats_samples (splitlot_id, ptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table ft_ptest_stats_summary
--

CREATE TABLE &GEXADMIN..ft_ptest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_ptest_stats_summary ON &GEXADMIN..ft_ptest_stats_summary (splitlot_id, ptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table ft_mptest_info
--

CREATE TABLE &GEXADMIN..ft_mptest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 tpin_arrayindex  NUMBER(6)  DEFAULT 0 NOT NULL,
 units    VARCHAR2(255) DEFAULT '',
 flags    CHAR(1)   DEFAULT NULL,
 ll     NUMBER   DEFAULT NULL,
 hl     NUMBER   DEFAULT NULL,
 testseq    NUMBER(5)  DEFAULT NULL,
 spec_ll    NUMBER   DEFAULT NULL,
 spec_hl    NUMBER   DEFAULT NULL,
 spec_target   NUMBER   DEFAULT NULL,
  res_scal NUMBER(3) DEFAULT NULL,
  ll_scal NUMBER(3) DEFAULT NULL,
  hl_scal NUMBER(3) DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_mptest_info ON &GEXADMIN..ft_mptest_info (splitlot_id) LOCAL COMPRESS; 


--
-- Table structure for table ft_mptest_limits
--

CREATE TABLE &GEXADMIN..ft_mptest_limits (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 ll     NUMBER   DEFAULT 0 NOT NULL,
 hl     NUMBER   DEFAULT 0 NOT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_mptest_limits ON &GEXADMIN..ft_mptest_limits (splitlot_id) LOCAL COMPRESS; 


--
-- Table structure for table ft_mptest_results
--

CREATE TABLE &GEXADMIN..ft_mptest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 run_id    NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 value    NUMBER   DEFAULT NULL,
 tpin_pmrindex  NUMBER(6)  DEFAULT 0
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_mptest_results ON &GEXADMIN..ft_mptest_results (splitlot_id, mptest_info_id) LOCAL COMPRESS;

--
-- Table structure for table ft_mptest_stats_samples
--

CREATE TABLE &GEXADMIN..ft_mptest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_mptest_stats_samples ON &GEXADMIN..ft_mptest_stats_samples (splitlot_id, mptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table ft_mptest_stats_summary
--

CREATE TABLE &GEXADMIN..ft_mptest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 mptest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 min_value   NUMBER   DEFAULT NULL,
 max_value   NUMBER   DEFAULT NULL,
 sum     NUMBER   DEFAULT NULL,
 square_sum   NUMBER   DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);
CREATE INDEX &GEXADMIN..ft_mptest_stats_summary ON &GEXADMIN..ft_mptest_stats_summary (splitlot_id, mptest_info_id) LOCAL COMPRESS;


--
-- Table structure for table ft_ftest_info
--

CREATE TABLE &GEXADMIN..ft_ftest_info (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 tnum    NUMBER(10)  DEFAULT 0 NOT NULL,
 tname    VARCHAR2(255) DEFAULT '',
 testseq    NUMBER(5)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ftest_info ON &GEXADMIN..ft_ftest_info (splitlot_id,ftest_info_id) LOCAL COMPRESS;

--
-- Table structure for table ft_ftest_results
--

CREATE TABLE &GEXADMIN..ft_ftest_results (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  NOT NULL,
 run_id    NUMBER(8)  DEFAULT 0 NOT NULL,
 flags    CHAR(1)   DEFAULT NULL,
 vect_nam   VARCHAR2(255) DEFAULT '',
 vect_off   NUMBER(6)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_r PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ftest_results ON &GEXADMIN..ft_ftest_results (splitlot_id, ftest_info_id) LOCAL COMPRESS;

--
-- Table structure for table ft_ftest_stats_samples
--

CREATE TABLE &GEXADMIN..ft_ftest_stats_samples (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT 0 NOT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ftest_stats_samples ON &GEXADMIN..ft_ftest_stats_samples (splitlot_id, ftest_info_id) LOCAL COMPRESS;

--
-- Table structure for table ft_ftest_stats_summary
--

CREATE TABLE &GEXADMIN..ft_ftest_stats_summary (
 splitlot_id   NUMBER(10)  DEFAULT 0 NOT NULL,
 ftest_info_id  NUMBER(5)  DEFAULT 0 NOT NULL,
 site_no    NUMBER(5)  DEFAULT 1 NOT NULL,
 exec_count   NUMBER(8)  DEFAULT NULL,
 fail_count   NUMBER(8)  DEFAULT NULL,
 ttime    NUMBER(10)  DEFAULT NULL
)
PARTITION BY RANGE (splitlot_id) 
(PARTITION FIRSTPART VALUES LESS THAN (MAXVALUE) TABLESPACE &GEXADMIN._ft_s PCTFREE 5 PCTUSED 80 &LOGGINGMODE. COMPRESS);

CREATE INDEX &GEXADMIN..ft_ftest_stats_summary ON &GEXADMIN..ft_ftest_stats_summary (splitlot_id, ftest_info_id) LOCAL COMPRESS;


--
-- PURGE PROCEDURE
--

--
-- Procedure purge_invalid_splitlots
--
CREATE PROCEDURE &GEXADMIN..purge_invalid_splitlots
IS 
  SplitlotNumber  NUMBER(10); 
  SplitlotsForErase VARCHAR2(2048); 
  -- Prepare cursor to have all Invalid SplitlotId for FT_SPLITLOT 
  CURSOR curInvalidSplitlotsFT IS 
    SELECT distinct splitlot_id FROM ft_splitlot 
    WHERE (valid_splitlot = 'N') 
     AND (((new_time(sysdate,'EDT','GMT') - to_date('19700101','yyyymmdd')) * (24*60*60)) > insertion_time+100); 
  -- Prepare cursor to have all Invalid SplitlotId for WT_SPLITLOT 
  CURSOR curInvalidSplitlotsWT IS 
    SELECT distinct splitlot_id FROM wt_splitlot 
    WHERE (valid_splitlot = 'N') 
     AND (((new_time(sysdate,'EDT','GMT') - to_date('19700101','yyyymmdd')) * (24*60*60)) > insertion_time+100); 
  -- Prepare cursor to have all Invalid SplitlotId for ET_SPLITLOT 
  CURSOR curInvalidSplitlotsET IS 
    SELECT distinct splitlot_id FROM et_splitlot 
    WHERE (valid_splitlot = 'N') 
     AND (((new_time(sysdate,'EDT','GMT') - to_date('19700101','yyyymmdd')) * (24*60*60)) > insertion_time+100); 
  
  ------------------------------------------------------------- 
  -- BEGIN SUBPROCEDURE DECLARATION 
  ------------------------------------------------------------- 
  PROCEDURE purge_invalid_elements 
  ( 
  TestingStage   VARCHAR2, -- 'FT', 'ET' or 'WT' 
  SplitlotsForErase VARCHAR2 
  ) 
  IS 
  BEGIN 
   -- Test entries before start purge 
   IF NOT((TestingStage='FT') OR (TestingStage='ET') OR (TestingStage='WT')) THEN RETURN; END IF; 
   IF (Length(SplitlotsForErase)<1) THEN RETURN; END IF; 
  
  
  -- A - Purge RUN RESULTS 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_RESULTS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   IF ((TestingStage='FT') OR (TestingStage='WT')) THEN 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_RESULTS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_RESULTS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   END IF; 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_RUN WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
  
  -- B - Purge STATS RESULTS 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_INFO WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   IF ((TestingStage='FT') OR (TestingStage='WT')) THEN 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_LIMITS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_INFO WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_LIMITS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_INFO WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PARTS_STATS_SAMPLES WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PARTS_STATS_SUMMARY WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   ELSE 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   END IF; 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
  
  -- C - Purge SPLITLOT entries 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PROD_ALARM WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SPLITLOT WHERE SPLITLOT_Id IN ('||SplitlotsForErase||')'; 
  
  END; 
  ------------------------------------------------------------- 
  -- END SUBPROCEDURE DECLARATION 
  ------------------------------------------------------------- 
 BEGIN 
  -- FT_SPLITLOT 
  -- have to found all invalid splitlot 
  -- and create a list for delete 
  -- Execute cursor 
  SplitlotsForErase := ''; 
  OPEN curInvalidSplitlotsFT; 
  LOOP 
   FETCH curInvalidSplitlotsFT INTO SplitlotNumber; 
   EXIT WHEN curInvalidSplitlotsFT%NOTFOUND; 
     IF (Length(SplitlotsForErase)>1) THEN SplitlotsForErase := CONCAT(SplitlotsForErase,','); END IF; 
     SplitlotsForErase := CONCAT(SplitlotsForErase, to_char(SplitlotNumber)); 
  END LOOP; 
  CLOSE curInvalidSplitlotsFT; 
  IF (Length(SplitlotsForErase)>1) THEN purge_invalid_elements('FT',SplitlotsForErase); END IF; 
  -- WT_SPLITLOT 
  -- have to found all invalid splitlot 
  -- and create a list for delete 
  -- Execute cursor 
  SplitlotsForErase := ''; 
  OPEN curInvalidSplitlotsWT; 
  LOOP 
   FETCH curInvalidSplitlotsWT INTO SplitlotNumber; 
   EXIT WHEN curInvalidSplitlotsWT%NOTFOUND; 
     IF (Length(SplitlotsForErase)>1) THEN SplitlotsForErase := CONCAT(SplitlotsForErase,','); END IF; 
     SplitlotsForErase := CONCAT(SplitlotsForErase, to_char(SplitlotNumber)); 
  END LOOP; 
  CLOSE curInvalidSplitlotsWT; 
  IF (Length(SplitlotsForErase)>1) THEN purge_invalid_elements('WT',SplitlotsForErase); END IF; 
  -- ET_SPLITLOT 
  -- have to found all invalid splitlot 
  -- and create a list for delete 
  -- Execute cursor 
  SplitlotsForErase := ''; 
  OPEN curInvalidSplitlotsET; 
  LOOP 
   FETCH curInvalidSplitlotsET INTO SplitlotNumber; 
   EXIT WHEN curInvalidSplitlotsET%NOTFOUND; 
     IF (Length(SplitlotsForErase)>1) THEN SplitlotsForErase := CONCAT(SplitlotsForErase,','); END IF; 
     SplitlotsForErase := CONCAT(SplitlotsForErase, to_char(SplitlotNumber)); 
  END LOOP; 
  CLOSE curInvalidSplitlotsET; 
  IF (Length(SplitlotsForErase)>1) THEN purge_invalid_elements('ET',SplitlotsForErase); END IF; 
 
END; 
/

CREATE PROCEDURE &GEXADMIN..purge_splitlots
 ( 
SamplesNbWeeks NUMBER, -- Nb of weeks before delete samples 
StatsNbWeeks NUMBER, -- Nb of weeks before delete stats 
EntriesNbWeeks NUMBER -- Nb of weeks before delete entries 
) 
IS  
   
  -------------------------------------------------------------  
  -- SUBPROCEDURE DECLARATION  
  -------------------------------------------------------------  
  PROCEDURE purge_elements  
  (  
  TestingStage VARCHAR2, -- 'FT', 'ET' or 'WT'  
  NbWeeks NUMBER,  -- Nb of weeks before delete  
  PurgeLevel NUMBER -- Level of purge  
     -- 1 for SAMPLES only  
     -- 2 for SAMPLES and STATS  
     -- 3 for SAMPLES, STATS and Entries  
  )  
  IS  
  SplitlotForErase NUMBER;  
  TableSpaceForErase NUMBER;  
  TableName   VARCHAR2(256);  
  PartitionForErase VARCHAR2(6);  
  -- Prepare cursor to have all tables name saved in tablespaces_for_erase  
  CURSOR curTablePartition(TBS NUMBER, TS VARCHAR2) IS  
    SELECT distinct TABLE_NAME, substr(TABLESPACE_NAME,-4,4) FROM USER_TAB_PARTITIONS  
    WHERE Length(TABLESPACE_NAME)>13 AND substr(TABLESPACE_NAME,-4,4)<=to_char(TBS,'0999')  
    AND substr(TABLE_NAME,0,2)=TS;  
  -- Prepare cursor to have tablespaces name for delete  
  CURSOR curTableSpace(TBS NUMBER, TS VARCHAR2) IS  
    SELECT distinct TABLESPACE_NAME FROM USER_TAB_PARTITIONS  
    WHERE Length(TABLESPACE_NAME)>13 AND substr(TABLESPACE_NAME,-4,4)<=to_char(TBS,'0999')  
    AND substr(TABLE_NAME,0,2)=TS;  
  -- Prepare cursor to have partition name for delete from specific table_name  
  CURSOR curPartition(TBS NUMBER, TN VARCHAR2) IS  
    SELECT distinct PARTITION_NAME FROM USER_TAB_PARTITIONS  
    WHERE Length(TABLESPACE_NAME)>13 AND substr(TABLESPACE_NAME,-4,4)<=to_char(TBS,'0999')  
    AND TABLE_NAME=TN ;  
   
  BEGIN  
   -- Test entries before start purge  
   IF NOT((TestingStage='FT') OR (TestingStage='ET') OR (TestingStage='WT')) THEN RETURN; END IF;  
   IF NOT(0 <= NbWeeks) THEN RETURN; END IF;  
   IF NOT((0 < PurgeLevel) AND (PurgeLevel <= 3)) THEN RETURN; END IF;  
   
   SELECT TO_CHAR(CURRENT_DATE - (NbWeeks*7), 'YYWW')+0 INTO TableSpaceForErase FROM dual ;  
   SELECT TO_CHAR(CURRENT_DATE - (NbWeeks*7), 'YYMMDD')*10000 INTO SplitlotForErase FROM dual ;  
   
   -- have to delete each partition of the table_RESULTS  
   -- and drop the tablespace  
   -- first found the tablespace number for the purge limit  
   -- if current date('YYWW') is 0639 and the NbWeeks is 4  
   -- the limit for the purge is 0635  
   -- all tablespace less than 0635 have to be delete  
   
  -- A - Purge sample RESULTS  
   -- 1 - Drop all partition table using tablespace for erase  
   -- 2 - Drop empty tablespace  
  -- B - Purge RUN RESULTS  
  -- C - Purge STATS RESULTS  
  -- D - Purge SPLITLOT entries  
   
  -- A - Purge sample RESULTS  
   -- Execute cursor  
   OPEN curTablePartition(TableSpaceForErase,TestingStage);  
   OPEN curTableSpace(TableSpaceForErase,TestingStage);  
   
   LOOP  
   -- 1 - Drop all partition table using tablespace for erase  
    FETCH curTablePartition INTO TableName, TableSpaceForErase;  
    EXIT WHEN curTablePartition%NOTFOUND;  
   
    -- Execute cursor  
    OPEN curPartition(TableSpaceForErase,TableName);  
   
    LOOP  
     FETCH curPartition INTO PartitionForErase;  
     EXIT WHEN curPartition%NOTFOUND; 
     EXECUTE IMMEDIATE 'ALTER TABLE '||TableName||' DROP PARTITION "'||trim(to_char(PartitionForErase,'099999'))||'"'; 
    END LOOP;  
    CLOSE curPartition;  
   
   END LOOP;  
   CLOSE curTablePartition;  
   LOOP  
   -- 2 - Drop empty tablespace  
    FETCH curTableSpace INTO TableName;  
    EXIT WHEN curTableSpace%NOTFOUND;  
    EXECUTE IMMEDIATE 'DROP TABLESPACE '||TableName||' INCLUDING CONTENTS AND DATAFILES CASCADE CONSTRAINTS';  
   END LOOP;  
   CLOSE curTableSpace;  
   
  -- B - Purge RUN RESULTS  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_RUN WHERE SPLITLOT_Id < '||SplitlotForErase;  
   
   IF (PurgeLevel=1) THEN RETURN; END IF;  
   
  -- C - Purge STATS RESULTS  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_INFO WHERE SPLITLOT_Id < '||SplitlotForErase;  
   IF ((TestingStage='FT') OR (TestingStage='WT')) THEN  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_LIMITS WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_FTEST_INFO WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_LIMITS WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_MPTEST_INFO WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PARTS_STATS_SAMPLES WHERE SPLITLOT_Id < '||SplitlotForErase;  
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PARTS_STATS_SUMMARY WHERE SPLITLOT_Id < '||SplitlotForErase;  
   ELSE 
    EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PTEST_STATS WHERE SPLITLOT_Id < '||SplitlotForErase;  
   END IF;  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SBIN WHERE SPLITLOT_Id < '||SplitlotForErase;  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_HBIN WHERE SPLITLOT_Id < '||SplitlotForErase;  
   
   IF (PurgeLevel=2) THEN RETURN; END IF;  
   
  -- D - Purge SPLITLOT entries  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_PROD_ALARM WHERE SPLITLOT_Id < '||SplitlotForErase;  
   EXECUTE IMMEDIATE 'DELETE FROM '||TestingStage ||'_SPLITLOT WHERE SPLITLOT_Id < '||SplitlotForErase;  
   
  END;  
  -------------------------------------------------------------  
  -- SUBPROCEDURE END  
  -------------------------------------------------------------  
 BEGIN  
  PURGE_ELEMENTS('FT',SamplesNbWeeks,1);  
  PURGE_ELEMENTS('ET',SamplesNbWeeks,1);  
  PURGE_ELEMENTS('WT',SamplesNbWeeks,1);  
   
  PURGE_ELEMENTS('FT',StatsNbWeeks,2);  
  PURGE_ELEMENTS('ET',StatsNbWeeks,2);  
  PURGE_ELEMENTS('WT',StatsNbWeeks,2);  
   
  PURGE_ELEMENTS('FT',EntriesNbWeeks,3);  
  PURGE_ELEMENTS('ET',EntriesNbWeeks,3);  
  PURGE_ELEMENTS('WT',EntriesNbWeeks,3);  
   
  PURGE_INVALID_SPLITLOTS();  
 
 END; 
/

CREATE PROCEDURE &GEXADMIN..add_purge_job
(
PurgeJobName  VARCHAR2, -- Name for the Job &GEXADMIN..Name
PurgeRepeatInterval VARCHAR2, -- Oracle Repeat Interval
PurgeJobComments VARCHAR2, -- Comments for creation
SamplesNbWeeks   VARCHAR2, -- Nb of weeks before delete samples
StatsNbWeeks   VARCHAR2, -- Nb of weeks before delete stats
EntriesNbWeeks   VARCHAR2 -- Nb of weeks before delete entries
)
IS
 JobName  VARCHAR2(256);
 CURSOR curJobs IS SELECT JOB_NAME FROM ALL_SCHEDULER_JOBS WHERE JOB_NAME=PurgeJobName;
BEGIN
 -- IF ALREADY EXIST HAVE TO DROP IT
 OPEN curJobs;
 LOOP
  FETCH curJobs INTO JobName;
  EXIT WHEN curJobs%NOTFOUND;
  sys.dbms_scheduler.drop_job(Job_Name => CONCAT('&GEXADMIN..',PurgeJobName), force => TRUE);
 END LOOP;
 sys.dbms_scheduler.create_job(
  job_name => CONCAT('&GEXADMIN..',PurgeJobName),
  job_type => 'STORED_PROCEDURE',
  job_action => '&GEXADMIN..PURGE_SPLITLOTS',
  repeat_interval => PurgeRepeatInterval,
  start_date => systimestamp at time zone '+2:00',
  job_class => 'DEFAULT_JOB_CLASS',
  comments => PurgeJobComments,
  auto_drop => FALSE,
  number_of_arguments => 3,
  enabled => FALSE);
 sys.dbms_scheduler.set_job_argument_value( job_name => PurgeJobName, argument_position => 1, argument_value => SamplesNbWeeks);
 sys.dbms_scheduler.set_job_argument_value( job_name => PurgeJobName, argument_position => 2, argument_value => StatsNbWeeks);
 sys.dbms_scheduler.set_job_argument_value( job_name => PurgeJobName, argument_position => 3, argument_value => EntriesNbWeeks);
 sys.dbms_scheduler.enable( PurgeJobName );
END;
/

--
-- Procedure et_insertion_validation
--
CREATE PROCEDURE &GEXADMIN..et_insertion_validation
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be validated
LotID    IN VARCHAR2, -- Lot of the splitlot to be validated
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted
TrackingLotID_Out OUT VARCHAR2, -- Tracking to be used in GexDB for this splitlot
LotID_Out   OUT VARCHAR2, -- LotID to be used in GexDB for this splitlot
WaferID_Out   OUT VARCHAR2, -- WaferID to be used in GexDB for this splitlot
ProductName   OUT VARCHAR2, -- Return the Product Name if it has to be overloaded
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 TrackingLotID_Out := TrackingLotID;
 LotID_Out := LotID;
 WaferID_Out := WaferID;
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure et_insertion_postprocessing
--
CREATE PROCEDURE &GEXADMIN..et_insertion_postprocessing
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure et_insertion_status
--
CREATE PROCEDURE &GEXADMIN..et_insertion_status
(
SplitlotID  IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID   IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID   IN VARCHAR2, -- WaferID of the splitlot to be inserted
Message   IN VARCHAR2, -- Error message in case the insertion failed
Status   IN NUMBER  -- Insertion status: 1 if insertion successful, 0 else
)
IS
BEGIN
 RETURN;
END;
/

--
-- Procedure et_filearchive_settings
--

CREATE PROCEDURE &GEXADMIN..et_filearchive_settings
(   
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID    IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted
UseArchiveSettings OUT NUMBER,  -- Return 1 if the Archivesettings should be used, 0 else    
MovePath   OUT VARCHAR2, -- Return the path to use if the file should be moved after insertion (DataPump settings)
FtpPort    OUT NUMBER,  -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)   
FtpServer   OUT VARCHAR2, -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpUser    OUT VARCHAR2, -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPassword   OUT VARCHAR2, -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPath    OUT VARCHAR2 -- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
)   
IS
BEGIN
 MovePath := '';   
 FtpServer := '';   
 FtpPort := 21;   
 FtpUser := '';   
 FtpPassword := '';   
 FtpPath := '';   
 UseArchiveSettings := 0;   
 RETURN;   
END;
/

--
-- Procedure wt_insertion_validation
--
CREATE PROCEDURE &GEXADMIN..wt_insertion_validation
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be validated
LotID    IN VARCHAR2, -- Lot of the splitlot to be validated
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted
TrackingLotID_Out OUT VARCHAR2, -- Tracking to be used in GexDB for this splitlot
LotID_Out   OUT VARCHAR2, -- LotID to be used in GexDB for this splitlot
WaferID_Out   OUT VARCHAR2, -- WaferID to be used in GexDB for this splitlot
ProductName   OUT VARCHAR2, -- Return the Product Name if it has to be overloaded
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 TrackingLotID_Out := TrackingLotID;
 LotID_Out := LotID;
 WaferID_Out := WaferID;
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure wt_insertion_postprocessing
--
CREATE PROCEDURE &GEXADMIN..wt_insertion_postprocessing
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure wt_insertion_status
--
CREATE PROCEDURE &GEXADMIN..wt_insertion_status
(
SplitlotID  IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID   IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID   IN VARCHAR2, -- WaferID of the splitlot to be inserted
Message   IN VARCHAR2, -- Error message in case the insertion failed
Status   IN NUMBER  -- Insertion status: 1 if insertion successful, 0 else
)
IS
BEGIN
 RETURN;
END;
/

--
-- Procedure wt_filearchive_settings
--
CREATE PROCEDURE &GEXADMIN..wt_filearchive_settings
(   
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID    IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted
UseArchiveSettings OUT NUMBER,  -- Return 1 if the Archivesettings should be used, 0 else    
MovePath   OUT VARCHAR2, -- Return the path to use if the file should be moved after insertion (DataPump settings)
FtpPort    OUT NUMBER,  -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)   
FtpServer   OUT VARCHAR2, -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpUser    OUT VARCHAR2, -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPassword   OUT VARCHAR2, -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPath    OUT VARCHAR2 -- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
)   
IS
BEGIN
 MovePath := '';
 FtpServer := '';
 FtpPort := 21;
 FtpUser := '';
 FtpPassword := '';
 FtpPath := '';
 UseArchiveSettings := 0;
 RETURN;
END;
/

--
-- Procedure ft_insertion_validation
--
CREATE PROCEDURE &GEXADMIN..ft_insertion_validation
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be validated
LotID    IN VARCHAR2, -- Lot of the splitlot to be validated
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted (not used for FT)
TrackingLotID_Out OUT VARCHAR2, -- Tracking to be used in GexDB for this splitlot
LotID_Out   OUT VARCHAR2, -- LotID to be used in GexDB for this splitlot
WaferID_Out   OUT VARCHAR2, -- WaferID to be used in GexDB for this splitlot
ProductName   OUT VARCHAR2, -- Return the Product Name if it has to be overloaded
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 TrackingLotID_Out := TrackingLotID;
 LotID_Out := LotID;
 WaferID_Out := WaferID;
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure ft_insertion_postprocessing
--
CREATE PROCEDURE &GEXADMIN..ft_insertion_postprocessing
(
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be validated
Message    OUT VARCHAR2, -- Return the Error message in case the validation fails
Status    OUT NUMBER  -- Return the validation status: 1 if validation successful, 0 else
)
IS
BEGIN
 -- Init variables
 Message := 'Success';
 Status := 1;
 RETURN;
END;
/

--
-- Procedure ft_insertion_status
--
CREATE PROCEDURE &GEXADMIN..ft_insertion_status
(
SplitlotID  IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID   IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID   IN VARCHAR2, -- WaferID of the splitlot to be inserted (not used for FT)
Message   IN VARCHAR2, -- Error message in case the insertion failed
Status   IN NUMBER  -- Insertion status: 1 if insertion successful, 0 else
)
IS
BEGIN
 RETURN;
END;
/

--
-- Procedure ft_filearchive_settings
--
CREATE PROCEDURE &GEXADMIN..ft_filearchive_settings
(   
SplitlotID   IN NUMBER,  -- SplitlotId of the splitlot to be inserted
TrackingLotID  IN VARCHAR2, -- Tracking lot of the splitlot to be inserted
LotID    IN VARCHAR2, -- Lot of the splitlot to be inserted
WaferID    IN VARCHAR2, -- WaferID of the splitlot to be inserted (not used for FT)
UseArchiveSettings OUT NUMBER,  -- Return 1 if the Archivesettings should be used, 0 else    
MovePath   OUT VARCHAR2, -- Return the path to use if the file should be moved after insertion (DataPump settings)
FtpPort    OUT NUMBER,  -- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)   
FtpServer   OUT VARCHAR2, -- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpUser    OUT VARCHAR2, -- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPassword   OUT VARCHAR2, -- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
FtpPath    OUT VARCHAR2 -- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
)   
IS
BEGIN
 MovePath := '';   
 FtpServer := '';   
 FtpPort := 21;   
 FtpUser := '';   
 FtpPassword := '';   
 FtpPath := '';   
 UseArchiveSettings := 0;   
 RETURN;   
END;
/

exit;
