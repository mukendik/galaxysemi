-- ---------------------------------------------------------
-- DO NOT EDIT THIS FILE UNLESS: 
-- o To customize the update 
-- o To execute it manually
-- ---------------------------------------------------------



-- ---------------------------------------------------------
-- Caution messages below are not commented to make sure that 
-- scripts are in the right state before to be ran manually
-- ---------------------------------------------------------
CAUTION: when running manually that script make sure to uncomment the line below to specify a working database
-- use database_name_xx;
CAUTION: when running manually that script make sure to comment each lines: "DECLARE EXIT HANDLER FOR SQLEXCEPTION" (if it exists) to be sure to catch all errors
CAUTION: when running manually that script make sure to source the script common_update_initialize.sql, then admin_update_initialize.sql before
CAUTION: when running manually that script make sure to source the script admin_update_finalize.sql, then common_update_finalize.sql after
CAUTION: 4. After running the last TDR update script, run the script tdr_update_finalize.sql, then common_update_finalize.sql.

-- ---------------------------------------------------------
-- START UPDATE
-- ---------------------------------------------------------
CALL is_admin_compatible_version(22, @status, @message);
CALL start_admin_update('Yield-Man Administration Server', 23, '5.5', @status, @message);

-- ---------------------------------------------------------
-- Update ym_admin_db definition
-- ---------------------------------------------------------

-- GCORE-7845
-- Create fake update of the YmAdminDB from B19 to B24

-- ---------------------------------------------------------
-- STOP UPDATE
-- ---------------------------------------------------------
CALL stop_admin_update(@status, @message);

