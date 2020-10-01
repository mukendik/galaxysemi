--
-- WAFER_INFO insert TRIGGER
--
DROP TRIGGER IF EXISTS gexdb_new.wt_wafer_info_insert_trigger;
DROP TRIGGER IF EXISTS gexdb_new.wt_wafer_info_update_trigger;

-- GRANT TRIGGER right to gexdb admin
GRANT TRIGGER ON gexdb_new.* TO 'gexdb_new'@'%' IDENTIFIED BY 'gexadmin_new'

