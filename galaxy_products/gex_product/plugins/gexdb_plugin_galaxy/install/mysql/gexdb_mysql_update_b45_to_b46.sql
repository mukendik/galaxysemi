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
CALL is_compatible_version(45, @status, @message);
CALL start_update('GEXDB V5.02 B46 (MySQL)', 502, 46, @status, @message);

-- GCORE-6330
-- Fake V7.5 update to reserve some TDR build numbers for V7.4 [39 to 49]

-- ---------------------------------------------------------
-- STOP UPDATE
-- ---------------------------------------------------------
CALL stop_update(@status, @message);
