--
-- WAFER_INFO insert TRIGGER
--
DROP TRIGGER IF EXISTS wt_wafer_info_insert_trigger;
DELIMITER $$
CREATE TRIGGER wt_wafer_info_insert_trigger AFTER INSERT ON wt_wafer_info
FOR EACH ROW
BEGIN
	call wt_consolidated_wafer_update(NEW.lot_id, NEW.wafer_id);
END $$                                                                     
DELIMITER ;

--
-- WAFER_INFO update TRIGGER
--
DROP TRIGGER IF EXISTS wt_wafer_info_update_trigger;
DELIMITER $$
CREATE TRIGGER wt_wafer_info_update_trigger AFTER UPDATE ON wt_wafer_info
FOR EACH ROW
BEGIN
	call wt_consolidated_wafer_update(NEW.lot_id, NEW.wafer_id);
END $$                                                                     
DELIMITER ;
