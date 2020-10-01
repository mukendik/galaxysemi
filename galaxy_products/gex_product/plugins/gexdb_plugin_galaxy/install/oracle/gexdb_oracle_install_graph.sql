--
-- First arg is the GexAdmin name
--

DEFINE GEXADMIN=&1
DEFINE LOGGINGMODE=&2

--
-- Create GEXDB users
--

CREATE USER &GEXADMIN. PROFILE DEFAULT
IDENTIFIED BY gexadmin DEFAULT TABLESPACE USERS
ACCOUNT UNLOCK;
GRANT CONNECT TO &GEXADMIN.;
GRANT RESOURCE TO &GEXADMIN.;

--
-- Create GEXDB tables: GLOBAL TABLES
--

CREATE TABLE &GEXADMIN..global_info (
	db_version_name			VARCHAR2(255)	NOT NULL,
	db_version_nb			NUMBER(6)		NOT NULL,
	db_version_build		NUMBER(6)		NOT NULL,
	incremental_splitlots	NUMBER(10)		NOT NULL
) NOLOGGING;

CREATE TABLE &GEXADMIN..incremental_update (
	db_update_name			VARCHAR2(255)	NOT NULL,
	initial_splitlots		NUMBER(10)		DEFAULT 0 NOT NULL,
	remaining_splitlots		NUMBER(10)		DEFAULT 0 NOT NULL,
	db_version_build		NUMBER(6)		NOT NULL
) NOLOGGING;

