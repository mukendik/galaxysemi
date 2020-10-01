select curdate(), curtime();
ALTER TABLE wt_run MODIFY RUN_ID mediumint(8) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_ptest_results MODIFY RUN_ID mediumint(8) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_ftest_results MODIFY RUN_ID mediumint(8) unsigned	NOT NULL DEFAULT '0';
ALTER TABLE wt_mptest_results MODIFY RUN_ID mediumint(8) unsigned	NOT NULL DEFAULT '0';
select curdate(), curtime();
