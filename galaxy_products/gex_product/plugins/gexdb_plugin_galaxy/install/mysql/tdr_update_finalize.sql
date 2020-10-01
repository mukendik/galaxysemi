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

-- CALL stop_global_tdr_update( @status, @message);

-- ---------------------------------------------------------
-- DROP COMMON PROCEDURES/FUNCTIONS
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS is_compatible_version;
DROP PROCEDURE IF EXISTS check_table_for_update;
DROP PROCEDURE IF EXISTS start_update;
DROP PROCEDURE IF EXISTS stop_update;
DROP PROCEDURE IF EXISTS add_status;
DROP PROCEDURE IF EXISTS remove_status;
DROP PROCEDURE IF EXISTS stop_update;