CREATE TABLE &GEXADMIN..product (
	product_name		VARCHAR2(255)	DEFAULT '' NOT NULL,
	description			VARCHAR2(1000)	DEFAULT NULL,
	CONSTRAINT pk_product PRIMARY KEY(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..file_host (
	file_host_id		NUMBER(10)		NOT NULL,
	host_name			VARCHAR2(255)	DEFAULT '',
	host_ftpuser		VARCHAR2(255)	DEFAULT '',
	host_ftppassword	VARCHAR2(255)	DEFAULT '',
	host_ftppath		VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_file_host PRIMARY KEY(file_host_id)
) NOLOGGING;

--
-- Create GEXDB tables: ELECTRICAL TEST TABLES
--

CREATE TABLE &GEXADMIN..et_syl_set (
	syl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1 NOT NULL,
	CONSTRAINT pk_et_syl_set PRIMARY KEY(syl_id),
	CONSTRAINT fk1_et_syl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_sbl_set (
	sbl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_et_sbl_set PRIMARY KEY(sbl_id),
	CONSTRAINT fk1_et_sbl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_sbl (
	sbl_id				NUMBER(10)		NOT NULL,
	bin_no				NUMBER			NOT NULL,
	bin_name			VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1NOT NULL,
	CONSTRAINT pk_et_sbl PRIMARY KEY(sbl_id,bin_no),
	CONSTRAINT fk1_et_sbl FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..et_sbl_set(sbl_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_wyr_format (
	site_name				VARCHAR2(255)	NOT NULL,
	column_id				NUMBER(3)		NOT NULL,
	column_nb				NUMBER(3)		NOT NULL,
	column_name				VARCHAR2(255)	NOT NULL,
	data_type				VARCHAR2(255)	NOT NULL,
	display					CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_et_wyr_format PRIMARY KEY(site_name,column_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_wyr (
	site_name				VARCHAR2(255)	NOT NULL,
	week_nb					NUMBER(3)		DEFAULT NULL,
	year					NUMBER(5)		DEFAULT NULL,
	date_in					DATE			DEFAULT NULL,
	date_out				DATE			DEFAULT NULL,
	product_name			VARCHAR2(255)	DEFAULT NULL,
	program_name			VARCHAR2(255)	DEFAULT NULL,
	tester_name				VARCHAR2(255)	DEFAULT NULL,
	lot_id					VARCHAR2(255)	DEFAULT NULL,
	subcon_lot_id			VARCHAR2(255)	DEFAULT NULL,
	user_split				VARCHAR2(1024)	DEFAULT NULL,
	yield					NUMBER			DEFAULT 0,
	parts_received			NUMBER(8)		DEFAULT 0,
	pretest_rejects			NUMBER(8)		DEFAULT 0,
	pretest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_tested			NUMBER(8)		DEFAULT 0,
	parts_pass				NUMBER(8)		DEFAULT 0,
	parts_pass_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_fail				NUMBER(8)		DEFAULT 0,
	parts_fail_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_retest			NUMBER(8)		DEFAULT 0,
	parts_retest_split		VARCHAR2(1024)	DEFAULT NULL,
	insertions				NUMBER(8)		DEFAULT 0,
	posttest_rejects		NUMBER(8)		DEFAULT 0,
	posttest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_shipped			NUMBER(8)		DEFAULT 0
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_metadata_mapping (
	meta_name				VARCHAR2(255)	NOT NULL,
	gex_name				VARCHAR2(255)	DEFAULT NULL,
	gexdb_table_name		VARCHAR2(255)	NOT NULL,
	gexdb_field_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	gex_display_in_gui		CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_et_metadata_mapping	PRIMARY KEY	(meta_name),
	CONSTRAINT uk1_et_metadata_mapping	UNIQUE		(gex_name),
	CONSTRAINT uk2_et_metadata_mapping	UNIQUE		(gexdb_field_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_metadata_link (
	link_name				VARCHAR2(255)	NOT NULL,
	gexdb_table1_name		VARCHAR2(255)	NOT NULL,
	gexdb_field1_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_table2_name		VARCHAR2(255)	NOT NULL,
	gexdb_field2_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_et_metadata_link		PRIMARY KEY(link_name),
	CONSTRAINT uk1_et_metadata_link		UNIQUE		(gexdb_field1_fullname,gexdb_field2_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_lot (
	lot_id				VARCHAR2(255)	DEFAULT '' NOT NULL,
	tracking_lot_id		VARCHAR2(255)	DEFAULT NULL,
	product_name		VARCHAR2(255)	DEFAULT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(2)			DEFAULT NULL,
	CONSTRAINT pk_et_lot PRIMARY KEY(lot_id),
	CONSTRAINT fk1_et_lot FOREIGN KEY(product_name) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING ;

CREATE TABLE &GEXADMIN..et_lot_hbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	hbin_name 			VARCHAR2(255)			DEFAULT '',
	hbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_et_lot_hbin PRIMARY KEY(lot_id,hbin_no),
	CONSTRAINT fk1_et_lot_hbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..et_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_lot_sbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	sbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	sbin_name 			VARCHAR2(255)			DEFAULT '',
	sbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_et_lot_sbin PRIMARY KEY(lot_id,sbin_no),
	CONSTRAINT fk1_et_lot_sbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..et_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_wafer_info (
	lot_id				VARCHAR2(255)	DEFAULT '' NOT NULL,
	wafer_id			VARCHAR2(255)	DEFAULT NULL,
	fab_id				VARCHAR2(255)	DEFAULT NULL,
	frame_id			VARCHAR2(255)	DEFAULT NULL,
	mask_id				VARCHAR2(255)	DEFAULT NULL,
	wafer_size			NUMBER			DEFAULT NULL,
	die_ht				NUMBER			DEFAULT NULL,
	die_wid				NUMBER			DEFAULT NULL,
	wafer_units			NUMBER(3)		DEFAULT NULL,
	wafer_flat			CHAR(1)			DEFAULT NULL,
	center_x			NUMBER(5)		DEFAULT NULL,
	center_y			NUMBER(5)		DEFAULT NULL,
	pos_x				CHAR(1)			DEFAULT NULL,
	pos_y				CHAR(1)			DEFAULT NULL,
	gross_die			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(2)			DEFAULT 0,
	CONSTRAINT pk_et_wafer_info PRIMARY KEY(lot_id,wafer_id),
	CONSTRAINT fk1_et_wafer_info FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..et_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_wafer_hbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	wafer_id			VARCHAR2(255)			DEFAULT NULL,
	hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	hbin_name 			VARCHAR2(255)			DEFAULT '',
	hbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_et_wafer_hbin PRIMARY KEY(lot_id,wafer_id,hbin_no),
	CONSTRAINT fk1_et_wafer_hbin FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..et_wafer_info(lot_id,wafer_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_wafer_sbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	wafer_id			VARCHAR2(255)			DEFAULT NULL,
	sbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	sbin_name 			VARCHAR2(255)			DEFAULT '',
	sbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_et_wafer_sbin PRIMARY KEY(lot_id,wafer_id,sbin_no),
	CONSTRAINT fk1_et_wafer_sbin FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..et_wafer_info(lot_id,wafer_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_splitlot (
	splitlot_id			NUMBER(10)		NOT NULL,
	lot_id				VARCHAR2(255)	DEFAULT 0 NOT NULL,
	sublot_id			VARCHAR2(255)	DEFAULT '',
	start_t				NUMBER(10)		DEFAULT 0 NOT NULL,
	finish_t			NUMBER(10)		DEFAULT 0 NOT NULL,
	week_nb				NUMBER(3)		DEFAULT 0 NOT NULL,
	year				NUMBER(5)		DEFAULT 0 NOT NULL,
	tester_name			VARCHAR2(255)	DEFAULT '',
	tester_type			VARCHAR2(255)	DEFAULT '',
	flags				CHAR(2)			DEFAULT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	data_provider		VARCHAR2(255)	DEFAULT '',
	data_type			VARCHAR2(255)	DEFAULT '',
	prod_data			CHAR(1)			DEFAULT 'Y' NOT NULL,
	job_nam				VARCHAR2(255)	DEFAULT NULL,
	job_rev				VARCHAR2(255)	DEFAULT NULL,
	oper_nam			VARCHAR2(255)	DEFAULT NULL,
	exec_typ			VARCHAR2(255)	DEFAULT NULL,
	exec_ver			VARCHAR2(255)	DEFAULT NULL,
	facil_id			VARCHAR2(255)	DEFAULT NULL,
	part_typ			VARCHAR2(256)	DEFAULT NULL,	
	user_txt			VARCHAR2(256)	DEFAULT NULL,	
	famly_id			VARCHAR2(256)	DEFAULT NULL,	
	proc_id				VARCHAR2(256)	DEFAULT NULL,
	file_host_id		NUMBER(10)		DEFAULT 0,
	file_path			VARCHAR2(255)	DEFAULT '',
	file_name			VARCHAR2(255)	DEFAULT '',
	valid_splitlot		CHAR(1)			DEFAULT 'N' NOT NULL,
	insertion_time		NUMBER(10)		DEFAULT NULL,
	subcon_lot_id		VARCHAR2(255)	DEFAULT '',
	wafer_id 			VARCHAR2(255)	DEFAULT NULL,
	syl_id				NUMBER(10),
	sbl_id				NUMBER(10),
	CONSTRAINT pk_et_splitlot PRIMARY KEY(splitlot_id),
	CONSTRAINT fk1_et_splitlot FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..et_lot(lot_id),
	CONSTRAINT fk2_et_splitlot FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..et_wafer_info(lot_id,wafer_id),
	CONSTRAINT fk3_et_splitlot FOREIGN KEY(syl_id) REFERENCES &GEXADMIN..et_syl_set(syl_id),
	CONSTRAINT fk4_et_splitlot FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..et_sbl_set(sbl_id),
	CONSTRAINT fk5_et_splitlot FOREIGN KEY(file_host_id) REFERENCES &GEXADMIN..file_host(file_host_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_prod_alarm (
	splitlot_id			NUMBER(10)		NOT NULL,
	alarm_cat			VARCHAR2(255)	NOT NULL,
	alarm_type			VARCHAR2(255)	NOT NULL,
	item_no				NUMBER			NOT NULL,
	item_name			VARCHAR2(255)	DEFAULT NULL,
	flags				CHAR(2)			NOT NULL,
	lcl					NUMBER			DEFAULT 0 NOT NULL,
	ucl					NUMBER			DEFAULT 0 NOT NULL,
	value				NUMBER			DEFAULT 0 NOT NULL,
	units				VARCHAR2(10)	DEFAULT NULL,
	CONSTRAINT fk1_et_prod_alarm FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..et_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_hbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	hbin_name			VARCHAR2(255)	DEFAULT '',
	hbin_cat			CHAR(1)			DEFAULT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_et_hbin PRIMARY KEY(splitlot_id,hbin_no),
	CONSTRAINT fk1_et_hbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..et_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_sbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_name			VARCHAR2(255)	DEFAULT '',
	sbin_cat			CHAR(1)			DEFAULT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_et_sbin PRIMARY KEY(splitlot_id,sbin_no),
	CONSTRAINT fk1_et_sbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..et_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_run (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	part_id				VARCHAR2(255)	DEFAULT NULL,
	part_x				NUMBER(6)		DEFAULT NULL,
	part_y				NUMBER(6)		DEFAULT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	tests_executed		NUMBER(5)		DEFAULT 0 NOT NULL,
	tests_failed		NUMBER(5)		DEFAULT 0 NOT NULL,
	firstfail_tnum		NUMBER(10)		DEFAULT NULL,
	firstfail_tname		VARCHAR2(255)	DEFAULT NULL,
	retest_index		NUMBER(3)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_et_run PRIMARY KEY(splitlot_id,run_id),
	CONSTRAINT fk1_et_run_1 FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..et_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_ptest_info (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	tnum				NUMBER(10)		DEFAULT 0 NOT NULL,
	tname				VARCHAR2(255)	DEFAULT '',
	units				VARCHAR2(255)	DEFAULT '',
	flags				CHAR(1)			DEFAULT NULL,
	ll					NUMBER			DEFAULT NULL,
	hl					NUMBER			DEFAULT NULL,
	testseq				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT pk_et_ptest_info PRIMARY KEY(splitlot_id,ptest_info_id), 
	CONSTRAINT fk1_et_ptest_info FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..et_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_ptest_stats (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	exec_count			NUMBER(5)		DEFAULT NULL,
	fail_count			NUMBER(5)		DEFAULT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	CONSTRAINT pk_et_ptest_stats PRIMARY KEY(splitlot_id,ptest_info_id), 
	CONSTRAINT fk1_et_ptest_stats FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..et_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..et_ptest_results (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	flags				CHAR(1)			DEFAULT NULL,
	value				NUMBER			DEFAULT NULL,
	CONSTRAINT pk_et_ptest_results PRIMARY KEY(splitlot_id,run_id,ptest_info_id), 
	CONSTRAINT fk1_et_ptest_results FOREIGN KEY(splitlot_id,run_id) REFERENCES &GEXADMIN..et_run(splitlot_id,run_id),
	CONSTRAINT fk2_et_ptest_results FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..et_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

--
-- Create GEXDB tables: WAFER SORT TABLES
--

CREATE TABLE &GEXADMIN..wt_syl_set (
	syl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1 NOT NULL,
	CONSTRAINT pk_wt_syl_set PRIMARY KEY(syl_id),
	CONSTRAINT fk1_wt_syl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_sbl_set (
	sbl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_wt_sbl_set PRIMARY KEY(sbl_id),
	CONSTRAINT fk1_wt_sbl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_sbl (
	sbl_id				NUMBER(10)		NOT NULL,
	bin_no				NUMBER			NOT NULL,
	bin_name			VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1NOT NULL,
	CONSTRAINT pk_wt_sbl PRIMARY KEY(sbl_id,bin_no),
	CONSTRAINT fk1_wt_sbl FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..wt_sbl_set(sbl_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_wyr_format (
	site_name				VARCHAR2(255)	NOT NULL,
	column_id				NUMBER(3)		NOT NULL,
	column_nb				NUMBER(3)		NOT NULL,
	column_name				VARCHAR2(255)	NOT NULL,
	data_type				VARCHAR2(255)	NOT NULL,
	display					CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_wt_wyr_format PRIMARY KEY(site_name,column_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_wyr (
	site_name				VARCHAR2(255)	NOT NULL,
	week_nb					NUMBER(3)		DEFAULT NULL,
	year					NUMBER(5)		DEFAULT NULL,
	date_in					DATE			DEFAULT NULL,
	date_out				DATE			DEFAULT NULL,
	product_name			VARCHAR2(255)	DEFAULT NULL,
	program_name			VARCHAR2(255)	DEFAULT NULL,
	tester_name				VARCHAR2(255)	DEFAULT NULL,
	lot_id					VARCHAR2(255)	DEFAULT NULL,
	subcon_lot_id			VARCHAR2(255)	DEFAULT NULL,
	user_split				VARCHAR2(1024)	DEFAULT NULL,
	yield					NUMBER			DEFAULT 0,
	parts_received			NUMBER(8)		DEFAULT 0,
	pretest_rejects			NUMBER(8)		DEFAULT 0,
	pretest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_tested			NUMBER(8)		DEFAULT 0,
	parts_pass				NUMBER(8)		DEFAULT 0,
	parts_pass_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_fail				NUMBER(8)		DEFAULT 0,
	parts_fail_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_retest			NUMBER(8)		DEFAULT 0,
	parts_retest_split		VARCHAR2(1024)	DEFAULT NULL,
	insertions				NUMBER(8)		DEFAULT 0,
	posttest_rejects		NUMBER(8)		DEFAULT 0,
	posttest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_shipped			NUMBER(8)		DEFAULT 0
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_metadata_mapping (
	meta_name				VARCHAR2(255)	NOT NULL,
	gex_name				VARCHAR2(255)	DEFAULT NULL,
	gexdb_table_name		VARCHAR2(255)	NOT NULL,
	gexdb_field_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	gex_display_in_gui		CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_wt_metadata_mapping	PRIMARY KEY	(meta_name),
	CONSTRAINT uk1_wt_metadata_mapping	UNIQUE		(gex_name),
	CONSTRAINT uk2_wt_metadata_mapping	UNIQUE		(gexdb_field_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_metadata_link (
	link_name				VARCHAR2(255)	NOT NULL,
	gexdb_table1_name		VARCHAR2(255)	NOT NULL,
	gexdb_field1_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_table2_name		VARCHAR2(255)	NOT NULL,
	gexdb_field2_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_wt_metadata_link		PRIMARY KEY(link_name),
	CONSTRAINT uk1_wt_metadata_link		UNIQUE		(gexdb_field1_fullname,gexdb_field2_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_lot (
	lot_id				VARCHAR2(255)	DEFAULT '' NOT NULL,
	tracking_lot_id		VARCHAR2(255)	DEFAULT NULL,
	product_name		VARCHAR2(255)	DEFAULT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(2)			DEFAULT NULL,
	CONSTRAINT pk_wt_lot PRIMARY KEY(lot_id),
	CONSTRAINT fk1_wt_lot FOREIGN KEY(product_name) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_lot_hbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	hbin_name 			VARCHAR2(255)			DEFAULT '',
	hbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_wt_lot_hbin PRIMARY KEY(lot_id,hbin_no),
	CONSTRAINT fk1_wt_lot_hbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..wt_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_lot_sbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	sbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	sbin_name 			VARCHAR2(255)			DEFAULT '',
	sbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_wt_lot_sbin PRIMARY KEY(lot_id,sbin_no),
	CONSTRAINT fk1_wt_lot_sbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..wt_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_wafer_info (
	lot_id				VARCHAR2(255)	DEFAULT '' NOT NULL,
	wafer_id			VARCHAR2(255)	DEFAULT NULL,
	fab_id				VARCHAR2(255)	DEFAULT NULL,
	frame_id			VARCHAR2(255)	DEFAULT NULL,
	mask_id				VARCHAR2(255)	DEFAULT NULL,
	wafer_size			NUMBER			DEFAULT NULL,
	die_ht				NUMBER			DEFAULT NULL,
	die_wid				NUMBER			DEFAULT NULL,
	wafer_units			NUMBER(3)		DEFAULT NULL,
	wafer_flat			CHAR(1)			DEFAULT NULL,
	center_x			NUMBER(5)		DEFAULT NULL,
	center_y			NUMBER(5)		DEFAULT NULL,
	pos_x				CHAR(1)			DEFAULT NULL,
	pos_y				CHAR(1)			DEFAULT NULL,
	gross_die			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(2)			DEFAULT 0,
	CONSTRAINT pk_wt_wafer_info PRIMARY KEY(lot_id,wafer_id),
	CONSTRAINT fk1_wt_wafer_info FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..wt_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_wafer_hbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	wafer_id			VARCHAR2(255)			DEFAULT NULL,
	hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	hbin_name 			VARCHAR2(255)			DEFAULT '',
	hbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_wt_wafer_hbin PRIMARY KEY(lot_id,wafer_id,hbin_no),
	CONSTRAINT fk1_wt_wafer_hbin FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..wt_wafer_info(lot_id,wafer_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_wafer_sbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	wafer_id			VARCHAR2(255)			DEFAULT NULL,
	sbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	sbin_name 			VARCHAR2(255)			DEFAULT '',
	sbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_wt_wafer_sbin PRIMARY KEY(lot_id,wafer_id,sbin_no),
	CONSTRAINT fk1_wt_wafer_sbin FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..wt_wafer_info(lot_id,wafer_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_splitlot (
	splitlot_id				NUMBER(10)		NOT NULL,
	lot_id					VARCHAR2(255)	DEFAULT 0 NOT NULL,
	sublot_id				VARCHAR2(255)	DEFAULT '',
	setup_t					NUMBER(10)		DEFAULT 0 NOT NULL,
	start_t					NUMBER(10)		DEFAULT 0 NOT NULL,
	finish_t				NUMBER(10)		DEFAULT 0 NOT NULL,
	week_nb					NUMBER(3)		DEFAULT 0 NOT NULL,
	year					NUMBER(5)		DEFAULT 0 NOT NULL,
	stat_num				NUMBER(3)		DEFAULT 0 NOT NULL,
	tester_name				VARCHAR2(255)	DEFAULT '',
	tester_type				VARCHAR2(255)	DEFAULT '',
	flags					CHAR(2)			DEFAULT NULL,
	nb_parts				NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_samples		NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_samples_good	NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_summary		NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_summary_good	NUMBER(8)		DEFAULT 0 NOT NULL,
	data_provider			VARCHAR2(255)	DEFAULT '',
	data_type				VARCHAR2(255)	DEFAULT '',
	prod_data				CHAR(1)			DEFAULT 'Y' NOT NULL,
	retest_index			NUMBER(3)		DEFAULT 0 NOT NULL,
	retest_hbins			VARCHAR2(255)	DEFAULT NULL,
	rework_code				NUMBER(3)		DEFAULT 0 NOT NULL,
	job_nam					VARCHAR2(255)	DEFAULT NULL,
	job_rev					VARCHAR2(255)	DEFAULT NULL,
	oper_nam				VARCHAR2(255)	DEFAULT NULL,
	exec_typ				VARCHAR2(255)	DEFAULT NULL,
	exec_ver				VARCHAR2(255)	DEFAULT NULL,
	test_cod				VARCHAR2(255)	DEFAULT NULL,
	facil_id				VARCHAR2(255)	DEFAULT NULL,
	tst_temp				VARCHAR2(255)	DEFAULT NULL,
	mode_cod				CHAR(1)			DEFAULT NULL,	
	rtst_cod				CHAR(1)			DEFAULT NULL,	
	prot_cod				CHAR(1)			DEFAULT NULL,	
	burn_tim				NUMBER(5)		DEFAULT NULL,	
	cmod_cod				CHAR(1)			DEFAULT NULL,	
	part_typ				VARCHAR2(256)	DEFAULT NULL,	
	user_txt				VARCHAR2(256)	DEFAULT NULL,	
	aux_file				VARCHAR2(256)	DEFAULT NULL,	
	pkg_typ					VARCHAR2(256)	DEFAULT NULL,
	famly_id				VARCHAR2(256)	DEFAULT NULL,	
	date_cod				VARCHAR2(256)	DEFAULT NULL,	
	floor_id				VARCHAR2(256)	DEFAULT NULL,	
	proc_id					VARCHAR2(256)	DEFAULT NULL,
	oper_frq				VARCHAR2(256)	DEFAULT NULL,	
	spec_nam				VARCHAR2(256)	DEFAULT NULL,	
	spec_ver				VARCHAR2(256)	DEFAULT NULL,
	flow_id					VARCHAR2(256)	DEFAULT NULL,
	setup_id				VARCHAR2(256)	DEFAULT NULL,
	dsgn_rev				VARCHAR2(256)	DEFAULT NULL,
	eng_id					VARCHAR2(256)	DEFAULT NULL,
	rom_cod					VARCHAR2(256)	DEFAULT NULL,
	serl_num				VARCHAR2(256)	DEFAULT NULL,
	supr_nam				VARCHAR2(256)	DEFAULT NULL,
	nb_sites				NUMBER(3)		DEFAULT 1 NOT NULL,
	head_num				NUMBER(3)		DEFAULT NULL,
	handler_typ				VARCHAR2(255)	DEFAULT NULL,
	handler_id				VARCHAR2(255)	DEFAULT NULL,
	card_typ				VARCHAR2(255)	DEFAULT NULL,
	card_id					VARCHAR2(255)	DEFAULT NULL,
	loadboard_typ			VARCHAR2(255)	DEFAULT NULL,
	loadboard_id			VARCHAR2(255)	DEFAULT NULL,
	dib_typ					VARCHAR2(255)	DEFAULT NULL,
	dib_id					VARCHAR2(255)	DEFAULT NULL,
	cable_typ				VARCHAR2(255)	DEFAULT NULL,
	cable_id				VARCHAR2(255)	DEFAULT NULL,
	contactor_typ			VARCHAR2(255)	DEFAULT NULL,
	contactor_id			VARCHAR2(255)	DEFAULT NULL,
	laser_typ				VARCHAR2(255)	DEFAULT NULL,
	laser_id				VARCHAR2(255)	DEFAULT NULL,
	extra_typ				VARCHAR2(255)	DEFAULT NULL,
	extra_id				VARCHAR2(255)	DEFAULT NULL,
	file_host_id			NUMBER(10)		DEFAULT 0,
	file_path				VARCHAR2(255)	DEFAULT '',
	file_name				VARCHAR2(255)	DEFAULT '',
	valid_splitlot			CHAR(1)			DEFAULT 'N' NOT NULL,
	insertion_time			NUMBER(10)		DEFAULT NULL,
	subcon_lot_id			VARCHAR2(255)	DEFAULT '',
	wafer_id 				VARCHAR2(255)	DEFAULT NULL,
	syl_id					NUMBER(10),
	sbl_id					NUMBER(10),
	CONSTRAINT pk_wt_splitlot PRIMARY KEY(splitlot_id),
	CONSTRAINT fk1_wt_splitlot FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..wt_lot(lot_id),
	CONSTRAINT fk2_wt_splitlot FOREIGN KEY(lot_id,wafer_id) REFERENCES &GEXADMIN..wt_wafer_info(lot_id,wafer_id),
	CONSTRAINT fk3_wt_splitlot FOREIGN KEY(syl_id) REFERENCES &GEXADMIN..wt_syl_set(syl_id),
	CONSTRAINT fk4_wt_splitlot FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..wt_sbl_set(sbl_id),
	CONSTRAINT fk5_wt_splitlot FOREIGN KEY(file_host_id) REFERENCES &GEXADMIN..file_host(file_host_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_prod_alarm (
	splitlot_id			NUMBER(10)		NOT NULL,
	alarm_cat			VARCHAR2(255)	NOT NULL,
	alarm_type			VARCHAR2(255)	NOT NULL,
	item_no				NUMBER			NOT NULL,
	item_name			VARCHAR2(255)	DEFAULT NULL,
	flags				CHAR(2)			NOT NULL,
	lcl					NUMBER			DEFAULT 0 NOT NULL,
	ucl					NUMBER			DEFAULT 0 NOT NULL,
	value				NUMBER			DEFAULT 0 NOT NULL,
	units				VARCHAR2(10)	DEFAULT NULL,
	CONSTRAINT fk1_wt_prod_alarm FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_parts_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_parts_stats_samples PRIMARY KEY(splitlot_id,site_no),
	CONSTRAINT fk1_wt_parts_stats_samples FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_parts_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_good				NUMBER(8)		DEFAULT NULL,
	nb_rtst				NUMBER(8)		DEFAULT NULL,
	CONSTRAINT pk_wt_parts_stats_summary PRIMARY KEY(splitlot_id,site_no),
	CONSTRAINT fk1_wt_parts_stats_summary FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_hbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	hbin_name			VARCHAR2(255)	DEFAULT '',
	hbin_cat			CHAR(1)			DEFAULT NULL,
	CONSTRAINT pk_wt_hbin PRIMARY KEY(splitlot_id,hbin_no),
	CONSTRAINT fk1_wt_hbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_hbin_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_hbin_stats_samples PRIMARY KEY(splitlot_id,hbin_no,site_no), 
	CONSTRAINT fk1_wt_hbin_stats_samples FOREIGN KEY(splitlot_id,hbin_no) REFERENCES &GEXADMIN..wt_hbin(splitlot_id,hbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_hbin_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_hbin_stats_summary PRIMARY KEY(splitlot_id,hbin_no,site_no), 
	CONSTRAINT fk1_wt_hbin_stats_summary FOREIGN KEY(splitlot_id,hbin_no) REFERENCES &GEXADMIN..wt_hbin(splitlot_id,hbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_sbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_name			VARCHAR2(255)	DEFAULT '',
	sbin_cat			CHAR(1)			DEFAULT NULL,
	CONSTRAINT pk_wt_sbin PRIMARY KEY(splitlot_id,sbin_no),
	CONSTRAINT fk1_wt_sbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_sbin_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_sbin_stats_samples PRIMARY KEY(splitlot_id,sbin_no,site_no), 
	CONSTRAINT fk1_wt_sbin_stats_samples FOREIGN KEY(splitlot_id,sbin_no) REFERENCES &GEXADMIN..wt_sbin(splitlot_id,sbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_sbin_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_sbin_stats_summary PRIMARY KEY(splitlot_id,sbin_no,site_no), 
	CONSTRAINT fk1_wt_sbin_stats_summary FOREIGN KEY(splitlot_id,sbin_no) REFERENCES &GEXADMIN..wt_sbin(splitlot_id,sbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_run (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	part_id				VARCHAR2(255)	DEFAULT NULL,
	part_x				NUMBER(6)		DEFAULT NULL,
	part_y				NUMBER(6)		DEFAULT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	tests_executed		NUMBER(5)		DEFAULT 0 NOT NULL,
	tests_failed		NUMBER(5)		DEFAULT 0 NOT NULL,
	firstfail_tnum		NUMBER(10)		DEFAULT NULL,
	firstfail_tname		VARCHAR2(255)	DEFAULT NULL,
	retest_index		NUMBER(3)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_run PRIMARY KEY(splitlot_id,run_id),
	CONSTRAINT fk1_wt_run FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ptest_info (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	tnum				NUMBER(10)		DEFAULT 0 NOT NULL,
	tname				VARCHAR2(255)	DEFAULT '',
	units				VARCHAR2(255)	DEFAULT '',
	flags				CHAR(1)			DEFAULT NULL,
	ll					NUMBER			DEFAULT NULL,
	hl					NUMBER			DEFAULT NULL,
	testseq				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT pk_wt_ptest_info PRIMARY KEY(splitlot_id,ptest_info_id), 
	CONSTRAINT fk1_wt_ptest_info FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ptest_limits (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	ll					NUMBER			DEFAULT 0 NOT NULL,
	hl					NUMBER			DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_ptest_limits PRIMARY KEY(splitlot_id,ptest_info_id,site_no), 
	CONSTRAINT fk1_wt_ptest_limits FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..wt_ptest_info(splitlot_id,ptest_in	fo_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ptest_results (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	flags				CHAR(1)			DEFAULT NULL,
	value				NUMBER			DEFAULT NULL,
	CONSTRAINT pk_wt_ptest_results PRIMARY KEY(splitlot_id,run_id,ptest_info_id), 
	CONSTRAINT fk1_wt_ptest_results FOREIGN KEY(splitlot_id,run_id) REFERENCES &GEXADMIN..wt_run(splitlot_id,run_id),
	CONSTRAINT fk2_wt_ptest_results FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..wt_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ptest_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	CONSTRAINT pk_wt_ptest_stats_samples PRIMARY KEY(splitlot_id,ptest_info_id,site_no), 
	CONSTRAINT fk1_wt_ptest_stats_samples FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..wt_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ptest_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT pk_wt_ptest_stats_summary PRIMARY KEY(splitlot_id,ptest_info_id,site_no), 
	CONSTRAINT fk1_wt_ptest_stats_summary FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..wt_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_mptest_info (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	mptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	tnum				NUMBER(10)		DEFAULT 0 NOT NULL,
	tname				VARCHAR2(255)	DEFAULT '',
	tpin_arrayindex		NUMBER(6)		DEFAULT 0 NOT NULL,
	units				VARCHAR2(255)	DEFAULT '',
	flags				CHAR(1)			DEFAULT NULL,
	ll					NUMBER			DEFAULT NULL,
	hl					NUMBER			DEFAULT NULL,
	testseq				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT pk_wt_mptest_info PRIMARY KEY(splitlot_id,mptest_info_id), 
	CONSTRAINT fk1_wt_mptest_info FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_mptest_limits (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	mptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	ll					NUMBER			DEFAULT 0 NOT NULL,
	hl					NUMBER			DEFAULT 0 NOT NULL,
	CONSTRAINT pk_wt_mptest_limits PRIMARY KEY(splitlot_id,mptest_info_id,site_no), 
	CONSTRAINT fk1_wt_mptest_limits FOREIGN KEY(splitlot_id,mptest_info_id) REFERENCES &GEXADMIN..wt_mptest_info(splitlot_id,mptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_mptest_results (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	mptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	flags				CHAR(1)			DEFAULT NULL,
	value				NUMBER			DEFAULT 0 NOT NULL,
	tpin_pmrindex		NUMBER(6)		DEFAULT 0,
	CONSTRAINT pk_wt_mptest_results PRIMARY KEY(splitlot_id,run_id,mptest_info_id), 
	CONSTRAINT fk1_wt_mptest_results FOREIGN KEY(splitlot_id,run_id) REFERENCES &GEXADMIN..wt_run(splitlot_id,run_id),
	CONSTRAINT fk2_wt_mptest_results FOREIGN KEY(splitlot_id,mptest_info_id) REFERENCES &GEXADMIN..wt_mptest_info(splitlot_id,mptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_mptest_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	mptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	CONSTRAINT pk_wt_mptest_stats_samples PRIMARY KEY(splitlot_id,mptest_info_id,site_no), 
	CONSTRAINT fk1_wt_mptest_stats_samples FOREIGN KEY(splitlot_id,mptest_info_id) REFERENCES &GEXADMIN..wt_mptest_info(splitlot_id,mptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_mptest_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	mptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	fail_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT pk_wt_mptest_stats_summary PRIMARY KEY(splitlot_id,mptest_info_id,site_no), 
	CONSTRAINT fk1_wt_mptest_stats_summary FOREIGN KEY(splitlot_id,mptest_info_id) REFERENCES &GEXADMIN..wt_mptest_info(splitlot_id,mptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ftest_info (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ftest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	tnum				NUMBER(10)		DEFAULT 0 NOT NULL,
	tname				VARCHAR2(255)	DEFAULT '',
	testseq				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT pk_wt_ftest_info PRIMARY KEY(splitlot_id,ftest_info_id), 
	CONSTRAINT fk1_wt_ftest_info FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..wt_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ftest_results (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ftest_info_id		NUMBER(5)		NOT NULL,
	run_id				NUMBER(5)		DEFAULT 0 NOT NULL,
	flags				CHAR(1)			DEFAULT NULL,
	vect_nam			VARCHAR2(255)	DEFAULT '',
	vect_off			NUMBER(6)		DEFAULT NULL,
	CONSTRAINT pk_wt_ftest_results PRIMARY KEY(splitlot_id,run_id,ftest_info_id), 
	CONSTRAINT fk2_wt_ftest_results FOREIGN KEY(splitlot_id,ftest_info_id) REFERENCES &GEXADMIN..wt_ftest_info(splitlot_id,ftest_info_id)
	CONSTRAINT fk1_wt_ftest_results FOREIGN KEY(splitlot_id,run_id) REFERENCES &GEXADMIN..wt_run(splitlot_id,run_id),
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ftest_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ftest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	CONSTRAINT pk_wt_ftest_stats_samples PRIMARY KEY(splitlot_id,ftest_info_id,site_no), 
	CONSTRAINT fk1_wt_ftest_stats_samples FOREIGN KEY(splitlot_id,ftest_info_id) REFERENCES &GEXADMIN..wt_ftest_info(splitlot_id,ftest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..wt_ftest_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ftest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT pk_wt_ftest_stats_summary PRIMARY KEY(splitlot_id,ftest_info_id,site_no), 
	CONSTRAINT fk1_wt_ftest_stats_summary FOREIGN KEY(splitlot_id,ftest_info_id) REFERENCES &GEXADMIN..wt_ftest_info(splitlot_id,ftest_info_id)
) NOLOGGING;

--
-- Create GEXDB tables: FINAL TEST TABLES
--

CREATE TABLE &GEXADMIN..ft_syl_set (
	syl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1 NOT NULL,
	CONSTRAINT pk_ft_syl_set PRIMARY KEY(syl_id),
	CONSTRAINT fk1_ft_syl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_sbl_set (
	sbl_id				NUMBER(10)		NOT NULL,
	product_id			VARCHAR2(255)	NOT NULL,
	creation_date		DATE			NOT NULL,
	file_name			VARCHAR2(255)	DEFAULT NULL,
	user_comment		VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_ft_sbl_set PRIMARY KEY(sbl_id),
	CONSTRAINT fk1_ft_sbl_set FOREIGN KEY(product_id) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_sbl (
	sbl_id				NUMBER(10)		NOT NULL,
	bin_no				NUMBER			NOT NULL,
	bin_name			VARCHAR2(255)	DEFAULT NULL,
	ll					NUMBER(5)		DEFAULT -1 NOT NULL,
	hl					NUMBER(5)		DEFAULT -1NOT NULL,
	CONSTRAINT pk_ft_sbl PRIMARY KEY(sbl_id,bin_no),
	CONSTRAINT fk1_ft_sbl FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..ft_sbl_set(sbl_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_wyr_format (
	site_name				VARCHAR2(255)	NOT NULL,
	column_id				NUMBER(3)		NOT NULL,
	column_nb				NUMBER(3)		NOT NULL,
	column_name				VARCHAR2(255)	NOT NULL,
	data_type				VARCHAR2(255)	NOT NULL,
	display					CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_ft_wyr_format PRIMARY KEY(site_name,column_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_wyr (
	site_name				VARCHAR2(255)	NOT NULL,
	week_nb					NUMBER(3)		DEFAULT NULL,
	year					NUMBER(5)		DEFAULT NULL,
	date_in					DATE			DEFAULT NULL,
	date_out				DATE			DEFAULT NULL,
	product_name			VARCHAR2(255)	DEFAULT NULL,
	program_name			VARCHAR2(255)	DEFAULT NULL,
	tester_name				VARCHAR2(255)	DEFAULT NULL,
	lot_id					VARCHAR2(255)	DEFAULT NULL,
	subcon_lot_id			VARCHAR2(255)	DEFAULT NULL,
	user_split				VARCHAR2(1024)	DEFAULT NULL,
	yield					NUMBER			DEFAULT 0,
	parts_received			NUMBER(8)		DEFAULT 0,
	pretest_rejects			NUMBER(8)		DEFAULT 0,
	pretest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_tested			NUMBER(8)		DEFAULT 0,
	parts_pass				NUMBER(8)		DEFAULT 0,
	parts_pass_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_fail				NUMBER(8)		DEFAULT 0,
	parts_fail_split		VARCHAR2(1024)	DEFAULT NULL,
	parts_retest			NUMBER(8)		DEFAULT 0,
	parts_retest_split		VARCHAR2(1024)	DEFAULT NULL,
	insertions				NUMBER(8)		DEFAULT 0,
	posttest_rejects		NUMBER(8)		DEFAULT 0,
	posttest_rejects_split	VARCHAR2(1024)	DEFAULT NULL,
	parts_shipped			NUMBER(8)		DEFAULT 0
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_metadata_mapping (
	meta_name				VARCHAR2(255)	NOT NULL,
	gex_name				VARCHAR2(255)	DEFAULT NULL,
	gexdb_table_name		VARCHAR2(255)	NOT NULL,
	gexdb_field_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	gex_display_in_gui		CHAR(1)			DEFAULT 'Y' NOT NULL,
	CONSTRAINT pk_ft_metadata_mapping	PRIMARY KEY	(meta_name),
	CONSTRAINT uk1_ft_metadata_mapping	UNIQUE		(gex_name),
	CONSTRAINT uk2_ft_metadata_mapping	UNIQUE		(gexdb_field_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_metadata_link (
	link_name				VARCHAR2(255)	NOT NULL,
	gexdb_table1_name		VARCHAR2(255)	NOT NULL,
	gexdb_field1_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_table2_name		VARCHAR2(255)	NOT NULL,
	gexdb_field2_fullname	VARCHAR2(255)	NOT NULL,
	gexdb_link_name			VARCHAR2(255)	DEFAULT NULL,
	CONSTRAINT pk_ft_metadata_link		PRIMARY KEY(link_name),
	CONSTRAINT uk1_ft_metadata_link		UNIQUE		(gexdb_field1_fullname,gexdb_field2_fullname)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_lot (
	lot_id				VARCHAR2(255)	DEFAULT '' NOT NULL,
	tracking_lot_id		VARCHAR2(255)	DEFAULT NULL,
	product_name		VARCHAR2(255)	DEFAULT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(2)			DEFAULT NULL,
	CONSTRAINT pk_ft_lot PRIMARY KEY(lot_id),
	CONSTRAINT fk1_ft_lot FOREIGN KEY(product_name) REFERENCES &GEXADMIN..product(product_name)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_lot_hbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	hbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	hbin_name 			VARCHAR2(255)			DEFAULT '',
	hbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_ft_lot_hbin PRIMARY KEY(lot_id,hbin_no),
	CONSTRAINT fk1_ft_lot_hbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..ft_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_lot_sbin (
	lot_id				VARCHAR2(255)			NOT NULL,
	sbin_no 			NUMBER(5) 				DEFAULT '0' NOT NULL,
	sbin_name 			VARCHAR2(255)			DEFAULT '',
	sbin_cat 			CHAR(1)					DEFAULT NULL,
	nb_parts			NUMBER(8)				DEFAULT '0' NOT NULL, 
	CONSTRAINT pk_ft_lot_sbin PRIMARY KEY(lot_id,sbin_no),
	CONSTRAINT fk1_ft_lot_sbin FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..ft_lot(lot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_splitlot (
	splitlot_id			NUMBER(10)		NOT NULL,
	lot_id				VARCHAR2(255)	DEFAULT 0 NOT NULL,
	sublot_id			VARCHAR2(255)	DEFAULT '',
	start_t				NUMBER(10)		DEFAULT 0 NOT NULL,
	setup_t				NUMBER(10)		DEFAULT 0 NOT NULL,
	finish_t			NUMBER(10)		DEFAULT 0 NOT NULL,
	week_nb				NUMBER(3)		DEFAULT 0 NOT NULL,
	year				NUMBER(5)		DEFAULT 0 NOT NULL,
	stat_num			NUMBER(3)		DEFAULT 0 NOT NULL,
	tester_name			VARCHAR2(255)	DEFAULT '',
	tester_type			VARCHAR2(255)	DEFAULT '',
	flags				CHAR(2)			DEFAULT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_samples	NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_samples_good NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_summary	NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_summary_good NUMBER(8)		DEFAULT 0 NOT NULL,
	data_provider		VARCHAR2(255)	DEFAULT '',
	data_type			VARCHAR2(255)	DEFAULT '',
	prod_data			CHAR(1)			DEFAULT 'Y' NOT NULL,
	retest_index		NUMBER(3)		DEFAULT 0 NOT NULL,
	retest_hbins		VARCHAR2(255)	DEFAULT NULL,
	rework_code			NUMBER(3)		DEFAULT 0 NOT NULL,
	job_nam				VARCHAR2(255)	DEFAULT NULL,
	job_rev				VARCHAR2(255)	DEFAULT NULL,
	oper_nam			VARCHAR2(255)	DEFAULT NULL,
	exec_typ			VARCHAR2(255)	DEFAULT NULL,
	exec_ver			VARCHAR2(255)	DEFAULT NULL,
	test_cod			VARCHAR2(255)	DEFAULT NULL,
	facil_id			VARCHAR2(255)	DEFAULT NULL,
	tst_temp			VARCHAR2(255)	DEFAULT NULL,
	mode_cod			CHAR(1)			DEFAULT NULL,	
	rtst_cod			CHAR(1)			DEFAULT NULL,	
	prot_cod			CHAR(1)			DEFAULT NULL,	
	burn_tim			NUMBER(5)		DEFAULT NULL,	
	cmod_cod			CHAR(1)			DEFAULT NULL,	
	part_typ			VARCHAR2(256)	DEFAULT NULL,	
	user_txt			VARCHAR2(256)	DEFAULT NULL,	
	aux_file			VARCHAR2(256)	DEFAULT NULL,	
	pkg_typ				VARCHAR2(256)	DEFAULT NULL,
	famly_id			VARCHAR2(256)	DEFAULT NULL,	
	date_cod			VARCHAR2(256)	DEFAULT NULL,	
	floor_id			VARCHAR2(256)	DEFAULT NULL,	
	proc_id				VARCHAR2(256)	DEFAULT NULL,
	oper_frq			VARCHAR2(256)	DEFAULT NULL,	
	spec_nam			VARCHAR2(256)	DEFAULT NULL,	
	spec_ver			VARCHAR2(256)	DEFAULT NULL,
	flow_id				VARCHAR2(256)	DEFAULT NULL,
	setup_id			VARCHAR2(256)	DEFAULT NULL,
	dsgn_rev			VARCHAR2(256)	DEFAULT NULL,
	eng_id				VARCHAR2(256)	DEFAULT NULL,
	rom_cod				VARCHAR2(256)	DEFAULT NULL,
	serl_num			VARCHAR2(256)	DEFAULT NULL,
	supr_nam			VARCHAR2(256)	DEFAULT NULL,
	nb_sites			NUMBER(3)		DEFAULT 1 NOT NULL,
	head_num			NUMBER(3)		DEFAULT NULL,
	handler_typ			VARCHAR2(255)	DEFAULT NULL,
	handler_id			VARCHAR2(255)	DEFAULT NULL,
	card_typ			VARCHAR2(255)	DEFAULT NULL,
	card_id				VARCHAR2(255)	DEFAULT NULL,
	loadboard_typ		VARCHAR2(255)	DEFAULT NULL,
	loadboard_id		VARCHAR2(255)	DEFAULT NULL,
	dib_typ				VARCHAR2(255)	DEFAULT NULL,
	dib_id				VARCHAR2(255)	DEFAULT NULL,
	cable_typ			VARCHAR2(255)	DEFAULT NULL,
	cable_id			VARCHAR2(255)	DEFAULT NULL,
	contactor_typ		VARCHAR2(255)	DEFAULT NULL,
	contactor_id		VARCHAR2(255)	DEFAULT NULL,
	laser_typ			VARCHAR2(255)	DEFAULT NULL,
	laser_id			VARCHAR2(255)	DEFAULT NULL,
	extra_typ			VARCHAR2(255)	DEFAULT NULL,
	extra_id			VARCHAR2(255)	DEFAULT NULL,
	file_host_id		NUMBER(10)		DEFAULT 0,
	file_path			VARCHAR2(255)	DEFAULT '',
	file_name			VARCHAR2(255)	DEFAULT '',
	valid_splitlot		CHAR(1)			DEFAULT 'N' NOT NULL,
	insertion_time		NUMBER(10)		DEFAULT NULL,
	subcon_lot_id		VARCHAR2(255)	DEFAULT '',
	syl_id				NUMBER(10),
	sbl_id				NUMBER(10),
	CONSTRAINT pk_ft_splitlot PRIMARY KEY(splitlot_id),
	CONSTRAINT fk1_ft_splitlot FOREIGN KEY(lot_id) REFERENCES &GEXADMIN..ft_lot(lot_id),
	CONSTRAINT fk3_ft_splitlot FOREIGN KEY(syl_id) REFERENCES &GEXADMIN..ft_syl_set(syl_id),
	CONSTRAINT fk4_ft_splitlot FOREIGN KEY(sbl_id) REFERENCES &GEXADMIN..ft_sbl_set(sbl_id),
	CONSTRAINT fk5_ft_splitlot FOREIGN KEY(file_host_id) REFERENCES &GEXADMIN..file_host(file_host_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_prod_alarm (
	splitlot_id			NUMBER(10)		NOT NULL,
	alarm_cat			VARCHAR2(255)	NOT NULL,
	alarm_type			VARCHAR2(255)	NOT NULL,
	item_no				NUMBER			NOT NULL,
	item_name			VARCHAR2(255)	DEFAULT NULL,
	flags				CHAR(2)			NOT NULL,
	lcl					NUMBER			DEFAULT 0 NOT NULL,
	ucl					NUMBER			DEFAULT 0 NOT NULL,
	value				NUMBER			DEFAULT 0 NOT NULL,
	units				VARCHAR2(10)	DEFAULT NULL,
	CONSTRAINT fk1_ft_prod_alarm FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_parts_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_parts_good		NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_parts_stats_samples PRIMARY KEY(splitlot_id,site_no),
	CONSTRAINT fk1_ft_parts_stats_samples FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_parts_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	nb_good				NUMBER(8)		DEFAULT NULL,
	nb_rtst				NUMBER(8)		DEFAULT NULL,
	CONSTRAINT pk_ft_parts_stats_summary PRIMARY KEY(splitlot_id,site_no),
	CONSTRAINT fk1_ft_parts_stats_summary FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_hbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	hbin_name			VARCHAR2(255)	DEFAULT '',
	hbin_cat			CHAR(1)			DEFAULT NULL,
	CONSTRAINT pk_ft_hbin PRIMARY KEY(splitlot_id,hbin_no),
	CONSTRAINT fk1_ft_hbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_hbin_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_hbin_stats_samples PRIMARY KEY(splitlot_id,hbin_no,site_no), 
	CONSTRAINT fk1_ft_hbin_stats_samples FOREIGN KEY(splitlot_id,HBIN_NO) REFERENCES &GEXADMIN..ft_hbin(splitlot_id,hbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_hbin_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_hbin_stats_summary PRIMARY KEY(splitlot_id,hbin_no,site_no), 
	CONSTRAINT fk1_ft_hbin_stats_summary FOREIGN KEY(splitlot_id,hbin_no) REFERENCES &GEXADMIN..ft_hbin(splitlot_id,hbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_sbin (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_name			VARCHAR2(255)	DEFAULT '',
	sbin_cat			CHAR(1)			DEFAULT NULL,
	CONSTRAINT pk_ft_sbin PRIMARY KEY(splitlot_id,sbin_no),
	CONSTRAINT fk1_ft_sbin FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_sbin_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	nb_parts			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_sbin_stats_samples PRIMARY KEY(splitlot_id,sbin_no,site_no), 
	CONSTRAINT fk1_ft_sbin_stats_samples FOREIGN KEY(splitlot_id,sbin_no) REFERENCES &GEXADMIN..ft_sbin(splitlot_id,sbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_sbin_stats_summary (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	bin_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_sbin_stats_summary PRIMARY KEY(splitlot_id,sbin_no,site_no), 
	CONSTRAINT fk1_ft_sbin_stats_summary FOREIGN KEY(splitlot_id,sbin_no) REFERENCES &GEXADMIN..ft_sbin(splitlot_id,sbin_no)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_run (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(8)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	part_id				VARCHAR2(255)	DEFAULT NULL,
	part_x				NUMBER(6)		DEFAULT NULL,
	part_y				NUMBER(6)		DEFAULT NULL,
	hbin_no				NUMBER(5)		DEFAULT 0 NOT NULL,
	sbin_no				NUMBER(5)		DEFAULT NULL,
	ttime				NUMBER(10)		DEFAULT NULL,
	tests_executed		NUMBER(5)		DEFAULT 0 NOT NULL,
	tests_failed		NUMBER(5)		DEFAULT 0 NOT NULL,
	firstfail_tnum		NUMBER(10)		DEFAULT NULL,
	firstfail_tname		VARCHAR2(255)	DEFAULT NULL,
	retest_index		NUMBER(3)		DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_run PRIMARY KEY(splitlot_id,run_id),
	CONSTRAINT fk1_ft_run FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_ptest_info (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	tnum				NUMBER(10)		DEFAULT 0 NOT NULL,
	tname				VARCHAR2(255)	DEFAULT '',
	units				VARCHAR2(255)	DEFAULT '',
	flags				CHAR(1)			DEFAULT NULL,
	ll					NUMBER			DEFAULT NULL,
	hl					NUMBER			DEFAULT NULL,
	testseq				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT pk_ft_ptest_info PRIMARY KEY(splitlot_id,ptest_info_id), 
	CONSTRAINT fk1_ft_ptest_info FOREIGN KEY(splitlot_id) REFERENCES &GEXADMIN..ft_splitlot(splitlot_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_ptest_limits (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	ll					NUMBER			DEFAULT 0 NOT NULL,
	hl					NUMBER			DEFAULT 0 NOT NULL,
	CONSTRAINT pk_ft_ptest_limits PRIMARY KEY(splitlot_id,ptest_info_id,site_no), 
	CONSTRAINT fk1_ft_ptest_limits FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..ft_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_ptest_results (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	run_id				NUMBER(8)		DEFAULT 0 NOT NULL,
	flags				CHAR(1)			DEFAULT NULL,
	value				NUMBER			DEFAULT NULL,
	CONSTRAINT pk_ft_ptest_results PRIMARY KEY(splitlot_id,run_id,ptest_info_id), 
	CONSTRAINT fk1_ft_ptest_results FOREIGN KEY(splitlot_id,run_id) REFERENCES &GEXADMIN..ft_run(splitlot_id,run_id),
	CONSTRAINT fk2_ft_ptest_results FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..ft_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..ft_ptest_stats_samples (
	splitlot_id			NUMBER(10)		DEFAULT 0 NOT NULL,
	ptest_info_id		NUMBER(5)		DEFAULT 0 NOT NULL,
	site_no				NUMBER(5)		DEFAULT 1 NOT NULL,
	exec_count			NUMBER(8)		DEFAULT 0 NOT NULL,
	fail_count			NUMBER(8)		DEFAULT NULL,
	min_value			NUMBER			DEFAULT NULL,
	max_value			NUMBER			DEFAULT NULL,
	sum					NUMBER			DEFAULT NULL,
	square_sum			NUMBER			DEFAULT NULL,
	CONSTRAINT pk_ft_ptest_stats_samples PRIMARY KEY(splitlot_id,ptest_info_id,site_no), 
	CONSTRAINT fk1_ft_ptest_stats_samples FOREIGN KEY(splitlot_id,ptest_info_id) REFERENCES &GEXADMIN..ft_ptest_info(splitlot_id,ptest_info_id)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_PTEST_STATS_SUMMARY (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	PTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	EXEC_COUNT			NUMBER(8)		DEFAULT NULL,
	FAIL_COUNT			NUMBER(8)		DEFAULT NULL,
	MIN_VALUE			NUMBER			DEFAULT NULL,
	MAX_VALUE			NUMBER			DEFAULT NULL,
	SUM					NUMBER			DEFAULT NULL,
	SQUARE_SUM			NUMBER			DEFAULT NULL,
	TTIME				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT PK_FT_PTEST_STATS_SUMMARY PRIMARY KEY(SPLITLOT_ID,PTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_PTEST_STATS_SUMMARY FOREIGN KEY(SPLITLOT_ID,PTEST_INFO_ID) REFERENCES &GEXADMIN..FT_PTEST_INFO(SPLITLOT_ID,PTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_MPTEST_INFO (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	MPTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	TNUM				NUMBER(10)		DEFAULT 0 NOT NULL,
	TNAME				VARCHAR2(255)	DEFAULT '',
	TPIN_ARRAYINDEX		NUMBER(6)		DEFAULT 0 NOT NULL,
	UNITS				VARCHAR2(255)	DEFAULT '',
	FLAGS				CHAR(1)			DEFAULT NULL,
	LL					NUMBER			DEFAULT NULL,
	HL					NUMBER			DEFAULT NULL,
	TESTSEQ				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT PK_FT_MPTEST_INFO PRIMARY KEY(SPLITLOT_ID,MPTEST_INFO_ID), 
	CONSTRAINT FK1_FT_MPTEST_INFO FOREIGN KEY(SPLITLOT_ID) REFERENCES &GEXADMIN..FT_SPLITLOT(SPLITLOT_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_MPTEST_LIMITS (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	MPTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	LL					NUMBER			DEFAULT 0 NOT NULL,
	HL					NUMBER			DEFAULT 0 NOT NULL,
	CONSTRAINT PK_FT_MPTEST_LIMITS PRIMARY KEY(SPLITLOT_ID,MPTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_MPTEST_LIMITS FOREIGN KEY(SPLITLOT_ID,MPTEST_INFO_ID) REFERENCES &GEXADMIN..FT_MPTEST_INFO(SPLITLOT_ID,MPTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_MPTEST_RESULTS (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	MPTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	RUN_ID				NUMBER(8)		DEFAULT 0 NOT NULL,
	FLAGS				CHAR(1)			DEFAULT NULL,
	VALUE				NUMBER			DEFAULT 0 NOT NULL,
	TPIN_PMRINDEX		NUMBER(6)		DEFAULT 0,
	CONSTRAINT PK_FT_MPTEST_RESULTS PRIMARY KEY(SPLITLOT_ID,RUN_ID,MPTEST_INFO_ID), 
	CONSTRAINT FK1_FT_MPTEST_RESULTS FOREIGN KEY(SPLITLOT_ID,RUN_ID) REFERENCES &GEXADMIN..FT_RUN(SPLITLOT_ID,RUN_ID),
	CONSTRAINT FK2_FT_MPTEST_RESULTS FOREIGN KEY(SPLITLOT_ID,MPTEST_INFO_ID) REFERENCES &GEXADMIN..FT_MPTEST_INFO(SPLITLOT_ID,MPTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_MPTEST_STATS_SAMPLES (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	MPTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	EXEC_COUNT			NUMBER(8)		DEFAULT 0 NOT NULL,
	FAIL_COUNT			NUMBER(8)		DEFAULT NULL,
	MIN_VALUE			NUMBER			DEFAULT NULL,
	MAX_VALUE			NUMBER			DEFAULT NULL,
	SUM					NUMBER			DEFAULT NULL,
	SQUARE_SUM			NUMBER			DEFAULT NULL,
	CONSTRAINT PK_FT_MPTEST_STATS_SAMPLES PRIMARY KEY(SPLITLOT_ID,MPTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_MPTEST_STATS_SAMPLES FOREIGN KEY(SPLITLOT_ID,MPTEST_INFO_ID) REFERENCES &GEXADMIN..FT_MPTEST_INFO(SPLITLOT_ID,MPTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_MPTEST_STATS_SUMMARY (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	MPTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	EXEC_COUNT			NUMBER(8)		DEFAULT 0 NOT NULL,
	FAIL_COUNT			NUMBER(8)		DEFAULT 0 NOT NULL,
	MIN_VALUE			NUMBER			DEFAULT NULL,
	MAX_VALUE			NUMBER			DEFAULT NULL,
	SUM					NUMBER			DEFAULT NULL,
	SQUARE_SUM			NUMBER			DEFAULT NULL,
	TTIME				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT PK_FT_MPTEST_STATS_SUMMARY PRIMARY KEY(SPLITLOT_ID,MPTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_MPTEST_STATS_SUMMARY FOREIGN KEY(SPLITLOT_ID,MPTEST_INFO_ID) REFERENCES &GEXADMIN..FT_MPTEST_INFO(SPLITLOT_ID,MPTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_FTEST_INFO (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	FTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	TNUM				NUMBER(10)		DEFAULT 0 NOT NULL,
	TNAME				VARCHAR2(255)	DEFAULT '',
	TESTSEQ				NUMBER(5)		DEFAULT NULL,
	CONSTRAINT PK_FT_FTEST_INFO PRIMARY KEY(SPLITLOT_ID,FTEST_INFO_ID), 
	CONSTRAINT FK1_FT_FTEST_INFO FOREIGN KEY(SPLITLOT_ID) REFERENCES &GEXADMIN..FT_SPLITLOT(SPLITLOT_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_FTEST_RESULTS (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	FTEST_INFO_ID		NUMBER(5)		NOT NULL,
	RUN_ID				NUMBER(8)		DEFAULT 0 NOT NULL,
	FLAGS				CHAR(1)			DEFAULT NULL,
	VECT_NAM			VARCHAR2(255)	DEFAULT '',
	VECT_OFF			NUMBER(6)		DEFAULT NULL,
	CONSTRAINT PK_FT_FTEST_RESULTS PRIMARY KEY(SPLITLOT_ID,RUN_ID,FTEST_INFO_ID), 
	CONSTRAINT FK1_FT_FTEST_RESULTS FOREIGN KEY(SPLITLOT_ID,RUN_ID) REFERENCES &GEXADMIN..FT_RUN(SPLITLOT_ID,RUN_ID),
	CONSTRAINT FK2_FT_FTEST_RESULTS FOREIGN KEY(SPLITLOT_ID,FTEST_INFO_ID) REFERENCES &GEXADMIN..FT_FTEST_INFO(SPLITLOT_ID,FTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_FTEST_STATS_SAMPLES (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	FTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	EXEC_COUNT			NUMBER(8)		DEFAULT 0 NOT NULL,
	FAIL_COUNT			NUMBER(8)		DEFAULT NULL,
	CONSTRAINT PK_FT_FTEST_STATS_SAMPLES PRIMARY KEY(SPLITLOT_ID,FTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_FTEST_STATS_SAMPLES FOREIGN KEY(SPLITLOT_ID,FTEST_INFO_ID) REFERENCES &GEXADMIN..FT_FTEST_INFO(SPLITLOT_ID,FTEST_INFO_ID)
) NOLOGGING;

CREATE TABLE &GEXADMIN..FT_FTEST_STATS_SUMMARY (
	SPLITLOT_ID			NUMBER(10)		DEFAULT 0 NOT NULL,
	FTEST_INFO_ID		NUMBER(5)		DEFAULT 0 NOT NULL,
	SITE_NO				NUMBER(5)		DEFAULT 1 NOT NULL,
	EXEC_COUNT			NUMBER(8)		DEFAULT NULL,
	FAIL_COUNT			NUMBER(8)		DEFAULT NULL,
	TTIME				NUMBER(10)		DEFAULT NULL,
	CONSTRAINT PK_FT_FTEST_STATS_SUMMARY PRIMARY KEY(SPLITLOT_ID,FTEST_INFO_ID,SITE_NO), 
	CONSTRAINT FK1_FT_FTEST_STATS_SUMMARY FOREIGN KEY(SPLITLOT_ID,FTEST_INFO_ID) REFERENCES &GEXADMIN..FT_FTEST_INFO(SPLITLOT_ID,FTEST_INFO_ID)
) NOLOGGING;

exit;
