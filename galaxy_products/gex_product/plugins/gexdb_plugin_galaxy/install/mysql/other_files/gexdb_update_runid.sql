select curdate(), curtime();
ALTER TABLE wt_run MODIFY run_id mediumint(7) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_ptest_results MODIFY run_id mediumint(7) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_ftest_results MODIFY run_id mediumint(7) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_mptest_results MODIFY run_id mediumint(7) unsigned	NOT NULL DEFAULT '0';
select curdate(), curtime();
