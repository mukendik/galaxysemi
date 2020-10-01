--
-- Create schema gexdb
--


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


--
-- PURGE PROCEDURE
--

DROP PROCEDURE IF EXISTS purge_invalid_splitlots;
DELIMITER $$
CREATE PROCEDURE purge_invalid_splitlots()
BEGIN
	CALL purge_splitlots(null, null, null);
END $$
DELIMITER ;

DROP PROCEDURE IF EXISTS purge_splitlots;
DELIMITER $$
CREATE PROCEDURE purge_splitlots
(
	SamplesNbWeeks	INT, 	# Nb of weeks before delete samples
	StatsNbWeeks	INT, 	# Nb of weeks before delete stats
	EntriesNbWeeks	INT		# Nb of weeks before delete entries
)
BEGIN
	DECLARE DateEraseEntries, DateEraseStats, DateEraseSamples INT;
	DECLARE DateSetup BIGINT;
	DECLARE HaveSplitlotIdForErase, SplitlotForErase INT;

	SELECT (SamplesNbWeeks*7) * (24*60*60) INTO DateEraseSamples;
	SELECT (StatsNbWeeks  *7) * (24*60*60) INTO DateEraseStats;
	SELECT (EntriesNbWeeks*7) * (24*60*60) INTO DateEraseEntries;

	# Total purge
	# verify if have TotalPurge information else skip this step
	# FOR FT_TABLES
	# Verify if have Purge to do
	IF (EntriesNbWeeks IS NULL) THEN
		# Only erase Invalid Splitlot older than 3mn
		SELECT 0 INTO DateSetup;
	ELSE
		SELECT UNIX_TIMESTAMP() - DateEraseEntries INTO DateSetup;
	END IF;
	SELECT max(splitlot_id) FROM ft_splitlot
		WHERE  (DateSetup > INSERTION_TIME) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_parts_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_parts_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_prod_alarm WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_splitlot WHERE splitlot_id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# FOR WT_TABLES
	# Verify if have Purge to do
	SELECT max(splitlot_id) FROM wt_splitlot
		WHERE  (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_parts_stats_samples WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_parts_stats_summary WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_prod_alarm WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_splitlot WHERE splitlot_id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# FOR ET_TABLES
	# Verify if have Purge to do
	SELECT max(splitlot_id) FROM et_splitlot
		WHERE  (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ))
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > insertion_time) OR ((valid_splitlot = 'N') AND ((UNIX_TIMESTAMP() - 180) > insertion_time ));
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_ptest_stats WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_prod_alarm WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_splitlot WHERE splitlot_id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	# stats purge
	# verify if have stats information else skip this step
	IF NOT (StatsNbWeeks IS NULL)
	THEN
		# FOR FT_TABLES
		# Verify if have Purge to do
		SELECT UNIX_TIMESTAMP() - DateEraseStats INTO DateSetup;
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;	
				DELETE FROM ft_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		# FOR WT_TABLES
		# Verify if have Purge to do
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_sbin_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_sbin_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_hbin_stats_samples WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_hbin_stats_summary WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	
		# FOR E_TABLES
		# Verify if have Purge to do
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_ptest_stats WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;	
	END IF;

	# SamplesNbWeeks purge
	# verify if have Samplesge information else skip this step
	IF NOT (SamplesNbWeeks IS NULL)
	THEN
		# FOR FT_TABLES
		# Verify if have Purge to do
		SELECT UNIX_TIMESTAMP() - DateEraseSamples INTO DateSetup;
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		# FOR WT_TABLES
		# Verify if have Purge to do
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	
		# FOR E_TABLES
		# Verify if have Purge to do
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  DateSetup > insertion_time
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > insertion_time);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_ptest_results WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;	
	END IF;

END $$
DELIMITER ;

