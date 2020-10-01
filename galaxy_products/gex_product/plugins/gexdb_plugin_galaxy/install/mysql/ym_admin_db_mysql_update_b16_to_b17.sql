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
CAUTION: when running manually that script make sure to source the script common_update_initialize.sql before and common_update_finalize.sql after


-- ---------------------------------------------------------
-- Update DB version
-- ---------------------------------------------------------
UPDATE ym_settings SET value='5.0' WHERE field='DB_VERSION_NB';
UPDATE ym_settings SET value='17' WHERE field='DB_BUILD_NB';
