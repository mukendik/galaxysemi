-- ---------------------------------------------------------
-- DO NOT EDIT THIS FILE UNLESS:
-- o To customize the update
-- o To execute it manually
-- ---------------------------------------------------------

-- ---------------------------------------------------------
-- CAUTION/INFO/QUESTION messages below are not commented to make sure that
-- scripts are in the right state before to be ran manually
-- ---------------------------------------------------------
CAUTION: When manually running this script, perform the following four steps:
CAUTION: 1. Uncomment the line below and specify a working database.
-- use database_name_xx;
CAUTION: 2. To catch all errors, comment each line "DECLARE EXIT HANDLER FOR SQLEXCEPTION" (if it exists) as shown below:
CAUTION: -- DECLARE EXIT HANDLER FOR SQLEXCEPTION
CAUTION: 3. Before running the first TDR update script, run the script common_update_initialize.sql, then tdr_update_initialize.sql.
CAUTION: 4. After running the last TDR update script, run the script tdr_update_finalize.sql, then common_update_finalize.sql.

-- ---------------------------------------------------------
-- START UPDATE
-- ---------------------------------------------------------
CALL is_compatible_version(49, @status, @message);
CALL start_update('GEXDB V5.03 B50 (MySQL)', 503, 50, @status, @message);

CREATE TABLE IF NOT EXISTS ft_gtl_splitlot (
  gtl_splitlot_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  session_id bigint(4) NOT NULL DEFAULT '0',
  sqlite_splitlot_id int(10) unsigned NOT NULL,
  splitlot_id int(10) unsigned NOT NULL,
  lot_id varchar(255) NOT NULL DEFAULT '',
  sublot_id varchar(255) NOT NULL DEFAULT '',
  setup_t int(10) NOT NULL DEFAULT '0',
  start_t int(10) NOT NULL DEFAULT '0',
  finish_t int(10) NOT NULL DEFAULT '0',
  stat_num tinyint(3) unsigned NOT NULL DEFAULT '0',
  tester_name varchar(255) NOT NULL DEFAULT '',
  tester_type varchar(255) NOT NULL DEFAULT '',
  flags binary(2) NOT NULL DEFAULT '\0\0',
  nb_parts mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_samples mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_samples_good mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_summary mediumint(8) NOT NULL DEFAULT '0',
  nb_parts_summary_good mediumint(8) NOT NULL DEFAULT '0',
  data_provider varchar(255) DEFAULT '',
  data_type varchar(255) DEFAULT '',
  prod_data char(1) NOT NULL DEFAULT 'Y',
  retest_phase varchar(255) DEFAULT NULL,
  retest_index tinyint(3) unsigned NOT NULL DEFAULT '0',
  retest_hbins varchar(255) DEFAULT NULL,
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
  recipe_id int(10) unsigned,
  PRIMARY KEY (gtl_splitlot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;

DROP PROCEDURE IF EXISTS ft_gtl_insertion_validation;

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

CALL add_column_if_not_exists(Database(), 'ft_ptest_rollinglimits',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_ptest_rollingstats',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_mptest_rollinglimits',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_mptest_rollingstats',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_event',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_ptest_outliers',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);
CALL add_column_if_not_exists(Database(), 'ft_mptest_outliers',
 'gtl_splitlot_id', 'int(10) unsigned DEFAULT NULL', @status, @message);

CALL change_column(Database(), 'ft_event',
 'run_id', 'mediumint(8) NOT NULL', @status, @message);

DROP TABLE IF EXISTS ft_gtl_info;
CREATE TABLE ft_gtl_info (
  gtl_splitlot_id int(10) unsigned NOT NULL,
  gtl_key varchar(255) NOT NULL,
  gtl_value varchar(255),
  PRIMARY KEY (gtl_splitlot_id,gtl_key)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;

-- ---------------------------------------------------------
-- STOP UPDATE
-- ---------------------------------------------------------
CALL stop_update(@status, @message);
