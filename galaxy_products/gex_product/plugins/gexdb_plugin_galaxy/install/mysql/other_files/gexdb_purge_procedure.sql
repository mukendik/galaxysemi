CREATE PROCEDURE `gexdb_purge_procedure`(Samples INT, Stats INT, Entry INT)
BEGIN
	DECLARE DateEraseEntry, DateEraseStats, DateEraseSamples INT;
	DECLARE MinSplitlotIdForErase, MaxSplitlotIdForErase INT;
      SELECT Samples INTO DateEraseSamples;
      SELECT Stats INTO DateEraseStats;
      SELECT Entry INTO DateEraseEntry;
	# Display current value
      SELECT Samples;
      SELECT Stats;
      SELECT Entry;

	# Total purge
	# verify if have TotalPurge information else skip this step
	IF NOT (DateEraseEntry IS NULL)
	THEN
		# FOR FT_TABLES
		# Get the Splitlot_Id where to start Purge
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM ft_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'TOTAL PURGE for ft_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM ft_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ftest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_mptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_mptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_splitlot WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;

		# FOR WT_TABLES
		# Get the Splitlot_Id where to start Purge
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM wt_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
	           SELECT 'TOTAL PURGE for wt_splitlot';
	           SELECT MinSplitlotIdForErase;
	           SELECT MaxSplitlotIdForErase;

			DELETE FROM wt_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ftest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_mptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_mptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_wafer_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_splitlot WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;

		# FOR ET_TABLES
		# Get the Splitlot_Id where to start Purge
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM et_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
	           SELECT 'TOTAL PURGE for et_splitlot';
	           SELECT MinSplitlotIdForErase;
	           SELECT MaxSplitlotIdForErase;
			DELETE FROM et_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_parts WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_wafer_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_splitlot WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;
	END IF;
	# Stats purge
	# verify if have Stats information else skip this step
	IF NOT (DateEraseStats IS NULL)
	THEN
		# FOR FT_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM ft_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'STATS PURGE for ft_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM ft_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_ftest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_mptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_mptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;

		# FOR WT_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM wt_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'STATS PURGE for wt_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM wt_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_ftest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_mptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_mptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;
	
		# FOR E_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM et_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseStats*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'STATS PURGE for et_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM et_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_ptest_limits WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_ptest_info WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_samples_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_stats_summary_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_sbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_hbin WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;	
	END IF;

	# Samples purge
	# verify if have Samplesge information else skip this step
	IF NOT (DateEraseSamples IS NULL)
	THEN
		# FOR FT_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM ft_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM ft_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'SAMPLES PURGE for ft_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM ft_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM ft_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;

		# FOR WT_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM wt_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM wt_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'SAMPLES PURGE for wt_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM wt_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_ftest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_result_mptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM wt_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;
	
		# FOR E_TABLES
		# Get the Splitlot_Id where to start TotalPurge
		SELECT max(splitlot_id) FROM et_splitlot
			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
			INTO MaxSplitlotIdForErase;

		IF NOT (MaxSplitlotIdForErase IS NULL)
		THEN
			SELECT min(splitlot_id) FROM et_splitlot
				WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)
				INTO MinSplitlotIdForErase;
			# have the min and the max splitlot_id for delete
	
			SELECT 'SAMPLES PURGE for et_splitlot';
			SELECT MinSplitlotIdForErase;
			SELECT MaxSplitlotIdForErase;
			DELETE FROM et_result_ptest WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
			DELETE FROM et_run WHERE MinSplitlotIdForErase <= splitlot_id AND splitlot_id <= MaxSplitlotIdForErase;
		END IF;	
	END IF;

END