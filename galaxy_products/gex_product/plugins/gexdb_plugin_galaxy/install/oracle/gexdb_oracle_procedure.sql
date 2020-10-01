CREATE PROCEDURE GexDB."GalaxyDb_CleanUpProcedure"
 (
Samples NUMBER, 
Stats NUMBER, 
EntryId NUMBER
)
IS
BEGIN
	DECLARE 
	DateEraseEntry NUMBER;
	DateEraseStats NUMBER;
	DateEraseSamples NUMBER;
	DateSetup NUMBER;
	HaveSplitlotIdForErase NUMBER;
	SplitlotForErase NUMBER;
BEGIN
	DateEraseSamples := Samples;
	DateEraseStats := Stats;
	DateEraseEntry := EntryId;

	-- Total purge
	-- verify if have TotalPurge information else skip this step
	-- FOR FT_TABLES
	-- Verify if have Purge to do
	IF (DateEraseEntry = 0)
	THEN
		HaveSplitlotIdForErase := NULL;
	ELSE
	DateSetup := UNIX_TIMESTAMP() - (DateEraseEntry*24*60*60*100);
	SELECT max(splitlot_id) 
		INTO HaveSplitlotIdForErase FROM ft_splitlot
		WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N');
	END IF;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE 
		CURSOR curSplitlot IS SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N');
		BEGIN
    OPEN curSplitlot;

		LOOP
		  FETCH curSplitlot INTO SplitlotForErase;
			IF curSplitlot%NOTFOUND THEN EXIT; END IF;
			DELETE FROM ft_result_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_result_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_result_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_samples_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_stats_summary_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM ft_splitlot WHERE splitlot_id = SplitlotForErase;
		END LOOP;
		CLOSE curSplitlot;
		END;
	END; END IF;

	-- FOR WT_TABLES
	-- Verify if have Purge to do
	SELECT max(splitlot_id) FROM wt_splitlot
		WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N')
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N');
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM wt_result_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_result_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_result_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_ftest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_mptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_samples_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_stats_summary_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_wafer_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM wt_splitlot WHERE splitlot_id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	-- FOR ET_TABLES
	-- Verify if have Purge to do
	SELECT max(splitlot_id) FROM et_splitlot
		WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N')
		INTO HaveSplitlotIdForErase;

	IF NOT (HaveSplitlotIdForErase IS NULL)
	THEN BEGIN
		DECLARE not_found BOOLEAN DEFAULT false;
		DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t)) OR (valid_splitlot = 'N');
		DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
		OPEN curSplitlot;
		SET not_found = false;
		REPEAT
			FETCH curSplitlot INTO SplitlotForErase;
			DELETE FROM et_result_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_ptest_limits WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_samples_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_stats_summary_parts WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_wafer_info WHERE splitlot_id = SplitlotForErase;
			DELETE FROM et_splitlot WHERE splitlot_id = SplitlotForErase;
		UNTIL not_found = true	END REPEAT;
		CLOSE curSplitlot;
	END; END IF;

	-- Stats purge
	-- verify if have Stats information else skip this step
	IF NOT (DateEraseStats IS NULL)
	THEN
		-- FOR FT_TABLES
		-- Verify if have Purge to do
		SELECT UNIX_TIMESTAMP() - (DateEraseStats*24*60*60*100) INTO DateSetup;
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_result_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_result_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_summary_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_samples_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_ftest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_summary_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_samples_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_mptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		-- FOR WT_TABLES
		-- Verify if have Purge to do
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_result_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_result_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_summary_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_samples_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_ftest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_summary_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_samples_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_mptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		-- FOR E_TABLES
		-- Verify if have Purge to do
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_summary_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_samples_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_ptest_limits WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_ptest_info WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_samples_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_summary_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_samples_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_stats_summary_hbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_sbin WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_hbin WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	END IF;

	-- Samples purge
	-- verify if have Samplesge information else skip this step
	IF NOT (DateEraseSamples IS NULL)
	THEN
		# FOR FT_TABLES
		# Verify if have Purge to do
		SELECT UNIX_TIMESTAMP() - (DateEraseSamples*24*60*60*100) INTO DateSetup;
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM ft_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM ft_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_result_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_result_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM ft_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		-- FOR WT_TABLES
		-- Verify if have Purge to do
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM wt_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM wt_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_result_ftest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_result_mptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM wt_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;

		-- FOR E_TABLES
		-- Verify if have Purge to do
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  DateSetup > UNIX_TIMESTAMP(setup_t)
			INTO HaveSplitlotIdForErase;

		IF NOT (HaveSplitlotIdForErase IS NULL)
		THEN BEGIN
			DECLARE not_found BOOLEAN DEFAULT false;
			DECLARE curSplitlot CURSOR FOR SELECT splitlot_id FROM et_splitlot WHERE  (DateSetup > UNIX_TIMESTAMP(setup_t));
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;
			OPEN curSplitlot;
			SET not_found = false;
			REPEAT
				FETCH curSplitlot INTO SplitlotForErase;
				DELETE FROM et_result_ptest WHERE splitlot_id = SplitlotForErase;
				DELETE FROM et_run WHERE splitlot_id = SplitlotForErase;
			UNTIL not_found = true	END REPEAT;
			CLOSE curSplitlot;
		END; END IF;
	END IF;

END;
END;