DROP PROCEDURE IF EXISTS et_insertion_validation;
DELIMITER $$
CREATE PROCEDURE et_insertion_validation(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS et_insertion_status;
DELIMITER $$
CREATE PROCEDURE et_insertion_status(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS et_filearchive_settings;
DELIMITER $$
CREATE PROCEDURE et_filearchive_settings(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
BEGIN
	SELECT '' INTO MovePath From dual;
	SELECT '' INTO FtpServer From dual;
	SELECT 21 INTO FtpPort FROM dual;
	SELECT '' INTO FtpUser From dual;
	SELECT '' INTO FtpPassword From dual;
	SELECT '' INTO FtpPath From dual;
	SELECT 0 INTO UseArchiveSettings FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS wt_insertion_validation;
DELIMITER $$
CREATE PROCEDURE wt_insertion_validation(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS wt_insertion_status;
DELIMITER $$
CREATE PROCEDURE wt_insertion_status(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS wt_filearchive_settings;
DELIMITER $$
CREATE PROCEDURE wt_filearchive_settings(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
BEGIN
	SELECT '' INTO MovePath From dual;
	SELECT '' INTO FtpServer From dual;
	SELECT 21 INTO FtpPort FROM dual;
	SELECT '' INTO FtpUser From dual;
	SELECT '' INTO FtpPassword From dual;
	SELECT '' INTO FtpPath From dual;
	SELECT 0 INTO UseArchiveSettings FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS ft_insertion_validation;
DELIMITER $$
CREATE PROCEDURE ft_insertion_validation(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be validated
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be validated
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be validated
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	OUT TrackingLotID_Out	VARCHAR(1024),	-- Tracking lot to be used in GexDB for this splitlot
	OUT LotID_Out			VARCHAR(1024),	-- Lot to be used in GexDB for this splitlot
	OUT ProductName			VARCHAR(1024),	-- Return the Product Name if it has to be overloaded
	OUT Message				VARCHAR(1024),	-- Return the Error message in case the validation fails
	OUT Status				INT)			-- Return the validation status: 1 if validation successful, 0 else
BEGIN
	SELECT TrackingLotID INTO TrackingLotID_Out From dual;
	SELECT LotID INTO LotID_Out From dual;
	SELECT 'Success' INTO Message From dual;
	SELECT 1 INTO Status FROM dual;
END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS ft_insertion_status;
DELIMITER $$
CREATE PROCEDURE ft_insertion_status(
	IN Splitlot			INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID	VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID			VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID			VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	IN Message			VARCHAR(1024),	-- Error message in case the insertion failed
	IN Status			INT)			-- Insertion status: 1 if insertion successful, 0 else
BEGIN

END $$                                                                     
DELIMITER ;

DROP PROCEDURE IF EXISTS ft_filearchive_settings;
DELIMITER $$
CREATE PROCEDURE ft_filearchive_settings(
	IN Splitlot				INT,			-- SplitlotId of the splitlot to be inserted
	IN TrackingLotID		VARCHAR(1024),	-- Tracking lot of the splitlot to be inserted
	IN LotID				VARCHAR(1024),	-- Lot of the splitlot to be inserted
	IN WaferID				VARCHAR(1024),	-- WaferID of the splitlot to be inserted (not used for FT)
	OUT UseArchiveSettings	INT,			-- Return 1 if the Archivesettings should be used, 0 else
	OUT MovePath			VARCHAR(1024),	-- Return the path to use if the file should be moved after insertion (DataPump settings)
	OUT FtpPort				INT,			-- Return the Ftp port to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpServer			VARCHAR(1024),	-- Return the Ftp server to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpUser				VARCHAR(1024),	-- Return the Ftp user to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPassword			VARCHAR(1024),	-- Return the Ftp password to use if the file should be Ftp'ed after insertion (DataPump settings)
	OUT FtpPath				VARCHAR(1024))	-- Return the Ftp path to use if the file should be Ftp'ed after insertion (DataPump settings)
BEGIN
	SELECT '' INTO MovePath From dual;
	SELECT '' INTO FtpServer From dual;
	SELECT 21 INTO FtpPort FROM dual;
	SELECT '' INTO FtpUser From dual;
	SELECT '' INTO FtpPassword From dual;
	SELECT '' INTO FtpPath From dual;
	SELECT 0 INTO UseArchiveSettings FROM dual;
END $$                                                                     
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
