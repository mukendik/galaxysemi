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
CALL is_compatible_version(26, @status, @message);
CALL start_update('GEXDB V3.03 B27 (MySQL)', 303, 27, @status, @message);
CALL add_status('UPDATING_CONSOLIDATION_TRIGGERS', @status, @message);
CALL add_status('UPDATING_CONSOLIDATION_TABLES', @status, @message);
CALL add_status('UPDATING_CONSOLIDATION_PROCEDURES', @status, @message);
CALL add_status('UPDATING_INDEXES', @status, @message);

-- ---------------------------------------------------------
-- CHECK UPDATE DATA PROCEDURE
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS update_b26_to_b27_data;
DELIMITER $$
CREATE PROCEDURE update_b26_to_b27_data(
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
       SET OUT_message="Error SQL Exception";
    SET OUT_status=0;
-- Copy ft_splitlot.lot_id into ft_splitlot.sublot_id
-- The original ft_splitlot.sublot_id will be lost.
    UPDATE ft_splitlot set sublot_id = lot_id;

-- Copy ft_lot into ft_sublot_info
-- using ft_sublot_info.sublot_id=ft_lot.lot_id (normal copy for the other fields)
    INSERT into ft_sublot_info 
    SELECT 
        lot_id as lot_id, 
        lot_id as sublot_id,
        product_name as product_name,
        nb_parts as nb_parts,
        nb_parts_good as nb_parts_good,
        flags as flags
    from ft_lot;

-- Copy ft_lot_hbin into ft_sublot_hbin
-- using ft_sublot_hbin.sublot_id=ft_lot_hbin.lot_id (normal copy for the other fields)
    INSERT into ft_sublot_hbin 
    SELECT 
        lot_id as lot_id, 
        lot_id as sublot_id,
        hbin_no as hbin_no,
        hbin_name as hbin_name,
        hbin_cat as hbin_cat,
        nb_parts as nb_parts
    from ft_lot_hbin;
 
-- Copy ft_lot_sbin into ft_sublot_sbin
-- using ft_sublot_sbin.sublot_id=ft_lot_sbin.lot_id (normal copy for the other fields)
    INSERT into ft_sublot_sbin 
    SELECT 
        lot_id as lot_id, 
        lot_id as sublot_id,
        sbin_no as sbin_no,
        sbin_name as sbin_name,
        sbin_cat as sbin_cat,
        nb_parts as nb_parts
    from ft_lot_sbin;
    SET OUT_status=1;
END $$
DELIMITER ;


-- ---------------------------------------------------------
-- Table structure for table ft_sublot_info
-- ---------------------------------------------------------
DROP TABLE IF EXISTS ft_sublot_info;
CREATE TABLE ft_sublot_info (
 lot_id varchar(255) NOT NULL DEFAULT '',
 sublot_id varchar(255) NOT NULL DEFAULT '',
 product_name varchar(255) DEFAULT NULL,
 nb_parts mediumint(8) NOT NULL DEFAULT '0',
 nb_parts_good mediumint(8) NOT NULL DEFAULT '0',
 flags binary(2) DEFAULT NULL,
 PRIMARY KEY (lot_id,sublot_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;
CALL update_log_message('CREATE TABLE','ft_sublot_info','DONE',null);

-- ---------------------------------------------------------
-- Table structure for table ft_sublot_hbin
-- ---------------------------------------------------------
DROP TABLE IF EXISTS ft_sublot_hbin;
CREATE TABLE ft_sublot_hbin (
 lot_id varchar(255) NOT NULL,
 sublot_id varchar(255) DEFAULT NULL,
 hbin_no smallint(5) unsigned NOT NULL DEFAULT '0',
 hbin_name varchar(255) NOT NULL DEFAULT '',
 hbin_cat char(1) DEFAULT NULL,
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0',
 PRIMARY KEY (lot_id,sublot_id,hbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;
CALL update_log_message('CREATE TABLE','ft_sublot_hbin','DONE',null);

-- ---------------------------------------------------------
-- Table structure for table ft_sublot_sbin
-- ---------------------------------------------------------
DROP TABLE IF EXISTS ft_sublot_sbin;
CREATE TABLE ft_sublot_sbin (
 lot_id varchar(255) NOT NULL,
 sublot_id varchar(255) DEFAULT NULL,
 sbin_no smallint(5) unsigned NOT NULL DEFAULT '0',
 sbin_name varchar(255) NOT NULL DEFAULT '',
 sbin_cat char(1) DEFAULT NULL,
 nb_parts mediumint(8) unsigned NOT NULL DEFAULT '0',
 PRIMARY KEY (lot_id,sublot_id,sbin_no)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ;
CALL update_log_message('CREATE TABLE','ft_sublot_sbin','DONE',null);

-- ---------------------------------------------------------
-- UPDATE DATA
-- ---------------------------------------------------------
CALL update_b26_to_b27_data(@status, @message);
DROP PROCEDURE IF EXISTS update_b26_to_b27_data;


-- ---------------------------------------------------------
-- STOP UPDATE
-- ---------------------------------------------------------
CALL stop_update(@status, @message);





